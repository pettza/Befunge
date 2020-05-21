#include "heap.hpp"
#include "stack.hpp"

// Initialization of static member variables
block Heap::heap[Heap::HEAP_SIZE];

block* Heap::free_list = heap;

block* const Heap::marked_null = (block*) (((int64_t) nullptr) | 0b10);

bool Heap::run_gc_once = false;


// Mark and sweep garbage collection with in-place stack
void Heap::collect_garbage()
{
    // Mark phase
    for(bef_t* s = Stack::sp; s >= Stack::stack; s--)
    {
        if (is_ptr(*s))
        {
            mark(*s);
            block_ptr prev = marked_null;
            block_ptr cur = s->ptr;
            block_ptr temp;
            mark(cur->head);

            while(true)
            {
                // If head is a poiter and the pointed block is not marked
                if (is_ptr(cur->head) && !is_marked(block_ptr(cur->head.ptr)->head))
                {
                    // Build stack in-place
                    temp = cur->head.ptr;
                    cur->head.ptr = prev;
                    prev = cur;
                    cur = temp;

                    mark(cur->head);
                }
                // If tail is a poiter and the pointed block is not marked
                else if (is_ptr(cur->tail) && !is_marked(block_ptr(cur->tail.ptr)->head))
                {
                    // Mark tail to indicate that it has also been traversed
                    mark(cur->tail);

                    // Build stack in-place
                    temp = cur->tail.ptr;
                    cur->tail.ptr = prev;
                    prev = cur;
                    cur = temp;
                    
                    mark(cur->head);
                }
                // Else, pop a block off the stack
                else
                {
                    temp = cur;
                    cur = prev;
                    if (cur == marked_null) break;

                    // prev is updated depending on whether head or tail was followed last
                    if (!is_marked(cur->tail))
                    {
                        prev = cur->head.ptr;
                        cur->head.ptr = temp;
                    }
                    else
                    {
                        prev = cur->tail.ptr;
                        cur->tail.ptr = temp;
                    }   
                }
            }

            unmark(*s);
        }
    }

    // Sweep phase
    for(block* b = heap; b < heap + HEAP_SIZE; b++)
    {
        // If block is marked, umark it
        if (is_marked(b->head))
        {
            unmark(b->head);
            unmark(b->tail);
        }
        // Else add it to the free list
        else
        {
            b->head.ptr = free_list;
            free_list = b;
        }
    }

    // If free_list points to the end of the heap, then no memory was freed
    if (free_list == heap + HEAP_SIZE)
    {
        std::cout << "Not enough memmory\n";
        exit(1);
    }
}

void Heap::printHeap()
{
    for(block* b = heap; b < heap + HEAP_SIZE; b++)
    {
        std::cout << b - heap << ':' << std::endl;

        std::cout << "\thead:" << ": ";
        if (is_ptr(b->head)) std::cout << "ptr: " << b->head.ptr - heap;
        else std::cout << (b->head.i>>2);
        std::cout << " marked: " << is_marked(b->head) << std::endl;
        
        std::cout << "\ttail:" << ": ";
        if (is_ptr(b->tail)) std::cout << "ptr: " << b->tail.ptr - heap;
        else std::cout << (b->tail.i>>2);
        std::cout << " marked: " << is_marked(b->tail) << std::endl;
    }
}
