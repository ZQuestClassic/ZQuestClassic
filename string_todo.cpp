//yes

//1 NINOUT NO RET
void remchr(int string[])
 * Remove all characters starting from pointer 'string'

 1 input
 int xlen(int string[])
 * Returns the length of characters of the (positive) hexadecimal integer pointed by 'string'

int xtoi(int string[])
 * Returns the (positive) hexadecimal integer pointed by 'string'

int ilen(int string[])
 * Returns the length of characters of the decimal integer pointed by 'string'

int atoi(int string[])
 * Returns the decimal integer pointed by 'string'






2 inputs

int strcspn(int str[], int keys[])
 * Returns the length of characters in 'str' before a character contained in
 * 'keys' is found

int strstr(int string[], int sub[])
 * Returns the position of the first occurence of sub-string 'sub' in 'string,
 * or -1 if sub is not found

int xtoa(int string, int num)
 * Places integer 'num' into string 'string' in base 16 without checking for overflow,
 * and returns the number of characters used

int itoa(int string[], int num)
 * Places integer 'num' into string 'string' without checking for overflow,
 * and returns the number of characters used

  int strcat(int dest[], int src[])
 * Appends string 'src' onto string 'dest' (assuming dest has enough extra memory
 * allocated to allow the operation)

   int strspn(int str[], int keys[])
 * Returns the length of characters in 'str' before a character not contained in
 * 'keys' is found

   int strchr(int string[], int character)
 * Returns the position of the first occurence of 'character' in 'string',
 * or -1 if none are found

  int strrchr(int string[], int character)
 * Returns the position of the last occurence of 'character' in 'string'
 * starting from the end, or -1 if none are found

 //OVERLOADS
int xlen(int string, int pos)
 * As xlen(), but with a specified starting position.

int xtoi(int string, int pos)
 * As xtoi(), but with a specified starting position.

 int ilen(int string, int pos)
 * As ilen(), but with a specified starting position.

 int atoi(int string, int pos)
 * As atoi(), but with a specified starting position.

void remnchr(int string[], int n)
 * Remove 'n' characters and shift the rest of the string back to pointer 'string'












3 inputs

int xtoa(int string, int num, bool upper)
 * Converts a hexadecimal value to a string. Set bool upper to force uppercase.

int strcatf(int dest[], int arg, int format)
 * Appends 'arg' onto 'dest' as the MF_ constant passed into 'format'

int itoa(int string, int pos, int num)
 * As itoa(), but with a specified starting position.

int strstr(int string, int pos, int sub)
 * As strstr(), but with a specified starting position.



void remnchr(int string, int pos, int n)
 * As remnchr(), but with a specified starting position



void strcat(int dest, int src, int spos)
 * As strcat(), but with a specified starting position

int strncat(int dest, int src, int n)
 * strcat for the first 'n' characters in src


int strchr(int string, int pos, int chr)
 * As strchr(), but with a specified starting position.



int strrchr(int string, int pos, int chr)
 * As strrchr(), but with a specified starting position.

 int strspn(int str, int pos, int keys)
 * As strspn(), but with a specified starting position.

 int strcspn(int str, int pos, int keys)
 * As strcspn(), but with a specified starting position.

 4 inputs

 void strncat(int dest, int src, int spos, int n)
 * As strncat(), but with a specified starting position.


int strncatf(int dest[], int arg, int format, int n)
 * As strcatf, using only 'n' characters of 'arg'


  * String Complement Span


int xtoa(int string, int pos, int num, bool upper)
 * As xtoa(), but with a specified starting position.



int strcmp(int str1, int pos1, int str2, int pos2)
 * As strcmp(), but with a specified starting position for each string.

 int strncmp(int str1, int pos1, int str2, int pos2, int n)
 * As strncmp(), but with a specified starting position for each string.







//probably
int UpperToLower(int chr)
 * Converts all upper case characters to lower case, leaving non-alphabetic
 * characters unchanged

int LowerToUpper(int chr)
 * Converts all lower case characters to upper case, leaving non-alphabetic
 * characters unchanged

int ConvertCase(int chr)
 * Converts lower case to upper case and upper case to lower case


 //maybe

 void memset(int ptr[], int value, int n)
 * Sets block of memory of size 'n' pointed by 'ptr' to 'value'

void memset(int ptr, int pos, int value, int n)
 * As memset(), but with a specified starting position

int memcpy(int dest[], int src[], int n)
 * Copys block of memory pointed by 'src' of size 'n' to 'dest' and returns
 * 'dest'

void memcpy(int dest, int dpos, int src, int spos, int n)
 * As memcpy(), but with a specified starting position

 int memmove(int dest[], int src[], int n)
 * As memcpy, but uses a buffer so memory space can overlap

 void sprintf(int ret[], int formatstr[], int a0, int a1, int a2,...)
 * Prints string 'formatstr' into 'ret' according to the arguments inputted (see C function for reference),
 * returning the number of characters used. Does not check for overflow in 'ret'
 * Overloaded up to a maximum of 16 arguments. Enter the right number of arguments for your format string;
 * there is (currently) no way to check how many arguments have been entered or of what type they are
 * Currently supported arguments:
 * '%s' - String
 * '%i' - Integer
 * '%f' - Float
 * '%d' - Number (Integer/Float depending on context)
 * '%n' - Nothing
 * '%p' - Pointer Address
 * '%c' - Single character
 * '%x' - Hexadecimal Integer (lower case)
 * '%X' - Hexadecimal Integer (upper case)
 * '\n' places a line feed ASCII character into the string

////////////////////
/// Print Format ///
////////////////////
void printf(int formatstr[], int a0, int a1, int a2,...)
 * Uses a buffer to print the results of sprintf(formatstr,...) straight to allegro.log
 * Overloaded up to a maximum of 16 arguments

bool sprintf_isMFCode(int chr)
 * Used to determine the format from sprintf and printf.


 //probably not -- problematic

 float aton(int string[])
 * Returns the number pointed by 'string', calling either atoi or atof depending on context

int aton(int string, int pos)
 * As aton(), but with a specified starting position.

/////////////////////
/// Number Length ///
/////////////////////
int nlen(int string[])
 * Returns the length of characters of the number pointed by 'string', calling either
 * ilen or flen depending on context

int nlen(int string, int pos)
 * As nlen(), but with a specified starting position.

 //float in ZScript and float in C differ. not sure if this will work.
 float atof(int string[])
 * Returns the floating point number pointed by 'string'

float atof(int string, int pos)
 * As atof(), but with a specified starting position.

////////////////////
/// Float Length ///
////////////////////
int flen(int string[])
 * Returns the length of characters of the floating point number pointed by 'string'

int flen(int string, int pos)
 * As flen(), but with a specified starting position.

 int ftoa(int string[], float num, bool printall)
 * Places float 'num' into string 'string' without checking for overflow,
 * and returns the number of characters used. If 'printall' is true, it will add 4 decimal places
 * regardless of the most significant digit

int ftoa(int string, int pos, float num, bool printall)
 * As ftoa(), but with a specified starting position.

int ftoa(int string, float num)
 * As ftoa(), except that 'printall' is false.

int ftoa(int string, int pos, float num)
 * As ftoa(), except that 'printall' is false, with a specified starting position.


 int ntoa(int string[], float num)
 * Checks whether 'num' is an integer or not, and calls the appropriate function

int ntoa(int string, int pos, float num)
 * As ntoa(), but with a specified starting position.