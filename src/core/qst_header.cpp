#include "base/ints.h"
#include "base/md5.h"
#include "base/version.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "dialog/externs.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern bool loadquest_report;
extern char const* loading_qst_name;
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];
extern bitstring midi_bitstr;
extern bool read_ext_zinfo, read_zinfo;

int32_t readheader(PACKFILE *f, zquestheader *Header, byte printmetadata)
{
	int32_t dummy;
	zquestheader tempheader{};
	tempheader.filename = Header->filename;
	char dummybuf[80];
	byte temp_map_count;
	byte temp_midi_flags[32] = {0};
	word version;
	char temp_pwd[30], temp_pwd2[30];
	int16_t temp_pwdkey;
	cvs_MD5Context ctx;
	memset(FFCore.quest_format, 0, sizeof(FFCore.quest_format));
	

	
	if(!pfread(tempheader.id_str,sizeof(tempheader.id_str),f))      // first read old header
	{
		Z_message("Unable to read header string\n");
		return qe_invalid;
	}
	
	// check header
	if(strcmp(tempheader.id_str,QH_NEWIDSTR))
	{
		if(strcmp(tempheader.id_str,QH_IDSTR))
		{
			Z_message("Invalid header string:  '%s' (was expecting '%s' or '%s')\n", tempheader.id_str, QH_IDSTR, QH_NEWIDSTR);
			return qe_invalid;
		}
	}
	
	int32_t templatepath_len=0;
	
	tempheader.external_zinfo = false;
	read_zinfo = false;
	if(!strcmp(tempheader.id_str,QH_IDSTR))                      //pre-1.93 version
	{
		byte padding;
		
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&tempheader.zelda_version,f))
		{
			return qe_invalid;
		}
		
		FFCore.quest_format[vZelda] = tempheader.zelda_version;
		
		if(tempheader.zelda_version > ZELDA_VERSION)
		{
			return qe_version;
		}
		
		FFCore.quest_format[vZelda] = tempheader.zelda_version;
		
		if(strcmp(tempheader.id_str,QH_IDSTR))
		{
			return qe_invalid;
		}
		
		if(bad_version(tempheader.zelda_version))
		{
			return qe_obsolete;
		}
		
		if(!p_igetw(&tempheader.internal,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempheader.quest_number,f))
		{
			return qe_invalid;
		}
		
		FFCore.quest_format[qQuestNumber] = tempheader.quest_number;
		
		if(!pfread(&quest_rules[0],2,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_map_count,f))
		{
			return qe_invalid;
		}
		
		FFCore.quest_format[qMapCount] = temp_map_count;
		
		if(!p_getc(&tempheader.old_str_count,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempheader.data_flags[ZQ_TILES],f))
		{
			return qe_invalid;
		}
		
		if(!pfread(temp_midi_flags,4,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempheader.data_flags[ZQ_CHEATS2],f))
		{
			return qe_invalid;
		}
		
		if(!pfread(dummybuf,14,f))
		{
			return qe_invalid;
		}
		
		if(!pfread(&quest_rules[2],2,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&dummybuf,f))
		{
			return qe_invalid;
		}
		
		if(!pfread(tempheader.version,9,f))
		{
			return qe_invalid;
		}
		
		if(!pfread(tempheader.title,sizeof(tempheader.title),f))
		{
			return qe_invalid;
		}
		// These fields are expected to end in null bytes!
		tempheader.title[sizeof(tempheader.title)-1] = 0;
		
		if(!pfread(tempheader.author,sizeof(tempheader.author),f))
		{
			return qe_invalid;
		}
		tempheader.author[sizeof(tempheader.author)-1] = 0;
		
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_pwdkey,f))
		{
			return qe_invalid;
		}
		
		if(!pfread(temp_pwd,30,f))
		{
			return qe_invalid;
		}
	
		get_questpwd(temp_pwd, temp_pwdkey, temp_pwd2);
		cvs_MD5Init(&ctx);
		cvs_MD5Update(&ctx, (const uint8_t*)temp_pwd2, (unsigned)strnlen(temp_pwd2, 30));
		cvs_MD5Final(tempheader.pwd_hash, &ctx);
		
		if(tempheader.zelda_version < 0x177)                       // lacks new header stuff...
		{
			//memset(tempheader.minver,0,20);                          //   char minver[9], byte build, byte foo[10]
			// Not anymore...
			memset(tempheader.minver,0,17);
			tempheader.build=0;
			tempheader.use_keyfile=0;
			memset(tempheader.old_foo, 0, 9);
		}
		else
		{
			if(!pfread(tempheader.minver,9,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempheader.build,f))
			{
				return qe_invalid;
			}
			
			FFCore.quest_format[vBuild] = tempheader.build;
			
			if(!p_getc(&tempheader.use_keyfile,f))
			{
				return qe_invalid;
			}
			
			if(!pfread(dummybuf,9,f))
			{
				return qe_invalid;
			}
		}                                                       // starting at minver
		
		if(tempheader.zelda_version < 0x187)                    // lacks newer header stuff...
		{
			memset(&quest_rules[4],0,16);                          //   word rules3..rules10
		}
		else
		{
			if(!pfread(&quest_rules[4],16,f))                      // read new header additions
			{
				return qe_invalid;                                  // starting at rules3
			}
			
			if(tempheader.zelda_version <= 0x190)
			{
				set_qr(qr_MEANPLACEDTRAPS,0);
			}
		}
		unpack_qrs();
		
		if((tempheader.zelda_version < 0x192)||
				((tempheader.zelda_version == 0x192)&&(tempheader.build<149)))
		{
			set_qr(qr_BRKNSHLDTILES,(get_qr(qr_BRKBLSHLDS_DEP)));
			set_bit(deprecated_rules,qr_BRKBLSHLDS_DEP,(get_qr(qr_BRKBLSHLDS_DEP)));
			set_qr(qr_BRKBLSHLDS_DEP,0);
		}
		
		if(tempheader.zelda_version >= 0x192)                       //  lacks newer header stuff...
		{
			byte *mf=temp_midi_flags;
			
			if((tempheader.zelda_version == 0x192)&&(tempheader.build<178))
			{
				mf=(byte*)dummybuf;
			}
			
			if(!pfread(mf,32,f))                  // read new header additions
			{
				return qe_invalid;                                  // starting at foo2
			}
			
			if(!pfread(dummybuf,18,f))                        // read new header additions
			{
				return qe_invalid;                                  // starting at foo2
			}
		}
		
		if((tempheader.zelda_version < 0x192)||
				((tempheader.zelda_version == 0x192)&&(tempheader.build<145)))
		{
			memset(tempheader.templatepath,0,2048);
		}
		else
		{
			if(!pfread(tempheader.templatepath,280,f))               // read templatepath
			{
				return qe_invalid;
			}
		}
		
		if((tempheader.zelda_version < 0x192)||
				((tempheader.zelda_version == 0x192)&&(tempheader.build<186)))
		{
			tempheader.use_keyfile=0;
		}
	}
	else
	{
		//section id
		if(!p_mgetl(&dummy,f))
		{
			return qe_invalid;
		}
		
		//section version info
		if(!p_igetw(&version,f))
		{
			return qe_invalid;
		}

		if (version > V_HEADER)
			return qe_version;
		
		FFCore.quest_format[vHeader] = version;
		
		if(!p_igetw(&dummy,f))
		{
			return qe_invalid;
		}
		
		//section size
		if(!p_igetl(&dummy,f))
		{
			return qe_invalid;
		}
		
		//finally...  section data
		if(!p_igetw(&tempheader.zelda_version,f))
		{
			return qe_invalid;
		}
		
		FFCore.quest_format[vZelda] = tempheader.zelda_version;
	
		//do some quick checking...
		if(tempheader.zelda_version > ZELDA_VERSION)
		{
			return qe_version;
		}
		
		if(strcmp(tempheader.id_str,QH_NEWIDSTR))
		{
			return qe_invalid;
		}
		
		if(bad_version(tempheader.zelda_version))
		{
			return qe_obsolete;
		}
		
		if(!p_getc(&tempheader.build,f))
		{
			return qe_invalid;
		}
	
		FFCore.quest_format[vBuild] = tempheader.build;
	
		if(version<3)
		{
			if(!pfread(temp_pwd,30,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_pwdkey,f))
			{
				return qe_invalid;
			}
			
			get_questpwd(temp_pwd, temp_pwdkey, temp_pwd2);
			cvs_MD5Init(&ctx);
			cvs_MD5Update(&ctx, (const uint8_t*)temp_pwd2, (unsigned)strnlen(temp_pwd2, 30));
			cvs_MD5Final(tempheader.pwd_hash, &ctx);
		}
		else
		{
			if(!pfread(tempheader.pwd_hash,sizeof(tempheader.pwd_hash),f))
			{
				return qe_invalid;
			}
		}
		
		if(!p_igetw(&tempheader.internal,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempheader.quest_number,f))
		{
			return qe_invalid;
		}
		
		FFCore.quest_format[qQuestNumber] = tempheader.quest_number;
		
		size_t versz = version < 8 ? 9 : 16;
		if(!pfread(tempheader.version,versz,f))
		{
			return qe_invalid;
		}
	
		//FFCore.quest_format[qQuestVersion] = tempheader.version;
		//needs to be copied as char[9] or stored as a s.str
		if(!pfread(tempheader.minver,versz,f))
		{
			return qe_invalid;
		}
	
		//FFCore.quest_format[qMinQuestVersion] = tempheader.minver;
		if(!pfread(tempheader.title,sizeof(tempheader.title),f))
		{
			return qe_invalid;
		}
		tempheader.title[sizeof(tempheader.title)-1] = 0;
		
		if(!pfread(tempheader.author,sizeof(tempheader.author),f))
		{
			return qe_invalid;
		}
		tempheader.author[sizeof(tempheader.author)-1] = 0;
		
		if(!p_getc(&tempheader.use_keyfile,f))
		{
			return qe_invalid;
		}
		
		/*
		  if(!pfread(tempheader.data_flags,sizeof(tempheader.data_flags),f))
		  {
		  return qe_invalid;
		  }
		  */
		if(!p_getc(&tempheader.data_flags[ZQ_TILES],f))
		{
			return qe_invalid;
		}
		
		if(!pfread(&dummybuf,4,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempheader.data_flags[ZQ_CHEATS2],f))
		{
			return qe_invalid;
		}
		
		if(!pfread(dummybuf,14,f))
		{
			return qe_invalid;
		}
		
		templatepath_len=sizeof(tempheader.templatepath);
		
		if(version==1)
		{
			templatepath_len=280;
		}
		
		if(!pfread(tempheader.templatepath,templatepath_len,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_map_count,f))
		{
			return qe_invalid;
		}
	
		if(version>=4)
		{
			if(!p_igetl(&tempheader.version_major,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.version_minor,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.version_patch,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.new_version_id_fourth,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.new_version_id_alpha,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.new_version_id_beta,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.new_version_id_gamma,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.new_version_id_release,f))
			{
				return qe_invalid;
			}

			// 2.55 alpha 86 incorrectly used 56 as the second version component. Fix that here.
			if (tempheader.version_major == 2 && tempheader.version_minor == 56 && tempheader.new_version_id_alpha == 86)
				tempheader.version_minor = 55;

			if(!p_igetw(&tempheader.new_version_id_date_year,f))
			{
				return qe_invalid;
			}
			if(!p_getc(&tempheader.new_version_id_date_month,f))
			{
				return qe_invalid;
			}
			if(!p_getc(&tempheader.new_version_id_date_day,f))
			{
				return qe_invalid;
			}
			if(!p_getc(&tempheader.new_version_id_date_hour,f))
			{
				return qe_invalid;
			}
			if(!p_getc(&tempheader.new_version_id_date_minute,f))
			{
				return qe_invalid;
			}
					
			if(!pfread(tempheader.new_version_devsig,256,f))
			{
				return qe_invalid;
			}
			if(!strcmp(tempheader.new_version_devsig, "Venrob"))
				strcpy(tempheader.new_version_devsig, "EmilyV99");
			if(!pfread(tempheader.new_version_compilername,256,f))
			{
				return qe_invalid;
			}
			if(!pfread(tempheader.new_version_compilerversion,256,f))
			{
				return qe_invalid;
			}
			if(!pfread(tempheader.product_name,1024,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempheader.compilerid,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.compilerversionnumber_first,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.compilerversionnumber_second,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.compilerversionnumber_third,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempheader.compilerversionnumber_fourth,f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&tempheader.developerid,f))
			{
				return qe_invalid;
			}
			if(!pfread(tempheader.made_in_module_name,1024,f))
			{
				return qe_invalid;
			}
			if(!pfread(tempheader.build_datestamp,256,f))
			{
				return qe_invalid;
			}
			if(!pfread(tempheader.build_timestamp,256,f))
			{
				return qe_invalid;
			}
		}
		else // <4
		{
			tempheader.version_major = 0;
			tempheader.version_minor = 0;
			tempheader.version_patch = 0;
			tempheader.new_version_id_fourth = 0;
			tempheader.new_version_id_alpha = 0;
			tempheader.new_version_id_beta = 0;
			tempheader.new_version_id_gamma = 0;
			tempheader.new_version_id_release = 0;
			tempheader.new_version_id_date_year = 0;
			tempheader.new_version_id_date_month = 0;
			tempheader.new_version_id_date_day = 0;
			tempheader.new_version_id_date_hour = 0;
			tempheader.new_version_id_date_minute = 0;
			
			memset(tempheader.new_version_devsig, 0, 256);
			memset(tempheader.new_version_compilername, 0, 256);
			memset(tempheader.new_version_compilerversion, 0, 256);
			memset(tempheader.product_name, 0, 1024);
			strcpy(tempheader.product_name, "ZQuest Classic");
			
			tempheader.compilerid = 0;
			tempheader.compilerversionnumber_first = 0;
			tempheader.compilerversionnumber_second = 0;
			tempheader.compilerversionnumber_third = 0;
			tempheader.compilerversionnumber_fourth = 0;
			tempheader.developerid = 0;
			
			memset(tempheader.made_in_module_name, 0, 1024);
			memset(tempheader.build_datestamp, 0, 256);
			memset(tempheader.build_timestamp, 0, 256);
		}

		if ( version >= 5 )
		{
			if(!pfread(tempheader.build_timezone,6,f))
			{
				return qe_invalid;
			}
		}
		else // < 5
		{
			memset(tempheader.build_timezone, 0, 6);
		}
		if ( version >= 6 )
		{
			byte b;
			if(!p_getc(&b,f))
			{
				return qe_invalid;
			}
			tempheader.external_zinfo = b?true:false;
			read_zinfo = true;
		}
		
		if(version >= 7)
		{
			if(!p_getc(&(tempheader.new_version_is_nightly),f))
			{
				return qe_invalid;
			}
		}
		else
		{
			tempheader.new_version_is_nightly = false;
			if(tempheader.zelda_version < 0x255)
			{
				switch(tempheader.zelda_version)
				{
					case 0x254:
						tempheader.version_major = 2;
						tempheader.version_minor = 54;
						break;
					case 0x250:
						switch(tempheader.build)
						{
							case 19:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_gamma = 1;
								break;
							case 20:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_gamma = 2;
								break;
							case 21:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_gamma = 3;
								break;
							case 22:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_gamma = 4;
								break;
							case 23:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_gamma = 5;
								break;
							case 24:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.new_version_id_release = -1;
								break;
							case 25:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 1;
								tempheader.new_version_id_gamma = 1;
								break;
							case 26:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 1;
								tempheader.new_version_id_gamma = 2;
								break;
							case 27: 
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 1;
								tempheader.new_version_id_gamma = 3;
								break;
							case 28:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 1;
								tempheader.new_version_id_release = -1;
								break;
							case 29:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 2;
								tempheader.new_version_id_release = -1;
								break;
							case 30:
								tempheader.version_major = 2;
								tempheader.version_minor = 50;
								tempheader.version_patch = 3;
								tempheader.new_version_id_gamma = 1;
								break;
							case 31:
								tempheader.version_major = 2;
								tempheader.version_minor = 53;
								tempheader.new_version_id_gamma = -1;
								break;
							case 32:
								tempheader.version_major = 2;
								tempheader.version_minor = 53;
								tempheader.new_version_id_release = -1;
								break;
							case 33:
								tempheader.version_major = 2;
								tempheader.version_minor = 53;
								tempheader.version_patch = 1;
								break;
						}
						break;
					
					case 0x211:
						tempheader.version_major = 2;
						tempheader.version_minor = 11;
						tempheader.new_version_id_beta = tempheader.build;
						break;
					case 0x210:
						tempheader.version_major = 2;
						tempheader.version_minor = 10;
						tempheader.new_version_id_beta = tempheader.build;
						break;
				}
			}
		}

		if (version>=9)
		{
			std::string version_string;
			if(!p_getcstr(&version_string, f))
			{
				return qe_invalid;
			}

			strncpy(tempheader.zelda_version_string, version_string.c_str(), sizeof(tempheader.zelda_version_string));
			snprintf(tempheader.zelda_version_string, sizeof(tempheader.zelda_version_string), "%s", version_string.c_str());
		}
		else
		{
			snprintf(tempheader.zelda_version_string, sizeof(tempheader.zelda_version_string), "%d.%d.%d", tempheader.version_major, tempheader.version_minor, tempheader.version_patch);
		}
	}

	if(printmetadata)
	{
		print_quest_metadata(tempheader, loading_qst_name);
	}
	
	//{ Version Warning
	int32_t vercmp = tempheader.compareVer();
	int32_t astatecmp = compare(int32_t(tempheader.getAlphaState()), getAlphaState());
	int32_t avercmp = compare(tempheader.getAlphaVer(), 0);
	if(vercmp > 0 || (!vercmp &&
		(astatecmp > 0 || (!astatecmp &&
			avercmp > 0))))
	{
		bool r = true;
		if(loadquest_report)
		{
			enter_sys_pal();
			r = alert_confirm("Quest saved in newer version",
				"This quest was last saved in a newer version of ZQuest."
				" Attempting to load this quest may not work correctly; to"
				" avoid issues, try loading this quest in at least '" + std::string(tempheader.getVerStr()) + "'"
				"\n\nWould you like to continue loading anyway? (Not recommended)");
			exit_sys_pal();
		}
		if(!r)
			return qe_silenterr;
	}
	//}
	
	read_ext_zinfo = tempheader.external_zinfo;
	
	*Header = tempheader;
	map_count=temp_map_count;
	vector<byte> vb{ temp_midi_flags, temp_midi_flags + 32 };
	midi_bitstr.inner() = vb;

	unpack_qrs();

	return 0;
}
