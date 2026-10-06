#ifndef ZASM_SERIALIZE_H_
#define ZASM_SERIALIZE_H_

#include "base/ints.h"
#include "core/zdefs.h"
#include "components/zasm/table.h"
#include <optional>
#include <string>

std::string zasm_var_to_string(int32_t var);
std::string zasm_arg_to_string(int32_t arg, ARGTY arg_ty);
std::string zasm_op_to_string(word scommand, int32_t arg1, int32_t arg2, int32_t arg3, const std::vector<int>* argvec, const std::string* argstr);
std::string zasm_op_to_string(const ffscript& c, const zasm_literals& literals);
std::string zasm_op_to_string(word scommand);
std::optional<int> parse_zasm_compare_arg(char const* buf);
// Its string or array goes in `literals`, which must not be full.
ffscript parse_zasm_op(std::string op_str, zasm_literals& literals);

#endif
