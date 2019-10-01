//std_functions.zh
//


//Returns the logarithm of x to the given base
float LogToBase(float x, float base){
	return Ln(x)/Ln(base);
}

//Returns the sign of n
int Sign(int n) {
	if (n > 0) return 1;
	else if (n < 0) return -1;
	else return 0;
}


//Finds the location of a combo, given its (x,y) coordinates on the screen
int ComboAt(int x, int y) {
	x = VBound(x,255,0);
	y = VBound(y,175,0);
	return (y & 240)+(x>>4);
}

//Snaps 'x' to the combo grid
//Equivalent to calling ComboX(ComboAt(x,foo));
int GridX(int x) {
	return (x >> 4) << 4;
}

//Snaps 'y' to the combo grid
//Equivalent to calling ComboY(ComboAt(foo,y));
int GridY(int y) {
	return (y >> 4) << 4;
}


//Returns the correct offset to be at the front of a sprite facing in the direction 'dir'
int AtFrontX(int dir) {

	int x = 0;
	if(dir == DIR_UP || dir == DIR_DOWN) x = 8;
	else if(dir == DIR_RIGHT) x = 16;
	return x;
}

int AtFrontY(int dir) {

	int y = 0;
	if(dir == DIR_DOWN) y = 16;
	else if(dir == DIR_LEFT || dir == DIR_RIGHT) y = 8;
	return y;
}

//Returns the correct offset to be 'dist' pixels away from the front of a sprite facing in the direction 'dir'
int InFrontX(int dir, int dist) {
	int x = 0;
	if(dir == DIR_LEFT) x = -16+dist;
	else if(dir == DIR_RIGHT) x = 16-dist;
	return x;
}

int InFrontY(int dir, int dist){
	int y = 0;
	if(dir == DIR_UP) y = -16+dist;
	else if(dir == DIR_DOWN) y = 16-dist;
	return y;
}

// Get the X and Y coordinates at the center of a sprite
int CenterX(ffc anFFC) { return anFFC->X+8*anFFC->TileWidth; }
int CenterY(ffc anFFC) { return anFFC->Y+8*anFFC->TileHeight; }
int CenterX(npc anNPC) { return anNPC->X+8*anNPC->TileWidth; }
int CenterY(npc anNPC) { return anNPC->Y+8*anNPC->TileHeight; }
int CenterX(eweapon anEWeapon) { return anEWeapon->X+8*anEWeapon->TileWidth; }
int CenterY(eweapon anEWeapon) { return anEWeapon->Y+8*anEWeapon->TileHeight; }
int CenterX(lweapon anLWeapon) { return anLWeapon->X+8*anLWeapon->TileWidth; }
int CenterY(lweapon anLWeapon) { return anLWeapon->Y+8*anLWeapon->TileHeight; }
int CenterLinkX() { return Link->X+8; }
int CenterLinkY() { return Link->Y+8; }

//Return the coordinates of a combo on the screen
int ComboX(int loc) {
	return loc%16*16;
}
int ComboY(int loc) {
	return loc&0xF0;
}

//Returns true if the combo at '(x, y)' has either an inherent or place flag of type 'flag'
bool ComboFI(int x, int y, int flag){

	int loc = ComboAt(x,y);

	return Screen->ComboF[loc] == flag || Screen->ComboI[loc] == flag;
}

//Returns true if the combo at 'loc' has either an inherent or place flag of type 'flag'
bool ComboFI(int loc, int flag){

	return Screen->ComboF[loc] == flag || Screen->ComboI[loc] == flag;
}

//Sets bit 'bit' of Screen->D[] register 'd' to 'state'
void SetScreenDBit(int dmap, int screen, int d, int bit, bool state){
	int curstate = Game->GetDMapScreenD(dmap, screen, d);
	if(state)	Game->SetDMapScreenD(dmap, screen, d, curstate |  (1 << bit));
	else 		Game->SetDMapScreenD(dmap, screen, d, curstate & ~(1 << bit));
}
void SetScreenDBit(int screen, int d, int bit, bool state){
	int curstate = Game->GetScreenD(screen, d);
	if(state)	Game->SetScreenD(screen, d, curstate |  (1 << bit));
	else 		Game->SetScreenD(screen, d, curstate & ~(1 << bit));
}
void SetScreenDBit(int d, int bit, bool state){
	if(state)	Screen->D[d] |= (1 << bit);
	else		Screen->D[d] &= ~(1 << bit);
}

