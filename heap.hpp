#pragma once

#include "bef_type.hpp"

class Heap
{
private:
    static constexpr size_t HEAP_SIZE = ((1<<24) * sizeof(int64_t)) / sizeof(block);

    static block heap[HEAP_SIZE];

    static block* free_list;

    static block* const marked_null;
    
    static bool run_gc_once;
    
    // The following class overrides the operator->
    // Because of the marking dereferencing a pointer needs
    // to ignore last 2 bits
    struct block_ptr
    {
        block* ptr;

        block_ptr(block* ptr = marked_null) : ptr(ptr) {}

        bool operator!=(block* other) { return ptr != other; }

        block* operator->() { return (block*) (((int64_t) ptr) & ~0b11); }

        operator block*() { return ptr; }
    };

    static void mark(bef_t& b) { b.i |= 0b10; }

    static void unmark(bef_t& b) { b.i &= ~0b10; }

    static bool is_marked(bef_t b) { return b.i & 0b10; }

    static void collect_garbage();

public:
    Heap() = delete;

    // The blocks are of fixed size 2 * sizeof(bef_t) = 2 * sizeof(int64_t)
    // so just get the first block available
    static block* alloc()
    {

        if (free_list == heap + HEAP_SIZE)
        {
            collect_garbage();
            run_gc_once = true;
        }

        block* prev = free_list;

        if (run_gc_once) free_list = free_list->head.ptr;
        else free_list++;
        
        return prev;
    }

    static void printHeap();
};
