#include "G_MainMenu.h"
#include "G_Game.h"
#include "R_Rendering.h"
#include "D_AssetsManager.h"

#include "../Network/netdef.h"
#include "../Network/packet.h"
#include "../Online/O_Lobby.h"

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
static void CALLBACK_OPTIONSMENU_ChangeDefaultClass(void);
static void CALLBACK_MAINMENU_About(void);
static void CALLBACK_Continue(void);

static void CALLBACK_HOSTMENU_Abort(void);
static void CALLBACK_JOINMENU_Abort(void);

static void CALLBACK_LOBBY_Ready(void);

static void CALLBACK_DISCONNECTED_Leave(void);

static void CALLBACK_SETUPHOSTMENU_Host(void);
static void CALLBACK_SETUPJOINMENU_Join(void);
// ----------------------------
// Define Menus
// ----------------------------
menuelement_t MainMenuElements[] =
{
    {"Continue",    {250, 170, 400, 40}, CALLBACK_Continue},
    {"Host  Game",   {250, 220, 400, 40}, CALLBACK_MAINMENU_HostGame},
    {"Join  Game",  {250, 270, 400, 40}, CALLBACK_MAINMENU_JoinGame},
    {"Options",     {250, 320, 400, 40}, CALLBACK_MAINMENU_Options},
    {"About",       {250, 370, 400, 40}, CALLBACK_MAINMENU_About},
    {"Quit",        {250, 420, 400, 40}, CALLBACK_MAINMENU_Quit}
};

menu_t MainMenu = {MENU_START, MainMenuElements, 6, &MainMenuElements[1], 0};

menuelement_t DeathMenuElements[] =
{
    {"Return",           {220, 250, 200, 40}, CALLBACK_ReturnToMainMenu}
};
menu_t DeathMenu = {MENU_DEATH, DeathMenuElements, 1, &DeathMenuElements[0], 0};

