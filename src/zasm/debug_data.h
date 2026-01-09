#ifndef ZASM_DEBUG_DATA_H_
#define ZASM_DEBUG_DATA_H_

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/ints.h"
#include "zasm/pc.h"

using TypeID = uint32_t;

struct SourceFile
{
	std::string path;
	std::string contents;
};

enum DebugScopeTag : uint8_t
{
	TAG_UNKNOWN,
	TAG_ROOT,
	TAG_FILE,
	TAG_NAMESPACE,
	TAG_SCRIPT,
	TAG_FUNCTION,
	TAG_CLASS,
	TAG_ENUM,
	TAG_BLOCK,
};

enum DebugScopeFlags : uint8_t
{
	SCOPE_FLAG_NONE = 0,
	SCOPE_FLAG_HIDDEN = 1,
	SCOPE_FLAG_INTERNAL = 2,
	SCOPE_FLAG_DEPRECATED = 4,
};

struct DebugScope
{
	DebugScopeTag tag;
	DebugScopeFlags flags;
	int parent_index = -1;
	int inheritance_index = -1;
	TypeID type_id;
	pc_t start_pc;
	pc_t end_pc;
	std::string name;
	std::vector<int> imports;
};

enum DebugSymbolStorage : uint8_t
{
	CONSTANT,
	LOC_STACK,
	LOC_GLOBAL,
	LOC_REGISTER,
	LOC_CLASS,
};

enum DebugSymbolFlags : uint8_t
{
	SYM_FLAG_NONE = 0,
	SYM_FLAG_HIDDEN = 1,
	SYM_FLAG_VARARGS = 2,
};

struct DebugSymbol
{
	int32_t scope_index; // Which scope owns this variable?
	int32_t offset;      // Based on storage. e.g., stack offset, or global index, or class field, ...
	TypeID type_id;
	uint32_t declaration_line;
	DebugSymbolStorage storage;
	DebugSymbolFlags flags;
	std::string name;
};

const int DEBUG_TYPE_TAG_TABLE_START = 32;

enum DebugTypeTag : uint8_t
{
	// 0-31 Reserved for Primitives (Implicit, not stored in types table).
	TYPE_VOID,
	// Note: only internal functions use TYPE_TEMPLATE_UNBOUNDED.
	// For user template functions, each function is emitted with the resolved types with no
	// indication it was a templated function.
	// I attempted to create a proper scoped template type, but our implementation of template types
	// made that difficult.
	// See: https://pastebin.com/shiWMdDu
	TYPE_TEMPLATE_UNBOUNDED,
	TYPE_UNTYPED,
	TYPE_BOOL,
	TYPE_INT,
	TYPE_LONG,
	TYPE_CHAR32,
	TYPE_RGB,

	// 32+ Stored in types table.
	TYPE_CONST    = 32, // Modifies another type.
	TYPE_ARRAY    = 33, // Array of another type.
	TYPE_CLASS    = 34, // Points to a TAG_CLASS scope.
	TYPE_ENUM     = 35, // Points to a TAG_ENUM scope.
	TYPE_BITFLAGS = 36, // Points to a TAG_ENUM scope.
};

struct DebugData;

struct DebugType
{
	DebugTypeTag tag;
	// Either a type id or a scope index - depends on tag.
	int32_t extra;

	const DebugType* asNonConst(const DebugData& debug_data) const;

	bool isVoid(const DebugData& debug_data) const;
	bool isUntyped(const DebugData& debug_data) const;
	bool isFixed(const DebugData& debug_data) const;
	bool isLong(const DebugData& debug_data) const;
	bool isBool(const DebugData& debug_data) const;
	bool isEnum(const DebugData& debug_data) const;
	bool isString(const DebugData& debug_data) const;
	bool isArray(const DebugData& debug_data) const;
	bool isClass(const DebugData& debug_data) const;
};

extern DebugType BasicTypes[8];

struct DebugLine
{
	pc_t pc;
	int32_t file_index;
	int32_t line_number;
	bool is_prologue_end;
};

struct DebugData
{
	static const int VERSION = 1;
	static const int DEBUG_LINE_OP_SIMPLE_STEP_MAX = 0xEF;
	static const int DEBUG_LINE_OP_SET_FILE = 0xF0;
	static const int DEBUG_LINE_OP_EXTENDED_STEP = 0xF1;
	static const int DEBUG_LINE_OP_PROLOGUE_END = 0xF2;

