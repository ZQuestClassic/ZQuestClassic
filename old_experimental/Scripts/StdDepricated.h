

//Returns a if cond is true, else b. Overloaded.
float Cond(bool cond, float a, float b) {
	if (cond) return a;
	else return b;
}
bool Cond(bool cond, bool a, bool b) {
	if (cond) return a;
	else return b;
}
npc Cond(bool cond, npc a, npc b) {
	if (cond) return a;
	else return b;
}
item Cond(bool cond, item a, item b) {
	if (cond) return a;
	else return b;
}
lweapon Cond(bool cond, lweapon a, lweapon b) {
	if (cond) return a;
	else return b;
}
eweapon Cond(bool cond, eweapon a, eweapon b) {
	if (cond) return a;
	else return b;
}
ffc Cond(bool cond, ffc a, ffc b) {
	if (cond) return a;
	else return b;
}
itemdata Cond(bool cond, itemdata a, itemdata b) {
	if (cond) return a;
	else return b;
}

// Chooses one of the options randomly and fairly.
float Choose(float a, float b) {
	if (Rand(2)==0) return a;
	else return b;
}

float Choose(float a, float b, float c) {
	int r = Rand(3);
	if (r==0) return a;
	else if (r==1) return b;
	else return c;
}

float Choose(float a, float b, float c, float d) {
	int r = Rand(4);
	if (r==0) return a;
	else if (r==1) return b;
	else if (r==2) return c;
	else return d;
}

float Choose(float a, float b, float c, float d, float e) {
	int r = Rand(5);
	if (r==0) return a;
	else if (r==1) return b;
	else if (r==2) return c;
	else if (r==3) return d;
	else return e;
}

float Choose(float a, float b, float c, float d, float e, float f) {
	int r = Rand(6);
	if (r==0) return a;
	else if (r==1) return b;
	else if (r==2) return c;
	else if (r==3) return d;
	else if (r==4) return e;
	else return f;
}




//Returns true if the integer is odd (or even in IsEven)
bool IsOdd(int x)
{
	return ((x & 1) == 1);
}

bool IsEven(int x)
{
	return ((x & 1) == 0);
}




//same as Clamp, but with reversed values.
int VBound(int x, int high, int low) {
	if(x<low) x=low;
	else if(x>high) x=high;
	return x;
}

//Returns the Quotient only of x divided by y
int Div(float x, float y) {
	return (x/y)<<0;
}


//Overload to Screen->DrawString which includes a position to start drawing from in the string
//Does not check for overflow
void DrawString(int layer, int x, int y, int font, int color, int background_color, int format, int opacity,
				int string, int start){
					int buffer[256];
					for(int i=start;string[i]!=0;i++) buffer[i-start] = string[i];
					Screen->DrawString(layer,x,y,font,color,background_color,format,opacity,buffer);
}

//Overload to Screen->DrawString which includes a start and end position to draw the string
//Does not check for overflow
void DrawString(int layer, int x, int y, int font, int color, int background_color, int format, int opacity,
				int string, int start, int end){
					int buffer[256];
					for(int i=start;i<end;i++) buffer[i-start] = string[i];
					Screen->DrawString(layer,x,y,font,color,background_color,format,opacity,buffer);
}


// A very simple layer 0 tile drawing routine.
void DrawTileSimple(int x, int y, int tile, int color)
{

	Screen->FastTile(0, x, y, tile, color, 128);
}


//Draws an ffc to a given layer. If the ffc is larger than 1x1 its graphics must all be comboed
void DrawToLayer(ffc f, int layer, int opacity){
	Screen->DrawCombo(layer,f->X,f->Y,f->Data,f->TileWidth,f->TileHeight,f->CSet,-1,-1,0,0,0,-1,0,true,opacity);
}

//Draws an npc to a given layer
void DrawToLayer(npc n, int layer, int opacity){
	Screen->DrawTile(layer,n->X,n->Y,n->Tile,n->TileWidth,n->TileHeight,n->CSet,-1,-1,0,0,0,0,true,opacity);
}

//Draws an lweapon to a given layer
void DrawToLayer(lweapon l, int layer, int opacity){
	Screen->DrawTile(layer,l->X,l->Y,l->Tile,l->TileWidth,l->TileHeight,l->CSet,-1,-1,0,0,0,0,true,opacity);
}

//Draws an eweapon to a given layer
void DrawToLayer(eweapon e, int layer, int opacity){
	Screen->DrawTile(layer,e->X,e->Y,e->Tile,e->TileWidth,e->TileHeight,e->CSet,-1,-1,0,0,0,0,true,opacity);
}

