#include <math.h>

#include "O_Game.h"
#include "../Engine/P_Physics.h"
#include "../Engine/G_Player.h"
#include "../Engine/G_Pathfinding.h"
#include "../Engine/G_AI.h"
#include "../Engine/T_TextRendering.h"

#include "../Network/replication.h"

dynamicSprite_t otherPlayerObject;

packedprojectilespawn_t projectilesToSend[MAX_PROJECTILES_TO_SEND_SIZE];
unsigned projectilesToSendLength = 0;

// Initializes the other player's sprite rapresentation
int O_GameInitializeOtherPlayer(void)
{
    // Init player
    otherPlayerObject.base.active = TRUE;
    otherPlayerObject.base.pos.x = (currentMap.playerStartingGridX * TILE_SIZE);
    otherPlayerObject.base.pos.y = (currentMap.playerStartingGridY * TILE_SIZE);
    
    // Move the second player a tile to the right (TODO: may change this in the future)
    if(!otherPlayer.isHost)
        otherPlayerObject.base.pos.x += TILE_SIZE;

    otherPlayerObject.base.collisionCircle.pos.x = otherPlayerObject.base.pos.x;
    otherPlayerObject.base.collisionCircle.pos.y = otherPlayerObject.base.pos.y;

    otherPlayerObject.base.angle = currentMap.playerStartingRot;
    otherPlayerObject.base.z = (HALF_TILE_SIZE) + ((TILE_SIZE) * currentMap.playerStartingLevel);
    otherPlayerObject.base.level = currentMap.playerStartingLevel;

    otherPlayerObject.base.gridPos.x = ((otherPlayerObject.base.pos.x+PLAYER_CENTER_FIX) / TILE_SIZE);
    otherPlayerObject.base.gridPos.y = ((otherPlayerObject.base.pos.y+PLAYER_CENTER_FIX) / TILE_SIZE);

    otherPlayerObject.base.collisionCircle.r = TILE_SIZE / 2;

    otherPlayerObject.base.spriteID = DS_PlayerTank+otherPlayer.selectedClass;
    otherPlayerObject.base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[DS_PlayerTank+otherPlayer.selectedClass];

    otherPlayerObject.type = DS_TYPE_OTHERPLAYER;
    
    otherPlayerObject.base.active = true;
    otherPlayerObject.base.level = 0;
    otherPlayerObject.base.level = SDL_clamp(otherPlayerObject.base.level, 0, MAX_N_LEVELS-1);

    otherPlayerObject.base.z = TILE_SIZE * (0);
    otherPlayerObject.verticalMovementDelta = 0.0f;

    otherPlayerObject.canBeHit = false;

    otherPlayerObject.base.collisionCircle.pos.x = otherPlayerObject.base.pos.x;
    otherPlayerObject.base.collisionCircle.pos.y = otherPlayerObject.base.pos.y;
    otherPlayerObject.base.collisionCircle.r = -1.0f;

    // Get ID

    //---------------------
    // Dynamic-Specific
    //---------------------
    otherPlayerObject.curAnim = tomentdatapack.sprites[otherPlayerObject.base.spriteID]->animations->animIdle;
    otherPlayerObject.curAnimLength = tomentdatapack.sprites[otherPlayerObject.base.spriteID]->animations->animIdleSheetLength;

    otherPlayerObject.animTimer = U_TimerCreateNew();
    otherPlayerObject.animPlay = false;
    otherPlayerObject.animFrame = 0;
    otherPlayerObject.animPlayOnce = false;
    
    
    /*
    // Absolute initialization, should not be repeted on next G_InitPlayer calls
    if(!player.hasBeenInitialized)
    {
        player.attributes.maxHealth = 100.0f;
        player.attributes.curHealth = player.attributes.maxHealth;
        
        player.attributes.maxMana = 100.0f;
        player.attributes.curMana = player.attributes.maxMana;

        G_PlayerSetWeapon(PLAYER_FP_HANDS);
        G_PlayerSetSpell(SPELL_NULL);
        player.hasBeenInitialized = true;

        player.hasAxe = false;
        player.hasGreatsword = false;
        player.hasFireball = false;
        player.hasIceDart = false;
    }
    */
   
    otherPlayerObject.state = DS_STATE_IDLE;

    printf("Other player gird %d | %d\n", otherPlayerObject.base.gridPos.x, otherPlayerObject.base.gridPos.y);
}

