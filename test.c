#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mcu-py.h"
#include "debug_public.h"


//#define NDEBUG //uncomment to disable asserts
#include <assert.h>

#define MEM_SIZE 400

#define INTERACTIVE true
//#define INTERACTIVE true

/*TODO:
- lots more comments
- error_handler can return code to continue executing?
- check all pub and priv headers needed
- if py_error_num no longer visible publically, why even record?
  - NEEDED! if function returns value (like py_allocate) and also sets error, need way to check
- remove stdio.h
- go back and replace int with int16 etc
- make sure no use of strlen etc outside of debug.c
- address all TODOs
- double check ; works as expected
- check memory passed in meets minimum
- 6.10 - chaining ie a < b < c
- StackItem should just be pointer not struct
  - bit in pointer shows if int
    - (x<<1)>>1 extends sign!
    - actually, dont do this. dont want to detect overflow on all calculations
  - actually, changed my mind again:
    - garbage collection for random ints would suck and waste memory
    - otoh, only need 16 bits for pointer. hmm, not sure...
      - wasting 2 bytes on stack may still be better than gc
    - compromise - 4 bit type so 5 bytes for int or for 2 pointers
      - this could work but slow
    - could use all 4 bytes for pointer but how does that work on x86?
      - #ifdef in engine? could work if defined in custom.h
    - SOLVED: greater of sizeof(int32) and sizeof(pointer)
      - wastes space on x86 but who cares
      - right size on SH4
      - wastes 2 bytes on MSP430
- x="a""b" works in python
- allocate and all other functions that can set error have error checking on return?
  - smallest but slowest is to check error at beginning of function
  - actually, wrapper function for compiling since speed not important
- track lines for errors instead of characters since characters not very accurate
  - move source line num for errors to global obj so functions dont need to pass line num in case callee errors 
- if space, bytecode for multiple values of one type for lists
- added exception to state LUT - need more syntax checks now?
  - already checking for 2+3) so what else?
- two bytecodes for tuple token depending on 8 or 16 bit arg count?
- more logical to change some enums to #define?
- some syntax errors are PY_ERROR_INPUT and some PY_ERROR_SYNTAX
- code for processing ] } ) is huge - convert to table and compare size
- redundant to have 0 at end of heap linked list and heap_ptr?
- remove second argument of py_error_set
- double check sp_count useful
- compile state machine checks for comma and other things. simplify other parts?
- goto for error handling? can be smaller and more uniform
- line_number in py is good use of mem?

CURRENTLY
- variables
  - assignments FIRST
  - locals and globals
  - a,b=1,5
    - noticed that at runtime ie a+2 requires resolution of a so clear not being assigned to
- user-defined functions
- keywords
  - if, while, for
*/

void test_error_handler(uint8_t e, uint16_t line_number)
{
    printf("Test error handler - error %d on line %u\n",e,line_number);

    return;
}

//Short term tests
//TODO: remove at the end
void scratch()
{
}

int main(int argc, char *argv[])
{
    uint8_t test_mem[MEM_SIZE];
    py_error_t py_return;

    //TODO: remove
    scratch();

    if (INTERACTIVE)
    {
        debug_enabled=true;
        char source[200];
        while(1)
        {
            printf("\n>> ");
            if (!fgets(source,sizeof(source),stdin))
            {
                printf("fgets error - exiting\n");
                exit(1);
            }
            py_return=py_init(test_mem,MEM_SIZE,NULL);
            py_return=py_execute(source);
            if (py_return!=PY_ERROR_NONE)
            printf("\nError: %s\n",debug_value("error",py_return)); 
        }
    }

    debug_enabled=false;

    /*

    //Test - execute code without initializing system
    py_return=py_execute("foo");
    assert(py_return==PY_ERROR_UNINITIALIZED);

    //Test - trigger error with no error handler defined
    py_return=py_init(test_mem,MEM_SIZE,NULL);
    assert(py_return==PY_ERROR_NONE);
    py_return=py_execute("2+");
    assert(py_return==PY_ERROR_SYNTAX);

    //Test - trigger error with error handler defined
    py_return=py_init(test_mem,MEM_SIZE,&test_error_handler);
    assert(py_return==PY_ERROR_NONE);
    py_return=py_execute("2+");
    assert(py_return==PY_ERROR_SYNTAX);

    //Test - invalid input
    const char *error_strings[]={"a ! b","0x","'a","05","not not not in"};
    for (int i=0;i<sizeof(error_strings)/sizeof(error_strings[0]);i++)
    {
        py_return=py_init(test_mem,MEM_SIZE,NULL);
        assert(py_return==PY_ERROR_NONE);
        py_return=py_execute(error_strings[i]);
        assert(py_return==PY_ERROR_INPUT);
    }

    //Test - parse symbols
    py_return=py_init(test_mem,MEM_SIZE,NULL);
    assert(py_return==PY_ERROR_NONE);
    py_return=py_execute("  -+-+-xyz,abc not in def += not not foo(5)-['foo+3',bar,5+7,0x42,{3:9},len,None,len(baz)]  #comment");
    assert(py_return==PY_ERROR_NONE);
    */

    debug_enabled=true;

    //Test - order of operations
    py_return=py_init(test_mem,MEM_SIZE,NULL);
    assert(py_return==PY_ERROR_NONE);
    py_return=py_execute("2+3*-4+(~5-6/7+8)+1%2-3+4**5+6+(1 in 2 not in 3)+((1+2)*(3+4))");
    assert(py_return==PY_ERROR_NONE);

    debug_enabled=false;

    //Test - functions
    py_return=py_init(test_mem,MEM_SIZE,NULL);
    assert(py_return==PY_ERROR_NONE);
    py_return=py_execute("2,+foo(2,3)*bar((5))");
    assert(py_return==PY_ERROR_NONE);
    




    debug_print_log();

}