//Returns the state of bit 'bit' of Screen->D[] register 'd'
bool GetScreenDBit(int dmap, int screen, int d, int bit){
	return ( Game->GetDMapScreenD(dmap, screen, d) & (1 << bit) ) != 0;
}
bool GetScreenDBit(int screen, int d, int bit){
	return ( Game->GetScreenD(screen, d) & (1 << bit) ) != 0;
}
bool GetScreenDBit(int d, int bit){
	return (Screen->D[d] & (1 << bit)) != 0;
}

//A shorthand way to get a combo on the current layer.
//Layer 0 is the screen itself.
int GetLayerComboD(int layer, int pos) {
	if (layer==0)
		return Screen->ComboD[pos];
	else
		return Game->GetComboData(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos);
}

//A shorthand way to set a combo on the current layer.
//Layer 0 is the screen itself.
void SetLayerComboD(int layer, int pos, int combo) {
	if (layer == 0)
		Screen->ComboD[pos] = combo;
	else
		Game->SetComboData(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos, combo);
}

//A shorthand way to get a combo flag on the current layer.
//Layer 0 is the screen itself.
int GetLayerComboF(int layer, int pos) {
	if (layer==0)
		return Screen->ComboF[pos];
	else
		return Game->GetComboFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos);
}

//A shorthand way to set a combo flag on the current layer.
//Layer 0 is the screen itself.
void SetLayerComboF(int layer, int pos, int flag) {
	if (layer == 0)
		Screen->ComboF[pos] = flag;
	else
		Game->SetComboFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos, flag);
}

//A shorthand way to get a combo type on the current layer.
//Layer 0 is the screen itself.
int GetLayerComboT(int layer, int pos) {
	if (layer==0)
		return Screen->ComboT[pos];
	else
		return Game->GetComboType(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos);
}

//A shorthand way to set a combo type on the current layer.
//Layer 0 is the screen itself.
void SetLayerComboT(int layer, int pos, int type) {
	if (layer == 0)
		Screen->ComboT[pos] = type;
	else
		Game->SetComboType(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos, type);
}

//A shorthand way to get a combo's solidity on the current layer.
//Layer 0 is the screen itself.
int GetLayerComboS(int layer, int pos) {
	if (layer==0)
		return Screen->ComboS[pos];
	else
		return Game->GetComboSolid(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos);
}

//A shorthand way to set a combo's solidity on the current layer.
//Layer 0 is the screen itself.
void SetLayerComboS(int layer, int pos, int solidity) {
	if (layer == 0)
		Screen->ComboS[pos] = solidity;
	else
		Game->SetComboSolid(Screen->LayerMap(layer), Screen->LayerScreen(layer), pos, solidity);
}

//Copies the combos and csets from one screen to another.
//Only copies layer 0!
void ScreenCopy(int destmap, int destscr, int srcmap, int srcscr) {
	for (int i = 0; i < 176; i++) {
		Game->SetComboData(destmap,destscr, i, Game->GetComboData(srcmap,srcscr,i));
		Game->SetComboCSet(destmap,destscr, i, Game->GetComboCSet(srcmap,srcscr,i));
	}
}

//Swaps a row of tiles of length 'length' between positions 'first' and 'second'
void SwapTileRow(int first, int second, int length){
	for(int i=0;i<length;i++) SwapTile(first+i,second+i);
}

//Copies a row of tiles of length 'length' from 'source' onto 'dest'
void CopyTileRow(int source, int dest, int length){
	for(int i=0;i<length;i++) CopyTile(source+i,dest+i);
}

//Clears a row of tiles of length 'length' starting from tile 'ref'
void ClearTileRow(int ref, int length){
	for(int i=0;i<length;i++) ClearTile(ref+i);
}

//Swaps a block of tiles defined by diagonal corners 'first' and 'last'
//with the block starting with top left tile 'second'
void SwapTileBlock(int first, int last, int second){
	if(last < first){
		int swap = first;
		first = last;
		last = swap;
	}
	int w = last%20-first%20;
	if(w < 0){
		first -= w;
		last += w;
		w = -w;
	}
	for(int i=0;i<=last-first;i++) if(i%20 <= w) SwapTile(first+i,second+i);
}

//Copies a block of tiles defined by diagonal corners 'sourcefirst' and 'sourcelast'
//onto the block starting with top left tile 'destfirst'
void CopyTileBlock(int sourcefirst, int sourcelast, int destfirst){
	if(sourcelast < sourcefirst){
		int swap = sourcefirst;
		sourcefirst = sourcelast;
		sourcelast = swap;
	}
	int w = sourcelast%20-sourcefirst%20;
	if(w < 0){
		sourcefirst -= w;
		sourcelast += w;
		w = -w;
	}
	for(int i=0;i<=sourcelast-sourcefirst;i++) if(i%20 <= w) CopyTile(sourcefirst+i,destfirst+i);
}

