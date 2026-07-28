
#ifndef __WS2812B_H__
#define __WS2812B_H__

#include "main.h"
#define PWM_FREQ     800000   // 800kHz
#define PWM_ARR      (168000000/PWM_FREQ - 1)  // =209
#define T0H          59       // 0码高电平(0.35μs)
#define T1H          117      // 1码高电平(0.7μs)
#define RESET_CYCLES 64       // 80μs/1.25μs
#define LED_NUM 42
void WS2812B_SetColor(uint8_t ledNum, uint8_t red, uint8_t green, uint8_t blue);
void WS2812B_Update(void);
void WS2812B_task_init(void);
extern uint8_t rgb_change_flag;
#endif

