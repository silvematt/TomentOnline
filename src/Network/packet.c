#include <stdio.h>
#include "packet.h"

// Global packet data to send out
pckt_t packetToSend;

// Input buffer when receiving packets
pckt_buffer_t inputPcktBuffer;

// Output buffer when sending packets
pckt_buffer_t outputPcktBuffer;

void PCKT_Zero(pckt_t* p)
{
    p->id = 0;
    memset(p->data, 0, PCKT_SIZE);
}

void PCKT_ZeroBuffer(pckt_buffer_t* pBuf)
{
    memset(pBuf->buffer, 0, PCKT_SIZE);
    pBuf->len = 0;
    pBuf->shorted = FALSE;
    pBuf->hasBegunWriting = FALSE;
    pBuf->packetsToWrite = 0;
    pBuf->packetOffset = 0;
}

int PCKT_ReceivePacket(int (*OnPacketArrives)(void))
{
    int recvVal = 0;
    recvVal = recv(otherPlayer.socket, inputPcktBuffer.buffer+inputPcktBuffer.len, (PCKT_SIZE*MAX_PCKTS_PER_BUFFER)-inputPcktBuffer.len, 0);

    // If invalid
    if(recvVal < 0)
    {
        if(WSAGetLastError() != WSAEWOULDBLOCK)
        {
            printf("Receive Error. RecvVal = %d | WSAError: %d\n", recvVal, WSAGetLastError());
            return 2;
        }
        else
            return 1;
    }

    printf("Receive val: %d\n", recvVal);

    bool allConsumed = false;
    while(!allConsumed)
    {
        if(inputPcktBuffer.shorted)
        {
            // We are waiting for a fragment
            int avail = PCKT_SIZE - inputPcktBuffer.len;
            printf("Fragment Received! Size: %d\n", recvVal);

            // If we got just what we were waiting for
            if(recvVal >= avail)
            {
                inputPcktBuffer.len += avail;

                // Reconstruct the packet
                if(inputPcktBuffer.len == PCKT_SIZE)
                {
                    //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
                    //OnPacketArrives must do the conversion, it may use the inputBuffer for its all purpose or allocate a pckt_t and release the inputBuffer for future use                  
                    printf("Packet recostructed!\n");
                    OnPacketArrives();

                    inputPcktBuffer.packetOffset += PCKT_SIZE;
                    inputPcktBuffer.len = 0;
                    inputPcktBuffer.shorted = FALSE;
                    recvVal -= avail;

                }
                else
                {
                    // How?
                    printf("Critical error. CODE-01\n");
                    return 2;
                }
            }
            else // antoher fragment
            {            
                // Short received, save what we got so far
                printf("Double shorted Val: %d\n", recvVal);
                inputPcktBuffer.shorted = TRUE;
                inputPcktBuffer.len += recvVal;
            }
        }
        else if(recvVal >= PCKT_SIZE)
        {
            //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
            //OnPacketArrives must do the conversion, it may use the inputBuffer for its all purpose or allocate a pckt_t and release the inputBuffer for future use  
            OnPacketArrives();
            inputPcktBuffer.packetOffset += PCKT_SIZE;
            recvVal -= PCKT_SIZE;
        }
        else
        {
            if(recvVal == 0)
            {
                allConsumed = true;
                PCKT_ZeroBuffer(&inputPcktBuffer);
            }
            else
            {
                // Short receive
                printf("Short received value of %d\n", recvVal);
                inputPcktBuffer.shorted = true;
                inputPcktBuffer.len = recvVal;
                allConsumed = true;

                memmove(inputPcktBuffer.buffer, inputPcktBuffer.buffer+inputPcktBuffer.packetOffset, recvVal);
                inputPcktBuffer.packetOffset = 0;
            }
        }
    }
}

