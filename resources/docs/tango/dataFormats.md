# Data structures and formats

## Text slots

A text slot represents the state of a string being displayed in much
the same way as an `npc` pointer represents the state of an enemy.
It stores the text, its style, and its position on the screen. A text slot
is said to be "active" if the text is currently being displayed. The data in
a text slot is mostly internal; most of the time, all you'll care about is
its style, its position, and whether it's active.

Text slots have definitions separate from the slots themselves. They are
defined in `__Tango_SlotDefs[]`. The slot definition controls the slot type,
where in the buffer text is stored, and where it's rendered on the offscreen
bitmap.

For more advanced uses, you may want to customize the available text slots.
If you want to display multiple strings at once, you'll need at least one slot
for each string. You may want to set up slots of different types to ensure
one is always available for a certain use when you need it or to ensure that
some strings are never covered up by others.

There are only a couple of properties to set in a slot definition: each slot
has a type and a certain amount of space allocated. The order in which they're
defined is also significant.

A slot's type has no direct impact on how it works; the only thing it affects
is what is returned by `Tango_GetFreeSlot()`. Any difference beyond that comes
down to how you choose to define and use them.

The ordering of slots is significant because they are drawn in order. Slot 0
is drawn first, then slot 1, and so on. That means that if multiple slots
are active at once, higher-numbered slots will appear in front of
lower-numbered ones.

There are two slots and one slot type defined by default. This should be plenty
for most quests, but you can add as many slots and types as you like. Slots are
integers from 0 to `__TANGO_NUM_SLOTS - 1`, and type IDs should be 0 or greater.

To control the number of slots, set `__TANGO_NUM_SLOTS`, set the size of
`__Tango_SlotData[]`, and add definitions to `__Tango_SlotDefs[]`. You may
also want to set the size of `__Tango_Buffer[]` to control the total amount of
space available for text.

Each slot is defined by seven numbers in `__Tango_SlotDefs[]`. These are,
in order:

1. Slot type
2. Starting index in `__Tango_Buffer[]`
3. Maximum length in `__Tango_Buffer[]`
4. X position in offscreen bitmap
5. Y position in offscreen bitmap
6. Width in offscreen bitmap
7. Height in offscreen bitmap

It is permissible for slots to overlap in both the buffer and the bitmap, but
be careful about this; using overlapping slots simultaneously is likely to
cause problems.

## Styles

Before any text can be displayed, the slot must have a style set. The style
defines how the text will be displayed - the font, the backdrop, the sound
effects, and so forth. Style IDs are integers 0 and up.

A style is simply a collection of settings. These are mostly cosmetic, but
there are also a few behavioral flags. These control whether the text can be
sped up by holding a button and whether the screen is frozen while it's
displayed, among other things.

Use `Tango_SetStyleAttribute()` to set each style attribute. You must at least
set the font and color; every other setting has a valid default. See the
[constants](constants.md) page for the available attributes and expected values.
Most of the constants defined by Tango are style-related. Once an attribute
is set, it won't change unless you change it yourself, so it's okay to set up
styles in the `Init` script and never touch them again.

To control the number of styles available, set `__TANGO_NUM_STYLES` and the
size of `__Tango_Styles[]`.

## Complex backdrops

To use a complex backdrop, you must create a properly formatted array
and provide a pointer to it with `TANGO_BACKDROP_DATA`. The array must exist
for as long as the slot is active.

The array must consist of a series of drawing commands followed by
`TANGO_DRAW_END`. A drawing command, in this context, means a `TANGO_DRAW`
constant followed by six numbers. The meanings of these numbers depends on
what is being drawn.

TANGO_DRAW_RECT
TANGO_DRAW_RECT_TRANS
:   A plain rectangle, either opaque or transparent.
    
    Data:
    
    1. CSet
    2. Color (0-15 within the CSet)
    3. X position
    4. Y position
    5. Width in pixels
    6. Height in pixels

TANGO_DRAW_TILE
TANGO_DRAW_TILE_TRANS
:   A block of tiles, either opaque or transparent.
    
    Data:
    
    1. Starting tile
    2. CSet
    3. X position
    4. Y position
    5. Width in tiles
    6. Height in tiles

TANGO_DRAW_COMBO
TANGO_DRAW_COMBO_TRANS
:   A block of tiles drawn from a combo, either opaque or transparent.
    
    Data:
    
    1. Starting combo
    2. CSet
    3. X position
    4. Y position
    5. Width in tiles
    6. Height in tiles

