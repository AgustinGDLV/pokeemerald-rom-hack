#ifndef GUARD_BATTLE_RAID_H
#define GUARD_BATTLE_RAID_H

#include "constants/songs.h"

void InitRaidVariables(void);
u16 GetRaidHPMultiplier(void);
void ApplyRaidHPMultiplier(struct Pokemon *mon);
u8 GetRaidBarrierNumber(void);
u8 GetRaidThresholdNumber(void);
u32 GetNextHealthThreshold(void);
bool8 ShouldCreateBarrier(s32 dmg);

void CB2_ChooseBall(void);

u32 CreateRaidBarrierSprite(u8 index);
void CreateAllRaidBarrierSprites(void);
void DestroyRaidBarrierSprite(u8 index);
void DestroyAllRaidBarrierSprites(void);

#define SHOULD_CREATE_BARRIERS (1 << 0)
#define SHOULD_BREAK_BARRIER (1 << 1)

#define MAX_RAID_DROPS      12
#define MAX_STAR_RATING     6
#define MAX_BARRIER_COUNT   5

#define RAID_BATTLE_BGM     MUS_RG_VS_WILD

#endif