int PCKT_SendPacket(int (*OnPacketIsSent)(void))
{
    if(!outputPcktBuffer.hasBegunWriting)
    {
        //printf("There is nothing to write.\n");
        return 1;
    }

    while(outputPcktBuffer.packetsToWrite > 0)
    {
        int sendVal = 0;
        if(!outputPcktBuffer.shorted)
        {
            sendVal = send(otherPlayer.socket, outputPcktBuffer.buffer+(outputPcktBuffer.packetOffset), PCKT_SIZE, 0);
            
            // If invalid
            if(sendVal < 0)
            {
                if(WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    printf("Send Error. SendVal = %d | WSAError: %d\n", sendVal, WSAGetLastError());
                    return 2;
                }
                else
                    return 1;
            }

            printf("%d Sent!\n", sendVal);

            if(sendVal > 0)
            {
                if(sendVal >= PCKT_SIZE)
                {
                    //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
                    //OnPacketIsSent must do the conversion, it may use the outputBuffer for its all purpose or allocate a pckt_t and release the outputBuffer for future use  
                    OnPacketIsSent();

                    // Check if there are other packets queued
                    if(outputPcktBuffer.packetsToWrite > 1)
                    {
                        // Don't zero the buffer, increment the offset and discard if previous information was shorted
                        outputPcktBuffer.packetOffset += PCKT_SIZE;
                        outputPcktBuffer.packetsToWrite--;
                        outputPcktBuffer.shorted = FALSE;
                        outputPcktBuffer.len = 0;

                        // Move the remaining packets at the beginning of the buffer so that next packets are appended
                        memmove(outputPcktBuffer.buffer, outputPcktBuffer.buffer+outputPcktBuffer.packetOffset, sizeof(outputPcktBuffer.buffer)-outputPcktBuffer.packetOffset);
                        outputPcktBuffer.packetOffset -= PCKT_SIZE;
                    }
                    else
                    {
                        // Reset the outuput buffer, it did all it had to do
                        PCKT_ZeroBuffer(&outputPcktBuffer);
                    }
                }
                else
                {
                    // Short send, save what we sent so far
                    outputPcktBuffer.shorted = TRUE;
                    outputPcktBuffer.len = sendVal;
                }
            }
        }
        else
        {
            // We couldn't send the whole thing, we have to send another fragment
            int avail = PCKT_SIZE - outputPcktBuffer.len;

            int sendValue = send(otherPlayer.socket, outputPcktBuffer.buffer+(outputPcktBuffer.packetOffset)+outputPcktBuffer.len, avail, 0);

            // If invalid
            if(sendValue < 0)
            {
                if(WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    printf("SendFrag Error. SendVal = %d | WSAError: %d\n", sendVal, WSAGetLastError());
                    return 2;
                }
                else
                    return 1;
            }

            printf("Fragment Sent! Size: %d\n", sendVal);

            if(sendValue == avail)
            {
                // We are done sending everyting

                outputPcktBuffer.shorted = FALSE;
                outputPcktBuffer.len += sendVal;

                if(outputPcktBuffer.len == PCKT_SIZE)
                {
                    //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
                    //OnPacketIsSent must do the conversion, it may use the outputBuffer for its all purpose or allocate a pckt_t and release the outputBuffer for future use  
                    OnPacketIsSent();

                    // Check if there are other packets queued
                    if(outputPcktBuffer.packetsToWrite > 1)
                    {
                        // Don't zero the buffer, increment the offset
                        outputPcktBuffer.packetOffset += PCKT_SIZE;
                        outputPcktBuffer.packetsToWrite--;
                        outputPcktBuffer.shorted = FALSE;
                        outputPcktBuffer.len = 0;

                        // Move the remaining packets at the beginning of the buffer so that next packets are appended
                        memmove(outputPcktBuffer.buffer, outputPcktBuffer.buffer+outputPcktBuffer.packetOffset, sizeof(outputPcktBuffer.buffer)-outputPcktBuffer.packetOffset);
                        outputPcktBuffer.packetOffset -= PCKT_SIZE;
                    }
                    else
                    {
                        // Reset the outuput buffer, it did all it had to do
                        PCKT_ZeroBuffer(&outputPcktBuffer);
                    }
                }
                else
                {
                    // How?
                    printf("Critical error. CODE-02\n");
                    return 2;
                }
            }
            else
            {
                // We need to send yet another fragment
                // Short sent, save what we got so far
                printf("DOUBLE FRAGMENTED\n\n");
                outputPcktBuffer.shorted = TRUE;
                outputPcktBuffer.len += sendVal;
            }
        }
    }
}

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH], byte pFavClass)
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_GREET;

    // Create and fill the content
    pckt_greet_t content;
    strcpy(content.name, pName);
    content.favoriteClass = pFavClass;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeSetClassPacket(pckt_t* packet, byte pClassToSet)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_SET_CLASS;

    // Create and fill the content
    pckt_set_class_t content;
    content.classSet = pClassToSet;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeReadyPacket(pckt_t* packet, byte pIsReady)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_READY;

    // Create and fill the content
    pckt_ready_t content;
    content.isReady = pIsReady;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeStartingPacket(pckt_t* packet, byte pStartingValue)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_STARTING;

    // Create and fill the content
    pckt_starting_t content;
    content.starting = pStartingValue;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}


