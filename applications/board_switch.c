/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-28     guozhuang       the first version
 */
//开关任务
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "can.h"
#include "board_switch.h"
#include <rtthread.h>
#include "WS2812B.h"
#include "board_switch.h"
extern uint8_t BOARD_SWITCH_FLAG ;
static uint8_t last_board_state =0;
extern uint8_t ota_start_flag;
extern uint16_t user_adc_buf[USER_ADC_BUF_DATA_SIZE];
extern uint16_t adc2_buffer[32];
void BOARD_SWITCH_thread_entry(void *parameter)
{
    while(1)
    {
        if(BOARD_SWITCH_FLAG == last_board_state)//若无变化
        {
            //1.关闭灯光
            //2.关闭AD采集 （负极板）
            //3.关闭电流采集（X2362）
        }
        else
        {
            last_board_state=BOARD_SWITCH_FLAG;
            if(BOARD_SWITCH_FLAG == BOARD_OPEN )
            {
                // 启动DMA传输
                if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)user_adc_buf, USER_ADC_BUF_DATA_SIZE) != HAL_OK)
                {
                    rt_kprintf("ADC1 DMA start failed!\n");
                }
                // 启动DMA传输
                if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_buffer, 32) != HAL_OK)
                {
                    rt_kprintf("ADC2 DMA start failed!\n");
                }
                // 启动定时器触发
                HAL_TIM_Base_Start(&htim3);
                HAL_TIM_Base_Start(&htim8);

            }
            else
            {
                HAL_ADC_Stop_DMA(&hadc1);
                HAL_ADC_Stop_DMA(&hadc2);
                HAL_ADC_Stop_DMA(&hdac);
            }

        }
//        if(ota_start_flag == 1)//若无变化
//           {
//            HAL_ADC_Stop_DMA(&hadc1);
//            HAL_ADC_Stop_DMA(&hadc2);
//            HAL_ADC_Stop_DMA(&hdac);
//           }
        rt_thread_mdelay(100);

    }
}
//开关创建任务初始化
int BOARD_SWITCH_task_init(void)
{
    rt_thread_t tid;

    /* 创建线程 */
    tid = rt_thread_create(
        "BD_thread",          // 线程名称（确保唯一）
        BOARD_SWITCH_thread_entry,      // 线程入口函数
        RT_NULL,              // 参数
        2048,                 // 堆栈大小（建议 >= 1024）
        16,                   // 优先级（数值越小优先级越高）
        10                    // 时间片（调度时间）
    );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        rt_kprintf("BD thread started successfully!\n");
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Failed to create ADC thread!\n");
        return -RT_ERROR;
    }
}
