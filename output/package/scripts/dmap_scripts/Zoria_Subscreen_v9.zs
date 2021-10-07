//v9
//Updated to dmapdata script type on 29th October, 2019

import "std.zh"

const int SFX_ERROR = 10;

//Debug Settings
const int SUBSCREEN_TEST_MODE = 0;
const int SUBSCREEN_DRAW_TEST = 1;

//Global arrays
int GRAM[214747];
int OwnsItems[256];

//Global array indices:
const int EXITING_SUBSCREEN 	= 500; 
const int SUBSCREEN 		= 501; //THe subscreen pointer.
const int ENABLEITEMS_PTR 	= 502; //The subscreen equipment items (equipable, not inventory) pointer. 
const int SUBSCREEN_OBJECTS 	= 503; 
const int BUTTON_ITEMS_PTR 	= 504;


////////////////////////////////////////
/// Scripted Subscreen Demonstration ///
/// for ZC 2.55 - Beta 40 and Higher ///
/// v0.9, 29-Oct-2019 - By: ZoriaRPG ///
////////////////////////////////////////


//Tile pages 114 to 117
//Global Arrays
int ButtonItems[3]; //Merge into another, if desired. Function pointers will be useful for this. 

//ButtonItems[] array indices:
const int BUTTONITEM_A 		= 0;
const int BUTTONITEM_B	 	= 1;
const int BUTTONITEM_SUBSCREEN 	= 2; /* This determines of the subscreen is active, so that functions that	
					change inventory (e.g. for warping) do not interfere with setting
					the item in the subscreen. */


//Generic Item to manipulate with itemdata
const int I_GENERIC 		= 123;
const int I_SUBSCREEN_BLANK 	= 124; 
const int I_SUBSC_OBJECT 	= 125; //A generic item for placed subscreen objects.

//! Subscreen Settings

//Base
const int SUBSC_FFC_INDEX 	= 32; //The FFC ID that we reserve for the subscreen ffc. 
const int SUBSC_FFC_DATA 	= 882; //THe FFC Data for the Subscreen FFC. (Freeze all except FFCs).

//Visual Effects & Fades
const int SUBSC_FADE_IN 	= 0; 	//0 == None, 1 == ZAPIN, 2 == ZAPOUT, 
const int SUBSC_FADE_OUT 	= 0; 
const int SUBSC_PAN_SPEED 	= 4; //pixels per frame. MUST BE A POWER OF 2!

//Sounds
const int SUBSC_CURSOR_SFX 	= 6;  //Cursor movement. 
const int SUBSC_SELECTED_SFX 	= 44; //Select an item.
const int SUBSC_CHANGEPANE_SFX 	= 44; //Change the pane.
const int SUBSC_OPEN_SFX 	= 37; //Open the subscreen
const int SUBSC_CLOSE_SFX 	= 46; //Close the subscreen.
const int SUBSC_SFX_EQUIP 	= 35; //Sound for items that can be equipped/unequipped. 

//General Settings
const int SUBSC_MAX_ITEMS 	= 24; //How many items are on the subscreen?
const int SUBSC_NUM_ROWS 	= 4; //Items grid forms this many (horizontal) rows
const int SUBSC_NUM_COLUMNS 	= 6; //Items grid forms this many (vertical) columns. 
const int SUBSC_LINK_INVISIBLE = 1; //Is Link invisible when the subscreen is active?
const int SUBSC_LAST 		= 24; //Last item in the array. 

//Item Misc Flags
const int MISC_SUBSC_OBJ 	= 18; //The index if i->Misc[] to use for marking as a subscreen object.
const int IS_SUBSCREEN_OBJECT 	= 1; //! This is used as =, but we should do it bitwise. 
const int IS_INVENTORY_A 	= 2; //Unused, deprecated. 
const int IS_INVENTORY_B 	= 4; //Unused, deprecated.

//Layout
const int SUBSC_INITIAL_X = 	118; //The starting X-position to draw items.
const int SUBSC_INITIAL_Y = 	20; //The starting Y-position to draw items.
const int SUBSC_X_SPACING = 	18; //Pixels between items
const int SUBSC_Y_SPACING = 	18;
const int SUBSC_EQUIP_A_X 	= 39; //X-Position of the current A-Item
const int SUBSC_EQUIP_A_Y 	= -16; //Y-Position of the current A-Item
const int SUBSC_EQUIP_B_X 	= 13; //X-Position of the current B-Item
const int SUBSC_EQUIP_B_Y	= -16; //Y-Position of the current B-Item

//Rows, Columns, Wrapping
const int SUBSC_WRAP_ROW = 		18; //Wrap from the bottom row to the top, or top to the bottom.
const int SUBSC_WRAP_COLUMS = 		6; //Wrap from the left to the right, or riht to the left column.
const int SUBSC_ITEMS_PER_ROW = 	6; //Number of items per row. Used for wrapping. 
const int SUBSC_ITEMS_PER_COLUMN = 	4; //Number of items per column; also used for wrapping. 

//Pane IDs
const int PANE_LEFT 		= 0; //Arbitrary values for each pane of the subscreen.
const int PANE_CENTRE 		= 1;
const int PANE_RIGHT 		= 2; 

//Graphical Settings
const int SUBSC_BG_LAYER 	= 7; //Layer to draw backgrounds.
const int SUBSC_OBJ_LAYER 	= 7; //Layer to draw objects.

//Colours
const int SUBSC_COLOUR_WHITE = 0x01; //Are either of these every used?
const int SUBSC_COLOUR_BLACK = 0x0F; //Possibly in the text drawing routines?

//Bitmap Settings ->To be deprecated, except that we use some of these as sizes for tile draws, and panning! 
const int BG_IS_BITMAP			= -1; /*
						If set to 0 or higher, drawing will use bitmaps, and draw them 
						to layer-N, where N is this value. This feature is deprecated,
						and slated for removal. Bitmaps are fine and dandy, but they
						are also limited in umber at present, and wasting three to
						create a subscreen, where DrawTile is perfectly usable, is silly. 
						*/
const int RT_SUBSCREEN 			= 0;	//Render target ID
const int SUBSC_BITMAP_X 		= 0;	//! We use some of these values for sizing other things. 
const int SUBSC_BITMAP_WIDTH 		= 256;	//! Be very careful when deprecating them. !!
const int SUBSC_BITMAP_Y 		= 0;
const int SUBSC_BITMAP_HEIGHT 		= 232;
const int SUBSC_BITMAP_DRAW_TO_X 	= 0;
const int SUBSC_BITMAP_DRAW_TO_Y 	= -56;
const int SUBSC_BITMAP_DRAW_MODE 	= 0; //0 = Normal, Trans = 1

//Tiles
const int SUBSC_TILE_BLANK		= 5; //A blank tile. 
const int SUBSC_FIRST_TILE 		= 29640; //Main central pane
const int SUBSC_TILE_LPANE 		= 30680; //First tile of the left pane
const int SUBSC_TILE_RPANE 		= 36140; //First tile of the right pane. 

const int SUBSC_NUMBER_OF_TILES 	= 0; //Unused 

const int SUBSC_TILE_INITIAL_X 		= 0; //Upper-left corner X-coordinate
const int SUBSC_TILE_INITIAL_Y 		= -56; //Upper-left corner Y-coordinate. 
const int SUBSC_TILE_LAYER 		= 7; //Layer to draw tiles. Do we not use SUBSC_BG_LAYER?
const int SUBSC_TILE_WIDTH 		= 20; //Width of the screen.
const int SUBSC_TILE_HEIGHT 		= 16; //Height of the screen and passive subscreen.
const int SUBSC_TILE_CSET 		= 11; //Drawing CSet

const int SUBSC_TILE_XSCALE 		= -1; //Scale factors: -1 or none, otherwise N%.
const int SUBSC_TILE_YSCALE 		= -1; 	
const int SUBSC_TILE_RX 		= 0; //Rotation. This is normally going to be 0, but...
const int SUBSC_TILE_RY 		= 0; //..you know...
const int SUBSC_TILE_RANGLE 		= 0; //...
const int SUBSC_TILE_FLIP 		= 0; //Flip the entire image. This may cause undesirable effects, if set. 
const int SUBSC_TILE_OPACITY 		= 128; //Opaque

//Cursor Item Appearance
const int CURSOR_TILE 		= 20140; //The tile, used by the cursor.
const int CURSOR_CSET 		= 6; //Its CSet
const int CURSOR_AFRAMES 	= 15; //Would be nice. 
const int CURSOR_ASPEED 	= 7; //Would also be nice. Requires additional work, and possibly a later beta.
const int CURSOR_LAYER 		= 7; /* Layer onto which the cursor is drawn. This should now automatically
					occur onthe same layer as the other objects, as the last draw.
					*/

//Text
const int SUBSC_TEXT_INV_FONT 		= 2; //z3 Small
const int SUBSC_TEXT_INV_FONT_SIZE 	= -1;
const int SUBSC_TEXT_Y_INVENTORY 	= -48; //Poistions for all the textual subscreen objects.
const int SUBSC_TEXT_X_KEYS1 		= 8;
const int SUBSC_TEXT_X_KEYS2 		= 24;
const int SUBSC_TEXT_X_KEYS3 		= 40;
const int SUBSC_TEXT_X_BOMBS 		= 96;
const int SUBSC_TEXT_X_SBOMBS 		= 112;
const int SUBSC_TEXT_X_ARROWS 		= 144;
const int SUBSC_TEXT_X_LIFE 		= 160;
const int SUBSC_TEXT_X_MAGIC 		= 192;
const int SUBSC_TEXT_X_MONEY 		= 224; 
const int SUBSC_TEXT_LAYER 		= 7;
const int SUBSC_TEXT_INV_FG_COLOUR 	= 0x01; //Foreground colour for text.
const int SUBSC_TEXT_INV_BG_COLOUR 	= 0x0F; //Shadow colour for text.
const int SUBSC_TEXT_BACKDROP 		= -1; //Will draw a solid backgrop behind it if set to 1 or higher. 
const int SUBSC_TEXT_INV_OPACITY 	= 128; //Opaque
const int SUBSC_TEXT_INV_SHADOW_OFFSET 	= 1; //X-Y offset of the shadow effect.
					

