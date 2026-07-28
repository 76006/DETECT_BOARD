/* Includes ------------------------------------------------------------------*/

#include "math_calc.h"
#include <math.h>

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



// 直角坐标 → 极坐标
// amp=sqrt(a^2+b^2)   phase = atan2(b,a)
Polar_t ComplexToPolar(Complex_t z)
{
    Polar_t result;
    result.magnitude = sqrtf(z.real * z.real + z.imag * z.imag);
    result.phase = atan2f(z.imag, z.real); // 注意使用atan2函数处理象限
    return result;
}

// 极坐标 → 直角坐标
// A∠θ=Acosθ+jAsinθ
Complex_t PolarToComplex(Polar_t p)
{
    Complex_t result;
//    result.real = p.magnitude * cosf(p.phase);
//    result.imag = p.magnitude * sinf(p.phase);

    result.real = p.magnitude * cosf(p.phase*PI/180);
    result.imag = p.magnitude * sinf(p.phase*PI/180);
    return result;
}

//复数运算规则
//https://baike.baidu.com/item/%E5%A4%8D%E6%95%B0%E8%BF%90%E7%AE%97%E6%B3%95%E5%88%99/2568041

//复数加法
// z1+z2=(a+bi)+ (c+di)= (a+c)+(b+d)i
Complex_t complex_add(Complex_t a, Complex_t b)
{
    Complex_t result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

//复数加法
// z1+z2 = (a+bi) - (c+di) =(a-c)+(b-d)i
Complex_t complex_sub(Complex_t a, Complex_t b)
{
    Complex_t result;
    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;
    return result;
}


// 复数乘法
// z1*z2 = (a+bi) *(c+di) =(ac -bd) +(ad+bc)i
Complex_t complex_mult(Complex_t a, Complex_t b)
{
    Complex_t result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}


// 复数除法 : https://baike.baidu.com/item/%E5%A4%8D%E6%95%B0%E9%99%A4%E6%B3%95/7891800
// z1/z2 = (a+ib)/(c+id) = =(a+ib)(c-id)/(c+id)(c-id) = =(ac+bd)/(c^2+d^2)+i(bc-ad)/(c^2+d^2)
Complex_t complex_div(Complex_t a, Complex_t b)
{
    Complex_t result;
    float denominator = b.real * b.real + b.imag * b.imag;
    result.real = (a.real * b.real + a.imag * b.imag) / denominator;
    result.imag = (a.imag * b.real - a.real * b.imag) / denominator;
    return result;
}


//A∠θ×B∠φ=(A×B)∠(θ+φ)
Polar_t PolarMul(Polar_t p1, Polar_t p2)
{
    Polar_t result;
    result.magnitude = p1.magnitude * p2.magnitude;
    result.phase = p1.phase + p2.phase;
    return result;
}
// A∠θ / B∠φ = (A/B) ∠(θ-φ)
Polar_t PolarDiv(Polar_t p1, Polar_t p2)
{
    Polar_t result;
    result.magnitude = p1.magnitude / p2.magnitude;
    result.phase = p1.phase - p2.phase;
    return result;
}


// 极坐标加法
Polar_t PolarAdd(Polar_t p1, Polar_t p2)
{
    Complex_t z1 = PolarToComplex(p1);
    Complex_t z2 = PolarToComplex(p2);
    Complex_t z_result;
    z_result.real = z1.real + z2.real;
    z_result.imag = z1.imag + z2.imag;
    return ComplexToPolar(z_result);
}

// 极坐标减法
Polar_t PolarSub(Polar_t p1, Polar_t p2)
{
    Complex_t z1 = PolarToComplex(p1);
    Complex_t z2 = PolarToComplex(p2);
    Complex_t z_result;
    z_result.real = z1.real - z2.real;
    z_result.imag = z1.imag - z2.imag;
    return ComplexToPolar(z_result);
}
