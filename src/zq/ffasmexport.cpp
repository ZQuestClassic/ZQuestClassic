
#include <string.h>

#include "zq/ffasmexport.h"
#include "zq/ffasm.h"
#include "base/zdefs.h"

using namespace std;

// TODO: use fmt::format
std::string varToString(int32_t arg)
{
	for(int32_t q = 0; variable_list[q].id != -1; ++q)
	{
		if(variable_list[q].maxcount>0)
		{
			int32_t start = variable_list[q].id;
			int32_t mult = zc_max(1,variable_list[q].multiple);
			if(arg >= start && arg < start+(variable_list[q].maxcount*mult))
			{
				for(int32_t w = 0; w < variable_list[q].maxcount; ++w)
				{
					if(arg!=start+(w*mult)) continue;
					
					char buf[80];
					if(strcmp(variable_list[q].name, "A")==0)
						sprintf(buf, "%s%d", variable_list[q].name, w+1);
					else sprintf(buf, "%s%d", variable_list[q].name, w);
					return string(buf);
				}
			}
		}
		else if(variable_list[q].id == arg) return string(variable_list[q].name);
	}
	return "(null)";
}

string getOpcodeString(ffscript const* line)
{
	script_command s_c = command_list[line->command];
	//al_trace("%s - %ld - %ld\n", s_c.name, line->arg1, line->arg2);
	char buf[600];
	char a1buf[256];
	char a2buf[256];
	if(s_c.args == 2)
	{
		if(s_c.arg1_type == 0)
		{
			if(s_c.arg2_type == 0)
			{
				sprintf(buf, "%s %s,%s", s_c.name, varToString(line->arg1).c_str(), varToString(line->arg2).c_str());
			}
			else
			{
				if(line->arg2 % 10000 != 0) sprintf(a2buf, "%d.%04d", line->arg2 / 10000, line->arg2 < 0 ? -(line->arg2) : line->arg2);
				else sprintf(a2buf, "%d", line->arg2/10000);
				sprintf(buf, "%s %s,%s", s_c.name, varToString(line->arg1).c_str(), a2buf);
			}
		}
		else
		{
			if(s_c.arg2_type == 0)
			{
				if(line->arg1 % 10000 != 0) sprintf(a1buf, "%d.%04d", line->arg1 / 10000, line->arg1 < 0 ? -(line->arg1) : line->arg1);
				else sprintf(a1buf, "%d", line->arg1/10000);
				sprintf(buf, "%s %s,%s", s_c.name, a1buf, varToString(line->arg2).c_str());
			}
			else
			{
				if(line->arg1 % 10000 != 0) sprintf(a1buf, "%d.%04d", line->arg1 / 10000, line->arg1 < 0 ? -(line->arg1) : line->arg1);
				else sprintf(a1buf, "%d", line->arg1/10000);
				if(line->arg2 % 10000 != 0) sprintf(a2buf, "%d.%04d", line->arg2 / 10000, line->arg2 < 0 ? -(line->arg2) : line->arg2);
				else sprintf(a2buf, "%d", line->arg2/10000);
				sprintf(buf, "%s %s,%s", s_c.name, a1buf, a2buf);
			}
		}
	}
	else if(s_c.args == 1)
	{
		if(s_c.arg1_type == 0)
		{
			sprintf(buf, "%s %s", s_c.name, varToString(line->arg1).c_str());
		}
		else
		{
			if(line->arg1 % 10000 != 0) sprintf(a1buf, "%d.%04d", line->arg1 / 10000, line->arg1 < 0 ? -(line->arg1) : line->arg1);
			else sprintf(a1buf, "%d", line->arg1/10000);
			sprintf(buf, "%s %s", s_c.name, a1buf);
		}
	}
	else
		sprintf(buf, "%s", s_c.name);
	return string(buf);
}

disassembled_script_data disassemble_script(script_data const* script)
{
	// al_trace("DISASSEMBLY:\n");
	ffscript const* zasm = script->zasm;
	disassembled_script_data data;
	data.first = script->meta;
	for(int32_t lineCount = 0; zasm[lineCount].command != 0xFFFF; ++lineCount)
	{
		shared_ptr<ZScript::Opcode> op(new ZScript::ArbitraryOpcode(getOpcodeString(&zasm[lineCount])));
		data.second.push_back(op);
	}
	return data;
}

void write_script(FILE* dest, disassembled_script_data const& data)
{
	string meta_str = data.first.get_meta();
	fwrite(meta_str.c_str(), sizeof(char), meta_str.size(), dest);
	for(auto line = data.second.begin(); line != data.second.end(); ++line)
	{
		string theline = (*line)->printLine();
		fwrite(theline.c_str(), sizeof(char), theline.size(), dest);
	}
}

void write_script(FILE* dest, script_data const* script)
{
	write_script(dest, disassemble_script(script));
}

