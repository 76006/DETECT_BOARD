/* Includes ------------------------------------------------------------------*/
#include "normal_calc.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>
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

rt_base_t level;



/** Table of CRC values for high-order byte */
static const uint8_t _table_crc_hi[] =
    {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

/** Table of CRC values for low-order byte */
static const uint8_t _table_crc_lo[] =
    {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40};
/**
 * @}
 */

/** @defgroup RTU_Private_Functions RTU Private Functions
 * @{
 */

/**
 * @brief   RTU CRC16 计算
 * @param   buffer 数据指针
 * @param   buffer_length 数据长度
 * @return  CRC16 值
 */
uint16_t modbus_rtu_crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ _table_crc_hi[i];
        crc_lo = _table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}

uint16_t modbus_rtu_crc16_frame(uint8_t *buffer, uint16_t buffer_length,uint16_t init_crc)
{
    uint8_t crc_hi = init_crc>>8; /* high CRC byte initialized */
    uint8_t crc_lo = init_crc & 0xff; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ _table_crc_hi[i];
        crc_lo = _table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}


#if    1  //小端模式(单片机存储 加密芯片存储使用)
//例缓存中 buf[2] = {0x01,0x00},则对应数据 0x0001
void write_u16_to_buf(uint16_t data_u16,uint8_t *buf)
{
    buf[0] = data_u16&0xff;
    buf[1] = data_u16>>8;
}

void write_u32_to_buf(uint32_t data_u32,uint8_t *buf)
{
    buf[0] = (data_u32)&0xff;
    buf[1] = (data_u32>>8)&0xff;
    buf[2] = (data_u32>>16)&0xff;
    buf[3] = (data_u32>>24)&0xff;
}

uint16_t get_u16_from_buf(uint8_t *buf)
{
    uint16_t data_u16;
    data_u16 = (buf[0]) | (buf[1]<<8);
    return data_u16;
}


uint32_t get_u32_from_buf(uint8_t *buf)
{
    uint32_t data_u32;
    data_u32 = (buf[0]) | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24);
    return data_u32;
}

#else   //大端模式

void write_u16_to_buf(uint16_t data_u16,uint8_t *buf)
{
    buf[0] = data_u16>>8;
    buf[1] = data_u16&0xff;
}

void write_u32_to_buf(uint32_t data_u32,uint8_t *buf)
{
    buf[0] = (data_u32>>24)&0xff;
    buf[1] = (data_u32>>16)&0xff;
    buf[2] = (data_u32>>8)&0xff;
    buf[3] = (data_u32)&0xff;
}

uint16_t get_u16_from_buf(uint8_t *buf)
{
    uint16_t data_u16;
    data_u16 = (buf[0]<<8) | buf[1];
    return data_u16;
}

uint32_t get_u32_from_buf(uint8_t *buf)
{
    uint32_t data_u32;
    data_u32 = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | (buf[3]);
    return data_u32;
}
#endif





//自己用的针对全局变量的时间读写、置位清位等函数
void my_event_set_bit(uint32_t *ev,uint32_t set_mask)
{
    ENTER_PROTECT_GLOBAL_VAR();
    *ev |= set_mask;
    EXIT_PROTECT_GLOBAL_VAR();
}

void my_event_clear_bit(uint32_t *ev,uint32_t clear_mask)
{
    ENTER_PROTECT_GLOBAL_VAR();
    *ev &= ~clear_mask;
    EXIT_PROTECT_GLOBAL_VAR();
}

void my_event_clear(uint32_t *ev)
{
    ENTER_PROTECT_GLOBAL_VAR();
    *ev = 0;
    EXIT_PROTECT_GLOBAL_VAR();
}
uint32_t my_event_val(uint32_t *ev)
{
    uint32_t temp = 0;
    ENTER_PROTECT_GLOBAL_VAR();
    temp = *ev ;
    EXIT_PROTECT_GLOBAL_VAR();
    return temp;
}




















// 定义一些常量
#define NTC_V_REF           3.3 // 参考电压
#define R_SERIES 10000 // 串联的10KΩ电阻

