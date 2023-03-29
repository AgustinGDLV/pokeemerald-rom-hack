#include "global.h"
#include "bg.h"
#include "sprite.h"
#include "main.h"
#include "battle.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item_menu.h"
#include "sound.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "random.h"
#include "palette.h"
#include "pokemon_icon.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "string_util.h"
#include "trainer_pokemon_sprites.h"
#include "util.h"
#include "window.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define MAX_NUM_PARTNERS 3
#define MAX_TEAM_SIZE 3

enum Windows
{
	WIN_RECOMMENDED_LEVEL,
	WIN_CHOOSE_PARTNER,
	WIN_RULES,
	WIN_TYPE_1,
	WIN_TYPE_2,
	WIN_INSTRUCTIONS,
	WINDOW_COUNT,
};

enum
{
	TAG_RAID_BATTLE_STAR = 0x2710,
	TAG_RAID_BATTLE_CURSOR,
};

struct Partner
{
	u16 id;
	u16 graphicsId;
	u16 team[MAX_TEAM_SIZE];
};

struct RaidBattleIntro
{
	u32* tilemapPtr;
	struct Partner partners[MAX_NUM_PARTNERS];
	u32 personality;
	u16 species;
	u8 rank;
	u8 selectedTeam;
	u16 monSpriteId;
	u8 outlinedSprite;
};

// const rom data
static const u8 sRaidStarGfx[]          = INCBIN_U8("graphics/misc/raid_battle_star.4bpp");
static const u16 sRaidStarPal[]         = INCBIN_U16("graphics/misc/raid_battle_star.gbapal");
static const u8 sRaidCursorGfx[]        = INCBIN_U8("graphics/misc/raid_battle_cursor.4bpp");
static const u16 sRaidCursorPal[]       = INCBIN_U16("graphics/misc/raid_battle_cursor.gbapal");
static const u32 sRaidIntroBgGfx[]      = INCBIN_U32("graphics/misc/raid_battle_intro_bg.4bpp.lz");
static const u32 sRaidIntroBgPal[]      = INCBIN_U32("graphics/misc/raid_battle_intro_bg.gbapal.lz");
static const u32 sRaidIntroBgMap[]      = INCBIN_U32("graphics/misc/raid_battle_intro_bg.bin.lz");

static const u8 sText_RecommendedLevel[]        = _("Recommended Level: ");
static const u8 sText_RaidIntroSelection[]      = _("{DPAD_UPDOWN}Pick {A_BUTTON}Choose {START_BUTTON}Random {B_BUTTON}Cancel");
static const u8 sText_RaidBattleRules[]         = _("Battle ends if:\n 4 Pokemon faint\n 10 turns pass");
static const u8 sText_RaidBattleChoosePartner[] = _("Available Partners");

static const struct WindowTemplate sRaidBattleIntroWinTemplates[WINDOW_COUNT + 1] =
{
	[WIN_RECOMMENDED_LEVEL] =
	{
		.bg = 1,
		.tilemapLeft = 16,
		.tilemapTop = 0,
		.width = 14,
		.height = 3,
		.paletteNum = 15,
		.baseBlock = 1,
	},
	[WIN_CHOOSE_PARTNER] =
	{
		.bg = 1,
		.tilemapLeft = 16,
		.tilemapTop = 3,
		.width = 14,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 43,
	},
	[WIN_RULES] =
	{
		.bg = 1,
		.tilemapLeft = 1,
		.tilemapTop = 13,
		.width = 10,
		.height = 6,
		.paletteNum = 15,
		.baseBlock = 71,
	},
	[WIN_TYPE_1] =
	{
		.bg = 1,
		.tilemapLeft = 8,
		.tilemapTop = 0,
		.width = 4,
		.height = 2,
		.paletteNum = 12,
		.baseBlock = 131,
	},
	[WIN_TYPE_2] =
	{
		.bg = 1,
		.tilemapLeft = 12,
		.tilemapTop = 0,
		.width = 4,
		.height = 2,
		.paletteNum = 12,
		.baseBlock = 149,
	},
	[WIN_INSTRUCTIONS] =
	{
		.bg = 1,
		.tilemapLeft = 8,
		.tilemapTop = 18,
		.width = 22,
		.height = 2,
		.paletteNum = 15,
		.baseBlock = 167,
	},
	DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sRaidBattleIntroBgTemplates[] =
{
	{ //Intro Background
		.bg = 2,
		.charBaseIndex = 0,
		.mapBaseIndex = 31,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 2,
		.baseTile = 0,
	},
	{ //Text
		.bg = 1,
		.charBaseIndex = 2,
		.mapBaseIndex = 6,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 1,
		.baseTile = 0,
	},
	{ //Instructions
		.bg = 0,
		.charBaseIndex = 1,
		.mapBaseIndex = 24,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 0,
		.baseTile = 0,
	}
};

static const struct OamData sStarOAM =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(8x8),
	.size = SPRITE_SIZE(8x8),
	.priority = 2, //On BG 2
};

