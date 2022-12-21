//////////////////////////////////
/// Hero onDeath Menu Example  ///
/// v2.0                       ///
/// 8th August, 2020           ///
/// By: ZoriRPG                ///
//////////////////////////////////

// An example Hero onDeath script.
hero script Death
{
	// gameover menu selection types
	enum { quitNONE, quitSAVE, quitRETRY, quitCONTINUE, quitEND, quitSAVERESUME, quitLAST };

	// menu data[] indices
	enum { qmdMENU_STATE, qmdTEXTSCROLL_X, qmdLAST };

	const int SFX_GAMEOVERSCREEN_CURSOR 	= 6;  //Sound to play when the player moves the game over cursor.
	const int SFX_GAMEOVERSCREEN_SELECTION 	= 44; //Sound to play when the user selectsl a game over menu option.
	const int MIDI_GAMEOVER 		= -3; //MIDI to play on Game Over screen.
		//Defaults: -3 is the internal Game Over, -2 is Level 9, -1 is Overworld

	// Other Graphics and Colours
	const int TILE_SAVECURSOR 		= 20;
	const int TILE_SAVECURSOR_CSET 		= 1;    //CSet for minitile cursor. 
	const int BLACK 			= 0x0F; //Black in the current palette
	const int RED	 			= 0x93; //Red in the current palette
	const int WHITE	 			= 0x01; //White in the current palette

	// Menu Object Positions and Settings
	const int DONT_SAVE_X 			= 102-12; //X position of the RETRY string.
	const int SAVE_X 			= 102-12; //X position of the SAVE string.
	const int SAVEANDRESUME_Y 		= 110;  //Y position of the END string.
	const int END_Y 			= 96;  //Y position of the END string.
	const int DONT_CONTINUE_Y 		= 82;  //Y position of the RETRY string.
	const int DONT_SAVE_Y 			= 68;  //Y position of the RETRY string.
	const int SAVE_Y 			= 54;  //X position of the SAVE string.
	const int CURSOR_OFFSET 		= 12;  //X offset of the cursor from the text. 

	const int CLEAR_TINT_FOR_MENU		= 1; //Set to 0 to retain tints.

	//Sets scrolling text on menu screen.
	void set_textscroll(int ptr)
	{
		ptr[qmdTEXTSCROLL_X] = Rand(-128,256);
	}

	/* 
		Draws the menu. THe first pointer *ptr is the script data[] array.
		The second pointer, *scrolling_msg is a message to display that 
		scrolls across the top of the screen
	*/
	int draw(int ptr, char32 scrolling_msg)
	{
		unless(IsValidArray(ptr))
		{
			printf("Invalid script data[] array passed to menuscripts::draw(*ptr,...). Aborting.\n");
			Quit();
		}
		unless(IsValidArray(scrolling_msg))
		{
			scrolling_msg = NULL;
		}
		--ptr[qmdTEXTSCROLL_X];
		if ( ptr[qmdTEXTSCROLL_X] < -128) ptr[qmdTEXTSCROLL_X] = 256;
		
		// Black out the screen. This happens with all choices, so it is outside the switch stmt.
		Screen->Rectangle(6,0,-56,256,256,BLACK,100, 0,0,0,true,128);
		
		//Draw scrolling message if any. This happens with all choices, so it is outside the switch stmt.
		if(scrolling_msg)
			Screen->DrawString(6,ptr[qmdTEXTSCROLL_X],0,FONT_SATURN,0x04, -1,0,scrolling_msg,128);
		
		switch(ptr[qmdMENU_STATE])
		{
			case quitSAVE:
			{
				
				// Draw Cursor
				Screen->FastTile(6,SAVE_X-CURSOR_OFFSET, SAVE_Y-4, TILE_SAVECURSOR, TILE_SAVECURSOR_CSET,128); 
				// Draw strings, red for selected
				Screen->DrawString(6,SAVE_X,SAVE_Y,FONT_SATURN,RED, -1,0,"SAVE AND QUIT",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_SAVE_Y,FONT_SATURN,WHITE, -1,0,"RETRY",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_CONTINUE_Y,FONT_SATURN,WHITE, -1,0,"CONTINUE",128);
				Screen->DrawString(6,DONT_SAVE_X,END_Y,FONT_SATURN,WHITE, -1,0,"END",128);
				Screen->DrawString(6,DONT_SAVE_X,SAVEANDRESUME_Y,FONT_SATURN,WHITE, -1,0,"SAVE",128);
				break;
			}
			case quitRETRY:
			{
				// Draw Cursor
				Screen->FastTile(6,DONT_SAVE_X-CURSOR_OFFSET, DONT_SAVE_Y-4, TILE_SAVECURSOR, TILE_SAVECURSOR_CSET,128); 
				// Draw strings, red for selected
				Screen->DrawString(6,SAVE_X,SAVE_Y,FONT_SATURN,WHITE, -1,0,"SAVE AND QUIT",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_SAVE_Y,FONT_SATURN,RED, -1,0,"RETRY",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_CONTINUE_Y,FONT_SATURN,WHITE, -1,0,"CONTINUE",128);
				Screen->DrawString(6,DONT_SAVE_X,END_Y,FONT_SATURN,WHITE, -1,0,"END",128);
				Screen->DrawString(6,DONT_SAVE_X,SAVEANDRESUME_Y,FONT_SATURN,WHITE, -1,0,"SAVE",128);
				break;
			}
			case quitCONTINUE:
			{
				// Draw cursor
				Screen->FastTile(6,DONT_SAVE_X-CURSOR_OFFSET, DONT_CONTINUE_Y-4, TILE_SAVECURSOR, TILE_SAVECURSOR_CSET,128); 
				// Draw strings, red for selected
				Screen->DrawString(6,SAVE_X,SAVE_Y,FONT_SATURN,WHITE, -1,0,"SAVE AND QUIT",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_SAVE_Y,FONT_SATURN,WHITE, -1,0,"RETRY",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_CONTINUE_Y,FONT_SATURN,RED, -1,0,"CONTINUE",128);
				Screen->DrawString(6,DONT_SAVE_X,END_Y,FONT_SATURN,WHITE, -1,0,"END",128);
				Screen->DrawString(6,DONT_SAVE_X,SAVEANDRESUME_Y,FONT_SATURN,WHITE, -1,0,"SAVE",128);
				break;
			}
			case quitEND:
			{
				// Draw cursor
				Screen->FastTile(6,DONT_SAVE_X-CURSOR_OFFSET, DONT_CONTINUE_Y-4, TILE_SAVECURSOR, TILE_SAVECURSOR_CSET,128); 
				// Draw strings, red for selected
				Screen->DrawString(6,SAVE_X,SAVE_Y,FONT_SATURN,WHITE, -1,0,"SAVE AND QUIT",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_SAVE_Y,FONT_SATURN,WHITE, -1,0,"RETRY",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_CONTINUE_Y,FONT_SATURN,WHITE, -1,0,"CONTINUE",128);
				Screen->DrawString(6,DONT_SAVE_X,END_Y,FONT_SATURN,RED, -1,0,"END",128);
				Screen->DrawString(6,DONT_SAVE_X,SAVEANDRESUME_Y,FONT_SATURN,WHITE, -1,0,"SAVE",128);
				break;
			}
			case quitSAVERESUME:
			{
				// Draw cursor
				Screen->FastTile(6,DONT_SAVE_X-CURSOR_OFFSET, DONT_CONTINUE_Y-4, TILE_SAVECURSOR, TILE_SAVECURSOR_CSET,128); 
				// Draw strings, red for selected
				Screen->DrawString(6,SAVE_X,SAVE_Y,FONT_SATURN,WHITE, -1,0,"SAVE AND QUIT",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_SAVE_Y,FONT_SATURN,WHITE, -1,0,"RETRY",128);
				Screen->DrawString(6,DONT_SAVE_X,DONT_CONTINUE_Y,FONT_SATURN,WHITE, -1,0,"CONTINUE",128);
				Screen->DrawString(6,DONT_SAVE_X,END_Y,FONT_SATURN,WHITE, -1,0,"END",128);
				Screen->DrawString(6,DONT_SAVE_X,SAVEANDRESUME_Y,FONT_SATURN,RED, -1,0,"SAVE",128);
				break;
			}
		}
	}

	/*
		This is the menu handler. It checks button presses, determines the
		correct menu state based on those presses, and affirms pressing START
		back to the script. 

		You must pass the script's data[] array to *ptr.
	*/
	int choice(int ptr)
	{
		unless(IsValidArray(ptr)) // If the user passed an invalid data pointer, error and quit. 
		{
			printf("Invalid script data[] array passed to menuscripts::choice(*ptr). Aborting.\n");
			Quit();
		}
		if ( Hero->PressDown )
		{
			switch(ptr[qmdMENU_STATE])
			{
				case quitSAVE: ptr[qmdMENU_STATE] = quitRETRY; break;
				case quitRETRY: ptr[qmdMENU_STATE] = quitCONTINUE; break;
				case quitCONTINUE: ptr[qmdMENU_STATE] = quitEND; break;
				case quitEND: ptr[qmdMENU_STATE] = quitSAVERESUME; break;
				case quitSAVERESUME: ptr[qmdMENU_STATE] = quitSAVE; break;
				default: break;
			}
			Game->PlaySound(SFX_GAMEOVERSCREEN_CURSOR);
			return quitNONE;
		}
		if ( Hero->PressUp )
		{
			switch(ptr[qmdMENU_STATE])
			{
				case quitSAVE: ptr[qmdMENU_STATE] = quitSAVERESUME; break;
				case quitRETRY: ptr[qmdMENU_STATE] = quitSAVE; break;
				case quitCONTINUE: ptr[qmdMENU_STATE] = quitRETRY; break;
				case quitEND: ptr[qmdMENU_STATE] = quitCONTINUE; break;
				case quitSAVERESUME: ptr[qmdMENU_STATE] = quitEND; break;
				default: break;
			}
			Game->PlaySound(SFX_GAMEOVERSCREEN_CURSOR);
			return quitNONE;
		}
		
		if ( Hero->PressStart )
		{
			Game->PlaySound(SFX_GAMEOVERSCREEN_SELECTION);
			return ptr[qmdMENU_STATE];
		}
		return quitNONE;
	}
	void run()
	{
		int data[qmdLAST];
		data[qmdMENU_STATE] = quitCONTINUE;
		int menuchoice = quitNONE;
		set_textscroll(data);
		
		if ( CLEAR_TINT_FOR_MENU ) Graphics->ClearTint(); 
		
		
		Audio->PlayMIDI(MIDI_GAMEOVER);
		until(menuchoice) // The menu loop, shows the menu until the player makes a selection.
		{
			//    GAME OVER SCREEN
			draw(data, "Git gud!");
			menuchoice = choice(data);
			Waitframe();
		}

		// The player made a selection, and we act on it, here.
		switch(menuchoice)
		{
			case quitSAVE:
			{
				draw(data, "Git gud!"); // Draw before command to ensure that the screen gfx aren't shown.
				Game->SaveAndQuit();
				break;
			}
			case quitSAVERESUME:
			{
				draw(data, "Git gud!");
				Game->Save();
				Game->Reload();
				break;
			}
			case quitRETRY:
			{
				draw(data, "Git gud!");
				Game->Reload();
				break;
			}
			case quitCONTINUE:
			{
				draw(data, "Git gud!");
				Game->Continue();
				break;
			}
			case quitEND:
			{
				draw(data, "Git gud!");
				Game->End();
				break;
			}
			default: break;
		}
	}
}
