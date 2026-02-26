#include "base/check.h"
#include "base/util.h"
#include "parser/AST.h"
#include "parser/Compiler.h"
#include "parser/CompilerUtils.h"
#include "parser/DocVisitor.h"
#include "parser/MetadataVisitor.h"
#include "parser/Opcode.h"
#include "parser/Types.h"
#include "parser/owning_vector.h"
#include "parser/parserDefs.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/defines.h"
#include "zsyssimple.h"
#include "ByteCode.h"
#include "CompileError.h"
#include "CompileOption.h"
#include "LibrarySymbols.h"
#include "y.tab.hpp"
#include <algorithm>
#include <filesystem>
#include <assert.h>
#include <cstdlib>
#include <optional>
#include <string>
#include <memory>

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Scope.h"
#include "SemanticAnalyzer.h"
#include "BuildVisitors.h"
#include "RegistrationVisitor.h"
#include "ReturnVisitor.h"
#include "ZScript.h"
#include <fmt/format.h>
#include <tuple>
#include <type_traits>

#ifdef HAS_SENTRY
#define SENTRY_BUILD_STATIC 1
#include "sentry.h"
#endif

using std::unique_ptr;
using std::shared_ptr;
using namespace ZScript;

#include <allegro/alcompat.h>
#if (DEVICE_SEPARATOR != 0) && (DEVICE_SEPARATOR != '\0')
#define HAVE_DIR_LIST
#endif
static void get_root_path(char* path, int32_t size)
{
#ifdef HAVE_DIR_LIST
	int32_t drive = _al_getdrive();
#else
	int32_t drive = 0;
#endif

    _al_getdcwd(drive, path, size - ucwidth(OTHER_PATH_SEPARATOR));
	fix_filename_case(path);
	fix_filename_slashes(path);
	put_backslash(path);
}

static std::filesystem::path derelativize_path(std::string src_path)
{
	char rootpath[PATH_MAX] = {0};
	get_root_path(rootpath, PATH_MAX);
	return (std::filesystem::path(rootpath) / src_path).lexically_normal();
}


extern std::vector<string> ZQincludePaths;

void ScriptParser::initialize(bool has_qrs)
{
	vid = 0;
	fid = 0;
	gid = 1;
	lid = 0;
	CompileError::initialize();
	CompileOption::initialize(has_qrs);
	includePaths.clear();
	includePaths.resize(0);
}

extern std::string input_script_filename;
extern std::string actual_input_script_filename;
extern std::string metadata_tmp_path;
extern uint32_t zscript_failcode;
extern std::vector<Diagnostic>* current_diagnostics;
extern bool zscript_error_out;
extern bool delay_asserts, ignore_asserts;
vector<ZScript::BasicCompileError> casserts;

static void _fill_metadata(std::string filename, Program* program, ScriptsData* result)
{
	zscript_error_out = false;
	MetadataVisitor md(*program, filename);
	if (zscript_error_out || md.hasFailed()) return;

	result->metadata = md.takeOutput();
}

static unique_ptr<ScriptsData> _compile_helper(string const& filename, bool include_metadata, bool include_docs)
{
	using namespace ZScript;

	input_script_filename = metadata_tmp_path.empty() ? filename : metadata_tmp_path;
	actual_input_script_filename = filename;
	zscript_failcode = 0;
	zscript_error_out = false;
	if(ignore_asserts) delay_asserts = true;
	casserts.clear();
	try
	{
		auto result = std::make_unique<ScriptsData>();
		current_diagnostics = &result->diagnostics;

		zconsole_info("%s", "Pass 1: Parsing");
		zconsole_idle();

		extern std::string metadata_orig_path;
		extern std::string metadata_tmp_path;
		shared_ptr<ASTFile> root(ScriptParser::parse_from_root(filename, metadata_orig_path, metadata_tmp_path));
		if (zscript_error_out || !root) return result;

		zconsole_info("%s", "Pass 2: Preprocessing");
		zconsole_idle();

		SimpleCompileErrorHandler handler(result.get());
		Program program(*root, &handler);

		if (handler.hasError())
			return result;
		if(zscript_error_out) return result;

		zconsole_info("%s", "Pass 3: Registration");
		zconsole_idle();

		RegistrationVisitor regVisitor(program);
		if (zscript_error_out || regVisitor.hasFailed())
		{
			if (include_metadata)
				_fill_metadata(filename, &program, result.get());
			return result;	
		} 

		zconsole_info("%s", "Pass 4: Analyzing Code");
		zconsole_idle();

		// TODO: figure out how to make this continue even after a strict error, for purposes
		// of metadata. Otherwise can't do 'Go To Definition' past the first strict error.
		SemanticAnalyzer semanticAnalyzer(program);
		if (zscript_error_out || semanticAnalyzer.hasFailed() || regVisitor.hasFailed())
		{
			if (include_metadata)
				_fill_metadata(filename, &program, result.get());
			return result;
		}

		FunctionData fd(program);
		if(zscript_error_out) return result;
		if (fd.globalVariables.size() > MAX_SCRIPT_REGISTERS)
		{
			log_error(CompileError::TooManyGlobal(NULL));
			if (include_metadata)
				_fill_metadata(filename, &program, result.get());
			return result;
		}

		zconsole_info("%s", "Pass 5: Checking code paths");
		zconsole_idle();
		
		ReturnVisitor rv(program);
		if (zscript_error_out || rv.hasFailed())
		{
			if (include_metadata)
				_fill_metadata(filename, &program, result.get());
			return result;
		}

		if (do_codegen())
		{
			zconsole_info("%s", "Pass 6: Generating object code");
			zconsole_idle();
	
			unique_ptr<IntermediateData> id(ScriptParser::generateOCode(fd));
			if (zscript_error_out || !id.get())
			{
				if (include_metadata)
					_fill_metadata(filename, &program, result.get());
				return result;
			}
	
			zconsole_info("%s", "Pass 7: Assembling");
			zconsole_idle();
			
			ScriptAssembler sa(*id.get());
			sa.assemble();
			if (sa.assemble_err)
			{
				if (include_metadata)
					_fill_metadata(filename, &program, result.get());
				return result;
			}
	
			result->fillFromAssembler(sa);
			if (zscript_error_out || (!ignore_asserts && casserts.size()))
			{
				return result;
			}
		}


		if (include_metadata)
			_fill_metadata(filename, &program, result.get());

		if (include_docs)
		{
			DocVisitor doc(program);
			if (zscript_error_out || doc.hasFailed())
				return result;
			result->docs = doc.getOutput();
		}

		zconsole_info("%s", "Success!");
		result->success = true;
		return result;
	}
	catch (compile_exception &e)
	{
		std::string error = fmt::format("An unexpected compile error has occurred:\n{}", e.what());
		zconsole_error(error);
		zscript_error_out = true;

		auto result = std::make_unique<ScriptsData>();
		result->success = false;
		Diagnostic diag{};
		diag.severity = DiagnosticSeverity::Error;
		diag.message = error;
		result->diagnostics.push_back(diag);
		return result;
	}
#ifndef _DEBUG
	catch (std::exception &e)
	{
#ifdef HAS_SENTRY
		sentry_value_t event = sentry_value_new_event();
		sentry_value_t exc = sentry_value_new_exception("Parser Runtime Error", e.what());
		sentry_value_set_stacktrace(exc, NULL, 0);
		sentry_event_add_exception(event, exc);
		sentry_capture_event(event);
#endif

		std::string error = fmt::format("An unexpected runtime error has occurred:\n{}", e.what());
		zconsole_error(error);
		zscript_error_out = true;

		auto result = std::make_unique<ScriptsData>();
		result->success = false;
		Diagnostic diag{};
		diag.severity = DiagnosticSeverity::Error;
		diag.message = error;
		result->diagnostics.push_back(diag);
		return result;
	}
#endif
}
unique_ptr<ScriptsData> ZScript::compile(string const& filename, bool metadata_visitor, bool doc_visitor)
{
	DataType::STRING = DataTypeArray::create(DataType::CHAR);

	auto ret = _compile_helper(filename, metadata_visitor, doc_visitor);

	DataTypeArray::created_arr_types.clear();

	if(!ignore_asserts)
		for(BasicCompileError const& error : casserts)
			error.handle();
	return ret;
}

int32_t ScriptParser::vid = 0;
int32_t ScriptParser::fid = 0;
int32_t ScriptParser::gid = 1;
int32_t ScriptParser::lid = 0;

string ScriptParser::prepareFilename(string const& filename)
{
	string retval = filename;

	util::regulate_path(retval);
	return retval;
}

vector<string> ScriptParser::includePaths;

string& cleanInclude(string& includePath)
{
	//Add a `/` to the end of the include path, if it is missing
	int32_t lastnot = includePath.find_last_not_of("/\\");
	int32_t last = includePath.find_last_of("/\\");
	if(lastnot != string::npos)
	{
		if(last == string::npos || last < lastnot)
			includePath += "/";
	}
	util::regulate_path(includePath);
	return includePath;
}

string* ScriptParser::checkIncludes(string& includePath, string const& importname, vector<string> includes)
{
	for (size_t q = 0; q < includes.size(); ++q ) //Loop through all include paths, or until valid file is found
	{
		includePath = includes.at(q);
		cleanInclude(includePath);
		includePath = prepareFilename(includePath + importname);
		FILE* f = fopen(includePath.c_str(), "r");
		if(!f) continue;
		fclose(f);
		return &includePath;
	}
	return NULL;
}

extern std::vector<std::filesystem::path> force_ignores;
bool ScriptParser::valid_include(ASTImportDecl& decl, string& ret_fname)
{
	if(decl.wasValidated())
	{
		ret_fname = decl.getFilename();
		return true;
	}
	string* fname = NULL;
	string includePath;
	string importname = prepareFilename(decl.getFilename());
	if(!decl.isInclude()) //Check root dir first for imports
	{
		FILE* f = fopen(importname.c_str(), "r");
		if(f)
		{
			fclose(f);
			fname = &importname;
		}
	}
	if(!fname)
	{
		// Scan include paths
		auto ss = std::filesystem::path(importname);
		if (std::filesystem::path(importname).is_absolute())
		{
			fname = &importname;
		}
		else
		{
			int32_t importfound = importname.find_first_not_of("/\\");
			if(importfound != string::npos) //If the import is not just `/`'s and `\`'s...
			{
				if(importfound != 0)
					importname = importname.substr(importfound); //Remove leading `/` and `\`
				//Convert the include string to a proper import path
				fname = checkIncludes(includePath, importname, ZQincludePaths);
				if(!fname)
				{
					fname = checkIncludes(includePath, importname, includePaths);
				}
			}
		}
	}
	string filename = fname ? *fname : prepareFilename(importname); //Check root dir last, if nothing has been found yet.
	ret_fname = filename;
	//Note: If the user gives an absolute path, `relpath` will be that absolute path!
	std::filesystem::path relpath = std::filesystem::path(filename).lexically_normal();
	std::filesystem::path abspath = derelativize_path(filename);
	FILE* f = fopen(abspath.string().c_str(), "r");
	if (!f)
		return false;

	fclose(f);
	decl.setFilename(abspath.string());
	ret_fname = abspath.string();

	if (std::find(force_ignores.begin(), force_ignores.end(), abspath) != force_ignores.end())
	{
		decl.disable();
		return true;
	}

	decl.validate();
	return true;
}

