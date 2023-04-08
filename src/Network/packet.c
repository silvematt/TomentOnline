#include "packet.h"

// Global packet data to send out
pckt_t packetToSend;

void PCKT_Zero(pckt_t* p)
{
    p->id = 0;
    memset(p->data, 0, MAX_PCKT_DATA);
}

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH])
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKT_GREET;

    // Create and fill the content
    pckt_greet_t content;
    strcpy(content.name, pName);

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(pckt_greet_t));

    return packet;
}
