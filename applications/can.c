/*
 * 程序清单：这是一个 CAN 设备使用例程
 * 例程导出了 can_test 命令到控制终端
 * 命令调用格式：can_test can1
 * 命令解释：命令第二个参数是要使用的 CAN 设备名称，为空则使用默认的 CAN 设备
 * 程序功能：通过 CAN 设备发送一帧，并创建一个线程接收数据然后打印输出。
*/

#include <rtthread.h>
#include "rtdevice.h"
#include "can.h"
#include "WS2812B.h"
#include "ota_handle.h"
#include <string.h>
#define CAN_DEV_NAME       "can1"      /* CAN 设备名称 */
#define NP_DIAG_RECORD_COUNT              256
#define NP_DIAG_AFTER_RF_MS                3000
#define NP_DIAG_CAP_DISCONNECT_HIGH       20000

static struct rt_semaphore rx_sem;     /* 用于接收消息的信号量 */
static rt_device_t can_dev;            /* CAN 设备句柄 */
extern SEND_INFO_t g_send_info;
MSG_CAN_MAIN_HEART_STRUCT MSG_MAIN_HEART;
MSG_CAN_MAIN_RF_INFO_STRUCT MSG_MAIN_RF_INFO;
MSG_CAN_MAIN_RGB_INFO_STRUCT MSG_MAIN_RGB_INFO;
rt_mq_t  can_tx_mq; //
rt_timer_t can_inform_timer;
uint8_t ota_start_flag =0 ;//OTA升级标志
extern uint8_t BOARD_SWITCH_FLAG ;

typedef enum
{
    NP_DIAG_RF_ON = 0,
    NP_DIAG_RF_OFF,
    NP_DIAG_LCR_SAMPLE,
    NP_DIAG_STATE_CHANGE,
    NP_DIAG_MQ_PUT,
    NP_DIAG_CAN_TX_OK,
    NP_DIAG_CAN_TX_FAIL,
    NP_DIAG_MQ_RESET
} NP_DIAG_EVENT_t;

typedef struct
{
    rt_tick_t tick;
    uint32_t seq;
    uint32_t value1;
    uint32_t value2;
    uint16_t queue_depth;
    uint8_t event;
    uint8_t rf_state;
    uint8_t neg_state;
} NP_DIAG_RECORD_t;

static NP_DIAG_RECORD_t g_np_diag_records[NP_DIAG_RECORD_COUNT];
static uint16_t g_np_diag_write_index;
static uint16_t g_np_diag_record_count;
static uint32_t g_np_diag_overwrite_count;
static uint32_t g_np_diag_seq;
static rt_tick_t g_np_diag_until_tick;
static rt_tick_t g_np_diag_rf_on_tick;
static rt_tick_t g_np_diag_rf_off_tick;
static uint8_t g_np_diag_after_rf_active;
static uint8_t g_np_diag_dumping;

static uint16_t negative_plate_diag_queue_depth(void)
{
    return can_tx_mq == RT_NULL ? 0 : can_tx_mq->entry;
}

static uint8_t negative_plate_diag_is_active(void)
{
    rt_tick_t now;

    if(MSG_MAIN_RF_INFO.RF_switch)
    {
        return 1;
    }

    if(!g_np_diag_after_rf_active)
    {
        return 0;
    }

    now = rt_tick_get();
    if((rt_int32_t)(g_np_diag_until_tick - now) > 0)
    {
        return 1;
    }

    g_np_diag_after_rf_active = 0;
    return 0;
}

static void negative_plate_diag_clear(void)
{
    rt_base_t level = rt_hw_interrupt_disable();

    g_np_diag_write_index = 0;
    g_np_diag_record_count = 0;
    g_np_diag_overwrite_count = 0;
    g_np_diag_seq = 0;
    g_np_diag_rf_on_tick = 0;
    g_np_diag_rf_off_tick = 0;
    g_np_diag_after_rf_active = 0;
    rt_hw_interrupt_enable(level);
}

