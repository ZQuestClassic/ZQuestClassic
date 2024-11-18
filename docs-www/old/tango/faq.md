# FAQ

## Why isn't my script working correctly?

Whenever something doesn't work, you should look at allegro.log to see
if any errors have been reported. You'll get more helpful messages using
full logging, which is enabled by importing loggingFull.zh instead of
loggingMinimal.zh. These are found near the bottom of the main file.

Some common errors:

* Using more styles than alloted
    * Fixed by increasing `__TANGO_NUM_STYLES` and the size of `__Tango_Styles[]`

* Using more menu options than alloted
    * Fixed by increasing `__TANGO_MAX_MENU_ITEMS` and the size of `__Tango_Data[]`

* Incorrect constants for custom functions
    * Use `Tango_ConvertFunctionName()` to make sure you've got them right

You can also try calling the function `Tango_ValidateConfiguration()`. This will
analyze your settings and report any errors it finds in allegro.log.

## How do I display a string?

1. Find a free text slot (`Tango_GetFreeSlot()`)
2. Clear all data in the slot (`Tango_ClearSlot()`)
3. Load text into the slot (`Tango_LoadString()` or `Tango_LoadMessage()`)
4. Set a style (`Tango_SetSlotStyle()`)
5. Set the position on the screen (`Tango_SetSlotPosition()`)
6. Activate the text slot (`Tango_ActivateSlot()`)

`Tango_LoadString()` loads a ZScript string (an `int` array);
`Tango_LoadMessage()` loads a ZC message (**Quest > Strings**). All built-in
function names follow this naming convention.

You'll probably want to combine these into a single function for simplicity.
tangoQuickStart.zh provides examples in the `ShowString()` and `ShowMessage()`
functions.

## What is a text slot?


Text slots are where strings are stored for processing and display. They store
the text, the style, and the position on the screen, as well as some internal
data. Before any text can be shown on the screen, it must be loaded into
one of the available slots.

Slot IDs are integers counting up from 0. Most often, you'll want to find
a free slot automatically, but you can also use hardcoded slot numbers.
Each slot has its own definition, which determines how long a string can be and
how much space is available for drawing it.

## What is a style?

A style determines the appearance and behavior of displayed text. For instance,
the font, the backdrop, and whether the text can be sped up by holding A
are all controlled by the style.

Style IDs are integers counting up from 0. Every text slot using the same style
will have the same appearance and behavior.

##How many text slots do I need?

You need at least as many slots as will ever be active at once. If you want
fifty strings onscreen at once, you need fifty slots. If you only want one
string active at a time, just one slot will suffice.

For more advanced uses, you may want more slots than are strictly necessary.
For instance, you might want to reserve certain slots for specific purposes,
even if they'll never all be in use at once.

## How do I add more text slots?

Set `__TANGO_NUM_SLOTS` to the number of slots you want and change the size of
`__Tango_Buffer[]` so that it's large enough to hold the text for all of them.

The slot definitions go in `__Tango_SlotDefs[]`. The first seven numbers define
slot 0, the next seven define slot 1, and so on. In order, these numbers are
the slot type, the starting index and size in `__Tango_Buffer[]`, the X and Y
coordinates on the bitmap, and the width and height on the bitmap. For example:

```cpp
int __Tango_SlotDefs[] = {
    // Slot 0
    TANGO_SLOT_NORMAL, // The slot type
    0, 512,   // Occupies __Tango_Buffer[0] to __Tango_Buffer[511]
    0, 0,     // Draws to a region at 0, 0 on the bitmap
    192, 128, // Drawing region is 192x128 pixels
    
    // Slot 1
    TANGO_SLOT_SMALL,
    512, 64, // Occupies __Tango_Buffer[512] to __Tango_Buffer[575]
    0, 128,  // Draws to a region at 0, 128 (just below slot 0)
    128, 32  // Drawing region is 128x32
    
    // Slot 2
    TANGO_SLOT_SMALL,
    576, 64, // Occupies __Tango_Buffer[576] to __Tango_Buffer[639]
    0, 160,  // Draws to a region at 0, 160 (just below slot 1)
    128, 32  // Drawing region is 128x32
};
```

The slot type is arbitrary. It can be any non-negative number. The default
slot type, `TANGO_SLOT_NORMAL`, can safely be renamed or deleted.

