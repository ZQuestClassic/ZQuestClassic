#ifndef ZASM_TABLE_H_
#define ZASM_TABLE_H_

#include "base/ints.h"
#include "components/zasm/defines.h"
#include <array>
#include <initializer_list>
#include <optional>
#include <string>
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

// Reads the result of the last comparison (see ARGFL_COMPARE_SET).
#define ARGFL_COMPARE_USED 0x01
// Produces a comparison result for the ARGFL_COMPARE_USED commands that follow.
#define ARGFL_COMPARE_SET  0x02
#define ARGFL_UNIMPL       0x04
// A jump to the pc in arg1, possibly conditional. GOTOR, GOTOTABLE, GOTORANGES and returns are
// left out on purpose.
#define ARGFL_GOTO         0x08
// Suspends the script until a later frame.
#define ARGFL_WAIT         0x10
// No side effects other than potentially writing some value to a register pointed at by one of
// its args. The optimizer uses this to know if it is safe to remove a command, given its register
// output is not needed.
#define ARGFL_PURE         0x20
// May stop the script or return something other than RUNSCRIPT_OK (like deleting the sprite that
// is running it). These are the only commands that the JIT checks the result of.
#define ARGFL_MAY_STOP     0x40
struct script_command
{
	char name[64];
	ASM_DEFINE command;
	byte args;
	ARGTY arg_type[3];
	byte arr_type; //0x1 = string, 0x2 = array
	byte flags; //ARGFL_

	constexpr bool is_register(int arg) const
	{
		return arg_type[arg] == ARGTY::READ_REG || arg_type[arg] == ARGTY::WRITE_REG || arg_type[arg] == ARGTY::READWRITE_REG;
	}

	constexpr bool writes_to_register(int arg) const
	{
		return arg_type[arg] == ARGTY::WRITE_REG || arg_type[arg] == ARGTY::READWRITE_REG;
	}
};

struct script_variable
{
    char name[64];
    int32_t id;
};

const script_command* get_script_command(int command);

// The ARGFL_ flags of each command, by command. Commands without an entry in the command table
// (like deleted ones) have none.
extern const std::array<byte, NUMCOMMANDS> script_command_flags;
// The flags of the terminal command 0xFFFF.
constexpr byte null_command_flags = ARGFL_MAY_STOP;

inline byte get_script_command_flags(int command)
{
	if ((unsigned)command < NUMCOMMANDS)
		return script_command_flags[command];
	return command == 0xFFFF ? null_command_flags : 0;
}

inline bool command_is_goto(int command)
{
	return get_script_command_flags(command) & ARGFL_GOTO;
}
inline bool command_is_wait(int command)
{
	return get_script_command_flags(command) & ARGFL_WAIT;
}
// Waits, plus RUNGENFRZSCR: commands that suspend the script and yield
// control back to the engine (RUNGENFRZSCR runs nested engine frames, which
// can't execute inside a jitted wasm frame - see jit_wasm.cpp). Correct resume
// depends on every consult site - CFG block splits, dispatch entries,
// yielding-function detection, region eligibility - agreeing on this set, so
// it lives here rather than being spelled out at each site. Those sites use
// instruction_is_suspend (zc/ffscript.h), which adds instructions that can
// suspend because of their args.
inline bool command_is_suspend(int command)
{
	return command_is_wait(command) || command == RUNGENFRZSCR;
}
inline bool command_uses_comparison_result(int command)
{
	return get_script_command_flags(command) & ARGFL_COMPARE_USED;
}
inline bool command_sets_comparison_result(int command)
{
	return get_script_command_flags(command) & ARGFL_COMPARE_SET;
}
// The SETx commands: write the last comparison's result to the register in arg1.
bool command_writes_comparison_result(int command);
inline bool command_could_return_not_ok(int command)
{
	return get_script_command_flags(command) & ARGFL_MAY_STOP;
}
// Returns true if writing to `reg` can stop the script doing it, like a command flagged
// ARGFL_MAY_STOP can: an lweapon script lifting itself with Hero->LiftedWeapon.
inline bool register_write_could_return_not_ok(int reg)
{
	return reg == HEROLIFTEDWPN;
}

// Which of each command's args are registers it writes, by command: bit i for arg i (see
// script_command::writes_to_register). Commands without an entry in the command table have none.
extern const std::array<byte, NUMCOMMANDS> script_command_written_args;

inline byte get_script_command_written_args(int command)
{
	return (unsigned)command < NUMCOMMANDS ? script_command_written_args[command] : 0;
}

// For instruction_is_suspend, by command: SUSPEND_INFO_COMMAND if the command itself can suspend
// the script, plus which of its args are registers it writes (bit i for arg i), since writing some
// registers can suspend it too. One lookup, because this is checked for every instruction each
// time a CFG is built.
constexpr byte SUSPEND_INFO_COMMAND = 0x80;
extern const std::array<byte, NUMCOMMANDS> script_command_suspend_info;

inline byte get_script_command_suspend_info(int command)
{
	return (unsigned)command < NUMCOMMANDS ? script_command_suspend_info[command] : 0;
}
inline bool command_is_pure(int command)
{
	return get_script_command_flags(command) & ARGFL_PURE;
}
const script_command* get_script_command(const std::string& name);
const script_variable* get_script_variable(int var);
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

// Returns true if a register reads from the stack.
bool does_register_use_stack(int reg);

bool has_register_dependency(int reg);

// Returns true if every read of a register gives a new value. Such a read can't be repeated,
// merged with another, or stand in for a value read earlier.
bool register_read_has_side_effects(int reg);

// A GOTOTABLE's target table compressed into runs of consecutive slots that
// share a target: {first slot index, target pc}, ascending by index. Ranged
// cases make long runs, so the JIT backends binary-search these instead of
// individual slots (one leaf per distinct run, not per key).
struct GotoTableRun
{
	int32_t start;  // first slot index of the run
	int32_t target; // target pc
};
std::vector<GotoTableRun> gototable_compress(const std::vector<int32_t>& table);

// One entry of a GOTORANGES vector: an inclusive key range and its target pc.
struct GotoRange
{
	int32_t start, end, target;
};
inline size_t gotoranges_count(const std::vector<int32_t>& table)
{
	return (table.size() - 1) / 3;
}
inline GotoRange gotoranges_at(const std::vector<int32_t>& table, size_t i)
{
	return {table[1 + i * 3], table[2 + i * 3], table[3 + i * 3]};
}

// Every pc a multi-target dispatch (GOTOTABLE/GOTORANGES) can jump to,
// including its default, given the command and its vector argument. Empty for
// any other command - single-target jumps keep their target in arg1 (see
// command_is_goto).
std::vector<int32_t> zasm_jump_targets(int command, const std::vector<int32_t>* vec);

#endif
