#pragma once

#include "bef_type.hpp"

//Singleton class for Heap operations
class Heap
{
private:
    static constexpr size_t HEAP_SIZE = ((1<<24) * sizeof(int64_t)) / sizeof(block);

    static block heap[HEAP_SIZE];

    static block* free_list;

    // Null pointer with its 2nd lsb marked, needed for garbage collection
    static block* const marked_null;

    // True if the GC has run once
    static bool run_gc_once;
    
    // The following class overrides the operator->
    // Because of the marking dereferencing a pointer needs
    // to ignore last 2 bits
    struct block_ptr
    {
        block* ptr;

        block_ptr(block* ptr = marked_null) : ptr(ptr) {}
        
        // Return the equivalent unmarked pointer
        block* operator->() { return (block*) (((int64_t) ptr) & ~0b11); }

        // Needed so block_ptr can be assigned to block*
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
        // If free_list points to the end of the heap, then run the GC
        if (free_list == heap + HEAP_SIZE)
        {
            collect_garbage();
            run_gc_once = true;
        }

        block* prev = free_list;

        // Get the next free block
        // If the GC has not run just increment the free_list pointer since initially the whole heap is free
        if (run_gc_once) free_list = free_list->head.ptr;
        else free_list++;
        
        return prev;
    }

    // Utility function that prints heap's contents
    static void printHeap();
};
