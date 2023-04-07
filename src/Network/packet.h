#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "netdef.h"

#define MAX_PCKT_DATA 1300

#define PCKT_GREET 1

typedef struct pckt_t
{
    byte id;
    byte data[MAX_PCKT_DATA];
} pckt_t;

typedef struct pckt_greet_t
{
    char name[NET_MAX_PLAYER_NAME_LENGTH];
} pckt_greet_t;


char* PCKT_PcktToBytes(pckt_t* p);
pckt_t* PCKT_BytesToPckt(const char* b);

pckt_t* PCKT_MakeGreetPacket(char pName[NET_MAX_PLAYER_NAME_LENGTH]);
pckt_greet_t* PCKT_GetGreetPacket(pckt_t* p);

#endif