//! Subscreen 'q[]' array indices:
//Operating Conditions
const int SUBSC_ACTIVE 		= 0; //Array index of GRAM[]: Is it active?

//1 to 9 reserved for expansion.

//Internal Variables
const int SUBSC_SELECTED 	= 10;	//THe selected item
const int SUBSC_LAST_SELECTED 	= 11;	//Holds the ID of the previous selected item.
					//We do not yet use this, but it could be used later. 
const int SUBSC_POSX 		= 12;	//Holds the X value for where to draw the next object. 
const int SUBSC_POSY 		= 13;	//Holds the Y value for where to draw the next object. 
//14 to 39 reserved for expansion. 

//Loops
const int SUBSC_FOR_LOOP 	= 40;	//The main for loop. Sets the current object ID. 
const int SUBSC_SLOTCHECK_LOOP 	= 41;	//The loop used to check what items are in slots, and assign the initial cursor position.
const int SUBSC_OBJDRAW_LOOP 	= 42;	//The loop used to copy objects to a higher layer.  
const int SUBSC_CLEANUP_LOOP 	= 43;	//The loop used to clean up everything. 
const int SUBSC_OBJ_PAN_DRAW_LOOP = 44; //Loop for drawing objects during the pan-in.
const int SUBSC_EXIT_CLEANUP_LOOP = 45; //Loop for drawing objects during the pan-in.

//Pane Info
const int PANE_ID 		= 100; //Holds the ID of the present Subscreen Pane
const int PANE_LEFT_DRAWN 	= 101; //Registers to store if drawing events have, or should occur. 
const int PANE_RIGHT_DRAWN 	= 103;
const int PANE_CENTRE_DRAWN 	= 104; 

//Bitmap Panning registers
const int PANE_CENTRE_X 	= 120;	//Registers to store temporary coordinates when panning. 
const int PANE_CENTRE_Y 	= 121; 
const int PANE_LEFT_X 		= 122; 
const int PANE_LEFT_Y 		= 123;
const int PANE_RIGHT_X 		= 124; //! These start at a value that draws offscreen, 
const int PANE_RIGHT_Y 		= 125; //! and reduces to 0 to pan into view. 

//Placed Subscreen Objects
const int SUBSC_OBJ_SELECTED 	= 150; //Holds the ID of the present, selected, placed object. 
const int SUBSC_OBJ_NUMOBJECTS 	= 151; 

void SubsNumObjects(int n){
	int optr = GetSubscreenObjectsPtr();
	optr[SUBSC_OBJ_NUMOBJECTS] = n;
}

int SubsNumObjects() { 
	int optr = GetSubscreenObjectsPtr();
	return optr[SUBSC_OBJ_NUMOBJECTS];
}

//(None of these are used at present. Reserved for background tiles, and objects.)
const int SUBSC_TILE_BASE 	= 200;
const int SUBSC_CSET_BASE 	= 201;
const int SUBSC_FLASH_BASE 	= 202;
const int SUBSC_AFRAMES_BASE 	= 203;
const int SUBSC_ASPEED_BASE 	= 204;
const int SUBSC_FRAME_BASE 	= 205;
const int SUBSC_DELAY_BASE 	= 206;
const int SUBSC_SCRIPT_BASE 	= 207; /*	These bases were for using tiles instead of items, to place objects
						as the base index of an array containing datum on object tiles, csets, 
						and so forth. */
//Item and Itemdata Registers
const int SUBSC_IDATA_SCRATCH 	= 29; //Used as scratch space for itemdata loading. 
const int SUBSC_ITEM_SCRATCH 	= 25; //Not implemented. 


//Subscreen Object Array Indices
const int SUBSC_OBJ_ID 		= 0; 
const int SUBSC_OBJ_X 		= 1; 
const int SUBSC_OBJ_Y 		= 2; 
const int SUBSC_OBJ_ITEM 	= 3;
const int SUBSC_OBJ_FUNCTION 	= 4; 
const int SUBSC_OBJ_NEXT_R 	= 5; 
const int SUBSC_OBJ_NEXT_L 	= 6;
const int SUBSC_OBJ_NEXT_U 	= 7;
const int SUBSC_OBJ_NEXT_D 	= 8;
const int SUBSC_OBJ_TILE 	= 9;
const int SUBSC_OBJ_CSET 	= 10;
const int SUBSC_OBJ_INFO	= 11; //A string or string ref. 
const int SUBSC_OBJ_TYPE 	= 12;
const int SUBSC_OBJ_LAST_INDEX 	= 13;

//Subscreen Object Misc[] indices
const int SUBSC_OBJ_MISC_OFFSET 	= 16; //First Misc index to use.  
const int SUBSC_OBJ_MISC_RIGHT 		= 0; 
const int SUBSC_OBJ_MISC_LEFT 		= 1;
const int SUBSC_OBJ_MISC_UP 		= 2; 
const int SUBSC_OBJ_MISC_DOWN 		= 3; 
const int SUBSC_OBJ_MISC_FUNCTION 	= 4;
const int SUBSC_OBJ_MISC_INFO 		= 5;
const int SUBSC_OBJ_MISC_TYPE	 	= 6;

//Type Values (these should probably be flags).
const int SUBSC_MISC_IS_SELECT_OBJECT 	= 1;


//Subscreen Object Functions
//List of functions
const int SubscSetItemA 	= 1;
const int SubscSetItemB 	= 2; 
const int SubscEquip 		= 3;
const int SubscEquipClass 	= 4;
const int SubscInfo 		= 5;

////////////////////////////////////
/// FFC Script for the Subscreen ///
////////////////////////////////////

//Returns the first free index of the objects array
int SubscGetFreeObj(){
	int optr = GetSubscreenObjectsPtr();
	int q;
	for ( q = 0; q < SizeOfArray(optr); q += SUBSC_OBJ_LAST_INDEX ) {
		if ( optr[q] == -1 ) return q;
	}
}

