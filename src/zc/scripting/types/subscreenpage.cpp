#include "zc/scripting/types/subscreenpage.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t subscreenpage_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SUBPGCURSORPOS: 
		{
			if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				ret = pg->cursor_pos * 10000;
			break;
		}
		case SUBPGINDEX: 
		{
			if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				ret = pg->getIndex() * 10000;
			break;
		}
		case SUBPGNUMWIDG: 
		{
			if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				ret = pg->size() * 10000;
			break;
		}
		case SUBPGSUBDATA:
		{
			if(checkSubPage(GET_REF(subscreenpageref)))
			{
				auto [sub,ty,_pgid,_ind] = from_subref(GET_REF(subscreenpageref));
				ret = get_subref(sub,ty,0,0);
			}
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void subscreenpage_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SUBPGCURSORPOS:
		{
			if(SubscrPage* pg = checkSubPage(GET_REF(subscreenpageref)))
				pg->cursor_pos = vbound(value/10000,0,255);
			break;
		}
		case SUBPGINDEX: break; //READ-ONLY
		case SUBPGNUMWIDG: break; //READ-ONLY
		case SUBPGSUBDATA: break; //READ-ONLY

		default:
			NOTREACHED();
	}
}

// subscreenpage arrays.

static ArrayRegistrar SUBPGWIDGETS_registrar(SUBPGWIDGETS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref){
			if (SubscrPage* pg = checkSubPage(ref))
				return (int)pg->size();
			return 0;
		},
		[](int ref, int index){
			if (checkSubPage(ref))
			{
				auto [sb, ty, pg, _ind] = from_subref(ref);
				return (int)get_subref(sb, ty, pg, index);
			}

			return 0;
		},
		[](int, int, int){ return false; }
	);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());