std::shared_ptr<ASTFile> ScriptParser::parse_from_root(std::string entry_filename, std::string metadata_orig_path, std::string metadata_tmp_path)
{
	std::map<std::string, std::shared_ptr<ASTFile>> parsed_files_cache;

	auto root_loc = LOC_NONE;
	root_loc.fname = "<root>";
	auto root_file = std::make_shared<ASTFile>(root_loc);

	// First parse the entry file. Do this now so we can early exit upon failure without parsing everything else.
	auto entry_file = parseFile(entry_filename);
	if (!entry_file)
	{
		log_error(CompileError::CantOpenSource(NULL));
		return nullptr;
	}
	parsed_files_cache[entry_filename] = entry_file;

	root_file->imports.insert(root_file->imports.begin(), new ASTImportDecl(new ASTString("bindings.zh")));
	root_file->imports.push_back(new ASTImportDecl(new ASTString(entry_filename)));

	// We're going to process the import graph via depth-first search, but iteratively. In order to process imports
	// in the order files are imported in source, a stack of iterators is used. This is equivalent to using recursion.
	// See https://en.m.wikipedia.org/wiki/Depth-first_search#Pseudocode for more.
	//
	// Expected parse order:
	// 1. The entry file (but just that, no children; for early exit on failure)
	// 2. The binding files (and all children, pre-order traversal)
	// 3. The entry file's children, pre-order traversal
	//
	// pre-order traversal - fancy way to say: when an import is processed parse it, then parse all its children before
	// processing the next import. See https://skilled.dev/course/tree-traversal-in-order-pre-order-post-order
	std::vector<std::tuple<ASTFile*, owning_vector<ASTImportDecl>::iterator>> stack;

	// Initialize the stack.
	stack.push_back({entry_file.get(), entry_file->imports.begin()});
	stack.push_back({root_file.get(), root_file->imports.begin()});

	bool success = true;
	while (!stack.empty())
	{
		auto& [file, importIt] = stack.back();

		if (importIt == file->imports.begin())
		{
			// Add include paths (ignoring duplicates).
			for (auto it : file->inclpaths)
			{
				std::string path = cleanInclude(it->path);

				bool dupe = false;
				for (auto it2 : includePaths)
				{
					if (!strcmp(it2.c_str(), path.c_str()))
					{
						dupe = true;
						break;
					}
				}

				if (!dupe)
					includePaths.push_back(path);
			}
		}

		if (importIt == file->imports.end())
		{
			stack.pop_back();
			continue;
		}

		auto importDecl = *(importIt++);
		if (importDecl->isDisabled())
			continue;

		std::string filename;
		if (!valid_include(*importDecl, filename))
		{
			log_error(CompileError::CantOpenImport(importDecl, filename));
			success = false;
			continue;
		}

		std::shared_ptr<ASTFile> imported_file;
		if (!parsed_files_cache.contains(filename))
		{
			imported_file = parseFile(filename);
			if (!imported_file)
			{
				importDecl->disable();
				success = false;
				log_error(CompileError::CantParseImport(importDecl, filename));
				continue;
			}

			parsed_files_cache[filename] = imported_file;
			stack.push_back({imported_file.get(), imported_file->imports.begin()});
			if (!metadata_orig_path.empty() && !metadata_tmp_path.empty() && filename == metadata_tmp_path)
				parsed_files_cache[metadata_orig_path] = imported_file;
		}

		importDecl->giveTree(parsed_files_cache[filename]);
	}

	return success ? root_file : nullptr;
}

bool ScriptParser::legacy_preprocess_one(ASTImportDecl& importDecl, int32_t reclimit)
{
	string filename;
	if(!valid_include(importDecl, filename))
	{
		log_error(CompileError::CantOpenImport(&importDecl, filename));
		return false;
	}
	if(importDecl.isDisabled()) return true;

	auto imported(parseFile(filename));
	if (!imported.get())
	{
		log_error(CompileError::CantParseImport(&importDecl, filename));
		return false;
	}

	// Save the AST in the import declaration.
	importDecl.giveTree(imported);

	// Recurse on imports.
	if (!legacy_preprocess(importDecl.getTree(), reclimit - 1))
		return false;

	return true;
}

// Don't use this. Only used for conditional imports.
// TODO: refactor caseImportCondDecl to use parse_from_root (and keep the cache around).
bool ScriptParser::legacy_preprocess(ASTFile* root, int32_t reclimit)
{
	assert(root);
	
	if (reclimit == 0)
	{
		auto error = CompileError::ImportRecursion(NULL, recursionLimit);
		log_error(error);
		throw compile_exception(error.toString());
	}
	for(auto it = root->inclpaths.begin();
	     it != root->inclpaths.end(); ++it)
	{
		bool dupe = false;
		string& path = cleanInclude((*it)->path);
		for(auto it2 = includePaths.begin();
			it2 != includePaths.end(); ++it2)
		{
			if(!strcmp((*it2).c_str(), path.c_str()))
			{
				dupe = true;
				break;
			}
		}
		if(!dupe)
			includePaths.push_back(path);
	}
	// Repeat parsing process for each of import files
	bool ret = true;
	for (auto it = root->imports.begin();
	     it != root->imports.end(); ++it)
	{
		if(!legacy_preprocess_one(**it, reclimit)) ret = false;
	}

	return ret;
}

static void setFunctionScopeLabels(Function* fn)
{
	auto& code = fn->getCode();
	auto* scope = fn->getInternalScope();
	if (!code.empty())
	{
		if (int label = code.front()->getLabel(); label != -1)
		{
			scope->start_label = label;
		}
		else
		{
			scope->start_label = ScriptParser::getUniqueLabelID();
			code.front()->setLabel(scope->start_label);
		}

		if (int label = code.back()->getLabel(); label != -1)
		{
			scope->end_label = label;
		}
		else
		{
			scope->end_label = ScriptParser::getUniqueLabelID();
			code.back()->setLabel(scope->end_label);
		}
	}
}

unique_ptr<IntermediateData> ScriptParser::generateOCode(FunctionData& fdata)
{
	Program& program = fdata.program;
	Scope* scope = &program.getScope();
	TypeStore* typeStore = &program.getTypeStore();
	vector<Datum*>& globalVariables = fdata.globalVariables;

	// Z_message("yes");
	bool failure = false;

	//we now have labels for the functions and ids for the global variables.
	//we can now generate the code to intialize the globals
	unique_ptr<IntermediateData> rval(new IntermediateData(fdata));

	// Push 0s for init stack space.
	/* Why? The stack should already be init'd to all 0, anyway?
	rval->globalsInit.push_back(
			new OSetImmediate(new VarArgument(EXP1),
			                  new LiteralArgument(0)));
	int32_t globalStackSize = *program.getScope().getRootStackSize();
	for (int32_t i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPushRegister(new VarArgument(EXP1)));*/

	// Generate variable init code.
	for (vector<Datum*>::iterator it = globalVariables.begin();
	     it != globalVariables.end(); ++it)
	{
		Datum& variable = **it;
		AST& node = *variable.getNode();
		
		CleanupVisitor cv(program, scope);
		node.execute(cv);

		OpcodeContext oc(typeStore);

		BuildOpcodes bo(program, scope);
		bo.visit(node, &oc);
		if (bo.hasError()) failure = true;
		appendElements(rval->globalsInit, oc.initCode);
		appendElements(rval->globalsInit, bo.getResult());
	}

	// Pop off everything.
	/* See above; why push this in the first place?
	for (int32_t i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPopRegister(new VarArgument(EXP2)));*/
	
	//Parse the indexes for class variables
	for(UserClass* user_class : program.classes)
	{
		user_class->getScope().parse_ucv();
	}
	
	//globals have been initialized, now we repeat for the functions
	vector<Function*> funs = program.getUserFunctions();
	appendElements(funs, program.getUserClassConstructors());
	appendElements(funs, program.getUserClassDestructors());

	for (auto fn : funs)
	{
		if (fn->isTemplateSkip())
		{
			for (auto& fun : fn->get_applied_funcs())
				funs.push_back(fun.get());
		}
	}

	// Inlined functions are included in the generated code for potential use by the debugger.
	// However, they must be processed last.
	//
	// The following loop reassigns each function's code to include a function header/footer.
	// Therefore all inline callsites should be processed first, otherwise they would accidentally
	// emit RETURN, etc.
	//
	// Note that if inlined functions call other inlined functions, this order isn't sufficient -
	// but currently only internal binding functions are inlined so this is not a problem.
	if (debug_data_should_emit_inlined_functions())
	{
		std::vector<Function*> inline_functions;
		for (auto it = funs.begin(); it != funs.end();)
		{
			Function* fn = *it;
			if (fn->getFlag(FUNCFLAG_INLINE))
			{
				inline_functions.push_back(fn);
				it = funs.erase(it);
				continue;
			}

			it++;
		}
		for (auto fn : inline_functions)
			funs.push_back(fn);
	}

	for (vector<Function*>::iterator it = funs.begin(); it != funs.end(); ++it)
	{
		Function& function = **it;
		if(function.is_aliased())
			continue;
		if(function.isTemplateSkip())
			continue;

		bool classfunc = function.getFlag(FUNCFLAG_CLASSFUNC) && !function.getFlag(FUNCFLAG_STATIC);
		int puc = 0;
		if(classfunc)
		{
			if(function.getFlag(FUNCFLAG_CONSTRUCTOR))
				puc = puc_construct;
			else if(function.getFlag(FUNCFLAG_DESTRUCTOR))
				puc = puc_destruct;
			else puc = puc_funcs;
		}

		if(function.getFlag(FUNCFLAG_INLINE) && !debug_data_should_emit_inlined_functions()) continue; //Skip inline func decls, they are handled at call location -V
		if(puc != puc_construct && function.prototype) continue; //Skip prototype func decls, they are ALSO handled at the call location -V
		if (function.isInternal()) continue;

		ASTFuncDecl& node = *function.node;

		bool isRun = ZScript::isRun(function);
		string scriptname;
		Script* functionScript = function.getScript();
		if (functionScript)
		{
			scriptname = functionScript->getName();
		}
		scope = function.getInternalScope();

		setLocation2(program, &node);

		if (node.isBinding())
		{
			CHECK(function.getFlag(FUNCFLAG_INLINE));

			auto code = function.takeCode();
			auto op = new OReturnFunc();
			int returnlabelid = ScriptParser::getUniqueLabelID();
			op->setLabel(returnlabelid);
			addOpcode2(code, new OReturnFunc());
			function.giveCode(code);
		}
		else if (classfunc)
		{
			UserClass& user_class = scope->getClass()->user_class;
			
			vector<std::shared_ptr<Opcode>> funccode;
			
			int32_t stackSize = getStackSize(function);
			// Start of the function.
			if (puc == puc_construct)
			{
				vector<Function*> destr = user_class.getScope().getDestructor();
				Opcode* first = nullptr;
				Function* destructor = destr.size() == 1 ? destr.at(0) : nullptr;
				if(destructor && !destructor->isNil())
				{
					Function* destructor = destr[0];
					first = new OSetImmediateLabel(new VarArgument(EXP1), new LabelArgument(destructor->getLabel(), true));
				}
				else
					first = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));

				first->setLabel(function.getLabel());
				addOpcode2(funccode, first);
				addOpcode2(funccode, new OSetRegister(new VarArgument(CLASS_THISKEY2),new VarArgument(CLASS_THISKEY)));
				addOpcode2(funccode, new OConstructClass(new VarArgument(CLASS_THISKEY),
					new VectorArgument(user_class.members)));
				std::vector<int> object_indices;
				for (auto&& member : user_class.getScope().getClassData())
				{
					auto& type = member.second->getNode()->resolveType(scope, nullptr);
					if (type.isObject())
					{
						object_indices.push_back(member.second->getIndex());

						script_object_type object_type;
						if (type.isArray())
							object_type = static_cast<const DataTypeArray*>(&type)->getElementType().getScriptObjectTypeId();
						else
							object_type = type.getScriptObjectTypeId();
						object_indices.push_back((int)object_type);
					}
				}
				if (!object_indices.empty())
					addOpcode2(funccode, new OMarkTypeClass(new VectorArgument(object_indices)));

				addOpcode2(funccode, new ORefInc(new LiteralArgument(-1))); // retain this
				addOpcode2(funccode, new ONoOp(function.getAltLabel()));
			}
			else if(puc == puc_destruct)
			{
				Opcode* first = new ODestructor(new StringArgument(user_class.getName()));
				first->setLabel(function.getLabel());
				addOpcode2(funccode, first);
			}
			else
			{
				addOpcode2(funccode, new ONoOp(function.getLabel()));
				addOpcode2(funccode, new ORefInc(new LiteralArgument(-1))); // retain this
			}
			// Push 0s for the local variables.
			for (int32_t i = stackSize - getParameterCount(function); i > 0; --i)
				addOpcode2(funccode, new OPushImmediate(new LiteralArgument(0)));
			
			// Set up the stack frame register
			addOpcode2(funccode, new OSetRegister(new VarArgument(SFRAME),
												new VarArgument(SP2)));
			if (puc == puc_construct)
				addOpcode2(funccode, new OPushRegister(new VarArgument(CLASS_THISKEY2)));

			// Retain references from parameters that are objects.
			// Callers pushed them on the stack, and here we mark that part of the stack
			// as retaining an object.
			for (auto&& datum : function.getInternalScope()->getLocalData())
			{
				// Exclude 'this' for now.
				if (dynamic_cast<BuiltinVariable*>(datum))
					continue;

				if (!datum->type.isObject())
					continue;

				auto position = lookupStackPosition(*scope, *datum);
				assert(position);
				if (!position)
					continue;

				addOpcode2(funccode, new OMarkTypeStack(new TypeArgument(&datum->type), new LiteralArgument(*position)));
				addOpcode2(funccode, new ORefInc(new LiteralArgument(*position)));
			}

			CleanupVisitor cv(program, scope);
			node.execute(cv);
			OpcodeContext oc(typeStore);
			BuildOpcodes bo(program, scope);
			bo.parsing_user_class = puc;
			bo.visit(node, &oc);

			if (bo.hasError()) failure = true;
			
			size_t prologue_end_index = funccode.size();
			appendElements(funccode, bo.getResult());
			int returnlabelid = bo.getReturnLabelID();

			if(function.getFlag(FUNCFLAG_NEVER_RETURN))
			{
				if(funccode.size())
					funccode.back()->mergeComment("[Opt:NeverRet]");
			}
			else
			{
				if (node.block && !node.block->statements.empty())
					setLocation2(program, node.block->statements.back()->location.fname, node.block->statements.back()->location.last_line);
				else
					setLocation2(program, nullptr);

				addOpcode2(funccode, new ONoOp(returnlabelid));

				// Release references from parameters that are objects.
				for (auto&& datum : function.getInternalScope()->getLocalData())
				{
					// Exclude 'this' for now.
					if (dynamic_cast<BuiltinVariable*>(datum))
						continue;

					auto position = lookupStackPosition(*scope, *datum);
					assert(position);
					if (datum->type.isObject() && position)
						addOpcode2(funccode, new ORefRemove(new LiteralArgument(*position)));
				}

				// Pop off everything
				addOpcode2PopArgs(funccode, stackSize);

				if (puc == puc_construct)
					addOpcode2(funccode, new ORefAutorelease(new VarArgument(CLASS_THISKEY)));
				if (puc != puc_destruct)
					addOpcode2(funccode, new ORefDec(new LiteralArgument(-1))); // release this

				if (puc == puc_construct) //return val
				{
					addOpcode2(funccode, new OSetRegister(new VarArgument(EXP1), new VarArgument(CLASS_THISKEY)));
					addOpcode2(funccode, new OPopRegister(new VarArgument(CLASS_THISKEY)));
				}
				addOpcode2(funccode, new OReturnFunc());
			}

			if (funccode[prologue_end_index]->getLabel())
				function.setPrologueEndLabel(funccode[prologue_end_index]->getLabel());
			else
				funccode[prologue_end_index]->setLabel(function.getPrologueEndLabel());

			function.giveCode(funccode);
		}
		else
		{
			vector<std::shared_ptr<Opcode>> funccode;
			
			int32_t stackSize = getStackSize(function);
			
			// Start of the function.
			addOpcode2(funccode, new ONoOp(function.getLabel()));
			
			// Push on the this, if a script
			bool hasRunThisParam = false;
			if (isRun)
			{
				ParserScriptType type = program.getScript(scriptname)->getType();
				hasRunThisParam = true;

				if (type == ParserScriptType::ffc )
				{
					addOpcode2(funccode, 
						new OPushRegister(new VarArgument(REFFFC)));
				}
				else if (type == ParserScriptType::item )
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFITEMDATA)));
				}
				else if (type == ParserScriptType::npc )
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFNPC)));
				}
				else if (type == ParserScriptType::lweapon )
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFLWPN)));
				}
				else if (type == ParserScriptType::eweapon )
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFEWPN)));
				}
				else if (type == ParserScriptType::dmapdata )
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFDMAPDATA)));
				}
				else if (type == ParserScriptType::itemsprite)
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFITEM)));
				}
				else if (type == ParserScriptType::subscreendata)
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFSUBSCREENDATA)));
				}
				else if (type == ParserScriptType::combodata)
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFCOMBODATA)));
				}
				else if (type == ParserScriptType::genericscr)
				{
					addOpcode2(funccode,
						new OPushRegister(new VarArgument(REFGENERICDATA)));
				}
				else hasRunThisParam = false;
			}
			
			// Push 0s for the local variables.
			int numParams = getParameterCount(function) + (hasRunThisParam ? 1 : 0);
			for (int32_t i = stackSize - numParams; i > 0; --i)
				addOpcode2(funccode, new OPushImmediate(new LiteralArgument(0)));
			
			// Set up the stack frame register
			addOpcode2(funccode, new OSetRegister(new VarArgument(SFRAME),
												new VarArgument(SP2)));

			// Retain references from parameters that are objects.
			// Callers pushed them on the stack, and here we mark that part of the stack
			// as retaining an object.
			for (auto&& datum : function.getInternalScope()->getLocalData())
			{
				if (!datum->type.isObject())
					continue;

				auto position = lookupStackPosition(*scope, *datum);
				assert(position);
				if (!position)
					continue;

				addOpcode2(funccode, new OMarkTypeStack(new TypeArgument(&datum->type), new LiteralArgument(*position)));
				addOpcode2(funccode, new ORefInc(new LiteralArgument(*position)));
			}

			CleanupVisitor cv(program, scope);
			node.execute(cv);
			OpcodeContext oc(typeStore);
			BuildOpcodes bo(program,scope);
			bo.visit(node, &oc);

			if (bo.hasError()) failure = true;
			
			size_t prologue_end_index = funccode.size();
			appendElements(funccode, bo.getResult());
			int returnlabelid = bo.getReturnLabelID();
			
			if(function.getFlag(FUNCFLAG_NEVER_RETURN))
			{
				if(funccode.size())
					funccode.back()->mergeComment("[Opt:NeverRet]");
			}
			else
			{
				if (node.block && !node.block->statements.empty())
					setLocation2(program, node.block->statements.back()->location.fname, node.block->statements.back()->location.last_line);
				else
					setLocation2(program, nullptr);

				// Add appendix code.
				addOpcode2(funccode, new ONoOp(returnlabelid));

				// Release references from parameters that are objects.
				for (auto&& datum : function.getInternalScope()->getLocalData())
				{
					auto position = lookupStackPosition(*scope, *datum);
					assert(position);
					if (datum->type.isObject() && position)
						addOpcode2(funccode, new ORefRemove(new LiteralArgument(*position)));
				}
				
				// Pop off everything.
				addOpcode2PopArgs(funccode, stackSize);
				
				//if it's a main script, quit.
				if (isRun)
					addOpcode2(funccode, new OQuit()); //exit the script
				else addOpcode2(funccode, new OReturnFunc());
			}

			if (funccode[prologue_end_index]->getLabel())
				function.setPrologueEndLabel(funccode[prologue_end_index]->getLabel());
			else
				funccode[prologue_end_index]->setLabel(function.getPrologueEndLabel());

			function.giveCode(funccode);
		}

		if (function.getFlag(FUNCFLAG_INLINE))
		{
			auto& code = function.getCode();
			int file = getFileDebugIndex(program, function.node->location.fname);
			int line = function.node->location.first_line;
			for (auto& op : code)
				op->setLocation(file, line);
		}

		setLocation2(program, nullptr);
		setFunctionScopeLabels(&function);
	}

	if (failure)
	{
		rval.reset();
		return unique_ptr<IntermediateData>(rval.release());;
	}

	return unique_ptr<IntermediateData>(rval.release());
}

