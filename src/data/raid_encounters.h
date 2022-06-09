#define HOENN_MAPSEC_COUNT      MAPSEC_DYNAMIC

// Sample raid data
static const struct Raid sOldaleTownRaids3Star[] = 
{
    {
        .species = SPECIES_PORYGON,
        .ability = RAID_ABILITY_HIDDEN,
        .drops =
		{
			/*100 %*/ ITEM_FIGY_BERRY,
			/* 80 %*/ ITEM_CHOPLE_BERRY,
			/* 80 %*/ ITEM_TAMATO_BERRY,
			/* 50 %*/ ITEM_WIKI_BERRY,
			/* 50 %*/ ITEM_NONE,
			/* 30 %*/ ITEM_BIG_NUGGET,
			/* 30 %*/ ITEM_NUGGET,
			/* 25 %*/ ITEM_RARE_CANDY,
			/* 25 %*/ ITEM_NORMAL_GEM,
			/*  5 %*/ ITEM_FOCUS_SASH,
			/*  4 %*/ ITEM_NONE,
			/*  1 %*/ ITEM_BOTTLE_CAP,
		},
    },
    {
        .species = SPECIES_KINGLER,
        .ability = RAID_ABILITY_RANDOM_1_2,
        .drops =
		{
			/*100 %*/ ITEM_PEARL,
			/* 80 %*/ ITEM_WACAN_BERRY,
			/* 80 %*/ ITEM_QUALOT_BERRY,
			/* 50 %*/ ITEM_NONE,
			/* 50 %*/ ITEM_NONE,
			/* 30 %*/ ITEM_BIG_PEARL,
			/* 30 %*/ ITEM_PEARL,
			/* 25 %*/ ITEM_RARE_CANDY,
			/* 25 %*/ ITEM_WATER_GEM,
			/*  5 %*/ ITEM_MYSTIC_WATER,
			/*  4 %*/ ITEM_NONE,
			/*  1 %*/ ITEM_BOTTLE_CAP,
		},
    }
};

const struct RaidData gRaidsByMapSection[HOENN_MAPSEC_COUNT][RAID_STAR_COUNT] =
{
    [MAPSEC_OLDALE_TOWN] =
    {
		[ONE_STAR_RAID ... SIX_STAR_RAID] = {sOldaleTownRaids3Star, ARRAY_COUNT(sOldaleTownRaids3Star)},
	},
};