#ifndef _ZASM_TABLE_H
#define _ZASM_TABLE_H

#include "base/headers.h"

// The version of the ZASM engine a script was compiled for
// NOT the same as V_FFSCRIPT, which is the version of the packfile format
// where the scripts are serialized
#define ZASM_VERSION        3

const char* ScriptTypeToString(ScriptType type);

#define ZMETA_AUTOGEN		0x01
#define ZMETA_DISASSEMBLED	0x02
#define ZMETA_IMPORTED		0x04

#define SCRIPT_FORMAT_DEFAULT		0
#define SCRIPT_FORMAT_INVALID		1
#define SCRIPT_FORMAT_DISASSEMBLED	2
#define SCRIPT_FORMAT_ZASM			3

#define METADATA_V			6
#define V_COMPILER_FIRST	BUILDTM_YEAR
#define V_COMPILER_SECOND	BUILDTM_MONTH
#define V_COMPILER_THIRD	BUILDTM_DAY
#define V_COMPILER_FOURTH	BUILDTM_HOUR
#define ZMETA_NULL_TYPE		1
struct zasm_meta
{
	word zasm_v;
	word meta_v;
	word ffscript_v;
	dword global_count;
	ScriptType script_type;
	std::string run_idens[8];
	byte run_types[8];
	byte flags;
	word compiler_v1, compiler_v2, compiler_v3, compiler_v4;
	std::string script_name;
	std::string author;
	std::string attributes[10];
	std::string attribytes[8];
	std::string attrishorts[8];
	std::string usrflags[16];
	std::string attributes_help[10];
	std::string attribytes_help[8];
	std::string attrishorts_help[8];
	std::string usrflags_help[16];
	std::string initd[8];
	std::string initd_help[8];
	int8_t initd_type[8];
	
	void setFlag(byte flag)
	{
		switch(flag)
		{
			case ZMETA_DISASSEMBLED:
				flags &= ~ZMETA_IMPORTED;
				flags |= ZMETA_DISASSEMBLED;
				break;
			case ZMETA_IMPORTED:
				flags &= ~ZMETA_DISASSEMBLED;
				flags |= ZMETA_IMPORTED;
				break;
			default:
				flags |= flag;
		}
	}
	bool valid() const
	{
		return zasm_v >= 2 && meta_v >= 1 && ffscript_v >= 16;
	}
	void zero()
	{
		zasm_v = 0;
		meta_v = 0;
		ffscript_v = 0;
		global_count = 1024;
		script_type = ScriptType::None;
		flags = 0;
		compiler_v1 = 0;
		compiler_v2 = 0;
		compiler_v3 = 0;
		compiler_v4 = 0;
		for(int32_t q = 0; q < 16; ++q)
		{
			usrflags[q].clear();
			usrflags_help[q].clear();
			if(q > 9) continue;
			attributes[q].clear();
			attributes_help[q].clear();
			if(q > 7) continue;
			initd[q].clear();
			initd_help[q].clear();
			initd_type[q] = -1;
			run_idens[q].clear();
			run_types[q] = ZMETA_NULL_TYPE;
			attribytes[q].clear();
			attribytes_help[q].clear();
			attrishorts[q].clear();
			attrishorts_help[q].clear();
		}
		script_name.clear();
		author.clear();
	}
	void autogen(bool clears = true)
	{
		if(clears) zero();
		zasm_v = ZASM_VERSION;
		meta_v = METADATA_V;
		ffscript_v = V_FFSCRIPT;
		flags = ZMETA_AUTOGEN;
		compiler_v1 = V_COMPILER_FIRST;
		compiler_v2 = V_COMPILER_SECOND;
		compiler_v3 = V_COMPILER_THIRD;
		compiler_v4 = V_COMPILER_FOURTH;
	}
	zasm_meta()
	{
		zero();
	}
	~zasm_meta()
	{
		
	}
	zasm_meta& operator=(zasm_meta const& other)
	{
		zasm_v = other.zasm_v;
		meta_v = other.meta_v;
		ffscript_v = other.ffscript_v;
		script_type = other.script_type;
		for(auto q = 0; q < 16; ++q)
		{
			usrflags[q] = other.usrflags[q];
			usrflags_help[q] = other.usrflags_help[q];
			if(q > 9) continue;
			attributes[q] = other.attributes[q];
			attributes_help[q] = other.attributes_help[q];
			if(q > 7) continue;
			initd[q] = other.initd[q];
			initd_help[q] = other.initd_help[q];
			initd_type[q] = other.initd_type[q];
			run_idens[q] = other.run_idens[q];
			run_types[q] = other.run_types[q];
			attribytes[q] = other.attribytes[q];
			attribytes_help[q] = other.attribytes_help[q];
			attrishorts[q] = other.attrishorts[q];
			attrishorts_help[q] = other.attrishorts_help[q];
			if(q > 3) continue;
		}
		flags = other.flags;
		compiler_v1 = other.compiler_v1;
		compiler_v2 = other.compiler_v2;
		compiler_v3 = other.compiler_v3;
		compiler_v4 = other.compiler_v4;
		script_name = other.script_name;
		author = other.author;
		return *this;
	}
	bool operator==(zasm_meta const& other) const
	{
		if(zasm_v != other.zasm_v) return false;
		if(meta_v != other.meta_v) return false;
		if(ffscript_v != other.ffscript_v) return false;
		if(script_type != other.script_type) return false;
		if(flags != other.flags) return false;
		if(compiler_v1 != other.compiler_v1) return false;
		if(compiler_v2 != other.compiler_v2) return false;
		if(compiler_v3 != other.compiler_v3) return false;
		if(compiler_v4 != other.compiler_v4) return false;
		for(auto q = 0; q < 16; ++q)
		{
			if(usrflags[q].compare(other.usrflags[q]))
				return false;
			if(usrflags_help[q].compare(other.usrflags_help[q]))
				return false;
			if(q > 9) continue;
			if(attributes[q].compare(other.attributes[q]))
				return false;
			if(attributes_help[q].compare(other.attributes_help[q]))
				return false;
			if(q > 7) continue;
			if(initd[q].compare(other.initd[q]))
				return false;
			if(initd_help[q].compare(other.initd_help[q]))
				return false;
			if(initd_type[q] != other.initd_type[q])
				return false;
			if(run_idens[q].compare(other.run_idens[q]))
				return false;
			if(run_types[q] != other.run_types[q])
				return false;
			if(attribytes[q].compare(other.attribytes[q]))
				return false;
			if(attribytes_help[q].compare(other.attribytes_help[q]))
				return false;
			if(attrishorts[q].compare(other.attrishorts[q]))
				return false;
			if(attrishorts_help[q].compare(other.attrishorts_help[q]))
				return false;
		}
		if(script_name.compare(other.script_name))
			return false;
		if(author.compare(other.author))
			return false;
		return true;
	}
	bool operator!=(zasm_meta const& other) const
	{
		return !(*this == other);
	}
	
