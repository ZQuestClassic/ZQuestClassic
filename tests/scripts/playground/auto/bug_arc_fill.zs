// https://discord.com/channels/876899628556091432/1523348144497365134
//
// Screen->Arc / bitmap->Arc filled pies used to be drawn by flood filling a
// probe point inside an arc+lines outline. Whenever the probe landed outside
// the wedge (wrap-around angle spans, or the probe's x clamped to 0 for pies
// offscreen to the left), the fill escaped and covered everything except the
// wedge. Filled pies are now rasterized directly as polygons.
//
// Also, filled pies went through a masked sprite blit, so drawing with color 0
// onto a bitmap could never write anything; now opaque fills write color 0.
//
// The Screen->Arc draws below have no assertions - the replay's per-frame gfx
// hashes are what locks their rendering.

#include "auto/test_runner.zs"

generic script bug_arc_fill
{
	void run()
	{
		Test::Init();

		// Color 0 pie carved out of a filled bitmap: previously a no-op.
		bitmap carve = Game->CreateBitmap(64, 64);
		carve->Rectangle(0, 0, 0, 63, 63, 0x27);
		carve->Arc(0, 32, 32, 20, 0, 270, 0);

		// Wrap-around span (end angle behind start angle) on a cleared
		// bitmap: allegro draws the 270 degree pie covering N/W/S; the old
		// flood fill escaped and painted everything else instead.
		bitmap wrap = Game->CreateBitmap(64, 64);
		wrap->Clear(0);
		wrap->Arc(0, 32, 32, 20, 90, 0, 0x35);

		Waitframe();

		// Inside the carved pie (0..270 anticlockwise from east = NE/NW/SW).
		Test::AssertEqual(carve->GetPixel(40, 24), 0);
		Test::AssertEqual(carve->GetPixel(24, 40), 0);
		// The SE quadrant is outside the pie and keeps the rectangle color.
		Test::AssertEqual(carve->GetPixel(40, 40), 0x27);

		// NE quadrant is outside the wrapped pie; the old escaped fill
		// painted it (and the rest of the bitmap).
		Test::AssertEqual(wrap->GetPixel(60, 4), 0);
		Test::AssertEqual(wrap->GetPixel(44, 20), 0);
		// Inside the wrapped pie.
		Test::AssertEqual(wrap->GetPixel(20, 32), 0x35);

		for (int i = 0; i < 60; ++i)
		{
			// 270 degree pie, anticlockwise from -180 to 90, leaving only the top-left quadrant
			// empty.
			Screen->Arc(6, 60, 60, 30, -180, 90, 0x01);

			// Wrap-around span.
			Screen->Arc(6, 140, 60, 30, 90, 0, 0x02);

			// Thin wedge and zero-span (degenerates to a single ray).
			Screen->Arc(6, 200, 60, 28, 10, 25, 0x03);
			Screen->Arc(6, 240, 60, 12, 45, 45, 0x09);

			// Center offscreen, wedge poking onscreen: covers polygon clipping
			// (the old fill happened to stay contained for this shape).
			Screen->Arc(6, -10, 120, 40, -45, 45, 0x04);

			// Pie entirely offscreen: the old fill probe was clamped to x=0,
			// landing outside the wedge and flooding the whole screen. Should
			// draw nothing.
			Screen->Arc(6, -30, 90, 25, 135, 225, 0x0A);

			// Translucent pie.
			Screen->Arc(6, 60, 150, 30, 0, 200, 0x05, 1, 0, 0, 0, true, true, OP_TRANS);

			// Closed-unfilled and open-unfilled arcs (unchanged code paths).
			Screen->Arc(6, 140, 150, 30, 30, 150, 0x06, 1, 0, 0, 0, true, false);
			Screen->Arc(6, 200, 150, 30, 30, 150, 0x07, 1, 0, 0, 0, false, false);

			// Nearly-full circle.
			Screen->Arc(6, 230, 20, 12, 0, 359, 0x08);

			Waitframe();
		}

		Test::End();
	}
}
