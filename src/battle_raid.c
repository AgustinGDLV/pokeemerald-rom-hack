#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h"
#include "battle_util.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_setup.h"
#include "party_menu.h"
#include "pokemon.h"
#include "international_string_util.h"
#include "item.h"
#include "util.h"
#include "battle_scripts.h"
#include "random.h"
#include "text.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "trig.h"
#include "window.h"
#include "battle_message.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "event_data.h"
#include "link.h"
#include "malloc.h"
#include "berry.h"
#include "pokedex.h"
#include "mail.h"
#include "field_weather.h"
#include "constants/abilities.h"
#include "constants/battle_anim.h"
#include "constants/battle_config.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_script_commands.h"
#include "constants/battle_string_ids.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/weather.h"

// Resets raid variables at the start of battle. Called in TryDoEventsBeforeFirstTurn
void InitRaidVariables(void)
{
    gBattleStruct->raid.starRating = gSpecialVar_0x8000; // variable is set when battle is created.
    gBattleStruct->raid.barriers = 0;
    gBattleStruct->raid.storedDmg = 0;
    gBattleStruct->raid.thresholdsRemaining = GetRaidThresholdNumber();
}

// Returns how many barriers to create at a threshold. Based on star rating and stats.
u8 GetRaidBarrierNumber(u8 battlerId)
{
    u16 species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES, NULL);
    u8 hp = gBaseStats[species].baseHP;
    u8 def = gBaseStats[species].baseDefense;
    u8 spDef = gBaseStats[species].baseSpDefense;
    u8 retVal;

    switch (hp + def + spDef) // Currently uses the sum of defenses to determine barrier count.
    {
        case 0 ... 199:
            retVal = 2;
            break;
        case 200 ... 300:
            retVal = 3;
            break;
        default: // > 300
            retVal = 4;
            break;
    }

    if (gBattleStruct->raid.starRating >= 5)
        retVal += 1;
    
    return retVal;
}

// Returns how many health thresholds a raid will have. Based on star rating.
u8 GetRaidThresholdNumber(void)
{
    u8 starRating = gBattleStruct->raid.starRating;
    switch (starRating)
    {
        case 5:
            return 2;
        case 3 ... 4:
            return 1;
        default:
            return 0;
    }
}

// Returns the next health threshold of a Raid Boss.
u32 GetNextHealthThreshold()
{
    u32 maxHP = GetMonData(&gEnemyParty[0], MON_DATA_MAX_HP, NULL);
    u8 total = GetRaidThresholdNumber();
    u8 remaining = gBattleStruct->raid.thresholdsRemaining;

    if (remaining == 0 || total == 0)
        return 0;
    else
        return (remaining * maxHP) / (total + 1);
}

// Returns whether a hit will reduce a Raid Boss to a health threshold.
bool8 ShouldCreateBarrier(u8 battlerId, s32 dmg)
{    
    u32 hp = GetMonData(&gEnemyParty[0], MON_DATA_HP, NULL);
    
    if (!(gBattleTypeFlags & BATTLE_TYPE_RAID))
        return FALSE;
    if (GetBattlerPosition(battlerId) != B_POSITION_OPPONENT_LEFT)
        return FALSE;
    if (gBattleStruct->raid.thresholdsRemaining == 0)
        return FALSE;
    
    if ((hp - dmg) <= GetNextHealthThreshold())
        return TRUE;
    else
        return FALSE;
}
