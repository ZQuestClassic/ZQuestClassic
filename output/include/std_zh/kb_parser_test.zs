import "std.zh"
//rev 8
//the cursor adjustment is not fast-enough to keep up with rapidly typed characters, or the timing is incorrect. 
//move the cursor djustments into the functions that insert char into the buffer! -Z (25th Dec, 2017 @ 15:30GMT)

const int SFX_KEYPRESS = 58;			

const int LINKTILEOFFSET = -261; const int KEY_DELAY = 6; //Trying this as 6.5 and timer changes as 1.0, using 65 and 10
const int LINE_LENGTH = 54; const int BUFFER_LENGTH = 55;
//int b1[55]; int b2[55]; int b3[55]; int b4[55]; int b5[55];
//int b6[55]; int b7[55]; int b8[55]; int b9[55]; int b10[55];
//int bufptr;

//Very fast typing can lock the timer in such a way that CursorAdvance() stops working, and the last typed char 
//overwrites the previous typed char. Why? I have no clue. -Z (25th Dec, 2017)
//Specifically, ram[ram_q] stops advancing. 
//This seems to have been an issue with function calls not setting ram[ram_q]++ fast enough???

const int CHAR_WIDTH = 4;
const int CURSOR_WIDTH = 4;
const int CURSOR_POINT_CHAR = 62;
const int CURSOR_Y_OFFSET = 4;
const int CURSOR_X_OFFSET = 4;
const int BUFFER_OUTPUT_X = 0;
const int BUFFER_OUTPUT_Y = 0;
const int CURSOR_BLINK_DUR = 70;
const int CURSOR_Y_OFS = 1;
const int TAB_WIDTH = 4; //number of spaces in a tab keypress. 
		//! The actual 'CHAR_TAB' shows as '^' in some fonts, hence doing it this way.

int ram[64]; const int RAM_q = 0; const int RAM_lastchar = 1; const int RAM_cursorspacing = 2; const int RAM_cursorposition = 3;


int buffer[214747]; //={CURSOR_POINT_CHAR, CHAR_SPACE}; 
int bufferoverlay[214747]; //handles the cursor.
bool ShiftKey;//deprecated
//int curbuffer = 0;
int q; int timer = KEY_DELAY; bool cantype = true;

int temp_q; //the current index of the buffer string that we are modifying. 
int cursorblink = 0;

int command_buffer[256];


//CursorAdv(RAM_q);
void CursorAdv(int n){
	TraceS("CursorAdvance: ram[RAM_cursorposition] is IMMEDIATE: "); Trace(ram[RAM_cursorposition]);
	if ( ram[RAM_cursorposition] == SizeOfArray(ram) ) return;
	bufferoverlay[ram[n]] = CHAR_SPACE;
	ram[n]++; //the current index of the cursor. 
	//bufferoverlay[ram[n]] = '_'; //Cursor.
	ram[RAM_cursorposition] += ram[RAM_cursorspacing];
	TraceS("CursorAdvance: ram[RAM_cursorposition] is AFTER: "); Trace(ram[RAM_cursorposition]);
	
}

//CursorRev(RAM_q);
void CursorRev(int n){
	TraceS("CursorReverse: ram[RAM_cursorposition] is IMMEDIATE: "); Trace(ram[RAM_cursorposition]);
	//if ( ram[RAM_cursorposition] == 0 ) return;
	bufferoverlay[ram[n]] = 0;
	ram[n]--;
	if ( ram[RAM_cursorposition] >= ram[RAM_cursorspacing] ) ram[RAM_cursorposition] -= ram[RAM_cursorspacing];
	if ( ram[RAM_cursorposition] < 0 ) ram[RAM_cursorposition] = 0;
	TraceS("CursorReverse: ram[RAM_cursorposition] is AFTER: "); Trace(ram[RAM_cursorposition]);
	//bufferoverlay[ram[n]] = '_'; //Cursor.
}

void ResetCursor(){
	//bufferoverlay[ram[]] = 0;
	ram[RAM_q] = 0;
	ram[RAM_cursorposition] = 0;
	ram[RAM_cursorspacing] = 0;
}

