#include "std.zh"

const int LTTP_BUMPER_FORCE = 3; //How fast Link gets pushed back when he hits a bumper
const int LTTP_BUMPER_ANIM_SPEED = 4; //How fast the bumpers animate in frames
const int SFX_LTTP_BUMPER = 76; //The sound that plays when Link hits a bumper

const int MAX_PUSH = 4; //Max speed the script can move Link in one frame

//Combo Setup: Each set of bumper combos should be arranged as so: 
//Bumper Frame 1
//Bumper Frame 2
//Bumper Frame 3
//Bumper Frame 4
//Each of the four combos is a frame in the bumper's animation.
//When Link hits the bumper it changes to frame 4 and then goes back a combo every LTTP_BUMPER_ANIM_SPEED frames.
//The FFC should use the first of the four bumper combos.
//D0: How many frames Link gets pushed back for when he hits the bumper

ffc script LttP_Bumper{
	void run(int Bounce){
		int Combo = this->Data;
		int BounceAngle = 0;
		int BounceCounter = 0;
		this->InitD[7] = 0;
		int AnimationCounter = 0;
		int PushX;
		int PushY;
		while(true){
			if(Distance(CenterX(this), CenterY(this), CenterLinkX(), CenterLinkY())<this->TileWidth*8+2&&Link->Z==0){
				if(this->InitD[7]==0){
					Game->PlaySound(SFX_LTTP_BUMPER);
					BounceAngle = Angle(CenterX(this), CenterY(this), CenterLinkX(), CenterLinkY());
					BounceCounter = Bounce;
					AnimationCounter = LTTP_BUMPER_ANIM_SPEED*4;
				}
				NoAction();
			}
			if(BounceCounter>0){
				PushX += VectorX(LTTP_BUMPER_FORCE, BounceAngle);
				PushY += VectorY(LTTP_BUMPER_FORCE, BounceAngle);
				BounceCounter--;
			}
			if(Abs(PushX)>0){
				for(int i=0; i<MAX_PUSH&&PushX<=-1; i++){
					if(CanWalk(Link->X, Link->Y, DIR_LEFT, 1, false)){
						Link->X--;
						PushX++;
					}
					else{
						if(Abs(PushY)>0)
							PushY += Sign(PushY)*Abs(PushX/2);
						PushX = 0;
					}
				}
				for(int i=0; i<MAX_PUSH&&PushX>=1; i++){
					if(CanWalk(Link->X, Link->Y, DIR_RIGHT, 1, false)){
						Link->X++;
						PushX--;
					}
					else{
						if(Abs(PushY)>0)
							PushY += Sign(PushY)*Abs(PushX/2);
						PushX = 0;
					}
				}
			}
			if(Abs(PushY)>0){
				for(int i=0; i<MAX_PUSH&&PushY<=-1; i++){
					if(CanWalk(Link->X, Link->Y, DIR_UP, 1, false)){
						Link->Y--;
						PushY++;
					}
					else{
						if(Abs(PushX)>0)
							PushX += Sign(PushX)*Abs(PushY/2);
						PushY = 0;
					}
				}
				for(int i=0; i<MAX_PUSH&&PushY>=1; i++){
					if(CanWalk(Link->X, Link->Y, DIR_DOWN, 1, false)){
						Link->Y++;
						PushY--;
					}
					else{
						if(Abs(PushX)>0)
							PushX += Sign(PushX)*Abs(PushY/2);
						PushY = 0;
					}
				}
			}
			if(AnimationCounter>0)
				AnimationCounter--;
			this->Data = Combo+Floor(AnimationCounter/LTTP_BUMPER_ANIM_SPEED);
			Waitframe();
		}
	}
}