menuelement_t OptionsMenuElements[] =
{
    {"Graphics:",       {220, 200, 400, 40}, CALLBACK_OPTIONSMENU_ChangeGraphics},
    {"Default class:",  {220, 250, 400, 40}, CALLBACK_OPTIONSMENU_ChangeDefaultClass},
    {"Return",          {220, 350, 200, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t OptionsMenu = {MENU_OPTIONS, OptionsMenuElements, 3, &OptionsMenuElements[0], 0};

menuelement_t EndGameMenuElements[] =
{
    {"Return",    {220, 350, 400, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t EndGameMenu = {MENU_END_GAME, EndGameMenuElements, 1, &EndGameMenuElements[0], 0};

menuelement_t AboutMenuElements[] =
{
    {"Return",    {220, 350, 400, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t AboutMenu = {MENU_ABOUT, AboutMenuElements, 1, &AboutMenuElements[0], 0};

menuelement_t HostGameMenuElements[] =
{
    {"Abort",   {220, 350, 400, 40}, CALLBACK_HOSTMENU_Abort},
};

menu_t HostGameMenu = {MENU_HOSTGAME, HostGameMenuElements, 1, &HostGameMenuElements[0], 0};

//------
menuelement_t SetupHostGameElements[] = 
{
    {"Host",     {250, 350, 400, 40}, CALLBACK_SETUPHOSTMENU_Host},
    {"Return",   {250, 400, 400, 40}, CALLBACK_ReturnToMainMenu},
};

textfield_t SetupHostGameTextFields[] =
{
    {240,270,280,45,false,"Enter Text...", 14, 1.0f} // Host username
};

menu_t SetupHostGameMenu = {MENU_SETUPHOSTGAME, SetupHostGameElements, 2, &SetupHostGameElements[0], 1, SetupHostGameTextFields};
// ----

menuelement_t JoinGameMenuElements[] =
{
    {"Abort",   {220, 350, 400, 40}, CALLBACK_JOINMENU_Abort},
};

menu_t JoinGameMenu = {MENU_JOINGAME, JoinGameMenuElements, 1, &JoinGameMenuElements[0], 0};

//------
menuelement_t SetupJoinGameElements[] = 
{
    {"Join",     {250, 500, 400, 40}, CALLBACK_SETUPJOINMENU_Join},
    {"Return",   {250, 550, 400, 40}, CALLBACK_ReturnToMainMenu},
};

textfield_t SetupJoinGameTextFields[] =
{
    {220,220,280,40,false,"Enter Text...", 14, 1.0f }, // Joiner username
    {220,320,280,40,false,"192.168.1.191", 20, 1.0f }, // Host address
    {220,420,280,40,false,"61530", 10, 1.0f },         // Host port
};

menu_t SetupJoinGameMenu = {MENU_SETUPJOINGAME, SetupJoinGameElements, 2, &SetupJoinGameElements[0], 3, SetupJoinGameTextFields};
// ----


menuelement_t InLobbyMenuElements[] =
{
    {"Ready",       {650, 550, 400, 40}, CALLBACK_LOBBY_Ready},
    {"Leave",       {50, 550, 200, 40}, NULL},
};
menu_t InLobbyMenu = {MENU_INLOBBY, InLobbyMenuElements, 2, &InLobbyMenuElements[0], 0};

menuelement_t DisconnectedMenuElements[] =
{
    {"Leave",   {220, 350, 400, 40}, CALLBACK_DISCONNECTED_Leave},
};

menu_t DisconnectedMenu = {MENU_DISCONNECTED, DisconnectedMenuElements, 1, &DisconnectedMenuElements[0], 0};


menu_t* currentMenu;

// Flags to keep track of the selected input field
bool isEditingTextField = false;
textfield_t* textFieldEditing = NULL;


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
            SDL_Rect titleRect = {175, 10, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->w, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->h};
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

            // Display favorite class current setting:
            switch(thisPlayer.favoriteClass)
            {
                case CLASS_TANK:
                    T_DisplayTextScaled(FONT_BLKCRY, "Tank", 400, 255, 1.0f);
                    break;
                
                case CLASS_HEALER:
                    T_DisplayTextScaled(FONT_BLKCRY, "Healer", 400, 255, 1.0f);
                    break;

                case CLASS_DPS:
                    T_DisplayTextScaled(FONT_BLKCRY, "Dps", 400, 255, 1.0f);
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
            T_DisplayTextScaled(FONT_BLKCRY, "Programmer:  Mattia  Silvestro  ( silvematt)\nVersion: 1.1", 80, 200, 1.0f);

            break;
        }

        case MENU_SETUPHOSTGAME:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "Host  Ga me", 210, 80, 2.0f);

            T_DisplayTextScaled(FONT_BLKCRY, "Username:", 240, 207, 1.5f);

            break;
        }

        case MENU_SETUPJOINGAME:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "Join  Ga me", 210, 50, 2.0f);

            T_DisplayTextScaled(FONT_BLKCRY, "Username:", 220, 175, 1.5f);
            T_DisplayTextScaled(FONT_BLKCRY, "IP Address:", 220, 270, 1.5f);
            T_DisplayTextScaled(FONT_BLKCRY, "Port:", 220, 375, 1.5f);

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

                int recVal = PCKT_ReceivePacket(NET_HostGameWaitForGreet);
                // Check for network error
                if(recVal == PCKT_RECEIVE_RETURN_ERROR)
                {
                    closesocket(otherPlayer.socket);

                    // Stop the game
                    G_SetMenu(&DisconnectedMenu);
                    A_ChangeState(GSTATE_MENU);
                    return;
                }
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
                G_SetMenu(&InLobbyMenu);
                A_ChangeState(GSTATE_MENU);
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

                // Keep trying to send until NET_JoinGameSendGreet gets called
                PCKT_SendPacket(NET_JoinGameSendGreet);
            }
            else if(otherPlayer.status == NETSTS_HAVE_TO_GREET)
            {
                // Receive the other player's greet packet until NET_JoinGameWaitForGreet sets otherPlayer.status to NETSTS_GREETED
                int recVal = PCKT_ReceivePacket(NET_JoinGameWaitForGreet);
                // Check for network error
                if(recVal == PCKT_RECEIVE_RETURN_ERROR)
                {
                    closesocket(otherPlayer.socket);

                    // Stop the game
                    G_SetMenu(&DisconnectedMenu);
                    A_ChangeState(GSTATE_MENU);
                    return;
                }
            }
            else if(otherPlayer.status == NETSTS_GREETED)
            {
                T_DisplayTextScaled(FONT_BLKCRY, "Lobby", 300, 30, 2.0f);

                G_SetMenu(&InLobbyMenu);
                A_ChangeState(GSTATE_MENU);
            }
            break;
        }

        case MENU_INLOBBY:
        {
            // Receive packets (wait for readypacket, class changes)
            int recVal = O_LobbyReceivePackets();
            // Check for network error
            if(recVal == PCKT_RECEIVE_RETURN_ERROR)
            {
                closesocket(otherPlayer.socket);

                // Stop the game
                G_SetMenu(&DisconnectedMenu);
                A_ChangeState(GSTATE_MENU);
                return;
            }

            O_LobbyRender();

            // Send packets (readypacket, class changes)
            O_LobbySendPackets();
            break;
        }

        case MENU_DISCONNECTED:
        {
            if(!otherPlayer.dead)
                T_DisplayTextScaled(FONT_BLKCRY, "You have been disconnected.", 100, 80, 2.0f);
            else
                T_DisplayTextScaled(FONT_BLKCRY, "The other player died!", 110, 80, 2.0f);
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

    // Display Text Fields
    for(int i = 0; i < currentMenu->textFieldsLength; i++)
    {
        SDL_Rect textFieldScreenpos = {currentMenu->textFields[i].x, currentMenu->textFields[i].y, currentMenu->textFields[i].w, currentMenu->textFields[i].h};
        SDL_Rect textFieldSize = {(0), (0), 309, 52};
        R_BlitIntoScreenScaled(&textFieldSize, currentMenu->textFields[i].isFocus ? tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01_ACTIVE]->texture : tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01]->texture, &textFieldScreenpos);

        T_DisplayTextScaled(FONT_BLKCRY, currentMenu->textFields[i].text, currentMenu->textFields[i].x, currentMenu->textFields[i].y, currentMenu->textFields[i].textScale);
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

    if(!isEditingTextField)
    {
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

    // Check Text Fields
    if(!isEditingTextField)
    {
        for(int i = 0; i < currentMenu->textFieldsLength; i++)
        {
            bool isOn = (( x > currentMenu->textFields[i].x) && ( x < currentMenu->textFields[i].x + currentMenu->textFields[i].w) && 
                        ( y > currentMenu->textFields[i].y) && ( y < currentMenu->textFields[i].y + currentMenu->textFields[i].h));

            if(e->type == SDL_MOUSEMOTION)
            {
                // Select hovering element
                //if (isOn)
                //    currentMenu->selectedElement = &currentMenu->elements[i];
            }
            else if(e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
            {
                if (isOn && !isEditingTextField)
                {
                    isEditingTextField = true;
                    currentMenu->textFields[i].isFocus = true;
                    textFieldEditing = &currentMenu->textFields[i];
                    
                    // If it's the first time clicking a "Enter Text..." field, remove the placeholder text
                    if(strcmp("Enter Text...", textFieldEditing->text) == 0)
                        textFieldEditing->text[0] = '\0';

                    SDL_StartTextInput();
                    return;
                }
            }
        }
    }
    else // if is editing a field
    {
        if(textFieldEditing != NULL)
        {
            // Get is on
            bool isOn = (( x > textFieldEditing->x) && ( x < textFieldEditing->x + textFieldEditing->w) && 
                        (  y > textFieldEditing->y) && ( y < textFieldEditing->y + textFieldEditing->h));

            // Check click out of box
            if((e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT && !isOn) ||
                (e->type == SDL_KEYUP && (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_ESCAPE)))
            {
                textFieldEditing->isFocus = false;
                textFieldEditing = NULL;
                isEditingTextField = false;
                SDL_StopTextInput();
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
    G_SetMenu(&SetupHostGameMenu);
    A_ChangeState(GSTATE_MENU);
}

static void CALLBACK_SETUPHOSTMENU_Host(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    NET_InitializeNet();
    if(NET_HostGameProcedure(SetupHostGameTextFields[0].text) == 0)
    {
        G_SetMenu(&HostGameMenu);
        A_ChangeState(GSTATE_MENU);
    }
}

static void CALLBACK_MAINMENU_JoinGame(void)
{
    G_SetMenu(&SetupJoinGameMenu);
    A_ChangeState(GSTATE_MENU);
}

static void CALLBACK_SETUPJOINMENU_Join(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    NET_InitializeNet();
    if(NET_JoinGameProcedure(SetupJoinGameTextFields[0].text, SetupJoinGameTextFields[1].text, SetupJoinGameTextFields[2].text) == 0)
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

static void CALLBACK_OPTIONSMENU_ChangeDefaultClass(void)
{
    if(thisPlayer.favoriteClass+1 < 3)
        thisPlayer.favoriteClass++;
    else
        thisPlayer.favoriteClass = 0;
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

static void CALLBACK_LOBBY_Ready(void)
{
    O_LobbySetReady(!thisPlayer.isReady);
}

static void CALLBACK_DISCONNECTED_Leave(void)
{
    G_SetMenu(&MainMenu);
    A_ChangeState(GSTATE_MENU);
}