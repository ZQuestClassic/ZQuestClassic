/* A script to prevent riverfolk from restoring hp when 
   they dive underwater. 
   v1.0
   25th February, 2020
   ZoriaRPG
*/

npc script submergekeephp
{
	void run()
	{
		int lasthp = this->HP;
		while(1)
		{
			if ( !this->Submerged() && (this->HP != lasthp ))
			{
				if ( this->HP < lasthp )
				{
					lasthp = this->HP;
				}
				else //re-emerging from water
				{
					this->HP = lasthp;
				}
			}
			Waitframe();
		}
	}
}
