#include "components/zasm/debug_data.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern void setZScriptVersion(int32_t s_version);
extern std::string zScript;

namespace {

std::vector<const script_data*> read_scripts;
script_data fake_script_data(ScriptType::None, 0);

// 3.0+ calls this.
int32_t read_quest_zasm(PACKFILE *f, word s_version)
{
	(void)s_version;
	int32_t num_commands;
	if(!p_igetl(&num_commands,f))
		return qe_invalid;
#ifdef ZC_FUZZ
	const int32_t command_limit = 300000;
#else
	const int32_t command_limit = 25000000;
#endif
	if (num_commands < 0 || num_commands > command_limit)
		return qe_invalid;

	std::vector<ffscript> zasm;
	zasm.reserve(num_commands);
	for(int32_t j=0; j<num_commands; j++)
	{
		ffscript temp_script;
		if(!p_igetw(&(temp_script.command),f))
			return qe_invalid;
		
		if(!p_igetl(&(temp_script.arg1),f))
			return qe_invalid;
		
		if(!p_igetl(&(temp_script.arg2),f))
			return qe_invalid;
		
		if(!p_igetl(&(temp_script.arg3),f))
			return qe_invalid;
		
		uint32_t sz = 0;
		if(!p_igetl(&sz,f))
			return qe_invalid;
		if(sz) //string found
		{
			temp_script.strptr = new std::string();
			char dummy;
			for(size_t q = 0; q < sz; ++q)
			{
				if(!p_getc(&dummy,f))
					return qe_invalid;
				temp_script.strptr->push_back(dummy);
			}
		}
		if(!p_igetl(&sz,f))
			return qe_invalid;
		if(sz) //vector found
		{
			temp_script.vecptr = new std::vector<int32_t>();
			int32_t dummy;
			for(size_t q = 0; q < sz; ++q)
			{
				if(!p_igetl(&dummy,f))
					return qe_invalid;
				temp_script.vecptr->push_back(dummy);
			}
		}
		zasm.emplace_back(std::move(temp_script));
	}

	assert(zasm_scripts.empty());
	zasm_script_id id = zasm_scripts.size();
	zasm_scripts.emplace_back(std::make_shared<zasm_script>(id, "@single", std::move(zasm)));

	return 0;
}

int32_t read_one_zmeta(PACKFILE *f, zasm_meta& temp_meta, word zmeta_version)
{
	char b33[34] = { 0 };
	b33[33] = 0;
	if(!p_igetw(&(temp_meta.zasm_v),f))
		return qe_invalid;
	if(!p_igetw(&(temp_meta.meta_v),f))
		return qe_invalid;
	if(!p_igetw(&(temp_meta.ffscript_v),f))
		return qe_invalid;
	if(!p_getc(&(temp_meta.script_type),f))
		return qe_invalid;
	
	for(int32_t q = 0; q < 8; ++q)
	{
		if(zmeta_version < 3)
		{
			for(int32_t c = 0; c < 33; ++c)
			{
				if(!p_getc(&(b33[c]),f))
				{
					return qe_invalid;
				}
			}
			temp_meta.run_idens[q].assign(b33);
		}
		else
		{
			if(!p_getcstr(&temp_meta.run_idens[q],f))
			{
				return qe_invalid;
			}
		}
	}
	
	for(int32_t q = 0; q < 8; ++q)
		if(!p_getc(&(temp_meta.run_types[q]),f))
			return qe_invalid;
	
	if(!p_getc(&(temp_meta.flags),f))
		return qe_invalid;
	
	if(!p_igetw(&(temp_meta.compiler_v1),f))
		return qe_invalid;
	if(!p_igetw(&(temp_meta.compiler_v2),f))
		return qe_invalid;
	if(!p_igetw(&(temp_meta.compiler_v3),f))
		return qe_invalid;
	if(!p_igetw(&(temp_meta.compiler_v4),f))
		return qe_invalid;
	
	if(zmeta_version == 2)
	{
		for(int32_t c = 0; c < 33; ++c)
		{
			if(!p_getc(&b33[c],f))
			{
				return qe_invalid;
			}
		}
		temp_meta.script_name.assign(b33);
		
		for(int32_t c = 0; c < 33; ++c)
		{
			if(!p_getc(&b33[c],f))
			{
				return qe_invalid;
			}
		}
		temp_meta.author.assign(b33);
	}
	else if(zmeta_version > 2)
	{
		if(!p_getcstr(&temp_meta.script_name,f))
		return qe_invalid;
		if(!p_getcstr(&temp_meta.author,f))
			return qe_invalid;
		word num_meta_attrib = (zmeta_version < 5 ? 4 : 10);
		string tmpstr;
		if (zmeta_version < 6)
		{
			for(auto q = 0; q < num_meta_attrib; ++q)
			{
				if(!p_getcstr(&temp_meta.attributes[q],f))
					return qe_invalid;
				if(!p_getwstr(&temp_meta.attributes_help[q],f))
					return qe_invalid;
			}
			for(auto q = 0; q < 8; ++q)
			{
				if(!p_getcstr(&tmpstr,f))
					return qe_invalid;
				if (8+q >= num_meta_attrib || !tmpstr.empty())
					temp_meta.attributes[8+q] = tmpstr;
				if(!p_getwstr(&tmpstr,f))
					return qe_invalid;
				if (8+q >= num_meta_attrib || !tmpstr.empty())
					temp_meta.attributes_help[8+q] = tmpstr;
			}
			for(auto q = 0; q < 8; ++q)
			{
				if(!p_getcstr(&temp_meta.attributes[16+q],f))
					return qe_invalid;
				if(!p_getwstr(&temp_meta.attributes_help[16+q],f))
					return qe_invalid;
			}
		}
		else
		{
			if (!p_igetw(&num_meta_attrib, f))
				return qe_invalid;
			if (num_meta_attrib > NUM_ZMETA_ATTRIBUTES)
				return qe_invalid;
			for(auto q = 0; q < num_meta_attrib; ++q)
			{
				if(!p_getcstr(&temp_meta.attributes[q],f))
					return qe_invalid;
				if(!p_getwstr(&temp_meta.attributes_help[q],f))
					return qe_invalid;
			}
		}
		
		for(auto q = 0; q < 16; ++q)
		{
			if(!p_getcstr(&temp_meta.usrflags[q],f))
				return qe_invalid;
			if(!p_getwstr(&temp_meta.usrflags_help[q],f))
				return qe_invalid;
		}
	}
	
	if(zmeta_version > 3)
	{
		for(auto q = 0; q < 8; ++q)
		{
			if(!p_getcstr(&temp_meta.initd_label[q],f))
				return qe_invalid;
			if(!p_getwstr(&temp_meta.initd_help[q],f))
				return qe_invalid;
		}
		for(auto q = 0; q < 8; ++q)
		{
			if(!p_getc(&temp_meta.initd_type[q],f))
				return qe_invalid;
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
			temp_meta.initd_label[q] = temp_meta.run_idens[q];
	}
	
	return 0;
}

int32_t read_old_ffscript(PACKFILE *f, word s_version, script_data *script, word zmeta_version)
{
	int32_t num_commands=1000;
	
	if(s_version>=2)
	{
		if(!p_igetl(&num_commands,f))
		{
			return qe_invalid;
		}
	}

#ifdef ZC_FUZZ
	const int32_t command_limit = 300000;
#else
	const int32_t command_limit = 10000000;
#endif
	if (num_commands < 0 || num_commands > command_limit)
	{
		return qe_invalid;
	}

	std::vector<ffscript> zasm;
	zasm.reserve(num_commands);

	if(s_version >= 16)
	{
		zasm_meta temp_meta;
		if (auto ret = read_one_zmeta(f, temp_meta, zmeta_version))
			return ret;
		script->meta = temp_meta;
	} else script->meta = {};
	
	for(int32_t j=0; j<num_commands; j++)
	{
		auto& sc = zasm.emplace_back();
		if(!p_igetw(&sc.command,f))
		{
			return qe_invalid;
		}
		
		if(sc.command == 0xFFFF)
			break;
		else
		{
			if(!p_igetl(&sc.arg1,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetl(&sc.arg2,f))
			{
				return qe_invalid;
			}
			
			if(s_version >= 24)
				if(!p_igetl(&sc.arg3,f))
					return qe_invalid;
			
			if(s_version >= 21)
			{
				uint32_t sz = 0;
				if(!p_igetl(&sz,f))
				{
					return qe_invalid;
				}
				if(sz) //string found
				{
					sc.strptr = new std::string();
					char dummy;
					for(size_t q = 0; q < sz; ++q)
					{
						if(!p_getc(&dummy,f))
						{
							return qe_invalid;
						}
						sc.strptr->push_back(dummy);
					}
				}
				if(!p_igetl(&sz,f))
				{
					return qe_invalid;
				}
				if(sz) //vector found
				{
					sc.vecptr = new std::vector<int32_t>();
					int32_t dummy;
					for(size_t q = 0; q < sz; ++q)
					{
						if(!p_igetl(&dummy,f))
						{
							return qe_invalid;
						}
						sc.vecptr->push_back(dummy);
					}
				}
			}
		}
	}
	
	if (script == &fake_script_data)
		return 0;

	// If the first command is unknown, invalidate the whole thing.
	// Saw this for https://www.purezc.net/index.php?page=quests&id=411 hero script 0
	if (!zasm.empty() && zasm[0].command >= NUMCOMMANDS && zasm[0].command != 0xFFFF)
	{
		al_trace("Warning: found script with bad instruction, disabling script: %s %d\n", ScriptTypeToString(script->id.type), script->id.index);
		zasm.clear();
	}

	zasm_script_id id = zasm_scripts.size();
	auto& zs = zasm_scripts.emplace_back(std::make_shared<zasm_script>(id, script->name(), std::move(zasm)));
	script->zasm_script = zs;
	script->pc = 0;
	script->end_pc = zs->size;
	if (script->valid())
	{
		zs->script_datas.push_back(script);
		read_scripts.push_back(script);
	}

	return 0;
}

// TODO: remove unused parameter.
int32_t read_one_ffscript(PACKFILE *f, zquestheader *, [[maybe_unused]] int32_t script_index, word s_version, script_data *script, word zmeta_version)
{
	ASSERT(script);
	if(s_version < 27)
		return read_old_ffscript(f, s_version, script, zmeta_version);

	char exists;
	if (!p_getc(&exists, f))
		return qe_invalid;
	if (!exists)
	{
		script->disable();
		return 0;
	}

	//Read meta
	{
		zasm_meta temp_meta;
		if (auto ret = read_one_zmeta(f, temp_meta, zmeta_version))
			return ret;
		script->meta = temp_meta;
	}
	if(!p_igetl(&script->pc, f))
		return qe_invalid;
	if(!p_igetl(&script->end_pc, f))
		return qe_invalid;
	
	if (s_version >= 30)
	{
		if (!p_getbmap(&script->script_d_init, f))
			return qe_invalid;
		if (!p_getbmap(&script->script_d_exports, f))
			return qe_invalid;
	}

	if (script == &fake_script_data)
		return 0;

	assert(zasm_scripts.size() == 1);
	auto& zs = zasm_scripts[0];
	script->zasm_script = zs;

	if (script->valid())
	{
		zs->script_datas.push_back(script);
		read_scripts.push_back(script);
	}

	return 0;
}

} // end namespace

int32_t readffscript(PACKFILE *f, zquestheader *Header)
{
	int32_t dummy;
	word s_version=0, zmeta_version=0;
	byte numscripts=0;
	numscripts=numscripts; //to avoid unused variables warnings
	int32_t ret;
	read_scripts.clear();
	zasm_scripts.clear();
	zasm_debug_data = {};
	
	//section version info
	if(!p_igetw(&s_version,f))
	{
		return qe_invalid;
	}

	if (s_version > V_FFSCRIPT)
			return qe_version;
	
	FFCore.quest_format[vFFScript] = s_version;
	
	if(!read_deprecated_section_cversion(f))
	{
		return qe_invalid;
	}
	
	if(s_version >= 18)
	{
		if(!p_igetw(&zmeta_version,f))
		{
			return qe_invalid;
		}
	}
	
	//section size
	if(!p_igetl(&dummy,f))
	{
		return qe_invalid;
	}

	if ( FFCore.quest_format[vLastCompile] < 13 ) FFCore.quest_format[vLastCompile] = s_version;
	al_trace("Loaded scripts last compiled in ZScript version: %d\n", (FFCore.quest_format[vLastCompile]));
	
	if(s_version >= 27)
	{
		ret = read_quest_zasm(f, s_version);
		if(ret)
			return qe_invalid;
	}
	
	//finally...  section data
	for(int32_t i = 0; i < ((s_version < 2) ? NUMSCRIPTFFCOLD : NUMSCRIPTFFC); i++)
	{
		ret = read_one_ffscript(f, Header, i, s_version, ffscripts[i], zmeta_version);
		
		if (ret)
		{
			return qe_invalid;
		}
	}
	
	/* HIGHLY UNORTHODOX UPDATING THING, by Deedee
	* This fixes changes to sprite jump values introduced in early 2.55 alphas.
	* Zoria didn't bump up the versions as liberally as he should have, but thankfully
	* there was a version bump a week before a change that broke stuff.
	*/
	if(((Header->zelda_version < 0x253)||((Header->zelda_version == 0x253)&&(Header->build<33))||((Header->zelda_version > 0x253) && s_version < 12)))
	{
		set_qr(qr_SPRITE_JUMP_IS_TRUNCATED,1);
	}
	if(s_version < 19)
	{
		set_qr(qr_FLUCTUATING_ENEMY_JUMP,1);
	}
	
	if(s_version > 1)
	{
		for(int32_t i = 0; i < NUMSCRIPTITEM; i++)
		{
			ret = read_one_ffscript(f, Header, i, s_version, itemscripts[i], zmeta_version);
			
			if (ret)
			{
				return qe_invalid;
			}
		}
		
		for(int32_t i = 0; i < NUMSCRIPTGUYS; i++)
		{
			ret = read_one_ffscript(f, Header, i, s_version, guyscripts[i], zmeta_version);
			
			if (ret)
			{
				return qe_invalid;
			}
		}
		
		for(int32_t i = 0; i < NUMSCRIPTWEAPONS; i++)
		{
			ret = read_one_ffscript(f, Header, i, s_version, &fake_script_data, zmeta_version);
			
			if (ret)
			{
				return qe_invalid;
			}
		}
	
		for(int32_t i = 0; i < NUMSCRIPTSCREEN; i++)
		{
			ret = read_one_ffscript(f, Header, i, s_version, screenscripts[i], zmeta_version);
			
			if (ret)
			{
				return qe_invalid;
			}
		}
	
		if(s_version > 16)
		{
			for(int32_t i = 0; i < NUMSCRIPTGLOBAL; ++i)
			{
				ret = read_one_ffscript(f, Header, i, s_version, globalscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
		else if(s_version > 13)
		{
			for(int32_t i = 0; i < NUMSCRIPTGLOBAL255OLD; ++i)
			{
				ret = read_one_ffscript(f, Header, i, s_version, globalscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			
			if(globalscripts[GLOBAL_SCRIPT_ONSAVE] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONSAVE];
				
			globalscripts[GLOBAL_SCRIPT_ONSAVE] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONSAVE);
		}
		else if(s_version > 4)
		{
			for(int32_t i = 0; i < NUMSCRIPTGLOBAL253; ++i)
			{
				ret = read_one_ffscript(f, Header, i, s_version, globalscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			
			if(globalscripts[GLOBAL_SCRIPT_ONLAUNCH] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONLAUNCH];
				
			globalscripts[GLOBAL_SCRIPT_ONLAUNCH] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH);
			
			if(globalscripts[GLOBAL_SCRIPT_ONCONTGAME] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONCONTGAME];
				
			globalscripts[GLOBAL_SCRIPT_ONCONTGAME] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONCONTGAME);
			
			if(globalscripts[GLOBAL_SCRIPT_F6] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_F6];
				
			globalscripts[GLOBAL_SCRIPT_F6] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_F6);
			
			if(globalscripts[GLOBAL_SCRIPT_ONSAVE] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONSAVE];
				
			globalscripts[GLOBAL_SCRIPT_ONSAVE] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONSAVE);
		}
		else
		{
			for(int32_t i = 0; i < NUMSCRIPTGLOBALOLD; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, globalscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			
			if(globalscripts[GLOBAL_SCRIPT_ONSAVELOAD] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONSAVELOAD];
				
			globalscripts[GLOBAL_SCRIPT_ONSAVELOAD] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONSAVELOAD);
			
			if(globalscripts[GLOBAL_SCRIPT_ONLAUNCH] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONLAUNCH];
				
			globalscripts[GLOBAL_SCRIPT_ONLAUNCH] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONLAUNCH);
			
			if(globalscripts[GLOBAL_SCRIPT_ONCONTGAME] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONCONTGAME];
				
			globalscripts[GLOBAL_SCRIPT_ONCONTGAME] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONCONTGAME);
			
			if(globalscripts[GLOBAL_SCRIPT_F6] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_F6];
				
			globalscripts[GLOBAL_SCRIPT_F6] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_F6);
			
			if(globalscripts[GLOBAL_SCRIPT_ONSAVE] != NULL)
				delete globalscripts[GLOBAL_SCRIPT_ONSAVE];
				
			globalscripts[GLOBAL_SCRIPT_ONSAVE] = new script_data(ScriptType::Global, GLOBAL_SCRIPT_ONSAVE);
		}
		
		if(s_version > 10) //expanded the number of Hero scripts to 5. 
		{
			for(int32_t i = 0; i < NUMSCRIPTHERO; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, playerscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
		else
		{
			for(int32_t i = 0; i < NUMSCRIPTHEROOLD; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, playerscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			if(playerscripts[3] != NULL)
				delete playerscripts[3];
					
			playerscripts[3] = new script_data(ScriptType::Hero, 3);
			
			if(playerscripts[4] != NULL)
				delete playerscripts[4];
					
			playerscripts[4] = new script_data(ScriptType::Hero, 4);
		}
		if(s_version > 8 && s_version < 10)
		{
			
			for(int32_t i = 0; i < NUMSCRIPTWEAPONS; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, ewpnscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			for(int32_t i = 0; i < NUMSCRIPTSDMAP; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, dmapscripts[i], zmeta_version);
			
				if (ret)
				{
					return qe_invalid;
				}
			}
			
		}
		if(s_version >= 10)
		{
			for(int32_t i = 0; i < NUMSCRIPTWEAPONS; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, lwpnscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			for(int32_t i = 0; i < NUMSCRIPTWEAPONS; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, ewpnscripts[i], zmeta_version);
				
				if (ret)
				{
					return qe_invalid;
				}
			}
			for(int32_t i = 0; i < NUMSCRIPTSDMAP; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, dmapscripts[i], zmeta_version);
			
				if (ret)
				{
					return qe_invalid;
				}
			}
			
		}
		if(s_version >=12)
		{
			for(int32_t i = 0; i < NUMSCRIPTSITEMSPRITE; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, itemspritescripts[i], zmeta_version);
					
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
		if(s_version >=15)
		{
			for(int32_t i = 0; i < NUMSCRIPTSCOMBODATA; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, comboscripts[i], zmeta_version);
					
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
		if(s_version >19)
		{
			word numgenscripts = NUMSCRIPTSGENERIC;
			if(!p_igetw(&numgenscripts,f))
			{
				return qe_invalid;
			}
			for(int32_t i = 0; i < numgenscripts; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, genericscripts[i], zmeta_version);
					
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
		if(s_version >21)
		{
			word numsubscripts = NUMSCRIPTSSUBSCREEN;
			if(!p_igetw(&numsubscripts,f))
			{
				return qe_invalid;
			}
			for(int32_t i = 0; i < numsubscripts; i++)
			{
				ret = read_one_ffscript(f, Header, i, s_version, subscreenscripts[i], zmeta_version);
					
				if (ret)
				{
					return qe_invalid;
				}
			}
		}
	}
	
	if(s_version > 2)
	{
		int32_t bufsize;
		p_igetl(&bufsize, f);
		if (bufsize < 0 || bufsize > 1024*1024*10)
		{
			// God help anyone storing more than 10MB of code in the script buffer.
			return qe_invalid;
		}
		char * buf = new char[bufsize+1];
		pfread(buf, bufsize, f);
		buf[bufsize]=0;
		
		zScript = string(buf);
			
		delete[] buf;
		word numffcbindings;
		p_igetw(&numffcbindings, f);
		
		for(int32_t i=0; i<numffcbindings; i++)
		{
			word id;
			p_igetw(&id, f);
			p_igetl(&bufsize, f);
			if (bufsize < 0 || bufsize > 1024)
				return qe_invalid;
			buf = new char[bufsize+1];
			pfread(buf, bufsize, f);
			buf[bufsize]=0;
			
			//fix for buggy older saved quests -DD
			if(id < NUMSCRIPTFFC-1)
				ffcmap[id].scriptname = buf;
				
			delete[] buf;
		}
		
		word numglobalbindings;
		p_igetw(&numglobalbindings, f);
		
		for(int32_t i=0; i<numglobalbindings; i++)
		{
			word id;
			p_igetw(&id, f);
			p_igetl(&bufsize, f);
			if (bufsize < 0 || bufsize > 1024)
				return qe_invalid;
			buf = new char[bufsize+1];
			pfread(buf, bufsize, f);
			buf[bufsize]=0;
			
			// id in principle should be valid, since slot assignment cannot assign a global script to a bogus slot.
			// However, because of a corruption bug, some 2.50.x quests contain bogus entries in the global bindings table.
			// Ignore these. -DD
			if (id < NUMSCRIPTGLOBAL)
			{
				//Disable old '~Continue's, they'd wreak havoc. Bit messy, apologies ~Joe
				if(strcmp(buf,"~Continue") == 0)
				{
					globalmap[id].scriptname = "";
					
					if(globalscripts[GLOBAL_SCRIPT_ONSAVELOAD] != NULL)
						globalscripts[GLOBAL_SCRIPT_ONSAVELOAD]->disable();
				}
				else
				{
					globalmap[id].scriptname = buf;
				}
			}
			
			delete[] buf;
		}
		
		if(s_version > 3)
		{
			word numitembindings;
			p_igetw(&numitembindings, f);
			
			for(int32_t i=0; i<numitembindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTITEM-1)
					itemmap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
		//(v9+)
		if(s_version > 8)
		{
			//npc scripts
			word numnpcbindings;
			p_igetw(&numnpcbindings, f);
			
			for(int32_t i=0; i<numnpcbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTGUYS-1)
					npcmap[id].scriptname = buf;
					
				delete[] buf;
			}
			//lweapon
			word numlwpnbindings;
			p_igetw(&numlwpnbindings, f);
			
			for(int32_t i=0; i<numlwpnbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTWEAPONS-1)
					lwpnmap[id].scriptname = buf;
					
				delete[] buf;
			}
			//eweapon
			word numewpnbindings;
			p_igetw(&numewpnbindings, f);
			
			for(int32_t i=0; i<numewpnbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTWEAPONS-1)
					ewpnmap[id].scriptname = buf;
					
				delete[] buf;
			}
			//hero
			word numherobindings;
			p_igetw(&numherobindings, f);
			
			for(int32_t i=0; i<numherobindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTHERO-1)
					playermap[id].scriptname = buf;
					
				delete[] buf;
			}
			//dmaps
			word numdmapbindings;
			p_igetw(&numdmapbindings, f);
			
			for(int32_t i=0; i<numdmapbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSDMAP-1)
					dmapmap[id].scriptname = buf;
					
				delete[] buf;
			}
			//screen
			word numscreenbindings;
			p_igetw(&numscreenbindings, f);
			
			for(int32_t i=0; i<numscreenbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSDMAP-1)
					screenmap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
		if(s_version > 11)
		{
			word numspritebindings;
			p_igetw(&numspritebindings, f);
			
			for(int32_t i=0; i<numspritebindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSDMAP-1)
					itemspritemap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
		if(s_version >= 15)
		{
			word numcombobindings;
			p_igetw(&numcombobindings, f);
			
			for(int32_t i=0; i<numcombobindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSCOMBODATA-1)
					comboscriptmap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
		if(s_version > 19)
		{
			word numgenericbindings;
			p_igetw(&numgenericbindings, f);
			
			for(int32_t i=0; i<numgenericbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSGENERIC-1)
					genericmap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
		if(s_version > 21)
		{
			word numsubscreenbindings;
			p_igetw(&numsubscreenbindings, f);
			
			for(int32_t i=0; i<numsubscreenbindings; i++)
			{
				word id;
				p_igetw(&id, f);
				p_igetl(&bufsize, f);
				if (bufsize < 0 || bufsize > 1024)
					return qe_invalid;
				buf = new char[bufsize+1];
				pfread(buf, bufsize, f);
				buf[bufsize]=0;
				
				//fix this too
				if(id <NUMSCRIPTSSUBSCREEN-1)
					subscreenmap[id].scriptname = buf;
					
				delete[] buf;
			}
		}
	}

	if (s_version >= 29)
	{
		std::vector<byte> debugDataBytes;
		if(!p_getlvec(&debugDataBytes, f))
			return qe_invalid;

		if (auto debugData = DebugData::decode(debugDataBytes))
		{
			zasm_debug_data = std::move(*debugData);
		}
		else
		{
			zprint2("Error: failed to decode zscript debug data\n");
			return qe_invalid;
		}
	}

	// Set ZScriptVersion to the value encoded in the script's meta.ffscript_v.
	// This is only updated when the scripts have been recompiled.
	// They should all match each other, but there may have been bugs causing this field
	// to not always be set so take the largest one.
	std::optional<word> zscript_version;
	if (s_version >= 16)
	{
		for (auto script : read_scripts)
		{
			// Scripts with "0" were saved in a version prior to this field being set.
			// See https://discord.com/channels/876899628556091432/1368485306394738718
			word ffscript_v = script->meta.ffscript_v == 0 ? 16 : script->meta.ffscript_v;
			if (!zscript_version.has_value())
				zscript_version = ffscript_v;
			else if (ffscript_v > zscript_version.value())
				zscript_version = ffscript_v;
		}

		if (!zscript_version.has_value())
			al_trace("WARNING: Setting zscript version to section version as fallback.\n");
	}
	setZScriptVersion(zscript_version.value_or(s_version));
	read_scripts.clear();

	return 0;
}
