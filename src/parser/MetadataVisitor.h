#ifndef META_DATA_VISITOR_H
#define META_DATA_VISITOR_H

#include "ASTVisitors.h"
#include "ZScript.h"

namespace ZScript
{
	class MetadataVisitor : public RecursiveVisitor
	{
	public:
		MetadataVisitor(ZScript::Program& program);
		using RecursiveVisitor::visit;
		void visit(AST& node, void* param = NULL);
		////////////////
		// Cases
		void caseFile(ASTFile& host, void* param);
		void caseNamespace(ASTNamespace& host, void* param);
		void caseScript(ASTScript& host, void* param);
		void caseClass(ASTClass& host, void* param);
		void caseDataDecl(ASTDataDecl& host, void* param);
		void caseFuncDecl(ASTFuncDecl& host, void* param);
		void caseExprIdentifier(ASTExprIdentifier& host, void* param);
		void caseExprArrow(ASTExprArrow& host, void* param);
		void caseExprCall(ASTExprCall& host, void* param);

		std::string getOutput();

	private:
		ZScript::Program& program;
	};
}

#endif