TANGO_DRAW_TEXT
:   A string of text. This is a ZScript string, which must exist as long as
    the slot is active. The font must be a Tango font, not a built-in one.
    If you want to center the text, you can use `Tango_GetStringWidth()`
    to find its width.
    
    Data:
    
    1. String pointer
    2. Font
    3. CSet
    4. Color (0-15 within the CSet)
    5. X position
    6. Y position

As an example, a backdrop consisting of a transparent rectangle with a block
of tiles over it might look like this:

```cpp
int sampleBackdrop[]= {
    TANGO_DRAW_RECT_TRANS,
        0,       // CSet
        4,       // Color
        4, 5,    // X, Y (pixels)
        184, 72, // Width, height (pixels)
    TANGO_DRAW_TILE,
        400,   // Tile
        5,     // CSet
        0, 0,  // X, Y (pixels)
        12, 5, // Width, height (tiles)
    TANGO_DRAW_END
};
```

## Fonts

Tango can't use ZC's built-in fonts directly; it needs additional data about
spacing to position everything correctly. Tango font definitions are arrays
that provide the necessary information.

Definitions for most built-in fonts are included, but they're not imported
by default. The files are in the tango/font directory. Although they aren't
constants (since ZScript doesn't allow constant arrays), the names are
written in all caps to indicate that they should be used as though they were.

You can create your own fonts, too. Using characters made from tiles, you can
extend built-in fonts with additional characters or create completely original
ones. The drawback of tile-based characters is that they have fewer color
options; they only have a CSet, not a CSet and a color. On the other hand,
tile-based characters can use multiple colors, and they can even be 8-bit.

A font definition is an array consisting of the following data, in order:

1. Font type
    
    This must be one of:
    
    * `TANGO_FONT_BUILTIN`
    * `TANGO_FONT_EXTENDED`
    * `TANGO_FONT_CUSTOM`

2. Width type
    
    This must be one of:
    
    * `TANGO_FONT_MONOSPACE`
    * `TANGO_FONT_PROPORTIONAL`

3. Font ID
    
    For built-in and extended fonts, this is the value of the built-in font;
    use one of std.zh's `FONT` constants. Custom fonts ignore this value.

4. Tile start
    
    This is the tile used by the first extended character (256) in extended
    fonts or the first printable character (33, '!') in custom fonts. Built-in
    fonts ignore this value.

5. Character height in pixels

    In tile-based fonts, this is the distance from the top of a tile to the
    bottom of the lowest character. Built-in fonts often include some space
    above each character; in that case, this is the distance from the top of
    that space to the bottom of the lowest character.

6. Space between lines in pixels

    This is the space from the bottom of one line to the top of the next,
    not the total line height.

7. (and up) Character widths in pixels
    
    The width of each character, including any trailing space. In monospaced
    fonts, this is a single value. For proportional fonts, this is a list of
    widths for each character starting from space (32). Characters 32-126
    follow ASCII order; characters 127 and up are arbitrary and vary by font.

If you want to create an extended font, it's easiest to copy one of
the included built-in font definitions, change `TANGO_FONT_BUILTIN` to
`TANGO_FONT_EXTENDED`, then add the starting tile and extra character widths.
Be aware that many built-in fonts leave some space above the glyphs. Extended
characters will need to account for this.


## Identifier conversion

`Tango_ConvertFunctionName()` and `Tango_ConvertVariableName()` will convert
names to numbers for you. But if you want to do it yourself, here's how.
Start by converting each character in the name to a number:

<pre>
a  b  c ... x  y  z  0  1  2  ... 7  8  9
1  2  3 ... 24 25 26 27 28 29 ... 34 35 36
</pre>

Multiply each number by 7<sup>n</sup> according to its position, add them up, and
divide by 10000.

For example, for a function called @dostuff2:

<pre>
d   4   x7 x7 x7 x7 x7 x7 x7 = 3294172 +
o   15  x7 x7 x7 x7 x7 x7    = 1764735 +
s   19  x7 x7 x7 x7 x7       =  319333 +
t   20  x7 x7 x7 x7          =   48020 +
u   21  x7 x7 x7             =    7203 +
f   6   x7 x7                =     294 +
f   6   x7                   =      42 +
2   29                       =      29
                               -------
                               5433828 / 10000 = 543.3828
</pre>

Note that this may change in future versions.
