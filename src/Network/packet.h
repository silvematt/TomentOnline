#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "netdef.h"

// Protocol relate
#define PROT_ID_TCP 1
#define PROT_ID_UDP 2

#define PCKT_SIZE sizeof(pckt_t) // sizeof(pckt_t)
#define MAX_PCKT_DATA 1300 // sizeof the char buffer inside the packet data

// Packets IDs
#define PCKTID_GREET        1
#define PCKTID_SET_CLASS    2
#define PCKTID_READY        3
#define PCKTID_STARTING     4
#define PCKTID_MOVEMENT     5

#define PCKT_BUFFER PCKT_SIZE

// Abs packet
typedef struct pckt_t
{
    byte protocol;
    byte id;
    byte data[MAX_PCKT_DATA];
} pckt_t;

// Packets struct
typedef struct pckt_greet_t
{
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    byte favoriteClass;
} pckt_greet_t;

typedef struct pckt_set_class_t
{
    byte classSet;
} pckt_set_class_t;

typedef struct pckt_ready_t
{
    byte isReady;
} pckt_ready_t;

typedef struct pckt_starting_t
{
    byte starting;  // 0 = not starting - 1 = starting - 2 = playing
} pckt_starting_t;

typedef struct pckt_movement_t
{
    float x,y;
    float angle;
} pckt_movement_t;


#define MAX_PCKTS_PER_BUFFER 20
typedef struct pckt_buffer_t
{
    char buffer [PCKT_SIZE*MAX_PCKTS_PER_BUFFER];
    
    // For short receive, lenght of what got received so far and flag
    int len;
    bool shorted;

    // For short send, this flag fills the buffer for writing and tells the program to not do it again on future PCKT_Send calls
    bool hasBegunWriting;
    int packetsToWrite; // how many packets are written in the buffer to send
    int packetOffset;
} pckt_buffer_t;


// Global packet data to send out
extern pckt_t packetToSend;

// Input buffer when receiving packets
extern pckt_buffer_t inputPcktBuffer;

// Output buffer when sending packets
extern pckt_buffer_t outputPcktBuffer;

void PCKT_Zero(pckt_t* p);
void PCKT_ZeroBuffer(pckt_buffer_t* pBuf);

void PCKT_ExtractFromBuffer(int offset);

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH], byte pFavClass);
pckt_t* PCKT_MakeSetClassPacket(pckt_t* packet, byte pClassToSet);
pckt_t* PCKT_MakeReadyPacket(pckt_t* packet, byte pIsReady);
pckt_t* PCKT_MakeStartingPacket(pckt_t* packet, byte pStarting);
pckt_t* PCKT_MakeMovementPacket(pckt_t* packet, float pX, float pY, float pAngle);

int PCKT_ReceivePacket(int (*OnPacketArrives)(void));
int PCKT_SendPacket(int (*OnPacketIsSent)(void));

#endif