//Clears a block of tiles defined by diagonal corners 'reffirst' and 'reflast'
void ClearTileBlock(int reffirst, int reflast){
	if(reflast < reffirst){
		int swap = reffirst;
		reffirst = reflast;
		reflast = swap;
	}
	int w = reflast%20-reffirst%20;
	if(w < 0){
		reffirst -= w;
		reflast += w;
		w = -w;
	}
	for(int i=0;i<=reflast-reffirst;i++) if(i%20 <= w) ClearTile(reffirst+i);
}



// Converts a counterclockwise degree angle (from -180 to 180) into one of the eight
// standard directions (DIR_UP etc.) used by ZC.
int AngleDir8(float angle) {
	if (angle <= 157.5 && angle > 112.5)
		return DIR_LEFTDOWN;
	else if (angle <= 112.5 && angle > 67.5)
		return DIR_DOWN;
	else if (angle <= 67.5 && angle > 22.5)
		return DIR_RIGHTDOWN;
	else if (angle <= 22.5 && angle > -22.5)
		return DIR_RIGHT;
	else if (angle <= -22.5 && angle > -67.5)
		return DIR_RIGHTUP;
	else if (angle <= -67.5 && angle > -112.5)
		return DIR_UP;
	else if (angle <= -112.5 && angle > -157.5)
		return DIR_LEFTUP;
	else
		return DIR_LEFT;
}

//The above, but for radian angles.
int RadianAngleDir8(float angle) {
	return AngleDir8(angle*57.2958);
}

// Converts a counterclockwise degree angle (from -180 to 180) into one of the four
// standard directions (DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT) used by ZC.
int AngleDir4(float angle) {
	if (angle <= 135 && angle > 45)
		return DIR_DOWN;
	else if (angle <= 45 && angle > -45)
		return DIR_RIGHT;
	else if (angle <= -45 && angle > -135)
		return DIR_UP;
	else
		return DIR_LEFT;
}

//The above, but for radian angles.
int RadianAngleDir4(float angle) {
	return AngleDir4(angle*57.2958);
}

//Returns the opposite direction to angle 'dir'
int OppositeDir(int dir) {
	return Cond(dir < 4, dir^1b, dir^11b);
}

//Converts directions to go round in a circle rather than U, D, L, R
int SpinDir(int dir) {
	if(dir==0)
		return DIR_UP;
	else if(dir==1)
		return DIR_RIGHT;
	else if(dir==2)
		return DIR_DOWN;
	else if(dir==3)
		return DIR_LEFT;
	return -1;
}

//Generalized and optimized rectangle collision checking function.
//Returns true if the bounding box of box1 and box2 overlap.
bool RectCollision(int box1_x1, int box1_y1, int box1_x2, int box1_y2, int box2_x1, int box2_y1, int box2_x2, int box2_y2) {
	if( box1_y2 < box2_y1 ) return false;
	else if( box1_y1 > box2_y2 ) return false;
	else if( box1_x2 < box2_x1 ) return false;
	else if( box1_x1 > box2_x2 ) return false;
	return true;
}

//Check for collisions of two squares given upper-left coordinates and a side length for each.
bool SquareCollision(int c1x, int c1y, int side1, int c2x, int c2y, int side2) {
	return RectCollision(c1x, c1y, c1x+side1, c1y+side1, c2x, c2y, c2x+side1, c2y+side1);
}

//Check for collisions of two squares given center coordinates and a halved side length for each.
bool SquareCollision2(int c1x, int c1y, int radius1, int c2x, int c2y, int radius2) {
	if( c1y + radius1 < c2y - radius2 ) return false;
	else if( c1y - radius1 > c2y + radius2 ) return false;
	else if( c1x + radius1 < c2x - radius2 ) return false;
	else if( c1x - radius1 > c2x + radius2 ) return false;
	return true;
}

//Returns true if the two circles c1 and c2 overlap.
bool CircleCollision(int c1x, int c1y, int radius1, int c2x, int c2y, int radius2) {
	return (Distance(c1x,c1y,c2x,c2y) <= (radius1+radius2));
}

