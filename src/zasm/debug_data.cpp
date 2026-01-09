// TODO ! compiler changes
// TODO ! handle run_types (see ScriptsData::fillFromAssembler)
// TODO ! handle saves
// TODO ! handle compat

#include "zasm/debug_data.h"
#include "base/check.h"
#include "base/ints.h"
#include "zasm/pc.h"
#include "zasm/table.h"

#include <algorithm>
#include <cstdint>
#include <fmt/ranges.h>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{

// Write a signed integer using Variable Length Quantity (ZigZag).
void write_signed_vlq(std::vector<byte>& buf, int32_t value)
{
	uint32_t u_val = (value << 1) ^ (value >> 31);
	do {
		byte byte = u_val & 0x7F;
		u_val >>= 7;
		if (u_val != 0) byte |= 0x80;
		buf.push_back(byte);
	} while (u_val != 0);
}

// Write unsigned VLQ.
void write_unsigned_vlq(std::vector<byte>& buf, uint32_t value)
{
	do {
		byte byte = value & 0x7F;
		value >>= 7;
		if (value != 0) byte |= 0x80;
		buf.push_back(byte);
	} while (value != 0);
}

uint32_t read_uvlq_from_buffer(const std::vector<byte>& buffer, size_t& cursor)
{
	uint32_t result = 0;
	int shift = 0;
	while (cursor < buffer.size())
	{
		byte b = buffer[cursor++];
		result |= (b & 0x7F) << shift;
		if (!(b & 0x80)) return result;
		shift += 7;
	}

	return result;
}

int32_t read_svlq_from_buffer(const std::vector<byte>& buf, size_t& cursor)
{
	uint32_t u_val = read_uvlq_from_buffer(buf, cursor);
	return (u_val >> 1) ^ -(int32_t)(u_val & 1);
}

std::vector<std::string_view> split_identifier(std::string_view str)
{
	std::vector<std::string_view> tokens;
	tokens.reserve(5);

	size_t start = 0;
	size_t end = str.find("::");

	while (end != std::string_view::npos)
	{
		tokens.push_back(str.substr(start, end - start));
		start = end + 2;
		end = str.find("::", start);
	}

	tokens.push_back(str.substr(start));
	return tokens;
}

} // end namespace

const DebugType* DebugType::asNonConst(const DebugData& debug_data) const
{
	if (tag == TYPE_CONST) return debug_data.getType(extra);
	return this;
}

bool DebugType::isVoid(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_VOID;
}

bool DebugType::isUntyped(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_UNTYPED;
}

bool DebugType::isFixed(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_INT;
}

bool DebugType::isLong(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_LONG;
}

bool DebugType::isBool(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_BOOL;
}

bool DebugType::isEnum(const DebugData& debug_data) const
{
	auto tag = asNonConst(debug_data)->tag;
	return tag == TYPE_ENUM || tag == TYPE_BITFLAGS;
}

bool DebugType::isString(const DebugData& debug_data) const
{
	auto type = asNonConst(debug_data);
	return type->tag == TYPE_ARRAY && type->extra == TYPE_CHAR32;
}

bool DebugType::isArray(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_ARRAY;
}

bool DebugType::isClass(const DebugData& debug_data) const
{
	return asNonConst(debug_data)->tag == TYPE_CLASS;
}

void DebugData::appendLineInfoSetFile(int file)
{
	debug_lines_encoded.push_back(DEBUG_LINE_OP_SET_FILE);
	write_unsigned_vlq(debug_lines_encoded, file);
}

void DebugData::appendLineInfoSimpleStep(byte d_pc)
{
	debug_lines_encoded.push_back(d_pc);
}

void DebugData::appendLineInfoExtendedStep(int d_pc, int d_line)
{
	debug_lines_encoded.push_back(DEBUG_LINE_OP_EXTENDED_STEP);
	write_unsigned_vlq(debug_lines_encoded, d_pc);
	write_signed_vlq(debug_lines_encoded, d_line);
}

void DebugData::appendLineInfoPrologueEnd()
{
	debug_lines_encoded.push_back(DEBUG_LINE_OP_PROLOGUE_END);
}

bool DebugData::exists() const
{
	return !source_files.empty();
}

int DebugData::getScopeIndex(const DebugScope* scope) const
{
	return (int32_t)(scope - &scopes[0]);
}

TypeID DebugData::getTypeID(const DebugType* type) const
{
	switch (type->tag)
	{
		case TYPE_VOID:
		case TYPE_UNTYPED:
		case TYPE_TEMPLATE_UNBOUNDED:
		case TYPE_BOOL:
		case TYPE_INT:
		case TYPE_LONG:
		case TYPE_CHAR32:
		case TYPE_RGB:
			return type->tag;
	}

	return (int32_t)(type - &types[0]) + DEBUG_TYPE_TAG_TABLE_START;
}

int DebugData::getSourceFileIndex(const SourceFile* source_file) const
{
	return (int32_t)(source_file - &source_files[0]);
}

const SourceFile* DebugData::getSourceFile(std::string path) const
{
	for (auto& file : source_files)
	{
		if (file.path == path)
			return &file;
	}

	return nullptr;
}

const std::vector<DebugLine>& DebugData::getLineTable() const
{
	buildLineTable();

	return line_table_cache;
}

std::pair<const char*, int> DebugData::resolveLocation(pc_t pc) const
{
	if (debug_lines_encoded.empty()) return {};

	const auto& [source_file, line] = resolveLocationSourceFile(pc);
	return {source_file->path.data(), line};
}

