#include "zc/scripting/types/portal.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "sprite.h"
#include "zc/ffscript.h"
#include "zc/scripting/types/savedportal.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

portal* checkPortal(int32_t ref, bool skiperr)
{
	extern portal mirror_portal;

	if(ref == -1)
		return &mirror_portal;

	portal* p = (portal*)portals.getByUID(ref);
	if(!p)
	{
		if(!skiperr)
			scripting_log_error_with_context("Invalid portal pointer: {}", ref);
		return nullptr;
	}
	return p;
}

int32_t portal_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case PORTALACLK:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->aclk*10000;
			break;
		}
		case PORTALAFRM:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->aframe*10000;
			break;
		}
		case PORTALASPD:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->aspd*10000;
			break;
		}
		case PORTALCLOSEDIS:
		{
			ret = 0;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->prox_active ? 0 : 10000; //Inverted
			break;
		}
		case PORTALDMAP:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->destdmap*10000;
			break;
		}
		case PORTALFRAMES:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->frames*10000;
			break;
		}
		case PORTALOTILE:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->o_tile*10000;
			break;
		}
		case PORTALSAVED:
		{
			ret = 0;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->saved_data;
			break;
		}
		case PORTALSCREEN:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->destscr*10000;
			break;
		}
		case PORTALWARPSFX:
		{
			ret = 0;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->wsfx ? 0 : 10000;
			break;
		}
		case PORTALWARPVFX:
		{
			ret = 0;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->weffect ? 0 : 10000;
			break;
		}
		case PORTALX:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->x.getZLong();
			break;
		}
		case PORTALY:
		{
			ret = -10000;
			if(portal* p = checkPortal(GET_REF(portalref)))
				ret = p->y.getZLong();
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void portal_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case PORTALACLK:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->aclk = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALAFRM:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->aframe = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALASPD:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->aspd = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALCLOSEDIS:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->prox_active = value==0; //Inverted
			break;
		}
		case PORTALDMAP:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->destdmap = vbound(value/10000,-1,MAXDMAPS-1);
			break;
		}
		case PORTALFRAMES:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->frames = vbound(value/10000, 0, 9999);
			break;
		}
		case PORTALOTILE:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->o_tile = vbound(value/10000, 0, NEWMAXTILES-1);
			break;
		}
		case PORTALSAVED:
		{
			if(ri->portalref < 0 || value < 0) break;
			if(portal* p = checkPortal(GET_REF(portalref)))
			{
				if(!value)
					p->saved_data = 0;
				else if(savedportal* sp = checkSavedPortal(value))
					p->saved_data = sp->getUID();
			}
			break;
		}
		case PORTALSCREEN:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->destscr = vbound(value/10000,0,255);
			break;
		}
		case PORTALWARPSFX:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->wsfx = vbound(value/10000,0,255);
			break;
		}
		case PORTALWARPVFX:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->weffect = vbound(value/10000,0,255);
			break;
		}
		case PORTALX:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->x = zslongToFix(value);
			break;
		}
		case PORTALY:
		{
			if(portal* p = checkPortal(GET_REF(portalref)))
				p->y = zslongToFix(value);
			break;
		}

		default:
			NOTREACHED();
	}
}
