///*
// * Copyright (c) 2006-2021, RT-Thread Development Team
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2025-07-28     guozhuang       the first version
// */
////气泡传感器任务
//#include "adc.h"
//#include "can.h"
//#include "BD.h"
//#include <rtthread.h>
//void BD_thread_entry(void *parameter)
//{
//    while(1)
//    {
//        if(rt_sem_take(&bd_sem, RT_WAITING_FOREVER) == RT_EOK)
//        {
//            if(adc2_buffer[2]<=BD_EMPTY||adc2_buffer[2]>=BD_FULL)
//            {
//                GENERATE_ERROR(BD_ERROR);
//            }
//        }
//        rt_thread_mdelay(500);
//
//    }
//}
////气泡传感器创建任务初始化
//int BD_task_init(void)
//{
//    rt_thread_t tid;
//
//    /* 创建线程 */
//    tid = rt_thread_create(
//        "BD_thread",          // 线程名称（确保唯一）
//        BD_thread_entry,      // 线程入口函数
//        RT_NULL,              // 参数
//        2048,                 // 堆栈大小（建议 >= 1024）
//        20,                   // 优先级（数值越小优先级越高）
//        10                    // 时间片（调度时间）
//    );
//
//    if (tid != RT_NULL)
//    {
//        rt_thread_startup(tid);
//        rt_kprintf("BD thread started successfully!\n");
//        return RT_EOK;
//    }
//    else
//    {
//        rt_kprintf("Failed to create ADC thread!\n");
//        return -RT_ERROR;
//    }
//}
