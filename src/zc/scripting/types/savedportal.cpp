#include "zc/scripting/types/savedportal.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "gamedata.h"
#include "zc/ffscript.h"
#include "zc/scripting/types/portal.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

savedportal* checkSavedPortal(int32_t ref, bool skiperr)
{
	savedportal* sp = game->getSavedPortal(ref);
	if(!sp)
	{
		if(!skiperr)
			scripting_log_error_with_context("Invalid savedportal pointer: {}", ref);
		return nullptr;
	}
	return sp;
}

int32_t getPortalFromSaved(savedportal* p)
{
	if(p == &(game->saved_mirror_portal))
		return -1;
	portal* prtl = nullptr;
	portals.forEach([&](sprite& spr)
	{
		portal* tmp = (portal*)&spr;
		if(p->getUID() == tmp->saved_data)
		{
			prtl = tmp;
			return true;
		}
		return false;
	});
	return prtl ? prtl->getUID() : 0;
}

int32_t savedportal_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SAVEDPORTALDESTDMAP:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->destdmap * 10000;
			break;
		}
		case SAVEDPORTALDSTSCREEN:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->destscr * 10000;
			break;
		}
		case SAVEDPORTALPORTAL:
		{
			ret = 0;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = getPortalFromSaved(p);
			break;
		}
		case SAVEDPORTALSPRITE:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->spr * 10000;
			break;
		}
		case SAVEDPORTALSRCDMAP:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->srcdmap * 10000;
			break;
		}
		case SAVEDPORTALSRCSCREEN:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->srcscr * 10000;
			break;
		}
		case SAVEDPORTALWARPSFX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->sfx * 10000;
			break;
		}
		case SAVEDPORTALWARPVFX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->warpfx * 10000;
			break;
		}
		case SAVEDPORTALX:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->x;
			break;
		}
		case SAVEDPORTALY:
		{
			ret = -10000;
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				ret = p->y;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void savedportal_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SAVEDPORTALDESTDMAP:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->destdmap = vbound(value/10000, -1, MAXDMAPS-1);
			break;
		}
		case SAVEDPORTALDSTSCREEN:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->destscr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALPORTAL:
		{
			if(ri->savportalref < 0 || value < 0) break;
			if(savedportal* sp = checkSavedPortal(GET_REF(savportalref)))
			{
				int32_t id = getPortalFromSaved(sp);
				if(id == value) break; //no change
				portal* p = checkPortal(value);
				if(p)
				{
					p->saved_data = sp->getUID();
					if(id > 0)
					{
						portal* p = checkPortal(id);
						p->saved_data = 0;
					}
				}
			}
			break;
		}
		case SAVEDPORTALSPRITE:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->spr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALSRCDMAP:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->srcdmap = vbound(value/10000, -1, MAXDMAPS-1);
			break;
		}
		case SAVEDPORTALSRCSCREEN:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->srcscr = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALWARPSFX:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->sfx = vbound(value/10000,0,MAX_SFX);
			break;
		}
		case SAVEDPORTALWARPVFX:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->warpfx = vbound(value/10000,0,255);
			break;
		}
		case SAVEDPORTALX:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->x = value;
			break;
		}
		case SAVEDPORTALY:
		{
			if(savedportal* p = checkSavedPortal(GET_REF(savportalref)))
				p->y = value;
			break;
		}

		default:
			NOTREACHED();
	}
}
