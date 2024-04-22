#option WARN_DEPRECATED error
#option NO_ERROR_HALT on

import "std.zh"
import "ghost.zh"

const int ARMOS_JUMP_HEIGHT = 1.5;
// npc->Attribute[] indices
const int ARMOS_ATTR_START_SOUND = 0;
const int ARMOS_ATTR_JUMP_SOUND = 1;
ffc script Armos_LttP
{
void run(int enemyID)
{
	 npc ghost;
	 float step;
	 int jumpSound;
	
	 // Initialize - come to life and set the combo
	 ghost=Ghost_InitAutoGhost(this, enemyID);
	 Ghost_SetFlag(GHF_NORMAL);
	 Ghost_SetFlag(GHF_KNOCKBACK_4WAY);
	 Ghost_SetFlag(GHF_FAKE_Z);
	 Audio->PlaySound(ghost->Attributes[ARMOS_ATTR_START_SOUND]);
	 SpawnAnimation(this, ghost);
	
	 step=ghost->Step/100;
	 jumpSound=ghost->Attributes[ARMOS_ATTR_JUMP_SOUND];
	
	 // Just jump toward Link forever
	 while(true)
	 {
		 if(Ghost_Z==0 && Ghost_Jump<=0)
		 {
			 Ghost_Jump=ARMOS_JUMP_HEIGHT;
			 Audio->PlaySound(jumpSound);
		 }
		
		 Ghost_MoveTowardLink(step, 3);
		 Ghost_Waitframe(this, ghost, true, true);
	 }
}

// A modified version of Ghost_SpawnAnimationFlicker(). This removes the
// Armos combo near the end of the animation.
void SpawnAnimation(ffc this, npc ghost)
{
	 int combo=this->Data;
	 bool collDet=ghost->CollDetection;
	 int xOffset=ghost->DrawXOffset;
	
	 Ghost_SetPosition(this, ghost);
	 ghost->CollDetection=false;
	
	 // Alternate drawing offscreen and in place for 64 frames
	 for(int i=0; i<32; i++)
	 {
		 this->Data=0;
		 ghost->DrawXOffset=32768;
		 Ghost_SetPosition(this, ghost);
		 Ghost_WaitframeLight(this, ghost);
		
		 this->Data=combo;
		 ghost->DrawXOffset=xOffset;
		 Ghost_SetPosition(this, ghost);
		 Ghost_WaitframeLight(this, ghost);
		
		 // The combo has to be removed shortly before the animation
		 // finishes; otherwise, it's possible to spawn two of them.
		 if(i==29)
		 {
			 Screen->ComboD[ComboAt(this->X, this->Y)]=Screen->UnderCombo;
			 Screen->ComboC[ComboAt(this->X, this->Y)]=Screen->UnderCSet;
		 }
	 }
	
	 this->Data=combo;
	 ghost->CollDetection=collDet;
	 ghost->DrawXOffset=xOffset;
}
}