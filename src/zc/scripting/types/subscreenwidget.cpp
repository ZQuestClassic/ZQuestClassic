#include "zc/scripting/types/subscreenwidget.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

void bad_subwidg_type(bool func, byte type)
{
	auto tyname = type < widgMAX ? subwidg_internal_names[type].c_str() : "";
	scripting_log_error_with_context("Widget type {} '{}' does not have this {}!",
		type, tyname, func ? "function" : "value");
}

int32_t subscreenwidget_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SUBWIDGDISPITM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				ret = 10000*widg->getDisplayItem().id;
			}
			break;
		}
		case SUBWIDGEQPITM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				ret = 10000*widg->getItemVal().id;
			}
			break;
		}
		case SUBWIDGH:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->h;
			break;
		}
		case SUBWIDGINDEX:
		{
			if(checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto [_sub,_ty,_pgid,ind] = from_subref(GET_REF(subscreenwidgref));
				ret = 10000*ind;
			}
			break;
		}
		case SUBWIDGPAGE:
		{
			if(checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto [sub,ty,pgid,_ind] = from_subref(GET_REF(subscreenwidgref));
				ret = get_subref(sub,ty,pgid,0);
			}
			break;
		}
		case SUBWIDGPGMODE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->pg_mode;
			break;
		}
		case SUBWIDGPGTARG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->pg_targ;
			break;
		}
		case SUBWIDGPOS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->pos;
			break;
		}
		case SUBWIDGPRESSSCRIPT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->generic_scrconfig.script;
			break;
		}
		case SUBWIDGREQCOUNTER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_counter;
			break;
		}
		case SUBWIDGREQCOUNTERCOND:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_counter_cond_type;
			break;
		}
		case SUBWIDGREQCOUNTERVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_counter_val;
			break;
		}
		case SUBWIDGREQLITEMLEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_litem_level;
			break;
		}
		case SUBWIDGREQLITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_litems;
			break;
		}
		case SUBWIDGREQSCRIPTDISABLED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = widg->is_disabled ? 10000 : 0;
			break;
		}
		case SUBWIDGREQ_LEVEL_STATE_LEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_lstate_level;
			break;
		}
		case SUBWIDGREQ_SCRSTATE_MAP:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_scrstate_map;
			break;
		}
		case SUBWIDGREQ_SCRSTATE_SCREEN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000 * widg->req_scrstate_scr;
			break;
		}
		case SUBWIDGSELECTORDSTH:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->selector_override.h;
			break;
		}
		case SUBWIDGSELECTORDSTW:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->selector_override.w;
			break;
		}
		case SUBWIDGSELECTORDSTX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->selector_override.x;
			break;
		}
		case SUBWIDGSELECTORDSTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				ret = 10000*widg->selector_override.y;
			break;
		}
		case SUBWIDGTRANSPGSFX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
			{
				auto& trans = widg->pg_trans;
				ret = 10000*trans.tr_sfx;
			}
			break;
		}
		case SUBWIDGTRANSPGTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
			{
				auto& trans = widg->pg_trans;
				ret = 10000*trans.type;
			}
			break;
		}
		case SUBWIDGTYPE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->getType();
			break;
		}
		case SUBWIDGTY_ALIGN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->align;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->align;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->align;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->align;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->align;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->align;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->align;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->align;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ANIMVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->anim_val;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_BUTTON:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNITM:
						ret = 10000*((SW_ButtonItem*)widg)->btn;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->btn;
						break;
					case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_ItemCooldownGauge*)widg)->button_id;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->button_id;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_bg.get_int_color();
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->c_bg.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_bg.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_bg.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_bg.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_bg.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_bg.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_bg.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_bg.get_int_color();
						break;
					case widgBGCOLOR:
						ret = 10000*((SW_Clear*)widg)->c_bg.get_int_color();
						break;
					case widgCOUNTERPERCBAR:
						ret = 10000*((SW_CounterPercentBar*)widg)->c_bg.get_int_color();
						break;
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_room_bg.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_bg2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_bg2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPBLNK:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_cmp_blink.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPOFF:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_cmp_off.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_FILL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgRECT:
						ret = 10000*((SW_Rect*)widg)->c_fill.get_int_color();
						break;
					case widgCOUNTERPERCBAR:
						ret = 10000*((SW_CounterPercentBar*)widg)->c_fill.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_OLINE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLINE:
						ret = 10000*((SW_Line*)widg)->c_line.get_int_color();
						break;
					case widgRECT:
						ret = 10000*((SW_Rect*)widg)->c_outline.get_int_color();
						break;
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->c_outline.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_PLAYER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_plr.get_int_color();
						break;
					case widgLMAP:
						ret = 10000*((SW_LMap*)widg)->c_plr.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_ROOM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLMAP:
						ret = 10000*((SW_LMap*)widg)->c_room.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_shadow.get_int_color();
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->c_shadow.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_shadow.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_shadow.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_shadow.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_shadow.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_shadow.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_shadow.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_shadow.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_shadow2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_shadow2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->c_text.get_int_color();
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->c_text.get_int_color();
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->c_text.get_int_color();
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->c_text.get_int_color();
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->c_text.get_int_color();
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_text.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_text.get_int_color();
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->c_text.get_int_color();
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->c_text.get_int_color();
						break;
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->c_number.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->c_text2.get_int_color();
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->c_text2.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_VISITED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_room_vis.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_UNVISITED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->c_room_unvis.get_int_color();
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_CONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->container;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COSTIND:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->costind;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -1;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_DELAY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->delay;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FLIP:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						ret = 10000*((SW_McGuffin*)widg)->flip;
						break;
					case widgTILEBLOCK:
						ret = 10000*((SW_TileBlock*)widg)->flip;
						break;
					case widgMINITILE:
						ret = 10000*((SW_MiniTile*)widg)->flip;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FONT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->fontid;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->fontid;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->fontid;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->fontid;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->fontid;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->fontid;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->fontid;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->fontid;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->fontid;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->frame_cset;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMES:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->frames;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->frame_tile;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_HEI:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->gauge_hei+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_WID:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->gauge_wid+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->grid_xoff;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->grid_yoff;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_HSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->hspace;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFCHAR:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*byte(((SW_Counter*)widg)->infchar);
						break;
					case widgOLDCTR:
						ret = 10000*byte(((SW_Counters*)widg)->infchar);
						break;
					case widgBTNCOUNTER:
						ret = 10000*byte(((SW_BtnCounter*)widg)->infchar);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFITM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->infitm;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->infitm;
						break;
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->inf_item;
						break;
					case widgITMCOOLDOWNGAUGE:
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMCLASS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						ret = 10000*((SW_ItemSlot*)widg)->iclass;
						break;
					case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_ItemCooldownGauge*)widg)->item_class;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->item_class;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMID:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						ret = 10000*((SW_ItemSlot*)widg)->iid;
						break;
					case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_ItemCooldownGauge*)widg)->specific_item_id;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->specific_item_id;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_LITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						ret = 10000*((SW_MMap*)widg)->compass_litems;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MAXDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->maxdigits;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->maxdigits;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MINDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->mindigits;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->mindigits;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->digits;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_NUMBER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						ret = 10000*((SW_McGuffin*)widg)->number;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PERCONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMISCGAUGE:
						ret = 10000*((SW_MiscGaugePiece*)widg)->per_container;
						break;
					case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_ItemCooldownGauge*)widg)->per_container;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->piece_cset;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						ret = 10000*((SW_TriFrame*)widg)->piece_tile;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHADOWTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						ret = 10000*((SW_Text*)widg)->shadtype;
						break;
					case widgITMCOOLDOWNTEXT:
						ret = 10000*((SW_ItemCooldownText*)widg)->shadtype;
						break;
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->shadtype;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->shadtype;
						break;
					case widgTIME:
						ret = 10000*((SW_Time*)widg)->shadtype;
						break;
					case widgCOUNTER:
						ret = 10000*((SW_Counter*)widg)->shadtype;
						break;
					case widgBTNCOUNTER:
						ret = 10000*((SW_BtnCounter*)widg)->shadtype;
						break;
					case widgOLDCTR:
						ret = 10000*((SW_Counters*)widg)->shadtype;
						break;
					case widgMMAPTITLE:
						ret = 10000*((SW_MMapTitle*)widg)->shadtype;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHOWDRAIN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMGAUGE:
						ret = 10000*((SW_MagicGaugePiece*)widg)->showdrain;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SPEED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->speed;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TABSIZE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXTBOX:
						ret = 10000*((SW_TextBox*)widg)->tabsize;
						break;
					case widgSELECTEDTEXT:
						ret = 10000*((SW_SelectedText*)widg)->tabsize;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TOTAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_ItemCooldownGauge*)widg)->total_points;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_UNITS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*(((SW_GaugePiece*)widg)->unit_per_frame+1);
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_VSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						ret = 10000*((SW_GaugePiece*)widg)->vspace;
						break;
					default:
						bad_subwidg_type(false, ty);
						ret = -10000;
						break;
				}
			}
			break;
		}
		case SUBWIDGW:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->w;
			break;
		}
		case SUBWIDGX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->x;
			break;
		}
		case SUBWIDGY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->y;
			break;
		}
		case SUBWIDG_DISPH:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->getH();
			break;
		}
		case SUBWIDG_DISPW:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->getW();
			break;
		}
		case SUBWIDG_DISPX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->getX();
			break;
		}
		case SUBWIDG_DISPY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				ret = 10000*widg->getY();
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void subscreenwidget_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SUBWIDGDISPITM: break; //READ-ONLY
		case SUBWIDGEQPITM: break; //READ-ONLY
		case SUBWIDGH:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->h = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGINDEX: break; //READ-ONLY
		case SUBWIDGPAGE: break; //READ-ONLY
		case SUBWIDGPGMODE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->pg_mode = vbound(value/10000,0,PGGOTO_MAX-1);
			break;
		}
		case SUBWIDGPGTARG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->pg_targ = vbound(value/10000,0,MAX_SUBSCR_PAGES-1);
			break;
		}
		case SUBWIDGPOS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->pos = vbound(value/10000,0,255);
			break;
		}
		case SUBWIDGPRESSSCRIPT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->generic_scrconfig.script = vbound(value/10000,0,NUMSCRIPTSGENERIC-1);
			break;
		}
		case SUBWIDGREQCOUNTER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_counter = vbound(value/10000,sscMIN,MAX_COUNTERS);
			break;
		}
		case SUBWIDGREQCOUNTERCOND:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_counter_cond_type = vbound(value/10000,CONDTY_NONE,CONDTY_MAX-1);
			break;
		}
		case SUBWIDGREQCOUNTERVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_counter_val = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGREQLITEMLEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_litem_level = vbound(value/10000,-1,MAXLEVELS);
			break;
		}
		case SUBWIDGREQLITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_litems = vbound(value/10000,0,LI_ALL);
			break;
		}
		case SUBWIDGREQSCRIPTDISABLED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->is_disabled = value != 0;
			break;
		}
		case SUBWIDGREQ_LEVEL_STATE_LEVEL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_lstate_level = vbound(value/10000,-1,MAXLEVELS);
			break;
		}
		case SUBWIDGREQ_SCRSTATE_MAP:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_scrstate_map = vbound(value/10000,0,map_count);
			break;
		}
		case SUBWIDGREQ_SCRSTATE_SCREEN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->req_scrstate_scr = vbound(value/10000,-1,MAPSCRSNORMAL-1);
			break;
		}
		case SUBWIDGSELECTORDSTH:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->selector_override.h = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTW:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->selector_override.w = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->selector_override.x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGSELECTORDSTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
				widg->selector_override.y = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGTRANSPGSFX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
			{
				auto& trans = widg->pg_trans;
				trans.tr_sfx = vbound(value/10000,0,MAX_SFX);
			}
			break;
		}
		case SUBWIDGTRANSPGTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref), {sstACTIVE, sstMAP}))
			{
				auto& trans = widg->pg_trans;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBWIDGTYPE: break; //READ-ONLY
		case SUBWIDGTY_ALIGN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,ALIGN_MAX-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->align = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->align = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->align = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->align = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->align = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->align = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->align = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->align = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ANIMVAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->anim_val = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_BUTTON:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-1,3);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNITM:
						((SW_ButtonItem*)widg)->btn = zc_max(0, val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->btn = zc_max(0, val);
						break;
					case widgITMCOOLDOWNGAUGE:
						((SW_ItemCooldownGauge*)widg)->button_id = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->button_id = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_bg.set_int_color(val);
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->c_bg.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_bg.set_int_color(val);;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_bg.set_int_color(val);;
						break;
					case widgTIME:
						((SW_Time*)widg)->c_bg.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_bg.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_bg.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_bg.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_bg.set_int_color(val);
						break;
					case widgBGCOLOR:
						((SW_Clear*)widg)->c_bg.set_int_color(val);
						break;
					case widgCOUNTERPERCBAR:
						((SW_CounterPercentBar*)widg)->c_bg.set_int_color(val);
						break;
					case widgMMAP:
						((SW_MMap*)widg)->c_room_bg.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_BG2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_bg2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_bg2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPBLNK:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_cmp_blink.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_CMPOFF:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_cmp_off.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_FILL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgRECT:
						((SW_Rect*)widg)->c_fill.set_int_color(val);
						break;
					case widgCOUNTERPERCBAR:
						((SW_CounterPercentBar*)widg)->c_fill.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_OLINE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLINE:
						((SW_Line*)widg)->c_line.set_int_color(val);
						break;
					case widgRECT:
						((SW_Rect*)widg)->c_outline.set_int_color(val);
						break;
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->c_outline.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_PLAYER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_plr.set_int_color(val);
						break;
					case widgLMAP:
						((SW_LMap*)widg)->c_plr.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_ROOM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLMAP:
						((SW_LMap*)widg)->c_room.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_shadow.set_int_color(val);
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->c_shadow.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_shadow.set_int_color(val);
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_shadow.set_int_color(val);
						break;
					case widgTIME:
						((SW_Time*)widg)->c_shadow.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_shadow.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_shadow.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_shadow.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_shadow.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_SHD2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_shadow2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_shadow2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->c_text.set_int_color(val);
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->c_text.set_int_color(val);
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->c_text.set_int_color(val);
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->c_text.set_int_color(val);
						break;
					case widgTIME:
						((SW_Time*)widg)->c_text.set_int_color(val);
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->c_text.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_text.set_int_color(val);
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->c_text.set_int_color(val);
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->c_text.set_int_color(val);
						break;
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->c_number.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_TXT2:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->c_text2.set_int_color(val);
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->c_text2.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_VISITED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_room_vis.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COLOR_UNVISITED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,MIN_SUBSCR_COLOR,MAX_SUBSCR_COLOR);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->c_room_unvis.set_int_color(val);
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_CONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->container = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_COSTIND:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->costind = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_DELAY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->delay = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FLIP:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						((SW_McGuffin*)widg)->flip = val;
						break;
					case widgTILEBLOCK:
						((SW_TileBlock*)widg)->flip = val;
						break;
					case widgMINITILE:
						((SW_MiniTile*)widg)->flip = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FONT:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,font_max-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->fontid = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->fontid = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->fontid = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->fontid = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->fontid = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->fontid = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->fontid = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->fontid = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->fontid = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->frame_cset = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMES:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,1,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->frames = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_FRAMETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,NEWMAXTILES-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->frame_tile = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_HEI:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,1,32)-1;
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->gauge_hei = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GAUGE_WID:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,1,32)-1;
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->gauge_wid = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-32768,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->grid_xoff = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_GRIDY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-32768,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->grid_yoff = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_HSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-128,127);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->hspace = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFCHAR:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				char val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->infchar = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->infchar = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->infchar = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_INFITM:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-1,MAXITEMS-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->infitm = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->infitm = val;
						break;
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE:
						((SW_GaugePiece*)widg)->inf_item = val;
						break;
					case widgITMCOOLDOWNGAUGE:
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMCLASS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,itype_maxusable-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						((SW_ItemSlot*)widg)->iclass = val;
						break;
					case widgITMCOOLDOWNGAUGE:
						((SW_ItemCooldownGauge*)widg)->item_class = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->item_class = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_ITEMID:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-1,MAXITEMS-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITEMSLOT:
						((SW_ItemSlot*)widg)->iid = val;
						break;
					case widgITMCOOLDOWNGAUGE:
						((SW_ItemCooldownGauge*)widg)->specific_item_id = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->specific_item_id = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_LITEMS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMMAP:
						((SW_MMap*)widg)->compass_litems = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MAXDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,5);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->maxdigits = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->maxdigits = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_MINDIG:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,5);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgCOUNTER:
						((SW_Counter*)widg)->mindigits = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->mindigits = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->digits = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_NUMBER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF:
						((SW_McGuffin*)widg)->number = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PERCONTAINER:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = zc_max(value/10000,1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMISCGAUGE:
						((SW_MiscGaugePiece*)widg)->per_container = val;
						break;
					case widgITMCOOLDOWNGAUGE:
						((SW_ItemCooldownGauge*)widg)->per_container = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECECSET:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,15);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->piece_cset = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_PIECETILE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,NEWMAXTILES-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMCGUFF_FRAME:
						((SW_TriFrame*)widg)->piece_tile = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHADOWTY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,sstsMAX-1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXT:
						((SW_Text*)widg)->shadtype = val;
						break;
					case widgITMCOOLDOWNTEXT:
						((SW_ItemCooldownText*)widg)->shadtype = val;
						break;
					case widgTEXTBOX:
						((SW_TextBox*)widg)->shadtype = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->shadtype = val;
						break;
					case widgTIME:
						((SW_Time*)widg)->shadtype = val;
						break;
					case widgCOUNTER:
						((SW_Counter*)widg)->shadtype = val;
						break;
					case widgBTNCOUNTER:
						((SW_BtnCounter*)widg)->shadtype = val;
						break;
					case widgOLDCTR:
						((SW_Counters*)widg)->shadtype = val;
						break;
					case widgMMAPTITLE:
						((SW_MMapTitle*)widg)->shadtype = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SHOWDRAIN:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-1,32767);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgMGAUGE:
						((SW_MagicGaugePiece*)widg)->showdrain = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_SPEED:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,1,65535);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->speed = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TABSIZE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,0,255);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgTEXTBOX:
						((SW_TextBox*)widg)->tabsize = val;
						break;
					case widgSELECTEDTEXT:
						((SW_SelectedText*)widg)->tabsize = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_TOTAL:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = zc_max(value/10000,1);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgITMCOOLDOWNGAUGE:
						((SW_ItemCooldownGauge*)widg)->total_points = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_UNITS:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,1,256);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->unit_per_frame = val-1;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGTY_VSPACE:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
			{
				auto val = vbound(value/10000,-128,127);
				auto ty = widg->getType();
				switch(ty)
				{
					case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
						((SW_GaugePiece*)widg)->vspace = val;
						break;
					default:
						bad_subwidg_type(false, ty);
						break;
				}
			}
			break;
		}
		case SUBWIDGW:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->w = vbound(value/10000,0,65535);
			break;
		}
		case SUBWIDGX:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBWIDGY:
		{
			if(SubscrWidget* widg = checkSubWidg(GET_REF(subscreenwidgref)))
				widg->y = vbound(value/10000,-32768,32767);
			break;
		}

		default:
			NOTREACHED();
	}
}

