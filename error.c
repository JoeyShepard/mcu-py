
#include <stdio.h>

#include "core.h"
#include "error.h"
#include <stdint.h>
#include "globals.h"

uint8_t py_error_set(uint8_t error_num, uint16_t line_number)
{
    py_error_num=error_num;

    if (error_num!=PY_ERROR_NONE)
    {
        if (py_error_func_ptr!=NULL)
        {
            py_error_func(py_error_num, line_number);
        }
    }

    return error_num;
}

