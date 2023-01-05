#include "base/zdefs.h"
#include <string>

extern bool DEBUG_PRINT_ZASM;
extern bool DEBUG_JIT_PRINT_ASM;
extern bool DEBUG_JIT_EXIT_ON_COMPILE_FAIL;

int script_debug_is_runtime_debugging();
void script_debug_set_file_type(int type);
void script_debug_printf(int32_t attributes, const char *format, ...);
void script_debug_printf(const char *format, ...);
void script_debug_print(const char *str);
void script_debug_print(int32_t attributes, const char *str);
void script_debug_print_command(int i);
std::string script_debug_registers_and_stack_to_string();
std::string script_debug_command_to_string(word scommand, int32_t arg1, int32_t arg2);
std::string script_debug_command_to_string(word scommand);
void script_debug_pre_command();
void script_debug_print_zasm(script_data* script);
