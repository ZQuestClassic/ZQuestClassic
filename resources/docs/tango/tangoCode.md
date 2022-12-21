# Tango code variables and functions

## Variables

@a0, @a1
:   Read/write
    
    Use these to access the slot's `a[0]` and `a[1]`.

@x, @y
:   Read/write
    
    The slot's position on the screen.

@cset, @color
:   Read/write
    
    The slot's current text color. `@color` refers to color 0-15 within
    the CSet and only applies to built-in font characters.

@speed
:   Read/write
    
    The current text speed.

@sfx
:   Read/write
    
    The current text sound.

@style
:   Read/write
    
    The slot's current style. You should only write to this at
    the beginning of a string, before any text is printed; changing
    styles mid-text may cause problems.

@next
:   Read/write
    
    The next message to be shown after the current one finishes.

@chosen
:   Read-only
    
    The last option chosen in a menu. This is 0 if a menu is currently active,
    if no menu has been displayed yet, or if the player canceled in
    the last menu.

@default
:   Read-only
    
    This is a pseudo-variable used to reset style variables to the default.
    There are only four valid uses:
    
    `@set(@cset @default)`
    
    `@set(@color @default)`
    
    `@set(@speed @default)`
    
    `@set(@sfx @default)`
    
    The result of any other use is undefined.


## Setter functions

@set(a1 a2)
:   `a1`: Writable variable
    
    `a2`: Number or condition
    
    Sets the value of a variable.

@inc(a1 a2)
:   `a1`: Writable variable
    
    `a2`: Number or condition
    
    Adds a value to a variable.


## Control functions

These functions affect the text display in ways not covered by other functions.

@append(a1)
:   `a1`: Number
    
    Appends message `a1` to the end of the current text.

@delay(a1)
:   `a1`: Number
    
    Suspends processing of the slot for `a1` frames. If the text is sped up
    (by holding down A), the delay will be reduced accordingly.

@pressa()
:   Prompts the player to press the the advance button (A by default)
    before continuing.

@suspend()
:   Suspends the current slot until a script calls `Tango_ResumeSlot()`.

@next(a1)
:   `a1`: Number

    Sets message `a1` to follow the current text. Equivalent to `@set(@next a1)`.

@goto(a1)
:   `a1`: Number
    
    Immediately terminates the current text and loads message `a1`.

@tab(a1)
:   `a1`: Number
    
    Indents the text to `a1` pixels. This may be left of the current position,
    causing the following text to be printed over earlier text.
    
    The behavior of this function with centered text is currently undefined.

@close()
:   Immediately deactivates the current slot.

@sync(a1)
@sync(a1 a2)
:   `a1`: Number
    
    `a2`: Number
    
    This can be used to synchronize multiple text slots or to synchronize
    a slot and script.
    
    The first string to call `@sync()` with one argument is suspended.
    When `@sync()` is called with the same argument from a second
    slot, both slots resume processing. For example, when two slots are
    active simultaneously and both call `@sync(5)`, the first one to reach
    that function will stop and wait for the other to catch up.
    
    If two arguments are used, any slots waiting for `a1` will be resumed,
    but the current slot will not stop if no other is waiting for it. It will
    treat the second argument the same way it would a single one. This is
    almost the same as calling `@sync(a1)@sync(a2)`, except the first call
    would never hang. This is convenient for synchronizing multiple strings,
    such as if two characters are talking to each other. Think of it this way:
    `@sync(5 6)` means "tell the other string I've reached point 5, then wait
    for it to reach point 6."
    
    `Tango_Sync()` works in much the same way. A call to `Tango_Sync()`
    will not return until a string calls `@sync()` with the same value.
    Do not count on this coordination being precisely timed, as there may be
    a discrepancy of a frame or two. Also, `Tango_Sync()` cannot be used
    on its own to synchronize multiple scripts; there must be at least
    one call to `@sync()` involved.

## Numeric functions

@add(a1 a2)
:   `a1`: Number
    
    `a2`: Number
    
    Returns a1+a2

