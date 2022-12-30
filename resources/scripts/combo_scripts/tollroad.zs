combodata script tollroad
{
	void run(int ctr, int msg)
	{
		while(1)
		{
			if ( ( Abs(Hero->X - this->PosX()) < 8 )
				&& ( Abs(Hero->Y - this->PosY()) < 8 )
			{
				Screen->Message(msg);
				Audio->PlaySound(this->Attribytes[0]);
				Game->Counter[ctr] = 0;
			}
			Waitframe();
		}
	}
}