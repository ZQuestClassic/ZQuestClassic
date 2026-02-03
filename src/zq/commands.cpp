#include "zq/commands.h"
#include "base/dmap.h"
#include "base/general.h"
#include "base/qst.h"
#include "base/zapp.h"
#include "dialog/compilezscript.h"
#include "dialog/quest_rules.h"
#include "zq/package.h"
#include "zq/zq_class.h"
#include "zq/zquest.h"
#include <fstream>

extern bool is_zq_replay_test;

// Removes the top layer encoding from a quest file. See open_quest_file.
// This has zero impact on the contents of the quest file. There should be no way for this to
// break anything.
static void do_unencrypt_qst_command(const char* input_filename, const char* output_filename)
{
	// If the file is already an unencrypted packfile, there's nothing to do.
	PACKFILE* pf_check = pack_fopen_password(input_filename, F_READ_PACKED, "");
	pack_fclose(pf_check);
	if (pf_check) return;

	int32_t error;
	PACKFILE* pf = open_quest_file(&error, input_filename, false);
	PACKFILE* pf2 = pack_fopen_password(output_filename, F_WRITE_PACKED, "");
	int c;
	while ((c = pack_getc(pf)) != EOF)
	{
		pack_putc(c, pf2);
	}
	pack_fclose(pf);
	pack_fclose(pf2);
	clear_quest_tmpfile();

	zq_exit(0);
}

// This will remove the PACKFILE compression. Incidentally, it also removes the top encoding layer.
static void do_uncompress_qst_command(const char* input_filename, const char* output_filename)
{
	auto unencrypted_result = try_open_maybe_legacy_encoded_file(input_filename, ENC_STR, nullptr, QH_NEWIDSTR, QH_IDSTR);
	if (unencrypted_result.not_found)
	{
		printf("qst not found\n");
		zq_exit(1);
	}
	if (!unencrypted_result.compressed && !unencrypted_result.encrypted)
	{
		// If the file is already an uncompressed file, there's nothing to do but copy it.
		fs::copy(input_filename, output_filename);
		zq_exit(0);
		return;
	}

	pack_fclose(unencrypted_result.decoded_pf);

	int32_t error;
	PACKFILE* pf = open_quest_file(&error, input_filename, false);
	PACKFILE* pf2 = pack_fopen_password(output_filename, F_WRITE, "");
	int c;
	while ((c = pack_getc(pf)) != EOF)
	{
		pack_putc(c, pf2);
	}
	pack_fclose(pf);
	pack_fclose(pf2);
	clear_quest_tmpfile();

	zq_exit(0);
}

// Copy a quest file by loading and resaving, exactly like if the user did it in the UI.
// Note there could be changes introduced in the loading or saving functions. These are
// typically for compatability, but could possibly be a source of bugs.
static void do_copy_qst_command(const char* input_filename, const char* output_filename)
{
	set_headless_mode();

	// We need to init some stuff before loading a quest file will work.
	int fake_errno = 0;
	allegro_errno = &fake_errno;
	get_qst_buffers();

	int ret = load_quest(input_filename, false);
	if (ret)
	{
		zq_exit(ret);
	}

	ret = save_quest(output_filename, false);
	zq_exit(ret);
}

static std::vector<std::string> wrap_qr_info(std::string info)
{
	static const int max_chars = 140;
	info = info.substr(0, info.find_last_not_of(" \t\n")+1);
	if (info.empty())
		return {};
	if (auto ws_idx = info.find_first_not_of(" \t\n"); ws_idx != std::string::npos)
		info = info.substr(ws_idx);
	if (info.size() <= max_chars && info.find_first_of("\n") == std::string::npos)
		return { info };
	std::vector<std::string> ret;
	
	size_t idx = 0;
	while (idx < info.size())
	{
		while (info[idx] == '\n')
		{
			ret.push_back("");
			++idx;
		}
		idx = info.find_first_not_of(" ", idx); // skip leading ws
		
		size_t newline_idx = info.find_first_of("\n", idx);
		size_t new_idx = idx + max_chars;
		if (newline_idx != std::string::npos)
			new_idx = zc_min(newline_idx, new_idx);
		if (new_idx < info.size())
		{
			for (int ptr = new_idx - idx; ptr >= 0; --ptr)
			{
				char c = info[idx+ptr];
				if (c == ' ' || c == '\n')
					break;
				if (ptr == max_chars) continue;
				new_idx = idx + ptr;
			}
		}
		std::string tmp = info.substr(idx, new_idx-idx);
		tmp = tmp.substr(0, tmp.find_last_not_of(" ")+1); // trim trailing ws
		ret.push_back(tmp);
		// if (info[new_idx] == "\n")
			// ++new_idx;
		idx = new_idx;
	}
	return ret;
}

