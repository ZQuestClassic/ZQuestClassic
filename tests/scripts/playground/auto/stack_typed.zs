#include "std.zh"
#include "auto/test_runner.zs"

// Tests the class template support for stacks: `stack<T>`.
// Typed stacks type-check their elements and return typed values (no casts
// needed), and object elements are reference counted like any other.

int st_count = 0;

class STNode
{
	int tag;

	STNode(int tag = 0)
	{
		st_count++;
		this->tag = tag;
	}

	~STNode()
	{
		st_count--;
	}
}

generic script stack_typed
{
	void run()
	{
		Test::Init();

		// Plain `stack` still works, and is the same type as `stack<untyped>`.
		{
			stack s = new stack();
			s->PushBack(5);
			s->PushBack("hi");
			Test::AssertEqual(<int>(s->PopFront()), 5, "untyped PopFront");
			stack<untyped> s2 = s;
			Test::AssertEqual(s2->Size, 1L, "stack<untyped> is plain stack");
		}

		// stack<int>
		{
			stack<int> si = new stack<int>();
			si->PushBack(42);
			si->PushFront(7);
			int x = si->PopFront();
			Test::AssertEqual(x, 7, "int PopFront");
			si->Set(0L, 3);
			int y = si->Get(0L);
			Test::AssertEqual(y, 3, "int Get");
			Test::AssertEqual(si->PeekBack(), 3, "int PeekBack");
		}

		// stack<STNode>: typed object round trip, no casts.
		{
			stack<STNode> sp = new stack<STNode>();
			STNode a = new STNode(7);
			sp->PushBack(a);
			Test::AssertEqual(RefCount(a), 2L, "RefCount after push");
			a = NULL;
			Test::AssertEqual(st_count, 1, "stack retains node");
			GC();
			Test::AssertEqual(st_count, 1, "stack retains node through GC");

			STNode b = sp->PeekBack();
			Test::AssertEqual(b->tag, 7, "typed PeekBack");
			STNode c = sp->PopBack();
			Test::Assert(b == c, "PopBack identity");
			b = NULL;
			c = NULL;
			Waitframe();
			Test::AssertEqual(st_count, 0, "node released");
		}

		// stack<int[]>
		{
			stack<int[]> sa = new stack<int[]>();
			sa->PushBack({1, 2, 3});
			int[] arr = sa->PopBack();
			Test::AssertEqual(arr[2], 3, "array element");
		}

		Test::End();
	}
}
