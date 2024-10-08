#include <math.h>

#include "I_InputHandling.h"
#include "G_Game.h"
#include "P_Physics.h"
#include "M_Map.h"
#include "G_Pathfinding.h"
#include "G_AI.h"
#include "G_MainMenu.h"

#include "../Online/O_Game.h"

// Game Timer
Timer* gameTimer;

Timer* playerUpdatePacketsTimer;    // times the send of player's updates

// Current Game Time
double curTime = 0;

// Game Time at last tick
double oldTime = 0;

// Doors
int doorstateLevel0[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
int doorstateLevel1[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
int doorstateLevel2[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)

float doorpositionsLevel0[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
float doorpositionsLevel1[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
float doorpositionsLevel2[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

// Projectiles travelling in the world
projectileNode_t* projectilesHead = NULL;

// Projectiles that hit something and are exploding
projectileNode_t* explodingProjectilesHead = NULL;

// Active puddles
mappudlle_t* activeMapPuddlesHead = NULL;

// Chat
incomingchatmessage_t* chatMsgsHead = NULL;
textfield_t chatField;

bool showFPS = false;
float readOnlyFPS = 0;
char fpsText[16];
int frameCountForFPSDisplay = 0;

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void)
{
    // Initialize game
    if(gameTimer == NULL)
        gameTimer = U_TimerCreateNew();

    T_SetTextField(&chatField, 1, 480, 274,39, 32, 0.75f);

    playerUpdatePacketsTimer = U_TimerCreateNew();

    gameTimer->Init(gameTimer);

    playerUpdatePacketsTimer->Init(playerUpdatePacketsTimer);

    // Initialize Doors //
    memset(doorstateLevel0, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));
    memset(doorstateLevel1, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));
    memset(doorstateLevel2, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));

    // All doors start closed
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            doorpositionsLevel0[y][x] = DOOR_FULLY_CLOSED;
            doorpositionsLevel1[y][x] = DOOR_FULLY_CLOSED;
            doorpositionsLevel2[y][x] = DOOR_FULLY_CLOSED;
        }

    // Empty projectile list
    if(projectilesHead != NULL)
    {
        projectileNode_t* current = projectilesHead;

        while(current != NULL)
        {
            projectileNode_t* tmp = current;
            current = current->next;
            free(tmp);
        }
        
        projectilesHead = NULL;
    }

    // Empty puddles list
    if(activeMapPuddlesHead != NULL)
    {
        mappudlle_t* current = activeMapPuddlesHead;

        while(current != NULL)
        {
            mappudlle_t* tmp = current;
            current = current->next;
            free(tmp->timer);
            free(tmp);
        }
        
        activeMapPuddlesHead = NULL;
    }

    G_ChangeMap("thefrozenend");
    
    gameTimer->Start(gameTimer);
    playerUpdatePacketsTimer->Start(playerUpdatePacketsTimer);
    
    REPL_InitializeNetworkIDs();

    // Send packet to notify other user the game started and initialize other player
    O_GameInitializeOtherPlayer();

    frameCountForFPSDisplay = 0;
}

//-------------------------------------
// Game Tick 
//-------------------------------------
void G_GameLoop(void)
{
    Uint32 start_time, frame_time;
    float fps;

    start_time = SDL_GetTicks();
    switch(application.gamestate)
    {
        case GSTATE_MENU:
            G_StateMenuLoop();
            break;

        case GSTATE_GAME:
            G_StateGameLoop();
            break;

        default:
            break;
    }

    frame_time = SDL_GetTicks()-start_time;
    fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;

    // To display fps on screen
    frameCountForFPSDisplay++;
    if(frameCountForFPSDisplay > N_FRAMES_SKIP_FOR_DISPLAY)
    {
        readOnlyFPS = fps;
        frameCountForFPSDisplay = 0;
    }
}


