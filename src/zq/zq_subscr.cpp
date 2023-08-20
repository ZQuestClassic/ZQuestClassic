//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/gui.h"
#include "subscr.h"
#include "zq/zq_subscr.h"
#include "jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "zq/zq_misc.h"
#include "tiles.h"
#include "qst.h"
#include "init.h"
#include <assert.h>
#include <fmt/format.h>
#include "dialog/info.h"
#include "dialog/alert.h"
#include "dialog/subscr_props.h"
#include "dialog/info_lister.h"

#ifndef _MSC_VER
#include <strings.h>
#endif
#include <string.h>

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

extern void large_dialog(DIALOG *d);
extern void large_dialog(DIALOG *d, float RESIZE_AMT);
extern bool zq_ignore_item_ownership;
bool zq_view_fullctr = false, zq_view_maxctr = false,
	zq_view_noinf = false, zq_view_allinf = false;
bool subscr_confirm_delete = true;

int32_t curr_widg;
char str_oname[512];
char str_cpyname[512];
char subscr_namebuf[512];
ZCSubscreen subscr_edit;
bool sso_selection[MAXSUBSCREENITEMS];
SubscrWidget* propCopyWidg = nullptr;
int copied_page = -1;
enum
{
	ssmouCLASSIC,
	ssmouMODERN,
	ssmouMAX
};
int ssmouse_type = ssmouMODERN;
int clicked_obj = -1;
byte ssmouse_flags = 0;
int dragx, dragy;
#define ssmflCLICKED   0x01
#define ssmflDRAG      0x02
#define ssmflRCLICK    0x04

gamedata *game;

void delete_subscreen(int32_t subscreenidx);
SubscrWidget* create_new_widget_of(int32_t type, int x = 0, int y = 0);
SubscrWidget* create_new_widget(int x = 0, int y = 0);

static ListData item_list(itemlist_num, &font);

int32_t sso_properties(SubscrWidget* widg, int32_t obj_ind)
{
    return call_subscrprop_dialog(widg,obj_ind) ? 0 : -1;
}

int32_t onBringToFront();
int32_t onBringForward();
int32_t onSendBackward();
int32_t onSendToBack();
int32_t onReverseArrangement();
int32_t onAlignLeft();
int32_t onAlignCenter();
int32_t onAlignRight();
int32_t onAlignTop();
int32_t onAlignMiddle();
int32_t onAlignBottom();
int32_t onDistributeLeft();
int32_t onDistributeCenter();
int32_t onDistributeRight();
int32_t onDistributeTop();
int32_t onDistributeMiddle();
int32_t onDistributeBottom();
int32_t onGridSnapLeft();
int32_t onGridSnapCenter();
int32_t onGridSnapRight();
int32_t onGridSnapTop();
int32_t onGridSnapMiddle();
int32_t onGridSnapBottom();
static int32_t onToggleConfDelete();

void subscr_properties(int indx)
{
	SubscrPage& pg = subscr_edit.cur_page();
	if(unsigned(indx) < pg.contents.size())
	{
		SubscrWidget* widg = pg.contents[indx];
		if(sso_properties(widg,indx)!=-1)
		{
			for(int32_t i=0; i<pg.contents.size(); i++)
			{
				if(!sso_selection[i] || i==indx)
					continue;
				
				pg.contents[i]->copy_prop(widg);
			}
		}
	}
}

void copy_properties(int indx)
{
	SubscrPage& pg = subscr_edit.cur_page();
	if(unsigned(indx) < pg.contents.size())
	{
		SubscrWidget* widg = pg.contents[indx];
		if(propCopyWidg)
			delete propCopyWidg;
		if(widg)
		{
			propCopyWidg = widg->clone();
			copied_page = subscr_edit.curpage;
		}
		else
		{
			propCopyWidg = nullptr;
			copied_page = -1;
		}
		update_sso_name();
	}
}
void paste_properties(int indx)
{
	SubscrPage& pg = subscr_edit.cur_page();
	if(propCopyWidg) // Hopefully unnecessary
	{
		SubscrPage& pg = subscr_edit.cur_page();
		for(int32_t i=0; i<pg.contents.size(); i++)
		{
			if(i == indx || sso_selection[i])
				pg.contents[i]->copy_prop(propCopyWidg);
		}
	}
}
void paste_widget_to(int indx)
{
	if(propCopyWidg)
	{
		SubscrWidget* targ = subscr_edit.cur_page().contents[indx];
		auto tx = targ->x, ty = targ->y;
		targ->copy_prop(propCopyWidg,true);
		targ->x = tx; targ->y = ty;
	}
}
SubscrWidget* paste_widget_new()
{
	if(!propCopyWidg) return nullptr;
	SubscrPage& pg = subscr_edit.cur_page();
    
	SubscrWidget* widg = propCopyWidg->clone();
	if(!widg) return nullptr;
	
	if(copied_page == subscr_edit.curpage)
	{
		widg->x+=zc_max(zinit.ss_grid_x,8);
		widg->y+=zc_max(zinit.ss_grid_y,8);
	}
	pg.contents.push_back(widg);
    
    update_sso_name();
    update_up_dn_btns();
	return widg;
}
SubscrWidget* paste_widget_new_at(int x, int y)
{
	SubscrWidget* widg = nullptr;
	if(widg = paste_widget_new())
	{
		widg->x = x;
		widg->y = y;
	}
	return widg;
}
int* force_paste_xy = nullptr;
int32_t onDuplCopiedWidget()
{
	if(force_paste_xy)
		paste_widget_new_at(force_paste_xy[0],force_paste_xy[1]);
    else paste_widget_new();
    return D_O_K;
}
int32_t onNewWidget()
{
	if(force_paste_xy)
		create_new_widget(force_paste_xy[0],force_paste_xy[1]);
    else create_new_widget();
    return D_O_K;
}
int32_t onDuplicateWidget()
{
	SubscrPage& pg = subscr_edit.cur_page();
    size_t objs = pg.contents.size();
    
    if(objs==0 || (key_shifts&KB_SHIFT_FLAG))
		return onDuplCopiedWidget();
    
    for(int32_t i=0; i<objs; ++i)
    {
        if(sso_selection[i] || i==curr_widg)
        {
			SubscrWidget* widg = pg.contents[i]->clone();
			if(!widg) continue;
			
            widg->x+=zc_max(zinit.ss_grid_x,8);
            widg->y+=zc_max(zinit.ss_grid_y,8);
			pg.contents.push_back(widg);
        }
    }
    
    update_sso_name();
    update_up_dn_btns();
    
    return D_O_K;
}

int32_t onWidgetProperties()
{
	subscr_properties(curr_widg);
	return D_O_K;
}
int32_t onSubscrCopy()
{
	copy_properties(curr_widg);
	return D_O_K;
}
int32_t onSubscrPaste()
{
	if(subscr_edit.cur_page().contents.empty())
		return onDuplCopiedWidget();
	if(key_shifts & KB_CTRL_FLAG)
		paste_widget_to(curr_widg);
	else paste_properties(curr_widg);
	return D_O_K;
}
int32_t onSubscrPasteAll()
{
	paste_widget_to(curr_widg);
	return D_O_K;
}
int32_t onSubscrPasteProps()
{
	paste_properties(curr_widg);
	return D_O_K;
}

int32_t onNewSubscreenObject();

bool delete_widget()
{
	SubscrPage& pg = subscr_edit.cur_page();
    size_t objs=pg.contents.size();
    
    if(objs==0)
        return false;
    
	pg.delete_widg(curr_widg);
	
	//...shift the selection array
    for(int32_t i=curr_widg; i<objs-1; ++i)
    {
        sso_selection[i]=sso_selection[i+1];
    }
    
    sso_selection[objs-1]=false;
    
    if(curr_widg==objs-1)
        --curr_widg;
    
    update_sso_name();
    update_up_dn_btns();
    
    return true;
}
int32_t onDeleteWidgetC()
{
	SubscrPage& pg = subscr_edit.cur_page();
    size_t objs=pg.contents.size();
    
    if(objs==0)
        return D_O_K;
	auto* widg = pg.get_widget(curr_widg);
	if(widg)
	{
		bool run = true;
		if(subscr_confirm_delete)
		{
			run = false;
			AlertDialog("Delete Widget?",
				fmt::format("Are you sure you want to delete widget {}, {}?",
					curr_widg, widg->getTypeName()),
				[&](bool ret,bool dsa)
				{
					run = ret;
					if(dsa)
						onToggleConfDelete();
				},
				"Yes","No",
				0,false, //timeout - none
				true //"Don't show this again"
			).show();
		}
		if(run)
			delete_widget();
	}
	return D_O_K;
}
int32_t onAddToSelection()
{
    if(curr_widg >= 0)
    {
        sso_selection[curr_widg]=true;
    }
    
    return D_O_K;
}

int32_t onRemoveFromSelection()
{
    if(curr_widg >= 0)
    {
        sso_selection[curr_widg]=false;
    }
    
    return D_O_K;
}

int32_t onInvertSelection()
{
    for(int32_t i=0; i<subscr_edit.cur_page().contents.size(); ++i)
    {
        sso_selection[i]=!sso_selection[i];
    }
    
    return D_O_K;
}

int32_t onClearSelection()
{
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        sso_selection[i]=false;
    }
    
    return D_O_K;
}

static int32_t onToggleInvis();
static int32_t onToggleShowUnowned();
static int32_t onToggleMaxCtr();
static int32_t onToggleMaxMaxCtr();
static int32_t onToggleNoInf();
static int32_t onToggleAllInf();
static int32_t onEditGrid();
static int32_t onSelectionOptions();
static int32_t onShowHideGrid();

