#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "netdef.h"
#include "replication.h"

// Protocol relate
#define PROT_ID_TCP 1
#define PROT_ID_UDP 2

#define PCKT_SIZE sizeof(pckt_t) // sizeof(pckt_t)
#define MAX_PCKT_DATA 1300 // sizeof the char buffer inside the packet data

#define PCKT_RECEIVE_RETURN_ERROR 240
#define PCKT_SEND_AND_CLOSE 640

// Packets IDs
#define PCKTID_GREET                1
#define PCKTID_SET_CLASS            2
#define PCKTID_READY                3
#define PCKTID_STARTING             4
#define PCKTID_PLAYERUPDATE         5
#define PCKTID_DOOR_CHANGE          6
#define PCKTID_PICKUP_PICKED        7
#define PCKTID_PROJECTILE_SPAWN     8
#define PCKTID_PROJECTILE_DESTR     9
#define PCKTID_AI_MOVEMENTS         10
#define PCKTID_AI_ATTACKED          11
#define PCKTID_AI_PLAY_ANIM         12
#define PCKTID_AI_INSTANTIATE       13
#define PCKTID_PUDDLES_INSTANTIATE  14
#define PCKTID_HEAL_OTHER           15
#define PCKTID_PLAYER_DEATH         16

#define PCKT_BUFFER PCKT_SIZE

// Abs packet
typedef struct pckt_t
{
    byte protocol;
    byte id;
    byte data[MAX_PCKT_DATA];
} pckt_t;

// Packets struct
typedef struct pckt_greet_t
{
    char name[NET_MAX_PLAYER_NAME_LENGTH];
    byte favoriteClass;
} pckt_greet_t;

typedef struct pckt_set_class_t
{
    byte classSet;
} pckt_set_class_t;

typedef struct pckt_ready_t
{
    byte isReady;
} pckt_ready_t;

typedef struct pckt_starting_t
{
    byte starting;  // 0 = not starting - 1 = starting - 2 = playing
} pckt_starting_t;

typedef struct pckt_playerupdate_t
{
    float x,y;
    float angle;
    float z;

    float curHealth;
    float maxHealth;

    float curMana;
    float maxMana;

    int curWeapon;
    int curSpell;
} pckt_playerupdate_t;

typedef struct pckt_door_change_t
{
    int level;
    int x,y;
    int state;
} pckt_door_change_t;

typedef struct pckt_pickup_picked_t
{
    int level;
    int x,y;
} pckt_pickup_picked_t;


typedef struct packedprojectilespawn_t
{
    int networkID;
    int spriteID;
    float angle;
    int level;
    float posX, posY, posZ;
    float verticalAngle;
    bool isOfPlayer;
    int aiOwnerID;
} packedprojectilespawn_t;

#define MAX_PROJECTILES_TO_SEND_SIZE 256    // max amounts of projectiles that can be saved to be sent this frame
#define MAX_PROJECTILESPAWN_PER_PACKET 32   // max amounts of projectiles per packet
typedef struct pckt_projectile_spawn_t
{
    uint32_t length;
    packedprojectilespawn_t projectiles[MAX_PROJECTILESPAWN_PER_PACKET];
} pckt_projectile_spawn_t;

typedef struct pckt_projectile_destr_t
{
    int networkID;
    int spriteID;
    bool forceDestroy;  // forces the destroy by skipping the isNetworkInstance check
} pckt_projectile_destr_t;

#define MAX_AIREPLICATIONT_PER_PACKET 64 // 64*sizeof(aireplicated_t)+content must be < 1300
typedef struct pckt_aimovementupdate_t
{
    uint32_t length;
    aireplicated_t ais[MAX_AIREPLICATIONT_PER_PACKET];
} pckt_aimovementupdate_t;

typedef struct pckt_aiattacked_t
{
    int networkID;
    float damage;
    bool died;
} pckt_aiattacked_t;

typedef struct pckt_aiplayanim_t
{
    int networkID;
    int anim;
    bool loop;
} pckt_aiplayanim_t;

typedef struct pckt_aiinstantiate_t
{
    int networkID;
    
    int level;
    int gridX, gridY;
    int spriteID;

    bool playAnimation;
    int animID;
    bool loop;
} pckt_aiinstantiate_t;


