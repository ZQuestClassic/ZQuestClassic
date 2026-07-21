// Compound assignment operators (`+=`, `-=`, ..., `??=`) and `++`/`--` used
// to evaluate their left-hand side twice when it was an array access: once to
// read the current value and once again to write the result. Any side effects
// in the array or index expression (like a function call) ran twice, so
// `arr[f()] += 5` called f() two times.

#include "auto/test_runner.zs"
#include "std.zh"

int calls;
int rhs_calls;
int arr[8];
bool barr[8];
int inner[4];

class Box
{
	int val;
}

Box g_box;
Box get_box()
{
	calls++;
	return g_box;
}

itemsprite g_item;
itemsprite get_item()
{
	calls++;
	return g_item;
}

int idx(int i)
{
	calls++;
	return i;
}

bool rhs_true()
{
	rhs_calls++;
	return true;
}

int rhs_value(int v)
{
	rhs_calls++;
	return v;
}

void reset(int val)
{
	calls = 0;
	rhs_calls = 0;
	for (int i = 0; i < 8; i++)
		arr[i] = val;
}

void reset_bool(bool val)
{
	calls = 0;
	rhs_calls = 0;
	for (int i = 0; i < 8; i++)
		barr[i] = val;
}

generic script bug_compound_assign_side_effects
{
	void run()
	{
		Test::Init();

		// Each compound assignment operator must evaluate the index
		// expression exactly once.
		reset(10);
		arr[idx(0)] += 5;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 15);

		reset(10);
		arr[idx(0)] -= 4;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 6);

		reset(10);
		arr[idx(0)] *= 3;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 30);

		reset(10);
		arr[idx(0)] /= 4;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 2.5);

		reset(10);
		arr[idx(0)] %= 3;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 1);

		reset(10);
		arr[idx(0)] <<= 1;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 20);

		reset(10);
		arr[idx(0)] >>= 1;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 5);

		reset(10);
		arr[idx(0)] &= 6;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 2);

		reset(10);
		arr[idx(0)] |= 5;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 15);

		reset(10);
		arr[idx(0)] ^= 3;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 9);

		// `a ~= b` is `a = a & ~b`.
		reset(10);
		arr[idx(0)] ~= 2;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 8);

		// Logical assignment operators evaluate the index once, and still
		// short-circuit the right-hand side.
		reset_bool(true);
		barr[idx(0)] &&= rhs_true();
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(barr[0], true);

		reset_bool(false);
		barr[idx(0)] &&= rhs_true();
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 0);
		Test::AssertEqual(barr[0], false);

		reset_bool(false);
		barr[idx(0)] ||= rhs_true();
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(barr[0], true);

		reset_bool(true);
		barr[idx(0)] ||= rhs_true();
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 0);
		Test::AssertEqual(barr[0], true);

		// Falsy-coalescing assignment.
		reset(0);
		arr[idx(0)] ?:= rhs_value(7);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(arr[0], 7);

		reset(10);
		arr[idx(0)] ?:= rhs_value(7);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 0);
		Test::AssertEqual(arr[0], 10);

		// Increment/decrement.
		reset(10);
		arr[idx(0)]++;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 11);

		reset(10);
		int post = arr[idx(0)]++;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(post, 10);
		Test::AssertEqual(arr[0], 11);

		reset(10);
		int pre = ++arr[idx(0)];
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(pre, 11);
		Test::AssertEqual(arr[0], 11);

		reset(10);
		arr[idx(0)]--;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(arr[0], 9);

		reset(10);
		int postdec = arr[idx(0)]--;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(postdec, 10);
		Test::AssertEqual(arr[0], 9);

		reset(10);
		int predec = --arr[idx(0)];
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(predec, 9);
		Test::AssertEqual(arr[0], 9);

		// Side effects in the array expression (not just the index).
		reset(10);
		int[][] nested = {inner};
		inner[1] = 40;
		nested[idx(0)][idx(1)] += 2;
		Test::AssertEqual(calls, 2);
		Test::AssertEqual(inner[1], 42);

		// The value of a compound assignment expression is the stored value.
		reset(10);
		int result = (arr[idx(2)] += 5);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(result, 15);
		Test::AssertEqual(arr[2], 15);

		// Nested compound assignments: each evaluates its own index once.
		reset(10);
		arr[idx(4)] += (arr[idx(5)] += 2);
		Test::AssertEqual(calls, 2);
		Test::AssertEqual(arr[5], 12);
		Test::AssertEqual(arr[4], 22);

		// The index expression may itself write to variables.
		reset(10);
		int i = 3;
		arr[i++] += 5;
		Test::AssertEqual(i, 4);
		Test::AssertEqual(arr[3], 15);

		// Side effects nested inside a larger index expression also run
		// only once each.
		reset(10);
		arr[1 + idx(idx(1))] += 5;
		Test::AssertEqual(calls, 2);
		Test::AssertEqual(arr[2], 15);

		// Compound assignment used inside a larger boolean condition.
		reset_bool(true);
		bool cond = (barr[idx(0)] &&= rhs_true()) && true;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(cond, true);
		Test::AssertEqual(barr[0], true);

		// Class member arrows: the object expression must be evaluated once.
		g_box = new Box();
		g_box->val = 10;
		calls = 0;
		get_box()->val += 5;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(g_box->val, 15);

		calls = 0;
		get_box()->val++;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(g_box->val, 16);

		calls = 0;
		int boxpost = get_box()->val--;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(boxpost, 16);
		Test::AssertEqual(g_box->val, 15);

		g_box->val = 0;
		calls = 0;
		rhs_calls = 0;
		get_box()->val ?:= rhs_value(7);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(g_box->val, 7);

		calls = 0;
		rhs_calls = 0;
		get_box()->val ?:= rhs_value(3);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 0);
		Test::AssertEqual(g_box->val, 7);

		// Internal variable arrows (engine read/write functions).
		g_item = Screen->CreateItem(I_HEARTCONTAINER);
		g_item->X = 10;
		calls = 0;
		get_item()->X += 7;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(g_item->X, 17);

		calls = 0;
		get_item()->X--;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(g_item->X, 16);

		// Indexed internal arrays: pointer and index each evaluated once.
		calls = 0;
		get_item()->Misc[idx(2)] += 3;
		Test::AssertEqual(calls, 2);
		Test::AssertEqual(g_item->Misc[2], 3);

		// Indexed internal arrays with side effects only in the index.
		Screen->D[3] = 10;
		calls = 0;
		Screen->D[idx(3)] += 5;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(Screen->D[3], 15);

		calls = 0;
		Screen->D[idx(3)]++;
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(Screen->D[3], 16);

		Screen->D[4] = 0;
		calls = 0;
		rhs_calls = 0;
		Screen->D[idx(4)] ?:= rhs_value(9);
		Test::AssertEqual(calls, 1);
		Test::AssertEqual(rhs_calls, 1);
		Test::AssertEqual(Screen->D[4], 9);

		Test::End();
	}
}
