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
- some uint functions return -1
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
- {5,2,2} is set {2,5} not dict
- 6.10 - chaining ie a < b < c
- add keywords
  - list
  - try/except?
  - add line and char number as args to error_handler
  - import? not for msp430
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
- track lines for errors instead of characters since characters not very accurate
- if space, bytecode for multiple values of one type for lists
- move source line num for errors to global obj so functions dont need to pass line num in case callee errors 
- double check optimizer on msp430 resolves py_sp etc defines. if not, buffer value before loop.
  - ie py_peek_stack
- exception to state LUT - need more syntax checks now?
  - already checking for 2+3) so what else?


IMPORTANT:
- keeping a list is not great but need it for globals
  - def foo():return x
      - enough to know this is expected to be a global if not referenced locally yet
  - separate ref count in list of globals to keep list small
    - if no more refs, delete from list and garbage collect
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
