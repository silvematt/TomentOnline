#include <math.h>

#include "G_Player.h"
#include "R_Rendering.h"
#include "I_InputHandling.h"
#include "M_Map.h"
#include "P_Physics.h"
#include "U_Utilities.h"
#include "G_Pathfinding.h"
#include "D_ObjectsCallbacks.h"
#include "G_AI.h"
#include "T_TextRendering.h"
#include "../Network/netdef.h"
#include "../Network/replication.h"
#include "../Online/O_Game.h"

player_t player;    // Player

// Init by init player, they change with resolution
int MAX_VERTICAL_HEAD_MOV = 25;
int MIN_VERTICAL_HEAD_MOV = -25;

static void I_DetermineInFrontGrid(void);
static void I_SetAttackCone(int id, int x, int y);

static bool I_PlayerAttack(int attackType);
static bool I_PlayerCastSpell(playerSpells_e attackType);

static void I_PlayerLadderMovements();

// ----------------------------------------------------
// Sets an SDL_Rect
// ----------------------------------------------------
void SDL_Rect_Set(SDL_Rect* r, int x, int y, int w, int h)
{
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
}

//-------------------------------------
// Initializes Player 
//-------------------------------------
void G_InitPlayer(void)
{
    // Init player
    player.position.x = (currentMap.playerStartingGridX * TILE_SIZE);
    player.position.y = (currentMap.playerStartingGridY * TILE_SIZE);
    
    // Move the second player a tile to the right (TODO: may change this in the future)
    if(!thisPlayer.isHost)
        player.position.x += TILE_SIZE;

    player.collisionCircle.pos.x = player.position.x;
    player.collisionCircle.pos.y = player.position.y;

    player.angle = currentMap.playerStartingRot;
    player.verticalHeadMovement = 0.0f;
    player.z = (HALF_TILE_SIZE) + ((TILE_SIZE) * currentMap.playerStartingLevel);
    player.level = currentMap.playerStartingLevel;

    player.gridPosition.x = ((player.position.x+PLAYER_CENTER_FIX) / TILE_SIZE);
    player.gridPosition.y = ((player.position.y+PLAYER_CENTER_FIX) / TILE_SIZE);

    player.collisionCircle.r = TILE_SIZE / 2;
    player.hasToClimb = false;

    // Init anim
    player.state = PSTATE_IDLE;
    player.animTimer = U_TimerCreateNew();
    player.animFrame = 0;
    player.animPlay = true;
    player.animPlayOnce = false;
    player.animTimer->Start(player.animTimer);


    // Absolute initialization, should not be repeted on next G_InitPlayer calls
    if(!player.hasBeenInitialized)
    {
        player.attributes.maxHealth = 100.0f;
        player.attributes.curHealth = player.attributes.maxHealth;
        
        player.attributes.maxMana = 100.0f;
        player.attributes.curMana = player.attributes.maxMana;

        player.attributes.spellPower = 1.0f;

        G_PlayerSetWeapon(PLAYER_FP_HANDS);
        G_PlayerSetSpell(SPELL_NULL);

        player.hasAxe = false;
        player.hasGreatsword = false;
        player.hasFireball = false;
        player.hasIceDart = false;
        player.hasMace = false;

        // Set selected class stuff
        switch(thisPlayer.selectedClass)
        {
            case CLASS_TANK:
            {
                player.attributes.maxHealth = 250.0f;
                player.attributes.curHealth = player.attributes.maxHealth;
                
                player.attributes.maxMana = 100.0f;
                player.attributes.curMana = player.attributes.maxMana;

                G_PlayerSetWeapon(PLAYER_FP_MACE);
                G_PlayerSetSpell(SPELL_NULL);
                break;
            }

            case CLASS_HEALER:
            {
                player.attributes.maxHealth = 100.0f;
                player.attributes.curHealth = player.attributes.maxHealth;
                
                player.attributes.maxMana = 250.0f;
                player.attributes.curMana = player.attributes.maxMana;

                G_PlayerSetWeapon(PLAYER_FP_AXE);
                G_PlayerSetSpell(SPELL_ICEDART1);
                break;
            }

            case CLASS_DPS:
            {
                player.attributes.maxHealth = 150.0f;
                player.attributes.curHealth = player.attributes.maxHealth;
                
                player.attributes.maxMana = 150.0f;
                player.attributes.curMana = player.attributes.maxMana;

                G_PlayerSetWeapon(PLAYER_FP_GREATSWORD);
                G_PlayerSetSpell(SPELL_ICEDART1);
                break;
            }
        }

        G_InitializeSkills();
    }

    // Rect for minimap
    SDL_Rect_Set(&player.surfaceRect, (int)player.position.x, (int)player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);

    player.crosshairHit = false;
    player.crosshairTimer = U_TimerCreateNew();

    player.isFightingBoss = false;
    player.bossFighting = NULL;
    player.hasBeenInitialized = true;

    // Do one tick
    G_PlayerTick();
}

//-------------------------------------
// Player's Tick 
//-------------------------------------
void G_PlayerTick(void)
{
    if(!player.hasToClimb)
    {
        // Get player grid pos
        player.gridPosition.x = ((player.position.x+PLAYER_CENTER_FIX) / TILE_SIZE);
        player.gridPosition.y = ((player.position.y+PLAYER_CENTER_FIX) / TILE_SIZE);
        
        //player.angle = M_PI / 4;
        player.angle += (playerinput.mouseInput.x * PLAYER_ROT_SPEED) * deltaTime;
        player.verticalHeadMovement += (playerinput.mouseInput.y * PLAYER_VERTICAL_HEAD_MOVEMENT_SPEED) * deltaTime;
        player.verticalHeadMovement = SDL_clamp(player.verticalHeadMovement, MIN_VERTICAL_HEAD_MOV, MAX_VERTICAL_HEAD_MOV);
        playerinput.mouseInput.x = 0; // kill the mouse movement after applying rot
        playerinput.mouseInput.y = 0; // kill the mouse movement after applying rot


        FIX_ANGLES(player.angle);

        playerinput.dir.x = cos(player.angle);
        playerinput.dir.y = sin(player.angle);

        //printf(" ANGLE: %f DIR: %f | %f\n", player.angle, playerinput.dir.x, playerinput.dir.y);

        // Calculate dx dy
        player.deltaPos.x = (playerinput.dir.x * playerinput.input.y) * PLAYER_SPEED * deltaTime;
        player.deltaPos.y = (playerinput.dir.y * playerinput.input.y) * PLAYER_SPEED * deltaTime;

        // Strafe
        if(playerinput.strafe.x != 0.0f)
        {
            float adjustedAngle = player.angle;
            vector2_t strafedDir;

            // Player wants to strafe
            if(playerinput.strafe.x >= 1.0f)
                adjustedAngle = player.angle + (M_PI / 2);
            else
                adjustedAngle = player.angle - (M_PI / 2);

            strafedDir.x = cos(adjustedAngle);
            strafedDir.y = sin(adjustedAngle);

            player.deltaPos.x += (strafedDir.x) * PLAYER_SPEED * deltaTime;
            player.deltaPos.y += (strafedDir.y) * PLAYER_SPEED * deltaTime;
        }

        // After calculating the movement, check for collision and in case, cancel the delta
        G_PlayerCollisionCheck();
        
        // Move Player normally
        player.position.x += player.deltaPos.x;
        player.position.y += player.deltaPos.y;

        // Clamp player in map boundaries
        player.position.x = SDL_clamp(player.position.x, 0.0f, (MAP_WIDTH * TILE_SIZE)-(HALF_TILE_SIZE));
        player.position.y = SDL_clamp(player.position.y, 0.0f, (MAP_WIDTH * TILE_SIZE)-(HALF_TILE_SIZE));

        // Compute centered pos for calculations
        player.centeredPos.x = player.position.x + PLAYER_CENTER_FIX;
        player.centeredPos.y = player.position.y + PLAYER_CENTER_FIX;

        // Update collision circle
        player.collisionCircle.pos.x = player.centeredPos.x;
        player.collisionCircle.pos.y = player.centeredPos.y;

        // Get player grid pos
        player.gridPosition.x = ((player.position.x+PLAYER_CENTER_FIX) / TILE_SIZE);
        player.gridPosition.y = ((player.position.y+PLAYER_CENTER_FIX) / TILE_SIZE);

        I_DetermineInFrontGrid();

        // Check for auto-callbacks upon collision
        int spriteID = R_GetValueFromSpritesMap(player.level, player.gridPosition.y, player.gridPosition.x);
        
        if(spriteID > 0 && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 3) && tomentdatapack.sprites[spriteID]->Callback != NULL)
        {
            // Call callback
            tomentdatapack.sprites[spriteID]->Callback(tomentdatapack.sprites[spriteID]->data);
                            
            // If the tapped sprite was a pickup, destroy it from the map after the player took it
            if(tomentdatapack.sprites[spriteID]->Callback == D_CallbackPickup)
            {
                R_SetValueFromSpritesMap(player.level, player.gridPosition.y, player.gridPosition.x, 0);
                R_SetValueFromCollisionMap(player.level, player.gridPosition.y, player.gridPosition.x, 0);
                G_SetObjectTMap(player.level, player.gridPosition.y, player.gridPosition.x, ObjT_Empty);
                O_GamePickPickup(player.level, player.gridPosition.x, player.gridPosition.y);
            }
        }

        // Check to restore the crosshair
        if(player.crosshairHit)
        {
            if(player.crosshairTimer->GetTicks(player.crosshairTimer) > CROSSHAIR_HIT_TIME_SECONDS*1000)
            {
                player.crosshairTimer->Stop(player.crosshairTimer);
                player.crosshairHit = false;
            }
        }
    }
    else
    {
        player.state = PSTATE_CLIMBING_LADDER;
        I_PlayerLadderMovements();
    }

    G_PlayerCheckPuddleDamage();
}