//Subscreen FFC
dmapdata script SubscreenScript{
	void run(){
		SSTest(1); bool pan_in = true;
		int q[265]; itemdata id[30]; item i[25]; item equip[2];
		int subscreenObjects[416]; //32*13 
		item subscObjects[33]; //0 is null, 1 to 32 are items.
		SetObjectSelected(1); //Clear to 1
		q[PANE_RIGHT_DRAWN] = 0;
		q[PANE_LEFT_DRAWN] = 0;
		//Wipe the array
		for ( q[SUBSC_FOR_LOOP] = SizeOfArray(subscreenObjects)-1; q[SUBSC_FOR_LOOP] >= 0; q[SUBSC_FOR_LOOP]-- )
					subscreenObjects[q[SUBSC_FOR_LOOP]] = -1;
		AssignSubscreenPointer(q); //Pute the pointer into GRAM[] so that global accessors work.
		SetSubscreenObjectsPtr(subscreenObjects);	
		q[SUBSC_ACTIVE] = 1;
		q[SUBSC_SELECTED] = 0; q[SUBSC_LAST_SELECTED] = 0; 
		q[SUBSC_POSX] = SUBSC_INITIAL_X; q[SUBSC_POSY] = SUBSC_INITIAL_Y; 
		q[PANE_CENTRE_DRAWN] = 1;
		q[PANE_ID] = PANE_CENTRE;
		SubPaneX(PANE_CENTRE,0);
		SubPaneY(PANE_CENTRE,-SUBSC_BITMAP_HEIGHT);
		SubPaneX(PANE_LEFT,-SUBSC_BITMAP_WIDTH);
		SubPaneY(PANE_LEFT,0);
		SubPaneX(PANE_RIGHT,SUBSC_BITMAP_WIDTH);
		SubPaneY(PANE_LEFT,0);
		
		//Populate with the IDs of items to place in the inventory selection box. 
		int subscreenitems[240]={I_SWORD1, I_SWORD2, I_SWORD3, I_SWORD4, I_BRANG1, I_BRANG2,
					I_CANDLE1, I_CANDLE2, I_BOMB, I_SBOMB, I_HAMMER, I_CBYRNA,
					I_BOOTS, I_BOW2, I_FLIPPERS, I_LADDER1, I_HOOKSHOT1, I_POTION1,
					I_POTION2, I_SHIELD1, I_WAND, I_WHISTLE, I_MAP, I_BAIT};
			//put the items on the screen.
		
		int EnableDisable[]={I_SHIELD1, I_LADDER1, I_FLIPPERS, I_BOOTS};
		StoreEnableItemsPtr(EnableDisable);

		id[SUBSC_LAST] = Game->LoadItemData(I_GENERIC);
		id[SUBSC_LAST]->Tile = CURSOR_TILE;
		id[SUBSC_LAST]->CSet = CURSOR_CSET;
		id[SUBSC_LAST]->AFrames = CURSOR_AFRAMES;
		id[SUBSC_LAST]->ASpeed = CURSOR_ASPEED;
		
		equip[0] = Screen->CreateItem(I_GENERIC);
		equip[0]->X = SUBSC_EQUIP_B_X; equip[0]->Y = SUBSC_EQUIP_B_Y;
		equip[0]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
		equip[0]->DrawYOffset = -1000;
		equip[0]->Pickup = IP_DUMMY;
		id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemB);
		equip[0]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
		equip[0]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
		
		
		equip[1] = Screen->CreateItem(I_GENERIC);
		equip[1]->X = SUBSC_EQUIP_A_X; equip[1]->Y = SUBSC_EQUIP_A_Y;
		equip[1]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
		equip[1]->DrawYOffset = -1000;
		equip[1]->Pickup = IP_DUMMY;
		id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemA);
		equip[1]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
		equip[1]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
		
		if ( SUBSC_FADE_IN == 1 ) Screen->ZapIn();
		if ( SUBSC_FADE_IN == 2 ) Screen->ZapOut();
		
		for ( q[SUBSC_FOR_LOOP] = 0; q[SUBSC_FOR_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_FOR_LOOP]++ ) {
			
			i[ q[SUBSC_FOR_LOOP] ] = Screen->CreateItem( subscreenitems[ q[SUBSC_FOR_LOOP] ]); 
			i[ q[SUBSC_FOR_LOOP] ]->X = q[SUBSC_POSX]; 
			i[ q[SUBSC_FOR_LOOP] ]->Y = q[SUBSC_POSY];
			i[ q[SUBSC_FOR_LOOP] ]->Pickup = IP_DUMMY;
			i[ q[SUBSC_FOR_LOOP] ]->DrawYOffset =	SubPaneY(PANE_CENTRE)*-1;	
			i[ q[SUBSC_FOR_LOOP] ]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
			
			if ( !Link->Item[ subscreenitems[q[SUBSC_FOR_LOOP] ] ] && !OwnsItems[ subscreenitems[q[SUBSC_FOR_LOOP] ] ] ) {
				i[ q[SUBSC_FOR_LOOP] ]->ID = I_SUBSCREEN_BLANK;
				i[ q[SUBSC_FOR_LOOP] ]->Tile = SUBSC_TILE_BLANK;
			}
			
			if ( q[SUBSC_POSX] < ( SUBSC_INITIAL_X + ( SUBSC_X_SPACING * (SUBSC_NUM_COLUMNS-1) ) ) )  {
				q[SUBSC_POSX] += SUBSC_X_SPACING;
			}
				
			else {
				q[SUBSC_POSX] = SUBSC_INITIAL_X; 
				if ( q[SUBSC_POSY] < ( SUBSC_INITIAL_Y + (SUBSC_Y_SPACING * (SUBSC_NUM_ROWS-1) ) ) ) q[SUBSC_POSY] += SUBSC_Y_SPACING;
			}
		}
		
		//Pan in the initial tiles. 
		do{ //Initial pan in. 
			pan_in = PanUpToDown(PANE_CENTRE,true);
			for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
						DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);	
			WaitNoAction();
		} while(pan_in);
		 
		
		//Find out what is in Link's B slot.
		for ( q[SUBSC_SLOTCHECK_LOOP] = 0; q[SUBSC_SLOTCHECK_LOOP] < 24; q[SUBSC_SLOTCHECK_LOOP]++ ) {
			if ( subscreenitems[ q[SUBSC_FOR_LOOP] ] == Link->ItemB ) { q[SUBSC_LAST_SELECTED] = subscreenitems[ q[SUBSC_FOR_LOOP] ]; q[SUBSC_SELECTED] =  subscreenitems[ q[SUBSC_FOR_LOOP] ]; break; }
		}
		//if nothing matches, check the A slot
		for ( q[SUBSC_SLOTCHECK_LOOP] = 0; q[SUBSC_SLOTCHECK_LOOP] < 24; q[SUBSC_SLOTCHECK_LOOP]++ ) {
			if ( q[SUBSC_SELECTED] != 0 ) break; //If we found a B item, skip this step. 
			if ( subscreenitems[ q[SUBSC_FOR_LOOP] ] == Link->ItemA )  { q[SUBSC_LAST_SELECTED] = subscreenitems[ q[SUBSC_FOR_LOOP] ]; q[SUBSC_SELECTED] =  subscreenitems[ q[SUBSC_FOR_LOOP] ]; break; }
		}
		i[SUBSC_LAST] = Screen->CreateItem(I_GENERIC);
		i[SUBSC_LAST]->Pickup = IP_DUMMY;
		i[SUBSC_LAST]->X = i[q[SUBSC_SELECTED]]->X;
		i[SUBSC_LAST]->Y = i[q[SUBSC_SELECTED]]->Y;
		i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
		
		/*
		//Draw the tiles to a bitmap?
		if ( BG_IS_BITMAP > -1 && BG_IS_BITMAP < 7 ) { 
			//Set up the bitmap.
			SetupSubscreenBitmap();
		}
		*/
		
		//Bitmaps are deprecated. 
		
		
		Game->PlaySound(SUBSC_OPEN_SFX);
		while( q[SUBSC_ACTIVE] ){
			SSTest(2);
			if ( !pan_in ) {
				if ( BG_IS_BITMAP > -1 && BG_IS_BITMAP < 8 ) {
					Screen->SetRenderTarget(RT_SCREEN);
					Screen->DrawBitmapEx(BG_IS_BITMAP, RT_SUBSCREEN, SUBSC_BITMAP_X, SUBSC_BITMAP_Y, SUBSC_BITMAP_WIDTH, 
					SUBSC_BITMAP_HEIGHT, SUBSC_BITMAP_X, SUBSC_BITMAP_Y, SUBSC_BITMAP_WIDTH, SUBSC_BITMAP_HEIGHT, 0, 0,0, 
					SUBSC_BITMAP_DRAW_MODE, 0, true);
				}
				else { 
					DrawSubscreenTiles(q[PANE_ID]);
				}
				DrawSubscreenInventoryText();
			}
			
			if ( q[PANE_ID] == PANE_CENTRE && !q[PANE_CENTRE_DRAWN] ) {
				q[SUBSC_POSX] = SUBSC_INITIAL_X; q[SUBSC_POSY] = SUBSC_INITIAL_Y; 
				for ( q[SUBSC_FOR_LOOP] = 0; q[SUBSC_FOR_LOOP] <= SUBSC_MAX_ITEMS; q[SUBSC_FOR_LOOP]++ ) {
					i[ q[SUBSC_FOR_LOOP] ] = Screen->CreateItem( subscreenitems[ q[SUBSC_FOR_LOOP] ]); 
					i[ q[SUBSC_FOR_LOOP] ]->X = q[SUBSC_POSX]; 
					i[ q[SUBSC_FOR_LOOP] ]->Y = q[SUBSC_POSY];
					i[ q[SUBSC_FOR_LOOP] ]->Pickup = IP_DUMMY;
					i[ q[SUBSC_FOR_LOOP] ]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
					i[ q[SUBSC_FOR_LOOP] ]->DrawYOffset = -1000;
					
					if ( !Link->Item[ subscreenitems[q[SUBSC_FOR_LOOP] ] ] && !OwnsItems[ subscreenitems[q[SUBSC_FOR_LOOP] ] ] ) {
						i[ q[SUBSC_FOR_LOOP] ]->ID = I_SUBSCREEN_BLANK;
						i[ q[SUBSC_FOR_LOOP] ]->Tile = SUBSC_TILE_BLANK;
					}
					
					if ( q[SUBSC_POSX] < ( SUBSC_INITIAL_X + ( SUBSC_X_SPACING * (SUBSC_NUM_COLUMNS-1) ) ) )  {
						q[SUBSC_POSX] += SUBSC_X_SPACING;
					}
						
					else {
						q[SUBSC_POSX] = SUBSC_INITIAL_X; 
						if ( q[SUBSC_POSY] < ( SUBSC_INITIAL_Y + (SUBSC_Y_SPACING * (SUBSC_NUM_ROWS-1) ) ) ) q[SUBSC_POSY] += SUBSC_Y_SPACING;
					}
				}
				//Find out what is in Link's B slot.
				for ( q[SUBSC_SLOTCHECK_LOOP] = 0; q[SUBSC_SLOTCHECK_LOOP] < 24; q[SUBSC_SLOTCHECK_LOOP]++ ) {
					if ( subscreenitems[ q[SUBSC_FOR_LOOP] ] == Link->ItemB ) { q[SUBSC_LAST_SELECTED] = subscreenitems[ q[SUBSC_FOR_LOOP] ]; q[SUBSC_SELECTED] =  subscreenitems[ q[SUBSC_FOR_LOOP] ]; break; }
				}
				//if nothing matches, check the A slot
				for ( q[SUBSC_SLOTCHECK_LOOP] = 0; q[SUBSC_SLOTCHECK_LOOP] < 24; q[SUBSC_SLOTCHECK_LOOP]++ ) {
					if ( q[SUBSC_SELECTED] != 0 ) break; //If we found a B item, skip this step. 
					if ( subscreenitems[ q[SUBSC_FOR_LOOP] ] == Link->ItemA )  { q[SUBSC_LAST_SELECTED] = subscreenitems[ q[SUBSC_FOR_LOOP] ]; q[SUBSC_SELECTED] =  subscreenitems[ q[SUBSC_FOR_LOOP] ]; break; }
				}
				i[SUBSC_LAST] = Screen->CreateItem(I_GENERIC);
				i[SUBSC_LAST]->Pickup = IP_DUMMY;
				i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
				
				i[SUBSC_LAST]->X = i[q[SUBSC_SELECTED]]->X;
				i[SUBSC_LAST]->Y = i[q[SUBSC_SELECTED]]->Y;
				i[SUBSC_LAST]->DrawYOffset = 0; //Show the cursor. 

				//Draw the tiles to a bitmap?
				
				/*
				if ( BG_IS_BITMAP > -1 && BG_IS_BITMAP < 7 ) { 
					//Set up the bitmap.
					SetupSubscreenBitmap();
				}
				*/
				//Bitmaps are deprecated, for now. Would anyone do this, when it wastes three RTs?
				
				q[PANE_CENTRE_DRAWN] = 1; //mark drawing done. 
			}
	
			
			if ( q[PANE_ID] == PANE_CENTRE ) {
				i[SUBSC_LAST]->X = i[q[SUBSC_SELECTED]]->X;
				i[SUBSC_LAST]->Y = i[q[SUBSC_SELECTED]]->Y;
				if ( SUBSC_OBJ_LAYER ) {
					for ( q[SUBSC_OBJDRAW_LOOP] = 0; q[SUBSC_OBJDRAW_LOOP] <= SUBSC_MAX_ITEMS; q[SUBSC_OBJDRAW_LOOP]++ )
						DrawItemToLayer(i[q[SUBSC_OBJDRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
				}
				/*
				if ( CURSOR_LAYER ) {
					DrawItemToLayer(i[SUBSC_LAST],CURSOR_LAYER, OP_OPAQUE);
				}
				*/
					//Handled another way now. 
				if ( Link->PressRight ) {
					if ( q[SUBSC_SELECTED] < ( SUBSC_MAX_ITEMS -1 ) ) { 
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED]++;
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
					else { 
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] = 0;
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
				}
				if ( Link->PressLeft ) {
					if ( q[SUBSC_SELECTED] > 0 ) { 
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED]--;
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
					else { 
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] = ( SUBSC_MAX_ITEMS -1 );
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
				}
				if ( Link->PressDown ) {
					if ( q[SUBSC_SELECTED] < ( SUBSC_MAX_ITEMS - SUBSC_ITEMS_PER_ROW ) ) { //18
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] += SUBSC_ITEMS_PER_ROW; //6
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
					else { //on bottom row
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] -= (SUBSC_MAX_ITEMS - SUBSC_ITEMS_PER_ROW); //18
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
				}
				if ( Link->PressUp ) {
					
					if ( q[SUBSC_SELECTED] > ( SUBSC_ITEMS_PER_ROW -1 )  ) {  //5
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] -= SUBSC_ITEMS_PER_ROW; //6
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
					else { 
						q[SUBSC_LAST_SELECTED] = q[SUBSC_SELECTED]; 
						q[SUBSC_SELECTED] += (SUBSC_MAX_ITEMS - SUBSC_ITEMS_PER_ROW); //18
						Game->PlaySound(SUBSC_CURSOR_SFX);
					}
				}
				if ( Link->PressA ) {
					if ( i[q[SUBSC_SELECTED]]->ID != I_SUBSCREEN_BLANK ) {
						//If the item is not usable, and only something to equip or
						//to unequip, such as shields, or flippers...
						if ( IsEnableDisableItem( i[q[SUBSC_SELECTED]]->ID ) ) {
							Game->PlaySound(SUBSC_SFX_EQUIP);
							//Be sure to set that Link owns it...
							OwnsItems[i[q[SUBSC_SELECTED]]->ID] = 1;
							//equip[0]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
							//Equip/Unequip that item. 
							DoEnableDisable( i[q[SUBSC_SELECTED]]->ID );
						}								
						else{
							//Set Link's A-slot item
							Link->ItemA = i[q[SUBSC_SELECTED]]->ID; 
							
							//Set the array index, so tha it is properly updated. 
							ButtonItems[BUTTONITEM_A] = i[q[SUBSC_SELECTED]]->ID; 
							//Load the itemdata so that we can set the tile for the 
							//subscreen-A-Slot icon.
							id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemA);
							//Set the A-Slot icon on the subscreen with the proper tile
							//and cset. 
							equip[1]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
							equip[1]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
							Game->PlaySound(SUBSC_SELECTED_SFX);
						}
					}
				}
				if ( Link->PressB ) {
					if ( i[q[SUBSC_SELECTED]]->ID != I_SUBSCREEN_BLANK ) {
						if ( IsEnableDisableItem( i[q[SUBSC_SELECTED]]->ID ) ) {
							Game->PlaySound(SUBSC_SFX_EQUIP);
							OwnsItems[i[q[SUBSC_SELECTED]]->ID] = 1;
							DoEnableDisable( i[q[SUBSC_SELECTED]]->ID );
						}
						else {
							
							Link->ItemB = i[q[SUBSC_SELECTED]]->ID;
							ButtonItems[BUTTONITEM_B] = i[q[SUBSC_SELECTED]]->ID; 	
							id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemB);
							
							equip[0]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
							equip[0]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
							Game->PlaySound(SUBSC_SELECTED_SFX);
						}
					}
				}
				
				if ( Link->PressR ) {
					Game->PlaySound(SUBSC_CHANGEPANE_SFX); 
					pan_in = true;
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					do{
						PanRightToLeft(PANE_CENTRE,false);
						pan_in = PanRightToLeft(PANE_RIGHT,true);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
						WaitNoAction();
					} while(pan_in);
					
					
					q[PANE_ID] = PANE_RIGHT;  //pan to right page
					q[PANE_CENTRE_DRAWN] = 0; 
				}  //pan to right page
				if ( Link->PressL ) { 
					
					Game->PlaySound(SUBSC_CHANGEPANE_SFX); 
					pan_in = true;
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					do{
						PanLeftToRight(PANE_CENTRE,false);
						pan_in = PanLeftToRight(PANE_LEFT,true);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
						WaitNoAction();
					} while(pan_in);
					
					
					q[PANE_ID] = PANE_LEFT;  //pan to left page
					q[PANE_CENTRE_DRAWN] = 0; 
				}
				if ( Link->PressStart ) {
					Link->PressStart = false; 
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					pan_in = true;
					Game->PlaySound(SUBSC_CLOSE_SFX);
					do{
						pan_in = PanExit(PANE_CENTRE,false,i);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
							/*Doing this causes graphical glitches:
							DrawItemToLayer(equip[0], SUBSC_OBJ_LAYER, OP_OPAQUE);
							DrawItemToLayer(equip[1], SUBSC_OBJ_LAYER, OP_OPAQUE);
							*/
						WaitNoAction();
					} while(pan_in);
					if ( SUBSC_FADE_OUT == 1 ) Screen->ZapIn();
					if ( SUBSC_FADE_OUT == 2 ) Screen->ZapOut();
					q[SUBSC_ACTIVE] = 0; //Mark that we should draw.
					
					for ( q[SUBSC_EXIT_CLEANUP_LOOP] = Screen->NumItems(); q[SUBSC_EXIT_CLEANUP_LOOP] > 0; q[SUBSC_EXIT_CLEANUP_LOOP]--) {
						i[SUBSC_LAST] = Screen->LoadItem(q[SUBSC_EXIT_CLEANUP_LOOP]);
						if ( i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] == IS_SUBSCREEN_OBJECT ) Remove(i[SUBSC_LAST]);
					}
					Remove(equip[0]); Remove(equip[1]);
					GRAM[EXITING_SUBSCREEN] = 1; 
					ButtonItems[BUTTONITEM_SUBSCREEN] = 0;
					/*if ( SUBSC_LINK_INVISIBLE ) */ Link->Invisible = false;
					Quit();
				}
				i[SUBSC_LAST]->X = i[q[SUBSC_SELECTED]]->X;
				i[SUBSC_LAST]->Y = i[q[SUBSC_SELECTED]]->Y;
			}
			if ( q[PANE_ID] == PANE_LEFT ) {
				if ( Link->PressR ) { 
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					
					Game->PlaySound(SUBSC_CHANGEPANE_SFX); 
					pan_in = true;
					do{
						PanRightToLeft(PANE_LEFT,false);
						pan_in = PanRightToLeft(PANE_CENTRE,true);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
							/*Doing this causes graphical glitches:
							//DrawItemToLayer(equip[0], SUBSC_OBJ_LAYER, OP_OPAQUE);
							//DrawItemToLayer(equip[1], SUBSC_OBJ_LAYER, OP_OPAQUE);
							*/
						WaitNoAction();
					} while(pan_in);
					
					
					q[PANE_ID] = PANE_CENTRE;  //pan to left page
					q[PANE_CENTRE_DRAWN] = 0; 
					
				}
				if ( Link->PressStart ) {
					Link->PressStart = false; 
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					Game->PlaySound(SUBSC_CLOSE_SFX);
					pan_in = true;
					do{
						pan_in = PanExit(PANE_LEFT,false,i);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
						
						WaitNoAction();
					} while(pan_in);

					for ( q[SUBSC_EXIT_CLEANUP_LOOP] = Screen->NumItems(); q[SUBSC_EXIT_CLEANUP_LOOP] > 0; q[SUBSC_EXIT_CLEANUP_LOOP]--) {
						i[SUBSC_LAST] = Screen->LoadItem(q[SUBSC_EXIT_CLEANUP_LOOP]);
						if ( i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] == IS_SUBSCREEN_OBJECT ) Remove(i[SUBSC_LAST]);
					}
					Remove(equip[0]); Remove(equip[1]);
					GRAM[EXITING_SUBSCREEN] = 1; 
					ButtonItems[BUTTONITEM_SUBSCREEN] = 0;
					/*if ( SUBSC_LINK_INVISIBLE )*/ Link->Invisible = false;
					if ( SUBSC_FADE_OUT == 1 ) Screen->ZapIn();
					if ( SUBSC_FADE_OUT == 2 ) Screen->ZapOut();
					
					q[SUBSC_ACTIVE] = 0;
					Quit();
				}
			}
			if ( q[PANE_ID] == PANE_RIGHT ) {
				q[SUBSC_OBJ_SELECTED] = 1;
				/*
				Insert the code that the subscreen generator output to allegro.log directly
				below this comment block.
				*/
				
				//! Place Subscreen Objects
				//Set its properties. 
				//0
				if ( !q[PANE_RIGHT_DRAWN] ) {
					q[PANE_RIGHT_DRAWN] = 1;
					SetSubscreenObjectProperties(0, 16, 32, I_SHIELD1, 
						SubscEquipClass, 2, SUBSC_MISC_IS_SELECT_OBJECT, 1,0,2,1);
					//Create the object.
					CreateSubscreenObject(subscObjects, 0); 
							/*Create the object using the array
							reading how many objects exist o store its position.
							*/
					SubsNumObjects( 1 ); //Update the number of objects. 
					
					//1
					SetSubscreenObjectProperties(1, 32, 32, I_FLIPPERS, 
						SubscEquip, 2, SUBSC_MISC_IS_SELECT_OBJECT, 1,0,2,1);
					//Create the object.
					CreateSubscreenObject(subscObjects, 1); 
							/*Create the object using the array
							reading how many objects exist o store its position.
							*/
					SubsNumObjects( 2 ); //Update the number of objects. 
						
					//2
					SetSubscreenObjectProperties(2, 32, 32, I_LADDER1, 
						SubscEquip, 2, SUBSC_MISC_IS_SELECT_OBJECT, 1,0,2,1);
					//Create the object.
					CreateSubscreenObject(subscObjects, 2); 
							/*Create the object using the array
							reading how many objects exist o store its position.
							*/
					SubsNumObjects( 3 ); //Update the number of objects. 
					/*
					Insert the code that the subscreen generator output to allegro.log directly
					below above comment block.
					*/
					
					//! Create the cursor
					i[SUBSC_LAST] = Screen->CreateItem(I_GENERIC);
					i[SUBSC_LAST]->Pickup = IP_DUMMY;
					i[SUBSC_LAST]->X = subscObjects[q[SUBSC_OBJ_SELECTED]]->X;
					i[SUBSC_LAST]->Y = subscObjects[q[SUBSC_OBJ_SELECTED]]->Y;
					i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] = IS_SUBSCREEN_OBJECT;
					//Stop it from appearing when the subscreen is raised
					//i[SUBSC_LAST]->DrawYOffset = -1000;
				
					
				}
				
				//! Set the initial selected
				
				
				//!!Draw the items and the cursor to a layer
				/*
				//Items draw
				for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(subscObjects[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
				
				//Cursor draw
				DrawItemToLayer(i[SUBSC_LAST],SUBSC_OBJ_LAYER, OP_OPAQUE);
				
				*/
				//! Check for L, R, and Start. We check for other input *after* these. 
				if ( Link->PressL ) { 
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					
					Game->PlaySound(SUBSC_CHANGEPANE_SFX); 
					pan_in = true;
					do{
						PanLeftToRight(PANE_RIGHT,false);
						pan_in = PanLeftToRight(PANE_CENTRE,true);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
							
							/*Doing this causes graphical glitches:
							//DrawItemToLayer(equip[0], SUBSC_OBJ_LAYER, OP_OPAQUE);
							//DrawItemToLayer(equip[1], SUBSC_OBJ_LAYER, OP_OPAQUE);
							*/
						WaitNoAction();
					} while(pan_in);
					
					
					q[PANE_ID] = PANE_CENTRE;  //pan to left page
					q[PANE_CENTRE_DRAWN] = 0; 
					
				}
				if ( Link->PressStart ) {
					Link->PressStart = false; 
					i[SUBSC_LAST]->DrawYOffset = -1000; //Hide the cursor. 
					Game->PlaySound(SUBSC_CLOSE_SFX);
					pan_in = true;
					do{
						pan_in = PanExit(PANE_RIGHT,false,i);
						for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] < SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ ) {
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
							Trace(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]]->ID);
						}
						
						WaitNoAction();
					} while(pan_in);

					for ( q[SUBSC_EXIT_CLEANUP_LOOP] = Screen->NumItems(); q[SUBSC_EXIT_CLEANUP_LOOP] > 0; q[SUBSC_EXIT_CLEANUP_LOOP]--) {
						i[SUBSC_LAST] = Screen->LoadItem(q[SUBSC_EXIT_CLEANUP_LOOP]);
						if ( i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] == IS_SUBSCREEN_OBJECT ) Remove(i[SUBSC_LAST]);
					}
					Remove(equip[0]); Remove(equip[1]);
					GRAM[EXITING_SUBSCREEN] = 1; 
					ButtonItems[BUTTONITEM_SUBSCREEN] = 0;
					/*if ( SUBSC_LINK_INVISIBLE )*/ Link->Invisible = false;
					if ( SUBSC_FADE_OUT == 1 ) Screen->ZapIn();
					if ( SUBSC_FADE_OUT == 2 ) Screen->ZapOut();
					
					q[SUBSC_ACTIVE] = 0;
					Quit();
				}
				
			}
			if ( !pan_in ) { //Prevent flicker of background. 
				DrawSubscreenTiles(q[PANE_ID]);
				if ( q[PANE_ID] == PANE_CENTRE ){
					for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 0; q[SUBSC_OBJ_PAN_DRAW_LOOP] <= SUBSC_MAX_ITEMS; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(i[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);

					DrawItemToLayer(equip[0], SUBSC_OBJ_LAYER, OP_OPAQUE);
					DrawItemToLayer(equip[1], SUBSC_OBJ_LAYER, OP_OPAQUE);
				}
				if ( q[PANE_ID] == PANE_LEFT ){
					//draws
				}
				if ( q[PANE_ID] == PANE_RIGHT ){
					if ( Link->PressEx1 ) TraceB(pan_in);
					//Items draw
					for ( q[SUBSC_OBJ_PAN_DRAW_LOOP] = 1; q[SUBSC_OBJ_PAN_DRAW_LOOP] <= 32; q[SUBSC_OBJ_PAN_DRAW_LOOP]++ )
							DrawItemToLayer(subscObjects[q[SUBSC_OBJ_PAN_DRAW_LOOP]],SUBSC_OBJ_LAYER, OP_OPAQUE);
				
					//Cursor draw
					DrawItemToLayer(i[SUBSC_LAST],SUBSC_OBJ_LAYER, OP_OPAQUE);
				
					SubscCursorMove(i[SUBSC_LAST], subscObjects);
				}
				DrawSubscreenInventoryText();
			}
			Waitframe();
		}
		for ( q[SUBSC_EXIT_CLEANUP_LOOP] = Screen->NumItems(); q[SUBSC_EXIT_CLEANUP_LOOP] > 0; q[SUBSC_EXIT_CLEANUP_LOOP]--) {
			i[SUBSC_LAST] = Screen->LoadItem(q[SUBSC_EXIT_CLEANUP_LOOP]);
			if ( i[SUBSC_LAST]->Misc[MISC_SUBSC_OBJ] == IS_SUBSCREEN_OBJECT ) Remove(i[SUBSC_LAST]);
		}
		Remove(equip[0]); Remove(equip[1]);
		GRAM[EXITING_SUBSCREEN] = 1; 
		ButtonItems[BUTTONITEM_SUBSCREEN] = 0;
		/*if ( SUBSC_LINK_INVISIBLE )*/ Link->Invisible = false;
		//Resume Ghost.zh here.

		Quit();
	}
}

