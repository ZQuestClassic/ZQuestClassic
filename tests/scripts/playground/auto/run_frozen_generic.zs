// genericdata->RunFrozen() runs a generic script for whole engine frames before the
// calling script continues. The frozen script is compiled into the same chunk as the
// caller, so on the web JIT this also covers a nested run of the same module.

#include "auto/test_runner.zs"

int frozen_frames;
int frozen_sum;

generic script run_frozen_generic_target
{
	void run(int a, int b)
	{
		for (int i = 0; i < 3; i++)
		{
			frozen_frames++;
			Waitframe();
		}
		frozen_sum = a + b;
		printf("frozen script done: %d + %d\n", a, b);
	}
}

generic script run_frozen_generic
{
	void run()
	{
		Test::Init();

		int script_id = Game->GetGenericScript("run_frozen_generic_target");
		Test::Assert(script_id != -1);
		genericdata data = Game->LoadGenericData(script_id);
		data->InitD[0] = 2;
		data->InitD[1] = 3;

		// The caller's own state must survive the nested frames.
		int before = 1234;
		Test::Assert(data->RunFrozen());
		Test::AssertEqual(frozen_frames, 3);
		Test::AssertEqual(frozen_sum, 5);
		Test::AssertEqual(before, 1234);

		// Again from a loop, so the same call site resumes more than once.
		for (int i = 0; i < 2; i++)
		{
			data->InitD[0] = i;
			data->InitD[1] = 10;
			Test::Assert(data->RunFrozen());
			Test::AssertEqual(frozen_sum, i + 10);
		}
		Test::AssertEqual(frozen_frames, 9);
		Test::AssertEqual(before, 1234);

		Test::End();
	}
}
