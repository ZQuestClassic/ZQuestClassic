interface soundLoop //lots of things use looping sounds. Might as well implement a catch all interface to simplify things.
{
	//boomerangs, hookshots, ambience , and boss roars have looping sounds by default. This serves as a getter
	soundLoop()
	{
		GetDefaultSoundLoop();
	}
	~soundLoop()
	{
		StopSFX();
	}
	//Because screens use flags and we cannot call the destructor for those. We need to abstract this out of the destructor.
	StopSFX()
	{
		sfxLoop=0;
	}
	SFX sfxLoop;
	{
		get { return sfxLoop; }
		set { sfxLoop=value; }
	}
}
