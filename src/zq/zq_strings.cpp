#include "zq/zq_strings.h"
#include "gui/editbox.h"
#include "gui/EditboxNew.h"
#include "base/gui.h"
#include "gui/jwin.h"
#include "dialog/alert.h"
#include "base/qst.h"
#include "tiles.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/msgstr.h"
#include "zc/ffscript.h"
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
std::string parse_to_legacy_msg_str_encoding(std::string const& s);
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
	
	{ jwin_iconbutton_proc,266,     74,     16,     16,   vc(14),  vc(1),  27,  D_EXIT, BTNICON_ARROW_UP,    0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,266,     99,     16,     16,   vc(14),  vc(1),  27,  D_EXIT, BTNICON_ARROW_DOWN,  0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,266,     44,     16,     16,   vc(14),  vc(1),  27,  D_EXIT, BTNICON_ARROW_UP,    0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,266,    129,     16,     16,   vc(14),  vc(1),  27,  D_EXIT, BTNICON_ARROW_DOWN,  0, NULL, NULL, NULL },
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

static int32_t zqstr_copysrc=-1;
static bool string_advpaste(int indx)
{
	static bitstring pasteflags;
	static const vector<CheckListInfo> advp_names =
	{
		{ "Text" },
		{ "NextString" },
		{ "X,Y,W,H" },
		{ "Portrait" },
		{ "Background" },
		{ "Font" },
		{ "SFX" },
		{ "Spacing" },
		{ "Margins" },
		{ "Shadow" },
		{ "Layer" },
		{ "Flags" },
	};
	static const string title = "Advanced Paste";
	static const string title_all = "Advanced Paste to All";
	bool to_all = indx < 0;
	if(!call_checklist_dialog(to_all ? title_all : title,advp_names,pasteflags))
		return false;
	
	if(to_all)
	{
		for(int q = 0; q < msg_count-1; ++q)
			MsgStrings[q].advpaste(MsgStrings[zqstr_copysrc], pasteflags);
	}
	else MsgStrings[indx].advpaste(MsgStrings[zqstr_copysrc], pasteflags);
	return true;
}
void strlist_rclick_func(int32_t index, int32_t x, int32_t y)
{
	// Don't do anything on (none) or <New String>
	if(index<=0 || index==msg_count-1)
		return;
	
	// Disable paste options if the copy source is invalid
	bool no_pasting = (zqstr_copysrc<=0 || zqstr_copysrc>=msg_count);
	NewMenu rcmenu
	{
		{ "Copy", [&]()
			{
				zqstr_copysrc = msg_at_pos(index);
			} },
		{},
		{ "Paste Text", [&]()
			{
				MsgStrings[msg_at_pos(index)].copyText(MsgStrings[zqstr_copysrc]);
				strlist_dlg[2].flags |= D_DIRTY;
				saved = false;
			}, nullopt, no_pasting },
		{ "Paste", [&]()
			{
				//Overloaded assignment copies both
				MsgStrings[msg_at_pos(index)] = MsgStrings[zqstr_copysrc];
				strlist_dlg[2].flags|=D_DIRTY;
				saved = false;
			}, nullopt, no_pasting },
		{ "Adv. Paste", [&]()
			{
				string_advpaste(msg_at_pos(index));
				strlist_dlg[2].flags|=D_DIRTY;
				saved = false;
			}, nullopt, no_pasting },
		{ "Adv. Paste to All", [&]()
			{
				string_advpaste(-1);
				strlist_dlg[2].flags|=D_DIRTY;
				saved = false;
			}, nullopt, no_pasting },
		{},
		{ "Set As Template", [&]()
			{
				sprintf(static_cast<char*>(strlist_dlg[22].dp), "%d", msg_at_pos(index));
				strlist_dlg[22].flags |= D_DIRTY;
			} },
	};
	rcmenu.pop(x, y);
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
		
		tmpsrc++;
	}
	
	*tmpsrc=0;
	strcpy(string,src);
	free(src);
	return string;
}

void strip_extra_spaces(std::string &string)
{
	string = string.substr(string.find_first_not_of(' '), string.find_last_not_of(' '));
}

bool scc_insert(char* buf, size_t& msgptr,byte cc,size_t limit = -1)
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
		default:
			return false;
	}

	return true;
}

char *MsgString(int32_t index, bool show_number, bool pad_number)
{
	static char u[100];
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
	bool indent = index>0 && prevIndex>=0 && MsgStrings[prevIndex].nextstring==index;
	sprintf(u, pad_number?"%s%3d":"%s%d",indent?"--> ":"",index);
	char *s=strcat(u,": ");
	
	char *t = new char[71];
	memset(t, 0, 71);
	
	size_t msgptr=0;

	auto msg_it = MsgStrings[index].create_iterator();

	msg_it.next();

	// Skip leading spaces.
	while (!msg_it.done())
	{
		if (msg_it.state == MsgStr::iterator::COMMAND)
		{
			if (scc_insert(t, msgptr, msg_it.command.code, 70))
			{
				msg_it.next();
				break;
			}
		}
		else if (msg_it.state == MsgStr::iterator::CHARACTER)
		{
			if (msg_it.character != " ")
				break;
		}

		msg_it.next();
	}

	while (!msg_it.done() && msgptr < 70)
	{
		if (msg_it.state == MsgStr::iterator::COMMAND)
		{
			scc_insert(t, msgptr, msg_it.command.code, 70);
		}
		else if (!msg_it.character.empty())
		{
			t[msgptr++] = msg_it.character[0];
		}

		msg_it.next();
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
			MsgStrings[msg_count++].setFromLegacyEncoding("<New String>");
		}
		
		strlist_dlg[7].dp=msgmore_xstring;
		strlist_dlg[8].dp=msgmore_ystring;
		strlist_dlg[10].dp=msgspeed_string;
		strlist_dlg[22].dp=template_string;
		
		int32_t ret=do_zqdialog(strlist_dlg,2);
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
					MsgStrings[index].listpos = pos;
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
			}
			else if(lp>=0)              // Canceled or edited an existing string
				strlist_dlg[2].d1 = lp; // Select previously selected message
			
			// Fix the quick-category menu
			strlist_dlg[17].d1=0;
			build_bistringcat_list();
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

std::string parse_to_legacy_msg_str_encoding(std::string const& s)
{
	std::string smsg;
	
	for(uint32_t i=0; i<s.size() && smsg.size()< MSGBUF_SIZE; i++)
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

int32_t mprvfont=0;
int32_t mprvflags=0;
int32_t mprvvspace=0;
int32_t mprvhspace=0;
int32_t mprvnextstring=0;
int32_t mprvw=0;
int32_t mprvh=0;

char* encode_msg_str(int32_t index)
{
	if(index==msg_count) //New string?
	{
		memset(msgbuf, 0, sizeof(msgbuf));
		return msgbuf;
	}

	std::string str = MsgStrings[index].serialize();
	strncpy(msgbuf, str.c_str(), sizeof(msgbuf));
	return msgbuf;
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
		
		if(FFCore.getQuestHeaderInfo(vZelda) >= 0x250) //not an error before this, don't pop up
		{
			jwin_alert("Notice","Your quest's string ordering was corrupted.",
				"It has been reverted to the default order.",NULL,
				"O&K",NULL,'k',0,get_zc_font(font_lfont));
		}
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
	MsgStrings[index].setFromLegacyEncoding("");
	MsgStrings[index].nextstring=0;
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
	return get_scc_command_num_args(cc).has_value();
}

int32_t msg_code_operands(byte cc)
{
	return get_scc_command_num_args(cc).value_or(0);
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