static const struct OamData sOamData_RedArrowCursor =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 0, //Above everything
};

static const struct SpriteTemplate sRaidBattleStarSpriteTemplate =
{
	.tileTag = TAG_RAID_BATTLE_STAR,
	.paletteTag = TAG_RAID_BATTLE_STAR,
	.oam = &sStarOAM,
	.anims = gDummySpriteAnimTable,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCallbackDummy,
};

static void SpriteCB_RaidCursor(struct Sprite* sprite);
static const struct SpriteTemplate sRaidBattleCursorSpriteTemplate =
{
	.tileTag = TAG_RAID_BATTLE_CURSOR,
	.paletteTag = TAG_RAID_BATTLE_CURSOR,
	.oam = &sOamData_RedArrowCursor,
	.anims = gDummySpriteAnimTable,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCB_RaidCursor,
};

static const struct SpriteSheet sRaidBattleStarSpriteSheet = {
    sRaidStarGfx, sizeof(sRaidStarGfx), TAG_RAID_BATTLE_STAR
};
static const struct SpritePalette sRaidBattleStarSpritePalette = {
    sRaidStarPal, TAG_RAID_BATTLE_STAR
};
static const struct SpriteSheet sRaidBattleCursorSpriteSheet = {
    sRaidCursorGfx, sizeof(sRaidCursorGfx), TAG_RAID_BATTLE_CURSOR
};
static const struct SpritePalette sRaidBattleCursorSpritePalette = {
    sRaidCursorPal, TAG_RAID_BATTLE_CURSOR
};

// functions
static void PrintInstructions(void);
static void CleanWindows(void);
static void CommitWindows(void);
static void LoadRaidBattleIntroGfx(void);
static void ClearTasksAndGraphicalStructs(void);
static void ClearVramOamPlttRegs(void);
static void Task_RaidBattleIntroFadeOut(u8 taskId);
static void Task_RaidBattleIntroWaitForKeyPress(u8 taskId);
static void Task_RaidBattleIntroFadeIn(u8 taskId);
static void InitRaidBattleIntro(void);
static void ShowStars(void);
static void ShowRaidPokemonSprite(void);
static void ShowRaidPokemonTypes(void);
static void ShowPartnerTeams(void);
static void ShowRaidCursor(void);
static void OutlineMonSprite(u8 spriteId);
static bool8 GetRaidBattleData(void);

EWRAM_DATA static struct RaidBattleIntro *sRaidBattleIntro = NULL;

// code
static void MainCB2_RaidBattleIntro(void)
{
	RunTasks();
	AnimateSprites();
	BuildOamBuffer();
	UpdatePaletteFade();
}

static void VBlankCB_RaidBattleIntro(void)
{
	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();
}

