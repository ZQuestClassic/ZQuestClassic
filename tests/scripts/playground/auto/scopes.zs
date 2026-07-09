#include "std.zh"
#include "auto/test_runner.zs"
#include "auto/global_objects.zs"

int GLOBAL_VAR_NEVER_SHADOWED = 1;
int GLOBAL_VAR = 1;
const int CONST_GLOBAL_VAR = 1;
bool GLOBAL_BOOL;

namespace A
{
	int GLOBAL_VAR = 2;
}

namespace A
{
	int DebugAddOne(int a = 1)
	{
		return a + 1;
	}

	int A_var = 9;

	void A_fn(int GLOBAL_VAR = 10)
	{
		Test::AssertEqual(GLOBAL_VAR, 10);
	}

	namespace B
	{
		int B_var;

		void B_fn()
		{
			Test::AssertEqual(GLOBAL_VAR, 2);
			if (A_var)
			{
				Test::AssertEqual(GLOBAL_VAR, 2);
				int GLOBAL_VAR = 3;
				Test::AssertEqual(GLOBAL_VAR, 3); // end of B_fn
			}
		}
	}
}

namespace A
{
	class DataBag
	{
		static int StaticClassFunction()
		{
			return 123;
		}

		int var1;
		int var2;
		
		DataBag()
		{
			var1 = 1;
			var2 = 2;
		}

		void dosomething()
		{
			int v1 = var1;
			int v2 = var2;
			Test::AssertEqual(v1, var1);
			Test::AssertEqual(v2, var2); // end of DataBag::dosomething
		}

		int sum(int a, ...int[] rest)
		{
			int sum = a + var1 + var2;
			for (int i = 0; i < SizeOfArray(rest); i++)
				sum += rest[i];
			return sum;
		}
	}

	CL cl;

	class CL
	{
		int A_var;
		CL this_ptr;
		int[] arr;
		untyped[] arr_untyped;

		CL()
		{
			A_var = 11;
			Test::AssertEqual(this->A_var, 11);
			arr = {0, 1, 2};
			arr_untyped = {0, 1, this};
			this_ptr = this;
			auto ptr = this;
			auto d = new DataBag();
			d->var1 = 1;
			d->var2 = 2;
			d->dosomething();
			Test::Assert(getVar());
			Test::Assert(ptr); // end of CL ctor
		}

		~CL()
		{
			int a = A_var;
			Test::Assert(a); // end of CL dtor
		}

		void trace()
		{
			Test::AssertEqual(A_var, 11);
			Test::AssertEqual(GLOBAL_VAR, 2);
		}

		int getVar()
		{
			return A_var;
		}
	}

	void trace()
	{
		Test::AssertEqual(A_var, 9);
	}
}

namespace D
{
	int D_var;
}

using namespace D;

void unused_fn()
{
	Test::Fail("Expected function to be unused");
}

void using_namespace_statement()
{
	using namespace A;
	Test::AssertEqual(A_var, 9);
}

void has_constant()
{
	const int constant_int = 1;
	Trace(constant_int);
}

void template_fn<T1, T2>(T1 val1, T2 val2)
{
	Trace(val1);
	Trace(val2);
	T1[][] double_arr = {{val1}};
	Trace(double_arr); // end of template_fn
}

void trace_all(int a, ...int[] b)
{
	Trace(a);
	Trace(b);
}

void trace_all_2(...int[] a)
{
	Trace(a); // end of trace_all_2
}

void num_fn(long a)
{
	Trace(a);
}

void num_fn(int a)
{
	Trace(a);
}

void long_fn(long a)
{
	Trace(a);
}

enum Enum
{
	Enum_A, Enum_B
};

Enum enum_fn(const Enum e)
{
	return e;
}

void loop_fn()
{
	for (int i = 0; i < 2; i++)
		Trace(i);

	if (int x = 1)
		Trace(x);
	else if (int y = 1)
		Trace(y);
	else
	{
		int z = 1;
		Trace(z);
	}
}

void do_fn()
{
	int i = 0;
	do
	{
		int j = 1;
		i += j;
	} while (i < 10)
}

void range_loop_fn()
{
	int[] arr = {1, 2, 3};
	for (x : arr)
	{
		int y = x + 1;
		Trace(y);
		if (y == 3) break;
	}
	else
	{
		int z = 1;
		Trace("for else");
		Trace(z);
	}
}

int DebugAdd(int a)
{
	return a + 10;
}

int DebugAdd(int a, int b)
{
	return a + b;
}

int DebugAdd(int a, long b)
{
	return a + b + 100L;
}

int DebugAdd(long a, int b)
{
	return a + b + 200L;
}

void DebugPrintCL(const A::CL cl)
{
	Trace(cl);
}

void DebugArrays()
{
	const bitmap b = new bitmap(32, 32);
	int[] arr1 = {1, 2};
	int arr2[] = {1, 2};
	int[] arr3 = Hero->Steps;
	bool[] arr4 = Hero->Steps;
	auto[] arr5 = {new A::CL(), NULL};
	untyped[] arr6 = {new A::CL(), 123, {1, 2}, b};
	const int[] arr7 = {1, 2};
	Trace(arr1);
	Trace(arr2);
	Trace(arr3);
	Trace(arr4);
	Trace(arr5);
	Trace(arr6);
	Trace(arr7); // end of DebugArrays
}

