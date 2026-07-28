/* Includes ------------------------------------------------------------------*/

#include "lcr_calc.h"
#include <math.h>
#include "user_define.h"
#include "dac.h"
#include "math_calc.h"
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










#if    0 //使用方波作为参考信号

static float LCR_Modulate_Signal(FRE_t set_fre,float *wave_buf, uint16_t wave_size,uint16_t mod_phase)
{
    uint16_t i;
    float v,wave_sum;
    uint16_t wave_buf_size,mod_size_low,mod_size_high,mod_i;
    if(mod_phase>=180)
        mod_phase = mod_phase%180;
    switch(set_fre)
    {
        case DAC_WORK_FRE_100Hz:
            wave_buf_size = DAC_WAVE_100HZ_BUF_SIZE;
            break;
        case DAC_WORK_FRE_1kHz:
            wave_buf_size = DAC_WAVE_1KHZ_BUF_SIZE;
            break;
        case DAC_WORK_FRE_10kHz:
            wave_buf_size = DAC_WAVE_10KHZ_BUF_SIZE;
            break;
        default:
            wave_buf_size = DAC_WAVE_1KHZ_BUF_SIZE;
            break;
    }
    mod_size_low = 0;
    mod_size_high = wave_buf_size/2;
    mod_size_low = wave_buf_size*mod_phase/360+mod_size_low;
    mod_size_high = mod_size_low+mod_size_high;
    //rt_kprintf("mod_size_low = %d,mod_size_high = %d,wave_buf_size=%d,",mod_size_low,mod_size_high,wave_buf_size);
    for(i=0,wave_sum = 0;i<wave_size;i++)
    {
        mod_i = i%wave_buf_size;
        if(mod_i>=mod_size_low && mod_i<=mod_size_high)
        {
            wave_sum = wave_sum + wave_buf[i];
            //rt_kprintf("%d,%d,%d,%d\n",i,PRINT_100xFLOAT(wave_buf[i]),100,PRINT_100xFLOAT(wave_sum));
        }
        else {
            wave_sum = wave_sum - wave_buf[i];
            //rt_kprintf("%d,%d,%d,%d\n",i,PRINT_100xFLOAT(wave_buf[i]),-100,PRINT_100xFLOAT(wave_sum));
        }
        //rt_kprintf("mod_i=%d,wave_sum=%d,wave_buf[%d]=%d\n",mod_i,PRINT_100xFLOAT(wave_sum),i,PRINT_100xFLOAT(wave_buf[i]));

    }
    v = 2*(PI/4) *wave_sum/wave_size;

    //rt_kprintf("v=%d,wave_sum=%d,wave_size=%d\n",PRINT_100xFLOAT(v),PRINT_100xFLOAT(wave_sum),wave_size);
    return v;
}

#else //使用正弦波做参考信号处理

static float LCR_Modulate_Signal(FRE_t set_fre,float *wave_buf, uint16_t wave_size,uint16_t mod_phase)
{
    uint16_t i;
    float v,wave_sum;
    uint16_t wave_buf_size,mod_i,mod_pos;
    const  uint16_t *wave_ref;
    if(mod_phase>=180)
        mod_phase = mod_phase%180;
    switch(set_fre)
    {
        case DAC_WORK_FRE_100Hz:
            wave_buf_size = DAC_WAVE_100HZ_BUF_SIZE;
            wave_ref = user_dac_wave_100hz_buf;
            break;
        case DAC_WORK_FRE_1kHz:
            wave_buf_size = DAC_WAVE_1KHZ_BUF_SIZE;
            wave_ref = user_dac_wave_1khz_buf;
            break;
        case DAC_WORK_FRE_10kHz:
            wave_buf_size = DAC_WAVE_10KHZ_BUF_SIZE;
            wave_ref = user_dac_wave_10khz_buf;
            break;
        default:
            wave_buf_size = DAC_WAVE_1KHZ_BUF_SIZE;
            wave_ref = user_dac_wave_1khz_buf;
            break;
    }
    mod_pos = wave_buf_size*mod_phase/360;
    for(i=0,wave_sum = 0;i<wave_size;i++)
    {
        mod_i = (i+mod_pos)%wave_buf_size;
        wave_sum = wave_sum + wave_buf[i]*(wave_ref[mod_i]*2.5/4096-2.5/2);
    }
    v = 2 *wave_sum/wave_size;
    //rt_kprintf("v=%d,wave_sum=%d,wave_size=%d\n",PRINT_100xFLOAT(v),PRINT_100xFLOAT(wave_sum),wave_size);
    return v;
}

#endif

