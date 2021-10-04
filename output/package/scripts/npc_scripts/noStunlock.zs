npc script noStunlock
{
	void run()
	{
		int defbrang = this->Defense[NPCD_BRANG];
		while(1)
		{
			if ( this->Stun )
			{
				this->Defense[NPCD_BRANG] = NPCDT_BLOCK;
			}
			else this->Defense[NPCD_BRANG] = defbrang;
			Waitframe();
		}
	}
}