void G_PlayerCollisionCheck()
{
    //---------------------------------------------------
    // Player->Collision Map
    //---------------------------------------------------

    // Calculate the player position relative to cell
    float playerXCellOffset = (int)(player.position.x+PLAYER_CENTER_FIX) % TILE_SIZE;
    float playerYCellOffset = (int)(player.position.y+PLAYER_CENTER_FIX) % TILE_SIZE;

    // Collision detection (Walls and solid sprites)
    if(player.deltaPos.x > 0)
    {
        int coll = G_CheckCollisionMap(player.level, player.gridPosition.y, player.gridPosition.x+1);

        // Player is moving right, check if it's too right
        if(coll != 0 && playerXCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.x = 0;
    }
    else
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y, player.gridPosition.x-1);

        // Player is moving left
        if(coll != 0 && playerXCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.x = 0;
    }

    if(player.deltaPos.y < 0)
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y-1, player.gridPosition.x);

        // Player is going up
        if(coll != 0 && playerYCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.y = 0;
    }
    else
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y+1, player.gridPosition.x);

        // Player is going down
        if(coll != 0 && playerYCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.y = 0;
    }

    //---------------------------------------------------
    // Player->Dynamic Sprites
    //---------------------------------------------------
    
    // The circle the player has if the delta movement is applied 
    // Radius+2 to avoid flickering
    circle_t hypoteticalPlayerCircle = {player.collisionCircle.pos.x += player.deltaPos.x, player.collisionCircle.pos.y += player.deltaPos.y, player.collisionCircle.r+2};
    for(int i = 0; i < allDynamicSpritesLength; i++)
    {
        if(allDynamicSprites[i]->base.active && allDynamicSprites[i]->base.level == player.level)
        {
            dynamicSprite_t* cur = allDynamicSprites[i];

            // Check for collision
            // If there's collision, do not apply movement
            if(P_CheckCircleCollision(&hypoteticalPlayerCircle, &cur->base.collisionCircle) > 0)
            {
                player.deltaPos.x = 0;
                player.deltaPos.y = 0;
            }

            // Check if there is collision currently, this can happen when network lags and the AI is moved in front of the player
            if(P_CheckCircleCollision(&player.collisionCircle, &cur->base.collisionCircle) > 0)
            {
                player.deltaPos.x = 0;
                player.deltaPos.y = 0;

                // Even without moving, there's a collision, resolve it
                float solvingAngle = atan2(player.collisionCircle.pos.y - cur->base.collisionCircle.pos.y, player.collisionCircle.pos.x - cur->base.collisionCircle.pos.x);
                float distanceBetweenCircles = P_GetDistance(player.collisionCircle.pos.x, player.collisionCircle.pos.y, cur->base.collisionCircle.pos.x, cur->base.collisionCircle.pos.y);

                float distToSolve = player.collisionCircle.r + cur->base.collisionCircle.r - distanceBetweenCircles;

                // Move player away from colliding object
                float displacementX = cos(solvingAngle) * distToSolve;
                float displacementY = sin(solvingAngle) * distToSolve;
                player.position.x += displacementX;
                player.position.y += displacementY;
            }
        }
    }


    // Check collision with other player
    if(P_CheckCircleCollision(&hypoteticalPlayerCircle, &otherPlayerObject.base.collisionCircle) > 0)
    {
        player.deltaPos.x = 0;
        player.deltaPos.y = 0;
    }

}


//-------------------------------------
// Handles Input from the player 
//-------------------------------------
void G_InGameInputHandling(const uint8_t* keyboardState)
{
    if(!chatField.isFocus)
    {
        // Forward / backwards
        if(keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
            playerinput.input.y += 1.0f;
        else if(keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S])
            playerinput.input.y -= 1.0f;
        else
            playerinput.input.y = 0.0f;
        
        // Strafe right and left
        if(keyboardState[SDL_SCANCODE_A])
            playerinput.strafe.x = -1.0f; 
        else if(keyboardState[SDL_SCANCODE_D])
            playerinput.strafe.x = 1.0f; 
        else
            playerinput.strafe.x = 0.0f; 

        /* Player fly
        if(keyboardState[SDL_SCANCODE_LCTRL])
            if(player.z > 1)
                player.z -= 100 * deltaTime; 

        if(keyboardState[SDL_SCANCODE_LSHIFT])
            if(player.z < 191)
                player.z += 100 * deltaTime; 
        */

        /*
        if(keyboardState[SDL_SCANCODE_KP_MINUS])
        {
            player.attributes.curHealth -= 1.0f;
            player.z -= 1.0f;
        }

        if(keyboardState[SDL_SCANCODE_KP_PLUS])
        {
            player.attributes.curHealth += 1.0f;
            player.z += 1.0f;
        }

        if(keyboardState[SDL_SCANCODE_KP_3])
        {
            player.attributes.curMana -= 1.0f;
        }

        if(keyboardState[SDL_SCANCODE_KP_6])
        {
            player.attributes.curMana += 1.0f;
        }
        */


        //playerinput.input.x = SDL_clamp(playerinput.input.x, -1.0f , 1.0f);
        playerinput.input.y = SDL_clamp(playerinput.input.y, -1.0f , 1.0f);
    } 
    else
    {
        playerinput.strafe.x = 0;
        playerinput.input.y = 0;
    }
    
}