void CB2_RaidBattleIntro(void)
{
    switch (gMain.state) {
        default:
        case 0:
            SetVBlankCallback(NULL); 
            ClearVramOamPlttRegs();
            gMain.state++;
            break;
        case 1:
            ClearTasksAndGraphicalStructs();
            gMain.state++;
            break;
        case 2:
            sRaidBattleIntro->tilemapPtr = AllocZeroed(BG_SCREEN_SIZE);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sRaidBattleIntroBgTemplates, 3);
            SetBgTilemapBuffer(2, sRaidBattleIntro->tilemapPtr);
            gMain.state++;
            break;
        case 3:
            LoadRaidBattleIntroGfx();
            gMain.state++;
            break;
        case 4:
            if (IsDma3ManagerBusyWithBgCopy() != TRUE)
            {
                ShowBg(0);
                ShowBg(1);
                ShowBg(2);
                CopyBgTilemapBufferToVram(2);
                gMain.state++;
            }
            break;
        case 5:
            InitWindows(sRaidBattleIntroWinTemplates);
            DeactivateAllTextPrinters();
            gMain.state++;
            break;
        case 6:
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
            gMain.state++;
            break;
        case 7:
            SetVBlankCallback(VBlankCB_RaidBattleIntro);
            InitRaidBattleIntro();
            CreateTask(Task_RaidBattleIntroFadeIn, 0);
            SetMainCallback2(MainCB2_RaidBattleIntro);
            break;
	}
}

static void Task_RaidBattleIntroFadeOut(u8 taskId)
{
	if (!gPaletteFade.active)
	{
        SetMainCallback2(CB2_ReturnToFieldContinueScript);
		Free(sRaidBattleIntro->tilemapPtr);
		Free(sRaidBattleIntro);
		FreeAllWindowBuffers();
		DestroyTask(taskId);
	}
}

static void Task_RaidBattleIntroWaitForKeyPress(u8 taskId)
{
	u8 i;
	u16 id;
	const u8* name;

	if (gMain.newKeys & A_BUTTON)
	{
		PRESSED_A:
		/*id = sRaidBattleIntro->partners[sRaidBattleIntro->selectedTeam].id;

		gSpecialVar_Result = 1;
		//FlagSet(FLAG_TAG_BATTLE);
		//VarSet(VAR_PARTNER, RAID_BATTLE_MULTI_TRAINER_TID);
		//VarSet(VAR_FACILITY_TRAINER_ID_PARTNER, id);
		//VarSet(VAR_PARTNER_BACKSPRITE, gRaidPartners[id].backSpriteId);

		//name = TryGetRivalNameByTrainerClass(gRaidPartners[id].trainerClass);
		//if (name == NULL)
        //  name = gRaidPartners[id].name;
		GetMonData(&gPlayerParty[0], MON_DATA_NICKNAME, gStringVar1); //placeholder
        //StringCopy(gStringVar1, name);
		//StringCopy(gStringVar7, name);
		GetSpeciesName(gStringVar2, SPECIES_SALAMENCE);
		//GetSpeciesName(gStringVar8, gRaidBattleSpecies);*/

		PlaySE(SE_SUCCESS);
		//BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		//gTasks[taskId].func = Task_RaidBattleIntroFadeOut;
	}
	else if (gMain.newKeys & B_BUTTON)
	{
		gSpecialVar_Result = FALSE;

		PlaySE(SE_FAILURE);
		BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		gTasks[taskId].func = Task_RaidBattleIntroFadeOut;
	}
	else if (gMain.newAndRepeatedKeys & START_BUTTON)
	{
		/*u8 numTeams = 0;
		PlaySE(SE_SELECT);
		for (i = 0; i < MAX_TEAM_SIZE; ++i)
		{
			if (sRaidBattleIntro->partners[i].graphicsId != 0)
				++numTeams;
		}

		sRaidBattleIntro->selectedTeam = Random() % numTeams;
		goto PRESSED_A;*/
	}
	else if (gMain.newAndRepeatedKeys & DPAD_UP)
	{
		PlaySE(SE_SELECT);
		if (sRaidBattleIntro->selectedTeam == 0)
		{
			for (i = 0; i < MAX_TEAM_SIZE; ++i)
			{
				if (sRaidBattleIntro->partners[i].graphicsId != 0)
					sRaidBattleIntro->selectedTeam++;
				else
					break;
			}

			sRaidBattleIntro->selectedTeam -= 1; //Prevent overflow
		}
		else
			sRaidBattleIntro->selectedTeam--;
	}
	else if (gMain.newAndRepeatedKeys & DPAD_DOWN)
	{
		PlaySE(SE_SELECT);
		sRaidBattleIntro->selectedTeam++;

		if (sRaidBattleIntro->selectedTeam >= MAX_TEAM_SIZE
		    || sRaidBattleIntro->partners[sRaidBattleIntro->selectedTeam].graphicsId == 0)
			sRaidBattleIntro->selectedTeam = 0;
	}
}

