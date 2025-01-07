#include <stdbool.h>
#include <stdint.h>
#include "globals_public.h"

#ifndef __GUARD_MCU_PY_GLOBALS_PRIVATE
    #define __GUARD_MCU_PY_GLOBALS_PRIVATE

    //structs
    struct py_settings_struct
    {
        bool initialized; //Global so initialized to 0
        uint8_t *mem;
    };
    
    //Globals
    extern struct py_settings_struct py_settings;

#endif
