/* Includes ------------------------------------------------------------------*/

#include "ota_handle.h"
#include "can.h"
#include <fal.h>
#include "normal_calc.h"

/* external variables --------------------------------------------------------*/

rt_mq_t  can_tx_mq; //
/* external typedef ----------------------------------------------------------*/

/* external function prototypes ----------------------------------------------*/

/* Private define ------------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Function implementation ---------------------------------------------------*/

#define DEFAULT_DOWNLOAD_PART                                       "download"

extern void rt_hw_cpu_reset(void);


typedef struct {
    uint16_t ota_on;
    uint16_t new_version;
    uint16_t file_crc16;
    uint32_t file_total_size;

    uint32_t file_write_size;
    uint16_t file_crc_cur;
    const struct fal_partition * dl_part;
    rt_timer_t ota_timer;//
}OTA_MANAGE_t;
OTA_MANAGE_t g_ota_manage;


void ota_timer_timeout(void *parameter)
{
    //在OTA后，如果时常太长，没有响应，则自动重启
    rt_hw_cpu_reset();
}


void ota_handle_resoure_init(void)
{
    g_ota_manage.ota_timer = rt_timer_create("ota_tim", ota_timer_timeout,
                                 RT_NULL,  50000,
                                 RT_TIMER_FLAG_ONE_SHOT);
    if (g_ota_manage.ota_timer == RT_NULL)
    {
        rt_kprintf("create g_ota_manage.ota_timer failed.\n");
        return ;
    }
}

rt_err_t ota_data_block_replay_reply_send_to_fop_mq(rt_uint8_t ota_data_ok,uint16_t frameid)
{
    struct rt_can_msg msg_can;
    rt_err_t err;
    msg_can.id = CAN_ID_GEN_BY_REG(frameid)|0x10000;
    msg_can.ide =RT_CAN_EXTID;
    msg_can.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.len = 2;
    /* 待发送的 1 字节数据 */
    msg_can.data[0] = ota_data_ok;
    msg_can.data[1] = 0;
    err = rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
//    rt_event_send(&fiber_optic_tx_event, FIBER_OPTIC_TX_EVENT_PUT_IN_BUF);
    return err;
}
rt_err_t ota_end_replay_reply_send_to_fop_mq(rt_uint8_t ota_ok)
{
    struct rt_can_msg msg_can;
    rt_err_t err;
    msg_can.id = CAN_ID_GEN_BY_REG(CAN_ID_HANDTOOL_OTA_END_REPLY);
    msg_can.ide =RT_CAN_EXTID;
    msg_can.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.len = 2;
    /* 待发送的 1 字节数据 */
    msg_can.data[0] = ota_ok;
    msg_can.data[1] = 0;
    err = rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
//    rt_event_send(&fiber_optic_tx_event, FIBER_OPTIC_TX_EVENT_PUT_IN_BUF);
    return err;
}
rt_err_t ota_start_reply_send_to_fop_mq(rt_uint8_t ota_continue)
{
    struct rt_can_msg msg_can;
    rt_err_t err;
    msg_can.id = CAN_ID_GEN_BY_REG(CAN_ID_HANDTOOL_OTA_START_REPLY);
    msg_can.ide =RT_CAN_EXTID;
    msg_can.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg_can.len = 2;
    /* 待发送的 1 字节数据 */
    msg_can.data[0] = ota_continue;
    msg_can.data[1] = 0;
    err = rt_mq_send(can_tx_mq,&msg_can,sizeof(msg_can));//返回为 RT_EOK ,代表发出去了
//    rt_event_send(&fiber_optic_tx_event, FIBER_OPTIC_TX_EVENT_PUT_IN_BUF);
    return err;
}

void mainboard_ota_start_request(rt_uint8_t *rec_data)
{
    rt_uint8_t ota_continue = 1;
    char* recv_partition = DEFAULT_DOWNLOAD_PART;
    //主控板发出OTA请求，这里准备一下，则回复可以/不可以升级
    //记录数据中的信息
    g_ota_manage.new_version = (rec_data[0]<<8) | rec_data[1];
    g_ota_manage.file_crc16 = (rec_data[2]<<8) | rec_data[3];
    g_ota_manage.file_total_size = (rec_data[4]<<24) | (rec_data[5]<<16) | (rec_data[6]<<8) | rec_data[7];
    g_ota_manage.file_write_size = 0;
    g_ota_manage.file_crc_cur = 0xFFFF;
    if(g_ota_manage.file_total_size >= DEVICE_OTA_ROM_SIZE) //ROM不够大
    {
        ota_continue = 0;
    }
    else {
        if ((g_ota_manage.dl_part = fal_partition_find(recv_partition)) == RT_NULL) //检查分区
        {
            rt_kprintf("Partition (%s) find error!\n", recv_partition);
            ota_continue = 0;
        }
        else {
            rt_kprintf("erase start\n");
            //擦除整个区，需要花费 2.3s
            if(fal_partition_erase_all(g_ota_manage.dl_part) < 0) //擦除分区
            {
                rt_kprintf("Firmware download failed! Partition (%s) erase error!", g_ota_manage.dl_part->name);
                ota_continue = 0;
            }
            else {
                //分区已经准备好了
                g_ota_manage.ota_on = 1;
                rt_timer_start(g_ota_manage.ota_timer);
            }
            rt_kprintf("erase end\n");
        }
    }
    ota_start_reply_send_to_fop_mq(ota_continue);
    rt_kprintf("new_version=%d,file_crc16=%04x,file_total_size=%d,\n",g_ota_manage.new_version,g_ota_manage.file_crc16,g_ota_manage.file_total_size);
}

void mainboard_ota_data_request(rt_uint16_t frame_id,rt_uint16_t data_len, rt_uint8_t *rec_data)
{
    //将文件内容写入单片机flash
    if(g_ota_manage.file_write_size/8 == frame_id)
    {
        if (fal_partition_write(g_ota_manage.dl_part, g_ota_manage.file_write_size, rec_data, data_len) < 0)
        {
            rt_kprintf("Firmware download failed! Partition (%s) write data error!", g_ota_manage.dl_part->name);
            ota_data_block_replay_reply_send_to_fop_mq(0,frame_id);//出错
        }
        else {
            g_ota_manage.file_write_size = g_ota_manage.file_write_size+data_len;
            g_ota_manage.file_crc_cur = modbus_rtu_crc16_frame(rec_data,data_len,g_ota_manage.file_crc_cur);

            ota_data_block_replay_reply_send_to_fop_mq(1,frame_id);//正常继续发数据
            if(g_ota_manage.file_write_size >= g_ota_manage.file_total_size)
            {
                if(g_ota_manage.file_crc_cur == g_ota_manage.file_crc16)
                {
                    //内容正常，且无遗漏
                    ota_end_replay_reply_send_to_fop_mq(1);
                    g_ota_manage.ota_on = 0;
                    //OTA成功
                    rt_thread_delay(2000);
                    /* Reset the device, Start new firmware */

                    rt_hw_cpu_reset();
                    /* wait some time for terminal response finish */
                    rt_thread_delay(200);
                }
                else {
                    ota_end_replay_reply_send_to_fop_mq(0);
                }
            }
            //开始统计时间，超时则自动重启
            rt_timer_stop(g_ota_manage.ota_timer);
            rt_timer_start(g_ota_manage.ota_timer);
        }
        rt_kprintf("s%04x,crc%04x\n",g_ota_manage.file_write_size,g_ota_manage.file_crc_cur);
    }
    else {
        ota_data_block_replay_reply_send_to_fop_mq(0,frame_id);
    }
}