ScriptAssembler::ScriptAssembler(IntermediateData& id) : program(id.program),
	assemble_err(false), ginit(id.globalsInit), rval(), runlabels(), lbl_ptrs(), lbl_ptrs_no_scopes(), label_index(), label_index_no_scopes()
{}

void ScriptAssembler::assemble()
{
	assemble_init();
	assemble_scripts();
	gather_run_labels();
	link_functions();
	optimize();
	output_code();
	finalize_labels();
	fill_debug_data();
}

void ScriptAssembler::assemble_init()
{
	allFunctions.clear();

	// Do the global inits
	// If there's a global script called "Init", append it to ~Init:
	Script* userInit = program.getScript("Init");
	if (userInit && (userInit->getType() != ParserScriptType::global || userInit->isPrototypeRun()))
		userInit = nullptr;
	
	map<int32_t,vector<Script*>> initScripts;
	
	if(userInit)
	{
		std::optional<int32_t> weight = userInit->getInitWeight();
		auto& vec = initScripts[weight ? *weight : 0];
		vec.push_back(userInit);
	}
	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		if(script.getType() != ParserScriptType::global) continue;
		if(script.isPrototypeRun()) continue; //skippable
		if(std::optional<int32_t> weight = script.getInitWeight())
		{
			auto& vec = initScripts[*weight];
			vec.push_back(&script);
		}
	}

	for(auto it = initScripts.begin(); it != initScripts.end(); ++it)
	{
		auto& vec = it->second;
		for(auto it = vec.begin(); it != vec.end(); ++it)
		{
			Script& script = **it;
			Function* run = script.getRun();

			setLocation2(program, run->node);

			//Function call the run function
			//push the stack frame pointer
			addOpcode2(ginit, new OPushRegister(new VarArgument(SFRAME)));
			addOpcode2(ginit, new OCallFunc(new LabelArgument(run->getLabel(), true)));
			addOpcode2(ginit,new OPopRegister(new VarArgument(SFRAME)));

			setLocation2(program, nullptr);
			allFunctions.push_back(run);
		}
	}

	// Generate a map of labels to functions.
	appendElements(allFunctions, getFunctions(program));
	appendElements(allFunctions, program.getUserClassConstructors());
	appendElements(allFunctions, program.getUserClassDestructors());
	for (size_t i = 0; i < allFunctions.size(); i++)
	{
		Function& function = *allFunctions[i];
		if(function.is_aliased())
			continue;
		if(function.isTemplateSkip())
		{
			for(auto& applied : function.get_applied_funcs())
				allFunctions.push_back(applied.get());
			continue;
		}
		functionsByLabel[function.getLabel()] = &function;
		if(function.getFlag(FUNCFLAG_CONSTRUCTOR))
			functionsByLabel[function.getAltLabel()] = &function;
	}

	gather_scope_labels();

	addOpcode2(ginit, new OQuit());
	optimize_code(ginit);

	Script* init = program.getScript("~Init");
	Function* init_fn = init->getScope().addFunction(&DataType::ZVOID, "run", {}, {});
	init_fn->giveCode(ginit);
	setFunctionScopeLabels(init_fn);
	run_functions.push_back(init_fn);

	assemble_script(init, init_fn, "void run()");
}

void ScriptAssembler::assemble_script(Script* scr, Function* run_fn, string const& runsig)
{
	int32_t numparams = run_fn->paramTypes.size();
	std::vector<std::shared_ptr<Opcode>> new_code;
	setLocation2(program, run_fn->getNode());

	auto fn_code = run_fn->takeCode();

	// Push on the params to the run.
	for (int i = 0; i < numparams; ++i)
		addOpcode2(new_code, new OPushRegister(new VarArgument(i)));
	if (numparams > 0)
		new_code[0]->setComment(fmt::format("{} Params",runsig));

	if (fn_code.size() > 0)
	{
		if (fn_code.size() == 1)
		{
			fn_code[0]->mergeComment(fmt::format("{} Body",runsig));
		}
		else
		{
			fn_code[0]->mergeComment(fmt::format("{} Body Start",runsig));
			fn_code.back()->mergeComment(fmt::format("{} Body End",runsig));
		}
		appendElements(new_code, fn_code);
	}

	run_fn->giveCode(new_code);
	setFunctionScopeLabels(run_fn);

	auto script_start_indx = rval.size();
	appendElements(rval, run_fn->getCode());

	Opcode* firstop = rval[script_start_indx].get();
	Opcode* lastop = rval.back().get();
	int startlbl = firstop->getLabel();
	if(startlbl < 0)
	{
		startlbl = ScriptParser::getUniqueLabelID();
		firstop->setLabel(startlbl);
	}
	int endlbl = lastop->getLabel();
	if(endlbl < 0)
	{
		endlbl = ScriptParser::getUniqueLabelID();
		lastop->setLabel(endlbl);
	}
	runlabels[scr] = {startlbl,endlbl};

	setLocation2(program, nullptr);
}

