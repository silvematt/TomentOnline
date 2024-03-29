#ifndef OGAME_H_INCLUDED
#define OGAME_H_INCLUDED

#include <stdbool.h>
#include "SDL.h"
#include "O_GameDef.h"
#include "../Network/netdef.h"
#include "../Network/packet.h"
#include "../Engine/U_DataTypes.h"
#include "../Engine/T_TextRendering.h"

extern dynamicSprite_t otherPlayerObject;

extern packedprojectilespawn_t projectilesToSend[MAX_PROJECTILES_TO_SEND_SIZE];
extern unsigned projectilesToSendLength;

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
void O_GameDestroyProjectile(int pNetworkID, int pSpriteID, bool pForceDestroy);

void O_GameSendAIUpdate(void);
void O_GameAITakeDamage(int pNetworkID, float pDamage, bool pDied);
void O_GameAIPlayAnim(int networkID, int anim, bool loop);
void O_GameAIInstantiate(int pNetworkID, int pLevel, int pGridX, int pGridY, int pSpriteID, bool pPlayAnim, int pAnimID, bool pLoop);
void O_GameSpawnPuddles(int length, packedpuddle_t puddles[MAX_PUDDLE_ABS_SIZE]);
void O_GameHealOther(float amount);
void O_GameSendDeathPacket(void);
void O_GameSendChatMessage(char* msg);

#endif
