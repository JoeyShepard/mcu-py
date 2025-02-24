#include <stdint.h>

#ifndef __GUARD_MCU_PY_CORE_PUBLIC
    #define __GUARD_MCU_PY_CORE_PUBLIC

    //typedefs
    typedef uint8_t py_error_t;

    //Functions
    py_error_t py_init(uint8_t *mem, uint16_t size, void (*error_func)(uint8_t, uint16_t));

#endif
