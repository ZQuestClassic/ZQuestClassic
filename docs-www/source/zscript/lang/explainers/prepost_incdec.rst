Pre- vs Post- increment/decrement
=================================

.. _prepost_incdec:

The difference between `++x` and `x++`
------------------------------------------

The important distinction between Pre- and Post- increment/decrement operators
is in the "return value" of the operator. Both `++x` and `x++` have the same side-effect;
the value of `x` will go up by 1 when the statement runs. However, if you *read* the value
returned by the operator, `++x` will read the value **AFTER** it goes up by 1, and
`x++` will read the value **BEFORE** it goes up by 1.

.. zscript::
	:style: body

	int x = 5;
	Trace(x++); // prints '5'
	Trace(x);   // prints '6'
	int y = 2;
	Trace(++y); // prints '3'
	Trace(y);   // prints '3'

.. note::
	One of the most common places you'll see these operators,
	is in the `increment` part of a :ref:`for loop<stmt_for>`.
	When it is there, generally the operator's "return value" is ignored, so
	it does not matter which you use.
