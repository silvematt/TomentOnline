#include "G_AIBehaviour.h"

#include <math.h>
#include "G_AI.h"
#include "G_Player.h"
#include "G_Pathfinding.h"
#include "P_Physics.h"
#include "G_AIBehaviour.h"

#include "../Online/O_Game.h"

// Enemy-Specific Behaviours

void G_AI_BehaviourMeeleEnemy(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

    cur->hasChanged = false;
    // Movements, done only for the host
    if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
    {
        // Calculate paths for both the player and the otherPlayer

        // Calculate player path
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

        // Calculate other player path
        path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

        // Select path
        if(path.isValid && otherPath.isValid)
        {
            if(path.nodesLength <= otherPath.nodesLength)
            {
                // Set the target as the this player
                cur->targetPos = &player.centeredPos;
                cur->targetGridPos = &player.gridPosition;
                cur->targetColl = &player.collisionCircle;
                cur->path = &path;
                
                // Check if aggro changed
                if(cur->hostAggro < cur->joinerAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 10;
                cur->joinerAggro = 0;
            }
            else
            {
                // Set the target as the other player
                cur->targetPos = &otherPlayerObject.base.centeredPos;
                cur->targetGridPos = &otherPlayerObject.base.gridPos;
                cur->targetColl = &otherPlayerObject.base.collisionCircle;
                cur->path = &otherPath;

                // Check if aggro changed
                if(cur->joinerAggro < cur->hostAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 0;
                cur->joinerAggro = 10;
            }
        }
        else if(path.isValid && !otherPath.isValid)
        {
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;
            cur->path = &path;        

            // Check if aggro changed
            if(cur->hostAggro < cur->joinerAggro)
                cur->hasChanged = true;

            cur->hostAggro = 10;
            cur->joinerAggro = 0;
        }
        else if(!path.isValid && otherPath.isValid)
        {
            cur->targetPos = &otherPlayerObject.base.centeredPos;
            cur->targetGridPos = &otherPlayerObject.base.gridPos;
            cur->targetColl = &otherPlayerObject.base.collisionCircle;
            cur->path = &otherPath;     

            // Check if aggro changed
            if(cur->joinerAggro < cur->hostAggro)
                cur->hasChanged = true;

            cur->hostAggro = 0;
            cur->joinerAggro = 10;   
        }
        else
            cur->path = &path; // not gonna happen anyway
            
        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Shortcut for cur->path
        path = *cur->path;

        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
        {            
            // From here on, the AI is chasing the player so it is safe to say that they're fighting
            if(player.hasBeenInitialized)
                cur->aggroedPlayer = true;

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->hasChanged = true;

                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                    cur->state = DS_STATE_MOVING;
                }
                else
                {
                    // Attacking
                    cur->state = DS_STATE_IDLE;
                }
        }
        else
        {
            cur->state = DS_STATE_IDLE;
        }


        // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                    break;
                }
            }
            else
            {
                // Move back
                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

        // Check Attack
        if(cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->hostAggro > cur->joinerAggro)
        {
            // In range for attacking
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            G_AIAttackPlayer(cur);
            cur->aggroedPlayer = true;
        }
    }
    else if(!thisPlayer.isHost)
    {
        // Update the displayPosition (pos is set directly when a packet arrives)
        if(cur->posArrived)
        {
            // Move smoothly the other player to last known pos
            float deltaX = (cur->base.pos.x) - cur->displayPos.x;
            float deltaY = (cur->base.pos.y) - cur->displayPos.y;
            float deltaZ = (cur->base.z) - cur->displayZ;

            cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
            cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
            cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
        }

        // Position is updated by packets
         // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                    break;
                }
            }
            else
            {
                // Move back
                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

        // Check Attack
        if(G_AICanAttack(cur) && cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->hostAggro < cur->joinerAggro)
        {
            // In range for attacking
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            G_AIAttackPlayer(cur);
            cur->aggroedPlayer = true;
        }
    }

    // Select Animation & Play it
    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
            
            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING  || cur->state == DS_STATE_SPECIAL1)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    return;
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }
}

void G_AI_BehaviourCasterEnemy(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Set the target as the player
    cur->targetPos = &player.centeredPos;
    cur->targetGridPos = &player.gridPosition;
    cur->targetColl = &player.collisionCircle;

    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
    float otherDist = sqrt((cur->base.centeredPos.x - otherPlayerObject.base.centeredPos.x)*(cur->base.centeredPos.x - otherPlayerObject.base.centeredPos.x) + (cur->base.centeredPos.y - otherPlayerObject.base.centeredPos.y)*(cur->base.centeredPos.y - otherPlayerObject.base.centeredPos.y));

    // Use the correct distance between
    float correctDist = cur->base.dist;

    cur->hasChanged = false;
    // Movements
    if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
    {
        // Calculate paths for both the player and the otherPlayer

        // Calculate player path
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

        // Calculate other player path
        path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

        // Select path
        if(path.isValid && otherPath.isValid)
        {
            if(path.nodesLength <= otherPath.nodesLength)
            {
                // Set the target as the this player
                cur->targetPos = &player.centeredPos;
                cur->targetGridPos = &player.gridPosition;
                cur->targetColl = &player.collisionCircle;
                cur->path = &path;
                
                // Check if aggro changed
                if(cur->hostAggro < cur->joinerAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 10;
                cur->joinerAggro = 0;

                correctDist = cur->base.dist;
            }
            else
            {
                // Set the target as the other player
                cur->targetPos = &otherPlayerObject.base.centeredPos;
                cur->targetGridPos = &otherPlayerObject.base.gridPos;
                cur->targetColl = &otherPlayerObject.base.collisionCircle;
                cur->path = &otherPath;

                // Check if aggro changed
                if(cur->joinerAggro < cur->hostAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 0;
                cur->joinerAggro = 10;

                correctDist = otherDist;
            }
        }
        else if(path.isValid && !otherPath.isValid)
        {
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;
            cur->path = &path;        

            // Check if aggro changed
            if(cur->hostAggro < cur->joinerAggro)
                cur->hasChanged = true;

            cur->hostAggro = 10;
            cur->joinerAggro = 0;

            correctDist = cur->base.dist;
        }
        else if(!path.isValid && otherPath.isValid)
        {
            cur->targetPos = &otherPlayerObject.base.centeredPos;
            cur->targetGridPos = &otherPlayerObject.base.gridPos;
            cur->targetColl = &otherPlayerObject.base.collisionCircle;
            cur->path = &otherPath;     

            // Check if aggro changed
            if(cur->joinerAggro < cur->hostAggro)
                cur->hasChanged = true;

            cur->hostAggro = 0;
            cur->joinerAggro = 10;   

            correctDist = otherDist;
        }
        else
        {
            cur->path = &path; // not gonna happen anyway
        }
            
        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Shortcut for cur->path
        path = *cur->path;
        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
        {
            // From here on, the AI is chasing the player so it is safe to say that they're fighting

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->hasChanged = true;

                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                    cur->state = DS_STATE_MOVING;
                }
                else
                {
                    // Attacking
                    cur->state = DS_STATE_IDLE;
                }
        }
        else
        {
            cur->state = DS_STATE_IDLE;
        }


        // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                        break;
                }
            }
            else
            {
                // Move back
                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

        // Check Attack
        // If the player is at attack distance OR if he was in combat before but the AI can't reach him to close up the distance (example: casters on towers)
        if(cur->base.dist < AI_SPELL_ATTACK_DISTANCE || otherDist < AI_SPELL_ATTACK_DISTANCE  || (cur->aggroedPlayer && !path.isValid))
        {
            // In range for attacking (casting spell)
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            cur->aggroedPlayer = true;
        }
    }
    else if(!thisPlayer.isHost)
    {
        // Update the displayPosition (pos is set directly when a packet arrives)
        if(cur->posArrived)
        {
            // Move smoothly the other player to last known pos
            float deltaX = (cur->base.pos.x) - cur->displayPos.x;
            float deltaY = (cur->base.pos.y) - cur->displayPos.y;
            float deltaZ = (cur->base.z) - cur->displayZ;

            cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
            cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
            cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }
        }

        // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                        break;
                }
            }
            else
            {
                // Move back
                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
    }

    // Select Animation & Play it
    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_SPECIAL1)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    return;
                }
                // Spawn the spell and go back to idle
                else if(cur->state == DS_STATE_ATTACKING)
                {
                    // Let only the host instance the projectile
                    if(thisPlayer.isHost)
                    {
                        // Attack chance, casters may fail spell
                        int attack      =  (rand() % (100)) + 1;

                        if(attack <= cur->attributes.attackChance)
                        {
                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;

                            if(cur->joinerAggro > cur->hostAggro)
                            {
                                projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                FIX_ANGLES_DEGREES(projAngle);
                                projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                            }

                            FIX_ANGLES_DEGREES(projAngle);

                            uint32_t networkID = REPL_GenerateNetworkID();

                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            
                            // Spawn it online
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);

                        }
                    }

                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }
}