//Draws an item to a given layer
void DrawToLayer(item i, int layer, int opacity){
	Screen->DrawTile(layer,i->X,i->Y,i->Tile,i->TileWidth,i->TileHeight,i->CSet,-1,-1,0,0,0,0,true,opacity);
}



//Returns the direction of the vector from point 1 to point 2, in degrees from -180 to 180. (0 = right)
float Angle(int x1, int y1, int x2, int y2) {
	return ArcTan(x2-x1, y2-y1)*57.2958;
}

//The above, but in radians.
float RadianAngle(int x1, int y1, int x2, int y2) {
	return ArcTan(x2-x1, y2-y1);
}

// Returns the X component of a vector with a degree angle.
// A length of 3 and angle of 0 returns 3.
// A length of 3 and angle of 45 returns approx. 1.57.
// A length of 3 and angle of 90 returns 0.
float VectorX(int len, float angle) {
	return Cos(angle)*len;
}

// Returns the Y component of a vector with a degree angle.
// A length of 3 and angle of 0 returns 0.
// A length of 3 and angle of 45 returns approx. 1.57.
// A length of 3 and angle of 90 returns 3.
float VectorY(int len, float angle) {
	return Sin(angle)*len;
}

//rotates X about a center point by an amount of degrees
float RotatePointX(float x, float y, float centerX, float centerY, float degrees) {
	float dx = x - centerX;
	float dy = y - centerY;
	return (Cos(degrees) * dx) - (Sin(degrees) * dy) + centerX;
}

//rotates Y about a center point by an amount of degrees
float RotatePointY(float x, float y, float centerX, float centerY, float degrees) {
	float dx = x - centerX;
	float dy = y - centerY;
	return (Sin(degrees) * dx) - (Cos(degrees) * dy) + centerY;
}

//scales X to centerX by a given scale
float ScalePointX(float x, float centerX, float scale) {
	return (scale * (x - centerX)) + centerX; 
}

//scales Y to centerY by a given scale
float ScalePointY(float y, float centerY, float scale) {
	return (scale * (y - centerY)) + centerY; 
}

//rotates and scales X about a center point by an amount of degrees
float RotateScalePointX(float x, float y, float centerX, float centerY, float degrees, float scaleX, float scaleY) {
	float dx = (x - centerX) * scaleX;
	float dy = (y - centerY) * scaleY;
	return (Cos(degrees) * dx) - (Sin(degrees) * dy) + centerX;
}

//rotates and scales Y about a center point by an amount of degrees
float RotateScalePointY(float x, float y, float centerX, float centerY, float degrees, float scaleX, float scaleY) {
	float dx = (x - centerX) * scaleX;
	float dy = (y - centerY) * scaleY;
	return (Sin(degrees) * dx) - (Cos(degrees) * dy) + centerY;
}




// Returns the dot product of two vectors.
float DotProduct( float x1, float y1, float x2, float y2 ) {
	return (x1 * x2 + y1 * y2);
}

// Returns the cross product of two vectors.
float CrossProduct( float x1, float y1, float x2, float y2 ) {
	return (x1 * y2 - y1 * x2);
}

// Finds the center of p1 and p2.
float Midpoint(float p1, float p2) {
	return Lerp(p1, p2, 0.5);
}


// Returns an angle pointing (t)percentage more accurate to the target than the specified radian_angle.
float TurnTowards( int X, int Y, int targetX, int targetY, float radian_angle, float t ) {
	float a = ArcTan( targetX - X, targetY - Y );
	float d = WrapAngle(a - radian_angle);
	if ( d > PI )
		d =-( PI2 - d );
	else if ( d < -PI )
		d = PI2 + d;
	return WrapAngle(radian_angle + d * t);
}


// Returns the X coordinate of the left edge of the hitbox.
int HitboxLeft(eweapon a) {
	return (a->X + a->HitXOffset);
}

int HitboxLeft(lweapon a) {
	return (a->X + a->HitXOffset);
}

int HitboxLeft(item a) {
	return (a->X + a->HitXOffset);
}

int HitboxLeft(npc a) {
	return (a->X + a->HitXOffset);
}

int HitboxLeft(ffc a) {
	return a->X;
}

// Returns the X coordinate of the right edge of the hitbox.
int HitboxRight(eweapon a) {
	return (a->X + a->HitXOffset + a->HitWidth - 1);
}

int HitboxRight(lweapon a) {
	return (a->X + a->HitXOffset + a->HitWidth - 1);
}

int HitboxRight(item a) {
	return (a->X + a->HitXOffset + a->HitWidth - 1);
}

