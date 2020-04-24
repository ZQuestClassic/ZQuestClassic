bitmap CardFaceBuf;
bitmap CardFaceA;
bitmap CardFaceB;

ffc script CardFlipTest
{
	using namespace CardFlip;
	void run()
	{
		if(!CardFaceBuf)
		{
			CardFaceBuf = Game->AllocateBitmap();
			CardFaceBuf->Create(0, 256, 176);
		}
		if(!CardFaceA)
		{
			CardFaceA = Game->AllocateBitmap();
			CardFaceA->Create(0, 256, 256);
		}
		if(!CardFaceB)
		{
			CardFaceB = Game->AllocateBitmap();
			CardFaceB->Create(0, 256, 256);
		}
		
		//Draw your card faces of the desired size to the bitmap. Bitmap should be 256x256
		CardFaceA->DrawScreen(0, 1, 0x10, 0, 0, 0);
		CardFaceB->DrawScreen(0, 1, 0x11, 0, 0, 0);
		
		int i;
		
		int cp[3] = {128, 88, 0}; //X, Y, Z position of the center of the card
		int texScale[2] = {64, 96}; //Width, Height of the texture
		int scale[2] = {64, 96}; //Width, Height of the card
		int rot[3]; //Yaw, Pitch, Roll of the card
		
		//Card facing down
		for(i=0; i<90; i++)
		{
			CardFaceBuf->ClearToColor(0, 0);
			DrawCard(CardFaceBuf, CardFaceA, CardFaceB, cp, texScale, scale, rot);
			CardFaceBuf->Blit(0, -2, 0, 0, 256, 176, 0, 0, 256, 176, 0, 0, 0, 0, 0, true);
			
			Waitframe();
		}
		//Flipping animation
		for(i=0; i<90; i++)
		{
			cp[2] += (i<45 ? 4 : -4);
			rot[1] += 18;
			//Scale the card up as it rises on the Z-axis since Quad3D doesn't actually do this
			scale[0] = texScale[0]*(1+0.5*(cp[2]/180));
			scale[1] = texScale[1]*(1+0.5*(cp[2]/180));
			
			CardFaceBuf->ClearToColor(0, 0);
			DrawCard(CardFaceBuf, CardFaceA, CardFaceB, cp, texScale, scale, rot);
			CardFaceBuf->Blit(0, -2, 0, 0, 256, 176, 0, 0, 256, 176, 0, 0, 0, 0, 0, true);
			
			Waitframe();
		}
		scale[0] = texScale[0];
		scale[1] = texScale[1];
			
		//Card facing up
		for(i=0; i<90; i++)
		{
			CardFaceBuf->ClearToColor(0, 0);
			DrawCard(CardFaceBuf, CardFaceA, CardFaceB, cp, texScale, scale, rot);
			CardFaceBuf->Blit(0, -2, 0, 0, 256, 176, 0, 0, 256, 176, 0, 0, 0, 0, 0, true);
			
			Waitframe();
		}
	}
}

ffc script CardRotationTest
{
	using namespace CardFlip;
	void run()
	{
		if(!CardFaceBuf)
		{
			CardFaceBuf = Game->AllocateBitmap();
			CardFaceBuf->Create(0, 256, 176);
		}
		if(!CardFaceA)
		{
			CardFaceA = Game->AllocateBitmap();
			CardFaceA->Create(0, 256, 256);
		}
		if(!CardFaceB)
		{
			CardFaceB = Game->AllocateBitmap();
			CardFaceB->Create(0, 256, 256);
		}
		
		CardFaceA->DrawScreen(0, 1, 0x10, 0, 0, 0);
		CardFaceB->DrawScreen(0, 1, 0x11, 0, 0, 0);
		
		int i;
		
		int cp[3]; //X, Y, Z position of the center of the card
		int texScale[2] = {64, 96}; //Width, Height of the texture
		int scale[2] = {64, 96}; //Width, Height of the card
		int rot[3]; //Yaw, Pitch, Roll of the card
		
		int changeRot[3] = {Rand(-2, 2), Rand(-2, 2), Rand(-2, 2)};
		while(true)
		{
			cp[0] = Link->InputMouseX;
			cp[1] = Link->InputMouseY;
			
			rot[0] += changeRot[0];
			rot[1] += changeRot[1];
			rot[2] += changeRot[2];
			
			CardFaceBuf->ClearToColor(0, 0);
			DrawCard(CardFaceBuf, CardFaceA, CardFaceB, cp, texScale, scale, rot);
			CardFaceBuf->Blit(0, -2, 0, 0, 256, 176, 0, 0, 256, 176, 0, 0, 0, 0, 0, true);
			Waitframe();
		}
	}
}

