Keywords and Operators
======================

.. todo::

	|wip|

.. _zslang_keywords:

ZScript has a number of keywords, reserved for various purposes. Any keywords listed here that would otherwise be valid :ref:`identifiers<zslang_identifiers>`, are **not** usable as identifiers, due to being reserved.

Misc
------

.. table::
	:widths: auto

	+------------------+-------------------------------------------------------------------+
	| misc keyword     | description                                                       |
	+==================+===================================================================+
	| | ``true``       | boolean :ref:`literal<zslang_literals>` values                    |
	| | ``false``      |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``script``       | used to declare :ref:`scripts<zslang_scripts>`                    |
	+------------------+-------------------------------------------------------------------+
	| ``class``        | used to declare :ref:`classes<zslang_classes>`                    |
	+------------------+-------------------------------------------------------------------+
	| | ``if``         | :ref:`Conditional statement<stmt_if>`                             |
	| | ``unless``     |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| | ``while``      | :ref:`Conditional loop<stmt_while>`                               |
	| | ``until``      |                                                                   |
	| | ``do``         |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``for``          | :ref:`Iterative loop<stmt_for>`                                   |
	+------------------+-------------------------------------------------------------------+
	| ``in``           | Inclusion, used in :ref:`for-each loops<stmt_for_each>`           |
	+------------------+-------------------------------------------------------------------+
	| ``loop``         | :ref:`Smart loop<stmt_loop>`                                      |
	+------------------+-------------------------------------------------------------------+
	| ``else``         | Alt path for a :ref:`condition<stmt_if_else>` or                  |
	|                  | :ref:`loop<stmt_loop_else>`                                       |
	+------------------+-------------------------------------------------------------------+
	| | ``break``      | :ref:`Loop flow control<stmt_break>`                              |
	| | ``continue``   |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``switch``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``case``         |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``default``      |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``return``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``import``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``typedef``      |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``catch``        |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``OPTION_VALUE`` |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``IS_INCLUDED``  |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``enum``         | used to declare :ref:`enumerations<enums>`                        |
	+------------------+-------------------------------------------------------------------+
	| ``namespace``    | used when declaring :ref:`namespaces<namespaces>`                 |
	+------------------+-------------------------------------------------------------------+
	| ``using``        | used to bring remote symbols into scope via                       |
	|                  | :ref:`using statements<using>`                                    |
	+------------------+-------------------------------------------------------------------+
	| ``always``       | modifier for :ref:`using statements<using_always>`                |
	+------------------+-------------------------------------------------------------------+
	| ``repeat``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``inline``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``internal``     |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``static``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``constexpr``    |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``new``          |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``delete``       |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| ``CONST_ASSERT`` |                                                                   |
	+------------------+-------------------------------------------------------------------+
	| | ``asm``        | reserved, no present use                                          |
	| | ``zasm``       |                                                                   |
	| | ``try``        |                                                                   |
	+------------------+-------------------------------------------------------------------+

Types and Modifiers
-------------------

*only contains primitive types*

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
	| ``rgb``       | Color type, used to represent color values.                           |
	+---------------+-----------------------------------------------------------------------+
	| ``const``     | Placed before a type to specify that the variable is unchangable.     |
	+---------------+-----------------------------------------------------------------------+

.. note::
	``const`` makes a variable 'constant', meaning it cannot be changed. However, some things
	are an additional level of constant, known as 'compile-time constant'. A constant is only
	'compile-time constant' if the value it is set to is 'compile-time constant'.

	Generally, things that are compile-time constant include:
	- number literals
	- boolean literals
	- results of basic operators, where they only operate on compile-time constants
	- calls to ``constexpr`` functions, where the parameters are compile-time constants

Operators
---------

Operators come in both keyword and syntactical varieties, listed here alongside any alternate forms.

Most operators are left-associative, except assignment operators and the Ternary operator.

.. note:: Example key:

	- ``expr`` means 'any expression'
	- ``type`` means 'an identifier which resolves to the name of a type'
	- ``var`` means 'an identifier which resolves to the name of a variable'
	- ``func`` means 'an identifier which resolves to the name of a function'
	- ``boolean`` means 'an expression evaluating to true or false'