void ScriptAssembler::assemble_scripts()
{
	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		if(script.getName() == "~Init") continue; //init script
		if(script.getType() == ParserScriptType::global && (script.getName() == "Init" || script.getInitWeight()))
			continue; //init script
		if(script.getType() == ParserScriptType::untyped)
			continue; //untyped script has no body
		Function& run = *script.getRun();
		if(run.prototype)
			continue; //Skip if run is prototype

		optimize_function(&run);
		run_functions.push_back(&run);
		assemble_script(&script, &run, run.getUnaliasedSignature(true).asString());
	}
}

void ScriptAssembler::gather_run_labels()
{
	for(auto& p : runlabels)
	{
		auto& lbls = p.second;
		lbl_ptrs.push_back(&lbls.first);
		lbl_ptrs.push_back(&lbls.second);
		lbl_ptrs_no_scopes.push_back(&lbls.first);
		lbl_ptrs_no_scopes.push_back(&lbls.second);
	}

	label_index = makeLabelUsageIndex(lbl_ptrs);
	label_index_no_scopes = makeLabelUsageIndex(lbl_ptrs_no_scopes);
}

void ScriptAssembler::link_functions()
{
	// Grab all labels directly jumped to.
	std::set<int32_t> usedLabels;
	GetLabels getlabel(usedLabels);
	getlabel.execute(rval, nullptr);
	std::set<int32_t> unprocessedLabels(usedLabels);

	// Grab labels used by each function until we run out of functions.
	while (!unprocessedLabels.empty())
	{
		int32_t label = *unprocessedLabels.begin();
		Function* function = find<Function*>(functionsByLabel, label).value_or(nullptr);
		if (function)
		{
			vector<shared_ptr<Opcode>> const& functionCode = function->getCode();
			GetLabels temp(usedLabels);
			temp.execute(functionCode, nullptr);
			insertElements(unprocessedLabels, temp.newLabels);
		}

		unprocessedLabels.erase(label);
	}

	if (debug_data_should_emit_inlined_functions())
	{
		for (Function* fn : allFunctions)
		{
			// Include code for binding functions, for use by the debugger.
			if (!fn->isInternal() && fn->isBinding())
				usedLabels.insert(fn->getLabel());
		}
	}

	for (int32_t label : usedLabels)
	{
		Function* function =
			find<Function*>(functionsByLabel, label).value_or(nullptr);
		if (function)
			used_functions.push_back(function);
	}
}

void ScriptAssembler::gather_scope_labels()
{
	std::vector<ZScript::Scope*> stack;
	for (Function* fn : allFunctions)
	{
		stack.push_back(fn->getInternalScope());
		if (fn->prologue_end_label.has_value())
		{
			lbl_ptrs.push_back(&fn->prologue_end_label.value());
			scope_labels.insert(fn->prologue_end_label.value());
		}
	}

	while (!stack.empty())
	{
		auto scope = stack.back();
		stack.pop_back();

		// If BuildVisitor never emits this scope (unused function), these labels will be -1.
		if (scope->start_label != -1)
		{
			lbl_ptrs.push_back(&scope->start_label);
			scope_labels.insert(scope->start_label);
		}
		if (scope->end_label != -1)
		{
			lbl_ptrs.push_back(&scope->end_label);
			scope_labels.insert(scope->end_label);
		}

		for (Scope* scope : scope->getChildren())
			stack.push_back(scope);
	}

	label_index = makeLabelUsageIndex(lbl_ptrs);
	label_index_no_scopes = makeLabelUsageIndex(lbl_ptrs_no_scopes);
}

void ScriptAssembler::optimize()
{
	for (auto fn : used_functions)
		optimize_function(fn);
}

void ScriptAssembler::optimize_function(Function* fn)
{
	auto code = fn->takeCode();
	bool only_remove_nops = fn->isBinding();
	optimize_code(code, only_remove_nops);
	fn->giveCode(code);
}

template <typename T>
static int trash_op(T* op, LabelUsageIndex* label_index, std::set<int>& scope_labels, std::list<shared_ptr<Opcode>>& code, std::list<shared_ptr<Opcode>>::iterator& it, std::function<bool(T*)> condfunc, std::vector<std::unique_ptr<Argument>>* trash_bin)
{
	if(condfunc && !condfunc(op))
	{
		++it;
		return 0;
	}

	auto lbl = op->getLabel();

	// Modifying this label would mess up debug data.
	// Although it's always safe to remove the first op.
	if (scope_labels.contains(lbl) && it != code.begin())
	{
		++it;
		return 0;
	}

	string comment = op->getComment();

	auto it2 = it;
	++it2;
	Opcode* nextcode = it2 == code.end() ? nullptr : it2->get();
	if(nextcode)
		nextcode->mergeComment(comment, true);
	if(lbl == -1) /*no label, just trash it*/
	{
		if constexpr (requires { op->takeFirstArgument(); }) {
			trash_bin->emplace_back(op->takeFirstArgument());
		} else if constexpr (requires { op->takeArgument(); }) {
			trash_bin->emplace_back(op->takeArgument());
		}
		it = code.erase(it);
		return 0;
	}

	if(!nextcode)
	{
		if constexpr (!std::is_same_v<ONoOp, T>)
		{
			ONoOp* nop = new ONoOp(lbl);
			nop->setComment(comment);
			nop->setLocation(op->file, op->line);
			if constexpr (requires { op->takeFirstArgument(); }) {
				trash_bin->emplace_back(op->takeFirstArgument());
			} else if constexpr (requires { op->takeArgument(); }) {
				trash_bin->emplace_back(op->takeArgument());
			}
			it = code.erase(it);
			it = code.insert(it,std::shared_ptr<Opcode>(nop));
		}
		return 1; /*can't merge with something that doesn't exist*/
	}

	auto lbl2 = nextcode->getLabel();
	if(lbl2 == -1) /*next code has no label, pass the label*/
	{
		nextcode->setLabel(lbl);
		if constexpr (requires { op->takeFirstArgument(); }) {
			trash_bin->emplace_back(op->takeFirstArgument());
		} else if constexpr (requires { op->takeArgument(); }) {
			trash_bin->emplace_back(op->takeArgument());
		}
		it = code.erase(it);
		return 0;
	}

	/*Else merge the two labels!*/
	if constexpr (requires { op->takeFirstArgument(); }) {
		trash_bin->emplace_back(op->takeFirstArgument());
	} else if constexpr (requires { op->takeArgument(); }) {
		trash_bin->emplace_back(op->takeArgument());
	}
	it = code.erase(it);
	MergeLabels::merge(lbl2, {lbl}, code, nullptr, label_index);
	return 0;
}

