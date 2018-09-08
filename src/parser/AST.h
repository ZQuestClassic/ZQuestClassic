#ifndef ZSCRIPT_AST_H
#define ZSCRIPT_AST_H

// Declare for flex and bison.
namespace ZScript
{
	class AST;
}
#define YYSTYPE ZScript::AST*

#ifdef _MSC_VER
#if (_MSC_VER <= 1600)
#define snprintf _snprintf
#endif
#endif

#include <list>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "y.tab.hpp"
#include "Compiler.h"
#include "CompileOption.h"

////////////////////////////////////////////////////////////////

extern std::string curfilename;

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations
	
	// ASTVisitors.h
	class ASTVisitor;
	// CompileError.h
	class CompileError;
	class CompileErrorHandler;
	// Scope.h
	class Scope;
	class ZClass;
	// ZScript.h
	class Datum;
	class Literal;
	class Function;

	// Local
	class AST; // virtual
	class ASTFile;
	class ASTFloat;
	class ASTString;
	class ASTSetOption;
	// Statements
	class ASTStmt; // virtual
	class ASTBlock;
	class ASTStmtIf;
	class ASTStmtIfElse;
	class ASTStmtSwitch;
	class ASTSwitchCases;
	class ASTStmtFor;
	class ASTStmtWhile;
	class ASTStmtDo;
	class ASTStmtReturn;
	class ASTStmtReturnVal;
	class ASTStmtBreak;
	class ASTStmtContinue;
	class ASTStmtEmpty;
	// Declarations
	class ASTDecl; // virtual
	class ASTScript;
	class ASTImportDecl;
	class ASTFuncDecl;
	class ASTDataDeclList;
	class ASTDataDecl;
	class ASTDataDeclExtraArray;
	class ASTTypeDef;
	// Expressions
	class ASTExpr; // virtual
	class ASTExprConst;
	class ASTExprAssign;
	class ASTExprIdentifier;
	class ASTExprArrow;
	class ASTExprIndex;
	class ASTExprCall;
	class ASTUnaryExpr; // virtual
	class ASTExprNegate;
	class ASTExprNot;
	class ASTExprBitNot;
	class ASTExprIncrement;
	class ASTExprPreIncrement;
	class ASTExprDecrement;
	class ASTExprPreDecrement;
	class ASTBinaryExpr; // virtual
	class ASTLogExpr; // virtual
	class ASTExprAnd;
	class ASTExprOr;
	class ASTRelExpr; // virtual
	class ASTExprGT;
	class ASTExprGE;
	class ASTExprLT;
	class ASTExprLE;
	class ASTExprEQ;
	class ASTExprNE;
	class ASTAddExpr; // virtual
	class ASTExprPlus;
	class ASTExprMinus;
	class ASTMultExpr; // virtual
	class ASTExprTimes;
	class ASTExprDivide;
	class ASTExprModulo;
	class ASTBitExpr; // virtual
	class ASTExprBitAnd;
	class ASTExprBitOr;
	class ASTExprBitXor;
	class ASTShiftExpr; // virtual
	class ASTExprLShift;
	class ASTExprRShift;
	// Literals
	class ASTLiteral; // virtual
	class ASTNumberLiteral;
	class ASTBoolLiteral;
	class ASTStringLiteral;
	class ASTArrayLiteral;
	class ASTOptionValue;
	// Types
	class ASTScriptType;
	class ASTDataType;

	//////////////////////////////////////////////////////////////////////////////
	class LocationData
	{
	public:
		static LocationData const NONE;

		int first_line;
		int last_line;
		int first_column;
		int last_column;
		std::string fname;

		LocationData()
			: first_line(-1), last_line(-1),
			  first_column(-1), last_column(-1),
			  fname(curfilename)
		{}

		LocationData(YYLTYPE loc)
			: first_line(loc.first_line), last_line(loc.last_line),
			  first_column(loc.first_column), last_column(loc.last_column),
			  fname(curfilename)
		{}

		std::string asString() const;
	};

	////////////////////////////////////////////////////////////////

	class AST
	{
	public:
		AST(LocationData const& location = LocationData::NONE);
		AST(AST const& rhs);
		virtual ~AST();
		// Calls subclass's copy constructor on self.
		virtual AST* clone() const = 0;

		virtual void execute(ASTVisitor& visitor, void* param = NULL) = 0;
		virtual std::string asString() const {return "unknown";}

		// Filename and linenumber.
		LocationData location;

		// List of expected compile error ids for this node. They are removed as
		// they are encountered.
		std::list<ASTExpr*> compileErrorCatches;

		// If this node has been disabled due to an error.
		bool disabled;
	
		// Subclass Predicates (replacing typeof and such).
		virtual bool isTypeArrow() const {return false;}
		virtual bool isTypeIndex() const {return false;}
		virtual bool isTypeIdentifier() const {return false;}
		virtual bool isTypeVarDecl() const {return false;}
		virtual bool isTypeArrayDecl() const {return false;}
		virtual bool isStringLiteral() const {return false;}
		virtual bool isArrayLiteral() const {return false;}

		// Clone a single node pointer.
		template <class Node>
		static Node* clone(Node* node) {return node ? node->clone() : NULL;}
		// Clone a single node auto pointer.
		template <class Node>
		static std::auto_ptr<Node> clone(std::auto_ptr<Node> const& node) {
			return node.get()
				? std::auto_ptr<Node>(node->clone())
				: std::auto_ptr<Node>();}
		// Clone a vector of AST nodes.
		template <class Node>
		static std::vector<Node*> clone(std::vector<Node*> const& nodes)
		{
			std::vector<Node*> clones;
			for (typename std::vector<Node*>::const_iterator it = nodes.begin();
			     it != nodes.end(); ++it)
				clones.push_back((*it)->clone());
			return clones;
		}	
		// Clone a list of AST nodes.
		template <class Node> static std::list<Node*> clone(std::list<Node*> const& nodes)
		{
			std::list<Node*> clones;
			for (typename std::list<Node*>::const_iterator it = nodes.begin();
			     it != nodes.end(); ++it)
				clones.push_back((*it)->clone());
			return clones;
		}
		// Execute a single node pointer.
		template <class Node>
		static void execute(Node* node, ASTVisitor& visitor, void* param = NULL)
		{
			if (node) node->execute(visitor, param);
		}
		// Execute a vector of AST nodes.
		template <class Node>
		static void execute(
				std::vector<Node*> const& nodes, ASTVisitor& visitor,
				void* param = NULL)
		{
			for (typename std::vector<Node*>::const_iterator it = nodes.begin();
			     it != nodes.end(); ++it)
				(*it)->execute(visitor, param);
		}
		// Execute a list of AST nodes.
		template <class Node>
		static void execute(
				std::list<Node*> const& nodes, ASTVisitor& visitor,
				void* param = NULL)
		{
			for (typename std::list<Node*>::const_iterator it = nodes.begin();
			     it != nodes.end(); ++it)
				(*it)->execute(visitor, param);
		}

	protected:
		// For leaf classes to call in their own assignment operators.
		AST& operator=(AST const& rhs);
	};


	////////////////////////////////////////////////////////////////

	class ASTFile : public AST
	{
	public:
		ASTFile(LocationData const& location = LocationData::NONE);
		ASTFile(ASTFile const& base);
		virtual ~ASTFile();
		virtual ASTFile& operator=(ASTFile const& rhs);
		virtual ASTFile* clone() const {return new ASTFile(*this);}
    
		virtual void execute(ASTVisitor& visitor, void* param = NULL);
		virtual std::string asString() const;

		// Add a declaration to the proper list based on its type.
		void addDeclaration(ASTDecl* declaration);
		bool hasDeclarations() const;

		std::vector<ASTSetOption*> options;
		std::vector<ASTImportDecl*> imports;
		std::vector<ASTDataDeclList*> variables;
		std::vector<ASTFuncDecl*> functions;
		std::vector<ASTTypeDef*> types;
		std::vector<ASTScript*> scripts;
	};

	class ASTFloat : public AST
	{
	public:
		enum Type {TYPE_DECIMAL, TYPE_BINARY, TYPE_HEX};
	
		ASTFloat(char* value, Type type,
		         LocationData const& location = LocationData::NONE);
		ASTFloat(char const* value, Type type,
		         LocationData const& location = LocationData::NONE);
		ASTFloat(std::string const& value, Type type,
		         LocationData const& location = LocationData::NONE);
		ASTFloat(long value, Type type,
		         LocationData const& location = LocationData::NONE);
		ASTFloat(ASTFloat const& base);
		ASTFloat& operator=(ASTFloat const& rhs);
		ASTFloat* clone() const {return new ASTFloat(*this);}
	
		void execute(ASTVisitor& visitor, void* param = NULL);
    	
		std::pair<std::string,std::string> parseValue();

		Type type;
		std::string value;
		bool negative;
	};

	class ASTString : public AST
	{
	public:
		ASTString(const char* str,
		          LocationData const& location = LocationData::NONE);
		ASTString(std::string const& str,
		          LocationData const& location = LocationData::NONE);
		ASTString(ASTString const& base);
		ASTString& operator=(ASTString const& rhs);
		ASTString* clone() const {return new ASTString(*this);}
	
		void execute(ASTVisitor& visitor, void* param = NULL);

		std::string getValue() const {return str;}
	private:
		std::string str;
	};

	class ASTSetOption : public AST
	{
	public:
		ASTSetOption(std::string const& name, ASTExprConst* expr,
		             LocationData const& location = LocationData::NONE);
		ASTSetOption(std::string const& name,
		             CompileOptionSetting setting,
		             LocationData const& location = LocationData::NONE);
		ASTSetOption(ASTSetOption const& base);
		virtual ASTSetOption& operator=(ASTSetOption const& rhs);
		virtual ASTSetOption* clone() const {return new ASTSetOption(*this);}

		virtual void execute(ASTVisitor& visitor, void* param = NULL);
		virtual std::string asString() const;

		CompileOptionSetting getSetting(
				CompileErrorHandler* = NULL) const;
	
		std::string name;
		CompileOption option;
		std::auto_ptr<ASTExprConst> expr;
		CompileOptionSetting setting;
	};

	////////////////////////////////////////////////////////////////
	// Statements

	class ASTStmt : public AST
	{
	public:
		ASTStmt(LocationData const& location = LocationData::NONE);
		ASTStmt(ASTStmt const& base) : AST(base) {}
		virtual ASTStmt* clone() const = 0;

		bool isDisabled() const {return disabled_;}
		void disable() {disabled_ = true;}
	
	protected:
		ASTStmt& operator=(ASTStmt const& rhs);

	private:
		bool disabled_;
	};
    
	class ASTBlock : public ASTStmt
	{
	public:
		ASTBlock(LocationData const& location = LocationData::NONE);
		ASTBlock(ASTBlock const& base);
		~ASTBlock();
		ASTBlock& operator=(ASTBlock const& rhs);
		ASTBlock* clone() const {return new ASTBlock(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		// List of scope options.
		std::vector<ASTSetOption*> options;
	
		// List of statements this block contains.
		std::vector<ASTStmt*> statements;
	};
    
	class ASTStmtIf : public ASTStmt
	{
	public:
		ASTStmtIf(ASTExpr* condition, ASTStmt* thenStatement,
		          LocationData const& location = LocationData::NONE);
		ASTStmtIf(ASTStmtIf const& base);
		virtual ~ASTStmtIf();
		ASTStmtIf& operator=(ASTStmtIf const& rhs);
		ASTStmtIf* clone() const {return new ASTStmtIf(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTExpr* condition;
		ASTStmt* thenStatement;
	};

	class ASTStmtIfElse : public ASTStmtIf
	{
	public:
		ASTStmtIfElse(
				ASTExpr* condition, ASTStmt* thenStatement, ASTStmt* elseStatement,
				LocationData const& location = LocationData::NONE);
		ASTStmtIfElse(ASTStmtIfElse const& base);
		~ASTStmtIfElse();
		ASTStmtIfElse& operator=(ASTStmtIfElse const& rhs);
		ASTStmtIfElse* clone() const {return new ASTStmtIfElse(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTStmt* elseStatement;
	};

	// A switch statement.
	class ASTStmtSwitch : public ASTStmt
	{
	public:
		ASTStmtSwitch(LocationData const& location = LocationData::NONE);
		ASTStmtSwitch(ASTStmtSwitch const& base);
		~ASTStmtSwitch();
		ASTStmtSwitch& operator=(ASTStmtSwitch const& rhs);
		ASTStmtSwitch* clone() const {return new ASTStmtSwitch(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		// The key expression used to switch.
		ASTExpr* key;
		// A vector of case groupings.
		std::vector<ASTSwitchCases*> cases;
	private:
	};

	// A grouping of switch statement labels, and the code for the group.
	class ASTSwitchCases : public AST
	{
	public:
		ASTSwitchCases(LocationData const& location = LocationData::NONE);
		ASTSwitchCases(ASTSwitchCases const& base);
		~ASTSwitchCases();
		ASTSwitchCases& operator=(ASTSwitchCases const& rhs);
		ASTSwitchCases* clone() const {return new ASTSwitchCases(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		// The list of case labels.
		std::vector<ASTExprConst*> cases;
		// If the default case is included in this grouping.
		bool isDefault;
		// The block to run.
		ASTBlock* block;
	};


	class ASTStmtFor : public ASTStmt
	{
	public:
		ASTStmtFor(ASTStmt* setup = NULL,
		           ASTExpr* test = NULL,
		           ASTStmt* increment = NULL,
		           ASTStmt* body = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTStmtFor(ASTStmtFor const& base);
		~ASTStmtFor();
		ASTStmtFor& operator=(ASTStmtFor const& rhs);
		ASTStmtFor* clone() const {return new ASTStmtFor(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTStmt* setup;
		ASTExpr* test;
		ASTStmt* increment;
		ASTStmt* body;
	};

	class ASTStmtWhile : public ASTStmt
	{
	public:
		ASTStmtWhile(ASTExpr* test = NULL,
		             ASTStmt* body = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTStmtWhile(ASTStmtWhile const& base);
		~ASTStmtWhile();
		ASTStmtWhile& operator=(ASTStmtWhile const& rhs);
		ASTStmtWhile* clone() const {return new ASTStmtWhile(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTExpr* test;
		ASTStmt* body;
	};

	class ASTStmtDo : public ASTStmt
	{
	public:
		ASTStmtDo(ASTExpr* test = NULL,
		          ASTStmt* body = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTStmtDo(ASTStmtDo const& base);
		~ASTStmtDo();
		ASTStmtDo& operator=(ASTStmtDo const& rhs);
		ASTStmtDo* clone() const {return new ASTStmtDo(*this);}
	
		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTExpr* test;
		ASTStmt* body;
	};

	class ASTStmtReturn : public ASTStmt
	{
	public:
		ASTStmtReturn(LocationData const& location = LocationData::NONE);
		ASTStmtReturn(ASTStmtReturn const& base);
		ASTStmtReturn& operator=(ASTStmtReturn const& rhs);
		ASTStmtReturn* clone() const {return new ASTStmtReturn(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
	};

	class ASTStmtReturnVal : public ASTStmtReturn
	{
	public:
		ASTStmtReturnVal(ASTExpr* value = NULL,
		                 LocationData const& location = LocationData::NONE);
		ASTStmtReturnVal(ASTStmtReturnVal const& base);
		~ASTStmtReturnVal();
		ASTStmtReturnVal& operator=(ASTStmtReturnVal const& rhs);
		ASTStmtReturnVal* clone() const {return new ASTStmtReturnVal(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTExpr* value;
	};

	class ASTStmtBreak : public ASTStmt
	{
	public:
		ASTStmtBreak(LocationData const& location = LocationData::NONE);
		ASTStmtBreak(ASTStmtBreak const& base);
		ASTStmtBreak& operator=(ASTStmtBreak const& rhs);
		ASTStmtBreak* clone() const {return new ASTStmtBreak(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
	};

	class ASTStmtContinue : public ASTStmt
	{
	public:
		ASTStmtContinue(LocationData const& location = LocationData::NONE);
		ASTStmtContinue(ASTStmtContinue const& base);
		ASTStmtContinue& operator=(ASTStmtContinue const& rhs);
		ASTStmtContinue* clone() const {return new ASTStmtContinue(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
	};

	class ASTStmtEmpty : public ASTStmt
	{
	public:
		ASTStmtEmpty(LocationData const& location = LocationData::NONE);
		ASTStmtEmpty(ASTStmtEmpty const& base);
		ASTStmtEmpty& operator=(ASTStmtEmpty const& rhs);
		ASTStmtEmpty* clone() const {return new ASTStmtEmpty(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
	};

	////////////////////////////////////////////////////////////////
	// Declarations

	// Different declaration subclasses.
	enum ASTDeclClassId
	{
		ASTDECL_CLASSID_NONE,
		ASTDECL_CLASSID_SCRIPT,
		ASTDECL_CLASSID_IMPORT,
		ASTDECL_CLASSID_CONSTANT,
		ASTDECL_CLASSID_FUNCTION,
		ASTDECL_CLASSID_DATALIST,
		ASTDECL_CLASSID_DATA,
		ASTDECL_CLASSID_TYPE
	};

	// virtual
	class ASTDecl : public ASTStmt
	{
	public:
		ASTDecl(LocationData const& location = LocationData::NONE);
		ASTDecl(ASTDecl const& base) : ASTStmt(base) {}
		virtual ASTDecl* clone() const = 0;

		// Return the subclass id.
		virtual ASTDeclClassId declarationClassId() const = 0;

	protected:
		virtual ASTDecl& operator=(ASTDecl const& rhs);
	};

	// Declares a script.
	class ASTScript : public ASTDecl
	{
	public:
		ASTScript(LocationData const& location = LocationData::NONE);
		ASTScript(ASTScript const& base);
		~ASTScript();
		ASTScript& operator=(ASTScript const& rhs);
		ASTScript* clone() const {return new ASTScript(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTDeclClassId declarationClassId() const {
			return ASTDECL_CLASSID_SCRIPT;}
    
		// Adds a declaration to the proper vector.
		void addDeclaration(ASTDecl& declaration);

		ASTScriptType* type;
		std::string name;
		std::vector<ASTSetOption*> options;
		std::vector<ASTDataDeclList*> variables;
		std::vector<ASTFuncDecl*> functions;
		std::vector<ASTTypeDef*> types;
	};

	class ASTImportDecl : public ASTDecl
	{
	public:
		ASTImportDecl(std::string const& filename,
		              LocationData const& location = LocationData::NONE);
		ASTImportDecl(ASTImportDecl const& base);
		virtual ASTImportDecl& operator=(ASTImportDecl const& rhs);
		virtual ASTImportDecl* clone() const {return new ASTImportDecl(*this);}
    
		virtual void execute(ASTVisitor& visitor, void* param = NULL);

		virtual ASTDeclClassId declarationClassId() const {
			return ASTDECL_CLASSID_IMPORT;}

		std::string const& getFilename() const {return filename_;}
		ASTFile* getTree() const {return tree_.get();}
		void giveTree(std::auto_ptr<ASTFile> tree) {tree_ = tree;}
	
	private:
		std::string filename_;
		std::auto_ptr<ASTFile> tree_;
	};

	class ASTFuncDecl : public ASTDecl
	{
	public:
		ASTFuncDecl(LocationData const& location = LocationData::NONE);
		ASTFuncDecl(ASTFuncDecl const& base);
		~ASTFuncDecl();
		ASTFuncDecl& operator=(ASTFuncDecl const& rhs);
		ASTFuncDecl* clone() const {return new ASTFuncDecl(*this);}
    
		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTDeclClassId declarationClassId() const {
			return ASTDECL_CLASSID_FUNCTION;}

		ASTDataType* returnType;
		std::vector<ASTDataDecl*> parameters;
		std::string name;
		ASTBlock* block;
	};

	// A line of variable/constant declarations:
	// int a, b, c[];
	class ASTDataDeclList : public ASTDecl
	{
	public:
		ASTDataDeclList(LocationData const& location = LocationData::NONE);
		ASTDataDeclList(ASTDataDeclList const& base);
		~ASTDataDeclList();
		ASTDataDeclList& operator=(ASTDataDeclList const& rhs);
		ASTDataDeclList* clone() const {return new ASTDataDeclList(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
		ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_DATALIST;}

		// The base type at the start of the line shared by all the declarations.
		ASTDataType* baseType;

		std::vector<ASTDataDecl*> const& declarations() const {return mDeclarations;}
		void addDeclaration(ASTDataDecl* declaration);

	private:
		// The list of individual data declarations.
		std::vector<ASTDataDecl*> mDeclarations;
	};

	// Declares a single variable or constant. May or may not be inside an
	// ASTDataDeclList.
	class ASTDataDecl : public ASTDecl
	{
	public:
		ASTDataDecl(LocationData const& location = LocationData::NONE);
		ASTDataDecl(ASTDataDecl const& base);
		~ASTDataDecl();
		ASTDataDecl& operator=(ASTDataDecl const& rhs);
		ASTDataDecl* clone() const {return new ASTDataDecl(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
		ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_DATA;}

		// The list containing this declaration. Should be set by that list when
		// this is added.
		ASTDataDeclList* list;

		// Reference back to the datum manager for this node. Should be set by
		// that Datum when it is created.
		Datum* manager;

		// This type of this data (minus the extra arrays). This should only be
		// set if this declaration is not part of a list, as the list's base type
		// should be used instead in that case.
		ASTDataType* baseType;

		// The symbol this declaration is binding.
		std::string name;

		ASTExpr* initializer() const {return mInitializer;}
		ASTExpr* initializer(ASTExpr* initializer);

		// Extra array type for this specific declaration. The final type is the
		// list's base type combined with these.
		std::vector<ASTDataDeclExtraArray*> extraArrays;

		// Resolves the type, using either the list's or this node's own base type
		// as appropriate.
		DataType const* resolveType(Scope* scope) const;

	private:
		// The initialization expression. Optional.
		ASTExpr* mInitializer;
	};

	// The extra array parameters appended to a data declaration name.
	class ASTDataDeclExtraArray : public AST
	{
	public:
		ASTDataDeclExtraArray(LocationData const& location = LocationData::NONE);
		ASTDataDeclExtraArray(ASTDataDeclExtraArray const& base);
		~ASTDataDeclExtraArray();
		ASTDataDeclExtraArray& operator=(ASTDataDeclExtraArray const& rhs);
		ASTDataDeclExtraArray* clone() const {return new ASTDataDeclExtraArray(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		// The vector of array dimensions. Empty means unspecified.
		std::vector<ASTExpr*> dimensions;

		// If this declares an a sized array.
		bool hasSize() const {return dimensions.size();}

		// Get the total size of this array at compile time.
		optional<int> getCompileTimeSize(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTTypeDef : public ASTDecl
	{
	public:
		ASTTypeDef(ASTDataType* type = NULL,
		           std::string const& name = "",
		           LocationData const& location = LocationData::NONE);
		ASTTypeDef(ASTTypeDef const& base);
		~ASTTypeDef();
		ASTTypeDef& operator=(ASTTypeDef const& rhs);
		ASTTypeDef* clone() const {return new ASTTypeDef(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_TYPE;}

		ASTDataType* type;
		std::string name;
	};

	////////////////////////////////////////////////////////////////
	// Expressions

	class ASTExpr : public ASTStmt
	{
	public:
		ASTExpr(LocationData const& location = LocationData::NONE);
		ASTExpr(ASTExpr const& base);
		virtual ASTExpr* clone() const = 0;

		virtual bool isConstant() const = 0;

		// Return this expression's value if it has already been resolved at
		// compile time.
		virtual optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const
		{return nullopt;}

		// Returns the read or write type for this expression. Null for either
		// means that it can't be read from/written to.
		virtual DataType const* getReadType() const {return getVarType();}
		virtual DataType const* getWriteType() const {return getVarType();}
	
		// phasing out this group for the above two.
		DataType const* getVarType() const {return varType;}
		void setVarType(DataType const& type) {varType = &type;}
		void setVarType(DataType& type) {varType = (DataType const*)&type;}
		void setVarType(DataType const* type) {varType = type;}
		void setVarType(DataType* type) {varType = (DataType const*)type;}

	private:
		DataType const* varType;

	protected:
		ASTExpr& operator=(ASTExpr const& rhs);
	};

	// Wrap around an expression to type it as constant.
	class ASTExprConst : public ASTExpr
	{
	public:
		ASTExprConst(ASTExpr* content = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTExprConst(ASTExprConst const& base);
		~ASTExprConst() {delete content;}
		ASTExprConst& operator=(ASTExprConst const& rhs);
		ASTExprConst* clone() const {return new ASTExprConst(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return true;}

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {
			return content ? content->getReadType() : NULL;}
		DataType const* getWriteType() const {return NULL;}
	
		ASTExpr* content;
	};

	class ASTExprAssign : public ASTExpr
	{
	public:
		ASTExprAssign(ASTExpr* left = NULL,
		              ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprAssign(ASTExprAssign const& base);
		~ASTExprAssign();
		ASTExprAssign& operator=(ASTExprAssign const& rhs);
		ASTExprAssign* clone() const {return new ASTExprAssign(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return right && right->isConstant();}

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {
			return right ? right->getReadType() : NULL;}
		DataType const* getWriteType() const {
			return right ? right->getWriteType() : NULL;}
	
		ASTExpr* left;
		ASTExpr* right;
	};

	class ASTExprIdentifier : public ASTExpr
	{
	public:
		ASTExprIdentifier(std::string const& name = "",
		                  LocationData const& location = LocationData::NONE);
		ASTExprIdentifier(ASTExprIdentifier const& base);
		ASTExprIdentifier& operator=(ASTExprIdentifier const& base);
		ASTExprIdentifier* clone() const {return new ASTExprIdentifier(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
		std::string asString() const;
		bool isTypeIdentifier() const {return true;}

		bool isConstant() const {return mIsConstant;}
		void markConstant() {mIsConstant = true;}

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const;
		DataType const* getWriteType() const;
	
		// The identifier components separated by '.'.
		std::vector<std::string> components;

		// What this identifier refers to.
		Datum* binding;
	private:
		bool mIsConstant;
	};

	class ASTExprArrow : public ASTExpr
	{
	public:
		ASTExprArrow(ASTExpr* left = NULL,
		             std::string const& right = "",
		             LocationData const& location = LocationData::NONE);
		ASTExprArrow(ASTExprArrow const& base);
		~ASTExprArrow();
		ASTExprArrow& operator=(ASTExprArrow const& rhs);
		ASTExprArrow* clone() const {return new ASTExprArrow(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
		std::string asString() const;
		bool isTypeArrow() const {return true;}

		bool isConstant() const {return false;}

		DataType const* getReadType() const;
		DataType const* getWriteType() const;
	
		ASTExpr* left;
		std::string right;
		ASTExpr* index;

		ZClass* leftClass;
		Function* readFunction;
		Function* writeFunction;
	};

	class ASTExprIndex : public ASTExpr
	{
	public:
		ASTExprIndex(ASTExpr* array = NULL,
		             ASTExpr* index = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTExprIndex(ASTExprIndex const& base);
		~ASTExprIndex() {delete array; delete index;}
		ASTExprIndex& operator=(ASTExprIndex const& rhs);
		ASTExprIndex* clone() const {return new ASTExprIndex(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);
		bool isTypeIndex() const {return true;}
    
		bool isConstant() const;

		DataType const* getReadType() const;
		DataType const* getWriteType() const;
	
		ASTExpr* array;
		ASTExpr* index;
	};

	class ASTExprCall : public ASTExpr
	{
	public:
		ASTExprCall(LocationData const& location = LocationData::NONE);
		ASTExprCall(ASTExprCall const& base);
		~ASTExprCall();
		ASTExprCall& operator=(ASTExprCall const& rhs);
		ASTExprCall* clone() const {return new ASTExprCall(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return false;}

		DataType const* getReadType() const;
		DataType const* getWriteType() const;
	
		ASTExpr* left;
		std::vector<ASTExpr*> parameters;

		Function* binding;
	};

	// virtual
	class ASTUnaryExpr : public ASTExpr
	{
	public:
		ASTUnaryExpr(LocationData const& location = LocationData::NONE);
		ASTUnaryExpr(ASTUnaryExpr const& base);
		virtual ~ASTUnaryExpr() {delete operand;}
		virtual ASTUnaryExpr* clone() const = 0;

		virtual bool isConstant() const {return operand->isConstant();}

		ASTExpr* operand;

	protected:
		ASTUnaryExpr& operator=(ASTUnaryExpr const& rhs);
	};

	class ASTExprNegate : public ASTUnaryExpr
	{
	public:
		ASTExprNegate(LocationData const& location = LocationData::NONE);
		ASTExprNegate(ASTExprNegate const& base);
		ASTExprNegate& operator=(ASTExprNegate const& rhs);
		ASTExprNegate* clone() const {return new ASTExprNegate(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	};

	class ASTExprNot : public ASTUnaryExpr
	{
	public:
		ASTExprNot(LocationData const& location = LocationData::NONE);
		ASTExprNot(ASTExprNot const& base);
		ASTExprNot& operator=(ASTExprNot const& rhs);
		ASTExprNot* clone() const {return new ASTExprNot(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {return &DataType::BOOL;}
		DataType const* getWriteType() const {return NULL;}
	};

	class ASTExprBitNot : public ASTUnaryExpr
	{
	public:
		ASTExprBitNot(LocationData const& location = LocationData::NONE);
		ASTExprBitNot(ASTExprBitNot const& base);
		ASTExprBitNot& operator=(ASTExprBitNot const& rhs);
		ASTExprBitNot* clone() const {return new ASTExprBitNot(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	};

	class ASTExprIncrement : public ASTUnaryExpr
	{
	public:
		ASTExprIncrement(LocationData const& location = LocationData::NONE);
		ASTExprIncrement(ASTExprIncrement const& base);
		ASTExprIncrement& operator=(ASTExprIncrement const& rhs);
		ASTExprIncrement* clone() const {return new ASTExprIncrement(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return false;}

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {
			return operand ? operand->getWriteType() : NULL;}
	};

	class ASTExprPreIncrement : public ASTUnaryExpr
	{
	public:
		ASTExprPreIncrement(LocationData const& location = LocationData::NONE);
		ASTExprPreIncrement(ASTExprPreIncrement const& base);
		ASTExprPreIncrement& operator=(ASTExprPreIncrement const& rhs);
		ASTExprPreIncrement* clone() const {return new ASTExprPreIncrement(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return false;}

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {
			return operand ? operand->getWriteType() : NULL;}
	};

	class ASTExprDecrement : public ASTUnaryExpr
	{
	public:
		ASTExprDecrement(LocationData const& location = LocationData::NONE);
		ASTExprDecrement(ASTExprDecrement const& base);
		ASTExprDecrement& operator=(ASTExprDecrement const& rhs);
		ASTExprDecrement* clone() const {return new ASTExprDecrement(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return false;}

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {
			return operand ? operand->getWriteType() : NULL;}
	};

	class ASTExprPreDecrement : public ASTUnaryExpr
	{
	public:
		ASTExprPreDecrement(LocationData const& location = LocationData::NONE);
		ASTExprPreDecrement(ASTExprPreDecrement const& base);
		ASTExprPreDecrement& operator=(ASTExprPreDecrement const& rhs);
		ASTExprPreDecrement* clone() const {
			return new ASTExprPreDecrement(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return false;}

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {
			return operand ? operand->getWriteType() : NULL;}
	};

	// virtual
	class ASTBinaryExpr : public ASTExpr
	{
	public:
		ASTBinaryExpr(ASTExpr* left = NULL,
		              ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTBinaryExpr(ASTBinaryExpr const& base);
		virtual ~ASTBinaryExpr();
		virtual ASTBinaryExpr* clone() const = 0;

		bool isConstant() const;

		ASTExpr* left;
		ASTExpr* right;

	protected:
		ASTBinaryExpr& operator=(ASTBinaryExpr const& rhs);
	};

	// virtual
	class ASTLogExpr : public ASTBinaryExpr
	{
	public:
		ASTLogExpr(ASTExpr* left = NULL,
		           ASTExpr* right = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTLogExpr(ASTLogExpr const& base);
		virtual ASTLogExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::BOOL;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTLogExpr& operator=(ASTLogExpr const& rhs);
	};

	class ASTExprAnd : public ASTLogExpr
	{
	public:
		ASTExprAnd(ASTExpr* left = NULL,
		           ASTExpr* right = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTExprAnd(ASTExprAnd const& base);
		ASTExprAnd& operator=(ASTExprAnd const& rhs);
		ASTExprAnd* clone() const {return new ASTExprAnd(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprOr : public ASTLogExpr
	{
	public:
		ASTExprOr(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprOr(ASTExprOr const& base);
		ASTExprOr& operator=(ASTExprOr const& rhs);
		ASTExprOr* clone() const {return new ASTExprOr(*this);}
    
		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// virtual
	class ASTRelExpr : public ASTBinaryExpr
	{
	public:
		ASTRelExpr(ASTExpr* left = NULL,
		           ASTExpr* right = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTRelExpr(ASTRelExpr const& base);
		virtual ASTRelExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::BOOL;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTRelExpr& operator=(ASTRelExpr const& rhs);
	};

	class ASTExprGT : public ASTRelExpr
	{
	public:
		ASTExprGT(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprGT(ASTExprGT const& base);
		ASTExprGT& operator=(ASTExprGT const& rhs);
		ASTExprGT* clone() const {return new ASTExprGT(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprGE : public ASTRelExpr
	{
	public:
		ASTExprGE(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprGE(ASTExprGE const& base);
		ASTExprGE& operator=(ASTExprGE const& rhs);
		ASTExprGE* clone() const {return new ASTExprGE(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprLT : public ASTRelExpr
	{
	public:
		ASTExprLT(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprLT(ASTExprLT const& base);
		ASTExprLT& operator=(ASTExprLT const& rhs);
		ASTExprLT* clone() const {return new ASTExprLT(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprLE : public ASTRelExpr
	{
	public:
		ASTExprLE(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprLE(ASTExprLE const& base);
		ASTExprLE& operator=(ASTExprLE const& rhs);
		ASTExprLE* clone() const {return new ASTExprLE(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprEQ : public ASTRelExpr
	{
	public:
		ASTExprEQ(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprEQ(ASTExprEQ const& base);
		ASTExprEQ& operator=(ASTExprEQ const& rhs);
		ASTExprEQ* clone() const {return new ASTExprEQ(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprNE : public ASTRelExpr
	{
	public:
		ASTExprNE(ASTExpr* left = NULL,
		          ASTExpr* right = NULL,
		          LocationData const& location = LocationData::NONE);
		ASTExprNE(ASTExprNE const& base);
		ASTExprNE& operator=(ASTExprNE const& rhs);
		ASTExprNE* clone() const {return new ASTExprNE(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// virtual
	class ASTAddExpr : public ASTBinaryExpr
	{
	public:
		ASTAddExpr(ASTExpr* left = NULL,
		           ASTExpr* right = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTAddExpr(ASTAddExpr const& base);
		virtual ASTAddExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTAddExpr& operator=(ASTAddExpr const& rhs);
	};

	class ASTExprPlus : public ASTAddExpr
	{
	public:
		ASTExprPlus(ASTExpr* left = NULL,
		            ASTExpr* right = NULL,
		            LocationData const& location = LocationData::NONE);
		ASTExprPlus(ASTExprPlus const& base);
		ASTExprPlus& operator=(ASTExprPlus const& rhs);
		ASTExprPlus* clone() const {return new ASTExprPlus(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprMinus : public ASTAddExpr
	{
	public:
		ASTExprMinus(ASTExpr* left = NULL,
		             ASTExpr* right = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTExprMinus(ASTExprMinus const& base);
		ASTExprMinus& operator=(ASTExprMinus const& rhs);
		ASTExprMinus* clone() const {return new ASTExprMinus(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// virtual
	class ASTMultExpr : public ASTBinaryExpr
	{
	public:
		ASTMultExpr(ASTExpr* left = NULL,
		            ASTExpr* right = NULL,
		            LocationData const& location = LocationData::NONE);
		ASTMultExpr(ASTMultExpr const& base);
		virtual ASTMultExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTMultExpr& operator=(ASTMultExpr const& rhs);
	};

	class ASTExprTimes : public ASTMultExpr
	{
	public:
		ASTExprTimes(ASTExpr* left = NULL,
		             ASTExpr* right = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTExprTimes(ASTExprTimes const& base);
		ASTExprTimes& operator=(ASTExprTimes const& rhs);
		ASTExprTimes* clone() const {return new ASTExprTimes(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprDivide : public ASTMultExpr
	{
	public:
		ASTExprDivide(ASTExpr* left = NULL,
		              ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprDivide(ASTExprDivide const& base);
		ASTExprDivide& operator=(ASTExprDivide const& rhs);
		ASTExprDivide* clone() const {return new ASTExprDivide(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprModulo : public ASTMultExpr
	{
	public:
		ASTExprModulo(ASTExpr* left = NULL,
		              ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprModulo(ASTExprModulo const& base);
		ASTExprModulo& operator=(ASTExprModulo const& rhs);
		ASTExprModulo* clone() const {return new ASTExprModulo(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// virtual
	class ASTBitExpr : public ASTBinaryExpr
	{
	public:
		ASTBitExpr(ASTExpr* left = NULL, ASTExpr* right = NULL,
		           LocationData const& location = LocationData::NONE);
		ASTBitExpr(ASTBitExpr const& base);
		virtual ASTBitExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTBitExpr& operator=(ASTBitExpr const& rhs);
	};

	class ASTExprBitAnd : public ASTBitExpr
	{
	public:
		ASTExprBitAnd(ASTExpr* left = NULL, ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprBitAnd(ASTExprBitAnd const& base);
		ASTExprBitAnd& operator=(ASTExprBitAnd const& rhs);
		ASTExprBitAnd* clone() const {return new ASTExprBitAnd(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprBitOr : public ASTBitExpr
	{
	public:
		ASTExprBitOr(ASTExpr* left = NULL, ASTExpr* right = NULL,
		             LocationData const& location = LocationData::NONE);
		ASTExprBitOr(ASTExprBitOr const& base);
		ASTExprBitOr& operator=(ASTExprBitOr const& rhs);
		ASTExprBitOr* clone() const {return new ASTExprBitOr(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprBitXor : public ASTBitExpr
	{
	public:
		ASTExprBitXor(ASTExpr* left = NULL, ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprBitXor(ASTExprBitXor const& base);
		ASTExprBitXor& operator=(ASTExprBitXor const& rhs);
		ASTExprBitXor* clone() const {return new ASTExprBitXor(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// virtual
	class ASTShiftExpr : public ASTBinaryExpr
	{
	public:
		ASTShiftExpr(
				ASTExpr* left = NULL, ASTExpr* right = NULL,
				LocationData const& location = LocationData::NONE);
		ASTShiftExpr(ASTShiftExpr const& base);
		virtual ASTShiftExpr* clone() const = 0;

		DataType const* getReadType() const {return &DataType::FLOAT;}
		DataType const* getWriteType() const {return NULL;}
	protected:
		ASTShiftExpr& operator=(ASTShiftExpr const& rhs);
	};

	class ASTExprLShift : public ASTShiftExpr
	{
	public:
		ASTExprLShift(ASTExpr* left = NULL, ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprLShift(ASTExprLShift const& base);
		ASTExprLShift& operator=(ASTExprLShift const& rhs);
		ASTExprLShift* clone() const {return new ASTExprLShift(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	class ASTExprRShift : public ASTShiftExpr
	{
	public:
		ASTExprRShift(ASTExpr* left = NULL, ASTExpr* right = NULL,
		              LocationData const& location = LocationData::NONE);
		ASTExprRShift(ASTExprRShift const& base);
		ASTExprRShift& operator=(ASTExprRShift const& rhs);
		ASTExprRShift* clone() const {return new ASTExprRShift(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
	};

	// Literals

	// virtual
	class ASTLiteral : public ASTExpr
	{
	public:
		ASTLiteral(LocationData const& location = LocationData::NONE);
		ASTLiteral(ASTLiteral const& base) : ASTExpr(base), manager(NULL) {}
		virtual ASTLiteral* clone() const = 0;

		Literal* manager;

		DataType const* getWriteType() const {return NULL;}

	protected:
		ASTLiteral& operator=(ASTLiteral const& rhs);
	};

	class ASTNumberLiteral : public ASTLiteral
	{
	public:
		ASTNumberLiteral(
				ASTFloat* value = NULL,
				LocationData const& location = LocationData::NONE);
		ASTNumberLiteral(ASTNumberLiteral const& base);
		~ASTNumberLiteral() {delete value;}
		ASTNumberLiteral& operator=(ASTNumberLiteral const& rhs);
		ASTNumberLiteral* clone() const {return new ASTNumberLiteral(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return true;}

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const;
		DataType const* getReadType() const {return &DataType::FLOAT;}
	
		ASTFloat* value;
	};

	class ASTBoolLiteral : public ASTLiteral
	{
	public:
		ASTBoolLiteral(
				bool value = false,
				LocationData const& location = LocationData::NONE);
		ASTBoolLiteral(ASTBoolLiteral const& base);
		ASTBoolLiteral& operator=(ASTBoolLiteral const& base);
		ASTBoolLiteral* clone() const {return new ASTBoolLiteral(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		bool isConstant() const {return true;}

		optional<long> getCompileTimeValue(
				CompileErrorHandler* errorHandler = NULL)
				const {
			return value ? 10000L : 0L;}
		DataType const* getReadType() const {return &DataType::BOOL;}
	
		bool value;
	};

	class ASTStringLiteral : public ASTLiteral
	{
	public:
		ASTStringLiteral(
				char const* str = "",
				LocationData const& location = LocationData::NONE);
		ASTStringLiteral(
				std::string const& str,
				LocationData const& location = LocationData::NONE);
		ASTStringLiteral(ASTString const& raw);
		ASTStringLiteral(ASTStringLiteral const& base);
		ASTStringLiteral& operator=(ASTStringLiteral const& rhs);
		ASTStringLiteral* clone() const {return new ASTStringLiteral(*this);}

		void execute (ASTVisitor& visitor, void* param = NULL);
		bool isStringLiteral() const {return true;}

		bool isConstant() const {return true;}

		// The data declaration that this literal may be part of. If NULL that
		// means this is not part of a data declaration. This should be managed by
		// that declaration and not modified by this object at all.
		ASTDataDecl* declaration;

		std::string value;
	};

	class ASTArrayLiteral : public ASTLiteral
	{
	public:
		ASTArrayLiteral(LocationData const& location = LocationData::NONE);
		ASTArrayLiteral(ASTArrayLiteral const& base);
		~ASTArrayLiteral();
		ASTArrayLiteral& operator=(ASTArrayLiteral const& rhs);
		ASTArrayLiteral* clone() const {return new ASTArrayLiteral(*this);}

		void execute (ASTVisitor& visitor, void* param = NULL);
		bool isArrayLiteral() const {return true;}

		bool isConstant() const {return true;}

		DataTypeArray const* getReadType() const {return iReadType;}
		void setReadType(DataTypeArray const* type) {iReadType = type;}

		// The data declaration that this literal may be part of. If NULL that
		// means this is not part of a data declaration. This should be managed by
		// that declaration and not modified by this object at all.
		ASTDataDecl* declaration;

		// Optional type specification.
		ASTDataType* type;
		// Optional size specification.
		ASTExpr* size;
		// The array elements.
		std::vector<ASTExpr*> elements;

	private:
		// Cached read type.
		DataTypeArray const* iReadType;
	};

	class ASTOptionValue : public ASTLiteral
	{
	public:
		ASTOptionValue(std::string const& name = "",
		               LocationData const& location = LocationData::NONE);
		ASTOptionValue(ASTOptionValue const& base);
		ASTOptionValue& operator=(ASTOptionValue const& base);
		ASTOptionValue* clone() const {return new ASTOptionValue(*this);}

		virtual void execute(ASTVisitor& visitor, void* param = NULL);
		virtual std::string asString() const;

		virtual bool isConstant() const {return true;}

		virtual DataType const* getReadType() const {
			return &DataType::FLOAT;}

		std::string name;
		CompileOption option;
		optional<long> value;
	};

	// Types

	class ASTScriptType : public AST
	{
	public:
		ASTScriptType(
				ScriptType type = ScriptType(),
				LocationData const& location = LocationData::NONE);
		ASTScriptType(ASTScriptType const& base);
		ASTScriptType& operator=(ASTScriptType const& rhs);
		ASTScriptType* clone() const {return new ASTScriptType(*this);}

		void execute(ASTVisitor& visitor, void* param = NULL);

		ScriptType type;
	};

	class ASTDataType : public AST
	{
	public:
		// Takes ownership of type.
		ASTDataType(
				DataType* type = NULL,
				LocationData const& location = LocationData::NONE);
		// Clones type.
		ASTDataType(
				DataType const& type,
				LocationData const& location = LocationData::NONE);
		ASTDataType(ASTDataType const& base);
		~ASTDataType() {delete type;}
		ASTDataType& operator=(ASTDataType const& rhs);
		ASTDataType* clone() const {return new ASTDataType(*this);}
	
		void execute(ASTVisitor& visitor, void* param = NULL);

		DataType const& resolve(Scope& scope);

		// Owned by this object.
		DataType* type;
	};
}

#endif

