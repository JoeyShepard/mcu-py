#include <stdint.h>

#ifndef __GUARD_MCU_PY_DEBUG_PUBLIC
    #define __GUARD_MCU_PY_DEBUG_PUBLIC

    //Globals
    extern bool debug_enabled;

    //Functions
    const char *debug_value(const char *group, uint16_t value);
    int debug(const char *format,...);
    int debug_key();
    int debug_log(const char *msg);
    int debug_log_int(uint32_t num);
    int debug_log_hex(uint32_t num);
    int debug_print_log();
    int debug_reset_log();
    int debug_stack();

#endif
