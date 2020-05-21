#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "bef_type.hpp"
#include "stack.hpp"
#include "heap.hpp"

// Width and height of code grid
constexpr int gridH = 25, gridW = 80;
// Instead of wrapping PC after every instruction surround grid with wrap instructions
constexpr int realH = gridH + 2, realW = gridW + 2;

enum class Direction : int { Up = -realW, Down = realW, Left = -1, Right = 1 };

// Class for program counter
class Position
{
private:
    template<class T>
    friend class CodeGrid;

    int pos;

public:
    Position(int y, int x) : pos((y + 1) * realW + (x + 1)) {}

    Position& operator+=(Direction dir)
    {
        pos += static_cast<int>(dir);

        return *this;
    }

    // Advance one horizontally
    Position& operator++() { pos++; return *this; }

    void wrapX()
    {
        auto quot_rem = std::div(pos, realW);
        pos = quot_rem.quot * realW;
        pos += (quot_rem.rem == 0) ? gridW : 1;
    }
    
    void wrapY() { pos %= realH * realW; }
};


// Type of code grid
template<class T>
class CodeGrid
{
private:
    std::array<T, realH * realW> code;

public:
    T& operator()(int y, int x) { return code[(y + 1) * realW + (x + 1)]; }

    T& operator()(const Position& pos) { return code[pos.pos]; }
};


void print_usage()
{
    std::cout << "Usage:\n./befunge93 <input_file>" << std::endl;
}


// Reads code from file
void readCode(CodeGrid<char>& code, const char* filename)
{
    std::ifstream file(filename);
    
    if (file.fail())
    {
        std::cerr << "Opening file fail" << std::endl;
        exit(1);
    }

    std::string line;
    for (int i = 0; i < gridH; i++)
    {
        getline(file, line, '\n');
        if (line.length() > gridW)
        {
            std::cerr << "Line width more than allowed. Line " << i << std::endl;
            exit(1);
        }

        Position p(i, 0);
        for(auto it = line.begin(); it != line.cend(); ++it, ++p) code(p) = *it;

        if (file.eof()) break;
    }

    file.close();
}

#define NEXT_INSTRUCTION(instr) goto* (instr);

// Enum for instructions
enum Instr
{
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Not,
    Grt,
    Pc_right,
    Pc_left,
    Pc_up,
    Pc_down,
    Pc_rand,
    Horif,
    Verif,
    Str,
    Dup,
    Swap,
    Pop,
    Print_int,
    Print_char,
    Bridge,
    Get,
    Put,
    In_int,
    In_char,
    End,
    Cell,
    Head,
    Tail,
    Num, // Push number instruction
    Nop, // <space>
    Unk  // Unknown instruction. This is needed because characters other than the instructions are permitted in string mode
};


