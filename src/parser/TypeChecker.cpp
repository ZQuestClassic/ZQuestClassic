//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "TypeChecker.h"
#include "ParseError.h"
#include "GlobalSymbols.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <string>

////////////////////////////////////////////////////////////////
// TypeCheck
    
// Statements

void TypeCheck::caseStmtAssign(ASTStmtAssign& host, void* param)
{
    host.getRVal()->execute(*this, param);
    if (failure) return;

	TypeCheckParam& p = *(TypeCheckParam*)param;
 	GetLValTypeParam lparam(*this, p);
    GetLValType temp;
    host.getLVal()->execute(temp, &lparam);
    if (failure) return;
	ZVarTypeId ltype = lparam.type;

    ZVarTypeId rtype = host.getRVal()->getType();

    if (!standardCheck(ltype, rtype, &host))
        failure = true;
}

void TypeCheck::caseStmtIf(ASTStmtIf &host, void *param)
{
    RecursiveVisitor::caseStmtIf(host, param);
    if (failure) return;

    ZVarTypeId type = host.getCondition()->getType();

    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
    caseStmtIf(host, param);
    host.getElseStmt()->execute(*this, param);
}
        
void TypeCheck::caseStmtSwitch(ASTStmtSwitch &host, void* param)
{
	RecursiveVisitor::caseStmtSwitch(host, param);
	if (failure) return;

	ZVarTypeId type = host.getKey()->getType();
	if (!standardCheck(ZVARTYPEID_FLOAT, type, &host))
		failure = true;
}

void TypeCheck::caseStmtFor(ASTStmtFor &host, void *param)
{
    RecursiveVisitor::caseStmtFor(host, param);
    if (failure) return;

    ZVarTypeId type = host.getTerminationCondition()->getType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
        
void TypeCheck::caseStmtWhile(ASTStmtWhile &host, void *param)
{
    RecursiveVisitor::caseStmtWhile(host, param);
    if (failure) return;

    ZVarTypeId type = host.getCond()->getType();
    if (!standardCheck(ZVARTYPEID_BOOL, type, &host))
        failure = true;
}
    
void TypeCheck::caseStmtReturn(ASTStmtReturn &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

    if (p.type != ZVARTYPEID_VOID)
    {
        printErrorMsg(&host, FUNCBADRETURN, ScriptParser::printType(p.type));
        failure = true;
    }
}

void TypeCheck::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
    host.getReturnValue()->execute(*this, param);
    if (failure) return;

	TypeCheckParam& p = *(TypeCheckParam*)param;
    if (!standardCheck(p.type, host.getReturnValue()->getType(), &host))
        failure = true;
}

// Declarations

void TypeCheck::caseArrayDecl(ASTArrayDecl &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

	ASTExpr *size = host.getSize();
	size->execute(*this, param);
    
	if (size->getType() != ZVARTYPEID_FLOAT)
	{
		printErrorMsg(&host, NONINTEGERARRAYSIZE, "");
		failure = true;
		return;
	}

    if (host.getList() != NULL)
    {
		ZVarTypeId arraytype = p.symbols.getVarTypeId(&host);
        list<ASTExpr *> l = host.getList()->getList();

        for(list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
        {
            (*it)->execute(*this, param);
            if (failure) return;

            if (!standardCheck(arraytype, (*it)->getType(), &host))
            {
                failure = true;
                return;
            }
        }
    }
}

void TypeCheck::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

    ASTExpr* init = host.getInitializer();
    init->execute(*this, param);
    if (failure) return;

    ZVarTypeId type = init->getType();
    ZVarTypeId ltype = p.symbols.getVarTypeId(&host);

    if (!standardCheck(ltype, type, &host))
        failure = true;
}

// Expressions

void TypeCheck::caseExprConst(ASTExprConst &host, void *param)
{
	ASTExpr* content = host.getContent();
	content->execute(*this, param);

	if (!host.isConstant())
	{
		failure = true;
        printErrorMsg(&host, EXPRNOTCONSTANT);
		return;
	}

	host.setType(content->getType());
	if (content->hasIntValue())
		host.setIntValue(content->getIntValue());
}