int HitboxRight(npc a) {
	return (a->X + a->HitXOffset + a->HitWidth - 1);
}

int HitboxRight(ffc a) {
	return a->X + a->TileWidth*16 - 1;
}

// Returns the Y coordinate of the top edge of the hitbox.
int HitboxTop(eweapon a) {
	return (a->Y + a->HitYOffset);
}

int HitboxTop(lweapon a) {
	return (a->Y + a->HitYOffset);
}

int HitboxTop(item a) {
	return (a->Y + a->HitYOffset);
}

int HitboxTop(npc a) {
	return (a->Y + a->HitYOffset);
}

int HitboxTop(ffc a) {
	return a->Y;
}

// Returns the Y coordinate of the bottom edge of the hitbox.
int HitboxBottom(eweapon a) {
	return (a->Y + a->HitYOffset + a->HitHeight - 1);
}

int HitboxBottom(lweapon a) {
	return (a->Y + a->HitYOffset + a->HitHeight - 1);
}

int HitboxBottom(item a) {
	return (a->Y + a->HitYOffset + a->HitHeight - 1);
}

int HitboxBottom(npc a) {
	return (a->Y + a->HitYOffset + a->HitHeight - 1);
}

//Uses TileWidth and TileHeight for the FFC's bounding box. 
int HitboxBottom(ffc a) {
	return a->Y + (a->TileHeight*16) - 1;
}

//Returns the number of an FFC, and -1 for a non-valid FFC (which should never happen)
int FFCNum(ffc f) {
	for(int i=1; i<=32; i++)
		if(f == Screen->LoadFFC(i))
			return i;
	return -1;
}


//Removes LWeapon 'l' from the screen
void Remove(lweapon l){
	if(!l->isValid()) return;
	l->DeadState = WDS_DEAD;
	l->X = 32768;
}

//Removes EWeapon 'e' from the screen
void Remove(eweapon e){
	if(!e->isValid()) return;
	e->DeadState = WDS_DEAD;
	e->X = 32768;
}

//Removes Item 'i' from the screen
void Remove(item i){
	if(!i->isValid()) return;
	i->X = 32768;
}

//Removes NPC 'n' from the screen
void Remove(npc n){
	if(!n->isValid()) return;
	n->X = 32768;
	n->HP = -1000;
}


//Returns the position of the first instance of the given combo, or -1.
int FirstComboOf(int t, int layer) {
	for (int i = 0; i < 176; i++) {
		if (layer == 0) {
			if (Screen->ComboD[i] == t)
				return i;
		}
		else {
			if (GetLayerComboD(layer,i) == t)
				return i;
		}
	}
	return -1;
}

//Returns the position of the last instance of the given combo, or -1.
int LastComboOf(int t, int layer) {
	for (int i = 175; i >= 0; i--) {
		if (layer == 0) {
			if (Screen->ComboD[i] == t)
				return i;
		}
		else {
			if (GetLayerComboD(layer,i) == t)
				return i;
		}
	}
	return -1;
}

//Returns the position of the first instance of the given combo, or -1.
int FirstComboTypeOf(int t, int layer) {
	for (int i = 0; i < 176; i++) {
		if (layer == 0) {
			if (Screen->ComboT[i] == t)
				return i;
		}
		else {
			if (GetLayerComboT(layer,i) == t)
				return i;
		}
	}
	return -1;
}

//Returns the position of the last instance of the given combo, or -1.
int LastComboTypeOf(int t, int layer) {
	for (int i = 175; i >= 0; i--) {
		if (layer == 0) {
			if (Screen->ComboT[i] == t)
				return i;
		}
		else {
			if (GetLayerComboT(layer,i) == t)
				return i;
		}
	}
	return -1;
}

//Returns the position of the first instance of the given combo flag, or -1.
//Checks inherent flags too!
int FirstComboFlagOf(int t, int layer) {
	for (int i = 0; i < 176; i++) {
		if (layer == 0) {
			if (Screen->ComboF[i] == t || Screen->ComboI[i] == t) return i;
		}
		else {
			if (Game->GetComboFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer),i) == t
				|| Game->GetComboInherentFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer),i) == t)
				return i;
		}
	}
	return -1;
}

//Returns the position of the last instance of the given combo flag, or -1.
//Checks inherent flags too!
int LastComboFlagOf(int t, int layer) {
	for (int i = 175; i >= 0; i--) {
		if (layer == 0) {
			if (Screen->ComboF[i] == t || Screen->ComboI[i] == t) return i;
		}
		else {
			if (Game->GetComboFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer),i) == t
				|| Game->GetComboInherentFlag(Screen->LayerMap(layer), Screen->LayerScreen(layer),i) == t)
				return i;
		}
	}
	return -1;
}