static MENU ss_arrange_menu[] =
{
    { (char *)"Bring to Front",       onBringToFront,          NULL, 0, NULL },
    { (char *)"Bring Forward",        onBringForward,          NULL, 0, NULL },
    { (char *)"Send Backward",        onSendBackward,          NULL, 0, NULL },
    { (char *)"Send to Back",         onSendToBack,            NULL, 0, NULL },
    { (char *)"Reverse",              onReverseArrangement,    NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_grid_snap_menu[] =
{
    { (char *)"Left Edges",           onGridSnapLeft,          NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onGridSnapCenter,        NULL, 0, NULL },
    { (char *)"Right Edges",          onGridSnapRight,         NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onGridSnapTop,           NULL, 0, NULL },
    { (char *)"Vertical Centers",     onGridSnapMiddle,        NULL, 0, NULL },
    { (char *)"Bottom Edges",         onGridSnapBottom,        NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_align_menu[] =
{
    { (char *)"Left Edges",           onAlignLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onAlignCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onAlignRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onAlignTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onAlignMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onAlignBottom,           NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"To Grid",              NULL,                    ss_grid_snap_menu, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_distribute_menu[] =
{
    { (char *)"Left Edges",           onDistributeLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onDistributeCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onDistributeRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                         NULL, 0, NULL },
    { (char *)"Top Edges",            onDistributeTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onDistributeMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onDistributeBottom,           NULL, 0, NULL },
    { NULL,                           NULL,                         NULL, 0, NULL }
};

static MENU ss_copypaste_menu[] =
{
	{ (char *)"&Copy Widget ",          onSubscrCopy,          NULL, 0, NULL },
    { (char *)"&Duplicate Widget ",     onDuplicateWidget,     NULL, 0, NULL },
    { (char *)"Paste Properties\t&V",   onSubscrPasteProps,    NULL, 0, NULL },
    { (char *)"Paste All ",             onSubscrPasteAll,      NULL, 0, NULL },
    { (char *)"",                       NULL,                  NULL, 0, NULL },
    { (char *)"Paste New ",             onDuplCopiedWidget,    NULL, 0, NULL },
    { NULL,                             NULL,                  NULL, 0, NULL }
};
static MENU subscreen_rc_menu[] =
{
    { (char *)"Properties ",            onWidgetProperties,    NULL, 0, NULL },
    { (char *)"New ",                   onNewWidget,           NULL, 0, NULL },
    { (char *)"Delete ",                onDeleteWidgetC,       NULL, 0, NULL },
    { (char *)"",                       NULL,                  NULL, 0, NULL },
    { (char *)"&Copy/Paste   ",         NULL,     ss_copypaste_menu, 0, NULL },
    { (char *)"",                       NULL,                  NULL, 0, NULL },
    { (char *)"&Arrange ",              NULL,       ss_arrange_menu, 0, NULL },
	{ (char *)"&Align ",                NULL,         ss_align_menu, 0, NULL },
	{ (char *)"&Distribute ",           NULL,    ss_distribute_menu, 0, NULL },
    { NULL,                             NULL,                  NULL, 0, NULL }
};
static MENU subscreen_rc_menu_nowidg[] =
{
    { (char *)"New ",                   onNewWidget,           NULL, 0, NULL },
    { (char *)"Paste New ",             onDuplCopiedWidget,    NULL, 0, NULL },
    { NULL,                             NULL,                  NULL, 0, NULL }
};
void update_subscr_dlg(bool start);
int32_t d_subscreen_proc(int32_t msg,DIALOG *d,int32_t)
{
	SubscrPage& pg = subscr_edit.cur_page();
	int hei = subscr_edit.sub_type==sstPASSIVE ? 56 : 168;
	switch(msg)
	{
		case MSG_START:
			clicked_obj = -1;
			ssmouse_flags = 0;
			onClearSelection();
			curr_widg=0;
			update_sso_name();
			update_up_dn_btns();
			return D_O_K;
		
		case MSG_VSYNC:
			d->flags|=D_DIRTY;
			break;
			
		case MSG_DRAW:
		{
			++subscr_item_clk;
			animate_subscr_buttonitems();
			BITMAP *buf = create_bitmap_ex(8,256,hei);
			
			if(buf)
			{
				clear_bitmap(buf);
				ZCSubscreen* subs = (ZCSubscreen*)(d->dp);
				show_custom_subscreen(buf, subs, 0, 0, true, sspUP | sspDOWN | sspSCROLLING);
				
				SubscrPage& pg = subs->cur_page();
				int cur_object = (unsigned(clicked_obj)<pg.contents.size())
					? clicked_obj
					: curr_widg;
				for(int32_t i=0; i<pg.contents.size(); ++i)
				{
					if(sso_selection[i] || i == cur_object)
					{
						auto c = i != cur_object
							? vc(zinit.ss_bbox_2_color)
							: vc(zinit.ss_bbox_1_color);
						sso_bounding_box(buf, pg.contents[i], c);
					}
				}
				
				if(zinit.ss_flags&ssflagSHOWGRID)
				{
					for(int32_t x=zinit.ss_grid_xofs; x<256; x+=zinit.ss_grid_x)
					{
						for(int32_t y=zinit.ss_grid_yofs; y<hei; y+=zinit.ss_grid_y)
						{
							buf->line[y][x]=vc(zinit.ss_grid_color);
						}
					}
				}
				
				stretch_blit(buf,screen,0,0,256,hei,d->x,d->y,d->w,d->h);
				
				destroy_bitmap(buf);
			}
		}
		return D_O_K;
		
		case MSG_WANTFOCUS:
			return D_WANTFOCUS;
	}
	
	bool rclick = false;
	bool shift = key[KEY_LSHIFT]||key[KEY_RSHIFT];
	int rclickobj = curr_widg;
	int scaled_mouse_x = gui_mouse_x(), scaled_mouse_y = gui_mouse_y();
	if(isinRect(scaled_mouse_x,scaled_mouse_y,d->x, d->y, d->x+d->w-1, d->y+d->h-1))
	{
		scaled_mouse_x = round((scaled_mouse_x - d->x) * (256.0/d->w));
		scaled_mouse_y = round((scaled_mouse_y - d->y) * (double(hei)/d->h));
	}
	else scaled_mouse_x = scaled_mouse_y = -999;
	switch(ssmouse_type)
	{
		case ssmouCLASSIC:
		{
			if(msg==MSG_CLICK)
			{
				SubscrPage& pg = subscr_edit.cur_page();
				for(int32_t i=pg.contents.size()-1; i>=0; --i)
				{
					SubscrWidget* widg = pg.contents[i];
					int32_t x=(widg->getX()+widg->getXOffs())*2;
					int32_t y=widg->getY()*2;
					int32_t w=widg->getW()*2;
					int32_t h=widg->getH()*2;
					
					if(isinRect(gui_mouse_x(),gui_mouse_y(),d->x+x, d->y+y, d->x+x+w-1, d->y+y+h-1))
					{
						if(shift)
						{
							if(sso_selection[i]==true)
							{
								sso_selection[i]=false;
							}
							else
							{
								sso_selection[curr_widg]=true;
								curr_widg=i;
								update_sso_name();
								update_up_dn_btns();
							}
						}
						else
						{
							onClearSelection();
							curr_widg=i;
							update_sso_name();
							update_up_dn_btns();
						}
						break;
					}
				}
				
				if(gui_mouse_b()&2) //right mouse button
					rclick = true;
			}
			break;
		}
		case ssmouMODERN:
		{
			switch(msg)
			{
				case MSG_CLICK:
				{
					if(clicked_obj >= pg.contents.size())
						clicked_obj = -1;
					if(shift) ssmouse_flags &= ~ssmflDRAG;
					if(ssmouse_flags&ssmflDRAG)
					{
						if(clicked_obj > -1)
						{
							SubscrWidget* widg = pg.contents[clicked_obj];
							int diffx = (scaled_mouse_x + dragx) - widg->x;
							int diffy = (scaled_mouse_y + dragy) - widg->y;
							widg->x += diffx;
							widg->y += diffy;
							for(int32_t i=pg.contents.size()-1; i>=0; --i)
							{
								if(i==clicked_obj) continue;
								SubscrWidget* widg = pg.contents[i];
								if(sso_selection[i])
								{
									widg->x += diffx;
									widg->y += diffy;
								}
							}
						}
					}
					else //Select hovered object
					{
						for(int32_t i=pg.contents.size()-1; i>=0; --i)
						{
							SubscrWidget* widg = pg.contents[i];
							int32_t x=widg->getX()+widg->getXOffs();
							int32_t y=widg->getY();
							int32_t w=widg->getW();
							int32_t h=widg->getH();
							
							if(isinRect(scaled_mouse_x,scaled_mouse_y,x,y,x+w-1, y+h-1))
							{
								clicked_obj = i;
								break;
							}
						}
					}
					if(!(ssmouse_flags&ssmflCLICKED))
					{
						if(clicked_obj > -1)
						{
							if(shift) //multiselect
							{
								if(sso_selection[clicked_obj]==true)
								{
									sso_selection[clicked_obj]=false;
									clicked_obj = -1;
								}
								else
								{
									sso_selection[curr_widg]=true;
									curr_widg=clicked_obj;
									update_sso_name();
									update_up_dn_btns();
								}
							}
							else if(sso_selection[clicked_obj])
							{
								sso_selection[curr_widg]=true;
								curr_widg=clicked_obj;
								update_sso_name();
								update_up_dn_btns();
							}
							else
							{
								onClearSelection();
								curr_widg=clicked_obj;
								update_sso_name();
								update_up_dn_btns();
							}
						}
						if(gui_mouse_b()&2) //rmouse
							ssmouse_flags |= ssmflRCLICK;
						else if(clicked_obj > -1 && (gui_mouse_b()&1))
						{
							ssmouse_flags |= ssmflDRAG;
							SubscrWidget* widg = pg.contents[clicked_obj];
							dragx = widg->x - scaled_mouse_x;
							dragy = widg->y - scaled_mouse_y;
						}
					}
					ssmouse_flags |= ssmflCLICKED;
					break;
				}
				case MSG_RRELEASE:
					if(ssmouse_flags&ssmflRCLICK)
					{
						rclick = true;
						rclickobj = clicked_obj;
						ssmouse_flags &= ~ssmflRCLICK;
					}
					[[fallthrough]];
				case MSG_LRELEASE:
					if((ssmouse_flags&ssmflCLICKED) && !(gui_mouse_b()&3)) //released click
					{
						clicked_obj = -1;
						ssmouse_flags = 0;
					}
					break;
			}
			break;
		}
	}
	if(rclick)
	{
		int xy[2] = {scaled_mouse_x,scaled_mouse_y};
		force_paste_xy = xy;
		if(unsigned(rclickobj) < pg.contents.size())
		{
			curr_widg = rclickobj;
			object_message(d,MSG_DRAW,0);
			
			update_subscr_dlg(false);
			popup_menu(subscreen_rc_menu,gui_mouse_x(),gui_mouse_y());
			
			clicked_obj = -1;
			ssmouse_flags = 0;
		}
		else
		{
			curr_widg = -1;
			object_message(d,MSG_DRAW,0);
			
			update_subscr_dlg(false);
			popup_menu(subscreen_rc_menu_nowidg,gui_mouse_x(),gui_mouse_y());
			
			clicked_obj = -1;
			ssmouse_flags = 0;
		}
		force_paste_xy = nullptr;
		return D_REDRAW;
	}
	return D_O_K;
}

int32_t onSSUp();
int32_t onSSDown();
int32_t onSSLeft();
int32_t onSSRight();
int32_t onSSPgDn();
int32_t onSSPgUp();

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t onSSUp()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_y:-1;
	
	SubscrPage& pg = subscr_edit.cur_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_widg)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->h+=delta;
			}
			else
			{
				pg.contents[i]->y+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSDown()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_y:1;
	
	SubscrPage& pg = subscr_edit.cur_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_widg)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->h+=delta;
			}
			else
			{
				pg.contents[i]->y+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSLeft()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_x:-1;
	
	SubscrPage& pg = subscr_edit.cur_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_widg)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->w+=delta;
			}
			else
			{
				pg.contents[i]->x+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSRight()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_x:1;
	
	SubscrPage& pg = subscr_edit.cur_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_widg)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->w+=delta;
			}
			else
			{
				pg.contents[i]->x+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t d_ssup_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			onSSUp();
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssdn_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			onSSDown();
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_sslt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			onSSLeft();
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssrt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			onSSRight();
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssup_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			SubscrPage& pg = subscr_edit.cur_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_widg)
				{
					--pg.contents[i]->h;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssdn_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			SubscrPage& pg = subscr_edit.cur_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_widg)
				{
					++pg.contents[i]->h;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_sslt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			SubscrPage& pg = subscr_edit.cur_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_widg)
				{
					--pg.contents[i]->w;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssrt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			SubscrPage& pg = subscr_edit.cur_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_widg)
				{
					++pg.contents[i]->w;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssup_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			subscr_edit.cur_page().move_cursor(SEL_UP);
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssdn_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			subscr_edit.cur_page().move_cursor(SEL_DOWN);
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_sslt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			subscr_edit.cur_page().move_cursor(SEL_LEFT);
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssrt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_iconbutton_proc(msg, d, c);
			subscr_edit.cur_page().move_cursor(SEL_RIGHT);
			return D_O_K;
		}
		break;
	}
	
	return jwin_iconbutton_proc(msg, d, c);
}

