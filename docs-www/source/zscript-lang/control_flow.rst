Control Flow
============

Conditional Statements (If / Unless)
--------------------------------------

.. _stmt_if:

A conditional statement allows executing a section of code only when a condition is met.
By using ``if(condition)``, the code will run when the condition evaluates to any value other than ``0``.
By using ``unless(condition)``, the code will run only when the condition evaluates to ``0`` (so, the opposite).

.. dropdown:: Example

	.. zscript::
		:fname: example.zs
		:height: 400
		
		ffc script f{void run(){}}
		void foo()
		{
			int x = 3;
			if(x < 5)
				printf("x is less than 5\n");
			
			unless(x > 10)
			{
				printf("x is NOT greater than 10\n");
			}
		}
		/* Prints:
		x is less than 5
		x is NOT greater than 10
		*/

After the conditional statement, whatever the next statement after that is will be the "body" of the statement.
The body code is the code that is executed when the condition is met.

To have more than one statement be part of the same condition, you can use block them together using ``{}``.

Conditional Else
----------------

.. _stmt_if_else:

Placing an ``else`` after a conditional statement's body, allows you to run a second body statement in the **opposite** condition.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs
		:height: 400

		ffc script f{void run(){}}
		void foo()
		{
			int x = 3;
			if(x > 6)
				printf("x is greater than 6\n");
			else
				printf("x is NOT greater than 6\n");
			
			unless(x < 10)
				printf("x is NOT less than 10\n");
			else if(x < 2)
				printf("x is less than 2\n");
			else
			{
				printf("x is between 2 and 10 (inclusive)\n");
			}
		}
		/* Prints:
		x is NOT greater than 6
		x is between 2 and 10 (inclusive)
		*/

As you can see in the example, you can use this to chain multiple conditionals together.


Conditional Loops (While / Until)
---------------------------------

.. _stmt_while:

Conditional loops work similarly to conditional statements, except the body code runs multiple times, until the condition fails.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			int x = 3;
			while(x > 0)
			{
				printf("x is %d\n", x);
				x -= 1;
			}
			int y = 0;
			until(y == 5)
			{
				y += 1;
				printf("y is %d\n", y);
			}
		}
		/* Prints:
		x is 3
		x is 2
		x is 1
		y is 1
		y is 2
		y is 3
		y is 4
		y is 5
		*/

Additionally, ``while`` and ``until`` loops can be started with the ``do`` keyword,
with the condition moved to the end, to create a ``do-while`` or ``do-until`` loop.
A loop using the ``do`` keyword will run at least one time, even if the condition isn't met.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			int x = 3;
			do
			{
				printf("x is %d\n", x);
				x -= 1;
			}
			while(x > 10);

			int y = 0;
			do
			{
				y += 1;
				printf("y is %d\n", y);
			}
			until(y < 2);
		}
		/* Prints:
		x is 3
		y is 1
		*/

Related: :ref:`Loop Else<stmt_loop_else>`, :ref:`Break<stmt_break>`, :ref:`Continue<stmt_continue>`

Iterative Loops (For)
---------------------

.. _stmt_for:

A basic ``for`` loop is made of 4 parts; the declaration, the condition, the increment, and the body.
The condition and the body function much the same as ``while`` loops.
The declaration allows declaring a new variable, or list of variables.
The increment allows a statement, or comma-delimited list of statements.

The format of these is ``for(declaration; condition; increment)``, followed by the body.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			for(int x = 0; x < 10; x += 2)
				printf("x is %d\n", x);
		}
		/* Prints:
		x is 0
		x is 2
		x is 4
		x is 6
		x is 8
		*/

Related: :ref:`Loop Else<stmt_loop_else>`, :ref:`Break<stmt_break>`, :ref:`Continue<stmt_continue>`

Iterative Loops (For-Each)
--------------------------

.. _stmt_for_each:

Alternatively, a ``for`` loop can be used with an array, to create what is refered to as a ``for-each`` loop.
This has a different format, ``for(varname : array)`` / ``for(varname in array)``.
The type of the variable will be the element type of the array.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			int[] some_values = {1,2,7,20,5};
			for(x : some_values)
				printf("x is %d\n", x);
		}
		/* Prints:
		x is 1
		x is 2
		x is 7
		x is 20
		x is 5
		*/

		void print_enemies()
		{
			for(e in Screen->NPCs)
				printf("Enemy number %d\n", e->ID);
		}
		// Prints the enemy ID number of each enemy on screen, in order.

Related: :ref:`Loop Else<stmt_loop_else>`, :ref:`Break<stmt_break>`, :ref:`Continue<stmt_continue>`