	bool parse_meta(const char *buffer);
	std::string get_meta() const;
};
ScriptType get_script_type(std::string const& name);
std::string get_script_name(ScriptType type);
#define D(n)               ((0x0000)+(n))
#define A(n)               ((0x0008)+(n))
#define GD(n)              ((0x02F5)+(n))

#define NUM_ZS_ARGS 3
struct ffscript
{
	word command;
	int32_t arg1, arg2, arg3;
	std::vector<int32_t> *vecptr;
	std::string *strptr;
	ffscript()
	{
		command = 0xFFFF;
		arg1 = 0;
		arg2 = 0;
		arg3 = 0;
		vecptr = nullptr;
		strptr = nullptr;
	}
	ffscript(word command, int32_t arg1 = 0, int32_t arg2 = 0, int32_t arg3 = 0): command(command), arg1(arg1), arg2(arg2), arg3(arg3)
	{
		vecptr = nullptr;
		strptr = nullptr;
	}
	~ffscript()
	{
		if(vecptr)
		{
			delete vecptr;
			vecptr = nullptr;
		}
		if(strptr)
		{
			delete strptr;
			strptr = nullptr;
		}
	}
	void give(ffscript& other)
	{
		other.command = command;
		other.arg1 = arg1;
		other.arg2 = arg2;
		other.arg3 = arg3;
		other.vecptr = vecptr;
		other.strptr = strptr;
		vecptr = nullptr;
		strptr = nullptr;
		clear();
	}
	void clear()
	{
		command = 0xFFFF;
		arg1 = 0;
		arg2 = 0;
		arg3 = 0;
		if(vecptr)
		{
			delete vecptr;
			vecptr = nullptr;
		}
		if(strptr)
		{
			delete strptr;
			strptr = nullptr;
		}
	}
	void copy(ffscript& other)
	{
		other.clear();
		other.command = command;
		other.arg1 = arg1;
		other.arg2 = arg2;
		other.arg3 = arg3;
		if(vecptr)
		{
			other.vecptr = new std::vector<int32_t>();
			for(int32_t val : *vecptr)
				other.vecptr->push_back(val);
		}
		if(strptr)
		{
			other.strptr = new std::string();
			for(char c : *strptr)
				other.strptr->push_back(c);
		}
	}
	