std::pair<const SourceFile*, int> DebugData::resolveLocationSourceFile(pc_t pc) const
{
	if (debug_lines_encoded.empty()) return {};

	buildLineTable();

    // Find first element with pc > target.
    auto it = std::upper_bound(line_table_cache.begin(), line_table_cache.end(), pc, 
        [](pc_t val, const DebugLine& entry) {
            return val < entry.pc;
        });

    // If we are at the start, or list is empty
    if (it == line_table_cache.begin()) return {nullptr, 0};

    // The entry strictly before upper_bound is the one covering the target pc.
    const auto& entry = *(--it);

    if (entry.file_index >= 0 && entry.file_index < source_files.size())
        return { &source_files[entry.file_index], entry.line_number };

    return {nullptr, 0};
}

// This mostly returns 1 pc, but for templated functions it will return 1 per instance of the
// function (unless a specific instance optimized out the line).
std::vector<pc_t> DebugData::resolveAllPcsFromSourceLocation(const SourceFile* source_file, int32_t line) const
{
	buildLineTable();

	std::vector<pc_t> candidates;

	int32_t target_file_idx = -1;
	for (size_t i = 0; i < source_files.size(); i++)
	{
		if (&source_files[i] == source_file)
		{
			target_file_idx = (int32_t)i;
			break;
		}
	}

	if (target_file_idx == -1) return candidates;

	int32_t min_diff = INT32_MAX;

	for (const auto& entry : line_table_cache)
	{
		if (entry.file_index != target_file_idx) continue;
		if (entry.line_number < line) continue;

		int32_t diff = entry.line_number - line;
		if (diff < min_diff)
		{
			min_diff = diff;
			candidates.clear();
			candidates.push_back(entry.pc);
		}
		else if (diff == min_diff)
		{
			candidates.push_back(entry.pc);
		}
	}

	std::set<const DebugScope*> scopes;
	std::vector<pc_t> results;
	for (pc_t pc : candidates)
	{
		const DebugScope* scope = resolveFunctionScope(pc);
		if (scopes.contains(scope))
			continue;

		scopes.insert(scope);
		results.push_back(pc);
	}

	return results;
}

void DebugData::buildLineTable() const
{
	if (line_table_built) return;

	line_table_cache.clear();
	line_table_cache.reserve(debug_lines_encoded.size() / 2);

	size_t cursor = 0;
	int32_t current_line = 1;
	int32_t current_file = 0;
	pc_t current_pc = 0;
	bool next_is_prologue_end = false;

	while (cursor < debug_lines_encoded.size())
	{
		byte cmd = debug_lines_encoded[cursor++];

		if (cmd == DEBUG_LINE_OP_SET_FILE)
		{
			current_file = read_uvlq_from_buffer(debug_lines_encoded, cursor);
			continue; 
		}

		if (cmd == DEBUG_LINE_OP_PROLOGUE_END)
		{
			next_is_prologue_end = true;
			continue;
		}

		size_t d_pc;
		int32_t d_line;
		if (cmd == DEBUG_LINE_OP_EXTENDED_STEP)
		{
			d_pc = read_uvlq_from_buffer(debug_lines_encoded, cursor);
			d_line = read_svlq_from_buffer(debug_lines_encoded, cursor);
		}
		else
		{
			d_pc = cmd;
			d_line = 1;
		}

		if (d_pc > 0 || next_is_prologue_end)
		{
			DebugLine entry{};
			entry.pc = current_pc;
			entry.file_index = current_file;
			entry.line_number = current_line;
			entry.is_prologue_end = next_is_prologue_end;
			line_table_cache.push_back(entry);
			next_is_prologue_end = false;
		}

		current_pc += d_pc;
		current_line += d_line;
	}

	line_table_built = true;
}

void DebugData::buildScopesSorted() const
{
	if (scopes_sorted) return;

	sorted_scopes.clear();
	sorted_scopes.reserve(scopes.size());

	for (size_t i = 0; i < scopes.size(); ++i)
	{
		const auto& s = scopes[i];
		// Only index scopes that actually have code ranges (Function, Block, etc.)
		if (s.end_pc > s.start_pc) 
			sorted_scopes.push_back(i);
	}

	// Sort Key 1: Start PC (Ascending)
	// Sort Key 2: End PC (Descending) -> Ensures Outer scopes appear BEFORE Inner scopes
	//             when they share the same start address.
	std::sort(sorted_scopes.begin(), sorted_scopes.end(), 
		[&](int32_t a, int32_t b) {
			const auto& sa = scopes[a];
			const auto& sb = scopes[b];
			if (sa.start_pc != sb.start_pc)
				return sa.start_pc < sb.start_pc;
			return sa.end_pc > sb.end_pc; 
	});

	scopes_sorted = true;
}

const DebugScope* DebugData::resolveScope(pc_t pc) const
{
	if (!scopes_sorted) buildScopesSorted();

	// Find the first scope that starts strictly after the pc.
	// The scope we want must be before this point.
	auto it = std::upper_bound(sorted_scopes.begin(), sorted_scopes.end(), pc, 
		[&](pc_t val, int32_t idx) {
			return val < scopes[idx].start_pc;
		});

	// Since the list is sorted by start_pc, the scopes with the highest start_pc
	// (most specific) are closest to our iterator.
	while (it != sorted_scopes.begin())
	{
		--it;
		const auto& s = scopes[*it];
		if (pc <= s.end_pc)
			return &s;
	}

	return nullptr;
}

