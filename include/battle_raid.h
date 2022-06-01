#ifndef GUARD_BATTLE_RAID_H
#define GUARD_BATTLE_RAID_H

#include "constants/songs.h"

void InitRaidVariables(void);
u16 GetRaidHPMultiplier(void);
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

// Raid Battle State
#define INTRO_COMPLETED             (1 << 0)
#define SHOULD_CREATE_SHIELDS       (1 << 1)
#define SHOULD_BREAK_SHIELD         (1 << 2)
#define RAID_BOSS_DEFEATED          (1 << 3)
#define CATCHING_RAID_BOSS          (1 << 4)

#define MAX_RAID_DROPS      12
#define MAX_STAR_RATING     6
#define MAX_BARRIER_COUNT   5

#define RAID_BATTLE_BGM     MUS_RG_VS_WILD

#endif