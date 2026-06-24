#include "components/zasm/defines.h"
#include "components/zasm/table.h"
#include "test_runner/test_runner.h"
#include "zc/scripting/arrays.h"

TestResults test_arrays([[maybe_unused]] bool verbose)
{
	TestResults tr{};
	std::vector<std::string> failures;

	for (int i = 0; i < NUMVARIABLES; i++)
	{
		auto [sv, _] = get_script_variable(i);
		if (!sv) continue;

		bool is_array = zasm_array_supports(i);
		const std::vector<int>& deps = get_register_dependencies(i);

		if (is_array && deps != std::vector<int>{rINDEX})
		{
			auto& name = get_script_variable(i).first->name;
			failures.push_back(fmt::format("expected zasm register {} to declare {{rINDEX}} in _get_register_dependencies", name));
		}
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