void do_dev_qrs_zscript_command(std::string const& fname)
{
	static std::map<int, std::string> qr_const_names = {
		{ qr_SOLIDBLK, "qr_SOLIDBLK" },
		{ qr_NOTMPNORET, "qr_NOTMPNORET" },
		{ qr_ALWAYSRET, "qr_ALWAYSRET" },
		{ qr_MEANTRAPS, "qr_MEANTRAPS" },
		{ qr_BSZELDA, "qr_BSZELDA" },
		{ qr_FADE, "qr_FADE" },
		{ qr_FADECS5, "qr_FADECS5" },
		{ qr_FASTDNGN, "qr_FASTDNGN" },
		{ qr_NOLEVEL3FIX, "qr_NOLEVEL3FIX" },
		{ qr_COOLSCROLL, "qr_COOLSCROLL" },
		{ qr_NOSCROLL, "qr_NOSCROLL" },
		{ qr_4TRI, "qr_4TRI" },
		{ qr_EXPLICIT_RAFTING, "qr_EXPLICIT_RAFTING" },
		{ qr_FASTFILL, "qr_FASTFILL" },
		{ qr_CAVEEXITNOSTOPMUSIC, "qr_CAVEEXITNOSTOPMUSIC" },
		{ qr_3TRI, "qr_3TRI" },
		{ qr_TIME, "qr_TIME" },
		{ qr_FREEFORM, "qr_FREEFORM" },
		{ qr_KILLALL, "qr_KILLALL" },
		{ qr_NOFLICKER, "qr_NOFLICKER" },
		{ qr_CONTFULL_DEP, "qr_CONTFULL_DEP" },
		{ qr_RLFIX, "qr_RLFIX" },
		{ qr_LENSHINTS, "qr_LENSHINTS" },
		{ qr_HERODUNGEONPOSFIX, "qr_HERODUNGEONPOSFIX" },
		{ qr_HOLDITEMANIMATION, "qr_HOLDITEMANIMATION" },
		{ qr_HESITANTPUSHBLOCKS, "qr_HESITANTPUSHBLOCKS" },
		{ qr_HIDECARRIEDITEMS, "qr_HIDECARRIEDITEMS" },
		{ qr_FFCSCROLL, "qr_FFCSCROLL" },
		{ qr_RAFTLENS, "qr_RAFTLENS" },
		{ qr_SMOOTHVERTICALSCROLLING, "qr_SMOOTHVERTICALSCROLLING" },
		{ qr_WHIRLWINDMIRROR, "qr_WHIRLWINDMIRROR" },
		{ qr_NOFLASHDEATH, "qr_NOFLASHDEATH" },
		{ qr_HOLDNOSTOPMUSIC, "qr_HOLDNOSTOPMUSIC" },
		{ qr_FIREPROOFHERO, "qr_FIREPROOFHERO" },
		{ qr_OUCHBOMBS, "qr_OUCHBOMBS" },
		{ qr_NOCLOCKS_DEP, "qr_NOCLOCKS_DEP" },
		{ qr_TEMPCLOCKS_DEP, "qr_TEMPCLOCKS_DEP" },
		{ qr_BRKBLSHLDS_DEP, "qr_BRKBLSHLDS_DEP" },
		{ qr_BRKNSHLDTILES, "qr_BRKNSHLDTILES" },
		{ qr_MEANPLACEDTRAPS, "qr_MEANPLACEDTRAPS" },
		{ qr_PHANTOMPLACEDTRAPS, "qr_PHANTOMPLACEDTRAPS" },
		{ qr_ALLOWFASTMSG, "qr_ALLOWFASTMSG" },
		{ qr_LINKEDCOMBOS, "qr_LINKEDCOMBOS" },
		{ qr_NOGUYFIRES, "qr_NOGUYFIRES" },
		{ qr_HEARTRINGFIX, "qr_HEARTRINGFIX" },
		{ qr_NOHEARTRING, "qr_NOHEARTRING" },
		{ qr_OVERWORLDTUNIC, "qr_OVERWORLDTUNIC" },
		{ qr_SWORDWANDFLIPFIX, "qr_SWORDWANDFLIPFIX" },
		{ qr_ENABLEMAGIC, "qr_ENABLEMAGIC" },
		{ qr_MAGICWAND_DEP, "qr_MAGICWAND_DEP" },
		{ qr_MAGICCANDLE_DEP, "qr_MAGICCANDLE_DEP" },
		{ qr_MAGICBOOTS_DEP, "qr_MAGICBOOTS_DEP" },
		{ qr_NONBUBBLEMEDICINE, "qr_NONBUBBLEMEDICINE" },
		{ qr_NONBUBBLEFAIRIES, "qr_NONBUBBLEFAIRIES" },
		{ qr_NONBUBBLETRIFORCE, "qr_NONBUBBLETRIFORCE" },
		{ qr_NEWENEMYTILES, "qr_NEWENEMYTILES" },
		{ qr_NOROPE2FLASH_DEP, "qr_NOROPE2FLASH_DEP" },
		{ qr_NOBUBBLEFLASH_DEP, "qr_NOBUBBLEFLASH_DEP" },
		{ qr_GHINI2BLINK_DEP, "qr_GHINI2BLINK_DEP" },
		{ qr_BITMAPOFFSETFIX, "qr_BITMAPOFFSETFIX" },
		{ qr_PHANTOMGHINI2_DEP, "qr_PHANTOMGHINI2_DEP" },
		{ qr_Z3BRANG_HSHOT, "qr_Z3BRANG_HSHOT" },
		{ qr_NOITEMMELEE, "qr_NOITEMMELEE" },
		{ qr_SHADOWS, "qr_SHADOWS" },
		{ qr_TRANSSHADOWS, "qr_TRANSSHADOWS" },
		{ qr_QUICKSWORD, "qr_QUICKSWORD" },
		{ qr_BOMBHOLDFIX, "qr_BOMBHOLDFIX" },
		{ qr_EXPANDEDLTM, "qr_EXPANDEDLTM" },
		{ qr_NOPOTIONCOMBINE_DEP, "qr_NOPOTIONCOMBINE_DEP" },
		{ qr_HEROFLICKER, "qr_HEROFLICKER" },
		{ qr_SHADOWSFLICKER, "qr_SHADOWSFLICKER" },
		{ qr_WALLFLIERS, "qr_WALLFLIERS" },
		{ qr_NOBOMBPALFLASH, "qr_NOBOMBPALFLASH" },
		{ qr_HEARTSREQUIREDFIX, "qr_HEARTSREQUIREDFIX" },
		{ qr_PUSHBLOCKCSETFIX, "qr_PUSHBLOCKCSETFIX" },
		{ qr_TRANSLUCENTDIVINEPROTECTIONROCKET_DEP, "qr_TRANSLUCENTDIVINEPROTECTIONROCKET_DEP" },
		{ qr_FLICKERINGDIVINEPROTECTIONROCKET_DEP, "qr_FLICKERINGDIVINEPROTECTIONROCKET_DEP" },
		{ qr_CMBCYCLELAYERS, "qr_CMBCYCLELAYERS" },
		{ qr_DMGCOMBOPRI, "qr_DMGCOMBOPRI" },
		{ qr_WARPSIGNOREARRIVALPOINT, "qr_WARPSIGNOREARRIVALPOINT" },
		{ qr_LTTPCOLLISION, "qr_LTTPCOLLISION" },
		{ qr_LTTPWALK, "qr_LTTPWALK" },
		{ qr_SLOWENEMYANIM_DEP, "qr_SLOWENEMYANIM_DEP" },
		{ qr_TRUEARROWS, "qr_TRUEARROWS" },
		{ qr_NOSAVE, "qr_NOSAVE" },
		{ qr_NOCONTINUE, "qr_NOCONTINUE" },
		{ qr_QUARTERHEART, "qr_QUARTERHEART" },
		{ qr_NOARRIVALPOINT, "qr_NOARRIVALPOINT" },
		{ qr_NOGUYPOOF, "qr_NOGUYPOOF" },
		{ qr_ALLOWMSGBYPASS, "qr_ALLOWMSGBYPASS" },
		{ qr_NODIVING, "qr_NODIVING" },
		{ qr_LAYER12UNDERCAVE, "qr_LAYER12UNDERCAVE" },
		{ qr_NOSCROLLCONTINUE, "qr_NOSCROLLCONTINUE" },
		{ qr_SMARTSCREENSCROLL, "qr_SMARTSCREENSCROLL" },
		{ qr_RINGAFFECTDAMAGE, "qr_RINGAFFECTDAMAGE" },
		{ qr_ALLOW10RUPEEDROPS_DEP, "qr_ALLOW10RUPEEDROPS_DEP" },
		{ qr_TRAPPOSFIX, "qr_TRAPPOSFIX" },
		{ qr_TEMPCANDLELIGHT, "qr_TEMPCANDLELIGHT" },
		{ qr_REDPOTIONONCE_DEP, "qr_REDPOTIONONCE_DEP" },
		{ qr_OLDSTYLEWARP, "qr_OLDSTYLEWARP" },
		{ qr_NOBORDER, "qr_NOBORDER" },
		{ qr_OLDTRIBBLES_DEP, "qr_OLDTRIBBLES_DEP" },
		{ qr_REFLECTROCKS, "qr_REFLECTROCKS" },
		{ qr_OLDPICKUP, "qr_OLDPICKUP" },
		{ qr_ENEMIESZAXIS, "qr_ENEMIESZAXIS" },
		{ qr_SAFEENEMYFADE, "qr_SAFEENEMYFADE" },
		{ qr_MORESOUNDS, "qr_MORESOUNDS" },
		{ qr_BRANGPICKUP, "qr_BRANGPICKUP" },
		{ qr_HEARTPIECEINCR_DEP, "qr_HEARTPIECEINCR_DEP" },
		{ qr_ITEMSONEDGES, "qr_ITEMSONEDGES" },
		{ qr_EATSMALLSHIELD_DEP, "qr_EATSMALLSHIELD_DEP" },
		{ qr_MSGFREEZE, "qr_MSGFREEZE" },
		{ qr_SLASHFLIPFIX, "qr_SLASHFLIPFIX" },
		{ qr_FIREMAGICSPRITE_DEP, "qr_FIREMAGICSPRITE_DEP" },
		{ qr_SLOWCHARGINGWALK, "qr_SLOWCHARGINGWALK" },
		{ qr_NOWANDMELEE, "qr_NOWANDMELEE" },
		{ qr_SLOWBOMBFUSES_DEP, "qr_SLOWBOMBFUSES_DEP" },
		{ qr_SWORDMIRROR, "qr_SWORDMIRROR" },
		{ qr_SELECTAWPN, "qr_SELECTAWPN" },
		{ qr_LENSSEESENEMIES, "qr_LENSSEESENEMIES" },
		{ qr_INSTABURNFLAGS, "qr_INSTABURNFLAGS" },
		{ qr_DROWN, "qr_DROWN" },
		{ qr_MSGDISAPPEAR, "qr_MSGDISAPPEAR" },
		{ qr_SUBSCREENOVERSPRITES, "qr_SUBSCREENOVERSPRITES" },
		{ qr_BOMBDARKNUTFIX, "qr_BOMBDARKNUTFIX" },
		{ qr_LONGBOMBBOOM_DEP, "qr_LONGBOMBBOOM_DEP" },
		{ qr_OFFSETEWPNCOLLISIONFIX, "qr_OFFSETEWPNCOLLISIONFIX" },
		{ qr_DMGCOMBOLAYERFIX, "qr_DMGCOMBOLAYERFIX" },
		{ qr_ITEMSINPASSAGEWAYS, "qr_ITEMSINPASSAGEWAYS" },
		{ qr_LOG, "qr_LOG" },
		{ qr_FIREPROOFHERO2, "qr_FIREPROOFHERO2" },
		{ qr_NOITEMOFFSET, "qr_NOITEMOFFSET" },
		{ qr_ITEMBUBBLE, "qr_ITEMBUBBLE" },
		{ qr_GOTOLESSNOTEQUAL, "qr_GOTOLESSNOTEQUAL" },
		{ qr_LADDERANYWHERE, "qr_LADDERANYWHERE" },
		{ qr_HOOKSHOTLAYERFIX, "qr_HOOKSHOTLAYERFIX" },
		{ qr_REPLACEOPENDOORS, "qr_REPLACEOPENDOORS" },
		{ qr_OLDLENSORDER, "qr_OLDLENSORDER" },
		{ qr_NOFAIRYGUYFIRES, "qr_NOFAIRYGUYFIRES" },
		{ qr_SCRIPTERRLOG, "qr_SCRIPTERRLOG" },
		{ qr_TRIGGERSREPEAT, "qr_TRIGGERSREPEAT" },
		{ qr_ENEMIESFLICKER, "qr_ENEMIESFLICKER" },
		{ qr_OVALWIPE, "qr_OVALWIPE" },
		{ qr_TRIANGLEWIPE, "qr_TRIANGLEWIPE" },
		{ qr_SMASWIPE, "qr_SMASWIPE" },
		{ qr_NOSOLIDDAMAGECOMBOS, "qr_NOSOLIDDAMAGECOMBOS" },
		{ qr_SHOPCHEAT, "qr_SHOPCHEAT" },
		{ qr_HOOKSHOTDOWNBUG, "qr_HOOKSHOTDOWNBUG" },
		{ qr_OLDHOOKSHOTGRAB, "qr_OLDHOOKSHOTGRAB" },
		{ qr_PEAHATCLOCKVULN, "qr_PEAHATCLOCKVULN" },
		{ qr_VERYFASTSCROLLING, "qr_VERYFASTSCROLLING" },
		{ qr_OFFSCREENWEAPONS, "qr_OFFSCREENWEAPONS" },
		{ qr_BROKENSTATUES, "qr_BROKENSTATUES" },
		{ qr_BOMBCHUSUPERBOMB, "qr_BOMBCHUSUPERBOMB" },
		{ qr_ITEMPICKUPSETSBELOW, "qr_ITEMPICKUPSETSBELOW" },
		{ qr_GANONINTRO, "qr_GANONINTRO" },
		{ qr_MELEEMAGICCOST, "qr_MELEEMAGICCOST" },
		{ qr_OLDMIRRORCOMBOS, "qr_OLDMIRRORCOMBOS" },
		{ qr_BROKENBOOKCOST, "qr_BROKENBOOKCOST" },
		{ qr_OLDSIDEVIEWSPIKES, "qr_OLDSIDEVIEWSPIKES" },
		{ qr_OLDINFMAGIC, "qr_OLDINFMAGIC" },
		{ qr_NEVERDISABLEAMMOONSUBSCREEN, "qr_NEVERDISABLEAMMOONSUBSCREEN" },
		{ qr_ITEMSCRIPTSKEEPRUNNING, "qr_ITEMSCRIPTSKEEPRUNNING" },
		{ qr_SCRIPTSRUNINHEROSTEPFORWARD, "qr_SCRIPTSRUNINHEROSTEPFORWARD" },
		{ qr_FIXSCRIPTSDURINGSCROLLING, "qr_FIXSCRIPTSDURINGSCROLLING" },
		{ qr_SCRIPTDRAWSINWARPS, "qr_SCRIPTDRAWSINWARPS" },
		{ qr_DYINGENEMYESDONTHURTHERO, "qr_DYINGENEMYESDONTHURTHERO" },
		{ qr_SIDEVIEWTRIFORCECELLAR, "qr_SIDEVIEWTRIFORCECELLAR" },
		{ qr_OUTOFBOUNDSENEMIES, "qr_OUTOFBOUNDSENEMIES" },
		{ qr_EPILEPSY, "qr_EPILEPSY" },
		{ qr_SCRIPT_FRIENDLY_ENEMY_TYPES, "qr_SCRIPT_FRIENDLY_ENEMY_TYPES" },
		{ qr_SMARTDROPS, "qr_SMARTDROPS" },
		{ qr_NO_L_R_BUTTON_INVENTORY_SWAP, "qr_NO_L_R_BUTTON_INVENTORY_SWAP" },
		{ qr_USE_EX1_EX2_INVENTORYSWAP, "qr_USE_EX1_EX2_INVENTORYSWAP" },
		{ qr_NOFASTMODE, "qr_NOFASTMODE" },
		{ qr_OLD_F6, "qr_OLD_F6" },
		{ qr_BROKEN_ASKIP_Y_FRAMES, "qr_BROKEN_ASKIP_Y_FRAMES" },
		{ qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY, "qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY" },
		{ qr_SIDEVIEWLADDER_FACEUP, "qr_SIDEVIEWLADDER_FACEUP" },
		{ qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS, "qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS" },
		{ qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS, "qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS" },
		{ qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK, "qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK" },
		{ qr_DOWN_DOESNT_GRAB_LADDERS, "qr_DOWN_DOESNT_GRAB_LADDERS" },
		{ qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS, "qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS" },
		{ qr_OLD_SIDEVIEW_CEILING_COLLISON, "qr_OLD_SIDEVIEW_CEILING_COLLISON" },
		{ qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES, "qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES" },
		{ qr_OLD_ENEMY_KNOCKBACK_COLLISION, "qr_OLD_ENEMY_KNOCKBACK_COLLISION" },
		{ qr_FADEBLACKWIPE, "qr_FADEBLACKWIPE" },
		{ qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT, "qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT" },
		{ qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT, "qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT" },
		{ qr_ANIMATECUSTOMWEAPONS, "qr_ANIMATECUSTOMWEAPONS" },
		{ qr_WEAPONSMOVEOFFSCREEN, "qr_WEAPONSMOVEOFFSCREEN" },
		{ qr_CHECKSCRIPTWEAPONOFFSCREENCLIP, "qr_CHECKSCRIPTWEAPONOFFSCREENCLIP" },
		{ qr_SHORTDGNWALK, "qr_SHORTDGNWALK" },
		{ qr_SCRIPT_WEAPONS_UNIQUE_SPRITES, "qr_SCRIPT_WEAPONS_UNIQUE_SPRITES" },
		{ qr_ANGULAR_REFLECTED_WEAPONS, "qr_ANGULAR_REFLECTED_WEAPONS" },
		{ qr_MIRRORS_USE_WEAPON_CENTER, "qr_MIRRORS_USE_WEAPON_CENTER" },
		{ qr_CUSTOMCOMBOSLAYERS1AND2, "qr_CUSTOMCOMBOSLAYERS1AND2" },
		{ qr_BUSHESONLAYERS1AND2, "qr_BUSHESONLAYERS1AND2" },
		{ qr_NEW_HERO_MOVEMENT, "qr_NEW_HERO_MOVEMENT" },
		{ qr_DISABLE_4WAY_GRIDLOCK, "qr_DISABLE_4WAY_GRIDLOCK" },
		{ qr_NEW_COMBO_ANIMATION, "qr_NEW_COMBO_ANIMATION" },
		{ qr_OLD_STRING_EDITOR_MARGINS, "qr_OLD_STRING_EDITOR_MARGINS" },
		{ qr_NO_OVERWRITING_HOPPING, "qr_NO_OVERWRITING_HOPPING" },
		{ qr_DUNGEON_DMAPS_PERM_SECRETS, "qr_DUNGEON_DMAPS_PERM_SECRETS" },
		{ qr_STRING_FRAME_OLD_WIDTH_HEIGHT, "qr_STRING_FRAME_OLD_WIDTH_HEIGHT" },
		{ qr_NO_SCROLL_WHILE_IN_AIR, "qr_NO_SCROLL_WHILE_IN_AIR" },
		{ qr_INSTANT_RESPAWN, "qr_INSTANT_RESPAWN" },
		{ qr_BROKEN_OVERWORLD_MINIMAP, "qr_BROKEN_OVERWORLD_MINIMAP" },
		{ qr_NO_STUNLOCK_BLOCK, "qr_NO_STUNLOCK_BLOCK" },
		{ qr_GREATER_MAX_TIME, "qr_GREATER_MAX_TIME" },
		{ qr_BROKEN_RING_POWER, "qr_BROKEN_RING_POWER" },
		{ qr_NO_OVERWORLD_MAP_CHARTING, "qr_NO_OVERWORLD_MAP_CHARTING" },
		{ qr_DUNGEONS_USE_CLASSIC_CHARTING, "qr_DUNGEONS_USE_CLASSIC_CHARTING" },
		{ qr_ARROWS_ALWAYS_PENETRATE, "qr_ARROWS_ALWAYS_PENETRATE" },
		{ qr_SWORDBEAMS_ALWAYS_PENETRATE, "qr_SWORDBEAMS_ALWAYS_PENETRATE" },
		{ qr_SET_XBUTTON_ITEMS, "qr_SET_XBUTTON_ITEMS" },
		{ qr_SMARTER_WATER, "qr_SMARTER_WATER" },
		{ qr_NO_HOPPING, "qr_NO_HOPPING" },
		{ qr_NO_SOLID_SWIM, "qr_NO_SOLID_SWIM" },
		{ qr_WATER_ON_LAYER_1, "qr_WATER_ON_LAYER_1" },
		{ qr_WATER_ON_LAYER_2, "qr_WATER_ON_LAYER_2" },
		{ qr_SHALLOW_SENSITIVE, "qr_SHALLOW_SENSITIVE" },
		{ qr_SMARTER_SMART_SCROLL, "qr_SMARTER_SMART_SCROLL" },
		{ qr_INSTANT_CONTINUE, "qr_INSTANT_CONTINUE" },
		{ qr_NONHEAVY_BLOCKTRIGGER_PERM, "qr_NONHEAVY_BLOCKTRIGGER_PERM" },
		{ qr_SET_YBUTTON_ITEMS, "qr_SET_YBUTTON_ITEMS" },
		{ qr_CORRECTED_EW_BRANG_ANIM, "qr_CORRECTED_EW_BRANG_ANIM" },
		{ qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER, "qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER" },
		{ qr_OVERHEAD_COMBOS_L1_L2, "qr_OVERHEAD_COMBOS_L1_L2" },
		{ qr_ALLOW_EDITING_COMBO_0, "qr_ALLOW_EDITING_COMBO_0" },
		{ qr_OLD_CHEST_COLLISION, "qr_OLD_CHEST_COLLISION" },
		{ qr_AUTOCOMBO_LAYER_1, "qr_AUTOCOMBO_LAYER_1" },
		{ qr_AUTOCOMBO_LAYER_2, "qr_AUTOCOMBO_LAYER_2" },
		{ qr_TRUEFIXEDBOMBSHIELD, "qr_TRUEFIXEDBOMBSHIELD" },
		{ qr_BOMBSPIERCESHIELD, "qr_BOMBSPIERCESHIELD" },
		{ qr_BROKEN_HORIZONTAL_WEAPON_ANIM, "qr_BROKEN_HORIZONTAL_WEAPON_ANIM" },
		{ qr_NEW_DARKROOM, "qr_NEW_DARKROOM" },
		{ qr_NEWDARK_L6, "qr_NEWDARK_L6" },
		{ qr_ENEMIES_SECRET_ONLY_16_31, "qr_ENEMIES_SECRET_ONLY_16_31" },
		{ qr_SCREEN80_OWN_MUSIC, "qr_SCREEN80_OWN_MUSIC" },
		{ qr_OLDCS2, "qr_OLDCS2" },
		{ qr_HARDCODED_ENEMY_ANIMS, "qr_HARDCODED_ENEMY_ANIMS" },
		{ qr_OLD_ITEMDATA_SCRIPT_TIMING, "qr_OLD_ITEMDATA_SCRIPT_TIMING" },
		{ qr_SIDESWIM, "qr_SIDESWIM" },
		{ qr_SIDESWIMDIR, "qr_SIDESWIMDIR" },
		{ qr_PUSHBLOCK_LAYER_1_2, "qr_PUSHBLOCK_LAYER_1_2" },
		{ qr_NEWDARK_SCROLLEDGE, "qr_NEWDARK_SCROLLEDGE" },
		{ qr_STEPTEMP_SECRET_ONLY_16_31, "qr_STEPTEMP_SECRET_ONLY_16_31" },
		{ qr_ALLTRIG_PERMSEC_NO_TEMP, "qr_ALLTRIG_PERMSEC_NO_TEMP" },
		{ qr_HARDCODED_LITEM_LTMS, "qr_HARDCODED_LITEM_LTMS" },
		{ qr_NO_BOTTLE_IF_ANY_COUNTER_FULL, "qr_NO_BOTTLE_IF_ANY_COUNTER_FULL" },
		{ qr_LIGHTBEAM_TRANSPARENT, "qr_LIGHTBEAM_TRANSPARENT" },
		{ qr_CANDLES_SHARED_LIMIT, "qr_CANDLES_SHARED_LIMIT" },
		{ qr_OLD_RESPAWN_POINTS, "qr_OLD_RESPAWN_POINTS" },
		{ qr_HOOKSHOTALLLAYER, "qr_HOOKSHOTALLLAYER" },
		{ qr_ANONE_NOANIM, "qr_ANONE_NOANIM" },
		{ qr_BLOCKHOLE_SAME_ONLY, "qr_BLOCKHOLE_SAME_ONLY" },
		{ qr_SWITCHOBJ_RUN_SCRIPT, "qr_SWITCHOBJ_RUN_SCRIPT" },
		{ qr_ITEMCOMBINE_NEW_PSTR, "qr_ITEMCOMBINE_NEW_PSTR" },
		{ qr_ITEMCOMBINE_CONTINUOUS, "qr_ITEMCOMBINE_CONTINUOUS" },
		{ qr_SCC_ITEM_COMBINES_ITEMS, "qr_SCC_ITEM_COMBINES_ITEMS" },
		{ qr_SCROLLING_KILLS_CHARGE, "qr_SCROLLING_KILLS_CHARGE" },
		{ qr_CUSTOMWEAPON_IGNORE_COST, "qr_CUSTOMWEAPON_IGNORE_COST" },
		{ qr_BLOCKS_DONT_LOCK_OTHER_LAYERS, "qr_BLOCKS_DONT_LOCK_OTHER_LAYERS" },
		{ qr_SCC_GOTO_RESPECTS_CONTFLAG, "qr_SCC_GOTO_RESPECTS_CONTFLAG" },
		{ qr_BROKEN_KEEPOLD_FLAG, "qr_BROKEN_KEEPOLD_FLAG" },
		{ qr_KEEPOLD_APPLIES_RETROACTIVELY, "qr_KEEPOLD_APPLIES_RETROACTIVELY" },
		{ qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST, "qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST" },
		{ qr_OLD_HALF_MAGIC, "qr_OLD_HALF_MAGIC" },
		{ qr_LEVEL_RESTART_CONT_POINT, "qr_LEVEL_RESTART_CONT_POINT" },
		{ qr_SUBSCR_OLD_SELECTOR, "qr_SUBSCR_OLD_SELECTOR" },
		{ qr_OLD_FAIRY_LIMIT, "qr_OLD_FAIRY_LIMIT" },
		{ qr_FAIRYDIR, "qr_FAIRYDIR" },
		{ qr_ARROWCLIP, "qr_ARROWCLIP" },
		{ qr_CONT_SWORD_TRIGGERS, "qr_CONT_SWORD_TRIGGERS" },
		{ qr_OLD_210_WATER, "qr_OLD_210_WATER" },
		{ qr_8WAY_SHOT_SFX_DEP, "qr_8WAY_SHOT_SFX_DEP" },
		{ qr_COPIED_SWIM_SPRITES, "qr_COPIED_SWIM_SPRITES" },
		{ qr_WRONG_BRANG_TRAIL_DIR, "qr_WRONG_BRANG_TRAIL_DIR" },
		{ qr_192b163_WARP, "qr_192b163_WARP" },
		{ qr_210_WARPRETURN, "qr_210_WARPRETURN" },
		{ qr_LESS_AWFUL_SIDESPIKES, "qr_LESS_AWFUL_SIDESPIKES" },
		{ qr_OLD_LADDER_ITEM_SIDEVIEW, "qr_OLD_LADDER_ITEM_SIDEVIEW" },
		{ qr_OLD_SECRETMONEY, "qr_OLD_SECRETMONEY" },
		{ qr_OLD_DOORREPAIR, "qr_OLD_DOORREPAIR" },
		{ qr_OLD_POTION_OR_HC, "qr_OLD_POTION_OR_HC" },
		{ qr_NO_LANMOLA_RINGLEADER, "qr_NO_LANMOLA_RINGLEADER" },
		{ qr_HARDCODED_BS_PATRA, "qr_HARDCODED_BS_PATRA" },
		{ qr_PATRAS_USE_HARDCODED_OFFSETS, "qr_PATRAS_USE_HARDCODED_OFFSETS" },
		{ qr_BROKEN_BIG_ENEMY_ANIMATION, "qr_BROKEN_BIG_ENEMY_ANIMATION" },
		{ qr_BROKEN_ATTRIBUTE_31_32, "qr_BROKEN_ATTRIBUTE_31_32" },
		{ qr_CAN_PLACE_GANON, "qr_CAN_PLACE_GANON" },
		{ qr_CAN_PLACE_TRAPS, "qr_CAN_PLACE_TRAPS" },
		{ qr_FIND_CLOSEST_BAIT, "qr_FIND_CLOSEST_BAIT" },
		{ qr_OLD_FLAMETRAIL_DURATION, "qr_OLD_FLAMETRAIL_DURATION" },
		{ qr_PERMANENT_WHISTLE_SECRETS, "qr_PERMANENT_WHISTLE_SECRETS" },
		{ qr_SMARTER_DROPS, "qr_SMARTER_DROPS" },
		{ qr_WAVY_NO_EPILEPSY, "qr_WAVY_NO_EPILEPSY" },
		{ qr_WAVY_NO_EPILEPSY_2, "qr_WAVY_NO_EPILEPSY_2" },
		{ qr_DYING_ENEMIES_IGNORE_STUN, "qr_DYING_ENEMIES_IGNORE_STUN" },
		{ qr_SHOP_ITEMS_VANISH, "qr_SHOP_ITEMS_VANISH" },
		{ qr_ANGULAR_REFLECT_BROKEN, "qr_ANGULAR_REFLECT_BROKEN" },
		{ qr_CSET1_LEVEL, "qr_CSET1_LEVEL" },
		{ qr_CSET5_LEVEL, "qr_CSET5_LEVEL" },
		{ qr_CSET7_LEVEL, "qr_CSET7_LEVEL" },
		{ qr_CSET8_LEVEL, "qr_CSET8_LEVEL" },
		{ qr_FADECS1, "qr_FADECS1" },
		{ qr_FADECS7, "qr_FADECS7" },
		{ qr_FADECS8, "qr_FADECS8" },
		{ qr_FADECS9, "qr_FADECS9" },
		{ qr_UNBEATABLES_DONT_KEEP_DEAD, "qr_UNBEATABLES_DONT_KEEP_DEAD" },
		{ qr_NO_LEAVE_ONE_ENEMY_ALIVE_TRICK, "qr_NO_LEAVE_ONE_ENEMY_ALIVE_TRICK" },
		{ qr_OLD_BRIDGE_COMBOS, "qr_OLD_BRIDGE_COMBOS" },
		{ qr_BROKEN_Z3_ANIMATION, "qr_BROKEN_Z3_ANIMATION" },
		{ qr_OLD_TILE_INITIALIZATION, "qr_OLD_TILE_INITIALIZATION" },
		{ qr_FLUCTUATING_ENEMY_JUMP, "qr_FLUCTUATING_ENEMY_JUMP" },
		{ qr_SPRITE_JUMP_IS_TRUNCATED, "qr_SPRITE_JUMP_IS_TRUNCATED" },
		{ qr_BUGGY_BUGGY_SLASH_TRIGGERS, "qr_BUGGY_BUGGY_SLASH_TRIGGERS" },
		{ qr_OLD_DRAWOFFSET, "qr_OLD_DRAWOFFSET" },
		{ qr_BROKEN_DRAWSCREEN_FUNCTIONS, "qr_BROKEN_DRAWSCREEN_FUNCTIONS" },
		{ qr_ENEMY_DROPS_USE_HITOFFSETS, "qr_ENEMY_DROPS_USE_HITOFFSETS" },
		{ qr_BROKEN_ITEM_CARRYING, "qr_BROKEN_ITEM_CARRYING" },
		{ qr_LEEVERS_DONT_OBEY_STUN, "qr_LEEVERS_DONT_OBEY_STUN" },
		{ qr_QUAKE_STUNS_LEEVERS, "qr_QUAKE_STUNS_LEEVERS" },
		{ qr_GANON_CANT_SPAWN_ON_CONTINUE, "qr_GANON_CANT_SPAWN_ON_CONTINUE" },
		{ qr_WIZZROBES_DONT_OBEY_STUN, "qr_WIZZROBES_DONT_OBEY_STUN" },
		{ qr_OLD_BUG_NET, "qr_OLD_BUG_NET" },
		{ qr_MANHANDLA_BLOCK_SFX, "qr_MANHANDLA_BLOCK_SFX" },
		{ qr_GRASS_SENSITIVE, "qr_GRASS_SENSITIVE" },
		{ qr_BETTER_RAFT, "qr_BETTER_RAFT" },
		{ qr_BETTER_RAFT_2, "qr_BETTER_RAFT_2" },
		{ qr_RAFT_SOUND, "qr_RAFT_SOUND" },
		{ qr_WARPS_RESTART_DMAPSCRIPT, "qr_WARPS_RESTART_DMAPSCRIPT" },
		{ qr_DMAP_0_CONTINUE_BUG, "qr_DMAP_0_CONTINUE_BUG" },
		{ qr_SCRIPT_WARPS_DMAP_SCRIPT_TOGGLE, "qr_SCRIPT_WARPS_DMAP_SCRIPT_TOGGLE" },
		{ qr_OLD_SCRIPTED_KNOCKBACK, "qr_OLD_SCRIPTED_KNOCKBACK" },
		{ qr_OLD_KEESE_Z_AXIS, "qr_OLD_KEESE_Z_AXIS" },
		{ qr_POLVIRE_NO_SHADOW, "qr_POLVIRE_NO_SHADOW" },
		{ qr_CONVEYORS_L1_L2, "qr_CONVEYORS_L1_L2" },
		{ qr_CUSTOMCOMBOS_EVERY_LAYER, "qr_CUSTOMCOMBOS_EVERY_LAYER" },
		{ qr_SUBSCR_BACKWARDS_ID_ORDER, "qr_SUBSCR_BACKWARDS_ID_ORDER" },
		{ qr_FASTCOUNTERDRAIN, "qr_FASTCOUNTERDRAIN" },
		{ qr_OLD_LOCKBLOCK_COLLISION, "qr_OLD_LOCKBLOCK_COLLISION" },
		{ qr_DECO_2_YOFFSET, "qr_DECO_2_YOFFSET" },
		{ qr_SCREENSTATE_80s_BUG, "qr_SCREENSTATE_80s_BUG" },
		{ qr_AUTOCOMBO_ANY_LAYER, "qr_AUTOCOMBO_ANY_LAYER" },
		{ qr_GOHMA_UNDAMAGED_BUG, "qr_GOHMA_UNDAMAGED_BUG" },
		{ qr_FFCPRELOAD_BUGGED_LOAD, "qr_FFCPRELOAD_BUGGED_LOAD" },
		{ qr_SWITCHES_AFFECT_MOVINGBLOCKS, "qr_SWITCHES_AFFECT_MOVINGBLOCKS" },
		{ qr_BROKEN_GETPIXEL_VALUE, "qr_BROKEN_GETPIXEL_VALUE" },
		{ qr_NO_LIFT_SPRITE, "qr_NO_LIFT_SPRITE" },
		{ qr_OLD_SIDEVIEW_LANDING_CODE, "qr_OLD_SIDEVIEW_LANDING_CODE" },
		{ qr_OLD_FFC_SPEED_CAP, "qr_OLD_FFC_SPEED_CAP" },
		{ qr_OLD_WIZZROBE_SUBMERGING, "qr_OLD_WIZZROBE_SUBMERGING" },
		{ qr_SPARKLES_INHERIT_PROPERTIES, "qr_SPARKLES_INHERIT_PROPERTIES" },
		{ qr_BROKENHITBY, "qr_BROKENHITBY" },
		{ qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME, "qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME" },
		{ qr_OLD_CONVEYOR_COLLISION, "qr_OLD_CONVEYOR_COLLISION" },
		{ qr_SENSITIVE_SOLID_DAMAGE, "qr_SENSITIVE_SOLID_DAMAGE" },
		{ qr_LENIENT_SOLID_DAMAGE, "qr_LENIENT_SOLID_DAMAGE" },
		{ qr_NO_SIDEVIEW_SOLID_DAMAGE, "qr_NO_SIDEVIEW_SOLID_DAMAGE" },
		{ qr_OLD_FFC_FUNCTIONALITY, "qr_OLD_FFC_FUNCTIONALITY" },
		{ qr_OLD_SHALLOW_SFX, "qr_OLD_SHALLOW_SFX" },
		{ qr_BUGGED_LAYERED_FLAGS, "qr_BUGGED_LAYERED_FLAGS" },
		{ qr_HARDCODED_FFC_BUSH_DROPS, "qr_HARDCODED_FFC_BUSH_DROPS" },
		{ qr_POUNDLAYERS1AND2, "qr_POUNDLAYERS1AND2" },
		{ qr_MOVINGBLOCK_FAKE_SOLID, "qr_MOVINGBLOCK_FAKE_SOLID" },
		{ qr_NEW_HERO_MOVEMENT2, "qr_NEW_HERO_MOVEMENT2" },
		{ qr_CARRYABLE_NO_ACROSS_SCREEN, "qr_CARRYABLE_NO_ACROSS_SCREEN" },
		{ qr_NO_SCROLL_WHILE_CARRYING, "qr_NO_SCROLL_WHILE_CARRYING" },
		{ qr_HELD_BOMBS_EXPLODE, "qr_HELD_BOMBS_EXPLODE" },
		{ qr_BROKEN_MOVING_BOMBS, "qr_BROKEN_MOVING_BOMBS" },
		{ qr_OLD_BOMB_HITBOXES, "qr_OLD_BOMB_HITBOXES" },
		{ qr_SCROLLWARP_NO_RESET_FRAME, "qr_SCROLLWARP_NO_RESET_FRAME" },
		{ qr_BROKEN_RAFT_SCROLL, "qr_BROKEN_RAFT_SCROLL" },
		{ qr_BROKEN_INPUT_DOWN_STATE, "qr_BROKEN_INPUT_DOWN_STATE" },
		{ qr_OLD_GUY_HANDLING, "qr_OLD_GUY_HANDLING" },
		{ qr_FREEFORM_SUBSCREEN_CURSOR, "qr_FREEFORM_SUBSCREEN_CURSOR" },
		{ qr_SUBSCR_PRESS_TO_EQUIP, "qr_SUBSCR_PRESS_TO_EQUIP" },
		{ qr_FAIRY_FLAG_COMPAT, "qr_FAIRY_FLAG_COMPAT" },
		{ qr_MIRROR_PRISM_LAYERS, "qr_MIRROR_PRISM_LAYERS" },
		{ qr_OLD_LENS_LAYEREFFECT, "qr_OLD_LENS_LAYEREFFECT" },
		{ qr_PUSHBLOCK_SPRITE_LAYER, "qr_PUSHBLOCK_SPRITE_LAYER" },
		{ qr_OLD_SUBSCR, "qr_OLD_SUBSCR" },
		{ qr_ITM_0_INVIS_ON_BTNS, "qr_ITM_0_INVIS_ON_BTNS" },
		{ qr_NO_BUTTON_VERIFY, "qr_NO_BUTTON_VERIFY" },
		{ qr_OLD_GAUGE_TILE_LAYOUT, "qr_OLD_GAUGE_TILE_LAYOUT" },
		{ qr_WALKTHROUGHWALL_NO_DOORSTATE, "qr_WALKTHROUGHWALL_NO_DOORSTATE" },
		{ qr_SPOTLIGHT_IGNR_SOLIDOBJ, "qr_SPOTLIGHT_IGNR_SOLIDOBJ" },
		{ qr_BROKEN_LIGHTBEAM_HITBOX, "qr_BROKEN_LIGHTBEAM_HITBOX" },
		{ qr_BROKEN_SWORD_SPIN_TRIGGERS, "qr_BROKEN_SWORD_SPIN_TRIGGERS" },
		{ qr_SCRIPT_CONTHP_IS_HEARTS, "qr_SCRIPT_CONTHP_IS_HEARTS" },
		{ qr_MULTI_PLATFORM_FFC, "qr_MULTI_PLATFORM_FFC" },
		{ qr_SEPARATE_BOMBABLE_TAPPING_SFX, "qr_SEPARATE_BOMBABLE_TAPPING_SFX" },
		{ qr_BROKEN_BOMB_AMMO_COSTS, "qr_BROKEN_BOMB_AMMO_COSTS" },
		{ qr_CONVEYORS_ALL_LAYERS, "qr_CONVEYORS_ALL_LAYERS" },
		{ qr_OLD_BROKEN_WARPEX_MUSIC, "qr_OLD_BROKEN_WARPEX_MUSIC" },
		{ qr_BROKEN_LIFTSWIM, "qr_BROKEN_LIFTSWIM" },
		{ qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING, "qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING" },
		{ qr_BROKEN_FLAME_ARROW_REFLECTING, "qr_BROKEN_FLAME_ARROW_REFLECTING" },
		{ qr_EW_FIRE_EMITS_LIGHT, "qr_EW_FIRE_EMITS_LIGHT" },
		{ qr_NEWDARK_TRANS_STACKING, "qr_NEWDARK_TRANS_STACKING" },
		{ qr_OLD_HERO_WARP_RETSQUARE, "qr_OLD_HERO_WARP_RETSQUARE" },
		{ qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS, "qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS" },
		{ qr_BROKEN_PUSHBLOCK_FLAG_CLONING, "qr_BROKEN_PUSHBLOCK_FLAG_CLONING" },
		{ qr_OLD_LANDING_SFX, "qr_OLD_LANDING_SFX" },
		{ qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS, "qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS" },
		{ qr_BLOCKS_DROWN, "qr_BLOCKS_DROWN" },
		{ qr_LAYER6_STRINGS_OVER_SUBSCREEN, "qr_LAYER6_STRINGS_OVER_SUBSCREEN" },
		{ qr_PUSHBLOCKS_FALL_IN_SIDEVIEW, "qr_PUSHBLOCKS_FALL_IN_SIDEVIEW" },
		{ qr_BROKEN_ICY_FLOOR_SIDEVIEW, "qr_BROKEN_ICY_FLOOR_SIDEVIEW" },
		{ qr_BROKEN_SYSTEM_COLORS, "qr_BROKEN_SYSTEM_COLORS" },
		{ qr_WEAPONS_EXTRA_SPAWN_FRAME, "qr_WEAPONS_EXTRA_SPAWN_FRAME" },
		{ qr_BROKEN_SCROLL_INSTEAD_OF_DROWN_FALL, "qr_BROKEN_SCROLL_INSTEAD_OF_DROWN_FALL" },
		{ qr_LIFTED_WEAPONS_RUN_SCRIPTS, "qr_LIFTED_WEAPONS_RUN_SCRIPTS" },
		{ qr_BOMB_BOOMS_CLEAR_SCRIPTS, "qr_BOMB_BOOMS_CLEAR_SCRIPTS" },
		{ qr_BETTER_ENGINE_AUTOROTATE, "qr_BETTER_ENGINE_AUTOROTATE" },
		{ qr_EW_ROCKS_HARDCODED_BREAK_ON_SOLID, "qr_EW_ROCKS_HARDCODED_BREAK_ON_SOLID" },
		{ qr_IMPRECISE_WEAPON_SOLIDITY_CHECKS, "qr_IMPRECISE_WEAPON_SOLIDITY_CHECKS" },
		{ qr_NO_STUNLOCK_IGNORE, "qr_NO_STUNLOCK_IGNORE" },
		{ qr_BROKEN_BLOCKHOLE_PITFALLS, "qr_BROKEN_BLOCKHOLE_PITFALLS" },
		{ qr_CUSTOM_WEAPON_BROKEN_SIZE, "qr_CUSTOM_WEAPON_BROKEN_SIZE" },
		{ qr_OLD_WEAPON_REFLECTION, "qr_OLD_WEAPON_REFLECTION" },
		{ qr_OLD_SPRITE_FALL_DROWN, "qr_OLD_SPRITE_FALL_DROWN" },
		{ qr_OLD_TERMINAL_VELOCITY, "qr_OLD_TERMINAL_VELOCITY" },
		{ qr_BETTER_PLAYER_JUMP_ANIM, "qr_BETTER_PLAYER_JUMP_ANIM" },
		{ qr_ARMOS_GRAVE_ON_LAYERS, "qr_ARMOS_GRAVE_ON_LAYERS" },
		{ qr_OLD_SCRIPT_LEVEL_GLOBAL_STATES, "qr_OLD_SCRIPT_LEVEL_GLOBAL_STATES" },
		{ qr_BROKEN_ARMOS_GRAVE_BIGHITBOX_COLLISION, "qr_BROKEN_ARMOS_GRAVE_BIGHITBOX_COLLISION" },
		{ qr_HERO_DIVE_UNDER_LAYER_1, "qr_HERO_DIVE_UNDER_LAYER_1" },
		{ qr_CLASSIC_DRAWING_ORDER, "qr_CLASSIC_DRAWING_ORDER" },
		{ qr_YSORT_SPRITES, "qr_YSORT_SPRITES" },
		{ qr_OLD_WEAPON_DRAW_ANIMATE_TIMING, "qr_OLD_WEAPON_DRAW_ANIMATE_TIMING" },
		{ qr_SCRIPTS_6_BIT_COLOR, "qr_SCRIPTS_6_BIT_COLOR" },
		{ qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION, "qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION" },
		{ qr_HIDE_BOTTOM_8_PIXELS, "qr_HIDE_BOTTOM_8_PIXELS" },
		{ qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN, "qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN" },
		{ qr_INVERTED_DARK_COMBO_TRIGGERS, "qr_INVERTED_DARK_COMBO_TRIGGERS" },
		{ qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET, "qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET" },
		{ qr_OLD_SCRIPTS_INTERNAL_ARRAYS_BOUND_INDEX, "qr_OLD_SCRIPTS_INTERNAL_ARRAYS_BOUND_INDEX" },
		{ qr_OLD_SCRIPTS_ARRAYS_NON_ZERO_DEFAULT_VALUE, "qr_OLD_SCRIPTS_ARRAYS_NON_ZERO_DEFAULT_VALUE" },
		{ qr_ROPE_ENEMIES_SPEED_NOT_CONFIGURABLE, "qr_ROPE_ENEMIES_SPEED_NOT_CONFIGURABLE" },
		{ qr_OLD_SCRIPT_VOLUME, "qr_OLD_SCRIPT_VOLUME" },
		{ qr_OLD_DMAP_INTRO_STRINGS, "qr_OLD_DMAP_INTRO_STRINGS" },
		{ qr_SCRIPTDRAWSFROZENMSG, "qr_SCRIPTDRAWSFROZENMSG" },
		{ qr_BROKEN_SIDEVIEW_SPRITE_JUMP, "qr_BROKEN_SIDEVIEW_SPRITE_JUMP" },
		{ qr_PARSER_250DIVISION, "qr_PARSER_250DIVISION" },
		{ qr_PARSER_NO_LOGGING, "qr_PARSER_NO_LOGGING" },
		{ qr_PARSER_SHORT_CIRCUIT, "qr_PARSER_SHORT_CIRCUIT" },
		{ qr_PARSER_BOOL_TRUE_DECIMAL, "qr_PARSER_BOOL_TRUE_DECIMAL" },
		{ qr_SPRITEXY_IS_FLOAT, "qr_SPRITEXY_IS_FLOAT" },
		{ qr_PARSER_TRUE_INT_SIZE, "qr_PARSER_TRUE_INT_SIZE" },
		{ qr_WPNANIMFIX, "qr_WPNANIMFIX" },
		{ qr_NOSCRIPTSDURINGSCROLL, "qr_NOSCRIPTSDURINGSCROLL" },
		{ qr_OLDSPRITEDRAWS, "qr_OLDSPRITEDRAWS" },
		{ qr_WEAPONSHADOWS, "qr_WEAPONSHADOWS" },
		{ qr_ITEMSHADOWS, "qr_ITEMSHADOWS" },
		{ qr_OLDEWPNPARENT, "qr_OLDEWPNPARENT" },
		{ qr_OLDCREATEBITMAP_ARGS, "qr_OLDCREATEBITMAP_ARGS" },
		{ qr_OLDQUESTMISC, "qr_OLDQUESTMISC" },
		{ qr_PARSER_FORCE_INLINE, "qr_PARSER_FORCE_INLINE" },
		{ qr_CLEARINITDONSCRIPTCHANGE, "qr_CLEARINITDONSCRIPTCHANGE" },
		{ qr_NOFFCWAITDRAW, "qr_NOFFCWAITDRAW" },
		{ qr_NOITEMWAITDRAW, "qr_NOITEMWAITDRAW" },
		{ qr_TRACESCRIPTIDS, "qr_TRACESCRIPTIDS" },
		{ qr_FIXDRUNKINPUTS, "qr_FIXDRUNKINPUTS" },
		{ qr_PARSER_BINARY_32BIT, "qr_PARSER_BINARY_32BIT" },
		{ qr_ALWAYS_DEALLOCATE_ARRAYS, "qr_ALWAYS_DEALLOCATE_ARRAYS" },
		{ qr_ONDEATH_RUNS_AFTER_DEATH_ANIM, "qr_ONDEATH_RUNS_AFTER_DEATH_ANIM" },
		{ qr_DISALLOW_SETTING_RAFTING, "qr_DISALLOW_SETTING_RAFTING" },
		{ qr_WEAPONS_EXTRA_DEATH_FRAME, "qr_WEAPONS_EXTRA_DEATH_FRAME" },
		{ qr_250WRITEEDEFSCRIPT, "qr_250WRITEEDEFSCRIPT" },
		{ qr_SETENEMYWEAPONSPRITESONWPNCHANGE, "qr_SETENEMYWEAPONSPRITESONWPNCHANGE" },
		{ qr_BROKENCHARINTDRAWING, "qr_BROKENCHARINTDRAWING" },
		{ qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES, "qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES" },
		{ qr_COMBOSCRIPTS_LAYER_0, "qr_COMBOSCRIPTS_LAYER_0" },
		{ qr_COMBOSCRIPTS_LAYER_1, "qr_COMBOSCRIPTS_LAYER_1" },
		{ qr_COMBOSCRIPTS_LAYER_2, "qr_COMBOSCRIPTS_LAYER_2" },
		{ qr_COMBOSCRIPTS_LAYER_3, "qr_COMBOSCRIPTS_LAYER_3" },
		{ qr_COMBOSCRIPTS_LAYER_4, "qr_COMBOSCRIPTS_LAYER_4" },
		{ qr_COMBOSCRIPTS_LAYER_5, "qr_COMBOSCRIPTS_LAYER_5" },
		{ qr_COMBOSCRIPTS_LAYER_6, "qr_COMBOSCRIPTS_LAYER_6" },
		{ qr_OLD_INIT_SCRIPT_TIMING, "qr_OLD_INIT_SCRIPT_TIMING" },
		{ qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS, "qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS" },
		{ qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE, "qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE" },
		{ qr_PARSER_STRINGSWITCH_INSENSITIVE, "qr_PARSER_STRINGSWITCH_INSENSITIVE" },
		{ qr_OLD_SLASHNEXT_SECRETS, "qr_OLD_SLASHNEXT_SECRETS" },
		{ qr_STEP_IS_FLOAT, "qr_STEP_IS_FLOAT" },
		{ qr_OLDMAPDATAFLAGS, "qr_OLDMAPDATAFLAGS" },
		{ qr_OLD_PRINTF_ARGS, "qr_OLD_PRINTF_ARGS" },
		{ qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN, "qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN" },
		{ qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS, "qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS" },
		{ qr_LOG_INVALID_UID_LOAD, "qr_LOG_INVALID_UID_LOAD" },
		{ qr_COMBODATA_INITD_MULT_TENK, "qr_COMBODATA_INITD_MULT_TENK" },
		{ qr_ZS_NO_NEG_ARRAY, "qr_ZS_NO_NEG_ARRAY" },
		{ qr_PRINTF_NO_0FILL, "qr_PRINTF_NO_0FILL" },
		{ qr_SHIELDBUBBLE, "qr_SHIELDBUBBLE" },
		{ qr_SETENEMYWEAPONSOUNDSONWPNCHANGE, "qr_SETENEMYWEAPONSOUNDSONWPNCHANGE" },
		{ qr_BROKEN_CONVEYORS, "qr_BROKEN_CONVEYORS" },
		{ qr_ZS_OLD_SUSPEND_FFC, "qr_ZS_OLD_SUSPEND_FFC" },
		{ qr_OLD_PIT_SENSITIVITY, "qr_OLD_PIT_SENSITIVITY" },
		{ qr_ACTIVE_SHIELD_PASSIVE_ROC_NO_SCRIPT, "qr_ACTIVE_SHIELD_PASSIVE_ROC_NO_SCRIPT" },
		{ qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING, "qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING" },
		{ qr_MAP_BUTTON_CLOSES_SUBSCREEN, "qr_MAP_BUTTON_CLOSES_SUBSCREEN" },
		{ qr_ONLY_MARK_SCREENS_VISITED_IF_MAP_VIEWABLE, "qr_ONLY_MARK_SCREENS_VISITED_IF_MAP_VIEWABLE" },
		{ qr_ACTIVE_SUB_IGNORE_8PX, "qr_ACTIVE_SUB_IGNORE_8PX" },
		{ qr_MIRROR_PRISM_FFCS, "qr_MIRROR_PRISM_FFCS" },
		{ qr_WEAPON_BETTER_SCREEN_EDGE_HITBOX, "qr_WEAPON_BETTER_SCREEN_EDGE_HITBOX" },
		{ qr_BROKEN_COPYCAT_SELF_TRIGGER, "qr_BROKEN_COPYCAT_SELF_TRIGGER" },
		{ qr_BROKEN_WATER_PASSIVE_DAMAGE, "qr_BROKEN_WATER_PASSIVE_DAMAGE" },
		{ qr_BROKEN_SELF_TRIGGERING_TRIGGERS, "qr_BROKEN_SELF_TRIGGERING_TRIGGERS" },
	};
	static std::map<std::string, std::string> qr_compat_names = {
		{ "qr_WEAPONS_EXTRA_FRAME", qr_const_names[qr_WEAPONS_EXTRA_DEATH_FRAME]}
	};
	GUI::ListData qrs = combinedQRList() + combinedZSRList();
	qrs.valsort();
	
	std::ostringstream missing_qrs;
	qrs.filter([&](GUI::ListItem& ref)
		{
			if (!qr_const_names.contains(ref.value))
				missing_qrs << ref.value << " [" << ref.text << "]\n";
			return true;
		});
	
	auto missing_str = missing_qrs.str();
	if (!missing_str.empty())
	{
		missing_str = missing_str.substr(0, missing_str.size() - 2); // trailing comma
		printf("error: missing qr constants!\n%s\n", missing_str.c_str());
		zq_exit(1);
	}

	std::ofstream file(fname);
	if (file.fail())
	{
		printf("error outputting file '%s'\n", fname.c_str());
		zq_exit(1);
	}
	
	file << "// Quest rules. Used to toggle legacy behavior and other quest-wide options.\n";
	file << "enum QR\n{\n";
	bool did_skip = false, first = true;
	const string indent = "\t";
	for (int q = 0; q < qr_MAX; ++q)
	{
		if (!qr_const_names.contains(q))
		{
			did_skip = true;
			continue; // unused
		}
		string info, name;
		if (qrs.hasKey(q))
		{
			auto const& ref = qrs.accessItem(q);
			info = ref.info;
			name = fmt::format("'{}'", ref.text);
		}
		else info = "@deprecated";
		
		vector<string> info_lines = wrap_qr_info(info);
		
		if (!name.empty())
		{
			if (info_lines.empty())
				info_lines.push_back(name);
			else
			{
				info_lines.insert(info_lines.begin(), "");
				info_lines.insert(info_lines.begin(), name);
			}
		}
		
		bool multiline_info = info_lines.size() > 1;
		
		if (multiline_info)
		{
			if (!first)
				file << "\n";
			for (string const& line : info_lines)
			{
				if (line.empty())
					file << indent << "//\n";
				else
					file << indent << "// " << line << "\n";
			}
		}
		file << indent << qr_const_names[q];
		if (did_skip)
		{
			file << " = ";
			if (!(q % 8))
				file << (q/8) << " * 8";
			else file << q;
		}
		file << ",";
		
		if (multiline_info || info_lines.empty())
			file << "\n";
		else file << " // " << info_lines[0] << "\n";
		
		did_skip = false;
		first = false;
	}
	file << indent << "qr_MAX";
	if (did_skip)
		file << " = " << qr_MAX;
	file << "\n};\n\n";
	
	file << "// Old names for old script compat\n";
	for(auto [old_name, new_name] : qr_compat_names)
		file << "const QR " << old_name << " = " << new_name << ";\n";
	file << "\n";

	file.close();

	zq_exit(0);
}