const char *sso_textstyle[sstsMAX]=
{
    "sstsNORMAL", "sstsSHADOW", "sstsSHADOWU", "sstsOUTLINE8", "sstsOUTLINEPLUS", "sstsOUTLINEX", "sstsSHADOWED", "sstsSHADOWEDU", "sstsOUTLINED8", "sstsOUTLINEDPLUS", "sstsOUTLINEDX"
};

const char *sso_fontname[ssfMAX]=
{
    "ssfZELDA", "ssfSS1", "ssfSS2", "ssfSS3", "ssfSS4", "ssfZTIME", "ssfSMALL", "ssfSMALLPROP", "ssfZ3SMALL", "ssfGBLA", "ssfZ3",
    "ssfGORON", "ssfZORAN", "ssfHYLIAN1", "ssfHYLIAN2", "ssfHYLIAN3", "ssfHYLIAN4", "ssfPROP", "ssfGBORACLE", "ssfGBORACLEP",
    "ssfDSPHANTOM", "ssfDSPHANTOMP",
	
	"ssfAT800", "ssfACORN", "ssfALLEG", "ssfAPL2", "ssfAPL280", "ssfAPL2GS", "ssfAQUA", "ssfAT400", "ssfC64", "ssfC64HR",
	"ssfCGA", "ssfCOCO", "ssfCOCO2", "ssfCOUPE", "ssfCPC", "ssfFANTASY", "ssfFDSKANA", "ssfFDSLIKE", "ssfFDSROM", "ssfFF",
	"ssfFUTHARK", "ssfGAIA", "ssfHIRA", "ssfJP", "ssfKONG", "ssfMANA", "ssfML", "ssfMOT", "ssfMSX0", "ssfMSX1", "ssfPET",
	"ssfPSTART", "ssfSATURN", "ssfSCIFI", "ssfSHERW", "ssfSINQL", "ssfSPEC", "ssfSPECLG", "ssfTI99", "ssfTRS", "ssfZ2", "ssfZX", "ssfLISA"
	
};

const char *sso_colortype[2]=
{
    "ssctSYSTEM", "ssctMISC"
};

const char *sso_specialcolor[ssctMAX]=
{
    "ssctTEXT", "ssctCAPTION", "ssctOVERWBG", "ssctDNGNBG", "ssctDNGNFG", "ssctCAVEFG", "ssctBSDK", "ssctBSGOAL", "ssctCOMPASSLT", "ssctCOMPASSDK", "ssctSUBSCRBG", "ssctSUBSCRSHADOW",
    "ssctTRIFRAMECOLOR", "ssctBMAPBG", "ssctBMAPFG", "ssctHERODOT", "ssctMSGTEXT"
};

const char *sso_specialcset[sscsMAX]=
{
    "sscsTRIFORCECSET", "sscsTRIFRAMECSET", "sscsOVERWORLDMAPCSET", "sscsDUNGEONMAPCSET", "sscsBLUEFRAMECSET", "sscsHCPIECESCSET", "sscsSSVINECSET"
};

const char *sso_specialtile[ssmstMAX]=
{
    "ssmstSSVINETILE", "ssmstMAGICMETER"
};

const char *sso_alignment[3]=
{
    "sstaLEFT", "sstaCENTER", "sstaRIGHT"
};

int32_t onActivePassive();

static MENU ss_edit_menu[] =
{
	{ (char *)"&New\tIns",                       onNewSubscreenObject,                 NULL, 0, NULL },
	{ (char *)"&Delete\tDel",                    onDeleteWidgetC,                      NULL, 0, NULL },
	{ (char *)"",                                NULL,                                 NULL, 0, NULL },
	{ (char *)"&Copy/Paste",                     NULL,                    ss_copypaste_menu, 0, NULL },
	{ (char *)"",                                NULL,                                 NULL, 0, NULL },
	//5
	{ (char *)"&Properties\tE",                  onWidgetProperties,                   NULL, 0, NULL },
	{ (char *)"",                                NULL,                                 NULL, 0, NULL },
	{ (char *)"&Arrange",                        NULL,                      ss_arrange_menu, 0, NULL },
	{ (char *)"Al&ign",                          NULL,                        ss_align_menu, 0, NULL },
	{ (char *)"Dis&tribute",                     NULL,                   ss_distribute_menu, 0, NULL },
	//10
	{ (char *)"",                                NULL,                                 NULL, 0, NULL },
	{ (char *)"Switch Active/Passive",           onActivePassive,                      NULL, 0, NULL },
	{ (char *)"",                                NULL,                                 NULL, 0, NULL },
	{ (char *)"&Take Snapshot\tZ",               onSnapshot,                           NULL, 0, NULL },
	{ NULL,                                      NULL,                                 NULL, 0, NULL }
};

int32_t onSubscrViewInfo()
{
	InfoDialog("Subscreen Editor View Menu",
		"\"Edit/Show Grid\" affects the background/placement grid."
		"\n\"Show Invisible Items\" makes items with the 'Invisible' flag checked show up."
		"\n\"Show Unowned Items\" views the subscreen with every item marked 'owned'."
			" This does NOT cause counters to show as infinite, even if it normally would."
		"\n\"Max Out Counters\" views the subscreen with every counter set to its' max."
		"\n\"Max Out Max Counters\" additionally sets every max counter to 65535."
		"\n\"Don't Show 'Infinite's\" makes all counter-related 'Infinite' checks show false."
		"\n\"Show Everything 'Infinite'\" makes all counter-related 'Infinite' checks show true.").show();
	return D_O_K;
}
static MENU ss_view_menu[] =
{
	{ (char *)"&Edit Grid",                      onEditGrid,                    NULL, 0, NULL },
	{ (char *)"&Show Grid",                      onShowHideGrid,                NULL, 0, NULL },
	{ (char *)"",                                NULL,                          NULL, 0, NULL },
	{ (char *)"Show In&visible Items",           onToggleInvis,                 NULL, 0, NULL },
	{ (char *)"Show Unowned Items",              onToggleShowUnowned,           NULL, 0, NULL },
	{ (char *)"",                                NULL,                          NULL, 0, NULL },
	{ (char *)"Max Out Counters",                onToggleMaxCtr,                NULL, 0, NULL },
	{ (char *)"Max Out Max Counters",            onToggleMaxMaxCtr,             NULL, 0, NULL },
	{ (char *)"Don't Show 'Infinite's",          onToggleNoInf,                 NULL, 0, NULL },
	{ (char *)"Show Everything 'Infinite'",      onToggleAllInf,                NULL, 0, NULL },
	{ (char *)"",                                NULL,                          NULL, 0, NULL },
	{ (char *)"&Help",                           onSubscrViewInfo,              NULL, 0, NULL },
	{ NULL,                                      NULL,                          NULL, 0, NULL }
};

static MENU ss_selection_menu[] =
{
    { (char *)"&Add to Selection\tA",       onAddToSelection,                 NULL, 0, NULL },
    { (char *)"&Remove from Selection\tR",  onRemoveFromSelection,            NULL, 0, NULL },
    { (char *)"&Invert Selection\tI",       onInvertSelection,                NULL, 0, NULL },
    { (char *)"&Clear Selection\tC",        onClearSelection,                 NULL, 0, NULL },
    { NULL,                                 NULL,                             NULL, 0, NULL }
};