static void negative_plate_diag_record(NP_DIAG_EVENT_t event, uint32_t seq,
                                       uint8_t neg_state, uint32_t value1,
                                       uint32_t value2, uint8_t force)
{
    NP_DIAG_RECORD_t *record;
    rt_base_t level;

    if(g_np_diag_dumping || (!force && !negative_plate_diag_is_active()))
    {
        return;
    }

    level = rt_hw_interrupt_disable();
    record = &g_np_diag_records[g_np_diag_write_index];
    record->tick = rt_tick_get();
    record->seq = seq;
    record->value1 = value1;
    record->value2 = value2;
    record->queue_depth = negative_plate_diag_queue_depth();
    record->event = (uint8_t)event;
    record->rf_state = MSG_MAIN_RF_INFO.RF_switch ? 1 : 0;
    record->neg_state = neg_state;

    g_np_diag_write_index = (g_np_diag_write_index + 1) % NP_DIAG_RECORD_COUNT;
    if(g_np_diag_record_count < NP_DIAG_RECORD_COUNT)
    {
        g_np_diag_record_count++;
    }
    else
    {
        g_np_diag_overwrite_count++;
    }
    rt_hw_interrupt_enable(level);
}

static uint32_t negative_plate_diag_next_seq(void)
{
    uint32_t seq;
    rt_base_t level = rt_hw_interrupt_disable();

    seq = ++g_np_diag_seq;
    rt_hw_interrupt_enable(level);
    return seq;
}

void negative_plate_diag_lcr_sample(uint32_t raw_cap, uint32_t filtered_cap, uint8_t neg_state)
{
    static uint8_t rf_sample_divider;

    if(!negative_plate_diag_is_active())
    {
        return;
    }

    if(MSG_MAIN_RF_INFO.RF_switch &&
       raw_cap >= NP_DIAG_CAP_DISCONNECT_HIGH &&
       filtered_cap >= NP_DIAG_CAP_DISCONNECT_HIGH)
    {
        rf_sample_divider++;
        if(rf_sample_divider < 10)
        {
            return;
        }
        rf_sample_divider = 0;
    }

    negative_plate_diag_record(NP_DIAG_LCR_SAMPLE, 0, neg_state,
                               raw_cap, filtered_cap, 0);
}

void negative_plate_diag_state_change(uint32_t cap, uint8_t old_state, uint8_t new_state)
{
    negative_plate_diag_record(NP_DIAG_STATE_CHANGE, 0, new_state, cap,
                               ((uint32_t)old_state << 8) | new_state, 1);
}

static const char *negative_plate_diag_state_name(uint8_t state)
{
    static const char *state_names[] =
    {
        "DISCONNECT", "OK", "HALF", "ERROR"
    };

    if(state >= (sizeof(state_names) / sizeof(state_names[0])))
    {
        return "UNKNOWN";
    }
    return state_names[state];
}

