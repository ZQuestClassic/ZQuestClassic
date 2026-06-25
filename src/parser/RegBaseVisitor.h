// Designed as a Visitor for shared code between RegistrationVisitor and SemanticAnalyzer,
// as they often require duplicate code blocks, which becomes unruly to maintain. This allows
// declaring the code once in a shared space that still has full access to the necessary object members.
#pragma once

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "ZScript.h"

namespace ZScript
{
	class RegBaseVisitor : public RecursiveVisitor
	{
	public:
		RegBaseVisitor(ZScript::Program& program);
		
		void handle_data_decl_registry(ASTDataDecl& host);

		void initInternalVar(ASTDataDeclList* node);
	};
}

