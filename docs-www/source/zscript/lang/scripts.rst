Scripts
=========

.. _zslang_scripts:

What is a script?
-----------------

Scripts are the foundation of ZScript. You cannot do anything in ZScript without
scripts. Every script has a `void run()` function, the entry point for the script.

Each script type runs its scripts based on different things occurring in the engine -
for example, an `npc script` runs when an `npc` (an enemy) is alive.

.. style:: h2

.. toctree::
	:maxdepth: 1

	/zscript/lang/script_types

Annotations
-----------

Scripts are capable of being targetted with a number of :ref:`annotations<annotations>`.

.. style:: zs_caption

For `String` values, the number in parentheses is the maximum length, in characters.

These annotations take a single value:

.. table::
	:widths: auto
	
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| Annotation Name                         | Value           | Purpose                                       |
	+=========================================+=================+===============================================+
	| `@Author`                               | String (256)    | Indicate who authored the script.             |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@Attribute0` - `@Attribute9`           | String (256)    | Sets labels in the Combo / Item editors.      |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@AttributeHelp0` - `@AttributeHelp9`   | String (65535)  | Sets help text in the Combo / Item editors.   |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@Flag0` - `@Flag15`                    | String (256)    | Sets labels in the Combo / Item editors.      |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@FlagHelp0` - `@FlagHelp15`            | String (65535)  | Sets help text in the Combo / Item editors.   |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@Attribyte0` - `@Attribyte7`           | String (256)    | Sets labels in the Combo editor.              |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@AttribyteHelp0` - `@AttribyteHelp7`   | String (65535)  | Sets help text in the Combo editor.           |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@Attrishort0` - `@Attrishort7`         | String (256)    | Sets labels in the Combo editor.              |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@AttrishortHelp0` - `@AttrishortHelp7` | String (65535)  | Sets help text in the Combo editor.           |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@InitD0` - `@InitD7`                   | String (256)    | Sets labels for the `void run` parameters.    |
	|                                         |                 | Defaults to the parameter names.              |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@InitDHelp0` - `@InitDHelp7`           | String (65535)  | Sets help text for the `void run` parameters. |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@InitDType0` - `@InitDType7`           | "D", "H", "LD", | Sets the input field type of the textbox for  |
	|                                         | "LH", "B", "-1" | inputting the `void run` parameters. Default  |
	|                                         |                 | based on `void run` parameter type, as:       |
	|                                         |                 |                                               |
	|                                         |                 | | `int`, `float`, `untyped` -> "D"            |
	|                                         |                 | | `long` -> "LD"                              |
	|                                         |                 | | `bool` -> "B"                               |
	|                                         |                 | | `rgb` -> "LH"                               |
	|                                         |                 | | else -> "-1"                                |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@InitScript`                           | `int`           | Only valid on `global` scripts. Marks that    |
	|                                         |                 | the script should be 'merged' into the global |
	|                                         |                 | script '~Init' that is auto-generated by the  |
	|                                         |                 | compiler.                                     |
	|                                         |                 |                                               |
	|                                         |                 | This means that this script will run once,    |
	|                                         |                 | for 1 frame, when the player begins a 'New    |
	|                                         |                 | Game'. The 'int' parameter determines the     |
	|                                         |                 | order that these will run in, starting with   |
	|                                         |                 | the *lower* numbers and counting up.          |
	|                                         |                 |                                               |
	|                                         |                 | A global script named :zs_title:`Init` will   |
	|                                         |                 | always be merged this way, using `0` as the   |
	|                                         |                 | value if this annotation is not provided.     |
	+-----------------------------------------+-----------------+-----------------------------------------------+

These annotations take a varying amount of values.
A value in [square brackets] indicates an 'optional' value.

.. table::
	:widths: auto
	
	+-----------------------------------------+-----------------------------------------------+-----------------------------------------------+
	| Annotation Name                         | Values                                        | Purpose                                       |
	+=========================================+===============================================+===============================================+
	| `@ExportInitD0` - `@ExportInitD7`       | | Name: String (256)                          | Sets the labels, helptext, and input field    |
	|                                         | | Help Text: [String (65535)]                 | type of the textbox for the specified         |
	|                                         | | Type: ["D", "H", "LD", "LH", "B", "-1"]     | `void run` parameters.                        |
	|                                         |                                               |                                               |
	|                                         |                                               | Name defaults to the variable name, help text |
	|                                         |                                               | defaults to blank, and type defaults based on |
	|                                         |                                               | variable type, as:                            |
	|                                         |                                               |                                               |
	|                                         |                                               | | `int`, `float`, `untyped` -> "D"            |
	|                                         |                                               | | `long` -> "LD"                              |
	|                                         |                                               | | `bool` -> "B"                               |
	|                                         |                                               | | `rgb` -> "LH"                               |
	|                                         |                                               | | else -> "-1"                                |
	+-----------------------------------------+-----------------------------------------------+-----------------------------------------------+
