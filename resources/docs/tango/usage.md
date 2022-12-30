# Usage

## Basic usage

There are three functions, listed listed [here](zscriptFuncs.md#global-script),
that need to be added to the active global script, or you can use the provided
`TangoActiveScript`.

Displaying a string involves working with two data structures: styles and
text slots. Styles determines how strings look and behave. Text slots are
where the text and processing state are stored. To display a string, you must
set up a style, load the text into a text slot, apply the style to the slot,
and then activate it.

All it takes to set up a style is a call to `Tango_SetStyleAttribute()` for
each attribute you want to set. Most attributes are optional, but you must
at least set a font and text color. The regular `FONT` constants from std.zh
can't be used; you'll need to import one of the font definitions in the
`tango/font` folder and use the `TANGO_FONT` constant defined there.

Once a style is set up, displaying text requires six steps:

1. Find a free text slot (`Tango_GetFreeSlot()`)
2. Clear any data already in the slot (`Tango_ClearSlot()`)
3. Load text into the slot (`Tango_LoadString()` or `Tango_LoadMessage()`)
4. Set a style (`Tango_SetSlotStyle()`)
5. Set the position (`Tango_SetSlotPosition()`)
6. Activate the text slot (`Tango_ActivateSlot()`)

If you want to try it out without learning all the details, or if you'd
just like a simple, straightforward example, check out tangoQuickStart.zh.
It provides a few simple functions and a minimal global script. There are
a few constants to set for colors, combos, and sounds; the defaults are
set up for the classic tileset.

For more information on text slots and styles, see the page on [data structures
and formats](dataFormats.md).

## Tango code

Tango implements a simple scripting language with functions embedded
in strings. Code is identified by the character `@` (by default;
set `__TANGO_CODE_DELIMITER` to change this).

`@` is followed immediately by a function name, then up to two arguments
in parentheses, separated by spaces. Parentheses are needed even if
a function takes no arguments. For example, the string
`"@playsfx(64)Hello, @savename()!"` would play sound 64 and print (possibly)
`"Hello, Link!"`.

Variables are also available. These can be used to set the properties of
the text. For instance, `@speed` controls how quickly the text is drawn.
Each slot has two variables, `@a0` and `@a1`, used to store arbitrary data or
communicate with ZScript. You can also create your own variables for
the same purposes.

It's also possible to use `@##` or `@(##)` to insert a character directly into
the text. The string `"@90@69@76@68@65"` is equivalent to `"ZELDA"` This is
useful for inserting line breaks (`@26`) and font characters beyond the
ASCII range. Also, `@0` can be used to terminate the text early; anything after
`@0` won't be printed.

Finally, it's possible to highlight text using double brackets. For example,
in the string `You got the ((Wooden Sword))!`, the text "Wooden Sword" would
be printed in a different color than the rest of the text. `(( ))`, `[[ ]]`,
`<< >>`, and `{{ }}` each have style settings allowing them to produce
different colors. This feature can be disabled by setting
`__TANGO_ENABLE_BRACKET_HIGHLIGHT` to 0.


## Menus

Tango code can be used to set up simple menus in strings using a few simple
function calls. It's also necessary to set the `TANGO_STYLE_MENU_CURSOR`
style attributes so the cursor will be drawn correctly.

To create a menu, add a call to `@choice()` for each choice followed by the
corresponding text. Each call to `@choice()` needs an argument - a number that
will be the value of that choice. The choices should be followed by a call to
`@domenu()`, which will run until the user makes a selection or cancels.
Afterward, the value of the selected choice will be stored in `@chosen`, or 0
if the user cancelled.

For example:
`"Pick one!@26@choice(1)Option A@26@choice(2)Option B@domenu()@26@if(@equal(@chosen 1) @append(2))@else(@append(3))"`

`@26` represents a line break; their use here indicates that a new line should
be started before each option and after the user picks an option. This menu
will look like this:

    Pick one!
    > Option A
      Option B

It will be followed by message 2 if the option A is selected or message 3 if
option B is selected.

Using ZScript functions, a menu can be created with cursor stops at arbitrary
locations on the screen. Creating a menu this way takes four or five steps:

1. Initialize menu data (`Tango_InitializeMenu()`)
2. Add choices (`Tango_AddChoice()`)
3. Set the cursor's appearance (`Tango_SetMenuCursor()`)
4. (optional) Set the menu sound effects (`Tango_SetMenuSFX()`)
5. Activate the menu (`Tango_ActivateMenu()`)

!!! note
    Tango only draws and moves the cursor. If you wish to display any other
    graphics, you'll have to draw them yourself. You may wish to do this
    in a `while(Tango_MenuIsActive())` loop.

## Extending Tango code

It's possible to create your own variables and functions to use in Tango code.

The name of the new function or variable must be a series of lower-case
letters and numbers, and the first character must be a letter. There's no
set limit on length, but the name must convert to a number small enough
to be represented in ZScript (no greater than 214747.9999).

The name of the function or variable needs to be converted to a number.
`Tango_ConvertFunctionName()` and `Tango_ConvertVariableName()` will do this
for you; they'll even write the result to allegro.log in a form suitable
to copy and paste directly into a script. The conversion process is described
[here](dataFormats.md#identifier-conversion).

User-defined functions can be numeric, effect, or condition functions.
To make Tango handle a function, add the implementation to
`__Tango_RunCustomFunction()`. The `function` argument is the function ID.
`args` is an array containing the arguments. A function can take up to four
arguments; any unused arguments will be 0. For debugging purposes, you may
want to call `Tango_LogUndefinedFunction()` if the function is not recognized.

A simple example:

```cpp
const float FUNC_DOSTUFF2 = 543.3828; // @dostuff2()

float __Tango_RunCustomFunction(float function, float args)
{
    if(function==FUNC_DOSTUFF2)
    {
        // This will just return the sum of the arguments.
        return args[0]+args[1];
    }
    else
    {
        // Unknown function
        Tango_LogUndefinedFunction(function);
    }
    
    return 0;
}
```

To implement a variable, use `Tango_GetCustomVar()` and `Tango_SetCustomVar()`.
The `var` argument is the variable id.

A trivial example of a custom variable:

```cpp
int myVar;
const float VAR_MYVAR = 4.0891; // @myvar

float Tango_GetCustomVar(float var)
{
    if(var==VAR_MYVAR)
        return myVar;
    else // Unknown
    {
        Tango_LogInvalidVariable(var);
        return 0;
    }
}

void Tango_SetCustomVar(float var, float value)
{
    if(var==VAR_MYVAR)
        myVar=value;
    else // Unknown
        Tango_LogInvalidVariable(var);
}
```

Note that it's possible, although very unlikely, for two function or variable
names to convert to the same number. In that case, you'll just have to rename
one of them. It's okay for a variable and a function to have the same name.

## String control codes

String control codes are supported only in ZC messages, not ZScript strings.
They're enabled by default, but they can be disabled if you don't need them.
To disable them, import stringControlCodeDisabled.zh instead of
stringControlCode.zh.

Control codes can be used alongside Tango code, but they cannot be combined.
Something like `\20\@rand(61, 63)` or `@if(@a0 \16\6)` will not work.

Note that codes 16 and 17 (add or remove item) don't work quite the same
as they normally do. The item is simply added to or removed from the inventory;
no changes are made to any counters.