//Returns true if there is a collision between the hitboxes of an lweapon and an eweapon.
bool Collision(lweapon a, eweapon b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an lweapon and an lweapon.
bool Collision(lweapon a, lweapon b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an eweapon and an eweapon.
bool Collision(eweapon a, eweapon b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an lweapon and an npc.
bool Collision(lweapon a, npc b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an eweapon and an npc.
bool Collision(eweapon a, npc b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an npc and an npc.
bool Collision(npc a, npc b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an item and an lweapon.
bool Collision(item a, lweapon b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an item and an eweapon.
bool Collision(item a, eweapon b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an item and an npc.
bool Collision(item a, npc b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an item and an item.
bool Collision(item a, item b) {
	int ax = a->X + a->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = a->Y + a->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+a->HitWidth, ay+a->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (a->Z + a->HitZHeight >= b->Z) && (a->Z <= b->Z + b->HitZHeight);
}

//A collision between an ffc and an lweapon. Uses TileWidth and TileHeight for the FFC's bounding box. Ignores the Z axis.
bool Collision(ffc f, lweapon b) {
	int bx = b->X + b->HitXOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), bx, by, bx+b->HitWidth, by+b->HitHeight);
}

//A collision between an ffc and an eweapon. Uses TileWidth and TileHeight for the FFC's bounding box. Ignores the Z axis.
bool Collision(ffc f, eweapon b) {
	int bx = b->X + b->HitXOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), bx, by, bx+b->HitWidth, by+b->HitHeight);
}

//A collision between an ffc and an npc. Uses TileWidth and TileHeight for the FFC's bounding box. Ignores the Z axis.
bool Collision(ffc f, npc b) {
	int bx = b->X + b->HitXOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), bx, by, bx+b->HitWidth, by+b->HitHeight);
}

//A collision between an ffc and an item. Uses TileWidth and TileHeight for the FFC's bounding box. Ignores the Z axis.
bool Collision(ffc f, item b) {
	int bx = b->X + b->HitXOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), bx, by, bx+b->HitWidth, by+b->HitHeight);
}

//A collision between an ffc and an ffc. Uses TileWidth and TileHeight for the FFCs' bounding boxes.
bool Collision(ffc f, ffc f2) {
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), f2->X, f2->Y, f2->X+f2->TileWidth*16, f2->Y+f2->TileHeight*16);
}

//A circular collision between an ffc and an ffc. Uses TileWidth and TileHeight to find the centre of the FFCs.
bool Collision(ffc f, int radius1, ffc f2, int radius2) {
	return CircleCollision(f->X+f->TileWidth/2, f->Y+f->TileHeight/2, radius1,f2->X+f2->TileWidth/2, f2->Y+f2->TileHeight/2, radius2);
}

// So that you don't have to remember the ordering of the args
bool Collision(eweapon a, lweapon b) {
	return Collision(b, a);
}

bool Collision(npc a, lweapon b) {
	return Collision(b, a);
}

bool Collision(npc a, eweapon b) {
	return Collision(b, a);
}

bool Collision(lweapon a, item b) {
	return Collision(b, a);
}

bool Collision(eweapon a, item b) {
	return Collision(b, a);
}

bool Collision(npc a, item b) {
	return Collision(b, a);
}

bool Collision(lweapon a, ffc b) {
	return Collision(b, a);
}

bool Collision(eweapon a, ffc b) {
	return Collision(b, a);
}

bool Collision(npc a, ffc b) {
	return Collision(b, a);
}

bool Collision(item a, ffc b) {
	return Collision(b, a);
}

// Returns true if there is a collision between Link's hitbox and the eweapon's.
// This only checks hitboxes.
bool LinkCollision(eweapon b) {
	int ax = Link->X + Link->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = Link->Y + Link->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+Link->HitWidth, ay+Link->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (Link->Z + Link->HitZHeight >= b->Z) && (Link->Z <= b->Z + b->HitZHeight);
}

// Returns true if there is a collision between Link's hitbox and the lweapon's.
// This only checks hitboxes.
bool LinkCollision(lweapon b) {
	int ax = Link->X + Link->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = Link->Y + Link->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+Link->HitWidth, ay+Link->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (Link->Z + Link->HitZHeight >= b->Z) && (Link->Z <= b->Z + b->HitZHeight);
}

// Returns true if there is a collision between Link's hitbox and the item's.
// This only checks hitboxes.
bool LinkCollision(item b) {
	int ax = Link->X + Link->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = Link->Y + Link->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+Link->HitWidth, ay+Link->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (Link->Z + Link->HitZHeight >= b->Z) && (Link->Z <= b->Z + b->HitZHeight);
}