static void do_compile_command(std::string path, int mode)
{
	is_zq_replay_test = true;
	set_headless_mode();

	int load_ret = load_quest(path.c_str(), false);
	bool success = load_ret == qe_OK;
	if (!success)
	{
		printf("Failed to load quest: %d\n", load_ret);
		zq_exit(1);
	}

	success = do_compile_and_slots(mode, false);
	if (!success)
	{
		printf("Failed to compile\n");
		zq_exit(1);
	}

	success = save_quest(path.c_str(), false) == 0;
	if (!success)
	{
		printf("Failed to save quest\n");
		zq_exit(1);
	}

	zq_exit(0);
}

static bool partial_load_test(std::string test_dir)
{
	auto classic_path = fs::path(test_dir) / "replays/classic_1st/classic_1st.qst";
	int ret = load_quest(classic_path.string().c_str(), false);
	if (ret)
	{
		printf("failed to load classic_1st.qst: ret == %d\n", ret);
		return false;
	}

	int cont = DMaps[0].cont;

	// Skip same stuff as used in zq_tiles.cpp for grabbing tiles from a qst.
	byte skip_flags[4];
	for (int i=0; i<skip_max; ++i)
		set_bit(skip_flags,i,1);
	set_bit(skip_flags,skip_tiles,0);
	set_bit(skip_flags,skip_header,0);
	zquestheader tempheader{};
	auto ptux_path = fs::path(test_dir) / "quests/PTUX.qst";
	ret = loadquest(ptux_path.string().c_str(), &tempheader, &QMisc, customtunes, false, skip_flags);

	if (ret)
	{
		printf("failed to load PTUX.qst: ret == %d\n", ret);
		return false;
	}

	if (DMaps[0].cont != cont)
	{
		printf("unexpected modification: DMaps[0].cont == %d, should be %d\n", DMaps[0].cont, cont);
		return false;
	}

	// TODO should run replay. Currently, resaving classic_1st.qst fails its replay (see test_save in test_zeditor.py)

	return true;
}