const DebugScope* DebugData::resolveFunctionScope(pc_t pc) const
{
	auto* scope = resolveScope(pc);
	if (!scope)
		return nullptr;

	while (scope->tag != TAG_FUNCTION)
	{
		if (scope->parent_index == -1)
			return nullptr;

		scope = &scopes[scope->parent_index];
	}

	return scope;
}

const DebugScope* DebugData::resolveClassScope(pc_t pc) const
{
	auto* scope = resolveScope(pc);
	if (!scope)
		return nullptr;

	while (scope->tag != TAG_CLASS)
	{
		if (scope->parent_index == -1)
			return nullptr;

		scope = &scopes[scope->parent_index];
	}

	return scope;
}

const DebugScope* DebugData::resolveFileScope(std::string fname) const
{
	for (const auto& s : scopes)
	{
		if (s.tag == TAG_FILE && s.name == fname)
			return &s;
	}

	return nullptr;
}

pc_t DebugData::findFunctionPrologueEnd(const DebugScope* scope) const
{
	DCHECK(scope && scope->tag == TAG_FUNCTION);
	buildLineTable();

	auto it = std::lower_bound(line_table_cache.begin(), line_table_cache.end(), scope->start_pc,
		[](const DebugLine& entry, pc_t val) {
			return entry.pc < val;
		});

	for (; it != line_table_cache.end(); ++it)
	{
		if (it->pc >= scope->end_pc)
			break;

		if (it->is_prologue_end)
			return it->pc;
	}

	return scope->start_pc;
}

DebugType BasicTypes[] = {
	DebugType{TYPE_VOID, TYPE_VOID},
	DebugType{TYPE_TEMPLATE_UNBOUNDED, TYPE_TEMPLATE_UNBOUNDED},
	DebugType{TYPE_UNTYPED, TYPE_UNTYPED},
	DebugType{TYPE_BOOL, TYPE_BOOL},
	DebugType{TYPE_INT, TYPE_INT},
	DebugType{TYPE_LONG, TYPE_LONG},
	DebugType{TYPE_CHAR32, TYPE_CHAR32},
	DebugType{TYPE_RGB, TYPE_RGB},
};

const DebugType* DebugData::getType(TypeID type_id) const
{
	if (type_id <= TYPE_RGB) return &BasicTypes[type_id];

	int32_t table_idx = type_id - DEBUG_TYPE_TAG_TABLE_START;
	if (table_idx >= types.size()) return nullptr;

	return &types[table_idx];
}

const DebugType* DebugData::getType(const DebugScope* scope) const
{
	int scope_index = getScopeIndex(scope);

	for (auto& type : types)
	{
		if (!(type.tag == TYPE_CLASS || type.tag == TYPE_ENUM || type.tag == TYPE_BITFLAGS))
			continue;

		if (type.extra == scope_index)
			return &type;
	}

	return nullptr;
}

const DebugType* DebugData::getTypeForScope(const DebugScope* scope) const
{
	int scope_index = getScopeIndex(scope);

	for (int i = 0; i < types.size(); i++)
	{
		auto& type = types[i];
		if ((type.tag == TYPE_CLASS || type.tag == TYPE_ENUM || type.tag == TYPE_BITFLAGS) && type.extra == scope_index)
			return &type;
	}

	return 0;
}

std::string DebugData::getTypeName(const DebugType* type) const
{
	if (!type)
		return "unknown";

	switch (type->tag)
	{
		case TYPE_VOID: return "void";
		case TYPE_UNTYPED: return "untyped";
		case TYPE_TEMPLATE_UNBOUNDED: return "T";
		case TYPE_BOOL: return "bool";
		case TYPE_INT: return "int";
		case TYPE_LONG: return "long";
		case TYPE_CHAR32: return "char32";
		case TYPE_RGB: return "rgb";
		case TYPE_CONST: return "const " + getTypeName(getType(type->extra));
		case TYPE_ARRAY: return getTypeName(getType(type->extra)) + "[]";
		case TYPE_BITFLAGS:
		case TYPE_CLASS:
		case TYPE_ENUM:
			return scopes[type->extra].name;
	}
}

std::string DebugData::getTypeName(TypeID type_id) const
{
	return getTypeName(getType(type_id));
}

std::pair<const SourceFile*, int> DebugData::getSymbolLocation(const DebugSymbol* symbol) const
{
	if (!symbol->declaration_line)
		return {nullptr, 0};

	const DebugScope* scope = &scopes[symbol->scope_index];
	while (scope && scope->tag != TAG_FILE && scope->parent_index != -1)
		scope = &scopes[scope->parent_index];

	for (const auto& source_file : source_files)
	{
		if (source_file.path == scope->name)
			return {&source_file, symbol->declaration_line};
	}

	return {nullptr, 0};
}

std::string DebugData::getFullSymbolName(const DebugSymbol* symbol) const
{
	std::vector<std::string> parts = {symbol->name};

	const DebugScope* cur = &scopes[symbol->scope_index];
	while (cur)
	{
		if (cur->tag == TAG_FUNCTION || cur->tag == TAG_CLASS || cur->tag == TAG_SCRIPT || cur->tag == TAG_NAMESPACE)
			parts.push_back(cur->name.empty() ? "?" : cur->name);

		if (cur->parent_index != -1)
			cur = &scopes[cur->parent_index];
		else
			cur = nullptr;
	}

	std::reverse(parts.begin(), parts.end());

	return fmt::format("{}", fmt::join(parts, "::"));
}

