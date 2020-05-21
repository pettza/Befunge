#pragma once

#include "bef_type.hpp"

//Singleton class for Stack operations
class Stack
{
friend class Heap;

private:
    static constexpr size_t STACK_SIZE = ((1<<20) * sizeof(int64_t)) / sizeof(bef_t);

    static bef_t stack[STACK_SIZE];

    // Stack pointer
    static bef_t* sp;

public:
    Stack() = delete;

    static void push(bef_t b) { *(++sp) = b; }

    static bef_t pop() { return (sp - stack < 0) ? int2bef(0) : *(sp--); }

    static bef_t head() { return (sp - stack < 0) ? int2bef(0) : *sp; }
};
