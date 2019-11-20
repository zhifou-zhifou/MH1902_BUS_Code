#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>	

void r_printf(uint32_t b, char *s);
void ouputString(char *title, void *pvbuff, uint32_t u32Len);
void ouputRes(char *pcFmt, void *pvbuff, uint32_t u32Len);



#endif
