// Sample data
static const struct FixedMonSet sRaidPartnerSets_May_RankAll[] =
{
	{
		.species = SPECIES_SCEPTILE,
		.nature = NATURE_JOLLY,
		.hpIv = 31,
		.atkIv = 31,
		.defIv = 31,
		.spAtkIv = 31,
		.spDefIv = 31,
		.spdIv = 31,
		.atkEv = 252,
		.spdEv = 252,
		.ability = RAID_ABILITY_HIDDEN, //ABILITY_UNBURDEN
		.item = ITEM_GRASS_GEM,
		.moves =
		{
			MOVE_LEAF_BLADE,
			MOVE_SWORDS_DANCE,
			MOVE_BRICK_BREAK,
			MOVE_AERIAL_ACE,
		},
		.ball = ITEM_POKE_BALL,
	},
	{
		.species = SPECIES_BLAZIKEN,
		.nature = NATURE_JOLLY,
		.hpIv = 31,
		.atkIv = 31,
		.defIv = 31,
		.spAtkIv = 31,
		.spDefIv = 31,
		.spdIv = 31,
		.atkEv = 252,
		.spdEv = 252,
		.ability = RAID_ABILITY_HIDDEN, // ABILITY_SPEED_BOOST
		.item = ITEM_LIFE_ORB,
		.moves =
		{
			MOVE_BLAZE_KICK,
			MOVE_PROTECT,
			MOVE_CLOSE_COMBAT,
			MOVE_SWORDS_DANCE,
		},
		.ball = ITEM_POKE_BALL,
	},
	{
		.species = SPECIES_SWAMPERT,
		.nature = NATURE_JOLLY,
		.hpIv = 31,
		.atkIv = 31,
		.defIv = 31,
		.spAtkIv = 31,
		.spDefIv = 31,
		.spdIv = 31,
		.atkEv = 252,
		.spdEv = 252,
		.ability = RAID_ABILITY_1, // ABILITY_TORRENT
		.item = ITEM_LIFE_ORB,
		.moves =
		{
			MOVE_WATERFALL,
			MOVE_ICE_PUNCH,
			MOVE_EARTHQUAKE,
			MOVE_FLIP_TURN,
		},
		.ball = ITEM_POKE_BALL,
	},
};

static const u8 sText_TrainerNameMay[];

const struct MultiRaidTrainer gRaidPartners[] =
{
	{ // sample data
        .owNum = OBJ_EVENT_GFX_MAY_NORMAL,
        .trainerClass = TRAINER_CLASS_RIVAL,
		.backSpriteId = TRAINER_BACK_PIC_MAY,
		.gender = FEMALE,
		.otId = 0xCD2F27AB,
		.sets =
		{
			[ONE_STAR_RAID ... SIX_STAR_RAID] = sRaidPartnerSets_May_RankAll,
		},
		.setSizes =
		{
			[ONE_STAR_RAID ... SIX_STAR_RAID] = ARRAY_COUNT(sRaidPartnerSets_May_RankAll),
		},
	},
};

const u8 gNumRaidPartners = ARRAY_COUNT(gRaidPartners);