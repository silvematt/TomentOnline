#ifndef OGAME_H_INCLUDED
#define OGAME_H_INCLUDED

#include <stdbool.h>
#include "SDL.h"
#include "O_GameDef.h"
#include "../Network/netdef.h"
#include "../Network/packet.h"
#include "../Engine/U_DataTypes.h"

extern dynamicSprite_t otherPlayerObject;

int O_GameInitializeOtherPlayer();

int O_GameOtherPlayerLoop();

int O_GameReceivePackets();
int O_GameSendPackets();

int O_GameOnPacketIsSent(void);
int O_GameOnPacketIsReceived(void);

#endif
