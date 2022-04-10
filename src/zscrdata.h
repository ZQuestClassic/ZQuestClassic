#ifndef ZSCR_DATA_H
#define ZSCR_DATA_H

#include "zdefs.h"

using std::map;
using std::string;
using std::vector;

namespace ZScript
{
	enum ScriptTypeID
	{
		scrTypeIdInvalid,
		scrTypeIdStart,
		scrTypeIdGlobal = scrTypeIdStart,
		scrTypeIdFfc,
		scrTypeIdItem,
		scrTypeIdNPC,
		scrTypeIdEWeapon,
		scrTypeIdLWeapon,
		scrTypeIdPlayer,
		scrTypeIdScreen,
		scrTypeIdDMap,
		scrTypeIdItemSprite,
		scrTypeIdUntyped,
		scrTypeIdComboData,
		scrTypeIdSubscreenData,
		
		scrTypeIdEnd
	};
}

void read_compile_data(map<string, ZScript::ScriptTypeID>& stypes, map<string, disassembled_script_data>& scripts)
{
	stypes.clear();
	scripts.clear();
	size_t stypes_sz, scripts_sz;
	size_t dummy;
	ZScript::ScriptTypeID _id;
	char buf[512] = {0};
	char buf2[512] = {0};
	
	FILE *tempfile = fopen("tmp2","rb");
			
	if(!tempfile)
	{
		//jwin_alert("Error","Unable to open the temporary file in current directory!",NULL,NULL,"O&K",NULL,'k',0,lfont);
		return;
	}
	
	fread(&stypes_sz, sizeof(size_t), 1, tempfile);
	for(size_t ind = 0; ind < stypes_sz; ++ind)
	{
		fread(&dummy, sizeof(size_t), 1, tempfile);
		dummy = fread(buf, sizeof(char), dummy, tempfile);
		buf[dummy] = 0;
		fread(&_id, sizeof(ZScript::ScriptTypeID), 1, tempfile);
		stypes[buf] = _id;
	}
	
	fread(&scripts_sz, sizeof(size_t), 1, tempfile);
	for(size_t ind = 0; ind < scripts_sz; ++ind)
	{
		fread(&dummy, sizeof(size_t), 1, tempfile);
		dummy = fread(buf, sizeof(char), dummy, tempfile);
		buf[dummy] = 0;
		
		disassembled_script_data dsd;
		
		fread(&(dsd.first), sizeof(zasm_meta), 1, tempfile);
		
		fread(&(dsd.format), sizeof(byte), 1, tempfile);
		
		size_t tmp;
		fread(&tmp, sizeof(size_t), 1, tempfile);
		for(size_t ind2 = 0; ind2 < tmp; ++ind2)
		{
			fread(&dummy, sizeof(size_t), 1, tempfile);
			dummy = fread(buf2, sizeof(char), dummy, tempfile);
			buf2[dummy] = 0;
			int32_t lbl;
			fread(&lbl, sizeof(int32_t), 1, tempfile);
			std::shared_ptr<ZScript::Opcode> oc = std::make_shared<ZScript::ArbitraryOpcode>(string(buf2));
			oc->setLabel(lbl);
			dsd.second.push_back(oc);
		}
		
		scripts[buf] = dsd;
	}
	fclose(tempfile);
	
	/*
	reader->read(&stypes_sz, sizeof(size_t));
	for(size_t ind = 0; ind < stypes_sz; ++ind)
	{
		reader->read(&dummy, sizeof(size_t));
		reader->read(buf, dummy, &dummy);
		buf[dummy] = 0;
		reader->read(&_id, sizeof(ZScript::ScriptTypeID));
		stypes[buf] = _id;
	}
	
	reader->read(&scripts_sz, sizeof(size_t));
	for(size_t ind = 0; ind < scripts_sz; ++ind)
	{
		reader->read(&dummy, sizeof(size_t));
		reader->read(buf, dummy, &dummy);
		buf[dummy] = 0;
		
		disassembled_script_data dsd;
		
		reader->read(&(dsd.first), sizeof(zasm_meta));
		
		reader->read(&(dsd.format), sizeof(byte));
		
		size_t tmp;
		reader->read(&tmp, sizeof(size_t));
		for(size_t ind2 = 0; ind2 < tmp; ++ind2)
		{
			reader->read(&dummy, sizeof(size_t));
			reader->read(buf2, dummy, &dummy);
			buf2[dummy] = 0;
			int32_t lbl;
			reader->read(&lbl, sizeof(int32_t));
			std::shared_ptr<ZScript::Opcode> oc = std::make_shared<ZScript::ArbitraryOpcode>(string(buf2));
			oc->setLabel(lbl);
			dsd.second.push_back(oc);
		}
		
		scripts[buf] = dsd;
	}
	*/
}