	bool operator==(ffscript const& other) const
	{
		//Compare primitive members
		if(command != other.command) return false;
		if(arg1 != other.arg1) return false;
		if(arg2 != other.arg2) return false;
		if(arg3 != other.arg3) return false;
		//Check for pointer existence differences
		if((vecptr==nullptr)!=(other.vecptr==nullptr)) return false;
		if((strptr==nullptr)!=(other.strptr==nullptr)) return false;
		//If both have a pointer, compare pointer size/contents
		if(vecptr)
		{
			if(vecptr->size() != other.vecptr->size())
				return false;
			if((*vecptr) != (*other.vecptr))
				return false;
		}
		if(strptr)
		{
			if(strptr->size() != other.strptr->size())
				return false;
			if(strptr->compare(*other.strptr))
				return false;
		}
		return true;
	}
	
	optional<dword> max_gvar() const;
};

struct script_id {
	auto operator<=>(const script_id&) const = default;

	ScriptType type;
	int index;
};

struct script_data
{
	ffscript* zasm;
	zasm_meta meta;
	script_id id;
	size_t size;
	bool optimized;
	
	void null_script(size_t newSize = 1)
	{
		if (newSize < 1)
			newSize = 1;
		if(zasm)
			delete[] zasm;
		zasm = new ffscript[newSize];
		zasm[0].clear();
		meta.zero();
		size = newSize;
		optimized = false;
	}
	
	bool valid() const
	{
		return (zasm && zasm[0].command != 0xFFFF);
	}
	
	void disable()
	{
		if(zasm)
		{
			zasm[0].clear();
			size = 1;
			optimized = false;
		}
	}
	
	void recalc_size()
	{
		if(zasm)
		{
			for(uint32_t q = 0;;++q)
			{
				if(zasm[q].command == 0xFFFF)
				{
					size = q+1;
					return;
				}
			}
		}

		size = 0;
	}
	
	void set(script_data const& other)
	{
		if(zasm)
			delete[] zasm;
		if(other.size)
		{
			zasm = new ffscript[other.size];
			for(size_t q = 0; q < other.size; ++q)
			{
				other.zasm[q].copy(zasm[q]);
			}
			size = other.size;
			optimized = other.optimized;
		}
		else
		{
			zasm = NULL;
			null_script();
		}
		meta = other.meta;
	}

	script_data(ScriptType type, int index) : zasm(NULL)
	{
		id = {type, index};
		null_script();
	}
	
	script_data(script_data const& other) : zasm(NULL)
	{
		set(other);
	}
	
	~script_data()
	{
		if(zasm)
			delete[] zasm;
	}
	
	void transfer(script_data& other)
	{
		other.meta = meta;
		if(other.zasm)
			delete[] other.zasm;
		other.zasm = zasm;
		other.size = size;
		other.optimized = optimized;
		zasm = NULL;
		null_script();
	}
	
	script_data& operator=(script_data const& other)
	{
		set(other);
		return *this;
	}
	
	bool equal_zasm(script_data const& other) const
	{
		if(size != other.size) return false;
		if(valid() != other.valid()) return false;
		for(auto q = 0; q < size; ++q)
		{
			if(zasm[q] != other.zasm[q]) return false;
		}
		return true;
	}
	
	bool operator==(script_data const& other) const
	{
		if(meta != other.meta) return false;
		return equal_zasm(other);
	}
};

enum
{
    ARGTY_UNUSED_REG,
    ARGTY_READ_REG,
    ARGTY_WRITE_REG,
    ARGTY_READWRITE_REG,
    ARGTY_LITERAL,
    ARGTY_COMPARE_OP,
    ARGTY_READ_GVAR,
    ARGTY_WRITE_GVAR,
};
#define ARGFL_COMPARE_USED 0x01
#define ARGFL_COMPARE_SET  0x02
#define ARGFL_UNIMPL       0x04
struct script_command
{
	string name;
	byte args;
	byte arg_type[3]; //ARGTY_
	byte arr_type; //0x1 = string, 0x2 = array
	byte flags; //ARGFL_

	bool is_register(int arg) const
	{
		return arg_type[arg] == ARGTY_READ_REG || arg_type[arg] == ARGTY_WRITE_REG || arg_type[arg] == ARGTY_READWRITE_REG;
	}

	bool writes_to_register(int arg) const
	{
		return arg_type[arg] == ARGTY_WRITE_REG || arg_type[arg] == ARGTY_READWRITE_REG;
	}
};

struct script_variable
{
    string name;
    int32_t id;
};

#endif

