Enums
=====

.. todo::

	|wip|

.. _zslang_enums:

.. _enums:

Basic Enums
-----------

Enums allow you to define an enumerated list of constants, each incremented
from the previous. Their most simple syntax is as follows:

.. zscript::

	enum
	{
		A, // 0
		B, // 1
		C, // 2
		D, // 3
	};

By default, the constants are of type `const int`, and increment by `1` each.
To define constants of a different pre-existing type, you can use syntax like:

.. zscript::

	enum = long
	{
		A, // 0L
		B, // 1L
		C, // 2L
		D, // 3L
	};

These constants will be of type `const long` instead of `const int`.

.. _enum_@increment:

Custom Increment
----------------

Additionally, you can change how much the enum increments by between each number.
Firstly, using `enum = long` has the special behavior of incrementing by `1L`
instead of `1` already. But, additionally, you can use the `@Increment` annotation
to directly specify an amount to increment by.

.. zscript::

	@Increment(2)
	enum
	{
		A, // 0
		B, // 2
		C, // 4
		D, // 6
	};

.. versionadded:: 3.0
	@Increment

Custom Type Creation
--------------------

.. todo::
	
	enum types
