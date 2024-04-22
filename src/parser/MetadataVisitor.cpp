#include "base/util.h"
#include "parser/AST.h"
#include "parserDefs.h"
#include "MetadataVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <regex>

using namespace ZScript;
using json = nlohmann::ordered_json;

std::string metadata_tmp_path;
std::string metadata_orig_path;

json root;
json* active;

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

static auto LocationData_json(const LocationData& loc)
{
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

static ASTExprIdentifier parseExprIdentifier(std::string str)
{
	ASTExprIdentifier ident;
	std::vector<std::string> components = util::split(str, "::");
	for (auto& str : components)
	{
		ident.components.push_back(std::move(str));
		ident.delimiters.push_back("::");
	}
	return ident;
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

static void parseCommentForLinks(std::string& comment, const AST* node)
{
	Scope* scope = scope = node->getScope();
	if (!scope)
		return;

	// Supports:
	// @link {symbol}
	// @link {symbol|text}
	// [symbol]
	const std::regex r("\\{\\@link ([a-zA-Z_][->a-zA-Z0-9_:]*)[|]?([^}]+)?\\}|\\[([a-zA-Z_][->a-zA-Z0-9_:]*)\\]");
	std::sregex_iterator it(comment.begin(), comment.end(), r);
	std::sregex_iterator end;

	std::vector<std::tuple<std::string, int, int, std::string>> matches;
	while (it != end)
	{
		auto pos = (*it).position(0);
		auto len = (*it).length(0);
		std::string symbol_name, link_text;
		if ((*it)[3].matched)
		{
			symbol_name = (*it)[3].str();
		}
		else
		{
			symbol_name = (*it)[1].str();
			link_text = (*it)[2].matched ? (*it)[2].str() : "";
		}
		matches.emplace_back(symbol_name, pos, len, link_text);
		it++;
	}

	for (auto it = matches.rbegin(); it != matches.rend(); it++)
	{
		auto& [symbol_name, pos, len, link_text] = *it;
		if (link_text.empty())
			link_text = symbol_name;

		const AST* symbol_node = nullptr;
		auto fn = lookupGetter(*scope, symbol_name);
		if (!fn)
			fn = lookupSetter(*scope, symbol_name);
		if (!fn)
		{
			// TODO: possibly support param types for overloaded functions: `CenterX(npc, bool)`
			auto ident = parseExprIdentifier(symbol_name);
			auto fns = lookupFunctions(*scope, ident.components, ident.delimiters, {}, false, false, true);
			if (!fns.empty())
				fn = fns[0];
		}
		if (fn && fn->node)
			symbol_node = fn->node;
		if (!symbol_node)
		{
			auto ident = parseExprIdentifier(symbol_name);
			if (auto datum = lookupDatum(*scope, ident, nullptr))
				symbol_node = datum->getNode();
		}
		if (!symbol_node)
		{
			auto ident = parseExprIdentifier(symbol_name);
			if (auto datum = lookupClassVars(*scope, ident, nullptr))
				symbol_node = datum->getNode();
		}
		if (!symbol_node)
		{
			auto ident = parseExprIdentifier(symbol_name);
			if (auto type = lookupDataType(*scope, ident, nullptr))
			{
				if (type->isUsrClass())
					symbol_node = type->getUsrClass()->getNode();
			}
		}

		if (!symbol_node)
		{
			comment.replace(pos, len, fmt::format("`{}`", link_text));
			logDebugMessage(fmt::format("could not resolve symbol \"{}\"", symbol_name).c_str());
			continue;
		}

		auto location = symbol_node->getIdentifierLocation();
		std::string path = location ? location->fname : symbol_node->location.fname;
		if (path == "ZQ_BUFFER" || path == metadata_tmp_path)
			path = metadata_orig_path;
		auto args = json{
			{"file", path},
			{"position", LocationData_json(*location)},
		};
		std::string command = fmt::format("zscript.openLink?{}", url_encode(args.dump()));
		comment.replace(pos, len, fmt::format("[`{}`](command:{})", link_text, command));
	}
}

template <typename T>
static std::string getName(const T& node)
{
	return node.getName();
}
static std::string getName(const ASTFile& node)
{
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

static std::string cleanComment(std::string comment)
{
	if (comment.empty())
		return comment;

	std::string copy = comment;
	util::trimstr(copy);
	return copy;
}

static void appendIdentifier(std::string symbol_id, const AST* symbol_node, const LocationData& loc)
{
	if (!symbol_node)
		return;

	if (!root["symbols"].contains(symbol_id))
	{
#ifdef _WIN32
		std::string uri = "file:///" + symbol_node->location.fname;
		util::replstr(uri, "\\", "/");
#else
		std::string uri = "file://" + symbol_node->location.fname;
#endif
		root["symbols"][symbol_id] = {
			// TODO LocationData_location_json
			{"loc", {
				{"range", LocationData_json(symbol_node->location)},
				{"uri", uri},
			}},
		};

		auto comment = cleanComment(symbol_node->doc_comment);
		if (!comment.empty())
		{
			parseCommentForLinks(comment, symbol_node);
			root["symbols"][symbol_id]["doc"] = comment;
		}
	}

	root["identifiers"].push_back({
		{"loc", LocationData_pos_json(loc)},
		{"symbol", symbol_id},
	});
}

MetadataVisitor::MetadataVisitor(Program& program)
	: program(program)
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
	RecursiveVisitor::visit(node, param);
}

void MetadataVisitor::caseFile(ASTFile& host, void* param)
{
	auto name = getName(host);
	if (name != "ZQ_BUFFER" && !name.ends_with("tmp.zs"))
		return;

	RecursiveVisitor::caseFile(host, param);
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
	auto prev_active = active;
	appendDocSymbol(SymbolKind::Module, host);
	RecursiveVisitor::caseScript(host, param);
	active = prev_active;
}

void MetadataVisitor::caseClass(ASTClass& host, void* param)
{
	auto prev_active = active;
	appendDocSymbol(SymbolKind::Class, host);
	RecursiveVisitor::caseClass(host, param);
	active = prev_active;
}

void MetadataVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	auto prev_active = active;
	appendDocSymbol(SymbolKind::Variable, host);
	RecursiveVisitor::caseDataDecl(host, param);
	active = prev_active;
}

void MetadataVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if (host.prototype)
		return;

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
	if (host.binding && !host.isConstant() && !host.binding->isBuiltIn())
		appendIdentifier(std::to_string(host.binding->id), host.binding->getNode(), host.componentNodes.back()->location);

	RecursiveVisitor::caseExprIdentifier(host, param);
}

void MetadataVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	if (host.u_datum && host.u_datum->getClass())
	{
		auto id = fmt::format("{},{}", host.u_datum->getClass()->getType()->getUniqueCustomId(), host.u_datum->getIndex());
		appendIdentifier(id, host.u_datum->getNode(), host.right->location);
	}
	RecursiveVisitor::caseExprArrow(host, param);
}

void MetadataVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	// TODO: create identifiers for namespace components
	if (auto expr_ident = dynamic_cast<ASTExprIdentifier*>(host.left.get()))
		appendIdentifier(std::to_string(host.binding->id), host.binding->getNode(), expr_ident->componentNodes.back()->location);
	RecursiveVisitor::caseExprCall(host, param);
}

std::string MetadataVisitor::getOutput()
{
	return root.dump(2);
}
