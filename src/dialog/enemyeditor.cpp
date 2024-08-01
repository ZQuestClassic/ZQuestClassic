#include "enemyeditor.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include "tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "items.h"
#include "base/qrs.h"
#include <fmt/format.h>
#include "zinfo.h"
#include "defdata.h"

extern bool saved;
extern zcmodule moduledata;
extern guydata* guysbuf;
extern itemdata* itemsbuf;
extern int32_t CSet;
extern char* guy_string[eMAXGUYS];
extern const char* old_guy_string[OLDMAXGUYS];
extern script_data* guyscripts[NUMSCRIPTGUYS];
extern script_data* ewpnscripts[NUMSCRIPTWEAPONS];
char* ordinal(int32_t num);
using std::string;
using std::to_string;

static size_t guy_tabs[8] = { 0 };
static bool guy_use_script_data = true;

static bool edited = false;

bool call_enemy_editor(int32_t index)
{
	guy_use_script_data = zc_get_config("zquest", "show_enemyscript_meta_attribs", 1) ? true : false;
	edited = false;
	EnemyEditorDialog(index).show();
	return edited;
}

EnemyEditorDialog::EnemyEditorDialog(guydata const& ref, int32_t index) :
	index(index), local_guyref(ref), enemy_name(guy_string[index]),
	list_enemies(GUI::ZCListData::enemies(true)),
	list_families(GUI::ZCListData::efamilies(true)),
	list_animations(GUI::ZCListData::eanimations(true)),
	list_guyscripts(GUI::ZCListData::npc_script()),
	list_ewpnscripts(GUI::ZCListData::eweapon_script()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_counters_nn(GUI::ZCListData::counters(true, true)),
	list_dropsets(GUI::ZCListData::dropsets()),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_eweaptype(GUI::ZCListData::eweaptypes()),
	list_deftypes(GUI::ZCListData::deftypes())
{}

EnemyEditorDialog::EnemyEditorDialog(int32_t index) :
	EnemyEditorDialog(guysbuf[index], index) 
{}

static const GUI::ListData list_walkmisc1
{
	{"1 Shot",0},
	{"1 (End-Halt)",1},
	{"Rapid-Fire",2},
	{"1 (Fast)",3},
	{"1 (Slanted)",4},
	{"3 Shots",5},
	{"4 Shots",6},
	{"5 Shots",7},
	{"3 (Fast)",8},
	{"Breath",9},
	{"8 Shots",10},
	{"Summon",11},
	{"Summon (Layer)",12},
};

static const GUI::ListData list_walkmisc2
{
	{"Normal",0},
	{"Split On Hit",1},
	{"Split On Death",2},
	{"8 Shots",3},
	{"Explode",4},
	{"Tribble",5},
};

static const GUI::ListData list_walkmisc7
{
	{"None",0},
	{"Temp. Jinx",1},
	{"Perm. Jinx",2},
	{"Cure Jinx",3},
	{"Lose Magic",4},
	{"Lose Rupees",5},
	{"Drunk",6},
	{"Eat (Items)",7},
	{"Eat (Magic)",8},
	{"Eat (Rupees)",9},
	{"Eat (Damage)",10},
};

static const GUI::ListData list_walkmisc9
{
	{"Normal",0},
	{"Rope",1},
	{"Vire",2},
	{"Pols Voice",3},
	{"Armos",4},
};

static const GUI::ListData list_gleeokmisc3
{
	{"1 Shot",0},
	{ "Breath",1}
};

static const GUI::ListData list_gohmamisc1
{
	{"1 Shot",0},
	{"3 Shots",0},
	{"Breath",2}
};

static const GUI::ListData list_manhandlamisc2
{
	{"1x1 (4 Heads)",0},
	{"2x2 (8 Heads)",1},
};

static const GUI::ListData list_aquamisc1
{
	{"Right (Facing Left)", 0},
	{"Left (Facing Right)", 1},
};

static const GUI::ListData list_patramisc4
{
	{"Big Circle",0},
	{"Oval",1},
};

static const GUI::ListData list_patramisc5
{
	{"None",0},
	{"Center Eye",1},
	{"Inner Eyes",2},
	{"Inner + Center",3},
};

static const GUI::ListData list_patramisc10
{
	{"1x1 Patra", 0},
	{"2x2 Patra", 1},
};

static const GUI::ListData list_patramisc20
{
	{"Random (Any)",0},
	{"Random (Single)",1},
	{"Barrage",2},
	{"Ring",3},
	{"Stream",4},
};

static const GUI::ListData list_patramisc22
{
	{"Never",0},
	{"When Expanding",1},
	{"Expand + Warning",2},
};

static const GUI::ListData list_patramisc25
{
	{"When Behind 1 Layer",0},
	{"When Vulnerable",1},
	{"When Invulnerable",2},
	{"When Behind 1 Layer",3},
};

static const GUI::ListData list_patramisc26
{
	{"Always",0},
	{"With Outer Ring",1},
	{"Without Outer Ring",2},
};

static const GUI::ListData list_patramisc28
{
	{"1 Shot",0},
	{"1 (Fast)",1},
	{"3 Shots",2},
	{"3 (Fast)",3},
	{"5 Shots",4},
	{"5 (Fast)",5},
	{"4 (Cardinal)",6},
	{"4 (Diagonal)",7},
	{"4 (Card/Diag Random)",8},
	{"8 Shots",9},
	{"Breath",10},
	{"Stream",11},
};

static const GUI::ListData list_dodongomisc10
{
	{"NES",0},
	{"BS-Zelda",1},
};

static const GUI::ListData list_digdoggermisc10
{
	{"Digdogger",0},
	{"Kid",1},
};

static const GUI::ListData list_wizzrobemisc1
{
	{"Teleport",0},
	{ "Phase",1 }
};

static const GUI::ListData list_wizzrobemisc2
{
	{"1 shot", 0},
	{"8 shot", 1},
	{"Summon", 2},
	{"Summon (Layer)", 3}
};

static const GUI::ListData list_keesemisc1
{
	{"Keese",0},
	{"Bat",1},
	{"Keese(Fast)",2},
};

static const GUI::ListData list_keesemisc2
{
	{"Normal",0},
	{"Tribble",1},
};

static const GUI::ListData list_trapmisc2
{
	{"Line Of Sight",0},
	{"Constant",1}
};

static const GUI::ListData list_trapmisc1
{
	{"4-Way",0},
	{"Horizontal",1},
	{"Vertical",2},
};

static const GUI::ListData list_leevermisc1
{
	{"Player's path + second",0},
	{"In place",1},
	{"Player's path",2}
};

static const GUI::ListData list_rockmisc10
{
	{"1x1",0},
	{"2x2",1},
};

// 0: no, 1: yes
static const GUI::ListData list_yesnomisc
{
	{"No",0},
	{"Yes",1},
};

// 0: yes, 1: no
static const GUI::ListData list_noyesmisc
{
	{"Yes",0},
	{"No",1},
};

static const GUI::ListData list_walkerspawn
{
	{"Puff",0},
	{"Flicker",1},
	{"Instant",2},
};

void EnemyEditorDialog::refreshScript()
{
	//loadEnemyType();
	//active
	int32_t sw_initd[8];
	for (auto q = 0; q < 8; ++q)
	{
		l_initd[q] = "InitD[" + to_string(q) + "]:";
		h_initd[q].clear();
		sw_initd[q] = -1;
	}
	if (local_guyref.script)
	{
		zasm_meta const& meta = guyscripts[local_guyref.script]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (unsigned(meta.initd_type[q]) < nswapMAX)
				sw_initd[q] = meta.initd_type[q];
			if (meta.initd[q].size())
				l_initd[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_initd[q] = meta.initd_help[q];
		}
	}
	else
	{
		for (auto q = 0; q < 8; ++q)
			sw_initd[q] = nswapDEC;
	}
	//weapon
	int32_t sw_wpninitd[8];
	for (auto q = 0; q < 8; ++q)
	{
		l_wpninitd[q] = "InitD[" + to_string(q) + "]:";
		h_wpninitd[q].clear();
		sw_wpninitd[q] = -1;
	}
	if (local_guyref.script)
	{
		zasm_meta const& meta = ewpnscripts[local_guyref.weaponscript]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (unsigned(meta.initd_type[q]) < nswapMAX)
				sw_wpninitd[q] = meta.initd_type[q];
			if (meta.initd[q].size())
				l_wpninitd[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_wpninitd[q] = meta.initd_help[q];
		}
	}
	else
	{
		for (auto q = 0; q < 8; ++q)
			sw_wpninitd[q] = nswapDEC;
	}

	for (auto q = 0; q < 8; ++q)
	{
		//active
		ib_initds[q]->setDisabled(h_initd[q].empty());
		l_initds[q]->setText(l_initd[q]);
		if (sw_initd[q] > -1)
			tf_initd[q]->setSwapType(sw_initd[q]);
		//weapon
		ib_wpninitds[q]->setDisabled(h_wpninitd[q].empty());
		l_wpninitds[q]->setText(l_wpninitd[q]);
		if (sw_wpninitd[q] > -1)
			tf_wpninitd[q]->setSwapType(sw_wpninitd[q]);
	}
}

void EnemyEditorDialog::loadEnemyType()
{
	for (int q = 0; q < 32; ++q)
		l_attribute[q] = fmt::format("Misc Attr. {}:", q + 1);
	l_bflag[0] = "Enemy is Completely Invisible";
	l_bflag[1] = "Item Specified in Attributes 13 Dispels Invisibility";
	l_bflag[2] = "Doesn't always return";
	l_bflag[3] = "Draw Invisible as Cloaked";
	l_bflag[4] = "BFlags[4]";
	l_bflag[5] = "BFlags[5]";
	l_bflag[6] = "BFlags[6]";
	l_bflag[7] = "BFlags[7]";
	l_bflag[8] = "BFlags[8]";
	l_bflag[9] = "BFlags[9]";
	l_bflag[10] = "BFlags[10]";
	l_bflag[11] = "Toggle Move Offscreen";
	l_bflag[12] = "Fast Drawing";
	l_bflag[13] = "Ignore Sideview Ladders/Platforms";
	l_bflag[14] = "Move Off-Grid (WIP)";
	l_bflag[15] = "Render Cloaked Instead of VISIBLE";
	
	switch(local_guyref.family)
	{
		case eeWALK:
		{
			l_attribute[0] = "Shot Type:";
			l_attribute[1] = "Death Type:";
			l_attribute[2] = "Death Attr. 1:";
			l_attribute[3] = "Death Attr. 2:";
			l_attribute[4] = "Death Attr. 3:";
			l_attribute[5] = "Extra Shots:";
			l_attribute[6] = "Touch Effects:";
			l_attribute[7] = "Effect Strength:";
			l_attribute[8] = "Walk Style:";			
			l_attribute[9] = "Walk Attribute:";
			l_bflag[4] = "Split in Place";
			break;
		}
		case eeGLEEOK:
		{
			l_attribute[0] = "Heads:";
			l_attribute[1] = "Head HP:";
			l_attribute[2] = "Shot Type:";
			l_attribute[3] = "Unused";
			l_attribute[4] = "Neck Segments:";
			l_attribute[5] = "Neck Offset 1:";
			l_attribute[6] = "Neck Offset 2:";
			l_attribute[7] = "Head Offset";
			l_attribute[8] = "Flying Head Offset";
			l_attribute[9] = "Unused";
			l_bflag[2] = "BFlags[2]";
			l_bflag[4] = "Obeys Spawn Points";
			l_bflag[5] = "Center Spawn X Point";
			l_bflag[6] = "Heads Block Projectiles";
			break;
		}
		case eeDIG:
		{
			l_attribute[0] = "Enemy 1 ID:";
			l_attribute[1] = "Enemy 2 ID:";
			l_attribute[2] = "Enemy 3 ID:";
			l_attribute[3] = "Enemy 4 ID";
			l_attribute[4] = "Enemy 1 Qty:";
			l_attribute[5] = "Enemy 2 Qty:";
			l_attribute[6] = "Enemy 3 Qty:";
			l_attribute[7] = "Enemy 4 Qty:";
			l_attribute[8] = "Unused";
			l_attribute[9] = "Type";
			l_bflag[2] = "BFlags[2]";
			break;
		}
		case eePATRA:
		{
			l_attribute[0] = "Outer Eyes:";
			l_attribute[1] = "Inner Eyes:";
			l_attribute[2] = "Eyes' HP:";
			l_attribute[3] = "Eye Movement:";
			l_attribute[4] = "Shooters:";
			l_attribute[5] = "Pattern Odds:";
			l_attribute[6] = "Pattern Cycles:";
			l_attribute[7] = "Eye Offset:";
			l_attribute[8] = "Eye CSet:";
			l_attribute[9] = "Type:";
			l_attribute[17] = "Shot chance (1/N per frame):";
			l_attribute[18] = "Firing cooldown:";
			l_attribute[19] = "Inner Eyes Firing Pattern:";
			l_attribute[20] = "Warning Spins:";
			l_attribute[21] = "Stays Still:";
			l_attribute[22] = "Outer Ring Loss Speed Boost:";
			l_attribute[23] = "Inner Ring Loss Speed Boost:";
			l_attribute[24] = "Can Fire:";
			l_attribute[25] = "Can Expand:";
			l_attribute[26] = "Inner Eye HP:";
			l_attribute[27] = "Center Eye Firing Pattern:";
			l_attribute[28] = "Outer Eye Radius:";
			l_attribute[29] = "Inner Eye Radius:";
			l_attribute[30] = "Outer Eye Expanded Radius:";
			l_attribute[31] = "Inner Eye Expanded Radius:";
			l_bflag[2] = "Has Firing Animation";
			l_bflag[4] = "Obeys Spawn Points";
			l_bflag[5] = "Slow down when Firing";
			l_bflag[6] = "Don't attack when expanding";
			l_bflag[7] = "Don't expand when spawned";
			l_bflag[8] = "Pattern Odds reset when Expanding";
			l_bflag[9] = "Don't 'expand' without orbiters";
			break;
		}
		case eePROJECTILE:
		{
			l_attribute[0] = "Shot Type:";
			l_attribute[2] = "Shot Attribute 1:";
			l_attribute[3] = "Shot Attribute 2:";
			break;
		}
		case eeGHOMA:
		{
			l_attribute[0] = "Shot Type:";
			l_bflag[2] = "BFlags[2]";
			l_bflag[4] = "Obeys Spawn Points";
			break;
		}
		case eeAQUA:
		{
			l_attribute[0] = "Side:";
			l_bflag[2] = "BFlags[2]";
			l_bflag[4] = "Obeys Spawn Points";
			break;
		}
		case eeMANHAN:
		{
			l_attribute[0] = "Frame Rate:";
			l_attribute[1] = "Size";
			l_bflag[2] = "BFlags[2]";
			break;
		}
		case eeLANM:
		{
			l_attribute[0] = "Segments:";
			l_attribute[1] = "Segment Lag:";
			l_attribute[2] = "Item per segment:";
			l_bflag[2] = "BFlags[2]";
			l_bflag[4] = "Obeys Spawn Points";
			break;
		}
		case eeMOLD:
		{
			l_attribute[0] = "Segments:";
			l_attribute[1] = "Item per segment:";
=
			l_bflag[2] = "BFlags[2]";
			l_bflag[4] = "Obeys Spawn Points";
			break;
		}
		case eeWIZZ:
		{
			l_attribute[0] = "Walk Style:";
			l_attribute[1] = "Shot Type:";
			l_attribute[2] = "Shot Attr. 1:";
			l_attribute[3] = "Solid Combos OK:";
			l_attribute[4] = "Teleport Delay:";
			l_bflag[4] = "Old Windrobe Teleport";
			break;
		}
		case eeDONGO:
		{
			l_attribute[9] = "Type:";
			l_bflag[2] = "BFlags[2]";
			break;
		}
		case eeKEESE:
		{
			l_attribute[0] = "Walk Style:";
			l_attribute[1] = "Death Type:";
			l_attribute[2] = "Enemy Id:";
			l_attribute[13] = "Landing Chance (1/N):",
			l_attribute[14] = "Landing Cooldown:",
			l_attribute[15] = "Halt Duration:";
			l_attribute[16] = "Acceleration Frame Interval";
			l_attribute[17] = "Acceleration Step Modifier:";
			l_attribute[18] = "Spawn Step:";
			l_attribute[19] = "Tribble Timer";
			break;
		}
		case eeTEK:
		{
			l_attribute[0] = "1/N Jump Start:";
			l_attribute[1] = "1/N Jump Cont.:";
			l_attribute[2] = "Jump Z Velocity:";
			break;
		}
		case eeLEV:
		{
			l_attribute[0] = "Emerge style:";
			l_attribute[1] = "Submerged CSet:";
			l_attribute[2] = "Emerge step:";
			break;
		}
		case eeWALLM:
		{
			l_attribute[0] = "Fixed Distance";
			break;
		}
		case eeTRAP:
		{
			l_attribute[0] = "Direction:";
			l_attribute[1] = "Move Style:";
			break;
		}
		case eeROCK:
		{
			l_attribute[9] = "Size:";
			
			break;
		}
		case eeNONE:
		{
			l_attribute[9] = "Boss Death Trigger:";

			break;
		}
		case eeGHINI: case eePEAHAT: //TODO DEPRECIATE ME
		{
			l_attribute[0] = "Stop On Pits";
			l_attribute[13] = "Landing Chance (1/N):",
			l_attribute[14] = "Landing Cooldown:",
			l_attribute[15] = "Halt Duration:";
			l_attribute[16] = "Acceleration Factor:";
			break;
		}
		case eeZORA:
		{
			l_bflag[4] = "Lava Only";
			l_bflag[5] = "Lava and Water";
			l_bflag[6] = "Shallow Water";
			l_bflag[7] = "Doesn't regenerate health";
			break;
		}
		case eeSPINTILE:
		{
			//no attributes or flags either (yet)
			break;
		}
		case eeGANON:
		{
			l_attribute[13] = "Death Shot Item ID"; //TODO MOVE ME TO MISC ZERO AND ADD ME TO DEFAULT DATA FOR OLD QUEST
			break;
		}
		default: //script and friendly families
		{
			break;
		}
	}

	//TODO ADD META DATA

	for (size_t q = 0; q < 32; ++q)
	{
		l_attributes[q]->setText(l_attribute[q]);
		if (q > 15) continue;
		l_bflags[q]->setText(l_bflag[q]);
	}
	updateWarnings();
	pendDraw();
}

void EnemyEditorDialog::updateWarnings()
{
	warnings.clear();
	warnbtn->setDisabled(warnings.empty());
}

//{ Widgets

std::shared_ptr<GUI::Widget> EnemyEditorDialog::NumberField(auto* data, auto _min, auto _max, string str)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(
		Label(text = str),
		TextField(
			type = GUI::TextField::type::INT_DECIMAL,
			maxLength = 5,
			val = *data,
			low = _min,
			high = _max,
			fitParent = true,
			onValChangedFunc = [data](GUI::TextField::type, std::string_view, int32_t val)
			{
				*data = val;
			}
		)
	);
}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::NameField(string str)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(
		Label(text=str),
		TextField(
			type = GUI::TextField::type::TEXT,
			maxLength = 63,
			fitParent = true,
			text = enemy_name,
			onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
			{
				enemy_name = text;
			}
		)
	);
}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::MiscAttribute(int32_t* index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return TextField(
		type = GUI::TextField::type::INT_DECIMAL,
		maxLength = 6,
		val = *index,
		low = -99999,
		high = 999999,
		fitParent = true,
		onValChangedFunc = [index](GUI::TextField::type, std::string_view, int32_t val)
		{
			*index = val;
		}
	);
}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::DropDownField(auto* field, GUI::ListData ls)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return DropDownList(
		data = ls,
		vPadding = 0_px,
		fitParent = true, selectedValue = *field,
		onSelectFunc = [field](int32_t val)
		{
			*field = val;
		}
	);
}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::MiscFlag(auto* bitstring, auto index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Checkbox(
		checked = *bitstring & index,
		fitParent = true,
		onToggleFunc = [bitstring, index](bool state)
		{
			SETFLAG(*bitstring, index, state);
		}
	);
}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	//MACRO DEFINES GO HERE LOCALLY INSTEAD OF IN GLOBAL SCOPE
#define MAXHALT (local_guyref.family==eeFIRE||local_guyref.family==eeOTHER||(local_guyref.family>=eeSCRIPT01&&local_guyref.family<=eeFFRIENDLY10))?999:16
#define MAXRATE (local_guyref.family==eeFIRE||local_guyref.family==eeOTHER||(local_guyref.family>=eeSCRIPT01&&local_guyref.family<=eeFFRIENDLY10))?999:16
#define MAXHOMING (local_guyref.family==eeFIRE||local_guyref.family==eeOTHER||(local_guyref.family>=eeSCRIPT01&&local_guyref.family<=eeFFRIENDLY10))?9999:256
#define MAXSTEP (local_guyref.family==eeFIRE||local_guyref.family==eeOTHER||(local_guyref.family>=eeSCRIPT01&&local_guyref.family<=eeFFRIENDLY10))?9999:256
	
	char titlebuf[256];
	sprintf(titlebuf, "Enemy %d: %s", index, guy_string[index]);
	
	//Family Specific
	string turnfreqorhaltrate = "Halt Rate:";
	if (local_guyref.family == eeKEESE || local_guyref.family == eeGHINI || local_guyref.family == eePEAHAT || local_guyref.family == eeMANHAN
		|| local_guyref.family == eeGLEEOK || local_guyref.family == eePATRA || local_guyref.family == eeDIG) turnfreqorhaltrate = "Turn Freq:";

	w_attributes[0] = MiscAttribute(&local_guyref.misc1);
	w_attributes[1] = MiscAttribute(&local_guyref.misc2);
	w_attributes[2] = MiscAttribute(&local_guyref.misc3);
	w_attributes[3] = MiscAttribute(&local_guyref.misc4);
	w_attributes[4] = MiscAttribute(&local_guyref.misc5);
	w_attributes[5] = MiscAttribute(&local_guyref.misc6);
	w_attributes[6] = MiscAttribute(&local_guyref.misc7);
	w_attributes[7] = MiscAttribute(&local_guyref.misc8);
	w_attributes[8] = MiscAttribute(&local_guyref.misc9);
	w_attributes[9] = MiscAttribute(&local_guyref.misc10);
	w_attributes[10] = MiscAttribute(&local_guyref.misc11);
	w_attributes[11] = MiscAttribute(&local_guyref.misc12);
	w_attributes[12] = MiscAttribute(&local_guyref.misc13);
	w_attributes[13] = MiscAttribute(&local_guyref.misc14);
	w_attributes[14] = MiscAttribute(&local_guyref.misc15);
	w_attributes[15] = MiscAttribute(&local_guyref.misc16);
	w_attributes[16] = MiscAttribute(&local_guyref.misc17);
	w_attributes[17] = MiscAttribute(&local_guyref.misc18);
	w_attributes[18] = MiscAttribute(&local_guyref.misc19);
	w_attributes[19] = MiscAttribute(&local_guyref.misc20);
	w_attributes[20] = MiscAttribute(&local_guyref.misc21);
	w_attributes[21] = MiscAttribute(&local_guyref.misc22);
	w_attributes[22] = MiscAttribute(&local_guyref.misc23);
	w_attributes[23] = MiscAttribute(&local_guyref.misc24);
	w_attributes[24] = MiscAttribute(&local_guyref.misc25);
	w_attributes[25] = MiscAttribute(&local_guyref.misc26);
	w_attributes[26] = MiscAttribute(&local_guyref.misc27);
	w_attributes[27] = MiscAttribute(&local_guyref.misc28);
	w_attributes[28] = MiscAttribute(&local_guyref.misc29);
	w_attributes[29] = MiscAttribute(&local_guyref.misc30);
	w_attributes[30] = MiscAttribute(&local_guyref.misc31);
	w_attributes[31] = MiscAttribute(&local_guyref.misc32);

	//WALKER
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_walkmisc1);
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_walkmisc2);
	w_attributes[6] = DropDownField(&local_guyref.misc7, list_walkmisc7);
	w_attributes[8] = DropDownField(&local_guyref.misc9, list_walkmisc9);

	//FLOATER (unimplemented) (should i just use keese for em?)

	//WORM (unimplemented)

	//GLEEOK
	w_attributes[2] = DropDownField(&local_guyref.misc3, list_gleeokmisc3);

	//DIG
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_enemies);
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_enemies);
	w_attributes[2] = DropDownField(&local_guyref.misc3, list_enemies);
	w_attributes[3] = DropDownField(&local_guyref.misc4, list_enemies);
	w_attributes[9] = DropDownField(&local_guyref.misc10, list_digdoggermisc10);

	//PATRA
	w_attributes[3] = DropDownField(&local_guyref.misc4, list_patramisc4);
	w_attributes[4] = DropDownField(&local_guyref.misc5, list_patramisc5);
	w_attributes[9] = DropDownField(&local_guyref.misc10, list_patramisc10);
	w_attributes[19] = DropDownField(&local_guyref.misc20, list_patramisc20);
	w_attributes[21] = DropDownField(&local_guyref.misc22, list_patramisc22);
	w_attributes[24] = DropDownField(&local_guyref.misc25, list_patramisc25);
	w_attributes[25] = DropDownField(&local_guyref.misc26, list_patramisc26);
	w_attributes[28] = DropDownField(&local_guyref.misc28, list_patramisc28);

	//PROJECTILE
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_walkmisc1);
	
	//GHOMA
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_gohmamisc1);
	
	//AQUAMENTUS
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_aquamisc1);

	//MANHANDLA
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_manhandlamisc2);

	//LANMOLA
	w_attributes[2] = DropDownField(&local_guyref.misc3, list_yesnomisc);

	//MOLDORM
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_yesnomisc);

	//WIZZ
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_wizzrobemisc1);
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_wizzrobemisc2);
	w_attributes[3] = DropDownField(&local_guyref.misc4, list_yesnomisc);

	//DODONGO
	w_attributes[9] = DropDownField(&local_guyref.misc10, list_dodongomisc10);

	//KEESE
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_keesemisc1);
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_keesemisc2);
	w_attributes[2] = DropDownField(&local_guyref.misc3, list_enemies);

	//LEV
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_leevermisc1);

	//WALLM
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_noyesmisc);

	//TRAP
	w_attributes[0] = DropDownField(&local_guyref.misc1, list_trapmisc1);
	w_attributes[1] = DropDownField(&local_guyref.misc2, list_trapmisc2);

	//ROCK
	w_attributes[9] = DropDownField(&local_guyref.misc10, list_rockmisc10);

	//NONE (Boss Death Triggers)
	w_attributes[9] = DropDownField(&local_guyref.misc10, list_yesnomisc);

	auto basics_tab = TabPanel(
		ptr = &guy_tabs[0],
		TabRef(name = "Basics", TabPanel(
			ptr = &guy_tabs[1],
			TabRef(name = "Data", Row(
				Column(
					Row(
						NameField(enemy_name)
					),
					Row(
						Columns<3>(
							Label(text = "Old"),
							tswatch[0] = SelTileSwatch(
								tile = local_guyref.tile,
								cset = local_guyref.cset,
								showFlip = false,
								showvals = false,
								onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
								{
									local_guyref.tile = t;
									local_guyref.cset = 14 ? 14 : c;
									for (int q = 0; q < 3; ++q)
										tswatch[q]->setCSet(local_guyref.cset);
								}
							),
							NumberField(&local_guyref.width, 0, 20, "W:"),
							NumberField(&local_guyref.height, 0, 20, "H:"),
							Label(text = "Special"),
							tswatch[1] = SelTileSwatch(
								tile = local_guyref.s_tile,
								cset = local_guyref.cset,
								showFlip = false,
								showvals = false,
								onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
								{
									local_guyref.s_tile = t;
									local_guyref.cset = 14 ? 14 : c;
									for (int q = 0; q < 3; ++q)
										tswatch[q]->setCSet(local_guyref.cset);
								}
							),
							NumberField(&local_guyref.s_width, 0, 20, "W:"),
							NumberField(&local_guyref.s_height, 0, 20, "H:"),
							Label(text = "New"),
							tswatch[2] = SelTileSwatch(
								tile = local_guyref.e_tile,
								cset = local_guyref.cset,
								showFlip = false,
								showvals = false,
								onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
								{
									local_guyref.e_tile = t;
									local_guyref.cset = 14 ? 14 : c;
									for (int q = 0; q < 3; ++q)
										tswatch[q]->setCSet(local_guyref.cset);
								}
							),
							NumberField(&local_guyref.e_width, 0, 20, "W:"),
							NumberField(&local_guyref.e_height, 0, 20, "H:")
						),
						Columns<4>(
							NumberField(&local_guyref.hp, 0, 32767, "HP"),
							NumberField(&local_guyref.dp, 0, 32767, "Damage"),
							NumberField(&local_guyref.wdp, 0, 32767, "W. Damage"),
							NumberField(&local_guyref.grumble, 0, 4, "Hunger"),
							NumberField(&local_guyref.hrate, 0, MAXHALT, turnfreqorhaltrate),
							NumberField(&local_guyref.rate, 0, MAXRATE, "Rate"),
							NumberField(&local_guyref.homing, 0, MAXHOMING, "Homing"),
							NumberField(&local_guyref.step, 0, MAXSTEP, "Step")
						)
					),
					Row(
						Columns<3>(
							Label(text="Type"),
							Label(text="Old Anim:"),
							Label(text="New Anim:"),
							DropDownField(&local_guyref.family,list_families),
							DropDownField(&local_guyref.anim, list_animations),
							DropDownField(&local_guyref.e_anim, list_animations),
							Row(
								//this one is a bit special there is no flag it just sets cset to 14
								Checkbox(
									text = "Use Boss Pal", hAlign = 0.0,
									checked = local_guyref.cset==14,
									fitParent=true,
									onToggleFunc = [&](bool state)
									{
										local_guyref.cset = state ? 14 : 8;
									}
								),
								NumberField(&local_guyref.bosspal, -1, 29, "Boss Pal:")
							),
							NumberField(&local_guyref.frate, 0, 256, "F Rate:"),
							NumberField(&local_guyref.e_frate, 0, 256, "F Rate:")
						)
					),
					Row(
						Label(text="Item Set:"),
						DropDownField(&local_guyref.item_set, list_dropsets)
					)
				)
			)),
			TabRef(name = "Attributes 1", Row(
				Columns<16>(
					Switcher(

					)
				)
			)),
			TabRef(name = "Attributes 2", Row(
				Columns<16>(
				)
			)),
			TabRef(name = "Size Flags", Row(
				Columns<10>(
					NumberField(&local_guyref.txsz,0,4,"TileWidth:"),
					NumberField(&local_guyref.tysz,0,4,"TileHeight:"),
					NumberField(&local_guyref.hxsz,0,64,"HitWidth:"),
					NumberField(&local_guyref.hysz,0,64,"HitHeight:"),
					NumberField(&local_guyref.hzsz,0,64,"HitZHeight:"),
					NumberField(&local_guyref.hxofs,-64,64,"HitXOffset:"),
					NumberField(&local_guyref.hyofs,-64,64,"HitYOffset:"),
					NumberField(&local_guyref.zofs,-1000,1000,"DrawZOffset:"),
					NumberField(&local_guyref.xofs,-1000,1000,"DrawXOffset:"),
					NumberField(&local_guyref.yofs,-1000,1000,"DrawYOffset:"),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_TILE_WIDTH),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_TILE_HEIGHT),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_HIT_WIDTH),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_HIT_HEIGHT),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_HIT_Z_HEIGHT),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_HIT_X_OFFSET),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_HIT_Y_OFFSET),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_DRAW_Z_OFFSET),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_DRAW_X_OFFSET),
					MiscFlag(&local_guyref.SIZEflags, guyflagOVERRIDE_DRAW_Y_OFFSET),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled"),
					Label(text="Enabled")
				)
			))
		))
	);

	auto defenses_tab = TabPanel(
		ptr = &guy_tabs[2],
		TabRef(name = "Defenses", TabPanel(
		))
	);
	auto flags_tab = TabPanel(
		ptr = &guy_tabs[3],
		TabRef(name = "Flags", TabPanel(
		))
	);
	auto movement_tab = TabPanel(
		ptr = &guy_tabs[4],
		TabRef(name = "Movement", TabPanel(
		))
	);
	auto attack_tab = TabPanel(
		ptr = &guy_tabs[5],
		TabRef(name = "Attack", TabPanel(
		))
	);
	auto effects_tab = TabPanel(
		ptr = &guy_tabs[6],
		TabRef(name = "Effects", TabPanel(
		))
	);
	auto scripts_tab = TabPanel(
		ptr = &guy_tabs[7],
		TabRef(name = "Scripts", TabPanel(
		))
	);
	window = Window(
		use_vsync = true,
		title = titlebuf,
		info = "Edit enemies, setting up their graphics, effects, and attributes.",
		onClose = message::CANCEL,
		shortcuts = {
			//i actually dont know of any, perhaps add shortcut keys for navigation?
		},
		Column(
			TabPanel(
				TabRef(name = "Basics", basics_tab),
				TabRef(name = "Defenses",defenses_tab),
				TabRef(name = "Flags",flags_tab),
				TabRef(name = "Movement",movement_tab),
				TabRef(name = "Attack",attack_tab),
				TabRef(name = "Effects",effects_tab),
				TabRef(name = "Scripts",scripts_tab)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				warnbtn = Button(
					text = "Warnings",
					minwidth = 90_px,
					onClick = message::WARNINGS),
				Button(
					text = "Clear",
					minwidth = 90_px,
					onClick = message::CLEAR),
				Button(
					text = "Default",
					minwidth = 90_px,
					onClick = message::DEFAULT),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshScript();
	updateWarnings();
	return window;
}

void EnemyEditorDialog::apply_enemy()
{
	guysbuf[index] = local_guyref;
	strncpy(guy_string[index], enemy_name.c_str(), 63);
	saved = false;
	edited = true;
}

bool EnemyEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	guydata blank;
	switch (msg.message)
	{
	case message::CLEAR:
	{
		bool doclear = false;
		AlertDialog("Are you sure?",
			"Clearing the enemy will set all values to null!",
			[&](bool ret, bool)
			{
				doclear = ret;
			}).show();
		if (doclear)
		{
			local_guyref = guysbuf[eNONE];
			sprintf(guy_string[index], "e%03d", index);
			rerun_dlg = true;
			return true;
		}
		return false;
	}
	case message::DEFAULT:
	{
		bool doclear = false;
		AlertDialog("Are you sure?",
			"This will reset all values back to their defaults!",
			[&](bool ret, bool)
			{
				doclear = ret;
			}).show();
		if (doclear)
		{
			if (index < 177)
			{
				local_guyref = default_guys[index];
				strncpy(guy_string[index], old_guy_string[index], strlen(old_guy_string[index]));
			}
			else
			{
				local_guyref = default_guys[eNONE]; //enemies above 176 dont have defaults!
				sprintf(guy_string[index], "e%03d", index);
			}
			rerun_dlg = true;
			return true;
		}
		return false;
	}

	case message::WARNINGS:
		if (warnings.size())
			displayinfo("Warnings", warnings,
				"The following issues were found with this enemy:");
		return false;
	case message::OK:
	{
		if (warnings.size())
		{
			bool cancel = false;
			AlertDialog alert("Warnings", warnings, [&](bool ret, bool)
				{
					if (!ret) cancel = true;
				});
			alert.setSubtext("The following issues were found with this enemy:");
			alert.show();
			if (cancel)
				return false;
		}
		apply_enemy();
		return true;
	}

	case message::CANCEL:
		return true;
	}
	return false;
}

