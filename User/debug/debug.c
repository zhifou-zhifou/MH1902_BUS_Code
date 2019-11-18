#include <stdio.h>
#include "debug.h"
#include "mhscpu.h"




void r_printf(uint32_t b, char *s)
{
	if (0 != b)
	{
		printf("pass: ");printf("%s", s);
	}
	else
	{
		printf("fail: ");printf("%s", s);
		while(1);
	}
}

void ouputString(char *title, void *pvbuff, uint32_t u32Len)
{
    int32_t i, j;
    uint8_t *pu8Buff = pvbuff;
    
    printf("%s", title);
    printf("%s", pu8Buff);
    printf("\n");
}


void ouputRes(char *pcFmt, void *pvbuff, uint32_t u32Len)
{
    int32_t i, j;
    uint8_t *pu8Buff = pvbuff;
    
    printf("%s", pcFmt);
    for (i = 0; i < u32Len; i++)
    {
        printf("%02X", pu8Buff[i]);
    }
    printf("\n");
}



