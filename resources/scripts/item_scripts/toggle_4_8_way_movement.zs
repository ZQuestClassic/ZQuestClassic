item script toggle_4_8_way_movement
{
	void run()
	{
		Game->FFRules[qr_LTTPWALK] = !Game->FFRules[qr_LTTPWALK];
	}
}