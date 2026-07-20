#ifndef ZC_DEBUGGER_VM_H_
#define ZC_DEBUGGER_VM_H_

#include "base/expected.h"
#include "components/zasm/eval.h"
#include "components/zasm/pc.h"
#include "zc/ffscript.h"
#include <optional>
#include <string>
#include <vector>

struct VM : VMInterface
{
	ScriptEngineData* current_data;
	int current_frame_index;
	bool suppress_errors_in_sandbox;

	int32_t readStack(int32_t offset) override;
	int32_t readGlobal(int32_t idx) override;
	int32_t readScript(int32_t idx) override;
	int32_t readRegister(int32_t id) override;
	std::optional<DebugValue> readObjectMember(DebugValue object, const DebugSymbol* sym) override;
	std::optional<std::vector<DebugValue>> readArray(DebugValue array) override;
	std::optional<DebugValue> readArrayElement(DebugValue array, int index) override;
	std::optional<int32_t> readArraySize(DebugValue array) override;
	std::optional<std::string> readString(int32_t string_ptr) override;
	void writeGlobal(int32_t offset, int32_t value) override;
	void writeScript(int32_t offset, int32_t value) override;
	void writeStack(int32_t offset, int32_t value) override;
	void writeRegister(int32_t offset, int32_t value) override;
	bool writeObjectMember(DebugValue object, const DebugSymbol* sym, DebugValue value) override;
	bool writeArrayElement(DebugValue array, int32_t index, DebugValue value) override;
	void decreaseObjectReference(DebugValue value, const DebugSymbol* sym) override;
	void increaseObjectReference(DebugValue value, const DebugSymbol* sym) override;
	expected<int32_t, std::string> executeSandboxed(pc_t start_pc, int this_zasm_var, int this_raw_value, const std::vector<int32_t>& args) override;
	DebugValue createArray(std::vector<int32_t> args, const DebugType* array_type) override;
	DebugValue createString(const std::string& str) override;
	int32_t getThisPointer() override;
};

#endif
