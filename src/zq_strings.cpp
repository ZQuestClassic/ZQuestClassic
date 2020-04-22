#include "zq_strings.h"
#include "editbox.h"
#include "EditboxNew.h"
#include "gui.h"
#include "jwin.h"
#include "qst.h"
#include "tiles.h"
#include "zc_alleg.h"
#include "zc_malloc.h"
#include "zdefs.h"
#include "zq_custom.h"
#include "zq_misc.h"
#include "zq_tiles.h"
#include "zqscale.h"
#include "zquest.h"
#include "zsys.h"
#include <map>
#include <string>

void editmsg(int index, int addAfter);
int strlist_del();
int addtomsglist(int index);
void build_bistringcat_list();
const char *stringcatlist(int index, int *list_size);
char *parse_msg_str(char *s);
int msg_code_operands(int cc);
int d_msg_preview_proc(int msg,DIALOG *d,int c);
int d_msg_edit_proc(int msg,DIALOG *d,int c);
int d_msgtile_proc(int msg,DIALOG *d,int c);
void strlist_rclick_func(int index, int x, int y);


std::map<int, int> msglistcache;

static ListData strlist_dlg_list(msgslist, &font);
static ListData stringcat_dlg_list(stringcatlist, &font);
MsgStr *curmsgstr = NULL;
int msg_x = 0;
int msg_y = 0;
int msgtile = 0;
int msgcset = 0;
char msgbuf[MSGSIZE*3];

int bistringcat[256]; // A dropdown menu containing all strings which begin with '--', which serves as a quick shortcut to large string blocks.
int bistringcat_cnt=-1;

// Dialogs

static int editmsg_string_list[] =
{ 2, 3, 4, 5, 8, 17, 18, 29, 32, -1 };

static int editmsg_attributes_list[] =
{ 9, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27,28, 33, 34, 35, 37, 38, 39, 40, 41, 42, 43, 44, -1 };

static int editmsg_portrait_list[] =
{ 45, 46, 47, 48, 49, 50, 51, 52, 53, -1 };

static TABPANEL editmsg_tabs[] =
{
    // (text)
    { (char*)"String",      D_SELECTED,  editmsg_string_list,      0,  NULL },
    { (char*)"Attributes",     0,        editmsg_attributes_list,     0,  NULL },
    { (char*)"Portrait",     0,        editmsg_portrait_list,     0,  NULL },
    { NULL,             0,           NULL,                         0,  NULL }
};

DIALOG strlist_dlg[] =
{
    // (dialog proc)       (x)     (y)      (w)     (h)     (fg)    (bg) (key)  (flags)    (d1)            (d2)        (dp)
    { jwin_win_proc,        20,      0, 200+72, 148+76,  vc(14),  vc(1),    0,  D_EXIT,       0,             0, (void *) "Edit Message String", NULL, NULL },
    { d_timer_proc,          0,      0,      0,      0,       0,      0,    0,       0,       0,             0,       NULL, NULL, NULL },
    { jwin_list_proc,       34,   21+8, 176+49,  131+3,jwin_pal[jcTEXTFG],jwin_pal[jcTEXTBG],0,D_EXIT|(D_USER<<1),0,     0, (void *)&strlist_dlg_list, NULL, (void *) &strlist_rclick_func },
    { jwin_button_proc,     40, 163+36,     71,     21,   vc(14),  vc(1),  13,  D_EXIT,       0,             0, (void *) "Edit", NULL, NULL },
    { jwin_button_proc,    200, 163+36,     71,     21,   vc(14),  vc(1),  27,  D_EXIT,       0,             0, (void *) "Done", NULL, NULL },
    { d_keyboard_proc,       0,      0,      0,      0,       0,     0,     0,       0, KEY_DEL,             0, (void *) strlist_del, NULL, NULL },
    
    { jwin_text_proc,       32,  165+4,    128,      8,   vc(15),  vc(1),   0,       0,       0,             0, (void *) "Message More X, Y: ", NULL, NULL },
    { jwin_edit_proc,      128,    165,     26,     16,   vc(12),  vc(1),   0,       0,       3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      158,    165,     26,     16,   vc(12),  vc(1),   0,       0,       3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       32, 165+22,    128,      8,   vc(15),  vc(1),   0,       0,       0,             0, (void *) "Default Text Speed: ", NULL, NULL },
    { jwin_edit_proc,      128, 165+18,     26,     16,   vc(12),  vc(1),   0,       0,       3,             0,       NULL, NULL, NULL },
    
    { jwin_button_proc,    266,     74,     16,     16,   vc(14),  vc(1),  27,  D_EXIT,       0,             0, (void *) "\x88", NULL, NULL },
    { jwin_button_proc,    266,     99,     16,     16,   vc(14),  vc(1),  27,  D_EXIT,       0,             0, (void *) "\x89", NULL, NULL },
    { jwin_button_proc,    266,     44,     16,     16,   vc(14),  vc(1),  27,  D_EXIT,       0,             0, (void *) "\x88", NULL, NULL },
    { jwin_button_proc,    266,    129,     16,     16,   vc(14),  vc(1),  27,  D_EXIT,       0,             0, (void *) "\x89", NULL, NULL },
    //15
    { jwin_check_proc,     188,    168,     0,      9,   vc(14),  vc(1),   0,       0,        1,             0, (void *) "Y is Offset", NULL, NULL },
    { jwin_button_proc,    120, 163+36,     71,     21,   vc(14),  vc(1),  13,  D_EXIT,       0,             0, (void *) "Add New Here", NULL, NULL },
    //17
    { jwin_droplist_proc,  34,   16,   161,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],0,D_EXIT,0,       0, (void *) &stringcat_dlg_list, NULL, NULL },
    { jwin_button_proc,    34+161+2,   18,     12,     12,   vc(14),  vc(1),  13,     D_EXIT, 0,             0, (void *) "?", NULL, NULL },
    { d_keyboard_proc,       0,      0,      0,      0,        0,     0,   'c',      0,       0,             0, (void *) close_dlg, NULL, NULL },
    { d_keyboard_proc,       0,      0,      0,      0,        0,     0,   'v',      0,       0,             0, (void *) close_dlg, NULL, NULL },
    //21
    { jwin_text_proc,      158, 165+22,    128,      8,   vc(15),  vc(1),   0,       0,       0,             0, (void *) "Template: ", NULL, NULL },
    { jwin_edit_proc,      204, 165+18,     36,     16,   vc(12),  vc(1),   0,       0,       5,             0,       NULL, NULL, NULL },
    { NULL,                  0,      0,      0,      0,        0,     0,     0,      0,       0,             0,       NULL,  NULL,  NULL }
};

