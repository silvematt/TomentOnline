#include "O_Lobby.h"

#include "../Engine/A_Application.h"
#include "../Engine/G_MainMenu.h"
#include "../Engine/D_AssetsManager.h"
#include "../Engine/R_Rendering.h"
#include "../Engine/T_TextRendering.h"

#include "../Network/netdef.h"
#include "../Network/packet.h"

bool thisPlayerReady;
bool otherPlayerReady;

gamedungeons_e selectedDungeon;

// Lobby menu buttons
static void CALLBACK_ClassSelectTank(void);
static void CALLBACK_ClassSelectHealer(void);
static void CALLBACK_ClassSelectDps(void);

menuelement_t lobbyButtons[] =
{
    {"CLASS_SELECT_TANK",       {14,  336, 32, 32}, CALLBACK_ClassSelectTank},
    {"CLASS_SELECT_HEALER",     {76,  336, 32, 32}, CALLBACK_ClassSelectHealer},
    {"CLASS_SELECT_DPS",        {134, 336, 32, 32}, CALLBACK_ClassSelectDps}
};
int lobbyButtonsLength = 3;

int O_LobbyDefineClassesHostwise(void)
{
    if(thisPlayer.favoriteClass != otherPlayer.favoriteClass)
    {
        printf("LOBBYINFO: Default classes are ok\n");
        thisPlayer.selectedClass = thisPlayer.favoriteClass;
        otherPlayer.selectedClass = otherPlayer.favoriteClass;
        return 0;
    }
    else
    {
        printf("LOBBYINFO: Default classes are not ok, prioritizing host...\n");

        // Both players wants the same class, prioritize the host
        thisPlayer.selectedClass = thisPlayer.favoriteClass;

        switch(thisPlayer.favoriteClass)
        {
            case CLASS_TANK:
                otherPlayer.selectedClass = CLASS_HEALER;
                break;

            case CLASS_HEALER:
                otherPlayer.selectedClass = CLASS_TANK;
                break;

            case CLASS_DPS:
                otherPlayer.selectedClass = CLASS_TANK;
                break;
        }

        return 1;
    }
}

int O_LobbyDefineClassesJoinerwise(void)
{
    if(thisPlayer.favoriteClass != otherPlayer.favoriteClass)
    {
        printf("LOBBYINFO: Default classes are ok\n");

        thisPlayer.selectedClass = thisPlayer.favoriteClass;
        otherPlayer.selectedClass = otherPlayer.favoriteClass;
        return 0;
    }
    else
    {
        printf("LOBBYINFO: Default classes are not ok, prioritizing host...\n");

        // Both players wants the same class, prioritize the host
        otherPlayer.selectedClass = otherPlayer.favoriteClass;

        switch(otherPlayer.favoriteClass)
        {
            case CLASS_TANK:
                thisPlayer.selectedClass = CLASS_HEALER;
                break;

            case CLASS_HEALER:
                thisPlayer.selectedClass = CLASS_TANK;
                break;

            case CLASS_DPS:
                thisPlayer.selectedClass = CLASS_TANK;
                break;
        }

        return 1;
    }
}

int O_LobbySetReady(void)
{

}

int O_LobbySetMap(void)
{

}

int O_LobbySetClass(playableclasses_e selected)
{
    // Send packet to change class
    pckt_t* setClassPacket = PCKT_MakeSetClassPacket(&packetToSend, selected);

    // outputpcktbuffer was already sending something, check if we can append this packet
    if(outputPcktBuffer.packetsToWrite < MAX_PCKTS_PER_BUFFER)
    {
        // Append this packet, it will be sent after the ones already in buffer
        outputPcktBuffer.hasBegunWriting = TRUE;
        memcpy(outputPcktBuffer.buffer+(outputPcktBuffer.packetsToWrite*PCKT_SIZE), (char*)setClassPacket, PCKT_SIZE);
        outputPcktBuffer.packetsToWrite++;

        printf("Set Class Packet made!\n");

        thisPlayer.selectedClass = selected;
        return 0;
    }
    else
    {
        // Outputpcktbuffer is full and this packet should be sent, what to do?
        printf("CRITICAL ERROR: Send buffer was full when trying to send SetClassPacket\n");
        return 2;
    }
}

int O_LobbyLeave(void)
{
    
}


