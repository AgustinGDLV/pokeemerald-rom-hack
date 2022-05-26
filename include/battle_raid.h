#ifndef GUARD_BATTLE_RAID_H
#define GUARD_BATTLE_RAID_H

void InitRaidVariables(void);
u8 GetRaidBarrierNumber(u8 battlerId);
u8 GetRaidThresholdNumber(void);
u32 GetNextHealthThreshold();
bool8 ShouldCreateBarrier(u8 battlerId, s32 dmg);

#define SHOULD_CREATE_BARRIERS (1 << 0)
#define SHOULD_BREAK_BARRIER (1 << 1)

#endif