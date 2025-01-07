#include <stdio.h>
#include "mcu-py.h"
unsigned char _py_find_symbol(const char *text)
{
    const char *commands_ptr=py_commands;

    for (int i=0;i<2000;i++)
    {
        if (*commands_ptr==0) 
        {
            printf("\n");
            if (*(commands_ptr+1)==0)
            {
                printf("done!\n");
                break;
            }
        }
        else putchar(*commands_ptr);
        commands_ptr++;
    }
}

unsigned char py_execute(const char *text)
{
    //Can't set error_num if uninitialized since stored in passed in memory so return error code but don't set py_error_num
    if (!py_settings.initialized) return PY_ERROR_UNINITIALIZED;

    char symbol[MAX_SYMBOL_SIZE];
    unsigned char state_symbol=STATE_NONE;

    _py_find_symbol(text);

    return py_error_set(PY_ERROR_UNDEFINED);
}