pckt_t* PCKT_MakePlayerUpdatePacket(pckt_t* packet, float pX, float pY, float pZ, float pAngle, float pCurHealth, float pMaxHealth, float pCurMana, float pMaxMana, int pCurWeapon, int pCurSpell)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_PLAYERUPDATE;

    // Create and fill the content
    pckt_playerupdate_t content;
    content.x = pX;
    content.y = pY;
    content.z = pZ;
    content.angle = pAngle;
    content.curHealth = pCurHealth;
    content.maxHealth = pMaxHealth;
    content.curMana = pCurMana;
    content.maxMana = pMaxMana;
    content.curWeapon = pCurWeapon;
    content.curSpell = pCurSpell;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeDoorChangePacket(pckt_t* packet, int pLevel, int pX, int pY, int pState)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_DOOR_CHANGE;

    // Create and fill the content
    pckt_door_change_t content;
    content.level = pLevel;
    content.x = pX;
    content.y = pY;
    content.state = pState;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakePickupPickedPacket(pckt_t* packet, int pLevel, int pX, int pY)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_PICKUP_PICKED;

    // Create and fill the content
    pckt_pickup_picked_t content;
    content.level = pLevel;
    content.x = pX;
    content.y = pY;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeProjectileSpawnPacket(pckt_t* packet, int pNetworkID, int pSpriteID, float pAngle, int pLevel, float pPosX, float pPosY, float pPosZ, float pVerticalAngle, bool pIsOfPlayer, int pAiOwnerID)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_PROJECTILE_SPAWN;

    // Create and fill the content
    pckt_projectile_spawn_t content;
    content.networkID = pNetworkID;
    content.spriteID = pSpriteID;
    content.angle = pAngle;
    content.level = pLevel;
    content.posX = pPosX;
    content.posY = pPosY;
    content.posZ = pPosZ;
    content.verticalAngle = pVerticalAngle;
    content.isOfPlayer = pIsOfPlayer;
    content.aiOwnerID = pAiOwnerID;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeProjectileDestrPacket(pckt_t* packet, int pNetworkID, int pSpriteID, bool pForceDestroy)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_PROJECTILE_DESTR;

    // Create and fill the content
    pckt_projectile_destr_t content;
    content.networkID = pNetworkID;
    content.spriteID = pSpriteID;
    content.forceDestroy = pForceDestroy;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeAIMovementUpdatePacket(pckt_t* packet)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_AI_MOVEMENTS;
    
    // Content and fill is done where this function is called
    // Create and fill the content
    pckt_aimovementupdate_t content;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeAIAttackPacket(pckt_t* packet, int pNetworkID, float pDamage, bool pDied)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_AI_ATTACKED;

    // Create and fill the content
    pckt_aiattacked_t content;
    content.networkID = pNetworkID;
    content.damage = pDamage;
    content.died = pDied;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeAIPlayAnimPacket(pckt_t* packet, int pNetworkID, int pAnimID, bool pLoop)
{
    PCKT_Zero(packet);

    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_AI_PLAY_ANIM;

    // Create and fill the content
    pckt_aiplayanim_t content;
    content.networkID = pNetworkID;
    content.anim = pAnimID;
    content.loop = pLoop;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeAIInstantiatePacket(pckt_t* packet, int pNetworkID, int pLevel, int pGridX, int pGridY, int pSpriteID, bool pPlayAnim, int pAnimID, bool pLoop)
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_AI_INSTANTIATE;

    // Create and fill the content
    pckt_aiinstantiate_t content;
    content.networkID = pNetworkID;
    content.level = pLevel;
    content.gridX = pGridX;
    content.gridY = pGridY;
    content.spriteID = pSpriteID;
    content.playAnimation = pPlayAnim;
    content.animID = pAnimID;
    content.loop = pLoop;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakePuddlesInstantiatePacket(pckt_t* packet, int pLength, packedpuddle_t pPuddles[MAX_PUDDLE_OBJECTS_INSTANTIATE])
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_PUDDLES_INSTANTIATE;

    // Create and fill the content
    pckt_puddle_instantiate_t content;
    content.length = pLength;

    for(int i = 0; i < pLength; i++)
    {
        content.puddles[i] = pPuddles[i];
    }

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}

pckt_t* PCKT_MakeHealOtherPacket(pckt_t* packet, float pAmount)
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKTID_HEAL_OTHER;

    // Create and fill the content
    pckt_heal_other_t content;
    content.healAmount = pAmount;

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(content));

    return packet;
}
