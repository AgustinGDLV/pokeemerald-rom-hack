#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h"
#include "battle_util.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "party_menu.h"
#include "pokemon.h"
#include "international_string_util.h"
#include "item.h"
#include "item_menu.h"
#include "overworld.h"
#include "random.h"
#include "rtc.h"
#include "text.h"
#include "save.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "window.h"
#include "constants/battle_string_ids.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/trainers.h"

// constant rom data
extern struct Evolution gEvolutionTable[][EVOS_PER_MON];

static const u16 sDynamaxBandTable[] =
{
	ITEM_DYNAMAX_BAND,
};

static const u16 sMaxMoveTable[] =
{
	[TYPE_NORMAL] = MOVE_MAX_STRIKE_P,
	[TYPE_FIGHTING] = MOVE_MAX_KNUCKLE_P,
	[TYPE_FLYING] = MOVE_MAX_AIRSTREAM_P,
	[TYPE_POISON] = MOVE_MAX_OOZE_P,
	[TYPE_GROUND] = MOVE_MAX_QUAKE_P,
	[TYPE_ROCK] = MOVE_MAX_ROCKFALL_P,
	[TYPE_BUG] = MOVE_MAX_FLUTTERBY_P,
	[TYPE_GHOST] = MOVE_MAX_PHANTASM_P,
	[TYPE_STEEL] = MOVE_MAX_STEELSPIKE_P,
	[TYPE_FIRE] = MOVE_MAX_FLARE_P,
	[TYPE_WATER] = MOVE_MAX_GEYSER_P,
	[TYPE_GRASS] = MOVE_MAX_OVERGROWTH_P,
	[TYPE_ELECTRIC] = MOVE_MAX_LIGHTNING_P,
	[TYPE_PSYCHIC] = MOVE_MAX_MINDSTORM_P,
	[TYPE_ICE] = MOVE_MAX_HAILSTORM_P,
	[TYPE_DRAGON] = MOVE_MAX_WYRMWIND_P,
	[TYPE_DARK] = MOVE_MAX_DARKNESS_P,
	[TYPE_FAIRY] = MOVE_MAX_STARFALL_P,
};

struct GMaxMove
{
	u8 moveType;
	u16 gmaxMove;
};

static const struct GMaxMove sGMaxMoveTable[] =
{
	[SPECIES_VENUSAUR_GMAX] = 	          {TYPE_GRASS,      MOVE_G_MAX_VINE_LASH_P},
	[SPECIES_CHARIZARD_GMAX] =            {TYPE_FIRE,       MOVE_G_MAX_WILDFIRE_P},
	[SPECIES_BLASTOISE_GMAX] =            {TYPE_WATER,      MOVE_G_MAX_CANNONADE_P},
	[SPECIES_BUTTERFREE_GMAX] =           {TYPE_BUG,        MOVE_G_MAX_BEFUDDLE_P},
	[SPECIES_PIKACHU_GMAX] =              {TYPE_ELECTRIC,   MOVE_G_MAX_VOLT_CRASH_P},
	[SPECIES_MEOWTH_GMAX] =               {TYPE_NORMAL,     MOVE_G_MAX_GOLD_RUSH_P},
	[SPECIES_MACHAMP_GMAX] =              {TYPE_FIGHTING,   MOVE_G_MAX_CHI_STRIKE_P},
	[SPECIES_GENGAR_GMAX] =               {TYPE_GHOST,      MOVE_G_MAX_TERROR_P},
	[SPECIES_KINGLER_GMAX] =              {TYPE_WATER,      MOVE_G_MAX_FOAM_BURST_P},
	[SPECIES_LAPRAS_GMAX] =               {TYPE_ICE,        MOVE_G_MAX_RESONANCE_P},
	[SPECIES_EEVEE_GMAX] =                {TYPE_NORMAL,     MOVE_G_MAX_CUDDLE_P},
	[SPECIES_SNORLAX_GMAX] =              {TYPE_NORMAL,     MOVE_G_MAX_REPLENISH_P},
	[SPECIES_GARBODOR_GMAX] =             {TYPE_POISON,     MOVE_G_MAX_MALODOR_P},
	[SPECIES_MELMETAL_GMAX] =             {TYPE_STEEL,      MOVE_G_MAX_MELTDOWN_P},
	[SPECIES_RILLABOOM_GMAX] =            {TYPE_GRASS,      MOVE_G_MAX_DRUM_SOLO_P},
	[SPECIES_CINDERACE_GMAX] =            {TYPE_FIRE,       MOVE_G_MAX_FIREBALL_P},
	[SPECIES_INTELEON_GMAX] =             {TYPE_WATER,      MOVE_G_MAX_HYDROSNIPE_P},
	[SPECIES_CORVIKNIGHT_GMAX] =          {TYPE_FLYING,     MOVE_G_MAX_WIND_RAGE_P},
	[SPECIES_ORBEETLE_GMAX] =             {TYPE_PSYCHIC,    MOVE_G_MAX_GRAVITAS_P},
	[SPECIES_DREDNAW_GMAX] =              {TYPE_WATER,      MOVE_G_MAX_STONESURGE_P},
	[SPECIES_COALOSSAL_GMAX] =            {TYPE_ROCK,       MOVE_G_MAX_VOLCALITH_P},
	[SPECIES_FLAPPLE_GMAX] =              {TYPE_GRASS,      MOVE_G_MAX_TARTNESS_P},
	[SPECIES_APPLETUN_GMAX] =             {TYPE_GRASS,      MOVE_G_MAX_SWEETNESS_P},
	[SPECIES_SANDACONDA_GMAX] =           {TYPE_GROUND,     MOVE_G_MAX_SANDBLAST_P},
	[SPECIES_TOXTRICITY_GMAX] =           {TYPE_ELECTRIC,   MOVE_G_MAX_STUN_SHOCK_P},
	[SPECIES_CENTISKORCH_GMAX] =          {TYPE_FIRE,       MOVE_G_MAX_CENTIFERNO_P},
	[SPECIES_HATTERENE_GMAX] =			  {TYPE_FAIRY,      MOVE_G_MAX_SMITE_P},
	[SPECIES_GRIMMSNARL_GMAX] =           {TYPE_DARK,       MOVE_G_MAX_SNOOZE_P},
	[SPECIES_ALCREMIE_GMAX] =             {TYPE_FAIRY,      MOVE_G_MAX_FINALE_P},
	[SPECIES_COPPERAJAH_GMAX] =           {TYPE_STEEL,      MOVE_G_MAX_STEELSURGE_P},
	[SPECIES_DURALUDON_GMAX] =            {TYPE_DRAGON,     MOVE_G_MAX_DEPLETION_P},
	[SPECIES_URSHIFU_GMAX] =              {TYPE_DARK,       MOVE_G_MAX_ONE_BLOW_P},
	[SPECIES_URSHIFU_RAPID_STRIKE_GMAX] = {TYPE_WATER,      MOVE_G_MAX_RAPID_FLOW_P},
};

