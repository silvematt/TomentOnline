#include <stdio.h>
#include "netdef.h"
#include "packet.h"

netplayer_t hostPlayer;
netplayer_t otherPlayer;

// Input related
boolean wantsToAbortHosting = FALSE;
boolean wantsToAbortJoining = FALSE;

// Used to store values when joining game
static char thisPlayerName[NET_MAX_PLAYER_NAME_LENGTH] = "\0";
static char remoteAddress[1024] = "\0";
static u_short remotePort = 0;

int NET_InitializeNet()
{
    WORD wVersion = MAKEWORD(2,2);
    WSADATA wsaData;

    WSAStartup(wVersion, &wsaData);

    return 0;
}

int NET_HostGameProcedure()
{
    printf("Enter username: ");
    gets(thisPlayerName);
    fflush(stdin);

    // Player 1 is the host
    hostPlayer.id = 0;
    strcpy(hostPlayer.name, "PLAYER 1");
    
    // Create the socket to listen
    hostPlayer.socket = INVALID_SOCKET;
    hostPlayer.socket = socket(AF_INET, SOCK_STREAM, 0);
    if(hostPlayer.socket == INVALID_SOCKET)
    {
        printf("Error creating a socket while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set the listen socket to be non-blocking
    u_long iMode = 1;
    ioctlsocket(hostPlayer.socket, FIONBIO, &iMode);

    // Set address of the host
    memset(hostPlayer.address.sin_zero, 0, sizeof(hostPlayer.address.sin_zero));
    hostPlayer.address.sin_family = AF_INET;
    hostPlayer.address.sin_addr.S_un.S_addr = htons(INADDR_ANY);
    hostPlayer.address.sin_port = htons(NET_LISTENING_PORT);
    int hostPlayerAddressLen = sizeof(hostPlayer.address);

    printf("IP Address: %s:%d\n", inet_ntoa(hostPlayer.address.sin_addr), (int)ntohs(hostPlayer.address.sin_port));

    // Attempt to bind
    if(bind(hostPlayer.socket, (struct sockaddr*)&hostPlayer.address, hostPlayerAddressLen) < 0)
    {
        printf("Error binding a socket while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 2;
    }

    // Set socket to listen mode
    if(listen(hostPlayer.socket, SOMAXCONN) < 0)
    {
        printf("Error setting a socket to listen while hosting a game | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        return 3;
    }

    // Other player hasn't connected
    otherPlayer.status = NETSTS_NULL;

    printf("Hosting procedure initialized.\n");

    return 0;
}

int NET_HostGameWaitForConnection()
{
    // Accept new connections
    boolean otherPlayerConnected = FALSE;
    if(!otherPlayerConnected && !wantsToAbortHosting)
    {
        struct sockaddr_in otherAddress;
        int otherAddressLength = sizeof(otherAddress);
        SOCKET acceptedSocket = accept(hostPlayer.socket, (struct sockaddr*)&otherAddress, &otherAddressLength);

        // Wait until other player connects
        if(acceptedSocket != SOCKET_ERROR)
        {
            printf("A player connected! | Addr: %s:%d\n", inet_ntoa(otherAddress.sin_addr), (int)ntohs(otherAddress.sin_port));

            // Set other player's socket to be non blocking
            u_long iMode = 1;
            ioctlsocket(acceptedSocket, FIONBIO, &iMode);

            otherPlayer.id = 1;
            strcpy(otherPlayer.name, "PLAYER 2");

            // Copy inner struct data to otherPlayer
            otherPlayer.socket = acceptedSocket;
            otherPlayer.address = otherAddress;
            otherPlayerConnected = TRUE;
            otherPlayer.status = NETSTS_JUST_CONNECTED;
        }
    }


    if(otherPlayerConnected)
    {
        // Shutdown and close the listening socket
        shutdown(hostPlayer.socket, SD_RECEIVE);
        closesocket(hostPlayer.socket);

        printf("Other player has connected, shutting down listening socket.\n");
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

int NET_HostGameAbortConnection()
{
    printf("Listening was aborted by user.\n");

    // Shutdown the socket
    shutdown(hostPlayer.socket, SD_RECEIVE);
    closesocket(hostPlayer.socket);

    return 0;
}

int NET_HostGameWaitForGreet()
{
    // Receive greet
    char buffer[MAX_PCKT_DATA];
    int recvVal = recv(otherPlayer.socket, buffer, MAX_PCKT_DATA, 0);

    if(recvVal > 0)
    {
        pckt_t* receivedPacket = PCKT_BytesToPckt(buffer);

        if(receivedPacket->id == PCKT_GREET)
        {
            // Manage packet, if receivedPacket->id == PCKT_GREET:
            pckt_greet_t* greetPacket = PCKT_GetGreetPacket(receivedPacket);

            printf("%d Packet ID: %d | Greet value: %s\n", recvVal, receivedPacket->id, greetPacket->name);

            strcpy(otherPlayer.name, greetPacket->name);

            // Dispose greet and received packet
            free(greetPacket);

            NET_HostGameSendGreet();

            otherPlayer.status = NETSTS_GREETED;
        }

        free(receivedPacket);
    }
}

int NET_HostGameSendGreet()
{
    // Send our greet to the other player
    pckt_t* greetPacket = PCKT_MakeGreetPacket(thisPlayerName);
    char* sendBuff = PCKT_PcktToBytes(greetPacket);
    int sendVal = send(otherPlayer.socket, sendBuff, MAX_PCKT_DATA, 0);

    if(sendVal < 0)
    {
        printf("Host could not send greet packet.\n");
    }
}


int NET_JoinGameProcedure()
{
    printf("Joining a game...\n");

    otherPlayer.status = NETSTS_NULL;
    // Set basic info
    otherPlayer.id = 0;
    strcpy(otherPlayer.name, "PLAYER 1");

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
    gets(thisPlayerName);
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

int NET_JoinGameWaitForConnection()
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

int NET_JoinGameAbortConnection()
{
    printf("Joining was aborted by user.\n");

    // Shutdown the socket
    shutdown(otherPlayer.socket, SD_BOTH);
    closesocket(otherPlayer.socket);

    return 0;
}

int NET_JoinGameOnConnectionEstabilishes()
{
    printf("Connection estabilished!\n");

    otherPlayer.status = NETSTS_JUST_CONNECTED;

    // Send greet
    pckt_t* greetPacket = PCKT_MakeGreetPacket(thisPlayerName);
    char* sendBuff = PCKT_PcktToBytes(greetPacket);
    int sendVal = send(otherPlayer.socket, sendBuff, MAX_PCKT_DATA, 0);

    printf("SENDED VALUE : %d | %s\n", sendVal, sendBuff);
    return 0;
}

int NET_JoinGameWaitForGreet()
{
    // Receive greet
    char buffer[MAX_PCKT_DATA];
    int recvVal = recv(otherPlayer.socket, buffer, MAX_PCKT_DATA, 0);

    if(recvVal > 0)
    {
        pckt_t* receivedPacket = PCKT_BytesToPckt(buffer);

        if(receivedPacket->id == PCKT_GREET)
        {
            // Manage packet, if receivedPacket->id == PCKT_GREET:
            pckt_greet_t* greetPacket = PCKT_GetGreetPacket(receivedPacket);

            printf("Received packet: %d Packet ID: %d | Greet value: %s\n", recvVal, receivedPacket->id, greetPacket->name);

            strcpy(otherPlayer.name, greetPacket->name);

            // Dispose greet and received packet
            free(greetPacket);
            free(receivedPacket);

            otherPlayer.status = NETSTS_GREETED;
        }
    }
}
