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

int O_LobbySetClass(void)
{

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
