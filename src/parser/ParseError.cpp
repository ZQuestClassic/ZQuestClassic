//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "ParseError.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
void printErrorMsg(AST *offender, int errorID, string param)
{
    ostringstream oss;
    
    if(offender)
    {
        LocationData d = offender->getLocation();
        oss << d.fname << ", line " << d.first_line << ": ";
    }
    
    switch(errorID)
    {
    case CANTOPENSOURCE:
        oss << "Fatal Error P00: Can't open or parse input file!";
        break;
        
    case CANTOPENIMPORT:
        oss << "Error P01 : Failure to parse imported file " << param.c_str() << ".";
        break;
        
    case IMPORTRECURSION:
        oss << "Fatal Error P02: Recursion limit of " << RECURSIONLIMIT << " hit while preprocessing. Perhaps you have circular imports?";
        break;
        
    case IMPORTBADSCOPE:
        oss << "Error P03: You may only place import statements at file scope.";
        break;
        
    case FUNCTIONREDEF:
        oss << "Error S04: Function " << param << " was already declared with that type signature." ;
        break;
        
    case FUNCTIONVOIDPARAM:
        oss << "Error S05: Function parameter " << param << " cannot have void type." ;
        break;
        
    case SCRIPTREDEF:
        oss << "Error S06: Duplicate script with name " << param << " already exists." ;
        break;
        
    case VOIDVAR:
        oss << "Error S07: Variable " << param << " can't have type void." ;
        break;
        
    case VOIDARR:
        oss << "Error A39: Array " << param << " can't have type void." ;
        break;
        
    case VARREDEF:
        oss << "Error S08: There is already a variable with name " << param << " defined in this scope." ;
        break;
        
    case ARRREDEF:
        oss << "Error A41: There is already an array with name " << param << " defined in this scope." ;
        break;
        
    case VARUNDECLARED:
        oss << "Error S09: Variable " << param << " is undeclared." ;
        break;
        
    case FUNCUNDECLARED:
        oss << "Error S10: Function " << param << " is undeclared." ;
        break;
        
    case SCRIPTNORUN:
        oss << "Error S11: Script " << param << " must implement void run()." ;
        break;
        
    case SCRIPTRUNNOTVOID:
        oss << "Error S12: Script " << param << "'s run() must have return type void." ;
        break;
        
    case SCRIPTNUMNOTINT:
        oss << "Error T13: Script " << param << " has id that's not an integer." ;
        break;
        
    case SCRIPTNUMTOOBIG:
        oss << "Error T14: Script " << param << "'s id must be between 0 and 255." ;
        break;
        
    case SCRIPTNUMREDEF:
        oss << "Error T15: Script " << param << "'s id is already in use." ;
        break;
        
    case IMPLICITCAST:
        oss << "Warning T16: Cast from " << param << "." ;
        break;
        
    case ILLEGALCAST:
        oss << "Error T17: Cannot cast from " << param << "." ;
        break;
        
    case VOIDEXPR:
        oss << "Error T18: Operand is void." ;
        break;
        
    case DIVBYZERO:
        oss << "Error T19: Constant division by zero." ;
        break;
        
    case CONSTTRUNC:
        oss << "Warning T20: Truncation of constant " << param << "." ;
        break;
        
    case NOFUNCMATCH:
        oss << "Error T21: Could not match type signature " << param << "." ;
        break;
        
    case TOOFUNCMATCH:
        oss << "Error T22: Two or more functions match type signature " << param << "." ;
        break;
        
    case FUNCBADRETURN:
        oss << "Error T23: This function must return a value." ;
        break;
        
    case TOOMANYGLOBAL:
        oss << "Error L24: Too many global variables." ;
        break;
        
    case SHIFTNOTINT:
        oss << "Warning T25: Constant bitshift by noninteger amount; truncating to nearest integer." ;
        break;
        
    case REFVAR:
        oss << "Error S26: Pointer types (ffc, etc) cannot be declared as global variables." ;
        break;
        
    case ARROWNOTPOINTER:
        oss << "Error T27: Left of the arrow (->) operator must be a pointer type (ffc, etc)." ;
        break;
        
    case ARROWNOFUNC:
        oss << "Error T28: That pointer type does not have a function " << param << "." ;
        break;
        
    case ARROWNOVAR:
        oss << "Error T29: That pointer type does not have a variable " << param << "." ;
        break;
        
    case TOOMANYRUN:
        oss << "Error S30: Script " << param << " may have only one run method." ;
        break;
        
    case INDEXNOTINT:
        oss << "Error T31: The index of " << param << " must be an integer." ;
        break;
        
    case SCRIPTBADTYPE:
        oss << "Error S32: Script " << param << " is of illegal type.";
        break;
        
    case BREAKBAD:
        oss << "Error G33: Break must lie inside of an enclosing for or while loop.";
        break;
        
    case CONTINUEBAD:
        oss << "Error G34: Continue must lie inside of an enclosing for or while loop.";
        break;
        
    case CONSTREDEF:
        oss << "Error P35: There is already a constant with name " << param << " defined.";
        break;
        
    case LVALCONST:
        oss << "Error T36: Cannot change the value of constant variable " << param << ".";
        break;
        
    case BADGLOBALINIT:
        oss << "Error T37: Global variables can only be initialized to constants or globals declared in the same script.";
        break;
        
    case DEPRECATEDGLOBAL:
        oss << "Warning S38: Script-scope global variable declaration syntax is deprecated; put declarations at file scope instead.";
        break;
        
    case REFARR:
        oss << "Error A40: Pointer types (ffc, etc) cannot be declared as global arrays." ;
        break;
    
    case WARNHEADER:
        oss << "Warning: The #included file was skipped." ;
        break;
        
    case ARRAYTOOSMALL:
        oss << "Error A42: Array is too small." ;
        break;
        
    case ARRAYLISTTOOLARGE:
        oss << "Error A43: Array initializer larger than specified dimensions." ;
        break;
        
    case ARRAYLISTSTRINGTOOLARGE:
        oss << "Error A44: String array initializer larger than specified dimensions, space must be allocated for NULL terminator." ;
        break;
        
    case NONINTEGERARRAYSIZE:
        oss << "Error A38: Arrays can only be initialized to numerical values" ;
        
    default:
        oss << "FATAL FATAL ERROR I0: bad internal error code" ;
        assert(false);
        break;
    }
    
#ifndef SCRIPTPARSER_COMPILE
    box_out(oss.str().c_str());
    box_eol();
#endif
}

