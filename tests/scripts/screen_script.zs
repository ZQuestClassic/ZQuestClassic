#include "std.zh"

screendata script ScreenScriptTest
{
	void run(int changeCMB, int changeSFX)
	{
		// Start of script, screen init timing
		Link->HP = Link->HP / 2;
		
		while(true)
		{
			// Combo changing, this pointer
			int pos = ComboAt(Link->X+8, Link->Y+8);
			if(this->ComboD[pos]==changeCMB)
			{
				++this->ComboD[pos];
				Audio->PlaySound(changeSFX);
			}
			
			// Drawing pre waitdraw
			Screen->DrawTile(6, Link->X, Link->Y-16, Link->Tile, 1, 1, Link->CSet, -1, -1, 0, 0, 0, 0, true, OP_OPAQUE);
			
			Waitdraw();
			
			// Drawing post waitdraw
			Screen->DrawTile(6, Link->X, Link->Y+16, Link->Tile, 1, 1, Link->CSet, -1, -1, 0, 0, 0, 2, true, OP_OPAQUE);
			
			Waitframe();
		}
	}
}