// Makes the sprite move back and forth horizontally.
static void SpriteCB_RaidCursor(struct Sprite* sprite)
{
    sprite->y2 = sRaidBattleIntro->selectedTeam * 33;

	if (sprite->data[1])
	{
		sprite->data[0] -= 1;
		if (sprite->data[0] == 0)
			sprite->data[1] = FALSE;
	}
	else
	{
		sprite->data[0] += 1;
		if (sprite->data[0] == 20)
			sprite->data[1] = TRUE;
	}

	sprite->x2 = 5 - sprite->data[0] / 4;
}

static void Task_RaidBattleIntroFadeIn(u8 taskId)
{
	//if (sRaidBattleIntro->outlinedSprite < 2)
	//	OutlineMonSprite(sRaidBattleIntro->monSpriteId);

	if (!gPaletteFade.active)
	{
        PlaySE(SE_RG_CARD_OPEN);
		gTasks[taskId].func = Task_RaidBattleIntroWaitForKeyPress;
	}
}

static void PrintInstructions(void)
{
	const u8 partnerColour[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
	/*{
		.bgColor = 0, //Transparent
		.fgColor = 2, //Dark Gray
		.shadowColor = 3, //Gray
	};*/

	const u8 colour[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
	/*{
		.bgColor = 0, //Transparent
		.fgColor = 1, //White
		.shadowColor = 2,
	};*/

	AddTextPrinterParameterized3(WIN_RULES, 0, 0, 2, colour, 0, sText_RaidBattleRules);

	StringCopy(gStringVar1, sText_RecommendedLevel);
	ConvertIntToDecimalStringN(gStringVar2, GetRaidRecommendedLevel(), 0, 3);
	StringAppend(gStringVar1, gStringVar2);
	AddTextPrinterParameterized3(WIN_RECOMMENDED_LEVEL, 0, 4, 0, colour, 0, gStringVar1);

	AddTextPrinterParameterized3(WIN_INSTRUCTIONS, 0, 2, 4, colour, 0, sText_RaidIntroSelection);

	AddTextPrinterParameterized3(WIN_CHOOSE_PARTNER, 3, 1, 4, partnerColour, 0, sText_RaidBattleChoosePartner);
}

static void ShowStars(void)
{
    u8 i;
	LoadSpritePalette(&sRaidBattleStarSpritePalette);
    LoadSpriteSheet(&sRaidBattleStarSpriteSheet);

	for (i = 0; i < sRaidBattleIntro->rank; i++)
		CreateSprite(&sRaidBattleStarSpriteTemplate, 10 + (9 * i), 8, 0);
}

static void ShowRaidPokemonSprite(void)
{
    u8 i, j;
    u16 species = sRaidBattleIntro->species;
	u32 personality = sRaidBattleIntro->personality;
	u32 otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
    u16 paletteOffset;
    u16 spriteId;
	const struct CompressedSpritePalette *pal = GetMonSpritePalStructFromOtIdPersonality(species, otId, personality);

	//Create black silhouette
	sRaidBattleIntro->monSpriteId = CreateMonPicSprite(species, otId, personality, TRUE, 45, 57, 0, pal->tag);
    gSprites[sRaidBattleIntro->monSpriteId].oam.priority = 0;

	paletteOffset = IndexOfSpritePaletteTag(pal->tag) * 16 + 0x100;
    BlendPalette(paletteOffset, 16, 16, RGB(4, 4, 4));
    CpuCopy32(gPlttBufferFaded + paletteOffset, gPlttBufferUnfaded + paletteOffset, 32);

	//Create white outline
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            spriteId = CreateMonPicSprite(species, otId, personality, TRUE, 44 + i*2, 56 + j*2, gSprites[sRaidBattleIntro->monSpriteId].oam.paletteNum + 1, TAG_NONE);
            gSprites[spriteId].oam.priority = 1;
        }
    }
    paletteOffset += 16;
    FillPalette(RGB_WHITE, paletteOffset, 32);
    CpuCopy32(gPlttBufferFaded + paletteOffset, gPlttBufferUnfaded + paletteOffset, 32);
}

