# Error codes

If you use loggingMinimal.zh instead of loggingFull.zh, error messages
will be logged only as ID numbers. These are all the possible error codes:

**101**: Attempted to load an invalid message

**102**: Attempted to load an invalid string

**103**: The text to be loaded would not fit in the buffer

**104**: The text contained an invalid or incomplete function call

**105**: The text contained an invalid function argument

**106**: The text contained an invalid character code

**107**: The message contained an incomplete string control code

**108**: The message contained a string control code, but they're disabled

**201**: The text contained an invalid character

**202**: An undefined function was called (you must call
`Tango_LogUndefinedFunction()` for this error to be reported)

**203**: An undefined variable was referenced (you must call
`Tango_LogUndefinedVariable()` for this error to be reported)

**204**: More than 4 arguments were passed to a function

**205**: An invalid argument was given to `@string()`

**301**: A menu with no choices was activated

**302**: Too many choices were added to a menu

**303**: Tried to move an invalid menu choice

**401**: An invalid style ID was used

**402**: A slot was activated using a style with no font set

**403**: An invalid slot ID was used

**404**: The array passed to `Tango_SaveSlotState()` or `Tango_RestoreSlotState()` was too small

**405**: The array passed to `Tango_SaveMenuState()` or `Tango_RestoreMenuState()` was too small
 
