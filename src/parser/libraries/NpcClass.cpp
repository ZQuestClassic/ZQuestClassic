#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

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
	typedef VectorBuilder<ZAsm::Variable> R;
	
	LibraryHelper lh(scope, varNull(), tNpcClass);

	lh.addGetter(varCSET(), tFloat, "CSet");

	/*
	  Use functions that accept two args (ID, value) per field for setters
	  use functions that accept one arg (ID) per field as getters
	  Plus One master function to set all attriutes using an array?
	*/   

	/*
	// bool NpcClass->isValid()
	defineFunction(
			lh, tBool, "isValid", P(), R() << varExp1(),
			new OIsValidEWpn(varExp1()));
    */
}
