#ifndef OGAME_H_INCLUDED
#define OGAME_H_INCLUDED

#include <stdbool.h>
#include "SDL.h"
#include "O_GameDef.h"
#include "../Network/netdef.h"
#include "../Network/packet.h"
#include "../Engine/U_DataTypes.h"

extern dynamicSprite_t otherPlayerObject;

int O_GameInitializeOtherPlayer(void);

int O_GameOtherPlayerLoop(void);

int O_GameReceivePackets(void);
int O_GameSendPackets(void);

int O_GameOnPacketIsSent(void);
int O_GameOnPacketIsReceived(void);

void O_GameOtherPlayerRender(void);

void O_GameSetDoorState(int level, int dX, int dY, doorstate_e state);
void O_GamePickPickup(int level, int dX, int dY);
void O_GameSpawnProjectile(int pNetworkID, int pSpriteID, float pAngle, int pLevel, float pPosX, float pPosY, float pPosZ, float pVerticalAngle, bool pIsOfPlayer, int pAiOwnerID);
void O_GameDestroyProjectile(int pNetworkID, int pSpriteID);

void O_GameSendAIUpdate(void);

#endif
