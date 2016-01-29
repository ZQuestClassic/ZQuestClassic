interface soundLoop //lots of things use looping sounds. Might as well implement a catch all interface to simplify things.
{
	__RealSoundLoop@ realSoundLoop;
	
	//boomerangs, hookshots, ambience , and boss roars have looping sounds by default. This actually Initializes those aswell.
	soundLoop()
	{
		@realSoundLoop = __getRealSoundLoop;
	}
	~soundLoop()
	{
		soundLoop.StopSFX();
	}
	//Because screens use flags and we cannot call the destructor for those. We need to abstract this out of the destructor.
	StopSFX()
	{
		realSoundLoop.sfxLoop=0;
	}
	SFX sfxLoop;
	{
		get const { return realSoundLoop.sfx; }
		set { realSoundLoop.sfx=value; }
	}
}