//Put char into buffer for normal keypresses:
void NormalKeyPresses()
{
	int k; 
	int LegalKeys[]= {
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
	KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_0, KEY_1, KEY_2, KEY_3,
	KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0_PAD, KEY_1_PAD, KEY_2_PAD, KEY_3_PAD, KEY_4_PAD, KEY_5_PAD,
	KEY_6_PAD, KEY_7_PAD, KEY_8_PAD, KEY_9_PAD, KEY_TILDE, KEY_MINUS, KEY_EQUALS, KEY_OPENBRACE, KEY_CLOSEBRACE,
	KEY_COLON, KEY_QUOTE, KEY_BACKSLASH, KEY_BACKSLASH2, KEY_COMMA, KEY_SEMICOLON, KEY_SLASH, KEY_SPACE, KEY_SLASH_PAD,
	KEY_ASTERISK, KEY_MINUS_PAD, KEY_PLUS_PAD, KEY_CIRCUMFLEX, KEY_COLON2, KEY_EQUALS_PAD, KEY_STOP };

	int sz = SizeOfArray(LegalKeys);
	
	for ( int kk = 0; kk < sz; kk++ )
	{
		k = LegalKeys[kk];
		if ( Input->Key[k] )
		{
			if ( ram[RAM_q] < 214746 ) 
			{
				TraceS("temp_q is: "); Trace(temp_q);
				TraceS("ram[RAM_q] is: "); Trace(ram[RAM_q]);
				buffer[ram[RAM_q]] = KeyToChar(k,( Input->Key[KEY_LSHIFT] || Input->Key[KEY_RSHIFT] || Input->Key[KEY_CAPSLOCK] ),true);
				TraceS("character is: "); Trace( buffer[ram[RAM_q]] );
				TraceS("key press registered: "); Trace(k);
				//CursorAdv(RAM_q);
				temp_q++;
				ram[RAM_q]++; //the current index of the cursor. 
				TraceS("CursorAdvance: ram[RAM_cursorposition] is IMMEDIATE: "); Trace(ram[RAM_cursorposition]);
	
				bufferoverlay[ram[RAM_q]] = CHAR_SPACE;
	
				ram[RAM_cursorposition] += ram[RAM_cursorspacing];
				TraceS("CursorAdvance: ram[RAM_cursorposition] is AFTER: "); Trace(ram[RAM_cursorposition]);
				Game->PlaySound(SFX_KEYPRESS);
				timer = KEY_DELAY; 
			}
		}
		
	}
}


