#include "auto/test_runner.zs"
#include "std.zh"

generic script script_drawing
{
	// From Moosh.
	// https://discord.com/channels/876899628556091432/1356403015447482400
	void DrawSlashLine(int layer, int x1, int y1, int x2, int y2, int taperPct, int w, int c, int op, int sideOff=0, bitmap target=NULL)
	{
		int dist = Distance(x1, y1, x2, y2);
		int ang = Angle(x1, y1, x2, y2);
		if(sideOff!=0)
		{
			x1 += VectorX(sideOff, ang-90);
			y1 += VectorY(sideOff, ang-90);
			x2 += VectorX(sideOff, ang-90);
			y2 += VectorY(sideOff, ang-90);
		}
		int verts[] = {x1,y1, 0,0, 0,0, x2,y2, 0,0, 0,0};
		verts[2] = Lerp(x1, x2, taperPct)+VectorX(w, ang-90);
		verts[3] = Lerp(y1, y2, taperPct)+VectorY(w, ang-90);
		
		verts[4] = Lerp(x1, x2, 1-taperPct)+VectorX(w, ang-90);
		verts[5] = Lerp(y1, y2, 1-taperPct)+VectorY(w, ang-90);
		
		verts[8] = Lerp(x1, x2, 1-taperPct)+VectorX(-w, ang-90);
		verts[9] = Lerp(y1, y2, 1-taperPct)+VectorY(-w, ang-90);
		
		verts[10] = Lerp(x1, x2, taperPct)+VectorX(-w, ang-90);
		verts[11] = Lerp(y1, y2, taperPct)+VectorY(-w, ang-90);
		if(target)
			target->Polygon(layer, 6, verts, c, op);
		else
			Screen->Polygon(layer, 6, verts, c, op);
	}

	void setLayerBlock(int layer)
	{
		for (int i = 0; i <= 6; i++)
		{
			mapdata lyr = Game->LoadTempScreen(i);
			int combo = i == layer ? 100 : 0;
			for (int y = 0; y < 20; y++)
			{
				lyr->ComboD[ComboAt(10*16, y*16)] = combo;
				lyr->ComboD[ComboAt(10*16, (y+1)*16)] = combo;
				lyr->ComboD[ComboAt(10*16, (y+2)*16)] = combo;
				lyr->ComboC[ComboAt(10*16, y*16)] = 0;
				lyr->ComboC[ComboAt(10*16, (y+1)*16)] = 0;
				lyr->ComboC[ComboAt(10*16, (y+2)*16)] = 0;
			}
		}
	}

	void run()
	{
		Test::Init();

		Test::loadRegion(0, 4);

		int i = 0;
		int frames = 60 * 3;

		while (i++ < frames)
		{
			Hero->X += 1;
			Hero->Y += 1;

			int layer = i % 7;
			DrawSlashLine(layer, 0, 0, Link->X, Link->Y, 0.25, 4, 0x01, OP_OPAQUE);
			DrawSlashLine(layer, 100, 0, Link->X, Link->Y, 0.25, 4, 0x01, OP_OPAQUE);
			DrawSlashLine(layer, 200, 0, Link->X, Link->Y, 0.25, 4, 0x01, OP_OPAQUE);

			// Primitives are always drawn above the combo for the same layer.
			setLayerBlock(layer);

			Waitframe();
		}
	}
}
