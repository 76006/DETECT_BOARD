/**
 * @file    flash_params.c
 * @brief   Flash参数存储模块
 * @details 使用STM32F407VGT6的第四个128KB扇区存储参数
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include "para_storage.h"

/* Flash相关定义 */
#define FLASH_START_ADDR         0x08000000     // Flash起始地址
#define FLASH_SIZE               (1024 * 1024)  // 1MB Flash
#define FLASH_PAGE_SIZE          0x20000        // 128KB扇区大小

/* 参数存储区域定义 - 第四个128KB扇区 */
#define PARAM_FLASH_SECTOR       FLASH_SECTOR_8              //
#define PARAM_FLASH_ADDR         (FLASH_START_ADDR + (4 * FLASH_PAGE_SIZE))
#define PARAM_FLASH_SIZE         FLASH_PAGE_SIZE // 128KB


/* 全局变量 */
param_data_t g_current_params;
static rt_bool_t g_params_loaded = RT_FALSE;

/* CRC32计算函数 */
static uint32_t calculate_crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t i, j;

    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

/* Flash解锁和锁定 */
static rt_err_t flash_unlock(void)
{
    HAL_FLASH_Unlock();
    return RT_EOK;
}

static rt_err_t flash_lock(void)
{
    HAL_FLASH_Lock();
    return RT_EOK;
}

/* 擦除Flash扇区 */
static rt_err_t flash_erase_sector(uint32_t sector)
{
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error;

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = sector;
    erase_init.NbSectors = 1;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK) {
        rt_kprintf("Flash erase failed! Sector: %d\n", sector);
        return RT_ERROR;
    }
    uint32_t verify_data = *(volatile uint32_t*)(4*FLASH_PAGE_SIZE+FLASH_START_ADDR);
    rt_kprintf("After erase - Address 0x%08X: 0x%08lX\n",
            sector*FLASH_PAGE_SIZE, verify_data);
    return RT_EOK;
}

/* 写入数据到Flash */
static rt_err_t flash_write(uint32_t address, param_data_t *params, uint32_t length)
{
//    uint32_t i;

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + 0, params->magic) != HAL_OK) {
            rt_kprintf("Flash program failed at address: 0x%08X\n", address);
            return RT_ERROR;
        }
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + 0x04, params->temperature_calibration) != HAL_OK) {
            rt_kprintf("Flash program failed at address: 0x%08X\n", address + 4);
            return RT_ERROR;
        }
    return RT_EOK;
}

/* 从Flash读取参数 */
static rt_err_t params_read_from_flash(param_data_t *params)
{
    const param_data_t *flash_params = (const param_data_t *)PARAM_FLASH_ADDR;

    // 检查魔数
    if (flash_params->magic != 0x55AA55AA) {
        rt_kprintf("Invalid magic number in flash\n");
        return RT_ERROR;
    }


    // 复制数据
    memcpy(params, flash_params, sizeof(param_data_t));

    return RT_EOK;
}

/* 保存参数到Flash */
static rt_err_t params_save_to_flash(const param_data_t *params)
{
    rt_err_t result = RT_EOK;
    param_data_t temp_params;

    // 复制参数并计算CRC
    memcpy(&temp_params, params, sizeof(param_data_t));
    temp_params.magic = 0x55AA55AA;

    // 解锁Flash
    if (flash_unlock() != RT_EOK) {
        return RT_ERROR;
    }

    // 擦除扇区
    if (flash_erase_sector(PARAM_FLASH_SECTOR) != RT_EOK) {
        flash_lock();
        return RT_ERROR;
    }

    // 写入数据
    if (flash_write(PARAM_FLASH_ADDR, &temp_params, sizeof(param_data_t)) != RT_EOK) {
        result = RT_ERROR;
    }

    // 锁定Flash
    flash_lock();

    if (result == RT_EOK) {
        rt_kprintf("Parameters saved to flash successfully\n");
    } else {
        rt_kprintf("Failed to save parameters to flash\n");
    }

    return result;
}

/* 初始化默认参数 */
static void params_init_default(param_data_t *params)
{
    memset(params, 0, sizeof(param_data_t));

    params->magic = 0x55AA55AA;

    // 设置默认值
    params->temperature_calibration = 200;

}

/* 初始化参数系统 */
rt_err_t params_system_init(void)
{
    rt_err_t ret;

    rt_kprintf("Initializing parameter system...\n");
    rt_kprintf("Flash address: 0x%08X, Size: %d KB\n",
               PARAM_FLASH_ADDR, PARAM_FLASH_SIZE / 1024);

    // 尝试从Flash读取参数
    ret = params_read_from_flash(&g_current_params);

    if (ret != RT_EOK) {
        rt_kprintf("No valid parameters in flash, using defaults\n");
        params_init_default(&g_current_params);

        // 保存默认参数到Flash
        params_save_to_flash(&g_current_params);
        g_params_loaded = RT_TRUE;
        return RT_EOK;
    } else {
        rt_kprintf("Parameters loaded from flash successfully\n");
    }

    // 增加启动计数
//    params_save_to_flash(&g_current_params);

    g_params_loaded = RT_TRUE;
    return RT_EOK;
}

/* 获取参数指针 */
param_data_t *params_get(void)
{
    if (!g_params_loaded) {
        rt_kprintf("Parameters not loaded yet!\n");
        return RT_NULL;
    }
    return &g_current_params;
}

/* 保存参数 */
rt_err_t params_save(void)
{
    if (!g_params_loaded) {
        return RT_ERROR;
    }
    return params_save_to_flash(&g_current_params);
}

/* 参数系统测试命令 */
static void params_test_cmd(int argc, char **argv)
{
    param_data_t *params = params_get();

    if (params == RT_NULL) {
        rt_kprintf("Parameters not available\n");
        return;
    }

    if (argc == 1) {
        // 显示当前参数
        rt_kprintf("=== Current Parameters ===\n");
        rt_kprintf("Magic: 0x%08X\n", params->magic);

        rt_kprintf("Temperature Offset: %d\n", params->temperature_calibration);


    } else if (argc == 3) {
        // 修改参数
        if (strcmp(argv[1], "name") == 0) {

        } else if (strcmp(argv[1], "temp_offset") == 0) {
            params->temperature_calibration = atoi(argv[2]);
            rt_kprintf("Temperature offset set to: %d\n", params->temperature_calibration);
        } else {
            rt_kprintf("Unknown parameter: %s\n", argv[1]);
            return;
        }

        // 保存修改
        if (params_save() == RT_EOK) {
            rt_kprintf("Parameters saved successfully\n");
        } else {
            rt_kprintf("Failed to save parameters\n");
        }
    } else {
        rt_kprintf("Usage:\n");
        rt_kprintf("params_test                  - show current parameters\n");
        rt_kprintf("params_test name <new_name>  - set device name\n");
        rt_kprintf("params_test temp_offset <value> - set temperature offset\n");
    }
}

/* 导出到MSH命令 */
MSH_CMD_EXPORT(params_test_cmd, parameter system test);

/* 自动初始化 */
//INIT_APP_EXPORT(params_system_init);