	static std::optional<DebugData> decode(const std::vector<byte>& buffer);

	std::vector<SourceFile> source_files;
	std::vector<byte> debug_lines_encoded;
	std::vector<DebugScope> scopes;
	std::vector<DebugSymbol> symbols;
	std::vector<DebugType> types;

	void appendLineInfoSetFile(int file);
	void appendLineInfoSimpleStep(byte d_pc);
	void appendLineInfoExtendedStep(int d_pc, int d_line);
	void appendLineInfoPrologueEnd();

	bool exists() const;

	int getScopeIndex(const DebugScope* scope) const;
	TypeID getTypeID(const DebugType* type) const;
	int getSourceFileIndex(const SourceFile* source_file) const;
	const SourceFile* getSourceFile(std::string path) const;
	const std::vector<DebugLine>& getLineTable() const;

	std::pair<const char*, int> resolveLocation(pc_t pc) const;
	std::pair<const SourceFile*, int> resolveLocationSourceFile(pc_t pc) const;
	std::vector<pc_t> resolveAllPcsFromSourceLocation(const SourceFile* source_file, int32_t line) const;
	const DebugScope* resolveScope(pc_t pc) const;
	const DebugScope* resolveFunctionScope(pc_t pc) const;
	const DebugScope* resolveClassScope(pc_t pc) const;
	const DebugScope* resolveFileScope(std::string fname) const;
	pc_t findFunctionPrologueEnd(const DebugScope* scope) const;
	const DebugType* getType(TypeID type_id) const;
	const DebugType* getType(const DebugScope* scope) const;
	const DebugType* getTypeForScope(const DebugScope* scope) const;
	std::string getTypeName(const DebugType* type) const;
	std::string getTypeName(TypeID type_id) const;
	std::pair<const SourceFile*, int> getSymbolLocation(const DebugSymbol* scope) const;
	std::string getFullSymbolName(const DebugSymbol* symbol) const;
	std::string getFullScopeName(const DebugScope* scope) const;
	std::string getFunctionSignature(const DebugScope* scope) const;
	uint32_t getFunctionAdditionalStackSize(const DebugScope* scope) const;
	std::string getDebugSymbolName(const DebugSymbol* symbol) const;
	std::vector<const DebugSymbol*> getChildSymbols(const DebugScope* scope) const;
	std::vector<const DebugScope*> getChildScopes(const DebugScope* scope) const;

	// Returns the DebugSymbol (variable/function) if found, else nullptr.
    const DebugSymbol* resolveSymbol(const std::string& identifier, const DebugScope* current_scope) const;
    // Returns the DebugScope (class/namespace) if found, else nullptr.
    const DebugScope* resolveScope(const std::string& identifier, const DebugScope* current_scope) const;
	std::vector<const DebugScope*> resolveFunctions(const std::string& identifier, const DebugScope* current_scope) const;

	bool canCoerceTypes(const DebugType* type_1, const DebugType* type_2) const;

	std::vector<byte> encode() const;
	std::string internalToStringForDebugging() const;

private:
	struct ResolveResult
	{
        const DebugSymbol* sym = nullptr;
        int32_t scope_idx = -1;
    };

    ResolveResult resolveEntity(const std::string& identifier, const DebugScope* current_scope) const;

	struct CacheEntry
	{
		pc_t pc = -1; // -1 indicates empty
		std::pair<const SourceFile*, int> result;
	};

	mutable std::vector<DebugLine> line_table_cache;
	mutable bool line_table_built = false;
	void buildLineTable() const;

	mutable std::array<CacheEntry, 1024> resolve_location_cache;

	mutable std::vector<int32_t> sorted_scopes;
	mutable bool scopes_sorted = false;
	void buildScopesSorted() const;

	void buildSymbolCache() const;
	mutable std::vector<std::vector<const DebugSymbol*>> scope_symbol_cache;
	mutable bool scope_symbol_cache_built;

	void buildScopeChildrenCache() const;
	mutable std::vector<std::vector<int32_t>> scope_children_cache;
	mutable bool scope_children_cache_built;
};

#endif