void G_AI_BehaviourSkeletonLord(dynamicSprite_t* cur)
{
    switch(cur->state)
    {
        case DS_STATE_NULL:
            break;

        case DS_STATE_IDLE:
        case DS_STATE_MOVING:
        case DS_STATE_CASTING:
        case DS_STATE_ATTACKING:
        case DS_STATE_DEAD:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

            cur->hasChanged = false;
            // Movements
            if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
            {
                // Calculate paths for both the player and the otherPlayer

                // Calculate player path
                path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

                // Calculate other player path
                path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

                // Select path
                if(path.isValid && otherPath.isValid)
                {
                    if(path.nodesLength <= otherPath.nodesLength)
                    {
                        // Set the target as the this player
                        cur->targetPos = &player.centeredPos;
                        cur->targetGridPos = &player.gridPosition;
                        cur->targetColl = &player.collisionCircle;
                        cur->path = &path;
                        
                        // Check if aggro changed
                        if(cur->hostAggro < cur->joinerAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 10;
                        cur->joinerAggro = 0;
                    }
                    else
                    {
                        // Set the target as the other player
                        cur->targetPos = &otherPlayerObject.base.centeredPos;
                        cur->targetGridPos = &otherPlayerObject.base.gridPos;
                        cur->targetColl = &otherPlayerObject.base.collisionCircle;
                        cur->path = &otherPath;

                        // Check if aggro changed
                        if(cur->joinerAggro < cur->hostAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 0;
                        cur->joinerAggro = 10;
                    }
                }
                else if(path.isValid && !otherPath.isValid)
                {
                    cur->targetPos = &player.centeredPos;
                    cur->targetGridPos = &player.gridPosition;
                    cur->targetColl = &player.collisionCircle;
                    cur->path = &path;        

                    // Check if aggro changed
                    if(cur->hostAggro < cur->joinerAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 10;
                    cur->joinerAggro = 0;
                }
                else if(!path.isValid && otherPath.isValid)
                {
                    cur->targetPos = &otherPlayerObject.base.centeredPos;
                    cur->targetGridPos = &otherPlayerObject.base.gridPos;
                    cur->targetColl = &otherPlayerObject.base.collisionCircle;
                    cur->path = &otherPath;     

                    // Check if aggro changed
                    if(cur->joinerAggro < cur->hostAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 0;
                    cur->joinerAggro = 10;   
                }
                else
                    cur->path = &path; // not gonna happen anyway
                    
                float deltaX = 0.0f;
                float deltaY = 0.0f; 

                // Shortcut for cur->path
                path = *cur->path;

                // Check if path is valid and if there's space to follow it
                if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                    (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
                {
                    // From here on, the AI is chasing the player so it is safe to say that they're fighting
                    if(player.hasBeenInitialized && !cur->aggroedPlayer)
                    {
                        cur->cooldowns[0]->Start(cur->cooldowns[0]);
                        cur->cooldowns[1]->Start(cur->cooldowns[1]);
                        cur->aggroedPlayer = true;
                    }

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                    deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                    // Check if we're far away from the target
                    if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                        P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                        {
                            cur->hasChanged = true;

                            cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                            cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                            // Recalculate centered pos after delta move
                            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                            cur->state = DS_STATE_MOVING;
                        }
                        else
                        {
                            // Attacking
                            cur->state = DS_STATE_IDLE;
                        }
                }
                else
                {
                    cur->state = DS_STATE_IDLE;
                }


                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // If the tile the AI ended up in is not occupied

                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

                // For ten seconds, attack melee or ranged
                if(cur->cooldowns[0]->GetTicks(cur->cooldowns[0]) < 10000)
                {
                    // Check fireball cooldown
                    if(cur->cooldowns[1]->GetTicks(cur->cooldowns[1]) > 3000)
                    {
                        // In range for attacking (casting spell)
                        G_AIPlayAnimationOnce(cur, ANIM_CAST_SPELL);
                        O_GameAIPlayAnim(cur->networkID, ANIM_CAST_SPELL, false);
                    }
                    // Check Attack
                    else if(cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->hostAggro >= cur->joinerAggro)
                    {
                        // In range for attacking
                        G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
                        G_AIAttackPlayer(cur);
                    }
                }
                else
                {
                    // Do a spell
                    int spell =  rand() % (2);
                    
                    // Hell
                    if(spell == 0)
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL1);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL1, true);
                    }
                    // Resurrection
                    else
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL2);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL2, true);
                    }
                }
            }
            else if(!thisPlayer.isHost)
            {
                // Update the displayPosition (pos is set directly when a packet arrives)
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

                if(G_AICanAttack(cur) && cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->joinerAggro > cur->hostAggro)
                {
                    // In range for attacking
                    G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
                    G_AIAttackPlayer(cur);
                }
            }
            
            break;
        }

        case DS_STATE_SPECIAL1:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            
            cur->hasChanged = true;
            if(thisPlayer.isHost)
            {
                if(cur->base.gridPos.x != 10 || cur->base.gridPos.y != 109)
                {
                    vector2Int_t targetPos = {10,109};
                    path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
                    cur->path = &path;

                    float deltaX = 0.0f;
                    float deltaY = 0.0f; 

                    // Check if path is valid and if there's space to follow it
                    if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                        G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
                    {
                        deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                        deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                        // Check if we're far away from the target
                        cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                        cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                        // Check if this AI changed grid pos
                        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                        {
                            // If the tile the AI ended up in is not occupied

                            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                            {
                                // Update the dynamic map
                                switch(cur->base.level)
                                {
                                    case 0:
                                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                        break;
                                    
                                    case 1:
                                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    case 2:
                                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    default:
                                    break;
                                }
                            }
                            else
                            {
                                // Move back
                                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                                cur->base.gridPos.x = oldGridPosX;
                                cur->base.gridPos.y = oldGridPosY;
                            }
                        }
                        
                        // Update collision circle
                        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
                    }
                }
                // 2) Fly in the sky
                else if(cur->base.z < 128 && !cur->cooldowns[2]->isStarted)
                {
                    // Fly
                    cur->verticalMovementDelta = 100.0f;
                    cur->canBeHit = false;
                }
                // 3) Charge and go down
                else
                {
                    // Hell cooldown
                    if(!cur->cooldowns[2]->isStarted)
                    {
                        cur->cooldowns[2]->Start(cur->cooldowns[2]);
                        cur->base.z = 128;
                        cur->verticalMovementDelta = 0;
                    }

                    if(cur->cooldowns[2]->GetTicks(cur->cooldowns[2]) > 500)
                    {
                        if(cur->base.z > 0)
                        {
                            cur->verticalMovementDelta = -400.0f;
                            cur->canBeHit = true;
                        }
                        else
                        {
                            cur->verticalMovementDelta = 0;
                            cur->base.z = 0;
                            cur->canBeHit = true;

                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;
                            for(int i = 0; i < 36; i++)
                            {
                                FIX_ANGLES_DEGREES(projAngle);
                                uint32_t networkID = REPL_GenerateNetworkID();
                                G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                                O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                                projAngle += 10;
                            }

                            
                            cur->cooldowns[0]->Start(cur->cooldowns[0]);
                            cur->cooldowns[2]->Stop(cur->cooldowns[2]);
                            G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                            O_GameAIPlayAnim(cur->networkID, ANIM_IDLE, true);
                        }
                    }
                }
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
            
            break;
        }

        case DS_STATE_SPECIAL2:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            cur->hasChanged = true;

            if(thisPlayer.isHost)
            {
                // 1) Reach the center of the arena
                if(cur->base.gridPos.x != 10 || cur->base.gridPos.y != 109)
                {
                    vector2Int_t targetPos = {10,109};
                    path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
                    cur->path = &path;

                    float deltaX = 0.0f;
                    float deltaY = 0.0f; 

                    // Check if path is valid and if there's space to follow it
                    if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                        G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
                    {
                        deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                        deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                        // Check if we're far away from the target
                        cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                        cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;


                        // Check if this AI changed grid pos
                        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                        {
                            // If the tile the AI ended up in is not occupied

                            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                            {
                                // Update the dynamic map
                                switch(cur->base.level)
                                {
                                    case 0:
                                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                        break;
                                    
                                    case 1:
                                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    case 2:
                                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    default:
                                    break;
                                }
                            }
                            else
                            {
                                // Move back
                                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                                cur->base.gridPos.x = oldGridPosX;
                                cur->base.gridPos.y = oldGridPosY;
                            }
                        }
                        
                        // Update collision circle
                        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
                    }   
                }
                // 2) Fly in the sky
                else if(cur->base.z < 128 && !cur->cooldowns[3]->isStarted)
                {
                    // Fly
                    cur->verticalMovementDelta = 100.0f;
                    cur->canBeHit = false;
                }
                // 3) Resurrect and go down
                else
                {
                    // Resurrect cooldown
                    if(!cur->cooldowns[3]->isStarted)
                    {
                        cur->cooldowns[3]->Start(cur->cooldowns[3]);
                        cur->base.z = 128;
                        cur->verticalMovementDelta = 0;

                        // Edge case, if the boss spawns over 255 minions, free up a bit of the array to allow the new to be spawned
                        if(allDynamicSpritesLength+4 >= OBJECTARRAY_DEFAULT_SIZE_HIGH)
                        {
                            for(int i = OBJECTARRAY_DEFAULT_SIZE_HIGH-5; i < OBJECTARRAY_DEFAULT_SIZE_HIGH; i++)
                            {
                                if(allDynamicSprites[i] != NULL)
                                    free(allDynamicSprites[i]);
                            }

                            allDynamicSpritesLength = OBJECTARRAY_DEFAULT_SIZE_HIGH-5;
                        }

                        // Select from: Resurrection, Mass Resurrection and Royal Resurrection
                        int resurrection =  rand() % (3);

                        if(resurrection == 0)   // Resurrection
                        {
                            // Spawn AI
                            if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1];
                                G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x+1, cur->base.gridPos.y);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x+1, cur->base.gridPos.y, 3, true, ANIM_SPECIAL1, false);
                            }

                            // Spawn AI
                            if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1];
                                G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x-1, cur->base.gridPos.y);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x-1, cur->base.gridPos.y, 3, true, ANIM_SPECIAL1, false);
                            }

                            // Spawn AI
                            if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x];
                                G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x, cur->base.gridPos.y+1);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y+1, 3, true, ANIM_SPECIAL1, false);
                            }

                            // Spawn AI
                            if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x];
                                G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x, cur->base.gridPos.y-1);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y-1, 3, true, ANIM_SPECIAL1, false);
                            }
                        }
                        // Royal Resurrection
                        else if(resurrection == 1)
                        {
                            // Spawn AI
                            if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1];
                                G_AIInitialize(spawned, 0, DS_SkeletonElite, cur->base.gridPos.x+1, cur->base.gridPos.y);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x+1, cur->base.gridPos.y, DS_SkeletonElite, true, ANIM_SPECIAL1, false);
                            }
                            else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1];
                                G_AIInitialize(spawned, 0, DS_SkeletonElite, cur->base.gridPos.x-1, cur->base.gridPos.y);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x-1, cur->base.gridPos.y, DS_SkeletonElite, true, ANIM_SPECIAL1, false);
                            }
                            else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x];
                                G_AIInitialize(spawned, 0, DS_SkeletonElite, cur->base.gridPos.x, cur->base.gridPos.y+1);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y+1, DS_SkeletonElite, true, ANIM_SPECIAL1, false);
                            }
                            else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] == NULL)
                            {
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x];
                                G_AIInitialize(spawned, 0, DS_SkeletonElite, cur->base.gridPos.x, cur->base.gridPos.y-1);
                                G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y-1, DS_SkeletonElite, true, ANIM_SPECIAL1, false);
                            }
                        }
                        // Mass resurrection
                        else if(resurrection == 2)
                        {
                            int massResurrectionNum = 6;
                            for(int i = 0; i < massResurrectionNum; i++)
                            {
                                int mGridX = (rand() % (20 - 1 + 1)) + 1;
                                int mGridY = (rand() % (118 - 100 + 1)) + 100;

                                if(currentMap.dynamicSpritesLevel0[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel0[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[mGridY][mGridX];
                                    G_AIInitialize(spawned, 0, DS_Skeleton, mGridX, mGridY);
                                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 0, mGridX, mGridY, DS_Skeleton, true, ANIM_SPECIAL1, false);
                                }
                            }
                        }
                    }

                    if(cur->cooldowns[3]->GetTicks(cur->cooldowns[3]) > 2000)
                    {
                        if(cur->base.z > 0)
                        {
                            cur->verticalMovementDelta = -400.0f;
                            cur->canBeHit = true;
                        }
                        else
                        {
                            cur->verticalMovementDelta = 0;
                            cur->base.z = 0;
                            cur->canBeHit = true;

                            cur->cooldowns[0]->Start(cur->cooldowns[0]);
                            cur->cooldowns[3]->Stop(cur->cooldowns[3]);
                            G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                            O_GameAIPlayAnim(cur->networkID, ANIM_IDLE, true);
                        }
                    }
                }
                break;
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }
                
                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
        }
    }

    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_CASTING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpell;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellSheetLength;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
                else if(cur->state == DS_STATE_CASTING)
                {
                    if(thisPlayer.isHost)
                    {
                        // Spawn the spell and go back to idle
                        // Attack chance, casters may fail spell
                        int attack      =  (rand() % (100)) + 1;

                        if(attack <= cur->attributes.attackChance)
                        {
                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;

                            if(cur->joinerAggro > cur->hostAggro)
                            {
                                projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                FIX_ANGLES_DEGREES(projAngle);
                                projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                            }

                            FIX_ANGLES_DEGREES(projAngle);
                            
                            uint32_t networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                            projAngle += 10;
                            FIX_ANGLES_DEGREES(projAngle);
                            
                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);

                            projAngle -= 20;
                            FIX_ANGLES_DEGREES(projAngle);
                            
                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                        }
                        cur->cooldowns[1]->Start(cur->cooldowns[1]);
                    }
                    
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }

    // Extra, close door of player's spawn
    if(thisPlayer.isHost && cur->isAlive && doorstateLevel0[119][10] == DState_Open && (player.gridPosition.y < 116 || otherPlayerObject.base.gridPos.y < 116) && cur->base.gridPos.y < 116)
    {
        G_SetDoorState(0, 119, 10, DState_Closing);
    }
}