// subscreenwidget arrays.

static ArrayRegistrar SUBWIDGGENFLAG_registrar(SUBWIDGGENFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::genflags, SUBSCRFLAG_GEN_COUNT> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPOSFLAG_registrar(SUBWIDGPOSFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::posflags, sspNUM> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGFLAG_registrar(SUBWIDGFLAG, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, bool> impl(
		[](SubscrWidget* widg){
			return widg->numFlags();
		},
		[](SubscrWidget* widg, int index) -> bool {
			return widg->flags & (1 << index);
		},
		[](SubscrWidget* widg, int index, bool value){
			SETFLAG(widg->flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGBTNPG_registrar(SUBWIDGBTNPG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidgetActive, &SubscrWidgetActive::pg_btns, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGBTNPRESS_registrar(SUBWIDGBTNPRESS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidgetActive, &SubscrWidgetActive::gen_script_btns, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPRESSINITD_registrar(SUBWIDGPRESSINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<SubscrWidgetActive, &SubscrWidgetActive::generic_scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORASPD_registrar(SUBWIDGSELECTORASPD, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].speed;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].speed = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORFRM_registrar(SUBWIDGSELECTORFRM, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].frames;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].frames = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORTILE_registrar(SUBWIDGSELECTORTILE, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].tile;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].tile = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES - 1>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORHEI_registrar(SUBWIDGSELECTORHEI, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].sh;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].sh = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORWID_registrar(SUBWIDGSELECTORWID, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].sw;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].sw = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORDELAY_registrar(SUBWIDGSELECTORDELAY, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].delay;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].delay = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORCSET_registrar(SUBWIDGSELECTORCSET, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].cset & 0xF;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			byte& cs = widg->selector_override.tileinfo[index].cset;
			cs = (cs & 0xF0) | value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORFLASHCSET_registrar(SUBWIDGSELECTORFLASHCSET, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return (widg->selector_override.tileinfo[index].cset & 0xF0) >> 4;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			byte& cs = widg->selector_override.tileinfo[index].cset;
			cs = (cs & 0x0F) | (value << 4);
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTRANSPGARGS_registrar(SUBWIDGTRANSPGARGS, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_TRANSITION_MAXARG;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			auto& trans = widg->pg_trans;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](SubscrWidgetActive* widg, int index, int value){
			auto& trans = widg->pg_trans;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTRANSPGFLAGS_registrar(SUBWIDGTRANSPGFLAGS, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, bool> impl(
		[](SubscrWidgetActive*){
			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](SubscrWidgetActive* widg, int index) -> bool {
			auto& trans = widg->pg_trans;
			return trans.flags & (1<<index);
		},
		[](SubscrWidgetActive* widg, int index, bool value){
			auto& trans = widg->pg_trans;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQOWNITEMS_registrar(SUBWIDGREQOWNITEMS, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_owned_items, MAXITEMS> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQUNOWNITEMS_registrar(SUBWIDGREQUNOWNITEMS, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_unowned_items, MAXITEMS> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQ_DMAP_FLOOR_registrar(SUBWIDGREQ_DMAP_FLOOR, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_dmap_floors, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQ_DMAP_LEVEL_registrar(SUBWIDGREQ_DMAP_LEVEL, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_dmap_levels, MAXLEVELS> impl;
	impl.setMul10000(true);
	return &impl;
}());
static ArrayRegistrar SUBWIDGREQ_DMAP_registrar(SUBWIDGREQ_DMAP, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_dmaps, MAXDMAPS> impl;
	impl.setMul10000(true);
	return &impl;
}());
static ArrayRegistrar SUBWIDGREQ_SCREEN_registrar(SUBWIDGREQ_SCREEN, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_maps, MAXMAPS> impl;
	impl.setMul10000(true);
	return &impl;
}());
static ArrayRegistrar SUBWIDGREQ_MAP_registrar(SUBWIDGREQ_MAP, []{
	static ScriptingArray_ObjectMemberSet<SubscrWidget, &SubscrWidget::req_screens, MAPSCRSNORMAL> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQ_SCRSTATE_STATE_registrar(SUBWIDGREQ_SCRSTATE_STATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::req_scrstate, mMAXIND> impl;
	impl.setMul10000(true);
	return &impl;
}());
static ArrayRegistrar SUBWIDGREQ_SCRSTATE_EXSTATE_registrar(SUBWIDGREQ_SCRSTATE_EXSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::req_exstate, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());
static ArrayRegistrar SUBWIDGREQ_LEVEL_STATE_registrar(SUBWIDGREQ_LEVEL_STATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::req_lvlstate, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPOSES_registrar(SUBWIDGPOSES, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget*){
			return 4;
		},
		[](SubscrWidget* widg, int index) -> int {
			switch (index)
			{
				case up: return widg->pos_up;
				case down: return widg->pos_down;
				case left: return widg->pos_left;
				case right: return widg->pos_right;
				default: NOTREACHED();
			}
		},
		[](SubscrWidget* widg, int index, int value){
			switch (index)
			{
				case up: widg->pos_up = value; break;
				case down: widg->pos_down = value; break;
				case left: widg->pos_left = value; break;
				case right: widg->pos_right = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTY_CORNER_registrar(SUBWIDGTY_CORNER, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
			switch (widg->getType())
			{
				case widgMINITILE:
					return 1;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return 4;

				default:
					bad_subwidg_type(false, widg->getType());
					return 0;
			}
		},
		[](SubscrWidget* widg, int index) -> int {
			switch (widg->getType())
			{
				case widgMINITILE:
					return ((SW_MiniTile*)widg)->crn;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return ((SW_GaugePiece*)widg)->mts[index].crn();
				
				default: NOTREACHED();
			}
		},
		[](SubscrWidget* widg, int index, int value){
			switch (widg->getType())
			{
				case widgMINITILE:
					((SW_MiniTile*)widg)->crn = value;
					break;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					((SW_GaugePiece*)widg)->mts[index].setCrn(value);
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 3>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTY_COUNTERS_registrar(SUBWIDGTY_COUNTERS, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
			switch (widg->getType())
			{
				case widgCOUNTER:
					return 3;

				case widgMISCGAUGE:
				case widgCOUNTERPERCBAR:
					return 1;

				default:
					bad_subwidg_type(false, widg->getType());
					return 0;
			}
		},
		[](SubscrWidget* widg, int index) -> int {
			switch (widg->getType())
			{
				case widgCOUNTER: return ((SW_Counter*)widg)->ctrs[index];
				case widgMISCGAUGE: return ((SW_MiscGaugePiece*)widg)->counter;
				case widgCOUNTERPERCBAR: return ((SW_CounterPercentBar*)widg)->counter;
				default: NOTREACHED();
			}
		},
		[](SubscrWidget* widg, int index, int value){
			switch (widg->getType())
			{
				case widgCOUNTER:
					((SW_Counter*)widg)->ctrs[index] = value;
					break;
				case widgMISCGAUGE:
					((SW_MiscGaugePiece*)widg)->counter = value;
					break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<sscMIN+1, MAX_COUNTERS-1>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTY_CSET_registrar(SUBWIDGTY_CSET, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
			switch (widg->getType())
			{
				case widgFRAME:
				case widgMCGUFF:
				case widgTILEBLOCK:
				case widgMINITILE:
					return 1;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return 4;

				default:
					bad_subwidg_type(false, widg->getType());
					return 0;
			}
		},
		[](SubscrWidget* widg, int index) -> int {
			switch (widg->getType())
			{
				case widgFRAME: return ((SW_2x2Frame*)widg)->cs.get_cset();
				case widgMCGUFF: return ((SW_McGuffin*)widg)->cs.get_cset();
				case widgTILEBLOCK: return ((SW_TileBlock*)widg)->cs.get_cset();
				case widgMINITILE: return ((SW_MiniTile*)widg)->cs.get_cset();

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return ((SW_GaugePiece*)widg)->mts[index].cset;

				default: NOTREACHED();
			}
		},
		[](SubscrWidget* widg, int index, int value){
			switch (widg->getType())
			{
				case widgFRAME:
					((SW_2x2Frame*)widg)->cs.set_int_cset(value);
					break;
				case widgMCGUFF:
					((SW_McGuffin*)widg)->cs.set_int_cset(value);
					break;
				case widgTILEBLOCK:
					((SW_TileBlock*)widg)->cs.set_int_cset(value);
					break;
				case widgMINITILE:
					((SW_MiniTile*)widg)->cs.set_int_cset(value);
					break;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					value = vbound(value, 0, 15);
					((SW_GaugePiece*)widg)->mts[index].cset = value;
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-sscsMAX, 15>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTY_TILE_registrar(SUBWIDGTY_TILE, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
			switch (widg->getType())
			{
				case widgFRAME:
				case widgMCGUFF:
				case widgTILEBLOCK:
				case widgMINITILE:
					return 1;

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return 4;

				default:
					bad_subwidg_type(false, widg->getType());
					return 0;
			}
		},
		[](SubscrWidget* widg, int index) -> int {
			switch (widg->getType())
			{
				case widgFRAME: return ((SW_2x2Frame*)widg)->tile;
				case widgMCGUFF: return ((SW_McGuffin*)widg)->tile;
				case widgTILEBLOCK: return ((SW_TileBlock*)widg)->tile;
				case widgMINITILE: return ((SW_MiniTile*)widg)->get_int_tile();

				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					return ((SW_GaugePiece*)widg)->mts[index].tile();

				default: NOTREACHED();
			}
		},
		[](SubscrWidget* widg, int index, int value){
			switch (widg->getType())
			{
				case widgFRAME:
					((SW_2x2Frame*)widg)->tile = value;
					break;
				case widgMCGUFF:
					((SW_McGuffin*)widg)->tile = value;
					break;
				case widgTILEBLOCK:
					((SW_TileBlock*)widg)->tile = value;
					break;
				case widgMINITILE:
					// TODO: setValueTransform already bounded to 0... but this is what
					// the code was doing before ScriptingArray refactor.
					value = vbound(value, -ssmstMAX, NEWMAXTILES-1);
					((SW_MiniTile*)widg)->set_int_tile(value);
					break;
				case widgLGAUGE: case widgMGAUGE: case widgMISCGAUGE: case widgITMCOOLDOWNGAUGE:
					((SW_GaugePiece*)widg)->mts[index].setTile(value);
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES-1>);
	return &impl;
}());
