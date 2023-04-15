//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_files.cc
//
//  File support for ZQuest.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <stdio.h>

#include "base/gui.h"
#include "zq_files.h"
#include "base/zdefs.h"
#include "zq_misc.h"
#include "zquest.h"
#include "qst.h"
#include "base/zsys.h"
#include "zq_class.h"
#include "tiles.h"
#include "zq_tiles.h"
#include "zq_custom.h"

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

#ifdef _MSC_VER
#define getcwd _getcwd
#define stricmp _stricmp
#define strupr _strupr
#endif

char qtbuf[31];

void reset_qt(int32_t index)
{
    bound(index,0,MAXQTS-1);
    char *s=QuestTemplates[index].name;
    
    for(int32_t i=0; i<31; i++)
        *(s++)=0;
        
    s=QuestTemplates[index].path;
    
    for(int32_t i=0; i<2048; i++)
        *(s++)=0;
}

void init_qts()
{
    for(int32_t i=0; i<MAXQTS; i++)
        reset_qt(i);
        
    strcpy(QuestTemplates[0].name,"New Default (can't change)");
    qt_count=1;
}

static DIALOG editqt_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,     32,   68,   257,  102,   vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Quest Template Editor", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_button_proc,     50,   140,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Load", NULL, NULL },
    { jwin_button_proc,     130,  140,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     210,  140,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       44,   103,   40,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    { jwin_edit_proc,       77,   99,   200,    16,      vc(12),  vc(1),  0,       0,          30,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       44,   118,  112,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Template File:", NULL, NULL },
    { jwin_text_proc,       44,   128,  233,  8,    vc(13),  vc(1),  0,       D_DISABLED,          0,             0,       NULL, NULL, NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0, (void *) onHelp, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void edit_qt()                                              //this is used to set the quest template for the current quest
{
    char tpath[2048];
    char tpath2[2048];
    strcpy(temppath, header.templatepath);
    
    if(temppath[0]==0)
    {
        getcwd(temppath,2048);
        fix_filename_case(temppath);
        fix_filename_slashes(temppath);
        put_backslash(temppath);
    }
    
    bool gotname;
    gotname=getname("Load Quest Template (.zqt)","zqt",NULL,temppath,true);
    
    if(gotname)
    {
        strcpy(tpath, temppath);
        chop_path(tpath);
        getcwd(tpath2,2048);
        fix_filename_case(tpath2);
        fix_filename_slashes(tpath2);
        put_backslash(tpath2);
        
        if(!strcmp(tpath, tpath2))
        {
            strcpy(header.templatepath, get_filename(temppath));
        }
        else
        {
            strcpy(header.templatepath, temppath);
        }
        
        if(!valid_zqt(temppath))
        {
            jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
            memset(header.templatepath, 0, 2048);
        }
    }
}

void edit_qt(int32_t index)
{
    int32_t ret;
    quest_template tqt;
    char tpath[2048];
    char tpath2[2048];
    tqt=QuestTemplates[index];
    editqt_dlg[0].dp2=get_zc_font(font_lfont);
    
    do
    {
        editqt_dlg[6].dp=QuestTemplates[index].name;
        editqt_dlg[8].dp=QuestTemplates[index].path;
        strcpy(temppath, QuestTemplates[index].path);
        bool gotname;
        
        large_dialog(editqt_dlg);
            
        ret=zc_popup_dialog(editqt_dlg,6);
        
        switch(ret)
        {
        case 2:
            gotname=getname("Load Quest Template (.zqt)","zqt",NULL,temppath,true);
            
            if(gotname)
            {
                strcpy(tpath, temppath);
                chop_path(tpath);
                getcwd(tpath2,2048);
                fix_filename_case(tpath2);
                fix_filename_slashes(tpath2);
                put_backslash(tpath2);
                
                if(!strcmp(tpath, tpath2))
                {
                    strcpy(QuestTemplates[index].path, get_filename(temppath));
                }
                else
                {
                    strcpy(QuestTemplates[index].path, temppath);
                }
            }
            
            break;
            
        case 3:
        
            if(!valid_zqt(temppath))
            {
                ret=2;
                jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
                break;
            }
            
            if(index==qt_count)
            {
                ++qt_count;
            }
            
            break;
            
        case 4:
            QuestTemplates[index]=tqt;
            break;
        }
    }
    while(ret==2);
}

const char *qtlist(int32_t index, int32_t *list_size)
{
    if(index>=0)
    {
        if(index>=qt_count)
            index=qt_count-1;
            
        return QuestTemplates[index].name;
    }
    
    *list_size=qt_count;
    return NULL;
}

static ListData qtlist_dlg_list(qtlist, &font);

static DIALOG qtlist_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,          0,      0,    303,    149,    vc(14),                vc(1),                  0,    D_EXIT,     0,           0, (void *) "Pick Quest Template",   NULL,   NULL  },
    { d_timer_proc,           0,      0,      0,      0,    0,                     0,                      0,    0,          0,           0,  NULL,                             NULL,   NULL  },
    { jwin_list_proc,         5,     26,    271,     87,    jwin_pal[jcBOXFG],     jwin_pal[jcLIGHT],      0,    D_EXIT,     0,           0, (void *) &qtlist_dlg_list,        NULL,   NULL  },
    { jwin_button_proc,     278,     44,     21,     21,    vc(14),                vc(1),                 13,    D_EXIT,     0,           0, (void *) "\x88",                  NULL,   NULL  },
    { jwin_button_proc,     278,     73,     21,     21,    vc(14),                vc(1),                 13,    D_EXIT,     0,           0, (void *) "\x89",                  NULL,   NULL  },
    { jwin_button_proc,      30,    120,     61,     21,    vc(14),                vc(1),                 13,    D_EXIT,     0,           0, (void *) "Edit",                  NULL,   NULL  },
    { jwin_button_proc,     110,    120,     61,     21,    vc(14),                vc(1),                 27,    D_EXIT,     0,           0, (void *) "OK",                    NULL,   NULL  },
    { jwin_button_proc,     190,    120,     61,     21,    vc(14),                vc(1),                 27,    D_EXIT,     0,           0, (void *) "Cancel",                NULL,   NULL  },
    { d_keyboard_proc,        0,      0,      0,      0,    0,                     0,                      0,    0,          KEY_DEL,     0, (void *) qtlist_del,              NULL,   NULL  },
    { NULL,                   0,      0,      0,      0,    0,                     0,                      0,    0,          0,           0,  NULL,                             NULL,   NULL  }
};

int32_t qtlist_del()
{
    if(qtlist_dlg[2].d1>0 && qtlist_dlg[2].d1<qt_count-1)
        return D_CLOSE;
        
    return D_O_K;
}

int32_t ListQTs(bool edit)
{
    qtlist_dlg[0].dp2=get_zc_font(font_lfont);
    int32_t index=0;
    quest_template *BackupQTs = (quest_template*)malloc(sizeof(quest_template)*MAXQTS);
    
    memcpy(BackupQTs,QuestTemplates,sizeof(quest_template)*qt_count);
    
    int32_t backup_qt_count=qt_count;
    
    while(index>-1)
    {
        bool hasroom=false;
        
        if(qt_count<MAXQTS)
        {
            hasroom=true;
            
            if(edit)
            {
                strcpy(QuestTemplates[qt_count++].name,"<new template>");
            }
        }
        
        large_dialog(qtlist_dlg);
            
        qtlist_dlg[2].x=int32_t(qtlist_dlg[0].x+(edit?5:15)*1.5);
        qtlist_dlg[3].proc=edit?jwin_button_proc:d_dummy_proc;
        qtlist_dlg[4].proc=edit?jwin_button_proc:d_dummy_proc;
        qtlist_dlg[5].proc=edit?jwin_button_proc:d_dummy_proc;
        qtlist_dlg[6].x=int32_t(qtlist_dlg[0].x+(edit?110:80)*1.5);
        qtlist_dlg[7].x=int32_t(qtlist_dlg[0].x+(edit?190:160)*1.5);
        qtlist_dlg[8].proc=edit?d_keyboard_proc:d_dummy_proc;
        
        int32_t ret=zc_popup_dialog(qtlist_dlg,2);
        
        index=qtlist_dlg[2].d1;
        
        int32_t doedit=false;
        
        switch(ret)
        {
        case 2:
            if(edit)
            {
                doedit=true;
            }
            else
            {
                if(index>0&&!valid_zqt(QuestTemplates[index].path))
                {
                    jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
                }
                else
                {
                    strcpy(header.templatepath, QuestTemplates[index].path);
                    index=-1;
                }
            }
            
            break;
            
        case 5:
            doedit=true;
            break;
            
        case 3:
            if(index>1&&index<qt_count-1)
            {
                zc_swap(QuestTemplates[index],QuestTemplates[index-1]);
                --qtlist_dlg[2].d1;
                index=qtlist_dlg[2].d1;
            }
            
            break;
            
        case 4:
            if(index>0&&index<qt_count-2)
            {
                zc_swap(QuestTemplates[index],QuestTemplates[index+1]);
                ++qtlist_dlg[2].d1;
                index=qtlist_dlg[2].d1;
            }
            
            break;
            
        case 6:
            if(index>0&&!valid_zqt(QuestTemplates[index].path))
            {
                jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
            }
            else
            {
                if(!edit)
                {
                    strcpy(header.templatepath, QuestTemplates[index].path);
                }
                
                index=-1;
            }
            
            break;
            
        case 0:
        case 7:
            if(edit)
            {
                qt_count=backup_qt_count+1;
                memcpy(QuestTemplates,BackupQTs,sizeof(quest_template)*qt_count);
            }
            
            index=-2;
            break;
            
        case 8:
            char buf[30];
            strncpy(buf,QuestTemplates[index].name,30);
            
            if(jwin_alert("Confirm Deletion", "Delete this quest template?",buf,"(The file will still exist.)","Yes","No",'y',27,get_zc_font(font_lfont))==1)
            {
                for(int32_t i=index; i<MAXQTS-1; i++)
                    QuestTemplates[i]=QuestTemplates[i+1];
                    
                reset_qt(MAXQTS-1);
                --qt_count;
            }
            
            break;
        }
        
        if(edit&&hasroom)
        {
            //      strcpy(QuestTemplates[--qt_count].name,"              ");
            reset_qt(--qt_count);
            sprintf(QuestTemplates[qt_count].name,"Untitled");
        }
        
        if(index>0 && doedit)
        {
            edit_qt(index);
        }
    }
    
    free(BackupQTs);
    return index;
}

int32_t onQuestTemplates()
{
    ListQTs(true);
    return D_O_K;
}

int32_t NewQuestFile(int32_t template_slot)
{
    memset(filepath,0,255);
    memset(temppath,0,255);
    first_save=false;
    box_start(1, "Initializing Quest", get_zc_font(font_lfont), get_zc_font(font_pfont), false);
    box_out("Please wait.");
    box_eol();
    box_out("This may take a few moments.");
    box_eol();
    
    init_quest(QuestTemplates[template_slot].path);
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

void alwaysOnRules()
{
	// Engine fixes
	set_bit(quest_rules, qr_DMGCOMBOLAYERFIX, 1);
	set_bit(quest_rules, qr_WARPSIGNOREARRIVALPOINT, 1);
	set_bit(quest_rules, qr_NOARRIVALPOINT, 1);
	set_bit(quest_rules, qr_CMBCYCLELAYERS, 1);
	set_bit(quest_rules, qr_KILLALL, 1);
	set_bit(quest_rules, qr_ITEMBUBBLE, 1);
	set_bit(quest_rules, qr_RINGAFFECTDAMAGE, 1);
	set_bit(quest_rules, qr_NEW_COMBO_ANIMATION, 1);
	set_bit(quest_rules, qr_BUSHESONLAYERS1AND2, 1);
	set_bit(quest_rules, qr_CUSTOMCOMBOSLAYERS1AND2, 1);
	set_bit(quest_rules, qr_CUSTOMCOMBOS_EVERY_LAYER, 1);
	set_bit(quest_rules, qr_HOOKSHOTLAYERFIX, 1);
	set_bit(quest_rules, qr_EPILEPSY, 1);
	set_bit(quest_rules, qr_FIXSCRIPTSDURINGSCROLLING, 1);
	set_bit(quest_rules, qr_ALWAYS_DEALLOCATE_ARRAYS, 1);
	set_bit(quest_rules, qr_ITEMSCRIPTSKEEPRUNNING, 1);
	set_bit(quest_rules, qr_FIXDRUNKINPUTS, 1);
	set_bit(quest_rules, qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE, 1);
	set_bit(quest_rules, qr_SMARTER_WATER, 1);
	set_bit(quest_rules, qr_LESS_AWFUL_SIDESPIKES, 1);
	set_bit(quest_rules, qr_SWITCHES_AFFECT_MOVINGBLOCKS, 1);
	// Script Logging
	set_bit(quest_rules, qr_SCRIPTERRLOG, 1);
	set_bit(quest_rules, qr_LOG, 1);
	set_bit(quest_rules, qr_TRACESCRIPTIDS, 1);
	// Misc Engine stuff (not really related to any particular ruleset, so, always on)
	set_bit(quest_rules, qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT, 1);
	set_bit(quest_rules, qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN, 1);
	set_bit(quest_rules, qr_COMBOSCRIPTS_LAYER_0, 1);
	set_bit(quest_rules, qr_ONDEATH_RUNS_AFTER_DEATH_ANIM, 1);
	set_bit(quest_rules, qr_FFCSCROLL, 1);
	set_bit(quest_rules, qr_GREATER_MAX_TIME, 1);
	/* currently bugged: wBrang Lv2 and above. --I think we fixed this? -Z 8th Feb, 2020 */
	set_bit(quest_rules, qr_WEAPONS_EXTRA_FRAME, 1);
	set_bit(quest_rules, qr_USE_EX1_EX2_INVENTORYSWAP, 1);
	set_bit(quest_rules, qr_CLEARINITDONSCRIPTCHANGE, 1);
	set_bit(quest_rules, qr_SCRIPT_WEAPONS_UNIQUE_SPRITES, 1); //Give Script 1 to 10 weapons, generated by ZScript unique sprites.
	set_bit(quest_rules, qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES, 1);
	set_bit(quest_rules, qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS, 1);
	set_bit(quest_rules, qr_SMARTER_SMART_SCROLL, 1);
}

int32_t onStrFix();
void applyRuleset(int32_t newRuleset)
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
				set_bit(quest_rules,i,0);
		}
	}
	onStrFix();
	
	alwaysOnRules(); //Set on things that should ALWAYS be on.
	
	int32_t fixesrules[] =
	{
		qr_FREEFORM, qr_SAFEENEMYFADE, qr_ITEMSONEDGES, qr_HERODUNGEONPOSFIX, qr_RLFIX,
		qr_NOLEVEL3FIX, qr_BOMBHOLDFIX, qr_HOLDNOSTOPMUSIC, qr_CAVEEXITNOSTOPMUSIC,
		qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX, /*qr_WPNANIMFIX,*/ qr_PUSHBLOCKCSETFIX,
		qr_TRAPPOSFIX, qr_NOBORDER, qr_OLDPICKUP, qr_SUBSCREENOVERSPRITES,
		qr_BOMBDARKNUTFIX, qr_OFFSETEWPNCOLLISIONFIX, qr_ITEMSINPASSAGEWAYS, qr_NOFLICKER, qr_FIREPROOFHERO2,
		qr_NOITEMOFFSET, qr_LADDERANYWHERE, qr_TRUEFIXEDBOMBSHIELD, qr_NOTMPNORET, qr_NOFLASHDEATH, qr_BROKENSTATUES, 
		qr_DYING_ENEMIES_IGNORE_STUN, qr_SHOP_ITEMS_VANISH, qr_EXPANDEDLTM, qr_CORRECTED_EW_BRANG_ANIM, -1
	};
	if(ruleset != rulesetNES) //Rules for all non-'Authentic NES' rulesets
	{
		for(int32_t i=0; fixesrules[i]!=-1; ++i)
		{
			if(fixesrules[i]!=qr_OLDPICKUP)
				set_bit(quest_rules, fixesrules[i], 1);
		}
	}
	
	switch(ruleset)
	{
		case rulesetNES: // Authentic NES
		{
			set_bit(quest_rules, qr_OLDPICKUP, 1);
			set_bit(quest_rules, qr_OLDSTYLEWARP, 1);
			set_bit(quest_rules, qr_OLDSPRITEDRAWS, 1);
			break;
		}
		case rulesetFixedNES: // Fixed NES
		{
			//Nothing to put here; NES fixes are checked above the switch.
			break;
		}
		case rulesetBSZ: // BS Zelda
		{
			set_bit(quest_rules, qr_TIME, 1);
			set_bit(quest_rules, qr_NOBOMBPALFLASH, 1);
			set_bit(quest_rules, qr_NEWENEMYTILES, 1);
			set_bit(quest_rules, qr_FASTDNGN, 1);
			set_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING, 1);
			set_bit(quest_rules, qr_COOLSCROLL, 1);
			set_bit(quest_rules, qr_BSZELDA, 1);
			set_bit(quest_rules, qr_SOLIDBLK, 1);
			set_bit(quest_rules, qr_HESITANTPUSHBLOCKS, 1);
			set_bit(quest_rules, qr_INSTABURNFLAGS, 1);
			set_bit(quest_rules, qr_FADE, 1); // Interpolated fading
			set_bit(quest_rules, qr_EXPANDEDLTM, 1);
			set_bit(quest_rules, qr_OLDSPRITEDRAWS, 1);
			break;
		}
		case rulesetZ3: // Zelda 3-esque
		{
			//{From BSZ
			set_bit(quest_rules, qr_TIME, 1);
			set_bit(quest_rules, qr_NOBOMBPALFLASH, 1);
			set_bit(quest_rules, qr_NEWENEMYTILES, 1);
			set_bit(quest_rules, qr_FASTDNGN, 1);
			set_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING, 1);
			set_bit(quest_rules, qr_COOLSCROLL, 1);
			set_bit(quest_rules, qr_BSZELDA, 1);
			set_bit(quest_rules, qr_SOLIDBLK, 1);
			set_bit(quest_rules, qr_HESITANTPUSHBLOCKS, 1);
			set_bit(quest_rules, qr_INSTABURNFLAGS, 1);
			set_bit(quest_rules, qr_FADE, 1); // Interpolated fading
			set_bit(quest_rules, qr_EXPANDEDLTM, 1);
			set_bit(quest_rules, qr_OLDSPRITEDRAWS, 1);
			//}
			//{Z3
			//Make water drownable
			set_bit(quest_rules, qr_DROWN, 1);
			set_bit(quest_rules, qr_HIDECARRIEDITEMS, 1);
			set_bit(quest_rules, qr_ALLOWMSGBYPASS, 1);
			set_bit(quest_rules, qr_ALLOWFASTMSG, 1);
			set_bit(quest_rules, qr_MSGDISAPPEAR, 1);
			set_bit(quest_rules, qr_MSGFREEZE, 1);
			//set_bit(quest_rules, qr_VERYFASTSCROLLING, 1); //People apparently do not like this one.
			set_bit(quest_rules, qr_ENABLEMAGIC, 1);
			set_bit(quest_rules, qr_NOWANDMELEE, 1);
			set_bit(quest_rules, qr_TRUEARROWS, 1);
			set_bit(quest_rules, qr_Z3BRANG_HSHOT, 1);
			set_bit(quest_rules, qr_TRANSSHADOWS, 1);
			set_bit(quest_rules, qr_SHADOWS, 1);
			set_bit(quest_rules, qr_LTTPWALK, 1);
			set_bit(quest_rules, qr_MORESOUNDS, 1);
			set_bit(quest_rules, qr_NEVERDISABLEAMMOONSUBSCREEN, 1);
			//}
			break;
		}
		case rulesetModern: // Modern-style
		{
			//{from BSZ
			set_bit(quest_rules, qr_TIME, 1);
			set_bit(quest_rules, qr_NOBOMBPALFLASH, 1);
			set_bit(quest_rules, qr_NEWENEMYTILES, 1);
			set_bit(quest_rules, qr_FASTDNGN, 1);
			set_bit(quest_rules, qr_SMOOTHVERTICALSCROLLING, 1);
			set_bit(quest_rules, qr_COOLSCROLL, 1);
			set_bit(quest_rules, qr_BSZELDA, 1);
			set_bit(quest_rules, qr_SOLIDBLK, 1);
			set_bit(quest_rules, qr_HESITANTPUSHBLOCKS, 1);
			set_bit(quest_rules, qr_INSTABURNFLAGS, 1);
			set_bit(quest_rules, qr_FADE, 1); // Interpolated fading
			set_bit(quest_rules, qr_EXPANDEDLTM, 1);
			set_bit(quest_rules, qr_OLDSPRITEDRAWS, 1);
			//}
			//{from Z3
			//Make water drownable
			set_bit(quest_rules, qr_DROWN, 1);
			set_bit(quest_rules, qr_HIDECARRIEDITEMS, 1);
			set_bit(quest_rules, qr_ALLOWMSGBYPASS, 1);
			set_bit(quest_rules, qr_ALLOWFASTMSG, 1);
			set_bit(quest_rules, qr_MSGDISAPPEAR, 1);
			set_bit(quest_rules, qr_MSGFREEZE, 1);
			//set_bit(quest_rules, qr_VERYFASTSCROLLING, 1); //People apparently do not like this one.
			set_bit(quest_rules, qr_ENABLEMAGIC, 1);
			set_bit(quest_rules, qr_NOWANDMELEE, 1);
			set_bit(quest_rules, qr_TRUEARROWS, 1);
			set_bit(quest_rules, qr_Z3BRANG_HSHOT, 1);
			set_bit(quest_rules, qr_TRANSSHADOWS, 1);
			set_bit(quest_rules, qr_SHADOWS, 1);
			set_bit(quest_rules, qr_LTTPWALK, 1);
			set_bit(quest_rules, qr_MORESOUNDS, 1);
			set_bit(quest_rules, qr_NEVERDISABLEAMMOONSUBSCREEN, 1);
			//}
			//{Modern
			set_bit(quest_rules, qr_OLDSPRITEDRAWS, 0);
			set_bit(quest_rules, qr_NO_SCROLL_WHILE_IN_AIR, 1);
			set_bit(quest_rules, qr_DUNGEON_DMAPS_PERM_SECRETS, 1);
			set_bit(quest_rules, qr_ANGULAR_REFLECTED_WEAPONS, 1);
			set_bit(quest_rules, qr_MIRRORS_USE_WEAPON_CENTER, 1);
			set_bit(quest_rules, qr_SPRITEXY_IS_FLOAT, 1);
			set_bit(quest_rules, qr_SIDEVIEWLADDER_FACEUP, 1);
			set_bit(quest_rules, qr_ITEMSHADOWS, 1);
			set_bit(quest_rules, qr_WEAPONSHADOWS, 1);
			set_bit(quest_rules, qr_NEW_HERO_MOVEMENT, 1);
			//set_bit(quest_rules, qr_STEP_IS_FLOAT, 1); //Step is broken in A72, needs to be fixed before we can use this. 
			set_bit(quest_rules, qr_HOLDITEMANIMATION, 1);
			set_bit(quest_rules, qr_DISABLE_4WAY_GRIDLOCK, 1);
			set_bit(quest_rules, qr_NO_HOPPING, 1);
			set_bit(quest_rules, qr_NO_SOLID_SWIM, 1);
			set_bit(quest_rules, qr_WATER_ON_LAYER_1, 1);
			set_bit(quest_rules, qr_WATER_ON_LAYER_2, 1);
			set_bit(quest_rules, qr_SHALLOW_SENSITIVE, 1);
			set_bit(quest_rules, qr_NONHEAVY_BLOCKTRIGGER_PERM, 1);
			set_bit(quest_rules, qr_CORRECTED_EW_BRANG_ANIM, 1);
			set_bit(quest_rules, qr_OVERHEAD_COMBOS_L1_L2, 1);
			set_bit(quest_rules, qr_AUTOCOMBO_LAYER_1, 1);
			set_bit(quest_rules, qr_AUTOCOMBO_LAYER_2, 1);
			set_bit(quest_rules, qr_AUTOCOMBO_ANY_LAYER, 1);
			set_bit(quest_rules, qr_NEW_DARKROOM, 1);
			set_bit(quest_rules, qr_PUSHBLOCK_LAYER_1_2, 1);
			set_bit(quest_rules, qr_HOOKSHOTALLLAYER, 1);
			set_bit(quest_rules, qr_ITEMCOMBINE_NEW_PSTR, 1);
			set_bit(quest_rules, qr_ITEMCOMBINE_CONTINUOUS, 1);
			set_bit(quest_rules, qr_FAIRYDIR, 1);
			set_bit(quest_rules, qr_BLOCKS_DONT_LOCK_OTHER_LAYERS, 1);
			set_bit(quest_rules, qr_CONVEYORS_L1_L2, 1);
			//}
			break;
		}
	}
}