void G_AI_BehaviourMorgathulTheKeeper(dynamicSprite_t* cur)
{
    // Check phase
    if(cur->bossPhase == 0 && cur->attributes.curHealth <= cur->attributes.maxHealth / 2)
    {
        cur->bossPhase = 1;
    }

    switch(cur->state)
    {
        case DS_STATE_NULL:
            break;

        case DS_STATE_IDLE:
        case DS_STATE_MOVING:
        case DS_STATE_CASTING:
        case DS_STATE_ATTACKING:
        case DS_STATE_DEAD:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

            cur->hasChanged = false;
            // Movements
            if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
            {
                // Calculate paths for both the player and the otherPlayer

                // Calculate player path
                path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

                // Calculate other player path
                path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

                // Select path
                if(path.isValid && otherPath.isValid)
                {
                    if(path.nodesLength <= otherPath.nodesLength)
                    {
                        // Set the target as the this player
                        cur->targetPos = &player.centeredPos;
                        cur->targetGridPos = &player.gridPosition;
                        cur->targetColl = &player.collisionCircle;
                        cur->path = &path;
                        
                        // Check if aggro changed
                        if(cur->hostAggro < cur->joinerAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 10;
                        cur->joinerAggro = 0;
                    }
                    else
                    {
                        // Set the target as the other player
                        cur->targetPos = &otherPlayerObject.base.centeredPos;
                        cur->targetGridPos = &otherPlayerObject.base.gridPos;
                        cur->targetColl = &otherPlayerObject.base.collisionCircle;
                        cur->path = &otherPath;

                        // Check if aggro changed
                        if(cur->joinerAggro < cur->hostAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 0;
                        cur->joinerAggro = 10;
                    }
                }
                else if(path.isValid && !otherPath.isValid)
                {
                    cur->targetPos = &player.centeredPos;
                    cur->targetGridPos = &player.gridPosition;
                    cur->targetColl = &player.collisionCircle;
                    cur->path = &path;        

                    // Check if aggro changed
                    if(cur->hostAggro < cur->joinerAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 10;
                    cur->joinerAggro = 0;
                }
                else if(!path.isValid && otherPath.isValid)
                {
                    cur->targetPos = &otherPlayerObject.base.centeredPos;
                    cur->targetGridPos = &otherPlayerObject.base.gridPos;
                    cur->targetColl = &otherPlayerObject.base.collisionCircle;
                    cur->path = &otherPath;     

                    // Check if aggro changed
                    if(cur->joinerAggro < cur->hostAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 0;
                    cur->joinerAggro = 10;   
                }
                else
                    cur->path = &path; // not gonna happen anyway
                    
                float deltaX = 0.0f;
                float deltaY = 0.0f; 

                // Shortcut for cur->path
                path = *cur->path;

                // Check if path is valid and if there's space to follow it
                if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                    (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
                {
                    // From here on, the AI is chasing the player so it is safe to say that they're fighting
                    if(player.hasBeenInitialized && !cur->aggroedPlayer)
                    {
                        printf("BOSS INITIALIZED! \n\n\n\n\n\n\n");
                        cur->cooldowns[0]->Start(cur->cooldowns[0]);
                        cur->cooldowns[1]->Start(cur->cooldowns[1]);
                        cur->cooldowns[2]->Start(cur->cooldowns[2]);

                         // Check boss fight
                        if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                        {
                            player.isFightingBoss = true;
                            player.bossFighting = cur;
                        }

                        // Resurrect the kroganar
                        G_AIPlayAnimationOnce(cur, ANIM_SPECIAL1);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL1, true);
                        cur->aggroedPlayer = true;
                        return;
                    }

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        printf("FIGHTING BOSS! \n\n\n\n\n\n\n");

                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // Dont get too close
                    float pDistance = P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime));
                    if(pDistance > 200.0f)
                    {
                        deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                        deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;
                    }
                    else
                    {
                        deltaX = deltaY = 0;
                    }

                    // Check if we're far away from the target
                    if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                        pDistance > AI_STOP_DISTANCE)
                        {
                            cur->hasChanged = true;

                            cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                            cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                            // Recalculate centered pos after delta move
                            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                            cur->state = DS_STATE_MOVING;
                        }
                        else
                        {
                            // Attacking
                            cur->state = DS_STATE_IDLE;
                        }
                }
                else
                {
                    cur->state = DS_STATE_IDLE;
                }


                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // If the tile the AI ended up in is not occupied

                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

                // Normal stance
                if(cur->cooldowns[0]->GetTicks(cur->cooldowns[0]) < 10000)
                {
                    // Check attack cooldown
                    if(cur->cooldowns[1]->GetTicks(cur->cooldowns[1]) > 1700)
                    {
                        // In range for attacking (casting spell)
                        G_AIPlayAnimationOnce(cur, ANIM_CAST_SPELL);
                        O_GameAIPlayAnim(cur->networkID, ANIM_CAST_SPELL, false);
                    }
                }
                else
                {
                    int spell =  rand() % (2);

                    // Violet void/copy
                    if(spell == 0)
                    {
                        G_AIPlayAnimationOnce(cur, ANIM_SPECIAL2);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL2, true);
                    }
                    // Orb Storm
                    else if(spell == 1)
                    {
                        // Set counter, how many orbs the boss is going to launch
                        if(cur->bossPhase == 0)
                        {
                            cur->counter1 = (rand() % (12 - 6 + 1)) + 6;
                            cur->value1 = 500; // spell speed, one orb every 500ms
                        }
                        else
                        {
                            cur->counter1 = (rand() % (20 - 12 + 1)) + 12;
                            cur->value1 = 250; // spell speed, one orb every 250ms
                        }

                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL3);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL3, true);

                        
                        cur->cooldowns[3]->Start(cur->cooldowns[3]);
                    }


                    /*
                    // Do a spell
                    
                    // Hell
                    if(spell == 0)
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL1);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL1, true);
                    }
                    // Resurrection
                    else
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL2);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL2, true);
                    }
                    */
                }
            }
            else if(!thisPlayer.isHost)
            {
                // Update the displayPosition (pos is set directly when a packet arrives)
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
            
            break;
        }

        case DS_STATE_SPECIAL1:
        case DS_STATE_SPECIAL2:
        case DS_STATE_SPECIAL3:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            
            cur->hasChanged = true;
            if(thisPlayer.isHost)
            {
                // Play animation
                
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
            
            break;
        }
    }

    // Select Animation & Play it
    int curAnimActionFrame = 0;
    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_CASTING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpell;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellSheetLength;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        // Check for Special 1 - Resurrect Kroganar
        if(cur->state == DS_STATE_SPECIAL1 && cur->cooldowns[2]->IsPaused(cur->cooldowns[2]) == false)
        {
            if(cur->animFrame == curAnimActionFrame && thisPlayer.isHost)
            {
                if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+2][cur->base.gridPos.x] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+2][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+2][cur->base.gridPos.x];
                    G_AIInitialize(spawned, 0, DS_Kroganar, cur->base.gridPos.x, cur->base.gridPos.y+2);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                    O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y+2, DS_Kroganar, true, ANIM_SPECIAL1, false);
                }
                else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-2][cur->base.gridPos.x] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-2][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-2][cur->base.gridPos.x];
                    G_AIInitialize(spawned, 0, DS_Kroganar, cur->base.gridPos.x, cur->base.gridPos.y-2);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                    O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x, cur->base.gridPos.y-2, DS_Kroganar, true, ANIM_SPECIAL1, false);
                }
                else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+2] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+2] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+2];
                    G_AIInitialize(spawned, 0, DS_Kroganar, cur->base.gridPos.x+2, cur->base.gridPos.y);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                    O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x+2, cur->base.gridPos.y, DS_Kroganar, true, ANIM_SPECIAL1, false);
                }
                else if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-2] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-2] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-2];
                    G_AIInitialize(spawned, 0, DS_Kroganar, cur->base.gridPos.x-2, cur->base.gridPos.y);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                    O_GameAIInstantiate(spawned->networkID, 0, cur->base.gridPos.x-2, cur->base.gridPos.y, DS_Kroganar, true, ANIM_SPECIAL1, false);
                }

                cur->cooldowns[2]->Pause(cur->cooldowns[2]);
            }
        }
        // Special 2, violet void
        else if(cur->state == DS_STATE_SPECIAL2 && cur->cooldowns[2]->IsPaused(cur->cooldowns[2]) == false)
        {
            if(cur->animFrame == curAnimActionFrame && thisPlayer.isHost)
            {
                if(cur->bossPhase == 1)
                {
                    // Do copy as well
                    int massResurrectionNum = 4;
                    for(int i = 0; i < massResurrectionNum; i++)
                    {
                        int mGridX = (rand() % (34 - 22 + 1)) + 22;
                        int mGridY = (rand() % (72 - 45 + 1)) + 45;

                        if(currentMap.dynamicSpritesLevel0[mGridY][mGridX] == NULL)
                        {
                            currentMap.dynamicSpritesLevel0[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                            dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[mGridY][mGridX];
                            G_AIInitialize(spawned, 0, DS_MorgathulCopy, mGridX, mGridY);
                            G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                            O_GameAIInstantiate(spawned->networkID, 0, mGridX, mGridY, DS_MorgathulCopy, true, ANIM_SPECIAL1, false);
                        }
                    }
                }

                // Do violet void
                int puddlesLength = 64;
                packedpuddle_t puddles[puddlesLength];
                int count = 0;

                for(int x = -4; x < 4; x++)
                {
                    for(int y = -4; y < 4; y++)
                    {
                        bool already = false;

                        // Check if there is already a puddle
                        mappudlle_t* puddle = activeMapPuddlesHead;
                        while(puddle != NULL)
                        {
                            if(puddle->gridX == cur->base.gridPos.x+x && puddle->gridY == cur->base.gridPos.y+y)
                            {
                                already = true;
                                break;
                            }
                            puddle = puddle->next;
                        }

                        if(!already)
                        {
                            puddles[count] = G_SpawnMapPuddle(REPL_GenerateNetworkID(), cur->base.gridPos.x+x, cur->base.gridPos.y+y, false, true, 30.0f, 7500, cur->base.level, TEXTURE_VioletVoid, false);
                            count++;
                        }
                    }
                }

                O_GameSpawnPuddles(count, puddles);

                cur->cooldowns[2]->Pause(cur->cooldowns[2]);
            }
        }
        // Orb storm
        else if(cur->state == DS_STATE_SPECIAL3)
        {
            if(thisPlayer.isHost)
            {
                if(cur->cooldowns[3]->GetTicks(cur->cooldowns[3]) > cur->value1)
                {
                    if(cur->counter1 > 0)
                    {
                        float projAngle = cur->base.angle + 180;
                        float projZ = player.z;

                        if(cur->joinerAggro > cur->hostAggro)
                        {
                            projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                            FIX_ANGLES_DEGREES(projAngle);
                            projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                        }

                        FIX_ANGLES_DEGREES(projAngle);
                            
                        uint32_t networkID = REPL_GenerateNetworkID();
                        G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                        O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                       
                        cur->cooldowns[3]->Start(cur->cooldowns[3]);
                        cur->counter1--;
                    }
                    else    // Exit
                    {
                        cur->cooldowns[0]->Start(cur->cooldowns[0]);
                        G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                        cur->cooldowns[3]->Stop(cur->cooldowns[3]);
                    }
                }
            }
        }

        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
                else if(cur->state == DS_STATE_CASTING)
                {
                    if(thisPlayer.isHost)
                    {
                        // Spawn the spell and go back to idle
                        // Attack chance, casters may fail spell
                        int attack      =  (rand() % (100)) + 1;

                        if(attack <= cur->attributes.attackChance)
                        {
                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;

                            if(cur->joinerAggro > cur->hostAggro)
                            {
                                projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                FIX_ANGLES_DEGREES(projAngle);
                                projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                            }

                            FIX_ANGLES_DEGREES(projAngle);
                            
                            uint32_t networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                            
                            // Copies cast only 1 projectile
                            projAngle += 15;
                            FIX_ANGLES_DEGREES(projAngle);
                            
                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                            

                            projAngle -= 25;
                            FIX_ANGLES_DEGREES(projAngle);
                            
                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                    
                        }
                        cur->cooldowns[1]->Start(cur->cooldowns[1]);
                    }
                    
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
                else if(cur->state == DS_STATE_SPECIAL1)
                {
                    cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    cur->cooldowns[2]->Start(cur->cooldowns[2]);
                }
                else if(cur->state == DS_STATE_SPECIAL2)
                {
                    cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    cur->cooldowns[2]->Start(cur->cooldowns[2]);
                }
                else if(cur->state == DS_STATE_SPECIAL3)
                {
                    cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    cur->cooldowns[3]->Start(cur->cooldowns[3]);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }

    // Extra, close door of player's spawn
    if(thisPlayer.isHost && cur->isAlive && doorstateLevel0[58][21] == DState_Open && (player.gridPosition.x > 24 || otherPlayerObject.base.gridPos.x > 24) && cur->base.gridPos.y < 116)
    {
        G_SetDoorState(0, 58, 21, DState_Closing);
    }
}

void G_AI_BehaviourMorgathulCopy(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Set the target as the player
    cur->targetPos = &player.centeredPos;
    cur->targetGridPos = &player.gridPosition;
    cur->targetColl = &player.collisionCircle;

    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
    float otherDist = sqrt((cur->base.centeredPos.x - otherPlayerObject.base.centeredPos.x)*(cur->base.centeredPos.x - otherPlayerObject.base.centeredPos.x) + (cur->base.centeredPos.y - otherPlayerObject.base.centeredPos.y)*(cur->base.centeredPos.y - otherPlayerObject.base.centeredPos.y));

    // Use the correct distance between
    float correctDist = cur->base.dist;

    cur->hasChanged = false;
    // Movements
    if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
    {
        // Calculate paths for both the player and the otherPlayer

        // Calculate player path
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

        // Calculate other player path
        path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

        // Select path
        if(path.isValid && otherPath.isValid)
        {
            if(path.nodesLength <= otherPath.nodesLength)
            {
                // Set the target as the this player
                cur->targetPos = &player.centeredPos;
                cur->targetGridPos = &player.gridPosition;
                cur->targetColl = &player.collisionCircle;
                cur->path = &path;
                
                // Check if aggro changed
                if(cur->hostAggro < cur->joinerAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 10;
                cur->joinerAggro = 0;

                correctDist = cur->base.dist;
            }
            else
            {
                // Set the target as the other player
                cur->targetPos = &otherPlayerObject.base.centeredPos;
                cur->targetGridPos = &otherPlayerObject.base.gridPos;
                cur->targetColl = &otherPlayerObject.base.collisionCircle;
                cur->path = &otherPath;

                // Check if aggro changed
                if(cur->joinerAggro < cur->hostAggro)
                    cur->hasChanged = true;

                cur->hostAggro = 0;
                cur->joinerAggro = 10;

                correctDist = otherDist;
            }
        }
        else if(path.isValid && !otherPath.isValid)
        {
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;
            cur->path = &path;        

            // Check if aggro changed
            if(cur->hostAggro < cur->joinerAggro)
                cur->hasChanged = true;

            cur->hostAggro = 10;
            cur->joinerAggro = 0;

            correctDist = cur->base.dist;
        }
        else if(!path.isValid && otherPath.isValid)
        {
            cur->targetPos = &otherPlayerObject.base.centeredPos;
            cur->targetGridPos = &otherPlayerObject.base.gridPos;
            cur->targetColl = &otherPlayerObject.base.collisionCircle;
            cur->path = &otherPath;     

            // Check if aggro changed
            if(cur->joinerAggro < cur->hostAggro)
                cur->hasChanged = true;

            cur->hostAggro = 0;
            cur->joinerAggro = 10;   

            correctDist = otherDist;
        }
        else
        {
            cur->path = &path; // not gonna happen anyway
        }
            
        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Shortcut for cur->path
        path = *cur->path;
        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
        {
            // From here on, the AI is chasing the player so it is safe to say that they're fightingx
            if(player.hasBeenInitialized && !cur->aggroedPlayer)
            {
                cur->cooldowns[0]->Start(cur->cooldowns[0]);
                cur->cooldowns[1]->Start(cur->cooldowns[1]);
                cur->cooldowns[3]->Start(cur->cooldowns[3]);
                cur->aggroedPlayer = true;
            }

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->hasChanged = true;

                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                    cur->state = DS_STATE_MOVING;
                }
                else
                {
                    // Attacking
                    cur->state = DS_STATE_IDLE;
                }
        }
        else
        {
            cur->state = DS_STATE_IDLE;
        }


        // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                        break;
                }
            }
            else
            {
                // Move back
                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

        // Check Attack
        // If the player is at attack distance OR if he was in combat before but the AI can't reach him to close up the distance (example: casters on towers)
        if((cur->base.dist < AI_SPELL_ATTACK_DISTANCE || otherDist < AI_SPELL_ATTACK_DISTANCE  || (cur->aggroedPlayer && !path.isValid)) && 
            cur->cooldowns[0]->GetTicks(cur->cooldowns[0]) > 1500)
        {
            // In range for attacking (casting spell)
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            O_GameAIPlayAnim(cur->networkID, ANIM_ATTACK1, false);
        }
    }
    else if(!thisPlayer.isHost)
    {
        // Update the displayPosition (pos is set directly when a packet arrives)
        if(cur->posArrived)
        {
            // Move smoothly the other player to last known pos
            float deltaX = (cur->base.pos.x) - cur->displayPos.x;
            float deltaY = (cur->base.pos.y) - cur->displayPos.y;
            float deltaZ = (cur->base.z) - cur->displayZ;

            cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
            cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
            cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }
        }

        // Check if this AI changed grid pos
        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
        {
            // If the tile the AI ended up in is not occupied

            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
            {
                // Update the dynamic map
                switch(cur->base.level)
                {
                    case 0:
                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                        break;
                    
                    case 1:
                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    case 2:
                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                        break;

                    default:
                        break;
                }
            }
            else
            {
                // Move back
                cur->base.gridPos.x = oldGridPosX;
                cur->base.gridPos.y = oldGridPosY;

                cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                cur->state = DS_STATE_MOVING;
            }
        }
        
        // Update collision circle
        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
    }

    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_CASTING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpell;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellSheetLength;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_SPECIAL1)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    return;
                }
                // Spawn the spell and go back to idle
                else if(cur->state == DS_STATE_ATTACKING)
                {
                    // Let only the host instance the projectile
                    if(thisPlayer.isHost)
                    {
                        // Attack chance, casters may fail spell
                        int attack      =  (rand() % (100)) + 1;

                        if(attack <= cur->attributes.attackChance)
                        {
                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;

                            if(cur->joinerAggro > cur->hostAggro)
                            {
                                projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                FIX_ANGLES_DEGREES(projAngle);
                                projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                            }

                            FIX_ANGLES_DEGREES(projAngle);

                            uint32_t networkID = REPL_GenerateNetworkID();

                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            
                            // Spawn it online
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);

                            cur->cooldowns[0]->Start(cur->cooldowns[0]);
                        }
                    }

                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }
}

