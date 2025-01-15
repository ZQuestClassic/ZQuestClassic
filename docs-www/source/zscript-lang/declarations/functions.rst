.. _zslang_functions:

Functions
---------

Functions are blocks of code, which can be 'called' from multiple locations. A
function, when called, can take any number of `parameters`, which are values
set as part of the call which become local variables inside the function.
Additionally, a function can `return` a single value back to the calling code.

All runnable code is inside functions; each :ref:`script<zslang_scripts>` has
a special `void run()` function, whose max number of `parameters` varys depending
on the script's type. The script begins execution by calling this function,
using values set in the editor for the `parameter` values.

.. zscript::

	int double(int val)
	{
		return val * 2;
	}

	global script Active
	{
		void run()
		{
			Trace(double(5)); // outputs '10.0000'
			Trace(double(25)); // outputs '50.0000'
			Trace(double(0.4)); // outputs '0.8000'
		}
	}

In the above example, the `double` function is defined, taking a single `int`
parameter, and returning an `int` value. The `global script Active` then calls
the internal function :ref:`Trace<globals_fun_trace>` 3 times, each time
giving it the return value of a call to the `double` function as it's parameter
(each time with a different number given to the `double` function).

Remote Declaration
------------------

You can declare functions in *remote* scopes by using an "identifier list"
instead of a single "identifier"- both of the following examples are identical.

.. grid:: 2 2 2 2

	.. grid-item::

		.. zscript::

			namespace foo
			{
				void bar()
				{
					Trace(5);
				}
			}

	.. grid-item::

		.. zscript::

			void foo::bar()
			{
				Trace(5);
			}

.. _func_opt_params:

Optional Parameters
-------------------

You can give function parameters a |ctc| initializer, to make them 'optional'.
If a function is called without some of it's optional parameters, they will
automatically use the default value provided instead. No non-optional
parameters may appear after an optional parameter.

.. tab-set::

	.. tab-item:: Ex. 1

		.. zscript::

			void foo(int x = 25)
			{
				Trace(x * 2);
			}
		
		.. zscript::
			:style: body

			foo(); // outputs '50'
			foo(10); // outputs '20'

	.. tab-item:: Ex. 2

		.. include:: /zscript-lang/examples/find_closest_npc.rst

	.. tab-item:: Ex. 3
		
		.. style::
			:classes: zs_caption
		
		Can't put a mandatory parameter after an optional parameter!

		.. zscript::

			void example(<error>int x = 5, int y</error>)
			{
				// syntax error, unexpected RPAREN, expecting ASSIGN
				// You must give each
			}

Variadic Parameters
-------------------

.. todo::

	Variadic Params

.. _func_proto:

Prototype Functions
-------------------

.. todo::

	Prototype Functions
