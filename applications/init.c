#include "init.h"
#include "adc.h"
#include "dac.h"
#include "can.h"
//#include "lcr_calc.h"
//#include "user_adc.h"
#include "lcr_task.h"
#include "XC2362.h"
#include "WS2812B.h"
#include "ota_handle.h"
#include "fal.h"
#include "my_encry.h"
#include "para_storage.h"
void Hardware_Init(void)
{
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_CAN1_Init();
    MX_DAC_Init();
    MX_SPI1_Init();
    MX_USART6_UART_Init();
    MX_ADC2_Init();
    MX_TIM8_Init();
    MX_TIM3_Init();
    MX_TIM2_Init();
    MX_TIM1_Init();
    XC2362_Init();
    my_encry_init();
}
void Software_Init(void)
{
    fal_init();
    adc_dma_init();//启动DMA传输ADC1 ADC2
    dac_dma_init();//启动DMA传输DAC_CH1 DAC_CH2
    user_can_resoure_init();//创建初始化CAN设备
    XC2362_DMA_Init(&hspi1); //启动SPI DMA传输任务
    ota_handle_resoure_init();
    params_system_init();
}
void Task_Init(void)
{

    user_can_task_init(); //CAN接收与发送任务
    if(is_encry_ok)
    {
        lcr_task_init();//电流监测任务  ，LCRDAC 输出到LCRADC1 与LCRADC2
        BD_task_init();//气泡传感器任务，接收传感器AD值
        current_sample_task_init();//发送SPI转换的AD值
        WS2812B_task_init();
        BOARD_SWITCH_task_init();
        peaktoggle_task_init();
    }
}


