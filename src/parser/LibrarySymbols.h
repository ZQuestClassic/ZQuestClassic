#ifndef LIBRARY_SYMBOLS_H_
#define LIBRARY_SYMBOLS_H_

#include "DataStructs.h"

using namespace ZScript;

void getConstant(int32_t refVar, Function* function, int32_t var);
void getVariable(int32_t refVar, Function* function, int32_t var);
void setVariable(int32_t refVar, Function* function, int32_t var);
void getIndexedVariable(int32_t refVar, Function* function, int32_t var);
void setIndexedVariable(int32_t refVar, Function* function, int32_t var);
void setBoolVariable(int32_t refVar, Function* function, int32_t var);
bool setConstExprForBinding(Function* fn);
void getInternalArray(int32_t refVar, Function* function, int32_t zasm_var);

#endif
