NPCDEAD
NPCKICKBUCKET
NPCSTOPBGSFX
NPCCANMOVE
NPCNEWDIR8
NPCNEWDIR
NPCCONSTWALK
NPCVARWALK
NPCHALTWALK
NPCFLOATWALK
NPCLINEDUP
NPCLINKINRANGE
NPCATTACK
NPCPLACEONAXIS
NPCADD
NPCFIREBREATH
NPCCANSLIDE
NPCSLIDE
NPCHITWITH 
NPCCOLLISION




bool NPCDEAD() //no args
//bool bpc->isDead()
//similar to isValid()

string ONPCDead::toString()
{
    return "NPCDEAD " + getArgument()->toString();
}

class ONPCDead : public UnaryOpcode
	{
	public:
		ONPCDead(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCDead(a->clone());
		}
	};
	
	{
	    Function* function = getFunction("isDead");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new ONPCDead(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }


bool NPCCANSLIDE() //no args
//npc->CanSlide()
//similar to isValid
    
    string ONPCCanSlide::toString()
{
    return "NPCCANSLIDE " + getArgument()->toString();
}

class ONPCCanSlide : public UnaryOpcode
	{
	public:
		ONPCCanSlide(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCCanSlide(a->clone());
		}
	};
	
	{
	    Function* function = getFunction("CanSlide");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new ONPCCanSlide(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

bool NPCSLIDE() //no args
//bool npc->Slide()
// or
//bool npc->Knockback()
//similar to isValid
//this would be better as npc->Knockback(int dir), setting sclk
//where first byte is clk, second byte is dir

        string ONPCSlide::toString()
{
    return "NPCSLIDE " + getArgument()->toString();
}

class ONPCSlide : public UnaryOpcode
	{
	public:
		ONPCSlide(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCSlide(a->clone());
		}
	};
	
	{
	    Function* function = getFunction("Slide");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new ONPCSlide(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

    //////////////
    
void NPCKICKBUCKET() //no args -> npc->kickbucket()
//npc->Remove()
//similar to BreakShield()

 string ONPCRemove::toString()
{
    return "NPCKICKBUCKET " + getArgument()->toString();
}

    class ONPCRemove : public UnaryOpcode
	{
	public:
		ONPCRemove(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCRemove(a->clone());
		}
	};
	
	{
	    Function* function = getFunction("Remove");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Break shield
        code.push_back(new ONPCRemove(new VarArgument(EXP1)));
        code.push_back(new OReturn());
         function->giveCode(code);
    }
    
void NPCSTOPBGSFX() //no args -> npc->stop_bgsfx(index)
//void npc->StopBGSFX
//similar to BreakShield()#
    
     string ONPCStopSFX::toString()
{
    return "NPCSTOPBGSFX " + getArgument()->toString();
}

    class ONPCStopSFX : public UnaryOpcode
	{
	public:
		ONPCStopSFX(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCStopSFX(a->clone());
		}
	};
	
	{
	    Function* function = getFunction("StopBGSFX");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Break shield
        code.push_back(new ONPCStopSFX(new VarArgument(EXP1)));
        code.push_back(new OReturn());
         function->giveCode(code);
    }
    
void NPCATTACK() //no args
//void FireWeapon();
//void npc->FireWeapon() or //npc->Attack()
//similar to BreakShield()
    
         string ONPCAttack::toString()
{
    return "NPCATTACK " + getArgument()->toString();
}

    class ONPCAttack : public UnaryOpcode
	{
	public:
		ONPCAttack(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCAttack(a->clone());
		}
	};
	
	{
    
    
	    Function* function = getFunction("Attack");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Break shield
        code.push_back(new ONPCAttack(new VarArgument(EXP1)));
        code.push_back(new OReturn());
         function->giveCode(code);
    }
    
void NPCNEWDIR(int arr{}) //if arr is NULL, use newdir()
//otherwise, use void newdir(int rate,int homing,int special);
//npc->NewDir4(int arr[])
//similar to GetName()

    string ONPCNewDir::toString()
{
    return "NPCNEWDIR " + getArgument()->toString();
}
    class ONPCNewDir : public UnaryOpcode
	{
	public:
		ONPCNewDir(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCNewDir(a->clone());
		}
	};
    {
	Function* function = getFunction("NewDir");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCNewDir(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }



void NPCCONSTWALK(int arr) //same as NPCNEWDIR, with NULL calling
//void constant_walk(); or otherwise  void constant_walk(int rate,int homing,int special);
//npc->ConstantWalk(int arr[])
//similar to GetName()
    
       string ONPCConstWalk::toString()
{
    return "NPCCONSTWALK " + getArgument()->toString();
}
    class ONPCConstWalk : public UnaryOpcode
	{
	public:
		ONPCConstWalk(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCConstWalk(a->clone());
		}
	};
    {
	Function* function = getFunction("ConstantWalk");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCConstWalk(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

void NPCVARWALK(int arr[]) //3, 4, or 7 args in array format
//npc->VariableWalk({args});
//void variable_walk(int rate,int homing,int special);
//void variable_walk_8(int rate,int homing,int newclk,int special);
//void variable_walk_8(int rate,int homing,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
    // the variable speed floater movement
//similar to GetName()
    
        string ONPCVarWalk::toString()
{
    return "NPCVARWALK " + getArgument()->toString();
}
    class ONPCVarWalk : public UnaryOpcode
	{
	public:
		ONPCVarWalk(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCVarWalk(a->clone());
		}
	};
    {
	Function* function = getFunction("VariableWalk");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCVarWalk(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

void NPCHALTWALK(arr[]) //5 or 6 args as array
//void halting_walk(int rate,int homing,int special,int hrate, int haltcnt);
//void halting_walk_8(int newrate,int newhoming, int newclk,int special,int newhrate, int haltcnt);
//void npc->HaltingWalk({args});
//similar to GetName()
    #
          string ONPCHaltWalk::toString()
{
    return "NPCHALTWALK " + getArgument()->toString();
}
    class ONPCHaltWalk : public UnaryOpcode
	{
	public:
		ONPCHaltWalk(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCHaltWalk(a->clone());
		}
	};
    {
	Function* function = getFunction("HaltingWalk");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCHaltWalk(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }


void NPCFLOATWALK(arr[]) //3 or 7 args
// the variable speed floater movement
    // ms is max speed
    // ss is step speed
    // s is step count
    // p is pause count
    // g is graduality :)
    //void floater_walk(int rate,int newclk,fix ms,fix ss,int s,int p, int g);
    //void floater_walk(int rate,int newclk,fix s);
//void npc->FloatingWalk({args});
//similar to GetName()
    #
     string ONPCFloatWalk::toString()
{
    return "NPCFLOATWALK " + getArgument()->toString();
}
    class ONPCFloatWalk : public UnaryOpcode
	{
	public:
		ONPCFloatWalk(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCFloatWalk(a->clone());
		}
	};
    {
	Function* function = getFunction("FloatingWalk");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCFloatWalk(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

void NPCFIREBREATH(bool seeklink)
//npc->BreathFire(bool seeklink)
//similar to GetName()
    
    string ONPCBreatheFire::toString()
{
    return "NPCFIREBREATH " + getArgument()->toString();
}
    class ONPCBreatheFire : public UnaryOpcode
	{
	public:
		ONPCBreatheFire(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCBreatheFire(a->clone());
		}
	};
    {
	Function* function = getFunction("BreathAttack");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCBreatheFire(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }


void NPCNEWDIR8(arr[]) //1 stack reg, as array ptr with 3, or 7 args
//void npc->NewDir8({args})
//void newdir_8(int rate,int homing, int special,int dx1,int dy1,int dx2,int dy2);
//void newdir_8(int rate,int homing, int special);
//similar to GetName()
    
    string ONPCNewDir8::toString()
{
    return "NPCNEWDIR8 " + getArgument()->toString();
}
    class ONPCNewDir8 : public UnaryOpcode
	{
	public:
		ONPCNewDir8(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCNewDir8(a->clone());
		}
	};
    {
	Function* function = getFunction("NewDir8");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCNewDir8(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }


    
    //these two are vars, not commands

bool NPCCOLLISION(int type, untyped ptr) //returns if the npc collided with an object of type, ptr
//bool npc->Collision(int obj_type, untyped ptr)
//TWO INPUT ONE RET?
//perhaps similar to loadmapdata?
    
    //int LoadMapData(mapdata, int map,int scr)
    {
        Function* function = getFunction("Collision");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCCOLLISION)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

int NPCLINEDUP(int, bool) //var, not functiom
// // Checks if enemy is lined up with Link. If so, returns direction Link is
    // at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
    //int lined_up(int range, bool dir8);
//int npc->LinedUp(int, bool)
//two inout one ret, similar to loadmapdata


//int LoadMapData(mapdata, int map,int scr)
    {
        Function* function = getFunction("LinedUp");
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(NPCLINEDUP)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

    ////////////////////////to decide





bool NPCLINKINRANGE(int)
//bool LinkInRange(int range);
// returns true if Link is within 'range' pixels of the enemy
//bool npc->LinkInRange(int dist_pix)
//ONE INPUT ONE RET
    
    //similar to GetDMapMusicTrack
    
    string ONPCLinkInRange::toString()
{
    return "NPCLINKINRANGE " + getArgument()->toString();
}

class ONPCLinkInRange : public UnaryOpcode
	{
	public:
		ONPCLinkInRange(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCLinkInRange(a->clone());
		}
	};
	
     //int GetDMapMusicTrack(game, int)
    {
	    Function* function = getFunction("LinkInRange");
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCLinkInRange(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }

npc NPCADD(args[])
//create an npc abnd return its refinfo
//npc->Create({args})
//similar to createnpc?
//ONE INPUT ONE RET
    
    string ONPCAdd::toString()
{
    return "NPCADD " + getArgument()->toString();
}

    class ONPCAdd : public UnaryOpcode
	{
	public:
		ONPCAdd(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCAdd(a->clone());
		}
	};

	//npc CreateNPC(screen, int)
    {
	    Function* function = getFunction("Create");
        
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCAdd(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }
    
bool NPCCANMOVE(arr[]) //1 stack reg, as array ptr with 1, 2, 3, or 7 args
//bool npc->CanMove({literal array})
    //bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2);
    //bool canmove(int ndir,fix s,int special);
    //bool canmove(int ndir,int special);
    //bool canmove(int ndir);
//ONE INPUT ONE RET
    
     //similar to GetDMapMusicTrack
    
    string ONPCCanMove::toString()
{
    return "NPCCANMOVE " + getArgument()->toString();
}

class ONPCCanMove : public UnaryOpcode
	{
	public:
		ONPCCanMove(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCCanMove(a->clone());
		}
	};
	
     //int GetDMapMusicTrack(game, int)
    {
	    Function* function = getFunction("CanMove");
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCCanMove(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }


bool NPCHITWITH({ARGS})
//args as array?
//one input one ret//similar to getdmapmusictrack

string ONPCHitWith::toString()
{
    return "NPCHITWITH " + getArgument()->toString();
}

class ONPCHitWith : public UnaryOpcode
	{
	public:
		ONPCHitWith(Argument *A) : UnaryOpcode(A) {}
		string toString();
		Opcode *clone()
		{
			return new ONPCHitWith(a->clone());
		}
	};
	
     //int GetDMapMusicTrack(game, int)
    {
	    Function* function = getFunction("SimulateHit");
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new ONPCHitWith(new VarArgument(EXP1)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }




void NPCPLACEONAXIS(bool, bool)
//// place the enemy in line with Link (red wizzrobes)
    //void place_on_axis(bool floater, bool solid_ok);
    //npc->LineUp(bool,bool)
    
    //TWO INPUT NO RET
    





virtual bool hit(sprite *s);
virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
virtual bool hit(weapon *w);

//Hit with a weapon, or a specific set of args for positions