void G_StateGameLoop(void)
{
    curTime = gameTimer->GetTicks(gameTimer);

    P_PhysicsTick();

    // Handle input
    I_HandleInputGame();

    // If the game state was changed by the game input handler prevent this loop from going forward
    if(application.gamestate != GSTATE_GAME)
        return;

    // Do Network
    int recVal  = O_GameReceivePackets();
    int sendVal = O_GameSendPackets();

    // Check for network error
    if(recVal == PCKT_RECEIVE_RETURN_ERROR)
    {
        closesocket(otherPlayer.socket);

        // Stop the game
        G_SetMenu(&DisconnectedMenu);
        A_ChangeState(GSTATE_MENU);
        return;
    }

    // Update the game unless we're dead
    // The game stops O_GameSendPackets sends the DeathPacket, so more frames could be processed while we wait for that
    // thisPlayer.dead is set immediatly on death detection 
    if(!thisPlayer.dead)
    {
        // Do stuff
        G_PlayerTick();
        G_UpdateDoors();
        G_AIUpdate();
        G_UpdateProjectiles();
        G_UpdateMapPuddles();

        // Update other player
        O_GameOtherPlayerLoop();

        P_PhysicsEndTick();

        // Render
        // Creates the frame
        R_ComposeFrame();

        // Displays it on the screen
        R_FinishUpdate();
    }
    
    oldTime = curTime;
}

