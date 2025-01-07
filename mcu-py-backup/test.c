#include <stdio.h>
#include "mcu-py.h"

//#define NDEBUG //uncomment to disable asserts
#include <assert.h>

#define MEM_SIZE 400

/*TODO:
- error_handler can return code to continue executing?
- separate out header files
*/

static void error_handler()
{
    printf("Test error handler - error %d\n",py_error_num);

    return;
}

//Short term tests
//TODO: remove at the end
void scratch()
{
}

int main(int argc, char *argv[])
{
    unsigned char test_mem[MEM_SIZE];

    //TODO: remove
    scratch();

    //Test 1 - execute code without initializing system
    unsigned char py_return=py_execute("foo");
    assert(py_return==PY_ERROR_UNINITIALIZED);

    //Test 2 - trigger error with no error handler defined
    py_init(test_mem,MEM_SIZE,0);
    assert(py_error_num==PY_ERROR_NONE);
    py_execute("foo");  //foo undefined!
    assert(py_error_num==PY_ERROR_UNDEFINED);

    //Test 3 - trigger error with error handler defined
    py_init(test_mem,MEM_SIZE,0);
    assert(py_error_num==PY_ERROR_NONE);
    py_execute("foo");  //foo undefined!
    assert(py_error_num==PY_ERROR_UNDEFINED);

    //Test 4 - check no error generated if symbol found
    py_init(test_mem,MEM_SIZE,0);
    assert(py_error_num==PY_ERROR_NONE);
    py_execute("None");  //foo undefined!
    assert(py_error_num==PY_ERROR_NONE);

    return 0;
}
