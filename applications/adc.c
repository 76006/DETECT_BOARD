/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-09     guozhuang       the first version
 */
#include "adc.h"
#include "BD.h"
#include "can.h"
#include <rtthread.h>
#define ADC_BUFFER_SIZE  32

uint16_t user_adc_buf[USER_ADC_BUF_DATA_SIZE];
uint16_t adc2_buffer[ADC_BUFFER_SIZE];
struct rt_semaphore adc_sem;
struct rt_semaphore bd_sem;
extern uint16_t adc_ok;
ADC_MANAGE_t g_adc_manage;
extern SEND_INFO_t g_send_info;
uint8_t BD_error_sendflag = 0 ;//气泡传感器发送标志
extern uint8_t ota_start_flag;
uint8_t ADC_info_print_flag=0;
uint8_t ADC2_info_print_flag=0;
uint16_t BD_EMPTY =1800;
uint16_t BD_FULL =4000;
extern float calibrate_num;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        rt_sem_release(&adc_sem);
    }
    else if(hadc->Instance == ADC2)
    {
        rt_sem_release(&bd_sem);
    }
}
//ADC1 DAM 初始化 开启对LCRADC1、LCRADC3的数据进行采样 ，使用TIM3频率采样
int adc_dma_init(void)
{
    rt_sem_init(&adc_sem, "adc_sem", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&bd_sem, "bd_sem", 0, RT_IPC_FLAG_FIFO);

    // 启动DMA传输
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)user_adc_buf, USER_ADC_BUF_DATA_SIZE) != HAL_OK)
    {
        rt_kprintf("ADC1 DMA start failed!\n");
    }
    // 启动DMA传输
    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_buffer, ADC_BUFFER_SIZE) != HAL_OK)
    {
        rt_kprintf("ADC2 DMA start failed!\n");
    }
    // 启动定时器触发
    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_Base_Start(&htim8);

    rt_kprintf("ADC DMA initialization complete!\n");
    return RT_EOK;
}
//气泡传感器任务
void BD_thread_entry(void *parameter)
{
    while(1)
    {
        if(ota_start_flag==0)
        {


            if(rt_sem_take(&bd_sem, RT_WAITING_FOREVER) == RT_EOK)
            {
                g_send_info.Bubble_sensor_zerocount=0;
    //            ifadc2_buffer
                // ADC2数据可以直接访问，因为DMA是循环模式
                for(int i=0; i<ADC_BUFFER_SIZE; i++)
                {
                    if (adc2_buffer[i]<=BD_EMPTY||adc2_buffer[1]>=BD_FULL)
                    {
                        g_send_info.Bubble_sensor_zerocount++;
                    }
                    if (ADC2_info_print_flag)
                    {
#ifdef VCTADC
                        if (i%2==0)
                        {
                            rt_kprintf("V[%d]: %d\n", i, (uint32_t)(((float)(adc2_buffer[i])*calibrate_num/4096/4)*1000*10));
                        }
                        else {
                            //                            rt_kprintf("V[%d]: %d\n", i, adc2_buffer[i]);
                        }
#else
                    rt_kprintf("adc2[%d]: %d\n", i, adc2_buffer[i]);
#endif

                    }
                }
                if(rt_sem_take(&bd_sem, RT_WAITING_FOREVER) == RT_EOK)
                {
                    if(g_send_info.Bubble_sensor_zerocount >= (ADC_BUFFER_SIZE/2) && BD_error_sendflag ==0)
                    {
                        GENERATE_ERROR(SET,BD_ERROR);
                        BD_error_sendflag=1;
                    }
                    else if(g_send_info.Bubble_sensor_zerocount <= (ADC_BUFFER_SIZE/16) && BD_error_sendflag ==1)
                    {
                        GENERATE_ERROR(RESET,BD_ERROR);
                        BD_error_sendflag=0;
                    }
                }
            }

        }
        rt_thread_mdelay(100);

    }
}
//气泡传感器创建任务初始化
int BD_task_init(void)
{
    rt_thread_t tid;

    /* 创建线程 */
    tid = rt_thread_create(
        "BD_thread",          // 线程名称（确保唯一）
        BD_thread_entry,      // 线程入口函数
        RT_NULL,              // 参数
        2048,                 // 堆栈大小（建议 >= 1024）
        25,                   // 优先级（数值越小优先级越高）
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
void peak_thread_entry(void *parameter)
{
    while(1)
    {
        HAL_GPIO_WritePin(PEAK_GPIO_Port, PEAK_Pin, GPIO_PIN_RESET);
        rt_thread_mdelay(50);
        HAL_GPIO_WritePin(PEAK_GPIO_Port, PEAK_Pin, GPIO_PIN_SET);
        rt_thread_mdelay(10);

    }
}
void peaktoggle_task_init(void)
{
    rt_thread_t tid;

    /* 创建线程 */
    tid = rt_thread_create(
        "peak_thread",          // 线程名称（确保唯一）
        peak_thread_entry,      // 线程入口函数
        RT_NULL,              // 参数
        2048,                 // 堆栈大小（建议 >= 1024）
        25,                   // 优先级（数值越小优先级越高）
        10                    // 时间片（调度时间）
    );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        rt_kprintf("peak thread started successfully!\n");
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Failed to create peak thread!\n");
        return -RT_ERROR;
    }
}
void User_Adc_Start(void)
{
    // 开启ADC和DMA
    // 传输完成，调用 HAL_ADC_ConvCpltCallback 函数
    // 传输一半，调用 HAL_ADC_ConvHalfCpltCallback 函数
    // 传输错误，调用 HAL_ADC_ErrorCallback 函数


//    HAL_ADC_DeInit(&hadc1); //加上这个后，两个通道才能同时有效
    User_Adc_Init();
    HAL_TIM_Base_Start(&htim3);
//    HAL_ADC_Start(&hadc1);                                   //
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)user_adc_buf, USER_ADC_BUF_DATA_SIZE);
//    HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*)user_adc_buf, USER_ADC_BUF_DATA_SIZE/2);
}

