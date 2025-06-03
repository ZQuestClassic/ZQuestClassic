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
			check("count", count, expected);
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
			// Variables count as a reference, but all new objects are added to an
			// "autorelease" pool that retains an implicit reference until the pool is drained.
			check("(1) RefCount(a)", RefCount(a), 2L);
			// The autorelease pool is drained when the script yields.
			yield();
			// The autorelease pool was drained.
			check("(2) RefCount(a)", RefCount(a), 1L);
			// Variables hold an object reference until the end of their scope, so the Person
			// has not been deleted yet.
			checkCountWithGC(1);
		}
		// Now it's been deleted.
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			// Get `a` down to one reference.
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			a = a;
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			const Person a = new Person();
			// Get `a` down to one reference.
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			a->hat = new Hat();
			check("RefCount(a->hat)", RefCount(a->hat), 2L);
			checkCountWithGC(2);
		}
		yield();
		checkCountWithGC(0);

		printf("=== Test %d - custom object functions === \n", ++tests);
		{
			Person a = new Person();
			Person b = new Person();
			yield();

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
		// 	yield();
		//
		// 	new Person()->waitThenShakeHands(b);
		// 	check("RefCount(b)", RefCount(b), 1L);
		//
		// 	checkCountWithGC(1);
		// }
		// checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			randgen rng = new randgen();
			yield();
			check("RefCount(a)", RefCount(rng), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			int scratch;
			{
				randgen rng = Game->LoadRNG();
				scratch = <untyped>rng;
				check("RefCount(rng)", RefCount(rng), 2L);
				yield();
			}
			// Now it's been deleted.
			check("RefCount(scratch)", RefCount(scratch), -1L);
		}

		printf("=== Test %d === \n", ++tests);
		{
			int scratch;
			{
				const randgen rng = Game->LoadRNG();
				scratch = <untyped>rng;
				check("RefCount(rng)", RefCount(rng), 2L);
				yield();
			}
			// Now it's been deleted.
			check("RefCount(scratch)", RefCount(scratch), -1L);
		}

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
			new Person();
			GC();
			check("count", count, 0);
		}

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
			GC();
			// Now `count` is back to 0.
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			check("RefCount(new Person())", RefCount(new Person()), 1L);
			yield();
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			yield();
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
			yield();
			usePerson1(a);
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			usePerson2(new Person());
			yield();
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a = usePerson3(new Person());
			// `usePerson3` calls Waitframe, so the extra reference here
			// is not from the initial creation being added to the autorelease pool.
			// Instead, functions add their return value to the autorelease pool
			// so that they can't possible hit 0 when returning.
			check("RefCount(a)", RefCount(a), 2L);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = usePerson4(new Person());
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d - simple block === \n", ++tests);
		{
			Person a = new Person();
			yield();

			{
				auto b = a;
				check("RefCount(b)", RefCount(b), 2L);
			}
			check("RefCount(a)", RefCount(a), 1L);
		}

		printf("=== Test %d - if === \n", ++tests);
		{
			Person a = new Person();
			yield();

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
			yield();

			if (auto b = a)
				check("RefCount(b)", RefCount(b), 2L);
			check("RefCount(a)", RefCount(a), 1L);
		}

		printf("=== Test %d - while === \n", ++tests);
		{
			Person a = new Person();
			yield();

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
			yield();

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
		printf("=== Test %d === \n", ++tests);
		{
			Person a = usePerson5(new Person(), false);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = usePerson5(new Person(), true);
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = createPerson1();
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = createPerson2();
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
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
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			check("RefCount(a)", RefCount(a), 2L);
			checkCountWithGC(1);
			yield();
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			yield();
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
			yield();

			check("(3) RefCount(a)", RefCount(a), 1L);
			check("(4) RefCount(b)", RefCount(b), 1L);
			b = a;
			check("(5) count", count, 1);
			check("(6) RefCount(a)", RefCount(a), 2L);
			check("(7) RefCount(b)", RefCount(b), 2L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			yield();
			people[0] = a;
			people[5] = people[0];
			check("RefCount(a)", RefCount(a), 3L);
			people[0] = NULL;
			check("RefCount(a)", RefCount(a), 2L);
			checkCountWithGC(1);
		}
		checkCountWithGC(1);
		people[5] = NULL;
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			{
				Person b = new Person();
				checkCountWithGC(2);
			}
			yield();
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			yield();
			ArrayPushBack(people, a);
			check("RefCount(a)", RefCount(a), 2L);
			Person b = a;
			a = NULL;
			check("RefCount(b)", RefCount(b), 2L);
			ArrayPopBack(people);
			check("RefCount(b)", RefCount(b), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person b = new Person();
			yield();
			Person c[] = {b};
			check("RefCount(b)", RefCount(c[0]), 2L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person c[] = {new Person()};
			yield();
			check("RefCount(b)", RefCount(c[0]), 1L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person c[] = {new Person()};
			yield();
			checkCountWithGC(1);
			ResizeArray(c, 0);
			checkCountWithGC(0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			Person b = new Person();
			a->children[0] = b;
			yield();
			check("RefCount(b)", RefCount(b), 2L);
		}
		checkCountWithGC(0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			yield();
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
			yield();
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
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			delete a;
			check("RefCount(a)", RefCount(a), 1L);
			checkCountWithGC(1);
		}
		checkCountWithGC(0);

		// untyped does not increase reference count.
		printf("=== Test %d === \n", ++tests);
		{
			untyped a = new Person();
			check("RefCount(a)", RefCount(a), 1L);
			check("(1) count", count, 1);
			yield();
			check("(2) count", count, 0);
		}

		printf("=== Test %d === \n", ++tests);
		{
			untyped a = new Person();
			Person b = a;
			yield();
			check("RefCount(a)", RefCount(a), 1L);
		}
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			untyped a = new Person();
			Person b = a;
			yield();
			check("RefCount(a)", RefCount(a), 1L);
			b = NULL;
			checkCountWithGC(0);
		}
		check("count", count, 0);

		printf("=== Test %d === \n", ++tests);
		{
			Person a = new Person();
			yield();
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
			yield();
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

		printf("=== Test %d === \n", ++tests);
		{
			// Only 255 randgens are allowed currently. Check that the
			// GC runs when needed to make room.
			for (int i = 0; i < 300; i++)
			{
				randgen rng = Game->LoadRNG();
				if (<untyped>rng == 0)
					printf("failed to allocate randgen\n");
			}
			GC();
		}

		// TODO: currently arrays aren't ref counted, and array literals are always deleted after their
		// scope ends.
		// printf("=== Test %d === \n", ++tests);
		// {
		// 	int b[2];
		// 	{
		// 		int a[] = {1, 2};
		// 		b = a;
		// 		Trace(b[0]);
		// 		Trace(b[1]);
		// 		check("RefCount(a)", RefCount(a), 2L); // -1 ...
		// 	}
		// 	Trace(b[0]); // -1 ...
		// 	Trace(b[1]); // -1 ...
		// }
		// checkCountWithGC(0);

		// Internal bitmaps aren't real script objects.
		printf("=== Test %d === \n", ++tests);
		{
			bitmap internal_bmp_1 = Game->LoadBitmapID(RT_SCREEN);
			bitmap internal_bmp_2 = Game->LoadBitmapID(RT_BITMAP0);
			check("RefCount(internal_bmp_1)", RefCount(internal_bmp_1), -1L);
			check("RefCount(internal_bmp_2)", RefCount(internal_bmp_2), -1L);

			yield();
			checkCountWithGC(0);
			internal_bmp_2 = NULL;
			checkCountWithGC(0);

			internal_bmp_1 = new bitmap();
			check("RefCount(internal_bmp_1)", RefCount(internal_bmp_1), 2L);
			yield();
		}
		checkCountWithGC(0);

		// Global objects are never collected by the GC. It's up to the programmer
		// to not "lose" them. For example, the following test does not
		// save `a` anywhere recoverable from a new session, so this is
		// a memory leak.
		printf("=== Test %d === \n", ++tests);
		{
			auto a = new Person();
			yield();
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
		new Person();
		check("RefCount(person)", RefCount(person), 2L);
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
}