std::string DebugData::getFullScopeName(const DebugScope* scope) const
{
	if (scope->tag == TAG_ROOT || scope->tag == TAG_FILE)
		return scope->name;

	std::vector<std::string> parts;

	const DebugScope* cur = scope;
	while (cur)
	{
		if (cur->tag == TAG_FUNCTION || cur->tag == TAG_CLASS || cur->tag == TAG_SCRIPT || cur->tag == TAG_NAMESPACE)
			parts.push_back(cur->name.empty() ? "?" : cur->name);

		if (cur->parent_index != -1)
			cur = &scopes[cur->parent_index];
		else
			cur = nullptr;
	}

	std::reverse(parts.begin(), parts.end());

	return fmt::format("{}", fmt::join(parts, "::"));
}

std::string DebugData::getFunctionSignature(const DebugScope* scope) const
{
	if (scope->tag != TAG_FUNCTION)
		return "";

	std::string name = getFullScopeName(scope);

	std::vector<std::string> params;
	for (auto symbol : getChildSymbols(scope))
	{
		if (symbol->flags & SYM_FLAG_HIDDEN)
			continue;

		bool varargs = symbol->flags & SYM_FLAG_VARARGS;
		std::string type_name = getTypeName(symbol->type_id);
		params.push_back(fmt::format("{}{} {}", varargs ? "..." : "", type_name, symbol->name));
	}

	std::string type_name = getTypeName(scope->type_id);
	return fmt::format("{} {}({})", type_name, name, fmt::join(params, ", "));
}

// Returns how much more the function will grow the stack in its prologue.
// Most functions have their parameters put onto the stack by the caller. Any other local data it
// needs is allocated at the start of the function.
uint32_t DebugData::getFunctionAdditionalStackSize(const DebugScope* scope) const
{
	uint32_t size = 0;

	// Run functions are special: they push their own parameters onto the stack.
	bool is_run = scope->name == "run" && scope->parent_index != -1 && scopes[scope->parent_index].tag == TAG_SCRIPT;
	if (!is_run)
	{
		buildScopeChildrenCache();
		int32_t s_idx = getScopeIndex(scope);
		auto& child_indices = scope_children_cache[s_idx];

		if (child_indices.empty())
			return 0;

		scope = &scopes[child_indices[0]];
	}

	// The largest stack offset (+1) is the additional stack size. No need to search all child
	// scopes - the top-most one will contain the highest stack offsets.
	auto symbols = getChildSymbols(scope);
	for (auto& symbol : symbols)
	{
		if (symbol->storage == LOC_STACK && symbol->offset + 1 > size)
			size = symbol->offset + 1;
	}

	return size;
}

std::string DebugData::getDebugSymbolName(const DebugSymbol* symbol) const
{
	auto format_storage = [](DebugSymbolStorage s, int32_t offset, TypeID type_id) -> std::string {
		switch(s) {
			case CONSTANT:
			{
				if (type_id == TYPE_INT)
					return fmt::format("Constant[{}]", offset / 10000);
				if (type_id == TYPE_LONG)
					return fmt::format("Constant[{}L]", offset);
				return fmt::format("Constant[{}]", offset);
			}
			case LOC_STACK:    return fmt::format("Stack[{}]", offset);
			case LOC_GLOBAL:   return fmt::format("Global[{}]", offset);
			case LOC_REGISTER: return fmt::format("Reg[{}]", get_script_variable(offset).first->name);
			case LOC_CLASS:    return fmt::format("ClassField[{}]", offset);
			default:           return fmt::format("Unknown[{}]", offset);
		}
	};

	std::string extra_info;
	if (symbol->flags & SYM_FLAG_HIDDEN)
		extra_info += " <HIDDEN>";
	if (symbol->flags & SYM_FLAG_VARARGS)
		extra_info += " <VARARG>";

	return fmt::format("{} {} @ {}{}",
		getTypeName(symbol->type_id),
		symbol->name,
		format_storage(symbol->storage, symbol->offset, symbol->type_id),
		extra_info);
}

void DebugData::buildSymbolCache() const
{
	if (scope_symbol_cache_built) return;
	scope_symbol_cache.resize(scopes.size());
	for (const auto& sym : symbols) {
		if (sym.scope_index >= 0 && sym.scope_index < scopes.size()) {
			scope_symbol_cache[sym.scope_index].push_back(&sym);
		}
	}
	scope_symbol_cache_built = true;
}

std::vector<const DebugSymbol*> DebugData::getChildSymbols(const DebugScope* scope) const
{
	buildSymbolCache();

	return scope_symbol_cache[getScopeIndex(scope)];
}

std::vector<const DebugScope*> DebugData::getChildScopes(const DebugScope* scope) const
{
	buildScopeChildrenCache();

	int32_t s_idx = getScopeIndex(scope);
	auto& child_indices = scope_children_cache[s_idx];

	std::vector<const DebugScope*> child_scopes;
	for (auto index : child_indices)
		child_scopes.push_back(&scopes[index]);
	return child_scopes;
}

