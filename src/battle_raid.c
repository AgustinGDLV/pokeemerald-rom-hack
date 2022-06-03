#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h"
#include "battle_util.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "event_data.h"
#include "party_menu.h"
#include "pokemon.h"
#include "international_string_util.h"
#include "item.h"
#include "item_menu.h"
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
#include "constants/moves.h"
#include "constants/battle_string_ids.h"

// Sourced from Bulbapedia and cut down to balance in a 2v1 setting.
static const u16 sStarRatingHPMultiplierTable[] =
{
    [1] = UQ_4_12(1.2),
    [2] = UQ_4_12(1.3),
    [3] = UQ_4_12(1.5),
    [4] = UQ_4_12(1.7),
    [5] = UQ_4_12(2.0),
    [6] = UQ_4_12(2.0),
};

// Resets raid variables at the start of battle. Called in TryDoEventsBeforeFirstTurn.
void InitRaidVariables(void)
{
    u8 i;

    gBattleStruct->raid.stars = gSpecialVar_0x8008; // variable is set before battle starts.
    gBattleStruct->raid.shields = 0;
    gBattleStruct->raid.storedDmg = 0; // used to "release" damage when barriers break.
    gBattleStruct->raid.thresholdsRemaining = GetRaidThresholdNumber();
    gBattleStruct->raid.stormTurns = 0;
    gBattleStruct->raid.state = INTRO_COMPLETED;

    for (i = 0; i < MAX_BARRIER_COUNT; i++)
    {
        gBattleStruct->raid.shieldSpriteIds[i] = MAX_SPRITES;
    }

    RecalcBattlerStats(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), &gEnemyParty[0]); // to apply HP multiplier
}

// Handles Raid Storm end turn events. Called in BattleTurnPassed.
void IncrementRaidStorm(void)
{
    if (gBattleStruct->raid.stormTurns < RAID_STORM_MAX)
        gBattleStruct->raid.stormTurns++;
    switch(gBattleStruct->raid.stormTurns)
    {
        case RAID_STORM_LEVEL_1:
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_GETTING_STRONGER;
            BattleScriptExecute(BattleScript_RaidStormBrews);
            break;
        case RAID_STORM_LEVEL_2:
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_GETTING_STRONGER;
            BattleScriptExecute(BattleScript_RaidStormBrews);
            break;
        case RAID_STORM_LEVEL_3:
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_GETTING_EVEN_STRONGER;
            BattleScriptExecute(BattleScript_RaidStormBrews);
            break;
        case RAID_STORM_MAX:
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_GETTING_TOO_STRONG;
            BattleScriptExecute(BattleScript_RaidDefeat);
            break;
    }
}

u16 GetRaidHPMultiplier(void)
{
    return sStarRatingHPMultiplierTable[gBattleStruct->raid.stars];
}

void ApplyRaidHPMultiplier(struct Pokemon *mon)
{
    u16 hp, maxHP, multiplier;
    hp = GetMonData(mon, MON_DATA_HP, NULL);
    maxHP = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    multiplier = sStarRatingHPMultiplierTable[gBattleStruct->raid.stars];

    hp = UQ_4_12_TO_INT((hp * multiplier) + UQ_4_12_ROUND);
    maxHP = UQ_4_12_TO_INT((maxHP * multiplier) + UQ_4_12_ROUND);

    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
}

// Returns how many barriers to create at a threshold. Based on star rating and defensive stats.
u8 GetRaidShieldNumber(void)
{
    u16 species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES, NULL);
    u8 hp = gBaseStats[species].baseHP;
    u8 def = gBaseStats[species].baseDefense;
    u8 spDef = gBaseStats[species].baseSpDefense;
    u8 retVal;

    // Currently uses the sum of defenses to determine barrier count.
    switch (hp + def + spDef)
    {
        case 0 ... 199:
            retVal = 3;
            break;
        case 200 ... 300:
            retVal = 4;
            break;
        default: // > 300
            retVal = MAX_BARRIER_COUNT;
            break;
    }

    if (gBattleStruct->raid.stars < 5)
        retVal -= 1;
    
    return retVal;
}

// Returns how many HP thresholds a raid will have. Based on star rating.
u8 GetRaidThresholdNumber(void)
{
    u8 starRating = gBattleStruct->raid.stars;
    switch (starRating)
    {
        case 5 ... MAX_STAR_RATING:
            return 2;
        case 3 ... 4:
            return 1;
        default:
            return 0;
    }
}

// Returns the next health threshold of a Raid Boss.
u32 GetNextHealthThreshold(void)
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
bool8 ShouldCreateBarrier(s32 dmg)
{    
    u32 hp = GetMonData(&gEnemyParty[0], MON_DATA_HP, NULL);
    
    if (!(gBattleTypeFlags & BATTLE_TYPE_RAID))
        return FALSE;
    if (gBattleStruct->raid.thresholdsRemaining == 0)
        return FALSE;
    
    if (hp <= dmg + GetNextHealthThreshold())
        return TRUE;
    else
        return FALSE;
}

