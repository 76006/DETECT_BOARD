/* Includes ------------------------------------------------------------------*/

#include "lcr_task.h"
#include "user_define.h"
#include "dac.h"
#include "adc.h"
#include "lcr_calc.h"
#include "median_filter.h"
#include "can.h"
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
extern SEND_INFO_t g_send_info;
/* Public function prototypes ------------------------------------------------*/

/* Function implementation ---------------------------------------------------*/

CON_RES_t g_con_res;
Median_Filter_t g_median_filter_for_c;

#define NEGATIVE_PLATE_LIMIT1_LOW                     15000 //小于这个值，代表断开
#define NEGATIVE_PLATE_LIMIT1_HIGH                    20000 //小于这个值，代表断开
#define NEGATIVE_PLATE_LIMIT2_LOW                     50000 //小于这个值，代表半连接，大于这个值，代表连接良好
#define NEGATIVE_PLATE_LIMIT2_HIGH                    70000 //小于这个值，代表半连接，大于这个值，代表连接良好
#define NEGATIVE_PLATE_LIMIT3_LOW                     100000 //小于这个值，是正常的，若长时间波动，代表负极板短路
#define NEGATIVE_PLATE_LIMIT3_HIGH                    5000000 //小于这个值，是正常的，若长时间波动，代表负极板短路

#define NEGATIVE_PLATE_ERROR_REDUCE                     10//出现错误后，多长时间才恢复
extern uint8_t BOARD_SWITCH_FLAG;
extern uint8_t ota_start_flag;
uint8_t NEGATIVE_PLATE_info_printflag=0;
static void lcr_info_judge(Median_Filter_t *mf,CON_RES_t *con)
{
    static NEGPT_STATE neg_state = NEGATIVE_PLATE_STATE_DISCONNECT,pre_neg_state = NEGATIVE_PLATE_STATE_DISCONNECT;//负极板连接状态定义
    uint32_t cap;//计算的电容大小
    uint32_t raw_cap;//未经过中值滤波的电容大小
    static uint16_t error_reduce = 0;
    if(neg_state == NEGATIVE_PLATE_STATE_ERROR && con->mea.C > NEGATIVE_PLATE_LIMIT3_HIGH)
    {
        error_reduce = NEGATIVE_PLATE_ERROR_REDUCE;
    }
    //对采集到的数据，进行中值滤波
    raw_cap = con->mea.C*1000;
    cap = median_filter_handle(mf,raw_cap);
    if(cap < NEGATIVE_PLATE_LIMIT1_LOW)
    {
        if(error_reduce)
            error_reduce--;
        else
            neg_state = NEGATIVE_PLATE_STATE_DISCONNECT;
    }
    else if(cap < NEGATIVE_PLATE_LIMIT1_HIGH)
    {
        if(pre_neg_state == NEGATIVE_PLATE_STATE_DISCONNECT || pre_neg_state == NEGATIVE_PLATE_STATE_CONNECT_HALF)
        {
            //临界区，不变
        }
        else {
            if(error_reduce)
                error_reduce--;
            else {
                neg_state = NEGATIVE_PLATE_STATE_DISCONNECT;
            }
        }
    }
    else if(cap < NEGATIVE_PLATE_LIMIT2_LOW)
    {
        if(error_reduce)
            error_reduce--;
        else
            neg_state = NEGATIVE_PLATE_STATE_CONNECT_HALF;
    }
    else if(cap < NEGATIVE_PLATE_LIMIT2_HIGH)
    {
        if(pre_neg_state == NEGATIVE_PLATE_STATE_DISCONNECT || pre_neg_state == NEGATIVE_PLATE_STATE_CONNECT_HALF)
        {
            //临界区，不变
        }
        else {
            if(error_reduce)
                error_reduce--;
            else {
                neg_state = NEGATIVE_PLATE_STATE_CONNECT_HALF;
            }
        }
    }
    else if(cap < NEGATIVE_PLATE_LIMIT3_LOW)
    {
        if(error_reduce)
            error_reduce--;
        else
            neg_state = NEGATIVE_PLATE_STATE_CONNECT_OK;
    }
    else if(cap < NEGATIVE_PLATE_LIMIT3_HIGH)
    {
        if(pre_neg_state == NEGATIVE_PLATE_STATE_CONNECT_OK || pre_neg_state == NEGATIVE_PLATE_STATE_ERROR)
        {
            //临界区，不变
        }
        else {

            if(error_reduce)
                error_reduce--;
            else
                neg_state = NEGATIVE_PLATE_STATE_CONNECT_OK;
        }
    }
    else if(cap > NEGATIVE_PLATE_LIMIT3_HIGH)
    {
        error_reduce = NEGATIVE_PLATE_ERROR_REDUCE;
        //一旦出现，则后面会有几次都是错误
        neg_state = NEGATIVE_PLATE_STATE_ERROR;
    }
//    switch(neg_state)
//    {
//        case NEGATIVE_PLATE_STATE_DISCONNECT:
//            break;
//        case NEGATIVE_PLATE_STATE_CONNECT_OK:
//            break;
//        case NEGATIVE_PLATE_STATE_CONNECT_HALF:
//            break;
//        case NEGATIVE_PLATE_STATE_ERROR:
//            break;
//    }
    if ( NEGATIVE_PLATE_info_printflag )
    {

        rt_kprintf("%10d ,%10d ,%d\n",cap,PRINT_1000xFLOAT(con->mea.C),error_reduce);

    }
    //赋值
    g_send_info.cap = cap;
    negative_plate_diag_lcr_sample(raw_cap, cap, (uint8_t)neg_state);
    if(pre_neg_state != neg_state)
    {
        negative_plate_diag_state_change(cap, (uint8_t)pre_neg_state, (uint8_t)neg_state);
        pre_neg_state = neg_state;
        g_send_info.neg_state = neg_state;
        monitor_board_inform_info_send();//立刻发送数据到主控板
    }

}