int O_GameOtherPlayerLoop(void)
{
    int oldGridPosX = otherPlayerObject.base.gridPos.x;
    int oldGridPosY = otherPlayerObject.base.gridPos.y;

    // Compute centered pos for calculations
    otherPlayerObject.base.centeredPos.x = otherPlayerObject.base.pos.x + PLAYER_CENTER_FIX;
    otherPlayerObject.base.centeredPos.y = otherPlayerObject.base.pos.y + PLAYER_CENTER_FIX;

    otherPlayerObject.base.gridPos.x = ((otherPlayerObject.base.pos.x+PLAYER_CENTER_FIX) / TILE_SIZE);
    otherPlayerObject.base.gridPos.y = ((otherPlayerObject.base.pos.y+PLAYER_CENTER_FIX) / TILE_SIZE);
    
    // Calculate runtime stuff
    // Get Player Space pos
    otherPlayerObject.base.pSpacePos.x = otherPlayerObject.base.centeredPos.x - player.centeredPos.x;
    otherPlayerObject.base.pSpacePos.y = otherPlayerObject.base.centeredPos.y - player.centeredPos.y;

    // Determine AI's level
    otherPlayerObject.base.level = (int)floor(otherPlayerObject.base.z / TILE_SIZE);
    otherPlayerObject.base.level = SDL_clamp(otherPlayerObject.base.level, 0, MAX_N_LEVELS-1);

    // Calculate the distance to player
    otherPlayerObject.base.dist = sqrt(otherPlayerObject.base.pSpacePos.x*otherPlayerObject.base.pSpacePos.x + otherPlayerObject.base.pSpacePos.y*otherPlayerObject.base.pSpacePos.y);

    float deltaX = 0.0f;
    float deltaY = 0.0f;

    // Update collision circle
    otherPlayerObject.base.collisionCircle.pos.x = otherPlayerObject.base.centeredPos.x;
    otherPlayerObject.base.collisionCircle.pos.y = otherPlayerObject.base.centeredPos.y;
}

void O_GameOtherPlayerRender(void)
{
    float scaleFactor = max(1 - (otherPlayerObject.base.dist - 50) / 950, 0.35);
    float ratio = ((float)SCREEN_WIDTH/PROJECTION_PLANE_WIDTH);
    T_DisplayTextScaled(FONT_BLKCRY_GREEN, otherPlayer.name, otherPlayerObject.overheadPos.x*ratio, otherPlayerObject.overheadPos.y*ratio, scaleFactor);
}

int O_GameReceivePackets(void)
{
    return PCKT_ReceivePacket(O_GameOnPacketIsReceived);
}