// FPS Images size are: 320x240
void G_PlayerRender(void)
{
    SDL_Rect screenPos = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect size = {(0), (0), 320, 240};

    // Select Animation
    SDL_Surface* curAnim;
    int curAnimLength = 0;
    int curAnimActionFrame = 0;
    float curAnimSpeedModifier = 0;

    switch(player.state)
    {
        case PSTATE_IDLE:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animIdle;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animIdleSheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animIdleActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animIdleSpeedModifier;
            break;

        case PSTATE_ATTACKING1:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animAttack;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animAttackSheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animAttackActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animAttackSpeedModifier;
            break;

        case PSTATE_CASTSPELL:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animCastSpell;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animCastSpellSheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animCastSpellActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animCastSpellkSpeedModifier;
            break;

        case PSTATE_DOINGSKILL1:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial1;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial1SheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial1ActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial1SpeedModifier;
            break;

        case PSTATE_DOINGSKILL2:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial2;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial2SheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial2ActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial2SpeedModifier;
            break;

        case PSTATE_DOINGSKILL3:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial3;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial3SheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial3ActionFrame;
            curAnimSpeedModifier = tomentdatapack.playersFP[player.curWeapon]->animations->animSpecial3SpeedModifier;
            break;
        
        default:
            curAnim = tomentdatapack.playersFP[player.curWeapon]->animations->animIdle;
            curAnimLength = tomentdatapack.playersFP[player.curWeapon]->animations->animIdleSheetLength;
            curAnimActionFrame = tomentdatapack.playersFP[player.curWeapon]->animations->animIdleActionFrame;
            curAnimSpeedModifier = 0;
            break;
    }

    if(player.animPlay)
    {
        if(player.animPlayOnce)
        {
            if(curAnimLength > 0)
                player.animFrame = ((int)floor(player.animTimer->GetTicks(player.animTimer) / (ANIMATION_SPEED_DIVIDER+curAnimSpeedModifier)) % curAnimLength);

            // Spawn projectile at the right frame
            if(player.state == PSTATE_CASTSPELL && player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
            {
                // Spawn a projectile   
                // Angle must be the same for each resolution
                float angle = (player.verticalHeadMovement / MAX_VERTICAL_HEAD_MOV) * 75.0f;
                uint32_t networkID = REPL_GenerateNetworkID();
                G_SpawnProjectile(networkID, player.curSpell, player.angle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, NULL, false);
                player.hasCasted = true;
                player.hasToCast = false;

                // Spawn it online
                O_GameSpawnProjectile(networkID, player.curSpell, player.angle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, 0);
            }

            // Attack
            if(player.state == PSTATE_ATTACKING1 && player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
            {
                I_PlayerAttack(0);
                player.hasCasted = true;
                player.hasToCast = false;
            }

            if(player.state == PSTATE_DOINGSKILL1)
            {
                switch(thisPlayer.selectedClass)
                {
                    case CLASS_TANK:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            I_PlayerAttack(1);
                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }

                    case CLASS_HEALER:
                    {
                        // Gain health
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            G_PlayerGainHealth(20.0f * player.attributes.spellPower);
                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                    }

                    case CLASS_DPS:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            I_PlayerAttack(2);
                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }
                }
            }

            if(player.state == PSTATE_DOINGSKILL2)
            {
                switch(thisPlayer.selectedClass)
                {
                    case CLASS_TANK:
                    {
                        player.isInvulnerable = true;
                        break;
                    }

                    case CLASS_HEALER:
                    {
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            // Spawn a projectile   
                            // Angle must be the same for each resolution
                            float angle = (player.verticalHeadMovement / MAX_VERTICAL_HEAD_MOV) * 75.0f;
                            uint32_t networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, SPELL_CONCENTRATED_HEAL, player.angle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, NULL, false);
                            player.hasCasted = true;
                            player.hasToCast = false;

                            // Spawn it online
                            O_GameSpawnProjectile(networkID, SPELL_CONCENTRATED_HEAL, player.angle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, 0);
                        }

                        break;
                    }

                    case CLASS_DPS:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            float projAngle = player.angle;
                            // Spawn a projectile   
                            // Angle must be the same for each resolution
                            float angle = (player.verticalHeadMovement / MAX_VERTICAL_HEAD_MOV) * 75.0f;
                            uint32_t networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, NULL, false);
                            // Spawn it online
                            O_GameSpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, 0);
                            
                            projAngle += 0.174533;
                            FIX_ANGLES(projAngle);

                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, NULL, false);
                            // Spawn it online
                            O_GameSpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, 0);

                            projAngle -= 0.349066;
                            FIX_ANGLES(projAngle);

                            networkID = REPL_GenerateNetworkID();
                            G_SpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, NULL, false);
                            // Spawn it online
                            O_GameSpawnProjectile(networkID, SPELL_SWORD_PROJECTILE, projAngle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, player.z-(HALF_TILE_SIZE), angle, true, 0);


                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }
                }
            }

            if(player.state == PSTATE_DOINGSKILL3)
            {
                switch(thisPlayer.selectedClass)
                {
                    case CLASS_TANK:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            unsigned puddlesLength = 9;
                            packedpuddle_t puddles[puddlesLength];
                            int count = 0;

                            for(int x = -1; x <= 1; x++)
                            {
                                for(int y = -1; y <= 1; y++)
                                {
                                    bool already = false;

                                    // Check if there is already a puddle
                                    mappudlle_t* puddle = activeMapPuddlesHead;
                                    while(puddle != NULL)
                                    {
                                        if(puddle->gridX == player.gridPosition.x+x && puddle->gridY == player.gridPosition.y+y)
                                        {
                                            already = true;
                                            break;
                                        }
                                        puddle = puddle->next;
                                    }

                                    if(!already)
                                    {
                                        puddles[count] = G_SpawnMapPuddle(REPL_GenerateNetworkID(), player.gridPosition.x+x, player.gridPosition.y+y, true, false, 50.0f, 5000, player.level, TEXTURE_IceConsacrated, false);
                                        count++;
                                    }
                                }
                            }

                            O_GameSpawnPuddles(count, puddles);

                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }

                    case CLASS_HEALER:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            G_PlayerGainHealth(100.0f * player.attributes.spellPower);
                            O_GameHealOther(100.0f    * player.attributes.spellPower);

                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }

                    case CLASS_DPS:
                    {
                        // Attack
                        if(player.animFrame == curAnimActionFrame && player.hasToCast && !player.hasCasted)
                        {
                            I_PlayerAttack(3);
                            player.hasCasted = true;
                            player.hasToCast = false;
                        }
                        break;
                    }
                }
            }

            // Prevent loop
            if(player.animFrame >= curAnimLength-1)
            {
                player.animPlay = false;
                
                // Go back to idle
                if(player.state == PSTATE_ATTACKING1 ||
                   player.state == PSTATE_CASTSPELL ||
                   player.state == PSTATE_DOINGSKILL1 ||
                   player.state == PSTATE_DOINGSKILL2 ||
                   player.state == PSTATE_DOINGSKILL3)
                   {
                    if(player.state == PSTATE_DOINGSKILL2 && thisPlayer.selectedClass == CLASS_TANK)
                            player.isInvulnerable = false;

                        player.state = PSTATE_IDLE;
                        player.hasToCast = false;
                   }
            }
        }
        else
        {
            // Allow loop
             if(curAnimLength > 0)
                player.animFrame = ((int)floor(player.animTimer->GetTicks(player.animTimer) / ANIMATION_SPEED_DIVIDER) % curAnimLength);
        }

        size.x = (320) * player.animFrame; 
    }

    // Blit FP
    R_BlitIntoScreenScaled(&size, curAnim, &screenPos);

    // Show FPS
    if(showFPS)
    {
        snprintf(fpsText, 16, "%d", (int)readOnlyFPS);
        T_DisplayText(FONT_BLKCRY, fpsText, 760, 20);
    }
}


