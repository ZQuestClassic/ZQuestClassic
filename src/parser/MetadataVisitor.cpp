#include "base/util.h"
#include "parser/AST.h"
#include "parser/ASTVisitors.h"
#include "parser/CommentUtils.h"
#include "parser/Types.h"
#include "parserDefs.h"
#include "MetadataVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <sstream>

using namespace ZScript;
using json = nlohmann::ordered_json;

std::string metadata_tmp_path;
std::string metadata_orig_path;

static json root;
static json* active;

// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolKind
enum class SymbolKind
{
	File = 1,
	Module,
	Namespace,
	Package,
	Class,
	Method,
	Property,
	Field,
	Constructor,
	Enum,
	Interface,
	Function,
	Variable,
	Constant,
	String,
	Number,
	Boolean,
	Array,
	Object,
	Key,
	Null,
	EnumMember,
	Struct,
	Event,
	Operator,
	TypeParameter,
};

static std::string make_uri(std::string path)
{
	if (path == metadata_tmp_path)
		path = metadata_orig_path;

	// For consistent test results no matter the machine.
	if (is_test())
		return !path.empty() ? fs::path(path).filename().string() : "";

	path = (fs::current_path() / path).string();

#ifdef _WIN32
		std::string uri = "file:///" + path;
		util::replstr(uri, "\\", "/");
#else
		std::string uri = "file://" + path;
#endif
	return uri;
}

static auto LocationData_json(const LocationData& loc)
{
	assert(loc.first_line > 0 && loc.first_column > 0);
	assert(loc.last_line > 0 && loc.last_column > 0);
	return json{
		{"start", {
			{"line", loc.first_line - 1}, {"character", loc.first_column - 1},
		}},
		{"end", {
			{"line", loc.last_line - 1}, {"character", loc.last_column - 1},
		}}
	};
}

static auto LocationData_pos_json(const LocationData& loc)
{
	assert(loc.first_line == loc.last_line);
	return json{
		{"line", loc.first_line - 1}, {"character", loc.first_column - 1}, {"length", loc.last_column - loc.first_column},
	};
}

static std::string url_encode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

static void linkifyComment(std::string& comment, const AST* node)
{
	bool check_params = true;
	auto matches = parseForSymbolLinks(comment, node, check_params);
	if (matches.empty())
		return;

	for (auto it = matches.rbegin(); it != matches.rend(); it++)
	{
		if (!it->symbol_node)
		{
			comment.replace(it->pos, it->len, fmt::format("`{}`", it->link_text));
			continue;
		}

		auto location = it->symbol_node->getIdentifierLocation().value_or(it->symbol_node->location);
		auto args = json{
			{"file", make_uri(location.fname)},
			{"position", LocationData_json(location)},
		};
		std::string command = fmt::format("zscript.openLink?{}", url_encode(args.dump()));
		comment.replace(it->pos, it->len, fmt::format("[`{}`](command:{})", it->link_text, command));
	}
}

template <typename T>
static std::string getName(const T& node)
{
	return node.getName();
}
static std::string getName(const ASTFile& node)
{
	if (!node.scope) return node.location.fname;
	return node.scope->getName().value_or("<file>");
}

template <typename T>
static LocationData getSelectionRange(const T& node)
{
	if (auto loc = node.getIdentifierLocation())
		return *loc;
	return node.location;
}

template <typename T>
static void appendDocSymbol(SymbolKind kind, const T& node)
{
	(*active).push_back({
		{"name", getName(node)},
		{"kind", kind},
		{"range", LocationData_json(node.location)},
		{"selectionRange", LocationData_json(getSelectionRange(node))},
		{"children", json::array()},
	});
	active = &(*active).back()["children"];
}

static std::string getComment(const AST* node)
{
	if (node->doc_comment.empty())
		return "";

	auto parsed_comment = node->getParsedComment();
	std::ostringstream s;
	if (!parsed_comment.description.empty())
		s << parsed_comment.description;
	for (const auto& [k, v] : parsed_comment.tags)
	{
		if (k == "zasm")
			continue;
		if (k == "vargs")
			continue;
		if (k == "zasm_var")
			continue;
		if (k == "internal_array")
			continue;

		s << "\n\n**@" << k << "**";
		if (!v.empty())
			s <<  " " << v;
	}

	std::string comment = s.str();
	util::trimstr(comment);
	return comment;
}

static void appendIdentifier(std::string symbol_id, const AST* symbol_node, const LocationData& loc)
{
	if (!symbol_node)
		return;

	// TODO: the doc_comment isn't always in a consistent place. For example see `utils::hmm // ...` in `metadata.zh` 
	if (auto n = dynamic_cast<const ASTDataDecl*>(symbol_node); n && n->list && symbol_node->doc_comment.empty())
		symbol_node = n->list;

	if (!root["symbols"].contains(symbol_id))
	{
		auto loc = symbol_node->getIdentifierLocation().value_or(symbol_node->location);
		root["symbols"][symbol_id] = {
			// TODO LocationData_location_json
			{"loc", {
				{"range", LocationData_json(loc)},
				{"uri", make_uri(loc.fname)},
			}},
		};

		auto comment = getComment(symbol_node);
		if (!comment.empty())
		{
			linkifyComment(comment, symbol_node);
			root["symbols"][symbol_id]["doc"] = comment;
		}
	}

	root["identifiers"].push_back({
		{"loc", LocationData_pos_json(loc)},
		{"symbol", symbol_id},
	});
}

