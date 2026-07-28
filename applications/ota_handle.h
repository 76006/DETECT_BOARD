#ifndef    _OTA_HANDLE_H_
#define    _OTA_HANDLE_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/
#define CAN_ID_HANDTOOL_OTA_START_REPLY                                    0x5110//手具板发出的OTA请求回复
#define CAN_ID_HANDTOOL_OTA_DATA_BLOCK_REPLY                               0x2A2//手具板发出的OTA数据块回复
#define CAN_ID_HANDTOOL_OTA_END_REPLY                                      0x5114//手具板发出的OTA成功/失败回复
#define DEVICE_OTA_ROM_SIZE                                (512*1024)//OTA用的下载分区大小
void ota_handle_resoure_init(void);

void mainboard_ota_start_request(rt_uint8_t *rec_data);

void mainboard_ota_data_request(rt_uint16_t frame_id,rt_uint16_t data_len, rt_uint8_t *rec_data);


#endif


