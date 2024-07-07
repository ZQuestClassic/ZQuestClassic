#ifndef WASM_COMPILER_H_
#define WASM_COMPILER_H_

#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>

struct WasmModule {
  std::vector<uint8_t> data;
};

enum class WasmSimpleBlockType : uint8_t {
  Void = 0x40, // SLEB128(-0x40)
  I32 = 0x7F,
  I64 = 0x7E,
};

enum class WasmValType : uint8_t {
  I32 = 0x7f,     // SLEB128(-0x01)
  I64 = 0x7e,     // SLEB128(-0x02)
  F32 = 0x7d,     // SLEB128(-0x03)
  F64 = 0x7c,     // SLEB128(-0x04)
  FuncRef = 0x70, // SLEB128(-0x10)
};

using WasmResultType = std::vector<WasmValType>;
struct WasmFuncType {
  const WasmResultType params;
  const WasmResultType results;
};

struct WasmFunc {
  uint32_t idx;
  uint32_t typeIdx;
  std::vector<WasmValType> locals;
  std::vector<uint8_t> code;
  std::string name;
};

struct WasmWriter {
  std::vector<uint8_t> code;

  std::vector<uint8_t> finish() { return code; }
  void emit(uint8_t byte) { code.push_back(byte); }

  void emitVarU32(uint32_t i) {
    do {
      uint8_t byte = i & 0x7f;
      i >>= 7;
      if (i != 0)
        byte |= 0x80;
      emit(byte);
    } while (i != 0);
  }
  void emitVarI32(int32_t i) {
    bool done;
    do {
      uint8_t byte = i & 0x7f;
      i >>= 7;
      done = ((i == 0) && !(byte & 0x40)) || ((i == -1) && (byte & 0x40));
      if (!done)
        byte |= 0x80;
      emit(byte);
    } while (!done);
  }

  size_t emitPatchableVarU32() {
    size_t offset = code.size();
    emitVarU32(UINT32_MAX);
    return offset;
  }
  size_t emitPatchableVarI32() {
    size_t offset = code.size();
    emitVarI32(INT32_MAX);
    return offset;
  }
  void patchVarI32(size_t offset, int32_t val) {
    for (size_t i = 0; i < 5; i++, val >>= 7) {
      uint8_t byte = val & 0x7f;
      if (i < 4)
        byte |= 0x80;
      code[offset + i] = byte;
    }
  }
  void patchVarU32(size_t offset, uint32_t val) {
    for (size_t i = 0; i < 5; i++, val >>= 7) {
      uint8_t byte = val & 0x7f;
      if (i < 4)
        byte |= 0x80;
      code[offset + i] = byte;
    }
  }
  void emitValType(WasmValType t) { emit(static_cast<uint8_t>(t)); }
};

struct WasmAssembler : public WasmWriter {
  enum class Op : uint8_t {
    Unreachable = 0x00,
    Nop = 0x01,
    Block = 0x02,
    Loop = 0x03,
    If = 0x04,
    Else = 0x05,
    End = 0x0b,
    Br = 0x0c,
    BrIf = 0x0d,
    BrTable = 0x0e,
    Return = 0x0f,

    // Call operators
    Call = 0x10,
    CallIndirect = 0x11,

    // Parametric operators
    Drop = 0x1a,
	Select = 0x1b,

    // Variable access
    LocalGet = 0x20,
    LocalSet = 0x21,
    LocalTee = 0x22,
    GlobalGet = 0x23,
    GlobalSet = 0x24,

    // Memory-related operators
    I32Load = 0x28,
    I32Store = 0x36,

    // Constants
    I32Const = 0x41,
    I64Const = 0x42,

    // Comparison operators
    I32Eqz = 0x45,
    I32Eq = 0x46,
    I32Ne = 0x47,
    I32LtS = 0x48,
    I32LtU = 0x49,
    I32GtS = 0x4A,
    I32GtU = 0x4B,
    I32LeS = 0x4C,
    I32LeU = 0x4D,
    I32GeS = 0x4E,
    I32GeU = 0x4F,