int O_GameSendPackets(void)
{
    // Send our position
    // Update is sent 20 times per seconds
    if(playerUpdatePacketsTimer->GetTicks(playerUpdatePacketsTimer) > 50)
    {
        // Make movement packet and send it to the other player
        // Send packet to notice other player that we are starting the game, he will start as soon as this packet is received
        pckt_t* playerPacket = PCKT_MakePlayerUpdatePacket(&packetToSend, player.position.x, player.position.y, player.z-HALF_TILE_SIZE, player.angle, player.attributes.curHealth, player.attributes.maxHealth, player.attributes.curMana, player.attributes.maxMana, player.curWeapon, player.curSpell);

        // outputpcktbuffer was already sending something, check if we can append this packet
        if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
        {
            // Append this packet, it will be sent after the ones already in buffer
            outputPcktBuffer.hasBegunWriting = TRUE;
            memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)playerPacket, PCKT_SIZE);
            outputPcktBuffer.packetsToWrite++;
        }
        else
        {
            // Outputpcktbuffer is full and this packet should be sent, what to do?
            printf("CRITICAL ERROR: Send buffer was full when in O_GameSendPackets\n");
        }

        playerUpdatePacketsTimer->Start(playerUpdatePacketsTimer);
    }

    if(thisPlayer.isHost)
        O_GameSendAIUpdate();


    // Send projectiles
    bool done = false;
    int projectilesSent = 0;
    int offset = 0;
    while(!done)
    {
        int projectilesToSendCount = projectilesToSendLength - projectilesSent; 

        if(projectilesToSendCount >= MAX_PROJECTILESPAWN_PER_PACKET)
        {
            // Make greet packet
            pckt_t* spawnProjectilePacket = PCKT_MakeProjectileSpawnPacket(&packetToSend, MAX_PROJECTILESPAWN_PER_PACKET, projectilesToSend);
            
            if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
            {
                // Store the packet in the output buffer
                outputPcktBuffer.hasBegunWriting = TRUE;
                memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)spawnProjectilePacket, PCKT_SIZE);
                outputPcktBuffer.packetsToWrite++;
            }
            else
            {
                printf("CRITICAL ERROR: Send buffer was full when in O_GameSpawnProjectile\n");
            }

            projectilesSent += MAX_PROJECTILESPAWN_PER_PACKET;
        }
        else
        {
            if(projectilesToSendCount == 0)
                done = true;
            else
            {
                // Send remainder

                pckt_t* spawnProjectilePacket = PCKT_MakeProjectileSpawnPacket(&packetToSend, projectilesToSendCount, projectilesToSend+projectilesSent);
            
                if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
                {
                    // Store the packet in the output buffer
                    outputPcktBuffer.hasBegunWriting = TRUE;
                    memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)spawnProjectilePacket, PCKT_SIZE);
                    outputPcktBuffer.packetsToWrite++;
                }
                else
                {
                    printf("CRITICAL ERROR: Send buffer was full when in O_GameSpawnProjectile\n");
                }

                projectilesToSendCount = 0;
                done = true;
            }
        }
    }
    // Reset abs array length
    projectilesToSendLength = 0;

    return PCKT_SendPacket(O_GameOnPacketIsSent);
}


int O_GameOnPacketIsSent(void)
{

}