const u8 gRaidBattleStarsByBadges[][2] =
{
	[0] = {NO_RAID,         NO_RAID},
	[1] = {ONE_STAR_RAID, 	ONE_STAR_RAID},
	[2] = {ONE_STAR_RAID,   TWO_STAR_RAID},
	[3] = {TWO_STAR_RAID,   TWO_STAR_RAID},
	[4] = {TWO_STAR_RAID,   THREE_STAR_RAID},
	[5] = {THREE_STAR_RAID, THREE_STAR_RAID},
	[6] = {THREE_STAR_RAID, FOUR_STAR_RAID},
	[7] = {FOUR_STAR_RAID,  FOUR_STAR_RAID},
	[8] = {FOUR_STAR_RAID,  FIVE_STAR_RAID},
	[9] = {FIVE_STAR_RAID,  SIX_STAR_RAID}, //Beat Game
};

const u8 gRaidBattleLevelRanges[RAID_STAR_COUNT][2] =
{
	[ONE_STAR_RAID]   = {15, 20},
	[TWO_STAR_RAID]   = {25, 30},
	[THREE_STAR_RAID] = {35, 40},
	[FOUR_STAR_RAID]  = {50, 55},
	[FIVE_STAR_RAID]  = {60, 65},
	[SIX_STAR_RAID]   = {75, 90},
};

//The chance that each move is replaced with an Egg Move
const u8 gRaidBattleEggMoveChances[RAID_STAR_COUNT] =
{
	[ONE_STAR_RAID]   = 0,
	[TWO_STAR_RAID]   = 10,
	[THREE_STAR_RAID] = 30,
	[FOUR_STAR_RAID]  = 50,
	[FIVE_STAR_RAID]  = 70,
	[SIX_STAR_RAID]   = 70,
};

static const u8 sRaidBattleDropRates[MAX_RAID_DROPS] =
{	//In percent
	100,
	80,
	80,
	50,
	50,
	30,
	30,
	25,
	25,
	5,
	4,
	1,
};

static const u16 sRaidHPMultipliers[] =
{
    [1]     = UQ_4_12(1.2),
    [2]     = UQ_4_12(1.3),
    [3]     = UQ_4_12(1.5),
    [4]     = UQ_4_12(1.7),
    [5]     = UQ_4_12(2.0),
    [6]     = UQ_4_12(2.0),
};

#include "data/raid_encounters.h"
#include "data/raid_partners.h"

// this file's functions
static u16 FindPlayerDynamaxBand(void);
static u16 FindTrainerDynamaxBand(u16 trainerId);

// EWRAM data
EWRAM_DATA u8 gRaidBattleStars = 0;
EWRAM_DATA u8 gRaidBattleLevel = 0;
EWRAM_DATA u16 gRaidBattleSpecies = 0;

