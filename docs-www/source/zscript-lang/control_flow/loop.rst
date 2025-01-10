Smart Loops ('loop')
====================

'loop' loops
------------

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

.. zscript::

	void infinite()
	{
		loop() // similar to 'while(true)'
		{
			// Run something every frame here
			Waitframe();
		}
	}

.. zscript::

	loop(x : 1=..10, 2) // similar to 'for(int x = 1; x < 10; x += 2)'
		printf("x is %d\n", x);
	
	loop(0=..3)
		printf("Test!\n");
	
	loop(const long l : 0=..1, 100L)
		printf("l is %d\n", l);
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

