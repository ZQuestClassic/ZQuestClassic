#ifndef ZSCR_DATA_H_
#define ZSCR_DATA_H_

#include "base/util.h"
#include "core/zdefs.h"
#include "components/zasm/debug_data.h"
#include "zconsole/ConsoleLogger.h"
#include "parser/Compiler.h"
#include "base/containers.h"
#include <sstream>

#ifndef IS_PARSER
#include "zq/zquest.h"
#endif //!IS_PARSER

using ZScript::disassembled_script_data;

#define ZC_CONSOLE_INFO_CODE -9998
#define ZC_CONSOLE_ERROR_CODE -9997
#define ZC_CONSOLE_WARN_CODE -9996
#define ZC_CONSOLE_DB_CODE -9995
#define ZC_CONSOLE_TERM_CODE -9994
#define ZC_CONSOLE_IDLE_CODE -9993

using std::map;
using std::string;
using std::vector;

void write_str(string const& str, FILE* f);
void read_str(string& str, FILE* f);
template<typename T>
void write_vec(vector<T> const& vec, FILE* f);
template<typename T>
void read_vec(vector<T>& vec, FILE* f);
template<uint_type Sz,typename T>
inline void write_boundedcont(bounded_vec<Sz,T> const& cont, FILE *f);
template<uint_type Sz,typename T>
inline void write_boundedcont(bounded_map<Sz,T> const& cont, FILE *f);
template<uint_type Sz,typename T>
inline void read_boundedcont(bounded_vec<Sz,T> &cont, FILE *f);
template<uint_type Sz,typename T>
inline void read_boundedcont(bounded_map<Sz,T> &cont, FILE *f);

template<integral_type T>
inline void write_var(T const& val, FILE *f)
{
	fwrite((char const*)&val, sizeof(T), 1, f);
}
template<integral_type T>
inline void read_var(T& val, FILE *f)
{
	fread((char*)&val, sizeof(T), 1, f);
}

inline void write_var(string const& val, FILE *f)
{
	write_str(val, f);
}
inline void read_var(string& val, FILE *f)
{
	read_str(val, f);
}
template<typename T>
inline void write_var(vector<T> const& val, FILE *f)
{
	write_vec(val, f);
}
template<typename T>
inline void read_var(vector<T>& val, FILE *f)
{
	read_vec(val, f);
}
template<uint_type Sz,typename T>
inline void write_var(bounded_vec<Sz,T> const& val, FILE *f)
{
	write_boundedcont(val, f);
}
template<uint_type Sz,typename T>
inline void write_var(bounded_map<Sz,T> const& val, FILE *f)
{
	write_boundedcont(val, f);
}
template<uint_type Sz,typename T>
inline void read_var(bounded_vec<Sz,T>& val, FILE *f)
{
	read_boundedcont(val, f);
}
template<uint_type Sz,typename T>
inline void read_var(bounded_map<Sz,T>& val, FILE *f)
{
	read_boundedcont(val, f);
}


inline void write_var(zfix const& val, FILE *f)
{
	write_var(val.val, f);
}
inline void read_var(zfix& val, FILE *f)
{
	read_var(val.val, f);
}

inline void write_var(exported_variable const& val, FILE *f)
{
	write_str(val.name, f);
	write_str(val.helptext, f);
	write_var(val.btn_type, f);
	write_var(val.min, f);
	write_var(val.max, f);
}
inline void read_var(exported_variable& val, FILE *f)
{
	read_str(val.name, f);
	read_str(val.helptext, f);
	read_var(val.btn_type, f);
	read_var(val.min, f);
	read_var(val.max, f);
}


inline void write_str(string const& str, FILE* f)
{
	size_t sz = str.size();
	write_var(sz, f);
	for(size_t q = 0; q < sz; ++q)
		fputc(str.at(q), f);
}
inline void read_str(string& str, FILE* f)
{
	size_t sz;
	read_var(sz, f);
	str.clear();
	for(size_t q = 0; q < sz; ++q)
		str.push_back(fgetc(f));
}

