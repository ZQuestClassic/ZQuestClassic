Literals
========

.. _zslang_literals:

Literals are a way of directly putting raw data in your code.

Primitive Literals
------------------

Primitive literals hold simple data of :ref:`primitive data types<types_prim>`.

Boolean
^^^^^^^

Booleans only have 2 values, each of which has it's own literal: `true` and `false`.

Numbers
^^^^^^^

.. table::
	:widths: grid

	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Name                     | Data Type           | Prefix/Suffix                              | Range                                           |
	+==========================+=====================+============================================+=================================================+
	| Decimal Number           | `int`/\ `float`     |                                            | `-214748.3648` to `214748.3647`                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Hexadecimal Number       | `int`/\ `float`     | :zs_num:`0x` prefix                        | `-0x346DC` to `0x346DC`                         |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Binary Number            | `int`/\ `float`     | `0b` prefix or :zs_num:`b` suffix          | `0b0` to `0b111111111111111111`                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Octal Number             | `int`/\ `float`     | `0o` prefix or :zs_num:`o` suffix          | `-0o643334` to `0o643334`                       |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Long Decimal Number      | `long`              | :zs_num:`L` suffix                         | `-2147483648L` to `2147483647L`                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Long Hexadecimal Number  | `long`              | :zs_num:`0x` prefix AND :zs_num:`L` suffix | `-0x80000000L` to `0x7FFFFFFFL`                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Long Binary Number       | `long`              | | :zs_num:`0b` prefix + :zs_num:`L` suffix | `0b0L` to `0b11111111111111111111111111111111L` |
	|                          |                     | | OR :zs_num:`Lb`/:zs_num:`bL` suffix      |                                                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+
	| Long Octal Number        | `long`              | | :zs_num:`0o` prefix + :zs_num:`L` suffix | `-0o20000000000L` to `0o17777777777L`           |
	|                          |                     | | OR :zs_num:`Lo`/:zs_num:`oL` suffix      |                                                 |
	+--------------------------+---------------------+--------------------------------------------+-------------------------------------------------+

.. note::
	If the integer portion of a number is 0, the leading 0 *can* be omitted (ex. `.9 == 0.9`).
	It can be easier to read if you put the 0 there, though.

Number Spacers
++++++++++++++

You may place spacers `_` between any 2 digits, which have no effect, but can help
make the number more readable. Example:

.. zscript::
	:style: body
	
	150_000 == 150000
	150_200_500L == 150200500L
	0x10_FF == 0x10FF
	0b1000_1000 == 0b10001000

Character Literals
^^^^^^^^^^^^^^^^^^

A character literal represents a single character value. It is represented as a single character,
between two single-quotes (ex. `'a'`).

Additionally, :ref:`escape characters<escape_characters>` can be used to insert
special characters, including `'` and `\\`.

Complex Literals
----------------

These literals represent more complex data. They may not be considered 'compile-time constant' in
all circumstances.

String Literals
^^^^^^^^^^^^^^^

A string literal represents an array of multiple character values. It is represented
as multiple characters, between two double-quotes (ex. `"apple"`).

Additionally, :ref:`escape characters<escape_characters>` can be used to insert
special characters, including `"` and `\\`.

Also, adjacent strings are automatically merged by the compiler. Ex:

.. zscript::
	:style: body
	
	printf("This is a string"
		" split across multiple lines!\n");
	// prints 'This is a string split across multiple lines!'

.. note::
	When typing a string as part of an :ref:`#include statement<directive_import>`,
	escape characters have no effect, and `\\` characters can be inserted normally.
	This is due to `\\` being common in file paths, and all strings worked this way
	in older versions of the program, so import path strings continue to act this way
	for compatibility purposes.


Array Literals
^^^^^^^^^^^^^^

.. todo::
	
	Array Literals

Difference between String/Array 'Literals' and 'Initializers'
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is a distinction between an array with an *initializer* (which looks and acts effectively
identical to a string literal), and a loose string/array literal that is not an initializer.
Notably, a loose literal creates a **temporary** array, which will stop existing almost
immediately. (These are useful, for example, in function call parameters, where they
would stop existing *after the whole call*).

.. _escape_characters:

Escape Characters
-----------------

Many of these originate from C.

.. table::
	:widths: grid

	+----------------+---------------------------------------+
	| Character      | Meaning                               |
	+================+=======================================+
	| \\a            | Alert (Beep, Bell)                    |
	+----------------+---------------------------------------+
	| \\b            | Backspace                             |
	+----------------+---------------------------------------+
	| \\f            | Formfeed Page Break                   |
	+----------------+---------------------------------------+
	| \\n            | Newline (Line Feed)                   |
	+----------------+---------------------------------------+
	| \\r            | Carriage Return                       |
	+----------------+---------------------------------------+
	| \\t            | Horizontal Tab                        |
	+----------------+---------------------------------------+
	| \\v            | Vertical Tab                          |
	+----------------+---------------------------------------+
	| \\\ \\         | Backslash                             |
	+----------------+---------------------------------------+
	| \\'            | Single-quote                          |
	+----------------+---------------------------------------+
	| \\"            | Double-quote                          |
	+----------------+---------------------------------------+
	| \\?            | Question Mark                         |
	+----------------+---------------------------------------+
	| \\x00 to \\xFF | Insert arbitrary hex character code   |
	+----------------+---------------------------------------+