static void ShowRaidPokemonTypes(void)
{
	u16 species = sRaidBattleIntro->species;
	u8 type1 = gBaseStats[species].type1;
	u8 type2 = gBaseStats[species].type2;

	BlitMenuInfoIcon(WIN_TYPE_1, type1 + 1, 0, 2);
	if (type1 != type2)
		BlitMenuInfoIcon(WIN_TYPE_2, type2 + 1, 0, 2);
}

static void ShowPartnerTeams(void)
{
	u8 i, j;

	for (i = 0; i < MAX_NUM_PARTNERS; ++i)
	{
		if (sRaidBattleIntro->partners[i].graphicsId != 0)
		{
            u32 spriteId;
            
			spriteId = CreateObjectGraphicsSprite(sRaidBattleIntro->partners[i].graphicsId, SpriteCallbackDummy, 126, 59 + (i * 33), 0);
            gSprites[spriteId].oam.priority = 0;

			for (j = 0; j < MAX_TEAM_SIZE; ++j)
			{
				u16 species = sRaidBattleIntro->partners[i].team[j];
				if (species != SPECIES_NONE)
				{
					LoadMonIconPalette(species);
					CreateMonIcon(species, SpriteCB_MonIcon, 158 + (32 * j), 59 + (i * 33), 0, 0xFFFFFFFF);
				}
			}
		}
	}
}

static void ShowRaidCursor(void)
{
	LoadSpriteSheet(&sRaidBattleCursorSpriteSheet);
	LoadSpritePalette(&sRaidBattleCursorSpritePalette);
	CreateSprite(&sRaidBattleCursorSpriteTemplate, 95, 59, 0);
}

//Cleans the windows
static void CleanWindows(void)
{
    u8 i;
	for (i = 0; i < WINDOW_COUNT; i++)
		FillWindowPixelBuffer(i, PIXEL_FILL(0));
}
//Display commited windows
static void CommitWindows(void)
{
    u8 i;
	for (i = 0; i < WINDOW_COUNT; i++)
	{
		CopyWindowToVram(i, 3);
		PutWindowTilemap(i);
	}
}

static void InitRaidBattleIntro(void)
{
	CleanWindows();
	CommitWindows();

    ShowRaidCursor();
	ShowStars();
    PrintInstructions();
	ShowRaidPokemonTypes();
	ShowPartnerTeams();
	ShowRaidPokemonSprite();
    
	//Display newly commited windows
	CommitWindows();
}

static void LoadRaidBattleIntroGfx(void)
{	
    DecompressAndCopyTileDataToVram(2, &sRaidIntroBgGfx, 0, 0, 0);
	LZDecompressWram(sRaidIntroBgMap, sRaidBattleIntro->tilemapPtr);
	LoadCompressedPalette(sRaidIntroBgPal, 0, 0x20);
	ListMenuLoadStdPalAt(0xC0, 1);
	Menu_LoadStdPalAt(0xF0);
}

static void ClearTasksAndGraphicalStructs(void)
{
	ScanlineEffect_Stop();
	ResetTasks();
	ResetSpriteData();
	ResetTempTileDataBuffers();
	ResetPaletteFade();
	FreeAllSpritePalettes();
}

