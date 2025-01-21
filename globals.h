#include <stdbool.h>
#include <stdint.h>
#include "core.h"
#include "globals_public.h"

#ifndef __GUARD_MCU_PY_GLOBALS_PRIVATE
    #define __GUARD_MCU_PY_GLOBALS_PRIVATE

    extern bool py_initialized; //Global so initialized to 0
    extern struct py_struct *py;

#endif
