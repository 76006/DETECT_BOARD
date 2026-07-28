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
#define CAN_DEV_NAME       "can1"      /* CAN 设备名称 */
#define CAN_INFORM_PERIOD_MS 25

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
                            MSG_MAIN_RF_INFO.RF_switch=rxmsg.data[RFSWITCH];
                            MSG_MAIN_RF_INFO.pulse_mode=rxmsg.data[PULSEMODE];
                            MSG_MAIN_RF_INFO.pulse_count=rxmsg.data[PULSECOUNT];
                            MSG_MAIN_RF_INFO.pulse_interval=rxmsg.data[PULSEINTERVAL];
                            if ( MSG_MAIN_RF_INFO.RF_switch )
                            {
                                rt_mq_control(can_tx_mq, RT_IPC_CMD_RESET, NULL);
                                rt_kprintf("RF_switch  open from mainboard");
                            }
                            else
                            {
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
    MSG_CAN_T msg_can;
    NEGPT_STATE neg_state = g_send_info.neg_state;//负极板连接状态定义
    uint32_t cap = g_send_info.cap;//计算的电容大小
    uint8_t zerocount=g_send_info.Bubble_sensor_zerocount;
    msg_can.msg.id = CAN_ID_GEN_BY_REG(CAN_ID_MONITOR_BOARD_INFORM_INFO);
    msg_can.msg.ide = RT_CAN_EXTID;     /* 扩展格式 */
    msg_can.msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.msg.len = 8;

    /* 待发送的 8 字节数据 */
    msg_can.msg.data[0] = neg_state;
    msg_can.msg.data[1] = zerocount;
    msg_can.msg.data[2] = 0;
    msg_can.msg.data[3] = 0;
    msg_can.msg.data[4] = cap>>24;
    msg_can.msg.data[5] = cap>>16;
    msg_can.msg.data[6] = cap>>8;
    msg_can.msg.data[7] = cap;

    return rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
}

rt_err_t monitor_board_inform_info_send(void)
{
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
    MSG_CAN_T msg_can;
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
    MSG_CAN_T msg_can;
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
    (void)parameter;
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
                                 RT_NULL, rt_tick_from_millisecond(CAN_INFORM_PERIOD_MS),
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

    rt_timer_start(can_inform_timer);//启动信息发送
    while(1)
    {
        if(rt_mq_recv(can_tx_mq,&msg_can,sizeof(msg_can),RT_WAITING_FOREVER) == RT_EOK)
        {
            retry = 0;
            while(1)
            {
                //如果发送的数据需要加密，则在这里进行加密
                /* 发送一帧 CAN 数据 */
                size = rt_device_write(can_dev, 0, &msg_can.msg, sizeof(msg_can.msg));

                if (size == 0)//没有发送成功
                {
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
                        rt_mq_control(can_tx_mq, RT_IPC_CMD_RESET, NULL);
                        break;
//                    }
                }
                else {
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


