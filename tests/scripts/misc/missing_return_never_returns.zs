// A non-void function isn't missing a return when the only path that doesn't return
// calls a function that never returns. Functions are analyzed in name order, so each
// caller here is analyzed before its callee, and the callee is only known to never
// return on a later pass than the one that first analyzed the caller.

int Caller(int x)
{
	if (x)
		return 1;
	Middle();
}

void Middle()
{
	NeverReturns();
}

void NeverReturns()
{
	while (true)
		Waitframe();
}

namespace Warned
{
	#option ON_MISSING_RETURN warn

	// Has no return statement at all, so this still warns (once).
	int NoReturnStatement()
	{
		Middle();
	}
}

ffc script missing_return_never_returns
{
	void run()
	{
		Caller(1);
		Warned::NoReturnStatement();
	}
}
