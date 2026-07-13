#include "zc/scripting/types/messagedata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t do_msgwidth(int32_t ID)
{
	if(BC::checkMessage(ID) != SH::_NoError)
	{
		return -1;
	}
	
	int32_t v = text_length(get_zc_font(MsgStrings[ID].font),
		MsgStrings[ID].s.substr(0,MsgStrings[ID].s.find_last_not_of(' ')+1).c_str());
	return v;
}

int32_t do_msgheight(int32_t ID)
{
	if(BC::checkMessage(ID) != SH::_NoError)
	{
		return -1;
	}
	return text_height(get_zc_font(MsgStrings[ID].font));
}

int32_t messagedata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case MESSAGEDATACSET: //b
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].cset) * 10000;
			break;
		}	
		case MESSAGEDATAFLAGS: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].stringflags) * 10000;
			break;
		}
		case MESSAGEDATAFONT: //B
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = (int32_t)MsgStrings[ID].font * 10000;
			break;
		}	
		case MESSAGEDATAH: //UNSIGNED SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].h) * 10000;
			break;
		}	
		case MESSAGEDATAHSPACE: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].hspace) * 10000;
			break;
		}	
		case MESSAGEDATALISTPOS: //WORD
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].listpos) * 10000;
			break;
		}	
		case MESSAGEDATANEXT: //W
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
			{
				ret = -10000; break;
			}
			else
			{
				ret = ((int32_t)MsgStrings[ID].nextstring) * 10000;
				break;
			}
		}	
		case MESSAGEDATAPORTCSET: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.cset) * 10000;
			break;
		}
		case MESSAGEDATAPORTHEI: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.th) * 10000;
			break;
		}
		case MESSAGEDATAPORTTILE: //INT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.tile) * 10000;
			break;
		}
		case MESSAGEDATAPORTWID: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.tw) * 10000;
			break;
		}
		case MESSAGEDATAPORTX: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.x) * 10000;
			break;
		}
		case MESSAGEDATAPORTY: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].portrait.y) * 10000;
			break;
		}
		case MESSAGEDATASFX: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].sfx) * 10000;
			break;
		}	
		case MESSAGEDATATEXTHEI:
		{
			ret = do_msgheight(GET_REF(msgdataref))*10000;
			break;
		}
		case MESSAGEDATATEXTLEN: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = int32_t(MsgStrings[ID].s.size()) * 10000;
			break;
		}
		case MESSAGEDATATEXTWID:
		{
			ret = do_msgwidth(GET_REF(msgdataref))*10000;
			break;
		}
		case MESSAGEDATATILE: //W
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].tile) * 10000;
			break;
		}	
		case MESSAGEDATATRANS: //BOOL
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((MsgStrings[ID].trans)?10000:0);
			break;
		}	
		case MESSAGEDATAVSPACE: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].vspace) * 10000;
			break;
		}	
		case MESSAGEDATAW: //UNSIGNED SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].w) * 10000;
			break;
		}	
		case MESSAGEDATAX: //SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].x) * 10000;
			break;
		}	
		case MESSAGEDATAY: //SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = ((int32_t)MsgStrings[ID].y) * 10000;
			break;
		}
		case MESSAGEDATA_SHADOW_TYPE:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * MsgStrings[ID].shadow_type;
			break;
		}
		case MESSAGEDATA_SHADOW_COLOR:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * MsgStrings[ID].shadow_color;
			break;
		}
		case MESSAGEDATA_ICON_MORE_SPRITE:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * MsgStrings[ID].icon_more.sprite;
			break;
		}
		case MESSAGEDATA_ICON_MORE_ANCHOR:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * int(MsgStrings[ID].icon_more.anchor);
			break;
		}
		case MESSAGEDATA_ICON_MORE_X:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * MsgStrings[ID].icon_more.x;
			break;
		}
		case MESSAGEDATA_ICON_MORE_Y:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				ret = -10000;
			else
				ret = 10000 * MsgStrings[ID].icon_more.y;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void messagedata_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case MESSAGEDATACSET: //b
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].cset = ((byte)vbound((value/10000), 0, 15));
			break;
		}	
		case MESSAGEDATAFLAGS: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].stringflags = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATAFONT: //B
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].font = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATAH: //UNSIGNED SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].h = ((uint16_t)vbound((value/10000), 0, USHRT_MAX));
			break;
		}	
		case MESSAGEDATAHSPACE: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].hspace = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATALISTPOS: //WORD
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].listpos = vbound((value/10000), 1, (msg_count-1));
			break;
		}	
		case MESSAGEDATANEXT: //W
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].nextstring = vbound((value/10000), 0, (msg_count-1));
			break;
		}	
		case MESSAGEDATAPORTCSET: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.cset = ((byte)vbound((value/10000), 0, 15));
			break;
		}
		case MESSAGEDATAPORTHEI: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.th = ((byte)vbound((value/10000), 0, 14));
			break;
		}
		case MESSAGEDATAPORTTILE: //INT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.tile = vbound((value/10000), 0, (NEWMAXTILES));
			break;
		}
		case MESSAGEDATAPORTWID: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.tw = ((byte)vbound((value/10000), 0, 16));
			break;
		}
		case MESSAGEDATAPORTX: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.x = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATAPORTY: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].portrait.y = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATASFX: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].sfx = ((word)vbound((value/10000), 0, MAX_SFX));
			break;
		}	
		case MESSAGEDATATILE: //W
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].tile = vbound((value/10000), 0, (NEWMAXTILES));
			break;
		}	
		case MESSAGEDATATRANS: //BOOL
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				(MsgStrings[ID].trans) = ((value)?true:false);
			break;
		}	
		case MESSAGEDATAVSPACE: //BYTE
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].vspace = ((byte)vbound((value/10000), 0, 255));
			break;
		}	
		case MESSAGEDATAW: //UNSIGNED SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].w = ((uint16_t)vbound((value/10000), 0, USHRT_MAX));
			break;
		}	
		case MESSAGEDATAX: //SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].x = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}	
		case MESSAGEDATAY: //SHORT
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].y = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}
		case MESSAGEDATA_SHADOW_TYPE:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].shadow_type = ((byte)vbound((value/10000), 0, sstsMAX-1));
			break;
		}
		case MESSAGEDATA_SHADOW_COLOR:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].shadow_color = ((byte)vbound((value/10000), 0, 255));
			break;
		}
		case MESSAGEDATA_ICON_MORE_SPRITE:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].icon_more.sprite = ((word)vbound((value/10000), 0, MAXSPRITES-1));
			break;
		}
		case MESSAGEDATA_ICON_MORE_ANCHOR:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].icon_more.anchor = ((message_anchor)vbound((value/10000), 0, int(message_anchor::max_anchor)-1));
			break;
		}
		case MESSAGEDATA_ICON_MORE_X:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].icon_more.x = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}
		case MESSAGEDATA_ICON_MORE_Y:
		{
			int32_t ID = GET_REF(msgdataref);

			if(BC::checkMessage(ID) != SH::_NoError)
				break;
			else
				MsgStrings[ID].icon_more.y = ((int16_t)vbound((value/10000), SHRT_MIN, SHRT_MAX));
			break;
		}
		default:
			NOTREACHED();
	}
}

// messagedata arrays.

static ArrayRegistrar MESSAGEDATAMARGINS_registrar(MESSAGEDATAMARGINS, []{
	static ScriptingArray_ObjectMemberCArray<MsgStr, &MsgStr::margins> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar MESSAGEDATAFLAGSARR_registrar(MESSAGEDATAFLAGSARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<MsgStr, &MsgStr::stringflags, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MESSAGEDATASEGMENTS_registrar(MESSAGEDATASEGMENTS, []{
	static ScriptingArray_ObjectComputed<MsgStr, int32_t> impl(
		[](MsgStr* msg){ return msg->segmentsAsZFixArray().size(); },
		[](MsgStr* msg, int index){ return msg->segmentsAsZFixArray()[index].getZLong(); },
		[](MsgStr*, int, int){}
	);
	impl.setMul10000(false);
	impl.setReadOnly();
	return &impl;
}());