#if    1//直插型NTC热敏电阻
// 假设我们有一个简化的查找表，这里只列出了几个点作为示例
float tempLookupTable[][2] = {
    {-40.0f, 197390.0f}, // -40°C时的电阻值
    {-30.0f, 114340.0f}, // -30°C时的电阻值
    {-20.0f, 68915.0f}, // -20°C时的电阻值
    {-10.0f, 42889.0f}, // -10°C时的电阻值
    {0.0f, 32116.0f}, // 0°C时的电阻值
    {25.0f, 10000.0f}, // 25°C时的电阻值
    {30.0f, 8047.0f}, // 30°C时的电阻值
    {35.0f, 6523.0f}, // 35°C时的电阻值
    {40.0f, 5318.0f}, // 40°C时的电阻值
    {45.0f, 4357.0f}, // 45°C时的电阻值
    {50.0f, 3588.0f}, // 50°C时的电阻值
    {60.0f, 2466.0f}, // 60°C时的电阻值
    {75.0f, 1452.0f}, // 75°C时的电阻值
    {90.0f, 890.0f}, // 90°C时的电阻值
    {100.0f, 657.0f} // 100°C时的电阻值
};
#elif    1
// 假设我们有一个简化的查找表，这里只列出了几个点作为示例
float tempLookupTable[][2] = {
    {-40.0f, 197390.0f}, // -40°C时的电阻值
    {-35.0f, 149390.0f}, // -35°C时的电阻值
    {-30.0f, 114340.0f}, // -30°C时的电阻值
    {-25.0f, 88381.0f}, // -25°C时的电阻值
    {-20.0f, 68915.0f}, // -20°C时的电阻值
    {-15.0f, 54166.0f}, // -15°C时的电阻值
    {-10.0f, 42889.0f}, // -10°C时的电阻值
    {-5.0f, 34169.0f}, // -5°C时的电阻值
    {0.0f, 27445.0f}, // 0°C时的电阻值
    {5.0f, 22165.0f}, // 5°C时的电阻值
    {10.0f, 18010.0f}, // 10°C时的电阻值
    {15.0f, 14720.0f}, // 15°C时的电阻值
    {20.0f, 12099.0f}, // 20°C时的电阻值
    {25.0f, 10000.0f}, // 25°C时的电阻值
    {30.0f, 8309.0f}, // 30°C时的电阻值
    {35.0f, 6939.0f}, // 35°C时的电阻值
    {40.0f, 5824.0f}, // 40°C时的电阻值
    {45.0f, 4911.0f}, // 45°C时的电阻值
    {50.0f, 4160.0f}, // 50°C时的电阻值
    {55.0f, 3539.0f}, // 55°C时的电阻值
    {60.0f, 3024.0f}, // 60°C时的电阻值
    {65.0f, 2593.0f}, // 65°C时的电阻值
    {70.0f, 2233.0f}, // 70°C时的电阻值
    {75.0f, 1929.0f}, // 75°C时的电阻值
    {80.0f, 1673.0f}, // 80°C时的电阻值
    {85.0f, 1455.0f}, // 85°C时的电阻值
    {90.0f, 1270.0f}, // 90°C时的电阻值
    {95.0f, 1112.0f}, // 95°C时的电阻值
    {100.0f, 976.0f}, // 100°C时的电阻值
    {110.0f, 759.0f}, // 110°C时的电阻值
    {125.0f, 532.0f} // 125°C时的电阻值
};
#endif



float Interpolate(float x, float x1, float y1, float x2, float y2)
{
    // 线性插值公式
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}
#define TABLE_SIZE                      sizeof(tempLookupTable)/sizeof(tempLookupTable[0])
// 二分查找函数
int binarySearch(float targetResistance) {
//    int mid;
//    int low=0,high=TABLE_SIZE - 2;
//    while (low < high) {
//        mid = low + (high - low) / 2;
//        if (tempLookupTable[mid][1] >= targetResistance && tempLookupTable[mid+1][1] <= targetResistance) {
//            return mid; // 找到精确匹配
//        } else if (tempLookupTable[mid][1] < targetResistance) {
//            high = mid - 1;
//        } else {
//            low = mid + 1;
//        }
//        //rt_kprintf("low=%d,high=%d,mid=%d\n  ",low,high,mid);
//    }
//    return low; // 返回大于目标值的第一个索引

    // 二分查找
    int left = 0;
    int right = TABLE_SIZE - 2;  // 保证能取到i+1
    int i = 0;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        float current = tempLookupTable[mid][1];
        float next = tempLookupTable[mid+1][1];

        if (current >= targetResistance && next <= targetResistance) {
            i = mid;
            break;
        } else if (current < targetResistance) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return i;
}


