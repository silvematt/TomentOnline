#ifndef MAIN_MENU_H_INCLUDED
#define MAIN_MENU_H_INCLUDED

#include <stdbool.h>

#include "SDL.h"
#include "T_TextRendering.h"

// How off the text the cursor should be
#define CURSOR_X_OFFSET 35

typedef enum
{
    MENU_START = 0,
    MENU_DEATH,
    MENU_OPTIONS,
    MENU_END_GAME,
    MENU_ABOUT,
    MENU_HOSTGAME,
    MENU_JOINGAME,
    MENU_INLOBBY,
    MENU_DISCONNECTED,
    MENU_SETUPHOSTGAME,
    MENU_SETUPJOINGAME
} menusIDs_e;


typedef struct menuelement_s
{
    char* text;
    SDL_Rect box;
    void (*OnClick)(void);
} menuelement_t;

typedef struct menu_s
{
    int ID;
    menuelement_t* elements;
    int elementsLength;
    menuelement_t* selectedElement;
    
    // Text fields
    int textFieldsLength;
    textfield_t* textFields;
} menu_t;


extern menu_t* currentMenu;

// Menus are defined at compile-time
extern menuelement_t MainMenuElements[];
extern menu_t MainMenu;

extern menuelement_t DeathMenuElements[];
extern menu_t DeathMenu;

extern menuelement_t OptionsMenuElements[];
extern menu_t OptionsMenu;

extern menuelement_t EndGameMenuElements[];
extern menu_t EndGameMenu;

extern menuelement_t AboutMenuElements[];
extern menu_t AboutMenu;

// TomentOnline
extern menuelement_t HostGameMenuElements[];
extern menu_t HostGameMenu;

extern menuelement_t JoinGameMenuElements[];
extern menu_t JoinGameMenu;

extern menuelement_t InLobbyMenuElements[];
extern menu_t InLobbyMenu;

extern menuelement_t DisconnectedMenuElements[];
extern menu_t DisconnectedMenu;

extern bool isEditingTextField;
extern textfield_t * textFieldEditing;

//-------------------------------------
// Initialize the Main Menu state
//-------------------------------------
void G_InitMainMenu();

//-------------------------------------
// Set the MenuState
//-------------------------------------
void G_SetMenu(menu_t* menu);

//-------------------------------------
// Renders what's gonna be behind the current menu
//-------------------------------------
void G_RenderCurrentMenuBackground(void);

//-------------------------------------
// Render menu routine, renders the elements of any menu
//-------------------------------------
void G_RenderCurrentMenu(void);

//-------------------------------------
// Manages Menu Input routine
//-------------------------------------
void G_InMenuInputHandling(SDL_Event* e);


#endif
