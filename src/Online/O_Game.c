#include <math.h>

#include "O_Game.h"
#include "../Engine/P_Physics.h"
#include "../Engine/G_Player.h"
#include "../Engine/G_Pathfinding.h"


dynamicSprite_t otherPlayerObject;

// Initializes the other player's sprite rapresentation
int O_GameInitializeOtherPlayer()
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

    otherPlayerObject.base.spriteID = DS_Skeleton;
    otherPlayerObject.base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[DS_Skeleton];

    otherPlayerObject.type = DS_TYPE_AI;
    
    otherPlayerObject.base.active = true;
    otherPlayerObject.base.level = 0;
    otherPlayerObject.base.level = SDL_clamp(otherPlayerObject.base.level, 0, MAX_N_LEVELS-1);

    otherPlayerObject.base.z = TILE_SIZE * (0);
    otherPlayerObject.verticalMovementDelta = 0.0f;

    otherPlayerObject.canBeHit = false;

    otherPlayerObject.base.collisionCircle.pos.x = otherPlayerObject.base.pos.x;
    otherPlayerObject.base.collisionCircle.pos.y = otherPlayerObject.base.pos.y;
    otherPlayerObject.base.collisionCircle.r = 32;

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

    // Add other player to the dynamic map
    currentMap.dynamicSpritesLevel0[otherPlayerObject.base.gridPos.y][otherPlayerObject.base.gridPos.x] = &otherPlayerObject;

    printf("Other player gird %d | %d\n", otherPlayerObject.base.gridPos.x, otherPlayerObject.base.gridPos.y);
}

int O_GameOtherPlayerLoop()
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
    otherPlayerObject.base.angle = ((atan2(-otherPlayerObject.base.pSpacePos.y, otherPlayerObject.base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(otherPlayerObject.base.angle);

    float deltaX = 0.0f;
    float deltaY = 0.0f;

    // Check if it changed grid pos
    if(!(oldGridPosX == otherPlayerObject.base.gridPos.x && oldGridPosY == otherPlayerObject.base.gridPos.y))
    {
        // If the tile the AI ended up in is not occupied

        if(G_CheckDynamicSpriteMap(otherPlayerObject.base.level, otherPlayerObject.base.gridPos.y, otherPlayerObject.base.gridPos.x) == false)
        {
            // Update the dynamic map
            switch(otherPlayerObject.base.level)
            {
                case 0:
                    currentMap.dynamicSpritesLevel0[otherPlayerObject.base.gridPos.y][otherPlayerObject.base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                    currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                    break;
                
                case 1:
                    currentMap.dynamicSpritesLevel1[otherPlayerObject.base.gridPos.y][otherPlayerObject.base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                    currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                    break;

                case 2:
                    currentMap.dynamicSpritesLevel2[otherPlayerObject.base.gridPos.y][otherPlayerObject.base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                    currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                    break;

                default:
                    break;
            }
        }
        else
        {
            // Move back
            otherPlayerObject.base.pos.x -= (deltaX * otherPlayerObject.speed) * deltaTime;
            otherPlayerObject.base.pos.y -= (deltaY * otherPlayerObject.speed) * deltaTime; 

            otherPlayerObject.base.gridPos.x = oldGridPosX;
            otherPlayerObject.base.gridPos.y = oldGridPosY;

            otherPlayerObject.state = DS_STATE_MOVING;
        }
    }
    
    // Update collision circle
    otherPlayerObject.base.collisionCircle.pos.x = otherPlayerObject.base.centeredPos.x;
    otherPlayerObject.base.collisionCircle.pos.y = otherPlayerObject.base.centeredPos.y;
}

int O_GameReceivePackets()
{
    return PCKT_ReceivePacket(O_GameOnPacketIsReceived);
}

static float lastSentX, lastSentY;
int O_GameSendPackets()
{
    if(lastSentX != player.position.x || lastSentY != player.position.y)
    {
        // Make movement packet and send it to the other player
        // Send packet to notice other player that we are starting the game, he will start as soon as this packet is received
        pckt_t* movementPacket = PCKT_MakeMovementPacket(&packetToSend, player.position.x, player.position.y);

        // outputpcktbuffer was already sending something, check if we can append this packet
        if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
        {
            // Append this packet, it will be sent after the ones already in buffer
            outputPcktBuffer.hasBegunWriting = TRUE;
            memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)movementPacket, PCKT_SIZE);
            outputPcktBuffer.packetsToWrite++;

            lastSentX = player.position.x;
            lastSentY = player.position.y;


            printf("Movement packet made!\n");
        }
        else
        {
            // Outputpcktbuffer is full and this packet should be sent, what to do?
            printf("CRITICAL ERROR: Send buffer was full when in O_GameSendPackets\n");
        }
    }

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
    memcpy(thisPcktBuffer, inputPcktBuffer.buffer, PCKT_SIZE);

    pckt_t* receivedPacket = (pckt_t*)thisPcktBuffer;

    printf("Packet received! ID: %d\n", receivedPacket->id);

    switch(receivedPacket->id)
    {
        case PCKTID_MOVEMENT:
        {
            // Manage packet, if receivedPacket->id == PCKT_GREET:
            pckt_movement_t movementPacket;
            memcpy(&movementPacket, receivedPacket->data, sizeof(movementPacket));

            printf("Packet received! ID: %d | - Values (%f,%f)\n", receivedPacket->id, movementPacket.x, movementPacket.y);

            // Update other player position
            otherPlayerObject.base.pos.x = movementPacket.x;
            otherPlayerObject.base.pos.y = movementPacket.y;

            break;
        }
    }
}