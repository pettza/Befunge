#pragma once

#include <iostream>
#include "stdint.h"

struct block;

// The basic type of befunge+
union bef_t
{
    int64_t i;
    block* ptr;

    explicit operator bool() { return (i>>2) != 0; }
};

// The type of heap blocks
struct block
{
    bef_t head, tail;
};


// Functions and operators for 62-bit integers

inline bef_t int2bef(int64_t i) { return {.i = (i<<2) + 1}; }

inline int64_t bef2int(bef_t b) { return b.i>>2; }

inline bef_t char2bef(char c) { return int2bef((int64_t) c); }

inline char bef2char(bef_t b) { return b.i >> 2; }

inline bef_t operator!(bef_t b) { return int2bef( !((bool) b) ); }

inline bef_t operator+(bef_t lhs, bef_t rhs) { return {.i = lhs.i + (rhs.i - 1)}; }

inline bef_t operator-(bef_t lhs, bef_t rhs) { return {.i = (lhs.i - rhs.i) + 1}; }

inline bef_t operator*(bef_t lhs, bef_t rhs) { return {.i = (lhs.i >> 2) * (rhs.i - 1) + 1}; }

inline bef_t operator/(bef_t lhs, bef_t rhs)
{ 
    return {.i = (((lhs.i >> 2) / (rhs.i >> 2)) << 2) + 1};
}

inline bef_t operator%(bef_t lhs, bef_t rhs)
{
    return {.i = (((lhs.i >> 2) % (rhs.i >> 2)) << 2) + 1};
}

inline bef_t operator>(bef_t lhs, bef_t rhs) { return int2bef(lhs.i > rhs.i); }

inline void print_int(bef_t b) { std::cout << (b.i>>2); std::cout.flush(); }

inline void print_char(bef_t b) { std::cout << char(b.i>>2); std::cout.flush(); }

inline bool is_ptr(bef_t b) { return !(b.i & 0b1); }