void G_StateMenuLoop(void)
{
    // Keep physic ticking even in menus, because game can be paused
    P_PhysicsTick();

    // Handles input
    I_HandleInputMenu();

    // If the game state was changed by the menu input handler prevent this loop from going forward
    // Not doing that could cause the program to freeze at the R_ComposeFrame() call, because in this loop we would try to render as if we were in the game state without having things initialized/updated
    if(application.gamestate != GSTATE_MENU)
        return;

    P_PhysicsEndTick();

    // Clears current render
    SDL_FillRect(win_surface, NULL, r_blankColor);

    // Creates the frame
    R_ComposeFrame();

    // Displays it on the screen
    R_FinishUpdate();
}

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
void G_UpdateDoors(void)
{
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
            {
                // LEVEL 0 
                // If the door is closed or open, continue
                if(doorstateLevel0[y][x] == DState_Closed || doorstateLevel0[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel0[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel0[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel0[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel0[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel0[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel0[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel0[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel0[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel0[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel0[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel0[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel0[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel0[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel0[y][x] = 1;
                        }
                    }
                }

                // LEVEL 1
                // If the door is closed or open, continue
                if(doorstateLevel1[y][x] == DState_Closed || doorstateLevel1[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel1[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel1[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel1[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel1[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel1[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel1[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel1[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel1[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel1[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel1[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel1[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel1[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel1[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel1[y][x] = 1;
                        }
                    }
                }

                // LEVEL 2
                // If the door is closed or open, continue
                if(doorstateLevel2[y][x] == DState_Closed || doorstateLevel2[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel2[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel2[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel2[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel2[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel2[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel2[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel2[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel2[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel2[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel2[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel2[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel2[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel2[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel2[y][x] = 1;
                        }
                    }
                }
            }
}

void G_ChangeMap(char* mapID)
{
    // Check if game has ended
    if(strcmp("END_GAME", mapID) == 0)
    {
        closesocket(otherPlayer.socket);
        
        player.hasBeenInitialized = false;
        G_SetMenu(&EndGameMenu);
        A_ChangeState(GSTATE_MENU);
    }
    else
    {
        M_LoadMapAsCurrent(mapID);
        G_InitPlayer();
    }
}

void G_UpdateProjectiles(void)
{   
    projectileNode_t* cur = projectilesHead;
    int i = 0;
    while(cur != NULL)
    {
        // Update base info needed for rendering
        cur->this.base.centeredPos.x = cur->this.base.pos.x;
        cur->this.base.centeredPos.y = cur->this.base.pos.y;

        cur->this.base.pSpacePos.x = cur->this.base.centeredPos.x - player.centeredPos.x;
        cur->this.base.pSpacePos.y = cur->this.base.centeredPos.y - player.centeredPos.y;

        cur->this.base.gridPos.x = cur->this.base.centeredPos.x / TILE_SIZE;
        cur->this.base.gridPos.y = cur->this.base.centeredPos.y / TILE_SIZE;

        cur->this.base.dist = sqrt(cur->this.base.pSpacePos.x*cur->this.base.pSpacePos.x + cur->this.base.pSpacePos.y*cur->this.base.pSpacePos.y);

        // If the projectile hasnt hit anything, check for hit
        if(!cur->this.isBeingDestroyed)
        {
            // Update pos
            cur->this.base.pos.x += cos(cur->this.base.angle) * cur->this.speed * deltaTime;
            cur->this.base.pos.y += sin(cur->this.base.angle) * cur->this.speed * deltaTime;

             // Determine Projectile's level
             // half a tile is added to the base.z because the collision with the bottom part should always be a bit higer than normal (otherwise the projectile hits a wall with the transparent part of the sprite)
            cur->this.base.level = (int)floor((cur->this.base.z+(HALF_TILE_SIZE)) / TILE_SIZE);
            cur->this.base.level = SDL_clamp(cur->this.base.level, 0, MAX_N_LEVELS-1);

            if(cur->this.verticalMovementDelta > 0 || cur->this.verticalMovementDelta < 0)
                cur->this.base.z += cur->this.verticalMovementDelta * deltaTime;


            // If this projectile is a network instance, let the other player do the collision checking
            if(!cur->isNetworkInstance)
            {
                // Check if projectile is not out of map
                bool insideMap = cur->this.base.gridPos.x >= 0 && cur->this.base.gridPos.y >= 0 && cur->this.base.gridPos.x < MAP_WIDTH && cur->this.base.gridPos.y < MAP_HEIGHT && cur->this.base.z > -TILE_SIZE && cur->this.base.z < TILE_SIZE*(MAX_N_LEVELS);

                // Destroy condition
                if(G_CheckCollisionMap(cur->this.base.level, cur->this.base.gridPos.y, cur->this.base.gridPos.x) == 1 || !insideMap)
                {
                    cur->this.isBeingDestroyed = true;
                    G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                    O_GameDestroyProjectile(cur->networkID, cur->this.base.spriteID, false);
                    i++;
                    cur = cur->next;
                    continue;
                }

                // AI hit
                dynamicSprite_t* sprite = NULL;
                if((sprite = G_GetFromDynamicSpriteMap(cur->this.base.level, cur->this.base.gridPos.y, cur->this.base.gridPos.x)) != NULL &&
                    cur->this.aiOwner != sprite && sprite->canBeHit)
                {
                    float damage = 0.0f;

                    // Damage sprite
                    switch(cur->this.base.spriteID)
                    {
                        case S_Fireball1:
                            damage = 55.65f;
                            break;

                        case S_IceDart1:
                            damage = 15.0f;
                            break;

                        case S_SwordProjectile:
                            damage = 60.5f;
                            break;

                        case S_MorgathulOrb:
                            damage = 0.0f;
                            break;

                        case S_IceBlast:
                            damage = 70.0f;
                            break;

                        default:
                            damage = 0.0f;
                            break;
                    }

                    if(cur->this.isOfPlayer)
                    {
                        player.crosshairHit = true;
                        player.crosshairTimer->Start(player.crosshairTimer);
                    }

                    G_AITakeDamage(sprite, damage, true);
                    O_GameAITakeDamage(sprite->networkID, damage, sprite->isAlive != true);

                    cur->this.isBeingDestroyed = true;
                    G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                    O_GameDestroyProjectile(cur->networkID, cur->this.base.spriteID, false);

                    i++;
                    cur = cur->next;

                    continue;
                }

                // Check other player heal
                if(cur->this.base.spriteID == SPELL_CONCENTRATED_HEAL)
                {
                    float otherplayerDist = sqrt((cur->this.base.centeredPos.x - otherPlayerObject.base.centeredPos.x)*(cur->this.base.centeredPos.x - otherPlayerObject.base.centeredPos.x) + (cur->this.base.centeredPos.y - otherPlayerObject.base.centeredPos.y)*(cur->this.base.centeredPos.y - otherPlayerObject.base.centeredPos.y));
                    if(cur->this.base.level == otherPlayerObject.base.level && cur->this.base.gridPos.x == otherPlayerObject.base.gridPos.x && cur->this.base.gridPos.y == otherPlayerObject.base.gridPos.y && otherplayerDist < TILE_SIZE-12)
                    {
                        // Heal other player
                        O_GameHealOther(55.0f * player.attributes.spellPower);
                        cur->this.isBeingDestroyed = true;
                        G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                        O_GameDestroyProjectile(cur->networkID, cur->this.base.spriteID, false);
                    }
                }
                // Player hit
                // Add distance checking for being more precise
                float playerDist = sqrt(cur->this.base.pSpacePos.x*cur->this.base.pSpacePos.x + cur->this.base.pSpacePos.y*cur->this.base.pSpacePos.y);
                if(!cur->this.isOfPlayer && cur->this.base.level == player.level && cur->this.base.gridPos.x == player.gridPosition.x && cur->this.base.gridPos.y == player.gridPosition.y && playerDist < TILE_SIZE-12)
                {
                    float damage = 0.0f;

                    // Damage sprite
                    switch(cur->this.base.spriteID)
                    {
                        case S_Fireball1:
                            damage = 55.65f;
                            break;

                        case S_IceDart1:
                            damage = 15.0f;
                            break;

                        case S_SwordProjectile:
                            damage = 60.5f;
                            break;

                        case S_MorgathulOrb:
                            damage = 50.25f;
                            break;

                        case S_IceBlast:
                            damage = 70.0f;
                            break;

                        default:
                            damage = 0.0f;
                            break;
                    }

                    G_PlayerTakeDamage(damage);

                    cur->this.isBeingDestroyed = true;
                    G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                    O_GameDestroyProjectile(cur->networkID, cur->this.base.spriteID, false);

                    i++;
                    cur = cur->next;

                    continue;
                }
            }
            // Check client side collision
            else if(cur->isNetworkInstance)
            {
                // Player hit
                // Add distance checking for being more precise
                float playerDist = sqrt(cur->this.base.pSpacePos.x*cur->this.base.pSpacePos.x + cur->this.base.pSpacePos.y*cur->this.base.pSpacePos.y);
                if(!cur->this.isOfPlayer && cur->this.base.level == player.level && cur->this.base.gridPos.x == player.gridPosition.x && cur->this.base.gridPos.y == player.gridPosition.y && playerDist < TILE_SIZE-12)
                {
                    float damage = 0.0f;

                    // Damage sprite
                    switch(cur->this.base.spriteID)
                    {
                        case S_Fireball1:
                            damage = 55.65f;
                            break;

                        case S_IceDart1:
                            damage = 15.0f;
                            break;

                        case S_SwordProjectile:
                            damage = 60.5f;
                            break;

                        case S_MorgathulOrb:
                            damage = 50.25f;
                            break;

                        case S_IceBlast:
                            damage = 70.0f;
                            break;

                        default:
                            damage = 0.0f;
                            break;
                    }

                    G_PlayerTakeDamage(damage);

                    cur->this.isBeingDestroyed = true;
                    G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                    
                    // Force destroy is true because we've requested the destroyment of an instance that on this side is networked but on the other side is local
                    O_GameDestroyProjectile(cur->networkID, cur->this.base.spriteID, true);
                    i++;
                    cur = cur->next;

                    continue;
                }
            }
        }
        // The projectile has hit, wait for the death animation to play
        else
        {
            if(cur->this.animFrame >= tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDieSheetLength-1)
            {
                // Destroy this
                if(projectilesHead == cur)
                    projectilesHead = cur->next;

                if(cur->next != NULL)
                    cur->next->previous = cur->previous;

                if(cur->previous != NULL)
                    cur->previous->next = cur->next;

                projectileNode_t* dead = cur;
                cur = cur->next;
                free(dead->this.animTimer);
                free(dead);
                continue;
            }
        }

        // Select Animation & Play it
        switch(cur->this.state)
        {
            case DS_STATE_IDLE:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdle;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdleSheetLength;
                break;

            case DS_STATE_DEAD:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDie;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDieSheetLength;
                break;

            case DS_STATE_ATTACKING:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animAttack;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animAttackSheetLength;
                break;

            default:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdle;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdleSheetLength;
                break;
        }

        if(cur->this.animPlay)
        {
            if(cur->this.animPlayOnce)
            {
                if(cur->this.curAnimLength > 0)
                    cur->this.animFrame = ((int)floor(cur->this.animTimer->GetTicks(cur->this.animTimer) / cur->this.animSpeed) % cur->this.curAnimLength);

                // Prevent loop
                if(cur->this.animFrame >= cur->this.curAnimLength-1)
                {
                    cur->this.animPlay = false;

                    // Go back to idle if it was attacking
                    if(cur->this.state == DS_STATE_ATTACKING)
                    {
                        G_AIPlayAnimationLoop(&cur->this, ANIM_IDLE);
                    }
                }
            }
            else
            {
                // Allow loop
                if(cur->this.curAnimLength > 0)
                    cur->this.animFrame = ((int)floor(cur->this.animTimer->GetTicks(cur->this.animTimer) / cur->this.animSpeed) % cur->this.curAnimLength);
            }
        }

        i++;
        cur = cur->next;
    }
}


void G_SpawnProjectile(uint32_t networkID, int id, float angle, int level, float posx, float posy, float posz, float verticalAngle, bool isOfPlayer, dynamicSprite_t* aiOwner, bool isNetworkInstance)
{
    // Allocate a node
    projectileNode_t* newNode = (projectileNode_t*)malloc(sizeof(projectileNode_t));

    newNode->isNetworkInstance = isNetworkInstance;
    newNode->networkID = networkID;

    // Set initial data like pos, dir and speed
    newNode->this.type = DS_TYPE_PROJECTILE;
    newNode->this.curAnim = NULL;
    newNode->this.curAnimLength = 0;
    newNode->this.animPlay = true;
    newNode->this.state = ANIM_IDLE;
    newNode->this.animTimer = U_TimerCreateNew();
    newNode->this.isBeingDestroyed = false;

    newNode->this.base.spriteID = id;
    newNode->this.base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[id];

    // Select speed
    switch(id)
    {
        case S_MorgathulOrb:
            newNode->this.speed = 700.0f;
            break;
        
        default:
            newNode->this.speed = 500.0f;
            break;
    }

    newNode->this.base.active = true;
    newNode->this.base.angle = angle;
    newNode->this.base.level = level;

    newNode->this.base.pos.x = posx;
    newNode->this.base.pos.y = posy;
    newNode->this.base.z = posz;

    // Vertical angle is how much the entity that launches the projectile is looking up/down
    newNode->this.verticalMovementDelta = verticalAngle;

    newNode->this.base.gridPos.x = posx / TILE_SIZE;
    newNode->this.base.gridPos.y = posy / TILE_SIZE;

    newNode->this.base.centeredPos.x = newNode->this.base.pos.x + (HALF_TILE_SIZE);
    newNode->this.base.centeredPos.y = newNode->this.base.pos.y + (HALF_TILE_SIZE);
    
    newNode->this.base.pSpacePos.x = newNode->this.base.centeredPos.x - player.centeredPos.x;
    newNode->this.base.pSpacePos.y = newNode->this.base.centeredPos.y - player.centeredPos.y;

    newNode->this.isOfPlayer = isOfPlayer;
    newNode->this.aiOwner = aiOwner;

    newNode->next = NULL;
    if(projectilesHead == NULL)
    {
        projectilesHead = newNode;
        projectilesHead->next = NULL;
        projectilesHead->previous = NULL;
    }
    else
    {
        projectileNode_t* current = projectilesHead;

        while(current->next != NULL)
            current = current->next;

        // Now we can add
        current->next = newNode;
        current->next->next = NULL;
        newNode->previous = current;
    }

    // Play idle anim
    G_AIPlayAnimationLoop(&newNode->this, ANIM_IDLE);
}

void G_UpdateMapPuddles(void)
{
    mappudlle_t* cur = activeMapPuddlesHead;
    while(cur != NULL)
    {
        if(cur->timer->GetTicks(cur->timer) >= cur->duration)
        {
            // Destroy this

            // Restore state
            // Modify floor
            if(cur->level == 0)
            {
                currentMap.floorMap[cur->gridY][cur->gridX] = cur->previousFloorID;
            }
            else if(cur->level == 1)
            {
                currentMap.level0[cur->gridY][cur->gridX].texturesArray[TEXTURE_ARRAY_TOP] = cur->previousFloorID;
            }
            else if (cur->level == 2)
            {
                currentMap.level1[cur->gridY][cur->gridX].texturesArray[TEXTURE_ARRAY_TOP] = cur->previousFloorID;
            }

            if(activeMapPuddlesHead == cur)
                activeMapPuddlesHead = cur->next;

            if(cur->next != NULL)
                cur->next->previous = cur->previous;

            if(cur->previous != NULL)
                cur->previous->next = cur->next;

            // Restore floor texture

            mappudlle_t* dead = cur;
            cur = cur->next;
            free(dead->timer);
            free(dead);
            continue;
        }

        cur = cur->next;
    }
}

packedpuddle_t G_SpawnMapPuddle(int networkID, int gridX, int gridY, bool damagesAI, bool damagesPlayer, float damage, int duration, int level, int newFloorID, bool isNetworkInstance)
{
    packedpuddle_t data;    // data to return

    mappudlle_t* newNode = (mappudlle_t*)malloc(sizeof(mappudlle_t));

    // Set initial data like pos, dir and speed
    newNode->timer = U_TimerCreateNew();
    newNode->timer->Start(newNode->timer);
    
    newNode->isNetworkedInstance = isNetworkInstance;
    data.networkID = newNode->networkID = networkID;

    data.gridX = newNode->gridX = gridX;
    data.gridY = newNode->gridY = gridY;
    data.level = newNode->level = level;
    data.duration = newNode->duration = duration;
    data.damagesAI = newNode->damagesAI = damagesAI;
    data.damagesPlayer = newNode->damagesPlayers = damagesPlayer;
    data.damage = newNode->damage = damage;
    data.newFloorID = newNode->newFloorID = newFloorID;
    
    // Modify floor
    if(level == 0)
    {
        data.previousFloorID = newNode->previousFloorID = currentMap.floorMap[gridY][gridX];
        currentMap.floorMap[gridY][gridX] = newFloorID;
    }
    else if(level == 1)
    {
        data.previousFloorID = newNode->previousFloorID = currentMap.level0[gridY][gridX].texturesArray[TEXTURE_ARRAY_TOP];
        currentMap.level0[gridY][gridX].texturesArray[TEXTURE_ARRAY_TOP] = newFloorID;
    }
    else if (level == 2)
    {
        data.previousFloorID = newNode->previousFloorID = currentMap.level1[gridY][gridX].texturesArray[TEXTURE_ARRAY_TOP];
        currentMap.level1[gridY][gridX].texturesArray[TEXTURE_ARRAY_TOP] = newFloorID;
    }

    newNode->next = NULL;
    if(activeMapPuddlesHead == NULL)
    {
        activeMapPuddlesHead = newNode;
        activeMapPuddlesHead->next = NULL;
        activeMapPuddlesHead->previous = NULL;
    }
    else
    {
        mappudlle_t* current = activeMapPuddlesHead;

        while(current->next != NULL)
            current = current->next;

        // Now we can add
        current->next = newNode;
        current->next->next = NULL;
        newNode->previous = current;
    }

    // Return packed puddle data
    return data;
}


void G_SpawnIncomingChatMessage(int fontID, int x, int y, char* msg, float duration, float scale)
{
    // Allocate a node
    incomingchatmessage_t* newNode = (incomingchatmessage_t*)malloc(sizeof(incomingchatmessage_t));

    newNode->x = x;
    newNode->y = y;
    strcpy(newNode->msg, msg);
    newNode->duration = duration;
    newNode->fontID = fontID;
    newNode->scale = scale;
    
    newNode->timer = U_TimerCreateNew();
    newNode->timer->Start(newNode->timer);

    // Add to list
    newNode->next = NULL;
    if(chatMsgsHead == NULL)
    {
        chatMsgsHead = newNode;
        chatMsgsHead->next = NULL;
        chatMsgsHead->previous = NULL;
    }
    else
    {
        incomingchatmessage_t* current = chatMsgsHead;

        while(current->next != NULL)
            current = current->next;

        // Now we can add
        current->next = newNode;
        current->next->next = NULL;
        newNode->previous = current;
    }
}
