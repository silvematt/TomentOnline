#ifndef AIBEHAVIOUR_H_INCLUDED
#define AIBEHAVIOUR_H_INCLUDED

#include "D_AssetsManager.h"
#include "U_DataTypes.h"

#define DISPLAY_POS_SMOOTH_SPEED 1.0f

// AI Specific, allows to modify the AI Behaviour for each enemy
void G_AI_BehaviourMeeleEnemy(dynamicSprite_t* cur);
void G_AI_BehaviourCasterEnemy(dynamicSprite_t* cur);
void G_AI_BehaviourSkeletonLord(dynamicSprite_t* cur);
void G_AI_BehaviourMorgathulTheKeeper(dynamicSprite_t* cur);
void G_AI_BehaviourMorgathulCopy(dynamicSprite_t* cur);

#endif
