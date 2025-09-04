#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <system_error>

#include "allegro/gui.h"
#include "base/files.h"
#include "base/misctypes.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "base/packfile.h"
#include "base/gui.h"
#include "base/util.h"
#include "zq/zq_files.h"
#include "base/zdefs.h"
#include "dialog/alertfunc.h"
#include "dialog/tilesetwizard.h"
#include "dialog/pickruleset.h"
#include "dialog/pickruletemplate.h"
#include "zq/zq_misc.h"
#include "zq/zquest.h"
#include "base/qst.h"
#include "base/zsys.h"
#include "zq/zq_class.h"
#include "tiles.h"
#include "zq/zq_tiles.h"
#include "zq/zq_custom.h"
#include "dialog/info.h"
#include "dialog/compilezscript.h"

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

#ifdef _MSC_VER
#define getcwd _getcwd
#define stricmp _stricmp
#define strupr _strupr
#endif

int32_t NewQuestFile(std::string tileset_path)
{
    memset(filepath,0,255);
    memset(temppath,0,255);
    first_save=false;
    box_start(1, "Initializing Quest", get_zc_font(font_lfont), get_zc_font(font_pfont), false);
    box_out("Please wait.");
    box_eol();
    box_out("This may take a few moments.");
    box_eol();
    init_quest(tileset_path);
    saved=true;
    box_end(false);
    refresh(rALL);
    return D_O_K;
}

static int32_t ruleset=0;
int32_t d_ruleset_radio_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_rulesettext_proc(int32_t msg, DIALOG *d, int32_t c);

