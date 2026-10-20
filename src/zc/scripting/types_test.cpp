#include "components/zasm/defines.h"
#include "components/zasm/table.h"
#include "test_runner/test_runner.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types.h"

TestResults test_register_routing([[maybe_unused]] bool verbose)
{
	TestResults tr{};
	std::vector<std::string> failures;

	// get_register/set_register handle these before consulting the routing table. Every other
	// register falls through to scripting_engine_get_register/set_register, which hits
	// NOTREACHED for a register with no subsystem.
	for (int i = 0; i < NUMVARIABLES; i++)
	{
		auto sv = get_script_variable(i);
		if (!sv) continue;

		if (i >= D(0) && i <= D(7)) continue;
		if (i >= GD(0) && i < GD(MAX_GLOBAL_VARIABLES)) continue;
		if (i >= SCRIPT_INST_VARS(0) && i < SCRIPT_INST_VARS(MAX_SCRIPT_INST_VARIABLES)) continue;
		if (zasm_array_supports(i)) continue;
		if (script_ref_for_register(i)) continue;

		if (register_routing_table[i] == EngineSubsystem::none)
			failures.push_back(fmt::format("zasm register {} has no subsystem in getEngineSubsystemForRegister (zc/scripting/types.cpp)", sv->name));
	}

	tr.total += 1;
	if (failures.size())
	{
		tr.failed += 1;
		for (auto& failure : failures)
			fmt::println("{}", failure);
	}

	return tr;
}
