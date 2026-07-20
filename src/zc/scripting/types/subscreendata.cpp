#include "zc/scripting/types/subscreendata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "new_subscr.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t subscreendata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SUBDATACURPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
				if (sub->sub_type == sstACTIVE || sub->sub_type == sstMAP)
					ret = 10000*sub->curpage;
			break;
		}
		case SUBDATACURSORPOS:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				SubscrPage& pg = sub->cur_page();
				ret = pg.cursor_pos * 10000;
			}
			break;
		}
		case SUBDATANUMPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
			{
				if (sub->sub_type == sstACTIVE || sub->sub_type == sstMAP)
					ret = 10000*sub->pages.size();
				else ret = 10000;
			}
			break;
		}
		case SUBDATASCRIPT:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				ret = sub->scrconfig.script * 10000;
			break;
		}
		case SUBDATASELECTORDSTH:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				ret = sub->selector_setting.h * 10000;
			break;
		}
		case SUBDATASELECTORDSTW:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				ret = sub->selector_setting.w * 10000;
			break;
		}
		case SUBDATASELECTORDSTX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				ret = sub->selector_setting.x * 10000;
			break;
		}
		case SUBDATASELECTORDSTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				ret = sub->selector_setting.y * 10000;
			break;
		}
		case SUBDATATRANSCLK:
		{
			ret = -10000;
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransClock' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open && subscr_pg_animating)
					ret = subscr_pg_clk*10000;
			}
			break;
		}
		case SUBDATATRANSFROMPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransFromPage' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					ret = subscr_pg_from*10000;
			}
			break;
		}
		case SUBDATATRANSLEFTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_left;
				ret = trans.tr_sfx * 10000;
			}
			break;
		}
		case SUBDATATRANSLEFTTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_left;
				ret = trans.type * 10000;
			}
			break;
		}
		case SUBDATATRANSRIGHTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_right;
				ret = trans.tr_sfx * 10000;
			}
			break;
		}
		case SUBDATATRANSRIGHTTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_right;
				ret = trans.type * 10000;
			}
			break;
		}
		case SUBDATATRANSTOPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransToPage' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					ret = subscr_pg_to*10000;
			}
			break;
		}
		case SUBDATATRANSTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = subscr_pg_transition;
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransType' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					ret = trans.type*10000;
			}
			break;
		}
		case SUBDATATYPE:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
				ret = sub->sub_type*10000;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void subscreendata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SUBDATACURPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
				if(sub->sub_type == sstACTIVE || sub->sub_type == sstMAP)
					sub->curpage = vbound(value/10000,0,sub->pages.size()-1);
			break;
		}
		case SUBDATACURSORPOS:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				SubscrPage& pg = sub->cur_page();
				//Should this be sanity checked? Or should nulling out
				// the cursor by setting it invalid be allowed? -Em
				pg.cursor_pos = vbound(value/10000,0,255);
			}
			break;
		}
		case SUBDATANUMPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref)))
				if((sub->sub_type == sstACTIVE || sub->sub_type == sstMAP) && value >= 10000)
				{
					size_t sz = value/10000;
					while(sub->pages.size() < sz)
						if(!sub->add_page(MAX_SUBSCR_PAGES))
							break;
					while(sub->pages.size() > sz)
						sub->delete_page(sub->pages.size()-1);
				}
			break;
		}
		case SUBDATASCRIPT:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				sub->scrconfig.script = vbound(value/10000,0,NUMSCRIPTSSUBSCREEN-1);
			break;
		}
		case SUBDATASELECTORDSTH:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				sub->selector_setting.h = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTW:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				sub->selector_setting.w = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				sub->selector_setting.x = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATASELECTORDSTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
				sub->selector_setting.y = vbound(value/10000,-32768,32767);
			break;
		}
		case SUBDATATRANSCLK:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransClock' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
				{
					int val = value/10000;
					if(val < 0)
						subscrpg_clear_animation();
					else if(!subscr_pg_animating)
					{
						SubscrTransition tr = subscr_pg_transition;
						tr.tr_sfx = 0;
						subscrpg_animate(subscr_pg_from,subscr_pg_to,tr,*CURRENT_ACTIVE_SUBSCREEN,*CURRENT_ACTIVE_SUBSCREEN);
						subscr_pg_clk = val;
					}
					else subscr_pg_clk = val;
				}
			}
			break;
		}
		case SUBDATATRANSFROMPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransFromPage' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					subscr_pg_from = vbound(value/10000,0,sub->pages.size()-1);
			}
			break;
		}
		case SUBDATATRANSLEFTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_left;
				trans.tr_sfx = vbound(value/10000,0,MAX_SFX);
			}
			break;
		}
		case SUBDATATRANSLEFTTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_left;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATRANSRIGHTSFX:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_right;
				trans.tr_sfx = vbound(value/10000,0,MAX_SFX);
			}
			break;
		}
		case SUBDATATRANSRIGHTTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = sub->trans_right;
				trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATRANSTOPG:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransToPage' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					subscr_pg_to = vbound(value/10000,0,sub->pages.size()-1);
			}
			break;
		}
		case SUBDATATRANSTY:
		{
			if(ZCSubscreen* sub = checkSubData(GET_REF(subscreendataref), {sstACTIVE, sstMAP}))
			{
				auto& trans = subscr_pg_transition;
				if(sub != CURRENT_ACTIVE_SUBSCREEN)
					Z_scripterrlog("'subscreendata->TransType' is only"
						" valid for the current active/map subscreen!\n");
				else if(subscreen_open)
					trans.type = vbound(value/10000,0,sstrMAX-1);
			}
			break;
		}
		case SUBDATATYPE: break; //READONLY

		default:
			NOTREACHED();
	}
}