int32_t set_ssmouse(int ty);
int32_t onSSMouseClassic()
{
	return set_ssmouse(0);
}
int32_t onSSMouseModern()
{
	return set_ssmouse(1);
}
int32_t onSSMouseInfo()
{
	InfoDialog("Subscreen Editor Mouse Settings",
		"In 'Classic' mode, the subscreen editor works as it always used to."
		"\nIn 'Modern' mode, it can click-and-drag objects around.").show();
	return D_O_K;
}
static MENU ss_mouseset_menu[] =
{
    { (char *)"&Classic",             onSSMouseClassic,        NULL, 0,          NULL },
    { (char *)"&Modern",              onSSMouseModern,         NULL, D_SELECTED, NULL },
    { (char *)"",                     NULL,                    NULL, 0,          NULL },
    { (char *)"&Help",                onSSMouseInfo,           NULL, 0,          NULL },
    { NULL,                           NULL,                    NULL, 0,          NULL }
};
int32_t set_ssmouse(int ty)
{
	ty = vbound(ty, 0, ssmouMAX-1);
	ssmouse_type = ty;
	for(int q = 0; q<ssmouMAX; ++q)
	{
		SETFLAG(ss_mouseset_menu[q].flags,D_SELECTED,q==ty);
	}
	return D_O_K;
}

void call_subscrsettings_dialog();
static int32_t onSubscreenSettings()
{
	call_subscrsettings_dialog();
	return D_O_K;
}
static MENU ss_settings_menu[] =
{
    { (char *)"&Subscreen Settings",        onSubscreenSettings,              NULL, 0, NULL },
    { (char *)"Se&lection Settings",        onSelectionOptions,               NULL, 0, NULL },
    { (char *)"&Mouse Settings",            NULL,                 ss_mouseset_menu, 0, NULL },
    { (char *)"Confirm Delete",             onToggleConfDelete,               NULL, 0, NULL },
    { NULL,                                 NULL,                             NULL, 0, NULL }
};

static MENU subscreen_menu[] =
{
    { (char *)"&Edit",               NULL,                                 ss_edit_menu, 0, NULL },
    { (char *)"&View",               NULL,                                 ss_view_menu, 0, NULL },
    { (char *)"&Selection",          NULL,                                 ss_selection_menu, 0, NULL },
    { (char *)"&Options",            NULL,                                 ss_settings_menu, 0, NULL },
    { NULL,                          NULL,                                 NULL, 0, NULL }
};

static std::string arrow_infos[5] =
{
	"L/R: Change selected object (-1/+1)"
		"\nU/D: Reorder selected object (-1/+1)",
	"Move selected object by 1 pixel",
	"Decrease/Increase selected object's width/height",
	"Move preview of item selector",
	"L/R: Change Page (-1/+1)"
		"\n-/+: Delete/Add Page"
		"\nLL/RR: Change Page (Home/End)"
		"\n<>: Swap Pages (left/right)"
};
static char pgbuf[16] = "Pg 1/1";
static DIALOG subscreen_dlg[] =
{
	// (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)                    (d2)     (dp)
	{ jwin_win_proc,        0,    0,      320,    240,  vc(0),              vc(11),           0,       D_EXIT,     0,                      0, (void *) "Subscreen Editor", NULL, NULL },
	{ jwin_button_proc,     192,  215,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,                      0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,     255,  215,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,                      0, (void *) "Cancel", NULL, NULL },
	{ jwin_frame_proc,      4,    37,     260,    172,  0,                  0,                0,       0,          FR_DEEP,                0, NULL, NULL, NULL },
	{ d_subscreen_proc,     6,    39,     256,    168,  0,                  0,                0,       0,          0,                      0, NULL, NULL, NULL },
	// 5
	{ d_dummy_proc,         11,   211,    181,    8,    0,                  0,                0,       0,          0,                      0, NULL, NULL, NULL },
	{ jwin_text_proc,       34,   211,    181,    16,   0,                  0,                0,       0,          0,                      0, NULL, NULL, NULL },
	{ jwin_text_proc,        4,   225,    30,     16,   0,                  0,                0,       0,          0,                      0, (void *) "Name:", NULL, NULL },
	{ jwin_edit_proc,       34,   221,    155,    16,   0,                  0,                0,       0,          64,                     0, NULL, NULL, NULL },
	// 9
	{ d_ssup_btn_proc,      284,  23,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_UP,       0, NULL, NULL, NULL },
	{ d_ssdn_btn_proc,      284,  53,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_DOWN,     0, NULL, NULL, NULL },
	{ d_sslt_btn_proc,      269,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_LEFT,     0, NULL, NULL, NULL },
	{ d_ssrt_btn_proc,      299,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_RIGHT,    0, NULL, NULL, NULL },
	// 13
	{ d_ssup_btn2_proc,     284,  70,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_UP,       0, NULL, NULL, NULL },
	{ d_ssdn_btn2_proc,     284,  100,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_DOWN,     0, NULL, NULL, NULL },
	{ d_sslt_btn2_proc,     269,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_LEFT,     0, NULL, NULL, NULL },
	{ d_ssrt_btn2_proc,     299,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_RIGHT,    0, NULL, NULL, NULL },
	// 17
	{ d_ssup_btn3_proc,     284,  117,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_CONTRACT_VERT,  0, NULL, NULL, NULL },
	{ d_ssdn_btn3_proc,     284,  147,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_EXPAND_VERT,    0, NULL, NULL, NULL },
	{ d_sslt_btn3_proc,     269,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_CONTRACT_HORZ,  0, NULL, NULL, NULL },
	{ d_ssrt_btn3_proc,     299,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_EXPAND_HORZ,    0, NULL, NULL, NULL },
	// 21
	{ d_ssup_btn4_proc,     284,  164,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_UP,       0, NULL, NULL, NULL },
	{ d_ssdn_btn4_proc,     284,  194,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_DOWN,     0, NULL, NULL, NULL },
	{ d_sslt_btn4_proc,     269,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_LEFT,     0, NULL, NULL, NULL },
	{ d_ssrt_btn4_proc,     299,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     BTNICON_ARROW_RIGHT,    0, NULL, NULL, NULL },
	// 25
	{ jwin_menu_proc,       4,    23,     0,      13,    0,                 0,                0,       0,          0,                      0, (void *) subscreen_menu, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_UP,                 0, (void *) onSSUp, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DOWN,               0, (void *) onSSDown, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_LEFT,               0, (void *) onSSLeft, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_RIGHT,              0, (void *) onSSRight, NULL, NULL },
	// 30
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGUP,               0, (void *) onSSPgUp, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGDN,               0, (void *) onSSPgDn, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'a',     0,          0,                      0, (void *) onAddToSelection, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'r',     0,          0,                      0, (void *) onRemoveFromSelection, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'i',     0,          0,                      0, (void *) onInvertSelection, NULL, NULL },
	// 35
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'l',     0,          0,                      0, (void *) onClearSelection, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_INSERT,             0, (void *) onNewSubscreenObject, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DEL,                0, (void *) onDeleteWidgetC, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'd',     0,          0,                      0, (void *) onDuplicateWidget, NULL, NULL },
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'e',     0,          0,                      0, (void *) onWidgetProperties, NULL, NULL },
	// 40
	{ d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'z',     0,          0,                      0, (void *) onSnapshot, NULL, NULL },
	{ jwin_infobtn_proc,    284,  38,     15,     15,    0,                 0,                0,       0,          0,                      0, (void*)&arrow_infos[0], NULL, NULL },
	{ jwin_infobtn_proc,    284,  85,     15,     15,    0,                 0,                0,       0,          0,                      0, (void*)&arrow_infos[1], NULL, NULL },
	{ jwin_infobtn_proc,    284,  132,    15,     15,    0,                 0,                0,       0,          0,                      0, (void*)&arrow_infos[2], NULL, NULL },
	{ jwin_infobtn_proc,    284,  179,    15,     15,    0,                 0,                0,       0,          0,                      0, (void*)&arrow_infos[3], NULL, NULL },
	// 45
	{ jwin_ctext_proc,      134,  0,      30,     16,    0,                 0,                0,       0,          0,                      0, pgbuf, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_ARROW_LEFT,     0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_ARROW_RIGHT,    0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_MINUS,          0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_PLUS,           0, NULL, NULL, NULL },
	// 50
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_ARROW_LEFT2,    0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_ARROW_RIGHT2,   0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_EXPAND_HORZ,    0, NULL, NULL, NULL },
	{ jwin_iconbutton_proc,  0,   0,      15,     15,    0,                 0,                0,       D_EXIT,     BTNICON_EXPAND_HORZ,    0, NULL, NULL, NULL },
	{ jwin_infobtn_proc,     0,   0,      15,     15,    0,                 0,                0,       0,          0,                      0, (void*)&arrow_infos[4], NULL, NULL },
	// 55
	{ jwin_text_proc,       0,    0,     181,     16,   0,                  0,                0,       0,          0,                      0, NULL, NULL, NULL },
	{ d_keyboard_proc,      0,    0,      0,       0,    0,                 0,                'v',     0,          0,                      0, (void *) onSubscrPaste, NULL, NULL },
	{ d_keyboard_proc,      0,    0,      0,       0,    0,                 0,                'c',     0,          0,                      0, (void *) onSubscrCopy, NULL, NULL },
	{ d_vsync_proc,         0,    0,      0,       0,    0,                 0,                0,       0,          0,                      0, NULL, NULL, NULL },
	{ d_timer_proc,         0,    0,      0,       0,    0,                 0,                0,       0,          0,                      0, NULL, NULL, NULL },
	{ NULL,                 0,    0,      0,       0,    0,                 0,                0,       0,          0,                      0, NULL, NULL, NULL }
};

int32_t onActivePassive()
{
	bool run = true;
	if(subscr_edit.sub_type == sstACTIVE && subscr_edit.pages.size()>1)
	{
		bool singular = subscr_edit.pages.size()==2;
		std::string pgs = singular ? "page 2" : fmt::format("pages 2-{}",subscr_edit.pages.size());
		AlertDialog(fmt::format("Delete Page{}",singular?"":"s"),
			fmt::format("Are you sure you want to delete {} by"
				" switching to a passive subscreen?", pgs),
			[&](bool ret,bool)
			{
				run = ret;
			}).show();
	}
	if(!run) return D_O_K;
	
    if(subscr_edit.sub_type == sstPASSIVE)
    {
        subscr_edit.sub_type = sstACTIVE;
        subscreen_dlg[3].h=172*2-4;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
    }
	else
    {
        subscr_edit.sub_type = sstPASSIVE;
        subscreen_dlg[3].h=116;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
		while(subscr_edit.pages.size() > 1)
			subscr_edit.delete_page(1);
    }
    update_subscr_dlg(false);
    return D_REDRAW;
}


