#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

NpcClass const& NpcClass::singleton()
{
	static NpcClass const instance;
	return instance;
}

void NpcClass::addTo(Scope& scope) const
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
	
	LibraryHelper lh(scope, NUL, tNpcClass);

	lh.addGetter(FCSET, tFloat, "CSet");

	/*
	  Use functions that accept two args (ID, value) per field for setters
	  use functions that accept one arg (ID) per field as getters
	  Plus One master function to set all attriutes using an array?
	*/   

	/*
	// bool NpcClass->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << EXP1,
			new OIsValidEWpn(new VarArgument(EXP1)));
    */
}