static void G_PlayerUIRender_ThisPlayer()
{
     // Player name
    T_DisplayText(FONT_BLKCRY, thisPlayer.name, 105, 5);
    
    // Player potrait
    // Display this player potrait
    SDL_Rect thisPotraitScreenPos = {2, 5, 90, 90};
    SDL_Rect thisPotraitSize = {(0), (0), 128, 128};

    R_BlitIntoScreenScaled(&thisPotraitSize, tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK+thisPlayer.selectedClass]->texture, &thisPotraitScreenPos);

    // HEALTH BAR
    SDL_Rect healthbarEmptyScreenPos = {105, 37, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect healthbarEmptySize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    R_BlitIntoScreenScaled(&healthbarEmptySize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY]->texture, &healthbarEmptyScreenPos);

    SDL_Rect healthbarFillScreenPos = {105, 37, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect healthbarFillSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    // Fill size.x of 0 means full health
    // Fill size.x of 160 means 0 health
    // H = 100 means B = 0
    // H = 0 means B = 160
    // The formula is the equation of the line between these two points
    healthbarFillSize.x = -160/player.attributes.maxHealth* player.attributes.curHealth + 160;

    // Fix bar border
    if(healthbarFillSize.x == 2)
        healthbarFillScreenPos.x+=2;

    if(healthbarFillSize.x >=3)
        healthbarFillScreenPos.x+=3;

    R_BlitIntoScreenScaled(&healthbarFillSize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL]->texture, &healthbarFillScreenPos);

    // MANA BAR
    SDL_Rect manabarEmptyScreenPos = {105, 66, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect manabarEmptySize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    R_BlitIntoScreenScaled(&manabarEmptySize, tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY]->texture, &manabarEmptyScreenPos);

    SDL_Rect manabarFillScreenPos = {105, 66, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect manabarFillSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    manabarFillSize.x = -160/player.attributes.maxMana* player.attributes.curMana + 160;

    // Fix bar border
    if(manabarFillSize.x == 2)
        manabarFillScreenPos.x+=2;
        
    if(manabarFillSize.x >= 3)
        manabarFillScreenPos.x+=3;

    R_BlitIntoScreenScaled(&manabarFillSize, tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL]->texture, &manabarFillScreenPos);

    // Render the selected weapon
    SDL_Surface* curWeapon;
    SDL_Surface* curSpell;

    // Select the weapon
    switch(player.curWeapon)
    {
        case PLAYER_FP_HANDS:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture;
            break;

        case PLAYER_FP_AXE:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_AXE]->texture;
            break;

        case PLAYER_FP_GREATSWORD:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_GREATSWORD]->texture;
            break;

        case PLAYER_FP_MACE:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_MACE]->texture;
            break;

        default:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture;
            break;
    }

    // Select the spell
    switch(player.curSpell)
    {
        case SPELL_NULL:
            curSpell = NULL;
            break;

        case SPELL_FIREBALL1:
            curSpell = tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_FIREBALL1]->texture;
            break;

        case SPELL_ICEDART1:
            curSpell = tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_ICEDART1]->texture;
            break;

        default:
            curSpell = NULL;
            break;
    }

    // Render icons

    // Render weapon
    SDL_Rect weaponIconScreenPos = {105, 95, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect weaponIconSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    if(curWeapon != NULL)
        R_BlitIntoScreenScaled(&weaponIconSize, curWeapon, &weaponIconScreenPos);

    // Render spell icon
    SDL_Rect spellIconScreenPos = {143, 95, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect spellIconSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    if(curSpell != NULL)
        R_BlitIntoScreenScaled(&spellIconSize, curSpell, &spellIconScreenPos);

    // Render skills
    SDL_Rect skill1ScreenPos = {240, 530, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect skill1Size = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    if(player.skills[0].timer->GetTicks(player.skills[0].timer) >= player.skills[0].cooldown)
        R_BlitIntoScreenScaled(&skill1Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM+player.skills[0].skillID]->texture, &skill1ScreenPos);
    else
    {
        R_BlitIntoScreenScaled(&skill1Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY]->texture, &skill1ScreenPos);

        char arr[32];
        sprintf(arr, "%.1f", (player.skills[0].cooldown-player.skills[0].timer->GetTicks(player.skills[0].timer)) / (float)1000);
        T_DisplayText(FONT_BLKCRY, arr, 245, 545);
    }

    SDL_Rect skill2ScreenPos = {390, 530, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect skill2Size = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    if(player.skills[1].timer->GetTicks(player.skills[1].timer) >= player.skills[1].cooldown)
        R_BlitIntoScreenScaled(&skill2Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM+player.skills[1].skillID]->texture, &skill2ScreenPos);
    else
    {
        R_BlitIntoScreenScaled(&skill2Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY]->texture, &skill2ScreenPos);

        char arr[32];
        sprintf(arr, "%.1f", (player.skills[1].cooldown-player.skills[1].timer->GetTicks(player.skills[1].timer)) / (float)1000);
        T_DisplayText(FONT_BLKCRY, arr, 395, 545);
    }

    SDL_Rect skill3ScreenPos = {535, 530, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect skill3Size = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

    if(player.skills[2].timer->GetTicks(player.skills[2].timer) >= player.skills[2].cooldown)
        R_BlitIntoScreenScaled(&skill3Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM+player.skills[2].skillID]->texture, &skill3ScreenPos);
    else
    {
        R_BlitIntoScreenScaled(&skill3Size, tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY]->texture, &skill3ScreenPos);

        char arr[32];
        sprintf(arr, "%.1f", (player.skills[2].cooldown-player.skills[2].timer->GetTicks(player.skills[2].timer)) / (float)1000);
        T_DisplayText(FONT_BLKCRY, arr, 540, 545);
    }    
    
    // Render text input field if need to
    if(chatField.isFocus)
    {
        SDL_Rect textFieldScreenpos = {chatField.x, chatField.y, chatField.w, chatField.h};
        SDL_Rect textFieldSize = {(0), (0), 309, 52};
        R_BlitIntoScreenScaled(&textFieldSize, chatField.isFocus ? tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01_ACTIVE]->texture : tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01]->texture, &textFieldScreenpos);

        T_DisplayTextScaled(FONT_BLKCRY, chatField.text, chatField.x, chatField.y, chatField.textScale);
    }

    // Render and handle chat
    incomingchatmessage_t* last = NULL;
    incomingchatmessage_t* cur = chatMsgsHead;
    while(cur != NULL)
    {
        if(cur->timer->GetTicks(cur->timer) >= cur->duration)
        {
            // Destroy this
            if(chatMsgsHead == cur)
                chatMsgsHead = cur->next;

            if(cur->next != NULL)
                cur->next->previous = cur->previous;

            if(cur->previous != NULL)
                cur->previous->next = cur->next;

            // Restore floor texture
            incomingchatmessage_t* dead = cur;
            cur = cur->next;
            free(dead->timer);
            free(dead);
            continue;
        }
        else
        {
            T_DisplayTextScaled(cur->fontID, cur->msg, cur->x, cur->y, cur->scale);
        }

        last = cur;
        cur = cur->next;
    }

    // Order Y
    int initialYPos = INCOMING_CHAT_INITIAL_Y_POS;
    int additivePos = 0;
    while(last != NULL)
    {
        last->y = initialYPos + additivePos;

        // Clip, should not happen since messages are destroyed quickly
        if(last->y < 0)
            last->y = 0;

        additivePos -= INCOMING_CHAT_DEFAULT_MESSAGES_SPACING;
        
        last = last->previous;
    }
}   

