# Virtual machine with garbage collector for Befunge-93 with heap

Exercise for the course "Programming Languages II" at NTUA

[Befunge-93](https://catseye.tc/view/befunge-93/doc/Befunge-93.markdown) is extended with the addition of a heap containing cons cells, which are tuples of values that are either integers or pointers to other cells, and two instructions:

c (cons): removes two values from the stack (first b, then a), allocates a cell on the heap containing the two values (a in first slot, b in second) and pushes the address on the stack

h (head): removes an address from the stack and pushes the first element of the cell it point to

t (tail): the same as h but for the second element