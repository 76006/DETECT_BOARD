#ifndef    _LCR_CALC_H_
#define    _LCR_CALC_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "user_define.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/




LCR_Mod_t LCR_Modulate(FRE_t set_fre,float *wave_buf, uint16_t wave_size);


void LCR_Mea_Calc(LCR_Mea_t *get_mea,FRE_t set_fre,float set_RF,LCR_Mod_t mVo,LCR_Mod_t mVx);

#endif


