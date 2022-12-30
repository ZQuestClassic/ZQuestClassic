npc script killchildrenondeath
{
	void run()
	{
		while(1)
		{
			if ( this->HP <= 0 )
			{
				for ( int q = Screen->NumNPCs(); q > 0; --q )
				{
					npc n = Screen->LoadNPC(q);
					if ( n->ParentUID == this->UID ) 
					{
						Remove(n);
					}
				}
				for ( int w = Screen->NumEWeapons(); w > 0; --w )
				{
					eweapon n = Screen->LoadEWeapon(w);
					if ( n->ParentUID == this->UID ) 
					{
						Remove(n);
					}
				}
				
			}
			Waitframe();
		}
	}
}