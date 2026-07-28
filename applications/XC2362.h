#ifndef __XC2362_H
#define __XC2362_H


#include "main.h"


#define ADC_CONV_PIN    GPIO_PIN_15  // CONV
#define ADC_CONV_PORT   GPIOA       // CONV

void XC2362_Init(void); //
uint16_t XC2362_ReadADC(SPI_HandleTypeDef *hspi);
void XC2362_DMA_Init(SPI_HandleTypeDef *hspi);
void XC2362_StartConversion(void);
void XC2362_Pross_Senddata(void);
void current_sample_task_init(void);
#endif /* __XC2362_H */