void G_AI_BehaviourTheFrozenLord(dynamicSprite_t* cur)
{
    // Check phase
    if(cur->bossPhase == 0 && cur->attributes.curHealth <= cur->attributes.maxHealth / 2)
    {
        cur->bossPhase = 1;
    }
    
    switch(cur->state)
    {
        case DS_STATE_NULL:
            break;

        case DS_STATE_IDLE:
        case DS_STATE_MOVING:
        case DS_STATE_CASTING:
        case DS_STATE_ATTACKING:
        case DS_STATE_DEAD:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

            cur->hasChanged = false;
            // Movements
            if(cur->isAlive && G_AICanAttack(cur) && thisPlayer.isHost)
            {
                // Calculate paths for both the player and the otherPlayer

                // Calculate player path
                path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, player.gridPosition, cur);

                // Calculate other player path
                path_t otherPath = G_PerformPathfinding(cur->base.level, cur->base.gridPos, otherPlayerObject.base.gridPos, cur);

                // Select path
                if(path.isValid && otherPath.isValid)
                {
                    if(path.nodesLength <= otherPath.nodesLength)
                    {
                        // Set the target as the this player
                        cur->targetPos = &player.centeredPos;
                        cur->targetGridPos = &player.gridPosition;
                        cur->targetColl = &player.collisionCircle;
                        cur->path = &path;
                        
                        // Check if aggro changed
                        if(cur->hostAggro < cur->joinerAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 10;
                        cur->joinerAggro = 0;
                    }
                    else
                    {
                        // Set the target as the other player
                        cur->targetPos = &otherPlayerObject.base.centeredPos;
                        cur->targetGridPos = &otherPlayerObject.base.gridPos;
                        cur->targetColl = &otherPlayerObject.base.collisionCircle;
                        cur->path = &otherPath;

                        // Check if aggro changed
                        if(cur->joinerAggro < cur->hostAggro)
                            cur->hasChanged = true;

                        cur->hostAggro = 0;
                        cur->joinerAggro = 10;
                    }
                }
                else if(path.isValid && !otherPath.isValid)
                {
                    cur->targetPos = &player.centeredPos;
                    cur->targetGridPos = &player.gridPosition;
                    cur->targetColl = &player.collisionCircle;
                    cur->path = &path;        

                    // Check if aggro changed
                    if(cur->hostAggro < cur->joinerAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 10;
                    cur->joinerAggro = 0;
                }
                else if(!path.isValid && otherPath.isValid)
                {
                    cur->targetPos = &otherPlayerObject.base.centeredPos;
                    cur->targetGridPos = &otherPlayerObject.base.gridPos;
                    cur->targetColl = &otherPlayerObject.base.collisionCircle;
                    cur->path = &otherPath;     

                    // Check if aggro changed
                    if(cur->joinerAggro < cur->hostAggro)
                        cur->hasChanged = true;

                    cur->hostAggro = 0;
                    cur->joinerAggro = 10;   
                }
                else
                    cur->path = &path; // not gonna happen anyway
                    
                float deltaX = 0.0f;
                float deltaY = 0.0f; 

                // Shortcut for cur->path
                path = *cur->path;

                // Check if path is valid and if there's space to follow it
                if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                    (G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false || G_GetFromDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == &otherPlayerObject))
                {
                    // From here on, the AI is chasing the player so it is safe to say that they're fighting
                    if(player.hasBeenInitialized && !cur->aggroedPlayer)
                    {
                        cur->cooldowns[0]->Start(cur->cooldowns[0]);
                        cur->cooldowns[1]->Start(cur->cooldowns[1]);
                        cur->aggroedPlayer = true;
                    }

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                    deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                    // Check if we're far away from the target
                    if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                        P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                        {
                            cur->hasChanged = true;

                            cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                            cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                            // Recalculate centered pos after delta move
                            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                            cur->state = DS_STATE_MOVING;
                        }
                        else
                        {
                            // Attacking
                            cur->state = DS_STATE_IDLE;
                        }
                }
                else
                {
                    cur->state = DS_STATE_IDLE;
                }


                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // If the tile the AI ended up in is not occupied

                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

                // For ten seconds, attack melee or ranged
                if(cur->cooldowns[0]->GetTicks(cur->cooldowns[0]) < 12000)
                {
                    // Check fireball cooldown
                    if(cur->cooldowns[1]->GetTicks(cur->cooldowns[1]) > 5000)
                    {
                        // In range for attacking (casting spell)
                        G_AIPlayAnimationOnce(cur, ANIM_CAST_SPELL);
                        O_GameAIPlayAnim(cur->networkID, ANIM_CAST_SPELL, false);
                    }
                    // Check Attack
                    else if(cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->hostAggro >= cur->joinerAggro)
                    {
                        // In range for attacking
                        G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
                        G_AIAttackPlayer(cur);
                    }
                }
                else
                {
                    // Do a spell
                    int spell =  rand() % (3);
                    
                    // Resurrection/CasterResurrection/Blizzard
                    if(spell == 0)
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL1);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL1, true);
                    }
                    // Resurrection
                    else if (spell == 1)
                    {
                        G_AIPlayAnimationLoop(cur, ANIM_SPECIAL2);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL2, true);
                    }
                    else
                    {
                        G_AIPlayAnimationOnce(cur, ANIM_SPECIAL3);
                        O_GameAIPlayAnim(cur->networkID, ANIM_SPECIAL3, false);
                    }
                }
            }
            else if(!thisPlayer.isHost)
            {
                // Update the displayPosition (pos is set directly when a packet arrives)
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;

                if(G_AICanAttack(cur) && cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level && cur->joinerAggro > cur->hostAggro)
                {
                    // In range for attacking
                    G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
                    G_AIAttackPlayer(cur);
                }
            }
            
            break;
        }

        case DS_STATE_SPECIAL1:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            
            cur->hasChanged = true;
            if(thisPlayer.isHost)
            {
                if(cur->base.gridPos.x != 67 || cur->base.gridPos.y != 73)
                {
                    vector2Int_t targetPos = {67,73};
                    path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
                    cur->path = &path;

                    float deltaX = 0.0f;
                    float deltaY = 0.0f; 

                    // Check if path is valid and if there's space to follow it
                    if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                        G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
                    {
                        deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                        deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                        // Check if we're far away from the target
                        cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                        cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                        // Check if this AI changed grid pos
                        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                        {
                            // If the tile the AI ended up in is not occupied

                            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                            {
                                // Update the dynamic map
                                switch(cur->base.level)
                                {
                                    case 0:
                                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                        break;
                                    
                                    case 1:
                                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    case 2:
                                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    default:
                                    break;
                                }
                            }
                            else
                            {
                                // Move back
                                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                                cur->base.gridPos.x = oldGridPosX;
                                cur->base.gridPos.y = oldGridPosY;
                            }
                        }
                        
                        // Update collision circle
                        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
                    }
                }
                // 2) Fly in the sky
                else if(cur->base.z < 128 && !cur->cooldowns[2]->isStarted)
                {
                    // Fly
                    cur->verticalMovementDelta = 100.0f;
                    cur->canBeHit = false;
                }
                // 3) Charge and go down
                else
                {
                    // Hell cooldown
                    if(!cur->cooldowns[2]->isStarted)
                    {
                        cur->cooldowns[2]->Start(cur->cooldowns[2]);
                        cur->base.z = 128;
                        cur->verticalMovementDelta = 0;

                        // Ice Spikes to fire
                        cur->counter1 = 20;

                        // Also do icy ground on phase 2
                        if(cur->bossPhase == 1)
                        {
                            int puddlesLength = 64;
                            packedpuddle_t puddles[puddlesLength];
                            int count = 0;

                            int hostTarget = rand() % 2;

                            int targetGridX = player.gridPosition.x;
                            int targetGridY = player.gridPosition.y;

                            if(hostTarget == 1)
                            {
                                targetGridX = otherPlayerObject.base.gridPos.x;
                                targetGridY = otherPlayerObject.base.gridPos.y;
                            }

                            for(int x = -4; x < 4; x++)
                            {
                                for(int y = -4; y < 4; y++)
                                {
                                    bool already = false;

                                    // Check if there is already a puddle
                                    mappudlle_t* puddle = activeMapPuddlesHead;
                                    while(puddle != NULL)
                                    {
                                        if(puddle->gridX == targetGridX+x && puddle->gridY == targetGridY+y)
                                        {
                                            already = true;
                                            break;
                                        }
                                        puddle = puddle->next;
                                    }

                                    if(!already)
                                    {
                                        puddles[count] = G_SpawnMapPuddle(REPL_GenerateNetworkID(), targetGridX+x, targetGridY+y, false, true, 50.0f, 20000, cur->base.level, TEXTURE_IcyGround, false);
                                        count++;
                                    }
                                }
                            }

                            O_GameSpawnPuddles(count, puddles);
                        }
                    }

                    if(cur->cooldowns[2]->GetTicks(cur->cooldowns[2]) > 250)
                    {
                        if(cur->counter1 <= 0)
                        {
                            if(cur->base.z > 0)
                            {
                                cur->verticalMovementDelta = -400.0f;
                                cur->canBeHit = true;
                            }
                            else
                            {
                                cur->verticalMovementDelta = 0;
                                cur->base.z = 0;
                                cur->canBeHit = true;

                                float projAngle = cur->base.angle + 180;
                                float projZ = player.z;

                                if(cur->joinerAggro > cur->hostAggro)
                                {
                                    projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                    FIX_ANGLES_DEGREES(projAngle);
                                    projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                                }

                                for(int i = 0; i < 36; i++)
                                {
                                    FIX_ANGLES_DEGREES(projAngle);
                                    uint32_t networkID = REPL_GenerateNetworkID();
                                    G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                                    O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                                    projAngle += 10;
                                }


                                cur->cooldowns[0]->Start(cur->cooldowns[0]);
                                cur->cooldowns[2]->Stop(cur->cooldowns[2]);
                                G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                                O_GameAIPlayAnim(cur->networkID, ANIM_IDLE, true);
                               
                            }
                        }
                        else
                        {
                            int hostTarget = rand() % 2;

                            float projAngle = cur->base.angle + 180;
                            float projZ = player.z;

                            if(hostTarget == 1)
                            {
                                projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                                FIX_ANGLES_DEGREES(projAngle);
                                projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                            }

                            FIX_ANGLES_DEGREES(projAngle);
                                
                            uint32_t networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                            O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                            
                            cur->counter1--;
                            cur->cooldowns[2]->Start(cur->cooldowns[2]);
                        }
                    }
                }
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
            
            break;
        }

        case DS_STATE_SPECIAL3:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            
            cur->hasChanged = true;
            if(thisPlayer.isHost)
            {
                // Play animation
                
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }

                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
            
            break;
        }

        case DS_STATE_SPECIAL2:
        {
            int oldGridPosX = cur->base.gridPos.x;
            int oldGridPosY = cur->base.gridPos.y;

            // Calculate centered pos
            cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
            cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

            // Calculate runtime stuff
            // Get Player Space pos
            cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
            cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

            cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
            cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

            // Determine AI's level
            cur->base.level = 0;
            cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

            if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
                cur->base.z += cur->verticalMovementDelta * deltaTime;

            cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
            FIX_ANGLES_DEGREES(cur->base.angle);
            
            // Set the target as the player
            cur->targetPos = &player.centeredPos;
            cur->targetGridPos = &player.gridPosition;
            cur->targetColl = &player.collisionCircle;

            // Calculate the distance to player
            cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);
            cur->hasChanged = true;

            if(thisPlayer.isHost)
            {
                // 1) Reach the center of the arena
                if(cur->base.gridPos.x != 67 || cur->base.gridPos.y != 73)
                {
                    vector2Int_t targetPos = {67,73};
                    path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
                    cur->path = &path;

                    float deltaX = 0.0f;
                    float deltaY = 0.0f; 

                    // Check if path is valid and if there's space to follow it
                    if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                        G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
                    {
                        deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                        deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                        // Check if we're far away from the target
                        cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                        cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;


                        // Check if this AI changed grid pos
                        if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                        {
                            // If the tile the AI ended up in is not occupied

                            if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                            {
                                // Update the dynamic map
                                switch(cur->base.level)
                                {
                                    case 0:
                                        currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                        break;
                                    
                                    case 1:
                                        currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    case 2:
                                        currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                        currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                        break;

                                    default:
                                    break;
                                }
                            }
                            else
                            {
                                // Move back
                                cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                                cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                                cur->base.gridPos.x = oldGridPosX;
                                cur->base.gridPos.y = oldGridPosY;
                            }
                        }
                        
                        // Update collision circle
                        cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                        cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
                    }   
                }
                // 2) Fly in the sky
                else if(cur->base.z < 128 && !cur->cooldowns[3]->isStarted)
                {
                    // Fly
                    cur->verticalMovementDelta = 100.0f;
                    cur->canBeHit = false;
                }
                // 3) Resurrect and go down
                else
                {
                    // Resurrect cooldown
                    if(!cur->cooldowns[3]->isStarted)
                    {
                        cur->cooldowns[3]->Start(cur->cooldowns[3]);
                        cur->base.z = 128;
                        cur->verticalMovementDelta = 0;

                        // Edge case, if the boss spawns over 255 minions, free up a bit of the array to allow the new to be spawned
                        if(allDynamicSpritesLength+4 >= OBJECTARRAY_DEFAULT_SIZE_HIGH)
                        {
                            for(int i = OBJECTARRAY_DEFAULT_SIZE_HIGH-5; i < OBJECTARRAY_DEFAULT_SIZE_HIGH; i++)
                            {
                                if(allDynamicSprites[i] != NULL)
                                    free(allDynamicSprites[i]);
                            }

                            allDynamicSpritesLength = OBJECTARRAY_DEFAULT_SIZE_HIGH-5;
                        }

                        // Select from: Mass Resurrection, Caster Resurrection and Icy Grounds
                        int resurrection =  rand() % (2);

                        if(resurrection == 0) // Mass Resurrection
                        {
                            int massResurrectionNum = 8;
                            for(int i = 0; i < massResurrectionNum; i++)
                            {
                                int mGridX = (rand() % (80 - 59 + 1)) + 59;
                                int mGridY = (rand() % (84 - 62 + 1)) + 62;

                                if(currentMap.dynamicSpritesLevel0[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel0[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[mGridY][mGridX];
                                    G_AIInitialize(spawned, 0, DS_Skeleton, mGridX, mGridY);
                                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 0, mGridX, mGridY, DS_Skeleton, true, ANIM_SPECIAL1, false);
                                }
                            }
                        }
                        // Caster Resurrection
                        else if(resurrection == 1)
                        {
                            int counter = 0; // how many caster he spawned

                            int mGridY = 61;
                            int mGridX = 62;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);

                                    counter++;
                                }
                            }
                            

                            if(counter < 4 && rand() % 3 != 0)
                            {
                                mGridY = 61;
                                mGridX = 68;
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 61;
                            mGridX = 78;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 65;
                            mGridX = 81;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 73;
                            mGridX = 81;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 82;
                            mGridX = 81;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 85;
                            mGridX = 74;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 85;
                            mGridX = 67;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }

                            mGridY = 85;
                            mGridX = 61;
                            if(counter < 4 && rand() % 3 != 0)
                            {
                                if(currentMap.dynamicSpritesLevel1[mGridY][mGridX] == NULL)
                                {
                                    currentMap.dynamicSpritesLevel1[mGridY][mGridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel1[mGridY][mGridX];
                                    G_AIInitialize(spawned, 1, DS_FrozenLordsCaster, mGridX, mGridY);
                                    //G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);

                                    O_GameAIInstantiate(spawned->networkID, 1, mGridX, mGridY, DS_FrozenLordsCaster, false, ANIM_SPECIAL1, false);
                                    counter++;
                                }
                            }
                        }
                    }

                    if(cur->cooldowns[3]->GetTicks(cur->cooldowns[3]) > 2000)
                    {
                        if(cur->base.z > 0)
                        {
                            cur->verticalMovementDelta = -400.0f;
                            cur->canBeHit = true;
                        }
                        else
                        {
                            cur->verticalMovementDelta = 0;
                            cur->base.z = 0;
                            cur->canBeHit = true;

                            cur->cooldowns[0]->Start(cur->cooldowns[0]);
                            cur->cooldowns[3]->Stop(cur->cooldowns[3]);
                            G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                            O_GameAIPlayAnim(cur->networkID, ANIM_IDLE, true);
                        }
                    }
                }
                break;
            }
            else if(!thisPlayer.isHost)
            {
                if(cur->posArrived)
                {
                    // Move smoothly the other player to last known pos
                    float deltaX = (cur->base.pos.x) - cur->displayPos.x;
                    float deltaY = (cur->base.pos.y) - cur->displayPos.y;
                    float deltaZ = (cur->base.z) - cur->displayZ;

                    cur->displayPos.x += (deltaX * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime;
                    cur->displayPos.y += (deltaY * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 
                    cur->displayZ     += (deltaZ * (cur->speed + DISPLAY_POS_SMOOTH_SPEED)) * deltaTime; 

                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }
                }
                
                // Check if this AI changed grid pos
                if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
                {
                    // Check boss fight
                    if(!player.isFightingBoss && cur->isBoss && cur->isAlive)
                    {
                        player.isFightingBoss = true;
                        player.bossFighting = cur;
                    }

                    // If the tile the AI ended up in is not occupied
                    if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                    {
                        // Update the dynamic map
                        switch(cur->base.level)
                        {
                            case 0:
                                currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                                break;
                            
                            case 1:
                                currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            case 2:
                                currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                                currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                                break;

                            default:
                            break;
                        }
                    }
                    else
                    {
                        // Move back
                        cur->base.gridPos.x = oldGridPosX;
                        cur->base.gridPos.y = oldGridPosY;

                        cur->base.pos.x = cur->base.gridPos.x*TILE_SIZE;
                        cur->base.pos.y = cur->base.gridPos.y*TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
        }
    }

    cur->animSpeed = ANIMATION_SPEED_DIVIDER;
    int curAnimActionFrame = 0;
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_CASTING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpell;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellSheetLength;
            curAnimActionFrame = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellActionFrame;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            curAnimActionFrame = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3ActionFrame;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    if(cur->animPlay)
    {
        // Cast spell
        if(cur->state == DS_STATE_CASTING && cur->cooldowns[1]->IsPaused(cur->cooldowns[1]) == false)
        {
            if(cur->animFrame == curAnimActionFrame && thisPlayer.isHost)
            {
                float projAngle = cur->base.angle + 180;
                float projZ = player.z;

                if(cur->joinerAggro > cur->hostAggro)
                {
                    projAngle = ((atan2(-(cur->base.centeredPos.y - (otherPlayerObject.lastPosY+PLAYER_CENTER_FIX)), (cur->base.centeredPos.x - (otherPlayerObject.lastPosX+PLAYER_CENTER_FIX))))* RADIAN_TO_DEGREE)*-1 + 180;
                    FIX_ANGLES_DEGREES(projAngle);
                    projZ = otherPlayerObject.lastPosZ + HALF_TILE_SIZE;
                }

                FIX_ANGLES_DEGREES(projAngle);
                
                if(cur->bossPhase == 0)
                {
                    uint32_t networkID = REPL_GenerateNetworkID();
                    G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                    O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                }
                else
                {
                    FIX_ANGLES_DEGREES(projAngle);
                            
                    uint32_t networkID = REPL_GenerateNetworkID();
                    G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                    O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                    
                    // Copies cast only 1 projectile
                    projAngle += 15;
                    FIX_ANGLES_DEGREES(projAngle);
                    
                    networkID = REPL_GenerateNetworkID();
                    G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                    O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
                    

                    projAngle -= 25;
                    FIX_ANGLES_DEGREES(projAngle);
                    
                    networkID = REPL_GenerateNetworkID();
                    G_SpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur, false);
                    O_GameSpawnProjectile(networkID, cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, projZ-(cur->base.z+HALF_TILE_SIZE), false, cur->networkID);
            
                }

                // Pause the cooldown, it will be resumed after this animation ends
                cur->cooldowns[1]->Pause(cur->cooldowns[1]);
            }
        }
        else if(cur->state == DS_STATE_SPECIAL3 && cur->cooldowns[3]->IsPaused(cur->cooldowns[3]) == false)
        {
            if(cur->animFrame == curAnimActionFrame && thisPlayer.isHost)
            {
                // Ground smash
                int puddlesLength = 64;
                packedpuddle_t puddles[puddlesLength];
                int count = 0;

                for(int x = -4; x < 4; x++)
                {
                    for(int y = -4; y < 4; y++)
                    {
                        bool already = false;

                        // Check if there is already a puddle
                        mappudlle_t* puddle = activeMapPuddlesHead;
                        while(puddle != NULL)
                        {
                            if(puddle->gridX == cur->base.gridPos.x+x && puddle->gridY == cur->base.gridPos.y+y)
                            {
                                already = true;
                                break;
                            }
                            puddle = puddle->next;
                        }

                        if(!already)
                        {
                            puddles[count] = G_SpawnMapPuddle(REPL_GenerateNetworkID(), cur->base.gridPos.x+x, cur->base.gridPos.y+y, false, true, 200.0f, 1000, cur->base.level, TEXTURE_IcyGround, false);
                            count++;
                        }
                    }
                }

                O_GameSpawnPuddles(count, puddles);

                // Pause the cooldown, it will be resumed after this animation ends
                cur->cooldowns[3]->Pause(cur->cooldowns[3]);
            }
        }

        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
            
            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
                else if(cur->state == DS_STATE_CASTING)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                    cur->cooldowns[1]->Start(cur->cooldowns[1]);
                }
                else if(cur->state == DS_STATE_SPECIAL3)
                {
                    if(cur->bossPhase == 0)
                    {
                        cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    }
                    else
                    {
                        // allow back-to-back spell
                        cur->cooldowns[2]->Stop(cur->cooldowns[2]);
                        cur->cooldowns[3]->Stop(cur->cooldowns[3]);
                    }

                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }

    // Extra, close door of player's spawn
    if(thisPlayer.isHost && cur->isAlive && doorstateLevel0[73][58] == DState_Open && (player.gridPosition.x > 64 || otherPlayerObject.base.gridPos.x > 64) && cur->base.gridPos.y < 116)
    {
        G_SetDoorState(0, 73, 58, DState_Closing);
    }
}