// code:
bool8 CanDynamax(u16 battlerId)
{
	u8 side = GetBattlerSide(battlerId);
	if (gBattleTypeFlags & BATTLE_TYPE_RAID
		//&& gBattleStruct->raid.energyPos == battlerId
		&& !(gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId])
		&& !(gBattleStruct->dynamax.dynamaxedIds & gBitTable[BATTLE_PARTNER(battlerId)])
		&& !(gBattleStruct->dynamax.toDynamax & gBitTable[BATTLE_PARTNER(battlerId)])
		&& !gBattleStruct->dynamax.alreadyDynamaxed[side])
		return TRUE;
	
	else if (FlagGet(FLAG_DYNAMAX_BATTLE) 
			&& side == B_SIDE_PLAYER && FindPlayerDynamaxBand()
		&& !(gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId])
		&& !(gBattleStruct->dynamax.dynamaxedIds & gBitTable[BATTLE_PARTNER(battlerId)])
		&& !(gBattleStruct->dynamax.toDynamax & gBitTable[BATTLE_PARTNER(battlerId)])
		&& !gBattleStruct->dynamax.alreadyDynamaxed[side])
		return TRUE;

	return FALSE;
}

void ApplyDynamaxHPMultiplier(struct Pokemon *mon)
{
    u16 hp, maxHP, multiplier;

	if (GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_SHEDINJA)
		return;

    hp = GetMonData(mon, MON_DATA_HP, NULL);
    maxHP = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    multiplier = UQ_4_12(2.0); // placeholder; TODO: Dynamax level

    hp = UQ_4_12_TO_INT((hp * multiplier) + UQ_4_12_ROUND);
    maxHP = UQ_4_12_TO_INT((maxHP * multiplier) + UQ_4_12_ROUND);

    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
}

void PrepareBattlerForDynamax(u16 battlerId)
{
	u8 i;
	struct Pokemon *mon;
	u8 side = GetBattlerSide(battlerId);

	gBattleStruct->dynamax.alreadyDynamaxed[side] = TRUE;
	gBattleStruct->dynamax.dynamaxedIds |= gBitTable[battlerId];
	gBattleStruct->dynamax.dynamaxTurns[side] = DYNAMAX_TURNS;
	for (i = 0; i < MAX_MON_MOVES; i++)
	{
		gBattleStruct->dynamax.ppBuffer[side][i] = gBattleMons[battlerId].pp[i];
		gBattleMons[battlerId].pp[i] = 10;
	}

	if (side == B_SIDE_PLAYER)
		mon = &gPlayerParty[gBattlerPartyIndexes[battlerId]];
	else
		mon = &gEnemyParty[gBattlerPartyIndexes[battlerId]];

	RecalcBattlerStats(battlerId, mon);
}

void UndoDynamax(u16 battlerId)
{
	u16 hp, maxHP, i;
	struct Pokemon *mon;
	u8 side = GetBattlerSide(battlerId);	

	gBattleStruct->dynamax.dynamaxedIds &= ~gBitTable[gActiveBattler];

	for (i = 0; i < MAX_MON_MOVES; i++) // revert move PP
	{
		gBattleMons[battlerId].pp[i] = gBattleStruct->dynamax.ppBuffer[side][i];
	}

	if (side == B_SIDE_PLAYER) // revert HP multiplier
		mon = &gPlayerParty[gBattlerPartyIndexes[battlerId]];
	else
		mon = &gEnemyParty[gBattlerPartyIndexes[battlerId]];
	RecalcBattlerStats(battlerId, mon);

	DestroyDynamaxIndicatorSprite(gHealthboxSpriteIds[battlerId]);
	UpdateHealthboxAttribute(gHealthboxSpriteIds[battlerId], mon, HEALTHBOX_ALL); // update healthbox
}

bool8 ShouldUseMaxMove(u16 battlerId, u16 baseMove)
{
	if (IsRaidBoss(battlerId))
		return !IsRaidBossUsingRegularMove(battlerId, baseMove);
	else if (gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId]
			 || gBattleStruct->dynamax.toDynamax & gBitTable[battlerId])
		return TRUE;
	return FALSE;
}

u16 GetMaxMove(u16 battlerId, u16 baseMove)
{
	u8 side = GetBattlerSide(battlerId);
	u16 move = baseMove;
	u16 species;

	if (baseMove == MOVE_NONE)
		return MOVE_NONE;

	if (side == B_SIDE_PLAYER) // Gigantamax Check
		species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_SPECIES, NULL);
	else
		species = GetMonData(&gEnemyParty[gBattlerPartyIndexes[battlerId]], MON_DATA_SPECIES, NULL);

	if (gBattleMoves[baseMove].split == SPLIT_STATUS)
		move = MOVE_MAX_GUARD;
	else if (IsGigantamaxSpecies(species)
			 && sGMaxMoveTable[species].moveType == gBattleMoves[baseMove].type)
		move = sGMaxMoveTable[species].gmaxMove;
	else
		move = sMaxMoveTable[gBattleMoves[baseMove].type];
	
	if (gBattleMoves[baseMove].split == SPLIT_SPECIAL)
		move++; // physical max move IDs are followed by their special counterpart
	
	return move;
}

