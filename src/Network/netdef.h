/// ---------------------------------------------------------------
/// Header that describes network entities
/// ---------------------------------------------------------------

#ifndef NETDEF_H_INCLUDED
#define NETDEF_H_INCLUDED

#include <WinSock2.h>

#define NET_MAX_PLAYER_NAME_LENGTH 24
#define NET_LISTENING_PORT 61530

// Enum that describes all the states the other player can be in
typedef enum netstatus
{
    NETSTS_NULL = 0,
    NETSTS_JUST_CONNECTED,
    NETSTS_HAVE_TO_GREET,
    NETSTS_GREETED,
    NETSTS_PREPARING,
    NETSTS_READY,
    NETSTS_PLAYING,
    NETSTS_DISCONNECTED
} netstatus_e;

// netplayer is used to represent a player on the net (other player)
typedef struct netplayer
{
    unsigned int id;
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    SOCKET socket;
    struct sockaddr_in address;

    netstatus_e status;
} netplayer_t;

extern netplayer_t hostPlayer;
extern netplayer_t otherPlayer;

extern boolean wantsToAbortHosting;
extern boolean wantsToAbortJoining;

// Initializes network related stuff
int NET_InitializeNet();

// Starts the procedure to host a game
int NET_HostGameProcedure();
int NET_HostGameWaitForConnection();
int NET_HostGameWaitForGreet();
int NET_HostGameSendGreet();

int NET_HostGameAbortConnection();

// Starts the procedure to join a game
int NET_JoinGameProcedure();
int NET_JoinGameWaitForConnection();
int NET_JoinGameOnConnectionEstabilishes();
int NET_JoinGameWaitForGreet();

int NET_JoinGameAbortConnection();


#endif
