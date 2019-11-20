#ifndef _JRREADER_H
#define _JRREADER_H
#include "type.h"

#define STX  0x02

#define RC_DATA       0
#define RC_SUCCESS    0
#define RC_FAILURE    1

#define RC_INVALID_DATA  2
#define RC_DDA_AUTH_FAILURE  3
#define RC_NO_CARD 4
#define RC_AUTH_NOT_PERFORMED  5
#define RC_MORE_CARDS  6
#define RC_Other_AP_CARDS 7
#define RC_US_CARDS 8
#define RC_SECOND_APPLICATION 9

extern unsigned char CardIN;

void UploadCardSn(void);//ª”ø®…œÀÕ
void SendKeyValue(void);//º¸÷µ…œÀÕ
void SerJRReaderHandle(Packet *recPacket);



#endif


