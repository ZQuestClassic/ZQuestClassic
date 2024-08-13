#ifndef META_DATA_VISITOR_H_
#define META_DATA_VISITOR_H_

#include "ASTVisitors.h"
#include "ZScript.h"
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

namespace ZScript
{
	class MetadataVisitor : public RecursiveVisitor
	{
	public:
		MetadataVisitor(ZScript::Program& program, std::string root_file_name);
		using RecursiveVisitor::visit;
		void visit(AST& node, void* param = NULL);
		////////////////
		// Cases
		void caseFile(ASTFile& host, void* param);
		void caseImportDecl(ASTImportDecl& host, void* param);
		void caseNamespace(ASTNamespace& host, void* param);
		void caseScript(ASTScript& host, void* param);
		void caseClass(ASTClass& host, void* param);
		void caseDataDecl(ASTDataDecl& host, void* param);
		void caseFuncDecl(ASTFuncDecl& host, void* param);
		void caseExprIdentifier(ASTExprIdentifier& host, void* param);
		void caseExprArrow(ASTExprArrow& host, void* param);
		void caseExprCall(ASTExprCall& host, void* param);

		json takeOutput();
	
	private:
		std::string root_file_name;
		bool is_enabled;
	};
}

#endif
