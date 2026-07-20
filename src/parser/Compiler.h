#ifndef COMPILER_H_
#define COMPILER_H_

#include "CompilerUtils.h"
#include "Types.h"
#include "base/ints.h"
#include "parser/CompileError.h"
#include "parserDefs.h"
#include "base/headers.h"
#include <fmt/format.h>
#include "components/zasm/debug_data.h"
#include "zq/ffasm.h"
#include <nlohmann/json.hpp>

#include <cstdio>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

using std::unique_ptr;
using ordered_json = nlohmann::ordered_json;

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
	class Argument;

	// ZScript.h
	class Program;
	class FunctionSignature;
	class Script;

	// DataStructs.h
	struct FunctionData;
	struct IntermediateData;
	
	////////////////////////////////////////////////////////////////
	
	class Opcode
	{
	public:
		Opcode() : file(-1), line(-1), label(-1) {}
		virtual ~Opcode() {}
		virtual std::string toString() const = 0;
		int getLabel() const
		{
			return label;
		}
		void setLabel(int l)
		{
			label = l;
		}
		void setLocation(int file, int line)
		{
			this->file = file;
			this->line = line;
		}
		string const& getComment() const
		{
			return comment;
		}
		void setComment(string const& str)
		{
			comment = str;
		}
		static void mergeComment(string& basestr, string const& str, bool before = false)
		{
			if(str.empty())
				return;
			if(basestr.empty())
			{
				basestr = str;
				return;
			}
			if(before)
				basestr = fmt::format("{} AND {}",str,basestr);
			else basestr = fmt::format("{} AND {}",basestr,str);
		}
		void mergeComment(string const& str, bool before = false)
		{
			Opcode::mergeComment(comment, str, before);
		}
		void writeLine(std::string& out, bool showlabel = false, bool showcomment = true, std::string_view fname = "", int pc = 0)
		{
			size_t line_start_idx = out.size();
			if (showlabel && label > -1)
			{
				// "l{}:"
				out += 'l';

				fmt::format_int fLabel(label);
				out.append(fLabel.data(), fLabel.size());

				out += ':';
			}
			else
			{
				out += ' ';
			}

			out += toString();

			if (showcomment && !comment.empty())
			{
				out += "; ";
				out += comment;
			}

			if (showcomment && line > 0)
			{
				size_t current_len = out.size() - line_start_idx;
				if (current_len < 80)
					out.append(80 - current_len, ' ');

				// " | {} {}:{}\n"
				out += " | ";

				fmt::format_int fPc(pc);
				out.append(fPc.data(), fPc.size());

				out += ' ';
				out += fname;
				out += ':';

				fmt::format_int fLine(line);
				out.append(fLine.data(), fLine.size());

				out += '\n';
			}
			else
			{
				out += '\n';
			}
		}
		Opcode * makeClone(bool copylabel = true, bool copycomment = true)
		{
			Opcode *dup = clone();
			if(copylabel)
				dup->setLabel(label);
			if(copycomment)
				dup->setComment(comment);
			dup->file = file;
			dup->line = line;
			return dup;
		}
		virtual void execute(ArgumentVisitor&, void*) {}
	protected:
		virtual Opcode *clone() const = 0;
	public:
		int file;
		int line;
	private:
		int label;
		string comment;
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
		zasm_meta meta;
		uint32_t pc, end_pc;
		byte format;
		bounded_map<word, int32_t> script_d_init {MAX_SCRIPT_INST_VARIABLES, 0};
		bounded_map<word, exported_variable> script_d_exports {MAX_SCRIPT_INST_VARIABLES, {}};
		std::string formatName(std::string name)
		{
			return name;
		}
		disassembled_script_data() : format(SCRIPT_FORMAT_DEFAULT)
		{}
	};

	using LabelUsageIndex = std::vector<std::vector<int32_t*>>;

	inline LabelUsageIndex makeLabelUsageIndex(const std::vector<int32_t*>& vec)
	{
		LabelUsageIndex result;

		for (int32_t* ptr : vec)
		{
			if (!ptr) continue;

			int lbl_id = *ptr;
			if (lbl_id < 0) continue;

			if (lbl_id >= result.size())
				result.resize(lbl_id + 1024); // Resize with some padding

			result[lbl_id].push_back(ptr);
		}

		return result;
	}

	struct ZasmCompilerResult
	{
		std::vector<std::shared_ptr<ZScript::Opcode>> zasm;
		std::map<std::string, disassembled_script_data> theScripts;
		std::map<std::string, ZScript::ParserScriptType::Id> scriptTypes;
		DebugData debugData;
	};

	class ScriptAssembler
	{
	public:
		Program& program;
		
		ScriptAssembler(IntermediateData& id);
		void assemble();
		vector<shared_ptr<Opcode>>& getCode() {return rval;}
		DebugData& getDebugData() {return debug_data;}
		map<Script*,std::pair<int32_t,int32_t>>& getLabelMap() {return runlabels;}
		bool assemble_err;

	private:
		vector<shared_ptr<Opcode>>& ginit;

		vector<Function*> allFunctions;
		map<int32_t, Function*> functionsByLabel;
		vector<Function*> used_functions;
		vector<Function*> run_functions;
		vector<shared_ptr<Opcode>> rval;
		map<Script*,std::pair<int32_t,int32_t>> runlabels;
		// label id -> 1-indexed lines (aka pc + 1 of rval).
		map<int32_t, int32_t> linenos;
		vector<int32_t*> lbl_ptrs;
		vector<int32_t*> lbl_ptrs_no_scopes;
		std::set<int> scope_labels;
		LabelUsageIndex label_index;
		LabelUsageIndex label_index_no_scopes;
		// Temporarly hold deleted arguments, such that label_index never points to released memory.
		std::vector<std::unique_ptr<Argument>> argument_trash_bin;
		DebugData debug_data;

		void assemble_init();
		void assemble_script(Script* scr, Function* run_fn, string const& runsig);
		void assemble_scripts();
		void gather_run_labels();
		void link_functions();
		void gather_scope_labels();
		void optimize();
		void optimize_function(Function* fn);
		void optimize_code(vector<shared_ptr<Opcode>>& code, bool only_remove_nops = false);
		void output_code();
		void finalize_labels();
		void fill_debug_data();

		struct ScopeProcessingContext
		{
			DebugData& debug_data;
			std::map<const Scope*, int32_t> scope_ptr_to_index;
			std::map<int, int> enum_id_to_scope_index;
			std::set<Datum*> processed_datum;
			std::vector<Scope*> compiler_scopes;       // Index matches debug_data.scopes
			std::vector<const DataType*> scope_types;  // Index matches debug_data.scopes
			std::vector<const DataType*> symbol_types; // Index matches debug_data.symbols

			ScopeProcessingContext(DebugData& dd) : debug_data(dd) {}
		};

		void fill_debug_scopes();
		bool init_debug_scope(Scope* scope, DebugScope& dScope, const fs::path& cur_path, bool& isWithinFunc, bool& skip_emit);
		void resolve_debug_scope_ranges(Scope* scope, DebugScope& dScope);
		bool fill_debug_scope_locals(Scope* scope, int32_t scopeIdx, ScopeProcessingContext& ctx);
		void process_enum_definition(const DataTypeCustom* customType, int32_t parentIdx, ScopeProcessingContext& ctx);
		void finalize_debug_scopes(ScopeProcessingContext& ctx);
	};

	class ScriptsData
	{
	public:
		void fillFromAssembler(ScriptAssembler& assembler);

		bool success;
		ZasmCompilerResult zasmCompilerResult;
		// Just the errors/warnings for the main input script.
		std::vector<Diagnostic> diagnostics;
		ordered_json metadata;
		std::string docs;
	};

	unique_ptr<ScriptsData> compile(std::string const& filename, bool metadata_visitor, bool doc_visitor);

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
		static bool legacy_preprocess_one(ASTImportDecl& decl, int32_t reclevel);
		static bool legacy_preprocess(ASTFile* root, int32_t reclevel);
		static std::shared_ptr<ASTFile> parse_from_root(std::string entry_filename, std::string metadata_orig_path, std::string metadata_tmp_path);
		static unique_ptr<IntermediateData> generateOCode(FunctionData& fdata);
		static void initialize(bool has_qrs);
		static std::pair<int32_t,bool> parseLong(std::pair<std::string,std::string> parts);

		static int32_t const recursionLimit = 30;
	private:
		static bool valid_include(ASTImportDecl& decl, std::string& ret_fname);
		static std::string prepareFilename(std::string const& filename);
		static std::string* checkIncludes(std::string& includePath, std::string const& importname, std::vector<std::string> includes);
		static std::vector<std::shared_ptr<Opcode>> assembleOne(
				Program& program, std::vector<std::shared_ptr<Opcode>> script,
				int32_t numparams, FunctionSignature const& runsig);
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