int main(int argc, char** argv)
{
    Position pc(0, 0);
    Direction pc_dir = Direction::Right;
    CodeGrid<char> rawCode;
    CodeGrid<void*> code;

    // Initialise code with nop intructions
    for (int y = 0; y < gridH; y++)
        for (int x = 0; x < gridW; x++)
            rawCode(y, x) = ' ';
    
    // Wrap instructions
    for (int y = 0; y < gridH; y++) code(y, -1) = code(y, gridW) = &&wrapX_label;
    for (int x = 0; x < gridW; x++) code(-1, x) = code(gridH, x) = &&wrapY_label;

    // If the command line arguments are not as expected print usage
    if (argc != 2)
    {
        print_usage();
        return 0;
    }
    else readCode(rawCode, argv[1]);

    // array mapping instructions to labels
    static void* labels[] =
    {
    // GCC does not yet support non-trivial designators unfortunately 
        /*[Add]        =*/ &&add_label,
        /*[Sub]        =*/ &&sub_label,
        /*[Mul]        =*/ &&mul_label,
        /*[Div]        =*/ &&div_label,
        /*[Mod]        =*/ &&mod_label,
        /*[Not]        =*/ &&not_label,
        /*[Grt]        =*/ &&grt_label,
        /*[Pc_right]   =*/ &&pc_right_label,
        /*[Pc_left]    =*/ &&pc_left_label,
        /*[Pc_up]      =*/ &&pc_up_label,
        /*[Pc_down]    =*/ &&pc_down_label,
        /*[Pc_rand]    =*/ &&pc_rand_label,
        /*[Horif]      =*/ &&horif_label,
        /*[Verif]      =*/ &&verif_label,
        /*[Str]        =*/ &&str_label,
        /*[Dup]        =*/ &&dup_label,
        /*[Swap]       =*/ &&swap_label,
        /*[Pop]        =*/ &&pop_label,
        /*[Print_int]  =*/ &&print_int_label,
        /*[Print_char] =*/ &&print_char_label,
        /*[Bridge]     =*/ &&bridge_label,
        /*[Get]        =*/ &&get_label,
        /*[Put]        =*/ &&put_label,
        /*[In_int]     =*/ &&in_int_label,
        /*[In_char]    =*/ &&in_char_label,
        /*[End]        =*/ &&end_label,
        /*[Cell]       =*/ &&cell_label,
        /*[Head]       =*/ &&hd_label,
        /*[Tail]       =*/ &&tl_label,
        /*[Num]        =*/ &&num_label,
        /*[Nop]        =*/ &&nop_label,
        /*[Unk]        =*/ &&unk_label
    };

    // function that uses the array above to do the mapping
    auto transf = [&] (int x, int y) -> void {
        switch(rawCode(y, x))                                  
        {                                                          
            case '+':       code(y, x) = labels[Add];        break;
            case '-':       code(y, x) = labels[Sub];        break;
            case '*':       code(y, x) = labels[Mul];        break;
            case '/':       code(y, x) = labels[Div];        break;
            case '%':       code(y, x) = labels[Mod];        break;
            case '!':       code(y, x) = labels[Not];        break;
            case '`':       code(y, x) = labels[Grt];        break;
            case '>':       code(y, x) = labels[Pc_right];   break;
            case '<':       code(y, x) = labels[Pc_left];    break;
            case '^':       code(y, x) = labels[Pc_up];      break;
            case 'v':       code(y, x) = labels[Pc_down];    break;
            case '?':       code(y, x) = labels[Pc_rand];    break;
            case '_':       code(y, x) = labels[Horif];      break;
            case '|':       code(y, x) = labels[Verif];      break;
            case '"':       code(y, x) = labels[Str];        break;
            case ':':       code(y, x) = labels[Dup];        break;
            case '\\':      code(y, x) = labels[Swap];       break;
            case '$':       code(y, x) = labels[Pop];        break;
            case '.':       code(y, x) = labels[Print_int];  break;
            case ',':       code(y, x) = labels[Print_char]; break;
            case '#':       code(y, x) = labels[Bridge];     break;
            case 'g':       code(y, x) = labels[Get];        break;
            case 'p':       code(y, x) = labels[Put];        break;
            case '&':       code(y, x) = labels[In_int];     break;
            case '~':       code(y, x) = labels[In_char];    break;
            case '@':       code(y, x) = labels[End];        break;
            case 'c':       code(y, x) = labels[Cell];       break;
            case 'h':       code(y, x) = labels[Head];       break;
            case 't':       code(y, x) = labels[Tail];       break;
            case '0'...'9': code(y, x) = labels[Num];        break;
            case ' ':       code(y, x) = labels[Nop];        break;
            default:        code(y, x) = labels[Unk];        break;
        }                                                          
    };
    
    // Do the mapping
    for (int64_t y = 0; y < gridH; y++)
        for (int64_t x = 0; x < gridW; x++)
            transf(x, y);
        
    // variable that holds next instruction
    // it is volatile so as to implement prefetching
    void* volatile instr;
    block* b;
    bef_t v, v1, v2;
    int64_t i, x, y;
    char c;

    instr = code(pc);
    NEXT_INSTRUCTION(instr)

//COMMAND         INITIAL STACK (bot->top)RESULT (STACK)

// + (add)         <value1> <value2>       <value1 + value2>
    add_label:
        pc += pc_dir;
        instr = code(pc);
        Stack::push(Stack::pop() + Stack::pop());
        NEXT_INSTRUCTION(instr)
    
// - (subtract)    <value1> <value2>       <value1 - value2>
    sub_label:
        pc += pc_dir;
        instr = code(pc);
        v2 = Stack::pop();
        v1 = Stack::pop();
        Stack::push(v1 - v2);
        NEXT_INSTRUCTION(instr)

// * (multiply)    <value1> <value2>       <value1 * value2>
    mul_label:
        pc += pc_dir;
        instr = code(pc);
        Stack::push(Stack::pop() * Stack::pop());
        NEXT_INSTRUCTION(instr)

// / (divide)      <value1> <value2>       <value1 / value2> (nb. integer)
    div_label:
        pc += pc_dir;
        instr = code(pc);
        v2 = Stack::pop();
        v1 = Stack::pop();
        Stack::push(v1 / v2);
        NEXT_INSTRUCTION(instr)
        
// % (modulo)      <value1> <value2>       <value1 mod value2>
    mod_label:
        pc += pc_dir;
        instr = code(pc);
        v2 = Stack::pop();
        v1 = Stack::pop();
        Stack::push(v1 % v2);
        NEXT_INSTRUCTION(instr)
        
// ! (not)         <value>                 <0 if value non-zero, 1 otherwise>
    not_label:
        pc += pc_dir;
        instr = code(pc);
        Stack::push(!Stack::pop());
        NEXT_INSTRUCTION(instr)
        
// ` (greater)     <value1> <value2>       <1 if value1 > value2, 0 otherwise>
    grt_label:
        pc += pc_dir;
        instr = code(pc);
        v2 = Stack::pop();
        v1 = Stack::pop();
        Stack::push(v1 > v2);
        NEXT_INSTRUCTION(instr)

// > (right)                               PC -> right
    pc_right_label:
        pc_dir = Direction::Right;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// < (left)                                PC -> left
    pc_left_label:
        pc_dir = Direction::Left;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// ^ (up)                                  PC -> up
    pc_up_label:
        pc_dir = Direction::Up;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// v (down)                                PC -> down
    pc_down_label:
        pc_dir = Direction::Down;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// ? (random)                              PC -> right? left? up? down? ???
    pc_rand_label:
        switch (std::rand() % 4)
        {
            case 0:
                pc_dir = Direction::Right;
                break;
            case 1:
                pc_dir = Direction::Left;
                break;
            case 2:
                pc_dir = Direction::Down;
                break;
            case 3:
                pc_dir = Direction::Up;
                break;
        }
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)
    