// Returns true if there is a collision between Link's hitbox and the npc's.
// This only checks hitboxes.  Uses TileWidth and TileHeight to find the centre of the FFCs.
bool LinkCollision(npc b) {
	int ax = Link->X + Link->HitXOffset;
	int bx = b->X + b->HitXOffset;
	int ay = Link->Y + Link->HitYOffset;
	int by = b->Y + b->HitYOffset;
	return RectCollision(ax, ay, ax+Link->HitWidth, ay+Link->HitHeight, bx, by, bx+b->HitWidth, by+b->HitHeight) && (Link->Z + Link->HitZHeight >= b->Z) && (Link->Z <= b->Z + b->HitZHeight);
}

// Returns true if there is a collision between Link's hitbox and the FFC's.
// This only checks hitboxes.
bool LinkCollision(ffc f) {
	int ax = Link->X + Link->HitXOffset;
	int ay = Link->Y + Link->HitYOffset;
	return RectCollision(f->X, f->Y, f->X+(f->TileWidth*16), f->Y+(f->TileHeight*16), ax, ay, ax+Link->HitWidth, ay+Link->HitHeight);
}

//Functions for those who are not comfortable with binary
//Returns true if the left mouse button is pressed
bool InputLeftClick() {
	return (Link->InputMouseB&MB_LEFTCLICK) != 0;
}

//Returns true if the right mouse button is pressed
bool InputRightClick() {
	return (Link->InputMouseB&MB_RIGHTCLICK) != 0;
}

//Returns the item ID of the item equipped to Link's A button
int GetEquipmentA() {
	return (Link->Equipment&0xFF);
}


//Returns the item ID of the item equipped to Link's B button
int GetEquipmentB() {
	return ((Link->Equipment&0xFF00)>>8);
}

//Returns true if Link is using item 'id'
bool UsingItem(int id){
	return (GetEquipmentA() == id && Link->InputA) || (GetEquipmentB() == id && Link->InputB);
}

//Returns the number of Triforce Pieces Link currently has
int NumTriforcePieces(){
	int ret = 0;
	for(int i=1;i<=8;i++)
		if(Game->LItems[i]&LI_TRIFORCE) ret++;
	return ret;
}

//Returns 1 if Screen Flag 'flag' is set from category 'category', 0 if it's not and -1 if an invalid flag is passed
//Flags are numbered starting from 0
int ScreenFlag(int category, int flag) {
	int catsizes[] = {3,7,5,3,2,4,4,2,3,7};
	if(flag < 0 || flag >= catsizes[category]) return -1;
	return Screen->Flags[category]&(1<<flag);
}

//Returns 1 if Screen Enemy Flag 'flag' is set from category 'category', 0 if it's not and -1 if an invalid flag is passed
//Flags are numbered starting from 0
int ScreenEFlag(int category, int flag) {
	int catsizes[] = {6,6,5};
	if(flag < 0 || flag >= catsizes[category]) return -1;
	return Screen->EFlags[category]&(1<<flag);
}

//Returns true if DMap Flag 'flag' is set on dmap 'dmap'
bool GetDMapFlag(int dmap, int flag){
	return (Game->DMapFlags[dmap]&flag)!=0;
}

//Sets a certain DMap flag to 'state'
void SetDMapFlag(int dmap, int flag, bool state){
	if(state) Game->DMapFlags[dmap] |= flag;
	else Game->DMapFlags[dmap] &= ~flag;
}

//Returns true if an item's Pickup state is set
//Use the IP_ constants for the 'pickup' argument of this function
bool GetItemPickup(item i, int pickup) {
	return (i->Pickup&pickup) != 0;
}

//Sets an item's Pickup state to 'state'
void SetItemPickup(item i, int pickup, bool state) {
	if(state) i->Pickup |= pickup;
	else i->Pickup &= ~pickup;
}

//Returns true if an npc's Misc. flag is set.
bool GetNPCMiscFlag(npc e, int flag) {
	return (e->MiscFlags&flag) != 0;
}

//Returns true if Link has the level item 'itm' from level 'level'
//Overloaded to use the current level if no 'level' arg is entered
//Use the LI_ constants for the 'itm' argument
bool GetLevelItem(int level, int itm) {
	return (Game->LItems[level]&itm) != 0;
}
bool GetLevelItem(int itm) {
	return (Game->LItems[Game->GetCurLevel()]&itm) != 0;
}
//Gives or removes a level item from Link's inventory
void SetLevelItem(int level, int itm, bool state) {
	if(state) Game->LItems[level] |= itm;
	else Game->LItems[level] &= ~itm;
}
void SetLevelItem(int itm, bool state) {
	if(state) Game->LItems[Game->GetCurLevel()] |= itm;
	else Game->LItems[Game->GetCurLevel()] &= ~itm;
}