static void ClearVramOamPlttRegs(void)
{
	DmaFill16(3, 0, VRAM, VRAM_SIZE);
	DmaFill32(3, 0, OAM, OAM_SIZE);
	DmaFill16(3, 0, PLTT, PLTT_SIZE);
	SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
	SetGpuReg(REG_OFFSET_BG3CNT, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG2CNT, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG1CNT, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG0CNT, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG3HOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG3VOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG2HOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG2VOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG1HOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG1VOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG0HOFS, DISPCNT_MODE_0);
	SetGpuReg(REG_OFFSET_BG0VOFS, DISPCNT_MODE_0);
}

static bool8 HasRaidBattleAlreadyBeenDone(void)
{
    return FALSE;
}

void IsRaidBattleAvailable(void)
{
	if (!HasRaidBattleAlreadyBeenDone() && GetRaidBattleData())
		gSpecialVar_Result = TRUE;
	else
		gSpecialVar_Result = FALSE;
}

void InitRaidIntro(void)
{
    gSpecialVar_Result = FALSE;
    if (sRaidBattleIntro == NULL)
	    sRaidBattleIntro = AllocZeroed(sizeof(struct RaidBattleIntro));
    
	if (GetRaidBattleData())
	{
		PlayRainStoppingSoundEffect();
		SetMainCallback2(CB2_RaidBattleIntro);
	}
}

static void OutlineMonSprite(u8 spriteId)
{
	u32 i = 0;
	u8 buffer[(64 * 64) / 2] = {0};
	u8* offset = (void*)(OBJ_VRAM0) + (gSprites[spriteId].oam.tileNum * 32);
	u8* originalOffset = offset;
    u8 nextByteColumn, nextByteRow, previousByteRow;
    u8 pixel1, pixel2, pixel3, pixel4, pixel5, pixel6, pixel7;
    bool8 isTopBorder, isBottomBorder, isLeftBorder, isRightBorder;

	for (i = 0; i < (64 * 64) / 2; i++)
	{
		/*Pixel Map - Bits
		6	7

		1	2	3

		4	5
		*/

		/*Pixel map - Bytes
		67
		12 3
		45
		*/

		nextByteColumn = 1;
		if (i % 4 == 3)
			nextByteColumn = 0x1D;

		nextByteRow = 4;
		if (i % 0x20 >= 0x1C)
			nextByteRow = 0xE4;

		previousByteRow = 4;
		if (i % 0x20 < 4)
			previousByteRow = 0xE4;

		//Next column
		pixel1 = offset[i] & 0xF;
		pixel2 = (offset[i] >> 4) & 0xF;

		pixel3 = offset[i + nextByteColumn] & 0xF;

		//Next row
		pixel4 = offset[i + nextByteRow] & 0xF;
		pixel5 = (offset[i + nextByteRow] >> 4) & 0xF;

		//Row Above
		pixel6 = offset[i - previousByteRow] & 0xF;
		pixel7 = (offset[i - previousByteRow] >> 4) & 0xF;

		isTopBorder = (i < 0xE4) && ((i % 0x20) < 4);
		isBottomBorder = (i >= 0x71C) && ((i % 0x20) >= 0x1C);
		isLeftBorder = ((i % 0x100) < 0x20) && ((i % 4) == 0);
		isRightBorder = ((i % 0x100) >= 0xE0) && ((i % 4) == 3);

		/*if (pixel1 != 0)
		{
			if (isTopBorder || isBottomBorder || isLeftBorder)
				buffer[i + 0] |= 0xF; //Set lower bit to white
			else
				buffer[i + 0] |= 0x1; //Set lower bit to black

			if (pixel2 == 0)
				buffer[i + 0] |= 0xF0; //Set upper bit

			if (pixel4 == 0)
				buffer[i + nextByteRow] |= 0xF; //Set lower bit

			if (i >= previousByteRow && pixel6 == 0)
				buffer[i - previousByteRow] |= 0xF; //Set lower bit/
		}

		if (pixel2 != 0)
		{
			if (isTopBorder || isBottomBorder || isRightBorder) //Right border
				buffer[i + 0] |= 0xF0; //Set upper bit to white
			else
				buffer[i + 0] |= 0x10; //Set upper bit to black

			if (pixel1 == 0)
				buffer[i + 0] |= 0xF; //Set lower bit

			if (pixel3 == 0 && (i % 0x100 < 0xE0 || i % 4 != 3))
				buffer[i + nextByteColumn] |= 0xF; //Set lower bit

			if (pixel5 == 0)
				buffer[i + nextByteRow] |= 0xF0; //Set upper bit

			if (i >= previousByteRow && pixel7 == 0)
				buffer[i - previousByteRow] |= 0xF0; //Set upper bit
		}

		if (pixel3 != 0 && !isRightBorder)
		{
			if (pixel2 == 0)
				buffer[i + 0] |= 0xF0; //Set upper bit
		}*/
	}

	//CpuCopy32(buffer, originalOffset, (64 * 64) / 2);
	sRaidBattleIntro->outlinedSprite++;
}