const char *color_str[16] =
{
    "Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Brown", "Light Gray",
    "Dark Gray", "Light Blue", "Light Green", "Light Cyan", "Light Red", "Light Magenta", "Yellow", "White"
};

const char *colorlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return color_str[index];
}

static ListData color_list(colorlist, &font);

static DIALOG grid_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,      158,    120,  vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Edit Grid Properties", NULL, NULL },
    { jwin_button_proc,     18,    95,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     81,    95,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       6,     29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Size:", NULL, NULL },
    { jwin_edit_proc,       42,    25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    // 5
    { jwin_text_proc,       6,     49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Size:", NULL, NULL },
    { jwin_edit_proc,       42,    45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Offset:", NULL, NULL },
    { jwin_edit_proc,       126,   25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Offset:", NULL, NULL },
    // 10
    { jwin_edit_proc,       126,   45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       6,     69,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Color:", NULL, NULL },
    { jwin_droplist_proc,   36,    65,    116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG sel_options_dlg[] =
{
    // (dialog proc)       (x)    (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,     0,    225,   120,   vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Selection Options", NULL, NULL },
    { jwin_button_proc,     51,    95,    61,    21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,    114,    95,    61,    21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_frame_proc,       6,    28,   213,    51,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,     0,       NULL, NULL, NULL },
    { jwin_text_proc,       10,    25,    48,     8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) " Selection Outlines ", NULL, NULL },
    // 5
    { jwin_text_proc,       14,    41,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Primary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    37,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { jwin_text_proc,       14,    61,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Secondary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    57,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

SubscrWidget* create_new_widget_of(int32_t type, int x, int y)
{
	SubscrWidget* widg = SubscrWidget::newType(type);
	if(!widg) return nullptr;
	widg->posflags = sspUP | sspDOWN | sspSCROLLING;
	widg->x = x;
	widg->y = y;
	widg->w=1;
	widg->h=1;
	
	int32_t temp_cso=curr_widg;
	curr_widg=subscr_edit.cur_page().contents.size();
	
	SubscrPage& pg = subscr_edit.cur_page();
	if(sso_properties(widg,pg.contents.size())!=-1)
	{
		pg.contents.push_back(widg);
		update_sso_name();
		update_up_dn_btns();
		return widg;
	}
	else
	{
		curr_widg=temp_cso;
		delete widg;
		return nullptr;
	}
}

SubscrWidget* create_new_widget(int x, int y)
{
	SubscrWidgListerDialog().show(); //pick an index!
    
	return create_new_widget_of(lister_sel_val, x, y);
}

int32_t onNewSubscreenObject()
{
	create_new_widget();
    return D_O_K;
}


void align_objects(bool *selection, int32_t align_type)
{
	auto& pg = subscr_edit.cur_page();
	auto* curwidg = pg.get_widget(curr_widg);
	if(!curwidg)
	{
		curr_widg = -1;
		return;
	}
	int32_t l=curwidg->getX()+curwidg->getXOffs();
	int32_t t=curwidg->getY();
	int32_t w=curwidg->getW();
	int32_t h=curwidg->getH();
	
	int32_t r=l+w-1;
	int32_t b=t+h-1;
	int32_t c=l+w/2;
	int32_t m=t+h/2;
	
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(selection[i]&&i!=curr_widg)
		{
			SubscrWidget& widg = *pg.contents[i];
			int32_t tl=widg.getX()+widg.getXOffs();
			int32_t tt=widg.getY();
			int32_t tw=widg.getW();
			int32_t th=widg.getH();
			
			int32_t tr=tl+tw-1;
			int32_t tb=tt+th-1;
			int32_t tc=tl+tw/2;
			int32_t tm=tt+th/2;
			
			switch(align_type)
			{
			case ssoaBOTTOM:
				widg.y+=b-tb;
				break;
				
			case ssoaMIDDLE:
				widg.y+=m-tm;
				break;
				
			case ssoaTOP:
				widg.y+=t-tt;
				break;
				
			case ssoaRIGHT:
				widg.x+=r-tr;
				break;
				
			case ssoaCENTER:
				widg.x+=c-tc;
				break;
				
			case ssoaLEFT:
			default:
				widg.x+=l-tl;
				break;
			}
		}
	}
}

void grid_snap_objects(bool *selection, int32_t snap_type)
{
	auto& pg = subscr_edit.cur_page();
	for(int32_t i=0; i < pg.contents.size(); ++i)
	{
		if(selection[i]||i==curr_widg)
		{
			SubscrWidget& widg = *pg.contents[i];
			int32_t tl=widg.getX()+widg.getXOffs();
			int32_t tt=widg.getY();
			int32_t tw=widg.getW();
			int32_t th=widg.getH();
			
			int32_t tr=tl+tw-1;
			int32_t tb=tt+th-1;
			int32_t tc=tl+tw/2;
			int32_t tm=tt+th/2;
			int32_t l1=(tl-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t l2=l1+zinit.ss_grid_x;
			int32_t c1=(tc-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t c2=c1+zinit.ss_grid_x;
			int32_t r1=(tr-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t r2=r1+zinit.ss_grid_x;
			
			int32_t t1=(tt-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t t2=t1+zinit.ss_grid_y;
			int32_t m1=(tm-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t m2=m1+zinit.ss_grid_y;
			int32_t b1=(tb-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t b2=b1+zinit.ss_grid_y;
			
			switch(snap_type)
			{
			case ssosBOTTOM:
				widg.y+=(abs(b1-tb)>abs(b2-tb))?(b2-tb):(b1-tb);
				break;
				
			case ssosMIDDLE:
				widg.y+=(abs(m1-tm)>abs(m2-tm))?(m2-tm):(m1-tm);
				break;
				
			case ssosTOP:
				widg.y+=(abs(t1-tt)>abs(t2-tt))?(t2-tt):(t1-tt);
				break;
				
			case ssosRIGHT:
				widg.x+=(abs(r1-tr)>abs(r2-tr))?(r2-tr):(r1-tr);
				break;
				
			case ssosCENTER:
				widg.x+=(abs(c1-tc)>abs(c2-tc))?(c2-tc):(c1-tc);
				break;
				
			case ssosLEFT:
			default:
				widg.x+=(abs(l1-tl)>abs(l2-tl))?(l2-tl):(l1-tl);
				break;
			}
		}
	}
}

typedef struct dist_obj
{
    int32_t index;
    int32_t l;
    int32_t t;
    int32_t w;
    int32_t h;
    int32_t r;
    int32_t b;
    int32_t c;
    int32_t m;
} dist_obj;

void distribute_objects(bool *, int32_t distribute_type)
{
	int32_t count=0;
	dist_obj temp_do[MAXSUBSCREENITEMS];
	
	auto& pg = subscr_edit.cur_page();
	for(int32_t i=0; i < pg.contents.size(); ++i)
	{
		if(sso_selection[i]==true||i==curr_widg)
		{
			SubscrWidget& widg = *pg.contents[i];
			temp_do[count].index=i;
			temp_do[count].l=widg.getX()+widg.getXOffs();
			temp_do[count].t=widg.getY();
			temp_do[count].w=widg.getW();
			temp_do[count].h=widg.getH();
			
			temp_do[count].r=temp_do[count].l+temp_do[count].w-1;
			temp_do[count].b=temp_do[count].t+temp_do[count].h-1;
			temp_do[count].c=temp_do[count].l+temp_do[count].w/2;
			temp_do[count].m=temp_do[count].t+temp_do[count].h/2;
			++count;
		}
	}
	
	if(count<3)
	{
		return;
	}
	
	//sort all objects in order of position, then index (yeah, bubble sort; sue me)
	dist_obj tempdo2;
	
	for(int32_t j=0; j<count-1; j++)
	{
		for(int32_t k=0; k<count-1-j; k++)
		{
			switch(distribute_type)
			{
			case ssodBOTTOM:
				if(temp_do[k+1].b<temp_do[k].b||((temp_do[k+1].b==temp_do[k].b)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodMIDDLE:
				if(temp_do[k+1].m<temp_do[k].m||((temp_do[k+1].m==temp_do[k].m)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodTOP:
				if(temp_do[k+1].t<temp_do[k].t||((temp_do[k+1].t==temp_do[k].t)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodRIGHT:
				if(temp_do[k+1].r<temp_do[k].r||((temp_do[k+1].r==temp_do[k].r)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodCENTER:
				if(temp_do[k+1].c<temp_do[k].c||((temp_do[k+1].c==temp_do[k].c)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodLEFT:
			default:
				if(temp_do[k+1].l<temp_do[k].l||((temp_do[k+1].l==temp_do[k].l)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
			}
		}
	}
	
	int32_t ld=temp_do[count-1].l-temp_do[0].l;
	int32_t td=temp_do[count-1].t-temp_do[0].t;
	int32_t rd=temp_do[count-1].r-temp_do[0].r;
	int32_t bd=temp_do[count-1].b-temp_do[0].b;
	int32_t cd=temp_do[count-1].c-temp_do[0].c;
	int32_t md=temp_do[count-1].m-temp_do[0].m;
	
	for(int32_t i=1; i<count-1; ++i)
	{
		if(unsigned(temp_do[i].index) >= pg.contents.size())
			continue;
		SubscrWidget& widg = *pg.contents[temp_do[i].index];
		switch(distribute_type)
		{
		case ssodBOTTOM:
			widg.y+=bd*i/(count-1)-temp_do[i].b+temp_do[0].b;
			break;
			
		case ssodMIDDLE:
			widg.y+=md*i/(count-1)-temp_do[i].m+temp_do[0].m;
			break;
			
		case ssodTOP:
			widg.y+=td*i/(count-1)-temp_do[i].t+temp_do[0].t;
			break;
			
		case ssodRIGHT:
			widg.x+=rd*i/(count-1)-temp_do[i].r+temp_do[0].r;
			break;
			
		case ssodCENTER:
			widg.x+=cd*i/(count-1)-temp_do[i].c+temp_do[0].c;
			break;
			
		case ssodLEFT:
		default:
			widg.x+=ld*i/(count-1)-temp_do[i].l+temp_do[0].l;
			break;
		}
	}
}


int32_t onReverseArrangement()
{
	auto& pg = subscr_edit.cur_page();
    int32_t i=0;
    int32_t j=pg.contents.size()-1;
    subscreen_object tempsso;
    
    sso_selection[curr_widg]=true;
    
    while(true)
    {
        while(i<pg.contents.size() && !sso_selection[i])
            i++;
            
        while(j>=0 && !sso_selection[j])
            j--;
            
        if(i>=j)
        {
            sso_selection[curr_widg]=false;
            return D_O_K;
        }
        
        if(curr_widg==i)
            curr_widg=j;
        else if(curr_widg==j)
            curr_widg=i;
		
		zc_swap(pg.contents[i],pg.contents[j]);
        
        i++;
        j--;
    }
}

int32_t onAlignLeft()
{
    align_objects(sso_selection, ssoaLEFT);
    return D_O_K;
}

int32_t onAlignCenter()
{
    align_objects(sso_selection, ssoaCENTER);
    return D_O_K;
}

int32_t onAlignRight()
{
    align_objects(sso_selection, ssoaRIGHT);
    return D_O_K;
}

int32_t onAlignTop()
{
    align_objects(sso_selection, ssoaTOP);
    return D_O_K;
}

int32_t onAlignMiddle()
{
    align_objects(sso_selection, ssoaMIDDLE);
    return D_O_K;
}

int32_t onAlignBottom()
{
    align_objects(sso_selection, ssoaBOTTOM);
    return D_O_K;
}

int32_t onDistributeLeft()
{
    distribute_objects(sso_selection, ssodLEFT);
    return D_O_K;
}

int32_t onDistributeCenter()
{
    distribute_objects(sso_selection, ssodCENTER);
    return D_O_K;
}

int32_t onDistributeRight()
{
    distribute_objects(sso_selection, ssodRIGHT);
    return D_O_K;
}

int32_t onDistributeTop()
{
    distribute_objects(sso_selection, ssodTOP);
    return D_O_K;
}

int32_t onDistributeMiddle()
{
    distribute_objects(sso_selection, ssodMIDDLE);
    return D_O_K;
}

int32_t onDistributeBottom()
{
    distribute_objects(sso_selection, ssodBOTTOM);
    return D_O_K;
}

int32_t onGridSnapLeft()
{
    grid_snap_objects(sso_selection, ssosLEFT);
    return D_O_K;
}

int32_t onGridSnapCenter()
{
    grid_snap_objects(sso_selection, ssosCENTER);
    return D_O_K;
}

int32_t onGridSnapRight()
{
    grid_snap_objects(sso_selection, ssosRIGHT);
    return D_O_K;
}

int32_t onGridSnapTop()
{
    grid_snap_objects(sso_selection, ssosTOP);
    return D_O_K;
}

int32_t onGridSnapMiddle()
{
    grid_snap_objects(sso_selection, ssosMIDDLE);
    return D_O_K;
}

int32_t onGridSnapBottom()
{
    grid_snap_objects(sso_selection, ssosBOTTOM);
    return D_O_K;
}

static int32_t onToggleInvis()
{
	bool show=!(zinit.ss_flags&ssflagSHOWINVIS);
	SETFLAG(zinit.ss_flags,ssflagSHOWINVIS,show);
	SETFLAG(ss_view_menu[3].flags,D_SELECTED,show);
	return D_O_K;
}
static int32_t onToggleShowUnowned()
{
	zq_ignore_item_ownership = !zq_ignore_item_ownership;
	SETFLAG(ss_view_menu[4].flags, D_SELECTED, zq_ignore_item_ownership);
	zc_set_config("editsubscr","show_all_items",zq_ignore_item_ownership?1:0);
    return D_O_K;
}
static int32_t onToggleMaxCtr()
{
	zq_view_fullctr = !zq_view_fullctr;
	SETFLAG(ss_view_menu[6].flags, D_SELECTED, zq_view_fullctr);
	zc_set_config("editsubscr","show_full_counters",zq_view_fullctr?1:0);
    return D_O_K;
}
static int32_t onToggleMaxMaxCtr()
{
	zq_view_maxctr = !zq_view_maxctr;
	SETFLAG(ss_view_menu[7].flags, D_SELECTED, zq_view_maxctr);
	zc_set_config("editsubscr","show_maxed_maxcounters",zq_view_maxctr?1:0);
    return D_O_K;
}
static int32_t onToggleNoInf()
{
	if(zq_view_allinf)
		onToggleAllInf();
	zq_view_noinf = !zq_view_noinf;
	SETFLAG(ss_view_menu[8].flags, D_SELECTED, zq_view_noinf);
	zc_set_config("editsubscr","show_no_infinites",zq_view_noinf?1:0);
    return D_O_K;
}
static int32_t onToggleAllInf()
{
	if(zq_view_noinf)
		onToggleNoInf();
	zq_view_allinf = !zq_view_allinf;
	SETFLAG(ss_view_menu[9].flags, D_SELECTED, zq_view_allinf);
	zc_set_config("editsubscr","show_all_infinites",zq_view_allinf?1:0);
    return D_O_K;
}
static int32_t onToggleConfDelete()
{
	subscr_confirm_delete = !subscr_confirm_delete;
	SETFLAG(ss_settings_menu[3].flags, D_SELECTED, subscr_confirm_delete);
	zc_set_config("editsubscr","confirm_delete",subscr_confirm_delete?1:0);
    return D_O_K;
}

static int32_t onEditGrid()
{
    grid_dlg[0].dp2=get_zc_font(font_lfont);
    char xsize[11];
    char ysize[11];
    char xoffset[4];
    char yoffset[4];
    sprintf(xsize, "%d", zc_max(zinit.ss_grid_x,1));
    sprintf(ysize, "%d", zc_max(zinit.ss_grid_y,1));
    sprintf(xoffset, "%d", zinit.ss_grid_xofs);
    sprintf(yoffset, "%d", zinit.ss_grid_yofs);
    grid_dlg[4].dp=xsize;
    grid_dlg[6].dp=ysize;
    grid_dlg[8].dp=xoffset;
    grid_dlg[10].dp=yoffset;
    grid_dlg[12].d1=zinit.ss_grid_color;
    
    large_dialog(grid_dlg);
        
    int32_t ret = zc_popup_dialog(grid_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_grid_x=zc_max(atoi(xsize),1);
        zinit.ss_grid_xofs=atoi(xoffset);
        zinit.ss_grid_y=zc_max(atoi(ysize),1);
        zinit.ss_grid_yofs=atoi(yoffset);
        zinit.ss_grid_color=grid_dlg[12].d1;
    }
    
    return D_O_K;
}

static int32_t onShowHideGrid()
{
    bool show=!(zinit.ss_flags&ssflagSHOWGRID);
    zinit.ss_flags&=~ssflagSHOWGRID;
    zinit.ss_flags|=(show?ssflagSHOWGRID:0);
    ss_view_menu[1].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
    return D_O_K;
}

int32_t onSelectionOptions()
{
    sel_options_dlg[0].dp2=get_zc_font(font_lfont);
    sel_options_dlg[6].d1=zinit.ss_bbox_1_color;
    sel_options_dlg[8].d1=zinit.ss_bbox_2_color;
    
    large_dialog(sel_options_dlg);
        
    int32_t ret = zc_popup_dialog(sel_options_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_bbox_1_color=sel_options_dlg[6].d1;
        zinit.ss_bbox_2_color=sel_options_dlg[8].d1;
    }
    
    return D_O_K;
}


void update_up_dn_btns()
{
    if(curr_widg<1)
    {
        subscreen_dlg[10].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[10].flags&=~D_DISABLED;
    }
    
    if(curr_widg>=subscr_edit.cur_page().contents.size()-1)
    {
        subscreen_dlg[9].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[9].flags&=~D_DISABLED;
    }
    
    subscreen_dlg[9].flags|=D_DIRTY;
    subscreen_dlg[10].flags|=D_DIRTY;
}

int32_t onSSCtrlPgUp()
{
    return onBringForward();
}

int32_t onSSCtrlPgDn()
{
    return onSendBackward();
}

static bool send_backwd()
{
	auto& pg = subscr_edit.cur_page();
	if(curr_widg > 0 && curr_widg < pg.contents.size())
	{
		pg.swap_widg(curr_widg,curr_widg-1);
		zc_swap(sso_selection[curr_widg],sso_selection[curr_widg-1]);
		--curr_widg;
		return true;
	}
	return false;
}
static bool send_fwd()
{
	auto& pg = subscr_edit.cur_page();
	if(curr_widg<subscr_edit.cur_page().contents.size()-1)
	{
		pg.swap_widg(curr_widg,curr_widg+1);
		zc_swap(sso_selection[curr_widg],sso_selection[curr_widg+1]);
		++curr_widg;
		return true;
	}
	return false;
}

int32_t onSendBackward()
{
	if(send_backwd())
		update_sso_name();
	update_up_dn_btns();
	return D_O_K;
}

int32_t onBringForward()
{
	if(send_fwd())
		update_sso_name();
	update_up_dn_btns();
	return D_O_K;
}

int32_t onBringToFront()
{
	if(send_fwd())
	{
		while(send_fwd());
		update_sso_name();
	}
	update_up_dn_btns();
    return D_O_K;
}

int32_t onSendToBack()
{
	if(send_backwd())
	{
		while(send_backwd());
		update_sso_name();
	}
	update_up_dn_btns();
    return D_O_K;
}
int32_t onSSPgDn()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgDn();
    }
    else
    {
        --curr_widg;
        
        if(curr_widg<0)
        {
            curr_widg=subscr_edit.cur_page().contents.size()-1;
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

int32_t onSSPgUp()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgUp();
    }
    else
    {
        if(!subscr_edit.cur_page().contents.empty())
        {
            ++curr_widg;
            
            if(curr_widg>=subscr_edit.cur_page().contents.size())
            {
                curr_widg=0;
            }
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_iconbutton_proc(msg, d, c);
        return onSSCtrlPgUp();
    }
    break;
    }
    
    return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_iconbutton_proc(msg, d, c);
        return onSSCtrlPgDn();
    }
    break;
    }
    
    return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_iconbutton_proc(msg, d, c);
        onSSPgDn();
        return D_O_K;
    }
    break;
    }
    
    return jwin_iconbutton_proc(msg, d, c);
}

int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_iconbutton_proc(msg, d, c);
        onSSPgUp();
        return D_O_K;
    }
    break;
    }
    
    return jwin_iconbutton_proc(msg, d, c);
}

int dlg_fontheight(DIALOG const& d)
{
	if(d.dp2)
		return text_height((FONT*)d.dp2);
	return text_height(font);
}
int dlg_fontlen(DIALOG const& d,char const* ptr = nullptr)
{
	if(!ptr) ptr = (char const*)d.dp;
	if(d.dp2)
		return text_length((FONT*)d.dp2,ptr);
	return text_length(font,ptr);
}
void update_subscr_dlg(bool start)
{
	if(start)
	{	
		subscreen_dlg[0].dp2=get_zc_font(font_lfont);
		refresh_subscr_items();
		curr_widg=0;
		
		if(subscr_edit.pages.empty())
			subscr_edit.pages.emplace_back();
		if(subscr_edit.pages.size() >= subscr_edit.curpage)
			subscr_edit.curpage = 0;
		if(subscr_edit.pages[subscr_edit.curpage].contents.empty())
		{
			curr_widg=-1;
		}
		
		onClearSelection();
		ss_view_menu[3].flags=zinit.ss_flags&ssflagSHOWINVIS?D_SELECTED:0;
		ss_view_menu[1].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
			
		subscreen_dlg[4].dp=(void *)&subscr_edit;
		subscreen_dlg[5].fg=jwin_pal[jcBOX];
		subscreen_dlg[5].bg=jwin_pal[jcBOX];
		strncpy(subscr_namebuf,subscr_edit.name.c_str(),63);
		subscreen_dlg[6].dp=(void *)str_oname;
		subscreen_dlg[55].dp=(void *)str_cpyname;
		subscreen_dlg[8].dp=(void *)subscr_namebuf;
		if(propCopyWidg)
		{
			delete propCopyWidg;
			propCopyWidg = nullptr;
			copied_page = -1;
		}
		update_sso_name();
		subscreen_dlg[10].flags|=D_DISABLED;
		
		if(subscr_edit.sub_type==sstPASSIVE)
		{
			subscreen_dlg[21].flags|=D_DISABLED;
			subscreen_dlg[22].flags|=D_DISABLED;
			subscreen_dlg[23].flags|=D_DISABLED;
			subscreen_dlg[24].flags|=D_DISABLED;
		}
		else
		{
			subscreen_dlg[21].flags&=~D_DISABLED;
			subscreen_dlg[22].flags&=~D_DISABLED;
			subscreen_dlg[23].flags&=~D_DISABLED;
			subscreen_dlg[24].flags&=~D_DISABLED;
		}
		
		subscr_edit.cur_page().move_legacy(SEL_VERIFY_RIGHT);
		
		bool enlarge = subscreen_dlg[0].d1==0;
		
		if(enlarge)
		{
			large_dialog(subscreen_dlg,2);
			subscreen_dlg[4].y-=32;
			subscreen_dlg[3].y-=31;
			subscreen_dlg[3].x+=1;
			
			if(subscr_edit.sub_type == sstPASSIVE)
				subscreen_dlg[3].h=60*2-4;
			else if(subscr_edit.sub_type == sstACTIVE)
				subscreen_dlg[3].h=172*2-4;
				
			subscreen_dlg[4].h=subscreen_dlg[3].h-4;
		}
		
		//Some fancier stuff for the subscreen update
		{
			for(int q = 0; subscreen_dlg[q].proc; ++q)
			{
				if(subscreen_dlg[q].proc == jwin_text_proc
					|| subscreen_dlg[q].proc == jwin_rtext_proc
					|| subscreen_dlg[q].proc == jwin_ctext_proc)
				{
					subscreen_dlg[q].dp2 = (void*)get_custom_font(CFONT_GUI);
				}
			}
			subscreen_dlg[0].h = 474 + 2 + dlg_fontheight(subscreen_dlg[6]) + dlg_fontheight(subscreen_dlg[55]);
			subscreen_dlg[8].y = subscreen_dlg[0].y + subscreen_dlg[0].h - 6 - subscreen_dlg[8].h;
			subscreen_dlg[8].x = subscreen_dlg[7].x + dlg_fontlen(subscreen_dlg[7]) + 2;
			subscreen_dlg[7].y = subscreen_dlg[8].y + (subscreen_dlg[8].h/2)
				- dlg_fontheight(subscreen_dlg[7])/2;
			subscreen_dlg[6].y = subscreen_dlg[8].y - 2 - dlg_fontheight(subscreen_dlg[6]);
			subscreen_dlg[55].y = subscreen_dlg[6].y-dlg_fontheight(subscreen_dlg[55]);
			{ //complex maths- line up strings by their first ':'
				subscreen_dlg[55].x = subscreen_dlg[6].x = subscreen_dlg[8].x;
				DIALOG* dlgs[] = {&subscreen_dlg[6],&subscreen_dlg[55]};
				char tmpstr[512] = {0};
				int maxlen=0;
				int lens[2];
				for(int ind = 0; ind < 2; ++ind)
				{
					char const* str = (char const*)dlgs[ind]->dp;
					for(int q = 0; str[q]; ++q)
					{
						tmpstr[q]=str[q];
						if(tmpstr[q] == ':')
						{
							tmpstr[q+1] = 0;
							break;
						}
					}
					int len = text_length((FONT*)dlgs[ind]->dp2,tmpstr);
					lens[ind] = len;
					if(len > maxlen)
						maxlen = len;
				}
				for(int ind = 0; ind < 2; ++ind)
				{
					dlgs[ind]->x += maxlen-lens[ind];
				}
			}
			subscreen_dlg[49].y = subscreen_dlg[48].y = subscreen_dlg[47].y =
				subscreen_dlg[46].y = subscreen_dlg[45].y =
				subscreen_dlg[3].y+168*2+2;
			subscreen_dlg[45].y += (subscreen_dlg[46].h-dlg_fontheight(subscreen_dlg[45]))/2;
			
			subscreen_dlg[50].y = subscreen_dlg[51].y = subscreen_dlg[52].y =
				subscreen_dlg[53].y = subscreen_dlg[54].y =
				subscreen_dlg[49].y+subscreen_dlg[49].h;
			
			subscreen_dlg[1].y = subscreen_dlg[2].y =
				subscreen_dlg[8].y+subscreen_dlg[8].h-subscreen_dlg[1].h;
			subscreen_dlg[2].x = subscreen_dlg[12].x+subscreen_dlg[12].w-subscreen_dlg[2].w;
			subscreen_dlg[1].x = subscreen_dlg[2].x-subscreen_dlg[1].w;
			subscreen_dlg[8].w = subscreen_dlg[1].x-subscreen_dlg[8].x-2;
		}
	}
	bool nopages = subscr_edit.sub_type==sstPASSIVE;
	for(int q = 45; q <= 54; ++q)
		SETFLAG(subscreen_dlg[q].flags,D_HIDDEN,nopages);
	if(!nopages)
	{
		sprintf(pgbuf, "Pg %d/%zd", subscr_edit.curpage+1,subscr_edit.pages.size());
		SETFLAG(subscreen_dlg[46].flags,D_DISABLED,subscr_edit.curpage<1);
		SETFLAG(subscreen_dlg[47].flags,D_DISABLED,subscr_edit.curpage>=subscr_edit.pages.size()-1);
		SETFLAG(subscreen_dlg[49].flags,D_DISABLED,subscr_edit.pages.size()>=MAX_SUBSCR_PAGES);
		SETFLAG(subscreen_dlg[50].flags,D_DISABLED,subscr_edit.curpage<1);
		SETFLAG(subscreen_dlg[52].flags,D_DISABLED,subscr_edit.curpage<1);
		SETFLAG(subscreen_dlg[51].flags,D_DISABLED,subscr_edit.curpage>=subscr_edit.pages.size()-1);
		SETFLAG(subscreen_dlg[53].flags,D_DISABLED,subscr_edit.curpage>=subscr_edit.pages.size()-1);
		int tw = 8+dlg_fontlen(subscreen_dlg[45]);
		subscreen_dlg[50].x = subscreen_dlg[46].x = subscreen_dlg[45].x-(tw/2)-subscreen_dlg[46].w;
		subscreen_dlg[51].x = subscreen_dlg[47].x = subscreen_dlg[45].x+(tw/2);
		subscreen_dlg[52].x = subscreen_dlg[48].x = subscreen_dlg[46].x - subscreen_dlg[48].w;
		subscreen_dlg[53].x = subscreen_dlg[49].x = subscreen_dlg[47].x + subscreen_dlg[47].w;
		subscreen_dlg[54].x = subscreen_dlg[45].x - subscreen_dlg[54].w/2;
	}
	// Disable pastes if no copies
	bool nocopies = (!propCopyWidg || propCopyWidg->getType()==widgNULL);
	SETFLAG(ss_copypaste_menu[2].flags,D_DISABLED,nocopies);
	SETFLAG(ss_copypaste_menu[3].flags,D_DISABLED,nocopies);
	SETFLAG(ss_copypaste_menu[5].flags,D_DISABLED,nocopies);
	SETFLAG(subscreen_rc_menu_nowidg[1].flags,D_DISABLED,nocopies);
}
void broadcast_dialog_message(DIALOG* dialog, int32_t msg, int32_t c);
bool edit_subscreen()
{
	bool b = zq_ignore_item_ownership;
	zq_ignore_item_ownership = zc_get_config("editsubscr","show_all_items_edit_subscr",1);
	zq_view_fullctr = zc_get_config("editsubscr","show_full_counters",0);
	zq_view_maxctr = zc_get_config("editsubscr","show_maxed_maxcounters",0);
	zq_view_noinf = zc_get_config("editsubscr","show_no_infinites",0);
	zq_view_allinf = zc_get_config("editsubscr","show_all_infinites",0);
	subscr_confirm_delete = zc_get_config("editsubscr","confirm_delete",0);
	game = new gamedata();
	game->set_time(0);
	resetItems(game,&zinit,true);
	
	SETFLAG(ss_view_menu[4].flags, D_SELECTED, zq_ignore_item_ownership);
	SETFLAG(ss_view_menu[6].flags, D_SELECTED, zq_view_fullctr);
	SETFLAG(ss_view_menu[7].flags, D_SELECTED, zq_view_maxctr);
	SETFLAG(ss_view_menu[8].flags, D_SELECTED, zq_view_noinf);
	SETFLAG(ss_view_menu[9].flags, D_SELECTED, zq_view_allinf);
	SETFLAG(ss_settings_menu[3].flags, D_SELECTED, subscr_confirm_delete);
	
	update_subscr_dlg(true);
	int dlg_ret = do_zqdialog_custom(subscreen_dlg,2,[&](int ret)
		{
			switch(ret)
			{
				case 46: // Page Left
					if(subscr_edit.curpage > 0)
						--subscr_edit.curpage;
					break;
				case 50: // Page Start
					subscr_edit.curpage = 0;
					break;
				case 47: // Page Right
					if(subscr_edit.curpage < subscr_edit.pages.size()-1)
						++subscr_edit.curpage;
					break;
				case 51: // Page End
					subscr_edit.curpage = subscr_edit.pages.size()-1;
					break;
				case 52: // Swap Left
					subscr_edit.swap_pages(subscr_edit.curpage,subscr_edit.curpage-1);
					break;
				case 53: // Swap Right
					subscr_edit.swap_pages(subscr_edit.curpage,subscr_edit.curpage+1);
					break;
				case 48: // Del Page
				{
					auto count = subscr_edit.cur_page().contents.size();
					bool del = !count;
					if(!del)
					{
						AlertDialog("Delete Current Page",
							fmt::format("Are you sure you want to delete the current page {},"
								" including {}?", subscr_edit.curpage,
								count==1?"its 1 widget":fmt::format("all its {} widgets",count)),
							[&](bool ret,bool)
							{
								del = ret;
							}).show();
					}
					if(del)
						subscr_edit.delete_page(subscr_edit.curpage);
					break;
				}
				case 49: // New Page
					subscr_edit.add_page(subscr_edit.curpage+1);
					break;
			}
			if(ret < 3)
				return true;
			update_subscr_dlg(false);
			return false;
		});
	
	if(dlg_ret==1)
	{
		subscr_edit.name = subscr_namebuf;
		saved=false;
		zinit.subscreen=ssdtMAX;
	}
	
	delete game;
	game=NULL;
	zq_ignore_item_ownership = b;
	return dlg_ret == 1;
}

const char *allsubscrtype_str[30] =
{
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *activesubscrtype_str[16] =
{
    "Blank",
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)"
};

const char *activelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return activesubscrtype_str[index];
}

const char *passivesubscrtype_str[16] =
{
    "Blank",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *passivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return passivesubscrtype_str[index];
}

const char *activepassive_str[sstMAX] =
{
    "Active", "Passive"
};

const char *activepassivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sstMAX;
        return NULL;
    }
    
    return activepassive_str[index];
}

static ListData passive_list(passivelist, &font);
static ListData active_list(activelist, &font);

int32_t sstype_drop_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t tempd1=d->d1;
    int32_t ret=jwin_droplist_proc(msg,d,c);
    
    if(tempd1!=d->d1)
    {
        (d+1)->dp=(d->d1)?(void*)&passive_list:(void*)&active_list;
        object_message(d+1,MSG_START,0);
        (d+1)->flags|=D_DIRTY;
    }
    
    return ret;
}

static ListData activepassive_list(activepassivelist, &font);

static DIALOG sstemplatelist_dlg[] =
{
    // (dialog proc)         (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,          0,    0,   265,  87,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "New Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_text_proc,         4,   28,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Type:", NULL, NULL },
    { jwin_text_proc,         4,   46,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Template:", NULL, NULL },
    { sstype_drop_proc,      33,   24,    72,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &activepassive_list, NULL, NULL },
    { jwin_droplist_proc,    50,   42,   211,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &active_list, NULL, NULL },
    { jwin_button_proc,      61,   62,    61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     142,   62,    61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool show_new_ss=true;

const char *subscreenlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = new_subscreen.size()+(show_new_ss?1:0);
        return NULL;
    }
    if(show_new_ss && index == new_subscreen.size())
		return "<New>";
	if(unsigned(index) < new_subscreen.size())
		return new_subscreen[index].name.c_str();
	return "";
}

const char *subscreenlist_either(int32_t index, int32_t *list_size, byte type)
{
    if(index<0)
    {
        int32_t i=0, j=0;
        
		while(j < new_subscreen.size())
		{
			if(new_subscreen[j].sub_type==type)
				++i;
			
			++j;
		}
        
        *list_size = i;
        return NULL;
    }
    
    int32_t i=-1, j=0;
    
	while(j < new_subscreen.size() && i!=index)
	{
		if(new_subscreen[j].sub_type==type)
			++i;
		
		++j;
	}
    
    return new_subscreen[j-1].name.c_str();
}

const char *subscreenlist_a(int32_t index, int32_t *list_size)
{
    return subscreenlist_either(index,list_size,sstACTIVE);
}

const char *subscreenlist_b(int32_t index, int32_t *list_size)
{
    return subscreenlist_either(index,list_size,sstPASSIVE);
}

static ListData subscreen_list(subscreenlist, &font);

DIALOG sslist_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     0,   0,   234,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_abclist_proc,    12,   24,   211,  95,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0, (void *) &subscreen_list, NULL, NULL },
    { jwin_button_proc,     12,   123,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Edit", NULL, NULL },
    { jwin_button_proc,     85,  123,  61,   21,   vc(14),  vc(1),  KEY_DEL,     D_EXIT,     0,             0, (void *) "Delete", NULL, NULL },
    { jwin_button_proc,     158,  123,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onEditSubscreens()
{
    int32_t ret=-1;
    sslist_dlg[0].dp2=get_zc_font(font_lfont);
    sstemplatelist_dlg[0].dp2=get_zc_font(font_lfont);
    
    large_dialog(sslist_dlg);
        
    while(ret!=0&&ret!=5)
    {
        ret=zc_popup_dialog(sslist_dlg,2);
        
		auto ind = sslist_dlg[2].d1;
        if(ret==4)
        {
            int32_t confirm = jwin_alert("Confirm Delete", "You are about to delete the selected subscreen!", "Are you sure?", NULL, "OK", "Cancel", KEY_ENTER, KEY_ESC, get_zc_font(font_lfont));
            
            if(confirm==1)
            {
                delete_subscreen(ind);
                saved=false;
            }
        }
        else if(ret==2 || ret ==3)
        {
			if(ind < new_subscreen.size())
				subscr_edit = new_subscreen[ind];
			else subscr_edit.clear();
            if(subscr_edit.sub_type==sstPASSIVE)
            {
                subscreen_dlg[3].h=116;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
			else
            {
				subscr_edit.sub_type = sstACTIVE;
                subscreen_dlg[3].h=340;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
            
            bool edit_it=true;
            
            if(subscr_edit.pages.empty())
            {
                large_dialog(sstemplatelist_dlg);
                    
                ret=zc_popup_dialog(sstemplatelist_dlg,4);
                
                if(ret==6)
                {
                    if(sstemplatelist_dlg[5].d1<15)
                    {
                        if(sstemplatelist_dlg[5].d1 != 0)
                        {
                            subscreen_object *tempsub;
                            
                            if(sstemplatelist_dlg[4].d1==0)
                            {
                                tempsub = default_subscreen_active[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
                            else
                            {
                                tempsub = default_subscreen_passive[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
							subscr_edit.load_old(tempsub);
                        }
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            subscr_edit.sub_type=sstACTIVE;
                            subscr_edit.name = activesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=172*2;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                        else
                        {
                            subscr_edit.sub_type=sstPASSIVE;
                            subscr_edit.name = passivesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                    else //Z3
                    {
                        subscreen_object *tempsub;
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            tempsub = z3_active_a;
                        }
                        else
                        {
                            tempsub = z3_passive_a;
                        }
						subscr_edit.load_old(tempsub);
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            subscr_edit.sub_type=sstACTIVE;
                            subscr_edit.name = activesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=344;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                            
                        }
                        else
                        {
                            subscr_edit.sub_type=sstPASSIVE;
                            subscr_edit.name = passivesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                }
                else
                {
                    edit_it=false;
                }
            }
            if(subscr_edit.pages.empty())
				subscr_edit.pages.emplace_back();
            
            if(edit_it && edit_subscreen())
			{
				if(ind < new_subscreen.size())
					new_subscreen[ind] = subscr_edit;
				else new_subscreen.push_back(subscr_edit);
			}
        }
    }
    
    position_mouse_z(0);
    return D_O_K;
}

void update_sso_name()
{
	auto* w = subscr_edit.cur_page().get_widget(curr_widg);
	if(w)
		sprintf(str_oname, "%3d:  %s", curr_widg, w->getTypeName().c_str());
	else
	{
		curr_widg = -1;
		sprintf(str_oname, "No object selected");
	}
	if(propCopyWidg)
		sprintf(str_cpyname, "Copied:  '%s' from pg %d", propCopyWidg->getTypeName().c_str(), copied_page);
	else sprintf(str_cpyname, "Copied:  Nothing");
	update_subscr_dlg(false);
	broadcast_dialog_message(MSG_DRAW, 0);
}

void center_zq_subscreen_dialogs()
{
    jwin_center_dialog(grid_dlg);
    jwin_center_dialog(sel_options_dlg);
    jwin_center_dialog(sslist_dlg);
    jwin_center_dialog(sstemplatelist_dlg);
    jwin_center_dialog(subscreen_dlg);
}

void delete_subscreen(int32_t subscreenidx)
{
	if(subscreenidx >= new_subscreen.size())
		return;
	
	int32_t i = 0;
	int32_t active_ind = 0, passive_ind = 0;
	for(auto it = new_subscreen.begin(); it != new_subscreen.end(); ++it)
	{
		if(i++ == subscreenidx)
		{
			if(it->sub_type == sstACTIVE) passive_ind = -1;
			else active_ind = -1;
			i = -1;
			new_subscreen.erase(it);
			break;
		}
		if(it->sub_type == sstACTIVE) ++active_ind;
		else ++passive_ind;
	}
	if(i != -1) active_ind = passive_ind = -1; //no deletion
	
	//fix dmaps
	int32_t dmap_count=count_dmaps();
	
	for(int32_t i=0; i<dmap_count; i++)
	{
		//decrement
		if(active_ind > -1 && DMaps[i].active_subscreen > active_ind)
			DMaps[i].active_subscreen--;
			
		if(passive_ind > -1 && DMaps[i].passive_subscreen > passive_ind)
			DMaps[i].passive_subscreen--;
	}
}