static int np_diag(int argc, char **argv)
{
    NP_DIAG_RECORD_t record;
    uint16_t count;
    uint16_t index;
    uint16_t i;
    uint16_t max_queue_depth = 0;
    uint16_t tx_fail_count = 0;
    uint32_t max_tx_delay = 0;
    uint32_t last_raw_cap = 0;
    uint32_t last_filtered_cap = 0;
    rt_int32_t first_bad_time = 0;
    rt_int32_t last_bad_tx_time = 0;
    rt_int32_t recovered_time = 0;
    uint8_t bad_state_seen = 0;
    uint8_t bad_tx_seen = 0;
    uint8_t recovered_seen = 0;
    uint8_t show_all = 0;
    const char *time_base;
    rt_base_t level;

    if(argc > 1 && strcmp(argv[1], "clear") == 0)
    {
        negative_plate_diag_clear();
        rt_kprintf("np_diag cleared\n");
        return 0;
    }

    if(argc > 1 && strcmp(argv[1], "all") == 0)
    {
        show_all = 1;
    }

    level = rt_hw_interrupt_disable();
    g_np_diag_dumping = 1;
    count = g_np_diag_record_count;
    index = (g_np_diag_write_index + NP_DIAG_RECORD_COUNT - count) % NP_DIAG_RECORD_COUNT;
    rt_hw_interrupt_enable(level);

    time_base = (g_np_diag_rf_off_tick != 0) ? "off" : "on";
    rt_kprintf("\nNP summary records=%u overwritten=%u rf_on=%u rf_off=%u\n",
               count, g_np_diag_overwrite_count,
               g_np_diag_rf_on_tick, g_np_diag_rf_off_tick);
    if(g_np_diag_rf_off_tick != 0)
    {
        rt_kprintf("NP time: off<0 before RF_OFF, off>0 after RF_OFF\n");
    }
    else
    {
        rt_kprintf("NP time: on=milliseconds after RF_ON\n");
    }
    for(i = 0; i < count; i++)
    {
        rt_int32_t relative_time;

        record = g_np_diag_records[index];
        if(g_np_diag_rf_off_tick != 0)
        {
            relative_time = (rt_int32_t)(record.tick - g_np_diag_rf_off_tick);
        }
        else
        {
            relative_time = (rt_int32_t)(record.tick - g_np_diag_rf_on_tick);
        }

        if(record.queue_depth > max_queue_depth)
        {
            max_queue_depth = record.queue_depth;
        }

        switch(record.event)
        {
        case NP_DIAG_RF_ON:
            rt_kprintf("NP RF_ON %s=%dms state=%s q=%u mode=%u count=%u interval=%u\n",
                       time_base, relative_time,
                       negative_plate_diag_state_name(record.neg_state),
                       record.value2,
                       (record.value1 >> 16) & 0xFF,
                       (record.value1 >> 8) & 0xFF,
                       record.value1 & 0xFF);
            if(record.neg_state != NEGATIVE_PLATE_STATE_CONNECT_OK && !bad_state_seen)
            {
                bad_state_seen = 1;
                first_bad_time = relative_time;
                recovered_seen = 0;
            }
            break;

        case NP_DIAG_RF_OFF:
            rt_kprintf("NP RF_OFF %s=%dms state=%s q=%u\n",
                       time_base, relative_time,
                       negative_plate_diag_state_name(record.neg_state),
                       record.value2);
            break;

        case NP_DIAG_LCR_SAMPLE:
            last_raw_cap = record.value1;
            last_filtered_cap = record.value2;
            if(show_all)
            {
                rt_kprintf("NP SAMPLE %s=%dms rf=%s state=%s raw=%u filt=%u\n",
                           time_base, relative_time,
                           record.rf_state ? "ON" : "OFF",
                           negative_plate_diag_state_name(record.neg_state),
                           record.value1, record.value2);
            }
            break;

        case NP_DIAG_STATE_CHANGE:
        {
            uint8_t old_state = (record.value2 >> 8) & 0xFF;
            uint8_t new_state = record.value2 & 0xFF;

            rt_kprintf("NP STATE %s=%dms rf=%s %s->%s raw=%u filt=%u\n",
                       time_base, relative_time,
                       record.rf_state ? "ON" : "OFF",
                       negative_plate_diag_state_name(old_state),
                       negative_plate_diag_state_name(new_state),
                       last_raw_cap, last_filtered_cap);
            if(new_state != NEGATIVE_PLATE_STATE_CONNECT_OK)
            {
                if(!bad_state_seen)
                {
                    bad_state_seen = 1;
                    first_bad_time = relative_time;
                }
                recovered_seen = 0;
            }
            else if(new_state == NEGATIVE_PLATE_STATE_CONNECT_OK && bad_state_seen)
            {
                recovered_seen = 1;
                recovered_time = relative_time;
            }
            break;
        }

        case NP_DIAG_MQ_PUT:
            if(show_all)
            {
                rt_kprintf("NP QUEUE %s=%dms seq=%u state=%s cap=%u q=%u result=%s\n",
                           time_base, relative_time, record.seq,
                           negative_plate_diag_state_name(record.neg_state),
                           record.value1, record.queue_depth,
                           (record.value2 == RT_EOK) ? "OK" : "FAIL");
            }
            break;

        case NP_DIAG_CAN_TX_OK:
            if(record.value2 > max_tx_delay)
            {
                max_tx_delay = record.value2;
            }
            if(record.neg_state != NEGATIVE_PLATE_STATE_CONNECT_OK)
            {
                bad_tx_seen = 1;
                last_bad_tx_time = relative_time;
            }
            if(show_all)
            {
                rt_kprintf("NP SENT %s=%dms seq=%u state=%s cap=%u delay=%ums q=%u\n",
                           time_base, relative_time, record.seq,
                           negative_plate_diag_state_name(record.neg_state),
                           record.value1, record.value2, record.queue_depth);
            }
            break;

        case NP_DIAG_CAN_TX_FAIL:
            tx_fail_count++;
            if(record.value2 > max_tx_delay)
            {
                max_tx_delay = record.value2;
            }
            rt_kprintf("NP TX_FAIL %s=%dms seq=%u state=%s cap=%u delay=%ums\n",
                       time_base, relative_time, record.seq,
                       negative_plate_diag_state_name(record.neg_state),
                       record.value1, record.value2);
            break;

        case NP_DIAG_MQ_RESET:
            rt_kprintf("NP RESET %s=%dms dropped=%u reason=%s\n",
                       time_base, relative_time, record.value1,
                       (record.value2 == 0) ? "RF_ON" : "TX_FAIL");
            break;

        default:
            break;
        }

        index = (index + 1) % NP_DIAG_RECORD_COUNT;
    }

    if(g_np_diag_rf_on_tick != 0 && g_np_diag_rf_off_tick != 0)
    {
        if(bad_state_seen)
        {
            rt_kprintf("NP RESULT RF=%ums BAD=YES first_bad=%dms\n",
                       g_np_diag_rf_off_tick - g_np_diag_rf_on_tick,
                       first_bad_time);
            if(bad_tx_seen && recovered_seen)
            {
                rt_kprintf("NP BAD last_sent=%dms recovered=%dms\n",
                           last_bad_tx_time, recovered_time);
            }
            else if(bad_tx_seen)
            {
                rt_kprintf("NP BAD last_sent=%dms recovered=NO\n",
                           last_bad_tx_time);
            }
            else if(recovered_seen)
            {
                rt_kprintf("NP BAD last_sent=NONE recovered=%dms\n",
                           recovered_time);
            }
            else
            {
                rt_kprintf("NP BAD last_sent=NONE recovered=NO\n");
            }
        }
        else
        {
            rt_kprintf("NP RESULT RF=%ums BAD=NO\n",
                       g_np_diag_rf_off_tick - g_np_diag_rf_on_tick);
        }
    }
    rt_kprintf("NP CAN max_delay=%ums max_queue=%u tx_fail=%u\n",
               max_tx_delay, max_queue_depth, tx_fail_count);
    if(!show_all)
    {
        rt_kprintf("NP use 'np_diag all' for samples, queue and sent frames\n");
    }

    level = rt_hw_interrupt_disable();
    g_np_diag_dumping = 0;
    rt_hw_interrupt_enable(level);
    return 0;
}
MSH_CMD_EXPORT(np_diag, show negative plate RF diagnostic timeline);