@rand(a1 a2)
:   `a1`: Number

    `a2`: Number
    
    Returns a random integer between `a1` and `a2`, inclusive.


## Text functions

These functions are replaced with text when they run.

@char(a1)
:   `a1`: Number
    
    Inserts the character with the given number. This is similar to
    inserting character codes; `@26` and `@char(26)` are essentially the same.
    The main difference is that `@char()` allows variables and functions
    to be used. `@char(@a0)` is legal; `@(@a0)` is not.

@number(a1)
:   `a1`: Number
    
    Inserts a number as text.

@ordinal(a1)
:   `a1`: Number
    
    Inserts a number as text followed by "st", "nd", "rd", or "th".

@savename()
:   Inserts the save file name.

@string(a1)
:   `a1`: Number (ZScript string pointer)
    
    Inserts an arbitrary ZScript string. Code in this string will not
    be evaluated. The maximum length of the string is determined by
    the constant `__TANGO_MAX_STRING_FUNC_LENGTH`.


## Flow functions

@while(a1 a2)
:   `a1`: Condition or number
    
    `a2`: Setter or effect function
    
    Executes `a2` as long as `a1` is nonzero. Runs once per frame while
    the condition is true.

@if(a1 a2)
@elseif(a1 a2)
@else(a2)
:   `a1`: Condition or number
    
    `a2`: Setter or effect function
    
    Execute `a2` if `a1` is nonzero.

@waituntil(a1)
:   `a1`: Condition or number function
    
    Supends processing of the slot until `a1` is nonzero.


## Condition functions

These functions should be used as the first argument to `@if()`, `@elseif()`,
`@while()`, and `@waituntil()`, but they can also be used as numeric functions.
They return 1 if the condition is true and 0 if it's false.

@equal(a1 a2)
:   `a1`: Number or condition
    
    `a2`: Number or condition
    
    True if `a1` and `a2` are equal.

@notequal(a1 a2)
:   `a1`: Number or condition
    
    `a2`: Number or condition
    
    True if `a1` and `a2` are not equal.

@greater(a1 a2)
:   `a1`: Number
    
    `a2`: Number
    
    True if `a1` is greater than `a2`.

@less(a1 a2)
:   `a1`: Number
    
    `a2`: Number
    
    True if `a1` is less than `a2`.

@and(a1 a2)
:   `a1`: Condition or number
    
    `a2`: Condition or number
    
    True if `a1` and `a2` are both nonzero.

@or(a1 a2)
:   `a1`: Condition or number
    
    `a2`: Condition or number
    
    True if at least one of `a1` and `a2` is nonzero.

@not(a1)
:   `a1`: Condition or number
    
    True if `a1` is 0, false otherwise.


## Menu functions

@choice(a1)
:   `a1`: number
    
    Creates a menu option with value `a1`. `a1` should be greater than 0.
    The maximum number of choices is determined by the constant
    `__TANGO_MAX_MENU_ITEMS`.

@domenu()
@domenu(a1)
@domenu(a1 a2)
:   `a1`: Number

    `a2`: Number
    
    Activates the menu and suspends further processing until a selection
    is made.
    
    For argument `a1`, add the following values to enable the corresponding
    feature or use 0 to disable both.
    
    **1**: The player can cancel instead of making a selection
    
    **2**: The menu is persistent (doesn't close after a selection is made)
    
    These correspond to the [ZScript `MENU` constants](constants.md#menu-flags).
    
    !!! note
        A persistent menu is only useful when a script is running and
        watching for each selection.
    
    `a2` specifies the initially selected choice, counting from 0. If an invalid
    number is given, no choice will be selected initially; this is sometimes
    useful to prevent the player from making a selection accidentally.

@menuwait()
:   Suspends processing of the slot until a menu selection is made.
    Don't use this together with `@domenu()`; it should be used when
    a script or another string will create the menu.


## Effect functions

These functions perform functions in the game. They do not return useful values.

@playsfx(a1)
:   `a1`: Number
    
    Play a sound.
