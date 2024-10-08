#include <math.h>

#include "G_AI.h"
#include "G_Player.h"
#include "G_Pathfinding.h"
#include "P_Physics.h"
#include "G_AIBehaviour.h"

#include "../Online/O_Game.h"

// Dynamic AI list
dynamicSprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE_HIGH];
unsigned int allDynamicSpritesLength = 0;

// Initializes the dynamic sprites
// Switch on the spriteID to be able to configure parameters for each entity, could be done in a file to avoid having to recompile to change stuff like speed
void G_AIInitialize(dynamicSprite_t* cur, int level, int spriteID, int x, int y)
{
    // Check if there's room for this ai
    if(allDynamicSpritesLength+1 >= OBJECTARRAY_DEFAULT_SIZE_HIGH)
    {
        printf("Cannot initialize AI, dynamic sprites length: %d\n", allDynamicSpritesLength);
        free (cur);
        return;
    }

    //---------------------
    // Base
    //---------------------
    cur->type = DS_TYPE_AI;
    
    cur->base.active = true;
    cur->base.level = level;
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    cur->base.z = TILE_SIZE * (level);
    cur->verticalMovementDelta = 0.0f;
    cur->base.gridPos.x = x;
    cur->base.gridPos.y = y;

    cur->canBeHit = true;

    // Get World Pos
    cur->displayPos.x = cur->base.pos.x = x * TILE_SIZE;
    cur->displayPos.y = cur->base.pos.y = y * TILE_SIZE;
    cur->displayZ = cur->base.z;

    cur->base.collisionCircle.pos.x = cur->base.pos.x;
    cur->base.collisionCircle.pos.y = cur->base.pos.y;
    cur->base.collisionCircle.r = 32;

    // Get ID
    cur->base.spriteID = spriteID;
    cur->base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[spriteID];

    //---------------------
    // Dynamic-Specific
    //---------------------
    cur->state = DS_STATE_IDLE;
    cur->isAlive = true;
    cur->speed = 2.0f;

    cur->curAnim = NULL;
    cur->curAnimLength = 0;
    cur->animTimer = U_TimerCreateNew();
    cur->animPlay = false;
    cur->animFrame = 0;
    cur->animPlayOnce = false;
    cur->posArrived = false;

    for(int i = 0; i < AI_MAX_SPELLS; i++)
        cur->cooldowns[i] = NULL;

    cur->bossPhase = 0;

    // Init attributes based on the sprite
    switch(cur->base.spriteID)
    {
        case DS_Skeleton:
            cur->base.name = "Skeleton";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;

            cur->speed = 2.0f;
            cur->attributes.maxHealth = 100.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
            break;

        case DS_SkeletonElite:
            cur->base.name = "Royal  Skeleton";
            cur->isBoss = false;
            cur->bossPreventClimbingLaddersWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;
            
            cur->speed = 4.0f;
            cur->attributes.maxHealth = 600.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 600.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 10.0f;
            cur->attributes.attackChance = 90;
            cur->attributes.criticalChance = 20;
            cur->attributes.criticalModifier = 2.0f;
            break;


        case DS_SkeletonBurnt:
            cur->base.name = "Burnt Skeleton";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourCasterEnemy;
            cur->spellInUse = SPELL_FIREBALL1;

            cur->speed = 3.0f;
            cur->attributes.maxHealth = 50.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
            break;

        case DS_SkeletonLord:
            cur->base.name = "Skeleton's  Lord";
            cur->isBoss = true;
            cur->bossPreventClimbingLaddersWhileFighting = false;
            cur->bossPreventOpeningDoorsWhileFighting = true;
            cur->bossPreventActivatingTriggersWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourSkeletonLord;
            cur->spellInUse = SPELL_FIREBALL1;
            
            cur->speed = 4.25f;
            cur->attributes.maxHealth = 3250.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 3250.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 16.0f;
            cur->attributes.attackChance = 90;
            cur->attributes.criticalChance = 10;
            cur->attributes.criticalModifier = 1.5f;

            // This boss has cooldowns for spells
            cur->cooldowns[0] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[1] = U_TimerCreateNew(); // Fireball cooldown
            cur->cooldowns[2] = U_TimerCreateNew(); // Spell1 cooldown
            cur->cooldowns[3] = U_TimerCreateNew(); // Spell2 cooldown
            break;

        case DS_MorgathulTheKeeper:

            cur->base.name = "Morgathul  The  Keeper";
            cur->isBoss = true;
            cur->bossPreventClimbingLaddersWhileFighting = false;
            cur->bossPreventOpeningDoorsWhileFighting = true;
            cur->bossPreventActivatingTriggersWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourMorgathulTheKeeper;
            cur->spellInUse = SPELL_MORGATHUL_ORB;

            cur->speed = 1.5f;
            cur->attributes.maxHealth = 2500.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 2500.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 25.0f;
            cur->attributes.attackChance = 100;
            cur->attributes.criticalChance = 10;
            cur->attributes.criticalModifier = 1.5f;

            // This boss has cooldowns for spells
            cur->cooldowns[0] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[1] = U_TimerCreateNew(); // Fireball cooldown
            cur->cooldowns[2] = U_TimerCreateNew(); // Spell1 cooldown
            cur->cooldowns[3] = U_TimerCreateNew(); // Spell2 cooldown
        break;

        case DS_Kroganar:
            cur->base.name = "Kroganar";
            cur->isBoss = false;
            cur->bossPreventClimbingLaddersWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;
            
            cur->speed = 5.0f;
            cur->attributes.maxHealth = 1250.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 1250.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 15.5f;
            cur->attributes.attackChance = 90;
            cur->attributes.criticalChance = 15;
            cur->attributes.criticalModifier = 1.25f;
            break;

        case DS_MorgathulCopy:
            cur->base.name = "Morgathul  Copy";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourMorgathulCopy;
            cur->spellInUse = SPELL_MORGATHUL_ORB;

            cur->speed = 1.5f;
            cur->attributes.maxHealth = 1.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 100;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;

            cur->cooldowns[0] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[1] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[2] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[3] = U_TimerCreateNew(); // Abs cooldown

            break;


        case DS_TheFrozenLord:

            cur->base.name = "The  Frozen  Lord";
            cur->isBoss = true;
            cur->bossPreventClimbingLaddersWhileFighting = false;
            cur->bossPreventOpeningDoorsWhileFighting = true;
            cur->bossPreventActivatingTriggersWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourTheFrozenLord;
            cur->spellInUse = SPELL_ICE_BLAST;

            cur->speed = 5.5f;
            cur->attributes.maxHealth = 4250.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 4250.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 30.0f;
            cur->attributes.attackChance = 100;
            cur->attributes.criticalChance = 20;
            cur->attributes.criticalModifier = 1.5f;

            // This boss has cooldowns for spells
            cur->cooldowns[0] = U_TimerCreateNew(); // Abs cooldown
            cur->cooldowns[1] = U_TimerCreateNew(); // Fireball cooldown
            cur->cooldowns[2] = U_TimerCreateNew(); // Spell1 cooldown
            cur->cooldowns[3] = U_TimerCreateNew(); // Spell2 cooldown
        break;

        case DS_FrozenLordsCaster:

            cur->base.name = "Frozen  Caster";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourCasterEnemy;
            cur->spellInUse = SPELL_ICEDART1;

            cur->speed = 2.5f;
            cur->attributes.maxHealth = 1.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 3500.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 20.0f;
            cur->attributes.attackChance = 100;
            cur->attributes.criticalChance = 10;
            cur->attributes.criticalModifier = 1.5f;
        break;

        default:
            printf("AI with ID %d was not initalized. Setting it with base stats.\n");
            cur->isBoss = false;
            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;
            cur->attributes.maxHealth = 100.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
        break;
    }

    cur->aggroedPlayer = false;
    cur->hostAggro = 0;
    cur->joinerAggro = 0;
    cur->hasChanged = false;

    // Start playing idle
    G_AIPlayAnimationLoop(cur, ANIM_IDLE);

    // Add it to the dynamic sprite list
    allDynamicSprites[allDynamicSpritesLength] = cur;
    cur->networkID = allDynamicSpritesLength;
    allDynamicSpritesLength++;
}