void ScriptAssembler::optimize_code(vector<shared_ptr<Opcode>>& code_vec, bool only_remove_nops)
{
	// Copy vector to list for O(1) modifications.
	std::list<shared_ptr<Opcode>> code(code_vec.begin(), code_vec.end());

	// Run automatic optimizations
	{
		{ //macros
			#define START_OPT_PASS() \
			for(auto it = code.begin(); it != code.end();) \
			{ \
				Opcode* ocode = it->get(); \
				auto lbl = ocode->getLabel(); \
				string comment = ocode->getComment(); \
				int file = ocode->file; \
				int line = ocode->line;
			#define END_OPT_PASS() \
				++it; \
			}
			
			#define TRASH_OP(ty, condfunc) \
			if (auto op = dynamic_cast<ty*>(ocode)) \
			{ \
				if (auto r = trash_op<ty>(op, &label_index, scope_labels, code, it, condfunc, &argument_trash_bin); r == 0) \
					continue; \
				else if (r == 1) break; \
			}

			#define MERGE_CONSEC_1(ty) \
			if(ty* op = dynamic_cast<ty*>(ocode)) \
			{ \
				auto it2 = it; \
				++it2; \
				if(it2 == code.end()) \
					break; \
				if(ty* op2 = dynamic_cast<ty*>(it2->get())) \
				{ \
					if(*op->getArgument() == *op2->getArgument()) \
					{ \
						auto lbl2 = op2->getLabel(); \
						op2->mergeComment(comment, true); \
						if(lbl2 == -1 && lbl > -1) \
						{ \
							op2->setLabel(lbl); \
							argument_trash_bin.emplace_back(op->takeArgument()); \
							it = code.erase(it); \
							continue; \
						} \
						argument_trash_bin.emplace_back(op->takeArgument()); \
						it = code.erase(it); \
						if(lbl > -1) \
						{ \
							MergeLabels::merge(lbl2, {lbl}, code, nullptr, &label_index); \
						} \
						continue; \
					} \
				} \
				++it; \
				continue; \
			}
			#define CONV_GOTO_CMP(ty, cmp) \
			if(ty* op = dynamic_cast<ty*>(ocode)) \
			{ \
				LabelArgument* label_arg = static_cast<LabelArgument*>(op->takeArgument()); \
				OGotoCompare* newop = new OGotoCompare(label_arg,new CompareArgument(cmp)); \
				newop->setLabel(lbl); \
				newop->setComment(comment); \
				newop->setLocation(file, line); \
				it = code.erase(it); \
				it = code.insert(it,std::shared_ptr<Opcode>(newop)); \
				continue; \
			}
			#define MERGE_GOTO_NEXT(ty) \
			if(ty* op = dynamic_cast<ty*>(ocode)) \
			{ \
				auto it2 = it; \
				++it2; \
				if(it2 == code.end()) \
					break; \
				LabelArgument* label_arg = (LabelArgument*)op->getArgument(); \
				Opcode* nextcode = it2->get(); \
				auto lbl2 = nextcode->getLabel(); \
				if(lbl2 > -1 && label_arg->getID() == lbl2) \
				{ \
					nextcode->mergeComment(comment, true); \
					argument_trash_bin.emplace_back(op->takeArgument()); \
					it = code.erase(it); \
					if(lbl > -1) \
					{ \
						MergeLabels::merge(lbl2, {lbl}, code, nullptr, &label_index); \
					} \
					continue; \
				} \
				++it; \
				continue; \
			}
			#define MERGE_GOTO_NEXT2(ty) \
			if(ty* op = dynamic_cast<ty*>(ocode)) \
			{ \
				auto it2 = it; \
				++it2; \
				if(it2 == code.end()) \
					break; \
				LabelArgument* label_arg = (LabelArgument*)op->getFirstArgument(); \
				Opcode* nextcode = it2->get(); \
				auto lbl2 = nextcode->getLabel(); \
				if(lbl2 > -1 && label_arg->getID() == lbl2) \
				{ \
					nextcode->mergeComment(comment, true); \
					argument_trash_bin.emplace_back(op->takeFirstArgument()); \
					it = code.erase(it); \
					if(lbl > -1) \
					{ \
						MergeLabels::merge(lbl2, {lbl}, code, nullptr, &label_index); \
					} \
					continue; \
				} \
				++it; \
				continue; \
			}
			#define MERGE_CONSEC_REPCOUNT_START(ty1,ty2) \
			{ \
				ty1* single_op = dynamic_cast<ty1*>(ocode); \
				ty2* multi_op = dynamic_cast<ty2*>(ocode); \
				if(single_op || multi_op) \
				{ \
					auto it2 = it; \
					++it2; \
					if(it2 == code.end()) \
						break; \
					auto const target_arg = single_op \
						? (single_op->getArgument()) \
						: (multi_op->getFirstArgument()); \
					size_t addcount = 0; \
					while(it2 != code.end()) \
					{ \
						Opcode* nextcode = it2->get(); \
						if(nextcode->getLabel() != -1) \
							break; /*can't combine*/ \
						ty1* single_next = dynamic_cast<ty1*>(nextcode); \
						ty2* multi_next = dynamic_cast<ty2*>(nextcode); \
						if(!(single_next || multi_next)) \
							break; /*can't combine*/ \
						if(*(Argument*)target_arg != *(Argument*)(single_next \
							? (single_next->getArgument()) \
							: (multi_next->getFirstArgument()))) \
							break; /*Different registers, can't combine*/ \
						if(multi_next) \
						{ \
							LiteralArgument const* larg = multi_next->getSecondArgument(); \
							addcount += larg->value; \
						} \
						else /*if single_next*/ \
							++addcount; \
						Opcode::mergeComment(comment, nextcode->getComment()); \
						it2 = code.erase(it2); \
					}
			#define MERGE_CONSEC_REPCOUNT_END(ty1,ty2) \
					if(addcount) \
					{ \
						if(single_op) \
						{ \
							auto arg = single_op->takeArgument(); \
							it = code.erase(it); \
							it = code.insert(it,std::shared_ptr<Opcode>(new ty2(arg,new LiteralArgument(addcount+1)))); \
							(*it)->setLabel(lbl); \
							(*it)->setComment(comment); \
							(*it)->setLocation(file, line);\
						} \
						else /*if multi_op*/ \
						{ \
							LiteralArgument* litarg = multi_op->getSecondArgument(); \
							litarg->value += addcount; \
							multi_op->setComment(comment); \
							multi_op->setLocation(file, line);\
						} \
					} \
					else if(multi_op) \
					{ \
						LiteralArgument* litarg = multi_op->getSecondArgument(); \
						if(litarg->value == 1) \
						{ \
							auto arg = multi_op->takeFirstArgument(); \
							it = code.erase(it); \
							it = code.insert(it,std::shared_ptr<Opcode>(new ty1(arg))); \
							(*it)->setLabel(lbl); \
							(*it)->setComment(comment); \
							(*it)->setLocation(file, line);\
						} \
					} \
					++it; \
					continue; \
				} \
			}
			#define MERGE_CONSEC_REPCOUNT(ty1,ty2) \
			MERGE_CONSEC_REPCOUNT_START(ty1,ty2) \
			MERGE_CONSEC_REPCOUNT_END(ty1,ty2)
			
		} //macros
		START_OPT_PASS() //Trim NoOps
			TRASH_OP(ONoOp, nullptr)
		END_OPT_PASS()

		if (only_remove_nops)
		{
			code_vec.assign(code.begin(), code.end());
			return;
		}

		START_OPT_PASS()
			// Change [PEEKAT reg,0] to [PEEK reg]
			if(OPeekAtImmediate* peekop = dynamic_cast<OPeekAtImmediate*>(ocode))
			{
				LiteralArgument* litarg = peekop->getSecondArgument();
				if(!litarg->value)
				{
					VarArgument* arg = peekop->takeFirstArgument();
					it = code.erase(it);
					it = code.insert(it, std::shared_ptr<Opcode>(new OPeek(arg)));
					(*it)->setLabel(lbl);
					(*it)->setComment(comment);
				}
				++it;
				continue;
			}
			// If [STORE reg,lit] is followed by [LOAD reg,lit], the LOAD
			// can be deleted, as 'reg' already will contain the value to be loaded.
			if(OStoreDirect* stored = dynamic_cast<OStoreDirect*>(ocode))
			{
				Argument const* regarg = stored->getFirstArgument();
				Argument const* litarg = stored->getSecondArgument();
				auto it2 = it;
				++it2;
				if(OLoad* loadd = dynamic_cast<OLoad*>(it2->get()))
				{
					if(*regarg == *loadd->getFirstArgument()
						&& *litarg == *loadd->getSecondArgument()
						&& loadd->getLabel() < 0)
					{
						stored->mergeComment(loadd->getComment());
						code.erase(it2);
						continue;
					}
				}
				++it;
				continue;
			}
		END_OPT_PASS()
		START_OPT_PASS()
			//Merge multiple consecutive identical pops/pushes
			MERGE_CONSEC_REPCOUNT_START(OPopRegister,OPopArgsRegister)
			{ // turn single-pop followed by single-push into peek
				size_t startcount = 1;
				if(multi_op)
				{
					LiteralArgument* litarg = multi_op->getSecondArgument();
					startcount = litarg->value;
				}
				if(addcount+startcount == 1)
				{
					Opcode* nextcode = it2->get();
					if(nextcode->getLabel() == -1)
					{
						if(OPushRegister* pusharg = dynamic_cast<OPushRegister*>(nextcode))
						{
							if(*target_arg == *pusharg->getArgument())
							{
								auto arg = pusharg->takeArgument();
								it2 = code.erase(it2);
								it = code.erase(it);
								it = code.insert(it,std::shared_ptr<Opcode>(new OPeek(arg)));
								(*it)->setLabel(lbl);
								(*it)->setComment(comment);
								++it;
								continue;
							}
						}
					}
				}
			}
			MERGE_CONSEC_REPCOUNT_END(OPopRegister,OPopArgsRegister)
			MERGE_CONSEC_REPCOUNT(OPushRegister,OPushArgsRegister)
			MERGE_CONSEC_REPCOUNT(OPushImmediate,OPushArgsImmediate)
			MERGE_CONSEC_REPCOUNT(OPushVargR,OPushVargsR)
			MERGE_CONSEC_REPCOUNT(OPushVargV,OPushVargsV)
			// goto if never, can be trashed
			TRASH_OP(OGotoCompare, [&](OGotoCompare* op)
				{
					auto cmp = op->getSecondArgument()->value;
					return !(cmp&CMP_FLAGS);
				})
			//Convert gotos to OGotoCompare
			CONV_GOTO_CMP(OGotoTrueImmediate, CMP_EQ)
			CONV_GOTO_CMP(OGotoFalseImmediate, CMP_NE)
			CONV_GOTO_CMP(OGotoMoreImmediate, CMP_GE)
			CONV_GOTO_CMP(OGotoLessImmediate, CMP_LE)
			//Merge consecutive identical gotos
			MERGE_CONSEC_1(OGotoImmediate)
			MERGE_CONSEC_1(OGotoRegister)
		END_OPT_PASS()
		START_OPT_PASS()
			if(OGotoCompare* op = dynamic_cast<OGotoCompare*>(ocode))
			{
				auto it2 = it;
				++it2;
				if(it2 == code.end())
					break;
				CompareArgument* cmparg = static_cast<CompareArgument*>(op->getSecondArgument());
				cmparg->value &= ~CMP_SETI;
				auto cmp = cmparg->value;
				if(OGotoCompare* op2 = dynamic_cast<OGotoCompare*>(it2->get()))
				{
					if(!op->getFirstArgument()->toString().compare(
						op2->getFirstArgument()->toString()))
					{
						CompareArgument* cmparg2 = static_cast<CompareArgument*>(op2->getSecondArgument());
						if((cmparg2->value & CMP_BOOL) != (cmp & CMP_BOOL)) //differing bool-states are weird...
						{
							++it;
							continue;
						}
						cmparg2->value &= ~CMP_SETI;
						cmparg2->value |= cmp; //merge compare types
						auto lbl2 = op2->getLabel();
						op2->mergeComment(comment, true);
						if(lbl2 == -1 && lbl > -1)
						{
							op2->setLabel(lbl);
							it = code.erase(it);
							continue;
						}
						it = code.erase(it);
						if(lbl > -1)
						{
							MergeLabels::merge(lbl2, {lbl}, code, nullptr, &label_index);
						}
						continue;
					}
				}
				if((cmp&CMP_FLAGS) == CMP_FLAGS)
				{
					LabelArgument* label_arg = static_cast<LabelArgument*>(op->takeFirstArgument());
					it = code.erase(it);
					OGotoImmediate* newop = new OGotoImmediate(label_arg);
					newop->setLabel(lbl);
					newop->setComment(comment);
					it = code.insert(it,std::shared_ptr<Opcode>(newop));
					continue;
				}
				++it;
				continue;
			}
		END_OPT_PASS()
		START_OPT_PASS()
			//Trim GOTOs that go to the line directly after them
			MERGE_GOTO_NEXT(OGotoImmediate)
			MERGE_GOTO_NEXT2(OGotoCompare)
			MERGE_GOTO_NEXT(OCallFunc)
		END_OPT_PASS()

		// TODO: unsure about correctness.
		// see https://discord.com/channels/876899628556091432/1457221235766333522

		// map<int,std::pair<int,int>> gotocmp_map;

		// START_OPT_PASS()
		// 	if(OGotoImmediate* op = dynamic_cast<OGotoImmediate*>(ocode))
		// 	{
		// 		if(lbl > -1) //redirect labels that jump to GOTOs
		// 		{
		// 			auto targ_lbl = static_cast<LabelArgument*>(op->getArgument())->getID();
		// 			if(targ_lbl != lbl)
		// 			{
		// 				bool is_scope_boundary = scope_labels.count(lbl);
		// 				if (is_scope_boundary)
		// 				{
		// 					MergeLabels::merge(targ_lbl, {lbl}, code, nullptr, &label_index_no_scopes);
		// 				}
		// 				else
		// 				{
		// 					MergeLabels::merge(targ_lbl, {lbl}, code, nullptr, &label_index);
		// 					op->setLabel(-1);
		// 				}
		// 			}
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// 	if(OGotoCompare* op = dynamic_cast<OGotoCompare*>(ocode))
		// 	{
		// 		if(lbl > -1) //store labels in map, for optimization in next pass
		// 		{
		// 			if (scope_labels.count(lbl))
		// 			{
		// 				++it;
		// 				continue;
		// 			}

		// 			int targ_lbl = static_cast<LabelArgument*>(op->getFirstArgument())->getID();
		// 			int cmp = static_cast<CompareArgument*>(op->getSecondArgument())->value & ~CMP_SETI;
		// 			gotocmp_map[lbl] = {targ_lbl,cmp};
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// END_OPT_PASS()
		// START_OPT_PASS()
		// 	//[X: GOTO N] [N: GOTOCMP x,cmp] -> [X: GOTOCMP x,cmp] [N: GOTOCMP x,cmp]
		// 	if(OGotoImmediate* op = dynamic_cast<OGotoImmediate*>(ocode))
		// 	{
		// 		LabelArgument* lbl_arg = static_cast<LabelArgument*>(op->getArgument());
		// 		auto it2 = gotocmp_map.find(lbl_arg->getID());
		// 		if(it2 != gotocmp_map.end())
		// 		{
		// 			lbl_arg = static_cast<LabelArgument*>(op->takeArgument());
		// 			lbl_arg->setID(it2->second.first);
		// 			it = code.erase(it);
		// 			OGotoCompare* newop = new OGotoCompare(lbl_arg, new CompareArgument(it2->second.second));
		// 			newop->setComment(comment);
		// 			//lbl == -1 is guaranteed
		// 			it = code.insert(it,std::shared_ptr<Opcode>(newop));
		// 			continue;
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// END_OPT_PASS()
		START_OPT_PASS()
			//[N: GOTOCMP N+2,c] [N+1: GOTO x] -> [N: GOTOCMP x,INV(c)]
			if(OGotoCompare* op = dynamic_cast<OGotoCompare*>(ocode))
			{
				auto it2 = it;
				++it2;
				if(it2 == code.end())
					break;

				auto it3 = it2;
				++it3;
				if(it3 == code.end())
					break;

				auto it3_lbl = it3->get()->getLabel();

				LabelArgument* lblarg = static_cast<LabelArgument*>(op->getFirstArgument());

				if(it3_lbl != -1 && it3_lbl == lblarg->getID())
        		{
					if(OGotoImmediate* op2 = dynamic_cast<OGotoImmediate*>(it2->get()))
					{
						LabelArgument* mid_lbl_arg = static_cast<LabelArgument*>(op2->getArgument());
						int op2_lbl = op2->getLabel();
	
						if (op2_lbl > -1)
						{
							// 1. LOGICAL FIX:
							// If any OTHER instructions jump to 'op2_lbl', they effectively want to jump
							// to the destination of op2. Redirect them there.
							// DO NOT move scope boundaries yet.
							int target_lbl = mid_lbl_arg->getID();
							MergeLabels::merge(target_lbl, {op2_lbl}, code, nullptr, &label_index_no_scopes);
	
							// 2. DEBUG FIX:
							// The Scope Boundary anchored at 'op2' must stay at this physical position.
							// Since op2 is being deleted, we attach the anchor to 'it3' (the next op).
							if (it3_lbl != -1)
							{
								// it3 already has a label. We must merge op2_lbl INTO it3_lbl.
								// We pass an empty code list because we already updated the code in Step 1.
								// We only want to update 'lbl_ptrs'.
								std::list<std::shared_ptr<Opcode>> empty_list;
								MergeLabels::merge(it3_lbl, {op2_lbl}, empty_list, nullptr, &label_index);
							}
							else
							{
								// it3 has no label. Simply move the label over.
								it3->get()->setLabel(op2_lbl);
								// Update local var so the check 'it3_lbl == lblarg->getID()' below works if needed
								// (though in your specific logic, it3_lbl is distinct from op2_lbl, so this is fine)
							}
						}
	
						lblarg->setID(mid_lbl_arg->getID());
						CompareArgument* cmparg = static_cast<CompareArgument*>(op->getSecondArgument());
						cmparg->value = INVERT_CMP(cmparg->value);

						op->mergeComment(op2->getComment());
						code.erase(it2);
						++it;
						continue;
					}
				}

				++it;
				continue;
			}
			MERGE_CONSEC_1(OGotoImmediate) //Redo this here due to timing stuff -Em
		END_OPT_PASS()
		// START_OPT_PASS()
		// 	// Change [PEEKAT reg,0] to [PEEK reg]
		// 	if(OPeekAtImmediate* peekop = dynamic_cast<OPeekAtImmediate*>(ocode))
		// 	{
		// 		LiteralArgument* litarg = peekop->getSecondArgument();
		// 		if(!litarg->value)
		// 		{
		// 			VarArgument* arg = peekop->takeFirstArgument();
		// 			it = code.erase(it);
		// 			it = code.insert(it, std::shared_ptr<Opcode>(new OPeek(arg)));
		// 			(*it)->setLabel(lbl);
		// 			(*it)->setComment(comment);
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// 	// If [STORE reg,lit] is followed by [LOAD reg,lit], the LOAD
		// 	// can be deleted, as 'reg' already will contain the value to be loaded.
		// 	if(OStoreDirect* stored = dynamic_cast<OStoreDirect*>(ocode))
		// 	{
		// 		Argument const* regarg = stored->getFirstArgument();
		// 		Argument const* litarg = stored->getSecondArgument();
		// 		auto it2 = it;
		// 		++it2;
		// 		if(OLoad* loadd = dynamic_cast<OLoad*>(it2->get()))
		// 		{
		// 			if(*regarg == *loadd->getFirstArgument()
		// 				&& *litarg == *loadd->getSecondArgument()
		// 				&& loadd->getLabel() < 0)
		// 			{
		// 				stored->mergeComment(loadd->getComment());
		// 				code.erase(it2);
		// 				continue;
		// 			}
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// END_OPT_PASS()
		// START_OPT_PASS()
		// 	//Merge multiple consecutive identical pops/pushes
		// 	MERGE_CONSEC_REPCOUNT_START(OPopRegister,OPopArgsRegister)
		// 	{ // turn single-pop followed by single-push into peek
		// 		size_t startcount = 1;
		// 		if(multi_op)
		// 		{
		// 			LiteralArgument* litarg = multi_op->getSecondArgument();
		// 			startcount = litarg->value;
		// 		}
		// 		if(addcount+startcount == 1)
		// 		{
		// 			Opcode* nextcode = it2->get();
		// 			if(nextcode->getLabel() == -1)
		// 			{
		// 				if(OPushRegister* pusharg = dynamic_cast<OPushRegister*>(nextcode))
		// 				{
		// 					if(*target_arg == *pusharg->getArgument())
		// 					{
		// 						auto arg = pusharg->takeArgument();
		// 						it2 = code.erase(it2);
		// 						it = code.erase(it);
		// 						it = code.insert(it,std::shared_ptr<Opcode>(new OPeek(arg)));
		// 						(*it)->setLabel(lbl);
		// 						(*it)->setComment(comment);
		// 						++it;
		// 						continue;
		// 					}
		// 				}
		// 			}
		// 		}
		// 	}
		// 	MERGE_CONSEC_REPCOUNT_END(OPopRegister,OPopArgsRegister)
		// 	MERGE_CONSEC_REPCOUNT(OPushRegister,OPushArgsRegister)
		// 	MERGE_CONSEC_REPCOUNT(OPushImmediate,OPushArgsImmediate)
		// 	MERGE_CONSEC_REPCOUNT(OPushVargR,OPushVargsR)
		// 	MERGE_CONSEC_REPCOUNT(OPushVargV,OPushVargsV)
		// 	// goto if never, can be trashed
		// 	TRASH_OP(OGotoCompare, [&](OGotoCompare* op)
		// 		{
		// 			auto cmp = op->getSecondArgument()->value;
		// 			return !(cmp&CMP_FLAGS);
		// 		})
		// 	//Convert gotos to OGotoCompare
		// 	CONV_GOTO_CMP(OGotoTrueImmediate, CMP_EQ)
		// 	CONV_GOTO_CMP(OGotoFalseImmediate, CMP_NE)
		// 	CONV_GOTO_CMP(OGotoMoreImmediate, CMP_GE)
		// 	CONV_GOTO_CMP(OGotoLessImmediate, CMP_LE)
		// 	//Merge consecutive identical gotos
		// 	MERGE_CONSEC_1(OGotoImmediate)
		// 	MERGE_CONSEC_1(OGotoRegister)
		// END_OPT_PASS()
		// START_OPT_PASS()
		// 	if(OGotoCompare* op = dynamic_cast<OGotoCompare*>(ocode))
		// 	{
		// 		auto it2 = it;
		// 		++it2;
		// 		if(it2 == code.end())
		// 			break;
		// 		CompareArgument* cmparg = static_cast<CompareArgument*>(op->getSecondArgument());
		// 		cmparg->value &= ~CMP_SETI;
		// 		auto cmp = cmparg->value;
		// 		if(OGotoCompare* op2 = dynamic_cast<OGotoCompare*>(it2->get()))
		// 		{
		// 			if(!op->getFirstArgument()->toString().compare(
		// 				op2->getFirstArgument()->toString()))
		// 			{
		// 				CompareArgument* cmparg2 = static_cast<CompareArgument*>(op2->getSecondArgument());
		// 				if((cmparg2->value & CMP_BOOL) != (cmp & CMP_BOOL)) //differing bool-states are weird...
		// 				{
		// 					++it;
		// 					continue;
		// 				}
		// 				cmparg2->value &= ~CMP_SETI;
		// 				cmparg2->value |= cmp; //merge compare types
		// 				auto lbl2 = op2->getLabel();
		// 				op2->mergeComment(comment, true);
		// 				if(lbl2 == -1 && lbl > -1)
		// 				{
		// 					op2->setLabel(lbl);
		// 					argument_trash_bin.emplace_back(op->takeFirstArgument());
		// 					it = code.erase(it);
		// 					continue;
		// 				}

		// 				argument_trash_bin.emplace_back(op->takeFirstArgument());
		// 				it = code.erase(it);
		// 				if(lbl > -1)
		// 				{
		// 					MergeLabels::merge(lbl2, {lbl}, code, nullptr, &label_index);
		// 				}
		// 				continue;
		// 			}
		// 		}
		// 		if((cmp&CMP_FLAGS) == CMP_FLAGS)
		// 		{
		// 			LabelArgument* label_arg = static_cast<LabelArgument*>(op->takeFirstArgument());
		// 			it = code.erase(it);
		// 			OGotoImmediate* newop = new OGotoImmediate(label_arg);
		// 			newop->setLabel(lbl);
		// 			newop->setComment(comment);
		// 			it = code.insert(it,std::shared_ptr<Opcode>(newop));
		// 			continue;
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// END_OPT_PASS()
		// START_OPT_PASS()
		// 	//[X: GOTO N] [N: GOTOCMP x,cmp] -> [X: GOTOCMP x,cmp] [N: GOTOCMP x,cmp]
		// 	if(OGotoImmediate* op = dynamic_cast<OGotoImmediate*>(ocode))
		// 	{
		// 		LabelArgument* lbl_arg = static_cast<LabelArgument*>(op->getArgument());
		// 		auto it2 = gotocmp_map.find(lbl_arg->getID());
		// 		if(it2 != gotocmp_map.end())
		// 		{
		// 			lbl_arg = static_cast<LabelArgument*>(op->takeArgument());
		// 			lbl_arg->setID(it2->second.first);
		// 			it = code.erase(it);
		// 			OGotoCompare* newop = new OGotoCompare(lbl_arg, new CompareArgument(it2->second.second));
		// 			newop->setComment(comment);
		// 			//lbl == -1 is guaranteed
		// 			it = code.insert(it,std::shared_ptr<Opcode>(newop));
		// 			continue;
		// 		}
		// 		++it;
		// 		continue;
		// 	}
		// END_OPT_PASS()
		START_OPT_PASS()
			MERGE_GOTO_NEXT(OGotoImmediate) //Redo this here due to timing stuff -Em
			MERGE_GOTO_NEXT2(OGotoCompare) //Redo this here due to timing stuff -Em
			// OSetImmediate + OTraceRegister -> OTraceImmediate
			if(OSetImmediate* setop = dynamic_cast<OSetImmediate*>(ocode))
			{
				Argument const* regarg = setop->getFirstArgument();
				auto it2 = it;
				++it2;
				Opcode* nextcode = it2->get();
				if(OTraceRegister* traceop = dynamic_cast<OTraceRegister*>(nextcode))
				{
					if(*regarg == *traceop->getArgument())
					{
						auto arg = setop->takeSecondArgument();
						Opcode::mergeComment(comment, traceop->getComment());

						// Capture labels before erasing
						int set_lbl = lbl; // The label on the SETV
						int trace_lbl = traceop->getLabel(); // The label on the TRACER

						it2 = code.erase(it2);
						it = code.erase(it);

						// Insert new op.
						it = code.insert(it, std::shared_ptr<Opcode>(new OTraceImmediate(arg)));
						Opcode* new_op = it->get();
						new_op->setComment(comment);
						new_op->setLocation(file, line);

						if (trace_lbl != -1)
						{
							if (set_lbl != -1)
							{
								// CASE 1: Both had labels. 
								// We keep the SET label, and alias the TRACE label to it.
								// This updates the DebugScope pointers automatically.
								MergeLabels::merge(set_lbl, {trace_lbl}, code, nullptr, &label_index);
								new_op->setLabel(set_lbl);
							}
							else
							{
								// CASE 2: Only TRACE had a label.
								// Move it to the new op (expanding the scope backwards).
								new_op->setLabel(trace_lbl);
							}
						}
						else
						{
							// CASE 3: Only SET (or neither) had a label.
							// Keep the original SET label.
							new_op->setLabel(set_lbl);
						}

						++it;
						continue;
					}
				}
			}
		END_OPT_PASS()
	}

	code_vec.assign(code.begin(), code.end());
}

