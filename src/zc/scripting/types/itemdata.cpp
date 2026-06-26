#include "zc/scripting/types/itemdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

int32_t item_flag(item_flags flag)
{
	if(invalid_item_id(GET_REF(itemdataref)))
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
		return 0;
	}
	return (itemsbuf.get(GET_REF(itemdataref)).flags & flag) ? 10000 : 0;
}
void item_flag(item_flags flag, bool val)
{
	if(invalid_item_id(GET_REF(itemdataref)))
	{
		scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
		return;
	}
	SETFLAG(itemsbuf[GET_REF(itemdataref)].flags, flag, val);
}

} // end namespace

int32_t itemdata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case IDATAAMOUNT:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			int32_t v = itemsbuf.get(GET_REF(itemdataref)).amount;
			ret = ((v&0x4000)?-1:1)*(v & 0x3FFF)*10000;
			break;
		}
		case IDATAASPEED:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).speed)*10000;
			break;
		case IDATABUNNYABLE:
			ret = item_flag(item_bunny_enabled);
			break;
		case IDATACOLLECTFLAGS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).collectflags)*10000;
			break;
		case IDATACOMBINE:
			ret = item_flag(item_combine);
			break;
		case IDATACONSTSCRIPT:
			ret = item_flag(item_passive_script);
			break;
		case IDATACOOLDOWN:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret = (itemsbuf.get(GET_REF(itemdataref)).cooldown) * 10000;
			break;
		case IDATACOST2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).cost_amount[1])*10000;
			break;
		case IDATACOSTCOUNTER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).cost_counter[0])*10000;
			break;
		case IDATACOSTCOUNTER2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).cost_counter[1])*10000;
			break;
		case IDATACOUNTER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).count)*10000;
			break;
		case IDATACSET:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}

			ret = (itemsbuf.get(GET_REF(itemdataref)).csets&15)*10000;

			// If we find quests that broke, use this code.
			// if (QHeader.compareVer(2, 55, 9) >= 0)
			// 	ret = (itemsbuf.get(ri->idata).csets&15)*10000;
			// else
			// 	ret = itemsbuf.get(ri->idata).csets*10000;
			break;
		case IDATADELAY:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).delay)*10000;
			break;
		case IDATADOWNGRADE:
			ret = item_flag(item_downgrade);
			break;
		case IDATADRAWLAYER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).drawlayer)*10000;
			break;
		case IDATADUPLICATES:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).duplicates)*10000;
			break;
		case IDATADURATION:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weapduration)*10000;
			break;
		case IDATADXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).xofs)*10000;
			break;
		case IDATADYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).yofs)*10000;
			break;
		case IDATAEDIBLE:
			ret = item_flag(item_edible);
			break;
		case IDATAFLAGUNUSED:
			ret = item_flag(item_unused);
			break;
		case IDATAFLASHCSET:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).csets>>4)*10000;
			break;
		case IDATAFRAMES:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).frames)*10000;
			break;
		case IDATAGAINLOWER:
			ret = item_flag(item_gain_old);
			break;
		case IDATAGRADUAL:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret = (itemsbuf.get(GET_REF(itemdataref)).amount&0x8000) ? 10000 : 0;
			break;
		}
		case IDATAHXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).hxofs)*10000;
			break;
		case IDATAHXSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).hxsz)*10000;
			break;
		case IDATAHYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).hyofs)*10000;
			break;
		case IDATAHYSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).hysz)*10000;
			break;
		case IDATAHZSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).hzsz)*10000;
			break;
		case IDATAID:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				ret = -10000;
				break;
			}
			ret=GET_REF(itemdataref)*10000;
			break;
		case IDATAJINXIMMUNE:
			ret = item_flag(item_jinx_immune);
			break;
		case IDATAJINXSWAP:
			ret = item_flag(item_flip_jinx);
			break;
		case IDATAKEEP:
			ret = item_flag(item_gamedata);
			break;
		case IDATAKEEPOLD:
			ret = item_flag(item_keep_old);
			break;
		case IDATALEVEL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).level)*10000;
			break;
		case IDATALTM:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).ltm)*10000;
			break;
		case IDATAMAGCOST:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).cost_amount[0])*10000;
			break;
		case IDATAMAGICTIMER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).magiccosttimer[0])*10000;
			break;
		case IDATAMAGICTIMER2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).magiccosttimer[1])*10000;
			break;
		case IDATAMAX:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).max)*10000;
			break;
		case IDATAMINHEARTS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).pickup_hearts)*10000;
			break;
		case IDATAMISC:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).misc_flags)*10000;
			break;
		case IDATAOVERRIDEFL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).overrideFLAGS)*10000;
			break;
		case IDATAOVERRIDEFLWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.override_flags)*10000;
			break;
		case IDATAPFLAGS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret = (itemsbuf.get(GET_REF(itemdataref)).pickup_string_flags)*10000;
			break;
		case IDATAPICKUP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).pickup)*10000;
			break;
		case IDATAPICKUPLITEMLEVEL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret = (itemsbuf.get(GET_REF(itemdataref)).pickup_litem_level)*10000;
			break;
		case IDATAPICKUPLITEMS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
				break;
			}
			ret = (itemsbuf.get(GET_REF(itemdataref)).pickup_litems)*10000;
			break;
		case IDATAPOWER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).power)*10000;
			break;
		case IDATAPSCRIPT:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).collect_scrconfig.script)*10000;
			break;
		case IDATAPSOUND:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).playsound)*10000;
			break;
		case IDATAPSTRING:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).pstring)*10000;
			break;
		case IDATARUPEECOST:
			ret = item_flag(item_rupee_magic);
			break;
		case IDATASCRIPT:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			
			ret=(itemsbuf.get(GET_REF(itemdataref)).scrconfig.script)*10000;
			break;
		case IDATASETMAX:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).setmax)*10000;
			break;
		case IDATASPRSCRIPT:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).sprite_scrconfig.script)*10000;
			break;
		case IDATASSWIMDISABLED:
			ret = item_flag(item_sideswim_disabled);
			break;
		case IDATATILE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).tile)*10000;
			break;
		case IDATATILEH:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).tileh)*10000;
			break;
		case IDATATILEHWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.tileh)*10000;
			break;
		case IDATATILEW:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).tilew)*10000;
			break;
		case IDATATILEWWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.tilew)*10000;
			break;
		case IDATATYPE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).type)*10000;
			break;
		case IDATAUSEBURNSPR:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = 0;
			}
			else ret = (itemsbuf.get(GET_REF(itemdataref)).weap_data.wflags & WFLAG_UPDATE_IGNITE_SPRITE) ? 10000 : 0;
			break;
		case IDATAUSEDEF:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.default_defense)*10000;
			break;
		case IDATAUSESOUND:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).usesound)*10000;
			break;
		case IDATAUSESOUND2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).usesound2)*10000;
			break;
		case IDATAUSEWPN:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.imitate_weapon)*10000;
			break;
		case IDATAVALIDATE:
			ret = item_flag(item_validate_only);
			break;
		case IDATAVALIDATE2:
			ret = item_flag(item_validate_only_2);
			break;
		case IDATAWEAPHXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.hxofs)*10000;
			break;
		case IDATAWEAPHXSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.hxsz)*10000;
			break;
		case IDATAWEAPHYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.hyofs)*10000;
			break;
		case IDATAWEAPHYSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.hysz)*10000;
			break;
		case IDATAWEAPHZSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.hzsz)*10000;
			break;
		case IDATAWEAPONSCRIPT:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.scrconfig.script)*10000;
			break;
		case IDATAWEAPXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.xofs)*10000;
			break;
		case IDATAWEAPYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weap_data.yofs)*10000;
			break;
		case IDATAWEAPZOFS:
			scripting_log_error_with_context("itemdata::WeaponDrawZOffset is not implemented");
			ret = 0;
			break;
		case IDATAWRANGE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret=(itemsbuf.get(GET_REF(itemdataref)).weaprange)*10000;
			break;
		case IDATA_WEAPONDATA:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				ret = -10000;
				break;
			}
			ret = FFCore.get_new_weapondata(wdata_type::itemdata, GET_REF(itemdataref));
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void itemdata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case IDATAAMOUNT:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			int32_t v = vbound(value/10000, -9999, 16383);
			auto& itm = itemsbuf[GET_REF(itemdataref)];
			itm.amount &= 0x8000;
			itm.amount |= (abs(v)&0x3FFF)|(v<0?0x4000:0);
			break;
		}
		case IDATAASPEED:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].speed=vbound(value/10000, 0, 214748);
			break;
		case IDATABUNNYABLE:
			item_flag(item_bunny_enabled, value);
			break;
		case IDATACOLLECTFLAGS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			//int32_t a = GET_D(rINDEX) / 10000;
			(itemsbuf[GET_REF(itemdataref)].collectflags)=vbound(value/10000, 0, 214747);
			break;
		case IDATACOMBINE:
			item_flag(item_combine, value);
			break;
		case IDATACONSTSCRIPT:
			item_flag(item_passive_script, value);
			break;
		case IDATACOOLDOWN:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].cooldown = zc_max(value/10000,0);
			break;
		case IDATACOST2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].cost_amount[1]=vbound(value/10000,32767,-32768);
			break;
		case IDATACOSTCOUNTER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].cost_counter[0]=(vbound(value/10000,-1,MAX_COUNTERS-1));
			break;
		case IDATACOSTCOUNTER2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].cost_counter[1]=(vbound(value/10000,-1,MAX_COUNTERS-1));
			break;
		case IDATACOUNTER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].count)=vbound(value/10000,0,MAX_COUNTERS-1);
			break;
		case IDATACSET:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}

			itemsbuf[GET_REF(itemdataref)].csets = (itemsbuf[GET_REF(itemdataref)].csets & 0xF0) | vbound(value/10000,0,15);

			// If we find quests that broke, use this code.
			// if (QHeader.compareVer(2, 55, 9) >= 0)
			// 	itemsbuf[ri->idata].csets = (itemsbuf[ri->idata].csets & 0xF0) | vbound(value/10000,0,15);
			// else
			// 	itemsbuf[ri->idata].csets = vbound(value/10000,0,13);
			break;
		case IDATADELAY:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].delay=vbound(value/10000, 0, 214748);
			break;
		case IDATADOWNGRADE:
			item_flag(item_downgrade, value);
			break;
		case IDATADRAWLAYER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].drawlayer)=vbound(value/10000, 0, 7);
			break;
		case IDATADUPLICATES:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].duplicates)=vbound(value/10000, 0, 255);
			break;
		case IDATADURATION:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weapduration)=vbound(value/10000, 0, 255);
			break;
		case IDATADXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].xofs)=(value/10000);
			break;
		case IDATADYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].yofs)=(value/10000);
			break;
		case IDATAEDIBLE:
			item_flag(item_edible, value);
			break;
		case IDATAFLAGUNUSED:
			item_flag(item_unused, value);
			break;
		case IDATAFLASHCSET:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}

			itemsbuf[ri->itemdataref].csets = (itemsbuf[ri->itemdataref].csets & 0xF) | (vbound(value/10000,0,15)<<4);
			break;
		case IDATAFRAMES:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].frames)=vbound(value/10000, 0, 214748);
			break;
		case IDATAGAINLOWER:
			item_flag(item_gain_old, value);
			break;
		case IDATAGRADUAL:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			SETFLAG(itemsbuf[GET_REF(itemdataref)].amount, 0x8000, value!=0);
			break;
		}
		case IDATAHXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].hxofs)=(value/10000);
			break;
		case IDATAHXSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].hxsz)=(value/10000);
			break;
		case IDATAHYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].hyofs)=(value/10000);
			break;
		case IDATAHYSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].hysz)=(value/10000);
			break;
		case IDATAHZSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].hzsz)=(value/10000);
			break;
		case IDATAJINXIMMUNE:
			item_flag(item_jinx_immune, value);
			break;
		case IDATAJINXSWAP:
			item_flag(item_flip_jinx, value);
			break;
		case IDATAKEEP:
			item_flag(item_gamedata, value);
			break;
		case IDATAKEEPOLD:
			item_flag(item_keep_old, value);
			break;
		case IDATALEVEL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].level)=vbound(value/10000, 0, 512);
			flushItemCache();
			break;
		case IDATALTM:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			auto new_value = value/10000;
			auto& itm = itemsbuf[GET_REF(itemdataref)];
			if (new_value != itm.ltm)
				cache_tile_mod_clear();
			itm.ltm = new_value;
			break;
		}
		case IDATAMAGCOST:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].cost_amount[0]=vbound(value/10000,32767,-32768);
			break;
		case IDATAMAGICTIMER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].magiccosttimer[0])=vbound(value/10000, 0, 214747);
			break;
		case IDATAMAGICTIMER2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].magiccosttimer[1])=vbound(value/10000, 0, 214747);
			break;
		case IDATAMAX:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].max)=value/10000;
			break;
		case IDATAMINHEARTS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].pickup_hearts=vbound(value/10000, 0, 214748);
			break;
		case IDATAMISC:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].misc_flags=value/10000;
			break;
		case IDATAOVERRIDEFL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].overrideFLAGS)=(value/10000);
			break;
		case IDATAOVERRIDEFLWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.override_flags)=(value/10000);
			break;
		case IDATAPFLAGS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].pickup_string_flags=vbound(value/10000, 0, 214748);
			break;
		case IDATAPICKUP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].pickup)=(value/10000);
			break;
		case IDATAPICKUPLITEMLEVEL:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].pickup_litem_level = vbound(value/10000, -1, MAXLEVELS-1);
			break;
		case IDATAPICKUPLITEMS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].pickup_litems = vbound(value/10000, 0, 214748) & LI_ALL;
			break;
		case IDATAPOWER:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].power)=value/10000;
			break;
		case IDATAPSCRIPT:
		{
			auto id = GET_REF(itemdataref);
			if(invalid_item_id(id))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", id);
				break;
			}
			//Need to get collect script ref, not standard idata ref!
			const int32_t new_ref = id != 0 ? -id : COLLECT_SCRIPT_ITEM_ZERO;
			FFScript::deallocateAllScriptOwned(ScriptType::Item, new_ref);

			auto& itm = itemsbuf[id];
			itm.collect_scrconfig.script = vbound(value/10000, 0, NUMSCRIPTITEM);
			itm.collect_scrconfig.inst_init.clear();
			if (itm.collect_scrconfig.script && !script_engine_data_exists(ScriptType::Item, new_ref))
			{
				auto& data = get_script_engine_data(ScriptType::Item, new_ref);
				data.reset();
				data.doscript = 0;
			}
			break;
		}
		case IDATAPSOUND:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].playsound)=vbound(value/10000, 0, MAX_SFX);
			break;
		case IDATAPSTRING:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].pstring=vbound(value/10000, 1, 255);
			break;
		case IDATARUPEECOST:
			item_flag(item_rupee_magic, value);
			break;
		case IDATASCRIPT:
		{
			auto id = GET_REF(itemdataref);
			if(invalid_item_id(id))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", id);
				break;
			}
			FFScript::deallocateAllScriptOwned(ScriptType::Item, id);
			auto& itm = itemsbuf[id];
			itm.scrconfig.script = vbound(value/10000, 0, NUMSCRIPTITEM);
			itm.scrconfig.inst_init.clear();
			if (itm.scrconfig.script && !script_engine_data_exists(ScriptType::Item, id))
			{
				auto& data = get_script_engine_data(ScriptType::Item, id);
				data.reset();
				data.doscript = ((itm.flags & item_passive_script) && game->get_item(id)) ? 1 : 0;
			}
			break;
		}
		case IDATASETMAX:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].setmax)=value/10000;
			break;
		case IDATASPRSCRIPT:
		{
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			auto& itm = itemsbuf[GET_REF(itemdataref)];
			itm.sprite_scrconfig.script=vbound(value/10000,0,NUMSCRIPTSITEMSPRITE);
			itm.sprite_scrconfig.inst_init.clear();
			break;
		}
		case IDATASSWIMDISABLED:
			item_flag(item_sideswim_disabled, value);
			break;
		case IDATATILE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			itemsbuf[GET_REF(itemdataref)].tile=vbound(value/10000, 0, NEWMAXTILES-1);
			break;
		case IDATATILEH:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].tileh)=(value/10000);
			break;
		case IDATATILEHWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.tileh)=(value/10000);
			break;
		case IDATATILEW:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].tilew)=(value/10000);
			break;
		case IDATATILEWWEAP:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.tilew)=(value/10000);
			break;
		case IDATATYPE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].type)=vbound(value/10000,0, 254);
			flushItemCache();
			break;
		case IDATAUSEBURNSPR:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
			}
			else SETFLAG(itemsbuf[GET_REF(itemdataref)].weap_data.wflags, WFLAG_UPDATE_IGNITE_SPRITE, value);
			break;
		case IDATAUSEDEF:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.default_defense)=vbound(value/10000, 0, 255);
			break;
		case IDATAUSESOUND:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].usesound)=vbound(value/10000, 0, MAX_SFX);
			break;
		case IDATAUSESOUND2:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].usesound2)=vbound(value/10000, 0, MAX_SFX);
			break;
		case IDATAUSEWPN:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.imitate_weapon)=vbound(value/10000, 0, 255);
			break;
		case IDATAVALIDATE:
			item_flag(item_validate_only, value);
			break;
		case IDATAVALIDATE2:
			item_flag(item_validate_only_2, value);
			break;
		case IDATAWEAPHXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.hxofs)=(value/10000);
			break;
		case IDATAWEAPHXSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.hxsz)=(value/10000);
			break;
		case IDATAWEAPHYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.hyofs)=(value/10000);
			break;
		case IDATAWEAPHYSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.hysz)=(value/10000);
			break;
		case IDATAWEAPHZSZ:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.hzsz)=(value/10000);
			break;
		case IDATAWEAPONSCRIPT:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.scrconfig.script)=vbound(value/10000, 0, 255);
			break;
		case IDATAWEAPXOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.xofs)=(value/10000);
			break;
		case IDATAWEAPYOFS:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weap_data.yofs)=(value/10000);
			break;
		case IDATAWEAPZOFS:
			scripting_log_error_with_context("itemdata::WeaponDrawZOffset is not implemented");
			break;
		case IDATAWRANGE:
			if(invalid_item_id(GET_REF(itemdataref)))
			{
				scripting_log_error_with_context("Invalid itemdata access: {}", GET_REF(itemdataref));
				break;
			}
			(itemsbuf[GET_REF(itemdataref)].weaprange)=vbound(value/10000, 0, 255);
			break;
		case IDATA_WEAPONDATA:
			break;

		default:
			NOTREACHED();
	}
}

