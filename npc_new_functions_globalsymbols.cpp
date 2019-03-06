//bool isDead(npc)
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
//bool CanSlide(npc)
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
//bool Slide(npc)
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
//void Remove(npc)
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
//void StopBGSFX(npc)
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
//void Attack(npc)
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
//void NewDir(int arr[])
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
//void ConstantWalk(int arr[])
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
//void VariableWalk(int arr[])
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
//void HaltingWalk(int arr[])
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
//void FloatingWalk(int arr[])
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
//void BreathAttack(bool seeklink)
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
//void NewDir8(int arr[])
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
//bool Collision(int obj_type, untyped obj_pointer)
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
//int LinedUp(int range, bool dir8)
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


//bool LinkInRange(int dist_in_pixels)
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
    
    //npc Create(int array[])
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
    //bool CanMove(int array[])
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
    //bool SimulateHit(int array[])
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