void DebugData::buildScopeChildrenCache() const
{
	if (scope_children_cache_built)
		return;

	scope_children_cache.resize(scopes.size());

	for (int i = 0; i < scopes.size(); i++)
	{
		int p = scopes[i].parent_index;
		if (p >= 0 && p < scopes.size())
			scope_children_cache[p].push_back(i);
	}

	scope_children_cache_built = true;
}

DebugData::ResolveResult DebugData::resolveEntity(const std::string& identifier, const DebugScope* current_scope) const
{
	buildSymbolCache();
	buildScopeChildrenCache();

	auto tokens = split_identifier(identifier);
	if (tokens.empty()) return {};

	ResolveResult current_res;
	int32_t ctx_scope_idx = current_scope ? getScopeIndex(current_scope) : -1;

	// Only used within find_member, but declared here to avoid allocating a ton.
	std::vector<int32_t> search_queue;
	std::vector<int32_t> visited;
	search_queue.reserve(16);
	visited.reserve(16);

	// Resolve symbols and scopes within a specific scope.
	auto find_member = [&](int32_t parent_idx, std::string_view name) -> ResolveResult 
	{
		ResolveResult res;

		search_queue.clear();
		visited.clear();
		search_queue.push_back(parent_idx);

		size_t head = 0;
		while(head < search_queue.size())
		{
			int32_t s_idx = search_queue[head++];

			bool already_visited = false;
			for (int32_t v : visited)
			{
				if (v == s_idx)
				{
					already_visited = true;
					break;
				}
			}
			if (already_visited) continue;
			visited.push_back(s_idx);

			// 1. Check Symbols
			if (s_idx < scope_symbol_cache.size()) {
				for (const auto* sym : scope_symbol_cache[s_idx]) {
					if (sym->name == name) {
						res.sym = sym;
						break;
					}
				}
			}

			// 2. Check Child Scopes
			if (s_idx < scope_children_cache.size()) {
				for (int32_t child_idx : scope_children_cache[s_idx]) {
					const auto& child = scopes[child_idx];
					if (child.tag == TAG_FILE) continue;

					if (child.name == name) {
						res.scope_idx = child_idx;
						break;
					}
				}
			}

			// If we found ANYTHING at this level (Symbol OR Scope), return.
			// This ensures we respect shadowing (Derived matches hide Base matches).
			if (res.sym || res.scope_idx != -1)
				return res;

			// Check inheritance (base class).
			if (scopes[s_idx].tag == TAG_CLASS && scopes[s_idx].inheritance_index != -1)
				search_queue.push_back(scopes[s_idx].inheritance_index);
		}
		return res;
	};

	// =========================================================
	// Phase 1: Context Walk (Start Point)
	// =========================================================
	int token_idx = 0;

	if (tokens[0].empty())
	{
		// "::Global" syntax not supported.
		return {};
	}
	else
	{
		// Walk up parents + imports.
		const DebugScope* walker = (ctx_scope_idx >= 0) ? &scopes[ctx_scope_idx] : nullptr;
		bool found = false;

		while (walker)
		{
			int32_t s_idx = getScopeIndex(walker);
			
			// Collect scan targets: Self + Imports
			std::vector<int32_t> lookups = { s_idx };
			lookups.insert(lookups.end(), walker->imports.begin(), walker->imports.end());

			for (int32_t lookup_idx : lookups) 
			{
				current_res = find_member(lookup_idx, tokens[0]);
				if (current_res.sym || current_res.scope_idx != -1)
				{
					found = true;
					break;
				}
			}

			if (found)
				break;

			if (walker->parent_index != -1)
				walker = &scopes[walker->parent_index];
			else 
				walker = nullptr;
		}

		if (!found)
			return {};

		token_idx++;
	}

	// =========================================================
	// Phase 2: Drill Down
	// =========================================================
	while (token_idx < tokens.size())
	{
		if (current_res.scope_idx == -1)
			return {};

		current_res = find_member(current_res.scope_idx, tokens[token_idx]);
		if (!current_res.sym && current_res.scope_idx == -1)
			return {};

		token_idx++;
	}

	return current_res;
}

const DebugSymbol* DebugData::resolveSymbol(const std::string& identifier, const DebugScope* current_scope) const
{
	ResolveResult res = resolveEntity(identifier, current_scope);
	return res.sym;
}

const DebugScope* DebugData::resolveScope(const std::string& identifier, const DebugScope* current_scope) const
{
	ResolveResult res = resolveEntity(identifier, current_scope);
	if (res.scope_idx != -1)
		return &scopes[res.scope_idx];
	return nullptr;
}