//Create an NPC and set its X and Y position in one command
npc CreateNPCAt(int id, int x, int y) {
	npc nme = Screen->CreateNPC(id);
	if(nme->isValid()) {
		nme->X = x;
		nme->Y = y;
	}
	return nme;
}

//Create an Item and set its X and Y position in one command
item CreateItemAt(int id, int x, int y) {
	item it = Screen->CreateItem(id);
	if(it->isValid()) {
		it->X = x;
		it->Y = y;
	}
	return it;
}

//Create an LWeapon and set its X and Y position in one command
lweapon CreateLWeaponAt(int id, int x, int y) {
	lweapon lw = Screen->CreateLWeapon(id);
	if(lw->isValid()) {
		lw->X = x;
		lw->Y = y;
	}
	return lw;
}

//Create an EWeapon and set its X and Y position in one command
eweapon CreateEWeaponAt(int id, int x, int y) {
	eweapon ew = Screen->CreateEWeapon(id);
	if(ew->isValid()) {
		ew->X = x;
		ew->Y = y;
	}
	return ew;
}


//Creates an lweapon at 'distx,disty' away from where Link is facing
lweapon NextToLink(int id, int distx, int disty) {
	lweapon l = CreateLWeaponAt(id, Link->X+InFrontX(Link->Dir, distx), Link->Y+InFrontY(Link->Dir, disty));
	if(l->isValid()) {
		l->Dir = Link->Dir;
	}
	return l;
}

//Creates an lweapon 'dist' pixels away from the front of Link
lweapon NextToLink(int id, int dist) {
	return NextToLink(id, dist, dist);
}

eweapon NextToNPC(npc n, int id, int distx, int disty) {
	eweapon e = CreateEWeaponAt(id, n->X+InFrontX(n->Dir, distx), n->Y+InFrontY(n->Dir, disty));
	if(e->isValid()) {
		e->Dir = n->Dir;
	}
	return e;
}
eweapon NextToNPC(npc n, int id, int dist) {
	return NextToNPC(n, id, dist, dist);
}


//Aim-type constants, for use with AimEWeapon
const int AT_NONE			= 0;
const int AT_4DIR			= 1;
const int AT_8DIR			= 2;
const int AT_ANGULAR		= 3;
const int AT_RAND4DIR		= 4;
const int AT_RAND8DIR		= 5;
const int AT_RANDANGULAR	= 6;

//Various methods for shooting at Link and at random
void AimEWeapon(eweapon e, int aimtype)
{
	int angle = RadianAngle(e->X, e->Y, Link->X, Link->Y);
	if(aimtype == AT_4DIR)
	{
		e->Dir = RadianAngleDir4(angle);
	}
	else if(aimtype == AT_8DIR)
	{
		e->Dir = RadianAngleDir8(angle);
	}
	else if(aimtype == AT_ANGULAR)
	{
		e->Angular = true;
		e->Angle = angle;
		e->Dir = RadianAngleDir8(angle);
	}
	else if(aimtype == AT_RAND4DIR)
	{
		e->Dir = Rand(4);
	}
	else if(aimtype == AT_RAND8DIR)
	{
		e->Dir = Rand(8);
	}
	else if(aimtype == AT_RANDANGULAR)
	{
		e->Angular = true;
		e->Angle = Randf(PI2);
		e->Dir = RadianAngleDir8(e->Angle);
	}
}

//Aiming at enemies and at random
void AimLWeapon(lweapon l, npc n, int aimtype)
{
	int angle = RadianAngle(l->X, l->Y, n->X, n->Y);
	if(aimtype == AT_4DIR)
	{
		l->Dir = RadianAngleDir4(angle);
	}
	else if(aimtype == AT_8DIR)
	{
		l->Dir = RadianAngleDir8(angle);
	}
	else if(aimtype == AT_ANGULAR)
	{
		l->Angular = true;
		l->Angle = angle;
		l->Dir = RadianAngleDir8(angle);
	}
	else
		AimLWeapon(l, aimtype);
}
void AimLWeapon(lweapon l, int aimtype)
{
	if(aimtype == AT_RAND4DIR)
	{
		l->Dir = Rand(4);
	}
	else if(aimtype == AT_RAND8DIR)
	{
		l->Dir = Rand(8);
	}
	else if(aimtype == AT_RANDANGULAR)
	{
		l->Angular = true;
		l->Angle = Randf(PI2);
		l->Dir = RadianAngleDir8(l->Angle);
	}
}

