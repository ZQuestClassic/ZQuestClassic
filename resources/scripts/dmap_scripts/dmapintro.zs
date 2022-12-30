dmapdata script dmapintro
{
	void run(int intro_msg, bool always_show_intro, int altflag)
	{
		dointro(intro_msg, always_show_intro, Game->LoadDMapData(this->ID), ((altflag > 0) ? altflag : DMF_SCRIPT1));
	}
	void dointro(int intro_msg, bool always_show_intro, dmapdata d, int useflag)
	{
		unless ( always_show_intro )
		{
			if ( d->Flags&useflag ) return;
		}
		d->Flags |= useflag;
		Screen->Message(intro_msg);
		
	}
}
		
		