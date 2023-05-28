#include "zq/zq_strings.h"
#include "editbox.h"
#include "EditboxNew.h"
#include "base/gui.h"
#include "jwin.h"
#include "qst.h"
#include "tiles.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "zq/zq_custom.h"
#include "zq/zq_misc.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include <map>
#include <string>

static bool strlist_numerical_sort = false;
void editmsg(int32_t index, int32_t addAfter);
int32_t strlist_del();
int32_t addtomsglist(int32_t index, bool allow_numerical_sort = true);
void build_bistringcat_list();
const char *stringcatlist(int32_t index, int32_t *list_size);
std::string parse_msg_str(std::string const& s);
int32_t msg_code_operands(byte cc);
int32_t d_msgtile_proc(int32_t msg,DIALOG *d,int32_t c);
void strlist_rclick_func(int32_t index, int32_t x, int32_t y);


std::map<int32_t, int32_t> msglistcache;

static ListData strlist_dlg_list(msgslist3, &font);
static ListData stringcat_dlg_list(stringcatlist, &font);
MsgStr *curmsgstr = NULL;
int32_t msg_x = 0;
int32_t msg_y = 0;
int32_t msgtile = 0;
int32_t msgcset = 0;
char msgbuf[MSGBUF_SIZE];

int32_t bistringcat[256]; // A dropdown menu containing all strings which begin with '--', which serves as a quick shortcut to large string blocks.
int32_t bistringcat_cnt=-1;

// Dialogs

static int32_t editmsg_string_list[] =
{ 2, 3, 4, 5, 8, 17, 18, 29, 32, 58, 59, -1 };

static int32_t editmsg_attributes_list[] =
{ 9, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27,28, 33, 34, 35, 37, 38, 39, 40, 41, 42, 43, 44, 54, 55, 56, 57, -1 };

static int32_t editmsg_portrait_list[] =
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
	{ jwin_win_proc,        20,      0, 200+72, 148+76,  vc(14),  vc(1),    0,  D_EXIT,       0,             0, (void *) "Edit Message Strings", NULL, NULL },
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
	{ jwin_check_proc,     213,     18,      0,      9,   vc(14),  vc(1),   0,  D_EXIT,       1,             0, (void *) "Sort Numerically", NULL, NULL },
	{ NULL,                  0,      0,      0,      0,        0,     0,     0,      0,       0,             0,       NULL,  NULL,  NULL }
};

extern ListData shadowstyle_list;

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

