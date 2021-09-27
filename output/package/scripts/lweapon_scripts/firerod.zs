lweapon script FireRod
{
	void run()
	{
		lweapon flame = Screen->CreateLWeapon(LW_FIRE);
		while(this->isValid())
		{
			flame->X = this->X;
			flame->Y = this->Y;
			flame->Dir = this->Dir;
			Waitframe();
		}
	}
}