#include "base/util.h"
#include "parser/AST.h"
#include "parser/CommentUtils.h"
#include "parserDefs.h"
#include "DocVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"
#include <nlohmann/json.hpp>
#include <fmt/format.h>

using namespace ZScript;
using ordered_json = nlohmann::ordered_json;

static ordered_json root;
static ordered_json* active;

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

template <typename T>
static std::string getName(const T& node)
{
	return node.getName();
}
static std::string getName(const ASTDataEnum& node)
{
	if (!node.getIdentifierLocation())
		return "";

	return node.getName();
}

static void linkifyString(std::string& string, const AST* node)
{
	bool check_params = false;
	auto matches = parseForSymbolLinks(string, node, check_params);
	for (auto it = matches.rbegin(); it != matches.rend(); it++)
	{
		if (it->symbol_name.starts_with("#")) // external docs link
		{
			if (it->link_text.empty())
				string.replace(it->pos, it->len, fmt::format("[#{}#]", it->symbol_name.substr(1)));
			else
				string.replace(it->pos, it->len, fmt::format("[#{}|{}#]", it->symbol_name.substr(1), it->link_text));
			continue;
		}
		if (!it->symbol_node)
		{
			string.replace(it->pos, it->len, fmt::format("`{}`", it->link_text));
			continue;
		}

		if (it->link_text.empty())
			string.replace(it->pos, it->len, fmt::format("[@{}@]", getSymbolId(it->symbol_node)));
		else
			string.replace(it->pos, it->len, fmt::format("[@{}|{}@]", getSymbolId(it->symbol_node), it->link_text));
	}
}

static ordered_json getCommentJson(const AST* node)
{
	// TODO: the doc_comment isn't always in a consistent place. For example see `utils::hmm // ...` in `metadata.zh` 
	if (auto n = dynamic_cast<const ASTDataDecl*>(node); n && n->list && node->doc_comment.empty() && !n->list->isEnum())
		node = n->list;
	else if (auto n = dynamic_cast<const ASTDataEnum*>(node); n)
		if (auto custom_type = dynamic_cast<const DataTypeCustom*>(n->baseType->type.get()); custom_type && custom_type->getSource())
			if (auto n = dynamic_cast<const ASTCustomDataTypeDef*>(custom_type->getSource()); n)
				node = n;

	if (node->doc_comment.empty())
		return nullptr;

	auto& parsed_comment = node->getParsedComment();
	ordered_json result = ordered_json::object();

	result["tags"] = ordered_json::array();
	for (auto [k, v] : parsed_comment.tags)
	{
		std::string str = v;
		if (!str.empty())
			linkifyString(str, node);
		result["tags"].push_back(ordered_json::array({k, str}));
	}

	std::string comment = parsed_comment.description;
	util::trimstr(comment);
	linkifyString(comment, node);
	result["text"] = comment;

	return result;
}

template <typename T>
static ordered_json* appendSymbol(SymbolKind kind, const T& node)
{
	std::optional<LocationData> loc;
	if (auto n = dynamic_cast<const ASTDataEnum*>(&node))
		loc = n->location;
	else
		loc = node.getIdentifierLocation();

	(*active).push_back({
		{"name", getName(node)},
		{"id", getSymbolId(&node)},
		{"kind", kind},
		{"comment", getCommentJson(&node)},
		{"children", ordered_json::array()},
	});
	ordered_json* result = &active->back();
	if (loc)
	{
		std::string fname = loc->fname;
#ifdef _WIN32
		util::replstr(fname, "\\", "/");
#endif
		(*result)["location"] = {
			{"file", fname},
			{"line", loc->first_line},
		};
	}
	return result;
}

DocVisitor::DocVisitor(Program& program) : RecursiveVisitor(program)
{
	root = {
		{"files", ordered_json::array()},
	};
	visit(program.getRoot());
}

void DocVisitor::visit(AST& node, void* param)
{
	RecursiveVisitor::visit(node, param);
}

void DocVisitor::caseFile(ASTFile& host, void* param)
{
	root["files"].push_back({
		{"name", host.location.fname},
		{"symbols", ordered_json::array()},
	});

	auto prev_active = active;
	active = &root["files"].back()["symbols"];
	RecursiveVisitor::caseFile(host, param);
	active = prev_active;
}

void DocVisitor::caseClass(ASTClass& host, void* param)
{
	auto symbol = appendSymbol(SymbolKind::Class, host);

	if (auto parent = host.user_class->getBaseClass())
		(*symbol)["parent"] = parent->getName();

	auto prev_active = active;
	active = &(*symbol)["children"];
	RecursiveVisitor::caseClass(host, param);
	active = prev_active;
}

void DocVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	auto symbol = appendSymbol(SymbolKind::Namespace, host);

	auto prev_active = active;
	active = &(*symbol)["children"];
	RecursiveVisitor::caseNamespace(host, param);
	active = prev_active;
}

void DocVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	auto symbol = appendSymbol(SymbolKind::Variable, host);
	(*symbol)["type"] = host.resolvedType->getName();
	if (auto init = host.getInitializer())
	{
		
		scope->in_static_init = host.is_static();
		if (auto value = init->getCompileTimeValue(nullptr, host.getScope()))
			(*symbol)["value"] = *value;
		scope->in_static_init = false;
	}
	RecursiveVisitor::caseDataDecl(host, param);
}

void DocVisitor::caseDataEnum(ASTDataEnum& host, void*)
{
	auto symbol = appendSymbol(SymbolKind::Enum, host);
	DataType const* baseType = host.baseType->resolve_ornull(*scope, this);
	(*symbol)["type"] = baseType->isLong() ? "long" : "int";

	if (auto prefix = host.getDocumentationPrefix(); prefix && !prefix->empty())
		(*symbol)["prefix"] = *prefix;

	auto prev_active = active;
	active = &(*symbol)["children"];
	scope->in_static_init = host.is_static;
	for (auto* decl : host.getDeclarations())
	{
		auto value = decl->getInitializer()->getCompileTimeValue(nullptr, decl->getScope());
		auto symbol = appendSymbol(SymbolKind::EnumMember, *decl);
		(*symbol)["type"] = decl->getInitializer()->getReadType(decl->getScope(), nullptr)->isLong() ? "long" : "int";
		(*symbol)["value"] = *value;
	}
	scope->in_static_init = false;
	active = prev_active;
}

void DocVisitor::caseFuncDecl(ASTFuncDecl& host, void*)
{
	if (host.prototype || host.getFlag(FUNCFLAG_NIL))
		return;

	auto kind = host.getFlag(FUNCFLAG_CONSTRUCTOR) ? SymbolKind::Constructor : SymbolKind::Function;
	auto symbol = appendSymbol(kind, host);
	(*symbol)["returnType"] = host.returnType->type->getName();
	(*symbol)["parameters"] = ordered_json::array();
	for (auto param : host.parameters)
	{
		ordered_json j_param = {
			{"name", param->getName()},
			{"type", param->resolvedType->getName()},
		};
		(*symbol)["parameters"].push_back(std::move(j_param));
	}
	for (int i = 0; i < host.optparams.size(); i++)
	{
		std::string snippet = getSourceCodeSnippet(host.optparams[i]->location);
		(*symbol)["parameters"][host.parameters.size() - host.optparams.size() + i]["default"] = snippet;
	}
	if (host.getFlag(FUNCFLAG_VARARGS))
		(*symbol)["varargs"] = true;
}

std::string DocVisitor::getOutput()
{
	return root.dump(2);
}
