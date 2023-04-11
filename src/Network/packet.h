#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "netdef.h"

// Protocol relate
#define PROT_ID_TCP 1
#define PROT_ID_UDP 2

#define PCKT_SIZE sizeof(pckt_t) // sizeof(pckt_t)
#define MAX_PCKT_DATA 1300 // sizeof the char buffer inside the packet data
#define PCKT_GREET 1

#define PCKT_BUFFER PCKT_SIZE

typedef struct pckt_t
{
    byte protocol;
    byte id;
    byte data[MAX_PCKT_DATA];
} pckt_t;

typedef struct pckt_greet_t
{
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    byte favoriteClass;
} pckt_greet_t;

typedef struct pckt_buffer_t
{
    char buffer [PCKT_SIZE];
    
    // For short receive, lenght of what got received so far and flag
    int len;
    bool shorted;

    // For short send, this flag fills the buffer for writing and tells the program to not do it again on future PCKT_Send calls
    bool hasBegunWriting;
} pckt_buffer_t;


// Global packet data to send out
extern pckt_t packetToSend;

// Input buffer when receiving packets
extern pckt_buffer_t inputPcktBuffer;

// Output buffer when sending packets
extern pckt_buffer_t outputPcktBuffer;

void PCKT_Zero(pckt_t* p);
void PCKT_ZeroBuffer(pckt_buffer_t* pBuf);

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH], byte pFavClass);

int PCKT_ReceivePacket(int (*OnPacketArrives)(void));
int PCKT_SendPacket(int (*OnPacketIsSent)(void));

#endif
