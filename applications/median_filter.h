#ifndef    _MEDIAN_FILTER_H_
#define    _MEDIAN_FILTER_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

#define MEDIAN_FILTER_TYPE                                  float

/* Public typedef ------------------------------------------------------------*/

typedef struct {
    uint8_t median_window_full;                  // 窗口是否已填满
    uint8_t median_index;                        // 当前写入位置
    uint32_t median_filter_window_size;//中值滤波窗口大小，必须为奇数（3,5,7,...）
    MEDIAN_FILTER_TYPE *median_window;  // 全局滑动窗口
    // 复制窗口数据用于排序（避免修改原数据）
    MEDIAN_FILTER_TYPE *sorted_window;
}Median_Filter_t;
typedef enum{
    MEDIAN_FILTER_ERROR_OK,        //正常
    MEDIAN_FILTER_ERROR_PARA,      //参数错误
    MEDIAN_FILTER_ERROR_NO_MEM,    //没有内存空间
}Median_Filter_Error_t;

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

Median_Filter_Error_t median_filter_init(Median_Filter_t *mf,uint32_t median_filter_window_size);


MEDIAN_FILTER_TYPE median_filter_handle(Median_Filter_t *mf,MEDIAN_FILTER_TYPE input);


MEDIAN_FILTER_TYPE median_filter_destory(Median_Filter_t *mf);

#endif