void DebugInternalObjects()
{
	const ffc f = Screen->LoadFFC(1);
	f->X = 10;
	f->Flags[0] = true;
	const npc n = Screen->CreateNPC(NPC_GEL);
	n->X = 10;
	const bitmap b = new bitmap(32, 32);
	b->DrawTile(1, 0, 0, 100, 2, 2, 0);
	Waitframe();
	Test::Assert(f);
	Test::Assert(n);
	const randgen rng = new randgen();
	untyped[] arr_rng = {rng};
	stack<int> stk_i = new stack<int>();
	stk_i->PushBack(42);
	const stack<bitmap> stk_b = new stack<bitmap>();
	stk_b->PushBack(b);
	stack stk_u = new stack();
	stk_u->PushBack(1337);
	Test::Assert(b); // end of DebugInternalObjects
}

int DebugFrames1(int a, int b)
{
	int c = a + b; // start of DebugFrames1
	int d = DebugFrames2(a + 1, b + 1);
	return c + d; // end of DebugFrames1
}

int DebugFrames2(int a, int b)
{
	int c = a * b;
	return c; // end of DebugFrames2
}

void DebugFunctionCallsWaitframe1()
{
	DebugFunctionCallsWaitframe2();
}

void DebugFunctionCallsWaitframe2()
{
	Waitframe();
	Test::Assert(true);
}

void DebugFunctionInfiniteLoop(bool forever)
{
	while (forever)
	{
		Test::Assert(true);
	}
}

class DebugClassDefaultCtor
{
	int data;
}

void DebugFunctionDefaultCtor1()
{
	auto c = new DebugClassDefaultCtor();
	c->data = 10;
	// Test that debugger can lookup untyped array element's type, even if the class had a default
	// ctor.
	untyped[] a = {c};
	DebugFunctionDefaultCtor2(a);
	Test::Assert(c->data);
}

void DebugFunctionDefaultCtor2(untyped[] c)
{
	Test::Assert(<DebugClassDefaultCtor>(c[0])->data); // end of DebugFunctionDefaultCtor2
}

int DebugFunctionVarargs(int first, ...int[] rest)
{
	int sum = first;
	for (int i = 0; i < SizeOfArray(rest); i++)
		sum += rest[i];
	return sum; // end of DebugFunctionVarargs
}

void AccessScriptScopedMembers()
{
	Test::AssertEqual(scopes.SCRIPT_SCOPED_GLOBAL, 123);
	Test::AssertEqual(scopes.scriptFunction(), 100);
}

// If we decided to optimize this, it should still be available in the debugger.
int returnOne()
{
	return 1;
}

generic script scopes
{
	int SCRIPT_SCOPED_GLOBAL = 123;

	int scriptFunction()
	{
		// Avoid any possible optimization.
		return Game->SubscreenSpeed ? 100 : 0;
	}

	void run()
	{
		Test::Init();

		// Test::AssertEqual(scopes::SCRIPT_SCOPED_GLOBAL, 123); // invalid - can't use :: for script scoped members
		Test::AssertEqual(scopes.SCRIPT_SCOPED_GLOBAL, 123);
		Test::AssertEqual(SCRIPT_SCOPED_GLOBAL, 123);
		Test::AssertEqual(scopes.scriptFunction(), 100);
		Test::AssertEqual(scriptFunction(), 100);
		// Static class functions can use . and :: - for some reason...
		Test::AssertEqual(A::DataBag::StaticClassFunction(), 123);
		Test::AssertEqual(A::DataBag.StaticClassFunction(), 123);
		AccessScriptScopedMembers();
		Test::AssertEqual(returnOne(), 1);

		Test::Assert(globalThings);
		A::A_fn();
		A::B::B_fn();
		using_namespace_statement();
		A::trace();
		new A::CL()->trace();
		template_fn(1337, 1337);
		template_fn("hello", 1337);
		Above(Hero);
		Test::AssertEqual(D_var, 0);
		has_constant();
		trace_all(1);
		trace_all(1, 2, 3);
		trace_all_2(1, 2, 3);
		num_fn(1L);
		num_fn(1);
		long_fn(1);
		enum_fn(Enum_A);
		loop_fn();
		do_fn();
		range_loop_fn();
		A::DebugAddOne();
		DebugAdd(0);
		DebugAdd(0L);
		DebugAdd(0, 0);
		DebugAdd(0, 0L);
		DebugAdd(0L, 0);
		// Would error: too many choices.
		// DebugAdd(0L, 0L);
		DebugPrintCL(A::cl);
		DebugArrays();
		DebugInternalObjects();
		DebugFrames1(2, 5);
		DebugFunctionCallsWaitframe1();
		DebugFunctionInfiniteLoop(false);
		DebugFunctionDefaultCtor1();
		DebugFunctionVarargs(1, 2, 3);
		Test::AssertEqual(new A::DataBag()->sum(3, 4, 5, 6), 21);
		Test::Assert(this->Running);

		GC();
		new A::CL(); // Create CL.
		// Trigger CL's destructor.
		Waitframe();
		GC();

		Test::End();
	}
}
