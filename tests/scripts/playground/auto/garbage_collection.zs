#include "std.zh"
#include "auto/test_runner.zs"

int count = 0;

class Hat
{
	Hat()
	{
		count++;
	}

	~Hat()
	{
		count--;
	}
}

class Person
{
	int age;
	Person children[3];
	Hat hat;
	Person lastPersonShookHandsWith;

	Person(int age = 0)
	{
		count++;
		this->age = age;
	}

	~Person()
	{
		count--;
	}

	void shakeHands(Person other)
	{
		// TODO: currently the implicit `this` variable does not retain a reference.
		// Otherwise this would be 2.
		Test::AssertEqual(RefCount(this), 1L, "RefCount(this)");
		Test::AssertEqual(RefCount(other), 2L, "RefCount(other)");

		lastPersonShookHandsWith = other;
	}

	void waitThenShakeHands(Person other)
	{
		Waitframe();

		Test::AssertEqual(RefCount(this), 1L, "RefCount(this)");
		Test::AssertEqual(RefCount(other), 2L, "RefCount(other)");

		lastPersonShookHandsWith = other;
	}
}

class Empty
{
}

int count_self_retaining_rng;

class SelfRetainingRng
{
	SelfRetainingRng self;
	randgen rng;

	SelfRetainingRng(randgen rng)
	{
		this->self = this;
		this->rng = rng;
		count_self_retaining_rng++;
	}

	~SelfRetainingRng()
	{
		count_self_retaining_rng--;
	}
}

class List
{
	int items[0];
	Person owner;
}

