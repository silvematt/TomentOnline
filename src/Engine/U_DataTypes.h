#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#include "U_Timer.h"

// 1 Radian
#define RADIAN 0.0174533        

// Byte definition
typedef	uint8_t byte;

typedef struct vector2_s
{
    float x, y;
} vector2_t;

typedef struct vector2Int_s
{
    int x,y;
} vector2Int_t;


typedef struct circle_s
{
    vector2_t pos;
    float r;
} circle_t;


//-------------------------------------
// Pathfinding
//-------------------------------------
// Node of a path
typedef struct pathnode_s
{
    vector2Int_t gridPos;

    // Costs
    int f, g, h;
    struct pathnode_s* parent;
} pathnode_t;

// Used by AI and such
typedef struct path_s
{
    bool isValid;
    pathnode_t* nodes[256];
    unsigned int nodesLength;
} path_t;

// -------------------------------
// Sprite data structure, represents static sprites
// -------------------------------
typedef struct sprite_s
{
    char* name;
    bool active;            // used for dynamics
    
    vector2_t pos;          // position in world
    float angle;
    
    vector2_t centeredPos;  // position in world centered
    vector2Int_t gridPos;   // position in grid
    vector2_t pSpacePos;    // position in player space        

    int level; // which level this sprite is in

    circle_t collisionCircle;

    int spriteID;
    int sheetLength;        // For Animated objects

    float dist;     // distance from player
    float height;   // how big the sprite will be drawn

    float z;
} sprite_t;

// Entity attributes
typedef struct entityattributes_s
{
    float curHealth;
    float maxHealth;

    float curMana;
    float maxMana;

    float spellPower;   // multiplier for certain skills (healer)

    float baseDamage;
    int attackChance;
    int criticalChance;
    float criticalModifier;
} entityattributes_t;

// Defines the states of dynamic sprites (AI)
typedef enum dynamicSpriteState_e
{
    DS_STATE_NULL = 0,
    DS_STATE_IDLE,
    DS_STATE_MOVING,
    DS_STATE_ATTACKING,
    DS_STATE_DEAD,
    DS_STATE_CASTING,
    DS_STATE_SPECIAL1,
    DS_STATE_SPECIAL2,
    DS_STATE_SPECIAL3
} dynamicSpriteState_e;

// Types of dynamic sprites
typedef enum dynamicSpriteType_e
{
    DS_TYPE_AI = 0,
    DS_TYPE_PROJECTILE,
    DS_TYPE_OTHERPLAYER
} dynamicSpriteType_e;

// Max number of spell an AI can have (Used to create Timers with which create spell logic)
#define AI_MAX_SPELLS 5

// -------------------------------
// Dynamic Sprite data structure, represents dynamic sprites such as AI or projectiles
// -------------------------------
typedef struct dynamicSprite_s
{
    sprite_t base;

    // Network
    uint32_t networkID;
    bool hasChanged;

    // Dnyamic-Specific
    dynamicSpriteType_e type;
    dynamicSpriteState_e state;
    bool isAlive;
    float speed;
    
    // Boss related stuff
    bool isBoss; // If true, when the AI will attack the player the boss UI will appear
    bool bossPreventOpeningDoorsWhileFighting; // If this is true, when this boss will be fought the player will not be able to interact with doors
    bool bossPreventClimbingLaddersWhileFighting; // If this is true, when this boss will be fought the player will not be able to interact with ladders
    bool bossPreventActivatingTriggersWhileFighting; // If this is true, when this boss will be fought the player will not be able to interact with triggers
    int bossPhase;
    int counter1; // a counter useful to build spells on it
    float value1; // a value useful to build spells on it

    bool canBeHit;

    void (*BehaviourUpdate)(struct dynamicSprite_s* this);
    Timer* cooldowns[AI_MAX_SPELLS]; // spells/abilities cooldowns, used by bosses and casters

    bool aggroedPlayer; // True if this AI already attacked/chased the player
    int spellInUse;
    
    SDL_Surface* curAnim;
    int curAnimLength;
    
    Timer* animTimer;
    bool animPlay;
    int animFrame;
    bool animPlayOnce;
    float animSpeed;

    entityattributes_t attributes;

    vector2_t* targetPos;
    vector2Int_t* targetGridPos;
    circle_t* targetColl;

    path_t* path;

    // Used for projectiles to distinguish between player's projectiles and AI's projectiles
    struct dynamicSprite_s* aiOwner;
    bool isOfPlayer;
    bool isBeingDestroyed; // called when a projectile hits and awaits the explosion animation to be removed from the list

    // How muc this dynamic needs to move each tick on the z axis
    float verticalMovementDelta;

    vector2_t overheadPos;

    // Aggro variables to determine which player this AI should follow and attack
    int hostAggro;
    int joinerAggro;

    // Online position updates
    bool posArrived;
    float lastPosX, lastPosY, lastPosZ;

    // Display positions
    
    /*
        When the host calculates the position packets and sends them to the joiner, there will always be some delay. 
        If the joiner sets the coordinates directly, there is no other problem than the fact that the movements of the AI will be visibly snapped to the movements coordinates inside the packets.

        If the joiner smoothly interoplates its current positions to the packet's position, the movements appear smooth, but the world may get out of synch as the host may do something like instantiating the AI on a tile that is occupied for the other player (because the smoothed movement is slower and not synchronized at any given frame)
        So it's definitely a no-go.
        
        The solution is to use a display position only on the joiner side.
        The joiner will keep setting directly what is the normal "pos" (now called the "internal" pos) of the AIs that will make sure the worlds are synched.
        But the Drawables will now be rendered using the display pos instead of the internal pos.

        The display pos is smoothly interpolated by what it was before and what arrived in the packet.

        The result is that the world will be kept in synch and the AI will appear moving smoothly on the joiner when lag occurs.
    */
    vector2_t displayPos;
    float displayZ;
    float displayDist;       

} dynamicSprite_t;

