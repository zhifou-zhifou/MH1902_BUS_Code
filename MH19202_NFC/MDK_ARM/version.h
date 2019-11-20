#ifndef __VERSION_H__
#define __VERSION_H__

#define APP_RL_MODE_BETA        1
#define APP_RL_MODE_RC          0

/*****************************************************
* App version
*/
#define APP_VERSION_MAJOR               1       //主版本号,仅在有大的功能变更或架构变更时自加1,并将次要版本号&修复号&编译序号归零
#define APP_VERSION_MINER               1       //次版本号,在有新功能增加或软件优化时自加1
#define APP_VERSION_FIX                 1       //修复号,在有BUG修复或用户提交的功能异常修复后自加1
#define APP_VERSION_BUILD               6       //编译版本号,原则上每次编译过后都需要自加1

//beta or rc
#define APP_VERSION_RL_MODE             APP_RL_MODE_BETA

#define APP_PRODUCT_CODE                0x11223344      //产品功能码,各个固件需要保持唯一

#if APP_VERSION_RL_MODE==APP_RL_MODE_BETA
	#define APP_VERSION_RL_MODE_STR                 "beta"
#else
	#define APP_VERSION_RL_MODE_STR                 "rc"
#endif

#define STR1(R)  #R  

#define STR2(R)  STR1(R)  

//#define APP_VERSION_STR_PART1   STR2(APP_VERSION_MAJOR)"."STR2(APP_VERSION_MINER)"."STR2(APP_VERSION_FIX)

#define APP_VERSION_STR_PART1   #1"."#1"."#1

//#define APP_VERSION_STR_PART2   "Build "STR2(APP_VERSION_BUILD)" ("APP_VERSION_RL_MODE_STR")"
//#define APP_VERSION_STR_PART2   "Build "STR2(APP_VERSION_BUILD)

//#define APP_VERSION_STR         APP_VERSION_STR_PART1" "APP_VERSION_STR_PART2

#define APP_VERSION_STR         \
    STR2(APP_VERSION_MAJOR)"."STR2(APP_VERSION_MINER)"."STR2(APP_VERSION_FIX)\
    " Build "STR2(APP_VERSION_BUILD)\
    " ("APP_VERSION_RL_MODE_STR")"
    
#endif
