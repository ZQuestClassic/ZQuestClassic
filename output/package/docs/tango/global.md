# Global script functions

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
    since one normally wants message windows drawn on top of everything else.

bool Tango_ValidateConfiguration()
:   This function will check the arrays and text slot definitions for errors
    and print a report to allegro.log. It can't catch all possible errors,
    but it should get the most common ones. Returns true if no errors were found.
    
    Note that not all potential problems are necessarily invalid; some, such as
    overlapping text slots, may be okay. These will be logged, but not
    considered errors.
    
    This function should not be used in published quests. It only exists to
    help you configure tango.zh correctly.