DIALOG editmsg_dlg[] =
{

    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc, 44,   0,   296,  220,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,    50,     24,   284,  164,  jwin_pal[jcBOXFG], jwin_pal[jcBOX],  0,  0,    0,    0, (void *) editmsg_tabs,  NULL, (void *)editmsg_dlg  },
    { jwin_frame_proc,        53,  89-9,   278,  54,   vc(14),  vc(1),  0,       0,          FR_DEEP,             0,       NULL, NULL, NULL },
    { d_msg_edit_proc,       61,   48,   240,  16,    vc(12),  vc(1),  0,       0,          MSGSIZE*3,            0,       NULL, NULL, NULL },
    { jwin_text_proc,       52,   158,  168,  8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Next string:", NULL, NULL },
    // 5
    { jwin_droplist_proc,      110,  154,  158,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_button_proc,     90,   187+6,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     170,  187+6,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { d_msg_preview_proc,   56,   92-9,   272,  48,   jwin_pal[jcTEXTBG],  jwin_pal[jcTEXTFG],  0,       0,          0,             0,       msgbuf, NULL, NULL },
    { jwin_edit_proc,      80,  66,  28+1,  16,    vc(12),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
    // 10
    { jwin_ctext_proc,     64,   70,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Y:", NULL, NULL },
    { jwin_frame_proc,     58,   102,    20,  20,    vc(14),  vc(1),  0,       0,          FR_DEEP,      0,       NULL, NULL, NULL },
    { d_msgtile_proc,      60,  104,    16,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      80,  84,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     64,   88,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "SFX:", NULL, NULL },
    // 15
    { jwin_edit_proc,      80,  48,  28+1,  16,    vc(12),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     64,   52,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "X:", NULL, NULL },
    { jwin_text_proc,       52,   140,  168,  8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "String font:", NULL, NULL },
    { jwin_droplist_proc,      110,  136,  158,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       NULL, NULL, NULL },
	{ jwin_edit_proc,      160,  48,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    // 20
    { jwin_ctext_proc,     136,   52,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Width:", NULL, NULL },
    { jwin_edit_proc,      160,  66,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     136,   70,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Height:", NULL, NULL },
    { jwin_edit_proc,      160,  84,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     136,   88,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "H Spacing:", NULL, NULL },
    // 25
    { jwin_edit_proc,      160,  102,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     136,   106,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "V Spacing:", NULL, NULL },
    { jwin_check_proc,     56,  126,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Is continuation of previous string", NULL, NULL },
    { jwin_check_proc,     56,  136,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Text wraps around bounding box", NULL, NULL },
    { jwin_button_proc,    270,   152,  56,  16,      vc(14),  vc(1),   0,       D_EXIT,     0,             0, (void *) "Next in List", NULL, NULL },
    // 30
    { jwin_button_proc,     250,  187+6,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Help", NULL, NULL },
    { d_keyboard_proc,         0,    0,     0,  0,    0,       0,      0,       0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    { jwin_check_proc,     52,  176,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Set previous string's \"Next string\" to this", NULL, NULL },
    { jwin_check_proc,     56,  146,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Full Tiled Background", NULL, NULL },
    { jwin_check_proc,     56,  156,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Transparent BG", NULL, NULL },
    // 35
	{ jwin_check_proc,     56,  166,      0,      9,   vc(14),  vc(1),   0,       0,       1,             0, (void *) "Transparent FG", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_edit_proc,      232,  48,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     208,   52,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "T. Margin:", NULL, NULL },
    { jwin_edit_proc,      232,  66,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    // 40
	{ jwin_ctext_proc,     208,   70,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "B. Margin:", NULL, NULL },
    { jwin_edit_proc,      232,  84,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     208,   88,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "L. Margin:", NULL, NULL },
    { jwin_edit_proc,      232,  102,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     208,   106,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "R. Margin:", NULL, NULL },
    // 45
    { jwin_edit_proc,      80,  66,  28+1,  16,    vc(12),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     64,   70,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Y:", NULL, NULL },
    { jwin_edit_proc,      80,  48,  28+1,  16,    vc(12),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     64,   52,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "X:", NULL, NULL },
	{ jwin_edit_proc,      160,  48,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    // 50
    { jwin_ctext_proc,     136,   52,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Tile Width:", NULL, NULL },
    { jwin_edit_proc,      160,  66,  28+1,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_ctext_proc,     136,   70,   192,  8,    vc(9),   vc(1),   0,       0,          0,             0, (void *) "Tile Height:", NULL, NULL },
	{ d_cstile_proc,      60,  104,    16,  16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

DIALOG editmsg_help_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
//  { jwin_textbox_proc,    4,   2+21,   320-8,  240-6-21,  0,       0,      0,       0,          0,        0,        NULL, NULL, NULL },
    { jwin_win_proc,        0,   0,   320,  240,  0,       vc(15), 0,      D_EXIT,       0,          0, (void *) "String Control Codes", NULL, NULL },
    { jwin_frame_proc,   4,   23,   320-8,  240-27,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
    { d_editbox_proc,    6,   25,   320-8-4,  240-27-4,  0,       0,      0,       0/*D_SELECTED*/,          0,        0,       NULL, NULL, NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC, (void *) close_dlg, NULL, NULL },
    { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_F12, (void *) onSnapshot, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static MENU strlist_rclick_menu[] =
{
    { (char *)"Copy",  NULL,  NULL, 0, NULL },
    { (char *)"",  NULL,  NULL, 0, NULL },
    { (char *)"Paste Style", NULL,  NULL, 0, NULL },
    { (char *)"Paste Text", NULL,  NULL, 0, NULL },
    { (char *)"Paste Both", NULL,  NULL, 0, NULL },
    { (char *)"Paste Style to All", NULL,  NULL, 0, NULL },
    { (char *)"",  NULL,  NULL, 0, NULL },
    { (char *)"Set As Template",  NULL,  NULL, 0, NULL },
    { NULL,                       NULL,  NULL, 0, NULL }
};

static int zqstr_copysrc=-1;
void strlist_rclick_func(int index, int x, int y)
{
	// Don't do anything on (none) or <New String>
	if(index<=0 || index==msg_count-1)
		return;
	
	// Disable paste options if the copy source is invalid
	if(zqstr_copysrc<=0 || zqstr_copysrc>=msg_count)
	{
		strlist_rclick_menu[2].flags|=D_DISABLED;
		strlist_rclick_menu[3].flags|=D_DISABLED;
		strlist_rclick_menu[4].flags|=D_DISABLED;
		strlist_rclick_menu[5].flags|=D_DISABLED;
	}
	else
	{
		strlist_rclick_menu[2].flags&=~D_DISABLED;
		strlist_rclick_menu[3].flags&=~D_DISABLED;
		strlist_rclick_menu[4].flags&=~D_DISABLED;
		strlist_rclick_menu[5].flags&=~D_DISABLED;
	}
	int ret=popup_menu(strlist_rclick_menu, x, y);

	switch(ret)
	{
	case 0: // Copy
		zqstr_copysrc=msg_at_pos(index);
		break;
	
	case 2: // Paste Style
		MsgStrings[msg_at_pos(index)].copyStyle(MsgStrings[zqstr_copysrc]);
		break;
	
	case 3: //Paste Text
		MsgStrings[msg_at_pos(index)].copyText(MsgStrings[zqstr_copysrc]);
		strlist_dlg[2].flags|=D_DIRTY;
		break;
	
	case 4: //Paste Both
		MsgStrings[msg_at_pos(index)] = MsgStrings[zqstr_copysrc]; //Overloaded assignment copies both
		strlist_dlg[2].flags|=D_DIRTY;
		break;
	
	case 5: //Paste Style to All
		if(jwin_alert("Paste Style to All", "Overwrite style of all strings?", NULL, NULL, "&Yes","&No",'y','n',lfont)==1)
		{
			for(int q = 0; q < msg_count-1; ++q)
			{
				MsgStrings[q].copyStyle(MsgStrings[zqstr_copysrc]);
			}
		}
		break;
	
	case 7: // Set as template
		sprintf(static_cast<char*>(strlist_dlg[22].dp), "%d", msg_at_pos(index));
		strlist_dlg[22].flags|=D_DIRTY;
		break;
	}
}

// Don't actually use this to strip spaces.
char *strip_extra_spaces(char *string)
{
    int len=(int)strlen(string);
    char *src=(char *)zc_malloc(len+1);
    char *tmpsrc=src;
    memcpy(src,string,len+1);
    memset(src,0,len+1);
    
    for(unsigned int i=0; string[i]&&i<strlen(string); i++)
    {
        *tmpsrc=string[i];
        
        if(*tmpsrc==' ')
        {
            while(string[i+1]==' '&&i<strlen(string))
            {
                i++;
            }
        }
        else if(*tmpsrc && (*tmpsrc < 32 || *tmpsrc > 126))
        {
            for(int numops=msg_code_operands(*tmpsrc-1); numops>0; numops--)
            {
                tmpsrc++;
                
                if((byte)(*tmpsrc)==255)
                {
                    tmpsrc+=2;
                }
            }
        }
        
        tmpsrc++;
    }
    
    *tmpsrc=0;
    //  memcpy(string,src,len);
    strcpy(string,src);
    zc_free(src);
    return string;
}

char *MsgString(int index, bool show_number, bool pad_number)
{
    bound(index,0,msg_strings_size-1);
    static char u[80];
    bool indent = is_large && index>0 && MsgStrings[addtomsglist(MsgStrings[index].listpos-1)].nextstring==index;
    sprintf(u, pad_number?"%s%3d":"%s%d",indent?"--> ":"",index);
    char *s=strcat(u,": ");
    
    char *t = new char[71];
    memset(t, 0, 71);
    int i=0;
    int length=strlen(MsgStrings[index].s);
    //return s;
    
    //remove preceding spaces;
    for(; i<length && (MsgStrings[index].s[i]==' ' || MsgStrings[index].s[i]<32 || MsgStrings[index].s[i]>126); ++i)
    {
        if(MsgStrings[index].s[i]!=' ')  // Is it a control code?
        {
            for(int numops=msg_code_operands(MsgStrings[index].s[i]-1); numops>0; numops--)
            {
                i++;
                
                if((byte)(MsgStrings[index].s[i])==255)
                    i+=2;
            }
        }
    }
    
    int msgptr=0;
    
    for(; msgptr<70 && i<MSGSIZE; i++)
    {
        if(i<length && MsgStrings[index].s[i]>=32 && MsgStrings[index].s[i]<=126)
        {
            t[msgptr++]=MsgStrings[index].s[i];
        }
        else if(i<length && MsgStrings[index].s[i])
        {
            for(int numops=msg_code_operands(MsgStrings[index].s[i]-1); numops>0; numops--)
            {
                i++;
                
                if((byte)(MsgStrings[index].s[i])==255)
                    i+=2;
            }
        }
    }
    
    for(; msgptr<=70; msgptr++)
        t[msgptr]=0;
        
    strip_extra_spaces(t);
    
    if(show_number)
        strcat(s, t);
    else
        strcpy(s, t);
        
    delete[] t;
    return s;
}

const char *msgslistImpl(int index, int *list_size, bool numbered)
{
    static char buf[80];
    memset(buf, 0, 80);
    
    if(index>=0)
    {
        if(index>=msg_count)
        {
            index=msg_count-1;
        }
        
        // Find the listpos corresponding to the index
        int pos = 0;
        
        std::map<int,int>::iterator res = msglistcache.find(index);
        
        if(res != msglistcache.end())
            pos = res->second;
        else
        {
            pos = addtomsglist(index);
        }
        
        memcpy(buf,MsgString(pos, numbered, numbered),80);
        return buf;
    }
    
    *list_size=msg_count;
    return NULL;
}

const char *msgslist(int index, int *list_size)
{
    return msgslistImpl(index, list_size, true);
}

//same as above, but without the numbers
const char *msgslist2(int index, int *list_size)
{
    return msgslistImpl(index, list_size, false);
}

FONT* getfont(int fonta)
{
    switch(fonta)
    {
    default:
        return zfont;
        
    case font_z3font:
        return z3font;
        
    case font_z3smallfont:
        return z3smallfont;
        
    case font_deffont:
        return deffont;
        
    case font_lfont:
        return lfont;
        
    case font_lfont_l:
        return lfont_l;
        
    case font_pfont:
        return pfont;
        
    case font_mfont:
        return mfont;
        
    case font_ztfont:
        return ztfont;
        
    case font_sfont:
        return sfont;
        
    case font_sfont2:
        return sfont2;
        
    case font_spfont:
        return spfont;
        
    case font_ssfont1:
        return ssfont1;
        
    case font_ssfont2:
        return ssfont2;
        
    case font_ssfont3:
        return ssfont3;
        
    case font_ssfont4:
        return ssfont4;
        
    case font_gblafont:
        return gblafont;
        
    case font_goronfont:
        return goronfont;
        
    case font_zoranfont:
        return zoranfont;
        
    case font_hylian1font:
        return hylian1font;
        
    case font_hylian2font:
        return hylian2font;
        
    case font_hylian3font:
        return hylian3font;
        
    case font_hylian4font:
        return hylian4font;
        
    case font_gboraclefont:
        return gboraclefont;
        
    case font_gboraclepfont:
        return gboraclepfont;
        
    case font_dsphantomfont:
        return dsphantomfont;
        
    case font_dsphantompfont:
        return dsphantompfont;
    
      //New fonts for 2.54+
		 case font_atari800font: return atari800font;
		 case font_acornfont: return acornfont;
		 case font_adosfont: return adosfont;
		 case font_baseallegrofont: return  baseallegrofont;  
		 case font_apple2font: return apple2font;
		 case font_apple280colfont: return apple280colfont;   
		 case font_apple2gsfont: return  apple2gsfont;
		 case font_aquariusfont: return  aquariusfont;  
		 case font_atari400font: return  atari400font;  
		 case font_c64font: return c64font;   
		 case font_c64hiresfont: return c64hiresfont;   
		 case font_cgafont: return cgafont;   
		 case font_cocofont: return cocofont;
		 case font_coco2font: return coco2font;
		 case font_coupefon: return  coupefont;
		 case font_cpcfon: return  cpcfont;
		 case font_fantasyfon: return  fantasyfont;
		 case font_fdskanafon: return  fdskanafont;
		 case font_fdslikefon: return  fdslikefont;
		 case font_fdsromanfon: return fdsromanfont; 
		 case font_finalffont: return finalffont; 
		 case font_futharkfont: return  futharkfont;
		 case font_gaiafont: return gaiafont; 
		 case font_hirafont: return hirafont; 
		 case font_jpfont: return jpfont; 
		 case font_kongfont: return  kongfont;
		 case font_manafont: return manafont; 
		 case font_mlfont: return  mlfont;
		 case font_motfont: return motfont;
		 case font_msxmode0font: return  msxmode0font;
		 case font_msxmode1font: return  msxmode1font;
		 case font_petfont: return  petfont;
		 case font_pstartfont: return  pstartfont;
		 case font_saturnfont: return  saturnfont;
		 case font_scififont: return  scififont;
		 case font_sherwoodfont: return sherwoodfont;
		 case font_sinqlfont: return  sinqlfont;
		 case font_spectrumfont: return  spectrumfont;
		 case font_speclgfont: return  speclgfont;
		 case font_ti99font: return  ti99font;
		 case font_trsfont: return  trsfont;
		 case font_z2font: return  z2font;
		 case font_zxfont: return zxfont;
		 case font_lisafont: return lisafont;
    }
}

void fix_string(word &str, word index)
{
    if(str == index)
        str = 0;
    else if(str > index)
        --str;
}

int strlist_del()
{
    if(msglistcache[strlist_dlg[2].d1]>0 && msglistcache[strlist_dlg[2].d1]<msg_count-1)
        return D_CLOSE;
        
    return D_O_K;
}

int onStrings()
{
    if(is_large && !strlist_dlg[0].d1)
    {
        large_dialog(strlist_dlg,2);
    }
    
    strlist_dlg[0].dp2=lfont;
    int index=0;
    char msgmore_xstring[5], msgmore_ystring[5], msgspeed_string[3], template_string[6];
    int morex=zinit.msg_more_x;
    int morey=zinit.msg_more_y;
    int msgspeed = zinit.msg_speed;
    sprintf(msgmore_xstring, "%d", zinit.msg_more_x);
    sprintf(msgmore_ystring, "%d", zinit.msg_more_y);
    sprintf(msgspeed_string, "%d", zinit.msg_speed);
    sprintf(template_string, "%d", 0);
    
    char tempbuf[50];
    sprintf(tempbuf, "0");
    
    strlist_dlg[17].d1=0;
    build_bistringcat_list();
    
    //Message more is offset
    strlist_dlg[15].flags=(zinit.msg_more_is_offset!=0)?D_SELECTED:0;
    
    while(index!=-1)
    {
        bool hasroom=false;
        
        if(msg_count<MAXMSGS)
        {
            hasroom=true;
            
            // About to overshoot msg string buffer - reallocate to 65535 strings
            if(msg_count >= msg_strings_size)
            {
                Z_message("Reallocating string buffer...\n");
                MsgStrings=(MsgStr*)_al_sane_realloc(MsgStrings,sizeof(MsgStr)*MAXMSGS);
                memset((void*)(&MsgStrings[msg_strings_size]), 0, sizeof(MsgStr)*(MAXMSGS-msg_strings_size));
                msg_strings_size = MAXMSGS;
                init_msgstrings(msg_count, msg_strings_size);
            }
            
            for(int i=0; i<MSGSIZE; i++)
                MsgStrings[msg_count].s[i]='\0';
                
            strcpy(MsgStrings[msg_count++].s,"<New String>");
            MsgStrings[msg_count].x=24;
            MsgStrings[msg_count].y=32;
            MsgStrings[msg_count].w=24*8;
            MsgStrings[msg_count].h=3*8;
            MsgStrings[msg_count].hspace=0;
            MsgStrings[msg_count].vspace=0;
            MsgStrings[msg_count].stringflags=0;
            MsgStrings[msg_count].listpos=msg_count;
            MsgStrings[msg_count].margins[up] = 8;
            MsgStrings[msg_count].margins[down] = 0;
            MsgStrings[msg_count].margins[left] = 8;
            MsgStrings[msg_count].margins[right] = 0;
            MsgStrings[msg_count].margins[right] = 0;
			MsgStrings[msg_count].portrait_tile = 0;
			MsgStrings[msg_count].portrait_cset = 0;
			MsgStrings[msg_count].portrait_x = 0;
			MsgStrings[msg_count].portrait_y = 0;
			MsgStrings[msg_count].portrait_tw = 1;
			MsgStrings[msg_count].portrait_th = 1;
        }
        
        strlist_dlg[7].dp=msgmore_xstring;
        strlist_dlg[8].dp=msgmore_ystring;
        strlist_dlg[10].dp=msgspeed_string;
        strlist_dlg[22].dp=template_string;
        
        int ret=zc_popup_dialog(strlist_dlg,2);
        index=msglistcache[strlist_dlg[2].d1];
        
        bool doedit=false;
        int addAfter = -1;
        
        switch(ret)
        {
        case 18:
            jwin_alert("String Index","Create a string beginning with two hyphens '--'",
                       "and it will be listed in this index, serving as a",
                       "shortcut to a location in your string list.","O&K",NULL,'k',0,lfont);
            break;
            
        case 17: // Go to category
        {
            strlist_dlg[2].d1 = MsgStrings[bistringcat[strlist_dlg[17].d1]].listpos;
            break;
        }
        
        case 11: // Move string up
        {
            if(index==0 || index==msg_count-1)
                break;
                
            // Find the string with index's listpos - 1, and increment it
            int nextlistpos = MsgStrings[index].listpos-1;
            
            if(nextlistpos<0)
                break;
                
            std::map<int,int>::iterator res = msglistcache.find(nextlistpos);
            
            if(res == msglistcache.end())
                (void)addtomsglist(nextlistpos);
                
            int otherindex = msglistcache[nextlistpos];
            
            if(otherindex==0)
                break; // It's obviously not in here...
                
            MsgStrings[index].listpos--;
            MsgStrings[otherindex].listpos++;
            
            // Fix msglistcache
            (void)addtomsglist(MsgStrings[index].listpos);
            (void)addtomsglist(MsgStrings[index].listpos+1);
            
            strlist_dlg[2].d1--;
            saved=false;
            break;
        }
        
        case 12: // Move string down
        {
            if(index==0 || index==msg_count-1)
                break;
                
            // Find the string with index's listpos + 1, and decrement it
            int nextlistpos = MsgStrings[index].listpos+1;
            
            if(nextlistpos>=msg_count-1)
                break;
                
            std::map<int,int>::iterator res = msglistcache.find(nextlistpos);
            
            if(res == msglistcache.end())
                (void)addtomsglist(nextlistpos);
                
            int otherindex = msglistcache[nextlistpos];
            
            if(otherindex==0)
                break; // It's obviously not in here...
                
            MsgStrings[index].listpos++;
            MsgStrings[otherindex].listpos--;
            
            // Fix msglistcache
            (void)addtomsglist(MsgStrings[index].listpos);
            (void)addtomsglist(MsgStrings[index].listpos-1);
            
            strlist_dlg[2].d1++;
            saved=false;
            break;
        }
        
        case 13: // Move string up - 12
        case 14: // Move string down - 12
        {
            if(index==0 || index==msg_count-1)
                break;
                
            int diff = (ret == 13) ? zc_max(MsgStrings[index].listpos-12, 1) : zc_min(MsgStrings[index].listpos+12, msg_count-2);
            int sign = (ret == 13) ? -1 : 1;
            
            int nextindex;
            
            // For all strings above or below, de/increment
            for(int i=MsgStrings[index].listpos; i!=diff+sign; i+=sign)
            {
                std::map<int,int>::iterator res = msglistcache.find(i);
                
                if(res == msglistcache.end())
                    (void)addtomsglist(i);
                    
                int otherindex = msglistcache[i];
                
                if(otherindex==0)
                    break; // It's obviously not in here...
                    
                if(i==diff)
                {
                    nextindex = index;
                }
                else
                {
                    res = msglistcache.find(i+sign);
                    
                    if(res == msglistcache.end())
                        (void)addtomsglist(i+sign);
                        
                    nextindex = msglistcache[i+sign];
                }
                
                MsgStrings[otherindex].listpos+=sign;
                MsgStrings[nextindex].listpos-=sign;
                (void)addtomsglist(MsgStrings[otherindex].listpos);
                (void)addtomsglist(MsgStrings[nextindex].listpos);
            }
            
            //MsgStrings[index].listpos=diff;
            //(void)addtomsglist(MsgStrings[index].listpos);
            strlist_dlg[2].d1=diff;
            saved=false;
            break;
        }
        
        case 16:
            addAfter=zc_min(index, msg_count-2);
            index=msg_count-1;
            strlist_dlg[2].d1 = index;
            
        case 2:
        case 3:
            doedit=true;
            break;
            
        case 0: // ???
        case 4:
        
            index=-1;
            zinit.msg_more_x=atoi(msgmore_xstring);
            zinit.msg_more_y=atoi(msgmore_ystring);
            zinit.msg_speed=atoi(msgspeed_string);
            zinit.msg_more_is_offset=(strlist_dlg[15].flags&D_SELECTED)?1:0;
            
            if(morex!=zinit.msg_more_x||morey!=zinit.msg_more_y||msgspeed!=zinit.msg_speed)
            {
                saved=false;
            }
            
            break;
            
        case 5: // Delete
            char buf[73], shortbuf[73];
            memset(buf, 0, 73);
            memset(shortbuf, 0, 73);
            strncpy(buf,MsgString(index, true, false),72);
            strip_extra_spaces(buf);
            shorten_string(shortbuf, buf, font, 72, 288);
            
            if(jwin_alert("Confirm Clear","Clear this message string?"," ",shortbuf,"Yes","No",'y',27,lfont)==1)
            {
                saved=false;
                word pos=MsgStrings[index].listpos;
                memset((void*)(&MsgStrings[index]), 0, sizeof(MsgStr));
                MsgStrings[index].x=24;
                MsgStrings[index].y=32;
                MsgStrings[index].w=24*8;
                MsgStrings[index].h=3*8;
                MsgStrings[index].listpos=pos; // Since the stuff below isn't being run, do this instead
                
                
                /*// Go through strings with higher listpos and decrement listpos
                for(int j=MsgStrings[index].listpos; j<msg_count; j++)
                {
                  MsgStrings[addtomsglist(j)].listpos--;
                }
                // Delete string
                for(int i=index; i<msg_strings_size-1; i++)
                  MsgStrings[i]=MsgStrings[i+1];
                // This is now inaccurate
                msglistcache.clear();
                reset_msgstr(msg_strings_size-1);
                --msg_count;
                int sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;
                
                for(int s=0; s<sc; s++)                           //room strings
                {
                  fix_string(TheMaps[s].str, index);
                }
                for(int i=0; i<16; i++)                           //info strings
                {
                  for(int j=0; j<3; j++)
                  {
                    fix_string(misc.info[i].str[j], index);
                  }
                }
                fix_string(misc.endstring, index);              //ending string */
                // Fix the quick-category menu
                strlist_dlg[17].d1=0;
                build_bistringcat_list();
                
                refresh(rMENU);
            }
            
            break;
            
        case 19: // copy
            if(index==msg_count-1)
                zqstr_copysrc=-1;
            else
                zqstr_copysrc=index;
                
            break;
            
        case 20: // paste
            if(zqstr_copysrc>0 && index>0)
            {
                if(index==msg_count-1)
                    ++msg_count;
                    
                MsgStrings[index]=MsgStrings[zqstr_copysrc];
            }
            
            break;
        }
        
        if(hasroom)
        {
            --msg_count;
        }
        
        if(index>0 && doedit)
        {
            int lp = addAfter>=0 ? MsgStrings[addAfter].listpos : -1;
            editmsg(index, addAfter);
            
            if(MsgStrings[index].listpos!=msg_count) // Created new string
            {
                // Select the new message
                strlist_dlg[2].d1 = MsgStrings[index].listpos;
                if((editmsg_dlg[32].flags&D_SELECTED)!=0)
                {
                    int prev=msg_at_pos(MsgStrings[index].listpos-1);
                    MsgStrings[prev].nextstring=index;
                }
                editmsg_dlg[27].flags=(MsgStrings[index].stringflags&STRINGFLAG_CONT)?D_SELECTED:0;
            }
            else if(lp>=0)              // Canceled or edited an existing string
                strlist_dlg[2].d1 = lp; // Select previously selected message
            
            // Fix the quick-category menu
            strlist_dlg[17].d1=0;
            build_bistringcat_list();
            refresh(rMENU);
        }
    }
    
    //if(smsg!=NULL) delete [] smsg;
    return D_O_K;
}

void editmsg(int index, int addAfter)
{
    char setitle[80];
    sprintf(setitle, "String Editor (%d)", index);
    
    encode_msg_str(index);
    
    if(index==msg_count) // Adding a new message
    {
        int templateID=atoi(static_cast<char*>(strlist_dlg[22].dp));
        if(templateID>0 && templateID<msg_count)
            MsgStrings[index].copyStyle(MsgStrings[templateID]);
    }
    
    msgtile = MsgStrings[index].tile;
    msgcset = MsgStrings[index].cset;
    
    editmsg_dlg[0].dp=setitle;
    editmsg_dlg[0].dp2=lfont;
    editmsg_dlg[3].dp=msgbuf;
//  ListData msgs_list(msgslist, is_large ? &sfont3 : &font);
//  ListData msgs_list(msgslist, is_large ? &lfont_l : &font);
    ListData msgs_list(msgslist, &font);
    ListData msgfont_list(msgfontlist, &font);
    editmsg_dlg[5].d1 = MsgStrings[MsgStrings[index].nextstring].listpos;
    editmsg_dlg[5].dp = (void *) &msgs_list;
    editmsg_dlg[18].d1 = MsgStrings[index].font;
    editmsg_dlg[18].dp = (void *) &msgfont_list;
    editmsg_dlg[9].flags = MsgStrings[index].trans ? D_SELECTED : 0;
    editmsg_dlg[12].d1 = MsgStrings[index].tile;
    editmsg_dlg[12].fg = MsgStrings[index].cset;
    editmsg_dlg[53].d1 = MsgStrings[index].portrait_tile;
    editmsg_dlg[53].d2 = MsgStrings[index].portrait_cset&15;
    char msg_ybuf[5];
    char msg_xbuf[5];
    char msg_wbuf[5];
    char msg_hbuf[5];
    char msg_hsbuf[5];
    char msg_vsbuf[5];
	char msg_margins_bufs[4][5];
	char prt_xbuf[5];
	char prt_ybuf[5];
	char prt_twbuf[5];
	char prt_thbuf[5];
    sprintf(msg_ybuf,"%d",MsgStrings[index].y);
    sprintf(msg_xbuf,"%d",MsgStrings[index].x);
    sprintf(msg_wbuf,"%d",MsgStrings[index].w);
    sprintf(msg_hbuf,"%d",MsgStrings[index].h);
    sprintf(msg_hsbuf,"%d",MsgStrings[index].hspace);
    sprintf(msg_vsbuf,"%d",MsgStrings[index].vspace);
    sprintf(prt_xbuf,"%d",MsgStrings[index].portrait_x);
    sprintf(prt_ybuf,"%d",MsgStrings[index].portrait_y);
    sprintf(prt_twbuf,"%d",MsgStrings[index].portrait_tw);
    sprintf(prt_thbuf,"%d",MsgStrings[index].portrait_th);
	for(int q = 0; q < 4; ++q)
	{
		sprintf(msg_margins_bufs[q],"%d",MsgStrings[index].margins[q]);
	}
    editmsg_dlg[9].dp = msg_ybuf;
    editmsg_dlg[15].dp = msg_xbuf;
    editmsg_dlg[19].dp = msg_wbuf;
    editmsg_dlg[21].dp = msg_hbuf;
    editmsg_dlg[23].dp = msg_hsbuf;
    editmsg_dlg[25].dp = msg_vsbuf;
	for(int q = 0; q < 4; ++q)
	{
		editmsg_dlg[37+(2*q)].dp = msg_margins_bufs[q];
	}
    editmsg_dlg[45].dp = prt_ybuf;
    editmsg_dlg[47].dp = prt_xbuf;
    editmsg_dlg[49].dp = prt_twbuf;
    editmsg_dlg[51].dp = prt_thbuf;
    char msg_sfxbuf[5];
    sprintf(msg_sfxbuf,"%d",MsgStrings[index].sfx);
    editmsg_dlg[13].dp = msg_sfxbuf;
    editmsg_dlg[27].flags=(MsgStrings[index].stringflags&STRINGFLAG_CONT)?D_SELECTED:0;
    editmsg_dlg[28].flags=(MsgStrings[index].stringflags&STRINGFLAG_WRAP)?D_SELECTED:0;
    editmsg_dlg[32].flags=0;
    editmsg_dlg[32].proc=jwin_check_proc;
    editmsg_dlg[33].flags=(MsgStrings[index].stringflags&STRINGFLAG_FULLTILE)?D_SELECTED:0;
    editmsg_dlg[34].flags=(MsgStrings[index].stringflags&STRINGFLAG_TRANS_BG)?D_SELECTED:0;
    editmsg_dlg[35].flags=(MsgStrings[index].stringflags&STRINGFLAG_TRANS_FG)?D_SELECTED:0;
        
    msg_x = 0;
    msg_y = 0;
    curmsgstr = &MsgStrings[index];
    
    if(is_large)
    {
        large_dialog(editmsg_dlg,2);
        
        if(editmsg_dlg[0].d1<2)
        {
            editmsg_dlg[0].d1 = 2;
            editmsg_dlg[3].h = 24;
            editmsg_dlg[2].x+=3;
            editmsg_dlg[2].y+=3;
            editmsg_dlg[2].w-=3;
            editmsg_dlg[2].h-=3;
        }
    }
    
    if(addAfter<=0) // Not <0 - wouldn't want to set message 0's next string
        editmsg_dlg[32].proc=d_dummy_proc;
    
    int ret = -1;
    
    do
    {
        ret = zc_popup_dialog(editmsg_dlg,3);
        
        if(ret==29)
        {
            ret = -1;
            
            if(MsgStrings[index].listpos < msg_count)
                editmsg_dlg[5].d1 = MsgStrings[index].listpos+1;
        }
        
        if(ret==30)
        {
            ret = -1;
            // Show string help
            editmsg_help_dlg[0].dp2= lfont;
	    FILE *stringshelpfile = fopen("docs/zstrings.txt", "r");
	    if (!stringshelpfile )
	    {
		stringshelpfile = fopen("zstrings.txt", "r");
		if ( stringshelpfile )
		{
			editmsg_help_dlg[2].dp = new EditboxModel(helpstr, new EditboxScriptView(&editmsg_help_dlg[2],(is_large?sfont3:font),0,vc(15),BasicEditboxView::HSTYLE_EOTEXT), true, (char *)"zstrings.txt");
           
		}
		else
		{
			Z_error("File Missing: zstrings.txt.");
		}
	    }
	    else
	    {
		editmsg_help_dlg[2].dp = new EditboxModel(helpstr, new EditboxScriptView(&editmsg_help_dlg[2],(is_large?sfont3:font),0,vc(15),BasicEditboxView::HSTYLE_EOTEXT), true, (char *)"docs/zstrings.txt");
            }
	    editmsg_help_dlg[2].bg = vc(15);
            ((EditboxModel*)editmsg_help_dlg[2].dp)->doHelp(); // This deletes the EditboxModel too.
        }
    }
    while(ret==-1);
    
    if(ret==6)
    {
        saved=false;
        char *tempstr_ = parse_msg_str(msgbuf);
        sprintf(MsgStrings[index].s, "%s", tempstr_);
        delete [] tempstr_;
        
        MsgStrings[index].nextstring = addtomsglist(editmsg_dlg[5].d1);
        MsgStrings[index].font = editmsg_dlg[18].d1;
        MsgStrings[index].trans = editmsg_dlg[9].flags != 0;
        MsgStrings[index].tile = editmsg_dlg[12].d1;
        MsgStrings[index].cset = editmsg_dlg[12].fg;
        MsgStrings[index].portrait_tile = editmsg_dlg[53].d1;
        MsgStrings[index].portrait_cset = editmsg_dlg[53].d2;
        MsgStrings[index].x = vbound((int)strtol(msg_xbuf, (char **)NULL, 10),-512,512);
        MsgStrings[index].y = vbound((int)strtol(msg_ybuf, (char **)NULL, 10),-512,512);
        MsgStrings[index].w = vbound((int)strtol(msg_wbuf, (char **)NULL, 10),8,512);
        MsgStrings[index].h = vbound((int)strtol(msg_hbuf, (char **)NULL, 10),8,512);
		for(int q = 0; q < 4; ++q)
		{
			MsgStrings[index].margins[q] = vbound((byte)strtol(msg_margins_bufs[q], (char **)NULL, 10),0,255);
		}
        MsgStrings[index].portrait_x = vbound((int)strtol(prt_xbuf, (char **)NULL, 10),0,255);
        MsgStrings[index].portrait_y = vbound((int)strtol(prt_ybuf, (char **)NULL, 10),0,255);
        MsgStrings[index].portrait_tw = vbound((int)strtol(prt_twbuf, (char **)NULL, 10),0,16);
        MsgStrings[index].portrait_th = vbound((int)strtol(prt_thbuf, (char **)NULL, 10),0,14);
        MsgStrings[index].hspace = vbound((int)strtol(msg_hsbuf, (char **)NULL, 10),0,128);
        MsgStrings[index].vspace = vbound((int)strtol(msg_vsbuf, (char **)NULL, 10),0,128);
        MsgStrings[index].sfx = (int)strtol(msg_sfxbuf, (char **)NULL, 10);
        MsgStrings[index].stringflags = editmsg_dlg[27].flags & D_SELECTED ? MsgStrings[index].stringflags | STRINGFLAG_CONT : MsgStrings[index].stringflags & ~STRINGFLAG_CONT;
        MsgStrings[index].stringflags = editmsg_dlg[28].flags & D_SELECTED ? MsgStrings[index].stringflags | STRINGFLAG_WRAP : MsgStrings[index].stringflags & ~STRINGFLAG_WRAP;
        MsgStrings[index].stringflags = editmsg_dlg[33].flags & D_SELECTED ? MsgStrings[index].stringflags | STRINGFLAG_FULLTILE : MsgStrings[index].stringflags & ~STRINGFLAG_FULLTILE;
        MsgStrings[index].stringflags = editmsg_dlg[34].flags & D_SELECTED ? MsgStrings[index].stringflags | STRINGFLAG_TRANS_BG : MsgStrings[index].stringflags & ~STRINGFLAG_TRANS_BG;
        MsgStrings[index].stringflags = editmsg_dlg[35].flags & D_SELECTED ? MsgStrings[index].stringflags | STRINGFLAG_TRANS_FG : MsgStrings[index].stringflags & ~STRINGFLAG_TRANS_FG;
        
        if(index==msg_count)
        {
            if(addAfter!=-1)
            {
                MsgStrings[index].listpos = MsgStrings[addAfter].listpos+1;
                
                // Go through strings with higher listpos and increment listpos
                for(int j=msg_count-1; j>=MsgStrings[index].listpos; j--)
                {
                    MsgStrings[addtomsglist(j)].listpos++;
                }
                
                msglistcache.clear();
            }
            
            ++msg_count;
            MsgStrings[msg_count].sfx=18;
            MsgStrings[msg_count].listpos=msg_count;
        }
    }
    else if(ret==7 && addAfter!=-1)
    {
        MsgStrings[index].listpos = msg_count;
    }
}

// Returns the actual string of a given listpos
int addtomsglist(int index)
{
    if(index==0)
        return 0; // '(None)' is always at the top
        
    if(index==msg_count)
        return msg_count; // '<New String>' is always at the bottom
        
    int pos = 0;
    // Easy heuristic:
    // - Search backwards if index>(msg_count/2)
    int increment = 1;
    int i = 0;
    
    if(index > (msg_count/2))
    {
        increment = -1;
        i = msg_count-1;
    }
    
    for(; i<msg_count && i>=0; i+=increment)
    {
        if(MsgStrings[i].listpos==index)
        {
            msglistcache[index] = pos = i;
            break;
        }
    }
    
    return pos;
}

const char *msgfont_str[font_max] =
{
    "Zelda NES", "Link to the Past", "LttP Small", "Allegro Default", "GUI Font Bold", "GUI Font", "GUI Font Narrow", "Zelda NES (Matrix)", "BS Time (Incomplete)", "Small", "Small 2",
    "S. Proportional", "SS 1 (Numerals)", "SS 2 (Incomplete)", "SS 3", "SS 4 (Numerals)", "Link's Awakening", "Goron", "Zoran", "Hylian 1", "Hylian 2",
    "Hylian 3", "Hylian 4", "Oracle", "Oracle Proportional", "Phantom", "Phantom Proportional",
	"Atari 800", 
	"Acorn",
	"ADOS",
	"Allegro",
	"Apple II",
	"Apple II 80 Column",
	"Apple IIgs",
	"Aquarius",
	"Atari 400",
	"C64",
	"C64 HiRes",
	"IBM CGA",
	"COCO Mode I",
	"COCO Mode II",
	"Coupe",
	"Amstrad CPC",
	"Fantasy Letters",
	"FDS Katakana",
	"FDSesque",
	"FDS Roman",
	"FF",
	"Elder Futhark",
	"Gaia",
	"Hira",
	"JP Unsorted",
	"Kong",
	"Mana",
	"Mario",
	"Mot CPU",
	"MSX Mode 0",
	"MSX Mode 1",
	"PET",
	"Homebrew",
	"Mr. Saturn",
	"Sci-Fi",
	"Sherwood",
	"Sinclair QL",
	"Spectrum",
	"Spectrum Large",
	"TI99",
	"TRS",
	"Zelda 2",
	"ZX",
	"Lisa"
};

const char *msgfontlist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = font_max;
        return NULL;
    }
    
    return msgfont_str[index];
}

void strip_trailing_spaces(char *str)
{
    for(int i=0; str[i]; i++)
    {
        if(str[i]==' ')
        {
            for(int k=i+1; true; k++)
            {
                // End of the string, just spaces so far; end at i
                if(str[k]=='\0')
                {
                    str[i]='\0';
                    return;
                }
                // Found a non-space character
                else if(str[k]!=' ')
                {
                    i=k;
                    break;
                }
            }
        }
    }
}

//Not really the right usage of "parse"...
char *parse_msg_str(char *s)
{
    //if(smsg!=NULL) delete [] smsg;
    char *smsg = new char[MSGSIZE+1];
    memset(smsg, 0, MSGSIZE+1);
    int msgptr=0;
    
    for(int i=0; msgptr<MSGSIZE; msgptr++, i++) // But what about long CCs?
    {
        // Is it a backslash-escaped number?
        if(*(s+i)=='\\' && i+4<MSGSIZE*3)
        {
            int msgcc = 0;
            byte twofiftyfives = 0;
            byte digits = 0;
            
            // Read the entire number
            while(i+1<MSGSIZE*3 && *(s+i+1)>='0' && *(s+i+1)<='9' && ++digits <= 5)
            {
                i++;
                msgcc*=10; // Move the current number one decimal place right.
                msgcc+=(*(s+i)-'0');
                
                // A hack to allow multi-byte numbers.
                if(msgcc >= 254)
                {
                    twofiftyfives = (msgcc/254)<<0;
                }
                
                smsg[msgptr] = (unsigned char)((msgcc % 254) + 1); // As 0 is null, we must store codes 1 higher than their actual value...
            }
            
            // A hack to allow multi-byte numbers, continued
            if(twofiftyfives > 0 && i+2<MSGSIZE*3)
            {
                smsg[++msgptr] = (unsigned char)0xff;
                smsg[++msgptr] = twofiftyfives;
            }
        }
        else
        {
            smsg[msgptr] = (*(s+i) >= 32 && *(s+i) <=126) ? *(s+i) : ' ';
        }
    }
    
    smsg[msgptr] = '\0';
    return smsg;
}

//Make sure this is synchronised with parsemsgcode in guys.cpp!
void put_msg_str(char *s,int x,int y,int, int ,int, int start_x, int start_y)
{
	bool oldmargin = get_bit(quest_rules,qr_OLD_STRING_EDITOR_MARGINS)!=0;
    int w = vbound((int)strtol((char*)editmsg_dlg[19].dp, (char **)NULL, 10),0,512);
    int h = vbound((int)strtol((char*)editmsg_dlg[21].dp, (char **)NULL, 10),0,512);
    int fonta = editmsg_dlg[18].d1;
    int flags = 0;
    flags |= (int)editmsg_dlg[27].flags & D_SELECTED ? STRINGFLAG_CONT : 0;
    flags |= (int)editmsg_dlg[28].flags & D_SELECTED ? STRINGFLAG_WRAP : 0;
    flags |= (int)editmsg_dlg[33].flags & D_SELECTED ? STRINGFLAG_FULLTILE : 0;
    flags |= (int)editmsg_dlg[34].flags & D_SELECTED ? STRINGFLAG_TRANS_BG : 0;
    flags |= (int)editmsg_dlg[35].flags & D_SELECTED ? STRINGFLAG_TRANS_FG : 0;
    int vspace = vbound((int)strtol((char*)editmsg_dlg[25].dp, (char **)NULL, 10),0,128);
    int hspace = vbound((int)strtol((char*)editmsg_dlg[23].dp, (char **)NULL, 10),0,128);
    int nextstring = addtomsglist(editmsg_dlg[5].d1);
	byte msg_margins[4];
	for(int q = 0; q < 4; ++q)
	{
		msg_margins[q] = oldmargin ? 0 : vbound((int)strtol((char*)editmsg_dlg[37+(2*q)].dp, (char **)NULL, 10),0,255);
	}
    int cursor_x = msg_margins[left];
    int cursor_y = msg_margins[up];
    
    int i=0;
    int msgcolour=misc.colors.msgtext;
    
    FONT *workfont = getfont(fonta);
    
    char *s2 = parse_msg_str(s);
    strip_trailing_spaces(s2);
    
    BITMAP *buf = create_bitmap_ex(8,zc_max(w+16,256+16),zc_max(h+16,32+16));
    clear_bitmap(buf);
    
    if(buf)
    {
        clear_bitmap(buf);
        
        if(msgtile)
		{
			if(flags & STRINGFLAG_FULLTILE)
			{
				draw_block_flip(buf,0,0,msgtile,msgcset,
					(int)ceil(w/16.0),(int)ceil(h/16.0),0,false,false);
			}
			else
			{
				frame2x2(buf,&misc,0,0,msgtile,msgcset,(w/8)+2,(h/8)+2,0,0,0);
			}
		}
            
        bool space=true;
        int tlength=0;
        
        for(; ;)
        {
            i=0;
            
            while((*(s2+i)))
            {
                char s3[145]; // Stores a complete word
                int j;
                int s3length = 1;
                int hjump=0;
                
                if(flags & STRINGFLAG_WRAP)
                {
                    if(space)
                    {
                        // Control codes and spaces are like one-character words
                        if((*(s2+i)) == ' ' || (*(s2+i)) < 32 || (*(s2+i)) > 126)
                        {
                            s3[0] = (*(s2+i));
                            s3[1] = 0;
                            hjump = hspace;
                            i++;
                        }
                        else space = false;
                    }
                    
                    if(!space)
                    {
                        // Complete words finish at spaces or control codes.
                        for(j=i; (*(s2+j)) != ' ' && (*(s2+j)) >= 32 && (*(s2+j)) <= 126 && (*(s2+j)); j++)
                        {
                            s3[j-i] = (*(s2+j));
                            hjump += hspace;
                            
                            if(s3[j-i]-1 == MSGC_NEWLINE)
                            {
                                j++;
                                break;
                            }
                        }
                        
                        space = true;
                        s3[j-i] = 0;
                        s3length = j-i;
                        i=j;
                    }
                }
                else
                {
                    space=false;
                    /*for(j=i; (*(s2+j)) < 32 || (*(s2+j)) > 126 && (*(s2+j)); j++)
                    {
                      s3[j-i] = (*(s2+j));
                      if(s3[j-i]-1 == MSGC_NEWLINE) {j++; break;}
                    }
                    s3[j-i]
                    s3[j-i+1] = 0;
                    i=j;*/
                    s3[0] = (*(s2+i));
                    
                    if(s3[0] >= 32 && s3[0] <= 126) hjump = hspace;
                    
                    s3[1] = 0;
                    i++;
                }
                
                tlength = text_length(workfont, s3);
                
                if(cursor_x+tlength+hjump > (w-msg_margins[right]) 
				   && ((cursor_x > (w-msg_margins[right]) || !(flags & STRINGFLAG_WRAP))
				        ? 1 : strcmp(s3," ")!=0))
                {
                    cursor_y += text_height(workfont) + vspace;
                    cursor_x=msg_margins[left];
                    //if(space) s3[0]=0;
                }
                
                // Evaluate what control code the character is, and skip over the CC's arguments by incrementing i (NOT k).
                // Interpret the control codes which affect text display (currently just MSGC_COLOR). -L
                for(int k=0; k < s3length; k++)
                {
                    switch(s3[k]-1)
                    {
                    case MSGC_NEWLINE:
                    {
                        if(cursor_x>msg_margins[left] || (cursor_y<=msg_margins[up] && cursor_x<=msg_margins[left])) // If the newline's already at the end of a line, ignore it
                        {
                            cursor_y += text_height(workfont) + vspace;
                            cursor_x=msg_margins[left];
                        }
                        
                        //No i++ here - s3 terminates at newlines.
                        break;
                    }
                    
                    case MSGC_COLOUR:
                    {
                        // Read the CC arguments directly from s2, while skipping over them.
                        // (Yes, i++ and not ++i) -L
                        int cset = (*(s2+(i++)))-1;
                        msgcolour = CSET(cset)+(*(s2+(i++)))-1;
                        break;
                    }
                    
                    default:
                        if(s3[k] >= 32 && s3[k] <= 126)
                        {
                            textprintf_ex(buf,workfont,cursor_x+(oldmargin?8:0),cursor_y+(oldmargin?8:0),msgcolour,-1,"%c",s3[k]);
                            cursor_x += workfont->vtable->char_length(workfont, s3[k]);
                            cursor_x += hspace;
                        }
                        else
                        {
                            for(int numops=msg_code_operands(s3[k]-1); numops>0; numops--)
                            {
                                i++;
                                
                                if((byte)s2[i]==255)
                                    i+=2;
                            }
                        }
                        
                        break;
                    }
                }
            }
            
            if(nextstring && MsgStrings[nextstring].stringflags & STRINGFLAG_CONT)
            {
                workfont = getfont(MsgStrings[nextstring].font);
                flags = MsgStrings[nextstring].stringflags;
                hspace = MsgStrings[nextstring].hspace;
                vspace = MsgStrings[nextstring].vspace;
                
                if(s2!=NULL) delete [] s2;
                
                s2 = new char[MSGSIZE+1];
                strcpy(s2, MsgStrings[nextstring].s);
                strip_trailing_spaces(s2);
                
                // Prevent an infinite loop...
                if(MsgStrings[nextstring].nextstring!=nextstring)
                    nextstring = MsgStrings[nextstring].nextstring;
                else
                    nextstring=0;
            }
            else
            {
                break;
            }
        }
        
        stretch_blit(buf,screen,start_x,start_y,256+16,32+16,x,y,(256+16)*(is_large?2:1),(32+16)*(is_large?2:1));
        destroy_bitmap(buf);
    }
    
    if(s2!=NULL) delete [] s2;
}

int mprvfont=0;
int mprvflags=0;
int mprvvspace=0;
int mprvhspace=0;
int mprvnextstring=0;
int mprvw=0;
int mprvh=0;

// Returns true if the given character should be encoded as a control code.
// lastWasSCC indicates whether this is immediately after a control code.
static inline bool isSCC(char character, bool lastWasSCC)
{
    // Non-ASCII character
    if(character<32 || character>126)
        return true;
    
    // Numeral after a control code
    if(lastWasSCC && character>='0' && character<='9')
        return true;
    
    // Backslash
    if(character=='\\')
        return true;
    
    return false;
}

// Load a stored string into msgbuf for editing.
void encode_msg_str(int index)
{
    memset(msgbuf, 0, MSGSIZE*3);
    
    // Adding a new string?
    if(index==msg_count)
        return;
        
    char *str=MsgStrings[index].s;
    int strPos=0;
    int msgbufPos=0;
    byte nextChar;
    char sccBuf[30]; // For string control codes
    char sccArgBuf[10];
    word sccArg;
    int sccNumArgs;
    bool lastWasSCC=false;
    
    while(msgbufPos<MSGSIZE*3 && strPos<MSGSIZE+1)
    {
        nextChar=str[strPos];
        
        if(nextChar=='\0')
            return;
            
        // Most regular text characters get put directly into msgbuf
        if(!isSCC(nextChar, lastWasSCC))
        {
            msgbuf[msgbufPos]=nextChar;
            lastWasSCC=false;
            msgbufPos++;
            strPos++;
        }
        
        // String control codes...
        else
        {
            // ASCII character as control code
            if(nextChar>=32)
            {
                sprintf(sccBuf, "\\%d", nextChar-1);
                strPos++;
            }
            else
            {
                // Load the code into sccBuf. The code is stored one higher
                // than its actual value to ensure that it's not null.
                nextChar--;
                sprintf(sccBuf, "\\%hu", (word)nextChar);
                strPos++;
                
                // One at a time, read arguments and append them to sccBuf
                for(sccNumArgs=msg_code_operands(nextChar); sccNumArgs>0; sccNumArgs--)
                {
                    // If strPos hasn't gone past the end of the string,
                    // just drop the control code.
                    if(strPos>MSGSIZE)
                        return;
                        
                    // If the argument is 253 or less, it's stored in one byte
                    // with one added so that it's not null.
                    nextChar=str[strPos];
                    sccArg=nextChar-1;
                    strPos++;
                    
                    // If there are at least two more bytes to read, they may
                    // be part of the same argument.
                    if(strPos<=MSGSIZE-2)
                    {
                        // If the next byte is 255, add the byte after that
                        // times 254 to the argument.
                        nextChar=str[strPos];
                        
                        if(nextChar==255)
                        {
                            nextChar=str[strPos+1];
                            sccArg+=254*nextChar;
                            strPos+=2;
                        }
                    }
                    
                    // Append the argument to sccBuf.
                    sprintf(sccArgBuf, "\\%hu", sccArg);
                    strcat(sccBuf, sccArgBuf);
                }
            }
            
            // Finally, copy the control code into msgbuf
            strcat(msgbuf, sccBuf);
            msgbufPos+=strlen(sccBuf);
            
            // And remember that we just entered a string control code
            lastWasSCC=true;
        }
    }
}

// Fix the strings if they were broken
void fix_string_list()
{
    int found[MAXMSGS];
    bool corrupted = false;
    memset(found, 0, MAXMSGS);
    
    for(int i=0; i<msg_count; i++)
    {
        found[MsgStrings[i].listpos] = 1;
    }
    
    for(int i=0; i<msg_count; i++)
    {
        if(found[i]==0)
            corrupted=true;
    }
    
    if(corrupted)
    {
        for(int i=0; i<msg_count; i++)
        {
            MsgStrings[i].listpos = i;
        }
        
        jwin_alert("Notice","Your quest's string ordering was corrupted.","It has been reverted to the default order.",NULL,"O&K",NULL,'k',0,lfont);
    }
}

void rebuild_string_list()
{
    fix_string_list();
    msglistcache.clear();
}

void reset_msgstr(int index)
{
    bound(index,0,msg_strings_size-1);
    /*
      char *s=MsgStrings[index].s;
      for(int i=0; i<76; i++)
      *(s++)=0;
      */
    memset(MsgStrings[index].s, 0, MSGSIZE+1);
    MsgStrings[index].nextstring=0;
    //memset(MsgStrings[index].expansion, 0, 32);
}

// Get the number of the message at the given list position
int msg_at_pos(int pos)
{
    for(int i=0; i<msg_count; i++)
    {
        if(MsgStrings[i].listpos==pos)
        {
            return i;
            break;
        }
    }
    
    // Hopefully, this'll never happen...
    return 0;
}

// Returns number of arguments to each control code
int msg_code_operands(int cc)
{
    switch(cc)
    {
	    
    case MSGC_WARP:
	return 6; //dmap, screen, x, y, effect, sound
    
    //portrait tile, x, y, width, height
    case MSGC_CHANGEPORTRAIT:
    case MSGC_GOTOIFCREEND:
	return 5;
    
    case MSGC_SETSCREEND:
	return 4;

    case MSGC_GOTOIFGLOBAL:
    case MSGC_GOTOIFCTRPC:
    case MSGC_GOTOIFCTR:
        return 3;
        
    case MSGC_COLOUR:
    case MSGC_CTRUP:
    case MSGC_CTRDN:
    case MSGC_CTRSET:
    case MSGC_CTRUPPC:
    case MSGC_CTRDNPC:
    case MSGC_CTRSETPC:
    case MSGC_GOTOIF:
    case MSGC_GOTOIFTRI:
    case MSGC_GOTOIFTRICOUNT:
    case MSGC_GOTOIFRAND:
        return 2;
        
    case MSGC_NEWLINE:
        return 0;
    }
    
    return 1;
}


// Category list functions


void build_bistringcat_list()
{
    memset(bistringcat,0,256*sizeof(int));
    bistringcat_cnt=1;
    
    for(int i = 0; i < msg_count && bistringcat_cnt<256; i++)
    {
        int m =  addtomsglist(i);
        
        if(MsgStrings[m].s[0]=='-' && MsgStrings[m].s[1]=='-')
        {
            bistringcat[bistringcat_cnt]=m;
            ++bistringcat_cnt;
        }
    }
}

const char *stringcatlist(int index, int *list_size)
{
    static char buf[80];
    
    if(index>=0)
    {
        memcpy(buf,MsgString(bistringcat[index], true, true),80);
        return buf;
    }
    
    *list_size=bistringcat_cnt;
    return NULL;
}


// Dialog procs
 //

extern int zqwin_scale;


int d_msg_preview_proc(int msg,DIALOG *d,int c)
{
    c=c;
    char *s=(char*)(d->dp);
    int w = vbound((int)strtol((char *)editmsg_dlg[19].dp, (char **) NULL, 10),8,512);
    int h = vbound((int)strtol((char *)editmsg_dlg[21].dp, (char **) NULL, 10),8,512);
    
    if(msg_x > zc_max(w-256,0)) msg_x = zc_max(w-256,0);
    
    if(msg_y > zc_max(h-32,0)) msg_y = zc_max(h-32,0);
    
    if(mprvfont != editmsg_dlg[18].d1 ||
            mprvflags != ((editmsg_dlg[27].flags & D_SELECTED ? STRINGFLAG_CONT : 0) | (editmsg_dlg[28].flags & D_SELECTED ? STRINGFLAG_WRAP : 0)) ||
            mprvvspace != vbound((int)strtol((char*)editmsg_dlg[25].dp, (char **)NULL, 10),0,128) ||
            mprvhspace != vbound((int)strtol((char*)editmsg_dlg[23].dp, (char **)NULL, 10),0,128) ||
            mprvnextstring != addtomsglist(editmsg_dlg[5].d1) ||
            mprvw != vbound((int)strtol((char*)editmsg_dlg[25].dp, (char **)NULL, 10),0,128) ||
            mprvh != vbound((int)strtol((char*)editmsg_dlg[23].dp, (char **)NULL, 10),0,128))
    {
    
        mprvfont = editmsg_dlg[18].d1;
        mprvflags = 0;
        mprvflags |= (int)editmsg_dlg[27].flags & D_SELECTED ? STRINGFLAG_CONT : 0;
        mprvflags |= (int)editmsg_dlg[28].flags & D_SELECTED ? STRINGFLAG_WRAP : 0;
        mprvvspace = vbound((int)strtol((char*)editmsg_dlg[25].dp, (char **)NULL, 10),0,128);
        mprvhspace = vbound((int)strtol((char*)editmsg_dlg[23].dp, (char **)NULL, 10),0,128);
        mprvnextstring = addtomsglist(editmsg_dlg[5].d1);
        mprvw = vbound((int)strtol((char*)editmsg_dlg[25].dp, (char **)NULL, 10),0,128);
        mprvh = vbound((int)strtol((char*)editmsg_dlg[23].dp, (char **)NULL, 10),0,128);
        
        msg = MSG_DRAW;
    }
    
    
    switch(msg)
    {
    case MSG_CLICK:
    {
        /*{
         int pos = (((gui_mouse_x())-(d->x+8))>>3)+(((gui_mouse_y())-(d->y+16))>>3)*24;
         int i = 0;
         while (pos>0 && i<(int)strlen(msgbuf)) {
           while (msgbuf[i] == '\\') {
             do {
               i++;
             }
             while(i<(int)strlen(msgbuf) && msgbuf[i] >= '0' && msgbuf[i] <= '9');
           }
           pos--;
           i++;
         }
         editmsg_dlg[3].d2 = i;
         (void)jwin_edit_proc(MSG_DRAW,&editmsg_dlg[3],c);*/
        int ox = gui_mouse_x();
        int oy = gui_mouse_y();
        int cmx = msg_x;
        int cmy = msg_y;
        int omx = cmx;
        int omy = cmy;
        
        while(gui_mouse_b())
        {
            if(cmx != msg_x || cmy != msg_y)
            {
                cmx = msg_x;
                cmy = msg_y;
                
                scare_mouse();
                object_message(d, MSG_DRAW, 0);
                unscare_mouse();
            }
            
            msg_x = vbound(omx+(ox-gui_mouse_x()),0,zc_max(0,w-256));
            msg_y = vbound(omy+(oy-gui_mouse_y()),0,zc_max(0,h-32));
            
            broadcast_dialog_message(MSG_IDLE, 0);
#ifdef _ZQUEST_SCALE_
            
            if(myvsync)
            {
                if(zqwin_scale > 1)
                {
                    stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
                }
                else
                {
                    blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
                }
                
                myvsync=0;
            }
            
#endif
        }
        
        break;
    }
    
    case MSG_DRAW:
        if(!(d->flags & D_GOTFOCUS))
        {
            d->d2=-1;
        }
        
        rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h, 0);
        put_msg_str(s,d->x,d->y,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],d->d2,msg_x,msg_y);
        break;
    }
    
    return D_O_K;
}

int d_msg_edit_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_edit_proc(msg,d,c);
    
    if(msg==MSG_CHAR)
        (void)d_msg_preview_proc(MSG_DRAW,&editmsg_dlg[8],c);
    else if(msg==MSG_START)
    {
        //hack to counteract jwin_edit_proc's automatic setting of the cursor to the far right
        d->d2=0;
    }
    
    return ret;
}

int d_msgtile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    switch(msg)
    {
    case MSG_CLICK:
        if(select_tile(d->d1,d->d2,1,d->fg,true))
        {
            msgtile = d->d1;
            msgcset = d->fg;
            return D_REDRAW;
        }
        
        break;
        
    case MSG_DRAW:
    {
        int dw = d->w;
        int dh = d->h;
        
        if(is_large)
        {
            dw /= 2;
            dh /= 2;
        }
        
        BITMAP *buf = create_bitmap_ex(8,dw,dh);
        
        if(buf)
        {
            clear_bitmap(buf);
            
            if(d->d1)
                puttile16(buf,d->d1,0,0,d->fg,0);
                
            stretch_blit(buf,screen,0,0,dw,dh,d->x-is_large,d->y-is_large,dw*(is_large?2:1),dh*(is_large?2:1));
            destroy_bitmap(buf);
        }
    }
    }
    
    return D_O_K;
}
