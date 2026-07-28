#include "XC2362.h"

#include "stdio.h"
#include "string.h"
#include "slid_ave_filter.h"
#include "can.h"
#include "para_storage.h"
#define Standard_voltage  3.3f
extern SEND_INFO_t g_send_info;
uint16_t adcResult;//结果值
float voltage;
// 定义发送缓冲区
#define Uart1buf_Send_Size 8 //串口发送缓存大小
float voltageBatch[Uart1buf_Send_Size]; //串口发送的电压 数组缓存
#define Receive_ADBUF_SIZE 8
uint8_t vofa_tail[4]={0x00, 0x00, 0x80, 0x7f};
volatile uint8_t activeBuffer = 0;                   // 当前DMA写入的缓冲区标志（0或1）
volatile uint8_t processBuffer = 0;                  // 待处理的缓冲区标志
volatile uint8_t bufferReady = 0;                    // 缓冲区就绪标志
float Calibration_NumA = 0;                          //校准数据A
float Calibration_NumB = 0;                          //校准数据B   校准结果为AX+B
float Calibration_Flag = 0;                         //校准标志
extern uint8_t BOARD_SWITCH_FLAG;
extern uint8_t ota_start_flag;
void Enable_DWT(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}



uint16_t adcBuffer1[1]={0}; //做一个暂存数组
uint16_t adc_count=0;//adc存储数据的个数
uint16_t count_front = 180;//延时处理
uint16_t count_back = 0;//延时处理
#define SEND_LINE                                   (8)
#define BSP_ADC_DATA_NUM                            (2048)//(1024*20)//
#define BSP_ADC_BUF_DATA_SIZE                       (BSP_ADC_DATA_NUM)
#define BSP_ADC_STATIS_BUF_SIZE                     (4096)//统计的缓存大小
uint16_t bsp_adc_buf[BSP_ADC_BUF_DATA_SIZE]={0};
uint16_t bsp_adc_statis_buf_size[2][BSP_ADC_STATIS_BUF_SIZE];//两个统计缓存，分开统计
uint16_t adc_ok = 0;
uint16_t statis_buf_send = 0;//指示发送哪个缓存
uint16_t statis_buf_save = 1;//指示保存哪个缓存
uint16_t statis_buf_send_pos = 0;

uint16_t statis_start = 0;

uint16_t statis_max = 0;
float fstatis_max = 0.0;

uint16_t fstatis_average_num = 0;
float fstatis_high_sum = 0.0;
float fstatis_high_average = 0.0;

float sample_voltage_f;
SlidingAvgFilter_t currentSlidAvgFilter={0};
uint16_t sample_count=0;
float calibrate_num        =           2.28;
uint8_t XC2362_info_print_flag =0;
extern param_data_t g_current_params;
// 初始化 CONV 引脚（上电后置高）
void XC2362_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ADC_CONV_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ADC_CONV_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_SET); // 初始高电平
}

void XC2362_DMA_Init(SPI_HandleTypeDef *hspi) {
    // 启动 SPI DMA 接收
    HAL_SPI_Receive_DMA(hspi, (uint8_t*)adcBuffer1, 1);

    // 初始化 CONV 引脚（上电后置高）
    HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_SET);
    XC2362_StartConversion();
}
void XC2362_StartConversion(void) {
  
    // 1. 拉低CONV引脚（下降沿触发ADC转换）
    HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_RESET);
    
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        uint16_t delay_i=0;
        // 1. 处理 ADC 数据（12 位转 16 位或其他格式）
        bsp_adc_buf[adc_count++] = adcBuffer1[0];
        if(adc_count>=2048)
        {
            adc_count=0;
            adc_ok=1;
            delay_i = count_front;
            while(delay_i--)
            {
            }
            HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_SET);

        }
        else {
            delay_i = count_front;
            while(delay_i--)
            {
            }
            HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_SET);
            //判断是否修改接收缓存，如果缓存已满，则切换接收缓存
            // 3. 启动下一次 ADC 转换
            XC2362_StartConversion();
            delay_i = count_back;
            HAL_SPI_DMAStop(&hspi1);
//            while(HAL_GPIO_ReadPin(ADC_CONV_PORT, ADC_CONV_PIN)== GPIO_PIN_SET)
//            {
//            }

            if (BOARD_SWITCH_FLAG==0)
            HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)adcBuffer1, 1);
        }
    }
}


//判断数值是否在范围内
//若 base 在 (compare*min_fact,compare*max_fact) 之间，则返回 1
uint8_t value_in_range(float base,float compare,float min_fact,float max_fact)
{
    float min_base = compare*min_fact;
    float max_base = compare*max_fact;
    if(base > min_base && base < max_base)
    {
        return 1;
    }
    else {
        return 0;
    }
}

void PreProcessSample(void)
{
    uint16_t i;
    uint16_t this_exceed_times = 0;
    static uint16_t idle_time = 0;
        for(i=0;i<BSP_ADC_BUF_DATA_SIZE;i++)
        {

            if(bsp_adc_buf[i]<4096)
                bsp_adc_statis_buf_size[1][bsp_adc_buf[i]] ++;
        }
//        statis_buf_save = !statis_buf_save;
//        statis_buf_send = !statis_buf_send;


}


