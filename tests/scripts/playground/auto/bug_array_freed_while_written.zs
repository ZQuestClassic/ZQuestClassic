#include "auto/test_runner.zs"

// Replacing an array element releases the object it held. When that object's destructor dropped the
// last reference to the array, the array was freed while it was still being written to.

class Hook
{
	~Hook()
	{
		hooks = NULL;
	}
}

Hook[] hooks;

generic script bug_array_freed_while_written
{
	void run()
	{
		Test::Init();

		hooks = {new Hook(), NULL};
		hooks[0] = NULL;
		Test::Assert(!hooks, "the destructor released the array");

		// The same, through a function that writes many elements.
		hooks = {new Hook(), NULL};
		ResizeArray(hooks, 0);
		Test::Assert(!hooks, "the destructor released the array");

		Test::End();
	}
}
