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

// Resets raid variables at the start of battle. Called in TryDoEventsBeforeFirstTurn.
void InitRaidVariables(void)
{
    u8 i;
    gBattleStruct->raid.starRating = gSpecialVar_0x8000; // variable is set when battle is created.
    gBattleStruct->raid.barriers = 0;
    gBattleStruct->raid.storedDmg = 0;
    gBattleStruct->raid.thresholdsRemaining = GetRaidThresholdNumber();
    for (i = 0; i < MAX_BARRIERS; i++)
    {
        gBattleStruct->raid.barrierSpriteIds[i] = MAX_SPRITES;
    }
}

// Returns how many barriers to create at a threshold. Based on star rating and stats.
u8 GetRaidBarrierNumber(void)
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
    
    if ((hp - dmg) <= GetNextHealthThreshold())
        return TRUE;
    else
        return FALSE;
}

// ================= SPRITE DATA ========================================================

static const u8 sRaidBarrierGfx[] = INCBIN_U8("graphics/battle_interface/raid_barrier.4bpp");
static const u16 sRaidBarrierPal[] = INCBIN_U16("graphics/battle_interface/raid_barrier.gbapal");

static const struct SpriteSheet sSpriteSheet_RaidBarrier =
{
    sRaidBarrierGfx, sizeof(sRaidBarrierGfx), TAG_RAID_BARRIER_TILE
};
static const struct SpritePalette sSpritePalette_RaidBarrier =
{
    sRaidBarrierPal, TAG_RAID_BARRIER_PAL
};

static const struct OamData sOamData_RaidBarrier =
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

static const s8 sBarrierPosition[2] = {48, 9};

// Sync up barrier sprites with healthbox.
static void SpriteCb_RaidBarrier(struct Sprite *sprite)
{
    u8 healthboxSpriteId = gBattleSpritesDataPtr->battleBars[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].healthboxSpriteId;
    sprite->y2 = gSprites[healthboxSpriteId].y2;
}

static const struct SpriteTemplate sSpriteTemplate_RaidBarrier =
{
    .tileTag = TAG_RAID_BARRIER_TILE,
    .paletteTag = TAG_RAID_BARRIER_PAL,
    .oam = &sOamData_RaidBarrier,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_RaidBarrier,
};

#define tBattler    data[0]

u32 CreateRaidBarrierSprite(u8 index)
{
    u32 spriteId, position;
    s16 x, y;

    if (gBattleStruct->raid.barriers > 0)
    {
        LoadSpritePalette(&sSpritePalette_RaidBarrier);
        LoadSpriteSheet(&sSpriteSheet_RaidBarrier);
    }

    GetBattlerHealthboxCoords(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), &x, &y);

    x += sBarrierPosition[0] - (index * 10);
    y += sBarrierPosition[1];

    if (gBattleStruct->raid.barriers > 0)
    {
        spriteId = CreateSpriteAtEnd(&sSpriteTemplate_RaidBarrier, x, y, 0);
    }

    gSprites[spriteId].tBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
    return spriteId;
}

void CreateAllRaidBarrierSprites(void)
{
    u8 i;
    for (i = 0; i < gBattleStruct->raid.barriers; i++)
    {
        if (gBattleStruct->raid.barrierSpriteIds[i] == MAX_SPRITES)
            gBattleStruct->raid.barrierSpriteIds[i] = CreateRaidBarrierSprite(i);
    }
}

void DestroyRaidBarrierSprite(u8 index)
{
    if (gBattleStruct->raid.barrierSpriteIds[index] != MAX_SPRITES)
    {
        DestroySprite(&gSprites[gBattleStruct->raid.barrierSpriteIds[index]]);
        gBattleStruct->raid.barrierSpriteIds[index] = MAX_SPRITES;
    }

    if (index == 0)
    {
        FreeSpritePaletteByTag(TAG_RAID_BARRIER_PAL);
        FreeSpriteTilesByTag(TAG_RAID_BARRIER_TILE);
    }
}

void DestroyAllRaidBarrierSprites(void)
{
    u32 i;

    for (i = 0; i < MAX_BARRIERS; i++)
    {
        if (gBattleStruct->raid.barrierSpriteIds[i] != MAX_SPRITES)
        {
            DestroySprite(&gSprites[gBattleStruct->raid.barrierSpriteIds[i]]);
            gBattleStruct->raid.barrierSpriteIds[i] = MAX_SPRITES;
        }
    }
    
    FreeSpritePaletteByTag(TAG_RAID_BARRIER_PAL);
    FreeSpriteTilesByTag(TAG_RAID_BARRIER_TILE);
}

#undef tBattler