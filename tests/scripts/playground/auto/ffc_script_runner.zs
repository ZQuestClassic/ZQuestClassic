// This tests some tricky scenarios where one script runs another, and that other script overwrites
// the "Script" variable of the calling script. The result should be a graceful "reset" of the
// script, not a crash or the script continuing.
// The only time the script should continue is when a script tries to edit its own Script, which is
// not allowed and so is ignored.

#include "auto/test_runner.zs"
#include "std.zh"

int product_result;
ffc multiply_numbers_ffc;

ffc script MultiplyNumbers
{
	void run(int a, int b, bool do_ffc_script)
	{
		multiply_numbers_ffc = this;

		int product = a * b;
		printf("product: %d\n", product);
		product_result = product;

		// Run another script that causes this one to terminate early by changing this ffc's script.
		if (do_ffc_script)
		{
			int script_id = Game->GetFFCScript("SayGoodbyeFFC");
			if (script_id == -1)
				Test::Fail("could not find script");

			// async behavior, gotta poll for finish.
			RunFFCScriptOrQuit(script_id, {0});
			while (!said_goodbye)
				Waitframe();
		}
		else
		{
			int script_id = Game->GetGenericScript("SayGoodbyeGeneric");
			if (script_id == -1)
				Test::Fail("could not find script");

			// sync behavior.
			RunGenericScriptFrz(script_id);
		}

		Test::Fail("(MultiplyNumbers) should not run");
	}
}

bool said_goodbye;

ffc script SayGoodbyeFFC
{
	void run()
	{
		int script_id = Game->GetFFCScript("SayHello");
		if (script_id == -1)
			Test::Fail("could not find script");
		multiply_numbers_ffc->Script = script_id;

		Trace("(SayGoodbyeFFC) Goodbye.");
		said_goodbye = true;
	}
}

generic script SayGoodbyeGeneric
{
	void run()
	{
		int script_id = Game->GetFFCScript("SayHello");
		if (script_id == -1)
			Test::Fail("could not find script");
		multiply_numbers_ffc->Script = script_id;

		Trace("(SayGoodbyeGeneric) Goodbye.");
		said_goodbye = true;
	}
}

bool said_hello;

ffc script SayHello
{
	void run()
	{
		Trace("Hello.");
		said_hello = true;
	}
}

bool ffc_deleted_self;

ffc script SelfDeletingFFC
{
	void run()
	{
		Trace("SelfDeletingFFC start");
		ClearFFC(this->ID);
		// Can't clear own script, so this is going to run.
		ffc_deleted_self = true;
	}
}

generic script ffc_script_runner
{
	void run()
	{
		Test::Init();

		int script_id = Game->GetFFCScript("MultiplyNumbers");

		// Use ffc script to change the initiating script at runtime.
		RunFFCScriptOrQuit(script_id, {2, 3, true});
		Waitframes(6);
		Test::AssertEqual(product_result, 6);
		Test::Assert(said_goodbye);
		Test::Assert(said_hello);

		said_goodbye = false;
		said_hello = false;

		// Use generic frozen script to change the initiating script at runtime.
		// TODO: I think this never worked. In 2.55.10 the output is:
		//   product: 12
		//   (SayGoodbyeGeneric) Goodbye.
		//   FFC(1, MultiplyNumbers): Stack over or underflow, stack pointer = 1024
		//   product: 0
		//   [Test] failed test: (MultiplyNumbers) should not run
		//   (SayGoodbyeFFC) Goodbye.
		//   Hello.
		//   [Test] failed assertion: expected 12 but got 0
		// RunFFCScriptOrQuit(script_id, {3, 4, false});
		// Waitframes(6);
		// Test::AssertEqual(product_result, 12);
		// Test::Assert(said_goodbye);
		// Test::Assert(said_hello);

		// This script will try to change it itself, which is not allowed.
		script_id = Game->GetFFCScript("SelfDeletingFFC");
		RunFFCScriptOrQuit(script_id, {0});
		Waitframes(6);
		Test::Assert(ffc_deleted_self);

		Test::End();
	}
}
