#include "G_MainMenu.h"
#include "G_Game.h"
#include "R_Rendering.h"
#include "T_TextRendering.h"
#include "D_AssetsManager.h"

#include "../Network/netdef.h"
#include "../Network/packet.h"

//-------------------------------------
// BUTTONS CALLBACKS
//-------------------------------------
static void CALLBACK_MAINMENU_NewGame(void);
static void CALLBACK_MAINMENU_HostGame(void);
static void CALLBACK_MAINMENU_JoinGame(void);
static void CALLBACK_MAINMENU_Options(void);
static void CALLBACK_MAINMENU_Quit(void);
static void CALLBACK_ReturnToMainMenu(void);
static void CALLBACK_OPTIONSMENU_ChangeGraphics(void);
static void CALLBACK_MAINMENU_About(void);
static void CALLBACK_Continue(void);

static void CALLBACK_HOSTMENU_Abort(void);
static void CALLBACK_JOINMENU_Abort(void);

// ----------------------------
// Define Menus
// ----------------------------
menuelement_t MainMenuElements[] =
{
    {"Continue",    {220, 150, 400, 40}, CALLBACK_Continue},
    {"Host  Game",   {220, 200, 400, 40}, CALLBACK_MAINMENU_HostGame},
    {"Join  Game",  {220, 250, 400, 40}, CALLBACK_MAINMENU_JoinGame},
    {"Options",     {220, 300, 400, 40}, CALLBACK_MAINMENU_Options},
    {"About",       {220, 350, 400, 40}, CALLBACK_MAINMENU_About},
    {"Quit",        {220, 400, 400, 40}, CALLBACK_MAINMENU_Quit}
};
menu_t MainMenu = {MENU_START, MainMenuElements, 6, &MainMenuElements[1]};

