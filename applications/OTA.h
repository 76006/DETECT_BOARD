/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-04     guozhuang       the first version
 */
#ifndef APPLICATIONS_OTA_H_
#define APPLICATIONS_OTA_H_
#define APP_START_ADDR    0x08020000
#define OTA_AREA_START  0x08040000//定义片内起始地址
#define OTA_AREA_SIZE   (256 * 1024)//定义存放的空间
uint32_t ota_file_size = 0;
#endif /* APPLICATIONS_OTA_H_ */