int O_GameOnPacketIsReceived(void)
{
    // When this function gets called, the packet arrived on the PCKT_ReceivePacket call and was saved inside the inputPacketBuffer->buffer
    // At this point, receivedPacket points at the inputPacketBuffer->buffer that contains the packet that arrived
    char thisPcktBuffer[PCKT_SIZE];
    memcpy(thisPcktBuffer, inputPcktBuffer.buffer+inputPcktBuffer.packetOffset, PCKT_SIZE);

    pckt_t* receivedPacket = (pckt_t*)thisPcktBuffer;

    printf("Packet received! ID: %d\n", receivedPacket->id);

    switch(receivedPacket->id)
    {
        case PCKTID_PLAYERUPDATE:
        {
            // Manage packet
            pckt_playerupdate_t playerPacket;
            memcpy(&playerPacket, receivedPacket->data, sizeof(playerPacket));

            printf("Packet received! ID: %d | - Values (%f,%f,%f)\n", receivedPacket->id, playerPacket.x, playerPacket.y, playerPacket.angle);

            // Update other player position
            otherPlayerObject.base.pos.x = playerPacket.x;
            otherPlayerObject.base.pos.y = playerPacket.y;
            otherPlayerObject.base.z = playerPacket.z;
            otherPlayerObject.base.angle = playerPacket.angle;

            otherPlayerObject.attributes.curHealth = playerPacket.curHealth;
            otherPlayerObject.attributes.maxHealth = playerPacket.maxHealth;
            otherPlayerObject.attributes.curMana = playerPacket.curMana;
            otherPlayerObject.attributes.maxMana = playerPacket.maxMana;

            otherPlayer.curWeapon = playerPacket.curWeapon;
            otherPlayer.curSpell = playerPacket.curSpell;
            break;
        }

        case PCKTID_DOOR_CHANGE:
        {
            // Manage packet
            pckt_door_change_t doorPacket;
            memcpy(&doorPacket, receivedPacket->data, sizeof(doorPacket));

            printf("Packet received! ID: %d | - Values (%d,%d,%d,%d)\n", receivedPacket->id, doorPacket.level, doorPacket.x, doorPacket.y, doorPacket.state);

            // Update doors
            switch(doorPacket.level)
            {
                case 0:
                    doorstateLevel0[doorPacket.y][doorPacket.x] = doorPacket.state;
                    break;

                case 1:
                    doorstateLevel1[doorPacket.y][doorPacket.x] = doorPacket.state;
                    break;

                case 2:
                    doorstateLevel2[doorPacket.y][doorPacket.x] = doorPacket.state;
                    break;
            }
            break;
        }

        case PCKTID_PICKUP_PICKED:
        {
            // Manage packet
            pckt_pickup_picked_t pickupPacket;
            memcpy(&pickupPacket, receivedPacket->data, sizeof(pickupPacket));

            printf("Packet received! ID: %d | - Values (%d,%d,%d)\n", receivedPacket->id, pickupPacket.level, pickupPacket.x, pickupPacket.y);

            R_SetValueFromSpritesMap(pickupPacket.level, pickupPacket.y, pickupPacket.x, 0);
            R_SetValueFromCollisionMap(pickupPacket.level, pickupPacket.y, pickupPacket.x, 0);
            G_SetObjectTMap(pickupPacket.level, pickupPacket.y, pickupPacket.x, ObjT_Empty);
            break;
        }

        case PCKTID_PROJECTILE_SPAWN:
        {
            // Manage packet
            pckt_projectile_spawn_t projectilePacket;
            memcpy(&projectilePacket, receivedPacket->data, sizeof(projectilePacket));

            printf("Packet received! ID: %d | Value: %d\n", receivedPacket->id, projectilePacket.length);

            for(int i = 0; i < projectilePacket.length; i++)
            {
                G_SpawnProjectile(projectilePacket.projectiles[i].networkID, projectilePacket.projectiles[i].spriteID, projectilePacket.projectiles[i].angle, projectilePacket.projectiles[i].level, projectilePacket.projectiles[i].posX, projectilePacket.projectiles[i].posY, projectilePacket.projectiles[i].posZ, projectilePacket.projectiles[i].verticalAngle, projectilePacket.projectiles[i].isOfPlayer, NULL, true);
            }

            break;
        }

        case PCKTID_PROJECTILE_DESTR:
        {
            // Manage packet
            pckt_projectile_destr_t projectilePacket;
            memcpy(&projectilePacket, receivedPacket->data, sizeof(projectilePacket));

            printf("Packet received! ID: %d\n", receivedPacket->id);

            projectileNode_t* cur = projectilesHead;
            int i = 0;
            while(cur != NULL)
            {
                if((cur->isNetworkInstance || projectilePacket.forceDestroy) && cur->networkID == projectilePacket.networkID)
                {
                    cur->this.isBeingDestroyed = true;
                    G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                    break;
                }

                i++;
                cur = cur->next;
            }

            break;
        }

        case PCKTID_AI_MOVEMENTS:
        {
            // The host shall discard these packets
            if(thisPlayer.isHost)
                break;

             // Manage packet
            pckt_aimovementupdate_t aiPacket;
            memcpy(&aiPacket, receivedPacket->data, sizeof(aiPacket));

            printf("Packet received! ID: %d - Length: %d\n", receivedPacket->id, aiPacket.length);

            for(int i = 0; i < aiPacket.length; i++)
            {
                aireplicated_t* cur = &aiPacket.ais[i];
                allDynamicSprites[cur->networkID]->base.pos.x = cur->x;
                allDynamicSprites[cur->networkID]->base.pos.y = cur->y;
                allDynamicSprites[cur->networkID]->base.z = cur->z;
                allDynamicSprites[cur->networkID]->hostAggro = cur->hostAggro;
                allDynamicSprites[cur->networkID]->joinerAggro = cur->joinerAggro;
            }

            break;
        }

        case PCKTID_AI_ATTACKED:
        {
            // Manage packet
            pckt_aiattacked_t aiPacket;
            memcpy(&aiPacket, receivedPacket->data, sizeof(aiPacket));

            printf("Packet received! ID: %d | - Values (%d,%f,%d)\n", receivedPacket->id, aiPacket.networkID, aiPacket.damage, aiPacket.died);

            if(allDynamicSprites[aiPacket.networkID]->isAlive)
                G_AITakeDamage(allDynamicSprites[aiPacket.networkID], aiPacket.damage);

            // Check our died with other players
            
            break;
        }

        case PCKTID_AI_PLAY_ANIM:
        {
            // Manage packet
            pckt_aiplayanim_t aiPacket;
            memcpy(&aiPacket, receivedPacket->data, sizeof(aiPacket));

            printf("Packet received! ID: %d | - Values (%d,%d)\n", receivedPacket->id, aiPacket.networkID, aiPacket.anim);

            if(allDynamicSprites[aiPacket.networkID]->isAlive)
            {
                if(aiPacket.loop)
                    G_AIPlayAnimationLoop(allDynamicSprites[aiPacket.networkID], aiPacket.anim);
                else
                    G_AIPlayAnimationOnce(allDynamicSprites[aiPacket.networkID], aiPacket.anim);
            }

            // Check our died with other players
            
            break;
        }

        case PCKTID_AI_INSTANTIATE:
        {
            // Manage packet
            pckt_aiinstantiate_t aiPacket;
            memcpy(&aiPacket, receivedPacket->data, sizeof(aiPacket));

            printf("Packet received! ID: %d | - Values (%d,%d)\n", receivedPacket->id, aiPacket.networkID, aiPacket.spriteID);

            // Spawn AI
            if(currentMap.dynamicSpritesLevel0[aiPacket.gridY][aiPacket.gridX] != NULL)
            {
                printf("Couldn't be able instantiate... game may be out of sync\n");
                scanf("%d");
            }

            currentMap.dynamicSpritesLevel0[aiPacket.gridY][aiPacket.gridX] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
            dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[aiPacket.gridY][aiPacket.gridX];
            G_AIInitialize(spawned, aiPacket.level, aiPacket.spriteID, aiPacket.gridX, aiPacket.gridY);

            if(aiPacket.playAnimation)
            {
                if(aiPacket.loop)
                    G_AIPlayAnimationLoop(spawned, aiPacket.animID);
                else
                    G_AIPlayAnimationOnce(spawned, aiPacket.animID);
            }

            if(spawned->networkID != aiPacket.networkID)
            {
                printf("Instantiated AI had a different network ID than the other part\n");
                scanf("%d");
            }
            break;
        }

        case PCKTID_PUDDLES_INSTANTIATE:
        {
            // Manage packet
            pckt_puddle_instantiate_t puddlePacket;
            memcpy(&puddlePacket, receivedPacket->data, sizeof(puddlePacket));

            printf("Packet received! ID: %d | - Values (%d)\n", receivedPacket->id, puddlePacket.length);

            for(int i = 0; i < puddlePacket.length; i++)
            {
                packedpuddle_t* cur = &puddlePacket.puddles[i];
                G_SpawnMapPuddle(cur->networkID, cur->gridX, cur->gridY, cur->damagesAI, cur->damagesPlayer, cur->damage, cur->duration, cur->level, cur->newFloorID, true);
            }
            
            break;
        }

        case PCKTID_HEAL_OTHER:
        {
            // Manage packet
            pckt_heal_other_t healPacket;
            memcpy(&healPacket, receivedPacket->data, sizeof(healPacket));

            printf("Packet received! ID: %d | - Values (%f)\n", receivedPacket->id, healPacket.healAmount);

            G_PlayerGainHealth(healPacket.healAmount);
            break;
        }
    }
}