// Fill needs to account for other player health
static void G_PlayerUIRender_OtherPlayer()
{
     // Player name
    T_DisplayTextScaled(FONT_BLKCRY, otherPlayer.name, 100, 145, .8f);

    // Player potrait
    // Display this player potrait
    SDL_Rect thisPotraitScreenPos = {2, 145, 70, 70};
    SDL_Rect thisPotraitSize = {(0), (0), 128, 128};

    R_BlitIntoScreenScaled(&thisPotraitSize, tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK+otherPlayer.selectedClass]->texture, &thisPotraitScreenPos);

    // HEALTH BAR
    SDL_Rect healthbarEmptyScreenPos = {85, 172, 122, 18};
    SDL_Rect healthbarEmptySize = {(0), (0), 161, 24};

    R_BlitIntoScreenScaled(&healthbarEmptySize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY]->texture, &healthbarEmptyScreenPos);

    SDL_Rect healthbarFillScreenPos = {85, 172, 122, 18};
    SDL_Rect healthbarFillSize = {(0), (0), 161, 24};

    healthbarFillSize.x = -160/otherPlayerObject.attributes.maxHealth* otherPlayerObject.attributes.curHealth + 160;

    // Fix bar border
    if(healthbarFillSize.x == 2)
        healthbarFillScreenPos.x+=2;

    if(healthbarFillSize.x >=3)
        healthbarFillScreenPos.x+=2;

    R_BlitIntoScreenScaled(&healthbarFillSize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL]->texture, &healthbarFillScreenPos);

    // MANA BAR
    SDL_Rect manabarEmptyScreenPos = {84, 195, 122, 18};
    SDL_Rect manabarEmptySize = {(0), (0), 161, 24};

    R_BlitIntoScreenScaled(&manabarEmptySize, tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY]->texture, &manabarEmptyScreenPos);

    SDL_Rect manabarFillScreenPos = {84, 195, 122, 18};
    SDL_Rect manabarFillSize = {(0), (0), 161, 24};

    manabarFillSize.x = -161/otherPlayerObject.attributes.maxMana* otherPlayerObject.attributes.curMana + 161;

    // Fix bar border
    if(manabarFillSize.x == 2)
        manabarFillScreenPos.x+=2;
        
    if(manabarFillSize.x >= 3)
        manabarFillScreenPos.x+=2;

    R_BlitIntoScreenScaled(&manabarFillSize, tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL]->texture, &manabarFillScreenPos);

    // Render the selected weapon
    SDL_Surface* curWeapon;
    SDL_Surface* curSpell;

    // Select the weapon
    switch(otherPlayer.curWeapon)
    {
        case PLAYER_FP_HANDS:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture;
            break;

        case PLAYER_FP_AXE:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_AXE]->texture;
            break;

        case PLAYER_FP_GREATSWORD:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_GREATSWORD]->texture;
            break;

        case PLAYER_FP_MACE:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_MACE]->texture;
            break;

        default:
            curWeapon = tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture;
            break;
    }

    // Select the spell
    switch(otherPlayer.curSpell)
    {
        case SPELL_NULL:
            curSpell = NULL;
            break;

        case SPELL_FIREBALL1:
            curSpell = tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_FIREBALL1]->texture;
            break;

        case SPELL_ICEDART1:
            curSpell = tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_ICEDART1]->texture;
            break;

        default:
            curSpell = NULL;
            break;
    }

    // Render icons

    // Render weapon
    SDL_Rect weaponIconScreenPos = {85, 220, 26, 26};
    SDL_Rect weaponIconSize = {(0), (0), 32, 32};

    if(curWeapon != NULL)
        R_BlitIntoScreenScaled(&weaponIconSize, curWeapon, &weaponIconScreenPos);

    // Render spell icon
    SDL_Rect spellIconScreenPos = {120, 220, 26, 26};
    SDL_Rect spellIconSize =  {(0), (0), 32, 32};

    if(curSpell != NULL)
        R_BlitIntoScreenScaled(&spellIconSize, curSpell, &spellIconScreenPos);
}

