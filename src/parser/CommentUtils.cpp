#include "parser/CommentUtils.h"
#include "parser/AST.h"
#include "parser/Scope.h"
#include "parser/ZScript.h"
#include <regex>
#include <string>
#include <vector>

using namespace ZScript;

static std::map<const AST*, uint16_t> symbolMap;
static uint16_t nextSymbolId = 1;

uint16_t getSymbolId(const AST* node)
{
	if (auto n = dynamic_cast<const ASTCustomDataTypeDef*>(node))
		return getSymbolId(n->definition.get());

	auto it = symbolMap.find(node);
	if (it != symbolMap.end())
		return it->second;

	return symbolMap[node] = nextSymbolId++;
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

std::vector<ParseCommentResult> parseForSymbolLinks(std::string comment, const AST* node, bool check_params)
{
	Scope* scope = scope = node->getScope();
	if (!scope)
		return {};

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

	std::vector<ParseCommentResult> matches;
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
		ParseCommentResult match{symbol_name, (int)pos, (int)len, link_text, nullptr};
		matches.push_back(match);
		it++;
	}

	for (auto it = matches.begin(); it != matches.end(); it++)
	{
		auto& [symbol_name, pos, len, link_text, symbol_node] = *it;

		bool had_link_text = !link_text.empty();
		if (!had_link_text)
			link_text = symbol_name;

		bool is_array = false;
		bool is_fn = false;
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
		if (!symbol_node && check_params)
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
				if (auto custom_type = dynamic_cast<const DataTypeCustom*>(type); custom_type)
					symbol_node = custom_type->getSource();
			}
		}

		if (!had_link_text)
		{
			if (is_array)
				link_text += "[]";
			else if (is_fn)
				link_text += "()";
		}

		if (!symbol_node)
			logDebugMessage(fmt::format("could not resolve symbol \"{}\"", symbol_name).c_str());
	}

	return matches;
}
