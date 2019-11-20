#include "mh523.h"
#include "delay.h"
#include "uart.h"
#include "rtc.h"
#include "stdio.h"
#include "bpk.h"

_calendar_obj calendar;//时钟结构体

static void RTC_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
}

//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//返回0:正常
//其他:错误代码
u8 RTC_Init(void)
{
	  SENSOR_EXTCmd(DISABLE);
		SENSOR_EXTPortCmd(SENSOR_Port_All, DISABLE);
		SENSOR_ANACmd(SENSOR_ANA_MESH, DISABLE);
		SENSOR_ClearITPendingBit();//所有外部攻击检测关闭
	   //printf("BKP=%04X\r\n",readBPK(5));
    if (readBPK(1)!=0x80808080)		//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
    {
			  RTC_Set(2019,8,23,10,50,55);  //设置时间	
			  writeBPK(1,0x80808080);
			  //printf("BKP=%08X\r\n",readBPK(1));
    }
    else//系统继续计时
    {

    }
		RTC_ITConfig(ENABLE);
    RTC_NVIC_Config();
    RTC_Get();//更新时间
    return 0; //ok
}
//RTC时钟中断
//每秒触发一次
//extern u16 tcnt;
void RTC_IRQHandler(void)
{  
    RTC_Get();//更新时间
    RTC_ClearITPendingBit();//清除秒中断标志位
	  RTC_SetAlarm(RTC_GetCounter()+1);
}
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{
    if(year%4==0) //必须能被4整除
    {
        if(year%100==0)
        {
            if(year%400==0)return 1;//如果以00结尾,还要能被400整除
            else return 0;
        } else return 1;
    } else return 0;
}
//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表
u8 const table_week[12]= {0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表
//平年的月份日期表
const u8 mon_table[12]= {31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 seccount=0;
    if(syear<1970||syear>2099)return 1;
    for(t=1970; t<syear; t++)	//把所有年份的秒钟相加
    {
        if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
        else seccount+=31536000;			  //平年的秒钟数
    }
    smon-=1;
    for(t=0; t<smon; t++)	 //把前面月份的秒钟数相加
    {
        seccount+=(u32)mon_table[t]*86400;//月份秒钟数相加
        if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数
    }
    seccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加
    seccount+=(u32)hour*3600;//小时秒钟数
    seccount+=(u32)min*60;	 //分钟秒钟数
    seccount+=sec;//最后的秒钟加上去

    RTC_SetRefRegister(seccount);	//设置RTC计数器的值
    RTC_ResetCounter();
		RTC_SetAlarm(1);
    RTC_Get();
    return 0;
}
//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void)
{
    static u16 daycnt=0;
    u32 timecount=0;
    u32 temp=0;
    u16 temp1=0;
    //timecount=RTC_GetCounter();
	  timecount += (RTC->RTC_REF+RTC_GetCounter());
	  //RTC_SetRefRegister(timecount);	//设置RTC计数器的值
    temp=timecount/86400;   //得到天数(秒钟数对应的)
    if(daycnt!=temp)//超过一天了
    {
        daycnt=temp;
        temp1=1970;	//从1970年开始
        while(temp>=365)
        {
            if(Is_Leap_Year(temp1))//是闰年
            {
                if(temp>=366)temp-=366;//闰年的秒钟数
                else {
                    temp1++;
                    break;
                }
            }
            else temp-=365;	  //平年
            temp1++;
        }
        calendar.w_year=temp1;//得到年份
        temp1=0;
        while(temp>=28)//超过了一个月
        {
            if(Is_Leap_Year(calendar.w_year)&&temp1==1)//当年是不是闰年/2月份
            {
                if(temp>=29)temp-=29;//闰年的秒钟数
                else break;
            }
            else
            {
                if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
                else break;
            }
            temp1++;
        }
        calendar.w_month=temp1+1;	//得到月份
        calendar.w_date=temp+1;  	//得到日期
    }
    temp=timecount%86400;     		//得到秒钟数
    calendar.hour=temp/3600;     	//小时
    calendar.min=(temp%3600)/60; 	//分钟
    calendar.sec=(temp%3600)%60; 	//秒钟
    calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//获取星期
    return 0;
}
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日
//返回值：星期号
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{
    u16 temp2;
    u8 yearH,yearL;

    yearH=year/100;
    yearL=year%100;
    // 如果为21世纪,年份数加100
    if (yearH>19)yearL+=100;
    // 所过闰年数只算1900年之后的
    temp2=yearL+yearL/4;
    temp2=temp2%7;
    temp2=temp2+day+table_week[month-1];
    if (yearL%4==0&&month<3)temp2--;
    return(temp2%7);
}




void GetTimes(unsigned char *timess)
{
	unsigned char data[8];
	//RTC_Get_Time(u8 *hour,u8 *min,u8 *sec,u8 *ampm);
	//RTC_Get_Date(u8 *year,u8 *month,u8 *date,u8 *week);

	//RTC_Get_Date(&data[0],&data[1],&data[2],&data[3]);
	//RTC_Get_Time(&data[4],&data[5],&data[6],&data[7]);

	timess[0]  = (unsigned char)(calendar.w_year-2000);//只取 年份的十位和个位 

	timess[1]  = calendar.w_month;								// ??? ??

	timess[2]  = calendar.w_date;								// ??? ??

	//timess[6]  = data[3];		          // ??? ????

	timess[3]  = calendar.hour;								// ??? §3?

	timess[4]  = calendar.min;	       		    // ??? ????

	timess[5]  = calendar.sec;			    			// ??? ????

//	printf("-->get data %02d-%02d-%02d %02d:%02d:%02d\r\n",timess[0],timess[1],timess[2],timess[3],timess[4],timess[5]);
}
unsigned char SetTime(unsigned char *timess)
{
	unsigned char ret=0;

	unsigned char data[8];
	
	printf("-->set data %02d-%02d-%02d %02d:%02d:%02d\r\n",timess[0],timess[1],timess[2],timess[3],timess[4],timess[5]);

	//year month day hour min sec
	RTC_Set(timess[0]+2000,timess[1],timess[2],timess[3],timess[4],timess[5]);
	
  delayms(50);
	
	printf("-->set time after-%04d-%02d-%02d %02d-%02d-%02d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);


	GetTimes(data);

	return 0;
}