.. toctree::
	:maxdepth: 1
	:hidden:

	/zscript-lang/explainers/prepost_incdec

.. |APPROX_EQUAL_MARGIN| replace:: :abbr:`APPROX_EQUAL_MARGIN (default 0.0100)`

.. table::
	:widths: grid

	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| priority | operator         | example                     | description                                                                   |
	+==========+==================+=============================+===============================================================================+
	| 1        | ``(`` ``)``      | ( *expr* )                  | Grouping - allows specifying precedence of an expression.                     |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 2        | ``<`` ``>``      | < *type* > *expr*           | Casting - allows changing the type of an expression                           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 3        | ``++``           | *var*\+\+                   | | Post-increment. Returns the variable's value, then adds 1 to the variable.  |
	|          |                  |                             | | Related: :ref:`Pre- vs Post- increment/decrement<prepost_incdec>`           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 3        | ``--``           | *var*\-\-                   | | Post-decrement. Returns the variable's value, then subtracts 1 from the     |
	|          |                  |                             |   variable.                                                                   |
	|          |                  |                             | | Related: :ref:`Pre- vs Post- increment/decrement<prepost_incdec>`           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 3        | ``(`` ``)``      | *func*\(\)                  | Function call                                                                 |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 3        | ``[`` ``]``      | *array*\[*expr*\]           | Indexing- used to access indexes of an array                                  |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 3        | ``->``           | *expr*->\ *identifier*      | Arrow - used to access members of objects                                     |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 4        | ``++``           | \+\+\ *var*                 | | Pre-increment. Adds 1 to the variable, then returns that value.             |
	|          |                  |                             | | Related: :ref:`Pre- vs Post- increment/decrement<prepost_incdec>`           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 4        | ``--``           | \-\-\ *var*                 | | Pre-decrement. Subtracts 1 from the variable, then returns that value.      |
	|          |                  |                             | | Related: :ref:`Pre- vs Post- increment/decrement<prepost_incdec>`           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 4        | ``-``            | -\ *expr*                   | Negation. Inverts the sign of the expression.                                 |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 4        | | ``!``          | | !\ *expr*                 | Boolean Not. Returns '**true**' if the expression was '**false**' or          |
	|          | | ``not``        | | not *expr*                | '**0**', otherwise returns '**false**'                                        |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 4        | | ``~``          | | ~\ *expr*                 | Bitwise Not. Returns the bitwise inverse of the expression (flipping          |
	|          | | ``bitnot``     | | bitnot *expr*             | every '0' bit to '1', and vice-versa)                                         |
	|          | | ``compl``      | | compl *expr*              |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 5        | ``^^^``          | *expr* ^^^ *expr*           | Exponentiation. Raises the first expression to the power of the second        |
	|          |                  |                             | expression.                                                                   |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 6        | ``*``            | *expr* \* *expr*            | Multiplication. Multiplies the two expressions together.                      |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 6        | ``/``            | *expr* / *expr*             | Division. Divides the first expression by the second expression.              |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 6        | ``%``            | *expr* % *expr*             | Modulo. Divides the first expression by the second expression, returning      |
	|          |                  |                             | only the remainder of the division.                                           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 7        | ``+``            | *expr* + *expr*             | Addition. Adds the two expressions together.                                  |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 7        | ``-``            | *expr* - *expr*             | Subtraction. Subtracts the second expression from the first.                  |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 8        | | ``<<``         | | *expr* << *expr*          | Bitwise shift (left and right). Shifts the bits of the left expression,       |
	|          | | ``>>``         | | *expr* >> *expr*          | by the number of places in the right expression (left or right                |
	|          |                  |                             | respectively). Right expr is always an **int**.                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 9        | ``<``            | *expr* < *expr*             | Returns **true** if the left expr is less than the right expr.                |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 9        | ``>``            | *expr* > *expr*             | Returns **true** if the left expr is greater than the right expr.             |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 9        | ``<=``           | *expr* <= *expr*            | Returns **true** if the left expr is less than or equal the right expr.       |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 9        | ``>=``           | *expr* >= *expr*            | Returns **true** if the left expr is greater than or equal the right expr.    |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 10       | | ``==``         | | *expr* == *expr*          | Returns **true** if the two expressions are exactly equal to each other.      |
	|          | | ``equals``     | | *expr* equals *expr*      |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 10       | | ``!=``         | | *expr* != *expr*          | Returns **false** if the two expressions are exactly equal to each other.     |
	|          | | ``<>``         | | *expr* <> *expr*          |                                                                               |
	|          | | ``not_eq``     | | *expr* not_eq *expr*      |                                                                               |
	|          | | ``not_equal``  | | *expr* not_equal *expr*   |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 10       | | ``~~``         | | *expr* ~~ *expr*          | Returns **true** if the two expressions are *close* to equal. How close       |
	|          | | ``appx_eq``    | | *expr* appx_eq *expr*     | numbers need to be to be considered "close to equal" is determined by the     |
	|          | | ``appx_equal`` | | *expr* appx_equal *expr*  | |APPROX_EQUAL_MARGIN| :ref:`option<zslang_options>`.                          |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 10       | | ``^^``         | | *expr* ^^ *expr*          | Converts both expressions to boolean values (via '!= 0'). Then returns        |
	|          | | ``xor``        | | *expr* xor *expr*         | **true** if EXACTLY one value is **true**.                                    |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 11       | | ``&``          | | *expr* & *expr*           | Performs a boolean 'and' operation, on each binary bit of the expressions.    |
	|          | | ``bitand``     | | *expr* bitand *expr*      |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 12       | | ``^``          | | *expr* ^ *expr*           | Performs a boolean 'xor' operation, on each binary bit of the expressions.    |
	|          | | ``bitxor``     | | *expr* bitxor *expr*      |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 13       | | ``|``          | | *expr* \| *expr*          | Performs a boolean 'or' operation, on each binary bit of the expressions.     |
	|          | | ``bitor``      | | *expr* bitor *expr*       |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 14       | | ``&&``         | | *expr* && *expr*          | Boolean And. Returns **true** if both expressions are **true**.               |
	|          | | ``and``        | | *expr* and *expr*         |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 15       | | ``||``         | | *expr* \|\| *expr*        | Boolean Or. Returns **true** if at least one expression is **true**.          |
	|          | | ``or``         | | *expr* or *expr*          |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 16       | ``?`` ``:``      | *boolean* ? *expr* : *expr* | Ternary Expression. If the boolean is **true**, return the first              |
	|          |                  |                             | expression. Otherwise return the second expression.                           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 17       | ``delete``       | delete *expr*               | Deprecated as of 3.0. See: :ref:`class/object allocation<classobj_alloc>`     |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | | ``=``          | | *var* = *expr*            | Sets the variable to the value of the expression.                             |
	|          | | ``:=``         | | *var* := *expr*           |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``+=``           | *var* += *expr*             | Same as '*var* = *var* + *expr*'                                              |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``-=``           | *var* -= *expr*             | Same as '*var* = *var* - *expr*'                                              |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``*=``           | *var* \*= *expr*            | Same as '*var* = *var* \* *expr*'                                             |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``/=``           | *var* /= *expr*             | Same as '*var* = *var* / *expr*'                                              |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``%=``           | *var* %= *expr*             | Same as '*var* = *var* % *expr*'                                              |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``<<=``          | *var* <<= *expr*            | Same as '*var* = *var* << *expr*'                                             |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``>>=``          | *var* >>= *expr*            | Same as '*var* = *var* >> *expr*'                                             |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | | ``&=``         | | *var* &= *expr*           | Same as '*var* = *var* & *expr*'                                              |
	|          | | ``and_eq``     | | *var* and_eq *expr*       |                                                                               |
	|          | | ``and_equal``  | | *var* and_equal *expr*    |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | | ``|=``         | | *var* \|= *expr*          | Same as '*var* = *var* \| *expr*'                                             |
	|          | | ``or_eq``      | | *var* or_eq *expr*        |                                                                               |
	|          | | ``or_equal``   | | *var* or_equal *expr*     |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | | ``^=``         | | *var* ^= *expr*           | Same as '*var* = *var* ^ *expr*'                                              |
	|          | | ``xor_eq``     | | *var* xor_eq *expr*       |                                                                               |
	|          | | ``xor_equal``  | | *var* xor_equal *expr*    |                                                                               |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``~=``           | *var* ~= *expr*             | Same as '*var* = *var* & (~\ *expr*)'                                         |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``&&=``          | *var* &&= *expr*            | Same as '*var* = *var* && *expr*'                                             |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+
	| 18       | ``||=``          | *var* \|\|= *expr*          | Same as '*var* = *var* \|\| *expr*'                                           |
	+----------+------------------+-----------------------------+-------------------------------------------------------------------------------+

