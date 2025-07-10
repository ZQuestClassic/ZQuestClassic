#include "Opcode.h"
#include "zasm/serialize.h"
#include "zasm/table.h"
#include <cstdlib>

using namespace ZScript;

string la_toString(int32_t value)
{
    char temp[128];
    string sign = value < 0 ? "-" : "";
    sprintf(temp,"%d", abs(value/10000));
    string first = string(temp);
    
    if(value % 10000 == 0)
        return sign + first;
        
    sprintf(temp,"%d", abs(value%10000));
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
    sprintf(temp, "GD%d", ID);
    return string(temp);
}

string LabelArgument::toString() const
{
	char buf[40];
	if(!haslineno)
		sprintf(buf, "l%d", ID);
	else if(altstr)
		sprintf(buf, "%d.%04d", lineno / 10000, abs(lineno % 10000));
	else
		sprintf(buf, "%d", lineno);
	return string(buf);
}

string StringArgument::toString() const
{
	return util::escape_string(value);
}

string VectorArgument::toString() const
{
	return util::stringify_vector(value, true);
}
