#include "tables_public.h"

#ifndef __GUARD_MCU_PY_TABLES_PRIVATE
    #define __GUARD_MCU_PY_TABLES_PRIVATE

    //Constants
    #define PY_SYMBOL_ROW_SIZE              5   //Size of row in py_symbol_state
    #define PY_OP_LOOKUP_KEYS_LONG_COUNT    4   //Number of elements in py_op_lookup_keys_long
    #define PY_OP_COMBOS_SIZE               3   //Size of row in py_op_combos
    #define PY_OP_COMBOS_COUNT              19  //Number of elements in py_op_combos

    //Globals
    extern const char *py_commands;
    extern const uint8_t py_symbol_state[][PY_SYMBOL_ROW_SIZE];
    extern const uint8_t py_state_chars[];
    extern const char *py_op_lookup_keys;
    extern const uint8_t py_op_lookup_vals[];
    extern const char *py_op_lookup_keys_long[];
    extern const uint8_t py_op_lookup_vals_long[];
    extern const uint8_t py_op_combos[][PY_OP_COMBOS_SIZE];
    extern const uint8_t py_op_precedence[];
    extern const uint8_t py_state_table[];

#endif