.. _operator_blurb:
.. caution:: Some operators behavior may not be intuitive. Notable behaviors:

	1. Exponentiation ``^^^``
		1. If either parameter is type ``long``, a long exponentiation will be performed. Ex. ``2L ^^^ 2L == 4L``.
		2. Per our definition, ``0^^^0 == 1``. With long type, ``0L ^^^ 0L == 1L``.
		3. If exponentiation overflows, the minimum representable value (``-214748.3648`` or ``-2147483648L``) will be returned.
	2. Bitwise operators ``~ << >> & ^ |`` where either parameter is type ``long`` will act on the full 32-bits. Otherwise, they will first truncate the value, and access the remaining ~~18 bits only.
	3. ``% 0`` and ``/ 0`` will produce a script error.
		1. ``% 0``, after producing the error, will return the value of ``% 1``.
		2. ``/ 0``, after producing the error, will return either ``214748.3647`` or ``-214748.3647``, matching the sign of the dividend.
	4. As the ``int`` type is not a real integer, 'Integer Division' does not exist- i.e. ``5 / 2 == 2.5``, instead of ``5 / 2 == 2``.

Syntax
------

While not technically 'keywords', this table shows reserved syntactical symbols/notations.

.. table::
	:widths: auto

	+-----------------+-------------------------------------------------+
	| syntax          | description                                     |
	+=================+=================================================+
	| ``,``           | delimiter                                       |
	+-----------------+-------------------------------------------------+
	| ``.``           | delimiter, decimal point                        |
	+-----------------+-------------------------------------------------+
	| ``;``           | line-ender                                      |
	+-----------------+-------------------------------------------------+
	| ``::``          | scope resolution                                |
	+-----------------+-------------------------------------------------+
	| ``:``           | delimiter, misc                                 |
	+-----------------+-------------------------------------------------+
	| ``(`` ``)``     | Parentheses                                     |
	+-----------------+-------------------------------------------------+
	| ``[`` ``]``     | Brackets                                        |
	+-----------------+-------------------------------------------------+
	| ``{`` ``}``     | Braces                                          |
	+-----------------+-------------------------------------------------+
	| ``#``           | Start of :ref:`hash-command<hash_instructions>` |
	+-----------------+-------------------------------------------------+
	| ``->``          | Arrow, for member access                        |
	+-----------------+-------------------------------------------------+
	| | ``=..=``      | :ref:`Range indicators<zslang_ranges>`          |
	| | ``=..``       |                                                 |
	| | ``..=``       |                                                 |
	| | ``..``        |                                                 |
	| | ``...``       |                                                 |
	+-----------------+-------------------------------------------------+
	| ``//``          | :ref:`Line Comment<comments>`                   |
	+-----------------+-------------------------------------------------+
	| | ``/*``        | :ref:`Block Comment<comments>`                  |
	| | ``*/``        |                                                 |
	+-----------------+-------------------------------------------------+
	| ``@``           | Handle for :ref:`annotations<annotations>`      |
	+-----------------+-------------------------------------------------+
	| | ``!!``        | Reserved, no current use                        |
	| | ``%%``        |                                                 |
	| | ``!%``        |                                                 |
	| | ``$$``        |                                                 |
	| | ``**``        |                                                 |
	| | ``@@``        |                                                 |
	| | ``$``         |                                                 |
	+-----------------+-------------------------------------------------+
