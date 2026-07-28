#ifndef    _SLID_AVE_FILTER_H_
#define    _SLID_AVE_FILTER_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/


// 滤波器状态结构体
typedef struct {
    uint16_t slid_window_size;
	uint16_t index;                          // 当前写入位置
    uint16_t count;                          // 当前已存储的数据量
    float sum;                              // 当前缓冲区总和（用于优化计算）
    float *buffer;  // 数据缓冲区
} SlidingAvgFilter_t;

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/




// 初始化滤波器
void SlidingAvg_Init(SlidingAvgFilter_t *filter,uint16_t slid_window_size);

// 添加新数据并计算滤波结果
float SlidingAvg_Filter(SlidingAvgFilter_t *filter, float new_data);


void SlidingAvg_destory(SlidingAvgFilter_t *filter);

#endif


