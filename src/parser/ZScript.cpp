#include "ZScript.h"

using namespace std;
using namespace ZScript;

// ZScript::Program

Program::Program(ASTProgram* program) : node(program), globalScope(table)
{
	assert(program);

	// Create a ZScript::Script for every script in the program.
	for (vector<ASTScript*>::const_iterator it = node->scripts.begin();
		 it != node->scripts.end(); ++it)
	{
		ASTScript* node = *it;
		scripts.push_back(Script(node));
		Script* script = &scripts.back();
		scriptsByName[script->getName()] = script;
		scriptsByNode[node] = script;
	}

	// Move script-scope variable and function declarations to global scope.
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

// ZScript::Script

Script::Script(ASTScript* script) : node(script)
{
	assert(script);
}

string Script::getName() const {return node->getName();}

ScriptType Script::getType() const {return node->getType()->getType();}

// ZScript::Variable

// ZScript::Function
