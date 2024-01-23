#include "base/zdefs.h"
#include <initializer_list>

struct CommandDependency
{
	int reg;
	ARGTY type;
};
std::initializer_list<CommandDependency> get_command_implicit_dependencies(int command);

// Many registers use the value within data registers when evaluating its value.
// For example, GLOBALRAM uses both rINDEX and rINDEX2 to index a specific array.
std::initializer_list<int> get_register_dependencies(int reg);