bool8 IsMaxMove(u16 move)
{
	return move >= MAX_MOVES_START && move <= MOVE_G_MAX_RAPID_FLOW_S;
}

bool8 ShouldDisplayMaxMoveInfo(u16 battlerId)
{
	if (gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId] ||
		(gBattleStruct->dynamax.playerSelect && !(gBattleStruct->dynamax.toDynamax & gBitTable[BATTLE_PARTNER(gActiveBattler)])))
		return TRUE;
	return FALSE;
}

bool8 IsGigantamaxSpecies(u16 species)
{
    return species >= SPECIES_VENUSAUR_GMAX && species <= SPECIES_URSHIFU_RAPID_STRIKE_GMAX;
}

u16 GetGigantamaxSpecies(u16 species, bool8 canGigantamax)
{
	u32 i;
	const struct Evolution* evolutions = gEvolutionTable[species];

	if (canGigantamax) //Mon can Gigantamax
	{
		for (i = 0; i < EVOS_PER_MON; i++)
		{
			if (evolutions[i].method == EVO_GIGANTAMAX)
				return evolutions[i].targetSpecies;
		}
	}

	return SPECIES_NONE;
}

u16 GetGigantamaxBaseForm(u16 species)
{
	u16 baseForm = GET_BASE_SPECIES_ID(GetMonData(&gPlayerParty[species], MON_DATA_SPECIES));

    if (baseForm != species)
        return baseForm;

	return SPECIES_NONE;
}

static bool8 IsItemDynamaxBand(u16 item)
{
    u8 i;
	for (i = 0; i < ARRAY_COUNT(sDynamaxBandTable); i++)
	{
		if (item == sDynamaxBandTable[i])
			return TRUE;
	}

	return FALSE;
}

static u16 FindTrainerDynamaxBand(u16 trainerId)
{
    u8 i;
	if (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_LINK))
		return ITEM_DYNAMAX_BAND;

	for (i = 0; i < MAX_TRAINER_ITEMS; i++)
	{
		if (IsItemDynamaxBand(gTrainers[trainerId].items[i]))
			return gTrainers[trainerId].items[i];
	}

	return ITEM_NONE;
}

static u16 FindPlayerDynamaxBand(void)
{
    u8 i;
	if (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_LINK))
		return ITEM_DYNAMAX_BAND;
	
    for (i = 0; i < ARRAY_COUNT(sDynamaxBandTable); ++i)
    {
        if (CheckBagHasItem(sDynamaxBandTable[i], 1))
            return sDynamaxBandTable[i];
    }
    
    return ITEM_NONE;
}

static u8 GetRaidMapSectionId(void)
{
    return GetCurrentRegionMapSectionId();
}

static u32 GetRaidRandomNumber(void)
{
	//Make sure no values are 0
	//u8 dayOfWeek = (gClock.dayOfWeek == 0) ? 8 : gClock.dayOfWeek;
	//u8 day = (gClock.day == 0) ? 32 : gClock.day;
	//u8 month = (gClock.month == 0) ? 13 : gClock.month;
    u8 hour = (RtcGetMinuteCount() / 60 == 0) ? 24 : RtcGetMinuteCount() / 60;
	u8 lastMapGroup = (gSaveBlock1Ptr->dynamicWarp.mapGroup == 0) ? 0xFF : gSaveBlock1Ptr->dynamicWarp.mapGroup;
	u8 lastMapNum = (gSaveBlock1Ptr->dynamicWarp.mapNum == 0) ? 0xFF : gSaveBlock1Ptr->dynamicWarp.mapNum;
	u8 lastWarpId = (gSaveBlock1Ptr->dynamicWarp.warpId == 0) ? 0xFF : gSaveBlock1Ptr->dynamicWarp.warpId;
	u16 lastPos = (gSaveBlock1Ptr->dynamicWarp.x + gSaveBlock1Ptr->dynamicWarp.y == 0) ? 0xFFFF : (u16) (gSaveBlock1Ptr->dynamicWarp.x + gSaveBlock1Ptr->dynamicWarp.y);
	#ifdef VAR_RAID_NUMBER_OFFSET
	u16 offset = VarGet(VAR_RAID_NUMBER_OFFSET); //Setting this var changes all the raid spawns for the current hour (helps with better Wishing Piece)
	#else
	u16 offset = 0;
	#endif

	//return ((hour * (day + month) * lastMapGroup * (lastMapNum + lastWarpId + lastPos)) + ((hour * (day + month)) ^ dayOfWeek) + offset) ^ T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    return ((hour * lastMapGroup * (lastMapNum + lastWarpId + lastPos)) + ((hour) ^ 8) + offset) ^ T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
}

static bool8 ShouldTryGigantamaxRaidMon(void)
{
	return gRaidBattleStars >= 6 //6-star Raid
		&& (GetRaidRandomNumber() % 100 >= 95 || GetRaidRandomNumber() % 100 < 20); //25% chance
}