static int32_t zqstr_copysrc=-1;
void strlist_rclick_func(int32_t index, int32_t x, int32_t y)
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
	int32_t ret=popup_menu(strlist_rclick_menu, x, y);

	switch(ret)
	{
	case 0: // Copy
		zqstr_copysrc=msg_at_pos(index);
		break;
	
	case 2: // Paste Style
		MsgStrings[msg_at_pos(index)].copyStyle(MsgStrings[zqstr_copysrc]);
		saved = false;
		break;
	
	case 3: //Paste Text
		MsgStrings[msg_at_pos(index)].copyText(MsgStrings[zqstr_copysrc]);
		strlist_dlg[2].flags|=D_DIRTY;
		saved = false;
		break;
	
	case 4: //Paste Both
		MsgStrings[msg_at_pos(index)] = MsgStrings[zqstr_copysrc]; //Overloaded assignment copies both
		strlist_dlg[2].flags|=D_DIRTY;
		saved = false;
		break;
	
	case 5: //Paste Style to All
		if(jwin_alert("Paste Style to All", "Overwrite style of all strings?", NULL, NULL, "&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
		{
			for(int32_t q = 0; q < msg_count-1; ++q)
			{
				MsgStrings[q].copyStyle(MsgStrings[zqstr_copysrc]);
			}
			saved = false;
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
	int32_t len=(int32_t)strlen(string);
	char *src=(char *)malloc(len+1);
	char *tmpsrc=src;
	memcpy(src,string,len+1);
	memset(src,0,len+1);
	
	for(size_t i=0; string[i]&&i<strlen(string); i++)
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
			for(int32_t numops=msg_code_operands(*tmpsrc-1); numops>0; numops--)
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
	free(src);
	return string;
}

void strip_extra_spaces(std::string &string)
{
	string = string.substr(string.find_first_not_of(' '), string.find_last_not_of(' '));
}

void scc_insert(char* buf, size_t& msgptr,byte cc,size_t limit = -1)
{
	switch(cc) //special scc inserts
	{
		case MSGC_MENUCHOICE:
			buf[msgptr++] = '>';
			break;
		case MSGC_DRAWTILE:
			buf[msgptr++] = '[';
			if(msgptr >= limit) break;
			buf[msgptr++] = ']';
			break;
		case MSGC_NAME:
			buf[msgptr++] = '(';
			if(msgptr >= limit) break;
			buf[msgptr++] = 'N';
			if(msgptr >= limit) break;
			buf[msgptr++] = 'a';
			if(msgptr >= limit) break;
			buf[msgptr++] = 'm';
			if(msgptr >= limit) break;
			buf[msgptr++] = 'e';
			if(msgptr >= limit) break;
			buf[msgptr++] = ')';
			break;
	}
}

char *MsgString(int32_t index, bool show_number, bool pad_number)
{
	static char u[80];
	bound(index,0,msg_strings_size-1);
	memset(u, 0, sizeof(u));
	if(index == 0)
	{
		strcpy(u, "  0: (None)");
		return u;
	}
	else if(index == msg_count)
	{
		sprintf(u, "%3d: <New String>", index);
		return u;
	}
	
	auto prevIndex = addtomsglist(index)-1;
	bool indent = index>0 && MsgStrings[prevIndex].nextstring==index;
	sprintf(u, pad_number?"%s%3d":"%s%d",indent?"--> ":"",index);
	char *s=strcat(u,": ");
	
	char *t = new char[71];
	memset(t, 0, 71);
	uint32_t i=0;
	uint32_t length = MsgStrings[index].s.size();
	//return s;
	
	size_t msgptr=0;
	//remove preceding spaces;
	for(; i<length && (MsgStrings[index].s[i]==' ' || MsgStrings[index].s[i]<32 || MsgStrings[index].s[i]>126);)
	{
		byte c = MsgStrings[index].s[i];
		++i;
		if(c!=' ')  // Is it a control code?
		{
			for(int32_t numops=msg_code_operands(c-1); numops>0; numops--)
			{
				i++;
				if(i>=length) break; //sanity!
				if((byte)(MsgStrings[index].s[i])==255)
					i+=2;
			}
			scc_insert(t,msgptr,c-1,70);
			if(msgptr) break;
		}
	}
	
	
	for(; msgptr<70 && i<MsgStrings[index].s.size(); i++)
	{
		byte c = MsgStrings[index].s[i];
		if(i<length && c>=32 && c<=126)
		{
			t[msgptr++]=c;
		}
		else if(i<length && c)
		{
			for(int32_t numops=msg_code_operands(c-1); numops>0; numops--)
			{
				i++;
				if(i>=length) break; //sanity!
				if((byte)(MsgStrings[index].s[i])==255)
					i+=2;
			}
			scc_insert(t,msgptr,c-1,70);
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

const char *msgslistImpl(int32_t index, int32_t *list_size, bool numbered, bool allow_num_sort = true)
{
	static std::string buf;
	buf = "";
	
	if(index>=0)
	{
		if(index>=msg_count)
		{
			index=msg_count-1;
		}
		
		// Find the listpos corresponding to the index
		int32_t pos = 0;
		
		std::map<int32_t,int32_t>::iterator res = msglistcache.find(index);
		
		if(res != msglistcache.end())
			pos = res->second;
		else
		{
			pos = addtomsglist(index, allow_num_sort);
		}
		
		buf = MsgString(pos, numbered, numbered);
		return buf.c_str();
	}
	
	*list_size=msg_count;
	return NULL;
}

const char *msgslist(int32_t index, int32_t *list_size)
{
	return msgslistImpl(index, list_size, true);
}

//same as above, but without the numbers
const char *msgslist2(int32_t index, int32_t *list_size)
{
	return msgslistImpl(index, list_size, false);
}

//same as msgslist, but allows numerical indexing if checked
const char *msgslist3(int32_t index, int32_t *list_size)
{
	if(index > -1 && index >= msg_count - 1)
	{
		static char b[80] = {0};
		sprintf(b, "%3d: <New String>", msg_count-1);
		return b;
	}
	return msgslistImpl(index, list_size, true, true);
}

void fix_string(word &str, word index)
{
	if(str == index)
		str = 0;
	else if(str > index)
		--str;
}

int32_t strlist_del()
{
	if(msglistcache[strlist_dlg[2].d1]>0 && msglistcache[strlist_dlg[2].d1]<msg_count-1)
		return D_CLOSE;
		
	return D_O_K;
}

void call_stringedit_dialog(size_t ind, int32_t templateID, int32_t addAfter);
int32_t onStrings()
{
	if(!strlist_dlg[0].d1)
	{
		large_dialog(strlist_dlg,2);
	}
	
	strlist_dlg[0].dp2=get_zc_font(font_lfont);
	int32_t index=0;
	char msgmore_xstring[5], msgmore_ystring[5], msgspeed_string[5], template_string[6];
	int32_t morex=zinit.msg_more_x;
	int32_t morey=zinit.msg_more_y;
	int32_t msgspeed = zinit.msg_speed;
	sprintf(msgmore_xstring, "%d", zinit.msg_more_x);
	sprintf(msgmore_ystring, "%d", zinit.msg_more_y);
	sprintf(msgspeed_string, "%d", zinit.msg_speed);
	static int32_t tid = 0;
	sprintf(template_string, "%d", tid);
	
	char tempbuf[50];
	sprintf(tempbuf, "0");
	
	strlist_dlg[17].d1=0;
	build_bistringcat_list();
	//Message more is offset
	strlist_dlg[15].flags=(zinit.msg_more_is_offset!=0)?D_SELECTED:0;
	SETFLAG(strlist_dlg[23].flags, D_SELECTED, strlist_numerical_sort);
	SETFLAG(strlist_dlg[11].flags, D_DISABLED, strlist_numerical_sort);
	SETFLAG(strlist_dlg[12].flags, D_DISABLED, strlist_numerical_sort);
	SETFLAG(strlist_dlg[13].flags, D_DISABLED, strlist_numerical_sort);
	SETFLAG(strlist_dlg[14].flags, D_DISABLED, strlist_numerical_sort);
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
				MsgStr* tmp = MsgStrings;
				MsgStrings = new MsgStr[MAXMSGS];
				for(auto q = 0; q < msg_strings_size; ++q)
					MsgStrings[q] = tmp[q];
				for(auto q = msg_strings_size; q < MAXMSGS; ++q)
					MsgStrings[q].clear();
				delete[] tmp;
				// MsgStrings=(MsgStr*)_al_sane_realloc(MsgStrings,sizeof(MsgStr)*MAXMSGS);
				// memset((void*)(&MsgStrings[msg_strings_size]), 0, sizeof(MsgStr)*(MAXMSGS-msg_strings_size));
				msg_strings_size = MAXMSGS;
				init_msgstrings(msg_count, msg_strings_size);
			}
			
			init_msgstr(&(MsgStrings[msg_count]));
			MsgStrings[msg_count].listpos = msg_count;
			MsgStrings[msg_count++].s = "<New String>";
		}
		
		strlist_dlg[7].dp=msgmore_xstring;
		strlist_dlg[8].dp=msgmore_ystring;
		strlist_dlg[10].dp=msgspeed_string;
		strlist_dlg[22].dp=template_string;
		
		int32_t ret=zc_popup_dialog(strlist_dlg,2);
		int32_t pos = strlist_dlg[2].d1;
		auto res = msglistcache.find(pos);
		if(res == msglistcache.end())
			(void)addtomsglist(pos);
		index=msglistcache[pos];
		
		bool doedit=false;
		int32_t addAfter = -1;
		
		switch(ret)
		{
			case 18:
				jwin_alert("String Index","Create a string beginning with two hyphens '--'",
						   "and it will be listed in this index, serving as a",
						   "shortcut to a location in your string list.","O&K",NULL,'k',0,get_zc_font(font_lfont));
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
				int32_t nextlistpos = MsgStrings[index].listpos-1;
				
				if(nextlistpos<0)
					break;
					
				std::map<int32_t,int32_t>::iterator res = msglistcache.find(nextlistpos);
				
				if(res == msglistcache.end())
					(void)addtomsglist(nextlistpos);
					
				int32_t otherindex = msglistcache[nextlistpos];
				
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
				int32_t nextlistpos = MsgStrings[index].listpos+1;
				
				if(nextlistpos>=msg_count-1)
					break;
					
				std::map<int32_t,int32_t>::iterator res = msglistcache.find(nextlistpos);
				
				if(res == msglistcache.end())
					(void)addtomsglist(nextlistpos);
					
				int32_t otherindex = msglistcache[nextlistpos];
				
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
					
				int32_t diff = (ret == 13) ? zc_max(MsgStrings[index].listpos-12, 1) : zc_min(MsgStrings[index].listpos+12, msg_count-2);
				int32_t sign = (ret == 13) ? -1 : 1;
				
				int32_t nextindex;
				
				// For all strings above or below, de/increment
				for(int32_t i=MsgStrings[index].listpos; i!=diff+sign; i+=sign)
				{
					std::map<int32_t,int32_t>::iterator res = msglistcache.find(i);
					
					if(res == msglistcache.end())
						(void)addtomsglist(i);
						
					int32_t otherindex = msglistcache[i];
					
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
			{
				std::string buf;
				char shortbuf[73];
				memset(shortbuf, 0, 73);
				buf = MsgString(index, true, false);
				strip_extra_spaces(buf);
				shorten_string(shortbuf, buf.c_str(), font, 72, 288);

				if (jwin_alert("Confirm Clear", "Clear this message string?", " ", shortbuf, "Yes", "No", 'y', 27, get_zc_font(font_lfont)) == 1)
				{
					saved = false;
					word pos = MsgStrings[index].listpos;
					memset((void*)(&MsgStrings[index]), 0, sizeof(MsgStr));
					MsgStrings[index].x = 24;
					MsgStrings[index].y = 32;
					MsgStrings[index].w = 24 * 8;
					MsgStrings[index].h = 3 * 8;
					MsgStrings[index].listpos = pos; // Since the stuff below isn't being run, do this instead


					/*// Go through strings with higher listpos and decrement listpos
					for(int32_t j=MsgStrings[index].listpos; j<msg_count; j++)
					{
						MsgStrings[addtomsglist(j)].listpos--;
					}
					// Delete string
					for(int32_t i=index; i<msg_strings_size-1; i++)
						MsgStrings[i]=MsgStrings[i+1];
					// This is now inaccurate
					msglistcache.clear();
					reset_msgstr(msg_strings_size-1);
					--msg_count;
					int32_t sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;

					for(int32_t s=0; s<sc; s++)                           //room strings
					{
						fix_string(TheMaps[s].str, index);
					}
					for(int32_t i=0; i<16; i++)                           //info strings
					{
						for(int32_t j=0; j<3; j++)
						{
							fix_string(misc.info[i].str[j], index);
						}
					}
					fix_string(misc.endstring, index);              //ending string */
					// Fix the quick-category menu
					strlist_dlg[17].d1 = 0;
					build_bistringcat_list();

					refresh(rMENU);
				}
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
					saved = false;
				}
				
				break;
			case 23: // sort
				strlist_numerical_sort = (strlist_dlg[23].flags & D_SELECTED)?true:false;
				SETFLAG(strlist_dlg[11].flags, D_DISABLED, strlist_numerical_sort);
				SETFLAG(strlist_dlg[12].flags, D_DISABLED, strlist_numerical_sort);
				SETFLAG(strlist_dlg[13].flags, D_DISABLED, strlist_numerical_sort);
				SETFLAG(strlist_dlg[14].flags, D_DISABLED, strlist_numerical_sort);
				for(auto q = 1; q < msg_count-1; ++q)
				{
					msglistcache[strlist_numerical_sort ? q : MsgStrings[q].listpos] = q;
				}
				break;
		}
		
		if(hasroom)
		{
			--msg_count;
		}
		
		if(index>0 && doedit)
		{
			int32_t lp = addAfter>=0 ? MsgStrings[addAfter].listpos : -1;
			int32_t templateID=atoi(static_cast<char*>(strlist_dlg[22].dp));
			
			auto oldspeed = zinit.msg_speed;
			zinit.msg_speed=atoi(msgspeed_string);
			call_stringedit_dialog(size_t(index), templateID, addAfter);
			zinit.msg_speed=oldspeed;
			if(MsgStrings[index].listpos!=msg_count) // Created new string
			{
				// Select the new message
				strlist_dlg[2].d1 = MsgStrings[index].listpos;
				// if((editmsg_dlg[32].flags&D_SELECTED)!=0)
				// {
					// int32_t prev=msg_at_pos(MsgStrings[index].listpos-1);
					// MsgStrings[prev].nextstring=index;
				// }
				// editmsg_dlg[27].flags=(MsgStrings[index].stringflags&STRINGFLAG_CONT)?D_SELECTED:0;
			}
			else if(lp>=0)              // Canceled or edited an existing string
				strlist_dlg[2].d1 = lp; // Select previously selected message
			
			// Fix the quick-category menu
			strlist_dlg[17].d1=0;
			build_bistringcat_list();
			refresh(rMENU);
		}
	}
	tid = atoi(static_cast<char*>(strlist_dlg[22].dp));
	if(tid < 0 || tid >= msg_count)
		tid = 0;
	//if(smsg!=NULL) delete [] smsg;
	return D_O_K;
}

char namebuf[9] = "[NAME]";
// Returns the actual string of a given listpos
int32_t addtomsglist(int32_t index, bool allow_numerical_sort)
{
	if(index==0)
		return 0; // '(None)' is always at the top
		
	if(index==msg_count)
		return msg_count; // '<New String>' is always at the bottom
		
	int32_t pos = 0;
	if(allow_numerical_sort && strlist_numerical_sort)
	{
		msglistcache[index] = pos = index;
	}
	else
	{
		// Easy heuristic:
		// - Search backwards if index>(msg_count/2)
		int32_t increment = 1;
		int32_t i = 0;
		
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
	}
	return pos;
}

extern std::string msgfont_str[font_max];

const char *msgfontlist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = font_max;
		return NULL;
	}
	
	return msgfont_str[index].c_str();
}

void strip_trailing_spaces(char *str)
{
	for(int32_t i=0; str[i]; i++)
	{
		if(str[i]==' ')
		{
			for(int32_t k=i+1; true; k++)
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
void strip_trailing_spaces(std::string& str)
{
	str = str.substr(0, str.find_last_not_of(' ')+1);
}

//Not really the right usage of "parse"...
std::string parse_msg_str(std::string const& s)
{
	std::string smsg;
	
	for(uint32_t i=0; i<s.size() && smsg.size()<MSG_NEW_SIZE; i++)
	{
		// Is it a backslash-escaped number?
		if(s[i]=='\\')
		{
			int32_t msgcc = 0;
			byte twofiftyfives = 0;
			byte digits = 0;
			
			bool neg = false;
			if(i+1 < s.size() && s[i+1] == '-')
			{
				neg = true;
				++i;
			}
			// Read the entire number
			while(i+1<s.size() && s[i+1]>='0' && s[i+1]<='9' && ++digits <= 5)
			{
				++i;
				msgcc*=10; // Move the current number one decimal place right.
				msgcc+=byte(s[i]-'0');
				
				// A hack to allow multi-byte numbers.
				if(msgcc >= 254)
				{
					twofiftyfives = (msgcc/254)<<0;
				}
			}
			if(neg)
			{
				msgcc = MAX_SCC_ARG;
				twofiftyfives = (msgcc/254)<<0;
			}
			smsg += (char)((msgcc % 254) + 1); // As 0 is null, we must store codes 1 higher than their actual value...

			// A hack to allow multi-byte numbers, continued
			if(twofiftyfives > 0)
			{
				smsg += (char)0xff;
				smsg += (char)twofiftyfives;
			}
		}
		else
		{
			smsg += (char)((s[i] >= 32 && s[i] <=126) ? s[i] : ' ');
		}
	}
	
	return smsg;
}

//Make sure this is synchronised with parsemsgcode in guys.cpp!
static int32_t ssc_tile_hei = -1;
word grab_next_argument(std::string const& s2, uint32_t* i)
{
	if(s2.size() <= *i)
		return 0;
	byte val=s2[(*i)++]-1;
	word ret=val;
	
	if(s2.size() <= (*i)+1)
		return ret;
	// If an argument is succeeded by 255, then it's a three-byte argument -
	// between 254 and 65535 (or whatever the maximum actually is)
	if((uint8_t)(s2[(*i)]) == 255)
	{
		val=s2[(*i)+1];
		word next=val;
		ret += 254*next;
		(*i)+=2;
	}
	
	return ret;
}

int32_t mprvfont=0;
int32_t mprvflags=0;
int32_t mprvvspace=0;
int32_t mprvhspace=0;
int32_t mprvnextstring=0;
int32_t mprvw=0;
int32_t mprvh=0;

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
char* encode_msg_str(std::string const& message)
{
	memset(msgbuf, 0, sizeof(msgbuf));
	char const* str = message.c_str();
	int len = strlen(str);
	int32_t strPos=0;
	int32_t msgbufPos=0;
	byte nextChar;
	char sccBuf[30]; // For string control codes
	char sccArgBuf[10];
	word sccArg;
	int32_t sccNumArgs;
	bool lastWasSCC=false;
	
	while(msgbufPos<MSGBUF_SIZE && strPos<MSG_NEW_SIZE+1)
	{
		nextChar=str[strPos];
		
		if(nextChar=='\0')
			return msgbuf;
			
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
			if(nextChar>=32 && nextChar <= 126)
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
					if(strPos>=len)
						return msgbuf;
						
					// If the argument is 253 or less, it's stored in one byte
					// with one added so that it's not null.
					nextChar=str[strPos];
					sccArg=nextChar-1;
					strPos++;
					
					// If there are at least two more bytes to read, they may
					// be part of the same argument.
					if(strPos<=len-2)
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
					if(sccArg == MAX_SCC_ARG)
						strcpy(sccArgBuf, "\\-1");
					else sprintf(sccArgBuf, "\\%hu", sccArg);
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
	return msgbuf;
}
char* encode_msg_str(int32_t index)
{
	if(index==msg_count) //New string?
	{
		memset(msgbuf, 0, sizeof(msgbuf));
		return msgbuf;
	}
	return encode_msg_str(MsgStrings[index].s.c_str());
}

// Fix the strings if they were broken
void fix_string_list()
{
	int32_t found[MAXMSGS];
	bool corrupted = false;
	memset(found, 0, MAXMSGS);
	
	for(int32_t i=0; i<msg_count; i++)
	{
		found[MsgStrings[i].listpos] = 1;
	}
	
	for(int32_t i=0; i<msg_count; i++)
	{
		if(found[i]==0)
			corrupted=true;
	}
	
	if(corrupted)
	{
		for(int32_t i=0; i<msg_count; i++)
		{
			MsgStrings[i].listpos = i;
		}
		
		jwin_alert("Notice","Your quest's string ordering was corrupted.","It has been reverted to the default order.",NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}
}

void rebuild_string_list()
{
	fix_string_list();
	msglistcache.clear();
}

void reset_msgstr(int32_t index)
{
	bound(index,0,msg_strings_size-1);
	MsgStrings[index].s = "";
	MsgStrings[index].nextstring=0;
	//memset(MsgStrings[index].expansion, 0, 32);
}

// Get the number of the message at the given list position
int32_t msg_at_pos(int32_t pos)
{
	for(int32_t i=0; i<msg_count; i++)
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

bool is_msgc(byte cc)
{
	switch(cc)
	{
		case MSGC_COLOUR: case MSGC_SPEED: case MSGC_GOTOIFGLOBAL:
		case MSGC_GOTOIFRAND: case MSGC_GOTOIF: case MSGC_GOTOIFCTR:
		case MSGC_GOTOIFCTRPC: case MSGC_GOTOIFTRI:
		case MSGC_GOTOIFTRICOUNT: case MSGC_CTRUP: case MSGC_CTRDN:
		case MSGC_CTRSET: case MSGC_CTRUPPC: case MSGC_CTRDNPC:
		case MSGC_CTRSETPC: case MSGC_GIVEITEM: case MSGC_TAKEITEM:
		case MSGC_WARP: case MSGC_SETSCREEND: case MSGC_SFX:
		case MSGC_MIDI: case MSGC_NAME: case MSGC_GOTOIFCREEND:
		//case MSGC_CHANGEPORTRAIT:
		case MSGC_NEWLINE: case MSGC_SHDCOLOR:
		case MSGC_SHDTYPE: case MSGC_DRAWTILE: case MSGC_ENDSTRING:
		case MSGC_WAIT_ADVANCE: case MSGC_SETUPMENU:
		case MSGC_MENUCHOICE: case MSGC_RUNMENU:
		case MSGC_GOTOMENUCHOICE: case MSGC_TRIGSECRETS:
		case MSGC_SETSCREENSTATE: case MSGC_SETSCREENSTATER:
		case MSGC_FONT: case MSGC_RUN_FRZ_GENSCR:
			return true;
	}
	return false;
}
int32_t msg_code_operands(byte cc)
{
	switch(cc)
	{
		case MSGC_WARP:
			return 6; //dmap, screen, x, y, effect, sound
		
		//portrait tile, x, y, width, height
		case MSGC_CHANGEPORTRAIT:
		case MSGC_GOTOIFCREEND:
		case MSGC_DRAWTILE:
		case MSGC_SETUPMENU:
		case MSGC_MENUCHOICE:
			return 5;
		
		case MSGC_SETSCREEND:
		case MSGC_SETSCREENSTATER:
			return 4;

		case MSGC_GOTOIFGLOBAL:
		case MSGC_GOTOIFCTRPC:
		case MSGC_GOTOIFCTR:
			return 3;
			
		case MSGC_COLOUR:
		case MSGC_SHDCOLOR:
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
		case MSGC_GOTOMENUCHOICE:
		case MSGC_SETSCREENSTATE:
		case MSGC_RUN_FRZ_GENSCR:
			return 2;
			
		case MSGC_NEWLINE:
		case MSGC_NAME:
		case MSGC_RUNMENU:
		case MSGC_ENDSTRING:
		case MSGC_WAIT_ADVANCE:
			return 0;
	}
	
	return 1;
}


// Category list functions


void build_bistringcat_list()
{
	memset(bistringcat,0,256*sizeof(int32_t));
	bistringcat_cnt=1;
	
	for(int32_t i = 0; i < msg_count && bistringcat_cnt<256; i++)
	{
		int32_t m =  addtomsglist(i);
		
		if(MsgStrings[m].s.size() > 1 && MsgStrings[m].s[0]=='-' && MsgStrings[m].s[1]=='-')
		{
			bistringcat[bistringcat_cnt]=m;
			++bistringcat_cnt;
		}
	}
}

const char *stringcatlist(int32_t index, int32_t *list_size)
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

int32_t d_msgtile_proc(int32_t msg,DIALOG *d,int32_t c)
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
		int32_t dw = d->w / 2;
		int32_t dh = d->h / 2;
		
		BITMAP *buf = create_bitmap_ex(8,dw,dh);
		
		if(buf)
		{
			clear_bitmap(buf);
			
			if(d->d1)
				puttile16(buf,d->d1,0,0,d->fg,0);
				
			stretch_blit(buf,screen,0,0,dw,dh,d->x,d->y,dw*2,dh*2);
			destroy_bitmap(buf);
		}
	}
	}
	
	return D_O_K;
}