namespace CardFlip
{
	void DrawCard(bitmap buf, bitmap faceA, bitmap faceB, int cp[], int texScale[], int scale[], int rot[])
	{
		rot[0] = WrapDegrees(rot[0]);
		rot[1] = WrapDegrees(rot[1]);
		rot[2] = WrapDegrees(rot[2]);
		
		//In order to prevent faceB from being flipped horizontally
		//we flip one of the rotations before transforming
		//and then undo the flip afterwards.
		//Quad3D()'s flip argument wasn't working when I tried
		int undoRot1 = 0;
		
		//Select which face to use depending on the rotation transformations
		//Two flips will cancel each other out
		bitmap face = faceA;
		if(rot[0]<-90||rot[0]>90)
		{
			face = face==faceA ? faceB : faceA;
		}
		if(rot[1]<-90||rot[1]>90)
		{
			face = face==faceA ? faceB : faceA;
			if(face==faceB)
				undoRot1 = -180;
		}
		
		rot[1] += undoRot1;
		
		//Points of the quad
		int px[4];
		int py[4];
		int pz[4];
		
		//Set the points according to scale[] and their base rotation of 0,0,0
		px[0] = -scale[0]/2;
		py[0] = -scale[1]/2;
		
		px[1] = scale[0]/2;
		py[1] = -scale[1]/2;
		
		px[2] = scale[0]/2;
		py[2] = scale[1]/2;
		
		px[3] = -scale[0]/2;
		py[3] = scale[1]/2;
		
		//Apply the three rotation transformations
		RotateX3D(rot[0], px, py, pz);
		RotateY3D(rot[1], px, py, pz);
		RotateZ3D(rot[2], px, py, pz);
		
		rot[1] -= undoRot1;
		
		int pos[12];
		
		//Add the transformed points to the center point
		pos[0] = cp[0]+px[0];
		pos[1] = cp[1]+py[0];
		pos[2] = cp[2]+pz[0];
		
		pos[3] = cp[0]+px[1];
		pos[4] = cp[1]+py[1];
		pos[5] = cp[2]+pz[1];
		
		pos[6] = cp[0]+px[2];
		pos[7] = cp[1]+py[2];
		pos[8] = cp[2]+pz[2];
		
		pos[9] = cp[0]+px[3];
		pos[10] = cp[1]+py[3];
		pos[11] = cp[2]+pz[3];
		
		int uv[8] = {0,0,  texScale[0]-1,0,  texScale[0]-1,texScale[1]-1,  0,texScale[1]-1};
		
		int cset[4] = {0,0,0,0};
		int size[2] = {1,1};
		
		//Draw the quad to the buffer bitmap for cards, to then be drawn to the screen
		buf->Quad3D(0, pos, uv, cset, size, 0, 0, PT_TEXTURE, face);
	}
	int VectorX3D(int distance, int yaw, int pitch)
	{
		return distance*Sin(pitch)*Cos(yaw);
	}

	int VectorY3D(int distance, int yaw, int pitch)
	{
		return distance*Sin(pitch)*Sin(yaw);
	}

	int VectorZ3D(int distance, int yaw, int pitch)
	{
		return distance*Cos(pitch);
	}

	void RotateX3D(int angle, int px, int py, int pz)
	{
		for(int i=0; i<SizeOfArray(px); i++)
		{
			int y = py[i];
			int z = pz[i];
			
			py[i] = y * Cos(angle) - z * Sin(angle);
			pz[i] = z * Cos(angle) + y * Sin(angle);
		}
	}

	void RotateY3D(int angle, int px, int py, int pz)
	{
		for(int i=0; i<SizeOfArray(px); i++)
		{
			int x = px[i];
			int z = pz[i];
			
			px[i] = x * Cos(angle) - z * Sin(angle);
			pz[i] = z * Cos(angle) + x * Sin(angle);
		}
	}

	void RotateZ3D(int angle, int px, int py, int pz)
	{
		for(int i=0; i<SizeOfArray(px); i++)
		{
			int x = px[i];
			int y = py[i];
			
			px[i] = x * Cos(angle) - y * Sin(angle);
			py[i] = y * Cos(angle) + x * Sin(angle);
		}
	}
}