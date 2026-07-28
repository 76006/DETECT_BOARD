/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-07     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "main.h"
#include "init.h"
#include "WS2812B.h"
#include "core_cm4.h"  // 根据MCU内核选择头文件（如CM3/CM4/CM7）
SEND_INFO_t g_send_info;
void change_finsh_baudrate(void);
uint8_t BOARD_SWITCH_FLAG=0;//00是开 1是关

//需要跟 link.lds一样
//更改中断向量表
#define RT_APP_PART_ADDR                    0x08020000//这是APP的起始地址(与 fal_cfg.h 中的配置相关)
/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
*/
static int ota_app_vtor_reconfig(void)
{
//    #define NVIC_VTOR_MASK   0x3FFFFF80
//    /* Set the Vector Table base location by user application firmware definition */
//    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;
    SCB->VTOR = RT_APP_PART_ADDR; /* Vector Table Relocation in Internal FLASH. */
    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);


void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // 启用跟踪单元
    DWT->CYCCNT = 0;                                 // 清零计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启用CYCCNT
}
int main(void)
{
    Hardware_Init();
    Software_Init();
    Task_Init();
    DWT_Init();
//    change_finsh_baudrate();
    int count = 1;
   // 启用定时器
//    rgb_change_flag=1;
    while (count++)
    {
//        bsp_can_send();
        LOG_D("Hello RT-Thread!");
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
void change_finsh_baudrate(void)
{
    rt_device_t finsh_device;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
    //更改 FINSH 波特率
    finsh_device = rt_device_find(RT_CONSOLE_DEVICE_NAME);
    config.baud_rate = BAUD_RATE_460800;
    /*控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(finsh_device, RT_DEVICE_CTRL_CONFIG, &config);
}
void DWT_Delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;                     // 获取当前计数器值
    uint32_t cycles = us * (SystemCoreClock / 1000000); // 计算所需周期数

    while ((DWT->CYCCNT - start) < cycles);           // 等待周期数达到
}
