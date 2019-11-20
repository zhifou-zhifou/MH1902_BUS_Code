#ifndef _PSAMAPI_H
#define _PSAMAPI_H

unsigned char pro_APDU_PSAM(unsigned char *sendbuf,unsigned short sendlen,unsigned char *recebuf,unsigned short *recelen);

int Mystrcmp(unsigned char *str, unsigned char *ptr,int len);

unsigned char PSAM_TestCommond(unsigned char *commond);



#endif