////////////////////////////////
/// Test and Utility Scripts ///
////////////////////////////////

item script test{
	void run(){
		int s[]="Using item: ";
		TraceS(s); Trace(Link->UsingItem);
		int ss[]="Tracing this->ID: ";
		TraceS(ss); Trace(this->ID);
	}
}

item script map{
	void run(){
		Link->InputMap = true;
		Link->PressMap = true;
	}
}

global script Init{
	void run(){
		SetDefaultItems();
	}
}

item script Equip{
	void run(){
		if ( Link->Item[this->ID] && OwnsItems[this->ID] ){ OwnsItems[this->ID] = 1; Link->Item[this->ID] = false; }
		if ( !Link->Item[this->ID] && OwnsItems[this->ID] ) Link->Item[this->ID] = true;
	}
}
		

////////////////////////
/// Global Functions ///
////////////////////////


//! Global Array Accessors

//Returns the pointer for the global array index that controls subscreen values. 
int GetSubscreenPtr(){ return GRAM[SUBSCREEN]; }

//Called from the ffc to store the array pointer in GRAM[]
void AssignSubscreenPointer(int ptr){
	GRAM[SUBSCREEN]=ptr;
}

//Get the pointer for the internal ffc array from anywhere. 
int GetEnableDisablePtr(){ return GRAM[ENABLEITEMS_PTR]; }


