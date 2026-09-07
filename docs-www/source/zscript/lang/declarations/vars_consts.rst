Variables / Constants
=====================

.. _zslang_variables:

Variables
---------

To declare a ``variable``, you first specify what :ref:`type<types>`
it will be, then an identifier for its name, then optionally
an initializer to give it a starting value. Finally, you can
list multiple variables in a single declaration as a
comma-delimited list.

If the initializer is omitted, the starting value will always
be `0`/`NULL`.

.. zscript::
	:style: body

	int x; // starts at '0'
	int y = 7;

	int a = 2, b = 3; // 2 variables of 'int' type
	int c, d, e = 5, f = 9; // c and d are '0'

.. _zslang_constants:

.. _constants:

Constants
---------

Declaring a ``constant`` is extremely similar to declaring a
``variable``- the only thing that determines a constant is the
'type' being `const`. Important notes about constants:

- A constant must have an initializer; without one, it will compile error.
- A constant's value cannot be modified after its initializer.
- If the entire initializer expression is "compile-time constant", then the value of the constant is also "compile-time constant"
- Constants can be declared in any scope (file, namespace, script, class, function, or block), not only at file scope. The constant is only visible within that scope.
- Outside of a function body, the initializer *must* be compile-time constant.

Inside a function body (including any block within it), the initializer does not have to
be compile-time constant - it may read an engine value, call a function, and so on. Such
a constant behaves like a normal variable that is initialized at runtime, except that
assigning to it afterwards is a compile error, and it cannot be used where a |ctc| is
required (array sizes, `case` values, `repeat()`, etc.).

.. zscript::

	void run()
	{
		const int max_hp = Hero->MaxHP; // read when this line runs
		<error>max_hp = 5;</error> // Error C036: Variable max_hp is constant and cannot be changed.
	}

At any other scope, the same initializer is an error:

.. zscript::
	:style: body

	<error>const int MAX_HP = Hero->MaxHP;</error> // Error C062: Constant MAX_HP is not at function scope, and the value is not constant at compile-time.

Function parameters can also be declared `const`, which prevents the function body
from reassigning them:

.. zscript::

	void foo(const int x, const bool flag)
	{
		<error>x = 5;</error> // Error C036: Variable x is constant and cannot be changed.
	}

.. _compiletime_const:

Compile-time constants
^^^^^^^^^^^^^^^^^^^^^^

Any value that is entirely known and unable to ever change starting at compile-time, is known as a
'compile-time constant'. Certain expressions and statements specifically require compile-time constant
values. But, what things are compile-time constant?

- Any :ref:`primitive literal<zslang_literals>` value, such as `5`, `2.7`, `true`.
- Any declared constant with an initializer that is compile-time constant
- The results of basic operators, where every operand is compile-time constant (ex. `5 + 2`, `2 < 6`)
- The return of a function call to a `constexpr` function, where every parameter is compile-time constnat
