#ifndef SKILLS_H_INCLUDED
#define SKILLS_H_INCLUDED

#include "U_DataTypes.h"

#define MAX_SKILLS_PER_CLASS 3

// Skills (class specific)
typedef enum skillIDs_e
{
    TANK_SKILL_SHIELD_SLAM,
    TANK_SKILL_SHIELD_BLOCK,
    TANK_SKILL_CONSACRATION,

    HEALER_SKILL_SELF_HEAL,
    HEALER_SKILL_CONCENTRATED_HEAL,
    HEALER_SKILL_DIVINE_INTERVENTION,

    DPS_SKILL_CHEAPSHOT,
    DPS_SKILL_OBLITERATE,
    DPS_SKILL_SPLIT
} skillIDs_e;

typedef struct skill_t
{
    skillIDs_e skillID;
    float manaConsumption;

    int cooldown;   // in ms
    Timer* timer;
} skill_t;


void G_InitializeSkills(void);

void G_DoSkill(skill_t skill);

#endif
