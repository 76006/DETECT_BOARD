#include "init.h"
#include "adc.h"
#include "dac.h"
#include "can.h"
//#include "lcr_calc.h"
//#include "user_adc.h"
#include "lcr_task.h"
#include "WS2812B.h"
#include "ota_handle.h"
#include "fal.h"
#include "my_encry.h"
void Hardware_Init(void)
{
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_CAN1_Init();
    MX_DAC_Init();
    MX_USART6_UART_Init();
    MX_ADC2_Init();
    MX_TIM8_Init();
    MX_TIM3_Init();
    MX_TIM2_Init();
    MX_TIM1_Init();
    my_encry_init();
}
void Software_Init(void)
{
    fal_init();
    adc_dma_init();//启动DMA传输ADC1 ADC2
    dac_dma_init();//启动DMA传输DAC_CH1 DAC_CH2
    user_can_resoure_init();//创建初始化CAN设备
    ota_handle_resoure_init();
}
void Task_Init(void)
{

    user_can_task_init(); //CAN接收与发送任务
    if(is_encry_ok)
    {
        lcr_task_init();//电流监测任务  ，LCRDAC 输出到LCRADC1 与LCRADC2
        BD_task_init();//气泡传感器任务，接收传感器AD值
        WS2812B_task_init();
        BOARD_SWITCH_task_init();
    }
}


