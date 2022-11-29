//import "std.zh"

ffc n[16];
lweapon lw[256];

ffc script f{
void run(){
	lweapon l = Screen->CreateLWeapon(23);
	l->UseSprite(40);
	TraceLWeapon(l);
	itemdata ww[6];
	int sz = SizeOfArrayItemdata(ww);

		for ( int q = 0; q < 256; q++ )
		{
			int ptrid = Debug->GetLWeaponPointer(lw[q]);
			TraceLWeapon(lw[q]); Trace(ptrid);
			if ( ptrid == -1 )
			{
				lw[q] = l; TraceLWeapon(lw[q]); break;
			}


		}
		bitmap b;
		mapdata m;
		spritedata sp;
		combodata cd;
		npcdata nd;
		int n = NPCData->GetTile(16);
		int n1 = nd->GetTile(16);
		Game->GameOverScreen[2] = 24380; //Tile
		Game->GameOverScreen[11] = 3; //MIDI

		/* String Literals

		*/

		TraceS("foo");
	}
}

global script active{
	void run(){
		while(1){
			if ( Input->Press[10] )
				for ( int q = 0; q < 256; q++ ) TraceLWeapon(lw[q]);
			Waitdraw(); Waitframe();
		}
	}
}

global script Init{
	void run(){
		for ( int q = 0; q < 256; q++ ) lw[q] = Debug->SetLWeaponPointer(-1);
		Game->GameOverScreen[0]=3;
		int s[]="Try Again";
		Game->GameOverStrings[0]=s;;
		/*
ffjbhsdkjbk
		//int ssss = SpriteData->GetTile(9);
		//int b = NPCData->GetWeapon(2);
		//int m = MapData->GetItem(1,1);

		//int d = Input->GetType(7);
		//int j = Input->Joypad[4];
		//if ( Input->Button[0] ) {
		//	Input->Mouse[4] = 1;
		//}
*/
		TraceNL();
		//Trace(Debug->RefFFC);
		TraceNL();
		//Trace(Debug->GDR[0]);
		//this->Data = 10;
	//this->X += 8;
		//int v = this->ID;
		//ffc nn = this->ID;
		//TraceFFC(nn);
		//if ( Game->JoypadPress[1] ) {}

	}
}

item script a{
	void run(){
		int v = this->ID;
		this->CSet = 6;
	}
}