void DetermineRaidStars(void)
{
	u16 numBadges, badgeFlag, min, max;
	u32 randomNum = GetRaidRandomNumber();

    for (badgeFlag = FLAG_BADGE01_GET; badgeFlag < FLAG_BADGE01_GET + NUM_BADGES; badgeFlag++)
    {
        if (FlagGet(badgeFlag))
            numBadges++;
    }

    min = gRaidBattleStarsByBadges[numBadges][0];
	max = gRaidBattleStarsByBadges[numBadges][1];

    gRaidBattleStars = 3;
    return;

	if (min == max)
		gRaidBattleStars = min;
	else
		gRaidBattleStars = (randomNum % ((max + 1) - min)) + min;
}

//Must call DetermineRaidStars first
void DetermineRaidSpecies(void)
{
	u16 index, altSpecies;
	u8 numStars = gRaidBattleStars;
	const struct RaidData* raid = &gRaidsByMapSection[GetCurrentRegionMapSectionId()][numStars];

	if (raid->data != NULL)
	{
		index = GetRaidRandomNumber() % raid->amount;
		gRaidBattleSpecies = raid->data[index].species;

		if (ShouldTryGigantamaxRaidMon())
		{
			altSpecies = GetGigantamaxSpecies(raid->data[index].species, TRUE);
			if (altSpecies != SPECIES_NONE)
				gRaidBattleSpecies = altSpecies; //Update with Gigantamax form
		}
	}
	else
		gRaidBattleSpecies = SPECIES_NONE;
}

void DetermineRaidLevel(void)
{
	u8 numStars = gRaidBattleStars;
	u8 min = gRaidBattleLevelRanges[numStars][0];
	u8 max = gRaidBattleLevelRanges[numStars][1];
	u32 randomNum = GetRaidRandomNumber();

    if (min == max)
		gRaidBattleLevel = min;
	else
		gRaidBattleLevel = (randomNum % (max - min)) + min;
}

u8 GetRandomRaidLevel(void)
{
	u8 numStars = gRaidBattleStars;
    u8 min = gRaidBattleLevelRanges[numStars][0];
    u8 max = gRaidBattleLevelRanges[numStars][1];
	return (Random() % (max - min)) + min;
}

u8 GetRaidRecommendedLevel(void)
{
	u8 numStars = gRaidBattleStars;
	return gRaidBattleLevelRanges[numStars][1] + 5; //Max level + 5
}

void DetermineRaidPartners(bool8* checkedPartners, u8 maxPartners)
{
	u32 i, index;
	u8 numStars = gRaidBattleStars;
	u16 numMarked = 0;
	u16 numViable = 0;
	u32 randomNum = GetRaidRandomNumber();

	for (i = 1; i < /*1000*/ 0xFFFFFFFF; i++)
	{
		if (randomNum == 0) //0 causes an infinite loop
			randomNum = 0xFFFFFFFF;

		randomNum ^= i;
		index = randomNum % gNumRaidPartners;

		if (checkedPartners[index] == 0)
		{
			numMarked++;

			if (gRaidPartners[index].sets[numStars] != NULL)
			{
				checkedPartners[index] = TRUE;
				numViable++;
			}
			else
				checkedPartners[index] = 0xFF;
		}

		if (numViable >= maxPartners) //Found at least 3 partners
			return;

		if (numMarked >= gNumRaidPartners)
			break;
	}

	if (numMarked < gNumRaidPartners) //Couldn't mark off everyone
	{
		for (i = 0; i < gNumRaidPartners; i++)
		{
			if (gRaidPartners[i].sets[numStars] != NULL)
			{
				checkedPartners[i] = TRUE;
				numViable++;
			}

			if (numViable >= maxPartners) //Found at least 3 partners
				return;
		}
	}
}

const struct FixedMonSet *GetRaidMultiSpread(u8 partnerNum, u8 partyIndex, u8 starRating)
{
    return &gRaidPartners[partnerNum].sets[starRating][partyIndex];
}

u8 GetRaidSpeciesAbilityNum(u16 species)
{
	u32 i = 0;
	u8 numStars = gRaidBattleStars;
	const struct RaidData* raid = &gRaidsByMapSection[GetRaidMapSectionId()][numStars];

	if (IsGigantamaxSpecies(species))
		return RAID_ABILITY_RANDOM_ALL; //Gigantamax Pokemon can have any one of their abilities

	if (raid->data != NULL)
	{
		for (i = 0; i < raid->amount; i++)
		{
			if (raid->data[i].species == species) //Max one species per dataset
				return raid->data[i].ability;
		}
	}

	return RAID_ABILITY_RANDOM_1_2;
}

// battle functions

