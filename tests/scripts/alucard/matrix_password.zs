// https://www.purezc.net/index.php?page=scripts&id=397

#include "std.zh"

const int MATRIX_PUZZLE_ALLOW_INPUT_WITH_A_BUTTON = 1; //Set to anything > 0 to allow Link to input password by standing on combos and pressing A.
const int CF_MATRIX_PUZZLE_SKIP_COMBO = 67;//Combo flag to be places on combos that are not a part of a puzzle.
const int SFX_MATRIX_PUZZLE_COMBO_CHANGE = 16; //Sound to play, when changing password combos by standing on combos and pressing A.
const int SFX_MATRIX_PUZZLE_SOLVED = 27; //Sound to play, when correct password is inputed.
const int CF_MATRIX_PUZZLE_LENS_VIS = 14; //Combo flag, which, if placed onto password combos, allows Link to crack the code with Lens of Truth.
const int CF_NO_AUTO_INPUT = 98; //Flag puzzle with this to prevent Link to input password by standing on combos and pressing A on per-puzzle basis.

ffc script MatrixPasswordPuzzle{
	void run (int origcmb, int sizeX, int sizeY){
		if (Screen->State[ST_SECRET]) Quit();
		//Process creating puzzle solution and resetting combination lock.
		int origpos = ComboAt(CenterX(this), CenterY(this));
		int solution[176];
		for (int i=0; i<176; i++){
			if (ComboFI(i,CF_MATRIX_PUZZLE_SKIP_COMBO)){
				solution[i]=-1;
				continue;
			}
			if (ComboX(i)<ComboX(origpos)){
				solution[i]=-1;
				continue;
			}
			if (ComboX(i)>=(ComboX(origpos)+(16*sizeX))){
				solution[i]=-1;
				continue;
			}
			if (ComboY(i)<ComboY(origpos)){
				solution[i]=-1;
				continue;
			}
			if (ComboY(i)>=(ComboY(origpos)+(16*sizeY))){
				solution[i]=-1;
				continue;
			}
			solution[i]= Screen->ComboD[i] - origcmb;
			Screen->ComboD[i] = origcmb;
		}
		while(true){
			//Process Auto-input
			if (MATRIX_PUZZLE_ALLOW_INPUT_WITH_A_BUTTON>0){
				int cmb = ComboAt(CenterLinkX(), CenterLinkY());
					if ((solution[cmb]>=0)&&(!ComboFI(cmb,CF_NO_AUTO_INPUT ))){
						if (Link->PressA || Link->PressEx1){
							Game->PlaySound(SFX_MATRIX_PUZZLE_COMBO_CHANGE);
							Screen->ComboD[cmb]++;
						}
					}
			}
			//Process cracking the code with Lens of Truth
			if (UsingItem(I_LENS)){//Allow Lens of Truth to crack the code
					for (int i=0; i<176; i++){
					if ((ComboFI(i,CF_MATRIX_PUZZLE_LENS_VIS))&&(solution[i]>=0))
					Screen->FastCombo(4, ComboX(i), ComboY(i), origcmb+solution[i], Screen->ComboC[origpos], OP_OPAQUE);
				}
			}
			//Check, if correct password is given.
			for (int i=0; i<176; i++){
				if (ComboFI(i, CF_MATRIX_PUZZLE_SKIP_COMBO)) continue;
				if (solution[i]>=0){
					if (Screen->ComboD[i]!=(solution[i] + origcmb)) break;
					//else Screen->Rectangle(3, ComboX(i), ComboY(i), ComboX(i)+15, ComboY(i)+15, 1, 1, 0, 0, 0, false, OP_OPAQUE);
				}
				//else Screen->Rectangle(3, ComboX(i), ComboY(i), ComboX(i)+15, ComboY(i)+15, 5, 1, 0, 0, 0, false, OP_OPAQUE);
				if (i==175){
					Game->PlaySound(SFX_MATRIX_PUZZLE_SOLVED);
					Screen->TriggerSecrets();
					Screen->State[ST_SECRET]=true;
					Quit();
				}
			}
			Waitframe();
		}
	}
}