void G_AIUpdate(void)
{
    for(int i = 0; i < allDynamicSpritesLength; i++)
    {
        dynamicSprite_t* cur = allDynamicSprites[i];

        if(cur == NULL || cur->base.active == false)
            continue;

        cur->BehaviourUpdate(cur);
        G_AICheckPuddleDamage(cur);
    }
}

void G_AIDie(dynamicSprite_t* cur)
{
    if(!cur->isAlive)
        return;

    cur->isAlive = false;
    G_AIPlayAnimationOnce(cur, ANIM_DIE);
    cur->base.collisionCircle.r = -1.0f; // Remove collision

    // Add to death map
    G_AddToDeadDynamicSpriteMap(cur, cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x);

    // Clear from dynamic map
    G_ClearFromDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x);

    // Check if this was the boss the player was fighting, if he was, release the UI
    if(player.isFightingBoss && player.bossFighting == cur)
    {
        player.isFightingBoss = false;
        player.bossFighting = NULL;
    }
}


void G_AIAttackPlayer(dynamicSprite_t* cur)
{
    // Attack chance
    int attack      =  (rand() % (100)) + 1;
    int critical    =  (rand() % (100)) + 1;

    if(attack <= cur->attributes.attackChance)
    {
        float dmg = cur->attributes.baseDamage;

        if(critical <= cur->attributes.criticalChance)
            dmg *= cur->attributes.criticalModifier;

        G_PlayerTakeDamage(dmg);
    }
}


