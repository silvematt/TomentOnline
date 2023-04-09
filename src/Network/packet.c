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
}

int PCKT_ReceivePacket(int (*OnPacketArrives)(void))
{
    int recvVal = 0;

    // If we are not waiting for a fragment
    if(!inputPcktBuffer.shorted)
    {
        // Receive normally
        recvVal = recv(otherPlayer.socket, inputPcktBuffer.buffer, PCKT_SIZE, 0);

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

        printf("%d Received!\n", recvVal);

        // If received something
        if(recvVal > 0)
        {
            // Check if the receive value is a packet worth of data
            if(recvVal >= PCKT_SIZE)
            {
                //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
                //OnPacketArrives must do the conversion, it may use the inputBuffer for its all purpose or allocate a pckt_t and release the inputBuffer for future use  
                OnPacketArrives();

                // Reset the input buffer when the packet arrived
                PCKT_ZeroBuffer(&inputPcktBuffer);
                return 0;
            }
            else
            {
                // Short received, save what we got so far
                inputPcktBuffer.shorted = TRUE;
                inputPcktBuffer.len = recvVal;
            }
        }
    }
    else
    {
        // We are waiting for a fragment
        int avail = PCKT_SIZE - inputPcktBuffer.len;

        recvVal = recv(otherPlayer.socket, inputPcktBuffer.buffer+(inputPcktBuffer.len), avail, 0);
        
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

        printf("Fragment Received! Size: %d\n", recvVal);

        // If we got just what we were waiting for
        if(recvVal == avail)
        {
            inputPcktBuffer.shorted = FALSE;
            inputPcktBuffer.len += recvVal;

            // Reconstruct the packet
            if(inputPcktBuffer.len == PCKT_SIZE)
            {
                //pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;
                //OnPacketArrives must do the conversion, it may use the inputBuffer for its all purpose or allocate a pckt_t and release the inputBuffer for future use                  
                OnPacketArrives();

                // Reset the input buffer when the packet arrived
                PCKT_ZeroBuffer(&inputPcktBuffer);
                return 0;
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
            inputPcktBuffer.shorted = TRUE;
            inputPcktBuffer.len += recvVal;

            return 1;
        }
    }
}

int PCKT_SendPacket(int (*OnPacketIsSent)(void))
{
    if(outputPcktBuffer.hasBegunWriting == FALSE)
    {
        printf("There is nothing to write.\n");
        return 1;
    }

    int sendVal = 0;
    if(!outputPcktBuffer.shorted)
    {
        sendVal = send(otherPlayer.socket, outputPcktBuffer.buffer, PCKT_SIZE, 0);

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

                // Reset the outuput buffer when the packet arrived
                PCKT_ZeroBuffer(&outputPcktBuffer);
                return 0;
            }
            else
            {
                // Short received, save what we got so far
                outputPcktBuffer.shorted = TRUE;
                outputPcktBuffer.len = sendVal;
            }
        }
    }
    else
    {
        // We couldn't send the whole thing, we have to send another fragment
        int avail = PCKT_SIZE - outputPcktBuffer.len;

        int sendValue = send(otherPlayer.socket, outputPcktBuffer.buffer+outputPcktBuffer.len, avail, 0);

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

                // Reset the outuput buffer when the packet arrived
                PCKT_ZeroBuffer(&outputPcktBuffer);
                return 0;
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
            outputPcktBuffer.shorted = TRUE;
            outputPcktBuffer.len += sendVal;

            return 1;
        }
    }
}

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH])
{
    PCKT_Zero(packet);
    
    // Create the packet
    packet->protocol = PROT_ID_TCP;
    packet->id = PCKT_GREET;

    // Create and fill the content
    pckt_greet_t content;
    strcpy(content.name, pName);

    // Convert content as packet.data
    memcpy(packet->data, &content, sizeof(pckt_greet_t));

    return packet;
}