//Turns a WPN_ constant to an EW_ constant
int WeaponTypeToID(int wpnt)
{
	if(wpnt == WPN_ENEMYFLAME) 		return EW_FIRE;
	else if(wpnt == WPN_ENEMYWIND)		return EW_WIND;
	else if(wpnt == WPN_ENEMYFIREBALL)	return EW_FIREBALL;
	else if(wpnt == WPN_ENEMYARROW)		return EW_ARROW;
	else if(wpnt == WPN_ENEMYBRANG)		return EW_BRANG;
	else if(wpnt == WPN_ENEMYSWORD)		return EW_BEAM;
	else if(wpnt == WPN_ENEMYROCK)		return EW_ROCK;
	else if(wpnt == WPN_ENEMYMAGIC)		return EW_MAGIC;
	else if(wpnt == WPN_ENEMYBOMB)		return EW_BOMB;
	else if(wpnt == WPN_ENEMYSBOMB)		return EW_SBOMB;
	else if(wpnt == WPN_ENEMYLITBOMB)	return EW_BOMBBLAST;
	else if(wpnt == WPN_ENEMYLITSBOMB)	return EW_SBOMBBLAST;
	else if(wpnt == WPN_ENEMYFIRETRAIL)	return EW_FIRETRAIL;
	else if(wpnt == WPN_ENEMYFLAME2)	return EW_FIRE2;
	else if(wpnt == WPN_ENEMYFIREBALL2)	return EW_FIREBALL2;
	return -1;
}


//Creates a timeout item (like a rupee or heart)
item CreateTimeoutItem(int id, int x, int y) {
	item Spawn = Screen->CreateItem(id);
	SetItemPickup(Spawn, IP_TIMEOUT, true);
	Spawn->HitWidth = 16; Spawn->HitHeight = 16;
	Spawn->X = x; Spawn->Y = y;
}

// Use the I_ constants.
// Warning: these iterate over every onscreen item.
// Iterating over every onscreen lweapon multiple times per frame may
// cause slowdown in Zelda Classic.
int NumItemsOf(int type) {
	int ret = 0;
	item it;
	for (int i = Screen->NumItems(); i >0 ; i--) {
		it = Screen->LoadItem(i);
		if (it->ID == type)
			ret++;
	}
	return ret;
}


// Use the LW_ constants.
// Warning: these iterate over every onscreen lweapon.
// Iterating over every onscreen lweapon multiple times per frame may
// cause slowdown in Zelda Classic.
int NumLWeaponsOf(int type) {
	int ret = 0;
	lweapon w;
	for (int i = Screen->NumLWeapons(); i > 0; i--) {
		w = Screen->LoadLWeapon(i);
		if (w->ID == type)
			ret++;
	}
	return ret;
}

int NumEWeaponsOf(int type) {
	int ret = 0;
	eweapon w;
	for (int i = Screen->NumEWeapons(); i > 0; i--) {
		w = Screen->LoadEWeapon(i);
		if (w->ID == type)
			ret++;
	}
	return ret;
}

int NumNPCsOf(int type) {
	int ret = 0;
	npc n;
	for (int i = Screen->NumNPCs(); i > 0 ; i--) {
		n = Screen->LoadNPC(i);
		if (n->ID == type)
			ret++;
	}
	return ret;
}

// Returns the first LWeapon of the given type. Use the LW_ constants.
// If none exist, it returns an uninitialised pointer.
lweapon LoadLWeaponOf(int type) {
	lweapon w;
	for (int i=1; i <= Screen->NumLWeapons(); i++) {
		w = Screen->LoadLWeapon(i);
		if (w->ID == type) return w;
	}
	lweapon w2;
	return w2;
}

// Returns the first EWeapon of the given type. Use the EW_ constants.
// If none exist, it returns an uninitialised pointer.
eweapon LoadEWeaponOf(int type) {
	eweapon w;
	for (int i=1; i <= Screen->NumEWeapons(); i++) {
		w = Screen->LoadEWeapon(i);
		if (w->ID == type) return w;
	}
	eweapon w2;
	return w2;
}

// Returns the first NPC of the given type. Use the NPCT_ constants.
// If none exist, it returns an uninitialised pointer.
npc LoadNPCOfType(int type) {
	npc n;
	for (int i=1; i <= Screen->NumNPCs(); i++) {
		n = Screen->LoadNPC(i);
		if (n->Type == type) return n;
	}
	npc n2;
	return n2;
}

// Returns the first NPC of the given ID. Use the NPC_ constants.
// If none exist, it returns an uninitialised pointer.
npc LoadNPCOf(int type) {
	npc n;
	for (int i=1; i <= Screen->NumNPCs(); i++) {
		n = Screen->LoadNPC(i);
		if (n->ID == type) return n;
	}
	npc n2;
	return n2;
}

