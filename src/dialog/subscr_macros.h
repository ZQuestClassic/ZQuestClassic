#ifndef ZC_DLG_SUBSCR_MACROS_H_
#define ZC_DLG_SUBSCR_MACROS_H_


#define NUM_FIELD(var,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = var, \
	onValChangedFunc = [=](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		var = val; \
	})

#define NUM_FIELD_OFFS(var,_min,_max,offs) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = var+offs, \
	onValChangedFunc = [=](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		var = val-offs; \
	})


#define CBOX(var, bit, txt, cspan) \
Checkbox( \
	colSpan = cspan, \
	text = txt, hAlign = 0.0, \
	checked = var & bit, \
	onToggleFunc = [=](bool state) \
	{ \
		SETFLAG(var, bit, state); \
	} \
)

#define CBOX_EX(var, bit, txt, ...) \
Checkbox( \
	__VA_ARGS__, \
	text = txt, \
	checked = var & bit, \
	onToggleFunc = [=](bool state) \
	{ \
		SETFLAG(var, bit, state); \
	} \
)

#define DDL(var, lister) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
	} \
)

#define DDL_PROC(var, lister, proc) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
		proc(); \
	} \
)

#define DDL_EX(var, lister, ...) \
DropDownList(data = lister, \
	fitParent = true, \
	__VA_ARGS__, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
	} \
)


#define SELECTOR_GRAPHIC(seltileinfo) \
Rows<3>( \
	SelTileSwatch( \
		colSpan = 2, \
		tile = seltileinfo.tile, \
		cset = seltileinfo.cset&0xF, \
		showvals = false, \
		onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t) \
		{ \
			seltileinfo.tile = t; \
			word c2 = (seltileinfo.cset&0xF0); \
			seltileinfo.cset = c2|(c&0x0F); \
		} \
	), \
	INFOBTN("Override the tile/cs used by Selectors"), \
	Label(text = "Pixel Width:", hAlign = 1.0), \
	NUM_FIELD(seltileinfo.sw,0,256), \
	INFOBTN("The width, in PIXELS, to draw from tiles. (This divided by 16, rounded up," \
		" is the tile width used)"), \
	Label(text = "Pixel Height:", hAlign = 1.0), \
	NUM_FIELD(seltileinfo.sh,0,256), \
	INFOBTN("The height, in PIXELS, to draw from tiles. (This divided by 16, rounded up," \
		" is the tile height used)"), \
	Label(text = "AFrames:", hAlign = 1.0), \
	NUM_FIELD(seltileinfo.frames,0,255), \
	INFOBTN("The number of frames in the animation"), \
	Label(text = "ASpeed:", hAlign = 1.0), \
	NUM_FIELD(seltileinfo.speed,0,255), \
	INFOBTN("The speed of the animation"), \
	Label(text = "ADelay:", hAlign = 1.0), \
	NUM_FIELD(seltileinfo.delay,0,255), \
	INFOBTN("The delay time of the animation"), \
	Label(text = "Flash CSet:", hAlign = 1.0), \
	TextField( \
		fitParent = true, \
		type = GUI::TextField::type::INT_DECIMAL, \
		low = 0, high = 13, val = seltileinfo.cset>>4, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
		{ \
			word c2 = seltileinfo.cset&0x0F; \
			seltileinfo.cset = c2|(val<<4); \
		}), \
	INFOBTN("The cset to flash to (set same as the normal cset to not flash)") \
)

#endif