global script a{
	void run(){
		
		

//int buffers[]={b1, b2, b3, b4, b5, b6, b7, b8, b9, b10};
	//bufptr = buffers;
    //int curbuffer = bufptr[ram[RAM_q]];
		Link->Y = 90;
		//Game->Cheat = 4;
		while(true){
			if ( PressControl() && Input->Key[KEY_I] ) Game->TypingMode = true;
			
			if ( timer > 0 ) cantype = false;
			if ( timer <= 0 ) cantype = true;
			if ( timer > 0 ) timer --;
			if ( timer <= 0 ) timer = 0;
			if ( cursorblink > 0 ) cursorblink--;
			if ( cursorblink <= 0 ) cursorblink = 100;
			if ( cantype && Game->TypingMode ) {
				if ( !PressControl() )
				{
					
					
					NormalKeyPresses();
					
					if ( Input->Key[KEY_BACKSPACE] ){ //backspace key
					//Do not permit backspace is we reach the first character of the buffer!
						if ( ram[RAM_q] < 214746 && ram[RAM_q] > 0 ) {
							TraceS("Prior to reversing cursor");
							TraceS("temp_q is: "); Trace(temp_q);
							TraceS("ram[RAM_q] is: "); Trace(ram[RAM_q]);
							CursorRev(RAM_q); temp_q--; buffer[ram[RAM_q]] = 0; //clear the last character typed.
							TraceS("Reversed cursor");
							TraceS("temp_q is: "); Trace(temp_q);
							TraceS("ram[RAM_q] is: "); Trace(ram[RAM_q]);
							Game->PlaySound(SFX_KEYPRESS);
							timer = KEY_DELAY;
						}
					}   
					
					if ( ram[RAM_q] < 214746 ) 
					{
					//tilde -- this might have issues unless we can set uncap false.
					
						//Special actions for non-character keys. 
							

						if ( Input->Key[KEY_TAB] ){ //tab
							for ( int w = 0; w < TAB_WIDTH; w++ )
							{
								buffer[ram[RAM_q]] = CHAR_SPACE; 
								CursorAdv(RAM_q); temp_q++; 
							}
							Game->PlaySound(SFX_KEYPRESS);
							timer = KEY_DELAY;
						}
						
						//Enter key goes here.
						if ( Input->Key[KEY_ENTER] || Input->Key[KEY_ENTER_PAD] ){ //enqueue
							int ps;
							while ( ps < 256 )
							{
								if ( buffer[ps] == 0 ) break;
								command_buffer[ps] = buffer[ps];
								++ps;
							}
							
							Game->PlaySound(SFX_KEYPRESS); 
							timer = KEY_DELAY;
							TraceS(command_buffer); //after this is in the command buffer, 
										//we will copy the to the STACK
										//and clear this buffer. 
										//Sending text to this buffer should 
										//temporarily block further KB input
										//setting (cantype) false. 
										// ( 25th Dec, 2017 " 15:45GMT )
						}
					
						
					
					
						if ( Input->Key[KEY_DEL] ){ //delete
							for ( int w = ram[RAM_q]; w >= 0; w-- ) buffer[w] = 0;
							Game->PlaySound(SFX_KEYPRESS);
							ResetCursor(); temp_q = 0;
							timer = KEY_DELAY; 
						}
					
						if ( Input->Key[KEY_LEFT] ){ //leftarrow
							//ram[RAM_q]--; 
							
							TraceS("Prior to reversing cursor");
							TraceS("temp_q is: "); Trace(temp_q);
							TraceS("ram[RAM_q] is: "); Trace(ram[RAM_q]);
							
							CursorRev(RAM_q);
							
							TraceS("Reversed cursor");
							TraceS("temp_q is: "); Trace(temp_q);
							TraceS("ram[RAM_q] is: "); Trace(ram[RAM_q]);
							Game->PlaySound(SFX_KEYPRESS);//temp_q--;
							timer = KEY_DELAY; 
						}
						if ( Input->Key[KEY_RIGHT] ){ //leftarrow
							CursorAdv(RAM_q);
							//ram[RAM_q]++;
							Game->PlaySound(SFX_KEYPRESS);//temp_q--; //needs to operate on temp_q so that the highest position is recorded.
							timer = KEY_DELAY; 
						}
					
						if ( Input->Key[KEY_UP] ){ //up
							//find the previous command in the list
							/* if ( ram[RAM_q] < 214746 ) {
								if ( PressShift() ){
								{	buffer[ram[RAM_q]] = '|'; q++; temp_q++; if ( SFX_KEYPRESS > 0 ) Game->PlaySound(SFX_KEYPRESS);}
								else { buffer[ram[RAM_q]] = CHAR_BSLASH; q++; temp_q++; if ( SFX_KEYPRESS > 0 ) Game->PlaySound(SFX_KEYPRESS);}
								timer = KEY_DELAY; 
							}
							*/
						}
						if ( Input->Key[KEY_DOWN] ){ //up
							//find the next command in the list, clear if none
							/* if ( ram[RAM_q] < 214746 ) {
								if ( PressShift() ){
								{	buffer[ram[RAM_q]] = '|'; q++; temp_q++; if ( SFX_KEYPRESS > 0 ) Game->PlaySound(SFX_KEYPRESS);}
								else { buffer[ram[RAM_q]] = CHAR_BSLASH; q++; temp_q++; if ( SFX_KEYPRESS > 0 ) Game->PlaySound(SFX_KEYPRESS);}
								timer = KEY_DELAY; 
							}
							*/
						}
					
						if ( Input->Key[KEY_ENTER_PAD] ){
							for ( int w = q; w >= 0; w-- ) buffer[w] = 0;
							Game->PlaySound(SFX_KEYPRESS);
							q = 0; temp_q = 0;
							timer = KEY_DELAY; 
						}
					
					
						//for ( int qqq = 0; qqram[RAM_q] < SizeOfArray(bufferoverlay); qqq++ ) bufferoverlay[ram[RAM_q]] = CHAR_SPACE;
						//bufferoverlay[ram[RAM_q]] = '_'; //Cursor.
					}
				}
			}
			
			//Adjust the cursor position based on the size of the last typed character.
			if ( ram[RAM_q] > 0 ) {
				ram[RAM_lastchar] = buffer[ram[RAM_q]-1];
				ram[RAM_cursorspacing] = GetCursorSpacing(ram[RAM_lastchar]);
			}
			//If we backspace all the way, rfeser the cursor position to 0 in all respects.
			if ( ram[RAM_q] == 0 ) {
				ram[RAM_lastchar] = 0;
				ram[RAM_cursorspacing] = 0;
				
			}
			
			/*
			if ( timer == KEY_DELAY ) {
				Trace(ram[RAM_lastchar]);
				Trace(GetCursorSpacing(ram[RAM_lastchar]));
				Trace(ram[RAM_cursorspacing]);
				Trace(ram[RAM_cursorposition]);
			}
			*/
				
			if ( Game->TypingMode ) 
			{
				Screen->DrawString(6,0,0,2, 0x01, -1, 0, GetBuffer(),128);
	
				if ( cursorblink > CURSOR_BLINK_DUR ) {
					int cursor[] = "_";
					Screen->DrawString(6,ram[RAM_cursorposition]+ram[RAM_cursorspacing],0+CURSOR_Y_OFS,2, 0x01, -1, 0, cursor,128); //cursor
					//Screen->DrawString(6,0,0+CURSOR_Y_OFS,2, 0x01, -1, 0, GetCursorOverlay(),128); //cursor
					/*
					Screen->Line(6, 
						0+0+(q*CHAR_WIDTH), 
						//BUFFER_OUTPUT_X+CURSOR_X_OFFSET+(q*CHAR_WIDTH), 
						//BUFFER_OUTPUT_Y+CURSOR_Y_OFFSET,
						6,
						0+0+(q*CHAR_WIDTH)+4,
						//BUFFER_OUTPUT_X+CURSOR_X_OFFSET+(q*CHAR_WIDTH) + CURSOR_WIDTH,
						6, 
						//BUFFER_OUTPUT_Y+CURSOR_Y_OFFSET,
						0x01, 1, 0, 0, 0, 128);
					*/
				}	
				if ( PressControl() && Input->Key[KEY_C] ) { Game->TypingMode = false; }
				//if ( Input->Key[KEY_F6] ) 
				//{
				//	Input->Key[KEY_F6] = false; //works. 
				//	TraceS("Pressed F6");
				//}
			}
			else
			{
				Screen->DrawString(6,0,0,2, 0x01, -1, 0, "Press Ctl+I to insert text, Ctl+C to break",128);
			}
			if ( Input->Key[KEY_F6] ) TraceS("Pressed F6");
			Waitdraw(); Waitframe();
		}
	}
}	