void O_GameSetDoorState(int level, int dX, int dY, doorstate_e state)
{
    // Make greet packet
    pckt_t* doorPacket = PCKT_MakeDoorChangePacket(&packetToSend, level, dX, dY, (int)state);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)doorPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameSetDoorState\n");
    }
}

void O_GamePickPickup(int level, int dX, int dY)
{
    // Make greet packet
    pckt_t* pickupPacket = PCKT_MakePickupPickedPacket(&packetToSend, level, dX, dY);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)pickupPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GamePickPickup\n");
    }
}

void O_GameSpawnProjectile(int pNetworkID, int pSpriteID, float pAngle, int pLevel, float pPosX, float pPosY, float pPosZ, float pVerticalAngle, bool pIsOfPlayer, int pAiOwnerID)
{
    // Adds a projectile to the projectile list to send
    if(projectilesToSendLength < MAX_PROJECTILES_TO_SEND_SIZE)
    {
        projectilesToSend[projectilesToSendLength].networkID = pNetworkID;
        projectilesToSend[projectilesToSendLength].spriteID = pSpriteID;
        projectilesToSend[projectilesToSendLength].angle = pAngle;
        projectilesToSend[projectilesToSendLength].level = pLevel;
        projectilesToSend[projectilesToSendLength].posX = pPosX;
        projectilesToSend[projectilesToSendLength].posY = pPosY;
        projectilesToSend[projectilesToSendLength].posZ = pPosZ;
        projectilesToSend[projectilesToSendLength].verticalAngle = pVerticalAngle;
        projectilesToSend[projectilesToSendLength].isOfPlayer = pIsOfPlayer;
        projectilesToSend[projectilesToSendLength].aiOwnerID = pAiOwnerID;

        projectilesToSendLength++;
    }
    else
    {
        printf("ERROR: Cannot send any more projectiles.\n");
    }
}