void G_AIPlayAnimationOnce(dynamicSprite_t* cur, objectanimationsID_e animID)
{
    cur->animTimer->Start(cur->animTimer);
    cur->animPlayOnce = true;
    cur->animFrame = 0;

    switch(animID)
    {
        case ANIM_IDLE:
            cur->state = DS_STATE_IDLE;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_ATTACK1:
            cur->state = DS_STATE_ATTACKING;
            cur->animSpeed = ANIMATION_ATTACK_SPEED_DIVIDER;
            break;

        case ANIM_CAST_SPELL:
            cur->state = DS_STATE_CASTING;
            cur->animSpeed = ANIMATION_ATTACK_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL1:
            cur->state = DS_STATE_SPECIAL1;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL2:
            cur->state = DS_STATE_SPECIAL2;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL3:
            cur->state = DS_STATE_SPECIAL3;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;
    }

    // Immediatly set the correct animation
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    cur->animPlay = true;
}

void G_AIPlayAnimationLoop(dynamicSprite_t* cur, objectanimationsID_e animID)
{
    cur->animTimer->Start(cur->animTimer);
    cur->animPlayOnce = false;
    cur->animFrame = 0;

    switch(animID)
    {
        case ANIM_IDLE:
            cur->state = DS_STATE_IDLE;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL1:
            cur->state = DS_STATE_SPECIAL1;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL2:
            cur->state = DS_STATE_SPECIAL2;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_SPECIAL3:
            cur->state = DS_STATE_SPECIAL3;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;
    }

    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSpeedModifier;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSpeedModifier;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SpeedModifier;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SpeedModifier;
            break;

        case DS_STATE_SPECIAL3:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial3SpeedModifier;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            cur->animSpeed = cur->animSpeed + tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSpeedModifier;
            break;
    }

    cur->animPlay = true;
}

void G_AITakeDamage(dynamicSprite_t* cur, float amount, bool setAggro)
{
    if(cur != NULL)
    {
        cur->attributes.curHealth -= amount;

        if(setAggro)
            cur->aggroedPlayer = true;
        
        // Check death
        if(cur->attributes.curHealth <= 0.0f)
            G_AIDie(cur);
    }
}

bool G_AICanAttack(dynamicSprite_t* cur)
{
    return (cur->isAlive && cur->state != DS_STATE_ATTACKING && cur->state != DS_STATE_DEAD && cur->state != DS_STATE_CASTING && cur->state != DS_STATE_SPECIAL1 && cur->state != DS_STATE_SPECIAL2 && cur->state != DS_STATE_SPECIAL3);
}

void G_AICheckPuddleDamage(dynamicSprite_t* ai)
{
    mappudlle_t* cur = activeMapPuddlesHead;
    while(cur != NULL)
    {   
        // If it is a networked instance, the damage needs to be dealt from the other player
        if(!cur->isNetworkedInstance && ai->isAlive)
        {
            if(cur->damagesAI && ai->base.gridPos.x == cur->gridX && ai->base.gridPos.y == cur->gridY && ai->base.level == cur->level)
            {
                // Take puddle damage
                G_AITakeDamage(ai, cur->damage * deltaTime, true);
                O_GameAITakeDamage(ai->networkID, cur->damage * deltaTime, ai->isAlive != true);
            }
        }

        cur = cur->next;
    }
}

void G_FreeDynamicSprite(dynamicSprite_t* ai)
{
    if(!ai)
    {
        printf("Trying to free a non-existing dynamic sprite\n");
        ai = NULL;
        return;
    }

    for(int i = 0; i < AI_MAX_SPELLS; i++)
        if(ai->cooldowns[i] != NULL)
        {
            free(ai->cooldowns[i]);
            ai->cooldowns[i] = NULL;
        }

    free(ai);
    ai = NULL;
}
