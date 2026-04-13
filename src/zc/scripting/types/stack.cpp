#include "zc/scripting/types/stack.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/ffscript.h"
#include "zc/scripting/script_object.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

UserDataContainer<user_stack, MAX_USER_STACKS> user_stacks = {script_object_type::stack, "stack"};

user_stack *checkStack(uint32_t id, bool skipError)
{
	return user_stacks.check(id, skipError);
}

int32_t stack_get_register(int32_t reg)
{
	int32_t ret = 0;
	user_stack* st = checkStack(GET_REF(stackref), true);

	switch (reg)
	{
		case STACKFULL:
		{
			if (st)
			{
				ret = st->full() ? 10000L : 0L;
			}
			else ret = -10000L;
			break;
		}
		case STACKSIZE:
		{
			if (st)
			{
				ret = st->size(); //NOT *10000
			}
			else ret = -10000L;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void stack_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	NOTREACHED();
}