float EstimateTemperature(float resistance)
{
#if    0 //使用顺序查表法
    int i;
    // 线性插值查找温度
    for (i = 0; i < TABLE_SIZE-1; i++) // 只遍历前四行，因为最后一行是边界
    {
        if (resistance <= tempLookupTable[i][1] && resistance >= tempLookupTable[i+1][1])
        {
            //rt_kprintf("%f,%f,%f,%f,%f\n",resistance, tempLookupTable[i][1], tempLookupTable[i][0], tempLookupTable[i+1][1], tempLookupTable[i+1][0]);
            return Interpolate(resistance, tempLookupTable[i][1], tempLookupTable[i][0], tempLookupTable[i+1][1], tempLookupTable[i+1][0]);
        }
    }
    // 如果电阻值不在表中，则返回一个边界温度
    return (resistance >= tempLookupTable[0][1]) ? tempLookupTable[0][0] : tempLookupTable[TABLE_SIZE-1][0];
#elif    1//采用二分法
    // 使用二分查找找到目标阻值的位置
    int index ;
    if (resistance >= tempLookupTable[0][1]) {
        return tempLookupTable[0][0]; // 目标值小于最小阻值，返回最低温度
    }
    if (resistance <= tempLookupTable[TABLE_SIZE - 1][1]) {
        return tempLookupTable[TABLE_SIZE - 1][0]; // 目标值大于最大阻值，返回最高温度
    }
    index = binarySearch(resistance);

    return Interpolate(resistance, tempLookupTable[index][1], tempLookupTable[index][0], tempLookupTable[index+1][1], tempLookupTable[index+1][0]);
#endif
}


float GetTemperature(float vol)//输入NTC的分压，返回这个NTC所代表的温度
{
    float R;
    //通过电压，计算出电阻
    R = vol/(NTC_V_REF-vol)*10000;
    //rt_kprintf("vol=%f,R=%f  ",vol,R);
    return EstimateTemperature(R);
}

int TemperatureTest(void)
{
    float R = 0,temp;
    while(R<205000)
    {
        if(R<1000)
        {
            R = R+100;
        }
        else if(R<10000)
        {
            R = R+500;
        }
        else if(R<100000)
        {
            R = R+2000;
        }
        else
        {
            R = R+10000;
        }
        temp = EstimateTemperature(R);
        rt_kprintf("R=%d,temp=%d\n",(int32_t)R,(int32_t)(temp*100));
    }
    return 0;
}
//INIT_APP_EXPORT(TemperatureTest);


#define CHIP_TEMP_V25                                   (1.43)
#define CHIP_TEMP_AVG_SLOPE                             (4.3/1000)
float GetChipTemperature(float vol)//输入采集到的以V为单位的电压，输出以摄氏度为单位的温度
{
    //rt_kprintf("vol=%d",PRINT_100xFLOAT(vol));
    return ((CHIP_TEMP_V25-vol)/CHIP_TEMP_AVG_SLOPE)+25;
}

