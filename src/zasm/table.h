#ifndef _ZASM_TABLE_H_
#define _ZASM_TABLE_H_

#include "base/zdefs.h"
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>

const script_command& get_script_command(int command);
std::optional<int> get_script_command(const std::string& name);
std::pair<const script_variable*, int> get_script_variable(int var);
std::optional<int> get_script_variable(const std::string& var_name);

struct CommandDependency
{
	int reg;
	ARGTY type;
};
std::initializer_list<CommandDependency> get_command_implicit_dependencies(int command);

// Many registers use the value within data registers when evaluating its value.
// For example, GLOBALRAM uses both rINDEX and rINDEX2 to index a specific array.
std::initializer_list<int> get_register_dependencies(int reg);

// Same as get_register_dependencies, but just returns a single register (at most) representing
// some ref that references a specific game entity.
std::optional<int> get_register_ref_dependency(int reg);

bool has_register_dependency(int reg);

#endif