template<typename T>
void write_vec(vector<T> const& vec, FILE* f)
{
	size_t sz = vec.size();
	write_var(sz, f);
	for(size_t q = 0; q < sz; ++q)
		write_var(vec.at(q), f);
}
template<typename T>
void read_vec(vector<T>& vec, FILE* f)
{
	size_t sz;
	read_var(sz, f);
	vec.clear();
	for(size_t q = 0; q < sz; ++q)
		read_var(vec.emplace_back(), f);
}


template<uint_type Sz,typename T>
inline void write_boundedcont(bounded_vec<Sz,T> const& cont, FILE *f)
{
	Sz sz = cont.size();
	write_var(sz, f);
	if(sz)
	{
		T dt = cont.defval();
		Sz writecnt_v = cont.capacity();
		Sz writecnt_m = 0;
		for(Sz q = 0; q < writecnt_v; ++q)
			if(cont.get(q) != dt)
				++writecnt_m;
		bool pairs = (writecnt_m * (sizeof(T)+sizeof(Sz))) <= writecnt_v * sizeof(T);
		write_var(byte(pairs ? 1 : 0), f);
		write_var(pairs ? writecnt_m : writecnt_v, f);
		if(pairs)
		{
			for(Sz q = 0; q < writecnt_v; ++q)
			{
				if(cont.get(q) == dt) continue;
				write_var(q, f);
				write_var(cont.get(q), f);
			}
		}
		else
		{
			for(Sz q = 0; q < writecnt_v; ++q)
				write_var(cont.get(q), f);
		}
	}
}
template<uint_type Sz,typename T>
inline void write_boundedcont(bounded_map<Sz,T> const& cont, FILE *f)
{
	Sz sz = cont.size();
	write_var(sz, f);
	if(sz)
	{
		T dt = cont.defval();
		auto lkey = cont.lastKey();
		Sz writecnt_v = lkey ? *lkey+1 : 0;
		Sz writecnt_m = 0;
		for(auto [k,v] : cont.inner())
			if(k < sz && v != dt)
				++writecnt_m;
		bool pairs = (writecnt_m * (sizeof(T)+sizeof(Sz))) <= writecnt_v * sizeof(T);
		write_var(byte(pairs ? 1 : 0), f);
		write_var(pairs ? writecnt_m : writecnt_v, f);
		if(pairs)
		{
			for(auto [k,v] : cont.inner())
			{
				if(k >= sz || v == dt) continue;
				write_var(k, f);
				write_var(v, f);
			}
		}
		else
		{
			for(Sz q = 0; q < writecnt_v; ++q)
				write_var(cont.get(q), f);
		}
	}
}

template<uint_type Sz,typename T>
inline void read_boundedcont(bounded_vec<Sz,T> &cont, FILE *f)
{
	cont.clear();
	Sz sz = 0;
	read_var(sz, f);
	cont.resize(sz);
	if(sz) //cont found
	{
		Sz count;
		byte pairs;
		read_var(pairs, f);
		read_var(count, f);
		Sz k;
		T v = cont.defval();
		if(pairs)
		{
			while(count--)
			{
				read_var(k, f);
				read_var(v, f);
				cont[k] = v;
			}
		}
		else
		{
			for(k = 0; k < count; ++k)
			{
				read_var(v, f);
				cont[k] = v;
			}
		}
		cont.normalize();
	}
}
template<uint_type Sz,typename T>
inline void read_boundedcont(bounded_map<Sz,T> &cont, FILE *f)
{
	cont.clear();
	Sz sz = 0;
	read_var(sz, f);
	cont.resize(sz);
	if(sz) //cont found
	{
		Sz count;
		byte pairs;
		read_var(pairs, f);
		read_var(count, f);
		Sz k;
		T v = cont.defval();
		if(pairs)
		{
			while(count--)
			{
				read_var(k, f);
				read_var(v, f);
				cont[k] = v;
			}
		}
		else
		{
			for(k = 0; k < count; ++k)
			{
				read_var(v, f);
				cont[k] = v;
			}
		}
		cont.normalize();
	}
}

