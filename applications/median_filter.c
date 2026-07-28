/* Includes ------------------------------------------------------------------*/
#include <rtthread.h>
#include <stdlib.h>  // 用于 qsort
#include <string.h>
#include "median_filter.h"
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




Median_Filter_Error_t median_filter_init(Median_Filter_t *mf,uint32_t median_filter_window_size)
{
    // 检查窗口大小是否为奇数
    if(median_filter_window_size == 0 || (median_filter_window_size % 2 == 0))
    {
        rt_kprintf("median_filter_window_size error\n");
        return MEDIAN_FILTER_ERROR_PARA;
    }
    mf->median_index = 0;
    mf->median_window_full = 0;
    mf->median_filter_window_size = median_filter_window_size;
    mf->median_window = rt_malloc(sizeof(MEDIAN_FILTER_TYPE)*median_filter_window_size);
    if(mf->median_window == NULL)
    {
        return MEDIAN_FILTER_ERROR_NO_MEM;
    }
    mf->sorted_window = rt_malloc(sizeof(MEDIAN_FILTER_TYPE)*median_filter_window_size);
    if(mf->sorted_window == NULL)
    {
        rt_free(mf->median_window);
        return MEDIAN_FILTER_ERROR_NO_MEM;
    }
    return MEDIAN_FILTER_ERROR_OK;
}





/**
 * @brief qsort所需的浮点数比较函数
 * @param a 第一个比较值指针
 * @param b 第二个比较值指针
 * @return 比较结果：-1(a<b), 0(a==b), 1(a>b)
 */
static int median_filter_compare(const void *a, const void *b) {
    MEDIAN_FILTER_TYPE fa = *(const MEDIAN_FILTER_TYPE *)a;
    MEDIAN_FILTER_TYPE fb = *(const MEDIAN_FILTER_TYPE *)b;
    return (fa > fb) ? 1 : ((fa < fb) ? -1 : 0);
}
/**
 * @brief 中值滤波函数
 * @param input 输入数据（MEDIAN_FILTER_TYPE）
 * @return 滤波后的结果
 */
MEDIAN_FILTER_TYPE median_filter_handle(Median_Filter_t *mf,MEDIAN_FILTER_TYPE input)
{
    // 更新窗口数据（FIFO）
    mf->median_window[mf->median_index] = input;
    mf->median_index++;
    if(mf->median_index>=mf->median_filter_window_size)
    {
        mf->median_index = 0;
        mf->median_window_full = 1;  // 标记窗口已填满
    }

    // 如果窗口未填满，直接返回当前输入
    if (!mf->median_window_full && mf->median_index != 0) {
        return input;
    }



    memcpy(mf->sorted_window, mf->median_window, mf->median_filter_window_size*sizeof(MEDIAN_FILTER_TYPE));

    // 使用 qsort 排序
    qsort(mf->sorted_window, mf->median_filter_window_size, sizeof(MEDIAN_FILTER_TYPE), median_filter_compare);

    // 取中值
    return mf->sorted_window[mf->median_filter_window_size / 2];
}

MEDIAN_FILTER_TYPE median_filter_destory(Median_Filter_t *mf)
{
    if(mf == NULL)
        return MEDIAN_FILTER_ERROR_PARA;
    rt_free(mf->median_window);
    rt_free(mf->sorted_window);
}


