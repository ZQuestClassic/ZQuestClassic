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

	// An object array whose ZASM table entry is missing silently reads and
	// writes object 0. The table in components/zasm is generated, but for array
	// registers the generator can only report what the table already says - a
	// missing entry stays missing however often the table is regenerated - so
	// check it against what the array types declare instead.
	std::vector<std::string> ref_failures;
	for (int i = 0; i < NUMVARIABLES; i++)
	{
		auto impl = zasm_array_get(i);
		if (!impl || !impl->resolvesObject())
			continue;

		auto expected = zasm_array_ref_register(i);
		auto actual = get_register_ref_dependency(i);
		if (expected == actual)
			continue;

		auto name = [](std::optional<int> reg) -> std::string {
			if (!reg) return "(none)";
			auto [sv, _] = get_script_variable(*reg);
			return sv ? std::string(sv->name) : fmt::format("{}", *reg);
		};
		auto [sv, _] = get_script_variable(i);
		ref_failures.push_back(fmt::format(
			"scripting array {} resolves its object from {}, but the zasm table says {}. "
			"Add it to get_register_ref_dependency in components/zasm/table.cpp.",
			sv ? sv->name : "?", name(expected), name(actual)));
	}

	tr.total += 1;
	if (ref_failures.size())
	{
		tr.failed += 1;
		for (auto& failure : ref_failures)
			fmt::println("{}", failure);
	}

	return tr;
}
