#include "base/compiler.h"
#include "base/zdefs.h"
#include <string>

extern bool DEBUG_JIT_PRINT_ASM;
extern bool DEBUG_JIT_EXIT_ON_COMPILE_FAIL;
extern bool DEBUG_PRINT_TO_FILE;
extern bool DEBUG_PRINT_TO_CONSOLE;

class ScriptDebugHandle
{
public:

	enum class OutputSplit
	{
		ByFrame,
		ByScript,
	};

	ScriptDebugHandle(zasm_script* script, OutputSplit output_split, std::string name);
	ScriptDebugHandle(ScriptDebugHandle&& rhs);
	ScriptDebugHandle& operator=(ScriptDebugHandle&& rhs);
	~ScriptDebugHandle();
	
	void update_file();
	ZC_FORMAT_PRINTF(3, 4)
	void printf(int32_t attributes, const char *format, ...);
	ZC_FORMAT_PRINTF(2, 3)
	void printf(const char *format, ...);
	void print(const char *str);
	void print(int32_t attributes, const char *str);
	void print_command(int i);
	void pre_command();

private:
	OutputSplit output_split;
	std::string name;
	zasm_script* script;
	ALLEGRO_FILE* file;
	int file_counter;

	void _print(int32_t attributes, const char *str);
};

ScriptDebugHandle* script_debug_get_handle(int type, script_data* script);
int script_debug_is_runtime_debugging();
std::string script_debug_registers_and_stack_to_string();
