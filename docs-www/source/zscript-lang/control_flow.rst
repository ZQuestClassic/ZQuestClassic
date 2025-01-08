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
				printf("x is between 2 and 10 (inclusive)\n")
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

.. zscript::
		:fname: example.zs
		:height: 400

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

Additionally, these loops can be started with the ``do`` keyword, with the condition moved to the end, to create a ``do-while`` or ``do-until`` loop.
A loop using the ``do`` keyword will run at least one time, even if the condition isn't met.

.. zscript::
		:fname: example.zs
		:height: 400

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

Iterative Loops (For / For-each)
--------------------------------

.. _stmt_for:

.. _stmt_for_each:

Smart Loops (loop)
------------------

.. _stmt_loop:

Loop Else
---------

.. _stmt_loop_else:

