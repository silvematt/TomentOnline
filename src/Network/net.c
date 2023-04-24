#include <stdio.h>
#include<windows.h>

#include "netdef.h"
#include "packet.h"
#include "../Online/O_Lobby.h"

netplayer_t thisPlayer;
netplayer_t otherPlayer;

// Input related
bool wantsToAbortHosting = FALSE;
bool wantsToAbortJoining = FALSE;

// Used to store values when joining game
static char remoteAddress[1024] = "\0";
static u_short remotePort = 0;

int NET_InitializeNet(void)
{
    WORD wVersion = MAKEWORD(2,2);
    WSADATA wsaData;

    WSAStartup(wVersion, &wsaData);

    PCKT_ZeroBuffer(&inputPcktBuffer);
    PCKT_ZeroBuffer(&outputPcktBuffer);

    return 0;
}

int NET_HostGameProcedure(void)
{
    printf("Enter username: ");
    gets(thisPlayer.name);
    fflush(stdin);

    // Player 1 is the host
    thisPlayer.id = 0;

    // Set who is host
    thisPlayer.isHost = true;
    otherPlayer.isHost = false;
    
    // Create the socket to listen
    thisPlayer.socket = INVALID_SOCKET;
    thisPlayer.socket = socket(AF_INET, SOCK_STREAM, 0);
    if(thisPlayer.socket == INVALID_SOCKET)
    {
        printf("Error creating a socket while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Set the listen socket to be non-blocking
    u_long iMode = 1;
    ioctlsocket(thisPlayer.socket, FIONBIO, &iMode);

    // Set address of the host
    memset(thisPlayer.address.sin_zero, 0, sizeof(thisPlayer.address.sin_zero));
    thisPlayer.address.sin_family = AF_INET;
    thisPlayer.address.sin_addr.S_un.S_addr = htons(INADDR_ANY);
    thisPlayer.address.sin_port = htons(NET_LISTENING_PORT);
    int hostPlayerAddressLen = sizeof(thisPlayer.address);

    printf("IP Address: %s:%d\n", inet_ntoa(thisPlayer.address.sin_addr), (int)ntohs(thisPlayer.address.sin_port));

    // Attempt to bind
    if(bind(thisPlayer.socket, (struct sockaddr*)&thisPlayer.address, hostPlayerAddressLen) < 0)
    {
        printf("Error binding a socket while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 2;
    }

    // Set socket to listen mode
    if(listen(thisPlayer.socket, SOMAXCONN) < 0)
    {
        printf("Error setting a socket to listen while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 3;
    }

    int flag = 1;
    int result = setsockopt(thisPlayer.socket,            /* socket affected */
                            IPPROTO_TCP,     /* set option at TCP level */
                            TCP_NODELAY,     /* name of option */
                            (char *) &flag,  /* the cast is historical cruft */
                            sizeof(int));    /* length of option value */
    if (result < 0) 
    printf("Error while disabling Nagle's Alg.\n");

    printf("Disable Nagle returned %d\n", result);

    // Other player hasn't connected
    otherPlayer.status = NETSTS_NULL;

    printf("Hosting procedure initialized.\n");

    return 0;
}

int NET_HostGameWaitForConnection(void)
{
    // Accept new connections
    bool otherPlayerConnected = FALSE;
    if(!otherPlayerConnected && !wantsToAbortHosting)
    {
        struct sockaddr_in otherAddress;
        int otherAddressLength = sizeof(otherAddress);
        SOCKET acceptedSocket = accept(thisPlayer.socket, (struct sockaddr*)&otherAddress, &otherAddressLength);

        // Wait until other player connects
        if(acceptedSocket != SOCKET_ERROR)
        {
            printf("A player connected! | Addr: %s:%d\n", inet_ntoa(otherAddress.sin_addr), (int)ntohs(otherAddress.sin_port));

            // Set other player's socket to be non blocking
            u_long iMode = 1;
            ioctlsocket(acceptedSocket, FIONBIO, &iMode);
            int flag = 1;
            int result = setsockopt(acceptedSocket,            /* socket affected */
                                    IPPROTO_TCP,     /* set option at TCP level */
                                    TCP_NODELAY,     /* name of option */
                                    (char *) &flag,  /* the cast is historical cruft */
                                    sizeof(int));    /* length of option value */
            if (result < 0) 
                printf("Error while disabling Nagle's Alg.\n");

            printf("Disable Nagle returned %d\n", result);


            otherPlayer.id = 1;

            // Copy inner struct data to otherPlayer
            otherPlayer.socket = acceptedSocket;
            otherPlayer.address = otherAddress;
            otherPlayerConnected = TRUE;
            otherPlayer.status = NETSTS_JUST_CONNECTED;
        }
    }


    if(otherPlayerConnected)
    {
        // Close the listening socket
        closesocket(thisPlayer.socket);

        printf("Other player has connected, closing listening socket.\n");
        return 0;
    }
    else if(wantsToAbortHosting)
    {
        NET_HostGameAbortConnection();
        return 2;
    }
    else
        return 1;
}

int NET_HostGameAbortConnection(void)
{
    printf("Listening was aborted by user.\n");

    // Close the socket
    closesocket(thisPlayer.socket);

    return 0;
}

int NET_HostGameWaitForGreet(void)
{
    printf("HOST WAIT FOR GREET\n");
    
    // When this function gets called, the packet arrived on the PCKT_ReceivePacket call and was saved inside the inputPacketBuffer->buffer
    // At this point, receivedPacket points at the inputPacketBuffer->buffer that contains the packet that arrived
    pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;

    if(receivedPacket->id == PCKTID_GREET)
    {
        // Manage packet, if receivedPacket->id == PCKT_GREET:
        pckt_greet_t greetPacket;
        memcpy(&greetPacket, receivedPacket->data, sizeof(greetPacket));

        printf("Packet received! ID: %d | Greet value: %s | Class: %d\n", receivedPacket->id, greetPacket.name, greetPacket.favoriteClass);

        // Parse packet
        strcpy(otherPlayer.name, greetPacket.name);
        otherPlayer.favoriteClass = greetPacket.favoriteClass;

        otherPlayer.status = NETSTS_HAVE_TO_GREET;
        return 0;
    }
}

int NET_HostGameMakeGreetPacket(void)
{
    // Make greet packet
    pckt_t* greetPacket = PCKT_MakeGreetPacket(&packetToSend, thisPlayer.name, thisPlayer.favoriteClass);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the greet packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)greetPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
        printf("GREET PACKET MADE!\n");
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in NET_HostGameMakeGreetPacket\n");
    }
}

int NET_HostGameSendGreet(void)
{
    // When this function gets called, the packet got sent on the PCKT_SendPacket call and was saved inside the output->buffer
    // At this point, receivedPacket points at the output->buffer that contains the packet that arrived
    pckt_t* sent = (pckt_t*)outputPcktBuffer.buffer;

    printf("Host sent greet %s\n", sent);

    O_LobbyDefineClassesHostwise();

    otherPlayer.status = NETSTS_GREETED;
}


int NET_JoinGameProcedure(void)
{
    printf("Joining a game...\n");
    
    otherPlayer.status = NETSTS_NULL;
    
    // Set basic info
    otherPlayer.id = 0;

    // Set who is host
    thisPlayer.isHost = false;
    otherPlayer.isHost = true;

    // Create socket
    otherPlayer.socket = INVALID_SOCKET;
    otherPlayer.socket = socket(AF_INET, SOCK_STREAM, 0);
    if(otherPlayer.socket == INVALID_SOCKET)
    {
        printf("Error creating a socket while joining a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set other player's socket to be non blocking
    u_long iMode = 1;
    ioctlsocket(otherPlayer.socket, FIONBIO, &iMode);

    
    // Setup address
    memset(otherPlayer.address.sin_zero, 0, sizeof(otherPlayer.address.sin_zero));
    otherPlayer.address.sin_family = AF_INET;
    otherPlayer.address.sin_addr.S_un.S_addr = INADDR_ANY;
    otherPlayer.address.sin_port = htons(0);
    
    // Attempt to bind
    if(bind(otherPlayer.socket, (struct sockaddr*)&otherPlayer.address, sizeof(otherPlayer.address)) < 0)
    {
        printf("Error binding a socket while joining a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 2;
    }

    // Get server address and port
    printf("Enter address in 192.168.1.1 format...\n");
    gets(remoteAddress);
    fflush(stdin);

    printf("Enter port number\n");
    scanf("%hu", &remotePort);
    fflush(stdin);

    printf("Enter username: ");
    gets(thisPlayer.name);
    fflush(stdin);

    // Setup host
    struct sockaddr_in hostAddress;
    int hostAddressLen = sizeof(hostAddress);
    memset(hostAddress.sin_zero, 0, sizeof(hostAddress.sin_zero));

    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.S_un.S_addr = inet_addr(remoteAddress);
    hostAddress.sin_port = htons(remotePort);

    printf("Inserted IP Address: %s:%d\n", inet_ntoa(hostAddress.sin_addr), (int)ntohs(hostAddress.sin_port));
    printf("Attempting to connect...\n");

    // Attempt to connect
    int conn = connect(otherPlayer.socket, (struct sockaddr*)&hostAddress, hostAddressLen);
    printf("CONN VALUE: %d\n", conn);
    // If connection was instant
    if(conn == 0)
    {
        NET_JoinGameOnConnectionEstabilishes();
        return 0;
    }
    // If we need to wait for connection
    else if(WSAGetLastError() == WSAEWOULDBLOCK)
    {
        return 0;
    }
    else
    {
        printf("Connection failed.\n");
        return 3;
    }
}

int NET_JoinGameWaitForConnection(void)
{
    printf("Waiting for connection...\n");

    // If connection is already estabilished
    if(otherPlayer.status == NETSTS_JUST_CONNECTED)
    {
        return 3;
    }
    else
    {
        // Wait for it
        fd_set connectionSet;
        FD_ZERO(&connectionSet);
        
        FD_SET(otherPlayer.socket, &connectionSet);

        struct timeval waitVal;
        waitVal.tv_sec = 0;

        int result = select(0, NULL, &connectionSet, NULL, &waitVal);

        // If process ended
        if(result > 0)
        {
            if(FD_ISSET(otherPlayer.socket, &connectionSet))
            {
                // Connection happened, find out what happened
                struct sockaddr_in connectedAddr;
                int connectedAddrLen = sizeof(connectedAddr);
                memset(&connectedAddr, 0, sizeof(connectedAddr));
                if (getpeername(otherPlayer.socket, (struct sockaddr *)&connectedAddr, &connectedAddrLen) == 0)
                {
                    printf("TCP Connection succeeded!\n");
                    NET_JoinGameOnConnectionEstabilishes();
                    return 0;
                }
                else
                {
                    printf("TCP Connection failed!\n");
                    return 4;
                }
            }
        }
        else if(wantsToAbortJoining)
        {
            NET_JoinGameAbortConnection();
            return 2;
        }

        return 1;
    }
}

int NET_JoinGameAbortConnection(void)
{
    printf("Joining was aborted by user.\n");

    // Close the socket
    closesocket(otherPlayer.socket);

    return 0;
}

int NET_JoinGameOnConnectionEstabilishes(void)
{
    printf("Connection estabilished!\n");

    int flag = 1;
    int result = setsockopt(otherPlayer.socket,            /* socket affected */
                            IPPROTO_TCP,     /* set option at TCP level */
                            TCP_NODELAY,     /* name of option */
                            (char *) &flag,  /* the cast is historical cruft */
                            sizeof(int));    /* length of option value */
    if (result < 0) 
        printf("Error while disabling Nagle's Alg.\n");

    printf("Disable Nagle returned %d\n", result);

    otherPlayer.status = NETSTS_JUST_CONNECTED;

    return 0;
}

int NET_JoinGameWaitForGreet(void)
{
    // When this function gets called, the packet arrived on the PCKT_ReceivePacket call and was saved inside the inputPacketBuffer->buffer
    // At this point, receivedPacket points at the inputPacketBuffer->buffer that contains the packet that arrived
    pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;

    if(receivedPacket->id == PCKTID_GREET)
    {
        // Manage packet, if receivedPacket->id == PCKT_GREET:
        pckt_greet_t greetPacket;
        memcpy(&greetPacket, receivedPacket->data, sizeof(greetPacket));

        printf("Packet received! ID: %d | Greet value: %s - Class: %d\n", receivedPacket->id, greetPacket.name, greetPacket.favoriteClass);
        
        // Parse packet
        strcpy(otherPlayer.name, greetPacket.name);
        otherPlayer.favoriteClass = greetPacket.favoriteClass;

        O_LobbyDefineClassesJoinerwise();

        otherPlayer.status = NETSTS_GREETED;

        return 0;
    }
}

int NET_JoinGameMakeGreetPacket(void)
{
    // Make greet packet
    pckt_t* greetPacket = PCKT_MakeGreetPacket(&packetToSend, thisPlayer.name, thisPlayer.favoriteClass);
    
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Store the greet packet in the output buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)greetPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;
        printf("GREET PACKET MADE!\n");
    }
    else
    {
        printf("CRITICAL ERROR: Send buffer was full when in NET_JoinGameMakeGreetPacket\n");
    }
}

int NET_JoinGameSendGreet(void)
{
    // When this function gets called, the packet got sent on the PCKT_SendPacket call and was saved inside the output->buffer
    // At this point, receivedPacket points at the output->buffer that contains the packet that arrived
    pckt_t* sent = (pckt_t*)outputPcktBuffer.buffer;

    printf("Join sent greet %s\n", sent);
    otherPlayer.status = NETSTS_HAVE_TO_GREET;
}