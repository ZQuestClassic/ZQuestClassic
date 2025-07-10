#include "fmt/core.h"
#include "parser/ByteCode.h"
#include "parser/LibrarySymbols.h"
#include "parser/CompileError.h"
#include "parser/Types.h"
#include "parser/parserDefs.h"
#include "parser/symbols/SymbolDefs.h"
#include "symbols/SymbolDefs.h"
#include <fmt/format.h>

void getConstant(int32_t refVar, Function* function, int32_t val)
{
	if(refVar != NUL)
	{
		throw compile_exception(fmt::format("Internal Constant {} has non-NUL refVar!", function->name));
	}
	function->setFlag(FUNCFLAG_INLINE);
	function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	addOpcode2(code, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val)));
	LABELBACK(label);
	function->giveCode(code);
	function->set_constexpr(CONSTEXPR_CBACK_HEADER(val)
		{
			return val;
		});
}

void getVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop object pointer
	if(refVar == NUL)
	{
		function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
		LABELBACK(label);
	}
	else
	{
		//Pop object pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(refVar)));
		LABELBACK(label);
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
	}
	RETURN();
	function->giveCode(code);
}

void getIndexedVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop index
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
	LABELBACK(label);
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
	RETURN();
	function->giveCode(code);
}

void getInternalArray(int32_t refVar, Function* function, int32_t zasm_var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;

	if (refVar == NUL)
	{
		function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
		addOpcode2 (code, new OLoadInternalArray(new VarArgument(EXP1), new LiteralVarArgument(zasm_var)));
		LABELBACK(label);
	}
	else
	{
		addOpcode2 (code, new OLoadInternalArrayRef(new VarArgument(EXP1), new LiteralVarArgument(zasm_var), new VarArgument(refVar)));
		LABELBACK(label);
		//pop object pointer
		if (refVar == REFSCREENDATA)
			function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
		else
			POPREF();
	}

	RETURN();
	function->giveCode(code);
}

void setVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	LABELBACK(label);
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

void setBoolVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	LABELBACK(label);
	//renormalize true to 1
	addOpcode2 (code, new OCastBoolI(new VarArgument(EXP1)));
	
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

void setIndexedVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off index
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
	LABELBACK(label);
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

static std::map<std::string, CONSTEXPR_CBACK_TY> const_expr_global_impl = {
	{"Untype", CONSTEXPR_CBACK_HEADER()
		{
			return args[0];
		}
	},

	{"Pow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				if(!*args[0] && !*args[1])
					val = 1; //0^0
				else val = int(pow(*args[0]/10000.0,*args[1]/10000.0)*10000);
			}
			return val;
		}
	},

	{"LPow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				if(!*args[0] && !*args[1])
					val = 1; //0^0
				else val = int(pow(*args[0],*args[1]));
			}
			return val;
		}
	},

	{"InvPow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				double v1 = *args[0]/10000.0;
				if(!*args[1])
				{
					handler->handleError(CompileError::DivByZero(&node));
					val = 10000;
				}
				else
				{
					double v2 = 10000.0 / *args[1];
					if(!v1 && !v2)
						val = 1; //0^0
					else val = int(pow(v1,v2));
				}
			}
			return val;
		}
	},

	{"Factorial", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				auto v = *args[0] / 10000;
				if(v < 2)
					val = (v >= 0) ? 10000 : 0;
				else
				{
					int prod = 1;
					for(int q = v; v > 1; --v)
						prod *= q;
					val = prod*10000;
				}
			}
			return val;
		}
	},

	{"Abs", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = *args[0];
				if(*val < 0)
					val = -*val;
			}
			return val;
		}
	},

	{"Sqrt", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double v = *args[0]/10000.0;
				if(v < 0)
				{
					handler->handleError(CompileError::NegSqrt(&node));
					val = -10000;
				}
				else
				{
					val = int(sqrt(v)*10000);
				}
			}
			return val;
		}
	},

	{"Floor", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doFloor().getZLong();
			return val;
		}
	},

	{"Truncate", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doTrunc().getZLong();
			return val;
		}
	},

	{"Round", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doRound().getZLong();
			return val;
		}
	},

	{"RoundAway", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doRoundAway().getZLong();
			return val;
		}
	},

	{"Ceiling", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doCeil().getZLong();
			return val;
		}
	},

	{"DegtoRad", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double rangle = (*args[0] / 10000.0) * (PI / 180.0);
				rangle += rangle < 0?-0.00005:0.00005;
				val = int32_t(rangle * 10000.0);
			}
			return val;
		}
	},

	{"RadtoDeg", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double rangle = (*args[0] / 10000.0) * (180.0 / PI);
				val = int32_t(rangle * 10000.0);
			}
			return val;
		}
	},

	{"WrapRadians", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = wrap_zslong_rad(*args[0]);
			}
			return val;
		}
	},

	{"WrapDegrees", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = wrap_zslong_deg(*args[0]);
			}
			return val;
		}
	},

	{"Max", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			for (const auto& arg : args)
			{
				if(!arg) //all args must be constant
					return nullopt;
				if(!val || *arg > *val)
					val = arg;
			}
			return val;
		}
	},

	{"Min", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			for (const auto& arg : args)
			{
				if(!arg) //all args must be constant
					return nullopt;
				if(!val || *arg < *val)
					val = arg;
			}
			return val;
		}
	},
};

static std::map<std::string, CONSTEXPR_CBACK_TY> const_expr_screen_impl = {
	// Empty now :)
};

bool setConstExprForBinding(Function* fn)
{
	std::map<std::string, CONSTEXPR_CBACK_TY> *impl_map = nullptr;

	if (fn->getExternalScope()->getParent()->isClass())
	{
		std::string class_name = fn->getExternalScope()->getParent()->getClass()->getName().value_or("");
		if (class_name == "screendata")
			impl_map = &const_expr_screen_impl;
		else
			return false;
	}
	else
	{
		impl_map = &const_expr_global_impl;
	}

	auto it = impl_map->find(fn->name);
	if (it != impl_map->end())
	{
		fn->set_constexpr(it->second);
		return true;
	}

	return false;
}