// Returns true if the combo at the given position is water.
bool IsWater(int position)
{
	int combo=Screen->ComboT[position];
	if(combo==CT_WATER || combo==CT_SWIMWARP || combo==CT_DIVEWARP || (combo>=CT_SWIMWARPB && combo<=CT_DIVEWARPD))
		return true;
	else
		return false;
}

// Returns true if the combo at the given position is a pit.
bool IsPit(int position)
{
	int combo=Screen->ComboT[position];
	if(combo==CT_PIT || combo==CT_PITR || (combo>=CT_PITB && combo<=CT_PITD))
		return true;
	else
		return false;
}

//This should allow any scripted object to easily mimic Link styled LOZ solidity collision
//checking, be it Link, FFCs, or enemies.
//Note - You should use full_tile=true if you don't want the upper eight pixels to overlap
//solid combos as per LOZ1 behavior.
bool CanWalk(int x, int y, int dir, int step, bool full_tile) {
	int c=8;
	int xx = x+15;
	int yy = y+15;
	if(full_tile) c=0;
	if(dir==0) return !(y-step<0||Screen->isSolid(x,y+c-step)||Screen->isSolid(x+8,y+c-step)||Screen->isSolid(xx,y+c-step));
	else if(dir==1) return !(yy+step>=176||Screen->isSolid(x,yy+step)||Screen->isSolid(x+8,yy+step)||Screen->isSolid(xx,yy+step));
	else if(dir==2) return !(x-step<0||Screen->isSolid(x-step,y+c)||Screen->isSolid(x-step,y+c+7)||Screen->isSolid(x-step,yy));
	else if(dir==3) return !(xx+step>=256||Screen->isSolid(xx+step,y+c)||Screen->isSolid(xx+step,y+c+7)||Screen->isSolid(xx+step,yy));
	return false; //invalid direction
}

//Returns true if Link is on a sideview screen
bool IsSideview() {
	return Screen->Flags[SF_ROOMTYPE] & 4;
}

//Returns true if the sprite at (x,y) is standing on a sideview platform on a sideview screen, as worked out
//by ZC's internal code.
//For 16 pixel high sprites only.
bool OnSidePlatform(int x, int y) {
	return (Screen->isSolid(x+4,y+16) && Screen->isSolid(x+12,y+16) && Screen->Flags[SF_ROOMTYPE]&4);
}

//Returns true if a sprite of height 'h' at position (x,y) with an offset of (xOff,yOff) is standing
//on a sideview platform on a sideview screen.
bool OnSidePlatform(int x, int y, int xOff, int yOff, int h) {
	return (Screen->isSolid((x+xOff)+4,(y+yOff)+h) && Screen->isSolid((x+xOff)+12,(y+yOff)+h) && Screen->Flags[SF_ROOMTYPE]&4);
}

//Get an NPC's name from an ID
void GetNPCName(int ID, int string)
{
	npc n = Screen->CreateNPC(ID);
	n->GetName(string);
	Remove(n);
}

void GetMessage(int ID, int string)
{
	Game->GetMessage(ID, string);
	int i;
	for(i = MAX_MESSAGELENGTH-2; string[i] == ' '; i--);
	string[i+1] = 0;
}

itemdata GetItemData(item i)
{
	return Game->LoadItemData(i->ID);
}

int GetHighestLevelItem(int itemclass)
{
	itemdata id;
	int ret = -1;
	int curlevel = -1000;
	//143 is default max items, increase if you add lots of your own
	for(int i = 0; i < 143; i++)
	{
		id = Game->LoadItemData(i);
		if(id->Family != itemclass)
			continue;
		if(id->Level > curlevel)
		{
			curlevel = id->Level;
			ret = i;
		}
	}
	return ret;
}

int GetHighestLevelItem(item i)
{
	itemdata argid = GetItemData(i);
	int ret = i->ID;
	int curlevel = argid->Level;
	itemdata id;
	//143 is max items, decrease to improve speed if you need to
	for(int i = 0; i < 256; i++)
	{
		id = Game->LoadItemData(i);
		if(id->Family != argid->Family)
			continue;
		if(id->Level > curlevel)
		{
			curlevel = id->Level;
			ret = i;
		}
	}
	return ret;
}

// Convert between map and DMap screens
int DMapToMap(int screen, int dmap) {
	return screen+Game->DMapOffset[dmap];
}

int MapToDMap(int screen, int dmap) {
	return screen-Game->DMapOffset[dmap];
}

