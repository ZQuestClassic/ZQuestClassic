#include <string>
#include <vector>
#include "parser/ZScript.h"

using namespace ZScript;

uint16_t getSymbolId(const AST* node);

struct ParseCommentResult {
	std::string symbol_name;
	int pos;
	int len;
	std::string link_text;
	const AST* symbol_node;
};

std::vector<ParseCommentResult> parseForSymbolLinks(std::string comment, const AST* node, bool check_params);
