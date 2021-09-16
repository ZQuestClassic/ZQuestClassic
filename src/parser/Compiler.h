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

#include <boost/move/unique_ptr.hpp>

#include <cstdio>
#include <map>
#include <vector>
#include <string>

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
		int getLabel()
		{
			return label;
		}
		void setLabel(int l)
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
		int label;
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

	boost::movelib::unique_ptr<ScriptsData> compile(std::string const& filename);

	class ScriptParser
	{
	public:
		static int getUniqueVarID()
		{
			return vid++;
		}
		static int getUniqueFuncID()
		{
			return fid++;
		}
		static int getUniqueLabelID()
		{
			return lid++;
		}
		static int getUniqueGlobalID()
		{
			return gid++;
		}
		static bool preprocess_one(ASTImportDecl& decl, int reclevel);
		static bool preprocess(ASTFile* root, int reclevel);
		static boost::movelib::unique_ptr<IntermediateData> generateOCode(FunctionData& fdata);
		static void assemble(IntermediateData* id);
		static void initialize();
		static std::pair<long,bool> parseLong(
				std::pair<std::string,std::string> parts, Scope* scope);

		static int const recursionLimit = 30;
	private:
		static std::string prepareFilename(std::string const& filename);
		static std::vector<Opcode *> assembleOne(
				Program& program, std::vector<Opcode*> script,
				int numparams);
		static int vid;
		static int fid;
		static int gid;
		static int lid;
	};
}

#endif