void zeditor_handle_commands()
{
	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	if (test_zc_arg)
	{
		std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

		set_headless_mode();

		// TODO: convert to TestRunner.

		bool success = true;
		if (!partial_load_test(test_dir))
		{
			success = false;
			printf("partial_load_test failed\n");
		}
		if (success)
			printf("all tests passed\n");
		zq_exit(success ? 0 : 1);
	}

	int dev_qrs_arg = zapp_check_switch("-dev-qrs-zscript", {"filename"});
	if (dev_qrs_arg)
	{
		std::string fname = zapp_get_arg_string(dev_qrs_arg + 1);
		do_dev_qrs_zscript_command(fname);
	}

	int package_arg = zapp_check_switch("-package", {"qst", "package name"});
	if (package_arg)
	{
		std::string input_filename = zapp_get_arg_string(package_arg + 1);
		std::string package_name = zapp_get_arg_string(package_arg + 2);
		if (auto error = package_create(input_filename, package_name))
			Z_error_fatal("%s\n", error->c_str());
		zq_exit(0);
	}

	int copy_qst_arg = zapp_check_switch("-copy-qst", {"input", "output"});
	if (copy_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(copy_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(copy_qst_arg + 2);
		do_copy_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int unencrypt_qst_arg = zapp_check_switch("-unencrypt-qst", {"input", "output"});
	if (unencrypt_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(unencrypt_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(unencrypt_qst_arg + 2);
		do_unencrypt_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int uncompress_qst_arg = zapp_check_switch("-uncompress-qst", {"input", "output"});
	if (uncompress_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(uncompress_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(uncompress_qst_arg + 2);
		do_uncompress_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int quick_assign_arg = zapp_check_switch("-quick-assign", {"qst"});
	if (quick_assign_arg > 0)
	{
		std::string path = zapp_get_arg_string(quick_assign_arg + 1);
		do_compile_command(path, 1);
	}

	int smart_assign_arg = zapp_check_switch("-smart-assign", {"qst"});
	if (smart_assign_arg > 0)
	{
		std::string path = zapp_get_arg_string(smart_assign_arg + 1);
		do_compile_command(path, 2);
	}

	int export_strings_arg = zapp_check_switch("-export-strings", {"input.qst", "output.tsv"});
	if (export_strings_arg > 0)
	{
		is_zq_replay_test = true;
		set_headless_mode();

		std::string input_filename = zapp_get_arg_string(export_strings_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_strings_arg + 2);

		int load_ret = load_quest(input_filename.c_str(), false);
		bool success = load_ret == qe_OK;
		if (!success)
		{
			printf("Failed to load quest: %d\n", load_ret);
			zq_exit(1);
		}

		success = save_strings_tsv(output_filename.c_str());
		if (!success)
		{
			printf("Failed to export strings\n");
			zq_exit(1);
		}

		zq_exit(0);
	}
}
