// An array literal whose elements have different types (here char32 and a
// typed enum) falls back to an untyped element type, so it can be assigned
// to an untyped or numeric array.
enum SpecialChars
{
	SC_EMPTY,
	SC_ENTER,
	SC_DONE
};

global script Global
{
	void run()
	{
		untyped chars[] =
		{
			'0', '1', 'A', 'B',
			SC_EMPTY, SC_ENTER, SC_DONE
		};
		int nums[] = { '0', SC_ENTER };
		char32 letters[] = { SC_DONE, 'Z' };
		printf("%d %d %d\n", chars[5], nums[1], letters[0]);
	}
}
