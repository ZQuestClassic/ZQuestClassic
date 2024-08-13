#include "base/util.h"
#include "parser/AST.h"
#include "parser/ASTVisitors.h"
#include "parserDefs.h"
#include "MetadataVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <regex>
#include <sstream>

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

static std::string make_uri(std::string path)
{
	if (path == metadata_tmp_path)
		path = metadata_orig_path;

	// For consistent test results no matter the machine.
	if (std::getenv("TEST_ZSCRIPT") != nullptr)
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

static ASTExprIdentifier parseExprIdentifier(const std::string& str)
{
	ASTExprIdentifier ident{};
	int i = 0;
	int j = 0;
	while (j < str.size())
	{
		if (j + 1 < str.size())
		{
			bool matches = false;
			matches |= str[j] == ':' && str[j + 1] == ':';
			matches |= str[j] == '-' && str[j + 1] == '>';
			if (matches)
			{
				ident.components.push_back(str.substr(i, j));
				ident.delimiters.push_back(str.substr(j, 2));
				j += 2;
				i = j;
				continue;
			}
		}

		j++;
		if (j == str.size())
			ident.components.push_back(str.substr(i));
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

	// identifier, followed by an optional and non-captured "[]" or "()"
	static std::string p_ident = "([a-zA-Z_][->:a-zA-Z0-9_]*)(?:\\[\\]|\\(\\))?";
	static std::string p_link = fmt::format("\\{{@link {}[|]?([^}}]+)?\\}}", p_ident);
	static std::string p_shorthand = fmt::format("\\[{}\\]", p_ident);
	static std::string p_regex = fmt::format("{}|{}", p_link, p_shorthand);
	// Supports:
	// @link {symbol}
	// @link {symbol|text}
	// [symbol]
	static const std::regex r(p_regex);
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

		bool is_array = false;
		bool is_fn = false;
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
		{
			symbol_node = fn->node;
			is_fn = true;
		}
		// Parameter?
		if (!symbol_node)
		{
			if (auto fn_node = dynamic_cast<const ASTFuncDecl*>(node))
			{
				for (auto param_node : fn_node->parameters.data())
				{
					if (param_node->getName() == symbol_name)
					{
						symbol_node = param_node;
						break;
					}
				}
			}
		}
		if (!symbol_node)
		{
			auto ident = parseExprIdentifier(symbol_name);
			if (auto datum = lookupDatum(*scope, ident, nullptr))
			{
				symbol_node = datum->getNode();
				is_array = datum->type.isArray();
			}
		}
		if (!symbol_node)
		{
			auto ident = parseExprIdentifier(symbol_name);
			if (auto datum = lookupClassVars(*scope, ident, nullptr))
			{
				symbol_node = datum->getNode();
				is_array = datum->type.isArray();
			}
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

		if (is_array)
			link_text += "[]";
		else if (is_fn)
			link_text += "()";

		auto location = symbol_node->getIdentifierLocation();
		std::string path = location ? location->fname : symbol_node->location.fname;
		auto args = json{
			{"file", make_uri(path)},
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

	auto parsed_comment = node->getParsedDocComment();
	std::ostringstream s;
	s << parsed_comment[""];
	for (const auto& [k, v] : parsed_comment)
	{
		if (k == "")
			continue;
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
		root["symbols"][symbol_id] = {
			// TODO LocationData_location_json
			{"loc", {
				{"range", LocationData_json(symbol_node->location)},
				{"uri", make_uri(symbol_node->location.fname)},
			}},
		};

		auto comment = getComment(symbol_node);
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
	if (name != root_file_name && name != metadata_tmp_path)
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
	auto user_class = host.resolvedType ? host.resolvedType->getUsrClass() : nullptr;
	if (user_class && host.list && host.list->baseType)
	{
		std::string symbol_id = fmt::format("custom.{}", user_class->getType()->getUniqueCustomId());
		appendIdentifier(symbol_id, user_class->getNode(), getSelectionRange(*host.list->baseType));
	}

	auto prev_active = active;
	appendDocSymbol(SymbolKind::Variable, host);
	RecursiveVisitor::caseDataDecl(host, param);
	active = prev_active;
}

void MetadataVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if (host.prototype)
		return;
	if (host.func && host.func->isTemplateSkip())
	{
		for(auto& applied : host.func->get_applied_funcs())
			visit(applied->getNode(), param);
		return;
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
		appendIdentifier(std::to_string(host.binding->id), host.binding->getNode(), host.componentNodes.back()->location);

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

	// TODO: create identifiers for namespace components
	if (auto expr_ident = dynamic_cast<ASTExprIdentifier*>(host.left.get()))
	{
		AST* symbol_node = host.binding->aliased_func ? host.binding->aliased_func->getNode() : host.binding->getNode();
		appendIdentifier(std::to_string(host.binding->id), symbol_node, expr_ident->componentNodes.back()->location);
	}
	else if (auto expr_ident = dynamic_cast<ASTExprArrow*>(host.left.get()))
		appendIdentifier(std::to_string(host.binding->id), host.binding->getNode(), expr_ident->right->location);
	RecursiveVisitor::caseExprCall(host, param);
}

json MetadataVisitor::takeOutput()
{
	return std::move(root);
}