int O_LobbyRender(void)
{
    T_DisplayTextScaled(FONT_BLKCRY, "Lobby", 300, 30, 2.0f);

    // Display this player name
    T_DisplayTextScaled(FONT_BLKCRY, thisPlayer.name, 50, 150, 1.0f);
    
    SDL_Rect defaultSize = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT};

    // Display this player potrait
    SDL_Rect thisPotraitScreenPos = {20, 190, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK+thisPlayer.selectedClass]->texture, &thisPotraitScreenPos);

    // Display this player classess
    SDL_Rect thisTankIcon = {14, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_TANK]->texture, &thisTankIcon);

    SDL_Rect thisHealerIcon = {76, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_HEALER]->texture, &thisHealerIcon);

    SDL_Rect thisDpsIcon = {134, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_DPS]->texture, &thisDpsIcon);

    SDL_Rect selectionIcon = {14, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    if(thisPlayer.selectedClass == 0)
        selectionIcon.x = 14;
    else if(thisPlayer.selectedClass == 1)
        selectionIcon.x = 76;
    else
        selectionIcon.x = 134;

    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION]->texture, &selectionIcon);
    
    SDL_Rect disabledIcon = {134, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    if(otherPlayer.selectedClass == 0)
        disabledIcon.x = 14;
    else if(otherPlayer.selectedClass == 1)
        disabledIcon.x = 76;
    else
        disabledIcon.x = 134;
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION_DISABLED]->texture, &disabledIcon);

    // Ready text
    T_DisplayTextScaled(FONT_BLKCRY, "Ready: ", 14, 405, 1.0f);

    SDL_Rect readyIcon = {120, 405, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_ICON_NOTREADY]->texture, &readyIcon);

    // Display other player name
    T_DisplayTextScaled(FONT_BLKCRY, otherPlayer.name, 650, 150, 1.0f);

    
    // Display OTHER player potrait
    SDL_Rect otherPotraitScreenPos = {630, 190, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK+otherPlayer.selectedClass]->texture, &otherPotraitScreenPos);
    
    // Display other player class

    // Display this player classess
    SDL_Rect otherClassIcon = {680, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_TANK+otherPlayer.selectedClass]->texture, &otherClassIcon);

    SDL_Rect otherClassSelection = {680, 336, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION]->texture, &otherClassSelection);
    
    // Ready text
    T_DisplayTextScaled(FONT_BLKCRY, "Ready: ", 610, 405, 1.0f);

    SDL_Rect otherReadyIcon = {716, 405, SCREEN_WIDTH, SCREEN_HEIGHT};
    R_BlitIntoScreenScaled(&defaultSize, tomentdatapack.uiAssets[G_ASSET_ICON_NOTREADY]->texture, &otherReadyIcon);
}

void O_LobbyInputHandling(SDL_Event* e)
{
    // Offset mouse pos
    static int x = 0;
    static int y = 0;

    //Get the mouse offsets
    x = e->button.x;
    y = e->button.y;

    for(int i = 0; i < lobbyButtonsLength; i++)
    {
        bool isOn = (( x > lobbyButtons[i].box.x) && ( x < lobbyButtons[i].box.x + lobbyButtons[i].box.w ) && 
                     ( y > lobbyButtons[i].box.y) && ( y < lobbyButtons[i].box.y + lobbyButtons[i].box.h ));

        if(e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
        {
            if (isOn && lobbyButtons[i].OnClick != NULL)
            {
                lobbyButtons[i].OnClick();
                return;
            }
        }
    }
}

static void CALLBACK_ClassSelectTank(void)
{
    O_LobbySetClass(CLASS_TANK);
}

static void CALLBACK_ClassSelectHealer(void)
{
    O_LobbySetClass(CLASS_HEALER);
}

static void CALLBACK_ClassSelectDps(void)
{
    O_LobbySetClass(CLASS_DPS);
}

int O_LobbySendPackets(void)
{
    return PCKT_SendPacket(O_LobbyOnPacketIsSent);
}

int O_LobbyOnPacketIsSent(void)
{
    
}

int O_LobbyReceivePackets(void)
{
    return PCKT_ReceivePacket(O_LobbyOnPacketIsReceived);
}

int O_LobbyOnPacketIsReceived(void)
{
    // When this function gets called, the packet arrived on the PCKT_ReceivePacket call and was saved inside the inputPacketBuffer->buffer
    // At this point, receivedPacket points at the inputPacketBuffer->buffer that contains the packet that arrived
    pckt_t* receivedPacket = (pckt_t*)inputPcktBuffer.buffer;

    printf("Packet received! ID: %d\n", receivedPacket->id);
    if(receivedPacket->id == PCKT_SET_CLASS)
    {
        // Manage packet, if receivedPacket->id == PCKT_GREET:
        pckt_set_class_t* setClassPacket = (pckt_set_class_t*)receivedPacket->data;

        printf("Packet received! ID: %d | - Class: %d\n", receivedPacket->id, setClassPacket->classSet);

        // Parse packet, check if it is the same as ours?
        if(setClassPacket->classSet == thisPlayer.selectedClass)
        {
            // Class clash, may be caused by delay, if this is the host, keep the class, if this is the client change it
            printf("Class clash!\n");
            if(thisPlayer.isHost)
            {
                switch(thisPlayer.selectedClass)
                {
                    case CLASS_TANK:
                        otherPlayer.selectedClass = CLASS_HEALER;
                        break;

                    case CLASS_HEALER:
                        otherPlayer.selectedClass = CLASS_TANK;
                        break;

                    case CLASS_DPS:
                        otherPlayer.selectedClass = CLASS_TANK;
                        break;
                }
            }
            else
            {
                switch(setClassPacket->classSet)
                {
                    case CLASS_TANK:
                        thisPlayer.selectedClass = CLASS_HEALER;
                        break;

                    case CLASS_HEALER:
                        thisPlayer.selectedClass = CLASS_TANK;
                        break;
                    
                    case CLASS_DPS:
                        thisPlayer.selectedClass = CLASS_TANK;
                        break;
                }

                otherPlayer.selectedClass = setClassPacket->classSet;
            }
        }
        else
            otherPlayer.selectedClass = setClassPacket->classSet;

        return 0;
    }
}