//Call to run the subscreen ffc. 
void DoSubscreen(){
	/*if ( SUBSC_LINK_INVISIBLE )*/ Link->Invisible = true;
	//Suspend ghost.zh
	ButtonItems[BUTTONITEM_SUBSCREEN] = 1;
	int s[]="SubscreenScript";
	int ff = Game->GetFFCScript(s);
	ffc f = Screen->LoadFFC(SUBSC_FFC_INDEX);
	f->Data = SUBSC_FFC_DATA;
	f->Script = ff;
}

//Store the pointer of the internal ffc array into GRAM[]
void StoreEnableItemsPtr(int arr){
	GRAM[ENABLEITEMS_PTR] = arr;
}

//Returns if an item is an equippable item.
bool IsEnableDisableItem(int itm){
	int it_ptr = GetEnableDisablePtr();
	for ( int q = (SizeOfArray(it_ptr)-1); q >= 0; q-- ) {
		if ( it_ptr[q] == itm ) return true;
	}
	return false;
}

//Equip or unequip an item.
void DoEnableDisable(int itm){
	if ( Link->Item[ itm ] ) Link->Item[itm] = false;
	else Link->Item[itm] = true;
}

//Sets some base items. 
void SetDefaultItems(){
	Link->Item[I_SHIELD1] = true;
	Link->Item[I_SWORD1] = true;
	Link->Item[I_SWORD2] = true;
	Link->Item[I_WAND] = true;
	Link->Item[I_WHISTLE] = true;
	Link->Item[I_BAIT] = true;
	Link->Item[I_HOOKSHOT1] = true;
	Link->Item[I_MAP] = true;
	OwnsItems[I_MAP] = 1;
	//for ( int q = 0; q < 256; q++ ) TraceB(Link->Item[q]);
}


//! Drawing Routines : 18 draws here. 
void DrawSubscreenInventoryText(){
	//Keys 1
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS1+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_KEYS], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS1, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_KEYS], 0, SUBSC_TEXT_INV_OPACITY);
	//Keys 2
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS2+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->LKeys[Game->GetCurLevel()], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS2, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->LKeys[Game->GetCurLevel()], 0, SUBSC_TEXT_INV_OPACITY);
	//Keys 3
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS3+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_SCRIPT5], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_KEYS3, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_SCRIPT5], 0, SUBSC_TEXT_INV_OPACITY);
	//Bombs
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_BOMBS+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_BOMBS], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_BOMBS, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_BOMBS], 0, SUBSC_TEXT_INV_OPACITY);
	//Super Bombs
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_SBOMBS+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_SBOMBS], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_SBOMBS, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_SBOMBS], 0, SUBSC_TEXT_INV_OPACITY);
	//Arrows
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_ARROWS+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_ARROWS], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_ARROWS, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_ARROWS], 0, SUBSC_TEXT_INV_OPACITY);
	//Life
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_LIFE+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_LIFE], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_LIFE, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_LIFE], 0, SUBSC_TEXT_INV_OPACITY);
	//Magic
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_MAGIC+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_MAGIC], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_MAGIC, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_MAGIC], 0, SUBSC_TEXT_INV_OPACITY);
	//Money
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_MONEY+SUBSC_TEXT_INV_SHADOW_OFFSET, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_BG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_RUPEES], 0, SUBSC_TEXT_INV_OPACITY);
	Screen->DrawInteger(SUBSC_TEXT_LAYER, SUBSC_TEXT_X_MONEY, SUBSC_TEXT_Y_INVENTORY,
		SUBSC_TEXT_INV_FONT, SUBSC_TEXT_INV_FG_COLOUR, SUBSC_TEXT_BACKDROP, SUBSC_TEXT_INV_FONT_SIZE, SUBSC_TEXT_INV_FONT_SIZE, 
		Game->Counter[CR_RUPEES], 0, SUBSC_TEXT_INV_OPACITY);
}

