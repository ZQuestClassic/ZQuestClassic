// Regression test for switch dispatch lowerings: dense switches compile to a
// jump table (GOTOTABLE), and ranged or sparse ones to a binary search over
// their case ranges (GOTORANGES). Each switch's key is a function parameter,
// which is never a compile-time value, so nothing is constant-folded and the
// dispatch actually executes. Covers the cases both must handle identically
// to the compare ladder they replace:
//   - a dense switch (enough labels to earn a table)
//   - keys below / above the table's range, and gaps inside it, hitting default
//   - ranged cases, matching non-integral keys between their bounds
//   - a negative-key switch (the table's base key is negative)
//   - fall-through between cases, and a switch with no default
//   - a switch whose keys are too sparse for a table
//   - wide ranges (a span far too big for a table), including their exact
//     bounds and the gaps between them
//   - ranges contiguous in fixed point sharing a case, which merge into one
//   - enough ranges for the binary search to recurse a few levels
//   - ranges with negative bounds, including one spanning zero

#include "auto/test_runner.zs"

// All-discrete and dense: exactly what earns a jump table.
int dense(int k)
{
	switch (k)
	{
		case 0: return 100;
		case 1: return 101;
		case 2: return 102;
		case 3: return 103;
		case 5: return 105; // 4 is a gap -> default
		case 6: return 106;
		case 7: return 107;
		case 8: return 108;
		default: return -1;
	}
}

// A ranged case matches non-integral keys between its bounds, which a table
// indexed by whole steps cannot express - so this dispatches by GOTORANGES.
int ranged(int k)
{
	switch (k)
	{
		case 0: return 100;
		case 1: return 101;
		case 2: return 102;
		case 3: return 103;
		case 6=..=8: return 168;
		default: return -1;
	}
}

int negatives(int k)
{
	switch (k)
	{
		case -5: return 1;
		case -4: return 2;
		case -3: return 3;
		case -2: return 4;
		case 0: return 5;
		default: return -1;
	}
}

int fallthrough(int k)
{
	int n = 0;
	switch (k)
	{
		case 0:
		case 1:
		case 2:
			n += 1;
		case 3:
			n += 10;
			break;
		case 4:
			n += 100;
			break;
		default:
			n = -1;
	}
	return n;
}

// No default: an unmatched key must skip the whole switch.
int no_default(int k)
{
	int n = -1;
	switch (k)
	{
		case 10: n = 0; break;
		case 11: n = 1; break;
		case 12: n = 2; break;
		case 13: n = 3; break;
	}
	return n;
}

// Too sparse for a table; dispatches by GOTORANGES over four one-key ranges.
int sparse(int k)
{
	switch (k)
	{
		case 0: return 1;
		case 1000: return 2;
		case 100000: return 3;
		case 12345678: return 4;
		default: return -1;
	}
}

// Wide ranges over a span far too big for a table (65536 whole keys), with a
// gap between the last two - the case GOTORANGES exists for.
int wide_ranges(int k)
{
	switch (k)
	{
		case 0=..=8191: return 1;
		case 8192=..=16383: return 2;
		case 16384=..=24575: return 3;
		case 32768=..=65535: return 4; // 24576...32767 is a gap
		default: return -1;
	}
}

// The first two labels are contiguous in fixed point (2.0001 follows 2), so
// the compiler merges them into a single [1, 4] range.
int merged_ranges(int k)
{
	switch (k)
	{
		case 1=..=2:
		case 2..=4: // exclusive left: 2 < k <= 4
			return 1;
		case 10: return 2;
		case 20: return 3;
		default: return -1;
	}
}

// Eight disjoint ranges, so the binary search recurses a few levels deep.
int many_ranges(int k)
{
	switch (k)
	{
		case 0=..=5: return 1;
		case 10=..=15: return 2;
		case 20=..=25: return 3;
		case 30=..=35: return 4;
		case 40=..=45: return 5;
		case 50=..=55: return 6;
		case 60=..=65: return 7;
		case 70=..=75: return 8;
		default: return -1;
	}
}

// Ranges with negative bounds, one spanning zero.
int negative_ranges(int k)
{
	switch (k)
	{
		case -10=..=-5: return 1;
		case -2=..=3: return 2;
		case 5: return 3;
		case 7=..=9: return 4;
		default: return -1;
	}
}

// Fall-through between ranged cases, and no default.
int range_fallthrough(int k)
{
	int n = 0;
	switch (k)
	{
		case 0=..=2:
			n += 1;
		case 5=..=6:
			n += 10;
			break;
		case 10=..=12:
			n += 100;
			break;
		case 20=..=21:
			n += 1000;
			break;
	}
	return n;
}

