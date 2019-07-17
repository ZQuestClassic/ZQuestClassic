//ffscript.h

LINKOTILE

enum linkspritetype { LSprwalkspr, LSprstabspr, LSprslashspr, LSprfloatspr, 
	LSprswimspr, LSprdivespr, LSprpoundspr,
LSprjumpspr, LSprchargespr, LSprcastingspr, 
	LSprholdspr1, LSprholdspr2, LSprholdsprw1, LSprholdsprw2, LSprlast };

	
	case LINKOTILE:
        ret=FFCore.getLinkOTile((ri->d[0]/10000), (ri->d[1]/10000));
        break;
        
int FFScript::getLinkOTile(long index1, long index2)
{
    {
	linkspritetype lst = index1;
	int dir = index2;
	int the_ret = 0;
	switch(lst)
	{
		case LSprwalkspr: the_ret = walkspr[dir][0];
		case LSprstabspr: the_ret = stabspr[dir][0];
		case LSprslashspr: the_ret = slashspr[dir][0];
		case LSprfloatspr: the_ret = floatspr[dir][0];
		case LSprswimspr: the_ret = swimspr[dir][0];
		case LSprdivespr: the_ret = divespr[dir][0];
		case LSprpoundspr: the_ret = poundspr[dir][0];
		case LSprjumpspr: the_ret = jumpspr[dir][0];
		case LSprchargespr: the_ret = chargespr[dir][0];
		case LSprcastingspr: the_ret = castingspr[0];
		case LSprholdspr1: the_ret = holdspr[0][0][0];
		case LSprholdspr2:  the_ret = holdspr[0][1][0]
		case LSprholdsprw1: the_ret = holdspr[1][0][0]
		case LSprholdsprw2: the_ret = holdspr[1][1][0]
		default: the_ret = 0;
	}
	
	return the_ret*10000;
}

 //bytecode
    #define LINKOTILE v
    
    case LINKOTILE:
        return "LINKOTILE";
    
    
    //globalsymbols


//int GetLinkOTile(game, int,int)
    {
        Function* function = getFunction("GetLinkOTile", 3);
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(LINKOTILE)));
        code.push_back(new OReturn());
        function->giveCode(code);
    }