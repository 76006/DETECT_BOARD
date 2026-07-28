/* Includes ------------------------------------------------------------------*/

#include "slid_ave_filter.h"
#include <rtthread.h>
/* external variables --------------------------------------------------------*/

/* external typedef ----------------------------------------------------------*/

/* external function prototypes ----------------------------------------------*/

/* Private define ------------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Function implementation ---------------------------------------------------*/



void SlidingAvg_Init(SlidingAvgFilter_t *filter,uint16_t slid_window_size)
{
    if(slid_window_size == 0)
    {
        rt_kprintf("slid_window_size error\n");
        return ;
    }

    filter->buffer = rt_malloc(sizeof(float)*slid_window_size);
    if(filter->buffer == NULL)
    {
        rt_kprintf("SlidingAvg_Init malloc error\n");
        return ;
    }

    filter->slid_window_size = slid_window_size;
    // 初始化缓冲区为0
    for (uint8_t i = 0; i < slid_window_size; i++) {
        filter->buffer[i] = 0.0f;
    }
    
    filter->index = 0;
    filter->count = 0;
    filter->sum = 0.0f;
}

float SlidingAvg_Filter(SlidingAvgFilter_t *filter, float new_data) {
    // 如果缓冲区未满，先累加新数据
    if (filter->count < filter->slid_window_size) {
        filter->sum += new_data;
        filter->buffer[filter->index] = new_data;
        filter->count++;
    } else {
        // 缓冲区已满，先减去旧数据
        filter->sum -= filter->buffer[filter->index];
        // 添加新数据
        filter->sum += new_data;
        filter->buffer[filter->index] = new_data;
    }
    
    // 更新索引（循环缓冲区）
    filter->index = (filter->index + 1) % filter->slid_window_size;
    
    // 返回平均值
    return filter->sum / (float)filter->count;
}


void SlidingAvg_destory(SlidingAvgFilter_t *filter)
{
    rt_free(filter->buffer);
}







