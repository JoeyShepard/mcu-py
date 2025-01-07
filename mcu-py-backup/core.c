#include <stdio.h>
#include "mcu-py.h"

unsigned char py_allocate(unsigned int size)
{
}

unsigned char py_error_set(unsigned char error_num)
{
    py_settings.mem[PY_MEM_ERROR_NUM]=42;
    py_error_num=error_num;

    return error_num;
}

unsigned char py_init(unsigned char *mem, unsigned int size, void (*error_func)())
{
    py_settings.mem=mem;
    py_settings.initialized=-1;

    _py_mem_size=size;
    _py_error_func_ptr=error_func;

    return py_error_set(PY_ERROR_NONE);
}