// subscreendata arrays.

static ArrayRegistrar SUBDATABTNLEFT_registrar(SUBDATABTNLEFT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ZCSubscreenActive, &ZCSubscreenActive::btn_left, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATABTNRIGHT_registrar(SUBDATABTNRIGHT, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<ZCSubscreenActive, &ZCSubscreenActive::btn_right, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATAINITD_registrar(SUBDATAINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<ZCSubscreenActive, &ZCSubscreenActive::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATAFLAGS_registrar(SUBDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreen, bool> impl(
		[](ZCSubscreen* sub){
			switch(sub->sub_type)
			{
				case sstACTIVE:
					return 2;
				case sstMAP:
					return 2;
				case sstPASSIVE:
					return 0;
				case sstOVERLAY:
					return 0;
			}

			return 0;
		},
		[](ZCSubscreen* sub, int index) -> bool {
			return sub->flags & (1<<index);
		},
		[](ZCSubscreen* sub, int index, bool value){
			SETFLAG(sub->flags, 1<<index, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORASPD_registrar(SUBDATASELECTORASPD, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].speed;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].speed = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSARGS_registrar(SUBDATATRANSARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive* sub){
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			auto& trans = subscr_pg_transition;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return;
			}

			if (!subscreen_open) return;

			auto& trans = subscr_pg_transition;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSLEFTARGS_registrar(SUBDATATRANSLEFTARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			auto& trans = sub->trans_left;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			auto& trans = sub->trans_left;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSRIGHTARGS_registrar(SUBDATATRANSRIGHTARGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_TRANSITION_MAXARG;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			auto& trans = sub->trans_right;
			return trans.arg[index] * SubscrTransition::argScale(trans.type, index);
		},
		[](ZCSubscreenActive* sub, int index, int value){
			auto& trans = sub->trans_right;
			trans.arg[index] = value / SubscrTransition::argScale(trans.type, index);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSFLAGS_registrar(SUBDATATRANSFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive* sub){
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return 0;
			}

			if (!subscreen_open) return 0;

			auto& trans = subscr_pg_transition;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			if (sub != CURRENT_ACTIVE_SUBSCREEN)
			{
				scripting_log_error_with_context("Only valid for the current active/map subscreen!");
				return;
			}

			if (!subscreen_open) return;

			auto& trans = subscr_pg_transition;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSLEFTFLAGS_registrar(SUBDATATRANSLEFTFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			auto& trans = sub->trans_left;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			auto& trans = sub->trans_left;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATATRANSRIGHTFLAGS_registrar(SUBDATATRANSRIGHTFLAGS, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, bool> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_TRANS_NUMFLAGS;
		},
		[](ZCSubscreenActive* sub, int index) -> bool {
			auto& trans = sub->trans_right;
			return trans.flags & (1<<index);
		},
		[](ZCSubscreenActive* sub, int index, bool value){
			auto& trans = sub->trans_right;
			SETFLAG(trans.flags, (1<<index), value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORCSET_registrar(SUBDATASELECTORCSET, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			return cs&0x0F;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			cs = (cs&0xF0) | value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORFLASHCSET_registrar(SUBDATASELECTORFLASHCSET, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			return (cs&0xF0)>>4;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			byte& cs = sub->selector_setting.tileinfo[index].cset;
			cs = (cs&0x0F) | (value<<4);
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORDELAY_registrar(SUBDATASELECTORDELAY, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].delay;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].delay = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORFRM_registrar(SUBDATASELECTORFRM, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].frames;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].frames = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORWID_registrar(SUBDATASELECTORWID, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].sw;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].sw = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORHEI_registrar(SUBDATASELECTORHEI, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].sh;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].sh = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBDATASELECTORTILE_registrar(SUBDATASELECTORTILE, []{
	static ScriptingArray_ObjectComputed<ZCSubscreenActive, int> impl(
		[](ZCSubscreenActive*){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](ZCSubscreenActive* sub, int index) -> int {
			return sub->selector_setting.tileinfo[index].tile;
		},
		[](ZCSubscreenActive* sub, int index, int value){
			sub->selector_setting.tileinfo[index].tile = value;
		}
	);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES - 1>);
	return &impl;
}());

static ArrayRegistrar SUBDATAPAGES_registrar(SUBDATAPAGES, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto* sub = checkSubData(ref))
				return (sub->sub_type == sstACTIVE || sub->sub_type == sstMAP) ? sub->pages.size() : 1;
			return 0;
		},
		[](int ref, int index) -> int {
			if (checkSubData(ref))
			{
				auto [sb, ty, _pg, _ind] = from_subref(ref);
				return get_subref(sb, ty, index, 0);
			}

			return 0;
		},
		[](int, int, int){ return false; }
	);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());
