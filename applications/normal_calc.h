#ifndef    _NORMAL_CALC_H_
#define    _NORMAL_CALC_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/


#define ADC_TO_VOL(val)                                      ((float)val*3.3/4096)

#define PRINT_1000xFLOAT(f)                                  (int32_t)(f*1000)
#define PRINT_100xFLOAT(f)                                   (int32_t)(f*100)
#define PRINT_10xFLOAT(f)                                    (int32_t)(f*10)
#define PRINT_FLOAT(f)                                       (int32_t)(f)

#define GET_MAX(m1,m2)                                       ((m1)>(m2)?(m1):(m2))//找两个数的最大值

#if    1
//下列宏中，类型定义要相互符合，例如 U16_ToBIT_ENCRY 就处理u16类型
//下面的宏，还需要升级，改成更复杂的按位加解密
#define U8_ToBIT_ENCRY(_u8_)                      (((_u8_&0xf0)>>4) | ((_u8_&0x0f)<<4)) //1字节的按位加密
#define U8_ToBIT_DECOD(_u8_)                      (((_u8_&0xf0)>>4) | ((_u8_&0x0f)<<4)) //1字节的按位解密
#define U16_ToBIT_ENCRY(_u16_)                    (((_u16_&0x000f)<<12) | ((_u16_&0x00f0)>>4) | ((_u16_&0x0f00)>>4) | ((_u16_&0xf000)>>4)) //2字节的按位加密
#define U16_ToBIT_DECOD(_u16_)                    (((_u16_&0x000f)<<4) | ((_u16_&0x00f0)<<4) | ((_u16_&0x0f00)<<4) | ((_u16_&0xf000)>>12)) //2字节的按位解密
#define U32_ToBIT_ENCRY(_u32_)                    (((_u32_&0x0000000f)<<28) | ((_u32_&0x000000f0)>>4) | ((_u32_&0x00000f00)>>4) | ((_u32_&0x0000f000)>>4) | \
                                                   ((_u32_&0x000f0000)>>4) | ((_u32_&0x00f00000)>>4) | ((_u32_&0x0f000000)>>4) | ((_u32_&0xf0000000)>>4)) //4字节的按位加密
#define U32_ToBIT_DECOD(_u32_)                    (((_u32_&0x0000000f)<<4) | ((_u32_&0x000000f0)<<4) | ((_u32_&0x00000f00)<<4) | ((_u32_&0x0000f000)<<4) | \
                                                   ((_u32_&0x000f0000)<<4) | ((_u32_&0x00f00000)<<4) | ((_u32_&0x0f000000)<<4) | ((_u32_&0xf0000000)>>28)) //4字节的按位解密
#else
#define _4BIT_REVER(_u8_)                         ((((_u8_)&0xf0)>>4)|((_u8_)&0x0f)<<4)
#define U8_ToBIT_ENCRY(_u8_)                      (encry_array[_4BIT_REVER(_u8_)]) //1字节的加密
#define U8_ToBIT_DECOD(_u8_)                      (decode_array[_4BIT_REVER(_u8_)]) //1字节的解密
#define U16_ToBIT_ENCRY(_u16_)                    (U8_ToBIT_ENCRY(_u16_ & 0xff) | (U8_ToBIT_ENCRY((_u16_>>8) & 0xff)<<8)) //2字节的加密
#define U16_ToBIT_DECOD(_u16_)                    (U8_ToBIT_DECOD(_u16_ & 0xff) | (U8_ToBIT_DECOD((_u16_>>8) & 0xff)<<8)) //2字节的解密
#define U32_ToBIT_ENCRY(_u32_)                    (U16_ToBIT_ENCRY(_u32_ & 0xffff) | (U16_ToBIT_ENCRY((_u32_>>16) & 0xffff)<<16)) //4字节的加密
#define U32_ToBIT_DECOD(_u32_)                    (U16_ToBIT_DECOD(_u32_ & 0xffff) | (U16_ToBIT_DECOD((_u32_>>16) & 0xffff)<<16)) //4字节的解密
#endif
/* Public typedef ------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

extern const uint8_t encry_array[256];
extern const uint8_t decode_array[256];

/* Public function prototypes ------------------------------------------------*/



uint16_t modbus_rtu_crc16(uint8_t *buffer, uint16_t buffer_length);
uint16_t modbus_rtu_crc16_frame(uint8_t *buffer, uint16_t buffer_length,uint16_t init_crc);

void write_u16_to_buf(uint16_t data_u16,uint8_t *buf);
void write_u32_to_buf(uint32_t data_u32,uint8_t *buf);
uint16_t get_u16_from_buf(uint8_t *buf);
uint32_t get_u32_from_buf(uint8_t *buf);


void my_event_set_bit(uint32_t *ev,uint32_t set_mask);
void my_event_clear_bit(uint32_t *ev,uint32_t clear_mask);
void my_event_clear(uint32_t *ev);
uint32_t my_event_val(uint32_t *ev);


float GetTemperature(float vol);
float GetChipTemperature(float vol);

void UID_to_16Byte(uint8_t *U_8byte);


void print_hex(const void *data, size_t length, size_t bytes_per_line);
#endif