// Insert every used function's code into `rval`.
void ScriptAssembler::output_code()
{
	// Sort by source location. This improves the debug info encoding.
	auto& files = program.getFiles();
	std::map<std::string, int> fileToDebugIndexMap;
	for (int i = 0; i < files.size(); i++)
		fileToDebugIndexMap[files[i]] = i;

	std::sort(used_functions.begin(), used_functions.end(), [&](const auto* a, const auto* b){
		int file_a = fileToDebugIndexMap[a->node->location.fname];
		int file_b = fileToDebugIndexMap[b->node->location.fname];
		if (file_a != file_b) return file_b > file_a;

		int line_a = a->node->location.first_line;
		int line_b = b->node->location.first_line;
		if (line_a != line_b) return line_b > line_a;

		return b->id > a->id;
	});

	setLocation2(program, nullptr);

	for (auto fn : used_functions)
	{
		vector<shared_ptr<Opcode>> functionCode = fn->getCode();
		auto rv_sz = rval.size();
		for (vector<shared_ptr<Opcode>>::iterator it = functionCode.begin();
				it != functionCode.end(); ++it)
			addOpcode2(rval, (*it)->makeClone());
		if(rval.size() == rv_sz+1)
			rval.back()->mergeComment(fmt::format("Func[{}] Body",fn->getUnaliasedSignature(true).asString()));
		else if(rval.size() > rv_sz)
		{
			rval[rv_sz]->mergeComment(fmt::format("Func[{}] Body Start" ,fn->getUnaliasedSignature(true).asString()));
			rval.back()->mergeComment(fmt::format("Func[{}] Body End" ,fn->getUnaliasedSignature(true).asString()));
		}
	}
}

void ScriptAssembler::finalize_labels()
{
	// Set the label line numbers.
	int32_t lineno = 1;

	for (auto it = rval.begin(); it != rval.end(); ++it)
	{
		if ((*it)->getLabel() != -1)
			linenos[(*it)->getLabel()] = lineno;
		lineno++;
	}

	// Now fill in those labels
	SetLabels setlabel;
	setlabel.execute(rval, &linenos);
	
	// ...and for tracking the run functions
	for(auto& pair : runlabels)
	{
		auto& pcs = pair.second;
		pcs.first = SetLabels::check(pcs.first, linenos, &setlabel.err);
		if(pcs.first)
			--pcs.first; //stupid 1-indexing...
		pcs.second = SetLabels::check(pcs.second, linenos, &setlabel.err);
		if(pcs.second)
			--pcs.second; //stupid 1-indexing...
	}

	if (setlabel.err)
		assemble_err = true;
}

