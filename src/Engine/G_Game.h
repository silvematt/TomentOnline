#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "G_Player.h"
#include "R_Rendering.h"
#include "U_Timer.h"
#include "../Network/packet.h"

// DOOR DEFINE
#define DOOR_FULLY_CLOSED 64.0f
#define DOOR_FULLY_OPENED 2.0f
#define DOOR_OPEN_SPEED 100.0f
#define DOOR_CLOSE_SPEED 100.0f


// Game Timer
extern Timer* gameTimer;
extern Timer* playerUpdatePacketsTimer;    // times the send of player's updates

// Current Game Time
extern double curTime;

// Game Time at last tick
extern double oldTime;

// Doors
extern int doorstateLevel0[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
extern int doorstateLevel1[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
extern int doorstateLevel2[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)

extern float doorpositionsLevel0[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
extern float doorpositionsLevel1[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
extern float doorpositionsLevel2[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

extern projectileNode_t* projectilesHead;
extern projectileNode_t* explodingProjectilesHead;

extern mappudlle_t* activeMapPuddlesHead;


//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void);

//-------------------------------------
// Loop of the application 
//-------------------------------------
void G_GameLoop(void);

//-------------------------------------
// Loop of the game while in menu (GSTATE_MENU) 
//-------------------------------------
void G_StateMenuLoop(void);

//-------------------------------------
// Loop of the game while in game (GSTATE_GAME) 
//-------------------------------------
void G_StateGameLoop(void);

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
void G_UpdateDoors(void);

//-------------------------------------
// Loads the map with ID mapID
//-------------------------------------
void G_ChangeMap(char* mapID);

//-------------------------------------
// Updates the spawned projectiles
//-------------------------------------
void G_UpdateProjectiles(void);

//-------------------------------------
// Spawns a new projectile
//-------------------------------------
void G_SpawnProjectile(uint32_t networkID, int id, float angle, int level, float posx, float posy, float posz, float verticalAngle, bool isOfPlayer, dynamicSprite_t* aiOwner, bool isNetworkInstance);

void G_UpdateMapPuddles(void);
packedpuddle_t G_SpawnMapPuddle(int networkID, int gridX, int gridY, bool damagesAI, bool damagesPlayer, float damage, int duration, int level, int newFloorID, bool isNetworkInstance);

void G_ResetGame(void);

#endif