std::vector<const DebugScope*> DebugData::resolveFunctions(const std::string& identifier, const DebugScope* current_scope) const
{
	buildSymbolCache();
	buildScopeChildrenCache();

	auto tokens = split_identifier(identifier);
	if (tokens.empty()) return {};

	std::vector<const DebugScope*> candidates;
	
	// Helper: Collect ALL function scopes with 'name' inside 'parent_idx'
	// (including base classes)
	auto collect_functions_recursive = [&](int32_t start_idx, std::string_view name) 
	{
		std::vector<int32_t> search_queue = { start_idx };
		std::unordered_set<int32_t> visited;

		size_t head = 0;
		while(head < search_queue.size())
		{
			int32_t s_idx = search_queue[head++];
			if (visited.count(s_idx)) continue;
			visited.insert(s_idx);

			// Check Child Scopes for FUNCTIONS
			if (s_idx < scope_children_cache.size()) {
				for (int32_t child_idx : scope_children_cache[s_idx]) {
					const auto& child = scopes[child_idx];
					
					// We only care about matching names that are FUNCTION scopes
					if (child.name == name && child.tag == TAG_FUNCTION) {
						candidates.push_back(&child);
					}
				}
			}

			// Check Inheritance (Base Class)
			if (scopes[s_idx].tag == TAG_CLASS && scopes[s_idx].inheritance_index != -1)
				search_queue.push_back(scopes[s_idx].inheritance_index);
		}
	};

	// =========================================================
	// Logic: Walk up until we find ANY match, then stop ascending
	// =========================================================
	
	// 1. If the identifier is simple ("MyFunc"), we walk up the stack.
	if (tokens.size() == 1)
	{
		const DebugScope* walker = current_scope;
		int32_t ctx_idx = current_scope ? getScopeIndex(current_scope) : -1;

		while (ctx_idx != -1)
		{
			const auto& scope = scopes[ctx_idx];
			size_t count_before = candidates.size();

			// A. Check Self
			collect_functions_recursive(ctx_idx, tokens[0]);

			// B. Check Imports (using namespace, etc)
			for (int32_t import_idx : scope.imports) {
				collect_functions_recursive(import_idx, tokens[0]);
			}

			// STOPPING CONDITION:
			// Standard C++ rules: if we found candidates in this scope (or its imports),
			// we stop walking up. We don't merge "local functions" with "global functions".
			// The local ones shadow the globals.
			if (candidates.size() > count_before)
				return candidates;

			ctx_idx = scope.parent_index;
		}
		return candidates; // Might be empty if nothing found anywhere
	}
	
	// 2. If the identifier is complex ("MyClass::MyFunc"), we resolve the parent first.
	else
	{
		// A. Resolve the container (e.g., "MyClass") using standard rules
		// We use the existing resolveScope to find the container.
		// We construct the parent path by removing the last token.
		std::string parent_path = identifier.substr(0, identifier.rfind(tokens.back()) - 2); 
		// (Note: simple string logic above implies '::' separator length is 2. 
		//  Better to join tokens[0]...tokens[n-1])

		const DebugScope* container = resolveScope(parent_path, current_scope);
		if (!container) return {};

		int32_t container_idx = getScopeIndex(container);
		
		// B. Collect functions inside that container
		collect_functions_recursive(container_idx, tokens.back());
		
		return candidates;
	}
}

bool DebugData::canCoerceTypes(const DebugType* type_1, const DebugType* type_2) const
{
	if (type_1 == type_2) return true;

	const DebugType* t1 = type_1->asNonConst(*this);
	const DebugType* t2 = type_2->asNonConst(*this);

	// If the function expects 'untyped', it accepts anything (int, float, array, etc).
	if (t2->tag == TYPE_UNTYPED)
		return true;

	if (t1->tag == TYPE_UNTYPED)
		return true;

	// Array covariance.
	if (t1->tag == TYPE_ARRAY && t2->tag == TYPE_ARRAY)
	{
		// Recursively check if the element type of T1 can convert to the element type of T2.
		// e.g. int[] -> untyped[]  requires  int -> untyped.
		const DebugType* elem_t1 = getType(t1->extra);
		const DebugType* elem_t2 = getType(t2->extra);
		
		return canCoerceTypes(elem_t1, elem_t2);
	}

	// Numeric promotions.
	bool t1_is_numeric = t1->tag == TYPE_INT || t1->tag == TYPE_LONG || t1->tag == TYPE_BOOL;
	bool t2_is_numeric = t2->tag == TYPE_INT || t2->tag == TYPE_LONG || t2->tag == TYPE_BOOL;

	if (t1_is_numeric && t2_is_numeric)
		return true;

	// Enum -> numeric.
	if ((t1->tag == TYPE_ENUM || t1->tag == TYPE_BITFLAGS) && t2_is_numeric)
		return true;

	// Class inheritance.
	if (t1->tag == TYPE_CLASS && t2->tag == TYPE_CLASS)
	{
		int32_t s1_idx = t1->extra; 
		int32_t s2_idx = t2->extra; 

		int safety = 0;
		while (s1_idx != -1 && safety++ < 100)
		{
			if (s1_idx == s2_idx) return true;
			if (s1_idx >= scopes.size()) break;
			s1_idx = scopes[s1_idx].inheritance_index;
		}
		return false;
	}

	return false;
}