bool8 IsRaidBoss(u16 battlerId)
{
    return gBattleTypeFlags & BATTLE_TYPE_RAID && GetBattlerPosition(battlerId) == B_POSITION_OPPONENT_LEFT;
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

bool8 IsRaidBossUsingRegularMove(u8 battlerId, u16 baseMove)
{
	return gBattleMoves[baseMove].split == SPLIT_STATUS
		|| baseMove == MOVE_STRUGGLE
		|| (gRaidBattleStars < 4 && (gRandomTurnNumber & 3) == 0) //25 % chance to use regular damaging move
		|| (gRaidBattleStars >= 4 && (gRandomTurnNumber % 100 >= 90)) //Harder foes have a lower chance of using regular moves
		|| gBattleMons[battlerId].status2 & (STATUS2_RECHARGE | STATUS2_MULTIPLETURNS);
}

void InitRaidVariables(void)
{
    u8 i;

    gBattleStruct->raid.stars = gSpecialVar_0x8008; // variable is set before battle starts.
    gBattleStruct->raid.shields = 0;
    gBattleStruct->raid.storedDmg = 0; // used to "release" damage when barriers break.
    gBattleStruct->raid.thresholdsRemaining = GetRaidThresholdNumber();
    gBattleStruct->raid.stormTurns = 0;
    gBattleStruct->raid.state = INTRO_COMPLETED;
	gBattleStruct->raid.energyPos = B_POSITION_PLAYER_LEFT;

    for (i = 0; i < MAX_BARRIER_COUNT; i++)
    {
        gBattleStruct->raid.shieldSpriteIds[i] = MAX_SPRITES;
    }

    RecalcBattlerStats(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), &gEnemyParty[0]); // to apply HP multiplier
}

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

void ApplyRaidHPMultiplier(struct Pokemon *mon)
{
    u16 hp, maxHP, multiplier;

	if (GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_SHEDINJA)
		return;

    hp = GetMonData(mon, MON_DATA_HP, NULL);
    maxHP = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    multiplier = sRaidHPMultipliers[gBattleStruct->raid.stars];

    hp = UQ_4_12_TO_INT((hp * multiplier) + UQ_4_12_ROUND);
    maxHP = UQ_4_12_TO_INT((maxHP * multiplier) + UQ_4_12_ROUND);

    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
}

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


// Used for opening the bag in the end sequence.
void CB2_ChooseBall(void)
{
    GoToBagMenu(ITEMMENULOCATION_BERRY_TREE, BALLS_POCKET, CB2_SetUpReshowBattleScreenAfterMenu2);
}

// Sprite data and functions:

// Raid Shields
static const u16 sRaidShieldGfx[] = INCBIN_U16("graphics/battle_interface/raid_barrier.4bpp");
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
#define tHide       data[1]
#define hOther_IndicatorSpriteId data[6]

u32 CreateRaidShieldSprite(u8 index)
{
    u32 spriteId, position;
    s16 x, y;
    
    GetBattlerHealthboxCoords(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), &x, &y);
    x += sShieldPosition[0] - (index * 10);
    y += sShieldPosition[1];

   	LoadSpritePalette(&sSpritePalette_RaidShield);
   	LoadSpriteSheet(&sSpriteSheet_RaidShield);
    spriteId = CreateSprite(&sSpriteTemplate_RaidShield, x, y, 0);

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

// Dynamax Trigger

static const u8 sDynamaxTriggerGfx[] = INCBIN_U8("graphics/battle_interface/dynamax_trigger.4bpp");
static const u16 sDynamaxTriggerPal[] = INCBIN_U16("graphics/battle_interface/dynamax_trigger.gbapal");

static const struct SpriteSheet sSpriteSheet_DynamaxTrigger =
{
    sDynamaxTriggerGfx, sizeof(sDynamaxTriggerGfx), TAG_DYNAMAX_TRIGGER_TILE
};
static const struct SpritePalette sSpritePalette_DynamaxTrigger =
{
    sDynamaxTriggerPal, TAG_DYNAMAX_TRIGGER_PAL
};