// _ (horizontal if) <boolean value>       PC->left if <value>, else PC->right
    horif_label:
        pc_dir = (bool) Stack::pop() ? Direction::Left : Direction::Right;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)
        
// | (vertical if)   <boolean value>       PC->up if <value>, else PC->down
    verif_label:
        pc_dir = (bool) Stack::pop() ? Direction::Up : Direction::Down;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)
        
// " (stringmode)                          Toggles 'stringmode'
    str_label:
        pc += pc_dir;
        while((c = rawCode(pc)) != '"')
        {
            void* w = code(pc);
            if ( w == &&wrapX_label) pc.wrapX();
            else if (w == &&wrapY_label) pc.wrapY();
            else
            {
                Stack::push(char2bef(c));
                pc += pc_dir;
            }
        }
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)
        
// : (dup)         <value>                 <value> <value>
    dup_label:
        pc += pc_dir;
        instr = code(pc);
        Stack::push(Stack::head());
        NEXT_INSTRUCTION(instr)
        
// \ (swap)        <value1> <value2>       <value2> <value1>
    swap_label:
        pc += pc_dir;
        instr = code(pc);
        v2 = Stack::pop();
        v1 = Stack::pop();
        Stack::push(v2);
        Stack::push(v1);
        NEXT_INSTRUCTION(instr)
    
