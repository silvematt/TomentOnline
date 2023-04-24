#include "G_Skills.h"
#include "G_Player.h"
#include "../Network/netdef.h"
#include "../Network/replication.h"
#include "../Online/O_Game.h"

void G_InitializeSkills(void)
{
    switch(thisPlayer.selectedClass)
    {
        case CLASS_TANK:
        {
            // Shield slam
            player.skills[0].skillID = TANK_SKILL_SHIELD_SLAM;
            player.skills[0].cooldown = 5500;
            player.skills[0].timer = U_TimerCreateNew();
            player.skills[0].manaConsumption = 15.0f;
            player.skills[0].timer->Start(player.skills[0].timer);

            // Shield block
            player.skills[1].skillID = TANK_SKILL_SHIELD_BLOCK;
            player.skills[1].cooldown = 20000;
            player.skills[1].timer = U_TimerCreateNew();
            player.skills[1].manaConsumption = 60.0f;
            player.skills[1].timer->Start(player.skills[1].timer);

            // Consacration
            player.skills[2].skillID = TANK_SKILL_CONSACRATION;
            player.skills[2].cooldown = 10000;
            player.skills[2].timer = U_TimerCreateNew();
            player.skills[2].manaConsumption = 40.0f;
            player.skills[2].timer->Start(player.skills[2].timer);
            break;
        }

        case CLASS_HEALER:
        {
            // Self heal
            player.skills[0].skillID = HEALER_SKILL_SELF_HEAL;
            player.skills[0].cooldown = 2500;
            player.skills[0].timer = U_TimerCreateNew();
            player.skills[0].manaConsumption = 15.0f;
            player.skills[0].timer->Start(player.skills[0].timer);

            // Concentrated heal
            player.skills[1].skillID = HEALER_SKILL_CONCENTRATED_HEAL;
            player.skills[1].cooldown = 3000;
            player.skills[1].timer = U_TimerCreateNew();
            player.skills[1].manaConsumption = 40.0f;
            player.skills[1].timer->Start(player.skills[1].timer);

            // Divine intervention
            player.skills[2].skillID = HEALER_SKILL_DIVINE_INTERVENTION;
            player.skills[2].cooldown = 20000;
            player.skills[2].timer = U_TimerCreateNew();
            player.skills[2].manaConsumption = 100.0f;
            player.skills[2].timer->Start(player.skills[2].timer);

            break;
        }

        case CLASS_DPS:
        {
            // CheapShot slash
            player.skills[0].skillID = DPS_SKILL_CHEAPSHOT;
            player.skills[0].cooldown = 6000;
            player.skills[0].timer = U_TimerCreateNew();
            player.skills[0].manaConsumption = 40.0f;
            player.skills[0].timer->Start(player.skills[0].timer);

            // Obliterate
            player.skills[1].skillID = DPS_SKILL_OBLITERATE;
            player.skills[1].cooldown = 10000;
            player.skills[1].timer = U_TimerCreateNew();
            player.skills[1].manaConsumption = 20.0f;
            player.skills[1].timer->Start(player.skills[1].timer);

            // Split
            player.skills[2].skillID = DPS_SKILL_SPLIT;
            player.skills[2].cooldown = 20000;
            player.skills[2].timer = U_TimerCreateNew();
            player.skills[2].manaConsumption = 80.0f;
            player.skills[2].timer->Start(player.skills[2].timer);
            break;
        }
    }
}
