#ifndef LOBBY_H_INCLUDED
#define LOBBY_H_INCLUDED

#include <stdbool.h>
#include "SDL.h"
#include "O_GameDef.h"

extern bool thisPlayerReady;
extern bool otherPlayerReady;

extern gamedungeons_e selectedDungeon;

// ------------------------------------------------------------------------------------------------------------------------------
// This function checks wether each of the favorite players classess are ok to play together (if they are not the same)
// If they're the same, the host keeps its own while the client will send a SetClass packet
// ------------------------------------------------------------------------------------------------------------------------------
int O_LobbyDefineClassesHostwise(void);
int O_LobbyDefineClassesJoinerwise(void);

int O_LobbyLeave(void);

int O_LobbySetReady(void);
int O_LobbySetMap(void);
int O_LobbySetClass(playableclasses_e selected);

int O_LobbyRender(void);

int O_LobbyStartGame(void);

int O_LobbySendPackets(void);
int O_LobbyOnPacketIsSent(void);

int O_LobbyReceivePackets(void);
int O_LobbyOnPacketIsReceived(void);

void O_LobbyInputHandling(SDL_Event* e);

#endif