// $ (pop)         <value>                 pops <value> but does nothing
    pop_label:
        pc += pc_dir;
        instr = code(pc);
        Stack::pop();
        NEXT_INSTRUCTION(instr)

// . (output int)  <value>                 outputs <value> as integer
    print_int_label:
        pc += pc_dir;
        instr = code(pc);
        print_int(Stack::pop());
        NEXT_INSTRUCTION(instr)

// , (output char) <value>                 outputs <value> as ASCII
    print_char_label:
        pc += pc_dir;
        instr = code(pc);
        print_char(Stack::pop());
        NEXT_INSTRUCTION(instr)

// # (bridge)                              'jumps' PC one farther; skips
//                                         over next command
    bridge_label:
        pc += pc_dir;
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// g (get)         <x> <y>                 <value at (x,y)>
    get_label:
        pc += pc_dir;
        instr = code(pc);
        y = bef2int(Stack::pop());
        x = bef2int(Stack::pop());
        Stack::push(char2bef(rawCode(y, x)));
        NEXT_INSTRUCTION(instr)

// p (put)         <value> <x> <y>         puts <value> at (x,y)
    put_label:
        pc += pc_dir;
        instr = code(pc);
        y = bef2int(Stack::pop());
        x = bef2int(Stack::pop());
        v = Stack::pop();
        rawCode(y, x) = bef2char(v);
        transf(x, y);
        NEXT_INSTRUCTION(instr)

// & (input int)                           <value user entered>
    in_int_label:
        pc += pc_dir;
        instr = code(pc);
        std::cin >> i;
        Stack::push(int2bef(i));
        NEXT_INSTRUCTION(instr)
        
// ~ (input character)                     <character user entered>
    in_char_label:
        pc += pc_dir;
        instr = code(pc);
        std::cin >> c;
        Stack::push(char2bef(c));
        NEXT_INSTRUCTION(instr)

// @ (end)                                 ends program
    end_label:
        return 0;

// c (cons)        <value1> <value2>       <address of allocated cons cell in the heap
//                                         with head = <value1> and tail = <value2> >
    cell_label:
        pc += pc_dir;
        instr = code(pc);
        b = Heap::alloc();
        v2 = Stack::pop();
        v1 = Stack::pop();
        b->head = v1;
        b->tail = v2;
        Stack::push(bef_t{.ptr = b});
        NEXT_INSTRUCTION(instr)

// h (head)        <value>                 <head of cons cell with address <value> >
    hd_label:
        pc += pc_dir;
        instr = code(pc);
        b = Stack::pop().ptr;
        Stack::push(b->head);
        NEXT_INSTRUCTION(instr)

// t (tail)        <value>                 <tail of cons cell with address <value> >
    tl_label:
        pc += pc_dir;
        instr = code(pc);
        b = Stack::pop().ptr;
        Stack::push(b->tail);
        NEXT_INSTRUCTION(instr)

// 0...9                                   push number
    num_label:
        Stack::push(int2bef(rawCode(pc) - '0'));
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

// <space>                                no operation
    nop_label:
        pc += pc_dir;
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

    unk_label:
        std::cerr << "Unknown instruction: " << rawCode(pc) << std::endl;
        return 1;

    wrapX_label:
        pc.wrapX();
        instr = code(pc);
        NEXT_INSTRUCTION(instr)

    wrapY_label:
        pc.wrapY();
        instr = code(pc);
        NEXT_INSTRUCTION(instr)
}