/* 接收数据回调函数 */
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

static void can_rx_thread(void *parameter)
{
    int i;
    struct rt_can_msg rxmsg = {0};


    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(can_dev, can_rx_call);

    while (1)
    {
        /* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
        rxmsg.hdr = -1;
        /* 阻塞等待接收信号量 */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        /* 从 CAN 读取一帧数据 */
        while(1)
        {
        uint8_t read_size=rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));

        //如果接收的数据需要解密，则在这里进行解密
        //对接收的数据进行解析
        if(read_size)
        {
    #if   1
            /* 打印数据 ID 及内容 */
            rt_kprintf("ID:%x IDE:%x RTR:%x len:%x  ", rxmsg.id, rxmsg.ide, rxmsg.rtr, rxmsg.len);
            for (i = 0; i < rxmsg.len; i++)
            {
                rt_kprintf("%2x ", rxmsg.data[i]);
            }

            rt_kprintf("\n");
    #endif
            switch((rxmsg.id>>CAN_SEND_ID_START_BIT)&0xFF)//获取信息来源
            {
                //源于主板
                case CAN_IDENTY_MAIN_BORAD:
                {
                    rt_kprintf(" Can data recv from mainboard \r\n");
                    if (((rxmsg.id>>CAN_REGIS_ID_BITS_NUM)&0x7)==0)
                    {
                        if(((rxmsg.id>>CAN_REGIS_ID_START_BIT)&0xFFFF)==CAN_ID_MAIN_BOARD_HEARTBEAT_INFO)
                        {//获取心跳值与版本
                            MSG_MAIN_HEART.version=rxmsg.data[0];
                            MSG_MAIN_HEART.heart_beat_count=(rxmsg.data[2]<<8)+rxmsg.data[3];
                        }
                        else if(((rxmsg.id>>CAN_REGIS_ID_START_BIT)&0xFFFF)==CAN_ID_MAIN_RF_SWITCH_CONTROL_INFO)
                        {
                            uint8_t previous_rf_switch = MSG_MAIN_RF_INFO.RF_switch ? 1 : 0;
                            uint16_t queued_before_reset = negative_plate_diag_queue_depth();

                            MSG_MAIN_RF_INFO.RF_switch=rxmsg.data[RFSWITCH];
                            MSG_MAIN_RF_INFO.pulse_mode=rxmsg.data[PULSEMODE];
                            MSG_MAIN_RF_INFO.pulse_count=rxmsg.data[PULSECOUNT];
                            MSG_MAIN_RF_INFO.pulse_interval=rxmsg.data[PULSEINTERVAL];
                            if ( MSG_MAIN_RF_INFO.RF_switch )
                            {
                                if(previous_rf_switch == 0)
                                {
                                    negative_plate_diag_clear();
                                    g_np_diag_rf_on_tick = rt_tick_get();
                                }
                                negative_plate_diag_record(NP_DIAG_RF_ON, 0, g_send_info.neg_state,
                                                           ((uint32_t)MSG_MAIN_RF_INFO.pulse_mode << 16) |
                                                           ((uint32_t)MSG_MAIN_RF_INFO.pulse_count << 8) |
                                                           MSG_MAIN_RF_INFO.pulse_interval,
                                                           queued_before_reset, 1);
                                rt_mq_control(can_tx_mq, RT_IPC_CMD_RESET, NULL);
                                negative_plate_diag_record(NP_DIAG_MQ_RESET, 0, g_send_info.neg_state,
                                                           queued_before_reset, 0, 1);
                                rt_kprintf("RF_switch  open from mainboard");
                            }
                            else
                            {
                                g_np_diag_rf_off_tick = rt_tick_get();
                                g_np_diag_until_tick = rt_tick_get() +
                                                       rt_tick_from_millisecond(NP_DIAG_AFTER_RF_MS);
                                g_np_diag_after_rf_active = 1;
                                negative_plate_diag_record(NP_DIAG_RF_OFF, 0, g_send_info.neg_state,
                                                           ((uint32_t)MSG_MAIN_RF_INFO.pulse_mode << 16) |
                                                           ((uint32_t)MSG_MAIN_RF_INFO.pulse_count << 8) |
                                                           MSG_MAIN_RF_INFO.pulse_interval,
                                                           queued_before_reset, 1);
                                rt_kprintf("RF_switch  close from mainboard");
                            }
                        }
                        else if(((rxmsg.id>>CAN_REGIS_ID_START_BIT)&0xFFFF)==CAN_ID_MAIN_RGB_SWITCH_CONTROL_INFO)
                        {
                            rgb_change_flag=1;
                            MSG_MAIN_RGB_INFO.RGB_switch=rxmsg.data[RGBSWITCH];
                            MSG_MAIN_RGB_INFO.RGB_RED=rxmsg.data[RGBRED];
                            MSG_MAIN_RGB_INFO.RGB_GREEN=rxmsg.data[RGBGREEN];
                            MSG_MAIN_RGB_INFO.RGB_BLUE=rxmsg.data[RGBBLUE];
                            rt_kprintf("RGB_change_receive\n");
                        }
                        else if(((rxmsg.id>>CAN_REGIS_ID_START_BIT)&0xFFFF)==CAN_ID_MAIN_OTA_ASK_INFO)//ota请求
                        {
                            mainboard_ota_start_request(rxmsg.data);
                            rt_timer_stop(can_inform_timer);//暂停信息发送
                            ota_start_flag=1;
                        }
                        else if(((rxmsg.id>>CAN_REGIS_ID_START_BIT)&0xFFFF)==CAN_ID_MAIN_BOARD_SWITCH_CONTROL_INFO)//OTA板子开关
                        {
                            BOARD_SWITCH_FLAG=1-rxmsg.data[0];//获取开关状态
                            rgb_change_flag=1;
                            MSG_MAIN_RGB_INFO.RGB_switch=BOARD_SWITCH_FLAG;
                            if(rxmsg.data[0]==1)
                            {
                                MSG_MAIN_RGB_INFO.RGB_RED=0;
                                MSG_MAIN_RGB_INFO.RGB_GREEN=0;
                                MSG_MAIN_RGB_INFO.RGB_BLUE=255;
                                monitor_board_handshake_send_to_can_mq();
                            }
                        }
    //            case CAN_ID_MAINBOARD_OTA_START_REQUEST:
    //                mainboard_ota_start_request(rxmsg_p->data);
    //                break;
    //            case CAN_ID_MAINBOARD_OTA_DATA_BLOCK:
    //                mainboard_ota_data_request(rxmsg_p->frame_id,rxmsg_p->len,rxmsg_p->data);
    //                break;
                    }
                    else if(ota_start_flag==1 && ((rxmsg.id>>CAN_REGIS_ID_BITS_NUM)&0x7)==1)
                    {
                        mainboard_ota_data_request((rxmsg.id&0xffff),8,rxmsg.data);
                    }
                }
                    break;
                default:

                    rt_kprintf(" Can data recv error%d\r\n ",(rxmsg.id>>24)&0xff);
                    break;

            }

        }
        else {
            break;
        }
        }

    }
}
rt_err_t monitor_board_inform_info_send_to_can_mq(void)
{
    static uint32_t last_diag_cap = 0xFFFFFFFF;
    static uint8_t last_diag_state = 0xFF;
    MSG_CAN_T msg_can = {0};
    NEGPT_STATE neg_state = g_send_info.neg_state;//负极板连接状态定义
    uint16_t sample_voltage_u16 = g_send_info.sample_voltage_u16;//
    uint32_t cap = g_send_info.cap;//计算的电容大小
    uint8_t zerocount=g_send_info.Bubble_sensor_zerocount;
    uint16_t queue_depth_before;
    rt_err_t result;
    msg_can.msg.id = CAN_ID_GEN_BY_REG(CAN_ID_MONITOR_BOARD_INFORM_INFO);
    msg_can.msg.ide = RT_CAN_EXTID;     /* 扩展格式 */
    msg_can.msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.msg.len = 8;

    /* 待发送的 8 字节数据 */
    msg_can.msg.data[0] = neg_state;
    msg_can.msg.data[1] = zerocount;
    msg_can.msg.data[2] = ((sample_voltage_u16)/256);
    msg_can.msg.data[3] = sample_voltage_u16%256;
    msg_can.msg.data[4] = cap>>24;
    msg_can.msg.data[5] = cap>>16;
    msg_can.msg.data[6] = cap>>8;
    msg_can.msg.data[7] = cap;

    msg_can.enqueue_tick = rt_tick_get();
    msg_can.diag_seq = negative_plate_diag_next_seq();
    queue_depth_before = negative_plate_diag_queue_depth();
    result = rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));

    if(negative_plate_diag_is_active() &&
       (result != RT_EOK || queue_depth_before > 0 ||
        cap != last_diag_cap || (uint8_t)neg_state != last_diag_state))
    {
        negative_plate_diag_record(NP_DIAG_MQ_PUT, msg_can.diag_seq, (uint8_t)neg_state,
                                   cap, (uint32_t)result, 0);
        last_diag_cap = cap;
        last_diag_state = (uint8_t)neg_state;
    }
    return result;//RT_EOK only means the frame entered the local queue
}