void O_GameDestroyProjectile(int pNetworkID, int pSpriteID, bool pForceDestroy)
{
    pckt_t* destrProjectilePacket = PCKT_MakeProjectileDestrPacket(&packetToSend, pNetworkID, pSpriteID, pForceDestroy);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)destrProjectilePacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameDestroyProjectile\n");
    }
}

void O_GameSendAIUpdate(void)
{
    // Make greet packet
    pckt_t* aiMovementPacket = PCKT_MakeAIMovementUpdatePacket(&packetToSend);

    // Create and fill the content
    pckt_aimovementupdate_t content;

    int counter = 0;
    content.length = 0;
    for(int i = 0; i < allDynamicSpritesLength; i++)
    {
        if(allDynamicSprites[i]->hasChanged && counter < MAX_AIREPLICATIONT_PER_PACKET)
        {
            // This ai needs to be updated on the other side
            content.ais[counter].networkID = allDynamicSprites[i]->networkID;
            content.ais[counter].x = allDynamicSprites[i]->base.pos.x;
            content.ais[counter].y = allDynamicSprites[i]->base.pos.y;
            content.ais[counter].z = allDynamicSprites[i]->base.z;
            content.ais[counter].hostAggro = allDynamicSprites[i]->hostAggro;
            content.ais[counter].joinerAggro = allDynamicSprites[i]->joinerAggro;
            
            counter++;
            content.length++;
        }
    }

    if(counter > 0)
    {
        // Convert content as packet.data
        memcpy(aiMovementPacket->data, &content, sizeof(content));

        printf("AI MOVEMENT PACKET MADE! LENGTH: %d\n", counter);

        if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
        {
            // Store the packet in the output buffer
            outputPcktBuffer.hasBegunWriting = TRUE;
            memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)aiMovementPacket, PCKT_SIZE);
            outputPcktBuffer.packetsToWrite++;
        }
        else
        {
            printf("CRITICAL ERROR: Send buffer was full when in O_GameSendAIUpdate\n");
        }
        
    }
}

void O_GameAITakeDamage(int pNetworkID, float pDamage, bool pDied)
{
    pckt_t* aiPacket = PCKT_MakeAIAttackPacket(&packetToSend, pNetworkID, pDamage, pDied);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)aiPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameAITakeDamage\n");
    }
}


void O_GameAIPlayAnim(int networkID, int anim, bool loop)
{
    pckt_t* aiPacket = PCKT_MakeAIPlayAnimPacket(&packetToSend, networkID, anim, loop);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
         // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)aiPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameAIPlayAnim\n");
    }
}

void O_GameAIInstantiate(int pNetworkID, int pLevel, int pGridX, int pGridY, int pSpriteID, bool pPlayAnim, int pAnimID, bool pLoop)
{
    pckt_t* aiPacket = PCKT_MakeAIInstantiatePacket(&packetToSend, pNetworkID, pLevel, pGridX, pGridY, pSpriteID, pPlayAnim, pAnimID, pLoop);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)aiPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameAIInstantiate\n");
    }
}

void O_GameSpawnPuddles(int length, packedpuddle_t puddles[MAX_PUDDLE_OBJECTS_INSTANTIATE])
{
    pckt_t* puddlePacket = PCKT_MakePuddlesInstantiatePacket(&packetToSend, length, puddles);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)puddlePacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameSpawnPuddles\n");
    }
}

void O_GameHealOther(float amount)
{
    pckt_t* healPacket = PCKT_MakeHealOtherPacket(&packetToSend, amount);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)healPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in O_GameHealOther\n");
    }
}