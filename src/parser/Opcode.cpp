#include "Opcode.h"
#include "components/zasm/serialize.h"
#include "components/zasm/table.h"
#include <cstdlib>

using namespace ZScript;

string la_toString(int32_t value)
{
    char temp[128];
    string sign = value < 0 ? "-" : "";
    snprintf(temp, sizeof(temp), "%d", abs(value/10000));
    string first = string(temp);

    if(value % 10000 == 0)
        return sign + first;

    snprintf(temp, sizeof(temp), "%d", abs(value%10000));
    string second = string(temp);
    
    while(second.length() < 4)
        second = "0" + second;
        
    return sign + first + "." + second;
}

string LiteralArgument::toString() const
{
	return la_toString(value);
}

string CompareArgument::toString() const
{
	return CMP_STR(value);
}

int32_t StringToVar(std::string var)
{
	return get_script_variable(var).value();
}

string VarArgument::toString() const
{
	return zasm_var_to_string(ID);
}

string LiteralVarArgument::toString() const
{
	return "@" + zasm_var_to_string(ID);
}

string GlobalArgument::toString() const
{
    char temp[40];
    snprintf(temp, sizeof(temp), "GD%d", ID);
    return string(temp);
}

string ScriptInstanceArgument::toString() const
{
    char temp[40];
    snprintf(temp, sizeof(temp), "SCR%d", ID);
    return string(temp);
}

string LabelArgument::toString() const
{
	char buf[40];
	if(!haslineno)
		snprintf(buf, sizeof(buf), "l%d", ID);
	else if(altstr)
		snprintf(buf, sizeof(buf), "%d.%04d", lineno / 10000, abs(lineno % 10000));
	else
		snprintf(buf, sizeof(buf), "%d", lineno);
	return string(buf);
}

string StringArgument::toString() const
{
	return util::escape_string(value);
}

string TypeArgument::toString() const
{
	return la_toString((int)script_object_type_id);
}

string VectorArgument::toString() const
{
	return util::stringify_vector(value, true);
}