static std::string getSymbolId(const DataType* type)
{
	if (auto t = dynamic_cast<const DataTypeCustom*>(type))
	{
		// For tests, use an id that is more stable (but not guaranteed to be unique).
		if (is_test())
			return type->getName();

		return fmt::format("custom.{}", t->getCustomId());
	}

	// unexpected, should have only called this with DataTypeCustom.
	assert(false);
	return "custom.?";
}

static std::string getSymbolId(const Datum* datum)
{
	// For tests, use an id that is more stable (but not guaranteed to be unique).
	if (is_test())
	{
		std::string name = datum->getName().value();
		if (datum->scope.getName())
			return fmt::format("{}-{}", fs::path(datum->scope.getName().value()).filename().string(), name);
		return name;
	}

	return std::to_string(datum->id);
}

static std::string getSymbolId(const Function* fn)
{
	// For tests, use an id that is more stable (but not guaranteed to be unique).
	if (is_test())
	{
		if (fn->getFlag(FUNCFLAG_CONSTRUCTOR))
			return fmt::format("ctor-{}", fn->name);
		if (fn->getFlag(FUNCFLAG_DESTRUCTOR))
			return fmt::format("dtor-{}", fn->name);
		if (fn->templ_bound_ts.size())
		{
			std::string name;
			for (const auto& type : fn->templ_bound_ts)
				name += type->getName() + "-";
			name += fn->name;
			return name;
		}

		return fn->name;
	}

	return fmt::format("fn.{}", fn->id);
}

static std::string getSymbolId(const Script* script)
{
	return fmt::format("script.{}", script->getName());
}

MetadataVisitor::MetadataVisitor(Program& program, std::string root_file_name)
	: RecursiveVisitor(program), root_file_name(root_file_name), is_enabled(false)
{
	root = {
		{"currentFileSymbols", json::array()},
		{"symbols",  json::object()},
		{"identifiers",  json::array()},
	};
	active = &root["currentFileSymbols"];
	visit(program.getRoot());
}

void MetadataVisitor::visit(AST& node, void* param)
{
	node.undisable();
	RecursiveVisitor::visit(node, param);
}

void MetadataVisitor::caseFile(ASTFile& host, void* param)
{
	auto name = getName(host);
	if (name != "<root>" && name != root_file_name && name != metadata_tmp_path)
		return;

	is_enabled = name == metadata_tmp_path || metadata_tmp_path.empty();
	RecursiveVisitor::caseFile(host, param);
}

void MetadataVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	if (!is_enabled)
	{
		RecursiveVisitor::caseImportDecl(host, param);
		return;
	}

	if (host.location.fname.empty())
	{
		RecursiveVisitor::caseImportDecl(host, param);
		return;
	}

	auto prev = host.location;
	host.location = {};
	host.location.fname = (fs::current_path() / host.getFilename()).string();
	appendIdentifier(fmt::format("import-{}", host.location.fname), &host, getSelectionRange(*host.getImportString()));
	host.location = prev;

	RecursiveVisitor::caseImportDecl(host, param);
}

void MetadataVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	auto prev_active = active;
	appendDocSymbol(SymbolKind::Namespace, host);
	RecursiveVisitor::caseNamespace(host, param);
	active = prev_active;
}

void MetadataVisitor::caseScript(ASTScript& host, void* param)
{
	if (!host.script)
		return;

	std::string symbol_id = getSymbolId(host.script);
	appendIdentifier(symbol_id, &host, getSelectionRange(*host.identifier));

	auto prev_active = active;
	appendDocSymbol(SymbolKind::Module, host);
	RecursiveVisitor::caseScript(host, param);
	active = prev_active;
}

void MetadataVisitor::caseClass(ASTClass& host, void* param)
{
	if (host.user_class)
	{
		std::string symbol_id = getSymbolId(host.user_class->getType());
		appendIdentifier(symbol_id, &host, getSelectionRange(*host.identifier));
	}

	auto prev_active = active;
	appendDocSymbol(SymbolKind::Class, host);
	RecursiveVisitor::caseClass(host, param);
	active = prev_active;
}

void MetadataVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	auto user_class = host.resolvedType ? host.resolvedType->getUsrClass() : nullptr;
	if (user_class && host.list && host.list->baseType)
	{
		// TODO: this user_class branch could be removed, but it catches things like `const npc[]`
		// (ex: auto npcs = Screen->NPCs), since in that case `custom_type->getSource()` is currently null
		// for some reason.
		std::string symbol_id = getSymbolId(user_class->getType());
		appendIdentifier(symbol_id, user_class->getNode(), getSelectionRange(*host.list->baseType));
	}
	else if (host.resolvedType && host.list)
	{
		auto resolvedType = host.resolvedType;
		if (host.resolvedType->isArray())
			resolvedType = &host.resolvedType->getBaseType();
		if (auto custom_type = dynamic_cast<const DataTypeCustom*>(resolvedType); custom_type)
		{
			std::string symbol_id = getSymbolId(custom_type);
			appendIdentifier(symbol_id, custom_type->getSource(), getSelectionRange(*host.list->baseType));
		}
	}

	if (host.manager)
	{
		std::string symbol_id = getSymbolId(host.manager);
		appendIdentifier(symbol_id, &host, getSelectionRange(*host.identifier));
	}

	auto prev_active = active;
	appendDocSymbol(SymbolKind::Variable, host);
	RecursiveVisitor::caseDataDecl(host, param);
	active = prev_active;
}

void MetadataVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	auto* base_type = host.baseType.get();
	if (base_type)
	{
		if (auto custom_type = dynamic_cast<const DataTypeCustom*>(base_type->type.get()); custom_type)
		{
			std::string symbol_id = getSymbolId(custom_type);
			appendIdentifier(symbol_id, custom_type->getSource(), getSelectionRange(*custom_type->getSource()));
		}
	}

	auto prev_active = active;
	appendDocSymbol(SymbolKind::Enum, host);
	for (auto* decl : host.getDeclarations())
	{
		if (decl->manager)
		{
			std::string symbol_id = getSymbolId(decl->manager);
			appendIdentifier(symbol_id, decl, getSelectionRange(*decl->identifier));
		}

		auto prev_active = active;
		appendDocSymbol(SymbolKind::EnumMember, *decl);
		active = prev_active;
	}
	active = prev_active;
}

void MetadataVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if (host.prototype)
		return;

	if (host.func)
	{
		std::string symbol_id = getSymbolId(host.func);
		appendIdentifier(symbol_id, &host, getSelectionRange(*host.identifier));
	}

	if (host.func && host.func->isTemplateSkip())
	{
		for(auto& applied : host.func->get_applied_funcs())
			visit(applied->getNode(), param);
		return;
	}

	if (host.returnType->type)
	{
		auto type = host.returnType.get()->type.get();
		if (auto custom_type = dynamic_cast<const DataTypeCustom*>(type); custom_type)
		{
			std::string symbol_id = getSymbolId(custom_type);
			appendIdentifier(symbol_id, custom_type->getSource(), getSelectionRange(*host.returnType));
		}
	}

	auto prev_active = active;
	auto kind = host.getFlag(FUNCFLAG_CONSTRUCTOR) ? SymbolKind::Constructor : SymbolKind::Function;
	appendDocSymbol(kind, host);
	if (!host.prototype)
		visit(host.block.get(), param);
	active = prev_active;
}

void MetadataVisitor::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	// TODO: create identifiers for namespace components
	if (host.binding && !host.componentNodes.empty())
		appendIdentifier(getSymbolId(host.binding), host.binding->getNode(), host.componentNodes.back()->location);

	RecursiveVisitor::caseExprIdentifier(host, param);
}

void MetadataVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	if (host.u_datum && host.u_datum->getClass())
	{
		auto id = fmt::format("{}::{}", host.u_datum->getClass()->getName(), host.u_datum->getName().value());
		appendIdentifier(id, host.u_datum->getNode()->list, host.right->location);
	}
	RecursiveVisitor::caseExprArrow(host, param);
}

void MetadataVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	if (!host.binding)
	{
		RecursiveVisitor::caseExprCall(host, param);
		return;
	}

	std::string symbol_id;
	AST* symbol_node;
	if (host.binding->aliased_func)
	{
		symbol_id = getSymbolId(host.binding);
		symbol_node = host.binding->aliased_func->getNode();
	}
	else if (host.binding->data_decl_source_node)
	{
		symbol_id = getSymbolId(host.binding->data_decl_source_node->manager);
		symbol_node = host.binding->data_decl_source_node;
	}
	else
	{
		symbol_id = getSymbolId(host.binding);
		symbol_node = host.binding->getNode();
	}

	// TODO: create identifiers for namespace components
	if (auto expr_ident = dynamic_cast<ASTExprIdentifier*>(host.left.get()))
	{
		appendIdentifier(symbol_id, symbol_node, expr_ident->componentNodes.back()->location);
	}
	else if (auto expr_ident = dynamic_cast<ASTExprArrow*>(host.left.get()))
	{
		appendIdentifier(symbol_id, symbol_node, expr_ident->right->location);
	}

	RecursiveVisitor::caseExprCall(host, param);
}

json MetadataVisitor::takeOutput()
{
	return std::move(root);
}
