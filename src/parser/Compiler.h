#ifndef COMPILER_H
#define COMPILER_H

#include "CompilerUtils.h"
#include "Types.h"
#include "parserDefs.h"
#include "ffasmexport.h"
#include "ffasm.h"

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
	class ASTIncludePath;

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
		virtual std::string toString() const = 0;
		int32_t getLabel() const
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
        
			if(!showlabel || label == -1)
				return " " + toString() + "\n";
            
			sprintf(buf, "l%d:", label);
			return std::string(buf)+toString()+"\n";
		}
		Opcode * makeClone(bool copylabel = true)
		{
			Opcode *dup = clone();
			if(copylabel)
				dup->setLabel(label);
			return dup;
		}
		virtual void execute(ArgumentVisitor&, void*) {}
	protected:
		virtual Opcode *clone() const = 0;
	private:
		int32_t label;
	};

	class ArbitraryOpcode : public Opcode
	{
	public:
		ArbitraryOpcode(std::string data) : str(data) {}
		ArbitraryOpcode(char const* data) : str(data) {}
		std::string str;
		std::string toString() const
		{
			return str;
		}
		Opcode *clone() const
		{
			return new ArbitraryOpcode(str);
		}
	};

	struct disassembled_script_data
	{
		zasm_meta first;
		std::vector<std::shared_ptr<ZScript::Opcode>> second;
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
		void write(FILE* dest, bool al = false, bool spaced = false) const
		{
			std::string str = first.get_meta();
			if(spaced) fwrite("\n\n", sizeof(char), 2, dest);
			fwrite(str.c_str(), sizeof(char), str.size(), dest);
			if(al)
			{
				al_trace("\n\n");
				safe_al_trace(str);
				al_trace("\n");
			}
			for(auto& line : second)
			{
				str = line->printLine();
				if(al)
					safe_al_trace(str);
				fwrite(str.c_str(), sizeof(char), str.size(), dest);
			}
		}
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
		static bool valid_include(ASTImportDecl& decl, std::string& ret_fname);
		static std::string prepareFilename(std::string const& filename);
		static std::string* checkIncludes(std::string& includePath, std::string const& importname, std::vector<std::string> includes);
		static std::vector<std::shared_ptr<Opcode>> assembleOne(
				Program& program, std::vector<std::shared_ptr<Opcode>> script,
				int32_t numparams);
		static int32_t vid;
		static int32_t fid;
		static int32_t gid;
		static int32_t lid;
		static std::vector<std::string> includePaths;
	};

	class compile_exception : public std::exception
	{
	public:
		const char * what() const noexcept override {
			return msg.c_str();
		}
		compile_exception(std::string const& msg) : msg(msg)
		{}
	private:
		std::string msg;
	};
}

#endif

