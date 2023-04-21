#include "netdef.h"
#include "replication.h"

uint32_t lastUsedNetworkID = 0;

void REPL_InitializeNetworkIDs()
{
    if(thisPlayer.isHost)
        lastUsedNetworkID = 0;
    else
        lastUsedNetworkID = 4294967295U / 2;
}

uint32_t REPL_GenerateNetworkID()
{
    return lastUsedNetworkID++;
}