//Store the subscreen panes into bitmaps. (Effectively deprecated.)
void SetupSubscreenBitmap(){
	Screen->SetRenderTarget(RT_BITMAP0);
	//Centre pane
	Screen->DrawTile( SUBSC_TILE_LAYER, SUBSC_TILE_INITIAL_X, SUBSC_TILE_INITIAL_Y,
						SUBSC_FIRST_TILE, SUBSC_TILE_WIDTH, SUBSC_TILE_HEIGHT, 
						SUBSC_TILE_CSET, SUBSC_TILE_XSCALE, SUBSC_TILE_YSCALE, 
						SUBSC_TILE_RX, SUBSC_TILE_RY, SUBSC_TILE_RANGLE, 
						SUBSC_TILE_FLIP, true, SUBSC_TILE_OPACITY);
	Screen->SetRenderTarget(RT_BITMAP1);
	//Left pane
	Screen->DrawTile( SUBSC_TILE_LAYER, SUBSC_TILE_INITIAL_X, SUBSC_TILE_INITIAL_Y,
						SUBSC_TILE_LPANE, SUBSC_TILE_WIDTH, SUBSC_TILE_HEIGHT, 
						SUBSC_TILE_CSET, SUBSC_TILE_XSCALE, SUBSC_TILE_YSCALE, 
						SUBSC_TILE_RX, SUBSC_TILE_RY, SUBSC_TILE_RANGLE, 
						SUBSC_TILE_FLIP, true, SUBSC_TILE_OPACITY);
	Screen->SetRenderTarget(RT_BITMAP2);
	//Right pane
	Screen->DrawTile( SUBSC_TILE_LAYER, SUBSC_TILE_INITIAL_X, SUBSC_TILE_INITIAL_Y,
						SUBSC_TILE_LPANE, SUBSC_TILE_WIDTH, SUBSC_TILE_HEIGHT, 
						SUBSC_TILE_CSET, SUBSC_TILE_XSCALE, SUBSC_TILE_YSCALE, 
						SUBSC_TILE_RX, SUBSC_TILE_RY, SUBSC_TILE_RANGLE, 
						SUBSC_TILE_FLIP, true, SUBSC_TILE_OPACITY);
	
}

//Draws an item to a given layer
void DrawItemToLayer(item i, int layer, int opacity){
	Screen->DrawTile(layer,i->X+SubPaneX(PANE_CENTRE),i->Y+SubPaneY(PANE_CENTRE),i->Tile+i->Frame,i->TileWidth,i->TileHeight,i->CSet,-1,-1,0,0,0,0,true,opacity);
	//Trace(i->Frame);
}


//Draws an item to a layer at x, y
void DrawItemToLayer(item i, int layer, int x, int y, int opacity){
	Screen->DrawTile(layer,x,y,i->Tile+i->Frame,i->TileWidth,i->TileHeight,i->CSet,-1,-1,0,0,0,0,true,opacity);
}


//Returns the present subscreen base tile. 
int GetPaneTile(int pane){
	if ( pane == PANE_LEFT ) return SUBSC_TILE_LPANE;
	else if ( pane == PANE_RIGHT ) return SUBSC_TILE_RPANE;
	else return SUBSC_FIRST_TILE;
}

//THe main drawing routine. Call as: DrawSubscreenTiles(GetSubscreenPane())
void DrawSubscreenTiles(int pane_id){
	Screen->DrawTile( SUBSC_TILE_LAYER, SUBSC_TILE_INITIAL_X+SubPaneX(pane_id), SUBSC_TILE_INITIAL_Y+SubPaneY(pane_id),
						GetPaneTile(pane_id), SUBSC_TILE_WIDTH, SUBSC_TILE_HEIGHT, 
						SUBSC_TILE_CSET, SUBSC_TILE_XSCALE, SUBSC_TILE_YSCALE, 
						SUBSC_TILE_RX, SUBSC_TILE_RY, SUBSC_TILE_RANGLE, 
						SUBSC_TILE_FLIP, true, SUBSC_TILE_OPACITY);
}


//! Subscreen Pane Panning

//Getters for Panning
int SubPaneX(int pane){
	int ss_ptr = GetSubscreenPtr();
	if ( pane == PANE_LEFT ) return ss_ptr[PANE_LEFT_X];
	else if ( pane == PANE_RIGHT ) return ss_ptr[PANE_RIGHT_X];
	else return ss_ptr[PANE_CENTRE_X];
}

int SubPaneY(int pane){
	int ss_ptr = GetSubscreenPtr();
	if ( pane == PANE_LEFT ) return ss_ptr[PANE_LEFT_Y];
	else if ( pane == PANE_RIGHT ) return ss_ptr[PANE_RIGHT_Y];
	else return ss_ptr[PANE_CENTRE_Y];
}

void SubPaneX(int pane, int value){
	int ss_ptr = GetSubscreenPtr();
	if ( pane == PANE_LEFT ) ss_ptr[PANE_LEFT_X] = value;
	else if ( pane == PANE_RIGHT ) ss_ptr[PANE_RIGHT_X] = value;
	else ss_ptr[PANE_CENTRE_X] = value;
}

void SubPaneY(int pane, int value){
	int ss_ptr = GetSubscreenPtr();
	if ( pane == PANE_LEFT ) ss_ptr[PANE_LEFT_Y] = value;
	else if ( pane == PANE_RIGHT ) ss_ptr[PANE_RIGHT_Y] = value;
	else ss_ptr[PANE_CENTRE_Y] = value;
}

