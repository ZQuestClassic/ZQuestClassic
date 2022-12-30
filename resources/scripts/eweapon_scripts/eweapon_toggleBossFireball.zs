eweapon script toggleBossFireball
{
	void run(bool enable)
	{
		if ( enable ) this->Level |= 1;
		else this->Level &= ~1;
	}
}