#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

#define POP_ARGS(num_args, t)                                           \
	for(int _i(0); _i < num_args; ++_i) \
		code.push_back(new OPopRegister(new VarArgument(t)))

using namespace ZScript;
using namespace ZScript::Libraries;

Screen const& Screen::singleton()
{
	static Screen const instance;
	return instance;
}

void Screen::addTo(Scope& scope) const
{
	TypeStore& typeStore = scope.getTypeStore();
	DataType tVoid = typeStore.getVoid();
	DataType tBool = typeStore.getBool();
	DataType tFloat = typeStore.getFloat();
	DataType tGame = typeStore.getGame();
	DataType tDebug = typeStore.getDebug();
	DataType tScreen = typeStore.getScreen();
	DataType tAudio = typeStore.getAudio();
	DataType tLink = typeStore.getLink();
	DataType tItemClass = typeStore.getItemClass();
	DataType tItem = typeStore.getItem();
	DataType tNpcClass = typeStore.getNpcClass();
	DataType tNpc = typeStore.getNpc();
	DataType tFfc = typeStore.getFfc();
	DataType tLWpn = typeStore.getLWpn();
	DataType tEWpn = typeStore.getEWpn();
	typedef VectorBuilder<DataType> P;
	
	LibraryHelper lh(scope, NUL, tScreen);
	LibraryHelper::call_tag const& asFunction = LibraryHelper::asFunction;

	addPair(lh, SDD, tFloat, "D", 8);
	addPair(lh, COMBODD, tFloat, "ComboD", 176);
	addPair(lh, COMBOCD, tFloat, "ComboC", 176);
	addPair(lh, COMBOFD, tFloat, "ComboF", 176);
	addPair(lh, COMBOID, tFloat, "ComboI", 176);
	addPair(lh, COMBOTD, tFloat, "ComboT", 176);
	addPair(lh, COMBOSD, tFloat, "ComboS", 176);
	addPair(lh, SCRDOORD, tFloat, "Door", 4);
	addPair(lh, SCREENSTATED, tBool, "State", 32);
	addPair(lh, LIT, tBool, "Lit");
	addPair(lh, WAVY, tFloat, "Wavy");
	addPair(lh, QUAKE, tFloat, "Quake");
	lh.addGetter(ITEMCOUNT, tFloat, "NumItems", asFunction);
	lh.addGetter(NPCCOUNT, tFloat, "NumNPCs", asFunction);
	lh.addGetter(LWPNCOUNT, tFloat, "NumLWeapons", asFunction);
	lh.addGetter(EWPNCOUNT, tFloat, "NumEWeapons", asFunction);
	addPair(lh, SCREENFLAGSD, tFloat, "Flags", 10);
	addPair(lh, SCREENEFLAGSD, tFloat, "EFlags", 3);
	lh.addGetter(ROOMTYPE, tFloat, "RoomType"); // make pair?
	addPair(lh, ROOMDATA, tFloat, "RoomData");
	addPair(lh, PUSHBLOCKX, tFloat, "MovingBlockX");
	addPair(lh, PUSHBLOCKY, tFloat, "MovingBlockY");
	addPair(lh, PUSHBLOCKCOMBO, tFloat, "MovingBlockCombo");
	addPair(lh, PUSHBLOCKCSET, tFloat, "MovingBlockCSet");
	addPair(lh, UNDERCOMBO, tFloat, "UnderCombo");
	addPair(lh, UNDERCSET, tFloat, "UnderCSet");

    // item LoadItem(screen this, int index)
    {
	    Function& function = lh.addFunction(
			    tItem, "LoadItem", P() << tFloat);
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadItemRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // item CreateItem(screen this, int itemId)
    {
	    Function& function = lh.addFunction(
			    tItem, "CreateItem", P() << tFloat);

        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateItemRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // ffc LoadFFC(screen this, int index)
    {
	    Function& function = lh.addFunction(tFfc, "LoadFFC", P() << tFloat);
        
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //code.push_back(new OSetRegister(new VarArgument(REFFFC), new VarArgument(EXP1)));
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // npc LoadNPC(screen this, int index)
    {
	    Function& function = lh.addFunction(tNpc, "LoadNPC", P() << tFloat);

	    int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadNPCRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // npc CreateNPC(screen this, int id)
    {
	    Function& function = lh.addFunction(
			    tNpc, "CreateNPC", P() << tFloat);
        
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateNPCRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // npc LoadLWeapon(screen this, int index)
    {
	    Function& function = lh.addFunction(
			    tLWpn, "LoadLWeapon", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadLWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // lweapon CreateLWeapon(screen this, int id)
    {
	    Function& function = lh.addFunction(
			    tLWpn, "CreateLWeapon", P() << tFloat);
        
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateLWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // lweapon CreateLWeaponDX(screen this, int type, int id)
    {
	    Function& function = lh.addFunction(
			    tLWpn, "CreateLWeaponDx", P() << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATELWPNDX)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
     
    // eweapon LoadEWeapon(screen this, int index)
    {
	    Function& function = lh.addFunction(
			    tEWpn, "LoadEWeapon", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadEWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // eweapon CreateEWeapon(screen this, int id)
    {
	    Function& function = lh.addFunction(
			    tEWpn, "CreateEWeapon", P() << tFloat);
        
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateEWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void ClearSprites(screen this, int)
    {
	    Function& function = lh.addFunction(
			    tVoid, "ClearSprites", P() << tFloat);
        
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OClearSpritesRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Rectangle(
    //   screen this, float, float, float, float, float, float,
    //   float, float, float, float, bool, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Rectangle",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ORectangleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        
        function.giveCode(code);
    }

    // void Circle(
    //   screen this, float, float, float, float, float,
    //   float, float, float, float, bool, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Circle",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OCircleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Arc(
    //    screen this, float, float, float, float, float, float,
    //    float, float, float, float, float, bool, bool, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Arc",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tBool << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OArcRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(14, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Ellipse(
    //    screen this, float, float, float, float,
    //    float, bool, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Ellipse",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OEllipseRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Line(
    //   screen this, float, float, float, float, float,
    //   float, float, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Line",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OLineRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Spline(
    //   screen this, float, float, float, float, float, float,
    //   float, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Spline",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OSplineRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void PutPixel(
    //   screen this, float, float, float, float,
    //   float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "PutPixel",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPutPixelRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void DrawCharacter(
    //   screen this, float, float, float, float, float,
    //   float, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawCharacter",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawCharRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(10, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void DrawInteger(
    //    screen this, float, float, float, float, float, float,
    //    float, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawInteger",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawIntRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void DrawTile(
    //   screen this, float, float, float, float,
    //   float, bool, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawTile",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawTileRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(15, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void DrawCombo(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawCombo",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tBool << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawComboRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(16, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Quad(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Quad",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OQuadRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(15, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Polygon(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Polygon",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPolygonRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void Triangle(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Triangle",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat);

        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OTriangleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(13, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Quad3D(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Quad3D",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OQuad3DRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Triangle3D(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Triangle3D",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OTriangle3DRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void FastTile(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "FastTile",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OFastTileRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void FastCombo(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "FastCombo",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OFastComboRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void DrawString(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawString",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawStringRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(9, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void DrawLayer(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawLayer",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawLayerRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void DrawScreen(screen, float, float, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawScreen",
			    P() << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawScreenRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void DrawBitmap(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawBitmap",
			    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tBool);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawBitmapRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void DrawBitmapEx(...)
    {
	    Function& function = lh.addFunction(
			    tVoid, "DrawBitmapEx",
			    P() << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat << tBool);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawBitmapExRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(16, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void SetRenderTarget(screen this, float bitmap)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetRenderTarget", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OSetRenderTargetRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void Message(screen this, float id)
    {
	    Function& function = lh.addFunction(
			    tVoid, "Message", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OMessageRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // bool isSolid(screen this, float x, float y)
    {
	    Function& function = lh.addFunction(
			    tBool, "isSolid", P() << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OIsSolid(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void SetSideWarp(screen this, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetSideWarp", P() << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OSetSideWarpRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void SetTileWarp(screen, float, float, float, float)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetTileWarp", P() << tFloat << tFloat << tFloat << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OSetTileWarpRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // float LayerScreen(screen this, float layer)
    {
	    Function& function = lh.addFunction(
			    tFloat, "LayerScreen", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OLayerScreenRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // float LayerMap(screen this, float layer)
    {
	    Function& function = lh.addFunction(
			    tFloat, "LayerMap", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OLayerMapRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void TriggerSecrets(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "TriggerSecrets", P());
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTriggerSecrets());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void ZapIn(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "ZapIn", P());
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OZapIn());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // void ZapOut(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "ZapOut", P());
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OZapOut());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // void OpeningWipe(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "OpeningWipe", P());
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OOpenWipe());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

	// void WavyIn(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "WavyIn", P());
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OWavyIn());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
            
	// void WavyOut(screen this)
    {
	    Function& function = lh.addFunction(tVoid, "WavyOut", P());
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OWavyOut());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    // float GetSideWarpDMap(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetSideWarpDMap", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpDMap(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // float GetSideWarpScreen(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetSideWarpScreen", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // float GetSideWarpType(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetSideWarpType", P() << tFloat);
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpType(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // float GetTileWarpDMap(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetTileWarpDMap", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpDMap(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // float GetTileWarpScreen(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetTileWarpScreen", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }

    // float GetTileWarpType(screen this, float warp)
    {
	    Function& function = lh.addFunction(
			    tFloat, "GetTileWarpType", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpType(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
    
    
    // void TriggerSecret(screen this, float secret)
    {
	    Function& function = lh.addFunction(
			    tVoid, "TriggerSecret", P() << tFloat);
	    
        int label = function.getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OTriggerSecretRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        function.giveCode(code);
    }
}