// -----------------------------------
// Renders Player's UI
// -----------------------------------
void G_PlayerUIRender(void)
{
    //printf("COORD: %d | %d\n", player.gridPosition.x, player.gridPosition.y);
    G_PlayerUIRender_ThisPlayer();
    G_PlayerUIRender_OtherPlayer();

    // Render crosshair
    SDL_Rect crosshairScreenPos = {(SCREEN_WIDTH / 2) - 6, (SCREEN_HEIGHT / 2) - 6, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect crosshairSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};
    
    R_BlitIntoScreenScaled(&crosshairSize, player.crosshairHit ? tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR_HIT]->texture : tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR]->texture, &crosshairScreenPos);

    // Boss UI
    if(player.isFightingBoss && player.bossFighting != NULL)
    {
        // HEALTH BAR
        SDL_Rect bossHealthbarEmptyScreenPos = {160, 470, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_Rect bossHealthbarEmptySize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

        R_BlitIntoScreenScaled(&bossHealthbarEmptySize, tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_EMPTY]->texture, &bossHealthbarEmptyScreenPos);

        SDL_Rect bossHealthbarFillScreenPos = {160, 470, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_Rect bossHealthbarFillSize = {(0), (0), SCREEN_WIDTH, SCREEN_HEIGHT};

        // Fill size.x of 0 means full health
        // Fill size.x of 460 means 0 health (same size of the healthbar image x)
        // H = 100 means B = 0
        // H = 0 means B = 160
        // The formula is the equation of the line between these two points
        bossHealthbarFillSize.x = -460/player.bossFighting->attributes.maxHealth* player.bossFighting->attributes.curHealth + 460;

        // Fix bar border
        if(bossHealthbarFillSize.x == 2)
            bossHealthbarFillScreenPos.x+=2;

        if(bossHealthbarFillSize.x >=3)
            bossHealthbarFillScreenPos.x+=3;

        R_BlitIntoScreenScaled(&bossHealthbarFillSize, tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_FILL]->texture, &bossHealthbarFillScreenPos);

        // Boss Name
        T_DisplayText(FONT_BLKCRY, player.bossFighting->base.name, 160, 440);
    }
}
//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_InGameInputHandlingEvent(SDL_Event* e)
{
    if(!chatField.isFocus)
    {
        switch(e->type)
        {
            case SDL_MOUSEMOTION:
                playerinput.mouseInput.x = e->motion.xrel;
                playerinput.mouseInput.y = -e->motion.yrel;
                break;

            case SDL_MOUSEBUTTONUP:
                if(e->button.button == SDL_BUTTON_LEFT)
                {
                    if(G_PlayerCanAttack())
                    {
                        G_PlayerPlayAnimationOnce(ANIM_ATTACK1);
                        player.hasToCast = true;
                        player.hasCasted = false;
                    }
                }
                break;


            case SDL_KEYUP:
                // Space player's interacions
                if(e->key.keysym.sym == SDLK_SPACE && player.state != PSTATE_CLIMBING_LADDER)
                {
                    // Interactions
                    objectType_e objType = G_GetFromObjectTMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);

                    if(objType == ObjT_Door)
                    {
                        printf("Tapped a door\n");
                        
                        if(player.isFightingBoss && player.bossFighting->bossPreventOpeningDoorsWhileFighting)
                        {
                            alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
                            R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X, ALERT_MESSAGE_DEF_Y, "You can't do that now.", 2.0f, 1.0f);
                        }
                        else
                        {
                            // Open/Close
                            if(G_GetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Closed || G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Closing)
                                G_SetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, DState_Opening);
                            
                            else if(G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Open || G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Opening)
                                G_SetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, DState_Closing);
                        }
                    }
                    else if(objType == ObjT_Empty)
                    {
                        printf("Tapped an empty space\n");
                    }
                    else if(objType == ObjT_Sprite)
                    {
                        printf("Tapped a sprite\n");
                        
                        int spriteID = R_GetValueFromSpritesMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);
                        if(tomentdatapack.sprites[spriteID]->Callback != NULL && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 3) == 0)
                        {
                            tomentdatapack.sprites[spriteID]->Callback(tomentdatapack.sprites[spriteID]->data);

                            // If the tapped sprite was a pickup, destroy it from the map after the player took it
                            if(tomentdatapack.sprites[spriteID]->Callback == D_CallbackPickup)
                            {
                                R_SetValueFromSpritesMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, 0);
                                R_SetValueFromCollisionMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, 0);
                                G_SetObjectTMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, ObjT_Empty);
                            }
                            else if(tomentdatapack.sprites[spriteID]->Callback == D_CallbackUseAltar)
                            {
                                R_SetValueFromSpritesMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, S_AltarEmpty);
                                G_SetObjectTMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, ObjT_Sprite);
                            }
                        }
                    }
                    else if(objType == ObjT_Wall)
                    {
                        printf("Tapped a wall\n");
                    }
                    else if(objType == ObjT_Trigger)
                    {
                        printf("Tapped a trigger\n");
                        
                        if(player.isFightingBoss && player.bossFighting->bossPreventActivatingTriggersWhileFighting)
                        {
                            alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
                            R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X, ALERT_MESSAGE_DEF_Y, "You can't do that now.", 2.0f, 1.0f);
                        }
                        else
                        {
                            wallObject_t* object = R_GetWallObjectFromMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);
                            if(tomentdatapack.walls[object->assetID]->Callback != NULL)
                            {
                                // Prevent the player from climbing ladders if he's in a bossfight that does not allow ladders
                                if(player.isFightingBoss && player.bossFighting->bossPreventClimbingLaddersWhileFighting &&
                                    (object->assetID == W_WallLadder || object->assetID == W_WallLadderDown))
                                {
                                    alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
                                    R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X, ALERT_MESSAGE_DEF_Y, "You can't do that now.", 2.0f, 1.0f);
                                }
                                else
                                    tomentdatapack.walls[object->assetID]->Callback(object->data);
                            }
                        }
                        
                    }
                }
                else if(e->key.keysym.sym == SDLK_c)
                {
                    if(G_PlayerCanAttack())
                        I_PlayerCastSpell(player.curSpell);
                }
                else if(e->key.keysym.sym == SDLK_ESCAPE)
                {
                    G_SetMenu(&MainMenu);
                    A_ChangeState(GSTATE_MENU);
                }

                /*
                else if(e->key.keysym.sym == SDLK_F1)
                {
                    //debugRendering = !debugRendering;
                }
                */
                /*
                // reloads the map (used for building maps)
                if(e->key.keysym.sym == SDLK_F5)
                {
                    int curx = player.position.x;
                    int cury = player.position.y;
                    int curz = player.z;
                    float curAngle = player.angle;
                    float curVerticalMovement = player.verticalHeadMovement;

                    G_InitGame();

                    player.position.x = curx;
                    player.position.y = cury;
                    player.z = curz;
                    player.angle = curAngle;
                    player.verticalHeadMovement = curVerticalMovement;
                }

                else if(e->key.keysym.sym == SDLK_F2)
                {
                    r_debugPathfinding = true;

                    // Update minimap
                    R_DrawMinimap();

                    vector2Int_t deb = {11, 9};
                    if(allDynamicSprites[0] != NULL)
                        G_PerformPathfindingDebug(allDynamicSprites[0]->base.level, allDynamicSprites[0]->base.gridPos, deb);
                    else
                    {
                        printf("Tried to debug pathfinding between first dynamic sprite and player, but no dynamic sprite is present in this map... Performing Pathfinding from center of the map to the player...\n");
                        vector2Int_t gridpos = {floor(MAP_WIDTH / 2), floor(MAP_HEIGHT / 2)};
                        G_PerformPathfindingDebug(player.level, gridpos, deb);
                    }

                    r_debugPathfinding = false;
                }
                */

                // Skills
                else if(G_PlayerCanAttack() && e->key.keysym.sym == SDLK_1)
                {
                    // Check cooldown
                    if(player.skills[0].timer->GetTicks(player.skills[0].timer) >= player.skills[0].cooldown)
                    {
                        // DO SKILL
                        G_PlayerPlayAnimationOnce(ANIM_SPECIAL1);
                        player.hasToCast = true;
                        player.hasCasted = false;

                        player.skills[0].timer->Start(player.skills[0].timer);
                    }
                }
                else if(G_PlayerCanAttack() && e->key.keysym.sym == SDLK_2)
                {
                    // Check cooldown
                    if(player.skills[1].timer->GetTicks(player.skills[1].timer) >= player.skills[1].cooldown)
                    {
                        // DO SKILL
                        G_PlayerPlayAnimationOnce(ANIM_SPECIAL2);
                        player.hasToCast = true;
                        player.hasCasted = false;
                        player.skills[1].timer->Start(player.skills[1].timer);
                    }
                }
                else if(G_PlayerCanAttack() && e->key.keysym.sym == SDLK_3)
                {
                    // Check cooldown
                    if(player.skills[2].timer->GetTicks(player.skills[2].timer) >= player.skills[2].cooldown)
                    {
                        // DO SKILL
                        G_PlayerPlayAnimationOnce(ANIM_SPECIAL3);
                        player.hasToCast = true;
                        player.hasCasted = false;

                        player.skills[2].timer->Start(player.skills[2].timer);
                    }
                }
                else if(G_PlayerCanAttack() && player.hasIceDart && e->key.keysym.sym == SDLK_4)
                    G_PlayerSetSpell(SPELL_ICEDART1);
                else if(G_PlayerCanAttack() &&  player.hasFireball && e->key.keysym.sym == SDLK_5)
                    G_PlayerSetSpell(SPELL_FIREBALL1);
                else if(e->key.keysym.sym == SDLK_RETURN)
                {
                    if(!chatField.isFocus)
                    {
                        isEditingTextField = true;
                        textFieldEditing = &chatField;
                        chatField.isFocus = true;
                        chatField.text[0] = '\0';

                        SDL_StartTextInput();
                    }
                }

            break;
        }
    }
    else
    {
        // Focus on chat
        switch(e->type)
        {
            case SDL_KEYUP:
                if(e->key.keysym.sym == SDLK_BACKSPACE && isEditingTextField && textFieldEditing != NULL)
                {
                    int len = strlen(textFieldEditing->text);
                    if(len > 0)
                        textFieldEditing->text[len - 1] = '\0'; // Set the last character to null terminator
                }
                // Send message / close chat
                else if(e->key.keysym.sym == SDLK_RETURN)
                {
                    if(strlen(chatField.text) > 0)
                    {
                        // Send message
                        O_GameSendChatMessage(chatField.text);

                        char tempTxt[INCOMING_CHAT_MSG_MAX_LENGTH] = "You: ";
                        strcat(tempTxt, chatField.text);
                        G_SpawnIncomingChatMessage(FONT_BLKCRY, 2, INCOMING_CHAT_INITIAL_Y_POS, tempTxt, INCOMING_CHAT_DEFAULT_DURATION, INCOMING_CHAT_DEFAULT_SCALE);
                    }

                    // Restore
                    isEditingTextField = false;
                    textFieldEditing = NULL;
                    chatField.isFocus = false;
                    SDL_StopTextInput();
                }
                break; 

            case SDL_TEXTINPUT:
                if(isEditingTextField && textFieldEditing != NULL)
                {
                    int len = strlen(textFieldEditing->text);

                    if(len < TEXTFIELD_MAX_LENGTH && len < textFieldEditing->textLimit)
                        strcat(textFieldEditing->text, e->text.text);
                }
            break;

        }
    }
}

