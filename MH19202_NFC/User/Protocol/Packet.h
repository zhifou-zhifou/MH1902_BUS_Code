#ifndef __PACKET_H
#define __PACKET_H
#include "type.h"

void M1_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket);
void CPU_Contact_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket);
void CPU_Proximity_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket);
void SendPacket(Packet *sendPacket);

#endif
