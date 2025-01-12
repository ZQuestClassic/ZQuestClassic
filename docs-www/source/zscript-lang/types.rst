Types
=====

.. _types_prim:

Primitive Types
---------------

.. table::
	:widths: auto

	+---------------+-----------------------------------------------------------------------+
	| name          | description                                                           |
	+===============+=======================================================================+
	| | ``int``     | Numeric types. Despite their names, these are neither integers nor    |
	| | ``float``   | floating-point numbers, but instead fixed-point numbers ranging       |
	|               | **-214748.3648** to **214748.3647**.                                  |
	+---------------+-----------------------------------------------------------------------+
	| ``long``      | Long integer type. Values range from **-2147483648L** to              |
	|               | **2147483647L**. :ref:`Special bitwise behavior<operator_blurb>`.     |
	+---------------+-----------------------------------------------------------------------+
	| ``auto``      | Type will be automatically determined by the parser.                  |
	+---------------+-----------------------------------------------------------------------+
	| ``void``      | Lack of a type; used to indicate a function returns nothing.          |
	+---------------+-----------------------------------------------------------------------+
	| ``untyped``   | General type; almost all types implicity cast to **untyped**.         |
	+---------------+-----------------------------------------------------------------------+
	| ``bool``      | Boolean type; available values **true**, **false**.                   |
	+---------------+-----------------------------------------------------------------------+
	| ``char32``    | Character type. Same boundaries as **int**, mainly exists for         |
	|               | typecasting. Used by char/string literals.                            |
	+---------------+-----------------------------------------------------------------------+
	| ``rgb``       | Color type, used to represent color values. See:                      |
	|               | :ref:`CreateRGB<globals_graphics_fun_creatergb>`                      |
	+---------------+-----------------------------------------------------------------------+

.. _types_const:

'Constant' types
----------------

By adding the ``const`` modifier to a type, that type becomes "constant". Declaring a Variable with
a constant type instead declares a Constant. Important notes about constants:

- A constant must have an initializer; without one, it will compile error.
- A constant's value cannot be modified after it's initializer.
- If the entire initializer expression is "compile-time constant", then the value of the constant is also "compile-time constant"

.. _compiletime_const:

Compile-time constants
^^^^^^^^^^^^^^^^^^^^^^

Any value that is entirely known and unable to ever change starting at compile-time, is known as a
'compile-time constant'. Certain expressions and statements specifically require compile-time constant
values. But, what things are compile-time constant?

- Any :ref:`primitive literal<zslang_literals>` value, such as ``5``, ``2.7``, ``true``.
- Any declared constant with an initializer that is compile-time constant
- The results of basic operators, where every operand is compile-time constant (ex. ``5 + 2``, ``2 < 6``)
- The return of a function call to a ``constexpr`` function, where every parameter is compile-time constnat

Other Types
-----------

For documentation on existing other types, see the :ref:`internal class / constant documentation<zsdoc_index>`.