void TypeCheck::caseNumConstant(ASTNumConstant &host, void *)
{
    host.setType(ZVARTYPEID_FLOAT);
    pair<string,string> parts = host.getValue()->parseValue();
    pair<long, bool> val = ScriptParser::parseLong(parts);

    if (!val.second)
        printErrorMsg(&host, CONSTTRUNC, host.getValue()->getValue());

    host.setIntValue(val.first);
}
        
void TypeCheck::caseBoolConstant(ASTBoolConstant &host, void *)
{
    host.setType(ZVARTYPEID_BOOL);
    host.setIntValue(host.getValue() ? 1L : 0L);
}
            
void TypeCheck::caseStringConstant(ASTStringConstant& host, void*)
{
	host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprDot(ASTExprDot &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

    if (p.symbols.isConstant(host.getName()))
	{
        host.setType(ZVARTYPEID_FLOAT);
		host.setIntValue(p.symbols.getConstantVal(host.getName()));
	}
    else
    {
        host.setType(p.symbols.getVarTypeId(&host));
    }
}
    
void TypeCheck::caseExprArrow(ASTExprArrow &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
    SymbolTable& st = p.symbols;

    // Annoyingly enough I have to treat arrowed variables as function calls
    // Get the left-hand type.
    host.getLVal()->execute(*this, param);
    if (failure) return;

    bool isIndexed = (host.getIndex() != NULL);
    if (isIndexed)
    {
        host.getIndex()->execute(*this, param);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            failure = true;
            return;
        }
    }

    ZVarTypeId type = host.getLVal()->getType();

    string name = "get" + host.getName();
    if (isIndexed) name += "[]";

	int functionId;
    switch (type)
    {
    case ZVARTYPEID_FFC:
    {
        functionId = FFCSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_LINK:
    {
        functionId = LinkSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_SCREEN:
    {
        functionId = ScreenSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_GAME:
    {
        functionId = GameSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ITEM:
    {
        functionId = ItemSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ITEMCLASS:
    {
        functionId = ItemclassSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_NPC:
    {
        functionId = NPCSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_LWPN:
    {
        functionId = LinkWeaponSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_EWPN:
    {
        functionId = EnemyWeaponSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_NPCDATA:
    {
        functionId = NPCDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_DEBUG:
    {
        functionId = DebugSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_AUDIO:
    {
        functionId = AudioSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_COMBOS:
    {
        functionId = CombosPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_SPRITEDATA:
    {
        functionId = SpriteDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_GRAPHICS:
    {
        functionId = GfxPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_TEXT:
    {
        functionId = TextPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_INPUT:
    {
        functionId = InputSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_MAPDATA:
    {
        functionId = MapDataSymbols::getInst().matchFunction(name);
        break;
    }
    
    case ZVARTYPEID_DMAPDATA:
    {
        functionId = DMapDataSymbols::getInst().matchFunction(name);
        break;
    }
    
    case ZVARTYPEID_SHOPDATA:
    {
        functionId = ShopDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ZMESSAGE:
    {
        functionId = MessageDataSymbols::getInst().matchFunction(name);
        break;
    }
    default:
	    failure = true;
	    printErrorMsg(&host, ARROWNOTPOINTER);
	    return;
    }

	vector<ZVarTypeId> functionParams = p.symbols.getFuncParamTypeIds(functionId);
    if (functionId == -1 || functionParams.size() != (isIndexed ? 2 : 1) || functionParams[0] != type)
    {
        failure = true;
        printErrorMsg(&host, ARROWNOVAR, host.getName() + (isIndexed ? "[]" : ""));
        return;
    }

    p.symbols.putNodeId(&host, functionId);
    host.setType(p.symbols.getFuncReturnTypeId(functionId));
}

void TypeCheck::caseExprArray(ASTExprArray &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

    ZVarTypeId type  = p.symbols.getVarTypeId(&host);
    host.setType(type);

    if (host.getIndex())
    {
        host.getIndex()->execute(*this, param);
        if (failure) return;

        if (!standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            failure = true;
            return;
        }
    }
}
    
void TypeCheck::caseFuncCall(ASTFuncCall &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

    // Yuck. Time to disambiguate these damn functions

    // Build the param types
    list<ASTExpr*> params = host.getParams();
    vector<ZVarTypeId> paramtypes;
    vector<int> possibleFuncIds;

    bool isdotexpr;
    IsDotExpr temp;
    host.getName()->execute(temp, &isdotexpr);

    // If this is a simple function, we already have what we need otherwise we
    // need the type of the thing being arrowed
    if (!isdotexpr)
    {
        ASTExprArrow* lval = (ASTExprArrow *)host.getName();
        lval->getLVal()->execute(*this, param);
        if (failure) return;
        ZVarTypeId lvaltype = lval->getLVal()->getType();

        if (!(lvaltype == ZVARTYPEID_FFC
              || lvaltype == ZVARTYPEID_LINK
			  || lvaltype == ZVARTYPEID_SCREEN
              || lvaltype == ZVARTYPEID_ITEM
			  || lvaltype == ZVARTYPEID_ITEMCLASS
              || lvaltype == ZVARTYPEID_GAME
              || lvaltype == ZVARTYPEID_NPC
              || lvaltype == ZVARTYPEID_LWPN
              || lvaltype == ZVARTYPEID_EWPN
              || lvaltype == ZVARTYPEID_NPCDATA
              || lvaltype == ZVARTYPEID_DEBUG
              || lvaltype == ZVARTYPEID_AUDIO
              || lvaltype == ZVARTYPEID_COMBOS
              || lvaltype == ZVARTYPEID_SPRITEDATA
              || lvaltype == ZVARTYPEID_GRAPHICS
              || lvaltype == ZVARTYPEID_TEXT
              || lvaltype == ZVARTYPEID_INPUT
              || lvaltype == ZVARTYPEID_MAPDATA
              || lvaltype == ZVARTYPEID_DMAPDATA
              || lvaltype == ZVARTYPEID_ZMESSAGE
              || lvaltype == ZVARTYPEID_SHOPDATA 
              || lvaltype == ZVARTYPEID_UNTYPED
              || lvaltype == ZVARTYPEID_DROPSET
              || lvaltype == ZVARTYPEID_PONDS
              || lvaltype == ZVARTYPEID_WARPRING
              || lvaltype == ZVARTYPEID_DOORSET
              || lvaltype == ZVARTYPEID_ZUICOLOURS
              || lvaltype == ZVARTYPEID_RGBDATA
              || lvaltype == ZVARTYPEID_PALETTE
              || lvaltype == ZVARTYPEID_TUNES
              || lvaltype == ZVARTYPEID_PALCYCLE
              || lvaltype == ZVARTYPEID_GAMEDATA
              || lvaltype == ZVARTYPEID_CHEATS
              ))
        {
	        printErrorMsg(lval, ARROWNOTPOINTER);
	        failure = true;
	        return;
        }

        // Prepend that type to the function parameters, as that is implicitly passed
        paramtypes.push_back(lvaltype);
    }

    // Now add the normal parameters.
    for (list<ASTExpr*>::iterator it = params.begin(); it != params.end(); it++)
    {
        (*it)->execute(*this, param);
        if (failure) return;

        paramtypes.push_back((*it)->getType());
    }

    string paramstring = "(";
    bool firsttype = true;
    for(vector<ZVarTypeId>::iterator it = paramtypes.begin(); it != paramtypes.end(); it++)
    {
        if (firsttype)
            firsttype = false;
        else
            paramstring += ", ";

        paramstring += ScriptParser::printType(*it);
    }
    paramstring += ")";

    if (isdotexpr)
    {
        possibleFuncIds = p.symbols.getPossibleNodeFuncIds(&host);

        vector<pair<int, int> > matchedfuncs;

        for (vector<int>::iterator it = possibleFuncIds.begin(); it != possibleFuncIds.end(); it++)
        {
            vector<ZVarTypeId> itParams = p.symbols.getFuncParamTypeIds(*it);

            // See if they match.
            if (itParams.size() != paramtypes.size())
                continue;

            bool matched = true;
            int diffs = 0;

            for (unsigned int i = 0; i < itParams.size(); i++)
            {
                if (!standardCheck(itParams[i], paramtypes[i], NULL))
                {
                    matched = false;
                    break;
                }

                if (itParams[i] != paramtypes[i])
                    diffs++;
            }

            if (matched)
            {
                matchedfuncs.push_back(pair<int,int>(*it, diffs));
            }
        }

        // Now find the closest match *sigh*
        vector<int> bestmatch;
        int bestdiffs = 10000;

        for (vector<pair<int, int> >::iterator it = matchedfuncs.begin(); it != matchedfuncs.end(); it++)
        {
            if ((*it).second < bestdiffs)
            {
                bestdiffs = (*it).second;
                bestmatch.clear();
                bestmatch.push_back((*it).first);
            }
            else if ((*it).second == bestdiffs)
            {
                bestmatch.push_back((*it).first);
            }
        }

        string fullname;

        if (((ASTExprDot *)host.getName())->getNamespace() == "")
            fullname = ((ASTExprDot*)host.getName())->getName();
        else
            fullname = ((ASTExprDot *)host.getName())->getNamespace() + "." + ((ASTExprDot *)host.getName())->getName();

        if (bestmatch.size() == 0)
        {
            printErrorMsg(&host, NOFUNCMATCH, fullname + paramstring);
            failure = true;
            return;
        }
        else if (bestmatch.size() > 1)
        {
            printErrorMsg(&host, TOOFUNCMATCH, fullname+paramstring);
            failure = true;
            return;
        }

        // WHEW!
        host.setType(p.symbols.getFuncReturnTypeId(bestmatch[0]));
        p.symbols.putNodeId(&host, bestmatch[0]);
    }
    else
    {
        // Still have to deal with the (%&# arrow functions
        // Luckily I will here assert that each type's functions MUST be unique
		ASTExprArrow& arrow = *(ASTExprArrow*)host.getName();
        string name = arrow.getName();
        ZVarTypeId type = arrow.getLVal()->getType();

		int functionId;
		switch(type)
		{
		case ZVARTYPEID_FFC:
		{
			functionId = FFCSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_LINK:
		{
			functionId = LinkSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_SCREEN:
		{
			functionId = ScreenSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_GAME:
		{
			functionId = GameSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_ITEM:
		{
			functionId = ItemSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_ITEMCLASS:
		{
			functionId = ItemclassSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_NPC:
		{
			functionId = NPCSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_LWPN:
		{
			functionId = LinkWeaponSymbols::getInst().matchFunction(name);
			break;
		}
		case ZVARTYPEID_EWPN:
		{
			functionId = EnemyWeaponSymbols::getInst().matchFunction(name);
			break;
		}
        case ZVARTYPEID_NPCDATA:
        {
            functionId = NPCDataSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_MAPDATA:
        {
            functionId = MapDataSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_DEBUG:
        {
            functionId = DebugSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_AUDIO:
        {
            functionId = AudioSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_COMBOS:
        {
            functionId = CombosPtrSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_SPRITEDATA:
        {
            functionId = SpriteDataSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_GRAPHICS:
        {
            functionId = GfxPtrSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_TEXT:
        {
            functionId = TextPtrSymbols::getInst().matchFunction(name);
            break;
        }
        case ZVARTYPEID_INPUT:
        {
            functionId = InputSymbols::getInst().matchFunction(name);
            break;
        }   
        case ZVARTYPEID_DMAPDATA:
        {
	        functionId = DMapDataSymbols::getInst().matchFunction(name);
	        break;
        }
        case ZVARTYPEID_ZMESSAGE:
        {
	        functionId = MessageDataSymbols::getInst().matchFunction(name);
	        break;
        }
        case ZVARTYPEID_SHOPDATA:
        {
	        functionId = ShopDataSymbols::getInst().matchFunction(name);
	        break;
        }
        case ZVARTYPEID_UNTYPED:
        {
	        functionId = UntypedSymbols::getInst().matchFunction(name);
	        break;
        }
		case ZVARTYPEID_DROPSET:
		{
	        functionId = DropsetSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_PONDS:
		{
	        functionId = PondSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_WARPRING:
		{
	        functionId = WarpringSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_DOORSET:
		{
	        functionId = DoorsetSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_ZUICOLOURS:
		{
	        functionId = MiscColourSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_RGBDATA:
		{
	        functionId = RGBSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_PALETTE:
		{
	        functionId = PaletteSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_TUNES:
		{
	        functionId = TunesSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_PALCYCLE:
		{
	        functionId = PalCycleSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_GAMEDATA:
		{
	        functionId = GamedataSymbols::getInst().matchFunction(name);
	        break;
		}
		case ZVARTYPEID_CHEATS:
		{
	        functionId = CheatsSymbols::getInst().matchFunction(name);
	        break;
		}
		default:
			assert(false);
		}
		
		vector<ZVarTypeId> functionParams = p.symbols.getFuncParamTypeIds(functionId);

        if (functionId == -1)
        {
            failure = true;
            printErrorMsg(&host, ARROWNOFUNC, name);
            return;
        }

        if (paramtypes.size() != functionParams.size())
        {
            failure = true;
            printErrorMsg(&host, NOFUNCMATCH, name + paramstring);
            return;
        }

        for (unsigned int i = 0; i < paramtypes.size(); i++)
        {
            if (!standardCheck(functionParams[i], paramtypes[i], NULL))
            {
                failure = true;
                printErrorMsg (&host, NOFUNCMATCH, name + paramstring);
                return;
            }
        }

        host.setType(p.symbols.getFuncReturnTypeId(functionId));
        p.symbols.putNodeId(&host, functionId);
    }
}

void TypeCheck::caseExprNegate(ASTExprNegate &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
        host.setIntValue(-val);
    }
}

void TypeCheck::caseExprNot(ASTExprNot &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
		host.setIntValue(!host.getOperand()->getIntValue());
    }
}

void TypeCheck::caseExprBitNot(ASTExprBitNot &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getOperand()->hasIntValue())
    {
        long val = host.getOperand()->getIntValue();
        host.setIntValue((~(val/10000))*10000);
    }
}

void TypeCheck::caseExprIncrement(ASTExprIncrement &host, void *param)
{
    host.getOperand()->execute(*this, param);
    if (failure) return;

	TypeCheckParam& p = *(TypeCheckParam*)param;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = p.symbols.getNodeId(host.getOperand());
        p.symbols.putNodeId(&host, fid);
    }

    GetLValType glvt;
 	GetLValTypeParam lparam(*this, p);
    host.getOperand()->execute(glvt, &lparam);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, lparam.type, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}
    
void TypeCheck::caseExprPreIncrement(ASTExprPreIncrement &host, void *param)
{
    host.getOperand()->execute(*this, param);
    if (failure) return;

	TypeCheckParam & p = *(TypeCheckParam*)param;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = p.symbols.getNodeId(host.getOperand());
        p.symbols.putNodeId(&host, fid);
    }

    if (!isexprdot)
    {
        int fid = p.symbols.getNodeId(host.getOperand());
        p.symbols.putNodeId(&host, fid);
    }

    GetLValType glvt;
 	GetLValTypeParam lparam(*this, p);
    host.getOperand()->execute(glvt, &lparam);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, lparam.type, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprDecrement(ASTExprDecrement &host, void *param)
{
    host.getOperand()->execute(*this, param);
    if (failure) return;

	TypeCheckParam& p = *(TypeCheckParam*)param;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = p.symbols.getNodeId(host.getOperand());
        p.symbols.putNodeId(&host, fid);
    }

    GetLValType glvt;
 	GetLValTypeParam lparam(*this, p);
    host.getOperand()->execute(glvt, &lparam);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, lparam.type, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}
    
void TypeCheck::caseExprPreDecrement(ASTExprPreDecrement &host, void *param)
{
    host.getOperand()->execute(*this, param);
    if (failure) return;

	TypeCheckParam& p = *(TypeCheckParam*)param;

    bool isexprdot;
    IsDotExpr temp;
    host.getOperand()->execute(temp, &isexprdot);

    if (!isexprdot)
    {
        int fid = p.symbols.getNodeId(host.getOperand());
        p.symbols.putNodeId(&host, fid);
    }

    GetLValType glvt;
 	GetLValTypeParam lparam(*this, p);
    host.getOperand()->execute(glvt, &lparam);
    if (failure) return;

    if (!standardCheck(ZVARTYPEID_FLOAT, lparam.type, &host))
    {
        failure = true;
        return;
    }

    host.setType(ZVARTYPEID_FLOAT);
}

void TypeCheck::caseExprAnd(ASTExprAnd &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval && secondval);
    }
}

void TypeCheck::caseExprOr(ASTExprOr &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_BOOL, ZVARTYPEID_BOOL)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval || secondval);
    }
}

void TypeCheck::caseExprGT(ASTExprGT &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval > secondval);
    }
}

void TypeCheck::caseExprGE(ASTExprGE &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval >= secondval);
    }
}

void TypeCheck::caseExprLT(ASTExprLT &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval < secondval);
    }
}

void TypeCheck::caseExprLE(ASTExprLE &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval <= secondval);
    }
}

void TypeCheck::caseExprEQ(ASTExprEQ &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

	host.getFirstOperand()->execute(*this, param);
	if (failure) return;
	host.getSecondOperand()->execute(*this, param);
	if (failure) return;

	if (!standardCheck(host.getFirstOperand()->getType(), host.getSecondOperand()->getType(), &host))
	{
		failure = true;
		return;
	}

    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval == secondval);
    }
}

void TypeCheck::caseExprNE(ASTExprNE &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;

	host.getFirstOperand()->execute(*this, param);
	if (failure) return;
	host.getSecondOperand()->execute(*this, param);
	if (failure) return;

	if (!standardCheck(host.getFirstOperand()->getType(), host.getSecondOperand()->getType(), &host))
	{
		failure = true;
		return;
	}

    host.setType(ZVARTYPEID_BOOL);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
		host.setIntValue(firstval != secondval);
    }
}

void TypeCheck::caseExprPlus(ASTExprPlus &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(firstval + secondval);
    }
}

void TypeCheck::caseExprMinus(ASTExprMinus &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(firstval - secondval);
    }
    }
    
void TypeCheck::caseExprTimes(ASTExprTimes &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        double temp = ((double)secondval)/10000.0;
        host.setIntValue((long)(firstval * temp));
    }
}

void TypeCheck::caseExprDivide(ASTExprDivide &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setIntValue((firstval / secondval) * 10000);
    }
}

void TypeCheck::caseExprModulo(ASTExprModulo &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();

        if (secondval == 0)
        {
            printErrorMsg(&host, DIVBYZERO);
            failure = true;
            return;
        }

        host.setIntValue(firstval % secondval);
    }
}

void TypeCheck::caseExprBitAnd(ASTExprBitAnd &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)&(secondval/10000))*10000);
    }
}
    
void TypeCheck::caseExprBitOr(ASTExprBitOr &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)|(secondval/10000))*10000);
    }
}

void TypeCheck::caseExprBitXor(ASTExprBitXor &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        long secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)^(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprLShift(ASTExprLShift &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasIntValue())
    {
        if (host.getSecondOperand()->getIntValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setIntValue(10000*(host.getSecondOperand()->getIntValue()/10000));
        }
    }
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        int secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)<<(secondval/10000))*10000);
    }
}
void TypeCheck::caseExprRShift(ASTExprRShift &host, void *param)
{
	TypeCheckParam& p = *(TypeCheckParam*)param;
	if (!checkExprTypes(p, host, ZVARTYPEID_FLOAT, ZVARTYPEID_FLOAT)) return;

    if (host.getSecondOperand()->hasIntValue())
    {
        if (host.getSecondOperand()->getIntValue() % 10000)
        {
            printErrorMsg(&host, SHIFTNOTINT);
            host.getSecondOperand()->setIntValue(10000*(host.getSecondOperand()->getIntValue()/10000));
        }
    }
    host.setType(ZVARTYPEID_FLOAT);

    if (host.getFirstOperand()->hasIntValue() && host.getSecondOperand()->hasIntValue())
    {
        long firstval = host.getFirstOperand()->getIntValue();
        int secondval = host.getSecondOperand()->getIntValue();
        host.setIntValue(((firstval/10000)>>(secondval/10000))*10000);
            }
        }
        
// Other

bool TypeCheck::standardCheck(ZVarTypeId targetType, ZVarTypeId sourceType, AST* toBlame)
{
	if (targetType == sourceType && targetType != ZVARTYPEID_VOID) return true;
	if (targetType == ZVARTYPEID_BOOL && sourceType == ZVARTYPEID_FLOAT) return true;
	if (sourceType == ZVARTYPEID_UNTYPED || targetType == ZVARTYPEID_UNTYPED) return true;

	if (toBlame)
	{
		string msg = ScriptParser::printType(sourceType) + " to " + ScriptParser::printType(targetType);
		printErrorMsg(toBlame, ILLEGALCAST, msg);
	}
	return false;
}

bool TypeCheck::checkExprTypes(TypeCheckParam& param, ASTUnaryExpr& expr, ZVarTypeId type)
{
	expr.getOperand()->execute(*this, &param);
	if (failure) return false;
	failure = !standardCheck(type, expr.getOperand()->getType(), &expr);
	return !failure;
}

bool TypeCheck::checkExprTypes(TypeCheckParam& param, ASTBinaryExpr& expr, ZVarTypeId firstType, ZVarTypeId secondType)
{
	expr.getFirstOperand()->execute(*this, &param);
	if (failure) return false;
	failure = !standardCheck(firstType, expr.getFirstOperand()->getType(), &expr);
	if (failure) return false;

	expr.getSecondOperand()->execute(*this, &param);
	if (failure) return false;
	failure = !standardCheck(secondType, expr.getSecondOperand()->getType(), &expr);
	return !failure;
}

////////////////////////////////////////////////////////////////
// GetLValType

void GetLValType::caseDefault(void *)
{
    assert(false);
}

void GetLValType::caseVarDecl(ASTVarDecl& host, void* param)
{
	GetLValTypeParam& p = *(GetLValTypeParam*)param;
    host.execute(p.tc, &p.tcParam);
    p.type = p.tcParam.symbols.getVarTypeId(&host);
}

void GetLValType::caseExprArrow(ASTExprArrow &host, void *param)
{
	GetLValTypeParam& p = *(GetLValTypeParam*)param;
    host.getLVal()->execute(p.tc, &p.tcParam);
    if (!p.tc.isOK()) return;

    ZVarTypeId type = host.getLVal()->getType();
    bool isIndexed = (host.getIndex() != 0);

    if (isIndexed)
    {
        host.getIndex()->execute(p.tc, &p.tcParam);
        if (!p.tc.isOK()) return;

        if (!TypeCheck::standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            p.tc.fail();
            return;
        }
    }

    string name = "set" + host.getName();
    if (isIndexed) name += "[]";

	int functionId;
    switch (type)
    {
    case ZVARTYPEID_FFC:
    {
        functionId = FFCSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_LINK:
    {
        functionId = LinkSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_SCREEN:
    {
        functionId = ScreenSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_GAME:
    {
        functionId = GameSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ITEM:
    {
        functionId = ItemSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ITEMCLASS:
    {
        functionId = ItemclassSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_NPC:
    {
        functionId = NPCSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_LWPN:
    {
        functionId = LinkWeaponSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_EWPN:
    {
        functionId = EnemyWeaponSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_NPCDATA:
    {
        functionId = NPCDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_MAPDATA:
    {
        functionId = MapDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_DEBUG:
    {
        functionId = DebugSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_AUDIO:
    {
        functionId = AudioSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_COMBOS:
    {
        functionId = CombosPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_SPRITEDATA:
    {
        functionId = SpriteDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_GRAPHICS:
    {
        functionId = GfxPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_TEXT:
    {
        functionId = TextPtrSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_INPUT:
    {
        functionId = InputSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_DMAPDATA:
    {
        functionId = DMapDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ZMESSAGE:
    {
        functionId = MessageDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_SHOPDATA:
    {
        functionId = ShopDataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_UNTYPED:
    {
        functionId = UntypedSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_DROPSET:
    {
        functionId = DropsetSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_PONDS:
    {
        functionId = PondSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_WARPRING:
    {
        functionId = WarpringSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_DOORSET:
    {
        functionId = DoorsetSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_ZUICOLOURS:
    {
        functionId = MiscColourSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_RGBDATA:
    {
        functionId = RGBSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_PALETTE:
    {
        functionId = PaletteSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_TUNES:
    {
        functionId = TunesSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_PALCYCLE:
    {
        functionId = PalCycleSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_GAMEDATA:
    {
        functionId = GamedataSymbols::getInst().matchFunction(name);
        break;
    }
    case ZVARTYPEID_CHEATS:
    {
        functionId = CheatsSymbols::getInst().matchFunction(name);
        break;
    }
    
    default:
    {
        p.tc.fail();
        printErrorMsg(&host, ARROWNOTPOINTER);
        return;
    }
    }

	vector<ZVarTypeId> functionParams = p.tcParam.symbols.getFuncParamTypeIds(functionId);
    if (functionId == -1 || functionParams.size() != (isIndexed ? 3 : 2) || functionParams[0] != type)
    {
        printErrorMsg(&host, ARROWNOVAR, host.getName() + (isIndexed ? "[]" : ""));
        p.tc.fail();
        return;
    }

    p.tcParam.symbols.putNodeId(&host, functionId);
    p.type = functionParams[isIndexed ? 2 : 1];
}

void GetLValType::caseExprDot(ASTExprDot &host, void *param)
{
	GetLValTypeParam& p = *(GetLValTypeParam*)param;
    host.execute(p.tc, &p.tcParam);
    int vid = p.tcParam.symbols.getNodeId(&host);

    if (vid == -1)
    {
        printErrorMsg(&host, LVALCONST, host.getName());
        p.tc.fail();
        return;
    }

    p.type = p.tcParam.symbols.getVarTypeId(&host);
}

void GetLValType::caseExprArray(ASTExprArray &host, void *param)
{
	GetLValTypeParam& p = *(GetLValTypeParam*)param;
    host.execute(p.tc, &p.tcParam);
    int vid = p.tcParam.symbols.getNodeId(&host);

    if (vid == -1)
    {
        printErrorMsg(&host, LVALCONST, host.getName());
        p.tc.fail();
        return;
    }

    p.type = p.tcParam.symbols.getVarTypeId(&host);

    if (host.getIndex())
    {
        host.getIndex()->execute(p.tc, &p.tcParam);

        if (!p.tc.isOK()) return;

        if (!TypeCheck::standardCheck(ZVARTYPEID_FLOAT, host.getIndex()->getType(), host.getIndex()))
        {
            p.tc.fail();
            return;
        }
    }
}

        
    

