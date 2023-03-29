#ifndef GUARD_BATTLE_RAID_H
#define GUARD_BATTLE_RAID_H

// Config
#define B_REQUIRE_DYNAMAX_BAND			TRUE	// If set to TRUE, players and trainers must have a Dynamax Band in order to Dynamax in trainer battles.

// Functions
bool8 CanDynamax(u16 battlerId);
void ApplyDynamaxHPMultiplier(struct Pokemon *mon);
void PrepareBattlerForDynamax(u16 battlerId);
void UndoDynamax(u16 battlerId);
bool8 ShouldUseMaxMove(u16 battlerId, u16 move);
u16 GetMaxMove(u16 battlerId, u16 baseMove);
bool8 ShouldDisplayMaxMoveInfo(u16 battlerId);
bool8 IsMaxMove(u16 move);
bool8 IsGigantamaxSpecies(u16 species);
u16 GetGigantamaxSpecies(u16 species, bool8 canGigantamax);
u16 GetGigantamaxBaseForm(u16 species);

void DetermineRaidStars(void);
void DetermineRaidSpecies(void);
void DetermineRaidLevel(void);
u8 GetRandomRaidLevel(void);
u8 GetRaidRecommendedLevel(void);
void DetermineRaidPartners(bool8* checkedPartners, u8 maxPartners);
const struct FixedMonSet *GetRaidMultiSpread(u8 partnerNum, u8 partyIndex, u8 starRating);
u8 GetRaidSpeciesAbilityNum(u16 species);

bool8 IsRaidBoss(u16 battlerId);
bool8 DoesRaidPreventMove(u16 move);
u8 GetRaidRepeatedAttackChance(void);
u8 GetRaidNullificationChance(void);
bool8 IsRaidBossUsingRegularMove(u8 bank, u16 baseMove);

void InitRaidVariables(void);
void IncrementRaidStorm(void);
void ApplyRaidHPMultiplier(struct Pokemon *mon);
u8 GetRaidShieldNumber(void);
u8 GetRaidThresholdNumber(void);
u32 GetNextHealthThreshold(void);
bool8 ShouldCreateBarrier(s32 dmg);

void CB2_ChooseBall(void);

u32 CreateRaidShieldSprite(u8 index);
void CreateAllRaidShieldSprites(void);
void DestroyRaidShieldSprite(u8 index);
void DestroyAllRaidShieldSprites(void);

void ChangeDynamaxTriggerSprite(u8 spriteId, u8 animId);
void CreateDynamaxTriggerSprite(u8 battlerId, u8 palId);
bool32 IsDynamaxTriggerSpriteActive(void);
void HideDynamaxTriggerSprite(void);
void DestroyDynamaxTriggerSprite(void);

u8 GetDynamaxIndicatorSpriteId(u32 healthboxSpriteId);
u32 CreateDynamaxIndicatorSprite(u32 battlerId);
void DestroyDynamaxIndicatorSprite(u32 healthboxSpriteId);

// Raid Battle State Flags
#define INTRO_COMPLETED             (1 << 0)
#define SHOULD_CREATE_SHIELDS       (1 << 1)
#define SHOULD_BREAK_SHIELD         (1 << 2)
#define CATCHING_RAID_BOSS          (1 << 3)

// Raid Storm Levels
#define RAID_STORM_LEVEL_1     3    // number of turns
#define RAID_STORM_LEVEL_2     6
#define RAID_STORM_LEVEL_3     9
#define RAID_STORM_MAX         10

// Other Constants
#define MAX_RAID_DROPS      12
#define MAX_STAR_RATING     6
#define MAX_BARRIER_COUNT   5

#define DYNAMAX_TURNS		3

enum MaxMoveEffect
{
	MAX_EFFECT_NONE,
	MAX_EFFECT_RAISE_TEAM_ATTACK,
	MAX_EFFECT_RAISE_TEAM_DEFENSE,
	MAX_EFFECT_RAISE_TEAM_SPEED,
	MAX_EFFECT_RAISE_TEAM_SP_ATK,
	MAX_EFFECT_RAISE_TEAM_SP_DEF,
	MAX_EFFECT_LOWER_ATTACK,
	MAX_EFFECT_LOWER_DEFENSE,
	MAX_EFFECT_LOWER_SPEED,
	MAX_EFFECT_LOWER_SP_ATK,
	MAX_EFFECT_LOWER_SP_DEF,
	MAX_EFFECT_SUN,
	MAX_EFFECT_RAIN,
	MAX_EFFECT_SANDSTORM,
	MAX_EFFECT_HAIL,
	MAX_EFFECT_MISTY_TERRAIN,
	MAX_EFFECT_GRASSY_TERRAIN,
	MAX_EFFECT_ELECTRIC_TERRAIN,
	MAX_EFFECT_PSYCHIC_TERRAIN,
	MAX_EFFECT_VINE_LASH,
	MAX_EFFECT_WILDFIRE,
	MAX_EFFECT_CANNONADE,
	MAX_EFFECT_EFFECT_SPORE_FOES,
	MAX_EFFECT_PARALYZE_FOES,
	MAX_EFFECT_CONFUSE_FOES_PAY_DAY,
	MAX_EFFECT_CRIT_PLUS,
	MAX_EFFECT_MEAN_LOOK,
	MAX_EFFECT_AURORA_VEIL,
	MAX_EFFECT_INFATUATE_FOES,
	MAX_EFFECT_RECYCLE_BERRIES,
	MAX_EFFECT_POISON_FOES,
	MAX_EFFECT_STEALTH_ROCK,
	MAX_EFFECT_DEFOG,
	MAX_EFFECT_POISON_PARALYZE_FOES,
	MAX_EFFECT_HEAL_TEAM,
	MAX_EFFECT_SPITE,
	MAX_EFFECT_GRAVITY,
	MAX_EFFECT_VOLCAITH_FOES,
	MAX_EFFECT_SANDBLAST_FOES,
	MAX_EFFECT_YAWN_FOE,
	MAX_EFFECT_LOWER_EVASIVENESS_FOES,
	MAX_EFFECT_AROMATHERAPY,
	MAX_EFFECT_CONFUSE_FOES,
	MAX_EFFECT_STEELSURGE,
	MAX_EFFECT_TORMENT_FOES,
	MAX_EFFECT_LOWER_SPEED_2_FOES,
	MAX_EFFECT_FIRE_SPIN_FOES,
};

enum RaidStars
{
	NO_RAID,
	ONE_STAR_RAID,
	TWO_STAR_RAID,
	THREE_STAR_RAID,
	FOUR_STAR_RAID,
	FIVE_STAR_RAID,
	SIX_STAR_RAID,
	RAID_STAR_COUNT,
};

enum
{
	RAID_ABILITY_1,
	RAID_ABILITY_2,
    RAID_ABILITY_HIDDEN,
	RAID_ABILITY_RANDOM_1_2,
	RAID_ABILITY_RANDOM_ALL,
};

#define RAID_BATTLE_BGM     MUS_RG_VS_WILD

// Structs

struct Raid
{
    u16 species;
    u8 ability;
    u16 drops[MAX_RAID_DROPS];
};

struct RaidData
{
    const struct Raid *data;
    u8 amount;
};

extern u8 gRaidBattleStars;
extern u8 gRaidBattleLevel;
extern u16 gRaidBattleSpecies;
extern const struct MultiRaidTrainer gRaidPartners[];
extern const u8 gNumRaidPartners;
extern const struct RaidData gRaidsByMapSection[][RAID_STAR_COUNT];

#endif