namespace errors_9
{
	WarpType x;

	// [WarpType::Invalid] [x->Invalid]
	void bad_cast()
	{
		<WarpType>Screen->TileWarpType[0];
	}
}
