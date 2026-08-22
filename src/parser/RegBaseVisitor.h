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
		
		// Visitor Cases
		void caseDataEnum(ASTDataEnum& host, void* param = NULL);
		
		// Annotation Helpers
		map<string, ASTAnnotation*> validate_annotation_keys(ASTAnnotationList& annot_list,
			vector<string> const& expected, string const& object_type_name);
		bool parse_annot_param_as(ASTAnnotation& annot, size_t idx, AnnotParam_Parsed& output,
			AnnotParam_Parsed::Type ty);
		bool validate_annot_param_count(ASTAnnotation& annot, size_t min, optional<size_t> max = nullopt);
		bool validate_annot_exclusions(string const& key, map<string, ASTAnnotation*> const& annots,
			vector<string> const& bad_keys);
		bool validate_annot_string_size(ASTAnnotation& annot, string& str, size_t max_length);
		
		// Annotation Handlers
		bool parse_annotations_enum(ASTDataEnum& node);
		bool parse_annotations_loop(ASTStmtRangeLoop& node);
		bool parse_annotations_data(ASTDataDeclList& node);
		bool parse_annotations_script(ASTScript& node);
		
		// Misc Virtuals
		virtual bool isRegistration() const {return false;}
		
	protected:
		bool hasChanged = false;
		
		void doRegister(AST& host);
		////////////////////////////////////////////////////////////////
		// Convenience Functions
		// Quickly checks if a node, or container of nodes, is all registered
		bool registered(AST& node) const;
		bool registered(AST* node) const; //Shortcut for pointer
		template <class Container>
		bool registered_vec(Container const& nodes) const
		{
			for (auto it = nodes.cbegin(); it != nodes.cend(); ++it)
			{
				if (!registered(*it)) return false;
			}
			return true;
		}

		template <class Container>
		void block_regvisit_vec(Container const& nodes, void* param = NULL)
		{
			for (auto it = nodes.cbegin();
				 it != nodes.cend(); ++it)
			{
				failure_temp = false;
				visit(**it, param);
				if (failure_halt) return;
			}
		}
	};
}