static void current_sample_task_thread(void *parameter)
{

    float more_send_data[SEND_LINE+2];
    uint16_t i,j,k,need_send;
    uint16_t recent_high_sum;
    SlidingAvg_Init(&currentSlidAvgFilter,10);
    uint16_t delay_i=0;

    while (1)
    {
    /* USER CODE END WHILE */


      if(adc_ok && ota_start_flag==0)
      {

          adc_ok = 0;
          PreProcessSample();
          need_send = 0;
          need_send = 1;
          fstatis_average_num = 0;
          fstatis_high_sum = 0;
          {
//              for(j=0;j<SEND_LINE/2;j++)
//              {
//                  more_send_data[j] = (float)bsp_adc_buf[j]*2.5*1000.0/4096.0;
//              }
//              for(j=SEND_LINE/2;j<SEND_LINE;j++)
//              {
//                  more_send_data[j] = bsp_adc_buf[j];
//              }
              {
                  statis_max = 0;

                  recent_high_sum = 0;
                  for(statis_buf_send_pos=0;statis_buf_send_pos<BSP_ADC_STATIS_BUF_SIZE;statis_buf_send_pos++)
                  {
                      recent_high_sum += bsp_adc_statis_buf_size[1][BSP_ADC_STATIS_BUF_SIZE-1-statis_buf_send_pos];//从上往下累加
                      if(statis_max<1 && recent_high_sum>60)
                      {
                          statis_max = BSP_ADC_STATIS_BUF_SIZE-1-statis_buf_send_pos;
                      }
                      if(statis_max>1)//取到数据了，就退出
                          break;
                  }
                  for(statis_buf_send_pos=0;statis_buf_send_pos<BSP_ADC_STATIS_BUF_SIZE;statis_buf_send_pos++)
                  {
                      bsp_adc_statis_buf_size[1][statis_buf_send_pos] = 0;//使用的数据要删除，后面才好继续统计
                  }
                  {
                      fstatis_max = (float)statis_max*3.3*1000.0/4096.0;//将高值和低值记录
                      //将数值累计
                      fstatis_average_num = 1;
                      fstatis_high_sum = fstatis_max;
                  }

              }
              fstatis_high_average = fstatis_high_sum/fstatis_average_num;//计算平均值
              sample_voltage_f = SlidingAvg_Filter(&currentSlidAvgFilter,fstatis_high_average*10);
              if(!value_in_range(sample_voltage_f,g_send_info.sample_voltage_f,0.9,1.1))
              {
                  //超过范围了
                  g_send_info.sample_voltage_f = sample_voltage_f;

                  more_send_data[SEND_LINE] = g_send_info.sample_voltage_u16;

                  //数值变化了，进行发送
              }
              g_send_info.sample_voltage_u16=((uint16_t)fstatis_max*(g_current_params.temperature_calibration))/100;
//              g_send_info.sample_voltage_u16=adcBuffer1[0];
              monitor_board_inform_info_send();
              XC2362_StartConversion();
              if (XC2362_info_print_flag)
              {
                  rt_kprintf("%d \n",g_send_info.sample_voltage_u16);
              }
//              while(HAL_GPIO_ReadPin(ADC_CONV_PORT, ADC_CONV_PIN)== GPIO_PIN_SET)
//              {
//              }
//              HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)adcBuffer1, 1);
//              g_send_info.sample_voltage_u16 = sample_voltage_f;
//              XC2362_StartConversion();
//              while(HAL_GPIO_ReadPin(ADC_CONV_PORT, ADC_CONV_PIN)== GPIO_PIN_SET)
//              {
//              }
//              HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)adcBuffer1, 1);

//              HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)(&bsp_adc_buf[sample_count++]), 1);
//              HAL_GPIO_WritePin(ADC_CONV_PORT, ADC_CONV_PIN, GPIO_PIN_SET);
//              XC2362_StartConversion();
          }

      }
      XC2362_StartConversion();
      delay_i = count_back;
//            while(HAL_GPIO_ReadPin(ADC_CONV_PORT, ADC_CONV_PIN)== GPIO_PIN_SET)
//            {
//            }
      HAL_SPI_DMAStop(&hspi1);
      if (BOARD_SWITCH_FLAG==0 && ota_start_flag==0)
      HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)adcBuffer1, 1);


      rt_thread_mdelay(50);
    /* USER CODE BEGIN 3 */
    }
}








void current_sample_task_resoure_init(void)
{

}

void current_sample_task_init(void)
{
    rt_thread_t thread_sample;
    /* 创建数据接收线程 */
    thread_sample = rt_thread_create("sample", current_sample_task_thread, RT_NULL, 4096, 26, 10);
    if (thread_sample != RT_NULL)
    {
        rt_thread_startup(thread_sample);
    }
    else
    {
        rt_kprintf("create current sample thread failed!\n");
    }
}

static int XC2362_INFO_PRINTFLAG(int argc, char **argv)
{
    XC2362_info_print_flag = 1-XC2362_info_print_flag;

    if (XC2362_info_print_flag)
    {
        rt_kprintf("XC2362_info_print_flag  is enabled\n");
    }
    else
    {
        rt_kprintf("XC2362_info_print_flag  is disabled\n");
    }
    return 0;
}
MSH_CMD_EXPORT(XC2362_INFO_PRINTFLAG,"XC2362_INFO_PRINTFLAG");

static int SETXC2362_CALINUM_(int argc, char **argv)
{

    if (argc != 2)
        {
//            rt_kprintf("Usage: set_value <integer_value>\n");
//            rt_kprintf("Current value: %.2f\n", calibrate_num);
            return;
        }
        // 将传入的整数转换为浮点数，然后除以100
        int input_value = atoi(argv[1]);
        g_current_params.temperature_calibration = input_value ;

        params_save();
        rt_kprintf("Output: %2d . %2d\n", input_value/100, input_value%100);
}
MSH_CMD_EXPORT(SETXC2362_CALINUM_,"SETXC2362_CALINUM_");
static int GETXC2362_CALINUM_(int argc, char **argv)
{
        int input_value = g_current_params.temperature_calibration;

        rt_kprintf("Output: %2d . %2d\n", input_value/100, input_value%100);
}
MSH_CMD_EXPORT(GETXC2362_CALINUM_,"GETXC2362_CALINUM_");