rt_err_t monitor_board_inform_info_send(void)
{
    uint32_t repeat_time = 20;
    if(g_send_info.sample_voltage_u16 > 200)
    {
        //要将定时器时长，变短
        rt_timer_control(can_inform_timer,RT_TIMER_CTRL_SET_TIME,&repeat_time);
    }
    return monitor_board_inform_info_send_to_can_mq();//
}
int can_test(int argc, char *argv[])
{
    struct rt_can_msg msg = {0};
    rt_size_t  size;
    msg.id = 0x78;              /* ID 为 0x78 */
    msg.ide = RT_CAN_STDID;     /* 标准格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = 8;                /* 数据长度为 8 */

    /* 待发送的 8 字节数据 */
    msg.data[0] = 0x00;
    msg.data[1] = 0x11;
    msg.data[2] = 0x22;
    msg.data[3] = 0x33;
    msg.data[4] = 0x44;
    msg.data[5] = 0x55;
    msg.data[6] = 0x66;
    msg.data[7] = 0x77;
    /* 发送一帧 CAN 数据 */
    size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
    if (size == 0)
    {
        rt_kprintf("can dev write data failed!\n");
    }

    // 更改后再发送十次
    for(rt_uint8_t send_ind = 0; send_ind < 10; send_ind++)
    {
        rt_thread_mdelay(1000);

        msg.data[0] = msg.data[0] + 0x01;
        msg.data[1] = msg.data[1] + 0x01;
        msg.data[2] = msg.data[2] + 0x01;
        msg.data[3] = msg.data[3] + 0x01;
        msg.data[4] = msg.data[4] + 0x01;
        msg.data[5] = msg.data[5] + 0x01;
        msg.data[6] = msg.data[6] + 0x01;
        msg.data[7] = msg.data[7] + 0x01;
        /* 发送一帧 CAN 数据 */
        size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
        if (size == 0)
        {
            rt_kprintf("can dev write data failed!\n");
        }
    }

}

