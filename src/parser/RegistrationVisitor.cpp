/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Venrob
 */

#include "RegistrationVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"

using std::string;
using std::vector;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// RegistrationVisitor

RegistrationVisitor::RegistrationVisitor(Program& program)
	: program(program)
{
	scope = &program.getScope();
	caseRoot(program.getRoot());
	assert(dynamic_cast<RootScope*>(scope));
}

void caseDefault(AST& host, void* param)
{
	assert(false); //This should never be reached.
}

//Handle the root file specially!
void caseRoot(ASTFile& host, void* param)
{
	int recursionLimit = 50;
	while(--recursionLimit)
	{
		caseFile(host, param);
		if(registered(host)) return;
	}
	//Failed recursionLimit
	//VENROBTODO CompileError here!
}

void caseFile(ASTFile& host, void* param)
{
	//VENROBTODO do stuff here!
}

