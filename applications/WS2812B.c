#include "WS2812B.h"
#include "can.h"
extern MSG_CAN_MAIN_RGB_INFO_STRUCT MSG_MAIN_RGB_INFO;
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_ch1;
uint16_t pwmBuffer[LED_NUM*24 + RESET_CYCLES];
uint8_t rgb_change_flag = 0 ;
extern uint8_t ota_start_flag;
void WS2812B_SetColor(uint8_t ledNum, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = (g << 16) | (r << 8) | b; // GRB格式

    for (int j=0;j<=ledNum;j++)
    {
        uint16_t *p = &pwmBuffer[j * 24];
        for(int i=0; i<24; i++) {
            p[i] = (color & (1<<(23-i))) ? T1H : T0H;
        }
    }

    // 填充RESET信号
    for(int i=0; i<RESET_CYCLES; i++) {
        pwmBuffer[LED_NUM*24 + i] = 0XFF;
    }
}
void WS2812B_Update(void) {
    // 停止DMA传输以防冲突
//    HAL_GPIO_WritePin(DLPS_GPIO_Port, DLPS_Pin, PIN_LOW);
//    HAL_GPIO_WritePin(DLPS_GPIO_Port, DLPS_Pin, PIN_HIGH);
    rt_thread_mdelay(100);
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);
    rt_thread_mdelay(100);
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)pwmBuffer,
            LED_NUM*24 + RESET_CYCLES);
    rt_thread_mdelay(10);
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t*)pwmBuffer,
            LED_NUM*24 + RESET_CYCLES);
    rt_thread_mdelay(1000);
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
    rt_thread_mdelay(10);
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);
    rt_thread_mdelay(100);

//    // 启动DMA传输
}
void WS2812B_SetRED(void)
{
    WS2812B_SetColor(LED_NUM,0xff,0,0);
    WS2812B_Update();
}
void WS2812B_SetGREEN(void)
{
    WS2812B_SetColor(LED_NUM,0,0xff,0);
    WS2812B_Update();
}
void WS2812B_SetBLUE(void)
{
    WS2812B_SetColor(LED_NUM,0,0,0xff);
    WS2812B_Update();
}
void WS2812B_RESET(void)
{
    WS2812B_SetColor(LED_NUM,0,0,0);
    WS2812B_Update();
}
void WS2812B_WHITE(void)
{
    WS2812B_SetColor(LED_NUM,0xff,0xff,0xff);
    WS2812B_Update();
}

void RGB_change_task(void *param)
{
    while(1)
    {
        if(rgb_change_flag==1&&ota_start_flag==0)
        {
            rgb_change_flag=0;
            HAL_GPIO_WritePin(DLPS_GPIO_Port, DLPS_Pin, (1-MSG_MAIN_RGB_INFO.RGB_switch));
            WS2812B_SetColor(LED_NUM,MSG_MAIN_RGB_INFO.RGB_RED,MSG_MAIN_RGB_INFO.RGB_GREEN,MSG_MAIN_RGB_INFO.RGB_BLUE);
//            WS2812B_SetColor(LED_NUM,MSG_MAIN_RGB_INFO.RGB_RED,255,MSG_MAIN_RGB_INFO.RGB_BLUE);
            WS2812B_Update();
            rt_kprintf("RGB_change\n");

        }
        rt_thread_mdelay(20);
    }

}
void WS2812B_task_init(void)
{
    rt_thread_t thread_sample;
    /* 创建数据接收线程 */
    thread_sample = rt_thread_create("RGB_change", RGB_change_task, RT_NULL, 4096, 16, 10);
    if (thread_sample != RT_NULL)
    {
        rt_thread_startup(thread_sample);
    }
    else
    {
        rt_kprintf("create current sample thread failed!\n");
    }
}
