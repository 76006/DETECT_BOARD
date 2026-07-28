#ifndef    _MATH_CALC_H_
#define    _MATH_CALC_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "user_define.h"
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

//直角坐标形式：ZX = a + jb（实部a，虚部b）
typedef struct {
    float real;    // 实部 (a)
    float imag;    // 虚部 (b)
} Complex_t;

//极坐标形式：ZX = A∠θ（幅度A，相位角θ）
typedef struct {
    float magnitude; // 幅度 (A)
    float phase;     // 相位角 (θ)
} Polar_t;

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/


Polar_t ComplexToPolar(Complex_t z);

Complex_t PolarToComplex(Polar_t p);

Complex_t complex_add(Complex_t a, Complex_t b);

Complex_t complex_sub(Complex_t a, Complex_t b);

Complex_t complex_mult(Complex_t a, Complex_t b);

Complex_t complex_div(Complex_t a, Complex_t b);

Polar_t PolarMul(Polar_t p1, Polar_t p2);

Polar_t PolarDiv(Polar_t p1, Polar_t p2);

Polar_t PolarAdd(Polar_t p1, Polar_t p2);

Polar_t PolarSub(Polar_t p1, Polar_t p2);



#endif


