#include <initializer_list>

// Many registers use the value within data registers when evaluating its value.
// For example, GLOBALRAM uses both rINDEX and rINDEX2 to index a specific array.
std::initializer_list<int> get_register_dependencies(int reg);
