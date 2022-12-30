# ZScript functions

## Global script

void Tango_Start()
:   Clears the menu and all text slots.
    
    Call this in the active global script before the main loop.
 
void Tango_Update1()
:   Processes each text slot and the menu.
    
    After calling this function, the render target will be set to `RT_SCREEN`.
    
    Call this in the active global script's main loop before `Waitdraw()`.
    It is recommended that it be called before any other functions so that
    the `TANGO_FLAG_BLOCK_INPUT` can be handled as soon as possible.

void Tango_Update2()
:   Draws all active text slots to the screen.
    
    After calling this function, the render target will be set to `RT_SCREEN`.
    
    Call this in the active global script's main loop after `Waitdraw()`.
    This should generally come after everything else that does any drawing,
    since you probably want text boxes drawn on top of everything else.

bool Tango_ValidateConfiguration()
:   This function will check the arrays and text slot definitions for errors
    and print a report to allegro.log. It can't catch all possible errors,
    but it should get the most common ones. Returns true if no errors were found.
    
    Note that not all potential problems are necessarily invalid; some, such as
    overlapping text slots, may be okay. These will be logged, but not
    considered errors.
    
    This function should not be used in published quests. It only exists to
    help you configure tango.zh correctly.

## Text slot management

int Tango_GetFreeSlot()
int Tango_GetFreeSlot(int slotType)
:   Get the number of a slot not currently in use. You can specify a slot type;
    if you don't care, pass in `TANGO_SLOT_ANY` or leave out the argument.

void Tango_ClearSlot(int slot)
:   Clear all data for the given slot. If the slot is active, it will be
    deactivated.

bool Tango_ReserveSlot(int slot)
:   Mark a slot as reserved, preventing `Tango_GetFreeSlot()` from returning it.
    Use this if you get a free slot and don't plan to use it immediately.
    Returns true if the reservation succeeded (i.e. the slot wasn't already
    reserved or active).
    
    The reserved state will be cleared if `Tango_ClearSlot()` is called or Link
    moves to a new screen.

void Tango_LoadString(int slot, int string[])
:   Loads a ZScript string (`int[]`) into the given slot.

void Tango_LoadMessage(int slot, int messageID)
:   Loads a ZC message (**Quest->Strings**) into the given slot.

void Tango_AppendString(int slot, int string[])
void Tango_AppendMessage(int slot, int messageID)
:   Loads a string or message into the given slot, appending it to the end of
    whatever text is already there.

