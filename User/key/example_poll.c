#include "multi_button.h"
#include "mh523.h"
#include "mhscpu.h"
#include "delay.h"
#include "stdio.h"

struct Button btn0;
struct Button btn1;
struct Button btn2;
struct Button btn3;
struct Button btn4;
struct Button btn5;


//PC2~PC5 接的是独立按键
static unsigned char KEY0_ReadPin(void)
{
    return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1);//读PC1  触摸按键
}

static unsigned char KEY1_ReadPin(void)
{
    return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2);//读PC2
}

static unsigned char KEY2_ReadPin(void)
{
    return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3);//读PC3
}

static unsigned char KEY3_ReadPin(void)
{
    return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4);//读PC4;
}

static unsigned char KEY4_ReadPin(void)
{
    return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5);//读PC5;
}

static unsigned char KEY5_ReadPin(void)//读组合键
{
    unsigned char KeyValue=0;
    KeyValue = 	KEY1_ReadPin()+KEY4_ReadPin();
    if(KeyValue == 0) return 0;
    else return 1;
}

//uint8_t read_button1_GPIO()
//{
//	return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
//}

void Key_Bsp_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//上拉输入
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Remap=GPIO_Remap_1;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    button_init(&btn0, KEY0_ReadPin, 0);
    button_start(&btn0);

    button_init(&btn1, KEY1_ReadPin, 0);
    button_start(&btn1);

    button_init(&btn2, KEY2_ReadPin, 0);
    button_start(&btn2);

    button_init(&btn3, KEY3_ReadPin, 0);
    button_start(&btn3);

    button_init(&btn4, KEY4_ReadPin, 0);
    button_start(&btn4);

    button_init(&btn5, KEY5_ReadPin, 0);
    button_start(&btn5);

}

void Key_Scan()
{
    static uint8_t btn1_event_val;
    //make the timer invoking the button_ticks() interval 5ms.
    //This function is implemented by yourself.
    //__timer_start(button_ticks, 0, 5);
    if(btn1_event_val != get_button_event(&btn0))
    {
        btn1_event_val = get_button_event(&btn0);
        if(btn1_event_val == SINGLE_CLICK)
        {
            printf("Touch\r\n");
        }
    }
    if(btn1_event_val != get_button_event(&btn1))
    {
        btn1_event_val = get_button_event(&btn1);
        if(btn1_event_val == SINGLE_CLICK)
        {
            printf("DOWN\r\n");
        }
    }
    if(btn1_event_val != get_button_event(&btn2))
    {
        btn1_event_val = get_button_event(&btn2);
        if(btn1_event_val == SINGLE_CLICK)
        {
            printf("UP\r\n");
        }
    }
    if(btn1_event_val != get_button_event(&btn3))
    {
        btn1_event_val = get_button_event(&btn3);
        if(btn1_event_val == SINGLE_CLICK)
        {
            printf("OK\r\n");
        }
    }
    if(btn1_event_val != get_button_event(&btn4))
    {
        btn1_event_val = get_button_event(&btn4);
        if(btn1_event_val == SINGLE_CLICK)
        {
            printf("FUN\r\n");
        }
    }
    if(btn1_event_val != get_button_event(&btn5))
    {
        btn1_event_val = get_button_event(&btn5);
        if(btn1_event_val == LONG_RRESS_START)
        {
            printf("UP+FUN\r\n");
        }
    }
}

