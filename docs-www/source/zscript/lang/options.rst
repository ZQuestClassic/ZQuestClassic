Compiler Options
================

.. _zslang_options:

Compiler options are things you can change to change how the compiler
interprets your code. Many of the available options exist for compatibility
for older scripts, when things about ZScript worked differently-
said options are usually best left at their defaults unless you specifically
know what you are doing. Other options are designed specifically to be
changable as needed for a variety of purposes, and can be changed at your leisure.

Option Scope
------------

Different parts of your scripts can use different options.
This is accomplished by using the :zs_kw:`#option` compiler directive.
The :zs_kw:`#option` directive may be used in any scope. It must be at the top
of the scope, before any other statements or declarations.

For example, if you're compiling an older script that relies
on legacy array syntax, you could put:

.. zscript::
	#option LEGACY_ARRAYS on

at the top of the script that requires the old behavior. Then, while
compiling that script, the compiler will use the old behavior for that scope.

.. _option_inherit:

Option Inheritance
------------------

In every scope except the 'script buffer' in the editor
defaults every single option to the special value :zs_kw:`inherit`.

This means that they will inherit the value for that option from the
scope above them. Since the 'script buffer' has nothing above it,
the options it uses are set in a menu in the editor,
``ZScript->Compiler Settings``. In the table below on this page,
the listed 'Default' values are what you will see in this menu
upon a fresh installation of the program.

You can technically manually set options to :zs_kw:`inherit`, though as
it will already be this for every scope, this has no effect.
You can also manually set options to the special value :zs_kw:`default`,
which will inherit the settings **directly** from the menu
in the editor, ignoring all the scopes above.

What are all the options?
-------------------------

Standard Options
^^^^^^^^^^^^^^^^

.. |onoffauto| replace:: :zs_kw:`on` :zs_kw:`off` :zs_kw:`auto`
.. |onoff| replace:: :zs_kw:`on` :zs_kw:`off`
.. |onoffew| replace:: :zs_kw:`on` :zs_kw:`off` :zs_kw:`error` :zs_kw:`warn`
.. |onoffw| replace:: :zs_kw:`on` :zs_kw:`off` :zs_kw:`warn`
.. |offew| replace:: :zs_kw:`off` :zs_kw:`error` :zs_kw:`warn`

.. table::
	:widths: auto

	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| Name                               | Default          | Possible Values\ [#v]_| Effect                                                          |
	+====================================+==================+=======================+=================================================================+
	| NO_LOGGING                         | :zs_kw:`off`     | |onoff|               | All functions that would log to the console are compiled away,  |
	|                                    |                  |                       | producing no code or output.                                    |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| NO_ERROR_HALT                      | :zs_kw:`off`     | |onoff|               | If on, the compiler will keep trying as long as it can, even    |
	|                                    |                  |                       | after it encounters an error. If off, it stops after one error. |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| APPROX_EQUAL_MARGIN                | `0.0100`         | numbers in `float`    | The `~~` (approximately equal) operator will return `true` if   |
	|                                    |                  | range                 | the difference between its operands is `<=` this value.         |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| STRING_SWITCH_CASE_INSENSITIVE     | :zs_kw:`off`     | |onoff|               | If enabled, :ref:`switch statements using strings<switch_strs>` |
	|                                    |                  |                       | will match the strings case-insensitively.                      |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| WARN_DEPRECATED                    | :zs_kw:`off`     | |onoffew|             | If :zs_kw:`on` or :zs_kw:`warn`, issues a warning when using    |
	|                                    |                  |                       | any internal symbol that is marked as ``deprecated``. Setting   |
	|                                    |                  |                       | to :zs_kw:`error` gives a compile error instead.                |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| ON_MISSING_RETURN                  | :zs_kw:`error`   | |offew|               | If :zs_kw:`off`, does nothing. If :zs_kw:`warn` or              |
	|                                    |                  |                       | :zs_kw:`error`, warns/errors when a function with a non-void    |
	|                                    |                  |                       | return type can exit without returning a valid value.           |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| LEGACY_ARRAYS                      | :zs_kw:`warn`    | |onoffw|              | If :zs_kw:`on`, does nothing. If :zs_kw:`warn` or :zs_kw:`off`, |
	|                                    |                  |                       | warns/errors when a variable with a non-array type tries to     |
	|                                    |                  |                       | hold an array.                                                  |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| DEFAULT_STATIC_SCRIPT_MEMBERS      | :zs_kw:`warn`    | |onoffauto|           | If :zs_kw:`auto`, picks between :zs_kw:`on` / :zs_kw:`off`      |
	|                                    |                  |                       | automatically. All variables and functions except `void run`    |
	|                                    |                  |                       | inside a script will be :zs_kw:`static` by default if           |
	|                                    |                  |                       | :zs_kw:`on`, requiring the :zs_kw:`nonstatic` keyword to make   |
	|                                    |                  |                       | specific ones non-static. If :zs_kw:`off`, functions/variables  |
	|                                    |                  |                       | in scripts will be non-static by default, requiring the         |
	|                                    |                  |                       | :zs_kw:`static` keyword to be static.                           |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+

.. plans::

	``STRING_SWITCH_CASE_INSENSITIVE`` may become deprecated in favor
	of an :ref:`annotation<annotations>` on switch statements instead.

Compatibility Options
^^^^^^^^^^^^^^^^^^^^^

.. versionremoved:: 3.0
	All of these options are no longer supported as of 3.0. The default behavior listed below is the only behavior in 3.0.

You almost certainly want to leave these in their default state, unless you know what you are doing.
	
.. table::
	:widths: auto

	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| Name                               | Default          | Possible Values\ [#v]_| Effect                                                          |
	+====================================+==================+=======================+=================================================================+
	| TRUNCATE_DIVISION_BY_LITERAL_BUG   | :zs_kw:`off`     | |onoff|               | Enables old bug where division specifically by a literal        |
	|                                    |                  |                       | was wrongly truncating, resulting in integer division.          |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| SHORT_CIRCUIT                      | :zs_kw:`on`      | |onoff|               | Allows boolean expressions to short-circuit, exiting early if   |
	|                                    |                  |                       | the final answer is already known.                              |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| BOOL_TRUE_RETURN_DECIMAL           | :zs_kw:`off`     | |onoff|               | Most boolean `true` values will use the internal representation |
	|                                    |                  |                       | `0.0001` instead of `1` if this is enabled.                     |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| TRUE_INT_SIZE                      | :zs_kw:`on`      | |onoff|               | Changes the size range of number literals. If off, their range  |
	|                                    |                  |                       | is `-214747.0000` to `214747.0000`.                             |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+
	| BINARY_32BIT                       | :zs_kw:`off`     | |onoff|               | Makes bitwise operators treat `int`\ s the same way they do     |
	|                                    |                  |                       | `long`\ s. Do not enable this. Just use `long`\ s instead.      |
	+------------------------------------+------------------+-----------------------+-----------------------------------------------------------------+

.. [#v]
	ANY option can be set to the special values :zs_kw:`inherit`
	and :zs_kw:`default`, described :ref:`above <option_inherit>`