void Tango_LoadString(int slot, int string[], int startChar)
void Tango_LoadMessage(int slot, int messageID, int startChar)
void Tango_AppendString(int slot, int string[], int startChar)
void Tango_AppendMessage(int slot, int messageID, int startChar)
:   Like the above, but specifies a starting point in the string to load.
    
    Loading will start from the first character after `startChar`. For instance:
    
        int str[]="abcd%efg";
        Tango_LoadString(0, str, '%');
    
    will load "efg" into slot 0.
    
    If the character specified is not in the string, the entire string
    will be loaded. The string must contain the actual character, not
    a character code (@##).

void Tango_SetSlotStyle(int slot, int style)
int Tango_GetSlotStyle(int slot)
:   Set or get the style used by the given slot.

void Tango_SetSlotPosition(int slot, int x, int y)
int Tango_GetSlotX(int slot)
int Tango_GetSlotY(int slot)
:   Set or get the position where the text slot will be shown on the screen.

void Tango_ActivateSlot(int slot)
:   Display the current slot on the screen. Do this only after setting
    the slot's text, style, and position. The render target will be set to
    `RT_SCREEN` after calling this function.

bool Tango_SlotIsActive(int slot)
:   Returns true if the slot is currently being displayed on the screen.
    A suspended slot is still considered active.

bool Tango_AnySlotIsActive()
:   Returns true if any slot is currently being displayed.

bool Tango_SlotIsFinished(int slot)
:   Returns true if the given slot is finished printing and is still
    being displayed.


## Style setup

void Tango_ClearStyle(int style)
:   Clear all data for the given style.

void Tango_SetStyleAttribute(int style, int attribute, int value)
int Tango_GetStyleAttribute(int style, int attribute)
:   Set or get an attribute of the given style. See [the style attributes
    section](constants.md#style-attributes) for the available attributes
    and expected values.
    
    !!! warning
        Modifying a style in use by an active text slot
        is not guaranteed to work correctly.

int Tango_GetSlotStyleAttribute(int slot, int attribute)
:   Get the value of an attribute from whatever style the given text slot
    is using.

int Tango_GetStringWidth(int str[], int font)
:   Returns the width of the given string when printed in the given font.
    Tango code will not be evaluated and line breaks will be ignored.
    This should be used for centering text when using `TANGO_DRAW_TEXT`
    in a complex backdrop.


## Menu

void Tango_InitializeMenu()
:   Clear all menu data and deactivate the menu if one is active.

void Tango_AddMenuChoice(int value, int x, int y)
:   Add a menu choice with the given value at the specified position.
    The value should be greater than 0.

void Tango_MoveMenuChoice(int choice, int x, int y)
:   Reposition a menu choice on the screen.

void Tango_SetMenuCursor(int combo, int cset)
void Tango_SetMenuCursor(int combo, int cset, int width, int height)
:   Set the appearance of the menu cursor. If width and height are not
    specified, they will default to 1.

void Tango_SetMenuSFX(int moveSound, int selectSound, int cancelSound)
:   Set the sounds to be used by the menu.

void Tango_SetMenuFlags(int flags)
:   Set flags for the menu. The value is 0 or one or more menu flags
    ORed together. See [the menu flags section](constants.md#menu-flags) for
    available flags.

void Tango_ActivateMenu()
:   Activate a menu set up using the above functions.

void Tango_DeactivateMenu()
:   Deactivate the active menu. This is typically only needed for
    persistent menus.

int Tango_GetLastMenuChoice()
:   Returns the value of the last selection made in a menu. Returns 0 if
    a non-persistent menu is currently active, no menu has been shown yet,
    or the user cancelled in the last menu.
    
    If the menu is persistent, this function will only return each selection
    for a single frame before returning to 0.

int Tango_GetCurrentMenuChoice()
:   Returns the value if the currently selected choice in the menu.
    Returns 0 if no menu is active or the current cursor position is invalid.

bool Tango_MenuIsActive()
:   Returns true if a menu is currently active.
    
    !!! caution
        If you display a string that creates a menu, the menu will not
        be active until at least the next call to `Tango_Update1()`, even if
        `TANGO_FLAG_INSTANTANEOUS` is used.
        
        This is not the case for menus created by scripts. The menu will be
        active as soon as `Tango_ActivateMenu()` is called.

void Tango_SaveMenuState(int state[])
:   Store the current state of the menu into output. This will store the
    values and positions of the options, the cursor and sound settings, and
    the currently selected option.
    
    The size of output must be at least `20 + 3 * __TANGO_MAX_MENU_ITEMS`.
    Later versions of tango.zh are likely to use more data; you may
    want to use a larger array for future-proofing.

void Tango_SetMenuAutosaveDest(int state[])
:   Sets an array in which the menu state will automatically be saved
    when a selection is made or the player cancels. The state will not
    be saved if the menu is closed for another reason. After setting
    the autosave array, you can call this again with an argument of `NULL`
    to clear it.
    
    !!! note
        The data to be saved includes the autosave destination, so restoring
        the menu state with this array will set it up to autosave again.

void Tango_RestoreMenuState(int oldState[])
:   Restore the state saved with `Tango_SaveMenuState()`.

int Tango_GetMenuCursorPosition()
:   Returns the current position of the menu cursor. Choices are numbered
    from 0. Setting the cursor position to an invalid number is allowed,
    so the position is not guaranteed to be valid.

void Tango_SetMenuCursorPosition(int pos)
:   Sets the cursor position to pos. If pos is invalid, no item will be
    selected, and the cursor will be hidden.


## Other

void Tango_Sync(int value)
:   Used to synchronize a script and a text slot. See `@sync()` for details.

void Tango_SetSlotVar(int slot, int what, float value)
float Tango_GetSlotVar(int slot, int what)
:   Use these to set and get a text slot's `@a0` and `@a1`. Use either
    `TANGO_VAR_A0` or `TANGO_VAR_A1` as the `what` argument.

void Tango_SuspendSlot(int slot)
:   Suspends processing of an active text slot. The slot will remain visible,
    but it will not advance.

void Tango_ResumeSlot(int slot)
:   Resumes processing of a suspended text slot.

float Tango_GetSlotScrollPos(int slot)
float Tango_GetSlotMaxScrollPos(int slot)
:   If the text is too long to be displayed all at once, it will scroll down.
    These functions get the current and maximum scroll positions. The minimum
    position is always 0, and the maximum is 0 or greater. The scroll amount
    is given in pixels.

void Tango_ScrollSlot(int slot, float amount)
:   Scrolls the text slot up or down. As above, the amount is in pixels.
    Poitive numbers scroll down, and negative numbers scroll up. The slot
    will not scroll beyond the beginning or end of the text.

int Tango_GetCurrentSlot()
:   Returns the ID of the slot currently being processed. This should only
    be used in `__Tango_RunCustomFunction()`, `Tango_GetCustomVar()`, and 
    `Tango_SetCustomVar()`. At any other time, the value is meaningless.

void Tango_SaveSlotState(int slot, int output[], int bitmap)
void Tango_SaveSlotState(int slot, int output[])
:   Store the state of a text slot into aoutputa. The current text, style,
    position, state, and variables will be saved. The style's data
    will not be saved.
    
    If `bitmap` is specified, the text already rendered will be copied to 
    that bitmap, and the render target will be set to `RT_SCREEN` afterward.
    
    The size of `output` must be at least the slot's length in the buffer + 21.
    Later versions of tango.zh are likely to use more data; you may
    want to use a larger array for future-proofing.

void Tango_RestoreSlotState(int slot, int oldState[], int bitmap)
void Tango_RestoreSlotState(int slot, int oldState[])
:   Restore the state of a slot saved with `Tango_SaveSlotState()`. The slot
    number should be the same, as it will be assumed that the position
    on the bitmap is the same.
    
    If a bitmap is specified, the render target will be set to `RT_SCREEN`
    afterward.

float Tango_ConvertFunctionName(int name[])
float Tango_ConvertVariableName(int name[])
:   Returns the converted value of the given name and prints a constant
    definition to allegro.log. Do not include `@` in the name.
    
        int name[] = "dostuff2";
        float value = Tango_ConvertFunctionName(name);
    
    `value` will be equal to 543.3828, and allegro.log will contain
    `const float FUNC_DOSTUFF2 = 543.3828;`.

void Tango_LogUndefinedFunction(float function)
:   This will log an error indicating the use of an unknown function.
    This is meant to be called from `__Tango_RunCustomFunction()` if
    the function is not recognized. It's only useful for debugging,
    so it can be considered optional.

void Tango_LogUndefinedVariable(float var)
:   This will log an error indicating the use of an unknown variable.
    It's meant to be called from `Tango_GetCustomVar()` and `Tango_SetCustomVar()`
    and is useful only for debugging.
