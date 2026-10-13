#include "base/zdefs.h"
#include "test_runner/test_runner.h"
#include "user_object.h"
#include "zc/ffscript.h"

TestResults test_user_objects([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	auto expect = [&](std::optional<dword> actual, std::optional<dword> expected, const char* what) {
		tr.total++;
		if (actual != expected)
		{
			tr.failed++;
			printf("find_destructor_pc: %s: expected %s but got %s\n", what,
				expected ? std::to_string(*expected).c_str() : "none",
				actual ? std::to_string(*actual).c_str() : "none");
		}
	};

	script_data script(ScriptType::Generic, 0);
	script.null_script(6); // The last instruction is the terminator.
	word commands[] = {NOP, STARTDESTRUCTOR, NOP, STARTDESTRUCTOR, NOP};
	for (int q = 0; q < 5; q++)
		script.zasm[q].command = commands[q];
	script.literals.set(script.zasm[1], {.str = "Foo"});
	script.literals.set(script.zasm[3], {.str = "Bar"});

	expect(find_destructor_pc(&script, "Bar", 4), 4, "correct pc");
	// A pc from before the scripts were recompiled. The search used to never advance past
	// the first instruction, looping forever.
	expect(find_destructor_pc(&script, "Bar", 2), 4, "stale pc");
	expect(find_destructor_pc(&script, "Foo", 4), 2, "stale pc of another destructor");
	expect(find_destructor_pc(&script, "Bar", 1000), 4, "stale pc past the end");
	expect(find_destructor_pc(&script, "Bar", 0), 4, "no pc");
	expect(find_destructor_pc(&script, "Baz", 4), std::nullopt, "missing destructor");

	return tr;
}
