#include "key.h"

#include "stdio.h"

void key_init()//按键IO口初始化
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//上拉输入
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Remap=GPIO_Remap_1;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}


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

static unsigned char KEY5_ReadPin(void)//读组合键  0:两个键同时按下   1：两个键都释放 都为高电平
{
    unsigned char KeyValue=0;
    KeyValue = 	KEY1_ReadPin()+KEY4_ReadPin();
    if(KeyValue == 0) return 0;
    //else if(KeyValue == 1)return 0;
	  else return 1;
}

KEY_COMPONENTS Key_Buf[KEY_NUM] =
{
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY0_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY1_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY2_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY3_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY4_ReadPin},
    {1,0,0,0,KEY_NULL,KEY_NULL,KEY5_ReadPin},
};//第四个参数KEY_DOWN_LEVEL表示按键按下时IO口的电平  0：低电平  1：高电平  我的电路按键按下是低电平 所以这里设置为0

//轮询每个按键的电平 并赋值给对应的虚拟电平
static void Get_Key_Level(void)
{
    unsigned char i;

    for(i = 0; i < KEY_NUM; i++)
    {
        if(Key_Buf[i].KEY_SHIELD == 0)
            continue;
        if(Key_Buf[i].READ_PIN() == Key_Buf[i].KEY_DOWN_LEVEL)
        {
            Key_Buf[i].KEY_LEVEL = 1;//虚拟电平 1：按下 0：抬起
        }
        else
        {
            Key_Buf[i].KEY_LEVEL = 0;
        }
    }
}

void ReadKeyStatus(void)
{
    unsigned char i;
    Get_Key_Level();
    for(i = 0; i < KEY_NUM; i++)
    {
        switch(Key_Buf[i].KEY_STATUS)
        {
            //状态0：没有按键按下
        case KEY_NULL:
            if(Key_Buf[i].KEY_LEVEL == 1)//有按键按下
            {
                Key_Buf[i].KEY_STATUS = KEY_SURE;//转入状态1
                Key_Buf[i].KEY_EVENT = KEY_NULL;//空事件
            }
            else
            {
                Key_Buf[i].KEY_EVENT = KEY_NULL;//空事件
            }
            break;
            //状态1：按键按下确认
        case KEY_SURE:
            if(Key_Buf[i].KEY_LEVEL == 1)//确认和上次相同
            {
                Key_Buf[i].KEY_STATUS = KEY_DOWN;//转入状态2
                Key_Buf[i].KEY_EVENT = KEY_NULL;//按下事件
                Key_Buf[i].KEY_COUNT = 0;//计数器清零
            }
            else
            {
                Key_Buf[i].KEY_STATUS = KEY_NULL;//转入状态0
                Key_Buf[i].KEY_EVENT = KEY_NULL;//空事件
            }
            break;
            //状态2：按键按下
        case KEY_DOWN:
            if(Key_Buf[i].KEY_LEVEL == 0)//按键释放，端口高电平
            {
                Key_Buf[i].KEY_STATUS = KEY_NULL;//转入状态0
                Key_Buf[i].KEY_EVENT = KEY_NULL;//空事件					  
            }
            else
            {
                Key_Buf[i].KEY_STATUS = KEY_LONG;//转入状态3
                Key_Buf[i].KEY_EVENT = KEY_DOWN;//按下事件						
            }
            break;
            //状态3：按键连续按下
        case KEY_LONG:
            if(Key_Buf[i].KEY_LEVEL != 1)//按键释放，端口高电平
            {
                Key_Buf[i].KEY_STATUS = KEY_NULL;//转入状态0
                Key_Buf[i].KEY_EVENT = KEY_UP;//松开事件          
            }
            else 
						if((Key_Buf[i].KEY_LEVEL == 1)
             && (++Key_Buf[i].KEY_COUNT >= KEY_LONG_DOWN_DELAY)&&(i==5)) //超过KEY_LONG_DOWN_DELAY没有释放
            {
                Key_Buf[i].KEY_EVENT = KEY_LONG;//长按事件
                Key_Buf[i].KEY_COUNT = 0;//计数器清零
            }
            else
            {
                Key_Buf[i].KEY_EVENT = KEY_NULL;//空事件
            }
            break;
        }
    }
    if(Key_Buf[5].KEY_LEVEL == 1 || Key_Buf[5].KEY_EVENT == KEY_LONG)
    {
        Key_Buf[1].KEY_EVENT = KEY_NULL;
        Key_Buf[1].KEY_STATUS= KEY_NULL;
        Key_Buf[4].KEY_EVENT = KEY_NULL;
        Key_Buf[4].KEY_STATUS= KEY_NULL;
    }
}

//按键测试函数
void Task_KEY_Scan(void)
{
    //ReadKeyStatus();

    if(Key_Buf[KEY0].KEY_EVENT == KEY_UP)
    {
        printf("Touch\n");
			  Key_Buf[KEY0].KEY_EVENT = KEY_NULL;//空事件
    }
    if(Key_Buf[KEY1].KEY_EVENT == KEY_UP)
    {
        printf("DOWN\n");
			  Key_Buf[KEY1].KEY_EVENT = KEY_NULL;//空事件
    }
    if(Key_Buf[KEY2].KEY_EVENT == KEY_UP)
    {
        printf("UP\n");
		  	Key_Buf[KEY2].KEY_EVENT = KEY_NULL;//空事件
    }
    if(Key_Buf[KEY3].KEY_EVENT == KEY_UP)
    {
        printf("OK\n");
			  Key_Buf[KEY3].KEY_EVENT = KEY_NULL;//空事件
    }
    if(Key_Buf[KEY4].KEY_EVENT == KEY_UP)
    {
        printf("FNC\n");
			  Key_Buf[KEY4].KEY_EVENT = KEY_NULL;//空事件
    }
    if(Key_Buf[KEY5].KEY_EVENT == KEY_LONG)
    {
        printf("DOWN+FUN\n");
			  Key_Buf[KEY5].KEY_EVENT = KEY_NULL;//空事件
    }

}

unsigned char GetKey()
{  
    if(Key_Buf[KEY0].KEY_EVENT == KEY_UP)
    {
			  Key_Buf[KEY0].KEY_EVENT = KEY_NULL;//空事件
        return 1;
    }
    else if(Key_Buf[KEY1].KEY_EVENT == KEY_UP)
    {
			  Key_Buf[KEY1].KEY_EVENT = KEY_NULL;//空事件
        return 2;
		 }
    else if(Key_Buf[KEY2].KEY_EVENT == KEY_UP)
    {
			  Key_Buf[KEY2].KEY_EVENT = KEY_NULL;//空事件
        return 3;
    }
    else if(Key_Buf[KEY3].KEY_EVENT == KEY_UP)
    {
			  Key_Buf[KEY3].KEY_EVENT = KEY_NULL;//空事件
        return 4;
    }
    else if(Key_Buf[KEY4].KEY_EVENT == KEY_UP)
    {
			  Key_Buf[KEY4].KEY_EVENT = KEY_NULL;//空事件
        return 5;
    }
    else if(Key_Buf[KEY5].KEY_EVENT == KEY_LONG)
    {
			  Key_Buf[KEY5].KEY_EVENT = KEY_NULL;//空事件
        return 6;
    }
    else
    {
        return 0;
    }

}



















