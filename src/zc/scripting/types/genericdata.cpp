#include "zc/scripting/arrays.h"

// genericdata arrays.

static ArrayRegistrar GENDATAEVENTSTATE_registrar(GENDATAEVENTSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<user_genscript, &user_genscript::eventstate, GENSCR_NUMEVENT> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GENDATAEXITSTATE_registrar(GENDATAEXITSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<user_genscript, &user_genscript::exitState, GENSCR_NUMST> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GENDATARELOADSTATE_registrar(GENDATARELOADSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<user_genscript, &user_genscript::reloadState, GENSCR_NUMST> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar GENDATAINITD_registrar(GENDATAINITD, []{
	static ScriptingArray_ObjectMemberContainer<user_genscript, &user_genscript::initd> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar GENDATADATA_registrar(GENDATADATA, []{
	static ScriptingArray_ObjectMemberContainer<user_genscript, &user_genscript::data> impl;
	impl.setMul10000(false);
	return &impl;
}());