// itemdata arrays.

static ArrayRegistrar IDATAINITDD_registrar(IDATAINITDD, []{
	static ScriptingArray_ObjectSubMemberContainer<itemdata, &itemdata::scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.setSideEffect([](auto* idata, auto*, auto*, int index, int val){
		if (get_qr(qr_SCRIPTS_SHARE_INITD))
		{
			idata->collect_scrconfig.run_args[index] = val;
			idata->sprite_scrconfig.run_args[index] = val;
		}
	});
	return &impl;
}());

static ArrayRegistrar IDATACOLLECTINITD_registrar(IDATACOLLECTINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<itemdata, &itemdata::collect_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar IDATASPRITEINITD_registrar(IDATASPRITEINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<itemdata, &itemdata::sprite_scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar IDATAMISCD_registrar(IDATAMISCD, []{
	static ScriptingArray_ObjectMemberCArray<itemdata, &itemdata::wpn_misc_d> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar IDATAMOVEFLAGS_registrar(IDATAMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<itemdata, &itemdata::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar IDATAWMOVEFLAGS_registrar(IDATAWMOVEFLAGS, []{
	static ScriptingArray_ObjectSubMemberBitwiseFlags<itemdata, &itemdata::weap_data, &weapon_data::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar IDATAWPNINITD_registrar(IDATAWPNINITD, []{
	static ScriptingArray_ObjectSubSubMemberContainer<itemdata, &itemdata::weap_data, &weapon_data::scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar IDATABURNINGSPR_registrar(IDATABURNINGSPR, []{
	static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::burnsprs> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar IDATABURNINGLIGHTRAD_registrar(IDATABURNINGLIGHTRAD, []{
	static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::light_rads> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar IDATAATTRIB_registrar(IDATAATTRIB, []{
	static ScriptingArray_ObjectComputed<itemdata, int> impl(
		[](itemdata*){
			return 10;
		},
		[](itemdata* item, int index) -> int {
			switch(index)
			{
				case 0: return (item->misc1);
				case 1: return (item->misc2);
				case 2: return (item->misc3);
				case 3: return (item->misc4);
				case 4: return (item->misc5);
				case 5: return (item->misc6);
				case 6: return (item->misc7);
				case 7: return (item->misc8);
				case 8: return (item->misc9);
				case 9: return (item->misc10);
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, int value){
			switch(index)
			{
				case 0: item->misc1 = value; break;
				case 1: item->misc2 = value; break;
				case 2: item->misc3 = value; break;
				case 3: item->misc4 = value; break;
				case 4: item->misc5 = value; break;
				case 5: item->misc6 = value; break;
				case 6: item->misc7 = value; break;
				case 7: item->misc8 = value; break;
				case 8: item->misc9 = value; break;
				case 9: item->misc10 = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar IDATAATTRIB_L_registrar(IDATAATTRIB_L, []{
	static ScriptingArray_ObjectComputed<itemdata, int> impl(
		[](itemdata*){
			return 10;
		},
		[](itemdata* item, int index) -> int {
			switch(index)
			{
				case 0: return (item->misc1);
				case 1: return (item->misc2);
				case 2: return (item->misc3);
				case 3: return (item->misc4);
				case 4: return (item->misc5);
				case 5: return (item->misc6);
				case 6: return (item->misc7);
				case 7: return (item->misc8);
				case 8: return (item->misc9);
				case 9: return (item->misc10);
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, int value){
			switch(index)
			{
				case 0: item->misc1 = value; break;
				case 1: item->misc2 = value; break;
				case 2: item->misc3 = value; break;
				case 3: item->misc4 = value; break;
				case 4: item->misc5 = value; break;
				case 5: item->misc6 = value; break;
				case 6: item->misc7 = value; break;
				case 7: item->misc8 = value; break;
				case 8: item->misc9 = value; break;
				case 9: item->misc10 = value; break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-1);
	impl.setMul10000(false);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar IDATASPRITE_registrar(IDATASPRITE, []{
	static ScriptingArray_ObjectMemberCArray<itemdata, &itemdata::wpn_sprites> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar IDATAFLAGS_registrar(IDATAFLAGS, []{
	static ScriptingArray_ObjectComputed<itemdata, bool> impl(
		[](itemdata*){
			return 16;
		},
		[](itemdata* item, int index) -> bool {
			switch(index)
			{
				case 0: return item->flags & item_flag1;
				case 1: return item->flags & item_flag2;
				case 2: return item->flags & item_flag3;
				case 3: return item->flags & item_flag4;
				case 4: return item->flags & item_flag5;
				case 5: return item->flags & item_flag6;
				case 6: return item->flags & item_flag7;
				case 7: return item->flags & item_flag8;
				case 8: return item->flags & item_flag9;
				case 9: return item->flags & item_flag10;
				case 10: return item->flags & item_flag11;
				case 11: return item->flags & item_flag12;
				case 12: return item->flags & item_flag13;
				case 13: return item->flags & item_flag14;
				case 14: return item->flags & item_flag15;
				case 15: return item->flags & item_passive_script;
				default: NOTREACHED();
			}
		},
		[](itemdata* item, int index, bool value){
			switch(index)
			{
				case 0:
					SETFLAG(item->flags, item_flag1, value);
					break;
				case 1:
					SETFLAG(item->flags, item_flag2, value);
					break;
				case 2:
					SETFLAG(item->flags, item_flag3, value);
					break;
				case 3:
					SETFLAG(item->flags, item_flag4, value);
					break;
				case 4:
					SETFLAG(item->flags, item_flag5, value);
					break;
				case 5:
					SETFLAG(item->flags, item_flag6, value);
					break;
				case 6:
					SETFLAG(item->flags, item_flag7, value);
					break;
				case 7:
					SETFLAG(item->flags, item_flag8, value);
					break;
				case 8:
					SETFLAG(item->flags, item_flag9, value);
					cache_tile_mod_clear();
					break;
				case 9:
					SETFLAG(item->flags, item_flag10, value);
					break;
				case 10:
					SETFLAG(item->flags, item_flag11, value);
					break;
				case 11:
					SETFLAG(item->flags, item_flag12, value);
					break;
				case 12:
					SETFLAG(item->flags, item_flag13, value);
					break;
				case 13:
					SETFLAG(item->flags, item_flag14, value);
					break;
				case 14:
					SETFLAG(item->flags, item_flag15, value);
					break;
				case 15:
					SETFLAG(item->flags, item_passive_script, value);
					break;
				default: NOTREACHED();
			}
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
