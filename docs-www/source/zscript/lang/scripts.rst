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

.. _script_static:

Instance Variables
------------------
Declaring variables inside a script creates 'instance variables'. These variables will
hold a different value for each instance of the script that is running. The `this` variable
of scripts is now an instance variable.

A variable declared with the `static` keyword will be a global variable instead, holding one
value for all instances of the script. Constants are automatically static.

The compiler option :ref:`DEFAULT_STATIC_SCRIPT_MEMBERS<zslang_options>` strongly affects instance variables.
It may mark all script variables and functions as `static` depending on the setting; this is used to preserve old scripts.

While static global variables in a script can be accessed using `ScriptName.var_name`,
instance variables cannot be accessed except by the script they belong to.

Exported Variables
^^^^^^^^^^^^^^^^^^
Script-scope variables can be annotated with some export-related :ref:`annotations<annotations>`.
Exported variables can have their initial value modified in the Editor when setting
up the script, similarly to `InitD[]`.

If an exported variable has an initializer, the initializer value will be used as a default
value, used automatically if no value is specified in the editor. A button in the editor
also allows "resetting to default" an exported variable's value.

A value in [square brackets] indicates an 'optional' value.

.. table::
	:widths: auto
	
	+----------------------+-------------------------------------------------------+-------------------------------------------------+
	| Annotation Name      | Values                                                | Purpose                                         |
	+======================+=======================================================+=================================================+
	| `@Export`            | | Name: String (256)                                  | Exports the specified variable to be modifiable |
	|                      | | Help Text: [String (65535)]                         | Sets the label, helptext, and input field       |
	|                      | | Type: ["D", "H", "LD", "LH", "B", "-1"]             | type of the textbox for the exported variable.  |
	|                      |                                                       |                                                 |
	|                      |                                                       | Name must be specified, help text               |
	|                      |                                                       | defaults to blank, and type defaults based on   |
	|                      |                                                       | variable type, as:                              |
	|                      |                                                       |                                                 |
	|                      |                                                       | | `int`, `float`, `untyped` -> "D"              |
	|                      |                                                       | | `long` -> "LD"                                |
	|                      |                                                       | | `bool` -> "B"                                 |
	|                      |                                                       | | `rgb` -> "LH"                                 |
	|                      |                                                       | | else -> "-1"                                  |
	+----------------------+-------------------------------------------------------+-------------------------------------------------+
	| `@ExportRange`       | | Min: int                                            | Sets a range for the exported variable. The     |
	|                      | | Max: int                                            | field in the editor will not allow setting its  |
	|                      |                                                       | value outside of the specified range.           |
	|                      |                                                       |                                                 |
	|                      |                                                       | | If used on a non-exported variable, errors.   |
	|                      |                                                       | | If used on a boolean variable, errors.        |
	+----------------------+-------------------------------------------------------+-------------------------------------------------+
	| `@ExportEngineValue` | String:                                               | Sets the exported value to use a special GUI.   |
	|                      | ["Tile", "Tile CSet", "Combo", "Combo CSet", "Color", | For example, "Tile", "Combo", and "Color" will  |
	|                      | "Item", "Enemy", "Counter", "Sprite Data", "SFX",     | use a Tile / Combo / Color selector instead of  |
	|                      | "MIDI", "Music", "Save Menu", "Message String",       | a number entry field. Most special values use   |
	|                      | "Weapon Type", "LWeapon Type", "EWeapon Type",        | Drop Down Lists, limiting the selection to      |
	|                      | "Dropset", "Font", "Bottle Type", "Combo Type",       | specific values based on the type.              |
	|                      | "Combo Flag"]                                         |                                                 |
	|                      |                                                       | Some of these special modes require multiple    |
	|                      |                                                       | variables in a row; for example, 'Tile CSet'    |
	|                      |                                                       | is only usable on a variable directly after     |
	|                      |                                                       | a variable exported with 'Tile' mode (causing   |
	|                      |                                                       | the two variables to share a gui widget)        |
	+----------------------+-------------------------------------------------------+-------------------------------------------------+

Static Functions
----------------
Functions declared inside scripts work very similarly to variables; an 'instance function'
can access all instance variables of the script, but any functions declared as `static`
will be unable to access instance variables from the script. Static variables can still
be accessed as normal.

The compiler option :ref:`DEFAULT_STATIC_SCRIPT_MEMBERS<zslang_options>` affects script functions
in almost the exact same way it affects variables; the main difference is that the
`void run()` function will not be made static by it.

Marking a `void run()` function as `static` is an error.

Script Annotations
------------------

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
	| `@ScriptInfo`                           | String (65535)  | Sets an info / summary / description of the   |
	|                                         |                 | script, shown in the editor UI.               |
	+-----------------------------------------+-----------------+-----------------------------------------------+
	| `@ScriptSetup`                          | String (65535)  | Sets a 'setup instructions' string, shown in  |
	|                                         |                 | the editor UI.                                |
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