void write_meta(zasm_meta const& meta, FILE* f)
{
	write_var(meta.zasm_v, f);
	write_var(meta.meta_v, f);
	write_var(meta.ffscript_v, f);
	write_var(byte(meta.script_type), f);
	for(auto q = 0; q < 8; ++q)
		write_str(meta.run_idens[q], f);
	for(auto q = 0; q < 8; ++q)
		write_var(meta.run_types[q], f);
	write_var(meta.flags, f);
	write_var(meta.compiler_v1, f);
	write_var(meta.compiler_v2, f);
	write_var(meta.compiler_v3, f);
	write_var(meta.compiler_v4, f);
	write_str(meta.script_name, f);
	write_str(meta.author, f);
	for(auto q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
		write_str(meta.attributes[q], f);
	for(auto q = 0; q < 16; ++q)
		write_str(meta.usrflags[q], f);
	for(auto q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
		write_str(meta.attributes_help[q], f);
	for(auto q = 0; q < 16; ++q)
		write_str(meta.usrflags_help[q], f);
	for(auto q = 0; q < 8; ++q)
		write_str(meta.initd_label[q], f);
	for(auto q = 0; q < 8; ++q)
		write_str(meta.initd_help[q], f);
	for(auto q = 0; q < 8; ++q)
		write_var(meta.initd_type[q], f);
}

void read_meta(zasm_meta& meta, FILE* f)
{
	read_var(meta.zasm_v, f);
	read_var(meta.meta_v, f);
	read_var(meta.ffscript_v, f);
	{
		byte st = (byte)meta.script_type;
		read_var(st, f);
		meta.script_type = (ScriptType)st;
	}
	for(auto q = 0; q < 8; ++q)
		read_str(meta.run_idens[q], f);
	for(auto q = 0; q < 8; ++q)
		read_var(meta.run_types[q], f);
	read_var(meta.flags, f);
	read_var(meta.compiler_v1, f);
	read_var(meta.compiler_v2, f);
	read_var(meta.compiler_v3, f);
	read_var(meta.compiler_v4, f);
	read_str(meta.script_name, f);
	read_str(meta.author, f);
	for(auto q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
		read_str(meta.attributes[q], f);
	for(auto q = 0; q < 16; ++q)
		read_str(meta.usrflags[q], f);
	for(auto q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
		read_str(meta.attributes_help[q], f);
	for(auto q = 0; q < 16; ++q)
		read_str(meta.usrflags_help[q], f);
	for(auto q = 0; q < 8; ++q)
		read_str(meta.initd_label[q], f);
	for(auto q = 0; q < 8; ++q)
		read_str(meta.initd_help[q], f);
	for(auto q = 0; q < 8; ++q)
		read_var(meta.initd_type[q], f);
}

void read_compile_data(ZScript::ZasmCompilerResult& zasmCompilerResult)
{
	zasmCompilerResult = {};

	size_t stypes_sz, scripts_sz;
	size_t dummy;
	ZScript::ParserScriptType::Id _id;
	string str, str2;
	
	FILE *tempfile = fopen("tmp2","rb");
			
	if(!tempfile)
	{
		//InfoDialog("Error","Unable to open the temporary file in current directory!").show();
		return;
	}
	
	read_var(stypes_sz, tempfile);
	for(size_t ind = 0; ind < stypes_sz; ++ind)
	{
		read_str(str, tempfile);
		read_var(_id, tempfile);
		zasmCompilerResult.scriptTypes[str] = _id;
	}
	
	read_var(scripts_sz, tempfile);
	for(size_t ind = 0; ind < scripts_sz; ++ind)
	{
		read_str(str, tempfile);
		
		disassembled_script_data dsd;
		
		read_meta(dsd.meta, tempfile);
		
		read_var(dsd.format, tempfile);
		
		read_var(dsd.pc, tempfile);
		read_var(dsd.end_pc, tempfile);
		
		read_boundedcont(dsd.script_d_init, tempfile);
		read_boundedcont(dsd.script_d_exports, tempfile);
		
		zasmCompilerResult.theScripts[str] = dsd;
	}

	size_t zasm_sz;
	read_var(zasm_sz, tempfile);
	zasmCompilerResult.zasm.reserve(zasm_sz);
	for(size_t ind2 = 0; ind2 < zasm_sz; ++ind2)
	{
		read_str(str, tempfile);
		read_str(str2, tempfile);
		
		int32_t lbl;
		read_var(lbl, tempfile);
		std::shared_ptr<ZScript::Opcode> oc = std::make_shared<ZScript::ArbitraryOpcode>(str);
		oc->setLabel(lbl);
		oc->setComment(str2);
		zasmCompilerResult.zasm.push_back(oc);
	}

	std::vector<byte> encodedDebugData;
	read_vec(encodedDebugData, tempfile);
	if (auto debugData = DebugData::decode(encodedDebugData))
		zasmCompilerResult.debugData = std::move(*debugData);
	else
		goto read_compile_error;

read_compile_error:
	fclose(tempfile);
}

void write_compile_data(const ZScript::ZasmCompilerResult& zasmCompilerResult)
{
	FILE *tempfile = fopen("tmp2","wb");
			
	if(!tempfile)
	{
		//InfoDialog("Error","Unable to create a temporary file in current directory!").show();
		return;
	}
	
	size_t dummy = zasmCompilerResult.scriptTypes.size();
	write_var(dummy, tempfile);
	for(auto it = zasmCompilerResult.scriptTypes.begin(); it != zasmCompilerResult.scriptTypes.end(); ++it)
	{
		string const& str = it->first;
		ZScript::ParserScriptType::Id v = it->second;
		write_str(str, tempfile);
		write_var(v, tempfile);
	}
	
	dummy = zasmCompilerResult.theScripts.size();
	write_var(dummy, tempfile);
	for(auto it = zasmCompilerResult.theScripts.begin(); it != zasmCompilerResult.theScripts.end(); ++it)
	{
		string const& str = it->first;
		const disassembled_script_data& v = it->second;
		write_str(str, tempfile);
		
		write_meta(v.meta, tempfile);
		
		write_var(v.format, tempfile);
		
		write_var(v.pc, tempfile);
		write_var(v.end_pc, tempfile);
		
		write_boundedcont(v.script_d_init, tempfile);
		write_boundedcont(v.script_d_exports, tempfile);
	}
	
	dummy = zasmCompilerResult.zasm.size();
	write_var(dummy, tempfile);
	for(auto it = zasmCompilerResult.zasm.begin(); it != zasmCompilerResult.zasm.end(); ++it)
	{
		string opstr = (*it)->toString();
		string const& commentstr = (*it)->getComment();
		int32_t lbl = (*it)->getLabel();
		
		write_str(opstr, tempfile);
		write_str(commentstr, tempfile);
		write_var(lbl, tempfile);
	}

	std::vector<byte> debugDataEncoded = zasmCompilerResult.debugData.encode();
	write_vec(debugDataEncoded, tempfile);

	fclose(tempfile);
}

void write_script(ZScript::ZasmCompilerResult& zasmCompilerResult, string& dest, bool commented, bool write_meta = true)
{
	// Estimate size of string to reduce re-allocations.
	if (commented || write_meta)
		dest.reserve(dest.size() + (zasmCompilerResult.zasm.size() * 140));
	else
		dest.reserve(dest.size() + (zasmCompilerResult.zasm.size() * 50));

	auto& debugData = zasmCompilerResult.debugData;
	map<uint,string> meta_map;

	if (write_meta)
	{
		for(auto& pair : zasmCompilerResult.theScripts)
			meta_map[pair.second.pc] = pair.second.meta.get_meta();
	}

	for(uint q = 0; q < zasmCompilerResult.zasm.size(); ++q)
	{
		if (write_meta)
		{
			auto it = meta_map.find(q);
			if (it != meta_map.end())
				dest += it->second;
		}

		ZScript::Opcode* line = zasmCompilerResult.zasm[q].get();
		line->writeLine(dest, false, commented, line->file >= 0 ? debugData.source_files[line->file].path : "", q);
	}

	if (write_meta)
	{
		dest += "\n\n";
		dest += zasmCompilerResult.debugData.internalToStringForDebugging();
	}

	dest += "\n";
}

#ifndef IS_PARSER

CConsoleLoggerEx parser_console;

static const int32_t DB_COLOR = CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BLUE | CConsoleLoggerEx::COLOR_INTENSITY;
static const int32_t WARN_COLOR = CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_GREEN;
static const int32_t ERR_COLOR = CConsoleLoggerEx::COLOR_RED;
static const int32_t INFO_COLOR = CConsoleLoggerEx::COLOR_WHITE;

void _print_zconsole(const char *buf, char const* header, int32_t color)
{
	if (!DisableCompileConsole)
	{
		int32_t v = parser_console.safeprint( color,header);
		if(v < 0) return; //Failed to print
	}
	
	safe_al_trace(buf);
	safe_al_trace("\n");
	if (!DisableCompileConsole)
	{
		parser_console.safeprint( color, buf);
		parser_console.safeprint( color, "\n");
	}
	else
	{
		box_out(buf);
		box_eol();
	}
}

void ReadConsole(char buf[], int code)
{
	switch(code)
	{
		case ZC_CONSOLE_IDLE_CODE: break;
		case ZC_CONSOLE_DB_CODE: _print_zconsole(buf,"[Debug] ",DB_COLOR); break;
		case ZC_CONSOLE_WARN_CODE: _print_zconsole(buf,"[Warn] ",WARN_COLOR); break;
		case ZC_CONSOLE_ERROR_CODE: _print_zconsole(buf,"[Error] ",ERR_COLOR); break;
		default: _print_zconsole(buf,"[Info] ",INFO_COLOR); break;
	}
}
#endif //!IS_PARSER

ScriptType get_script_type(string const& name)
{
	if(name=="GLOBAL")
		return ScriptType::Global;
	else if(name=="FFC")
		return ScriptType::FFC;
	else if(name=="SCREEN")
		return ScriptType::Screen;
	else if(name=="HERO" || name=="PLAYER" || name=="LINK")
		return ScriptType::Hero;
	else if(name=="ITEMDATA" || name=="ITEM")
		return ScriptType::Item;
	else if(name=="LWEAPON" || name=="LWPN")
		return ScriptType::Lwpn;
	else if(name=="NPC")
		return ScriptType::NPC;
	else if(name=="EWEAPON" || name=="EWPN")
		return ScriptType::Ewpn;
	else if(name=="DMAP")
		return ScriptType::DMap;
	else if(name=="ITEMSPRITE")
		return ScriptType::ItemSprite;
	else if(name=="COMBO" || name=="COMBODATA")
		return ScriptType::Combo;
	
	return ScriptType::None;
}

string get_script_name(ScriptType type)
{
	switch(type)
	{
		case ScriptType::Global:
			return "GLOBAL";
		case ScriptType::FFC:
			return "FFC";
		case ScriptType::Screen:
			return "SCREEN";
		case ScriptType::Hero:
			return "HERO";
		case ScriptType::Item:
			return "ITEMDATA";
		case ScriptType::Lwpn:
			return "LWEAPON";
		case ScriptType::NPC:
			return "NPC";
		case ScriptType::Ewpn:
			return "EWEAPON";
		case ScriptType::DMap:
			return "DMAP";
		case ScriptType::ItemSprite:
			return "ITEMSPRITE";
		case ScriptType::Combo:
			return "COMBODATA";
		case ScriptType::Generic: case ScriptType::GenericFrozen:
			return "GENERIC";
		case ScriptType::EngineSubscreen:
			return "SUBSCREEN";
		case ScriptType::None:
		default:
			return "UNKNOWN";
	}
}

//Output metadata as a single string
string zasm_meta::get_meta() const
{
	std::ostringstream oss;
	oss << "#ZASM_VERSION = " << zasm_v
		<< "\n#METADATA_VERSION = " << meta_v
		<< "\n#FFSCRIPT_VERSION = " << ffscript_v
		<< "\n#SCRIPT_NAME = " << script_name;
	if(author.size())
		oss << "\n#AUTHOR = " << author;
	oss << "\n#SCRIPT_TYPE = " << get_script_name(script_type).c_str()
		<< "\n#AUTO_GEN = " << ((flags & ZMETA_AUTOGEN) ? "TRUE" : "FALSE")
		<< "\n#COMPILER_V1 = " << compiler_v1
		<< "\n#COMPILER_V2 = " << compiler_v2
		<< "\n#COMPILER_V3 = " << compiler_v3
		<< "\n#COMPILER_V4 = " << compiler_v4;
	for(auto q = 0; q < 8; ++q)
	{
		if(!run_idens[q].size())
			continue;
		oss << "\n#PARAM_TYPE_" << q << " = " << ZScript::getDataTypeName(run_types[q])
			<< "\n#PARAM_NAME_" << q << " = " << run_idens[q];
	}
	for(auto q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
	{
		if(attributes[q].size())
			oss << "\n#ATTRIBUTE_" << q << " = " << attributes[q];
		if(attributes_help[q].size())
			oss << "\n#ATTRIBUTE_HELP_" << q << " = "
				<< util::escape_characters(attributes_help[q]);
	}
	for(auto q = 0; q < 16; ++q)
	{
		if(usrflags[q].size())
			oss << "\n#FLAG_" << q << " = " << usrflags[q];
		if(usrflags_help[q].size())
			oss << "\n#FLAG_HELP_" << q << " = "
				<< util::escape_characters(usrflags_help[q]);
	}
	for(auto q = 0; q < 8; ++q)
	{
		if(initd_label[q].size())
			oss << "\n#INITD_" << q << " = " << initd_label[q];
		if(initd_help[q].size())
			oss << "\n#INITD_HELP_" << q << " = "
				<< util::escape_characters(initd_help[q]);
		if(initd_type[q] > -1)
			oss << "\n#INITD_TYPE_" << q << " = "
				<< to_string(int32_t(initd_type[q]));
	}
	oss << "\n";
	return oss.str();
}

//Parse a single line of metadata
bool zasm_meta::parse_meta(const char *buffer)
{
	string line(buffer);
	size_t space_pos = line.find_first_of(" \t=");
	if(space_pos == string::npos) return false;
	string cmd = line.substr(0, space_pos); //The command portion
	size_t end_space_pos = line.find_first_not_of(" \t=", space_pos);
	if(end_space_pos == string::npos) return false;
	string val = line.substr(end_space_pos); //The value portion
	size_t endpos = val.find_last_not_of(" \t\r\n\0");
	if(endpos != string::npos) ++endpos;
	val = val.substr(0, endpos); //trim trailing whitespace
	
	if(cmd == "#ZASM_VERSION")
	{
		zasm_v = atoi(val.c_str());
	}
	else if(cmd == "#METADATA_VERSION")
	{
		meta_v = atoi(val.c_str());
	}
	else if(cmd == "#FFSCRIPT_VERSION")
	{
		ffscript_v = atoi(val.c_str());
	}
	else if(cmd == "#SCRIPT_TYPE" || cmd == "#TYPE")
	{
		util::upperstr(val);
		script_type = get_script_type(val);
	}
	else if(cmd == "#SCRIPT_NAME")
	{
		util::replchar(val, ' ', '_');
		script_name = val;
	}
	else if(cmd == "#AUTHOR")
	{
		author = val;
	}
	else if(cmd == "#AUTO_GEN")
	{
		util::upperstr(val);
		if(val=="TRUE")
			flags |= ZMETA_AUTOGEN;
		else if(val=="FALSE")
			flags &= ~ZMETA_AUTOGEN;
		else
		{
			if(atoi(val.c_str())!=0)
			{
				flags |= ZMETA_AUTOGEN;
			}
			else flags &= ~ZMETA_AUTOGEN;
		}
	}
	else if(cmd == "#COMPILER_V1")
	{
		compiler_v1 = atoi(val.c_str());
	}
	else if(cmd == "#COMPILER_V2")
	{
		compiler_v2 = atoi(val.c_str());
	}
	else if(cmd == "#COMPILER_V3")
	{
		compiler_v3 = atoi(val.c_str());
	}
	else if(cmd == "#COMPILER_V4")
	{
		compiler_v4 = atoi(val.c_str());
	}
	else if(cmd.size() == 13 && !cmd.compare(0,12,"#PARAM_NAME_"))
	{
		byte ind = cmd.at(12) - '1';
		if (ind < 8)
		{
			util::replchar(val, ' ', '_');
			run_idens[ind] = val;
		}
		else return false;
	}
	else if(cmd.size() == 13 && !cmd.compare(0,12,"#PARAM_TYPE_"))
	{
		byte ind = cmd.at(12) - '1';
		if (ind < 8)
		{
			util::replchar(val, ' ', '_');
			run_types[ind] = ZScript::getTypeId(val);
		}
		else return false;
	}
	else if (cmd.size() == 12 && !cmd.compare(0, 11, "#ATTRIBUTE_"))
	{
		byte ind = cmd.at(11) - '0';
		if (ind < 10) // 1 digit number
		{
			attributes[ind] = val;
		}
		else return false;
	}
	else if (cmd.size() == 13 && !cmd.compare(0, 11, "#ATTRIBUTE_"))
	{
		if (byte(cmd.at(11)) - '0' >= 10 || byte(cmd.at(12)) - '0' >= 10)
			return false;
		byte ind = atoi(cmd.c_str()+11);
		if (ind >= NUM_ZMETA_ATTRIBUTES)
			return false;
		attributes[ind] = val;
	}
	else if (cmd.size() == 17 && !cmd.compare(0, 16, "#ATTRIBUTE_HELP_"))
	{
		byte ind = cmd.at(16) - '0';
		if (ind < 10) // 1 digit number
		{
			attributes_help[ind] = util::unescape_characters(val);
		}
		else return false;
	}
	else if (cmd.size() == 18 && !cmd.compare(0, 16, "#ATTRIBUTE_HELP_"))
	{
		if (byte(cmd.at(16)) - '0' >= 10 || byte(cmd.at(17)) - '0' >= 10)
			return false;
		byte ind = atoi(cmd.c_str()+16);
		if (ind >= NUM_ZMETA_ATTRIBUTES)
			return false;
		attributes_help[ind] = util::unescape_characters(val);
	}
	else if (cmd.size() == 12 && !cmd.compare(0, 11, "#ATTRIBYTE_"))
	{
		byte ind = cmd.at(11) - '0';
		if (ind < 8)
		{
			attributes[8+ind] = val;
		}
		else return false;
	}
	else if (cmd.size() == 17 && !cmd.compare(0, 16, "#ATTRIBYTE_HELP_"))
	{
		byte ind = cmd.at(16) - '0';
		if (ind < 8)
		{
			attributes_help[8+ind] = util::unescape_characters(val);
		}
		else return false;
	}
	else if (cmd.size() == 13 && !cmd.compare(0, 12, "#ATTRISHORT_"))
	{
		byte ind = cmd.at(12) - '0';
		if (ind < 8)
		{
			attributes[16+ind] = val;
		}
		else return false;
	}
	else if (cmd.size() == 18 && !cmd.compare(0, 17, "#ATTRISHORT_HELP_"))
	{
		byte ind = cmd.at(17) - '0';
		if (ind < 8)
		{
			attributes_help[16+ind] = util::unescape_characters(val);
		}
		else return false;
	}
	else if ((cmd.size() == 7 || cmd.size() == 8) && !cmd.compare(0, 6, "#FLAG_"))
	{
		byte ind = cmd.at(6) - '0';
		if (cmd.size() == 8)
			ind = (ind * 10) + cmd.at(7) - '0';
		if (ind < 16)
		{
			usrflags[ind] = val;
		}
		else return false;
	}
	else if ((cmd.size() == 12 || cmd.size() == 13) && !cmd.compare(0, 11, "#FLAG_HELP_"))
	{
		byte ind = cmd.at(11) - '0';
		if (cmd.size() == 13)
			ind = (ind * 10) + cmd.at(12) - '0';
		if (ind < 16)
		{
			usrflags_help[ind] = util::unescape_characters(val);
		}
		else return false;
	}
	else if (cmd.size() == 8 && !cmd.compare(0, 7, "#INITD_"))
	{
		byte ind = cmd.at(7) - '0';
		if (ind < 8)
		{
			initd_label[ind] = val;
		}
		else return false;
	}
	else if (cmd.size() == 13 && !cmd.compare(0, 12, "#INITD_HELP_"))
	{
		byte ind = cmd.at(12) - '0';
		if (ind < 8)
		{
			initd_help[ind] = util::unescape_characters(val);
		}
		else return false;
	}
	else if (cmd.size() == 13 && !cmd.compare(0, 12, "#INITD_TYPE_"))
	{
		byte ind = cmd.at(12) - '0';
		if (ind < 8)
		{
			initd_type[ind] = atoi(val.c_str());
		}
		else return false;
	}
	else return false;
	
	return true;
}

#endif

