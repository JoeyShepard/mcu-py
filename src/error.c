#include <stdio.h>

#include "core.h"
#include "error.h"
#include <stdint.h>
#include "globals.h"

uint8_t py_error_set(uint8_t error_num) 
{
    py->error_num=error_num;

    if (error_num!=PY_ERROR_NONE)
    {
        if (py->error_func!=NULL)
        {
            py->error_func(error_num, py->line_number);
        }
    }
    return error_num;
}