//Panning routines
//Pan left
bool PanLeftToRight(int pane, bool in){
	//Slide in fromthe left
	Link->PressStart = false;
	NoAction();
	if ( in ) {
		if ( SubPaneX(pane) <= (SCREEN_LEFT-SUBSC_PAN_SPEED) ) {
			SubPaneX(pane, ( SubPaneX(pane) + SUBSC_PAN_SPEED ) );
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
	if ( !in ) {
		if ( SubPaneX(pane) <= (SCREEN_RIGHT+SUBSC_PAN_SPEED) ) {
			SubPaneX(pane, ( SubPaneX(pane) + SUBSC_PAN_SPEED ) );
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
}

bool PanRightToLeft(int pane, bool in){
	//Slide in fromthe left
	Link->PressStart = false;
	NoAction();
	if ( in ) {
		if ( SubPaneX(pane) > (SCREEN_LEFT) ) {
			SubPaneX(pane,SubPaneX(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
	if ( !in ) {
		if ( SubPaneX(pane)+SCREEN_RIGHT > (SCREEN_LEFT-SUBSC_PAN_SPEED) ) {
			SubPaneX(pane,SubPaneX(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
}

bool PanUpToDown(int pane, bool in){
	//Slide in fromthe left
	Link->PressStart = false;
	NoAction();
	if ( in ) {
		if ( SubPaneY(pane) <= (SCREEN_TOP-SUBSC_PAN_SPEED) ) {
			//if ( SubPaneY(pane) <= (SCREEN_TOP+SUBSC_PAN_SPEED) ) 
			SubPaneY(pane,SubPaneY(pane)+SUBSC_PAN_SPEED);
			//else
			//SubPaneY(pane,SubPaneY(pane)+1);
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
	if ( !in ) {
		if ( SubPaneY(pane) <= (SCREEN_BOTTOM-SUBSC_PAN_SPEED) ) {
			SubPaneY(pane,SubPaneY(pane)+SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
}

bool PanDownToUp(int pane, bool in){
	//Slide in fromthe left
	Link->PressStart = false;
	NoAction();
	if ( in ) {
		if ( SubPaneY(pane) > (SUBSCREEN_TOP+SUBSC_PAN_SPEED) ) {
			SubPaneY(pane,SubPaneY(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
			
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
	if ( !in ) {
		if ( SubPaneY(pane)+SUBSC_BITMAP_HEIGHT > (SUBSCREEN_TOP+SUBSC_PAN_SPEED) ) {
			SubPaneY(pane,SubPaneY(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
}

//A special pan out routine for exiting the subscreen.
bool PanExit(int pane, bool in, item arr){
	//Slide in fromthe left
	Link->PressStart = false;
	NoAction();
	int q;
	if ( in ) {
		if ( SubPaneY(pane) > (SUBSCREEN_TOP+SUBSC_PAN_SPEED) ) {
			SubPaneY(pane,SubPaneY(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			return true;
			
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		return false;
	}
	if ( !in ) {
		if ( SubPaneY(pane)+SUBSC_BITMAP_HEIGHT > (SUBSCREEN_TOP+SUBSC_PAN_SPEED) ) {
			SubPaneY(pane,SubPaneY(pane)-SUBSC_PAN_SPEED);
			DrawSubscreenTiles(pane);
			for ( q = 0; q <= SUBSC_MAX_ITEMS; q++ )
				DrawItemToLayer(arr[q],SUBSC_OBJ_LAYER, OP_OPAQUE);
						
			return true;
		}
		//Draw tiles or bitmap.
		//tiles
		DrawSubscreenTiles(pane);
		for ( q = 0; q <= SUBSC_MAX_ITEMS; q++ ) 
			Remove(arr[ q ]); //<= to clean up cursor. 
					
		return false;
	}
}
 
//! Utilities

void ChangeItemScript(itemdata a, itemdata b){ 
	b->Script = a->Script;
}

void ChangeItemScript(itemdata a, item b){ 
	itemdata c = Game->LoadItemData(b->ID);
	a->Script = c->Script;
}

void ChangeItemPScript(itemdata a, itemdata b){ 
	b->PScript = a->PScript;
}

void ChangeItemPScript(itemdata a, item b){ 
	itemdata c = Game->LoadItemData(b->ID);
	a->PScript = c->PScript;
}


//Sample Global Active Script
global script active{
	void run(){
		SetDefaultItems();
		int a[]="ZC 2.54 Beta 46 Demo Quest One, v0.6"; TraceNL(); TraceS(a); TraceNL();
		Game->DisableActiveSubscreen = true; //Prevent the internal subscreen from appearing. 
		
		while(true){
			
			
			//Preserve the items on DMap change.
			//if ( !ButtonItems[BUTTONITEM_SUBSCREEN] ){
				if ( Link->ItemA != ButtonItems[BUTTONITEM_A] && ButtonItems[BUTTONITEM_A] != 0 ) Link->ItemA = ButtonItems[BUTTONITEM_A];
				if ( Link->ItemB != ButtonItems[BUTTONITEM_B] && ButtonItems[BUTTONITEM_B] != 0 ) Link->ItemB = ButtonItems[BUTTONITEM_B];
			//}
			//! The button items also shift when the subscreen opens and closes.
			//! Not sure why... Could this be doing it?
			
			if ( GRAM[EXITING_SUBSCREEN] ) {
				GRAM[EXITING_SUBSCREEN] = 0; 
				if ( SUBSC_FADE_IN == 1 ) Screen->ZapIn();
				if ( SUBSC_FADE_IN == 2 ) Screen->ZapOut();
				
			}
			
			//if ( !ButtonItems[BUTTONITEM_SUBSCREEN] ){
				if ( Link->ItemA != ButtonItems[BUTTONITEM_A] && ButtonItems[BUTTONITEM_A] != 0 ) Link->ItemA = ButtonItems[BUTTONITEM_A];
				if ( Link->ItemB != ButtonItems[BUTTONITEM_B] && ButtonItems[BUTTONITEM_B] != 0 ) Link->ItemB = ButtonItems[BUTTONITEM_B];
			//}
			Waitdraw();
			
			//Try callin it after waitdraw too, to prevent buggyness?
			//if ( !ButtonItems[BUTTONITEM_SUBSCREEN] ){
				if ( Link->ItemA != ButtonItems[BUTTONITEM_A] && ButtonItems[BUTTONITEM_A] != 0 ) Link->ItemA = ButtonItems[BUTTONITEM_A];
				if ( Link->ItemB != ButtonItems[BUTTONITEM_B] && ButtonItems[BUTTONITEM_B] != 0 ) Link->ItemB = ButtonItems[BUTTONITEM_B];
			//}
			Waitframe();
		}
	}
}

//Debug

void SSTest(int m){
	if ( SUBSCREEN_TEST_MODE < m ) return;
	if ( m == 1 ) {
		int s[]="Subscreen loaded.";
		TraceS(s);
	}
	if ( m == 2 ) {
		int s[]="Subscreen while loop running";
		TraceS(s);
	}
	if ( m == 4 ) {
		int s[]="Doing clean-up.";
		TraceS(s);
	}
}



///! Object adding and reading



int SubscreenObjects[32];
//item Objects[32];

void SetSubscreenObjectsPtr(int arr){
	GRAM[SUBSCREEN_OBJECTS] = arr; //SubscreenObjects[]
}

int GetSubscreenObjectsPtr(){ return GRAM[SUBSCREEN_OBJECTS]; }



void SetSubscreenObjectProperties(int id, int x, int y, int item_id, int tile, int cset, int function, int info, int type, int up, int down, int left, int right){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	optr[ofs+SUBSC_OBJ_ID] = id;
	optr[ofs+SUBSC_OBJ_X] = x;
	optr[ofs+SUBSC_OBJ_Y] = y;
	optr[ofs+SUBSC_OBJ_ITEM] = item_id;
	optr[ofs+SUBSC_OBJ_FUNCTION] = function;
	optr[ofs+SUBSC_OBJ_INFO] = info;
	optr[ofs+SUBSC_OBJ_TILE] = tile;
	optr[ofs+SUBSC_OBJ_CSET] = cset;
	optr[ofs+SUBSC_OBJ_NEXT_R] = right;
	optr[ofs+SUBSC_OBJ_NEXT_L] = left;
	optr[ofs+SUBSC_OBJ_NEXT_D] = down;
	optr[ofs+SUBSC_OBJ_NEXT_U] = up;
	optr[ofs+SUBSC_OBJ_TYPE] = type;
}

void SetSubscreenObjectProperties(int id, int x, int y, int item_id, int function, int info, int type, int up, int down, int left, int right){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	itemdata idata = Game->LoadItemData(item_id);
	optr[ofs+SUBSC_OBJ_ID] = id;
	optr[ofs+SUBSC_OBJ_X] = x;
	optr[ofs+SUBSC_OBJ_Y] = y;
	optr[ofs+SUBSC_OBJ_ITEM] = item_id;
	optr[ofs+SUBSC_OBJ_FUNCTION] = function;
	optr[ofs+SUBSC_OBJ_INFO] = info;
	optr[ofs+SUBSC_OBJ_TILE] = idata->Tile;
	optr[ofs+SUBSC_OBJ_CSET] = idata->CSet;
	optr[ofs+SUBSC_OBJ_NEXT_R] = right;
	optr[ofs+SUBSC_OBJ_NEXT_L] = left;
	optr[ofs+SUBSC_OBJ_NEXT_D] = down;
	optr[ofs+SUBSC_OBJ_NEXT_U] = up;
	optr[ofs+SUBSC_OBJ_TYPE] = type;
}

void CreateSubscreenObject(item arr, int id, int obj_type){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	arr[id] = Screen->CreateItem(I_SUBSC_OBJECT);
	int debug[]="Created item: ";
	TraceS(debug); Trace(arr[id]->ID);
	arr[id]->Tile = optr[ofs+SUBSC_OBJ_TILE];
	arr[id]->CSet = optr[ofs+SUBSC_OBJ_CSET];
	arr[id]->X = optr[ofs+SUBSC_OBJ_X];
	arr[id]->Y = optr[ofs+SUBSC_OBJ_Y];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT] = optr[ofs+SUBSC_OBJ_NEXT_R];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT] = optr[ofs+SUBSC_OBJ_NEXT_L];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN] = optr[ofs+SUBSC_OBJ_NEXT_D];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP] = optr[ofs+SUBSC_OBJ_NEXT_U];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_FUNCTION] = optr[ofs+SUBSC_OBJ_FUNCTION];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO] = optr[ofs+SUBSC_OBJ_INFO];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_TYPE] = obj_type;

}


void CreateSubscreenObject(item arr, int id){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	arr[id] = Screen->CreateItem(I_SUBSC_OBJECT);
	arr[id]->Tile = optr[ofs+SUBSC_OBJ_TILE];
	arr[id]->CSet = optr[ofs+SUBSC_OBJ_CSET];
	arr[id]->X = optr[ofs+SUBSC_OBJ_X];
	arr[id]->Y = optr[ofs+SUBSC_OBJ_Y];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT] = optr[ofs+SUBSC_OBJ_NEXT_R];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT] = optr[ofs+SUBSC_OBJ_NEXT_L];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN] = optr[ofs+SUBSC_OBJ_NEXT_D];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP] = optr[ofs+SUBSC_OBJ_NEXT_U];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_FUNCTION] = optr[ofs+SUBSC_OBJ_FUNCTION];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO] = optr[ofs+SUBSC_OBJ_INFO];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO] = optr[ofs+SUBSC_OBJ_INFO];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_TYPE] = optr[ofs+SUBSC_OBJ_TYPE];
}

void SetSubscreenObjectProperty(item arr, int id, int property, int value){
	id+=1;
	if ( property == SUBSC_OBJ_ID ) return;
	else if ( property == SUBSC_OBJ_X ) arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->X = value;
	else if ( property == SUBSC_OBJ_Y ) arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Y = value;
	else if ( property == SUBSC_OBJ_ITEM ) arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->ID = value;
	else if ( property == SUBSC_OBJ_NEXT_R ) 
		arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT] = value;
	else if ( property == SUBSC_OBJ_NEXT_L ) 
		arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT] = value;
	else if ( property == SUBSC_OBJ_NEXT_D ) 
		arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN] = value;
	else if ( property == SUBSC_OBJ_NEXT_U ) 
		arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP] = value;
	else if ( property == SUBSC_OBJ_FUNCTION ) 
		arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_FUNCTION] = value;
	else if ( property == SUBSC_OBJ_NEXT_U ) 
		arr[ ( id * SUBSC_OBJ_INFO ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO] = value;
	else if ( property == SUBSC_OBJ_TYPE ) 
		arr[ ( id * SUBSC_OBJ_TYPE ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_TYPE] = value;
	else return; //default
}

void SetSubscreenObjectProperty(int id, int property, int value){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	optr[ ( id * SUBSC_OBJ_LAST_INDEX ) + property ] = value;
}

//Returns an object property from an item. 
int GetSubscreenObjectProperty(item arr, int id, int property){
	id+=1;
	if ( property == SUBSC_OBJ_ID ) return id;
	if ( property == SUBSC_OBJ_X ) return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->X;
	if ( property == SUBSC_OBJ_Y ) return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Y;
	if ( property == SUBSC_OBJ_ITEM ) return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->ID;
	if ( property == SUBSC_OBJ_NEXT_R ) 
		return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT];
	if ( property == SUBSC_OBJ_NEXT_L ) 
		return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT];
	if ( property == SUBSC_OBJ_NEXT_D ) 
		return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN];
	if ( property == SUBSC_OBJ_NEXT_U ) 
		return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP];
	if ( property == SUBSC_OBJ_FUNCTION ) 
		return arr[ ( id * SUBSC_OBJ_LAST_INDEX ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_FUNCTION];
	if ( property == SUBSC_OBJ_NEXT_U ) 
		return arr[ ( id * SUBSC_OBJ_INFO ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO];
	if ( property == SUBSC_OBJ_TYPE ) 
		return arr[ ( id * SUBSC_OBJ_TYPE ) ]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_TYPE];
	return 0; //default
}

	
	
//Returns an object property from the global array. 
int GetSubscreenObjectProperty(int id, int property){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	return optr[ ( id * SUBSC_OBJ_LAST_INDEX ) + property ];
}

//Creates a subscreen object while defining its properties. 
void CreateSubscreenObject(item arr, int id, int x, int y, int item_id, int tile, int cset, int function, int info, int type, int up, int down, int left, int right){
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	id+=1;
	optr[ofs+SUBSC_OBJ_ID] = id;
	optr[ofs+SUBSC_OBJ_X] = x;
	optr[ofs+SUBSC_OBJ_Y] = y;
	optr[ofs+SUBSC_OBJ_ITEM] = item_id;
	optr[ofs+SUBSC_OBJ_FUNCTION] = function;
	optr[ofs+SUBSC_OBJ_TILE] = tile;
	optr[ofs+SUBSC_OBJ_CSET] = cset;
	optr[ofs+SUBSC_OBJ_NEXT_R] = right;
	optr[ofs+SUBSC_OBJ_NEXT_L] = left;
	optr[ofs+SUBSC_OBJ_NEXT_D] = down;
	optr[ofs+SUBSC_OBJ_NEXT_U] = up;
	optr[ofs+SUBSC_OBJ_TYPE] = type;
	optr[ofs+SUBSC_OBJ_INFO] = info;
	arr[id] = Screen->CreateItem(I_SUBSC_OBJECT);
	arr[id]->Tile = optr[ofs+SUBSC_OBJ_TILE];
	arr[id]->CSet = optr[ofs+SUBSC_OBJ_CSET];
	arr[id]->X = optr[ofs+SUBSC_OBJ_X];
	arr[id]->Y = optr[ofs+SUBSC_OBJ_Y];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT] = optr[ofs+SUBSC_OBJ_NEXT_R];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT] = optr[ofs+SUBSC_OBJ_NEXT_L];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN] = optr[ofs+SUBSC_OBJ_NEXT_D];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP] = optr[ofs+SUBSC_OBJ_NEXT_U];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_FUNCTION] = optr[ofs+SUBSC_OBJ_FUNCTION];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_INFO] = optr[ofs+SUBSC_OBJ_INFO];
	arr[id]->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_TYPE] = optr[ofs+SUBSC_OBJ_TYPE];
}

