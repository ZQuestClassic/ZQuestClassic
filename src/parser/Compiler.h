#ifndef COMPILER_H
#define COMPILER_H

// Prevent indirectly including windows.h
#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif

#include "CompilerUtils.h"
#include "Types.h"
#include "parserDefs.h"
#include "../ffasmexport.h"

#include <cstdio>
#include <map>
#include <memory>
#include <vector>
#include <string>
using std::unique_ptr;

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations

	// AST.h
	class ASTFile;
	class ASTImportDecl;

	// ByteCode.h
	class ArgumentVisitor;

	// ZScript.h
	class Program;

	// DataStructs.h
	struct FunctionData;
	struct IntermediateData;
	
	////////////////////////////////////////////////////////////////
	
	class Opcode
	{
	public:
		Opcode() : label(-1) {}
		virtual ~Opcode() {}
		virtual std::string toString()=0;
		int32_t getLabel()
		{
			return label;
		}
		void setLabel(int32_t l)
		{
			label=l;
		}
		std::string printLine(bool showlabel = false)
		{
			char buf[100];
        
			if(label == -1)
				return " " + toString() + "\n";
            
			sprintf(&buf[0], "l%d:", label);
			return (showlabel ? std::string(&buf[0]) : " ")+ toString() + "\n";
		}
		Opcode * makeClone()
		{
			Opcode *dup = clone();
			dup->setLabel(label);
			return dup;
		}
		virtual void execute(ArgumentVisitor&, void*) {}
	protected:
		virtual Opcode *clone()=0;
	private:
		int32_t label;
	};

	class ArbitraryOpcode : public Opcode
	{
	public:
		ArbitraryOpcode(std::string data) : str(data) {}
		std::string str;
		std::string toString()
		{
			return str;
		}
		Opcode *clone()
		{
			return new ArbitraryOpcode(str);
		}
	};

	struct disassembled_script_data
	{
		zasm_meta first;
		std::vector<ZScript::Opcode*> second;
		byte format;
		std::string formatName(std::string name)
		{
			char buf[64];
			std::string fmt = "%s";
			switch(format)
			{
				case SCRIPT_FORMAT_DISASSEMBLED:
					fmt = "++%s";
					break;
				case SCRIPT_FORMAT_ZASM:
					fmt = "==%s";
					break;
			}
			sprintf(buf, fmt.c_str(), name.c_str());
			return std::string(buf);
		}
		disassembled_script_data() : format(SCRIPT_FORMAT_DEFAULT)
		{}
	};

	class ScriptsData
	{
	public:
		ScriptsData(Program&);
		std::map<std::string, disassembled_script_data> theScripts;
		std::map<std::string, ScriptType> scriptTypes;
	};

	unique_ptr<ScriptsData> compile(std::string const& filename);

	class ScriptParser
	{
	public:
		static int32_t getUniqueVarID()
		{
			return vid++;
		}
		static int32_t getUniqueFuncID()
		{
			return fid++;
		}
		static int32_t getUniqueLabelID()
		{
			return lid++;
		}
		static int32_t getUniqueGlobalID()
		{
			return gid++;
		}
		static bool preprocess_one(ASTImportDecl& decl, int32_t reclevel);
		static bool preprocess(ASTFile* root, int32_t reclevel);
		static unique_ptr<IntermediateData> generateOCode(FunctionData& fdata);
		static void assemble(IntermediateData* id);
		static void initialize();
		static std::pair<int32_t,bool> parseLong(
				std::pair<std::string,std::string> parts, Scope* scope);

		static int32_t const recursionLimit = 30;
	private:
		static std::string prepareFilename(std::string const& filename);
		static std::vector<Opcode *> assembleOne(
				Program& program, std::vector<Opcode*> script,
				int32_t numparams);
		static int32_t vid;
		static int32_t fid;
		static int32_t gid;
		static int32_t lid;
	};
}

#endif

