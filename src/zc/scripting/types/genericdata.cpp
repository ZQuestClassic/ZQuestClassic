#include "zc/scripting/types/genericdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

#include <cstdint>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

void user_genscript::clear()
{
	wait_atleast = true;
	waituntil = SCR_TIMING_START_FRAME;
	waitevent = false;
	exitState = 0;
	reloadState = 0;
	eventstate = 0;
	auto ind = scrconfig.script;
	scrconfig.clear();
	scrconfig.script = ind;
	data.clear();
	quit();
}

void user_genscript::launch()
{
	quit();
	doscript() = true;
	wait_atleast = true;
	waituntil = SCR_TIMING_START_FRAME;
	waitevent = false;
}

void user_genscript::quit()
{
	FFCore.reset_script_engine_data(ScriptType::Generic, scrconfig.script);
	_doscript = false;
}

byte& user_genscript::doscript()
{
	return _doscript;
}

byte const& user_genscript::doscript() const
{
	return _doscript;
}

user_genscript& user_genscript::get(int ind)
{
	if(ind < 1 || ind >= NUMSCRIPTSGENERIC)
		ind = 0;
	user_scripts[ind].scrconfig.script = ind;
	return user_scripts[ind];
}
user_genscript user_genscript::user_scripts[NUMSCRIPTSGENERIC];

user_genscript* checkGenericScr(int32_t ref)
{
	if (BC::checkBounds(ref, 1, NUMSCRIPTSGENERIC-1) != SH::_NoError)
		return NULL;

	return &user_genscript::get(ref);
}

int32_t genericdata_get_register(int32_t reg)
{
	int32_t ret = 0;
	user_genscript* genscr = checkGenericScr(GET_REF(genericdataref));

	switch (reg)
	{
		case GENDATARUNNING:
		{
			ret = 0;
			if (genscr)
			{
				ret = genscr->doscript() ? 10000L : 0L;
			}
			break;
		}
		case GENDATASIZE:
		{
			ret = 0;
			if (genscr)
			{
				ret = genscr->dataSize()*10000;
			}
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void genericdata_set_register(int32_t reg, int32_t value)
{
	user_genscript* genscr = checkGenericScr(GET_REF(genericdataref));
	if (!genscr)
		return;

	switch (reg)
	{
		case GENDATARUNNING:
		{
			if(value)
				genscr->launch();
			else genscr->quit();
			break;
		}
		case GENDATASIZE:
		{
			genscr->dataResize(value/10000);
			break;
		}

		default:
			NOTREACHED();
	}
}

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
	static ScriptingArray_ObjectSubMemberContainer<user_genscript, &user_genscript::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar GENDATADATA_registrar(GENDATADATA, []{
	static ScriptingArray_ObjectMemberContainer<user_genscript, &user_genscript::data> impl;
	impl.setMul10000(false);
	return &impl;
}());