static std::string normalize_file_path(const std::string& file, const fs::path& base_path)
{
	auto path = fs::path(file).lexically_normal();
	auto relative_path = path.lexically_relative(base_path);
	return relative_path.empty() ? path.generic_string() : relative_path.generic_string();
}

void ScriptAssembler::fill_debug_data()
{
	int32_t prev_file = 0;
	int32_t prev_line = 1;
	int32_t prev_pc = 0;

	std::set<int> prologue_end_labels;
	for (auto& fn : allFunctions)
		prologue_end_labels.insert(fn->getPrologueEndLabel());

	for (size_t pc = 0; pc < rval.size(); ++pc)
	{
		Opcode* op = rval[pc].get();

		if (op->line <= 0) continue;

		int label = op->getLabel();
		bool is_prologue_end = label != -1 && prologue_end_labels.contains(label);

		bool file_changed = false;
		if (op->file >= 0 && op->file != prev_file)
		{
			// Flush the range using the old file.
			int32_t gap = pc - prev_pc;
			if (gap > 0)
			{
				debug_data.appendLineInfoExtendedStep(gap, 0);
				prev_pc = pc;
			}

			debug_data.appendLineInfoSetFile(op->file);
			prev_file = op->file;
			file_changed = true;
		}

		// Skip if line hasn't changed (and we didn't just switch files).
		if (!file_changed && !is_prologue_end && op->line == prev_line)
			continue;

		int32_t d_line = op->line - prev_line;
		int32_t d_pc = pc - prev_pc;

		if (d_line == 1 && d_pc >= 0 && d_pc <= DebugData::DEBUG_LINE_OP_SIMPLE_STEP_MAX)
			debug_data.appendLineInfoSimpleStep(d_pc);
		else
			debug_data.appendLineInfoExtendedStep(d_pc, d_line);

		if (is_prologue_end)
			debug_data.appendLineInfoPrologueEnd();

		prev_line = op->line;
		prev_pc = pc;
	}

	auto cur_path = fs::current_path();
	const auto& files = program.getFiles();
	for (int i = 0; i < files.size(); i++)
	{
		const std::string& file = files[i];
		const std::string& contents = getSourceCodeContents(file);
		debug_data.source_files.push_back({
			.path = normalize_file_path(file, cur_path),
			.contents = contents,
		});
	}

	fill_debug_scopes();
}

class DebugTypeBuilder
{
	std::vector<DebugType>& table;
	std::map<std::string, uint32_t> cache; // function signature -> id.

public:
	DebugTypeBuilder(std::vector<DebugType>& out_table) : table(out_table) {}

	std::optional<int32_t> getPrimitiveID(const DataType* type)
	{
		if (type->isConstant()) return std::nullopt;
		if (type->isArray()) return std::nullopt;
		if (type->isTemplate()) return TYPE_TEMPLATE_UNBOUNDED;

		if (const DataTypeSimple* t = dynamic_cast<const DataTypeSimple*>(type))
		{
			if (t->getId() == ZTID_VOID) return TYPE_VOID;
			if (t->getId() == ZTID_UNTYPED) return TYPE_UNTYPED;
			if (t->getId() == ZTID_LONG) return TYPE_LONG;
			if (t->getId() == ZTID_FLOAT) return TYPE_INT; // Not a typo. What ZScript calls "int", the compiler calls "float", even though it is really a fixed int.
			if (t->getId() == ZTID_BOOL) return TYPE_BOOL;
			if (t->getId() == ZTID_CHAR) return TYPE_CHAR32;
			if (t->getId() == ZTID_RGBDATA) return TYPE_RGB;
		}

		return std::nullopt;
	}

	uint32_t getTypeID(const DataType* type, const std::map<UserClass*, int>& class_scope_map, const std::map<int, int>& enum_id_to_scope_index)
	{
		auto primitive_id = getPrimitiveID(type);
		if (primitive_id)
			return primitive_id.value();

		std::string key = type->getName();
		if (cache.count(key)) return cache[key];

		uint32_t result_id = 0;

		if (type->isConstant())
		{
			// Recursively get the underlying type.
			uint32_t base_id = getTypeID(type->getMutType(), class_scope_map, enum_id_to_scope_index);

			DebugType dt{};
			dt.tag = TYPE_CONST;
			dt.extra = base_id;
			result_id = addEntry(dt);
		}
		else if (type->isArray())
		{
			auto arrType = static_cast<const DataTypeArray*>(type);
			uint32_t elem_id = getTypeID(&arrType->getElementType(), class_scope_map, enum_id_to_scope_index);

			DebugType dt{};
			dt.tag = TYPE_ARRAY;
			dt.extra = elem_id;
			result_id = addEntry(dt);
		}
		else if (type->isClass())
		{
			UserClass* cls = type->getUsrClass();
			DebugType dt{};
			dt.tag = TYPE_CLASS;
			dt.extra = class_scope_map.at(cls);
			result_id = addEntry(dt);
		}
		else if (type->isEnum())
		{
			const DataTypeCustom* enum_type = static_cast<const DataTypeCustom*>(type);
			DebugType dt{};
			dt.tag = enum_type->isBitflagsEnum() ? TYPE_BITFLAGS : TYPE_ENUM;
			dt.extra = enum_id_to_scope_index.at(enum_type->getCustomId());
			result_id = addEntry(dt);
		}

		cache[key] = result_id;
		return result_id;
	}

private:
	uint32_t addEntry(DebugType dt)
	{
		uint32_t id = table.size() + DEBUG_TYPE_TAG_TABLE_START;
		table.push_back(dt);
		return id;
	}
};

void ScriptAssembler::fill_debug_scopes()
{
	ScopeProcessingContext ctx(debug_data);
	auto cur_path = fs::current_path();
	RootScope* rootScope = &program.getScope();

	struct StackEntry
	{
		Scope* scope;
		int32_t parent_idx;
		bool is_within_func;
	};

	std::vector<StackEntry> stack;
	stack.push_back({rootScope, -1, false});

	while (!stack.empty())
	{
		auto [scope, parentIdx, isWithinFunction] = stack.back();
		stack.pop_back();

		DebugScope dScope{};

		bool skip_emit = false;
		if (!init_debug_scope(scope, dScope, cur_path, isWithinFunction, skip_emit))
			continue;

		// There's no reason for files to be scoped within each other. The only reason compiler
		// Scopes do that is to support `#option` impacting included files. That could probably be
		// supported in a better way than making File scopes nest, but for now just hardcode the
		// parent to be the root scope.
		if (scope->isFile())
			dScope.parent_index = 0;
		else
			dScope.parent_index = parentIdx;

		int32_t myIdx = parentIdx; 

		if (!skip_emit)
		{
			if (dScope.tag == TAG_BLOCK || dScope.tag == TAG_FUNCTION)
				resolve_debug_scope_ranges(scope, dScope);

			int32_t currentScopeIdx = ctx.debug_data.scopes.size();
			ctx.debug_data.scopes.push_back(dScope);
			ctx.compiler_scopes.push_back(scope);
			ctx.scope_ptr_to_index[scope] = currentScopeIdx;

			if (scope->isFunction())
				ctx.scope_types.push_back(static_cast<FunctionScope*>(scope)->function.returnType);
			else
				ctx.scope_types.push_back(nullptr);

			bool has_content = fill_debug_scope_locals(scope, currentScopeIdx, ctx);
			if (dScope.tag == TAG_BLOCK && !has_content)
			{
				// Drop.
				ctx.debug_data.scopes.pop_back();
				ctx.compiler_scopes.pop_back();
				ctx.scope_types.pop_back();
				ctx.scope_ptr_to_index.erase(scope);
			}
			else
			{
				myIdx = currentScopeIdx;
				if (dScope.tag == TAG_FILE)
					ctx.debug_data.scopes[0].imports.push_back(myIdx);
			}

			if (scope->isFunction())
			{
				auto fs = static_cast<FunctionScope*>(scope);
				for (auto alias_fn : fs->function.getAliases())
				{
					auto aliasedDebugScope = dScope;
					aliasedDebugScope.name = alias_fn->name;
					aliasedDebugScope.flags |= SCOPE_FLAG_HIDDEN;
					currentScopeIdx = ctx.debug_data.scopes.size();
					ctx.debug_data.scopes.push_back(aliasedDebugScope);
					ctx.compiler_scopes.push_back(scope);
					ctx.scope_types.push_back(ctx.scope_types.back());
					fill_debug_scope_locals(scope, currentScopeIdx, ctx);
				}
			}
		}

		std::vector<Function*> child_funcs;
		if (scope->isClass())
		{
			auto* cs = static_cast<ClassScope*>(scope);
			appendElements(child_funcs, cs->getConstructors());
			appendElements(child_funcs, cs->getDestructor());
		}
		if (scope->isFile() || scope->isClass() || scope->isNamespace() || scope->isScript())
			appendElements(child_funcs, scope->getLocalFunctions());

		std::vector<Scope*> seen_children;
		for (auto it = child_funcs.rbegin(); it != child_funcs.rend(); it++)
		{
			Function* fn = *it;
			if (fn->isTemplateSkip())
			{
				for (auto& applied : fn->get_applied_funcs())
				{
					stack.push_back({applied->getInternalScope(), myIdx, isWithinFunction});
					seen_children.push_back(applied->getInternalScope());
				}
			}
			else
			{
				stack.push_back({fn->getInternalScope(), myIdx, isWithinFunction});
				seen_children.push_back(fn->getInternalScope());
			}
		}

		auto children = scope->getChildren();
		for (auto it = children.rbegin(); it != children.rend(); it++)
		{
			if (util::contains(seen_children, *it))
				continue;

			stack.push_back({*it, myIdx, isWithinFunction});
		}
	}

	finalize_debug_scopes(ctx);
}

bool ScriptAssembler::init_debug_scope(Scope* scope, DebugScope& dScope, const fs::path& cur_path, bool& isWithinFunc, bool& skip_emit)
{
	dScope.name = scope->getName().value_or("");
	skip_emit = false;

	if (scope->isRoot())
	{
		dScope.tag = TAG_ROOT;
	}
	else if (scope->isFile())
	{
		if (dScope.name == "<root>")
		{
			skip_emit = true; 
			return true; 
		}

		dScope.tag = TAG_FILE;
		dScope.name = normalize_file_path(dScope.name, cur_path);
	}
	else if (scope->isScript())
	{
		dScope.tag = TAG_SCRIPT;
	}
	else if (scope->isFunction())
	{
		dScope.tag = TAG_FUNCTION;
		isWithinFunc = true;

		auto* fs = static_cast<FunctionScope*>(scope);

		if (fs->function.getFlag(FUNCFLAG_DESTRUCTOR) && !dScope.name.starts_with("~"))
			dScope.name = "~" + dScope.name;

		bool is_run = std::find(run_functions.begin(), run_functions.end(), &fs->function) != run_functions.end();
		if (!is_run)
		{
			if (fs->function.isInternal()) return false;
			bool is_binding = fs->function.isBinding();
			bool is_used = std::find(used_functions.begin(), used_functions.end(), &fs->function) != used_functions.end();
			if (!is_binding && !is_used) return false;
		}

		if (fs->function.getFlag(FUNCFLAG_DEPRECATED))
			dScope.flags |= SCOPE_FLAG_DEPRECATED;
		if (fs->function.is_aliased())
			dScope.flags |= SCOPE_FLAG_HIDDEN;
		if (fs->function.isBinding())
			dScope.flags |= SCOPE_FLAG_INTERNAL;
	}
	else if (scope->isClass())
	{
		dScope.tag = TAG_CLASS;
	}
	else if (scope->isNamespace())
	{
		dScope.tag = TAG_NAMESPACE;
	}
	else
	{
		if (!isWithinFunc)
			return false;

		dScope.tag = TAG_BLOCK;
	}

	return true;
}

void ScriptAssembler::resolve_debug_scope_ranges(Scope* scope, DebugScope& dScope)
{
	if (scope->start_label == -1)
		return;

	dScope.start_pc = linenos.at(scope->start_label) - 1;
	dScope.end_pc = linenos.at(scope->end_label) - 1;
	CHECK(dScope.end_pc >= dScope.start_pc);
}

