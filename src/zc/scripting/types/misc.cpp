#include "zc/scripting/types/misc.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "gamedata.h"
#include "zc/scripting/common.h"
#include "zc/zscriptversion.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

template <typename T, size_t N>
static int read_array(const T(&arr)[N], int index)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return 0;

	return arr[index];
}

template <typename T, size_t N>
static bool write_array(T(&arr)[N], int index, T value)
{
	if (BC::checkIndex(index, 0, N - 1) != SH::_NoError)
		return false;

	arr[index] = value;
	return true;
}

int32_t misc_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case CLASS_THISKEY: ret = ri->thiskey; break;
		case CLASS_THISKEY2: ret = ri->thiskey2; break;
		case GDD: // Unused, remove?
			ret = read_array(game->global_d, GET_D(rINDEX) / 10000);
			break;
		case PC: ret = ri->pc; break;
		case REFBITMAP: ret = ri->bitmapref; break;
		case REFBOTTLESHOP: ret = ri->bottleshopref; break;
		case REFBOTTLETYPE: ret = ri->bottletyperef; break;
		case REFCOMBODATA: ret = ri->combodataref; break;
		case REFCOMBOTRIGGER: ret = ri->combotriggerref; break;
		case REFDIRECTORY: ret = ri->directoryref; break;
		case REFDMAPDATA: ret = ri->dmapdataref; break;
		case REFDROPSETDATA: ret = ri->dropsetdataref; break;
		case REFEWPN: ret = ri->ewpnref; break;
		case REFFFC:
			ret = ZScriptVersion::ffcRefIsSpriteId() ? ri->ffcref : ri->ffcref * 10000;
			break;
		case REFFILE: ret = ri->fileref; break;
		case REFGENERICDATA: ret = ri->genericdataref; break;
		case REFITEM: ret = ri->itemref; break;
		case REFITEMDATA: ret = ri->itemdataref; break;
		case REFLWPN: ret = ri->lwpnref; break;
		case REFMAPDATA: ret = ri->mapdataref; break;
		case REFMSGDATA: ret = ri->msgdataref; break;
		case REFMUSIC: ret = ri->musicref; break;
		case REFNPC: ret = ri->npcref; break;
		case REFNPCDATA: ret = ri->npcdataref; break;
		case REFPALDATA: ret = ri->paldataref; break;
		case REFPORTAL: ret = ri->portalref; break;
		case REFRNG: ret = ri->rngref; break;
		case REFSAVEMENU: ret = ri->savemenuref; break;
		case REFSAVPORTAL: ret = ri->savportalref; break;
		case REFSCREEN: ret = ri->screenref; break;
		case REFSHOPDATA: ret = ri->shopdataref; break;
		case REFSPRITE: ret = ri->spriteref; break;
		case REFSPRITEDATA: ret = ri->spritedataref; break;
		case REFSTACK: ret = ri->stackref; break;
		case REFSUBSCREENDATA: ret = ri->subscreendataref; break;
		case REFSUBSCREENPAGE: ret = ri->subscreenpageref; break;
		case REFSUBSCREENWIDG: ret = ri->subscreenwidgref; break;
		case REFWEBSOCKET: ret = ri->websocketref; break;
		case SP:
			ret = ri->sp * 10000;
			break;
		case SP2:
			ret = ri->sp;
			break;
		case SWITCHKEY:
			ret = ri->switchkey;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void misc_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case CLASS_THISKEY: ri->thiskey = value; break;
		case CLASS_THISKEY2: ri->thiskey2 = value; break;
		case GDD:
			write_array(game->global_d, GET_D(rINDEX) / 10000, value);
			break;
		case PC:
			ri->pc = value;
			break;
		case REFBITMAP: ri->bitmapref = value; break;
		case REFBOTTLESHOP:  ri->bottleshopref = value; break;
		case REFBOTTLETYPE:  ri->bottletyperef = value; break;
		case REFCOMBODATA: ri->combodataref = value; break;
		case REFCOMBOTRIGGER: ri->combotriggerref = value; break;
		case REFDIRECTORY: ri->directoryref = value; break;
		case REFDMAPDATA: ri->dmapdataref = value; break;
		case REFDROPSETDATA:  ri->dropsetdataref = value; break;
		case REFEWPN:
			ri->ewpnref = value;
			break;
		case REFFFC:
			ri->ffcref = ZScriptVersion::ffcRefIsSpriteId() ? value : value / 10000;
			break;
		case REFFILE: ri->fileref = value; break;
		case REFGENERICDATA:  ri->genericdataref = value; break;
		case REFITEM:
			ri->itemref = value;
			break;
		case REFITEMDATA:
			ri->itemdataref = value;
			break;
		case REFLWPN:
			ri->lwpnref = value;
			break;
		case REFMAPDATA: ri->mapdataref = value; break;
		case REFMSGDATA: ri->msgdataref = value; break;
		case REFMUSIC: ri->musicref = value; break;
		case REFNPC:
			ri->npcref = value;
			break;
		case REFNPCDATA: ri->npcdataref = value; break;
		case REFPALDATA: ri->paldataref = value; break;
		case REFPORTAL:
		{
			ri->portalref = value;
			break;
		}
		case REFRNG: ri->rngref = value; break;
		case REFSAVEMENU: ri->savemenuref = value; break;
		case REFSAVPORTAL:
		{
			ri->savportalref = value;
			break;
		}
		case REFSCREEN: ri->screenref = value; break;
		case REFSHOPDATA: ri->shopdataref = value; break;
		case REFSPRITE:
			ri->spriteref = value;
			break;
		case REFSPRITEDATA: ri->spritedataref = value; break;
		case REFSTACK: ri->stackref = value; break;
		case REFSUBSCREENDATA: ri->subscreendataref = value; break;
		case REFSUBSCREENPAGE: ri->subscreenpageref = value; break;
		case REFSUBSCREENWIDG: ri->subscreenwidgref = value; break;
		case REFWEBSOCKET: ri->websocketref = value; break;
		case SP:
			ri->sp = value / 10000;
			break;
		case SP2:
			ri->sp = value;
			break;
		case SWITCHKEY:
			ri->switchkey = value;
			break;

		default:
			NOTREACHED();
	}
}
