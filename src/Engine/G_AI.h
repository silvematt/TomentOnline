#ifndef AI_H_INCLUDED
#define AI_H_INCLUDED

#include "D_AssetsManager.h"
#include "U_DataTypes.h"

#define AI_STOP_DISTANCE 90.0f
#define AI_MELEE_ATTACK_DISTANCE 100.0f

#define AI_SPELL_ATTACK_DISTANCE 600.0f


// Dynamic AI list
extern dynamicSprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE_HIGH];
extern unsigned int allDynamicSpritesLength;

//-------------------------------------
// Initializes the AI
//-------------------------------------
void G_AIInitialize(dynamicSprite_t* cur, int level, int spriteID, int x, int y);

//-------------------------------------
// Updates all AI entities
//-------------------------------------
void G_AIUpdate(void);

//-------------------------------------
// Sets an AI to be dead
//-------------------------------------
void G_AIDie(dynamicSprite_t* cur);

//-------------------------------------
// Plays an animation only once
//-------------------------------------
void G_AIPlayAnimationOnce(dynamicSprite_t* cur, objectanimationsID_e animID);

//-------------------------------------
// Sets an animation to be played in loop
//-------------------------------------
void G_AIPlayAnimationLoop(dynamicSprite_t* cur, objectanimationsID_e animID);

//-------------------------------------
// Damages the AI entity
//-------------------------------------
void G_AITakeDamage(dynamicSprite_t* cur, float amount, bool setAggro);

//-------------------------------------
// True if the AI can attack
//-------------------------------------
bool G_AICanAttack(dynamicSprite_t* cur);

//-------------------------------------
// Damages the player
//-------------------------------------
void G_AIAttackPlayer(dynamicSprite_t* cur);


void G_AICheckPuddleDamage(dynamicSprite_t* ai);

void G_FreeDynamicSprite(dynamicSprite_t* ai);

#endif
