#ifndef LOBBY_H_INCLUDED
#define LOBBY_H_INCLUDED

#include <stdbool.h>
#include "O_GameDef.h"

extern bool thisPlayerReady;
extern bool otherPlayerReady;

extern gamedungeons_e selectedDungeon;

int O_LobbyLeave(void);

int O_LobbySetReady(void);
int O_LobbySetMap(void);
int O_LobbySetClass(void);

int O_LobbyRender(void);

int O_LobbyStartGame(void);

#endif
