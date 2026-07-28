/**
 * @file    flash_params.h
 * @brief   Flash参数存储头文件
 */

#ifndef __PARA_STORAGE_H__
#define __PARA_STORAGE_H__

#include <rtthread.h>

/* 参数数据结构 */
typedef struct {
    uint32_t magic;

    /* 用户参数区域 */
    uint32_t  temperature_calibration;

} param_data_t;

/* 函数声明 */
rt_err_t params_system_init(void);
param_data_t *params_get(void);
rt_err_t params_save(void);

#endif /* __FLASH_PARAMS_H__ */
