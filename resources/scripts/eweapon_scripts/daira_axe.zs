////////////////////////////////////
/// Axe Weapon Handler For Daira ///
////////////////////////////////////
eweapon script dairaaxe
{
	void run()
	{
		this->X = Clamp(this->X, 1, 255);
		this->Y = Clamp(this->Y, 1, 175);
		int timer = 22;
		while(this->isValid())
		{
			this->X = Clamp(this->X, 1, 255);
			this->Y = Clamp(this->Y, 1, 175);
			--timer;
			if ( timer < 1 ) { Remove(this); Quit(); }
			Waitframe();
		}
	}
}