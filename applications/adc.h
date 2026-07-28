/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-09     guozhuang       the first version
 */
#include "main.h"
#ifndef APPLICATIONS_ADC_H_
#define APPLICATIONS_ADC_H_
int adc_dma_init(void);
void BD_thread_entry(void *parameter);
int BD_task_init(void);
void User_Adc_Init(void);
void User_Adc_Start(void);
void User_Adc_Stop(void);
uint16_t User_Adc_Get_Value(float *wave1,float *wave2,uint16_t wave_buf_size);
void User_Adc_Set_DacWorkFre(FRE_t DacWorkFre);//设置DAC的工作频率
void User_Adc2_Init(void);
#define USER_ADC_CH_NUM                              (2)//AD的通道数量
#define USER_ADC_DATA_NUM                            (USER_VOUT_DATA_NUM)//每个通道要存储多少数据
#define USER_ADC_BUF_DATA_SIZE                       (USER_ADC_CH_NUM*USER_ADC_DATA_NUM)
/* Public typedef ------------------------------------------------------------*/
typedef struct {
    uint16_t chip_ref;//芯片内部参考电压
    float chip_temprt;//芯片温度

    uint16_t is_init;
    FRE_t set_fre;

}ADC_MANAGE_t;


struct rt_semaphore bd_sem;

#endif /* APPLICATIONS_ADC_H_ */
