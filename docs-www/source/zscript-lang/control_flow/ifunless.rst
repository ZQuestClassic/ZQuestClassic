If / Unless / Else
==================

Conditional Statements
----------------------

.. _stmt_if:

A conditional statement allows executing a section of code only when a condition is met.
By using ``if(condition)``, the code will run when the condition evaluates to any value other than ``0``.
By using ``unless(condition)``, the code will run only when the condition evaluates to ``0`` (so, the opposite).

.. zscript::
	
	int x = 3;
	if(x < 5)
		printf("x is less than 5\n");
	
	unless(x > 10)
	{
		printf("x is NOT greater than 10\n");
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

.. zscript::

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
	/* Prints:
	x is NOT greater than 6
	x is between 2 and 10 (inclusive)
	*/

As you can see in the example, you can use this to chain multiple conditionals together.