Generally, you shouldn't let slots overlap in the buffer or the bitmap.
If they won't be in use at the same time, however, it won't cause problems.
If you use a very large number of slots, it may even be necessary for them
to share some space on the bitmap.

## What is the purpose of slot types?

A slot's type makes no difference to how it's handled internally. The only
thing it actually affects is what's returned by `Tango_GetFreeSlot()`. Slot
types exist for users to create their own distictions. There are three reasons
you might use different slot types:

1. Reservation
    
    In order to display a string, you must first have a free slot. If no slots
    are free, you can't show any text without clearing one out. If there's
    something you need to be able to display, you can create a dedicated slot
    type for it, so there will always be a slot free when you need it.
    
    For instance, suppose you want NPCs to spout random lines of dialog in the
    background. You might define several slots of type `SLOT_BG` to use for this
    purpose, and one of type `SLOT_FG` for the text box when Link talks to
    someone. This way, you can have the NPCs use as many `SLOT_BG` slots as they
    like, and the main `SLOT_FG` will always be available when you need it.

2. Drawing order
    
    Text slots are drawn to the screen in order from first to last. If you want
    certain strings to be drawn above or below others, you can enforce this by
    using different slot types.
    
    Going back to the same example, you wouldn't want your NPCs' random lines
    to appear on top of the main text box. If slots 0-4 are `SLOT_BG` and
    slot 5 is `SLOT_FG`, you can be sure the most important text will always
    be on top.

3. Efficient use of space
    
    If you plan to have one or two long strings appear alongside a lot of
    short ones, you can use different slot types to avoid wasting space
    in `__Tango_Buffer[]`. Because `__Tango_Buffer[]` is a global array,
    it is part of the quest's save data. Making it larger than necessary
    causes useless data to be saved and loaded, making both take a little
    longer (though only slightly). More importantly, there's limited space
    for drawing on the offscreen bitmap. If you have ten slots all using
    192x128 pixels, they'll have to overlap, potentially causing graphical
    errors.
    
    If your random NPC dialog will only ever be a line or two, you can give
    `SLOT_BG` a small amount of space in the buffer and bitmap, while `SLOT_FG`
    might have room for several hundred characters and space to draw a large
    text box.

Ordinarily, it is expected that all slots of the same type have the same amount
of space allocated in the buffer and on the bitmap, but this is not enforced.
If you want to make some `SLOT_BG`s small and some large, you can do that, but
it's up to you to ensure they're used correctly.

## How do I create a style?

With a series of calls to `Tango_SetStyleAttribute()`. Each call will set
a single attribute of the style. At a minimum, you must set a font and color
for the text. See [the constants page](constants.md) for the available settings.
Check out tangoQuickStart.zh for an example.

If you want to unload a style after it's been set up, use `Tango_ClearStyle()`.
This is only necessary if you want to reuse an ID for a different style,
which is rarely done.

## When should I set up a style?

Any time before it's actually used is fine. The most common method is to set up
all of your styles at the beginning of the active global script. It's also okay
to do it in the `Init` script. Styles will stay set up until you clear them or
change them.

Another approach is to set a style up immediately before displaying a string
that uses it; this may be preferable when using a large number of different
styles.

A combination of those two methods is also possible. If you want multiple
styles that are mostly the same, you might set the common attributes
in advance and set the distinct ones on demand.

Changing a style while it's in use is permitted, but changes to the text color
and font will not affect any text that has already been printed.

## Why isn't the screen freeze style flag working?

There are two functions in the main file, `__Tango_FreezeScreen()` and
`__Tango_UnfreezeScreen()`, that are blank by default. In order for the
screen freeze style flag to work, you must implement these functions.

The reason that they're blank is that there isn't a reasonable default
implementation; they're too dependent on the quest. Grabbing arbitrary FFCs
or changing combo definitions in an unknown quest might cause serious problems.

The recommended method is to set two unused FFCs to combos of type
**Screen Freeze (Except FFCs)** and **Screen Freeze (FFCs Only)**. Be aware,
however, that if an FFC script triggers this, that script will freeze, too.
If you're using ghost.zh, `SuspendGhostZHScripts()` and `ResumeGhostZHScripts()`
should be used as well. Other headers may have similar suspend functions that
should also be included.
