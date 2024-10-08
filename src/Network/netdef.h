/// ---------------------------------------------------------------
/// Header that describes network entities
/// ---------------------------------------------------------------

#ifndef NETDEF_H_INCLUDED
#define NETDEF_H_INCLUDED

#include <stdbool.h>
#include <WinSock2.h>

#include "../Online/O_GameDef.h"

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
    // Status
    unsigned int id;
    SOCKET socket;
    struct sockaddr_in address;
    netstatus_e status;
    bool isHost;

    // Host/Join
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    playableclasses_e favoriteClass; // The class contained in the greet packet, used to initialize the lobby state and set in the Options main menu

    // Lobby
    playableclasses_e selectedClass;
    bool isReady;
    bool startingGame;
    bool gameStated;

    // Game
    // Object specific details like health and mana are in otherPlayerObject
    int curWeapon;
    int curSpell;

    bool dead;
} netplayer_t;

extern netplayer_t thisPlayer;
extern netplayer_t otherPlayer;

extern bool wantsToAbortHosting;
extern bool wantsToAbortJoining;


// Initializes network related stuff
int NET_InitializeNet(void);
void NET_InitializeNetPlayers(void);

// Starts the procedure to host a game
int NET_HostGameProcedure(char* passedUsername);
int NET_HostGameWaitForConnection(void);
int NET_HostGameWaitForGreet(void);

int NET_HostGameMakeGreetPacket(void);
int NET_HostGameSendGreet(void);

int NET_HostGameAbortConnection(void);

// Starts the procedure to join a game
int NET_JoinGameProcedure(char* passedUsername, char* passedIP, char* passedPort);
int NET_JoinGameWaitForConnection(void);
int NET_JoinGameOnConnectionEstabilishes(void);
int NET_JoinGameWaitForGreet(void);

int NET_JoinGameMakeGreetPacket(void);
int NET_JoinGameSendGreet(void);

int NET_JoinGameAbortConnection(void);

#endif