//Creates and returns an exact copy of the passed LWeapon. Assumes that the passed pointer is valid.
lweapon Duplicate(lweapon a) {
	lweapon b = Screen->CreateLWeapon(a->ID);
	b->X = a->X;
	b->Y = a->Y;
	b->Z = a->Z;
	b->Jump = a->Jump;
	b->Extend = a->Extend;
	b->TileWidth = a->TileWidth;
	b->TileHeight = a->TileHeight;
	b->HitWidth = a->HitWidth;
	b->HitHeight = a->HitHeight;
	b->HitZHeight = a->HitZHeight;
	b->HitXOffset = a->HitXOffset;
	b->HitYOffset = a->HitYOffset;
	b->DrawXOffset = a->DrawXOffset;
	b->DrawYOffset = a->DrawYOffset;
	b->DrawZOffset = a->DrawZOffset;
	b->Tile = a->Tile;
	b->CSet = a->CSet;
	b->DrawStyle = a->DrawStyle;
	b->Dir = a->Dir;
	b->OriginalTile = a->OriginalTile;
	b->OriginalCSet = a->OriginalCSet;
	b->FlashCSet = a->FlashCSet;
	b->NumFrames = a->NumFrames;
	b->Frame = a->Frame;
	b->ASpeed = a->ASpeed;
	b->Damage = a->Damage;
	b->Step = a->Step;
	b->Angle = a->Angle;
	b->Angular = a->Angular;
	b->CollDetection = a->CollDetection;
	b->DeadState = a->DeadState;
	b->Flash = a->Flash;
	b->Flip = a->Flip;
	for (int i = 0; i < 16; i++)
		b->Misc[i] = a->Misc[i];
	return b;
}

//Creates and returns an exact copy of the passed EWeapon. Assumes that the passed pointer is valid.
eweapon Duplicate(eweapon a) {
	eweapon b = Screen->CreateEWeapon(a->ID);
	b->X = a->X;
	b->Y = a->Y;
	b->Z = a->Z;
	b->Jump = a->Jump;
	b->Extend = a->Extend;
	b->TileWidth = a->TileWidth;
	b->TileHeight = a->TileHeight;
	b->HitWidth = a->HitWidth;
	b->HitHeight = a->HitHeight;
	b->HitZHeight = a->HitZHeight;
	b->HitXOffset = a->HitXOffset;
	b->HitYOffset = a->HitYOffset;
	b->DrawXOffset = a->DrawXOffset;
	b->DrawYOffset = a->DrawYOffset;
	b->DrawZOffset = a->DrawZOffset;
	b->Tile = a->Tile;
	b->CSet = a->CSet;
	b->DrawStyle = a->DrawStyle;
	b->Dir = a->Dir;
	b->OriginalTile = a->OriginalTile;
	b->OriginalCSet = a->OriginalCSet;
	b->FlashCSet = a->FlashCSet;
	b->NumFrames = a->NumFrames;
	b->Frame = a->Frame;
	b->ASpeed = a->ASpeed;
	b->Damage = a->Damage;
	b->Step = a->Step;
	b->Angle = a->Angle;
	b->Angular = a->Angular;
	b->CollDetection = a->CollDetection;
	b->DeadState = a->DeadState;
	b->Flash = a->Flash;
	b->Flip = a->Flip;
	for (int i = 0; i < 16; i++)
		b->Misc[i] = a->Misc[i];
	return b;
}




//Kills all of Link's inputs
void NoAction()
{
	Link->InputUp = false; Link->PressUp = false;
	Link->InputDown = false; Link->PressDown = false;
	Link->InputLeft = false; Link->PressLeft = false;
	Link->InputRight = false; Link->PressRight = false;
	Link->InputR = false; Link->PressR = false;
	Link->InputL = false; Link->PressL = false;
	Link->InputA = false; Link->PressA = false;
	Link->InputB = false; Link->PressB = false;
	Link->InputEx1 = false; Link->PressEx1 = false;
	Link->InputEx2 = false; Link->PressEx2 = false;
	Link->InputEx3 = false; Link->PressEx3 = false;
	Link->InputEx4 = false; Link->PressEx4 = false;
}

//NoAction, then Waitframe or (equivalent of) Waitframes
void WaitNoAction()
{
	NoAction();
	Waitframe();
}
void WaitNoAction(int frames)
{
	for(int i = 0; i < frames; i++)
		WaitNoAction();
}












