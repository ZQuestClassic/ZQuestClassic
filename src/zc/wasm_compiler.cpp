#include "zc/wasm_compiler.h"

uint32_t WasmModuleBuilder::internFuncType(const WasmResultType &params,
                                         const WasmResultType &results) {
  for (size_t i = 0; i < types.size(); i++) {
    if (types[i].params.size() != params.size())
      continue;
    if (types[i].results.size() != results.size())
      continue;
    bool same = true;
    for (size_t j = 0; j < params.size(); j++)
      if (types[i].params[j] != params[j])
        same = false;
    for (size_t j = 0; j < results.size(); j++)
      if (types[i].results[j] != results[j])
        same = false;
    if (same)
      return i;
  }
  types.push_back(WasmFuncType{params, results});
  return types.size() - 1;
}

uint32_t WasmModuleBuilder::declareFunction(const WasmResultType &params,
                                          const WasmResultType &results) {
  uint32_t type = internFuncType(params, results);
  uint32_t idx = functions.size() + function_imports.size();
  functions.push_back(WasmFunc{idx, type, {}, {}, ""});
  return idx;
}

void WasmModuleBuilder::defineFunction(uint32_t idx,
                                       std::vector<WasmValType> locals,
                                       std::vector<uint8_t> &&code) {
  auto &fn = functions[idx - function_imports.size()];
  fn.locals = locals;
  fn.code = code;
}

uint32_t WasmModuleBuilder::addFunction(uint32_t type,
                                      std::vector<WasmValType> locals,
                                      std::vector<uint8_t> &&code) {
  uint32_t idx = functions.size() + function_imports.size();
  functions.push_back(WasmFunc{idx, type, locals, code, ""});
  return idx;
}

void WasmModuleBuilder::setFunctionName(uint32_t idx, std::string name) {
  functions[idx - function_imports.size()].name = name;
}

uint32_t WasmModuleBuilder::importFunction(std::string name, int num_params,
                                         int num_returns) {
  WasmResultType params;
  WasmResultType returns;
  for (int i = 0; i < num_params; i++)
    params.push_back(WasmValType::I32);
  for (int i = 0; i < num_returns; i++)
    returns.push_back(WasmValType::I32);
  uint32_t type = internFuncType(params, returns);
  function_imports.emplace_back(type, name);
  return function_imports.size() - 1;
}

void WasmModuleBuilder::exportFunction(uint32_t fid, std::string name) {
  function_exports.emplace_back(fid, name);
}

uint32_t WasmModuleBuilder::addGlobal(std::string name) {
  globals.push_back(name);
  return globals.size() - 1;
}

void WasmModuleBuilder::addIndirectFunction(uint32_t idx) {
  indirectFunctionTable.push_back(idx);
}

void WasmModuleBuilder::recordStartFunction(uint32_t idx) {
  startFunction = idx;
}

WasmModule WasmModuleBuilder::finish() {
  WasmModuleWriter writer;
  writer.emitMagic();
  writer.emitVersion();
  writer.emitTypeSection(types);
  writer.emitImportSection(function_imports);
  writer.emitFunctionSection(functions);
  // writer.emitMemorySection();
  writer.emitGlobalSection(globals);
  writer.emitExportSection(function_exports);
  if (startFunction != uint32_t(-1))
    writer.emitStartSection(startFunction);
  writer.emitElementSection(indirectFunctionTable);
  writer.emitCodeSection(functions);
  writer.emitNameSection(moduleName, functions, globals);
  return WasmModule{writer.finish()};
}

WasmModule WasmCompiler::finish() { return builder.finish(); }