    // Numeric operators
    I32Add = 0x6a,
    I32Sub = 0x6b,
    I32Mul = 0x6c,
    I32DivS = 0x6d,
    I32DivU = 0x6e,
    I32RemS = 0x6F,
    I32RemU = 0x70,
    I32And = 0x71,
    I32Or = 0x72,
    I32Xor = 0x73,
    I32Shl = 0x74,
    I32ShrS = 0x75,
    I32ShrU = 0x76,

    I64Mul = 0x7e,
    I64DivS = 0x7f,
    I64Wrap = 0xA7,
    I32ExtendS = 0xAC,

    RefNull = 0xd0,

    ThreadsPrefix = 0xfe,
    MiscPrefix = 0xfc,
  };
  enum class MiscOp : uint8_t {
    TableGrow = 0x0f,
    TableInit = 0x0c,
  };
  enum class ThreadsOp : uint8_t {
    AtomicNotify = 0x00,
    AtomicWait32 = 0x01,
    AtomicStore8 = 0x19,
  };
  void emitOp(Op op) { emit(static_cast<uint8_t>(op)); }

  size_t emitPatchableI32Const() {
    emitOp(Op::I32Const);
    return emitPatchableVarI32();
  }
  void emitI32Const(int32_t val) {
    emitOp(Op::I32Const);
    emitVarI32(val);
  }
  void emitI64Const(int32_t val) {
    emitOp(Op::I64Const);
    emitVarI32(val);
  }
  void emitMemArg(uint32_t align, uint32_t offset) {
    emitVarU32(align);
    emitVarU32(offset);
  }
  static const uint32_t Int32SizeLog2 = 2;
  void emitI32Load(uint32_t offset = 0) {
    // Base address on stack.
    emitOp(Op::I32Load);
    emitMemArg(Int32SizeLog2, offset);
  }
  void emitI32Store(uint32_t offset = 0) {
    // Base address and value to store on stack.
    emitOp(Op::I32Store);
    emitMemArg(Int32SizeLog2, offset);
  }

  void emitLocalGet(uint32_t idx) {
    emitOp(Op::LocalGet);
    emitVarU32(idx);
  }
  void emitLocalSet(uint32_t idx) {
    emitOp(Op::LocalSet);
    emitVarU32(idx);
  }
  void emitLocalTee(uint32_t idx) {
    emitOp(Op::LocalTee);
    emitVarU32(idx);
  }

  void emitGlobalGet(uint32_t idx) {
    emitOp(Op::GlobalGet);
    emitVarU32(idx);
  }
  void emitGlobalSet(uint32_t idx) {
    emitOp(Op::GlobalSet);
    emitVarU32(idx);
  }

  void emitI32Eqz() { emitOp(Op::I32Eqz); }
  void emitI32Eq() { emitOp(Op::I32Eq); }
  void emitI32Ne() { emitOp(Op::I32Ne); }
  void emitI32LtS() { emitOp(Op::I32LtS); }
  void emitI32LtU() { emitOp(Op::I32LtU); }
  void emitI32GtS() { emitOp(Op::I32GtS); }
  void emitI32LeS() { emitOp(Op::I32LeS); }
  void emitI32LeU() { emitOp(Op::I32LeU); }
  void emitI32GeS() { emitOp(Op::I32GeS); }
  void emitI32GeU() { emitOp(Op::I32GeU); }

  void emitI32Add() { emitOp(Op::I32Add); }
  void emitI32Sub() { emitOp(Op::I32Sub); }
  void emitI32Mul() { emitOp(Op::I32Mul); }
  void emitI32DivS() { emitOp(Op::I32DivS); }
  void emitI32DivU() { emitOp(Op::I32DivU); }
  void emitI32RemS() { emitOp(Op::I32RemS); }
  void emitI32RemU() { emitOp(Op::I32RemU); }
  void emitI32And() { emitOp(Op::I32And); }
  void emitI32Or() { emitOp(Op::I32Or); }
  void emitI32Xor() { emitOp(Op::I32Xor); }
  void emitI32Shl() { emitOp(Op::I32Shl); }
  void emitI32ShrS() { emitOp(Op::I32ShrS); }
  void emitI32ShrU() { emitOp(Op::I32ShrU); }