static void lcr_task_thread(void *parameter)
{
    uint32_t count = 0;
    median_filter_init(&g_median_filter_for_c,11);
    while(1)
    {
        if(BOARD_SWITCH_FLAG==0 && ota_start_flag == 0)//若无变化
        {
            User_Dac_Dma_Start();
            rt_thread_mdelay(10);

            User_Adc_Start();
            rt_thread_mdelay(20);
            User_Adc_Stop();
            User_Adc_Get_Value(g_con_res.V1_OUT,g_con_res.V2_OUT,g_con_res.out_size);
            g_con_res.mod_v1 = LCR_Modulate(g_con_res.set_fre,g_con_res.V1_OUT,g_con_res.out_size);
            g_con_res.mod_v2 = LCR_Modulate(g_con_res.set_fre,g_con_res.V2_OUT,g_con_res.out_size);

            //rt_kprintf("\nmod_v1.amp=%d,mod_v1.deg=%d,\nmod_v2.amp=%d,mod_v2.deg=%d\n\n",PRINT_100xFLOAT(g_con_res.mod_v1.amp),PRINT_100xFLOAT(g_con_res.mod_v1.deg),PRINT_100xFLOAT(g_con_res.mod_v2.amp),PRINT_100xFLOAT(g_con_res.mod_v2.deg));

            LCR_Mea_Calc(&g_con_res.mea,g_con_res.set_fre,g_con_res.set_Rf,g_con_res.mod_v1,g_con_res.mod_v2);
            count++;

            //rt_kprintf("%7d ,",PRINT_1000xFLOAT(g_con_res.mea.C));


            //rt_thread_mdelay(1000);
            User_Dac_Dma_Stop();
            rt_thread_mdelay(10);

            lcr_info_judge(&g_median_filter_for_c,&g_con_res);
            if(count % 5 == 0)
            {
                rt_kprintf("\n");
            }
        }
        else {
            rt_thread_mdelay(1000);
        }
    }
}









void lcr_task_resoure_init(void)
{

}


void lcr_task_init(void)
{
    rt_thread_t thread_lcr;

    User_Dac_Init();
    User_Adc_Init();

    g_con_res.set_fre = DAC_WORK_FRE_1kHz; //频率大小设定
    g_con_res.out_size = USER_VOUT_DATA_NUM;
    g_con_res.set_Rf = 5000;//Rf阻值设定
    User_Dac_Set_Fre(g_con_res.set_fre);
    User_Adc_Set_DacWorkFre(g_con_res.set_fre);

#if     0
    rt_kprintf("\n\nuse RF1 RG1 \n");
    g_con_res.set_Rf = 100;//Rf阻值设定
#elif    1
    rt_kprintf("\n\nuse RF2 RG2 \n");
    g_con_res.set_Rf = 5000;//Rf阻值设定
#elif    1
    rt_kprintf("\n\nuse RF3 RG3 \n");
    g_con_res.set_Rf = 100000;//Rf阻值设定
#endif

    /* 创建数据接收线程 */
    thread_lcr = rt_thread_create("lcr", lcr_task_thread, RT_NULL, 1024, 15, 10);
    if (thread_lcr != RT_NULL)
    {
        rt_thread_startup(thread_lcr);
    }
    else
    {
        rt_kprintf("create lcr thread failed!\n");
    }
}

static int NEGATIVE_PLATE_PRINTFLAG(int argc, char **argv)
{
    NEGATIVE_PLATE_info_printflag = 1-NEGATIVE_PLATE_info_printflag;

    if (NEGATIVE_PLATE_info_printflag)
    {
        rt_kprintf("NEGATIVE_PLATE_info_printflag  is enabled\n");
    }
    else
    {
        rt_kprintf("NEGATIVE_PLATE_info_printflag  is disabled\n");
    }
    return 0;
}
MSH_CMD_EXPORT(NEGATIVE_PLATE_PRINTFLAG,"NEGATIVE_PLATE_PRINTFLAG");