bool8 DoesRaidPreventMove(u16 move)
{
    switch(move) // data from Bulbapedia
    {
        case MOVE_BUG_BITE:
        case MOVE_COVET:
        case MOVE_INCINERATE:
        case MOVE_KNOCK_OFF:
        case MOVE_PERISH_SONG:
        case MOVE_PLUCK:
        case MOVE_SELF_DESTRUCT:
        case MOVE_SUPER_FANG:
        case MOVE_THIEF:
        case MOVE_TRICK:
            return TRUE;
    }
    return FALSE;
}

u8 GetRaidRepeatedAttackChance(void)
{
	u8 starRating = gBattleStruct->raid.stars;
    switch (starRating)
    {
		case 0 ... 2:
			return 0; //Never
		case 3:
			return 30; //30 % of the time after KO or Status Move
		case 4:
			return 50; //50 % of the time after KO or Status Move
		default:
			return 70; //70 % of the time after KO or Status Move
	}
}

u8 GetRaidNullificationChance(void)
{
    u8 starRating = gBattleStruct->raid.stars;

	if (gDisableStructs[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].isFirstTurn)
		return 0; //Don't use first attack with this

    switch (starRating)
    {
		case 0 ... 2:
			return 0; //Never
		case 3:
			return 20; //20 % chance before each attack
		case 4:
			return 35; //35 % chance before each attack
		default:
			return 50; //50 % chance before each attack
	}
}

// Used for opening the bag in the end sequence.
void CB2_ChooseBall(void)
{
    GoToBagMenu(ITEMMENULOCATION_BERRY_TREE, BALLS_POCKET, CB2_SetUpReshowBattleScreenAfterMenu2);
}

// Shield sprite data:
static const u8 sRaidShieldGfx[] = INCBIN_U8("graphics/battle_interface/raid_barrier.4bpp");
static const u16 sRaidShieldPal[] = INCBIN_U16("graphics/battle_interface/raid_barrier.gbapal");

static const struct SpriteSheet sSpriteSheet_RaidShield =
{
    sRaidShieldGfx, sizeof(sRaidShieldGfx), TAG_RAID_SHIELD_TILE
};
static const struct SpritePalette sSpritePalette_RaidShield =
{
    sRaidShieldPal, TAG_RAID_SHIELD_PAL
};

static const struct OamData sOamData_RaidShield =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = SPRITE_SHAPE(16x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x16),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const s8 sShieldPosition[2] = {48, 9};

// Sync up barrier sprites with healthbox.
static void SpriteCb_RaidBarrier(struct Sprite *sprite)
{
    u8 healthboxSpriteId = gBattleSpritesDataPtr->battleBars[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].healthboxSpriteId;
    sprite->y2 = gSprites[healthboxSpriteId].y2;
}

static const struct SpriteTemplate sSpriteTemplate_RaidShield =
{
    .tileTag = TAG_RAID_SHIELD_TILE,
    .paletteTag = TAG_RAID_SHIELD_PAL,
    .oam = &sOamData_RaidShield,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_RaidBarrier,
};

#define tBattler    data[0]

u32 CreateRaidShieldSprite(u8 index)
{
    u32 spriteId, position;
    s16 x, y;

    if (gBattleStruct->raid.shields > 0)
    {
        LoadSpritePalette(&sSpritePalette_RaidShield);
        LoadSpriteSheet(&sSpriteSheet_RaidShield);
    }

    GetBattlerHealthboxCoords(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), &x, &y);

    x += sShieldPosition[0] - (index * 10);
    y += sShieldPosition[1];

    if (gBattleStruct->raid.shields > 0)
    {
        spriteId = CreateSpriteAtEnd(&sSpriteTemplate_RaidShield, x, y, 0);
    }

    gSprites[spriteId].tBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
    return spriteId;
}

void CreateAllRaidShieldSprites(void)
{
    u8 i;
    for (i = 0; i < gBattleStruct->raid.shields; i++)
    {
        if (gBattleStruct->raid.shieldSpriteIds[i] == MAX_SPRITES)
            gBattleStruct->raid.shieldSpriteIds[i] = CreateRaidShieldSprite(i);
    }
}

void DestroyRaidShieldSprite(u8 index)
{
    if (gBattleStruct->raid.shieldSpriteIds[index] != MAX_SPRITES)
    {
        DestroySprite(&gSprites[gBattleStruct->raid.shieldSpriteIds[index]]);
        gBattleStruct->raid.shieldSpriteIds[index] = MAX_SPRITES;
    }

    if (index == 0)
    {
        FreeSpritePaletteByTag(TAG_RAID_SHIELD_PAL);
        FreeSpriteTilesByTag(TAG_RAID_SHIELD_TILE);
    }
}

void DestroyAllRaidShieldSprites(void)
{
    u32 i;

    for (i = 0; i < MAX_BARRIER_COUNT; i++)
    {
        if (gBattleStruct->raid.shieldSpriteIds[i] != MAX_SPRITES)
        {
            DestroySprite(&gSprites[gBattleStruct->raid.shieldSpriteIds[i]]);
            gBattleStruct->raid.shieldSpriteIds[i] = MAX_SPRITES;
        }
    }
    
    FreeSpritePaletteByTag(TAG_RAID_SHIELD_PAL);
    FreeSpriteTilesByTag(TAG_RAID_SHIELD_TILE);
}

#undef tBattler