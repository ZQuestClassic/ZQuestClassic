# ZScript constants

## Style attributes

These are the attributes that can be set with `Tango_SetStyleAttribute()`.
Attributes associated with specific backdrop types are excluded here;
they're listed with the relevant `TANGO_BACKDROP` constants in the
[backdrop types](#backdrop-types) section.

TANGO_STYLE_BACKDROP_TYPE
:   The type of backdrop to display behind the text; this should be
    [one of the `TANGO_BACKDROP` constants below](#backdrop-types). There are
    also several more attributes associated with each backdrop type.

TANGO_STYLE_TEXT_FONT
:   The default font used to render the text. This must be a Tango font
    definition, not a ZC font.

TANGO_STYLE_TEXT_X
TANGO_STYLE_TEXT_Y
:   The position of the text. These are offsets from the values set with
    `Tango_SetSlotPosition()`.

TANGO_STYLE_TEXT_WIDTH
TANGO_STYLE_TEXT_HEIGHT
:   The size of the area in which text will be displayed.

TANGO_STYLE_TEXT_CSET
TANGO_STYLE_TEXT_COLOR
:   The default color of the text. `TANGO_STYLE_TEXT_COLOR` is used to select
    color 0-15 in the given CSet; this only applies to characters in
    built-in fonts.

TANGO_STYLE_TEXT_SPEED
:   The delay before each character is printed. A speed of 0 means there is
    no delay between characters; this is similar to `TANGO_FLAG_INSTANTANEOUS`,
    but differs in that the speed can be changed mid-text. The flag requires
    less processing, so it should be preferred whenever possible.

TANGO_STYLE_TEXT_SFX
:   The default sound played when printing a character.

TANGO_STYLE_TEXT_END_SFX
:   The sound played when the end of the text is reached.

TANGO_STYLE_MORE_COMBO
TANGO_STYLE_MORE_CSET
:   The appearance of the "more..." icon.

TANGO_STYLE_MORE_X
TANGO_STYLE_MORE_Y
:   The position of the "more..." icon. These are offsets from the values
    set with `Tango_SetSlotPosition()`.

TANGO_STYLE_FLAGS
:   Flags controlling various aspects of the text slot's appearance and
    behavior. These are [`TANGO_FLAG` constants](#style-flags), which can be
    ORed together.

TANGO_STYLE_ALT_CSET_1
TANGO_STYLE_ALT_CSET_2
TANGO_STYLE_ALT_CSET_3
TANGO_STYLE_ALT_CSET_4
TANGO_STYLE_ALT_COLOR_1
TANGO_STYLE_ALT_COLOR_2
TANGO_STYLE_ALT_COLOR_3
TANGO_STYLE_ALT_COLOR_4
:   Set highlight colors to use for double-bracketed text.
    
    1 is `(( ))`
    
    2 is `[[ ]]`
    
    3 is `{{ }}`
    
    4 is `<< >>`

TANGO_STYLE_ALT_OFFSET_1
TANGO_STYLE_ALT_OFFSET_2
TANGO_STYLE_ALT_OFFSET_3
TANGO_STYLE_ALT_OFFSET_4
:   Similar to the above, this sets a tile offset for double-bracketed text.
    This adds a number to the base tile of custom and extended fonts; this is
    applied in addition to the color change. It's meant for 8-bit tiles,
    as those can't simply be recolored.

TANGO_STYLE_MENU_CHOICE_INDENT
:   How much extra space to add before each menu choice to make room for
    the cursor, in pixels.

TANGO_STYLE_MENU_CURSOR_COMBO
TANGO_STYLE_MENU_CURSOR_CSET
TANGO_STYLE_MENU_CURSOR_WIDTH
TANGO_STYLE_MENU_CURSOR_HEIGHT
:   The appearance of the menu cursor. Width and height are in tiles.

TANGO_STYLE_MENU_CURSOR_MOVE_SFX
TANGO_STYLE_MENU_SELECT_SFX
TANGO_STYLE_MENU_CANCEL_SFX
:   The sounds to play in the menu.

TANGO_STYLE_MENU_SCROLL_UP_COMBO
TANGO_STYLE_MENU_SCROLL_UP_CSET
TANGO_STYLE_MENU_SCROLL_UP_X
TANGO_STYLE_MENU_SCROLL_UP_Y
TANGO_STYLE_MENU_SCROLL_DOWN_COMBO
TANGO_STYLE_MENU_SCROLL_DOWN_CSET
TANGO_STYLE_MENU_SCROLL_DOWN_X
TANGO_STYLE_MENU_SCROLL_DOWN_Y
:   If a menu is too large to fit in the text area, these combos will be drawn
    to indicate that it can scroll up or down.

## Style flags

Constants to be used with `TANGO_STYLE` flags. These can be combined
with bitwise OR.

TANGO_FLAG_ENABLE_SPEEDUP
:   The text can be sped up by holding down A.

TANGO_FLAG_ENABLE_SUPER_SPEED
:   The text can be printed instantly by pressing B.

TANGO_FLAG_AUTO_ADVANCE
:   The player will not be prompted to press A when the text finishes;
    it will advance or disappear automatically.

TANGO_FLAG_CARRY_OVER
:   The slot will not be deactivated when the screen changes.

TANGO_FLAG_BLOCK_INPUT
:   While the slot is active, all keys will be unpressed.
    
    !!! caution
        Unpressing keys is handled by `Tango_Update1()`. FFC scripts and
        anything earlier in the global script will see the unmodified input.

TANGO_FLAG_LINE_BY_LINE
:   Text is printed one line at a time instead of one character at a time.

TANGO_FLAG_INSTANTANEOUS
:   The text is printed all at once, regardless of the nominal speed.
    `@while()` still runs only once per frame and holds up further processing
    until its condition is false. `@delay()` and `@waituntil()` still work.

TANGO_FLAG_PERSISTENT
:   When the text is finished printing, the player is not prompted
    to press A and the text does not disappear.

TANGO_FLAG_CENTERED
:   Text is centered in the printable area.

TANGO_FLAG_FREEZE_SCREEN
:   The game is frozen while the slot is active.
    
    !!! note 
        In order for this flag to work, you must implement the functions
        `__Tango_FreezeScreen()` and `__Tango_UnfreezeScreen()`.


## Backdrop types

These are the different backdrop types used with `TANGO_STYLE_BACKDROP_TYPE`
and related attributes used to configure them.

TANGO_BACKDROP_CLEAR
:   No backdrop.

TANGO_BACKDROP_COLOR
TANGO_BACKDROP_COLOR_TRANS
:   A rectangle of a single color, either opaque or transparent.
    
    Related attributes:
    
    TANGO_STYLE_TEXT_CSET
    TANGO_STYLE_TEXT_COLOR
    :   The rectangle's CSet and color. The color is 0-15 within the CSet.
    
    TANGO_STYLE_BACKDROP_WIDTH
    TANGO_STYLE_BACKDROP_HEIGHT
    :   The size of the rectangle in pixels.

TANGO_BACKDROP_TILE
TANGO_BACKDROP_TILE_TRANS
:   A block of tiles, either opaque or transparent.
    
    Related attributes:
    
    TANGO_STYLE_BACKDROP_TILE
    :   The top-left tile of the block.
    
    TANGO_STYLE_BACKDROP_CSET
    :   The CSet of the backdrop.
    
    TANGO_STYLE_BACKDROP_WIDTH
    TANGO_STYLE_BACKDROP_HEIGHT
    :   The size of the backdrop in tiles.

TANGO_BACKDROP_COMBO
TANGO_BACKDROP_COMBO_TRANS
:   A block of tiles drawn from a combo, either opaque or transparent.
    
    Related attributes:
    
    TANGO_STYLE_BACKDROP_COMBO
    :   The combo using the top-left tile of the block.
    
    TANGO_STYLE_BACKDROP_CSET
    :   The CSet of the backdrop.
    
    TANGO_STYLE_BACKDROP_WIDTH
    TANGO_STYLE_BACKDROP_HEIGHT
    :   The size of the backdrop in tiles.

TANGO_BACKDROP_COMPLEX
:   A backdrop made up of multiple drawing commands.
    
    Related attributes:
    
    TANGO_STYLE_BACKDROP_DATA
    :   The array of data from which to draw the backdrop. For details, see
        [the complex backdrops section](dataFormats.md#complex-backdrops).

## Menu flags

These should be used as arguments to `Tango_SetMenuFlags()`.

TANGO_MENU_CAN_CANCEL
:   The user can cancel out of the menu instead of making a selection.

TANGO_MENU_PERSISTENT
:   The menu is persistent, meaning it will not close when a selection is made.
    It will still close if the user cancels.
