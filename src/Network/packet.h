#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "netdef.h"

// Protocol relate
#define PROT_ID_TCP 1
#define PROT_ID_UDP 2

#define MAX_PCKT_DATA 1300
#define PCKT_GREET 1

typedef struct pckt_t
{
    byte protocol;
    byte id;
    byte data[MAX_PCKT_DATA];
} pckt_t;

typedef struct pckt_greet_t
{
    char name[NET_MAX_PLAYER_NAME_LENGTH];
} pckt_greet_t;

// Global packet data to send out
extern pckt_t packetToSend;

void PCKT_Zero(pckt_t* p);

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH]);

#endif
