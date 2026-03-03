#include "std.zh"
#include "auto/test_runner.zs"

int TestInitScriptIndex = 0;
int TestInitScriptValues[3];

@InitScript(1)
global script TestInitScript1
{
    void run()
	{
		// Should run last.
        TestInitScriptValues[TestInitScriptIndex++] = 3;
    }
}

@InitScript(0)
global script TestInitScript2
{
    void run()
	{
		// Should run first.
		TestInitScriptValues[TestInitScriptIndex++] = 1;
    }
}

@InitScript(0)
global script TestInitScript3
{
    void run()
	{
		// Should run second.
        TestInitScriptValues[TestInitScriptIndex++] = 2;
    }
}

generic script init_scripts
{
	void run()
	{
		Test::Init();

		Test::AssertEqual(TestInitScriptValues[0], 1);
		Test::AssertEqual(TestInitScriptValues[1], 2);
		Test::AssertEqual(TestInitScriptValues[2], 3);

		Test::End();
	}
}
