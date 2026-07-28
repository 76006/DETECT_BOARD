///*
// * Copyright (c) 2006-2021, RT-Thread Development Team
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2025-07-09     guozhuang       the first version
// */
#ifndef APPLICATIONS_DAC_H_
#define APPLICATIONS_DAC_H_
#include "main.h"


typedef struct {
    const uint16_t *set_wave_buf;
    uint16_t set_wave_size;
    uint16_t is_init;
    FRE_t set_fre;
}USER_DAC_t;


/* Public variables ----------------------------------------------------------*/

extern const uint16_t user_dac_wave_10khz_buf[DAC_WAVE_10KHZ_BUF_SIZE];
extern const uint16_t user_dac_wave_1khz_buf[DAC_WAVE_1KHZ_BUF_SIZE];
extern const uint16_t user_dac_wave_100hz_buf[DAC_WAVE_100HZ_BUF_SIZE];

/* Public function prototypes ------------------------------------------------*/

//void User_Dac_Set_Fre(FRE_t Fre);
//
//void User_Dac_Init(void);

void User_Dac_Dma_Start(void);
void User_Dac_Set_Fre(FRE_t Fre);
void User_Dac_Dma_Stop(void);
int dac_dma_init(void);
void User_Dac_Init(void);
#endif /* APPLICATIONS_DAC_H_ */
