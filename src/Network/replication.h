#ifndef REPLICATION_H_INCLUDED
#define REPLICATION_H_INCLUDED

#include "stdio.h"
#include "stdint.h"

typedef struct aireplicated_t
{
    uint32_t networkID;
    float x,y,z;
    char hostAggro, joinerAggro;
} aireplicated_t;

extern uint32_t lastUsedNetworkID;

void REPL_InitializeNetworkIDs();
uint32_t REPL_GenerateNetworkID();


#endif