std::optional<DebugData> DebugData::decode(const std::vector<byte>& buffer)
{
	if (buffer.empty())
		return std::nullopt;

	DebugData result{};
	size_t cursor = 0;

	uint32_t version = read_uvlq_from_buffer(buffer, cursor);
	if (version != 1)
		return std::nullopt;

	// Source files.
	uint32_t file_count = read_uvlq_from_buffer(buffer, cursor);
	result.source_files.reserve(file_count);
	for (uint32_t i = 0; i < file_count; ++i)
	{
		SourceFile file;

		// Path.
		uint32_t path_len = read_uvlq_from_buffer(buffer, cursor);
		if (cursor + path_len > buffer.size()) return std::nullopt;
		file.path.assign(reinterpret_cast<const char*>(&buffer[cursor]), path_len);
		cursor += path_len;

		// Contents.
		uint32_t contents_len = read_uvlq_from_buffer(buffer, cursor);
		if (cursor + contents_len > buffer.size()) return std::nullopt;
		file.contents.assign(reinterpret_cast<const char*>(&buffer[cursor]), contents_len);
		cursor += contents_len;

		result.source_files.push_back(std::move(file));
	}

	// Debug lines (encoded ops).
	uint32_t debug_lines_size = read_uvlq_from_buffer(buffer, cursor);
	if (buffer.size() - cursor < debug_lines_size)
		return std::nullopt;

	result.debug_lines_encoded.assign(buffer.begin() + cursor, buffer.begin() + cursor + debug_lines_size);
	cursor += debug_lines_size;

	// Types.
	uint32_t count = read_uvlq_from_buffer(buffer, cursor);
	result.types.resize(count);
	for (uint32_t i = 0; i < count; i++)
	{
		result.types[i].tag = (DebugTypeTag)read_uvlq_from_buffer(buffer, cursor);
		result.types[i].extra = read_svlq_from_buffer(buffer, cursor);
	}

	// Scopes.
	count = read_uvlq_from_buffer(buffer, cursor);
	result.scopes.reserve(count);
	for (uint32_t i = 0; i < count; i++)
	{
		DebugScope scope{};
		scope.tag = (DebugScopeTag)read_uvlq_from_buffer(buffer, cursor);
		scope.flags = (DebugScopeFlags)read_uvlq_from_buffer(buffer, cursor);
		scope.parent_index = read_svlq_from_buffer(buffer, cursor);

		if (scope.tag == TAG_CLASS)
			scope.inheritance_index = read_svlq_from_buffer(buffer, cursor);

		uint32_t imported_scopes_len = read_uvlq_from_buffer(buffer, cursor);
		if (cursor + imported_scopes_len > buffer.size()) return std::nullopt;
		for (uint32_t j = 0; j < imported_scopes_len; j++)
			scope.imports.push_back(read_uvlq_from_buffer(buffer, cursor));

		if (scope.tag == TAG_BLOCK || scope.tag == TAG_FUNCTION)
		{
			scope.start_pc = read_uvlq_from_buffer(buffer, cursor);
			scope.end_pc = read_uvlq_from_buffer(buffer, cursor);
		}

		if (scope.tag != TAG_BLOCK)
		{
			uint32_t name_len = read_uvlq_from_buffer(buffer, cursor);
			if (cursor + name_len > buffer.size()) return std::nullopt;
			scope.name.assign((const char*)&buffer[cursor], name_len);
			cursor += name_len;

			scope.type_id = read_uvlq_from_buffer(buffer, cursor);
		}

		result.scopes.push_back(std::move(scope));
	}

	// Symbols.
	count = read_uvlq_from_buffer(buffer, cursor);
	result.symbols.reserve(count);
	for (uint32_t i = 0; i < count; i++)
	{
		DebugSymbol symbol{};
		symbol.scope_index = read_svlq_from_buffer(buffer, cursor);
		symbol.offset = read_svlq_from_buffer(buffer, cursor); // Signed!
		symbol.type_id = read_uvlq_from_buffer(buffer, cursor);
		symbol.flags = (DebugSymbolFlags)read_uvlq_from_buffer(buffer, cursor);
		symbol.storage = (DebugSymbolStorage)read_uvlq_from_buffer(buffer, cursor);
		symbol.declaration_line = read_uvlq_from_buffer(buffer, cursor);

		uint32_t name_len = read_uvlq_from_buffer(buffer, cursor);
		if (cursor + name_len > buffer.size()) return std::nullopt;
		symbol.name.assign((const char*)&buffer[cursor], name_len);
		cursor += name_len;

		result.symbols.push_back(std::move(symbol));
	}

	return result;
}

std::vector<byte> DebugData::encode() const
{
	std::vector<byte> buffer;
	buffer.reserve(debug_lines_encoded.size());

	// Version.
	write_unsigned_vlq(buffer, VERSION);

	// Source files.
	write_unsigned_vlq(buffer, source_files.size());
	for (const auto& file : source_files)
	{
		// Path.
		write_unsigned_vlq(buffer, file.path.size());
		const byte* path_ptr = reinterpret_cast<const byte*>(file.path.data());
		buffer.insert(buffer.end(), path_ptr, path_ptr + file.path.size());

		// Contents.
		write_unsigned_vlq(buffer, file.contents.size());
		const byte* contents_ptr = reinterpret_cast<const byte*>(file.contents.data());
		buffer.insert(buffer.end(), contents_ptr, contents_ptr + file.contents.size());
	}

	// Debug lines (encoded ops).
	write_unsigned_vlq(buffer, debug_lines_encoded.size());
	buffer.insert(buffer.end(), debug_lines_encoded.begin(), debug_lines_encoded.end());

	// Types.
	write_unsigned_vlq(buffer, types.size());
	for (const auto& t : types)
	{
		write_unsigned_vlq(buffer, t.tag);
		write_signed_vlq(buffer, t.extra);
	}

	// Scopes.
	write_unsigned_vlq(buffer, scopes.size());
	for (const auto& scope : scopes)
	{
		write_unsigned_vlq(buffer, scope.tag);
		write_unsigned_vlq(buffer, scope.flags);
		write_signed_vlq(buffer, scope.parent_index);

		if (scope.tag == TAG_CLASS)
			write_signed_vlq(buffer, scope.inheritance_index);

		write_unsigned_vlq(buffer, scope.imports.size());
		for (const auto& index : scope.imports)
			write_unsigned_vlq(buffer, index);

		if (scope.tag == TAG_BLOCK || scope.tag == TAG_FUNCTION)
		{
			write_unsigned_vlq(buffer, scope.start_pc);
			write_unsigned_vlq(buffer, scope.end_pc);
		}

		if (scope.tag != TAG_BLOCK)
		{
			write_unsigned_vlq(buffer, scope.name.size());
			buffer.insert(buffer.end(), scope.name.begin(), scope.name.end());
	
			write_unsigned_vlq(buffer, scope.type_id);
		}
	}

	// Symbols.
	write_unsigned_vlq(buffer, symbols.size());
	for (const auto& symbol : symbols)
	{
		write_signed_vlq(buffer, symbol.scope_index);
		write_signed_vlq(buffer, symbol.offset);
		write_unsigned_vlq(buffer, symbol.type_id);
		write_unsigned_vlq(buffer, symbol.flags);
		write_unsigned_vlq(buffer, symbol.storage);
		write_unsigned_vlq(buffer, symbol.declaration_line);

		write_unsigned_vlq(buffer, symbol.name.size());
		buffer.insert(buffer.end(), symbol.name.begin(), symbol.name.end());
	}

	return buffer;
}

