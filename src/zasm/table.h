#ifndef ZASM_TABLE_H_
#define ZASM_TABLE_H_

#include "base/ints.h"
#include "zasm/defines.h"
#include <initializer_list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class ARGTY : byte
{
    UNUSED_REG,
    READ_REG,
    WRITE_REG,
    READWRITE_REG,
    LITERAL,
    LITERAL_REG,
    COMPARE_OP,
};

#define ARGFL_COMPARE_USED 0x01
#define ARGFL_COMPARE_SET  0x02
#define ARGFL_UNIMPL       0x04
struct script_command
{
	char name[64];
	ASM_DEFINE command;
	byte args;
	ARGTY arg_type[3];
	byte arr_type; //0x1 = string, 0x2 = array
	byte flags; //ARGFL_

	bool is_register(int arg) const
	{
		return arg_type[arg] == ARGTY::READ_REG || arg_type[arg] == ARGTY::WRITE_REG || arg_type[arg] == ARGTY::READWRITE_REG;
	}

	bool writes_to_register(int arg) const
	{
		return arg_type[arg] == ARGTY::WRITE_REG || arg_type[arg] == ARGTY::READWRITE_REG;
	}
};

struct script_variable
{
    char name[64];
    int32_t id;
    word maxcount;
};

const script_command* get_script_command(int command);
const script_command* get_script_command(const std::string& name);
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
const std::vector<int>& get_register_dependencies(int reg);

// Same as get_register_dependencies, but just returns a single register (at most) representing
// some ref that references a specific game entity.
std::optional<int> get_register_ref_dependency(int reg);

bool has_register_dependency(int reg);

#endif