  void emitI64Mul() { emitOp(Op::I64Mul); }
  void emitI64DivS() { emitOp(Op::I64DivS); }
  void emitI64Wrap() { emitOp(Op::I64Wrap); }
  void emitI32ExtendS() { emitOp(Op::I32ExtendS); }

  void emitDrop() { emitOp(Op::Drop); }
  void emitSelect() { emitOp(Op::Select); }

  void emitCall(uint32_t idx) {
    emitOp(Op::Call);
    emitVarU32(idx);
  }

  void emitCallIndirect(uint32_t calleeType, uint32_t table = 0) {
    emitOp(Op::CallIndirect);
    emitVarU32(calleeType);
    emitVarU32(table);
  }

  void emitRefNull(WasmValType type) {
    emitOp(Op::RefNull);
    emitValType(type);
  }
  void emitMiscOp(MiscOp op) {
    emitOp(Op::MiscPrefix);
    emit(static_cast<uint8_t>(op));
  }
  void emitThreadOp(ThreadsOp op) {
    emitOp(Op::ThreadsPrefix);
    emit(static_cast<uint8_t>(op));
  }
  void emitTableGrow(uint32_t idx) {
    emitMiscOp(MiscOp::TableGrow);
    emitVarU32(idx);
  }
  void emitTableInit(uint32_t dst, uint32_t src) {
    emitMiscOp(MiscOp::TableInit);
    emitVarU32(dst);
    emitVarU32(src);
  }

  void emitAtomicNotify32(uint32_t align = 0, uint32_t offset = 0) {
    emitThreadOp(ThreadsOp::AtomicNotify);
    // TODO emitMemArg
    emit(align);
    emit(offset);
  }

  void emitAtomicWait32(uint32_t align = 0, uint32_t offset = 0) {
    emitThreadOp(ThreadsOp::AtomicWait32);
    emit(align);
    emit(offset);
  }

  void emitAtomicStore8(uint32_t align = 0, uint32_t offset = 0) {
    emitThreadOp(ThreadsOp::AtomicStore8);
    emit(align);
    emit(offset);
  }

  void emitNop() {
    emitOp(Op::Nop);
    emit(static_cast<uint8_t>(WasmSimpleBlockType::Void));
  }

  void emitBlock() {
    emitOp(Op::Block);
    emit(static_cast<uint8_t>(WasmSimpleBlockType::Void));
  }
  void emitBlock(WasmValType blockType) {
    emitOp(Op::Block);
    emit(static_cast<uint8_t>(blockType));
  }
  void emitLoop() {
    emitOp(Op::Loop);
    emit(static_cast<uint8_t>(WasmSimpleBlockType::Void));
  }

  void emitIf(WasmSimpleBlockType type = WasmSimpleBlockType::Void) {
    emitOp(Op::If);
    emit(static_cast<uint8_t>(type));
  }
  void emitElse() { emitOp(Op::Else); }
  void emitEnd() { emitOp(Op::End); }

  void emitBr(uint32_t offset) {
    emitOp(Op::Br);
    emitVarU32(offset);
  }
  void emitBrIf(uint32_t offset) {
    emitOp(Op::BrIf);
    emitVarU32(offset);
  }
  void emitBrTable() { emitOp(Op::BrTable); }
  void emitUnreachable() { emitOp(Op::Unreachable); }
  void emitReturn() { emitOp(Op::Return); }
};

struct WasmModuleWriter : WasmWriter {
  enum class SectionId : uint8_t {
    Custom = 0,
    Type = 1,
    Import = 2,
    Function = 3,
    Table = 4,
    Memory = 5,
    Global = 6,
    Export = 7,
    Start = 8,
    Elem = 9,
    Code = 10,
    Data = 11,
    DataCount = 12,
  };

  enum class DefinitionKind : uint8_t {
    Function = 0x00,
    Table = 0x01,
    Memory = 0x02,
    Global = 0x03,
  };

