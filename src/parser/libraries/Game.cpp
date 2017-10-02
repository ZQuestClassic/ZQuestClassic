#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Game const& Game::singleton()
{
	static Game const instance;
	return instance;
}

void Game::addTo(Scope& scope) const
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
	typedef VectorBuilder<int> R;
	typedef VectorBuilder<Opcode*> O;
	
	LibraryHelper lh(scope, NUL, tGame);
	LibraryHelper::call_tag const& asFunction = LibraryHelper::asFunction;

	addPair(lh, GAMEDEATHS, tFloat, "NumDeaths");
	addPair(lh, GAMECHEAT, tFloat, "Cheat");
	addPair(lh, GAMETIME, tFloat, "Time");
	addPair(lh, GAMEHASPLAYED, tBool, "HasPlayed");
	addPair(lh, GAMETIMEVALID, tBool, "TimeValid");
	addPair(lh, GAMEGUYCOUNT, tFloat, "GuyCount", 2); // XXX 2?
	addPair(lh, GAMECONTSCR, tFloat, "ContinueScreen");
	addPair(lh, GAMECONTDMAP, tFloat, "ContinueDMap");
	addPair(lh, GAMECOUNTERD, tFloat, "Counter", 32);
	addPair(lh, GAMEMCOUNTERD, tFloat, "MCounter", 32);
	addPair(lh, GAMEDCOUNTERD, tFloat, "DCounter", 32);
	addPair(lh, GAMEGENERICD, tFloat, "Generic", 256);
	addPair(lh, GAMELITEMSD, tFloat, "LItems", 512);
	addPair(lh, GAMELKEYSD, tFloat, "LKeys", 512);
	addPair(lh, DMAPFLAGSD, tFloat, "DMapFlags", 512);
	addPair(lh, DMAPLEVELD, tFloat, "DMapLevel", 512);
	addPair(lh, DMAPCOMPASSD, tFloat, "DMapCompass", 512);
	addPair(lh, DMAPCONTINUED, tFloat, "DMapContinue", 512);
	addPair(lh, DMAPMIDID, tFloat, "DMapMIDI", 512);
	addPair(lh, GAMESTANDALONE, tBool, "Standalone");
	addPair(lh, GAMEENTRSCR, tFloat, "LastEntranceScreen");
	addPair(lh, GAMEENTRDMAP, tFloat, "LastEntranceDMap");
	addPair(lh, GAMECLICKFREEZE, tBool, "ClickToFreezeEnabled");
	addPair(lh, DMAPOFFSET, tFloat, "DMapOffset", 512);
	addPair(lh, DMAPMAP, tFloat, "DMapMap", 512);

	// Joypad and Keyboard. -Z
	addPair(lh, BUTTONPRESS, tBool, "ButtonPress", 18);
	addPair(lh, BUTTONINPUT, tBool, "ButtonInput", 18);
	addPair(lh, BUTTONHELD, tBool, "ButtonHeld", 18);
	lh.addGetter(READKEY, tBool, "ReadKey", 127);
	addPair(lh, KEYPRESS, tBool, "KeyPress", 127);
	lh.addGetter(JOYPADPRESS, tFloat, "JoypadPress", 18);

	addPair(lh, DISABLEDITEM, tBool, "DisableItem", 256);
	addPair(lh, DMAPLEVELPAL, tFloat, "DMapPalette", 512);
	lh.addGetter(ZELDAVERSION, tFloat, "Version");
	lh.addGetter(ZELDABUILD, tFloat, "Build");
	lh.addGetter(ZELDABETA, tFloat, "Beta");
	addPair(lh, NOACTIVESUBSC, tBool, "DisableActiveSubscreen");
	lh.addGetter(GAMENUMMESSAGES, tFloat, "HighestStringID");
	lh.addGetter(GAMENUMMESSAGES, tFloat, "NumMessages");

	lh.addGetter(CURSCR, tFloat, "GetCurScreen", asFunction);
	lh.addGetter(CURDSCR, tFloat, "GetCurDMapScreen", asFunction);
	lh.addGetter(CURMAP, tFloat, "GetCurMap", asFunction);
	lh.addGetter(CURDMAP, tFloat, "GetCurDMap", asFunction);
	lh.addGetter(CURLEVEL, tFloat, "GetCurLevel", asFunction);
	lh.addGetter(GETMIDI, tFloat, "GetMIDI", asFunction);
	lh.addGetter(GAMEMAXMAPS, tFloat, "MapCount", asFunction);
	
	// itemclass Game->LoadItemData(float id)
	defineFunction(
			lh, tItemClass, "LoadItemData",
			P() << tFloat, R() << EXP1,
			O() << new OSubImmediate(new VarArgument(EXP1),
			                         new LiteralArgument(10000))
			    << new OLoadItemDataRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFITEMCLASS)));

    // bool Game->GetScreenState(float, float, float)
    {
	    Function& function = lh.addFunction(
			    tBool, "GetScreenState",
			    P() << tFloat << tFloat << tFloat);
	    
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode*> code;
        // Pop off the parameterss
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        // Find position.
        code.push_back(new OSubImmediate(new VarArgument(EXP1),
                                         new LiteralArgument(10000)));
        code.push_back(new OMultImmediate(new VarArgument(EXP1),
                                          new LiteralArgument(1360000)));
        code.push_back(new OAddRegister(new VarArgument(INDEX),
                                        new VarArgument(EXP1)));
        // Grab the screen state.
        code.push_back(new OSetRegister(new VarArgument(EXP1),
                                        new VarArgument(SCREENSTATEDD)));
        // Interpret as bool.
        code.push_back(new OCompareImmediate(new VarArgument(EXP1),
                                             new LiteralArgument(0)));
        code.push_back(new OGotoTrueImmediate(new LabelArgument(done)));
        code.push_back(new OSetImmediate(new VarArgument(EXP1),
                                         new LiteralArgument(10000)));
        // Return from function. (done label)
        appendReturn(code, done);

        code.front()->setLabel(function.getLabel());
        function.giveCode(code);
    }

    // void Game->SetScreenState(float, float, float, bool)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetScreenState",
			    P() << tFloat << tFloat << tFloat << tBool);
	    
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode*> code;
        // Pop off the parameters
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        // Find position.
        code.push_back(new OSubImmediate(new VarArgument(EXP1),
                                         new LiteralArgument(10000)));
        code.push_back(new OMultImmediate(new VarArgument(EXP1),
                                          new LiteralArgument(1360000)));
        code.push_back(new OAddRegister(new VarArgument(INDEX),
                                        new VarArgument(EXP1)));
        // Convert last argument to bool.
        code.push_back(new OCompareImmediate(new VarArgument(SFTEMP),
                                             new LiteralArgument(0)));
        code.push_back(new OGotoTrueImmediate(new LabelArgument(done)));
        code.push_back(new OSetImmediate(new VarArgument(SFTEMP),
                                         new LiteralArgument(10000)));
        // Set the state.
        code.push_back(new OSetRegister(new VarArgument(SCREENSTATEDD),
                                        new VarArgument(SFTEMP)));
        code.back()->setLabel(done);
        // Return from function.
        appendReturn(code);

        code.front()->setLabel(function.getLabel());
        function.giveCode(code);
    }
    
    // float Game->GetScreenD(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenD",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SDDD)));

    // void Game->SetScreenD(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenD",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SDDD),
		                     new VarArgument(SFTEMP)));
    
    // float Game->GetDMapScreenD(float, float, float)
    defineFunction(
		    lh, tFloat, "GetDMapScreenD",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SDDDD)));

    // void Game->SetDMapScreenD(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetDMapScreenD",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 <<   EXP1 << SFTEMP,
		    new OSetRegister(new VarArgument(SDDDD),
		                     new VarArgument(SFTEMP)));

    // void Game->PlaySound(float id)
    defineFunction(
		    lh, tVoid, "PlaySound",
		    P() << tFloat, R() << EXP1,
		    new OPlaySoundRegister(new VarArgument(EXP1)));

    // void Game->PlayMIDI(float id)
    defineFunction(
		    lh, tVoid, "PlayMIDI",
		    P() << tFloat, R() << EXP1,
		    new OPlayMIDIRegister(new VarArgument(EXP1)));

    // void Game->PlayEnhancedMusic(float, float)
    defineFunction(
		    lh, tVoid, "PlayEnhancedMusic",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OPlayEnhancedMusic(new VarArgument(EXP2),
		                           new VarArgument(EXP1)));

    // void Game->GetDMapMusicFilename(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapMusicFilename",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OGetDMapMusicFilename(new VarArgument(EXP2),
		                              new VarArgument(EXP1)));

    // float Game->GetDMapMusicTrack(float)
    defineFunction(
		    lh, tFloat, "GetDMapMusicTrack",
		    P() << tFloat, R() << EXP1,
		    new OGetDMapMusicTrack(new VarArgument(EXP1)));

    // void Game->SetDMapEnhancedMusic(float, float, float)
    defineFunction(
		    lh, tVoid, "SetDMapEnhancedMusic",
		    P() << tFloat << tFloat << tFloat,
		    new OSetDMapEnhancedMusic());

    // float Game->GetComboData(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboData",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBODDM)));

    // void Game->SetComboData(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboData",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBODDM),
		                     new VarArgument(EXP2)));

    // float Game->GetComboCSet(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboCSet",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBOCDM)));

    // void Game->SetComboCSet(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboCSet",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBOCDM),
		                     new VarArgument(EXP2)));

    // float Game->GetComboFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboFlag",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBOFDM)));

    // void Game->SetComboFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboFlag",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBOFDM),
		                     new VarArgument(EXP2)));

    // float Game->GetComboType(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboType",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBOTDM)));

    // void Game->SetComboType(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboType",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBOTDM),
		                     new VarArgument(EXP2)));

    // float Game->GetComboInherentFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboInherentFlag",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBOIDM)));

    // void Game->SetComboInherentFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboInherentFlag",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBOIDM),
		                     new VarArgument(EXP2)));

    // float Game->GetComboSolid(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboSolid",
		    P() << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(COMBOSDM)));
    
    // void Game->SetComboSolid(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboSolid",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(COMBOSDM),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenFlags(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenFlags",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenFlags(new VarArgument(EXP1)));

    // float Game->GetScreenEFlags(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEFlags",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenEFlags(new VarArgument(EXP1)));

    // void Game->Save()
    defineFunction(lh, "Save", new OSave());

    // void Game->End()
    defineFunction(lh, "End", new OEnd());

    // float Game->ComboTile(float)
    defineFunction(
		    lh, tFloat, "ComboTile",
		    P() << tFloat, R() << EXP2,
		    new OComboTile(new VarArgument(EXP1), new VarArgument(EXP2)));

    // void Game->GetSaveName(string buffer)
    defineFunction(
		    lh, tVoid, "GetSaveName",
		    P() << tFloat, R() << EXP1,
		    new OGetSaveName(new VarArgument(EXP1)));

    // void Game->SetSaveName(string buffer)
    defineFunction(
		    lh, tVoid, "SetSaveName",
		    P() << tFloat, R() << EXP1,
		    new OSetSaveName(new VarArgument(EXP1)));
    
    // void Game->GetMessage(game, float, float)
    defineFunction(
		    lh, tVoid, "GetMessage",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OGetMessage(new VarArgument(EXP2),
		                    new VarArgument(EXP1)));
    
    // void Game->GetDMapName(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapName",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OGetDMapName(new VarArgument(EXP2),
		                     new VarArgument(EXP1)));

    // void Game->GetDMapTitle(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapTitle",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OGetDMapTitle(new VarArgument(EXP2),
		                      new VarArgument(EXP1)));

    // void Game->GetDMapIntro(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapIntro",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OGetDMapIntro(new VarArgument(EXP2),
		                      new VarArgument(EXP1)));
    
    // void Game->GreyscaleOn()
    defineFunction(lh, "GreyscaleOn", new OGreyscaleOn());
            
	// void Game->GreyscaleOff()
    defineFunction(lh, "GreyscaleOff", new OGreyscaleOff());
    
    // void Game->SetMessage(float, float)
    defineFunction(
		    lh, tVoid, "SetMessage",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OSetMessage(new VarArgument(EXP2),
		                    new VarArgument(EXP1)));

    // void Game->SetDMapName(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapName",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OSetDMapName(new VarArgument(EXP2),
		                     new VarArgument(EXP1)));

    // void Game->SetDMapTitle(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapTitle",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OSetDMapTitle(new VarArgument(EXP2),
		                      new VarArgument(EXP1)));

    // void Game->SetDMapIntro(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapIntro",
		    P() << tFloat << tFloat,
		    R() <<   EXP2 <<   EXP1,
		    new OSetDMapIntro(new VarArgument(EXP2),
		                      new VarArgument(EXP1)));
    
    // bool Game->ShowSaveScreen()
    defineFunction(
		    lh, tBool, "ShowSaveScreen",
		    new OShowSaveScreen(new VarArgument(EXP1)));
    
    // void Game->ShowSaveQuitScreen()
    defineFunction(lh, "ShowSaveQuitScreen", new OShowSaveQuitScreen());
    
    // float Game->GetFFCScript(float)
    defineFunction(
		    lh, tFloat, "GetFFCScript",
		    P() << tFloat, R() << EXP1,
		    new OGetFFCScript(new VarArgument(EXP1)));
    
    // float Game->GetItemScript(float)
    defineFunction(
		    lh, tFloat, "GetItemScript",
		    P() << tFloat, R() << EXP1,
		    new OGetItemScript(new VarArgument(EXP1)));
    
    // float Game->GetScreenEnemy(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEnemy",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenEnemy(new VarArgument(EXP1)));

    // float Game->GetScreenDoor(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenDoor",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenDoor(new VarArgument(EXP1)));

    // void Game->SetScreenEnemy(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEnemy",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENENEMY),
		                     new VarArgument(EXP2)));

    // void Game->SetScreenDoor(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenDoor",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENDOOR),
		                     new VarArgument(EXP2)));
    
    // void Game->SetScreenWidth(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWidth",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENWIDTH),
		                     new VarArgument(SFTEMP)));
    
    // float Game->GetScreenWidth(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWidth",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENWIDTH)));
    
    // void Game->SetScreenHeight(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenHeight",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENHEIGHT),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenHeight(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenHeight",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENHEIGHT)));

    // void Game->SetScreenViewX(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenViewX",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENVIEWX),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenViewX(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenViewX",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENVIEWX)));
    
    // void Game->SetScreenViewY(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenViewY",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENVIEWY),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenViewY(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenViewY",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENVIEWY)));

    // void Game->SetScreenGuy(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenGuy",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENGUY),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenGuy(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenGuy",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENGUY)));

    // void Game->SetScreenString(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenString",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENSTRING),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenString(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenString",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENSTRING)));

    // void Game->SetScreenRoomType(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenRoomType",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENROOM),
		                     new VarArgument(SFTEMP)));
    
    // float Game->GetScreenRoomType(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenRoomType",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENROOM)));

    // void Game->SetScreenEntryX(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEntryX",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENENTX),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenEntryX(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEntryX",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENENTX)));

    // void Game->SetScreenEntryY(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEntryY",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENENTY),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenEntryY(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEntryY",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENENTY)));

    // void Game->SetScreenItem(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenItem",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENITEM),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenItem(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenItem",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENITEM)));

    // void Game->SetScreenUndercombo(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenUndercombo",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENUNDCMB),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenUndercombo(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenUndercombo",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENUNDCMB)));

    // void Game->SetScreenUnderCSet(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenUnderCSet",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENUNDCST),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenUnderCSet(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenUnderCSet",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENUNDCST)));

    // void Game->SetScreenCatchall(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenCatchall",
		    P() << tFloat << tFloat << tFloat,
		    R() <<  INDEX << INDEX2 << SFTEMP,
		    new OSetRegister(new VarArgument(SETSCREENCATCH),
		                     new VarArgument(SFTEMP)));

    // float Game->GetScreenCatchall(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenCatchall",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SETSCREENCATCH)));
    
    // void Game->SetScreenLayerOpacity(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerOpacity",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENLAYOP),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenLayerOpacity(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerOpacity",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenLayerOpacity(new VarArgument(EXP1)));

    // void Game->SetScreenSecretCombo(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretCombo",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENSECCMB),
		                     new VarArgument(EXP2)));
    
    // float Game->GetScreenSecretCombo(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretCombo",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenSecretCombo(new VarArgument(EXP1)));

    // void Game->SetScreenSecretCSet(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretCSet",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENSECCST),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenSecretCSet(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretCSet",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenSecretCSet(new VarArgument(EXP1)));

    // void Game->SetScreenSecretFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretFlag",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENSECFLG),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenSecretFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretFlag",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenSecretFlag(new VarArgument(EXP1)));

    // void Game->SetScreenLayerMap(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerMap",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENLAYMAP),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenLayerMap(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerMap",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenLayerMap(new VarArgument(EXP1)));

    // void Game->SetScreenLayerScreen(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerScreen",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENLAYSCR),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenLayerScreen(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerScreen",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenLayerScreen(new VarArgument(EXP1)));

    // void Game->SetScreenPath(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenPath",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENPATH),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenPath(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenPath",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenPath(new VarArgument(EXP1)));

    // void Game->SetScreenWarpReturnX(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWarpReturnX",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENWARPRX),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenWarpReturnX(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWarpReturnX",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenWarpReturnX(new VarArgument(EXP1)));

    // void Game->SetScreenWarpReturnY(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWarpReturnY",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    R() << INDEX2 <<   EXP1 <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SETSCREENWARPRY),
		                     new VarArgument(EXP2)));

    // float Game->GetScreenWarpReturnY(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWarpReturnY",
		    P() << tFloat << tFloat << tFloat,
		    R() <<   EXP1 << INDEX2 <<  INDEX,
		    new OGetScreenWarpReturnY(new VarArgument(EXP1)));
}
