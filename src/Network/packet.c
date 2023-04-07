#include "packet.h"

char* PCKT_PcktToBytes(pckt_t* p)
{
    char* buffer = malloc(MAX_PCKT_DATA);

    memcpy(buffer, p, sizeof(*p));

    return buffer;
}

pckt_t* PCKT_BytesToPckt(const char* b)
{
    pckt_t* packet = (pckt_t*)malloc(sizeof(pckt_t));;

    memcpy(packet, b, sizeof(*packet));

    return packet;
}

pckt_t* PCKT_MakeGreetPacket(char pName[NET_MAX_PLAYER_NAME_LENGTH])
{
    // Create the packet
    pckt_t* packet = (pckt_t*)malloc(sizeof(pckt_t));;
    packet->id = PCKT_GREET;

    // Create and fill the content
    pckt_greet_t content;
    strcpy(content.name, pName);

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(pckt_greet_t));

    return packet;
}

pckt_greet_t* PCKT_GetGreetPacket(pckt_t* p)
{
    pckt_greet_t* greetPckt = (pckt_greet_t*)malloc(sizeof(pckt_greet_t));

    memcpy(greetPckt, p->data, sizeof(pckt_greet_t));

    return greetPckt;   
}