  enum class LimitsFlags : uint8_t {
    Default = 0x0,
    HasMaximum = 0x1,
    IsShared = 0x2,
    IsI64 = 0x4,
  };

  enum class ElemSegmentKind : uint8_t { Active = 0x0, Passive = 0x1 };

  void emitMagic() {
    emit(0x00);
    emit(0x61);
    emit(0x73);
    emit(0x6D);
  }
  void emitVersion() {
    emit(0x01);
    emit(0x00);
    emit(0x00);
    emit(0x00);
  }
  void emitResultType(const WasmResultType &type) {
    emitVarU32(type.size());
    for (WasmValType t : type)
      emitValType(t);
  }
  void emitSectionId(SectionId id) { emit(static_cast<uint8_t>(id)); }
  void emitTypeSection(const std::vector<WasmFuncType> &types) {
    emitSectionId(SectionId::Type);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitVarU32(types.size());
    for (const auto &type : types) {
      emit(0x60); // Type constructor for function types.
      emitResultType(type.params);
      emitResultType(type.results);
    }
    patchVarU32(patchLoc, code.size() - start);
  }
  void emitMemorySection() {
    emitSectionId(SectionId::Memory);
    emitVarU32(1);

    emit(static_cast<uint8_t>(DefinitionKind::Memory));
    emit(static_cast<uint8_t>(LimitsFlags::IsShared) |
         static_cast<uint8_t>(LimitsFlags::HasMaximum));
    emitVarU32(1);
    emitVarU32(1);
  }
  void emitName(const char *name) {
    emitVarU32(strlen(name));
    while (*name)
      emit(*name++);
  }
  void emitGlobalSection(std::vector<std::string> globals) {
    emitSectionId(SectionId::Global);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();

    emitVarU32(globals.size());
    for (int i = 0; i < globals.size(); i++) {
      emit((int)WasmValType::I32);
      emit(1); // mutable
      emit(0x41);
      emit(0);
      emit(0x0b);
    }

    patchVarU32(patchLoc, code.size() - start);
  }
  void emitImportSection(
      const std::vector<std::pair<uint32_t, std::string>> &function_imports) {
    emitSectionId(SectionId::Import);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();

    emitVarU32(1 + function_imports.size());

    emitName("env");
    emitName("memory");
    emit(static_cast<uint8_t>(DefinitionKind::Memory));
    emit(static_cast<uint8_t>(LimitsFlags::IsShared) |
         static_cast<uint8_t>(LimitsFlags::HasMaximum));
    emitVarU32(1);
    emitVarU32(32768);

    for (auto fn : function_imports) {
      emitName("env");
      emitName(fn.second.c_str());
      emit(static_cast<uint8_t>(DefinitionKind::Function));
      emitVarU32(fn.first);
    }

    patchVarU32(patchLoc, code.size() - start);
  }
  void emitExportSection(
      const std::vector<std::pair<uint32_t, std::string>> &function_exports) {
    emitSectionId(SectionId::Export);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();

    emitVarU32(function_exports.size());
    for (auto [funcIdx, name] : function_exports) {
      emitName(name.c_str());
      emit(static_cast<uint8_t>(DefinitionKind::Function));
      emitVarU32(funcIdx);
    }

    patchVarU32(patchLoc, code.size() - start);
  }
  void emitFunctionSection(const std::vector<WasmFunc> &funcs) {
    emitSectionId(SectionId::Function);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitVarU32(funcs.size());
    for (const auto &func : funcs)
      emitVarU32(func.typeIdx);
    patchVarU32(patchLoc, code.size() - start);
  }
  void emitElementSection(const std::vector<uint32_t> &indirectFunctions) {
    if (indirectFunctions.empty())
      return;
    emitSectionId(SectionId::Elem);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitVarU32(1);
    emit(static_cast<uint8_t>(ElemSegmentKind::Passive));
    emit(0x00); // elemkind: funcref
    emitVarU32(indirectFunctions.size());
    for (auto idx : indirectFunctions)
      emitVarU32(idx);
    patchVarU32(patchLoc, code.size() - start);
  }
  void emitStartSection(uint32_t startFunction) {
    emitSectionId(SectionId::Start);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitVarU32(startFunction);
    patchVarU32(patchLoc, code.size() - start);
  }
  std::vector<uint8_t> encodeLocals(const std::vector<WasmValType> &locals) {
    uint32_t runs = 0;
    {
      size_t local = 0;
      while (local < locals.size()) {
        WasmValType t = locals[local++];
        while (local < locals.size() && locals[local] == t)
          local++;
        runs++;
      }
    }
    WasmWriter writer;
    writer.emitVarU32(runs);
    {
      size_t local = 0;
      while (local < locals.size()) {
        WasmValType t = locals[local++];
        uint32_t count = 1;
        while (local < locals.size() && locals[local] == t)
          count++, local++;
        writer.emitVarU32(count);
        writer.emitValType(t);
      }
    }
    return writer.finish();
  }
  void emitCodeSection(const std::vector<WasmFunc> &funcs) {
    emitSectionId(SectionId::Code);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitVarU32(funcs.size());
    for (const auto &func : funcs) {
      std::vector<uint8_t> locals = encodeLocals(func.locals);
      emitVarU32(locals.size() + func.code.size());
      code.insert(code.end(), locals.begin(), locals.end());
      code.insert(code.end(), func.code.begin(), func.code.end());
    }
    patchVarU32(patchLoc, code.size() - start);
  }