void write_compile_data(map<string, ZScript::ScriptTypeID>& stypes, map<string, disassembled_script_data>& scripts)
{
	size_t dummy = stypes.size();
	FILE *tempfile = fopen("tmp2","wb");
			
	if(!tempfile)
	{
		//jwin_alert("Error","Unable to create a temporary file in current directory!",NULL,NULL,"O&K",NULL,'k',0,lfont);
		return;
	}
	
	fwrite(&dummy, sizeof(size_t), 1, tempfile);
	for(auto it = stypes.begin(); it != stypes.end(); ++it)
	{
		string const& str = it->first;
		ZScript::ScriptTypeID v = it->second;
		dummy = str.size();
		fwrite(&dummy, sizeof(size_t), 1, tempfile);
		fwrite((void*)str.c_str(), sizeof(char), dummy, tempfile);
		fwrite(&v, sizeof(ZScript::ScriptTypeID), 1, tempfile);
	}
	
	dummy = scripts.size();
	fwrite(&dummy, sizeof(size_t), 1, tempfile);
	for(auto it = scripts.begin(); it != scripts.end(); ++it)
	{
		string const& str = it->first;
		disassembled_script_data& v = it->second;
		dummy = str.size();
		fwrite(&dummy, sizeof(size_t), 1, tempfile);
		fwrite((void*)str.c_str(), sizeof(char), dummy, tempfile);
		
		fwrite(&(v.first), sizeof(zasm_meta), 1, tempfile);
		
		fwrite(&(v.format), sizeof(byte), 1, tempfile);
		
		dummy = v.second.size();
		fwrite(&dummy, sizeof(size_t), 1, tempfile);
		
		for(auto it = v.second.begin(); it != v.second.end(); ++it)
		{
			string opstr = (*it)->toString();
			int32_t lbl = (*it)->getLabel();
			
			dummy = opstr.size();
			fwrite(&dummy, sizeof(size_t), 1, tempfile);
			fwrite((void*)opstr.c_str(), sizeof(char), dummy, tempfile);
			
			fwrite(&lbl, sizeof(int32_t), 1, tempfile);
		}
	}
	
	//fwrite(zScript.c_str(), sizeof(char), zScript.size(), tempfile);
	fclose(tempfile);
	/*
	
	writer->write(&dummy, sizeof(size_t));
	for(auto it = stypes.begin(); it != stypes.end(); ++it)
	{
		string const& str = it->first;
		ZScript::ScriptTypeID v = it->second;
		dummy = str.size();
		writer->write(&dummy, sizeof(size_t));
		writer->write((void*)str.c_str(), dummy);
		writer->write(&v, sizeof(ZScript::ScriptTypeID));
	}
	
	dummy = scripts.size();
	writer->write(&dummy, sizeof(size_t));
	for(auto it = scripts.begin(); it != scripts.end(); ++it)
	{
		string const& str = it->first;
		disassembled_script_data& v = it->second;
		dummy = str.size();
		writer->write(&dummy, sizeof(size_t));
		writer->write((void*)str.c_str(), dummy);
		
		writer->write(&(v.first), sizeof(zasm_meta));
		
		writer->write(&(v.format), sizeof(byte));
		
		dummy = v.second.size();
		writer->write(&dummy, sizeof(size_t));
		
		for(auto it = v.second.begin(); it != v.second.end(); ++it)
		{
			string opstr = (*it)->toString();
			int32_t lbl = (*it)->getLabel();
			
			dummy = opstr.size();
			writer->write(&dummy, sizeof(size_t));
			writer->write((void*)opstr.c_str(), dummy);
			
			writer->write(&lbl, sizeof(int32_t));
		}
	}
	*/
}

#ifdef IS_PARSER
#include "parser/Compiler.h"
void write_compile_data(map<string, ZScript::ScriptType>& stypes, map<string, disassembled_script_data>& scripts)
{
	map<string, ZScript::ScriptTypeID> sid_types;
	for(auto it = stypes.begin(); it != stypes.end(); ++it)
	{
		sid_types[it->first] = (ZScript::ScriptTypeID)(it->second.getId());
	}
	write_compile_data(sid_types, scripts);
}

#endif //IS_PARSER

#endif

