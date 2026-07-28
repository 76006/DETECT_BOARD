#ifndef    _USER_DEFINE_H_
#define    _USER_DEFINE_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

#define PRINT_1000xFLOAT(f)                                  (int32_t)(f*1000)
#define PRINT_100xFLOAT(f)                                   (int32_t)(f*100)
#define PRINT_10xFLOAT(f)                                    (int32_t)(f*10)
#define PRINT_FLOAT(f)                                       (int32_t)(f)


//下列定义要与实际相同
#define DAC_WAVE_10KHZ_BUF_SIZE                                     (50)
#define DAC_WAVE_1KHZ_BUF_SIZE                                      (100)
#define DAC_WAVE_100HZ_BUF_SIZE                                     (200)


//以下数值，只有在发布时，才需要动
#define DEVICE_NUMBER_CODE                                 (0x03)//占用一个字节，设备码，用于代表监测板
#define COMMUNITICATION_PROTOCOL_VER                       (0x01)//占用一个字节，通信协议版本
#define DEVICE_SOFTWARE_VER                                (10000)//占用一个字节，软件版本
#define DEVICE_HARDWARE_VER                                (10000)//占用一个字节，硬件版本



#define PI                                                   (3.14159)
#define USER_VOUT_DATA_NUM                                    (200)//(1024)//存储运算多少数据
#define DAC_OUT_RANGE                                         (1)//0代表DAC正常输出
                                                                 //1代表DAC输出为0.1V---2.4V
#define ADC_WORK_MODE                                        (1)//0是ADC1的独立模式，两个通道无法满足10k的DAC对应采样
                                                                //1是ADC1和ADC2的规则同步模式





/****************************************以下是测量电流的定义*******************************/

//发送模式
//0 单条数据发送
//1 传输8条线
//2 找最大值和最小值,并传输
//3 找众数传输
//4 更有效率的找众数传输
//5 持续的采集数据，然后对采集的数据进行统计，取统计后出现次数多的边界数据-----------在高电压下，数据比较散，得出的结果不对
//6 持续的采集数据，然后对采集的数据进行统计，取统计后最近出现次数多的边界数据(因为如果只取一个数据，则可能最近出现了很多次，但出现次数多的那个值可能距离边界略远)-----------在高电压下，数据比较散，得出的结果不对
//7 持续的采集数据，然后对采集的数据进行统计，前后的第数位数据作为输出 --- 数据是在1.6V左右上下的
//8 持续的采集数据，然后对采集的数据进行统计，前后的第数位数据作为输出 --- 数据是从0左右开始的
//9 持续的采集数据，然后对采集的数据进行统计，前后的第数位数据作为输出 --- 为了适配在F1中，数据运算速度不够的问题
#define SEND_MODE                                                   (9)










/* Public typedef ------------------------------------------------------------*/
typedef enum {
    DAC_WORK_FRE_100Hz, //设定DAC工作频率为100Hz
    DAC_WORK_FRE_1kHz, //设定DAC工作频率为1kHz
    DAC_WORK_FRE_10kHz //设定DAC工作频率为10kHz
}FRE_t;

typedef struct {
    //调制后的幅度和相位
    float amp;
    float deg;
}LCR_Mod_t;

typedef struct {
    float R;
    float X;
    float C;
    float L;
}LCR_Mea_t;

typedef struct {
    FRE_t set_fre;
    uint16_t out_size;
    float set_Rf;
    float V1_OUT[USER_VOUT_DATA_NUM];
    float V2_OUT[USER_VOUT_DATA_NUM];
    LCR_Mod_t mod_v1,mod_v2;
    LCR_Mea_t mea;
}CON_RES_t;


typedef enum {
    NEGATIVE_PLATE_STATE_DISCONNECT = 0,  //负极板断开连接
    NEGATIVE_PLATE_STATE_CONNECT_OK,  //负极板连接OK
    NEGATIVE_PLATE_STATE_CONNECT_HALF,//负极板半连接
    NEGATIVE_PLATE_STATE_ERROR, //负极板连接错误(短路、不合适的负极板等)
}NEGPT_STATE;
typedef struct {
    NEGPT_STATE neg_state;//负极板连接状态定义
    uint8_t Bubble_sensor_zerocount;//气泡传感器数据中0值个数
    float sample_voltage_f;//采样电压
    uint16_t sample_voltage_u16;//
    uint32_t cap;//计算的电容大小
}SEND_INFO_t;

typedef enum {
    NP_DISCONNECT = 0X1200,  //负极板断开连接
    NP_SHORTCIRCUIT ,  //负极板短路
    BD_ERROR,//气泡传感器  表示有气泡或者无液体
}ERROR_STATE;
/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/








#endif