generic script garbage_collection
{
	Person people[10];
	Person globalPerson;
	int tests = 0;
	int frames = 0;

	void check(char32[] context, int actual, int expected)
	{
		Test::AssertEqual(actual, expected, context);
	}

	// Checks count is expected value, even if GC runs.
	void checkCountWithGC(int expected)
	{
		if (count != expected)
			check("count (pre GC)", count, expected);
		GC();
		if (count != expected)
			check("count (post GC)", count, expected);
	}

	void yield()
	{
		Waitframe();
		frames++;
	}

	void run()
	{
		Test::Init();

		// * `count` is the number of Person and Hat objects currently alive.
		// * `RefCount` returns the number of references to an object.
		//    If this hits zero, the object is immediately deleted.
		// * `checkCountWithGC` asserts the count is the expected value,
		//   and that running the GC (mark-and-sweep) doesn't change that value.
		// * At the start of each test, `count` should be zero.

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();

			// Variables retain objects.
			check("(1) RefCount(a)", RefCount(a), 1L);
			check("count", count, 1);
			a = NULL;
			check("count", count, 0);

			// All new objects are added to an "autorelease" pool that retains an implicit reference
			// until the pool is drained, or until it is retained in some other way (such as assigning
			// to a variable).
			check("RefCount(new Person())", RefCount(new Person()), 1L);
			check("count", count, 1); // It still exists.
			// The autorelease pool is drained when the script yields.
			yield();
			check("count", count, 0);

			// Variables retain objects until the end of their scope.
			a = new Person();
			checkCountWithGC(1);
		}
		// Now it's been deleted.
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			new Person();
			// This Person was not stored in a variable, so it is deleted the
			// next time the autorelease pool is drained.
			yield();
			check("count", count, 0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			a = a;
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			a->hat = new Hat();
			check("RefCount(a->hat)", RefCount(a->hat), 1L);
			checkCountWithGC(2);
		}
		yield();
		checkCountWithGC(0);

		printf("=== Test %d - custom object functions === \n", ++tests);
		{
			Person a = new Person();
			Person b = new Person();

			a->shakeHands(b);
			check("RefCount(a)", RefCount(a), 1L);
			check("RefCount(b)", RefCount(b), 2L);

			checkCountWithGC(2);
		}
		checkCountWithGC(0);

		// TODO: currently does not work. The `this` variable is a bit special codegen-wise
		// (I think the stack position may be wrong or invalid for it). See ScriptParser::generateOCode.
		// printf("=== Test %d - custom object functions, called on loose object === \n", ++tests);
		// {
		// 	Person a = new Person();
		// 	Person b = new Person();
		//
		// 	new Person()->waitThenShakeHands(b);
		// 	check("RefCount(b)", RefCount(b), 1L);
		//
		// 	checkCountWithGC(1);
		// }
		// checkCountWithGC(0);

		printf("=== Test %d - randgen === \n", ++tests);
		{
			randgen rng = new randgen();
			yield();
			check("RefCount(a)", RefCount(rng), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - int does not retain randgen === \n", ++tests);
		{
			int scratch;
			{
				randgen rng = Game->LoadRNG();
				check("RefCount(rng)", RefCount(rng), 1L);
				scratch = <untyped>rng;
				check("RefCount(rng)", RefCount(rng), 1L); // scratch does not retain a reference.
			}
			// Now it's been deleted.
			int ref = RefCount(scratch);
			check("RefCount(scratch)", ref, -1L);
		}

		printf("=== Test %d - int does not retain const randgen === \n", ++tests);
		{
			int scratch;
			{
				const randgen rng = Game->LoadRNG();
				scratch = <untyped>rng;
				check("RefCount(rng)", RefCount(rng), 1L);
			}
			// Now it's been deleted.
			int ref = RefCount(scratch);
			check("RefCount(scratch)", ref, -1L);
		}

		printf("=== Test %d - GC keeps objects in autorelease pool === \n", ++tests);
		{
			new Person();
			GC();
			check("count", count, 1);
			yield();
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			int age = (new Person(10))->age;
			yield();
			check("age", age, 10);
			check("count", count, 0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a;
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			int age = (new Person(10))->age + (new Person(20))->age;
			yield();
			check("age", age, 30);
			check("count", count, 0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			// Cheeky little test. `count` should be evaluated as 0, then 1, then 2.
			int c = count + (new Person())->age + count + (new Person())->age + count;
			check("c", c, 3);
			yield();
			// Now `count` is back to 0.
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			globalPerson = a;
			check("RefCount(a)", RefCount(a), 2L);
			checkCountWithGC(1);
		}
		checkCountWithGC(1);
		globalPerson = NULL;
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			usePerson1(a);
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - pass free object as parameter === \n", ++tests);
		{
			usePerson2(new Person());
			checkCountWithGC(0);
		}

		printf("=== Test %d - functions add returned objects to autorelease pool === \n", ++tests);
		{
			// Functions add their return value to the autorelease pool so that they can't possibly
			// hit 0 when returning.
			Person a = usePerson3(new Person());
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - simple block === \n", ++tests);
		{
			Person a = new Person();

			{
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);
			}
			check("RefCount(a)", RefCount(a), 1L);
		}

		printf("=== Test %d - if === \n", ++tests);
		{
			Person a = new Person();

			if (a)
			{
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);
			}
			check("RefCount(a)", RefCount(a), 1L);
		}

		printf("=== Test %d - if w/ declaration === \n", ++tests);
		{
			Person a = new Person();

			if (auto b = a)
				check("RefCount(b)", RefCount(b), 2L);
			check("RefCount(a)", RefCount(a), 1L);
		}

		printf("=== Test %d - while === \n", ++tests);
		{
			Person a = new Person();

			int i = 0;
			while (i < 3)
			{
				check("RefCount(a)", RefCount(a), 1L);
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);

				i++;
			}

			check("RefCount(a)", RefCount(a), 1L);

			i = 0;
			while (i < 3)
			{
				check("RefCount(a)", RefCount(a), 1L);
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);

				i++;
			}
			else
			{
				check("RefCount(a)", RefCount(a), 1L);
				int b1 = 123;
				auto b2 = a;
				check("RefCount(b2)", RefCount(b2), 2L);
			}

			check("RefCount(a)", RefCount(a), 1L);

			i = 0;
			while (i < 3)
			{
				if (i == 2) break;

				check("RefCount(a)", RefCount(a), 1L);
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);

				i++;
			}

			check("RefCount(a)", RefCount(a), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - if w/ declaration inside while === \n", ++tests);
		{
			Person a = new Person();

			while (a)
			{
				auto b1 = a;
				check("RefCount(b1)", RefCount(b1), 2L);

				if (auto b2 = a)
				{
					check("RefCount(b2)", RefCount(b2), 3L);

					auto b3 = a;
					check("RefCount(b3)", RefCount(b3), 4L);

					break;
				}
			}
			check("RefCount(a)", RefCount(a), 1L);
		}
		checkCountWithGC(0);

		// The next two tests excercise breaking from inner scopes when outer scopes still retain an
		// object.
		printf("=== Test %d - usePerson5 === \n", ++tests);
		{
			Person a = usePerson5(new Person(), false);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - usePerson5 === \n", ++tests);
		{
			Person a = usePerson5(new Person(), true);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - createPerson1 === \n", ++tests);
		{
			Person a = createPerson1();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - createPerson2 === \n", ++tests);
		{
			Person a = createPerson2();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - createPerson2 === \n", ++tests);
		{
			Person a = usePerson3(createPerson2());
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = createPerson3();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			auto b = a;
			check("RefCount(a)", RefCount(a), 2L);
			Person c = b;
			check("RefCount(a)", RefCount(a), 3L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			check("(1) count", count, 1);
			Person b = new Person();
			check("(2) count", count, 2);

			check("(3) RefCount(a)", RefCount(a), 1L);
			check("(4) RefCount(b)", RefCount(b), 1L);
			b = a;
			check("(5) count", count, 1);
			check("(6) RefCount(a)", RefCount(a), 2L);
			check("(7) RefCount(b)", RefCount(b), 2L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - arrays 1 === \n", ++tests);
		{
			Person a = new Person();
			people[0] = a;
			people[5] = people[0];
			check("RefCount(a)", RefCount(a), 3L);
			people[0] = NULL;
			check("RefCount(a)", RefCount(a), 2L);
			checkCountWithGC(1);
		}
		Trace("(a)");
		checkCountWithGC(1);
		people[5] = NULL;
		Trace("(b)");
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			{
				Person b = new Person();
				checkCountWithGC(2);
			}
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - arrays 2 === \n", ++tests);
		{
			auto a = new Person();
			ArrayPushBack(people, a);
			check("RefCount(a)", RefCount(a), 2L);
			Person b = a;
			a = NULL;
			check("RefCount(b)", RefCount(b), 2L);
			ArrayPopBack(people);
			check("RefCount(b)", RefCount(b), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - arrays 3 === \n", ++tests);
		{
			Person b = new Person();

			Person c[] = {b};
			check("RefCount(c)", RefCount(c), 1L);

			check("RefCount(b)", RefCount(b), 2L);
			c = NULL;
			check("RefCount(b)", RefCount(b), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - arrays 4 === \n", ++tests);
		{
			Person c[] = {new Person()};
			check("RefCount(c)", RefCount(c), 1L);
			check("RefCount(c[0])", RefCount(c[0]), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - arrays 5 === \n", ++tests);
		{
			Person c[] = {new Person()};
			checkCountWithGC(1);
			ResizeArray(c, 0);
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			Person b = new Person();
			a->children[0] = b;
			b->children[0] = a;
			check("RefCount(b)", RefCount(b), 2L);
		}
		// a and b create a cyclical reference that is only broken after a full GC.
		check("count (pre GC)", count, 2);
		GC();
		check("count (post GC)", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			a->children[0] = a;
			check("RefCount(b)", RefCount(a), 2L);
		}
		// `a` has a reference to itself, so it is still around.
		check("count", count, 1);
		GC();
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			Person b = new Person();
			a->children[0] = b;
			b->children[0] = a;
			check("RefCount(a)", RefCount(a), 2L);
			check("RefCount(b)", RefCount(b), 2L);
			a = NULL;
			check("RefCount(b->children[0])", RefCount(b->children[0]), 1L);
		}
		// `a` and `b` have references to each other, so both are still around.
		check("count", count, 2);
		GC();
		check("count", count, 0);

		// The 'delete' operator doesn't actually do anything now.
		printf("=== Test %d === \n", ++tests);
		{
			Person a;
			check("RefCount(a)", RefCount(a), -1L);
			a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			delete a;
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		// untyped does not increase reference count.
		printf("=== Test %d - untyped === \n", ++tests);
		{
			untyped a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			check("(1) count", count, 1);
			yield(); // Release the single reference, which is from the autorelease pool.
			check("(2) count", count, 0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			untyped a = new Person();
			Person b = a;
			check("RefCount(a)", RefCount(a), 1L);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
		}
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			untyped a = new Person();
			Person b = a;
			check("RefCount(a)", RefCount(a), 1L);
			b = NULL;
			checkCountWithGC(0);
		}
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			untyped arr[3] = {a, a, a};
			check("RefCount(a)", RefCount(a), 1L);
			a = NULL;
			checkCountWithGC(0);
		}
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			untyped arr[2];
			{
				Person a = new Person();
				yield();
				arr[0] = a;
				arr[1] = a;
				check("RefCount(a)", RefCount(a), 1L);
			}
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			for (int i = 0; i < 10; i++)
			{
				check("RefCount(a)", RefCount(a), 1L);
				Person b = a;
				check("RefCount(a)", RefCount(a), 2L);
				Person c = new Person();
				yield();
				checkCountWithGC(2);
			}
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - randgen (many) === \n", ++tests);
		{
			// Only 255 randgens are allowed currently. Check that the
			// GC runs when needed to make room.
			for (int i = 0; i < 300; i++)
			{
				randgen rng = Game->LoadRNG();
				if (rng == NULL)
					Test::Fail("failed to allocate randgen");

				auto retainer = new SelfRetainingRng(rng); // Only gets deleted after a full GC.
				check("RefCount(rng)", RefCount(rng), 2L);
				// Make sure the retainers are actually alive.
				// Stop checking b/c eventually they get cleared by the GC.
				if (i < 100)
					Test::AssertEqual(count_self_retaining_rng, i + 1);
			}
			GC();
			Test::AssertEqual(count_self_retaining_rng, 0);
		}

		printf("=== Test %d - Arrays and array literals === \n", ++tests);
		{
			int b[2];
			check("RefCount(b)", RefCount(b), 1L);
			{
				int a[] = {1, 2};
				check("RefCount(a)", RefCount(a), 1L);
				b = a;
				check("RefCount(a)", RefCount(a), 2L);
			}
			Test::AssertEqual(b[0], 1);
			check("RefCount(b)", RefCount(b), 1L);
		}

		printf("=== Test %d === \n", ++tests);
		{
			int[][] b = {{1}, {2}};
			check("(a) RefCount(b)", RefCount(b), 1L);
			check("RefCount(b[0])", RefCount(b[0]), 1L);
			check("(a) RefCount(b[1])", RefCount(b[1]), 1L);
			int[] c = b[1];
			check("(b) RefCount(b[1])", RefCount(b[1]), 2L);
			b[1] = NULL;
			check("(b) RefCount(b)", RefCount(b), 1L);
		}

		printf("=== Test %d - bitmaps === \n", ++tests);
		{
			// Internal bitmaps aren't real script objects.
			bitmap internal_bmp_1 = Game->LoadBitmapID(RT_SCREEN);
			bitmap internal_bmp_2 = Game->LoadBitmapID(RT_BITMAP0);
			check("RefCount(internal_bmp_1)", RefCount(internal_bmp_1), -1L);
			check("RefCount(internal_bmp_2)", RefCount(internal_bmp_2), -1L);
			internal_bmp_2 = NULL;

			// User bitmaps are.
			internal_bmp_1 = new bitmap();
			check("RefCount(internal_bmp_1)", RefCount(internal_bmp_1), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d - class member arrays === \n", ++tests);
		{
			List list = new List();
			ArrayPushBack(list->items, 1337);
			list->owner = new Person();

			check("RefCount(list->items)", RefCount(list->items), 1L);

			auto items = list->items;
			check("RefCount(items)", RefCount(items), 2L);
			check("items[0]", items[0], 1337);

			list->items = NULL;
			check("RefCount(items)", RefCount(items), 1L);

			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - return local string === \n", ++tests);
		{
			auto s = makeString(5);
			check("RefCount(s)", RefCount(s), 1L);
			Test::AssertEqual(strlen(s), 5);
		}

		printf("=== Test %d - return local array === \n", ++tests);
		{
			auto arr = makeArray();
			check("RefCount(arr)", RefCount(arr), 1L);
			Test::AssertEqual(SizeOfArray(arr), 3);
		}

		printf("=== Test %d - string literal === \n", ++tests);
		{
			char32[] str = "hi there\n";
			check("RefCount(str)", RefCount(str), 1L);
			Trace(str);

			check("RefCount(\"literal\")", RefCount("literal"), 1L);
		}

		printf("=== Test %d - array literal === \n", ++tests);
		{
			Person[] arr = {new Person(), new Person(), new Person()};
			check("RefCount(arr)", RefCount(arr), 1L);
			check("RefCount(arr[0])", RefCount(arr[0]), 1L);
			Test::AssertEqual(SizeOfArray(arr), 3);

			check("RefCount({0, 1, 2})", RefCount({0, 1, 2}), 1L);
			check("RefCount({NULL})", RefCount({NULL}), 1L);
			check("RefCount({new Person()})", RefCount({new Person()}), 1L);
			check("RefCount({new Person()}[0])", RefCount({new Person()}[0]), 1L);
			checkCountWithGC(5);
			yield();
		}
		checkCountWithGC(0);

		printf("=== Test %d - array of strings === \n", ++tests);
		{
			char32[][] arr = {"hi", "there"};
			check("RefCount(arr)", RefCount(arr), 1L);
			check("RefCount(arr[0])", RefCount(arr[0]), 1L);
			Test::AssertEqual(SizeOfArray(arr), 2);

			check("RefCount({0, 1, 2})", RefCount({0, 1, 2}), 1L);
			check("RefCount({\"hi\", \"there\"})", RefCount({"hi", "there"}), 1L);
			check("RefCount({\"hi\", \"there\"}[0])", RefCount({"hi", "there"}[0]), 1L);
			Test::AssertEqual(SizeOfArray({"hi", "there"}[1]), 6);
		}

		printf("=== Test %d - 2d array of strings === \n", ++tests);
		{
			char32[][][] arr = {{"hi", "there"}, {""}};
			check("RefCount(arr)", RefCount(arr), 1L);
			check("RefCount(arr[0])", RefCount(arr[0]), 1L);
			check("RefCount(arr[0][0])", RefCount(arr[0][0]), 1L);
			Test::AssertEqual(SizeOfArray(arr), 2);
		}

		// Internal arrays are wrapped in a script array, which is ref-counted.
		printf("=== Test %d === \n", ++tests);
		{
			npc[] b = Screen->NPCs;
			check("RefCount(b)", RefCount(b), 1L);
			npc[] c = b;
			check("RefCount(b)", RefCount(b), 2L);
			// Same as above.
			check("RefCount(Screen->NPCs)", RefCount(Screen->NPCs), 2L);
		}

		printf("=== Test %d - varargs === \n", ++tests);
		{
			storeVarargs(1, 2, 3);
			check("RefCount(storedVarargs)", RefCount(storedVarargs), 1L);
		}

		// Global objects are never collected by the GC. It's up to the programmer
		// to not "lose" them. For example, the following test does not
		// save `a` anywhere recoverable from a new session, so this is
		// a memory leak.
		printf("=== Test %d - GlobalObject === \n", ++tests);
		{
			auto a = new Person();
			checkCountWithGC(1);
			GlobalObject(a);
		}
		checkCountWithGC(1);

		// This avoids cutting off the end of the replay when using --update, if you've
		// modified how many times Waitframe is called.
		while (frames < 100)
			yield();
	}

	void usePerson1(Person person)
	{
		check("RefCount(person)", RefCount(person), 2L);
	}

	void usePerson2(Person person)
	{
		check("RefCount(person)", RefCount(person), 1L);
	}

	Person usePerson3(Person person)
	{
		{
			new Person();
		}
		yield();
		check("RefCount(person)", RefCount(person), 1L);
		return person;
	}

	Person usePerson4(Person person)
	{
		yield();
		check("(a) RefCount(person)", RefCount(person), 1L);
		return person;
	}

	Person usePerson5(Person person, bool exitEarlier)
	{
		yield();
		check("(a) RefCount(person)", RefCount(person), 1L);

		auto a = person;
		check("(b) RefCount(person)", RefCount(a), 2L);

		if (person)
		{
			auto b = person;
			check("(c) RefCount(person)", RefCount(b), 3L);

			while (person)
			{
				auto c = person;
				check("(d) RefCount(person)", RefCount(c), 4L);
				if (c) break;
			}

			check("(e) RefCount(person)", RefCount(b), 3L);

			while (person)
			{
				auto c = person;
				if (c)
				{
					auto d = c;
					check("(f) RefCount(person)", RefCount(d), 5L);
					break;
				}
			}

			check("(g) RefCount(person)", RefCount(b), 3L);

			while (exitEarlier)
			{
				int c = 13;
				c = 14;
				if (c) return person;
			}

			check("(g2) RefCount(person)", RefCount(b), 3L);

			auto b2 = person;
			check("(g3) RefCount(person)", RefCount(b2), 4L);
			b2 = NULL;

			switch (count)
			{
				case 1:
				{
					auto c = person;
					check("(h) RefCount(person)", RefCount(c), 4L);
					break;
				}
				default: Quit();
			}

			check("(g4) RefCount(person)", RefCount(person), 3L);

			while (true)
			{
				auto b2 = person;

				switch (count)
				{
					case 1:
					{
						auto c = person;
						check("(i) RefCount(person)", RefCount(c), 5L);
						break 2; // break while
					}
					default: Quit();
				}
			}

			check("(g5) RefCount(person)", RefCount(person), 3L);

			while (person)
			{
				auto d = person;
				check("(j) RefCount(person)", RefCount(d), 4L);

				if (d)
				{
					auto e = person;
					check("(k) RefCount(person)", RefCount(e), 5L);
					if (e) return e;
				}
			}
		}

		return NULL;
	}

	Person createPerson1()
	{
		auto person = new Person();
		return person;
	}

	Person createPerson2()
	{
		return new Person();
	}

	Person createPerson3()
	{
		auto person = new Person();
		if (person) return person;
		else return NULL;
	}

	void storeVarargs(...int[] varargs)
	{
		check("RefCount(varargs)", RefCount(varargs), 1L);
		storedVarargs = varargs;
	}

	char32[] makeString(int len)
	{
		char32[] s = "";
		for (int i = 0; i < len; i++) ArrayPushFront(s, '.');
		return s;
	}

	int[] makeArray()
	{
		return {1, 2, 3};
	}
}

int[] storedVarargs = NULL;