void G_PlayerTakeDamage(float dmg)
{
    if(player.isInvulnerable)
        return;

    player.attributes.curHealth -= dmg;

    if(player.attributes.curHealth <= 0.0f)
    {
        G_PlayerDeath();
    }
    
    player.attributes.curHealth = SDL_clamp(player.attributes.curHealth, 0, player.attributes.maxHealth);
}

void G_PlayerGainHealth(float amount)
{
    player.attributes.curHealth += amount;
    player.attributes.curHealth = SDL_clamp(player.attributes.curHealth, 0, player.attributes.maxHealth);
}

void G_PlayerGainMana(float amount)
{
    player.attributes.curMana += amount;
    player.attributes.curMana = SDL_clamp(player.attributes.curMana, 0, player.attributes.maxMana);
}

void G_PlayerDeath()
{
    O_GameSendDeathPacket();
    thisPlayer.dead = true;

    // Death screen will be shown when O_GameSendDeathPacket is sent
}

//-------------------------------------
// Checks the collision map at player's level and returns what found
//-------------------------------------
int G_CheckCollisionMap(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return currentMap.collisionMapLevel0[y][x];

        case 1:
            return currentMap.collisionMapLevel1[y][x];

        case 2:
            return currentMap.collisionMapLevel2[y][x];
    }
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_GetDoorState(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return doorstateLevel0[y][x];

        case 1:
            return doorstateLevel1[y][x];

        case 2:
            return doorstateLevel2[y][x];
    }
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_SetDoorState(int level, int y, int x, doorstate_e state)
{
    switch(level)
    {
        case 0:
            doorstateLevel0[y][x] = state;
            break;

        case 1:
            doorstateLevel1[y][x] = state;
            break;

        case 2:
            doorstateLevel2[y][x] = state;
            break;
    }

    // Update online
    O_GameSetDoorState(level, x, y, state);
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
float G_GetDoorPosition(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return doorpositionsLevel0[y][x];

        case 1:
            return doorpositionsLevel1[y][x];

        case 2:
            return doorpositionsLevel2[y][x];
    }
}

//-------------------------------------
// Checks object T map at player's level and returns what found
//-------------------------------------
int G_GetFromObjectTMap(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return currentMap.objectTMapLevel0[y][x];

        case 1:
            return currentMap.objectTMapLevel1[y][x];

        case 2:
            return currentMap.objectTMapLevel2[y][x];
    }
}

//-------------------------------------
// Sets object T map at player's level
//-------------------------------------
void G_SetObjectTMap(int level, int y, int x, int value)
{
    switch(level)
    {
        case 0:
            currentMap.objectTMapLevel0[y][x] = value;

        case 1:
            currentMap.objectTMapLevel1[y][x] = value;

        case 2:
            currentMap.objectTMapLevel2[y][x] = value;
    }
}