static bool8 GetRaidBattleData(void)
{
	u32 i, j, k;
	bool8 checkedPartners[gNumRaidPartners]; //numRaidPartners

	DetermineRaidStars();
	DetermineRaidSpecies();
	DetermineRaidLevel();
	sRaidBattleIntro->rank = gRaidBattleStars;
	sRaidBattleIntro->species = gRaidBattleSpecies;

	if (gRaidBattleSpecies == SPECIES_NONE)
		return FALSE;

	for (i = 0; i < gNumRaidPartners; i++)
		checkedPartners[i] = FALSE;

	DetermineRaidPartners(checkedPartners, MAX_NUM_PARTNERS);

	k = 0;
	for (i = 0; i < gNumRaidPartners; i++)
	{
		if (checkedPartners[i] == TRUE) //0xFF means not viable
		{
			struct Partner* partner = &sRaidBattleIntro->partners[k++];

			partner->id = i;
			partner->graphicsId = gRaidPartners[i].owNum;

			for (j = 0; j < MAX_TEAM_SIZE; ++j)
			{
				const struct FixedMonSet *spread = GetRaidMultiSpread(i, j, sRaidBattleIntro->rank);
				if (spread != NULL)
					partner->team[j] = spread->species;
				else
					break;
			}
		}

		if (k >= MAX_NUM_PARTNERS)
			break;
	}

	if (k == 0) //No partners found
		return FALSE;

	return TRUE;

    /*Test Data:
    //gRaidBattleStars = 6;
	//sRaidBattleIntro->species = SPECIES_SALAMENCE;
	//sRaidBattleIntro->rank = 6;
    sRaidBattleIntro->personality = 0xFFFFFFFF;

	sRaidBattleIntro->partners[0].graphicsId = OBJ_EVENT_GFX_STEVEN;
	sRaidBattleIntro->partners[0].team[0] = SPECIES_TYRANITAR;
	sRaidBattleIntro->partners[0].team[1] = SPECIES_MAMOSWINE;
	sRaidBattleIntro->partners[0].team[2] = SPECIES_GRANBULL;

	sRaidBattleIntro->partners[1].graphicsId = OBJ_EVENT_GFX_MAY_NORMAL;
	sRaidBattleIntro->partners[1].team[0] = SPECIES_GOLURK;
	sRaidBattleIntro->partners[1].team[1] = SPECIES_MAGNEZONE;
	sRaidBattleIntro->partners[1].team[2] = SPECIES_SALAMENCE;

	sRaidBattleIntro->partners[2].graphicsId = OBJ_EVENT_GFX_RED;
	sRaidBattleIntro->partners[2].team[0] = SPECIES_PIKACHU_ORIGINAL_CAP;
	sRaidBattleIntro->partners[2].team[1] = SPECIES_SNORLAX;
	sRaidBattleIntro->partners[2].team[2] = SPECIES_MEWTWO;

	return TRUE;*/
}