void User_Adc_Init(void)
{

    if(g_adc_manage.is_init == 0)
    {
        g_adc_manage.is_init = 1;
        g_adc_manage.set_fre = DAC_WORK_FRE_1kHz;
    }
    MX_TIM3_Init();
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    User_Adc_Set_DacWorkFre(g_adc_manage.set_fre);
    MX_ADC1_Init();

}

void User_Adc_Stop(void)
{

    //ADC_ConversionStop_Disable(&hadc1);
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim3);

}

uint16_t User_Adc_Get_Value(float *wave1,float *wave2,uint16_t wave_buf_size)
{
    uint16_t i;
    if(wave_buf_size > USER_ADC_BUF_DATA_SIZE/USER_ADC_CH_NUM)
        wave_buf_size = USER_ADC_BUF_DATA_SIZE/USER_ADC_CH_NUM;

    for(i=0;i<wave_buf_size;i++)
    {
        //将数字信号，转换成电压信号，且电压信号是减去负值的
        //wave1[i] = user_adc_buf[i*2]*2.5/4096-2.5/2;
        //wave2[i] = user_adc_buf[i*2+1]*2.5/4096-2.5/2;

        //将ADC采样值换算成电压值(带偏移)
        //wave1[i] = user_adc_buf[i*2]*2.5/4096*2-2.5+0.11;//后面的0.11是偏移量
        //wave2[i] = user_adc_buf[i*2+1]*2.5/4096*2-2.5;

        //将ADC采样值换算成电压值(不带偏移)
        wave1[i] = user_adc_buf[i*2]*2.5/4096*2-2.5;
        wave2[i] = user_adc_buf[i*2+1]*2.5/4096*2-2.5;
        if (ADC_info_print_flag)
        {
            rt_kprintf("%d,%d,%d,%d,%d\n",i,user_adc_buf[i*2],user_adc_buf[i*2+1],PRINT_100xFLOAT(wave1[i]),PRINT_100xFLOAT(wave2[i]));
        }
    }
    return wave_buf_size;
}



