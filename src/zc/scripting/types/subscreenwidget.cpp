#include "zc/scripting/arrays.h"

// subscreenwidget arrays.

static ArrayRegistrar SUBWIDGGENFLAG_registrar(SUBWIDGGENFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::genflags, SUBSCRFLAG_GEN_COUNT> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPOSFLAG_registrar(SUBWIDGPOSFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidget, &SubscrWidget::posflags, sspNUM> impl;
	impl.setDefaultValue(0);
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGBTNPG_registrar(SUBWIDGBTNPG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidgetActive, &SubscrWidgetActive::pg_btns, 8> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGBTNPRESS_registrar(SUBWIDGBTNPRESS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SubscrWidgetActive, &SubscrWidgetActive::gen_script_btns, 8> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPRESSINITD_registrar(SUBWIDGPRESSINITD, []{
	static ScriptingArray_ObjectMemberCArray<SubscrWidgetActive, &SubscrWidgetActive::generic_initd> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORASPD_registrar(SUBWIDGSELECTORASPD, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].speed;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].speed = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORFRM_registrar(SUBWIDGSELECTORFRM, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].frames;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].frames = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<1, 255>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORTILE_registrar(SUBWIDGSELECTORTILE, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].tile;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].tile = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES - 1>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORHEI_registrar(SUBWIDGSELECTORHEI, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].sh;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].sh = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORWID_registrar(SUBWIDGSELECTORWID, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].sw;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].sw = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORDELAY_registrar(SUBWIDGSELECTORDELAY, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
			return SUBSCR_SELECTOR_NUMTILEINFO;
		},
		[](SubscrWidgetActive* widg, int index) -> int {
			return widg->selector_override.tileinfo[index].delay;
		},
		[](SubscrWidgetActive* widg, int index, int value){
			widg->selector_override.tileinfo[index].delay = value;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORCSET_registrar(SUBWIDGSELECTORCSET, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGSELECTORFLASHCSET_registrar(SUBWIDGSELECTORFLASHCSET, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 0x0F>);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTRANSPGARGS_registrar(SUBWIDGTRANSPGARGS, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, int> impl(
		[](SubscrWidgetActive* widg){
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
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGTRANSPGFLAGS_registrar(SUBWIDGTRANSPGFLAGS, []{
	static ScriptingArray_ObjectComputed<SubscrWidgetActive, bool> impl(
		[](SubscrWidgetActive* widg){
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQOWNITEMS_registrar(SUBWIDGREQOWNITEMS, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, bool> impl(
		[](SubscrWidget* widg){
			return MAXITEMS;
		},
		[](SubscrWidget* widg, int index) -> bool {
			return widg->req_owned_items.contains(index);
		},
		[](SubscrWidget* widg, int index, bool value){
			if (value)
				widg->req_owned_items.insert(index);
			else
				widg->req_owned_items.erase(index);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGREQUNOWNITEMS_registrar(SUBWIDGREQUNOWNITEMS, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, bool> impl(
		[](SubscrWidget* widg){
			return MAXITEMS;
		},
		[](SubscrWidget* widg, int index) -> bool {
			return widg->req_unowned_items.contains(index);
		},
		[](SubscrWidget* widg, int index, bool value){
			if (value)
				widg->req_unowned_items.insert(index);
			else
				widg->req_unowned_items.erase(index);
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SUBWIDGPOSES_registrar(SUBWIDGPOSES, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
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
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static void bad_subwidg_type(bool func, byte type)
{
	auto tyname = type < widgMAX ? subwidg_internal_names[type].c_str() : "";
	scripting_log_error_with_context("Widget type {} '{}' does not have this {}!",
		type, tyname, func ? "function" : "value");
}

static ArrayRegistrar SUBWIDGTY_CORNER_registrar(SUBWIDGTY_CORNER, []{
	static ScriptingArray_ObjectComputed<SubscrWidget, int> impl(
		[](SubscrWidget* widg){
			switch (widg->getType())
			{
				case widgMINITILE:
					return 1;

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
					((SW_GaugePiece*)widg)->mts[index].setCrn(value);
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
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
	impl.setDefaultValue(-10000);
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
					value = vbound(value, 0, 15);
					((SW_GaugePiece*)widg)->mts[index].cset = value;
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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

				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
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
				case widgLGAUGE:
				case widgMGAUGE:
				case widgMISCGAUGE:
					((SW_GaugePiece*)widg)->mts[index].setTile(value);
					break;

				default: NOTREACHED();
			}
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, NEWMAXTILES-1>);
	return &impl;
}());
