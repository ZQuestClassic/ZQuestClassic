#ifndef ZC_SCRIPTING_STACK_H_
#define ZC_SCRIPTING_STACK_H_

#include <cstdint>
#include <vector>

struct user_stack;

user_stack* checkStack(uint32_t id, bool skipError = false);
void register_existing_user_stack(user_stack* stack);
std::vector<user_stack*> get_user_stacks();

int32_t stack_get_register(int32_t reg);
void stack_set_register(int32_t reg, int32_t value);

#endif