#if     0//不启用加密和解密
const uint8_t encry_array[256] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
250, 251, 252, 253, 254, 255,
};
const uint8_t decode_array[256] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
250, 251, 252, 253, 254, 255,
};
#else //启用加密和解密
const uint8_t encry_array[256] = {
80, 139, 5, 85, 96, 149, 20, 56, 78, 74,
171, 118, 226, 181, 152, 194, 223, 229, 145, 236,
6, 251, 122, 143, 55, 216, 140, 4, 158, 242,
111, 210, 68, 231, 35, 233, 71, 39, 211, 126,
93, 83, 123, 244, 37, 67, 240, 76, 141, 142,
193, 77, 155, 3, 174, 182, 212, 0, 45, 73,
198, 222, 227, 64, 137, 250, 40, 16, 189, 156,
154, 166, 176, 246, 27, 7, 34, 133, 213, 57,
94, 121, 249, 28, 23, 130, 88, 17, 253, 255,
115, 89, 90, 10, 168, 201, 239, 188, 101, 91,
220, 153, 164, 172, 52, 42, 72, 232, 224, 190,
61, 116, 237, 8, 60, 207, 53, 31, 119, 238,
184, 22, 9, 129, 79, 51, 205, 192, 247, 124,
245, 162, 135, 66, 12, 206, 230, 165, 26, 24,
50, 204, 95, 43, 161, 36, 138, 30, 180, 179,
159, 92, 107, 214, 97, 157, 131, 209, 19, 58,
221, 65, 241, 218, 125, 105, 177, 44, 186, 69,
200, 59, 108, 252, 32, 103, 195, 41, 234, 117,
112, 225, 208, 175, 202, 109, 235, 38, 183, 98,
81, 196, 104, 54, 150, 219, 2, 100, 187, 228,
147, 128, 25, 197, 47, 203, 114, 248, 48, 254,
82, 87, 148, 243, 146, 70, 127, 49, 178, 163,
132, 102, 18, 170, 86, 106, 14, 1, 173, 151,
33, 185, 217, 215, 75, 84, 13, 199, 62, 167,
144, 191, 136, 29, 11, 15, 21, 160, 169, 110,
99, 46, 120, 113, 134, 63,
};
const uint8_t decode_array[256] = {
57, 227, 196, 53, 27, 2, 20, 75, 113, 122,
93, 244, 134, 236, 226, 245, 67, 87, 222, 158,
6, 246, 121, 84, 139, 202, 138, 74, 83, 243,
147, 117, 174, 230, 76, 34, 145, 44, 187, 37,
66, 177, 105, 143, 167, 58, 251, 204, 208, 217,
140, 125, 104, 116, 193, 24, 7, 79, 159, 171,
114, 110, 238, 255, 63, 161, 133, 45, 32, 169,
215, 36, 106, 59, 9, 234, 47, 51, 8, 124,
0, 190, 210, 41, 235, 3, 224, 211, 86, 91,
92, 99, 151, 40, 80, 142, 4, 154, 189, 250,
197, 98, 221, 175, 192, 165, 225, 152, 172, 185,
249, 30, 180, 253, 206, 90, 111, 179, 11, 118,
252, 81, 22, 42, 129, 164, 39, 216, 201, 123,
85, 156, 220, 77, 254, 132, 242, 64, 146, 1,
26, 48, 49, 23, 240, 18, 214, 200, 212, 5,
194, 229, 14, 101, 70, 52, 69, 155, 28, 150,
247, 144, 131, 219, 102, 137, 71, 239, 94, 248,
223, 10, 103, 228, 54, 183, 72, 166, 218, 149,
148, 13, 55, 188, 120, 231, 168, 198, 97, 68,
109, 241, 127, 50, 15, 176, 191, 203, 60, 237,
170, 95, 184, 205, 141, 126, 135, 115, 182, 157,
31, 38, 56, 78, 153, 233, 25, 232, 163, 195,
100, 160, 61, 16, 108, 181, 12, 62, 199, 17,
136, 33, 107, 35, 178, 186, 19, 112, 119, 96,
46, 162, 29, 213, 43, 130, 73, 128, 207, 82,
65, 21, 173, 88, 209, 89,
};
#endif



void UID_to_16Byte(uint8_t *U_8byte)//将 8 Byte UID 生成 8 Btye UID 密钥，形成16字节给加密芯片
{
    uint16_t crc16,i;
    //加密芯片中共16字节用于存储UID
    //这里将 8 Byte UID 生成 8 Btye UID 密钥，8 Btye UID 密钥对上层不可见(电脑上位机、云服务器)
    // 8 Btye UID 密钥 存储时按位错位存储
    for(i=0;i<8;i++)
    {
        U_8byte[8+i] = U_8byte[i]+1;
    }
    crc16 = modbus_rtu_crc16(U_8byte,14);
    U_8byte[14] = crc16&0xff;
    U_8byte[15] = crc16>>8;

    rt_kprintf("UID\n");
    for(i=0;i<16;i++)
    {
        rt_kprintf("%02x",U_8byte[i]);
    }
    rt_kprintf("\n");
}


















/**
 * @brief 以 print_hex 格式打印数组内容
 * @param data     数组指针
 * @param length   数组长度
 * @param bytes_per_line 每行显示的字节数（默认为16）
 */
void print_hex(const void *data, size_t length, size_t bytes_per_line)
{
    const unsigned char *ptr = (const unsigned char *)data;
    size_t offset = 0;

    while (offset < length) {
        // 打印偏移地址（如 00000000）
        rt_kprintf("%08lx  ", (unsigned long)offset);

        // 打印十六进制字节
        for (size_t i = 0; i < bytes_per_line; i++) {
            if (offset + i < length) {
                rt_kprintf("%02x ", ptr[offset + i]);
            } else {
                rt_kprintf("   "); // 对齐填充
            }
            if (i == 7) rt_kprintf(" "); // 每8字节加空格分隔
        }

        // 打印ASCII字符（不可见字符显示为'.'）
        rt_kprintf(" |");
        for (size_t i = 0; i < bytes_per_line; i++) {
            if (offset + i < length) {
                unsigned char c = ptr[offset + i];
                if (c >= 0x20 && c <= 0x7E)
                    rt_kprintf("%c", c);
                else
                    rt_kprintf(".");
            } else {
                rt_kprintf(" ");
            }
        }
        rt_kprintf("|\n");

        offset += bytes_per_line;
    }
}