LCR_Mod_t LCR_Modulate(FRE_t set_fre,float *wave_buf, uint16_t wave_size)
{
    float v1,v2;
    LCR_Mod_t mod;
    v1 = LCR_Modulate_Signal(set_fre,wave_buf,wave_size,0); //相当于对信号进行同相分量检测
    v2 = LCR_Modulate_Signal(set_fre,wave_buf,wave_size,90); //相当于对信号进行正交分量检测
    mod.amp = sqrt(v1*v1+v2*v2);
#if     1
    // 计算 atan(y/x)，结果范围 [-π/2, π/2]
    mod.deg = atan(v2/v1) *180/PI;
    //rt_kprintf("v2=%d,v1=%d,deg=%d\n",PRINT_100xFLOAT(v2),PRINT_100xFLOAT(v1),PRINT_100xFLOAT(mod.deg));
#else
    // 计算 atan2(y, x)，结果范围 [-π, π]
    mod.deg = atan2(v2,v1) *180/PI;
    rt_kprintf("v2=%d,v1=%d,deg=%d\n",PRINT_100xFLOAT(v2),PRINT_100xFLOAT(v1),PRINT_100xFLOAT(mod.deg));
#endif
    return mod;
}



//根据频率、电抗计算电容
//1F = 10^3mF = 10^6uF
//1uF = 10^3nF = 10^6pF
double LCR_Calc_C(FRE_t set_fre,float X)
{
    float fre;
    double C;
    switch(set_fre)
    {
        case DAC_WORK_FRE_100Hz:
            fre = 100;
            break;
        case DAC_WORK_FRE_1kHz:
            fre = 1000;
            break;
        case DAC_WORK_FRE_10kHz:
            fre = 10000;
            break;
        default:
            fre = 1000;
            break;
    }
    //C = 1/(2*PI*fre*fabs(X));//计算结果是F
    //C = 1/(2*PI*fre*fabs(X))*1000000;//计算结果是uF
    C = 1/(2*PI*fre*fabs(X))*1000000000;//计算结果是nF
    return (double)C;
}

//根据频率、电抗计算电感
//1H = 10^3mH = 10^6uH
//1uH = 10^3nH = 10^6pH
float LCR_Calc_L(FRE_t set_fre,float X)
{
    float fre,L;
    switch(set_fre)
    {
        case DAC_WORK_FRE_100Hz:
            fre = 100;
            break;
        case DAC_WORK_FRE_1kHz:
            fre = 1000;
            break;
        case DAC_WORK_FRE_10kHz:
            fre = 10000;
            break;
        default:
            fre = 1000;
            break;
    }
    //L = fabs(X)/(2*PI*fre);//计算结果是H
    L = fabs(X)/(2*PI*fre)*1000000;//计算结果是uH
    return L;
}

void LCR_Mea_Calc(LCR_Mea_t *get_mea,FRE_t set_fre,float set_RF,LCR_Mod_t mVo,LCR_Mod_t mVx)
{
    Polar_t pVo,pVx,pVt1,pVt2;
    Complex_t Zx;

    //rt_kprintf("\t\t\tGcal=%d\n",PRINT_100xFLOAT(mVo.amp/mVx.amp*1000)); //校准用数据
    pVo.magnitude = mVo.amp;
    pVo.phase = mVo.deg;

    pVx.magnitude = mVx.amp;
    pVx.phase = mVx.deg;

    pVt1 = PolarDiv(pVo,pVx);

    //rt_kprintf("Rf/Zx=%d,<%d\n",PRINT_100xFLOAT(pVt1.magnitude),PRINT_100xFLOAT(pVt1.phase));
    pVt2.magnitude = set_RF;
    pVt2.phase = 0;

    pVt1 = PolarDiv(pVt2,pVt1);
    //rt_kprintf("Zx=%d,%d\n",PRINT_100xFLOAT(pVt1.magnitude),PRINT_100xFLOAT(pVt1.phase));



    //相位只能在-90°---90°
    //纯电阻 0°
    //纯电感 90°
    //纯电容 -90°
//    if(pVt1.phase<-90)
//        pVt1.phase = pVt1.phase+180;
//    else if(pVt1.phase>90)
//        pVt1.phase = pVt1.phase-180;

    Zx = PolarToComplex(pVt1);


#if    1 //取反-----------为解决测量电容时，存在的R、X值符号不对的问题
    if(pVt1.phase <= -90)
    {
        Zx.real = -Zx.real;
    }
    else if(pVt1.phase >= 90)
    {
        Zx.real = -Zx.real;
        Zx.imag = -Zx.imag;
    }
    else if(pVt1.phase >=45)
    {
        Zx.imag = -Zx.imag;
    }
#endif
    get_mea->R = Zx.real;
    get_mea->X = Zx.imag;
    get_mea->C = LCR_Calc_C(set_fre,get_mea->X);
    get_mea->L = LCR_Calc_L(set_fre,get_mea->X);

//    rt_kprintf("amp=%d,pha=%d\n",PRINT_100xFLOAT(pVt1.magnitude),PRINT_100xFLOAT(pVt1.phase));
//    rt_kprintf("Zxreal=%d,Zximag=%d\n",PRINT_100xFLOAT(Zx.real),PRINT_100xFLOAT(Zx.imag));
//    rt_kprintf("R=%d,X=%d,C=%d,L=%d\n",PRINT_100xFLOAT(get_mea->R),PRINT_100xFLOAT(get_mea->X),PRINT_1000xFLOAT(get_mea->C),PRINT_100xFLOAT(get_mea->L));
}






