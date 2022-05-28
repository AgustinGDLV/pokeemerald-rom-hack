#ifndef GUARD_BATTLE_RAID_H
#define GUARD_BATTLE_RAID_H

void InitRaidVariables(void);
u8 GetRaidBarrierNumber(void);
u8 GetRaidThresholdNumber(void);
u32 GetNextHealthThreshold(void);
bool8 ShouldCreateBarrier(s32 dmg);

u32 CreateRaidBarrierSprite(u8 index);
void CreateAllRaidBarrierSprites(void);
void DestroyRaidBarrierSprite(u8 index);
void DestroyAllRaidBarrierSprites(void);

#define SHOULD_CREATE_BARRIERS (1 << 0)
#define SHOULD_BREAK_BARRIER (1 << 1)

#endif