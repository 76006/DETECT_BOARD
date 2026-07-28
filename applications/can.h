/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-10     guozhuang       the first version
 */
#ifndef APPLICATIONS_CAN_H_
#define APPLICATIONS_CAN_H_

#if     1
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "main.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>
#include "stdbool.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/
//CAN ID 中各位的含义定义
#define CAN_SEND_ID_BITS_NUM                                              (5)//CAN的发送在ID中占用位数
#define CAN_SEND_ID_START_BIT                                             (24)//CAN的发送在ID中的起始位
#define CAN_SEND_ID_MASK                                                  ((1<<CAN_SEND_ID_BITS_NUM)-1)//
#define CAN_REC_ID_BITS_NUM                                               (5)//CAN的接收在ID中占用位数
#define CAN_REC_ID_START_BIT                                              (19)//CAN的发送在ID中的起始位
#define CAN_REC_ID_MASK                                                   ((1<<CAN_REC_ID_BITS_NUM)-1)//
#define CAN_REGIS_ID_BITS_NUM                                             (16)//CAN的寄存器在ID中占用位数
#define CAN_REGIS_ID_START_BIT                                            (0)//CAN的寄存器在ID中的起始位
#define CAN_REGIS_ID_MASK                                                 ((1<<CAN_REGIS_ID_BITS_NUM)-1)//

#define CAN_IDENTY_MAIN_BORAD                                             (0x04)//CAN通信中主控板标识
#define CAN_IDENTY_MONITOR_BORAD                                          (0x08)//CAN通信中监测板标识
#define CAN_IDENTY_SCREEN_BORAD                                           (0x0F)//CAN通信中显控板标识


//CAN 发送源和接收源的 设备定义
#define CAN_SEND_SOURCE                                                   (CAN_IDENTY_MONITOR_BORAD)//发送源的定义
#define CAN_REC_DESTI_MAIN_BORAD                                          (CAN_IDENTY_MAIN_BORAD)//接收源，主控板定义
#define CAN_REC_DESTI_SCREEN                                              (CAN_IDENTY_SCREEN_BORAD)//接收源，显控板定义


//29bit的从监测板发出的CAN ID
#define CAN_ID_MONITOR_BOARD_HANDSHAKE_INFO                               (0x5100)//监测板与主控板的握手
#define CAN_ID_MONITOR_BOARD_INFORM_INFO                                  (0x5104)//监测板发出的信息，包含负极板状态，电流大小，电容大小
#define CAN_ID_MONITOR_BOARD_ERROR_INFO                                   (0x5108)//监测板发出的错误信息(目前未用)

//29bit的从监测板发出的CAN ID
#define CAN_ID_MAIN_BOARD_HEARTBEAT_INFO                                    (0xFF04)//主控板发送的心跳
#define CAN_ID_MAIN_RF_SWITCH_CONTROL_INFO                                  (0x5000)//主控板发送的射频开关控制
#define CAN_ID_MAIN_RGB_SWITCH_CONTROL_INFO                                   (0x5008)//主控板发送的RGB开关控制
#define CAN_ID_MAIN_OTA_ASK_INFO                                   (0x5010)//主控板发送的OTA请求
#define CAN_ID_MAIN_BOARD_SWITCH_CONTROL_INFO                                    (0x5020)//主控板发送的OTA请求
;
#define CAN_ID_GEN(source,desti,reg)                                      ((((source)&CAN_SEND_ID_MASK)<<CAN_SEND_ID_START_BIT) | \
                                                                           (((desti)&CAN_REC_ID_MASK)<<CAN_REC_ID_START_BIT) | \
                                                                           (((reg)&CAN_REGIS_ID_MASK)<<CAN_REGIS_ID_START_BIT) )
#define CAN_ID_GEN_BY_REG(reg)                                            CAN_ID_GEN(CAN_SEND_SOURCE,CAN_REC_DESTI_MAIN_BORAD,reg)

#define GENERATE_ERROR(flag,ec)                                                 error_code_info_send_to_can_mq(flag,ec)
/* Public typedef ------------------------------------------------------------*/
typedef struct {
    struct rt_can_msg msg;
    rt_tick_t enqueue_tick;
    uint32_t diag_seq;
}MSG_CAN_T;

typedef struct {
    uint8_t version;
    uint16_t heart_beat_count;
}MSG_CAN_MAIN_HEART_STRUCT;
typedef struct {
    uint8_t device_id;
    uint8_t protocol_version;
    uint8_t software_version;
    uint8_t hardware_version;

}MSG_CAN_HANDSHAKE_STRUCT;

enum RF_INFO_DATA_FORMAT
{
    RFSWITCH=0,
    PULSEMODE,
    PULSECOUNT,
    PULSEINTERVAL
};
enum BOARD_SWITCH_INFO_FORMAT
{
    BOARD_OPEN=0,
    BOARD_OFF,
};
typedef struct {
    bool RF_switch;//脉冲开关
    uint8_t pulse_mode;//脉冲模式
    uint8_t pulse_count;//脉冲数量
    uint8_t pulse_interval;//脉冲间隔
}MSG_CAN_MAIN_RF_INFO_STRUCT;

enum RGB_INFO_DATA_FORMAT
{
    RGBSWITCH=0,
    RGBRED,
    RGBGREEN,
    RGBBLUE
};
enum ERRORFLAG
{
    ERRORRESET=0,
    ERRORSET,
};

typedef struct {
    bool RGB_switch;
    uint8_t RGB_RED;//脉冲模式
    uint8_t RGB_GREEN;//脉冲数量
    uint8_t RGB_BLUE;//脉冲间隔
}MSG_CAN_MAIN_RGB_INFO_STRUCT;


/* Public variables ----------------------------------------------------------*/
extern uint8_t ota_start_flag;
extern MSG_CAN_MAIN_RF_INFO_STRUCT MSG_MAIN_RF_INFO;

/* Public function prototypes ------------------------------------------------*/


rt_err_t monitor_board_inform_info_send_to_can_mq(void);

rt_err_t monitor_board_inform_info_send(void);

rt_err_t error_code_info_send_to_can_mq(uint8_t flag,uint16_t ec);

rt_err_t monitor_board_handshake_send_to_can_mq(void);

void user_can_resoure_init(void);

void user_can_task_init(void);

void negative_plate_diag_lcr_sample(uint32_t raw_cap, uint32_t filtered_cap, uint8_t neg_state);

void negative_plate_diag_state_change(uint32_t cap, uint8_t old_state, uint8_t new_state);


#endif
#endif /* APPLICATIONS_CAN_H_ */