int RunSubscreenObjectFunction(int function, int id, int press){
	id+=1;
	int optr = GRAM[SUBSCREEN_OBJECTS];
	int ofs = id * SUBSC_OBJ_LAST_INDEX; //offset
	Trace(function); Trace(id); Trace(press);
	if ( function == 0 ) { return 0; }
	else if ( function == SubscSetItemA && press == 2 ) { 
		//Set Link's A-slot item
		Link->ItemA = optr[ ofs + SUBSC_OBJ_ITEM ];
		
		//Set the array index, so tha it is properly updated. 
		ButtonItems[BUTTONITEM_A] = optr[ ofs + SUBSC_OBJ_ITEM ];
		//Load the itemdata so that we can set the tile for the 
		//subscreen-A-Slot icon.
		//!Needs to be done inside the pane ffc. 
		//! id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemA);
		//Set the A-Slot icon on the subscreen with the proper tile
		//and cset.
		//equip[1]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
		//equip[1]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet; 
		Game->PlaySound(SUBSC_SELECTED_SFX);
		return optr[ ofs + SUBSC_OBJ_ITEM ]; }
	else if ( function == SubscSetItemB && press == 1 ) { 
		//Set Link's A-slot item
		Link->ItemB = optr[ ofs + SUBSC_OBJ_ITEM ];
		
		//Set the array index, so tha it is properly updated. 
		ButtonItems[BUTTONITEM_B] = optr[ ofs + SUBSC_OBJ_ITEM ];
		//Load the itemdata so that we can set the tile for the 
		//subscreen-A-Slot icon.
		//!Needs to be done inside the pane ffc. 
		//! id[SUBSC_IDATA_SCRATCH] = Game->LoadItemData(Link->ItemA);
		//Set the A-Slot icon on the subscreen with the proper tile
		//and cset.
		//equip[1]->Tile = id[SUBSC_IDATA_SCRATCH]->Tile;
		//equip[1]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet; 
		Game->PlaySound(SUBSC_SELECTED_SFX);
		return optr[ ofs + SUBSC_OBJ_ITEM ]; }
	else if ( function == SubscEquip && press ) { 
		Game->PlaySound(SUBSC_SFX_EQUIP);
		//Be sure to set that Link owns it...
		OwnsItems[ optr[ ofs + SUBSC_OBJ_ITEM ] ] = 1;
		//equip[0]->CSet = id[SUBSC_IDATA_SCRATCH]->CSet;
		//Equip/Unequip that item. 
		DoEnableDisable( optr[ ofs + SUBSC_OBJ_ITEM ] );
		return 1000; }
	else if ( function == SubscEquipClass && press ) { 
		//Debug
		
		itemdata itd = Game->LoadItemData( optr[ ofs + SUBSC_OBJ_ITEM ] );
		int ic = itd->Family; 
		
		for ( int q = 0; q < 256; q++ ) {
			itd = Game->LoadItemData(q);
			if ( Link->Item[q] && itd->Family == ic && q != optr[ ofs + SUBSC_OBJ_ITEM ] ) DoEnableDisable(q);
			//Disable enabled items, and enable the selected item
			DoEnableDisable(optr[ ofs + SUBSC_OBJ_ITEM ]);
		}
		
		return 2000; 
	}
	else if ( function == SubscInfo ) { return optr[ofs+SUBSC_OBJ_INFO]; }
	else Game->PlaySound(SFX_ERROR);
	
}


//void StoreSubscreenButtonItemsPtr(){ }

ffc script PlaceSubscreenObjectAndTrace{
	void run(int item_id_type, int tile, int cset, int function_info, int up, int down, int left, int right){
		Waitframe(); //Wait so that the global active script can report to the log, first. 
		int COMMA[]=", ";
		int LPAREN[]="(";
		int RPAREN[]=")";
		int IDENTIFIER[]="SetSubscreenObjectProperties";
		int SEMI[]=";";
		int x = this->X;
		int y = this->Y;
		int item_id = GetHighFloat(item_id_type);
		int type = GetLowFloat(item_id_type);
		int thisid = this->ID -1;
		int info = GetLowFloat(function_info);
		int function = GetHighFloat(function_info);
		if  ( tile < 0 ) { tile = Game->ComboTile(this->Data); }
		if ( cset < 0 ) cset = this->CSet;
		
		int S_ID[3]; itoa(S_ID, thisid);
		int S_X[10]; ftoa(S_X, x);
		int S_TYPE[2]; itoa(S_TYPE, type);
		int S_Y[10]; ftoa(S_Y,y);
		int S_IID[4]; itoa(S_IID, item_id);
		int S_ITILE[6]; itoa(S_ITILE, tile);
		int S_ICSET[3]; itoa(S_ICSET, cset);
		int S_FUNCT[2]; itoa(S_FUNCT, function);
		int S_INFO[7]; itoa(S_INFO, info);
		int S_UP[3]; itoa(S_UP, up);
		int S_DOWN[3]; itoa(S_DOWN, down);
		int S_RIGHT[3]; itoa(S_RIGHT, right);
		int S_LEFT[3]; itoa(S_LEFT, left);
		//Trace the InitD and the Position of this ffc. 
		TraceNL();
		TraceS(IDENTIFIER); TraceS(LPAREN); TraceS(S_ID); TraceS(COMMA); TraceS(S_X); TraceS(COMMA); 
		TraceS(S_Y); TraceS(COMMA); TraceS(S_IID); TraceS(COMMA); TraceS(S_ITILE); TraceS(COMMA); 
		TraceS(S_ICSET); TraceS(COMMA); TraceS(S_FUNCT); TraceS(COMMA); TraceS(S_INFO); TraceS(COMMA); 
		TraceS(S_TYPE); TraceS(COMMA); TraceS(S_UP); TraceS(COMMA); TraceS(S_DOWN); TraceS(COMMA); 
		TraceS(S_LEFT); TraceS(COMMA); TraceS(S_RIGHT); TraceS(RPAREN); TraceS(SEMI); TraceNL();
		
		int s1[]="CreateSubscreenObject(subscObjects, SubscGetFreeObj());";
		int s2[]="SubsNumObjects( SubscGetFreeObj() );";
		TraceNL(); TraceS(s1); TraceNL(); Trace(s2); TraceNL();
		//Do we need printf for this, to use args and other fancy stuff 	
		//so that out output is formatted as functions...or will TraceS suffice?
	}
}


//Set the nitial object from a pool of placed objects

//Select a placed object




int GetObjectSelected(){ return GRAM[SUBSC_OBJ_SELECTED]; }
void SetObjectSelected(int s) { GRAM[SUBSC_OBJ_SELECTED] = s; }




int SubscCursorMove(item cursor, item itemptrs){ //itemptrs is the items[32] array
	int optr = GetSubscreenPtr();
	item sel = Screen->LoadItem(GRAM[SUBSC_OBJ_SELECTED]); //optr[SUBSC_OBJ_SELECTED]);
	int dir[4]={ sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_UP], sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_DOWN], sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_LEFT], sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_MISC_RIGHT]};
	//itemptrs[32];
	int d1[]="Cursor selection: ";
	int fun[]="Object has a function of: ";
	//press right
	if ( Link->PressRight ) { 
		Game->PlaySound(SUBSC_CURSOR_SFX); 
		cursor->X = itemptrs[ dir[3] ]->X; 
		cursor->Y = itemptrs[ dir[3] ]->Y; 
		GRAM[SUBSC_OBJ_SELECTED] = dir[3];
		
		TraceS(fun); Trace(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION]);
	}
	//press left
	if ( Link->PressLeft ) { Game->PlaySound(SUBSC_CURSOR_SFX); cursor->X = itemptrs[ dir[2] ]->X; cursor->Y = itemptrs[ dir[2] ]->Y; GRAM[SUBSC_OBJ_SELECTED] = dir[2]; TraceS(fun); Trace(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION]);}
	//press down
	if ( Link->PressDown ) { Game->PlaySound(SUBSC_CURSOR_SFX);  cursor->X = itemptrs[ dir[1] ]->X; cursor->Y = itemptrs[ dir[1] ]->Y; GRAM[SUBSC_OBJ_SELECTED] = dir[1]; TraceS(fun); Trace(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION]);}
	//press up
	if ( Link->PressUp ) { Game->PlaySound(SUBSC_CURSOR_SFX); cursor->X = itemptrs[ dir[0] ]->X; cursor->Y = itemptrs[ dir[0] ]->Y; GRAM[SUBSC_OBJ_SELECTED] = dir[0]; TraceS(fun); Trace(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION]); }
	if ( Link->PressA ) { RunSubscreenObjectFunction(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION], optr[SUBSC_OBJ_SELECTED], 2);}

	if ( Link->PressB ) { RunSubscreenObjectFunction(sel->Misc[SUBSC_OBJ_MISC_OFFSET+SUBSC_OBJ_FUNCTION], optr[SUBSC_OBJ_SELECTED], 1);}
}

ffc script SubscreenCreator{
	void run(){
		while(true){
		//determine what object the mouse cursor is over
		//if selected, open a menu. 
		//Menu code
			Waitframe();
		}
	}
}