// -------------------------------
// Door states
// -------------------------------
typedef enum doorstate_e
{
    DState_Closed = 0,
    DState_Opening,
    DState_Open,
    DState_Closing
} doorstate_e;


// -------------------------------
// Holds all the info got during raycast to draw walls
// -------------------------------
typedef struct walldata_s
{
    float rayAngle;
    int x;
    float curX, curY;
    bool isVertical;
    vector2Int_t gridPos;
    float distance;
    struct wallObject_s* objectHit;
    int level;
    
    // Extra Data
    // For Thin Wall Extra data is 0 = should draw this column of pixel because it is visible 1 = should not draw this column of pixel because the door is open
    int extraData;  
} walldata_t;

// -------------------------------
// Drawable Types
// -------------------------------
typedef enum drawabletype_e
{
    DRWB_WALL = 0,
    DRWB_SPRITE,
    DRWB_DYNAMIC_SPRITE
} drawabletype_e;

// -------------------------------
// Drawables are object in the world that will need to be drawn
// A drawable array holds everything to render and allows Zdepth
// -------------------------------
typedef struct drawabledata_s 
{
    drawabletype_e type;

    // Pointers
    walldata_t* wallPtr;
    sprite_t* spritePtr;
    dynamicSprite_t* dynamicSpritePtr;

    // Quick access varaibles
    float dist;
} drawabledata_t;

typedef struct projectileNode_s
{
    dynamicSprite_t this;
    
    // If a projectile is a network instance, no collision checking should happen on this player, the other player will send a message to update us when the projectile hit something 
    bool isNetworkInstance; 
    uint32_t networkID;
    
    struct projectileNode_s* next;
    struct projectileNode_s* previous;
} projectileNode_t;

typedef struct alertMessage_s
{
    int x,y;
    char* message;
    float size, duration;
    Timer* timer;
    
    struct alertMessage_s* next;
} alertMessage_t;


typedef enum orientation_e
{
    NORTH = 0,
    NORTH_WEST,
    WEST,
    SOUTH_WEST,
    SOUTH,
    SOUTH_EAST,
    EAST,
    NORTH_EAST,
    NO_ORIENTATION,
    NORTH_SOUTH,    // Specials
    WEST_EAST,
    ALL_FOUR_DIR,
} orientation_e;

typedef struct mappuddle_s
{
    int networkID;
    bool isNetworkedInstance;
    
    int gridX;
    int gridY;

    bool damagesAI;
    bool damagesPlayers;

    float damage;
    int duration; // in ms

    int level;
    int previousFloorID;
    int newFloorID;

    Timer* timer;

    struct mappuddle_s* next;
    struct mappuddle_s* previous;
} mappudlle_t;

#endif