menuelement_t DeathMenuElements[] =
{
    {"Restart  Game",    {220, 200, 400, 40}, CALLBACK_MAINMENU_NewGame},
    {"Return",           {220, 250, 200, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t DeathMenu = {MENU_DEATH, DeathMenuElements, 2, &DeathMenuElements[0]};

menuelement_t OptionsMenuElements[] =
{
    {"Graphics:",    {220, 200, 400, 40}, CALLBACK_OPTIONSMENU_ChangeGraphics},
    {"Return",       {220, 250, 200, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t OptionsMenu = {MENU_OPTIONS, OptionsMenuElements, 2, &OptionsMenuElements[0]};

menuelement_t EndGameMenuElements[] =
{
    {"Return",    {220, 350, 400, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t EndGameMenu = {MENU_END_GAME, EndGameMenuElements, 1, &EndGameMenuElements[0]};

menuelement_t AboutMenuElements[] =
{
    {"Return",    {220, 350, 400, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t AboutMenu = {MENU_ABOUT, AboutMenuElements, 1, &AboutMenuElements[0]};

menuelement_t HostGameMenuElements[] =
{
    {"Abort",   {220, 350, 400, 40}, CALLBACK_HOSTMENU_Abort},
};

menu_t HostGameMenu = {MENU_HOSTGAME, HostGameMenuElements, 1, &HostGameMenuElements[0]};

menuelement_t JoinGameMenuElements[] =
{
    {"Abort",   {220, 350, 400, 40}, CALLBACK_JOINMENU_Abort},
};

menu_t JoinGameMenu = {MENU_JOINGAME, JoinGameMenuElements, 1, &JoinGameMenuElements[0]};

menu_t* currentMenu;


void G_InitMainMenu()
{
    G_SetMenu(&MainMenu);
}

//-------------------------------------
// Renders what's gonna be behind the current menu
//-------------------------------------
void G_RenderCurrentMenuBackground(void)
{
    switch(currentMenu->ID)
    {
        case MENU_START:
        {
            SDL_Rect titleRect = {110, 10, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->w, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->h};
            R_BlitIntoScreenScaled(NULL, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture, &titleRect);
            break;
        }

        case MENU_OPTIONS:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "Settings", 180, 80, 2.0f);

            // Display graphics current setting:
            switch(r_CurrentGraphicsSetting)
            {
                case GRAPHICS_LOW:
                    T_DisplayTextScaled(FONT_BLKCRY, "Low", 350, 205, 1.0f);
                    break;
                
                case GRAPHICS_MEDIUM:
                    T_DisplayTextScaled(FONT_BLKCRY, "Medium", 350, 205, 1.0f);
                    break;

                case GRAPHICS_HIGH:
                    T_DisplayTextScaled(FONT_BLKCRY, "High", 350, 205, 1.0f);
                    break;
            }
            break;
        }

        case MENU_DEATH:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "You  died!", 180, 80, 2.0f);
            break;
        }

        case MENU_END_GAME:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "You  won!", 180, 80, 2.0f);
            T_DisplayTextScaled(FONT_BLKCRY, "You slayed the Lord of all  Skeletons\nand you earnt your freedom.", 140, 200, 1.0f);
            break;
        }

        case MENU_ABOUT:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "About", 210, 80, 2.0f);
            T_DisplayTextScaled(FONT_BLKCRY, "Programmer:  Mattia Silvestro  ( silvematt)\nVersion: 1.0", 80, 200, 1.0f);

            break;
        }

        // Host Game
        case MENU_HOSTGAME:
        {
            if(otherPlayer.status == NETSTS_NULL)
            {
                T_DisplayTextScaled(FONT_BLKCRY, "Waiting for players...", 210, 80, 2.0f);
                int result = NET_HostGameWaitForConnection();

                // If user wanted to abort
                if(result == 2)
                {
                    wantsToAbortHosting = FALSE;
                    G_SetMenu(&MainMenu);
                    A_ChangeState(GSTATE_MENU);
                    return;
                }
            }
            // If the connection just began, wait for other player's greet
            else if(otherPlayer.status == NETSTS_JUST_CONNECTED)
            {
                T_DisplayTextScaled(FONT_BLKCRY, "Retrieving info...", 210, 80, 2.0f);

                PCKT_ReceivePacket(NET_HostGameWaitForGreet);
            }
            // If he has greet, send our greet and when that is done, set the status = NETSTS_GREETEED (in this case NETSTS_HAVE_TO_GREET is referred to us)
            else if(otherPlayer.status == NETSTS_HAVE_TO_GREET)
            {
                // If it never tried to send, there's nothing setup to send yet
                if(!outputPcktBuffer.hasBegunWriting)
                    NET_HostGameMakeGreetPacket();

                // Keep trying to send until NET_HostGameSendGreet gets called and changes otherPlayer.status to NETSTS_GREETED
                PCKT_SendPacket(NET_HostGameSendGreet);
            }
            else if(otherPlayer.status == NETSTS_GREETED)
            {
                char string[22+NET_MAX_PLAYER_NAME_LENGTH] = "Connected to player: ";
                strcat(string, otherPlayer.name);
                T_DisplayTextScaled(FONT_BLKCRY, string, 210, 80, 2.0f);

                // Receive packets (wait for readypacket etc)
            }

            break;
        }

        case MENU_JOINGAME:
        {
            if(otherPlayer.status == NETSTS_NULL)
            {
                T_DisplayTextScaled(FONT_BLKCRY, "Connecting to player...", 200, 80, 2.0f);
                int result = NET_JoinGameWaitForConnection();

                // If user wanted to abort
                if(result == 2)
                {
                    wantsToAbortJoining = FALSE;
                    G_SetMenu(&MainMenu);
                    A_ChangeState(GSTATE_MENU);
                    return;
                }
            }
            else if(otherPlayer.status == NETSTS_JUST_CONNECTED)
            {
                T_DisplayTextScaled(FONT_BLKCRY, "Retrieving info...", 200, 80, 2.0f);

                // Send our greet packet
                // If it never tried to send yet, there's nothing setup to send, so set it up
                if(!outputPcktBuffer.hasBegunWriting)
                    NET_JoinGameMakeGreetPacket();

                // Keep trying to send until NET_JoinGameSendGreet gets called and changes hostPlayer.hasGreeted = TRUE so we can wait for the host's greet
                PCKT_SendPacket(NET_JoinGameSendGreet);
            }
            else if(otherPlayer.status == NETSTS_HAVE_TO_GREET)
            {
                // Receive the other player's greet packet until NET_JoinGameWaitForGreet sets otherPlayer.status to NETSTS_GREETED
                PCKT_ReceivePacket(NET_JoinGameWaitForGreet);
            }
            else if(otherPlayer.status == NETSTS_GREETED)
            {
                char string[22+NET_MAX_PLAYER_NAME_LENGTH] = "Connected to player: ";
                strcat(string, otherPlayer.name);
                T_DisplayTextScaled(FONT_BLKCRY, string, 210, 80, 2.0f);

                // Receive packets (wait for readypacket etc)
            }
            break;
        }
    }
}

//-------------------------------------
// Render menu routine
//-------------------------------------
void G_RenderCurrentMenu(void)
{
    // Display text
    for(int i = 0; i < currentMenu->elementsLength; i++)
    {
        T_DisplayTextScaled(FONT_BLKCRY, currentMenu->elements[i].text, currentMenu->elements[i].box.x, currentMenu->elements[i].box.y, 1.25f);
    }

    // Display cursor
    SDL_Rect cursorRect = {currentMenu->selectedElement->box.x - CURSOR_X_OFFSET, currentMenu->selectedElement->box.y, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture->w, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture->h};
    R_BlitIntoScreen(NULL, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture, &cursorRect);
}

void G_InMenuInputHandling(SDL_Event* e)
{
    // Offset mouse pos
    static int x = 0;
    static int y = 0;

    //Get the mouse offsets
    x = e->button.x;
    y = e->button.y;

    for(int i = 0; i < currentMenu->elementsLength; i++)
    {
        bool isOn = (( x > currentMenu->elements[i].box.x) && ( x < currentMenu->elements[i].box.x + currentMenu->elements[i].box.w ) && 
                     ( y > currentMenu->elements[i].box.y) && ( y < currentMenu->elements[i].box.y + currentMenu->elements[i].box.h ));

        if(e->type == SDL_MOUSEMOTION)
        {
            // Select hovering element
            if (isOn)
                currentMenu->selectedElement = &currentMenu->elements[i];
        }
        else if(e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
        {
            if (isOn && currentMenu->selectedElement->OnClick != NULL)
            {
                currentMenu->selectedElement->OnClick();
                return;
            }
        }
    }
}

void G_SetMenu(menu_t* newMenu)
{
    currentMenu = newMenu;
    currentMenu->elementsLength = newMenu->elementsLength;
}

//-------------------------------------
// BUTTONS CALLBACKS
//-------------------------------------
static void CALLBACK_MAINMENU_NewGame(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    G_InitGame();
    A_ChangeState(GSTATE_GAME);
}

static void CALLBACK_MAINMENU_HostGame(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    NET_InitializeNet();
    if(NET_HostGameProcedure() == 0)
    {
        G_SetMenu(&HostGameMenu);
        A_ChangeState(GSTATE_MENU);
    }
}

static void CALLBACK_MAINMENU_JoinGame(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    NET_InitializeNet();
    if(NET_JoinGameProcedure() == 0)
    {
        G_SetMenu(&JoinGameMenu);
        A_ChangeState(GSTATE_MENU);
    }
}

static void CALLBACK_MAINMENU_Options(void)
{
    G_SetMenu(&OptionsMenu);
    A_ChangeState(GSTATE_MENU);
}


static void CALLBACK_MAINMENU_Quit(void)
{
    application.quit = true;
}


static void CALLBACK_ReturnToMainMenu(void)
{
    G_SetMenu(&MainMenu);
    A_ChangeState(GSTATE_MENU);
}

static void CALLBACK_OPTIONSMENU_ChangeGraphics(void)
{
    if(r_CurrentGraphicsSetting+1 < 3)
        r_CurrentGraphicsSetting++;
    else
        r_CurrentGraphicsSetting = 0;

    R_SetRenderingGraphics(r_CurrentGraphicsSetting);
    R_ClearRendering();
}

static void CALLBACK_Continue(void)
{
    if(player.hasBeenInitialized)
            A_ChangeState(GSTATE_GAME);
}

static void CALLBACK_MAINMENU_About(void)
{
    G_SetMenu(&AboutMenu);
    A_ChangeState(GSTATE_MENU);
}

static void CALLBACK_HOSTMENU_Abort(void)
{
    wantsToAbortHosting =  TRUE;
}

static void CALLBACK_JOINMENU_Abort(void)
{
    wantsToAbortJoining =  TRUE;
}