Smart Loops (loop)
------------------

.. _stmt_loop:

Smart loops, declared using the ``loop`` keyword, are able to be optimized better by the compiler than
normal loops. There are a few different ways to use smart loops.

Firstly, you can simply declare ``loop()``, which results in an infinite loop, similar to ``while(true)``.

Otherwise, you can follow the pattern ``loop(typename varname : range, increment)``.
 - 'typename' specifies a type for the loop's variable
 	- You can omit it, letting it default to ``const int``
 - 'varname' is an identifier declaring the name of the loop's variable
	- You can omit it to not name the variable
	- If you do so, you must also omit 'typename' and the ``:``
 - The ``:`` can be replaced with ``in``
 - 'range' is a :ref:`value range<zslang_ranges>`
	- The loop's variable starts at the low end, and increments by the increment value until it would exit the range
 - 'increment' is the increment value of the range
	- You can omit it, letting it default to ``1``
	- If you do so, you must also omit the ``,``
	- If a negative increment is supplied, the loop's variable will start at the high end of the range, and count down.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void infinite()
		{
			loop() // similar to 'while(true)'
			{
				// Run something every frame here
				Waitframe();
			}
		}
		void foo()
		{
			loop(x : 1=..10, 2) // similar to 'for(int x = 1; x < 10; x += 2)'
				printf("x is %d\n", x);
			
			loop(0=..3)
				printf("Test!\n");
			
			loop(const long l : 0=..1, 100L)
				printf("l is %d\n", l);
		}
		/* Prints:
		x is 1
		x is 3
		x is 5
		x is 7
		x is 9
		Test!
		Test!
		Test!
		l is 0
		l is 0.01
		l is 0.02
		l is 0.03
			(this continues for a while)
		l is 0.98
		l is 0.99
		*/

Related: :ref:`Loop Else<stmt_loop_else>`, :ref:`Break<stmt_break>`, :ref:`Continue<stmt_continue>`

Loop Else
---------

.. _stmt_loop_else:

An ``else`` can be added after a loop, which has special meaning related to the :ref:`break statement<stmt_break>`.
The code in the body of the ``else`` will run in the condition that the loop ends WITHOUT a :ref:`break statement<stmt_break>` running.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			for(enemy : Screen->NPCs)
			{
				if(Distance(Hero->X, Hero->Y, enemy->X, enemy->Y) < 64)
				{
					printf("Found an enemy %d near the Hero!\n", enemy->ID);
					break;
				}
			}
			else
			{
				printf("Found no enemies near the Hero!\n");
			}
		}

Related: :ref:`break<stmt_break>`, :ref:`while<stmt_while>`, :ref:`for<stmt_for>`,
:ref:`for-each<stmt_for_each>`, :ref:`loop<stmt_loop>`

Loop Flow (break/continue)
--------------------------

.. _stmt_break:

.. _stmt_continue:

When inside a loop, you don't need to strictly just run the code until the condition ends.
By using the ``break`` statement, you can exit a loop early.
By using the ``continue`` statement, you can skip ahead to the next iteration of the loop.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		ffc script f{void run(){}}
		void foo()
		{
			for(int q = 0; q < 10; ++q)
			{
				printf("q is %d\n", q);
				if(q == 2)
					break;
			}
			for(int p = 0; p < 10; ++p)
			{
				unless(p % 3) //skip multiples of 3
					continue;
				printf("p is %d\n", p);
			}
		}
		/* Prints:
		q is 0
		q is 1
		q is 2
		p is 1
		p is 2
		p is 4
		p is 5
		p is 7
		p is 8
		p is 10
		*/

Additionally, if you have nested loops (loops inside of loops), you can target any of the loops you are in,
by specifying a number of loops to go back.

.. dropdown:: Example

	.. zscript::
		:fname: example.zs

		global script example
		{
			bool check_something(int x, int y)
			{
				return RandGen->Rand(1, 100) <= 10; // %10 random chance
			}
			bool check_something_else(int x, int y)
			{
				return RandGen->Rand(1, 100) <= 2; // %2 random chance
			}
			void run()
			{
				for(int x = 0; x < 10; ++x)
				{
					for(int y = 0; y < 10; ++y)
					{
						if(check_something(x, y))
						{
							printf("Found something at %d,%d\n", x, y);
							continue; // go to the next 'y'
						}
						else if(check_something_else(x, y))
						{
							printf("Found something else at %d,%d\n", x, y);
							continue 2; // go to the next 'x'
						}
						printf("Found nothing at %d,%d\n", x, y);
					}
				}
			}
		}

