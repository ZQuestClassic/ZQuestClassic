#include "ZScript.h"

#include "ParseError.h"

using namespace std;
using namespace ZScript;

// ZScript::Program

Program::Program(ASTProgram* program) : node(program), globalScope(table)
{
	assert(node);

	// Create a ZScript::Script for every script in the program.
	for (vector<ASTScript*>::const_iterator it = node->scripts.begin();
		 it != node->scripts.end(); ++it)
	{
		scripts.push_back(new Script(*this, *it));
		Script* script = scripts.back();
		scriptsByName[script->getName()] = script;
		scriptsByNode[*it] = script;
	}
}

Program::~Program()
{
	for (vector<Script*>::iterator it = scripts.begin(); it != scripts.end(); ++it)
		delete *it;
}

Script* Program::getScript(string const& name) const
{
	map<string, Script*>::const_iterator it = scriptsByName.find(name);
	if (it == scriptsByName.end()) return NULL;
	return it->second;
}

Script* Program::getScript(ASTScript* node) const
{
	map<ASTScript*, Script*>::const_iterator it = scriptsByNode.find(node);
	if (it == scriptsByNode.end()) return NULL;
	return it->second;
}

bool Program::hasError() const
{
	for (vector<Script*>::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
		if ((*it)->hasError()) return true;
	return false;
}

void Program::printErrors() const
{
	for (vector<Script*>::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
		(*it)->printErrors();
}

// ZScript::Script

Script::Script(Program& program, ASTScript* script) : node(script)
{
	assert(node);

	// Create script scope.
	scope = program.globalScope.makeScriptChild(getName());
	scope->varDeclsDeprecated = true;
}

string Script::getName() const {return node->getName();}

ScriptType Script::getType() const {return node->getType()->getType();}

Function* Script::getRun() const
{
	vector<Function*> possibleRuns = scope->getLocalFunctions("run");
	if (possibleRuns.size() != 1) return NULL;
	return possibleRuns[0];
}

bool Script::hasError() const
{
	// Has a scope.
	if (!scope) return true;

	// Has a proper script type.
	ScriptType scriptType = getType();
	if (scriptType != SCRIPTTYPE_GLOBAL
		&& scriptType != SCRIPTTYPE_ITEM
		&& scriptType != SCRIPTTYPE_FFC)
	{
		return true;
	}

	// Invalid run function.
	vector<Function*> possibleRuns = scope->getLocalFunctions("run");
	if (possibleRuns.size() != 1) return true;
	Function& run = *possibleRuns[0];
	if (*run.returnType != ZVarType::ZVOID) return true;

	return false;
}

void Script::printErrors() const
{
	string const& name = getName();

	// Failed to create scope means an existing script already has this name.
	if (!scope) printErrorMsg(node, SCRIPTREDEF, name);

	// Invalid script type.
	ScriptType scriptType = getType();
	if (scriptType != SCRIPTTYPE_GLOBAL
		&& scriptType != SCRIPTTYPE_ITEM
		&& scriptType != SCRIPTTYPE_FFC)
	{
		printErrorMsg(node, SCRIPTBADTYPE, name);
	}

	// Invalid run function.
	vector<Function*> possibleRuns = scope->getLocalFunctions("run");
	if (possibleRuns.size() > 1)
		printErrorMsg(node, TOOMANYRUN, name);
	else if (possibleRuns.size() == 0)
		printErrorMsg(node, SCRIPTNORUN, name);
	else if (*possibleRuns[0]->returnType != ZVarType::ZVOID)
		printErrorMsg(node, SCRIPTRUNNOTVOID, name);

}

// ZScript::Variable

// ZScript::Function
