#include "stack.hpp"

// Initialization of static member variables
bef_t Stack::stack[Stack::STACK_SIZE];
bef_t* Stack::sp = stack - 1;