static const struct OamData sOamData_DynamaxTrigger =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = ST_OAM_SQUARE,
    .x = 0,
    .matrixNum = 0,
    .size = 2,
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sSpriteAnim_DynamaxTriggerOff[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_DynamaxTriggerOn[] =
{
    ANIMCMD_FRAME(16, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_DynamaxTrigger[] =
{
    sSpriteAnim_DynamaxTriggerOff,
    sSpriteAnim_DynamaxTriggerOn,
};

static void SpriteCb_DynamaxTrigger(struct Sprite *sprite);

static const struct SpriteTemplate sSpriteTemplate_DynamaxTrigger =
{
    .tileTag = TAG_DYNAMAX_TRIGGER_TILE,
    .paletteTag = TAG_DYNAMAX_TRIGGER_PAL,
    .oam = &sOamData_DynamaxTrigger,
    .anims = sSpriteAnimTable_DynamaxTrigger,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_DynamaxTrigger
};

void ChangeDynamaxTriggerSprite(u8 spriteId, u8 animId)
{
    StartSpriteAnim(&gSprites[spriteId], animId);
}

#define SINGLES_DYNAMAX_TRIGGER_POS_X_OPTIMAL (30)
#define SINGLES_DYNAMAX_TRIGGER_POS_X_PRIORITY (31)
#define SINGLES_DYNAMAX_TRIGGER_POS_X_SLIDE (15)
#define SINGLES_DYNAMAX_TRIGGER_POS_Y_DIFF (-11)

#define DOUBLES_DYNAMAX_TRIGGER_POS_X_OPTIMAL (30)
#define DOUBLES_DYNAMAX_TRIGGER_POS_X_PRIORITY (31)
#define DOUBLES_DYNAMAX_TRIGGER_POS_X_SLIDE (15)
#define DOUBLES_DYNAMAX_TRIGGER_POS_Y_DIFF (-4)

void CreateDynamaxTriggerSprite(u8 battlerId, u8 palId)
{
    LoadSpritePalette(&sSpritePalette_DynamaxTrigger);
    if (GetSpriteTileStartByTag(TAG_DYNAMAX_TRIGGER_TILE) == 0xFFFF)
        LoadSpriteSheet(&sSpriteSheet_DynamaxTrigger);
    if (gBattleStruct->dynamax.triggerSpriteId == 0xFF)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
            gBattleStruct->dynamax.triggerSpriteId = CreateSprite(&sSpriteTemplate_DynamaxTrigger,
                                                             gSprites[gHealthboxSpriteIds[battlerId]].x - DOUBLES_DYNAMAX_TRIGGER_POS_X_SLIDE,
                                                             gSprites[gHealthboxSpriteIds[battlerId]].y - DOUBLES_DYNAMAX_TRIGGER_POS_Y_DIFF, 0);
        else
            gBattleStruct->dynamax.triggerSpriteId = CreateSprite(&sSpriteTemplate_DynamaxTrigger,
                                                             gSprites[gHealthboxSpriteIds[battlerId]].x - SINGLES_DYNAMAX_TRIGGER_POS_X_SLIDE,
                                                             gSprites[gHealthboxSpriteIds[battlerId]].y - SINGLES_DYNAMAX_TRIGGER_POS_Y_DIFF, 0);
    }
    gSprites[gBattleStruct->dynamax.triggerSpriteId].tBattler = battlerId;
    gSprites[gBattleStruct->dynamax.triggerSpriteId].tHide = FALSE;

    ChangeDynamaxTriggerSprite(gBattleStruct->dynamax.triggerSpriteId, palId);
}

static void SpriteCb_DynamaxTrigger(struct Sprite *sprite)
{
    s32 xSlide, xPriority, xOptimal;
    s32 yDiff;

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        xSlide = DOUBLES_DYNAMAX_TRIGGER_POS_X_SLIDE;
        xPriority = DOUBLES_DYNAMAX_TRIGGER_POS_X_PRIORITY;
        xOptimal = DOUBLES_DYNAMAX_TRIGGER_POS_X_OPTIMAL;
        yDiff = DOUBLES_DYNAMAX_TRIGGER_POS_Y_DIFF;
    }
    else
    {
        xSlide = SINGLES_DYNAMAX_TRIGGER_POS_X_SLIDE;
        xPriority = SINGLES_DYNAMAX_TRIGGER_POS_X_PRIORITY;
        xOptimal = SINGLES_DYNAMAX_TRIGGER_POS_X_OPTIMAL;
        yDiff = SINGLES_DYNAMAX_TRIGGER_POS_Y_DIFF;
    }

    if (sprite->tHide)
    {
        if (sprite->x != gSprites[gHealthboxSpriteIds[sprite->tBattler]].x - xSlide)
            sprite->x++;

        if (sprite->x >= gSprites[gHealthboxSpriteIds[sprite->tBattler]].x - xPriority)
            sprite->oam.priority = 2;
        else
            sprite->oam.priority = 1;

        sprite->y = gSprites[gHealthboxSpriteIds[sprite->tBattler]].y - yDiff;
        sprite->y2 = gSprites[gHealthboxSpriteIds[sprite->tBattler]].y2 - yDiff;
        if (sprite->x == gSprites[gHealthboxSpriteIds[sprite->tBattler]].x - xSlide)
            DestroyDynamaxTriggerSprite();
    }
    else
    {
        if (sprite->x != gSprites[gHealthboxSpriteIds[sprite->tBattler]].x - xOptimal)
            sprite->x--;

        if (sprite->x >= gSprites[gHealthboxSpriteIds[sprite->tBattler]].x - xPriority)
            sprite->oam.priority = 2;
        else
            sprite->oam.priority = 1;

        sprite->y = gSprites[gHealthboxSpriteIds[sprite->tBattler]].y - yDiff;
        sprite->y2 = gSprites[gHealthboxSpriteIds[sprite->tBattler]].y2 - yDiff;
    }
}

bool32 IsDynamaxTriggerSpriteActive(void)
{
    if (GetSpriteTileStartByTag(TAG_DYNAMAX_TRIGGER_TILE) == 0xFFFF)
        return FALSE;
    else if (IndexOfSpritePaletteTag(TAG_DYNAMAX_TRIGGER_PAL) != 0xFF)
        return TRUE;
    else
        return FALSE;
}

void HideDynamaxTriggerSprite(void)
{
    if (gBattleStruct->dynamax.triggerSpriteId != 0xFF)
    {
        ChangeDynamaxTriggerSprite(gBattleStruct->dynamax.triggerSpriteId, 0);
        gSprites[gBattleStruct->dynamax.triggerSpriteId].tHide = TRUE;
    }
}

void DestroyDynamaxTriggerSprite(void)
{
    FreeSpritePaletteByTag(TAG_DYNAMAX_TRIGGER_PAL);
    FreeSpriteTilesByTag(TAG_DYNAMAX_TRIGGER_TILE);
    if (gBattleStruct->dynamax.triggerSpriteId != 0xFF)
        DestroySprite(&gSprites[gBattleStruct->dynamax.triggerSpriteId]);
    gBattleStruct->dynamax.triggerSpriteId = 0xFF;
}

// Dynamax Indicator

static const u8 sDynamaxIndicatorGfx[] = INCBIN_U8("graphics/battle_interface/dynamax_indicator.4bpp");
static const u16 sDynamaxIndicatorPal[] = INCBIN_U16("graphics/battle_interface/dynamax_indicator.gbapal");

static const struct SpriteSheet sSpriteSheet_DynamaxIndicator =
{
    sDynamaxIndicatorGfx, sizeof(sDynamaxIndicatorGfx), TAG_DYNAMAX_INDICATOR_TILE
};
static const struct SpritePalette sSpritePalette_DynamaxIndicator =
{
    sDynamaxIndicatorPal, TAG_DYNAMAX_INDICATOR_PAL
};

static const struct OamData sOamData_DynamaxIndicator =
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

static void SpriteCb_DynamaxIndicator(struct Sprite *sprite);

static const struct SpriteTemplate sSpriteTemplate_DynamaxIndicator =
{
    .tileTag = TAG_DYNAMAX_INDICATOR_TILE,
    .paletteTag = TAG_DYNAMAX_INDICATOR_PAL,
    .oam = &sOamData_DynamaxIndicator,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCb_DynamaxIndicator,
};

static const s8 sIndicatorPositions[][2] =
{
    [B_POSITION_PLAYER_LEFT] = {52, -9},
    [B_POSITION_OPPONENT_LEFT] = {44, -9},
    [B_POSITION_PLAYER_RIGHT] = {52, -9},
    [B_POSITION_OPPONENT_RIGHT] = {44, -9},
};

u8 GetDynamaxIndicatorSpriteId(u32 healthboxSpriteId)
{
    u8 spriteId = gSprites[healthboxSpriteId].oam.affineParam;
    if (spriteId >= MAX_SPRITES)
        return 0xFF;
    return gSprites[spriteId].hOther_IndicatorSpriteId;
}

u32 CreateDynamaxIndicatorSprite(u32 battlerId)
{
    u32 spriteId, position;
    s16 x, y;

    if (gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId])
    {
        LoadSpritePalette(&sSpritePalette_DynamaxIndicator);
        LoadSpriteSheet(&sSpriteSheet_DynamaxIndicator);
    }

    position = GetBattlerPosition(battlerId);
    GetBattlerHealthboxCoords(battlerId, &x, &y);

    x += sIndicatorPositions[position][0];
    y += sIndicatorPositions[position][1];

    if (gBattleMons[battlerId].level >= 100)
        x -= 4;
    else if (gBattleMons[battlerId].level < 10)
        x += 5;

   if (gBattleStruct->dynamax.dynamaxedIds & gBitTable[battlerId])
    {
        spriteId = CreateSpriteAtEnd(&sSpriteTemplate_DynamaxIndicator, x, y, 0);
    }

    gSprites[gSprites[gHealthboxSpriteIds[battlerId]].oam.affineParam].hOther_IndicatorSpriteId = spriteId;
    gSprites[spriteId].tBattler = battlerId;
    return spriteId;
}

void DestroyDynamaxIndicatorSprite(u32 healthboxSpriteId)
{
    u32 i;
    s16 *spriteId = &gSprites[gSprites[healthboxSpriteId].oam.affineParam].hOther_IndicatorSpriteId;

    if (*spriteId != 0xFF)
    {
        DestroySprite(&gSprites[*spriteId]);
        *spriteId = 0xFF;
    }

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (gSprites[gSprites[gHealthboxSpriteIds[i]].oam.affineParam].hOther_IndicatorSpriteId != 0xFF)
            break;
    }
    // Free Sprite pal/tiles only if no indicator sprite is active for all battlers.
    if (i == MAX_BATTLERS_COUNT)
    {
        FreeSpritePaletteByTag(TAG_DYNAMAX_INDICATOR_PAL);
        FreeSpriteTilesByTag(TAG_DYNAMAX_INDICATOR_TILE);
    }
}

static void SpriteCb_DynamaxIndicator(struct Sprite *sprite)
{
    u8 healthboxSpriteId = gBattleSpritesDataPtr->battleBars[GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)].healthboxSpriteId;
    sprite->y2 = gSprites[healthboxSpriteId].y2;
}

#undef tBattler
#undef tHide
#undef hOther_IndicatorSpriteId