bool ScriptAssembler::fill_debug_scope_locals(Scope* scope, int32_t scopeIdx, ScopeProcessingContext& ctx)
{
	if (scope->isRoot()) return false;

	auto localData = scope->getLocalData();
	std::stable_sort(localData.begin(), localData.end(), [](Datum* a, Datum* b) {
		bool aBuiltin = a->isBuiltIn();
		bool bBuiltin = b->isBuiltIn();

		// Builtins first.
		if (aBuiltin != bBuiltin)
			return aBuiltin > bBuiltin;

		// Then by stack offset (descending).
		return ZScript::getStackOffset(*a).value_or(INT_MAX) > ZScript::getStackOffset(*b).value_or(INT_MAX);
	});

	bool found_something = false;

	for (auto* type : scope->getLocalDataTypes())
	{
		if (type->isEnum())
		{
			found_something = true;
			process_enum_definition(static_cast<const DataTypeCustom*>(type), scopeIdx, ctx);
		}
	}

	const Function* fn = (scope->isFunction()) ? &static_cast<FunctionScope*>(scope)->function : nullptr;

	for (size_t i = 0; i < localData.size(); ++i)
	{
		Datum* datum = localData[i];
		if (ctx.processed_datum.count(datum)) continue;

		DebugSymbol symbol{};
		symbol.scope_index = scopeIdx;
		symbol.name = datum->getName().value_or("?");
		if (datum->getNode())
			symbol.declaration_line = datum->getNode()->location.first_line;

		if (auto v = datum->getCompileTimeValue())
		{
			symbol.storage = CONSTANT;
			symbol.offset = v.value();
		}
		else if (auto pos = lookupStackPosition(*scope, *datum))
		{
			symbol.storage = LOC_STACK;
			symbol.offset = *pos;
		}
		else if (datum->getGlobalId().has_value())
		{
			symbol.storage = LOC_GLOBAL;
			symbol.offset = datum->getGlobalId().value();
		}
		else if (auto d = dynamic_cast<InternalVariable*>(datum))
		{
			symbol.storage = (d->zasm_register != -1) ? LOC_REGISTER : CONSTANT;
			symbol.offset = (d->zasm_register != -1) ? d->zasm_register : 0;
		}
		else
		{
			continue;
		}

		// Only used for 'this'.
		if (datum->isBuiltIn())
		{
			// Run functions for scripts have a 'this', which should not be hidden in the debugger
			// variables view. This variable is defined on the stack.
			// Class functions have a 'this', which should be hidden in the debugger variables view.
			// This variable is defined by a register.
			// TODO: it would be better if the places that define these builtin variables also
			// declare the metadata necessary for the debugger, instead of hardcoding it here.
			CHECK(datum->getName() == "this");

			if (fn->isBinding() && fn->getClass() && fn->getClass()->internalRefVar != NUL)
			{
				// Class functions for internal classes have a 'this' symbol, but should be hidden.
				symbol.flags |= SYM_FLAG_HIDDEN;
			}
			else if (!fn->node->isRun())
			{
				// Must be a user class method.
				symbol.storage = LOC_REGISTER;
				symbol.offset = CLASS_THISKEY;
				symbol.flags |= SYM_FLAG_HIDDEN;
			} // Otherwise, is a run function.
		}

		if (i == localData.size() - 1 && fn && fn->getFlag(FUNCFLAG_VARARGS))
			symbol.flags |= SYM_FLAG_VARARGS;
		if (auto node = dynamic_cast<const ASTDataDecl*>(datum->getNode()); node && node->getFlag(ASTDataDecl::FL_HIDDEN))
			symbol.flags |= SYM_FLAG_HIDDEN;

		ctx.debug_data.symbols.push_back(symbol);
		ctx.symbol_types.push_back(&datum->type);
		found_something = true;
	}

	if (scope->isClass())
	{
		auto cs = static_cast<ClassScope*>(scope);
		for (auto& [name, var] : cs->getClassData())
		{
			DebugSymbol symbol{};
			symbol.scope_index = scopeIdx;
			symbol.name = var->getName().value_or("?");
			symbol.declaration_line = var->getNode()->location.first_line;

			if (var->is_internal)
			{
				int zasm_register = var->getZasmRegister();
				DCHECK(zasm_register != -1);
				if (zasm_register == -1) continue;

				symbol.storage = LOC_REGISTER;
				symbol.offset = zasm_register;
			}
			else
			{
				symbol.storage = LOC_CLASS;
				symbol.offset = var->getIndex();
			}

			if (auto node = dynamic_cast<const ASTDataDecl*>(var->getNode()); node && node->getFlag(ASTDataDecl::FL_HIDDEN))
				symbol.flags |= SYM_FLAG_HIDDEN;

			ctx.debug_data.symbols.push_back(symbol);
			ctx.symbol_types.push_back(&var->type);
			found_something = true;
		}
	}

	return found_something || !scope->getUsingNamespaces().empty();
}

void ScriptAssembler::process_enum_definition(const DataTypeCustom* customType, int32_t parentIdx, ScopeProcessingContext& ctx)
{
	DebugScope enumScope{};
	enumScope.tag = TAG_ENUM;
	enumScope.name = customType->getName();
	enumScope.parent_index = parentIdx;

	int32_t enumIdx = (int32_t)ctx.debug_data.scopes.size();
	ctx.debug_data.scopes.push_back(enumScope);
	ctx.scope_types.push_back(customType->isLong() ? &DataType::LONG : &DataType::FLOAT);
	ctx.compiler_scopes.push_back(nullptr);
	
	ctx.enum_id_to_scope_index[customType->getCustomId()] = enumIdx;

	auto source = static_cast<const ASTCustomDataTypeDef*>(customType->getSource());
	for (auto decl : source->definition->getDeclarations())
	{
		ctx.processed_datum.insert(decl->manager);

		DebugSymbol symbol{};
		symbol.scope_index = enumIdx;
		symbol.name = decl->getName();
		symbol.storage = CONSTANT;
		symbol.offset = decl->manager->getCompileTimeValue().value_or(0);
		symbol.declaration_line = decl->manager->getNode()->location.first_line;

		ctx.debug_data.symbols.push_back(symbol);
		ctx.symbol_types.push_back(decl->resolvedType);
	}

	// Get the nearest namespace or root tag, and add an import for this enum. This makes every enum
	// member a global variable. If we ever add scoped enums, simply skip this.
	int p = parentIdx;
	while (p != -1)
	{
		auto& scope = ctx.debug_data.scopes[p];
		if (scope.tag == TAG_ROOT || scope.tag == TAG_NAMESPACE)
		{
			scope.imports.push_back(enumIdx);
			break;
		}

		p = scope.parent_index;
	}
}

void ScriptAssembler::finalize_debug_scopes(ScopeProcessingContext& ctx)
{
	// Fixup scope indices for namespaces and class inheritance.
	for (size_t i = 0; i < ctx.compiler_scopes.size(); i++)
	{
		const Scope* scope = ctx.compiler_scopes[i];
		if (!scope) continue;

		for (auto* ns : scope->getUsingNamespaces())
		{
			if (ctx.scope_ptr_to_index.count(ns))
				ctx.debug_data.scopes[i].imports.push_back(ctx.scope_ptr_to_index.at(ns));
		}

		if (scope->isClass())
		{
			auto* cs = static_cast<const ClassScope*>(scope);
			if (cs->user_class.getBaseClass())
			{
				const auto& baseScope = cs->user_class.getBaseClass()->getScope();
				if (ctx.scope_ptr_to_index.count(&baseScope))
					ctx.debug_data.scopes[i].inheritance_index = ctx.scope_ptr_to_index.at(&baseScope);
			}
		}
	}

	// Build types table.
	std::map<UserClass*, int> class_scope_map;
	for (size_t i = 0; i < ctx.debug_data.scopes.size(); i++)
	{
		if (ctx.debug_data.scopes[i].tag == TAG_CLASS && ctx.compiler_scopes[i])
			class_scope_map[&ctx.compiler_scopes[i]->getClass()->user_class] = i;
	}

	DebugTypeBuilder type_builder(ctx.debug_data.types);
	for (size_t i = 0; i < ctx.scope_types.size(); i++)
	{
		if (const DataType* t = ctx.scope_types[i])
			ctx.debug_data.scopes[i].type_id = type_builder.getTypeID(t, class_scope_map, ctx.enum_id_to_scope_index);
	}
	for (size_t i = 0; i < ctx.symbol_types.size(); i++)
		ctx.debug_data.symbols[i].type_id = type_builder.getTypeID(ctx.symbol_types[i], class_scope_map, ctx.enum_id_to_scope_index);
}

std::pair<int32_t,bool> ScriptParser::parseLong(std::pair<string, string> parts, Scope* scope)
{
	// Not sure if this should really check for negative numbers;
	// in most contexts, that's checked beforehand. parts only
	// includes the minus if this is a constant. - Saf
	bool negative=false;
	std::pair<int32_t, bool> rval;
	rval.second=true;
	bool intOneLarger = *lookupOption(*scope, CompileOption::OPT_TRUE_INT_SIZE) != 0;

	if(parts.first.data()[0]=='-')
	{
		negative=true;
		parts.first = parts.first.substr(1);
	}

	if(parts.second.size() > 4)
	{
		rval.second = false;
		parts.second = parts.second.substr(0,4);
	}

	if(parts.first.size() > 6)
	{
		rval.second = false;
		parts.first = parts.first.substr(0,6);
	}

	int32_t firstpart = atoi(parts.first.c_str());
	if(intOneLarger) //MAX_INT should be 214748, but if that is the value, there should be no float component. -V
	{
		if(firstpart > 214748)
		{
			firstpart = 214748;
			rval.second = false;
		}
	}
	else if(firstpart > 214747)
	{
		firstpart = 214747;
		rval.second = false;
	}

	int32_t intval = ((int32_t)(firstpart))*10000;
	//add fractional part; tricky!
	int32_t fpart = 0;


	while(parts.second.length() < 4)
		parts.second += "0";

	for(uint32_t i = 0; i < 4; i++)
	{
		fpart *= 10;
		fpart += parts.second[i] - '0';
	}

	if(intOneLarger && firstpart == 214748 && (negative ? fpart > 3648 : fpart > 3647))
	{
		fpart = negative ? 3648 : 3647;
		rval.second = false;
	}


	rval.first = intval + fpart;
	if(negative)
		rval.first = -rval.first;
	return rval;
}

void ScriptsData::fillFromAssembler(ScriptAssembler& assembler)
{
	for(auto& pair : assembler.getLabelMap())
	{
		Script* script = pair.first;
		auto& pcs = pair.second;
		string const& name = script->getName();
		
		zasm_meta& meta = zasmCompilerResult.theScripts[name].meta;
		zasmCompilerResult.theScripts[name].pc = pcs.first;
		zasmCompilerResult.theScripts[name].end_pc = pcs.second;
		zasmCompilerResult.scriptTypes[name] = script->getType().getId();
		
		meta = script->getMetadata();
		meta.script_type = script->getType().getTrueId();
		meta.script_name = name;
		meta.author = script->getAuthor();
		if(Function* run = script->getRun())
		{
			int32_t ind = 0;
			for(vector<shared_ptr<const string>>::const_iterator it = run->paramNames.begin();
				it != run->paramNames.end(); ++it)
			{
				meta.run_idens[ind] = (**it);
				if(!meta.initd[ind].size())
					meta.initd[ind] = meta.run_idens[ind];
				if (++ind > 7) break; //sanity check
			}
			ind = 0;
			for(vector<DataType const*>::const_iterator it = run->paramTypes.begin();
				it != run->paramTypes.end(); ++it)
			{
				std::optional<DataTypeId> id = assembler.program.getTypeStore().getTypeId(**it);
				meta.run_types[ind] = id ? *id : ZTID_VOID;
				int8_t ty = -1;
				if(id) switch(*id)
				{
					case ZTID_BOOL:
						ty = nswapBOOL;
						break;
					case ZTID_LONG:
						ty = nswapLDEC;
						break;
					case ZTID_FLOAT:
					case ZTID_UNTYPED:
						ty = nswapDEC;
						break;
				}
				if(meta.initd_type[ind] < 0)
					meta.initd_type[ind] = ty;
				if (++ind > 7) break; //sanity check
			}
		}
	}

	zasmCompilerResult.zasm = std::move(assembler.getCode());
	zasmCompilerResult.debugData = std::move(assembler.getDebugData());
}