static void I_DetermineInFrontGrid(void)
{
    // Update front grid pos
    // Bottom cell
    if(player.angle < (2*M_PI)/3 && player.angle > M_PI/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x;
        player.inFrontGridPosition.y = player.gridPosition.y+1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Left 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

        // Set extra forward right
        I_SetAttackCone(5, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y+1);

        // Set extra forward left
        I_SetAttackCone(6, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y+1);
        
    }
    // Top cell
    else if(player.angle > (4*M_PI)/3 && player.angle < (5*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x;
        player.inFrontGridPosition.y = player.gridPosition.y -1;

        // Prevent going outside boundaries
        if(player.inFrontGridPosition.y < 0)
            player.inFrontGridPosition.y = 0;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Left 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Set extra forward right
        I_SetAttackCone(5, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y-1);

        // Set extra forward left
        I_SetAttackCone(6, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y-1);
    }
    // Right cell
    else if(player.angle < M_PI/6 || player.angle > (11*M_PI) / 6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Set extra forward up
        I_SetAttackCone(5, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y-1);

        // Set extra forward down
        I_SetAttackCone(6, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y+1);
    }
    // Left Cell
    else if(player.angle > (5*M_PI)/6 && player.angle < (7*M_PI)/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y;

        if(player.inFrontGridPosition.x < 0)
            player.inFrontGridPosition.x = 0;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Set extra forward up
        I_SetAttackCone(5, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y-1);

        // Set extra forward down
        I_SetAttackCone(6, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y+1);
    }
    // Top Right Cell
    else if(player.angle < (11*M_PI) / 6 && player.angle > (5*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y -1;
        
        // Prevent going outside boundaries
        if(player.inFrontGridPosition.y < 0)
            player.inFrontGridPosition.y = 0;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Left
        I_SetAttackCone(2, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y-1);

        // Set extra forward right
        I_SetAttackCone(5, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Set extra left
        I_SetAttackCone(6, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);
    }
    // Top Left Cell
    else if(player.angle > (7*M_PI) / 6 && player.angle < (4*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y -1;

        // Prevent going outside boundaries
        if(player.inFrontGridPosition.y < 0)
            player.inFrontGridPosition.y = 0;
        if(player.inFrontGridPosition.x < 0)
            player.inFrontGridPosition.x = 0;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y-1);

        // Set extra forward right
        I_SetAttackCone(5, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Set extra forward down
        I_SetAttackCone(6, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

    }
    // Bottom Right Cell
    else if(player.angle < M_PI/3 && player.angle > M_PI/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y +1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Left 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y+1);

        // Set extra forward up
        I_SetAttackCone(5, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Set extra forward left
        I_SetAttackCone(6, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);

    }
    // Bottom Left Cell
    else if(player.angle > (2*M_PI)/3 && player.angle < (M_PI*5)/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y +1;

        if(player.inFrontGridPosition.x < 0)
            player.inFrontGridPosition.x = 0;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Right 
        I_SetAttackCone(3, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Set extra forward
        I_SetAttackCone(4, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y+1);

        // Set extra forward up
        I_SetAttackCone(5, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Set extra forward right
        I_SetAttackCone(6, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);
    }
}

void G_PlayerPlayAnimationOnce(objectanimationsID_e animID)
{
    player.animTimer->Start(player.animTimer);
    player.animPlayOnce = true;
    player.animFrame = 0;

    switch(animID)
    {
        case ANIM_ATTACK1:
            player.state = PSTATE_ATTACKING1;
            break;

        case ANIM_CAST_SPELL:
            player.state = PSTATE_CASTSPELL;
            break;

        case ANIM_SPECIAL1:
            player.state = PSTATE_DOINGSKILL1;
            break;

        case ANIM_SPECIAL2:
            player.state = PSTATE_DOINGSKILL2;
            break;

        case ANIM_SPECIAL3:
            player.state = PSTATE_DOINGSKILL3;
            break;
    }

    player.animPlay = true;
}

void G_PlayerDrainMana(float amount)
{
    player.attributes.curMana -= amount;

    if(player.attributes.curMana < 0.0f)
        player.attributes.curMana = 0;
}

static bool I_PlayerAttack(int attackType)
{
    dynamicSprite_t* ai;

    for(int i = 0; i < ATTACK_CONE_SIZE; i++)
    {
        bool found;
        ai = G_GetFromDynamicSpriteMap(player.level, player.attackConeGridPos[i].y, player.attackConeGridPos[i].x);
        
        // We found one
        if(ai != NULL)
        {   
            // Check if AI is in front
            float anglediff = (int)((player.angle * (180/M_PI)) - ai->base.angle + 180 + 360) % 360 - 180;
            bool inFront = (anglediff <= ATTACK_CONE_MAX_DIFF && anglediff>=-ATTACK_CONE_MAX_DIFF);
            bool inRange = (ai->base.dist <= player.weaponDistance);

            if(!inFront || !inRange)
                ai = NULL;
            else
                break;
        }
    }

    float damage = 15.0f;

    if(attackType == 0)
    {
        switch(player.curWeapon)
        {
            case PLAYER_FP_HANDS:
                damage = 15.0f;
                break;

            case PLAYER_FP_AXE:
                damage = 33.5f;
                break;

            case PLAYER_FP_MACE:
                damage = 55.5f;
                break;

            case PLAYER_FP_GREATSWORD:
                damage = 100.0f;
                break;

            default:
                damage = 15.0f;
                break;
        }
    }
    else
    {
        // Tank shield slam
        if(attackType == 1)
            damage = 100.0f;
        // DPS cheap shot
        else if(attackType == 2)
            damage = 120.0f;
        // DPS split
        else if(attackType == 3)
            damage = 200.0f;
    }
    

    if(ai != NULL && ai->canBeHit)
    {
        printf("Hit an enemy.\n");

        player.crosshairHit = true;
        player.crosshairTimer->Start(player.crosshairTimer);

        if(ai->isAlive)
        {
            G_AITakeDamage(ai, damage, true);
            bool died = (ai->attributes.curHealth <= 0.0f);
            O_GameAITakeDamage(ai->networkID, damage, died);
        }
        
        return true;
    }
    else
    {
        printf("Hit the air.\n");
        return false;
    }
}

static bool I_PlayerCastSpell(playerSpells_e spell)
{
    float manaNeeded = 9999999.9f;

    // Checks if the passed spell exists, it does if the next switch case doesn't fall in the default case
    bool checkSpell = false;

    // Check if player has enough mana
    switch(spell)
    {
        case SPELL_NULL:
            checkSpell = false;
            break;

        case SPELL_FIREBALL1:
            manaNeeded = 23.5f;
            checkSpell = true;
            break;

        case SPELL_ICEDART1:
            manaNeeded = 3.25f;
            checkSpell = true;
            break;

        default:
            checkSpell = false;
            break;
    }

    if(checkSpell)
    {
        if(player.attributes.curMana > manaNeeded)
        {
            G_PlayerPlayAnimationOnce(ANIM_CAST_SPELL);
            player.hasToCast = true;
            player.hasCasted = false;
            G_PlayerDrainMana(manaNeeded);
        }
        else
        {
            // Alert no mana
            alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
            R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X, ALERT_MESSAGE_DEF_Y, "You need more  Mana", 2.0f, 1.0f);
        }
    }
}

static void I_SetAttackCone(int id, int x, int y)
{
    // Prevent setting negative values that can cause going out of boundaries (negative grids values), hence crash 
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;

    player.attackConeGridPos[id].x = x;
    player.attackConeGridPos[id].y = y;
}

bool G_PlayerCanAttack(void)
{
    return (player.state != PSTATE_ATTACKING1 && player.state != PSTATE_DOINGSKILL1 && player.state != PSTATE_DOINGSKILL2 && player.state != PSTATE_DOINGSKILL3 && player.state != PSTATE_CASTSPELL && player.state != PSTATE_CLIMBING_LADDER);
}

void G_PlayerSetWeapon(playerFPID_e weaponID)
{
    switch(weaponID)
    {
        case PLAYER_FP_HANDS:
            player.curWeapon = PLAYER_FP_HANDS;
            player.weaponDistance = 85.0f;
            break;
        
        case PLAYER_FP_AXE:
            player.curWeapon = PLAYER_FP_AXE;
            player.weaponDistance = 90.0f;
            break;

        case PLAYER_FP_GREATSWORD:
            player.curWeapon = PLAYER_FP_GREATSWORD;
            player.weaponDistance = 105.0f;
            break;

        case PLAYER_FP_MACE:
            player.curWeapon = PLAYER_FP_MACE;
            player.weaponDistance = 105.0f;
            break;

        default:
            player.curWeapon = PLAYER_FP_HANDS;
            player.weaponDistance = 85.0f;
            break;
    }
}

void G_PlayerSetSpell(playerSpells_e spellID)
{
    switch(spellID)
    {
        case SPELL_NULL:
            player.curSpell = SPELL_NULL;
            break;
        
        case SPELL_FIREBALL1:
            player.curSpell = SPELL_FIREBALL1;
            break;
    
        case SPELL_ICEDART1:
            player.curSpell = SPELL_ICEDART1;
            break;

        default:
            player.curSpell = SPELL_NULL;
            break;
    }
}

static void I_PlayerLadderMovements()
{
    bool doingSomething = false;

    if(player.climbingUp)
    {
        // Go UP first
        if(abs(player.z - player.climbingPosZ) > 1.0f)
        {
            if(player.climbingPosZ > player.z)
                player.z += PLAYER_CLIMBING_LADDER_UP_SPEED * deltaTime;
            else
                player.z -= PLAYER_CLIMBING_LADDER_UP_SPEED * deltaTime;

            doingSomething = true;
        }   
        else
        {
            // Check the difference between the desired and actual pos
            if(abs(player.position.x - player.climbingPosX) > 1.0f)
            {
                // Get pos closer to climbing pos
                if(player.climbingPosX > player.position.x)
                    player.position.x += PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
                else
                    player.position.x -= PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;

                doingSomething = true;
            }

            if(abs(player.position.y - player.climbingPosY) > 1.0f)
            {
                if(player.climbingPosY > player.position.y)
                    player.position.y += PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
                else
                    player.position.y -= PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
                doingSomething = true;
            }
        }
    }
    else
    {
        // Go X-Y first
        // Check the difference between the desired and actual pos
        if(abs(player.position.x - player.climbingPosX) > 1.0f)
        {
            // Get pos closer to climbing pos
            if(player.climbingPosX > player.position.x)
                player.position.x += PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
            else
                player.position.x -= PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;

            doingSomething = true;
        }
        else if(abs(player.position.y - player.climbingPosY) > 1.0f)
        {
            if(player.climbingPosY > player.position.y)
                player.position.y += PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
            else
                player.position.y -= PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
            doingSomething = true;
        }
        else if(abs(player.z - player.climbingPosZ) > 1.0f)
        {
            if(player.climbingPosZ > player.z)
                player.z += PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;
            else
                player.z -= PLAYER_CLIMBING_LADDER_DOWN_SPEED * deltaTime;

            doingSomething = true;
        }   
    }
    
    // Compute centered pos for calculations
    player.centeredPos.x = player.position.x + PLAYER_CENTER_FIX;
    player.centeredPos.y = player.position.y + PLAYER_CENTER_FIX;

    // Exit condition
    if(!doingSomething)
    {
        player.position.x = player.climbingPosX;
        player.position.y = player.climbingPosY;

        // Compute centered pos for calculations
        player.centeredPos.x = player.position.x + PLAYER_CENTER_FIX;
        player.centeredPos.y = player.position.y + PLAYER_CENTER_FIX;

        player.z = player.climbingPosZ;

        player.hasToClimb = false;
        player.state = PSTATE_IDLE;
    }
}

void G_PlayerCheckPuddleDamage(void)
{
    mappudlle_t* cur = activeMapPuddlesHead;
    while(cur != NULL)
    {   
        if(cur->damagesPlayers && player.gridPosition.x == cur->gridX && player.gridPosition.y == cur->gridY && player.level == cur->level)
        {
            // Take puddle damage
            G_PlayerTakeDamage(cur->damage * deltaTime);
        }

        cur = cur->next;
    }
}
