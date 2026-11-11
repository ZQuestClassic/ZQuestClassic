#pragma once

#include "parser/AST.h"

using namespace ZScript;

namespace ParserHelper {

ParserScriptType getScriptType(std::string name);

bool isValidIdentifier(std::string identifier);

}
