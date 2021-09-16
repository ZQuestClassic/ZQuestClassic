const bool ONSIDEVIEWPLATFORM_CHECK_OBJ_WIDTH = true;
const int ONSIDEVIEWPLATFORM_OVERHANG_DEPTH = 4;


//Returns true if the sprite at (x,y) is standing on a sideview platform on a sideview screen, as worked out
//by ZC's internal code.
//For 16 pixel high sprites only.
bool OnSidePlatform(int x, int y) 
{
	unless ( IsSideview() ) return false;
	return ((Screen->isSolid(x+ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,y+16) 
		|| Screen->isSolid(x+15-ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,y+16)));
}

//Returns true if a sprite of height 'h' at position (x,y) with an offset of (xOff,yOff) is standing
//on a sideview platform on a sideview screen.
bool OnSidePlatform(int x, int y, int xOff, int yOff, int h) 
{
	unless ( IsSideview() ) return false;
	return ((Screen->isSolid((x+xOff)+ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,(y+yOff)+h) 
		|| Screen->isSolid((x+xOff)+15-ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,(y+yOff)+h)));
}

//Returns true if a sprite of height 'h' and width `w`, 
//at position (x,y) with an offset of (xOff,yOff) is standing
//on a sideview platform on a sideview screen.
bool OnSidePlatform(int x, int y, int xOff, int yOff, int h, int w) 
{
	unless ( IsSideview() ) return false;
	return ((Screen->isSolid((x+xOff)+ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,(y+yOff)+h) 
		|| Screen->isSolid((x+xOff)+w+(15-ONSIDEVIEWPLATFORM_OVERHANG_DEPTH),(y+yOff)+h)));
}


bool OnSidePlatform(lweapon l) 
{
	unless ( IsSideview() ) return false;
	return ((Screen->isSolid((l->X+l->HitXOffset)+ONSIDEVIEWPLATFORM_OVERHANG_DEPTH,(l->Y+l->HitYOffset+l->HitHeight) 
		|| Screen->isSolid((l->X+l->HitXOffset)+l->HitWidth-ONSIDEVIEWPLATFORM_OVERHANG_DEPTH),(l->Y+l->HitYOffset)+l->HitHeight)));
}