  void emitNameSection(std::string moduleName,
                       const std::vector<WasmFunc> &functions,
                       const std::vector<std::string> &globals) {
    emitSectionId(SectionId::Custom);
    size_t patchLoc = emitPatchableVarU32();
    size_t start = code.size();
    emitName("name");

    emit(0); // module
    size_t patchLoc2 = emitPatchableVarU32();
    size_t start2 = code.size();
    emitName(moduleName.c_str());
    patchVarU32(patchLoc2, code.size() - start2);

    emit(1); // functions
    patchLoc2 = emitPatchableVarU32();
    start2 = code.size();
    size_t patchNumNames = emitPatchableVarU32();
    size_t numFunctionNames = 0;
    for (size_t i = 0; i < functions.size(); i++) {
      if (!functions[i].name.empty()) {
        emitVarU32(functions[i].idx);
        emitName(functions[i].name.c_str());
        numFunctionNames++;
      }
    }
    patchVarU32(patchNumNames, numFunctionNames);
    patchVarU32(patchLoc2, code.size() - start2);

    emit(7); // globals
    patchLoc2 = emitPatchableVarU32();
    start2 = code.size();
    emitVarU32(globals.size());
    for (size_t i = 0; i < globals.size(); i++) {
      emitVarU32(i);
      emitName(globals[i].c_str());
    }
    patchVarU32(patchLoc2, code.size() - start2);

    patchVarU32(patchLoc, code.size() - start);
  }
};

struct WasmModuleBuilder {
  std::string moduleName;
  std::vector<WasmFuncType> types;
  std::vector<WasmFunc> functions;
  std::vector<uint32_t> indirectFunctionTable;
  std::vector<std::pair<uint32_t, std::string>> function_imports;
  std::vector<std::pair<uint32_t, std::string>> function_exports;
  std::vector<std::string> globals;
  uint32_t startFunction = -1;

  uint32_t internFuncType(const WasmResultType &params,
                        const WasmResultType &results);
  uint32_t declareFunction(const WasmResultType &params,
                         const WasmResultType &results);
  void defineFunction(uint32_t idx, std::vector<WasmValType> locals,
                      std::vector<uint8_t> &&code);
  uint32_t addFunction(uint32_t type, std::vector<WasmValType> locals,
                     std::vector<uint8_t> &&code);
  void setFunctionName(uint32_t idx, std::string name);
  uint32_t importFunction(std::string name, int num_params, int num_returns);
  void exportFunction(uint32_t fid, std::string name);
  uint32_t addGlobal(std::string name);
  void addIndirectFunction(uint32_t idx);
  void recordStartFunction(uint32_t idx);

  WasmModule finish();
};

struct WasmCompiler {
  WasmModuleBuilder builder;
  WasmModule finish();
};

#endif