void call_ruleset_dlg();
void call_ruletemplate_dlg();
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
		
	/*
	int32_t ret=ListQTs(false);
	if (ret==-2)
	{
		return D_O_K;
	}
	*/
	int32_t ret=0;
	NewQuestFile(ret);
	set_bit(quest_rules, qr_PARSER_SHORT_CIRCUIT, 1);
	set_bit(quest_rules, qr_PARSER_TRUE_INT_SIZE, 1);
	set_bit(quest_rules, qr_ANIMATECUSTOMWEAPONS, 0); //always OFF
	alwaysOnRules();
	if(RulesetDialog > 0)
	{
		PickRuleset();
		PickRuleTemplate();
	}
	if(zc_get_config("zquest","auto_filenew_bugfixes",1))
	{
		applyRuleTemplate(ruletemplateCompat);
	}
	return D_O_K;
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
    
    int32_t ret = save_quest(filepath, false);
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
    if(disable_saving)
    {
        jwin_alert("ZQuest","Saving is","disabled in this version.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
        return D_O_K;
    }
#ifdef __EMSCRIPTEN__
		if(!getname("Save Quest As (.qst)","qst",NULL,get_initial_file_dialog_folder().c_str(),true))
        return D_O_K;
#else
		if(!getname("Save Quest As (.qst)","qst",NULL,filepath,true))
        return D_O_K;
#endif
        
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
    
    int32_t ret = save_quest(temppath, false);
    char buf[1024],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(!ret)
    {
        strcpy(filepath,temppath);
		update_recent_quest(temppath);
        sprintf(buf,"ZQuest - [%s]", get_filename(filepath));
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
	bool compressed=true;
	bool encrypted=true;
	char ext[2048];
	char ext2[5];
	strcpy(ext,get_extension(path));
	strupr(ext);
	
	for(int32_t i=0; i<10; ++i)
	{
		sprintf(ext2,"qu%d",i);
		
		if(stricmp(ext,ext2)==0)
		{
			compressed=false;
			encrypted=false;
			break;
		}
	}
	
	if(stricmp(ext,"qsu")==0)
	{
		compressed=false;
		encrypted=false;
	}
	
	int32_t ret = load_quest(path, compressed, encrypted);
	
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
int32_t onOpen()
{
	restore_mouse();
	
	if(checksave()==0)
		return D_O_K;
		
	static EXT_LIST list[] =
	{
		{ (char *)"Quest Files (*.qst)", (char *)"qst"                                     },
		{ (char *)"Unencrypted Quest Files (*.qsu)", (char *)"qsu"                                     },
		{ (char *)"Quest Auto-backups (*.qb?)", (char *)"qb0,qb1,qb2,qb3,qb4,qb5,qb6,qb7,qb8,qb9" },
		{ (char *)"Quest Timed Auto-saves (*.qt?)", (char *)"qt0,qt1,qt2,qt3,qt4,qt5,qt6,qt7,qt8,qt9" },
		{ (char *)"Uncompressed Quest Timed Auto-saves (*.qu?)", (char *)"qu0,qu1,qu2,qu3,qu4,qu5,qu6,qu7,qu8,qu9" },
		{ (char *)"All Files (*.*)",                             NULL                                              },
		{ NULL,                                                  NULL                                              }
	};
	
#ifdef __EMSCRIPTEN__
		if(!getname("Load File",NULL,list,get_initial_file_dialog_folder().c_str(),true))
			return D_O_K;
#else
		if(!getname("Load File",NULL,list,filepath,true))
			return D_O_K;
#endif
	
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
        int32_t ret = load_quest(filepath, true, true);
        
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
        NewQuestFile(0);
        
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
        
    if(zc_popup_dialog(import_map_bias_dlg,2)==2)
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
    
    if(!getname("Import Map (.map)","map",NULL,datapath,false))
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
        
    if(!getname("Export Map (.map)","map",NULL,datapath,false))
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

int32_t onImport_DMaps_old()
{
    if(!getname("Import DMaps (.dmp)","dmp",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    if(!load_dmaps(temppath,0))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    else
    {
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
    }
    
    return D_O_K;
}


int32_t onExport_Tilepack()
{
	savesometiles("Save Tile Package", 0);
	return D_O_K;	
}

int32_t onAbout_Module()
{
	about_module("About Module (.zmod)", 0);
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
    if(!getname("Import DMaps (.zdmap)","zdmap",NULL,datapath,false))
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
		if(getname("Load ZTILE(.ztile)", "ztile", NULL,datapath,false))
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
		if(getname("Load ZALIAS(.zalias)", "zalias", NULL,datapath,false))
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

int32_t onExport_DMaps_old()
{
    if(!getname("Export DMaps (.dmp)","dmp",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_dmaps(temppath))
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

int32_t onImport_Pals()
{
    if(!getname("Import Palettes (.zpl)","zpl",NULL,datapath,false))
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
    if(!getname("Export Palettes (.zpl)","zpl",NULL,datapath,false))
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
    if(!getname("Import String Table (.zqs)","zqs",NULL,datapath,false))
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
    if(!getname("Export String Table (.zqs)","zqs",NULL,datapath,false))
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


int32_t onExport_MsgsText()
{
    if(!getname("Export Text Dump (.txt)","txt",NULL,datapath,false))
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

int32_t onImport_Combos_old()
{
    int32_t ret=getnumber("Import Start Page",0);
    
    if(cancelgetnum)
    {
        return D_O_K;
    }
    
    bound(ret,0,COMBO_PAGES-1);
    
    if(!getname("Import Combo Table (.cmb)","cmb",NULL,datapath,false))
        return D_O_K;
        
    if(!load_combos(temppath, ret*COMBOS_PER_PAGE))
    {
        // if(!load_combos(temppath)) {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    else
        saved=false;
        
    refresh(rALL);
    return D_O_K;
}


int32_t onExport_Combos()
{
    if(!getname("Export Combos (.zcombo)","zcombo",NULL,datapath,false))
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

int32_t onExport_Combos_old()
{
    if(!getname("Export Combo Table (.cmb)","cmb",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_combos(temppath))
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

int32_t onImport_Tiles()
{
    int32_t ret=getnumber("Import Start Page",0);
    
    if(cancelgetnum)
    {
        return D_O_K;
    }
    
    bound(ret,0,TILE_PAGES-1);
    
    if(!getname("Import Tiles (.ztileset)","ztileset",NULL,datapath,false))
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
    if(!getname("Export Tiles (.ztileset)","ztileset",NULL,datapath,false))
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
    if(!getname("Import Enemies (.guy)","guy",NULL,datapath,false))
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
    if(!getname("Export Enemies (.guy)","guy",NULL,datapath,false))
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
    if(!getname("Import Combo Alias (.zca)","zca",NULL,datapath,false))
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
    if(!getname("Export Combo Alias (.zca)","zca",NULL,datapath,false))
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
    if(!getname("Import Graphics Pack (.zgp)","zgp",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    // usetiles=true;
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
    if(!getname("Export Graphics Pack (.zgp)","zgp",NULL,datapath,false))
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

int32_t onImport_Subscreen()
{
    if(!getname("Import Subscreen (.sub)","sub",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    
    // usetiles=true;
    if(!load_subscreen(temppath))
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    refresh(rALL);
    return D_O_K;
}

int32_t onExport_Subscreen()
{
    if(!getname("Export Subscreen (.sub)","sub",NULL,datapath,false))
        return D_O_K;
        
    bool cancel;
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(save_subscreen(temppath, &cancel))
    {
        if(!cancel)
        {
            sprintf(buf,"ZQuest");
            sprintf(buf2,"Saved %s",name);
        }
        else
        {
            sprintf(buf,"ZQuest");
            sprintf(buf2,"Did not save %s",name);
        }
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onImport_ZQT()
{
    if(!getname("Import Quest Template (.zqt)","zqt",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    // usetiles=true;
    int32_t error = load_quest(temppath,true, false);
    
    if(error != qe_OK && error != qe_cancel)
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    register_blank_tiles();
    loadlvlpal(Color);
    setup_combo_animations();
    setup_combo_animations2();
    Map.setCurrMap(Map.getCurrMap());                         // for bound checking
    refresh(rALL);
    refresh_pal();
    return D_O_K;
}

int32_t onExport_ZQT()
{
    if(!getname("Export Quest Template (.zqt)","zqt",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(!save_unencoded_quest(temppath, true))
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

int32_t onImport_UnencodedQuest()
{
    if(!getname("Import Unencoded Quest (.qsu)","qsu",NULL,datapath,false))
        return D_O_K;
        
    saved=false;
    // usetiles=true;
    int32_t ret = load_quest(temppath,false,false);
    
    if(ret != qe_OK && ret != qe_cancel)
    {
        char buf[256+20],name[256];
        extract_name(temppath,name,FILENAMEALL);
        sprintf(buf,"Unable to load %s",name);
        jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    }
    
    register_blank_tiles();
    loadlvlpal(Color);
    setup_combo_animations();
    setup_combo_animations2();
    Map.setCurrMap(Map.getCurrMap());                         // for bound checking
    refresh(rALL);
    refresh_pal();
    return D_O_K;
}

int32_t onExport_UnencodedQuest()
{
    if(!getname("Export Unencoded Quest (.qsu)","qsu",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAMEALL);
    
    if(!save_unencoded_quest(temppath, false))
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
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	int32_t doorscount = 0;
	DoorComboSet tempDoorComboSet;
	memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
	int32_t lastset = 0;
	int32_t firstset = 0;
	int32_t last = 0;
	int32_t ret = 1;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&doorscount,f,true))
	{
		return 0;
	}
	al_trace("doorscount is: %d\n", doorscount);
	if(!p_igetl(&firstset,f,true))
	{
		return 0;
	}
	if(!p_igetl(&last,f,true))
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
	
	else if ( ( section_version > V_DOORS ) || ( section_version == V_DOORS && section_cversion > CV_DOORS ) )
	{
		al_trace("Cannot read .zdoors packfile made using V_DOORS (%d) subversion (%d)\n", section_version, section_cversion);
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
		if(!pfread(&tempDoorComboSet.name,sizeof(tempDoorComboSet.name),f,true))
		{
			return 0;
		}
		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f,true))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f,true))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f,true))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f,true))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f,true))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.flags[j],f,true))
			{
				return 0;
			}
		}
		memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
	}
	return ret;
}


int32_t writezdoorsets(PACKFILE *f, int32_t first = 0, int32_t count = door_combo_set_count)
{
	dword section_version=V_DOORS;
	dword section_cversion=CV_DOORS;
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
	if(!p_iputw(section_cversion,f))
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
		if(!pfwrite(&DoorComboSets[i].name,sizeof(DoorComboSets[0].name),f))
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
	dword section_cversion=CV_DOORS;
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
	if(!p_iputw(section_cversion,f))
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
		if(!pfwrite(&DoorComboSets[index].name,sizeof(DoorComboSets[0].name),f))
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
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	int32_t doorscount = 0;
	DoorComboSet tempDoorComboSet;
	memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
	int32_t firstset = 0;
	int32_t last = 0;
	int32_t ret = 1;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&doorscount,f,true))
	{
		return 0;
	}
	al_trace("doorscount is: %d\n", doorscount);
	if(!p_igetl(&firstset,f,true))
	{
		return 0;
	}
	if(!p_igetl(&last,f,true))
	{
		return 0;
	}
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zdoors packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_DOORS ) || ( section_version == V_DOORS && section_cversion > CV_DOORS ) )
	{
		al_trace("Cannot read .zdoors packfile made using V_DOORS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zdoors packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	//section data for doors
	{
		//if(i+deststart >= door_combo_set_count)
		//{
		//	al_trace("Reached the current door count trying to import doorsets.\n");
		//	ret = 2; break;
		//}
		//al_trace("Door readcycle %d\n", i-deststart);
		//Clear per set
		memset(&tempDoorComboSet, 0, sizeof(DoorComboSet));
		//name
		if(!pfread(&tempDoorComboSet.name,sizeof(tempDoorComboSet.name),f,true))
		{
			return 0;
		}
		//up door
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int32_t j=0; j<9; j++)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f,true))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int32_t j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f,true))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f,true))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f,true))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int32_t j=0; j<4; j++)
		{
			if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f,true))
			{
				return 0;
			}
		}
		for(int32_t j=0; j<4; j++)
		{
			if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f,true))
			{
				return 0;
			}
		}
		//flags
		for(int32_t j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.flags[j],f,true))
			{
				return 0;
			}
		}
		memcpy(&DoorComboSets[index], &tempDoorComboSet, sizeof(tempDoorComboSet));
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
    jwin_center_dialog(editqt_dlg);
    jwin_center_dialog(ruleset_dlg);
    jwin_center_dialog(import_map_bias_dlg);
    jwin_center_dialog(qtlist_dlg);
}