void User_Adc_Set_DacWorkFre(FRE_t DacWorkFre)//设置DAC的工作频率
{
    switch(DacWorkFre)
    {
        //根据不同的DAC工作频率，设定ADC的采样频率，方便分析
        case DAC_WORK_FRE_100Hz:
            //一个周期采样200个点，
            //168000000/42/200/200=100
            htim3.Init.Prescaler = 42-1;
            htim3.Init.Period = 200-1;
            g_adc_manage.set_fre = DAC_WORK_FRE_100Hz;
            break;
        case DAC_WORK_FRE_1kHz:
            //一个周期采样100个点，
            //168000000/21/80/100=1000
            htim3.Init.Prescaler = 21-1;
            htim3.Init.Period = 40-1;

//            htim3.Init.Prescaler = 2-1; //进测试
//            htim3.Init.Period = 90-1;
            g_adc_manage.set_fre = DAC_WORK_FRE_1kHz;
            break;
        case DAC_WORK_FRE_10kHz:
            //一个周期采样50个点，
            //168000000/21/160/50=10000
            htim3.Init.Prescaler = 21-1;
            htim3.Init.Period = 160-1;
            g_adc_manage.set_fre = DAC_WORK_FRE_10kHz;
            break;
        default:
            htim3.Init.Prescaler = 21-1;
            htim3.Init.Period = 80-1;
            g_adc_manage.set_fre = DAC_WORK_FRE_1kHz;
            break;
    }
    htim3.Instance = TIM3;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
}

static void User_Adc2_DMA2_Config(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Channel4_5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}

void User_Adc2_Init(void)
{
    MX_ADC2_Init();
    User_Adc2_DMA2_Config();
}
static int ADC_INFO_PRINTFLAG(int argc, char **argv)
{
    ADC_info_print_flag = 1-ADC_info_print_flag;

    if (ADC_info_print_flag)
    {
        rt_kprintf("ADC_info_print_flag  is enabled\n");
    }
    else
    {
        rt_kprintf("ADC_info_print_flag  is disabled\n");
    }
    return 0;
}
MSH_CMD_EXPORT(ADC_INFO_PRINTFLAG,"ADC_INFO_PRINTFLAG");
static int ADC2_INFO_PRINTFLAG(int argc, char **argv)
{
    ADC2_info_print_flag = 1-ADC2_info_print_flag;

    if (ADC2_info_print_flag)
    {
        rt_kprintf("ADC2_INFO_PRINTFLAG  is enabled\n");
    }
    else
    {
        rt_kprintf("ADC2_INFO_PRINTFLAG  is disabled\n");
    }
    return 0;
}
MSH_CMD_EXPORT(ADC2_INFO_PRINTFLAG,"ADC2_INFO_PRINTFLAG");

static int SETBD_EMPTY(int argc, char **argv)
{

    if (argc != 2)
        {
//            rt_kprintf("Usage: set_value <integer_value>\n");
//            rt_kprintf("Current value: %.2f\n", calibrate_num);
            return;
        }

        // 将传入的整数转换为浮点数，然后除以100
        BD_EMPTY = atoi(argv[1]);

        rt_kprintf("Output: %d \n", BD_EMPTY);
}
MSH_CMD_EXPORT(SETBD_EMPTY,"SETBD_EMPTY");
static int SETBD_FULL(int argc, char **argv)
{

    if (argc != 2)
        {
//            rt_kprintf("Usage: set_value <integer_value>\n");
//            rt_kprintf("Current value: %.2f\n", calibrate_num);
            return;
        }

        // 将传入的整数转换为浮点数，然后除以100
    BD_FULL = atoi(argv[1]);

    rt_kprintf("Output: %d \n", BD_FULL);
}
MSH_CMD_EXPORT(SETBD_FULL,"SETBD_FULL");