static DIALOG ruleset_dlg[] =
{
    { jwin_win_proc,           0,     0,  230,   180,  vc(14),              vc(1),                 0,       D_EXIT,     0,             0, (void *) "New Quest", NULL, NULL },
    { jwin_button_proc,       40,   153,   61,    21,  vc(14),              vc(1),                13,       D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_frame_proc,   102,   80-2-7,   128,  43,   0,       0,      0,       0,             FR_ETCHED,       0,       NULL, NULL, NULL },
    
    { d_dummy_proc,			    20,    71,   61,    9,  vc(14),              vc(1),                 0,       0,     0,             0,       0, NULL, NULL },
    { d_ruleset_radio_proc,       20,    91-8,   61,    9,  vc(14),              vc(1),                 0,       0,     0,             0, (void *) "Authentic NES (8-bit)", NULL, NULL },
    { d_ruleset_radio_proc,       20,    101-8,   61,    9,  vc(14),              vc(1),                 0,       0,     0,             0, (void *) "Fixed NES (8-bit)", NULL, NULL },
    { d_ruleset_radio_proc,       20,   111-8,   61,    9,  vc(14),              vc(1),                 0,       0,     0,             0, (void *) "SNES (BS/16-bit)", NULL, NULL },
    { d_ruleset_radio_proc,       20,   121-8,   61,    9,  vc(14),              vc(1),                 0,       0,     0,             0, (void *) "SNES (Enhanced)", NULL, NULL },
    { d_rulesettext_proc,      108,   85,   0,    0,  vc(14),              vc(1),                 0,       0,     0,             0,       NULL, NULL, NULL },
    // 9
    { jwin_text_proc,       16,   24,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Please specify the ruleset template for your new quest:", NULL, NULL },
    { jwin_text_proc,       16,   34,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "These rules affect select sets of engine features that", NULL, NULL },
    { jwin_text_proc,       16,   44,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "are enabled by default, that you may later toggle on/off,", NULL, NULL },
    { jwin_text_proc,       16,   54,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "based on the mechanics that you wish to use in your game.", NULL, NULL },
    { jwin_text_proc,       16,   64,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) " ", NULL, NULL },
    // 14
    { jwin_text_proc,       16,   28,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "You have embarked on a new quest!", NULL, NULL },
    { jwin_text_proc,       16,   38,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Please select an initial ruleset template:", NULL, NULL },
    { jwin_text_proc,       16,   48,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "These settings affect the features of", NULL, NULL },
    { jwin_text_proc,       16,   58,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "the game engine that are enabled.", NULL, NULL },
    
    { jwin_text_proc,       8,  130,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "After creation, you can toggle individual Rules from the menu", NULL, NULL },
    { jwin_text_proc,       8,  140,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "options: 'Quest >> Options' and 'ZScript >> Quest Specific Settings'.", NULL, NULL },
    // There's no d_timer_proc; don't be silly.
    { d_ruleset_radio_proc,       20,   81-8,   61,    9,  vc(14),              vc(1),                 0,       D_SELECTED,     0,             0, (void *) "Modern", NULL, NULL },
    { jwin_text_proc,       8,  130,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "To customise, open the 'Quest >> Rules' and ", NULL, NULL },
    { jwin_text_proc,       8,  140,  128,    8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "'ZScript >> Quest Specific Settings' dialogues.", NULL, NULL },
    { jwin_button_proc,       93+40-4,   153,   61,    21,  vc(14),              vc(1),                13,       D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t d_ruleset_radio_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t temp = ruleset;
    int32_t ret = jwin_radiofont_proc(msg,d,c);
    
    if(ruleset_dlg[3].flags & D_SELECTED) ruleset = rulesetNONE;
    else if(ruleset_dlg[4].flags & D_SELECTED) ruleset = rulesetNES;
    else if(ruleset_dlg[5].flags & D_SELECTED) ruleset = rulesetFixedNES;
    else if(ruleset_dlg[6].flags & D_SELECTED) ruleset = rulesetBSZ;
    else if(ruleset_dlg[7].flags & D_SELECTED) ruleset = rulesetZ3;
    else if(ruleset_dlg[20].flags & D_SELECTED) ruleset = rulesetModern;
    
    if(temp != ruleset)
    {
        return D_REDRAW;
    }
    
    return ret;
}

int32_t d_rulesettext_proc(int32_t msg, DIALOG *d, int32_t)
{
	if(msg!=MSG_DRAW)
		return D_O_K;
		
	char buf[42];
	char buf2[42];
	char buf3[42];
	char buf4[42];
	
	switch(ruleset)
	{
		case rulesetNES: // Original NES
			sprintf(buf,  "Emulates the behaviour, the quirks,");
			sprintf(buf2, "bugs, and oddities found in the NES");
			sprintf(buf3, "game 'The Legend of Zelda'.");
			sprintf(buf4, "All but a few rules are off.");
			break;
			
		case rulesetFixedNES: // Fixed NES
			sprintf(buf,  "Corrects a large number of oddities");
			sprintf(buf2, "found in the original NES engine, ");
			sprintf(buf3, "such as bomb interactions. ");
			sprintf(buf4, "Enables all 'NES Fixes' Rules");
			break;
			
		case rulesetBSZ: // BS Zelda
			sprintf(buf,  "Adds expanded animations befitting a");
			sprintf(buf2, "Super Famicom era game: Expanded");
			sprintf(buf3, "enemy tiles, fast scrolling, new push-");
			sprintf(buf4, "blocks, transition wipes, etc.");
			break;
			
		case rulesetZ3: // Zelda 3-esque
			sprintf(buf,  "As 16-bit, plus diagonal movement,");
			sprintf(buf2, "new message strings, magic use, real");
			sprintf(buf3, "arrows, more sounds, drowning, ");
			sprintf(buf4, "modern boomerang/item interaction.");
			break;
		
		case rulesetModern: // 255
			sprintf(buf,  "Enables all new 2.55 features including");
			sprintf(buf2, "new Hero movement/step speed, new");
			sprintf(buf3, "combo animations, scripting extensions,");
			sprintf(buf4, "and other engine enhancements.");
			break;
	}
	
	FONT *f = font;
	textprintf_ex(screen,f,d->x-1,d->y-11,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",buf);
	textprintf_ex(screen,f,d->x-1,d->y-11+12,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",buf2);
	textprintf_ex(screen,f,d->x-1,d->y-11+24,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",buf3);
	textprintf_ex(screen,f,d->x-1,d->y-11+36,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",buf4);
	return D_O_K;
}

int32_t onStrFix();
void applyRuleset(int32_t newRuleset, byte *qrptr)
{
	ruleset = newRuleset;
	for(int32_t i=0; i<qr_MAX; ++i)
	{
		switch(i)
		{
			case qr_PARSER_NO_LOGGING:
			case qr_PARSER_250DIVISION:
			case qr_PARSER_SHORT_CIRCUIT:
			case qr_PARSER_BOOL_TRUE_DECIMAL:
			case qr_PARSER_TRUE_INT_SIZE:
			case qr_PARSER_BINARY_32BIT:
			case qr_PARSER_STRINGSWITCH_INSENSITIVE:
				//Ignore compiler options
				break;
			case qr_OLD_STRING_EDITOR_MARGINS:
			case qr_STRING_FRAME_OLD_WIDTH_HEIGHT:
				//These get their own fix in 'onStrFix()'
				break;
			default:
				set_qr(i,0,qrptr);
		}
	}
	onStrFix();
	applyRuleTemplate(ruletemplateFixCompat, qrptr);
	
	//Set on things that should ALWAYS be on.
	static const int rALWAYS_ON[] = {
		// Engine fixes
		qr_DMGCOMBOLAYERFIX, qr_WARPSIGNOREARRIVALPOINT, qr_NOARRIVALPOINT, qr_CMBCYCLELAYERS,
		qr_KILLALL, qr_ITEMBUBBLE, qr_SHIELDBUBBLE, qr_RINGAFFECTDAMAGE, qr_NEW_COMBO_ANIMATION,
		qr_BUSHESONLAYERS1AND2, qr_CUSTOMCOMBOSLAYERS1AND2, qr_CUSTOMCOMBOS_EVERY_LAYER,
		qr_HOOKSHOTLAYERFIX, qr_EPILEPSY, qr_FIXSCRIPTSDURINGSCROLLING, qr_ALWAYS_DEALLOCATE_ARRAYS,
		qr_ITEMSCRIPTSKEEPRUNNING, qr_FIXDRUNKINPUTS, qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE,
		qr_SMARTER_WATER, qr_LESS_AWFUL_SIDESPIKES, qr_SWITCHES_AFFECT_MOVINGBLOCKS,
		// Script Logging
		qr_SCRIPTERRLOG, qr_LOG, qr_TRACESCRIPTIDS,
		// Misc Engine stuff (not really related to any particular ruleset, so, always on)
		qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT, qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN,
		qr_COMBOSCRIPTS_LAYER_0, qr_ONDEATH_RUNS_AFTER_DEATH_ANIM, qr_FFCSCROLL, qr_GREATER_MAX_TIME,
		qr_WEAPONS_EXTRA_DEATH_FRAME, qr_USE_EX1_EX2_INVENTORYSWAP, qr_CLEARINITDONSCRIPTCHANGE,
		qr_SCRIPT_WEAPONS_UNIQUE_SPRITES, qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES,
		qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS, qr_SMARTER_SMART_SCROLL,
		qr_LIFTED_WEAPONS_RUN_SCRIPTS, qr_BOMB_BOOMS_CLEAR_SCRIPTS,
	};
	static const int r_FIXES[] = {
		qr_FREEFORM, qr_SAFEENEMYFADE, qr_ITEMSONEDGES, qr_HERODUNGEONPOSFIX, qr_RLFIX,
		qr_NOLEVEL3FIX, qr_BOMBHOLDFIX, qr_HOLDNOSTOPMUSIC, qr_CAVEEXITNOSTOPMUSIC,
		qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX, /*qr_WPNANIMFIX,*/ qr_PUSHBLOCKCSETFIX,
		qr_TRAPPOSFIX, qr_NOBORDER, qr_SUBSCREENOVERSPRITES,
		qr_BOMBDARKNUTFIX, qr_OFFSETEWPNCOLLISIONFIX, qr_ITEMSINPASSAGEWAYS, qr_NOFLICKER, qr_FIREPROOFHERO2,
		qr_NOITEMOFFSET, qr_LADDERANYWHERE, qr_TRUEFIXEDBOMBSHIELD, qr_NOTMPNORET, qr_NOFLASHDEATH, qr_BROKENSTATUES, 
		qr_DYING_ENEMIES_IGNORE_STUN, qr_SHOP_ITEMS_VANISH, qr_EXPANDEDLTM, qr_CORRECTED_EW_BRANG_ANIM,
	};
	static const int rNES[] = {
		qr_OLDPICKUP, qr_OLDSTYLEWARP,
	};
	static const int rBSZ[] = {
		qr_TIME, qr_NOBOMBPALFLASH, qr_NEWENEMYTILES, qr_FASTDNGN, qr_SMOOTHVERTICALSCROLLING,
		qr_COOLSCROLL, qr_BSZELDA, qr_SOLIDBLK, qr_HESITANTPUSHBLOCKS, qr_INSTABURNFLAGS,
		qr_FADE, qr_EXPANDEDLTM,
	};
	static const int rZ3[] = {
		qr_DROWN, qr_HIDECARRIEDITEMS, qr_ALLOWMSGBYPASS, qr_ALLOWFASTMSG, qr_MSGDISAPPEAR,
		qr_MSGFREEZE, qr_ENABLEMAGIC, qr_NOWANDMELEE, qr_TRUEARROWS, qr_Z3BRANG_HSHOT,
		qr_TRANSSHADOWS, qr_SHADOWS, qr_LTTPWALK, qr_MORESOUNDS, qr_NEVERDISABLEAMMOONSUBSCREEN,
	};
	static const int rMODERN[] = {
		qr_NO_SCROLL_WHILE_IN_AIR, qr_DUNGEON_DMAPS_PERM_SECRETS,
		qr_ANGULAR_REFLECTED_WEAPONS, qr_MIRRORS_USE_WEAPON_CENTER,
		qr_SPRITEXY_IS_FLOAT, qr_SIDEVIEWLADDER_FACEUP,
		qr_ITEMSHADOWS, qr_WEAPONSHADOWS,
		qr_NEW_HERO_MOVEMENT, qr_STEP_IS_FLOAT,
		qr_HOLDITEMANIMATION, qr_DISABLE_4WAY_GRIDLOCK,
		qr_NO_HOPPING, qr_NO_SOLID_SWIM,
		qr_WATER_ON_LAYER_1, qr_WATER_ON_LAYER_2,
		qr_SHALLOW_SENSITIVE, qr_NONHEAVY_BLOCKTRIGGER_PERM,
		qr_CORRECTED_EW_BRANG_ANIM, qr_OVERHEAD_COMBOS_L1_L2,
		qr_AUTOCOMBO_LAYER_1, qr_AUTOCOMBO_LAYER_2,
		qr_AUTOCOMBO_ANY_LAYER, qr_NEW_DARKROOM,
		qr_PUSHBLOCK_LAYER_1_2, qr_HOOKSHOTALLLAYER,
		qr_ITEMCOMBINE_NEW_PSTR, qr_ITEMCOMBINE_CONTINUOUS,
		qr_FAIRYDIR, qr_BLOCKS_DONT_LOCK_OTHER_LAYERS,
		qr_CONVEYORS_L1_L2, qr_CONVEYORS_ALL_LAYERS,
		qr_EW_FIRE_EMITS_LIGHT, qr_BLOCKS_DROWN,
		qr_NO_STUNLOCK_IGNORE, qr_PUSHBLOCKS_FALL_IN_SIDEVIEW,
		qr_BETTER_PLAYER_JUMP_ANIM, qr_ARMOS_GRAVE_ON_LAYERS,
	};
	static const int r_NON_MODERN[] = {
		qr_OLDSPRITEDRAWS, qr_CLASSIC_DRAWING_ORDER,
	};
	static const int rMODERN_TMPL[] = {
		ruletemplateFixCompat, ruletemplateNewSubscreen,
	};
	for(auto qr : rALWAYS_ON)
		set_qr(qr, 1, qrptr);
	switch(ruleset)
	{
		case rulesetNES: // Authentic NES
		{
			for(auto qr : r_NON_MODERN)
				set_qr(qr, 1, qrptr);
			for(auto qr : rNES)
				set_qr(qr, 1, qrptr);
			break;
		}
		case rulesetFixedNES: // Fixed NES
		{
			for(auto qr : r_NON_MODERN)
				set_qr(qr, 1, qrptr);
			for(auto qr : r_FIXES)
				set_qr(qr, 1, qrptr);
			break;
		}
		case rulesetBSZ: // BS Zelda
		{
			for(auto qr : r_NON_MODERN)
				set_qr(qr, 1, qrptr);
			for(auto qr : r_FIXES)
				set_qr(qr, 1, qrptr);
			for(auto qr : rBSZ)
				set_qr(qr, 1, qrptr);
			break;
		}
		case rulesetZ3: // Zelda 3-esque
		{
			for(auto qr : r_NON_MODERN)
				set_qr(qr, 1, qrptr);
			for(auto qr : r_FIXES)
				set_qr(qr, 1, qrptr);
			for(auto qr : rBSZ)
				set_qr(qr, 1, qrptr);
			for(auto qr : rZ3)
				set_qr(qr, 1, qrptr);
			break;
		}
		case rulesetModern: // Modern-style
		{
			for(auto qr : r_FIXES)
				set_qr(qr, 1, qrptr);
			for(auto qr : rBSZ)
				set_qr(qr, 1, qrptr);
			for(auto qr : rZ3)
				set_qr(qr, 1, qrptr);
			for(auto qr : r_NON_MODERN)
				set_qr(qr, 0, qrptr);
			for(auto qr : rMODERN)
				set_qr(qr, 1, qrptr);
			for(auto tmpl : rMODERN_TMPL)
				applyRuleTemplate(tmpl, qrptr);
			break;
		}
	}
}

int32_t PickRuleset()
{
	call_ruleset_dlg(); return D_O_K;
}
int32_t PickRuleTemplate()
{
	call_ruletemplate_dlg(); return D_O_K;
}

int32_t onNew()
{
	if(checksave()==0)
		return D_O_K;

	restore_mouse();
	return call_tileset_wizard() ? D_O_K : D_CLOSE;
}

int32_t onSave()
{
    restore_mouse();
    
    if(disable_saving)
    {
        jwin_alert("ZQuest","Saving is","disabled in this version.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
        return D_O_K;
    }
    
    if(!first_save)
        return onSaveAs();
    else if(OverwriteProtection)
    {
        jwin_alert("ZQuest","Overwriting quests is disabled.","Change this in the options dialog.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
        return D_O_K;
    }
    
	if(zc_get_config("zquest","quick_compile_on_save",0))
	{
		if(!do_compile_and_slots(1,false))
		{
			InfoDialog("ZQuest","Failed compile on save! Saving quest anyway...").show();
		}
	}
	
    //bool compress = !UncompressedAutoSaves;
	//Don't tie regular saves being uncompressed to the autosave option.
	bool compress = true;
    if (util::get_ext(temppath) == ".qsu") compress = false;
    int32_t ret = save_unencoded_quest(filepath, compress, filepath);
    char buf[256+20],name[256];
    extract_name(filepath,name,FILENAMEALL);
    
    if(!ret)
    {
        sprintf(buf,"Saved %s",name);
        jwin_alert("ZQuest",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
        saved=true;
        first_save=true;
        header.dirty_password=false;
    }
    else
    {
        sprintf(buf,"Error saving %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
	set_last_timed_save(nullptr);
    return D_O_K;
}

void update_recent_quest(char const* path);

int32_t onSaveAs()
{
	static EXT_LIST list[] =
	{
		{ (char *)"Quest Files (*.qst)", (char *)"qst"                                     },
		{ (char *)"Unencrypted Quest Files (*.qsu)", (char *)"qsu"                                     },
		{ NULL,                                                  NULL                                              }
	};

	if(disable_saving)
	{
		jwin_alert("ZQuest","Saving is","disabled in this version.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		return D_O_K;
	}
#ifdef __EMSCRIPTEN__
	std::string path;
	if (auto result = prompt_for_new_file("Save Quest As (.qst)","qst",list,get_initial_file_dialog_folder(),true); !result)
	{
		return D_O_K;
	}
	else path = *result;
#else
	std::string path;
	if (auto result = prompt_for_new_file("Save Quest As (.qst)", "qst", list, filepath, true); !result)
	{
		return D_O_K;
	}
	else path = *result;
#endif

	// TODO: stop using temppath.
	strcpy(temppath, path.c_str());

	if(exists(temppath))
	{
		if(OverwriteProtection)
		{
			jwin_alert("ZQuest","Overwriting quests is disabled.","Change this in the options dialog.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
			return D_O_K;
		}
		
		if(jwin_alert("Confirm Overwrite",temppath,"already exists.","Write over existing file?","&Yes","&No",'y','n',get_zc_font(font_lfont))==2)
		{
			return D_O_K;
		}
	}
	
	if(zc_get_config("zquest","quick_compile_on_save",0))
	{
		if(!do_compile_and_slots(1,false))
		{
			InfoDialog("ZQuest","Failed compile on save! Saving quest anyway...").show();
		}
	}
	
	//bool compress = !UncompressedAutoSaves;
	bool compress = true;
	if (util::get_ext(temppath) == ".qsu") compress = false;
	int32_t ret = save_unencoded_quest(temppath, compress, temppath);
	char buf[1024],name[256];
	extract_name(temppath,name,FILENAMEALL);
	
	if(!ret)
	{
		strcpy(filepath,temppath);
		update_recent_quest(temppath);
		sprintf(buf,"ZC Editor - [%s]", get_filename(filepath));
		set_window_title(buf);
		sprintf(buf,"Saved %s",name);
		jwin_alert("ZQuest",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		saved=true;
		first_save=true;
		header.dirty_password=false;
	}
	else
	{
		sprintf(buf,"Error saving %s",name);
		jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}
	
	refresh(rMENU);
	set_last_timed_save(nullptr);
	return D_O_K;
}

int32_t open_quest(char const* path)
{
	std::error_code ec;
	if (fs::equivalent("./tilesets", fs::path(path).parent_path(), ec))
	{
		jwin_alert("Warning",
			"You've opened a qst in the tilesets folder - instead, you should probably use File>New so that a new file is made.",
			"Files in this folder may be overwritten by the software updater.",
			NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}

	int32_t ret = load_quest(path);
	
	if(ret == qe_OK)
	{
		update_recent_quest(path);
		saved = true;
		strcpy(filepath, path);
		first_save=true;
	}
	else if(ret == qe_cancel)
	{
		saved = true;
		filepath[0]=0;
	}
	else
	{
		char buf[256+20],name[256];
		extract_name(path,name,FILENAMEALL);
		sprintf(buf,"Unable to load %s",name);
		jwin_alert("Error",buf,qst_error[ret],NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		filepath[0]=0;
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	Map.setCurrMap(zinit.last_map);
	Map.setCopyFFC(-1); //Do not have an initial ffc on the clipboard. 
	Map.setCurrScr(zinit.last_screen);
	Map.ClearCommandHistory();
	rebuild_trans_table();
	rebuild_string_list();
	onDrawingModeNormal();
	refresh(rALL);
	set_last_timed_save(nullptr);
	return ret;
}

int32_t customOpen(char const* path)
{
	restore_mouse();
	
	if(checksave()==0)
		return D_O_K;
	
	open_quest(path);
	return D_O_K;
}
char* get_qst_name(char const* def_path)
{
	static EXT_LIST list[] =
	{
		{ (char *)"Quest Files (*.qst)", (char *)"qst"                                     },
		{ (char *)"Quest Auto-backups (*.qb?)", (char *)"qb0,qb1,qb2,qb3,qb4,qb5,qb6,qb7,qb8,qb9" },
		{ (char *)"Quest Timed Auto-saves (*.qt?)", (char *)"qt0,qt1,qt2,qt3,qt4,qt5,qt6,qt7,qt8,qt9" },
		{ (char *)"Uncompressed Quest Timed Auto-saves (*.qu?)", (char *)"qu0,qu1,qu2,qu3,qu4,qu5,qu6,qu7,qu8,qu9" },
		{ (char *)"Unencrypted Quest Files (*.qsu)", (char *)"qsu"                                     },
		{ (char *)"Quest Template Files (*.zqt)", (char *)"zqt"                                     },
		{ (char *)"All Files (*.*)",                             NULL                                              },
		{ NULL,                                                  NULL                                              }
	};
	
#ifdef __EMSCRIPTEN__
	return prompt_for_existing_file_compat("Load File","",list,get_initial_file_dialog_folder().c_str(),true)
#else
	return prompt_for_existing_file_compat("Load File","",list,def_path ? def_path : filepath,true)
#endif
		? temppath : nullptr;
}
int32_t onOpen()
{
	restore_mouse();
	
	if(checksave()==0)
		return D_O_K;
	
	if(!get_qst_name())
		return D_O_K;
	
	open_quest(temppath);
	return D_O_K;
}

int32_t onRevert()
{
    if(jwin_alert("Confirm Revert","Are you sure you want to lose","all changes since last save?",NULL,"Yes","No",'y','n',get_zc_font(font_lfont))==2)
    {
        return D_O_K;
    }
    
    if(filepath[0]!=0)
    {
        int32_t ret = load_quest(filepath);
        
        if(!ret)
        {
            saved = true;
        }
        else
        {
            char buf[256+20],name[256];
            extract_name(filepath,name,FILENAMEALL);
            sprintf(buf,"Unable to load %s",name);
            jwin_alert("Error",buf,qst_error[ret],NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
            filepath[0]=0;
        }
        
        setup_combo_animations();
        setup_combo_animations2();
        refresh(rALL);
    }
    else
    {
        NewQuestFile(DEFAULT_TILESET);
        
        if(RulesetDialog > 0)
		{
            PickRuleset();
			PickRuleTemplate();
		}
    }
    
    onDrawingModeNormal();
	set_last_timed_save(nullptr);
    return D_O_K;
}

static DIALOG import_map_bias_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc, 80,   80-8,   160+1,  64+20+1,   vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Import Bias", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_button_proc,     90,   120+10,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     170,  120+10,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_radio_proc,      130, 104-8,   96+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Total Copy", NULL, NULL },
    { jwin_radio_proc,      130, 114-8,  120+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Prefer Original", NULL, NULL },
    { jwin_radio_proc,      130, 124-8,  104+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Prefer Import", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t get_import_map_bias()
{
    import_map_bias_dlg[0].dp2=get_zc_font(font_lfont);
    
    for(int32_t i=0; i<3; i++)
    {
        import_map_bias_dlg[i+4].flags=0;
    }
    
    import_map_bias_dlg[ImportMapBias+4].flags=D_SELECTED;
    
    large_dialog(import_map_bias_dlg);
        
    if(do_zqdialog(import_map_bias_dlg,2)==2)
    {
        for(int32_t i=0; i<3; i++)
        {
            if(import_map_bias_dlg[i+4].flags&D_SELECTED)
            {
                ImportMapBias=i;
				zc_set_config("zquest","import_map_bias",ImportMapBias);
                break;
            }
        }
        
        return 0;
    }
    
    return -1;
}

int32_t onImport_Map()
{
    if(Map.getCurrMap()>=Map.getMapCount())
        return D_O_K;
        
    if(get_import_map_bias()==-1)
    {
        return D_O_K;
    }
    
    if(!prompt_for_existing_file_compat("Import Map (.map)","map",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    int32_t ret=Map.load(temppath);
    
    if(ret)
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,loaderror[ret],NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
        
        if(ret>1)
            Map.clearmap(false);
    }
    else
    {
        bool willaffectlayers=false;
        
        for(int32_t i=0; i<MAPSCRS; i++)
        {
            for(int32_t j=0; !willaffectlayers && j<6; j++)
            {
                if(TheMaps[Map.getCurrMap()*MAPSCRS+i].layermap[j]>Map.getMapCount())
                {
                    willaffectlayers=true;
                    break;
                }
            }
            
            fix_layers(&TheMaps[Map.getCurrMap()*MAPSCRS+i], false);
        }
        
        if(willaffectlayers)
        {
            jwin_alert("Layers Changed",
                       "One or more screens in the imported map had",
                       "layers on maps that do not exist. The map numbers",
                       "of the affected layers will be reset to 0.",
                       "&OK", NULL, 'o', 0, get_zc_font(font_lfont));
        }
    }
    
    refresh(rSCRMAP+rMAP+rMENU);
    return D_O_K;
}

int32_t onExport_Map()
{
    if(Map.getCurrMap()>=Map.getMapCount())
        return D_O_K;
        
    if(!prompt_for_new_file_compat("Export Map (.map)","map",NULL,datapath,false))
        return D_O_K;
        
    int32_t ret = Map.save(temppath);
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(!ret)
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}


int32_t onExport_Tilepack()
{
	savesometiles("Save Tile Package", 0);
	return D_O_K;	
}

int32_t onImport_Tilepack_To()
{
	writesometiles_to("Load Tile Package to:", 0);
	return D_O_K;	
}

int32_t onExport_DMaps()
{

    
    savesomedmaps("Read DMaps",0);
    
    return D_O_K;
}


int32_t onImport_DMaps()
{
    if(!prompt_for_existing_file_compat("Import DMaps (.zdmap)","zdmap",NULL,datapath,false))
        return D_O_K;
    
    PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
	if(f)
	{
		if(!readsomedmaps(f))
		{
			char buf[256+20],name[256];
			extract_name(temppath,name,FILENAMEALL);
			sprintf(buf,"Unable to load %s",name);
			jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		}
		else
		{
			char name[256];
			extract_name(temppath,name,FILENAMEALL);
			char tmpbuf[256+20]={0};
			sprintf(tmpbuf,"Loaded %s",name);
			
			
			
			
			int32_t maxMap=0;
			for(int32_t i=0; i<MAXDMAPS; i++)
			{
			    if(DMaps[i].map>maxMap)
				maxMap=DMaps[i].map;
			}
			
			if(maxMap>map_count)
			{
			    int32_t ret=jwin_alert("Not enough maps",
					       "The imported DMaps use more maps than are",
					       " currently available. Do you want to add",
					       "more maps or change the DMaps' settings?",
					       "&Add maps","&Modify DMaps",'a','m',get_zc_font(font_lfont));
			    if(ret==1)
				setMapCount2(maxMap+1);
			    else
			    {
				for(int32_t i=0; i<MAXDMAPS; i++)
				{
				    if(DMaps[i].map>=map_count)
					DMaps[i].map=0;
				}
			    }
			}
			
			jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		}
	}
	pack_fclose(f);
   
    saved=false;

    
    return D_O_K;
}

int32_t onImport_Tilepack()
{
		if(prompt_for_existing_file_compat("Load ZTILE(.ztile)", "ztile", NULL,datapath,false))
		{  
			char name[256];
			extract_name(temppath,name,FILENAMEALL);
			PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
			if(f)
			{
				if (!readtilefile(f))
				{
					al_trace("Could not read from .ztile packfile %s\n", name);
					jwin_alert("ZTILE File: Error","Could not load the specified Tile.",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
				else
				{
					jwin_alert("ZTILE File: Success!","Loaded the source tiles to your tile sheets!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
			}
	
			pack_fclose(f);
		}
		return D_O_K;
}

int32_t onExport_Combopack()
{
	savesomecombos("Save Combo Package", 0);
	return D_O_K;
	
}

int32_t onImport_Combopack_To()
{
	writesomecombos_to("Load Combo Package to:", 0);
	return D_O_K;
	
}

int32_t onImport_Combopack()
{
		loadcombopack("Load Combo Package to:", 0);
		return D_O_K;
}


int32_t onExport_Comboaliaspack()
{
	savesomecomboaliases("Save Combo Alias Package", 0);
	return D_O_K;
	
}

int32_t onImport_Comboaliaspack_To()
{
	writesomecomboaliases_to("Load Combo Alias Package to:", 0);
	return D_O_K;
	
}

int32_t onImport_Comboaliaspack()
{
		if(prompt_for_existing_file_compat("Load ZALIAS(.zalias)", "zalias", NULL,datapath,false))
		{  
			char name[256];
			extract_name(temppath,name,FILENAMEALL);
			PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
			if(f)
			{
				if (!readcomboaliasfile(f))
				{
					al_trace("Could not read from .zalias packfile %s\n", name);
					jwin_alert("ZALIAS File: Error","Could not load the specified combo aliases.",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
				else
				{
					jwin_alert("ZALIAS File: Success!","Loaded the source combo aliases to your combo alias tables!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
					saved=false;
				}
			}
	
			pack_fclose(f);
		}
		return D_O_K;
}

int32_t onImport_Pals()
{
    if(!prompt_for_existing_file_compat("Import Palettes (.zpl)","zpl",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    if(!load_pals(temppath,0))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    return D_O_K;
}

int32_t onExport_Pals()
{
    if(!prompt_for_new_file_compat("Export Palettes (.zpl)","zpl",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_pals(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onImport_Msgs()
{
    if(!prompt_for_existing_file_compat("Import String Table (.zqs)","zqs",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    if(!load_msgstrs(temppath,0))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    return D_O_K;
}

int32_t onExport_Msgs()
{
    if(!prompt_for_new_file_compat("Export String Table (.zqs)","zqs",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_msgstrs(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onImport_StringsTSV()
{
    if(!prompt_for_existing_file_compat("Import Strings (.tsv)","tsv",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    if(!load_strings_tsv(temppath))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    return D_O_K;
}

int32_t onExport_StringsTSV()
{
    if(!prompt_for_new_file_compat("Export Strings (.tsv)","tsv",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_strings_tsv(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}


int32_t onExport_MsgsText()
{
    if(!prompt_for_new_file_compat("Export Text Dump (.txt)","txt",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_msgstrs_text(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onImport_Combos()
{
	writesomecombos("Load Combo Set", 0);
	return D_O_K;
	
}


int32_t onExport_Combos()
{
    if(!prompt_for_new_file_compat("Export Combos (.zcombo)","zcombo",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    //writetilefile(f,first_tile_id,the_tile_count);
    
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
	if(f)
	{
		writecombofile(f,0,MAXCOMBOS);
		pack_fclose(f);
		
		char tmpbuf[256+20]={0};
		sprintf(tmpbuf,"Saved %s",name);
		jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}
	else
	{
		sprintf(buf,"Error");
		sprintf(buf2,"Error saving %s",name);
	}
    
    return D_O_K;
}

int32_t onImport_Tiles()
{
    int32_t ret=getnumber("Import Start Page",0);
    
    if(cancelgetnum)
    {
        return D_O_K;
    }
    
    bound(ret,0,TILE_PAGES-1);
    
    if(!prompt_for_existing_file_compat("Import Tiles (.ztileset)","ztileset",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    char name[256];
    extract_name(temppath,name,FILENAMEALL);
    PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
	if(f)
	{
		if(!readtilefile_to_location(f,0,ret))
		{
			char buf[256+20];
			sprintf(buf,"Unable to load %s",name);
			jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		}
		else
		{
			char tmpbuf[256+20]={0};
			
			sprintf(tmpbuf,"Saved %s",name);
			jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
		}
	}
	pack_fclose(f);
    
    
    refresh(rALL);
    return D_O_K;
}

int32_t onExport_Tiles()
{
    if(!prompt_for_new_file_compat("Export Tiles (.ztileset)","ztileset",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    //writetilefile(f,first_tile_id,the_tile_count);
    
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
	if(f)
	{
		writetilefile(f,0,NEWMAXTILES);
		pack_fclose(f);
		
		char tmpbuf[256+20]={0};
		
		sprintf(tmpbuf,"Saved %s",name);
		jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}
	else
	{
		
		sprintf(buf,"Error");
		sprintf(buf2,"Error saving %s",name);
	}
    
    return D_O_K;
}

int32_t onImport_Guys()
{
    if(!prompt_for_existing_file_compat("Import Enemies (.guy)","guy",NULL,datapath,false))
        return D_O_K;
        
    if(!load_guys(temppath))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    refresh(rALL);
    return D_O_K;
}

int32_t onExport_Guys()
{
    if(!prompt_for_new_file_compat("Export Enemies (.guy)","guy",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_guys(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}





bool save_combo_alias(const char *path);
bool load_combo_alias(const char *path);
int32_t onImport_ComboAlias()
{
    if(!prompt_for_existing_file_compat("Import Combo Alias (.zca)","zca",NULL,datapath,false))
        return D_O_K;
        
    if(!load_combo_alias(temppath))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    refresh(rALL);
    return D_O_K;
}

int32_t onExport_ComboAlias()
{
    if(!prompt_for_new_file_compat("Export Combo Alias (.zca)","zca",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_combo_alias(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onImport_ZGP()
{
    if(!prompt_for_existing_file_compat("Import Graphics Pack (.zgp)","zgp",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    if(!load_zgp(temppath))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    refresh(rALL);
    return D_O_K;
}

int32_t onExport_ZGP()
{
    if(!prompt_for_new_file_compat("Export Graphics Pack (.zgp)","zgp",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_zgp(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

//Doorsets
int32_t readzdoorsets(PACKFILE *f, int32_t first, int32_t count, int32_t deststart)
{
	dword section_version=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	int32_t doorscount = 0;
	DoorComboSet tempDoorComboSet;
	std::string dcs_name;
	memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
	int32_t lastset = 0;
	int32_t firstset = 0;
	int32_t last = 0;
	int32_t ret = 1;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	if(!p_igetl(&doorscount,f))
	{
		return 0;
	}
	al_trace("doorscount is: %d\n", doorscount);
	if(!p_igetl(&firstset,f))
	{
		return 0;
	}
	if(!p_igetl(&last,f))
	{
		return 0;
	}
	//if the params passed would be invalid:
	if ( first < firstset || first >= door_combo_set_count ) 
	{
		first = firstset;
	}
	if ( lastset < 0 || lastset > count )
	{
		lastset = doorscount;
	}
	else
	{
		lastset = firstset + count;
	}
	//end params sanity guard
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zdoors packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_DOORS ))
	{
		al_trace("Cannot read .zdoors packfile made using V_DOORS (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zdoors packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	//section data for doors
	for(int32_t i=firstset+deststart; i<lastset+deststart; ++i)
	{
		if(i+deststart >= door_combo_set_count)
		{
			al_trace("Reached the current door count trying to import doorsets.\n");
			ret = 2; break;
		}
		al_trace("Door readcycle %d\n", i-deststart);
		//Clear per set
		memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
		//name
		char name[21];
		if(!pfread(&name,sizeof(name),f))
		{
			return 0;
		}
		dcs_name = name;

		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f))
				{
					return 0;
				}
			}
		}
		//down door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f))
				{
					return 0;
				}
			}
		}
		//left door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f))
				{
					return 0;
				}
			}
		}
		//right door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.flags[j],f))
			{
				return 0;
			}
		}
		memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
		DoorComboSetNames[i] = dcs_name;
	}
	return ret;
}


int32_t writezdoorsets(PACKFILE *f, int32_t first = 0, int32_t count = door_combo_set_count)
{
	dword section_version=V_DOORS;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	int32_t doorscount = door_combo_set_count;
	int32_t firstset = first;
	int32_t lastset = count;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	if(!write_deprecated_section_cversion(section_version,f))
	{
		return 0;
	}
	if(!p_iputl(doorscount,f))
	{
		return 0;
	}
	if(!p_iputl(firstset,f))
	{
		return 0;
	}
	if(!p_iputl(lastset,f))
	{
		return 0;
	}
	//if the params passed would be invalid:
	if ( firstset < 0 || firstset >= doorscount )
	{
		firstset = 0;
	}
	if ( lastset < 1 || lastset >= doorscount )
	{
		lastset = doorscount;
	}
	else
	{
		lastset = firstset + count;
	}
	//end params sanity guard
	
	//doorset data
	for(int32_t i=firstset; i<lastset; ++i)
        {
		al_trace("Door writecycle %d\n", i);
		//name
		char name[21];
		memset(name, 21, (char)0);
		strcpy(name, DoorComboSetNames[i].c_str());
		if(!pfwrite(name,sizeof(name),f))
		{
			return 0;
		}
		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_u[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_u[j][k],f))
				{
					return 0;
				}
			}
		}
		//down door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_d[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_d[j][k],f))
				{
					return 0;
				}
			}
		}
		//left door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_l[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_l[j][k],f))
				{
					return 0;
				}
			}
		}
		//right door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_r[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_r[j][k],f))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_u[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_u[j],f))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_d[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_d[j],f))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_l[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_l[j],f))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_r[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_r[j],f))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_iputw(DoorComboSets[i].walkthroughcombo[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_putc(DoorComboSets[i].walkthroughcset[j],f))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].flags[j],f))
			{
				return 0;
			}
		}
        }
	return 1;
}




int32_t writeonezdoorset(PACKFILE *f, int32_t index)
{
	dword section_version=V_DOORS;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	int32_t doorscount = door_combo_set_count;
	int32_t firstset = 0;
	int32_t lastset = 1;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	if(!write_deprecated_section_cversion(section_version,f))
	{
		return 0;
	}
	if(!p_iputl(doorscount,f))
	{
		return 0;
	}
	if(!p_iputl(firstset,f))
	{
		return 0;
	}
	if(!p_iputl(lastset,f))
	{
		return 0;
	}
	//doorset data
	
        {
		//name
		char name[21];
		memset(name, 21, (char)0);
		strcpy(name, DoorComboSetNames[index].c_str());
		if(!pfwrite(name,sizeof(name),f))
		{
			return 0;
		}

		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[index].doorcombo_u[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[index].doorcset_u[j][k],f))
				{
					return 0;
				}
			}
		}
		//down door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[index].doorcombo_d[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[index].doorcset_d[j][k],f))
				{
					return 0;
				}
			}
		}
		//left door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[index].doorcombo_l[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[index].doorcset_l[j][k],f))
				{
					return 0;
				}
			}
		}
		//right door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[index].doorcombo_r[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[index].doorcset_r[j][k],f))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[index].bombdoorcombo_u[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[index].bombdoorcset_u[j],f))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[index].bombdoorcombo_d[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[index].bombdoorcset_d[j],f))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[index].bombdoorcombo_l[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[index].bombdoorcset_l[j],f))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[index].bombdoorcombo_r[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[index].bombdoorcset_r[j],f))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_iputw(DoorComboSets[index].walkthroughcombo[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_putc(DoorComboSets[index].walkthroughcset[j],f))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[index].flags[j],f))
			{
				return 0;
			}
		}
        }
	return 1;
}




int32_t readonezdoorset(PACKFILE *f, int32_t index)
{
	dword section_version=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	int32_t doorscount = 0;
	DoorComboSet tempDoorComboSet;
	std::string dcs_name;
	memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
	int32_t firstset = 0;
	int32_t last = 0;
	int32_t ret = 1;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	if(!p_igetl(&doorscount,f))
	{
		return 0;
	}
	al_trace("doorscount is: %d\n", doorscount);
	if(!p_igetl(&firstset,f))
	{
		return 0;
	}
	if(!p_igetl(&last,f))
	{
		return 0;
	}
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zdoors packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_DOORS ))
	{
		al_trace("Cannot read .zdoors packfile made using V_DOORS (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zdoors packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	//section data for doors
	{
		//Clear per set
		memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
		char name[21];
		if(!pfread(&name,sizeof(name),f))
		{
			return 0;
		}
		dcs_name = name;
		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f))
				{
					return 0;
				}
			}
		}
		//down door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f))
				{
					return 0;
				}
			}
		}
		//left door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f))
				{
					return 0;
				}
			}
		}
		//right door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.flags[j],f))
			{
				return 0;
			}
		}
		memcpy(&DoorComboSets[index], &tempDoorComboSet, sizeof(tempDoorComboSet));
		DoorComboSetNames[index] = dcs_name;
	}
	return ret;
}

int32_t onExport_Doorset()
{
	do_exportdoorset("Export Doorsets", 0);
	return D_O_K;
}
int32_t onImport_Doorset()
{
	do_importdoorset("Import Doorsets", 0);
	return D_O_K;
}

void center_zq_files_dialogs()
{
    jwin_center_dialog(ruleset_dlg);
    jwin_center_dialog(import_map_bias_dlg);
}
