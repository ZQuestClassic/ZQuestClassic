//import "std.zh"

int arr[NNN];
const int NNN = 4;

int a2a[25*(16+3)];
//int arr[NNN];

//ffc fn[16];
//lweapon lw[256];

ffc script f{
void run(){
//	lweapon l = Screen->CreateLWeapon(23);
//	l->UseSprite(40);
//	TraceLWeapon(l);
//	itemdata ww[6];
//	int sz = SizeOfArrayItemdata(ww);
/*
		for ( int q = 0; q < 256; q++ )
		{
			int ptrid = Debug->GetLWeaponPointer(lw[q]);
			TraceLWeapon(lw[q]); Trace(ptrid);
			if ( ptrid == -1 )
			{
				lw[q] = l; TraceLWeapon(lw[q]); break;
			}


		} */
		int xy;
		bitmap b;
		switch(xy)
		{

			case 1: break;
			case NNN: break;
			//case (NNN+16); break;
			default: break;
		}
		//bitmap b;
		//mapdata m;
		//spritedata sp;
		//combodata cd;
		//npcdata nd;

		//int n2 = NPCData->GetAttribute(56,4);

		int n1 = NPCData->GetTile(56);
		TraceS("NPCData->Tile: "); Trace(n1);
		NPCData->SetTile(56,150);
		//Trace(n2);
		int n3 = MapData->GetRoomType(Game->GetCurMap(), Game->GetCurScreen());
		TraceS("Mapdata->RoomType: "); Trace(n3);

		NPCData->SetSTile(56,19);

		int n4 = NPCData->GetSTile(56);
		TraceS("STile: "); Trace(n4);

		int n5 = NPCData->GetETile(56);
		TraceS("NPCData->ETile: "); Trace(n5);

		NPCData->SetETile(56,4886);

		Game->GameOverScreen[2] = 24380; //Tile
		Game->GameOverScreen[11] = 3; //MIDI

		//int d1 = Game->GetScreenEnemy(Game->GetCurMap(), Game->GetCurScreen(), 0);
		//Trace(d1);

		int dd = MapData->GetEnemy(Game->GetCurMap(), Game->GetCurScreen(), 0);
		TraceS("Mapdata->Enemy: ");Trace(dd);

		/* String Literals

		*/

		//TraceS("foo");
	}
}

/*
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

ffjbhsdkjbk
		//int ssss = SpriteData->GetTile(9);
		//int b = NPCData->GetWeapon(2);
		//int m = MapData->GetItem(1,1);

		//int d = Input->GetType(7);
		//int j = Input->Joypad[4];
		//if ( Input->Button[0] ) {
		//	Input->Mouse[4] = 1;
		//}

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
} */
