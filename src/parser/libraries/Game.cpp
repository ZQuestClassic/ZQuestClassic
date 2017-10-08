#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

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
	typedef VectorBuilder<ZAsm::Variable> R;
	typedef VectorBuilder<Opcode> O;
	
	LibraryHelper lh(scope, varNull(), tGame);
	LibraryHelper::call_tag const& asFunction = LibraryHelper::asFunction;

	addPair(lh, varGAMEDEATHS(), tFloat, "NumDeaths");
	addPair(lh, varGAMECHEAT(), tFloat, "Cheat");
	addPair(lh, varGAMETIME(), tFloat, "Time");
	addPair(lh, varGAMEHASPLAYED(), tBool, "HasPlayed");
	addPair(lh, varGAMETIMEVALID(), tBool, "TimeValid");
	addPair(lh, varGAMEGUYCOUNT(), tFloat, "GuyCount", 2); // XXX 2?
	addPair(lh, varGAMECONTSCR(), tFloat, "ContinueScreen");
	addPair(lh, varGAMECONTDMAP(), tFloat, "ContinueDMap");
	addPair(lh, varGAMECOUNTERD(), tFloat, "Counter", 32);
	addPair(lh, varGAMEMCOUNTERD(), tFloat, "MCounter", 32);
	addPair(lh, varGAMEDCOUNTERD(), tFloat, "DCounter", 32);
	addPair(lh, varGAMEGENERICD(), tFloat, "Generic", 256);
	addPair(lh, varGAMELITEMSD(), tFloat, "LItems", 512);
	addPair(lh, varGAMELKEYSD(), tFloat, "LKeys", 512);
	addPair(lh, varDMAPFLAGSD(), tFloat, "DMapFlags", 512);
	addPair(lh, varDMAPLEVELD(), tFloat, "DMapLevel", 512);
	addPair(lh, varDMAPCOMPASSD(), tFloat, "DMapCompass", 512);
	addPair(lh, varDMAPCONTINUED(), tFloat, "DMapContinue", 512);
	addPair(lh, varDMAPMIDID(), tFloat, "DMapMIDI", 512);
	addPair(lh, varGAMESTANDALONE(), tBool, "Standalone");
	addPair(lh, varGAMEENTRSCR(), tFloat, "LastEntranceScreen");
	addPair(lh, varGAMEENTRDMAP(), tFloat, "LastEntranceDMap");
	addPair(lh, varGAMECLICKFREEZE(), tBool, "ClickToFreezeEnabled");
	addPair(lh, varDMAPOFFSET(), tFloat, "DMapOffset", 512);
	addPair(lh, varDMAPMAP(), tFloat, "DMapMap", 512);

	// Joypad and Keyboard. -Z
	addPair(lh, varBUTTONPRESS(), tBool, "ButtonPress", 18);
	addPair(lh, varBUTTONINPUT(), tBool, "ButtonInput", 18);
	addPair(lh, varBUTTONHELD(), tBool, "ButtonHeld", 18);
	lh.addGetter(varREADKEY(), tBool, "ReadKey", 127);
	addPair(lh, varKEYPRESS(), tBool, "KeyPress", 127);
	lh.addGetter(varJOYPADPRESS(), tFloat, "JoypadPress", 18);

	addPair(lh, varDISABLEDITEM(), tBool, "DisableItem", 256);
	addPair(lh, varDMAPLEVELPAL(), tFloat, "DMapPalette", 512);
	lh.addGetter(varZELDAVERSION(), tFloat, "Version");
	lh.addGetter(varZELDABUILD(), tFloat, "Build");
	lh.addGetter(varZELDABETA(), tFloat, "Beta");
	addPair(lh, varNOACTIVESUBSC(), tBool, "DisableActiveSubscreen");
	lh.addGetter(varGAMENUMMESSAGES(), tFloat, "HighestStringID");
	lh.addGetter(varGAMENUMMESSAGES(), tFloat, "NumMessages");

	lh.addGetter(varCURSCR(), tFloat, "GetCurScreen", asFunction);
	lh.addGetter(varCURDSCR(), tFloat, "GetCurDMapScreen", asFunction);
	lh.addGetter(varCURMAP(), tFloat, "GetCurMap", asFunction);
	lh.addGetter(varCURDMAP(), tFloat, "GetCurDMap", asFunction);
	lh.addGetter(varCURLEVEL(), tFloat, "GetCurLevel", asFunction);
	lh.addGetter(varGETMIDI(), tFloat, "GetMIDI", asFunction);
	lh.addGetter(varGAMEMAXMAPS(), tFloat, "MapCount", asFunction);
	
	// itemclass Game->LoadItemData(float id)
	defineFunction(
			lh, tItemClass, "LoadItemData",
			P() << tFloat, R() << varExp1(),
			O() << opSUBV(varExp1(), 10000)
			    << opLOADITEMDATAR(varExp1())
			    << opSETR(varExp1(), varREFITEMCLASS()));

    // bool Game->GetScreenState(float, float, float)
    {
	    Function& function = lh.addFunction(
			    tBool, "GetScreenState",
			    P() << tFloat << tFloat << tFloat);
	    
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode> code;
        // Pop off the parameterss
        code.push_back(opPOP(varIndex2()));
        code.push_back(opPOP(varIndex1()));
        code.push_back(opPOP(varExp1()));
        code.push_back(opPOP(varNull()));
        // Find position.
        code.push_back(opSUBV(varExp1(), 10000));
        code.push_back(opMULTV(varExp1(), 1360000));
        code.push_back(opADDR(varIndex1(), varExp1()));
        // Grab the screen state.
        code.push_back(opSETR(varExp1(), varSCREENSTATEDD()));
        // Interpret as bool.
        code.push_back(opCOMPAREV(varExp1(), 0));
        code.push_back(opGOTOTRUE(done));
        code.push_back(opSETV(varExp1(), 10000));
        // Return from function. (done label)
        appendReturn(code, done);
        function.setCode(code);
    }

    // void Game->SetScreenState(float, float, float, bool)
    {
	    Function& function = lh.addFunction(
			    tVoid, "SetScreenState",
			    P() << tFloat << tFloat << tFloat << tBool);
	    
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode> code;
        // Pop off the parameters
        code.push_back(opPOP(varSFTemp()));
        code.push_back(opPOP(varIndex2()));
        code.push_back(opPOP(varIndex1()));
        code.push_back(opPOP(varExp1()));
        code.push_back(opPOP(varNull()));
        // Find position.
        code.push_back(opSUBV(varExp1(), 10000));
        code.push_back(opMULTV(varExp1(), 1360000));
        code.push_back(opADDR(varIndex1(), varExp1()));
        // Convert last argument to bool.
        code.push_back(opCOMPAREV(varSFTemp(), 0));
        code.push_back(opGOTOTRUE(done));
        code.push_back(opSETV(varSFTemp(), 10000));
        // Set the state.
        code.push_back(opSETR(varSCREENSTATEDD(), varSFTemp())
                       .withLabel(done));
        // Return from function.
        appendReturn(code);
        function.setCode(code);
    }

    // float Game->GetScreenD(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenD",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSDDD()));

    // void Game->SetScreenD(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenD",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSDDD(), varSFTemp()));

    // float Game->GetDMapScreenD(float, float, float)
    defineFunction(
		    lh, tFloat, "GetDMapScreenD",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varExp1(), varSDDDD()));

    // void Game->SetDMapScreenD(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetDMapScreenD",
		    P() << tFloat   << tFloat    << tFloat  << tFloat,
		    R() << varIndex1() << varIndex2() << varExp1() << varSFTemp(),
		    opSETR(varSDDDD(), varSFTemp()));

    // void Game->PlaySound(float id)
    defineFunction(
		    lh, tVoid, "PlaySound",
		    P() << tFloat, R() << varExp1(),
		    opPLAYSOUNDR(varExp1()));

    // void Game->PlayMIDI(float id)
    defineFunction(
		    lh, tVoid, "PlayMIDI",
		    P() << tFloat, R() << varExp1(),
		    opPLAYMIDIR(varExp1()));

    // void Game->PlayEnhancedMusic(float, float)
    defineFunction(
		    lh, tVoid, "PlayEnhancedMusic",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opPLAYENHMUSIC(varExp2(), varExp1()));

    // void Game->GetDMapMusicFilename(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapMusicFilename",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opGETMUSICFILE(varExp2(), varExp1()));

    // float Game->GetDMapMusicTrack(float)
    defineFunction(
		    lh, tFloat, "GetDMapMusicTrack",
		    P() << tFloat, R() << varExp1(),
		    opGETMUSICTRACK(varExp1()));

    // void Game->SetDMapEnhancedMusic(float, float, float)
    defineFunction(
		    lh, tVoid, "SetDMapEnhancedMusic",
		    P() << tFloat << tFloat << tFloat,
		    opSETDMAPENHMUSIC());

    // float Game->GetComboData(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboData",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBODDM()));

    // void Game->SetComboData(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboData",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBODDM(), varExp2()));

    // float Game->GetComboCSet(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboCSet",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBOCDM()));

    // void Game->SetComboCSet(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboCSet",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBOCDM(), varExp2()));

    // float Game->GetComboFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboFlag",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBOFDM()));

    // void Game->SetComboFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboFlag",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBOFDM(), varExp2()));

    // float Game->GetComboType(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboType",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBOTDM()));

    // void Game->SetComboType(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboType",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBOTDM(), varExp2()));

    // float Game->GetComboInherentFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboInherentFlag",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBOIDM()));

    // void Game->SetComboInherentFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboInherentFlag",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBOIDM(), varExp2()));

    // float Game->GetComboSolid(float, float, float)
    defineFunction(
		    lh, tFloat, "GetComboSolid",
		    P() << tFloat    << tFloat  << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1(),
		    opSETR(varExp1(), varCOMBOSDM()));
    
    // void Game->SetComboSolid(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetComboSolid",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varCOMBOSDM(), varExp2()));

    // float Game->GetScreenFlags(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenFlags",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENFLAGS(varExp1()));

    // float Game->GetScreenEFlags(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEFlags",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENEFLAGS(varExp1()));

    // void Game->Save()
    defineFunction(lh, "Save", opSAVE());

    // void Game->End()
    defineFunction(lh, "End", opGAMEEND());

    // float Game->ComboTile(float)
    defineFunction(
		    lh, tFloat, "ComboTile",
		    P() << tFloat, R() << varExp2(),
		    opCOMBOTILE(varExp1(), varExp2()));

    // void Game->GetSaveName(string buffer)
    defineFunction(
		    lh, tVoid, "GetSaveName",
		    P() << tFloat, R() << varExp1(),
		    opGETSAVENAME(varExp1()));

    // void Game->SetSaveName(string buffer)
    defineFunction(
		    lh, tVoid, "SetSaveName",
		    P() << tFloat, R() << varExp1(),
		    opSETSAVENAME(varExp1()));
    
    // void Game->GetMessage(game, float, float)
    defineFunction(
		    lh, tVoid, "GetMessage",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opGETMESSAGE(varExp2(), varExp1()));
    
    // void Game->GetDMapName(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapName",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opGETDMAPNAME(varExp2(), varExp1()));

    // void Game->GetDMapTitle(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapTitle",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opGETDMAPTITLE(varExp2(), varExp1()));

    // void Game->GetDMapIntro(float, float)
    defineFunction(
		    lh, tVoid, "GetDMapIntro",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opGETDMAPINTRO(varExp2(), varExp1()));
    
    // void Game->GreyscaleOn()
    defineFunction(lh, "GreyscaleOn", opGREYSCALEON());
            
	// void Game->GreyscaleOff()
    defineFunction(lh, "GreyscaleOff", opGREYSCALEOFF());
    
    // void Game->SetMessage(float, float)
    defineFunction(
		    lh, tVoid, "SetMessage",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opSETMESSAGE(varExp2(), varExp1()));

    // void Game->SetDMapName(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapName",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opSETDMAPNAME(varExp2(), varExp1()));

    // void Game->SetDMapTitle(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapTitle",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opSETDMAPTITLE(varExp2(), varExp1()));

    // void Game->SetDMapIntro(float, float)
    defineFunction(
		    lh, tVoid, "SetDMapIntro",
		    P() << tFloat  << tFloat,
		    R() << varExp2() << varExp1(),
		    opSETDMAPINTRO(varExp2(), varExp1()));
    
    // bool Game->ShowSaveScreen()
    defineFunction(
		    lh, tBool, "ShowSaveScreen",
		    opSAVESCREEN(varExp1()));
    
    // void Game->ShowSaveQuitScreen()
    defineFunction(lh, "ShowSaveQuitScreen", opSAVEQUITSCREEN());
    
    // float Game->GetFFCScript(float)
    defineFunction(
		    lh, tFloat, "GetFFCScript",
		    P() << tFloat, R() << varExp1(),
		    opGETFFCSCRIPT(varExp1()));
    
    // float Game->GetItemScript(float)
    defineFunction(
		    lh, tFloat, "GetItemScript",
		    P() << tFloat, R() << varExp1(),
		    opGETITEMSCRIPT(varExp1()));
    
    // float Game->GetScreenEnemy(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEnemy",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENENEMY(varExp1()));

    // float Game->GetScreenDoor(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenDoor",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENDOOR(varExp1()));

    // void Game->SetScreenEnemy(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEnemy",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENENEMY(), varExp2()));

    // void Game->SetScreenDoor(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenDoor",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENDOOR(), varExp2()));
    
    // void Game->SetScreenWidth(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWidth",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENWIDTH(), varSFTemp()));
    
    // float Game->GetScreenWidth(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWidth",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENWIDTH()));
    
    // void Game->SetScreenHeight(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenHeight",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENHEIGHT(), varSFTemp()));

    // float Game->GetScreenHeight(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenHeight",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENHEIGHT()));

    // void Game->SetScreenViewX(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenViewX",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENVIEWX(), varSFTemp()));

    // float Game->GetScreenViewX(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenViewX",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENVIEWX()));
    
    // void Game->SetScreenViewY(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenViewY",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENVIEWY(), varSFTemp()));

    // float Game->GetScreenViewY(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenViewY",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENVIEWY()));

    // void Game->SetScreenGuy(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenGuy",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENGUY(), varSFTemp()));

    // float Game->GetScreenGuy(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenGuy",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENGUY()));

    // void Game->SetScreenString(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenString",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENSTRING(), varSFTemp()));

    // float Game->GetScreenString(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenString",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENSTRING()));

    // void Game->SetScreenRoomType(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenRoomType",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENROOM(), varSFTemp()));
    
    // float Game->GetScreenRoomType(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenRoomType",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENROOM()));

    // void Game->SetScreenEntryX(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEntryX",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENENTX(), varSFTemp()));

    // float Game->GetScreenEntryX(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEntryX",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENENTX()));

    // void Game->SetScreenEntryY(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenEntryY",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENENTY(), varSFTemp()));

    // float Game->GetScreenEntryY(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenEntryY",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENENTY()));

    // void Game->SetScreenItem(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenItem",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENITEM(), varSFTemp()));

    // float Game->GetScreenItem(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenItem",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENITEM()));

    // void Game->SetScreenUndercombo(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenUndercombo",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENUNDCMB(), varSFTemp()));

    // float Game->GetScreenUndercombo(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenUndercombo",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENUNDCMB()));

    // void Game->SetScreenUnderCSet(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenUnderCSet",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENUNDCST(), varSFTemp()));

    // float Game->GetScreenUnderCSet(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenUnderCSet",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENUNDCST()));

    // void Game->SetScreenCatchall(float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenCatchall",
		    P() << tFloat   << tFloat    << tFloat,
		    R() << varIndex1() << varIndex2() << varSFTemp(),
		    opSETR(varSETSCREENCATCH(), varSFTemp()));

    // float Game->GetScreenCatchall(float, float)
    defineFunction(
		    lh, tFloat, "GetScreenCatchall",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varSETSCREENCATCH()));
    
    // void Game->SetScreenLayerOpacity(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerOpacity",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENLAYOP(), varExp2()));

    // float Game->GetScreenLayerOpacity(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerOpacity",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENLAYOP(varExp1()));

    // void Game->SetScreenSecretCombo(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretCombo",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENSECCMB(), varExp2()));
    
    // float Game->GetScreenSecretCombo(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretCombo",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENSECCMB(varExp1()));

    // void Game->SetScreenSecretCSet(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretCSet",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENSECCST(), varExp2()));

    // float Game->GetScreenSecretCSet(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretCSet",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENSECCST(varExp1()));

    // void Game->SetScreenSecretFlag(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenSecretFlag",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENSECFLG(), varExp2()));

    // float Game->GetScreenSecretFlag(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenSecretFlag",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENSECFLG(varExp1()));

    // void Game->SetScreenLayerMap(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerMap",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENLAYMAP(), varExp2()));

    // float Game->GetScreenLayerMap(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerMap",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENLAYMAP(varExp1()));

    // void Game->SetScreenLayerScreen(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenLayerScreen",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENLAYSCR(), varExp2()));

    // float Game->GetScreenLayerScreen(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenLayerScreen",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENLAYSCR(varExp1()));

    // void Game->SetScreenPath(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenPath",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENPATH(), varExp2()));

    // float Game->GetScreenPath(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenPath",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENPATH(varExp1()));

    // void Game->SetScreenWarpReturnX(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWarpReturnX",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENWARPRX(), varExp2()));

    // float Game->GetScreenWarpReturnX(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWarpReturnX",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENWARPRX(varExp1()));

    // void Game->SetScreenWarpReturnY(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetScreenWarpReturnY",
		    P() << tFloat    << tFloat  << tFloat   << tFloat,
		    R() << varIndex2() << varExp1() << varIndex1() << varExp2(),
		    opSETR(varSETSCREENWARPRY(), varExp2()));

    // float Game->GetScreenWarpReturnY(float, float, float)
    defineFunction(
		    lh, tFloat, "GetScreenWarpReturnY",
		    P() << tFloat  << tFloat    << tFloat,
		    R() << varExp1() << varIndex2() << varIndex1(),
		    opGETSCREENWARPRY(varExp1()));
}
