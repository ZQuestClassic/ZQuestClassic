#include "base/ints.h"
#include "parser/parserDefs.h"
#include "zasm/table.h"
#include <optional>
#include <sstream>
#include <vector>
#include <string>
#include <fmt/format.h>
#include <fmt/ranges.h>

std::string zasm_var_to_string(int32_t var)
{
	const auto& [sv, w] = get_script_variable(var);
	if (!sv)
		return "(null)";

	if (sv->maxcount > 0)
	{
		if (strcmp(sv->name, "A") == 0)
			return fmt::format("{}{}", sv->name, w + 1);

		return fmt::format("{}{}", sv->name, w);
	}

	return sv->name;
}

std::string zasm_arg_to_string(int32_t arg, ARGTY arg_ty)
{
	switch(arg_ty)
	{
		case ARGTY::UNUSED_REG:
		case ARGTY::READ_REG:
		case ARGTY::WRITE_REG:
		case ARGTY::READWRITE_REG:
			return zasm_var_to_string(arg);
		case ARGTY::LITERAL_REG:
			return "@" + zasm_var_to_string(arg);
		case ARGTY::LITERAL:
			return std::to_string(arg);
		case ARGTY::COMPARE_OP:
			return CMP_STR(arg);
		default:
			return "ERROR";
	}
}

std::string zasm_op_to_string(word scommand, int32_t arg1, int32_t arg2, int32_t arg3, std::vector<int>* argvec, std::string* argstr)
{
	std::stringstream ss;
	auto c = get_script_command(scommand);

	int args[] = {arg1, arg2, arg3};
	#define SS_WIDTH(w) std::setw(w) << std::setfill(' ') << std::left
	ss << SS_WIDTH(15) << c->name;
	if (c->args >= 1)
	{
		ss << " " << SS_WIDTH(16) << zasm_arg_to_string(args[0], c->arg_type[0]);
	}
	for(int q = 1; q < c->args; ++q)
	{
		ss << " " << SS_WIDTH(7) << zasm_arg_to_string(args[q], c->arg_type[q]);
	}
	if (c->arr_type)
	{
		ss << " " << SS_WIDTH(7);
		if(c->arr_type == 1)
		{
			// NOTE: currently possible to encounter a null pointer here, since the qst loading code
			// will create no string for these commands if the size was 0.
			if (argstr)
				ss << '"' << *argstr << '"';
		}
		else //if(c->arr_type == 2)
		{
			ss << fmt::format("{{ {} }}", fmt::join(*argvec, ", "));
		}
	}

	return ss.str();
}

std::string zasm_op_to_string(word scommand, int32_t arg1, int32_t arg2, int32_t arg3)
{
	std::stringstream ss;
	auto c = get_script_command(scommand);
	
	int args[] = {arg1, arg2, arg3};
	#define SS_WIDTH(w) std::setw(w) << std::setfill(' ') << std::left
	ss << SS_WIDTH(15) << c->name;
	if (c->args >= 1)
	{
		ss << " " << SS_WIDTH(16) << zasm_arg_to_string(args[0], c->arg_type[0]);
	}
	for(int q = 1; q < c->args; ++q)
	{
		ss << " " << SS_WIDTH(7) << zasm_arg_to_string(args[q], c->arg_type[q]);
	}

	return ss.str();
}

std::string zasm_op_to_string(const ffscript& c)
{
	return zasm_op_to_string(c.command, c.arg1, c.arg2, c.arg3, c.vecptr, c.strptr);
}

std::string zasm_op_to_string(word scommand)
{
	return get_script_command(scommand)->name;
}

std::optional<int> parse_zasm_compare_arg(char const* buf)
{
	int cmp = 0;
	if(buf[0] == 'B')
	{
		cmp |= CMP_BOOL;
		++buf;
	}
	if(buf[0] == 'I')
	{
		cmp |= CMP_SETI;
		++buf;
	}
	if(!strcmp(buf,"<"))
		cmp |= CMP_LT;
	else if(!strcmp(buf,">"))
		cmp |= CMP_GT;
	else if(!strcmp(buf,"<="))
		cmp |= CMP_LE;
	else if(!strcmp(buf,">="))
		cmp |= CMP_GE;
	else if(!strcmp(buf,"=="))
		cmp |= CMP_EQ;
	else if(!strcmp(buf,"!="))
		cmp |= CMP_NE;
	else if(!strcmp(buf,"Never"))
		;
	else if(!strcmp(buf,"Always"))
		cmp |= CMP_FLAGS;
	else return nullopt; //nonmatching
	return cmp;
}

static std::optional<int> parse_zasm_numerical_arg(const std::vector<std::string>& tokens, int& token_index, ARGTY type)
{
	switch (type)
	{
		case ARGTY::READ_REG:
		case ARGTY::WRITE_REG:
		case ARGTY::READWRITE_REG:
		case ARGTY::UNUSED_REG:
		{
			auto& text = tokens[token_index++];
			if (auto var = get_script_variable(text))
				return *var;
			return std::nullopt;
		}
		break;

		case ARGTY::LITERAL_REG:
		{
			auto& text = tokens[token_index++];
			if (text[0] != '@')
				return std::nullopt;

			if (auto var = get_script_variable(text.substr(1)))
				return *var;
			return std::nullopt;
		}

		case ARGTY::LITERAL:
		{
			auto& text = tokens[token_index++];
			return std::stoi(text);
		}

		case ARGTY::COMPARE_OP:
		{
			auto& text = tokens[token_index++];
			return parse_zasm_compare_arg(text.c_str()).value();
		}
	}

	return std::nullopt;
}

ffscript parse_zasm_op(std::string op_str)
{
	ffscript result{};

	auto tokens = util::split_args(op_str);
	assert(tokens.size() > 0);

	auto sc = get_script_command(tokens[1]);
	result.command = sc->command;

	int token_index = 2;
	if (sc->args >= 1)
		result.arg1 = parse_zasm_numerical_arg(tokens, token_index, sc->arg_type[0]).value();
	if (sc->args >= 2)
		result.arg2 = parse_zasm_numerical_arg(tokens, token_index, sc->arg_type[1]).value();
	if (sc->args >= 3)
		result.arg3 = parse_zasm_numerical_arg(tokens, token_index, sc->arg_type[2]).value();

	if (sc->arr_type == 1)
	{
		result.strptr = new std::string(tokens[token_index++]);
	}
	else if (sc->arr_type == 2)
	{
		result.vecptr = new std::vector<int32_t>();
		CHECK(tokens[token_index++] == "{");
		while (token_index < tokens.size() - 1)
		{
			int val = std::stoi(tokens[token_index++]);
			result.vecptr->push_back(val);
		}
		CHECK(tokens[token_index++] == "}");
	}

	return result;
}
