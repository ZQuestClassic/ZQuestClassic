#ifndef DOC_VISITOR_H
#define DOC_VISITOR_H

#include "ASTVisitors.h"
#include "ZScript.h"

namespace ZScript
{
	class DocVisitor : public RecursiveVisitor
	{
	public:
		DocVisitor(ZScript::Program& program);
		using RecursiveVisitor::visit;
		void visit(AST& node, void* param = NULL);
		////////////////
		// Cases
		void caseFile(ASTFile& host, void* param);
		void caseClass(ASTClass& host, void* param);
		void caseDataDecl(ASTDataDecl& host, void* param);
		void caseDataEnum(ASTDataEnum& host, void* param);
		void caseFuncDecl(ASTFuncDecl& host, void* param);

		std::string getOutput();
	};
}

#endif