std::string DebugData::internalToStringForDebugging() const
{
	std::stringstream ss;

	// Build adjacency list.
	std::map<int32_t, std::vector<int32_t>> scope_children;
	std::vector<int32_t> root_indices;

	for (size_t i = 0; i < scopes.size(); ++i)
	{
		if (scopes[i].parent_index == -1)
			root_indices.push_back((int32_t)i);
		else
			scope_children[scopes[i].parent_index].push_back((int32_t)i);
	}

	// Map symbols to scopes.
	std::map<int32_t, std::vector<const DebugSymbol*>> scope_symbols;
	for (const auto& sym : symbols)
	{
		scope_symbols[sym.scope_index].push_back(&sym);
	}

	auto get_tag_name = [](DebugScopeTag t) -> const char* {
		switch(t) {
			case TAG_ROOT:       return "ROOT";
			case TAG_NAMESPACE:  return "NAMESPACE";
			case TAG_SCRIPT:     return "SCRIPT";
			case TAG_FUNCTION:   return "FUNCTION";
			case TAG_CLASS:      return "CLASS";
			case TAG_ENUM:       return "ENUM";
			case TAG_BLOCK:      return "BLOCK";
			case TAG_FILE:       return "FILE";
			default:             return "UNKNOWN";
		}
	};

	std::function<void(int32_t, int)> print_scope = [&](int32_t scope_idx, int indent) {
		// Make the header text.
		// Example: [FUNCTION] void MyFunc <PC:100-200>

		const auto& scope = scopes[scope_idx];
		std::string pad(indent * 2, ' ');
		std::string name = scope.tag == TAG_BLOCK ?
			"" :
			scope.name.empty() ? "<anon>" : scope.name;

		std::vector<std::string> parts;
		parts.push_back(fmt::format("[{}]", get_tag_name(scope.tag)));
		if (scope.type_id != TYPE_VOID)
			parts.push_back(getTypeName(scope.type_id));
		parts.push_back(name);
		if (scope.inheritance_index >= 0 && scope.inheritance_index < (int)scopes.size())
			parts.push_back(fmt::format(" extends {}", scopes[scope.inheritance_index].name));

		if (scope.tag == TAG_FUNCTION || scope.tag == TAG_BLOCK)
			parts.push_back(fmt::format("<PC:{}-{} IDX:{}>", scope.start_pc, scope.end_pc, scope_idx));
		else
			parts.push_back(fmt::format("<IDX:{}>", scope_idx));

		if (scope.flags & SCOPE_FLAG_HIDDEN)
			parts.push_back("<HIDDEN>");
		if (scope.flags & SCOPE_FLAG_INTERNAL)
			parts.push_back("<INTERNAL>");
		if (scope.flags & SCOPE_FLAG_DEPRECATED)
			parts.push_back("<DEPRECATED>");

		std::erase_if(parts, [](auto& v){
			return v.empty();
		});
		ss << fmt::format("{}{}\n", pad, fmt::join(parts, " "));

		if (scope_symbols.count(scope_idx))
		{
			for (const auto* sym : scope_symbols[scope_idx])
			{
				std::string extra_info;
				if (sym->flags & SYM_FLAG_HIDDEN)
					extra_info += " <HIDDEN>";
				if (sym->flags & SYM_FLAG_VARARGS)
					extra_info += " <VARARG>";

				std::string type_name = getTypeName(sym->type_id);

				// Example: - int myVar @ Stack[4]
				ss << fmt::format("{}  - {}\n", pad, getDebugSymbolName(sym));
			}
		}

		if (scope_children.count(scope_idx))
		{
			for (int32_t child_idx : scope_children[scope_idx])
			{
				print_scope(child_idx, indent + 1);
			}
		}
	};

	if (root_indices.empty() && !scopes.empty())
	{
		ss << "<Error: No Root Scopes Found>\n";
	}

	ss << "Scopes:\n\n";
	for (int32_t root : root_indices)
	{
		print_scope(root, 0);
	}

	// Types.
	ss << "\nTypes:\n\n";
	for (int i = 0; i < types.size() + DEBUG_TYPE_TAG_TABLE_START; i++)
		ss << i << ": " << getTypeName(i) << '\n';

	return ss.str();
}