rt_err_t error_code_info_send_to_can_mq(uint8_t flag,uint16_t ec)
{
    MSG_CAN_T msg_can = {0};
    msg_can.msg.id = CAN_ID_GEN_BY_REG(CAN_ID_MONITOR_BOARD_ERROR_INFO);
    msg_can.msg.ide = RT_CAN_EXTID;     /* 标准格式 */
    msg_can.msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.msg.len = 8;
    /* 待发送的 8 字节数据 */
    msg_can.msg.data[0] = flag;
    msg_can.msg.data[1] = 0x00;
    msg_can.msg.data[2] = ec>>8;
    msg_can.msg.data[3] = ec;
    msg_can.msg.data[4] = 0x00;
    msg_can.msg.data[5] = 0x00;
    msg_can.msg.data[6] = 0x00;
    msg_can.msg.data[7] = 0x00;
    return rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
}






rt_err_t monitor_board_handshake_send_to_can_mq(void)
{
    MSG_CAN_T msg_can = {0};
    msg_can.msg.id = CAN_ID_GEN_BY_REG(CAN_ID_MONITOR_BOARD_HANDSHAKE_INFO);
    msg_can.msg.ide = RT_CAN_EXTID;     /* 标准格式 */
    msg_can.msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.msg.len = 8;
    /* 待发送的 8 字节数据 */
    msg_can.msg.data[0] = DEVICE_NUMBER_CODE>>8;
    msg_can.msg.data[1] = DEVICE_NUMBER_CODE&0XFF;
    msg_can.msg.data[2] = COMMUNITICATION_PROTOCOL_VER>>8;
    msg_can.msg.data[3] = COMMUNITICATION_PROTOCOL_VER&0XFF;
    msg_can.msg.data[4] = DEVICE_SOFTWARE_VER>>8;
    msg_can.msg.data[5] = DEVICE_SOFTWARE_VER&0XFF;
    msg_can.msg.data[6] = DEVICE_HARDWARE_VER>>8;
    msg_can.msg.data[7] = DEVICE_HARDWARE_VER&0XFF;
    return rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
}

