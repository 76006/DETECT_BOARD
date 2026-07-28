/* Includes ------------------------------------------------------------------*/

#include "my_encry.h"
#include "UID_Encryption.h"
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


uint8_t is_encry_ok = 0;//0代表验证错误，1代表验证正确
/**** 验证密钥所需的一些参数 */
#define KEY_LOCATION                        0x0800C000  //密钥在当前芯片中的存储地址
#define UID_LOCATION                        0x1FFF7A10  //当前芯片UID所在地址
#define STM32_UID_ENCRY                        0
uint8_t myCID[12] = {0x15,0x10,0x21,0x91,
                     0x12,0x41,0x51,0x36,
                     0x74,0x51,0x20,0x05};//用户自定义ID
//实际的ID：
//uint8_t myCID[12] = {0x24,0x57,0x95,0x02,
//                     0x52,0x24,0xA6,0xC2,
//                     0xF1,0xF0,0xCE,0xAD};//用户自定义ID
uint32_t key_location,uid_location;

void my_encry_set_loc(uint32_t key_loc,uint32_t uid_loc)
{
    key_location = KEY_LOCATION;//要让 key_location 等于 KEY_LOCATION
    uid_location = UID_LOCATION;//要让 uid_location 等于 UID_LOCATION
}
void my_encry_set_CID(void)
{
//    myCID[0] = myCID[0]+myCID[0];
//    myCID[1] = myCID[1]+0x21;
//    myCID[3] = myCID[3]-0x21;
//    myCID[5] = myCID[5]+0x21;
//    myCID[7] = myCID[7]-0x01;
//    myCID[8] = myCID[8]-0x01;
//    myCID[9] = myCID[9]-0x09;
//    myCID[10] = myCID[10]+0x01;
}

void my_encry_init(void)
{
#if    STM32_UID_ENCRY
    uint8_t checkResult = 1; //验证结果
    //初始化参数
    my_encry_set_loc(0x0800C000,0x1FFF7A10);
    my_encry_set_CID();
    checkResult = UID_Encryption_Key_Check(  (void*)key_location,//传入当前芯片FLASH中的密钥，此处输入的参数与烧录器配置应当完全一致
                                             (void*)uid_location,//传入当前芯片UID
                                             myCID,              //传入用户自定义ID, 此处输入的参数与烧录器配置应当完全一致
                                              LENGTH_12,         //传入密钥长度， 此处输入的参数与烧录器配置应当完全一致
                                              LITTLE_ENDIA,      //传入端序选择， 此处输入的参数与烧录器配置应当完全一致
                                              ALGORITHM_0);      //传入算法选择， 此处输入的参数与烧录器配置应当完全一致

    is_encry_ok = !checkResult;

#if     1
    {
        uint8_t i;
        rt_kprintf("\nmyCID:{");
        for(i=0;i<12;i++)
            rt_kprintf("0x%02X,",myCID[i]);
        rt_kprintf("}\n%08x,%08x\n",key_location,uid_location);
        rt_kprintf("%d\n",(uint32_t *)key_location);
        rt_kprintf("is_encry_ok=%d\n",is_encry_ok);
    }
#endif

#else   //没开启验证，则默认为有效
    is_encry_ok = 1;
#endif
}