// Single puddle, used in pckt_puddle_istantiate to embed more than 1 per packet
typedef struct packedpuddle_t
{
    int networkID;

    int gridX, gridY;
    bool damagesAI;
    bool damagesPlayer; 
    float damage; 
    int duration; 
    int level; 
    int newFloorID;
    int previousFloorID;
} packedpuddle_t;

#define MAX_PUDDLE_ABS_SIZE 2048
#define MAX_PUDDLE_OBJECTS_INSTANTIATE 30   //sizeof(packedpuddle_t) * 30 <= MAX_PCKT_DATA
typedef struct pckt_puddle_instantiate_t
{
    unsigned length;
    packedpuddle_t puddles[MAX_PUDDLE_OBJECTS_INSTANTIATE];
} pckt_puddle_instantiate_t;

typedef struct pckt_heal_other_t
{
    float healAmount;
} pckt_heal_other_t;

#define MAX_PCKTS_PER_BUFFER 50
typedef struct pckt_buffer_t
{
    char buffer [PCKT_SIZE*MAX_PCKTS_PER_BUFFER];
    
    // For short receive, lenght of what got received so far and flag
    int len;
    bool shorted;

    // For short send, this flag fills the buffer for writing and tells the program to not do it again on future PCKT_Send calls
    bool hasBegunWriting;
    int packetsToWrite; // how many packets are written in the buffer to send
    int packetOffset;
} pckt_buffer_t;


// Global packet data to send out
extern pckt_t packetToSend;

// Input buffer when receiving packets
extern pckt_buffer_t inputPcktBuffer;

// Output buffer when sending packets
extern pckt_buffer_t outputPcktBuffer;

void PCKT_Zero(pckt_t* p);
void PCKT_ZeroBuffer(pckt_buffer_t* pBuf);

void PCKT_ExtractFromBuffer(int offset);

pckt_t* PCKT_MakeGreetPacket(pckt_t* packet, char pName[NET_MAX_PLAYER_NAME_LENGTH], byte pFavClass);
pckt_t* PCKT_MakeSetClassPacket(pckt_t* packet, byte pClassToSet);
pckt_t* PCKT_MakeReadyPacket(pckt_t* packet, byte pIsReady);
pckt_t* PCKT_MakeStartingPacket(pckt_t* packet, byte pStarting);
pckt_t* PCKT_MakePlayerUpdatePacket(pckt_t* packet, float pX, float pY, float pZ, float pAngle, float pCurHealth, float pMaxHealth, float pCurMana, float pMaxMana, int pCurWeapon, int pCurSpell);
pckt_t* PCKT_MakeDoorChangePacket(pckt_t* packet, int pLevel, int pX, int pY, int pState);
pckt_t* PCKT_MakePickupPickedPacket(pckt_t* packet, int pLevel, int pX, int pY);
pckt_t* PCKT_MakeProjectileSpawnPacket(pckt_t* packet, unsigned pLength, packedprojectilespawn_t pProjectiles[MAX_PROJECTILES_TO_SEND_SIZE]);
pckt_t* PCKT_MakeProjectileDestrPacket(pckt_t* packet, int pNetworkID, int pSpriteID, bool pForceDestroy);
pckt_t* PCKT_MakeAIMovementUpdatePacket(pckt_t* packet);
pckt_t* PCKT_MakeAIAttackPacket(pckt_t* packet, int pNetworkID, float pDamage, bool pDied);
pckt_t* PCKT_MakeAIPlayAnimPacket(pckt_t* packet, int pNetworkID, int pAnimID, bool pLoop);
pckt_t* PCKT_MakeAIInstantiatePacket(pckt_t* packet, int pNetworkID, int pLevel, int pGridX, int pGridY, int pSpriteID, bool pPlayAnim, int pAnimID, bool pLoop);
pckt_t* PCKT_MakePuddlesInstantiatePacket(pckt_t* packet);
pckt_t* PCKT_MakeHealOtherPacket(pckt_t* packet, float pAmount);
pckt_t* PCKT_MakePlayerDeathPacket(pckt_t* packet);

int PCKT_ReceivePacket(int (*OnPacketArrives)(void));
int PCKT_SendPacket(int (*OnPacketIsSent)(void));

#endif
