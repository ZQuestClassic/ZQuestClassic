#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

void init_msgstr(MsgStr *str)
{
	str->setFromLegacyEncoding("");
	str->nextstring=0;
    str->tile=0;
    str->cset=0;
    str->trans=false;
    str->font=font_zfont;
    str->y=32;
    str->sfx=18;
    str->listpos=0;
    str->x=24;
	str->w=get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 24*8 : 26*8;
	str->h=get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 3*8 : 5*8;
    str->hspace=0;
    str->vspace=0;
    str->stringflags=0;
	str->margins[up] = 8;
	str->margins[down] = 0;
	str->margins[left] = 8;
	str->margins[right] = 8;
	str->portrait.clear();
	str->shadow_type = 0;
	str->shadow_color = 0;
	str->drawlayer = 6;
}

void init_msgstrings(int32_t start, int32_t end)
{
    if(end <= start || end-start > msg_strings_size)
        return;
        
    for(int32_t i=start; i<end; i++)
    {
        init_msgstr(&MsgStrings[i]);
        MsgStrings[i].listpos=i;
    }
    
    if(start==0)
    {
		MsgStrings[0].setFromLegacyEncoding("(None)");
        MsgStrings[0].listpos = 0;
    }
}

int32_t readstrings(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_strings);

	MsgStr tempMsgString;
	init_msgstr(&tempMsgString);
	
	word temp_msg_count=0;
	word temp_expansion[16];
	memset(temp_expansion, 0, 16*sizeof(word));
	char buf[8193] = {0};
	byte tempbyte;
	if(Header->zelda_version < 0x193)
	{
		int32_t strings_to_read=0;
		if (!should_skip)
			set_qr(qr_OLD_STRING_EDITOR_MARGINS,true);
		if((Header->zelda_version < 0x192)||
			((Header->zelda_version == 0x192)&&(Header->build<31)))
		{
			strings_to_read=128;
			temp_msg_count=Header->old_str_count;
			
			// Some sort of string count corruption seems to be common in old quests
			if(temp_msg_count>128)
				temp_msg_count=128;
		}
		else if((Header->zelda_version == 0x192)&&(Header->build<140))
		{
			strings_to_read=255;
			temp_msg_count=Header->old_str_count;
		}
		else
		{
			if(!p_igetw(&temp_msg_count,f))
				return qe_invalid;
			
			strings_to_read=temp_msg_count;
			
			if (!should_skip && temp_msg_count >= msg_strings_size)
			{
				Z_message("Reallocating string buffer...\n");
				
				// if((MsgStrings=(MsgStr*)_al_sane_realloc(MsgStrings,sizeof(MsgStr)*MAXMSGS))==NULL)
					// return qe_nomem;
					
				//memset(MsgStrings, 0, sizeof(MsgStr)*MAXMSGS);
				delete[] MsgStrings;
				MsgStrings = new MsgStr[MAXMSGS];
				msg_strings_size = MAXMSGS;
				for(auto q = 0; q < msg_strings_size; ++q)
					MsgStrings[q].clear();
			}
		}
		
		//reset the message strings
		if (!should_skip)
			init_msgstrings(0,msg_strings_size);
		
		for(int32_t x=0; x<strings_to_read; x++)
		{
			init_msgstr(&tempMsgString);
			tempMsgString.listpos = x;
			
			if(!pfread(buf,73,f))
			{
				return qe_invalid;
			}
			
			buf[74] = '\0';
			tempMsgString.setFromLegacyEncoding(buf);
				
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			
			if((Header->zelda_version < 0x192)||
				((Header->zelda_version == 0x192)&&(Header->build<148)))
			{
				tempMsgString.nextstring=tempbyte?x+1:0;
				
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
			}
			else
			{
				if(!p_igetw(&tempMsgString.nextstring,f))
					return qe_invalid;
				
				if(!pfread(temp_expansion,32,f))
					return qe_invalid;
			}
			
			if (!should_skip)
				MsgStrings[x] = tempMsgString;
		}
	}
	else
	{
		int32_t dummy_int;
		word s_version;
		
		//section version info
		if(!p_igetw(&s_version,f))
			return qe_invalid;

		if (s_version > V_STRINGS)
			return qe_version;
	
		FFCore.quest_format[vStrings] = s_version;
		
		if(!read_deprecated_section_cversion(f))
			return qe_invalid;
		
		//section size
		if(!p_igetl(&dummy_int,f))
			return qe_invalid;
		
		//finally...  section data
		if(!p_igetw(&temp_msg_count,f))
			return qe_invalid;
		
		if(temp_msg_count >= msg_strings_size && !should_skip)
		{
			Z_message("Reallocating string buffer...\n");
			
			delete[] MsgStrings;
			MsgStrings = new MsgStr[MAXMSGS];
			msg_strings_size = MAXMSGS;
			for(auto q = 0; q < msg_strings_size; ++q)
				MsgStrings[q].clear();
		}
		
		//reset the message strings
		if(s_version < 7)
			set_qr(qr_OLD_STRING_EDITOR_MARGINS,true);
		if (!should_skip)
			init_msgstrings(0,msg_strings_size);
		
		int32_t string_length=(s_version<2)?73:145;
		
		for(int32_t i=0; i<temp_msg_count; i++)
		{
			init_msgstr(&tempMsgString);
			if (s_version >= 11)
				tempMsgString.encoding_type = MsgStr::EncodingType::Ascii;
			tempMsgString.listpos = i;
			if(s_version > 8)
				if(!p_igetl(&string_length,f))
					return qe_invalid;

			if (string_length < 0 || string_length > 8193)
				return qe_invalid;

			if (string_length > 0)
			{
				if (!pfread(buf, string_length, f))
					return qe_invalid;
			}
			else buf[0] = 0;

			if(!p_igetw(&tempMsgString.nextstring,f))
				return qe_invalid;
			
			if(s_version<2)
			{
				buf[72] = '\0';
				tempMsgString.setFromLegacyEncoding(buf);
			}
			else
			{
				// June 2008: A bug corrupted the last 4 chars of a string.
				// Discard these.
				if (!should_skip)
				{
					if(s_version<3)
						for(int32_t j=140; j<144; j++)
							buf[j] = '\0';
					if(string_length > 8192) string_length = 8192;
					buf[string_length]='\0'; //Force-terminate
					tempMsgString.set(buf, tempMsgString.encoding_type);
				}
				
				if ( s_version >= 6 )
				{
					if(!p_igetl(&tempMsgString.tile,f))
						return qe_invalid;
				}
				else
				{
					if(!p_igetw(&tempMsgString.tile,f))
						return qe_invalid;
				}
				
				if(!p_getc(&tempMsgString.cset,f))
					return qe_invalid;
				
				byte dummy_char;
				
				if(!p_getc(&dummy_char,f)) // trans is stored as a char...
					return qe_invalid;
				
				tempMsgString.trans=dummy_char!=0;
				
				if(!p_getc(&tempMsgString.font,f))
					return qe_invalid;
				
				if(s_version < 5)
				{
					if(!p_getc(&tempMsgString.y,f))
						return qe_invalid;
				}
				else
				{
					if(!p_igetw(&tempMsgString.x,f))
						return qe_invalid;
					
					if(!p_igetw(&tempMsgString.y,f))
						return qe_invalid;
					
					if(!p_igetw(&tempMsgString.w,f))
						return qe_invalid;
					
					if(!p_igetw(&tempMsgString.h,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.hspace,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.vspace,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.stringflags,f))
						return qe_invalid;
				}
				
				if(s_version >= 7)
				{
					for(int32_t q = 0; q < 4; ++q)
					{
						if(!p_getc(&tempMsgString.margins[q],f))
							return qe_invalid;
					}
					
					if(!p_igetl(&tempMsgString.portrait.tile,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.portrait.cset,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.portrait.x,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.portrait.y,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.portrait.tw,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.portrait.th,f))
						return qe_invalid;
				}
				
				if(s_version >= 8)
				{
					if(!p_getc(&tempMsgString.shadow_type,f))
						return qe_invalid;
					
					if(!p_getc(&tempMsgString.shadow_color,f))
						return qe_invalid;
				}
				
				if(s_version >= 10)
				{
					if(!p_getc(&tempMsgString.drawlayer,f))
						return qe_invalid;
				}
				
				if (s_version < 12)
				{
					if (!p_getc(&tempbyte,f))
						return qe_invalid;
					tempMsgString.sfx = tempbyte;
				}
				else
				{
					if(!p_igetw(&tempMsgString.sfx,f))
						return qe_invalid;
				}
				
				if(s_version>3)
					if(!p_igetw(&tempMsgString.listpos,f))
						return qe_invalid;
			}

			if (!should_skip)
				MsgStrings[i].copyAll(tempMsgString);
		}
	}
	
	if (!should_skip)
		msg_count=temp_msg_count;
	
	return 0;
}
