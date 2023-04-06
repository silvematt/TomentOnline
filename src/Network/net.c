#include <stdio.h>
#include "netdef.h"
#include "packet.h"

netplayer_t hostPlayer;
netplayer_t otherPlayer;

int NET_InitializeNet()
{
    WORD wVersion = MAKEWORD(2,2);
    WSADATA wsaData;

    WSAStartup(wVersion, &wsaData);
}

int NET_HostGameProcedure()
{
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

    printf("Waiting for connection...\n");

    // Accept new connections
    boolean otherPlayerConnected = FALSE;
    boolean wishToCancel = FALSE;
    while(!otherPlayerConnected && !wishToCancel)
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

            // Send greet?
        }

        // Else check if user wishes to cancel
    }

    if(wishToCancel)
    {
        printf("Listening was aborted by user.\n");

        // Shutdown the socket
        shutdown(hostPlayer.socket, SD_RECEIVE);
        closesocket(hostPlayer.socket);

        return 4;
    }
    else if(otherPlayerConnected)
    {
        // Shutdown and close the listening socket
        shutdown(hostPlayer.socket, SD_RECEIVE);
        closesocket(hostPlayer.socket);

        printf("All done.\n");

        // Receive greet (for testing purposes, may not receive because socket is non-blocking, set to blocking and it will receive the other player's greet)
        char buffer[MAX_PCKT_DATA];
        int recvVal = recv(otherPlayer.socket, buffer, MAX_PCKT_DATA, 0);
        pckt_t* receivedPacket = PCKT_BytesToPckt(buffer);

        // Manage packet, if receivedPacket->id == PCKT_GREET:
        pckt_greet_t* greetPacket = PCKT_GetGreetPacket(receivedPacket);

        printf("%d Packet ID: %d | Greet value: %s\n", recvVal, receivedPacket->id, greetPacket->name);

        return 0;
    }
    else
    {
        printf("Error 5?\n");
        return 5;
    }
}


int NET_JoinGameProcedure()
{
    printf("Joining a game...\n");

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
    char readAddress[1024] = "\0";
    printf("Enter address in 192.168.1.1 format...\n");
    gets(readAddress);
    fflush(stdin);

    u_short readPort;
    printf("Enter port number\n");
    scanf("%hu", &readPort);
    fflush(stdin);

    char username[NET_MAX_PLAYER_NAME_LENGTH];
    printf("Enter username: ");
    gets(username);
    fflush(stdin);

    
    // Setup host
    struct sockaddr_in hostAddress;
    int hostAddressLen = sizeof(hostAddress);
    memset(hostAddress.sin_zero, 0, sizeof(hostAddress.sin_zero));

    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.S_un.S_addr = inet_addr(readAddress);
    hostAddress.sin_port = htons(readPort);

    printf("Inserted IP Address: %s:%d\n", inet_ntoa(hostAddress.sin_addr), (int)ntohs(hostAddress.sin_port));
    printf("Attempting to connect...\n");

    // Attempt to connect
    int conn = connect(otherPlayer.socket, (struct sockaddr*)&hostAddress, hostAddressLen);

    if(conn == 0)
    {
        // Set the socket to be non-blocking
        u_long iMode = 1;
        ioctlsocket(otherPlayer.socket, FIONBIO, &iMode);

        printf("Connection successfull!\n");

        otherPlayer.status = NETSTS_JUST_CONNECTED;

        // Send greet
        // Make packet
        pckt_t* greetPacket = PCKT_MakeGreetPacket(username);
        char* sendBuff = PCKT_PcktToBytes(greetPacket);
        int sendVal = send(otherPlayer.socket, sendBuff, MAX_PCKT_DATA, 0);

        printf("SENDED VALUE : %d | %s\n", sendVal, sendBuff);
        return 0;
    }
    else
    {
        printf("Error while connecting to remote host. | WSAError: %d\n", WSAGetLastError());
        WSACleanup();
        fflush(stdin);
        return 3;
    }
}