static void can_inform_timer_timeout(void *parameter)
{
    static uint32_t small_cnt = 0;
    uint32_t repeat_time = 50;
    if(g_send_info.sample_voltage_u16 < 200)
    {
        small_cnt ++;
        if(small_cnt>=5)//在一定之间中，值比较小时，可以间隔长一点
        {
            small_cnt = 0;
            rt_timer_control(can_inform_timer,RT_TIMER_CTRL_SET_TIME,&repeat_time);
        }
    }
    monitor_board_inform_info_send_to_can_mq();
}

void user_can_resoure_init(void)
{
    /* 初始化 CAN 接收信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    //创建消息队列
    can_tx_mq = rt_mq_create("can_mq",sizeof(MSG_CAN_T),50,RT_IPC_FLAG_PRIO);
    if(can_tx_mq == NULL)
    {
        rt_kprintf("create can_tx_mq failed!\n");
        return ;
    }

    //创建定时器，定时发送 通知信息
    can_inform_timer = rt_timer_create("info_tx", can_inform_timer_timeout,
                                 RT_NULL, 10,
                                 RT_TIMER_FLAG_PERIODIC);
    if (can_inform_timer == RT_NULL)
    {
        rt_kprintf("can_inform_timer create failed!\n");
        return ;
    }
}


static void can_tx_thread(void *parameter)
{
    rt_size_t  size;
    uint8_t retry = 0;
    MSG_CAN_T  msg_can;
    static uint32_t last_diag_cap = 0xFFFFFFFF;
    static uint8_t last_diag_state = 0xFF;

    rt_timer_start(can_inform_timer);//启动信息发送
    while(1)
    {
        if(rt_mq_recv(can_tx_mq,&msg_can,sizeof(msg_can),RT_WAITING_FOREVER) == RT_EOK)
        {
//            rt_kprintf("sample_voltage_u16=%d\r\n",((msg_can.msg.data[2]<<8)|(msg_can.msg.data[3])));
            retry = 0;
            while(1)
            {
                //如果发送的数据需要加密，则在这里进行加密
                /* 发送一帧 CAN 数据 */
                size = rt_device_write(can_dev, 0, &msg_can.msg, sizeof(msg_can.msg));

                if (size == 0)//没有发送成功
                {
                    if((msg_can.msg.id & CAN_REGIS_ID_MASK) == CAN_ID_MONITOR_BOARD_INFORM_INFO)
                    {
                        uint32_t cap = ((uint32_t)msg_can.msg.data[4] << 24) |
                                       ((uint32_t)msg_can.msg.data[5] << 16) |
                                       ((uint32_t)msg_can.msg.data[6] << 8) |
                                       msg_can.msg.data[7];
                        negative_plate_diag_record(NP_DIAG_CAN_TX_FAIL, msg_can.diag_seq,
                                                   msg_can.msg.data[0], cap,
                                                   rt_tick_get() - msg_can.enqueue_tick, 0);
                    }
//                    retry++;
//                    if(retry < 41)//
//                    {
//                        rt_thread_mdelay(1);//等一会再发
//                    }
//                    else {
////                        rt_mq_urgent(can_tx_mq,&msg_can,sizeof(msg_can));//将当前没有成功发送的数据，插入到消息的最前面(不保证成功)
//                        retry = 0;
//                        rt_kprintf("can dev write data failed!\n");
//                        rt_timer_stop(can_inform_timer);//暂停信息发送
                        //                    rt_timer_start(can_inform_timer);//暂停信息发送
                        //通知其他任务，当前无法向主控板传输数据
                        {
                            uint16_t dropped_count = negative_plate_diag_queue_depth();
                            rt_mq_control(can_tx_mq, RT_IPC_CMD_RESET, NULL);
                            negative_plate_diag_record(NP_DIAG_MQ_RESET, 0,
                                                       g_send_info.neg_state,
                                                       dropped_count, 1, 0);
                        }
                        break;
//                    }
                }
                else {
                    if((msg_can.msg.id & CAN_REGIS_ID_MASK) == CAN_ID_MONITOR_BOARD_INFORM_INFO)
                    {
                        uint8_t neg_state = msg_can.msg.data[0];
                        uint32_t cap = ((uint32_t)msg_can.msg.data[4] << 24) |
                                       ((uint32_t)msg_can.msg.data[5] << 16) |
                                       ((uint32_t)msg_can.msg.data[6] << 8) |
                                       msg_can.msg.data[7];
                        uint32_t delay_tick = rt_tick_get() - msg_can.enqueue_tick;

                        if(negative_plate_diag_is_active() &&
                           (delay_tick > 2 || cap != last_diag_cap || neg_state != last_diag_state))
                        {
                            negative_plate_diag_record(NP_DIAG_CAN_TX_OK, msg_can.diag_seq,
                                                       neg_state, cap, delay_tick, 0);
                            last_diag_cap = cap;
                            last_diag_state = neg_state;
                        }
                    }
//                    rt_timer_start(can_inform_timer);//暂停信息发送
                    break;
                }
            }
        }
//        rt_thread_mdelay(1);
    }
}

void user_can_task_init(void){

    rt_err_t res;
    rt_thread_t thread_rx,thread_tx;
    /* 查找 CAN 设备 */
    can_dev = rt_device_find(CAN_DEV_NAME);
    if (!can_dev)
    {
        rt_kprintf("find %s failed!\n", CAN_DEV_NAME);
        return RT_ERROR;
    }


    /* 以中断接收及中断发送方式打开 CAN 设备 */
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);

    /* 创建数据接收线程 */
    thread_rx = rt_thread_create("can_rx", can_rx_thread, RT_NULL, 1024, 14, 10);
    if (thread_rx != RT_NULL)
    {
        rt_thread_startup(thread_rx);
    }
    else
    {
        rt_kprintf("create can_rx thread failed!\n");
    }

    /* 创建数据发送过程 */
    thread_tx = rt_thread_create("can_tx", can_tx_thread, RT_NULL, 1024, 16, 10);
    if (thread_tx != RT_NULL)
    {
        rt_thread_startup(thread_tx);
    }
    else
    {
        rt_kprintf("create can_tx thread failed!\n");
    }
}


