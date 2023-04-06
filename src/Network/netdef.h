/// ---------------------------------------------------------------
/// Header that describes network entities
/// ---------------------------------------------------------------

#ifndef NETDEF_H_INCLUDED
#define NETDEF_H_INCLUDED

#include <WinSock2.h>

#define NET_MAX_PLAYER_NAME_LENGTH 24
#define NET_LISTENING_PORT 61530

typedef struct netplayer
{
    unsigned int id;
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    SOCKET socket;
    struct sockaddr_in address;
} netplayer_t;


// Hosting
extern netplayer_t hostPlayer;
extern netplayer_t otherPlayer;


int NET_InitializeNet();

int NET_HostGameProcedure();

int NET_JoinGameProcedure();

#endif
