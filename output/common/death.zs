namespace death
{
	void SkipAnimation(bool allowContinue)
	{
		if ( Link->HP < 1 )
		{
			if ( allowContinue ) Game->ShowContinueScreen();
			else Game->ShowSaveScreen();
		}
	}
}

//call death::SkipAnimation(bool) in any scrip.

