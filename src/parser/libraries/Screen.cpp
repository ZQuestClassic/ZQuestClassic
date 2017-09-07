#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
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
	DataTypeSimple const* tVoid = &DataType::ZVOID;
	DataTypeSimple const* tBool = &DataType::BOOL;
	DataTypeSimple const* tFloat = &DataType::FLOAT;
	DataTypeClass const* tGame = &DataType::GAME;
	DataTypeClass const* tDebug = &DataType::DEBUG;
	DataTypeClass const* tScreen = &DataType::SCREEN;
	DataTypeClass const* tAudio = &DataType::AUDIO;
	DataTypeClass const* tLink = &DataType::_LINK;
	DataTypeClass const* tItemClass = &DataType::ITEMCLASS;
	DataTypeClass const* tItem = &DataType::ITEM;
	DataTypeClass const* tNpcClass = &DataType::NPCDATA;
	DataTypeClass const* tNpc = &DataType::NPC;
	DataTypeClass const* tFfc = &DataType::FFC;
	DataTypeClass const* tLWpn = &DataType::LWPN;
	DataTypeClass const* tEWpn = &DataType::EWPN;
	DataType const* tEnd = NULL;
	
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
			    tItem, "LoadItem", tFloat, tEnd);
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
			    tItem, "CreateItem", tFloat, tEnd);

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
	    Function& function = lh.addFunction(tFfc, "LoadFFC", tFloat, tEnd);
        
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
	    Function& function = lh.addFunction(tNpc, "LoadNPC", tFloat, tEnd);

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
			    tNpc, "CreateNPC", tFloat, tEnd);
        
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
			    tLWpn, "LoadLWeapon", tFloat, tEnd);
	    
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
			    tLWpn, "CreateLWeapon", tFloat, tEnd);
        
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
			    tLWpn, "CreateLWeaponDx", tFloat, tFloat, tEnd);
	    
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
			    tEWpn, "LoadEWeapon", tFloat, tEnd);
	    
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
			    tEWpn, "CreateEWeapon", tFloat, tEnd);
        
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
			    tVoid, "ClearSprites", tFloat, tEnd);
        
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tBool, tFloat,    
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tBool, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tBool, tBool, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tBool, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, 
	            tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tBool, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tBool, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat,
			    tEnd);

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
			    tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, 
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, 
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat, 
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat, tBool,
			    tEnd);
	    
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
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tFloat, tFloat, tFloat,
			    tFloat, tFloat, tFloat, tBool, 
			    tEnd);
	    
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
			    tVoid, "SetRenderTarget", tFloat, tEnd);
	    
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
			    tVoid, "Message", tFloat, tEnd);
	    
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
			    tBool, "isSolid", tFloat, tFloat, tEnd);
	    
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
			    tVoid, "SetSideWarp", tFloat, tFloat, tFloat, tFloat, tEnd);
	    
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
			    tVoid, "SetTileWarp", tFloat, tFloat, tFloat, tFloat, tEnd);
	    
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
			    tFloat, "LayerScreen", tFloat, tEnd);
	    
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
			    tFloat, "LayerMap", tFloat, tEnd);
	    
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
	    Function& function = lh.addFunction(tVoid, "TriggerSecrets", tEnd);
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
	    Function& function = lh.addFunction(tVoid, "ZapIn", tEnd);
	    
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
	    Function& function = lh.addFunction(tVoid, "ZapOut", tEnd);
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
	    Function& function = lh.addFunction(tVoid, "OpeningWipe", tEnd);
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
	    Function& function = lh.addFunction(tVoid, "WavyIn", tEnd);
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
	    Function& function = lh.addFunction(tVoid, "WavyOut", tEnd);
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
			    tFloat, "GetSideWarpDMap", tFloat, tEnd);
	    
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
			    tFloat, "GetSideWarpScreen", tFloat, tEnd);
	    
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
			    tFloat, "GetSideWarpType", tFloat, tEnd);
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
			    tFloat, "GetTileWarpDMap", tFloat, tEnd);
	    
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
			    tFloat, "GetTileWarpScreen", tFloat, tEnd);
	    
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
			    tFloat, "GetTileWarpType", tFloat, tEnd);
	    
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
			    tVoid, "TriggerSecret", tFloat, tEnd);
	    
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
