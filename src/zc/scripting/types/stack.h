#ifndef ZC_SCRIPTING_STACK_H_
#define ZC_SCRIPTING_STACK_H_

#include <cstdint>

struct user_stack;

user_stack* checkStack(uint32_t id, bool skipError = false);

int32_t stack_get_register(int32_t reg);
void stack_set_register(int32_t reg, int32_t value);

#endif
