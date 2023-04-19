#include "replication.h"

uint32_t lastUsedNetworkID = 0;

uint32_t REPL_GenerateNetworkID()
{
    return lastUsedNetworkID++;
}