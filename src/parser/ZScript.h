#ifndef ZSCRIPT_H
#define ZSCRIPT_H

namespace ZScript
{
	struct Variable
	{
		Variable(ZVarType const* type, string const& name, int id) : type(type), name(name), id(id) {}
		ZVarType const* type;
		string name;
		int id;
	};

	struct Function
	{
		Function(ZVarType const* returnType, string const& name, vector<ZVarType const*> paramTypes, int id)
				: returnType(returnType), name(name), paramTypes(paramTypes), id(id) {}
		ZVarType const* returnType;
		string name;
		vector<ZVarType const*> paramTypes;
		int id;
	};
}

#endif