generic script switches
{
	void run()
	{
		Test::Init();

		Test::AssertEqual(dense(0), 100);
		Test::AssertEqual(dense(1), 101);
		Test::AssertEqual(dense(2), 102);
		Test::AssertEqual(dense(3), 103);
		Test::AssertEqual(dense(4), -1);  // gap inside the table
		Test::AssertEqual(dense(5), 105);
		Test::AssertEqual(dense(8), 108); // last slot
		Test::AssertEqual(dense(9), -1);  // past the table
		Test::AssertEqual(dense(-1), -1); // before the table
		Test::AssertEqual(dense(-8), -1); // far before

		// A non-integral key matches no discrete case, whatever the base is.
		Test::AssertEqual(dense(2.5), -1);
		Test::AssertEqual(dense(-0.5), -1);
		Test::AssertEqual(dense(8.5), -1);

		// Ranges match non-integral keys between their bounds.
		Test::AssertEqual(ranged(2), 102);
		Test::AssertEqual(ranged(6), 168);
		Test::AssertEqual(ranged(8), 168);
		Test::AssertEqual(ranged(6.5), 168); // inside case 6=..=8
		Test::AssertEqual(ranged(2.5), -1);  // between discrete cases
		Test::AssertEqual(ranged(8.5), -1);  // just past the range
		Test::AssertEqual(ranged(9), -1);

		Test::AssertEqual(negatives(-5), 1);
		Test::AssertEqual(negatives(-4), 2);
		Test::AssertEqual(negatives(-2), 4);
		Test::AssertEqual(negatives(-1), -1); // gap
		Test::AssertEqual(negatives(0), 5);
		Test::AssertEqual(negatives(1), -1);  // past

		Test::AssertEqual(fallthrough(0), 11);  // falls into case 3
		Test::AssertEqual(fallthrough(2), 11);  // falls into case 3
		Test::AssertEqual(fallthrough(3), 10);
		Test::AssertEqual(fallthrough(4), 100);
		Test::AssertEqual(fallthrough(5), -1);  // default

		Test::AssertEqual(no_default(10), 0);
		Test::AssertEqual(no_default(13), 3);
		Test::AssertEqual(no_default(14), -1); // no match
		Test::AssertEqual(no_default(0), -1);  // no match

		Test::AssertEqual(sparse(0), 1);
		Test::AssertEqual(sparse(1), -1);
		Test::AssertEqual(sparse(1000), 2);
		Test::AssertEqual(sparse(12345678), 4);

		// Wide ranges: exact bounds on both sides of every boundary.
		Test::AssertEqual(wide_ranges(0), 1);         // first bound
		Test::AssertEqual(wide_ranges(8191), 1);      // last key of range 1
		Test::AssertEqual(wide_ranges(8000.5), 1);    // non-integral inside
		Test::AssertEqual(wide_ranges(8191.5), -1);   // in the fixed-point crack
		                                              // between adjacent ranges
		Test::AssertEqual(wide_ranges(8192), 2);      // first key of range 2
		Test::AssertEqual(wide_ranges(16383), 2);
		Test::AssertEqual(wide_ranges(16384), 3);
		Test::AssertEqual(wide_ranges(24575), 3);     // last key before the gap
		Test::AssertEqual(wide_ranges(24576), -1);    // gap
		Test::AssertEqual(wide_ranges(30000.5), -1);  // non-integral in the gap
		Test::AssertEqual(wide_ranges(32767), -1);    // gap, last key
		Test::AssertEqual(wide_ranges(32768), 4);
		Test::AssertEqual(wide_ranges(65535), 4);     // last key of all
		Test::AssertEqual(wide_ranges(65536), -1);    // past everything
		Test::AssertEqual(wide_ranges(-1), -1);       // before everything

		// Merged ranges: keys around the seam behave as one [1, 4] range.
		Test::AssertEqual(merged_ranges(1), 1);
		Test::AssertEqual(merged_ranges(2), 1);      // last key of the first label
		Test::AssertEqual(merged_ranges(2.5), 1);    // crosses the seam
		Test::AssertEqual(merged_ranges(3), 1);      // inside the second label
		Test::AssertEqual(merged_ranges(4), 1);
		Test::AssertEqual(merged_ranges(4.0001), -1);// just past
		Test::AssertEqual(merged_ranges(0.9999), -1);// just before
		Test::AssertEqual(merged_ranges(10), 2);
		Test::AssertEqual(merged_ranges(15), -1);
		Test::AssertEqual(merged_ranges(20), 3);

		// Many ranges: first, last, middles, gaps, and non-integral keys.
		Test::AssertEqual(many_ranges(0), 1);        // first bound
		Test::AssertEqual(many_ranges(5), 1);        // first range's end
		Test::AssertEqual(many_ranges(7), -1);       // gap
		Test::AssertEqual(many_ranges(12.5), 2);     // non-integral inside
		Test::AssertEqual(many_ranges(8.5), -1);     // non-integral in a gap
		Test::AssertEqual(many_ranges(35), 4);
		Test::AssertEqual(many_ranges(36), -1);      // gap
		Test::AssertEqual(many_ranges(50), 6);
		Test::AssertEqual(many_ranges(70), 8);       // last range's start
		Test::AssertEqual(many_ranges(75), 8);       // last key of all
		Test::AssertEqual(many_ranges(76), -1);      // past everything
		Test::AssertEqual(many_ranges(-3), -1);      // before everything

		// Negative bounds, and a range spanning zero.
		Test::AssertEqual(negative_ranges(-10), 1);
		Test::AssertEqual(negative_ranges(-5), 1);
		Test::AssertEqual(negative_ranges(-4), -1);   // gap
		Test::AssertEqual(negative_ranges(-2), 2);
		Test::AssertEqual(negative_ranges(-0.5), 2);  // non-integral, negative
		Test::AssertEqual(negative_ranges(0), 2);
		Test::AssertEqual(negative_ranges(3), 2);
		Test::AssertEqual(negative_ranges(3.5), -1);  // just past
		Test::AssertEqual(negative_ranges(5), 3);
		Test::AssertEqual(negative_ranges(6), -1);    // gap
		Test::AssertEqual(negative_ranges(9), 4);
		Test::AssertEqual(negative_ranges(10), -1);   // past everything

		// Fall-through across ranged cases, and no default.
		Test::AssertEqual(range_fallthrough(1), 11);   // falls into 5=..=6
		Test::AssertEqual(range_fallthrough(0.5), 11); // non-integral falls too
		Test::AssertEqual(range_fallthrough(5), 10);
		Test::AssertEqual(range_fallthrough(11), 100);
		Test::AssertEqual(range_fallthrough(20), 1000);
		Test::AssertEqual(range_fallthrough(3), 0);    // no match
		Test::AssertEqual(range_fallthrough(13), 0);       // no match

		Test::End();
	}
}