int GetCursorSpacing(int chr, int font){
	if ( font == 0 ) {
		int spacing[]={ 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 16, //tab
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //(29)
			0, 0, // (31)
			4, 2, 4, 6, 6, 4, 5, 2, 3, 3, 
			4, 4, 3, 4, 2, 0, 4, 3, 4, 4, //literal 3
			4, 4, 4, 4, 4, 4, 2, 2, 4, 4, //equals
			4, 4, 5, //@ sign
			//A 
			5, 5, 4, 5, 4, 4, 5, 4, 2, 4, 
			//K
			5, 4, 6, 6, 5, 5, 5, 5, 5, 4, 
			//U
			5, 6, 6, 6, 4, 4, //Z
			//lbrace
			4, 5, 4, 4, 4, 4, 
			//a
			5, 5, 4, 5, 4, 4, 5, 4, 2, 4, 
			//k
			5, 4, 6, 6, 5, 5, 5, 5, 5, 4, 
			//u
			5, 6, 6, 4, 4, 4, //z
			//{
			4, 2, 4, 5, 
			//127
			0, 
			//Forward Del
			0 
		};
		return spacing[chr];
	}
}

int GetCursorSpacing(int chr){
	//Custom kerning... Organised by English-language, and C-Programming linguistic frequency for short-circuiting,
	 
	if ( chr == ' ' ) return 4;
	
	if ( chr == 'E' ) return 4; if ( chr == 'T' ) return 4; if ( chr == 'A' ) return 5; if ( chr == 'O' ) return 5; if ( chr == 'I' ) return 2;
	if ( chr == 'N' ) return 6; if ( chr == 'S' ) return 5; if ( chr == 'H' ) return 4; if ( chr == 'R' ) return 5; if ( chr == 'D' ) return 5; 
	if ( chr == 'L' ) return 4; if ( chr == 'C' ) return 4; if ( chr == 'U' ) return 5; if ( chr == 'M' ) return 6; if ( chr == 'W' ) return 6;
	if ( chr == 'F' ) return 4; if ( chr == 'G' ) return 5; if ( chr == 'Y' ) return 4; if ( chr == 'P' ) return 5; if ( chr == 'B' ) return 5;  
	if ( chr == 'V' ) return 6; if ( chr == 'K' ) return 5; if ( chr == 'J' ) return 4; if ( chr == 'X' ) return 6; if ( chr == 'Q' ) return 5;
	if ( chr == 'Z' ) return 4;
	
	if ( chr == 'e' ) return 4; if ( chr == 't' ) return 4; if ( chr == 'a' ) return 5; if ( chr == 'o' ) return 5; if ( chr == 'i' ) return 2;
	if ( chr == 'n' ) return 6; if ( chr == 's' ) return 5; if ( chr == 'h' ) return 4; if ( chr == 'r' ) return 5; if ( chr == 'd' ) return 5; 
	if ( chr == 'l' ) return 4; if ( chr == 'c' ) return 4; if ( chr == 'u' ) return 5; if ( chr == 'm' ) return 6; if ( chr == 'w' ) return 6;
	if ( chr == 'f' ) return 4; if ( chr == 'g' ) return 5; if ( chr == 'y' ) return 4; if ( chr == 'p' ) return 5; if ( chr == 'b' ) return 5; 
	if ( chr == 'v' ) return 6; if ( chr == 'k' ) return 5; if ( chr == 'j' ) return 4; if ( chr == 'x' ) return 4; if ( chr == 'q' ) return 5; 
	if ( chr == 'z' ) return 4;
	
	if ( chr == '.' ) return 2; if ( chr == '!' ) return 2; if ( chr == ',' ) return 3; if ( chr == '?' ) return 4; if ( chr == ':' ) return 2;
	
	if ( chr == '1' ) return 3; if ( chr == '2' ) return 4; if ( chr == '3' ) return 4; if ( chr == '4' ) return 4; if ( chr == '5' ) return 4;
	if ( chr == '6' ) return 4; if ( chr == '7' ) return 4; if ( chr == '8' ) return 4; if ( chr == '9' ) return 4; if ( chr == '0' ) return 4;
	
	if ( chr == ';' ) return 2; if ( chr == '(' ) return 3; if ( chr == ')' ) return 3; if ( chr == '-' ) return 4; 
	
	if ( chr == CHAR_QUOTE ) return 2; //Zeelda to be a parser bug. Comparing the buffer character ' ' ' to 'A' returned 5.
	if ( chr == CHAR_DQUOTE ) return 4; 
	
	if ( chr == '{' ) return 4; if ( chr == '}' ) return 4; if ( chr == '&' ) return 5; if ( chr == '|' ) return 2; if ( chr == '_' ) return 4;
	if ( chr == '=' ) return 4; if ( chr == '*' ) return 4; if ( chr == '+' ) return 4; if ( chr == '/' ) return 4; if ( chr == '%' ) return 4;
	if ( chr == '<' ) return 4; if ( chr == '>' ) return 4; if ( chr == '[' ) return 4; if ( chr == ']' ) return 4; if ( chr == '~' ) return 5;
	 
	
	if ( chr == '@' ) return 5; if ( chr == '#' ) return 6; if ( chr == '$' ) return 6; if ( chr == CHAR_POUNDS ) return 5;  if ( chr == '^' ) return 4;
	  
	if ( chr == CHAR_BSLASH ) return 5; if ( chr == '`' ) return 4; 
	
}
	
int GetBuffer(){ 
    //int ptr = bufptr;
    //return bufptr[buffer];
    return buffer;
    
}	

int GetCursorOverlay(){
	return bufferoverlay;
}