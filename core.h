#include <stdint.h>
#include "core_public.h"

#ifndef __GUARD_MCU_PY_CORE_PRIVATE
    #define __GUARD_MCU_PY_CORE_PRIVATE
   
    //Python settings
    //===============
    struct __attribute__((packed)) py_struct
    {
        uint16_t mem_size;
        void (*error_func)(uint8_t,uint16_t);
        uint8_t error_num;
        uint8_t *sp;
        uint16_t sp_count;
        uint8_t *heap_ptr;
        uint8_t *heap_begin;
    };

    //Stack functionality
    //===================
    //__attribute__ lets struct take up 5 bytes intead of 8 on x86 so easier to test code
    //that will will run on microcontroller where size savings matters
    //Ok to remove __atrribute__ if memory is not constrained
    struct __attribute__((packed)) StackItem
    {
        uint8_t info;               //Data type and other information
        union
        {
            struct
            {
                uint16_t ID;
                uint16_t address;   //Offset into passed-in memory, NOT native pointer
            };
            int32_t int32;
        };
    };

    py_error_t py_push(struct StackItem item);
    

    //Memory management
    //=================
    uint8_t *py_allocate(uint16_t size);
    py_error_t py_append(uint8_t *obj, const void *data, uint16_t data_size);
    uint16_t py_free();


    //Objects
    //=======
    enum ObjectTypes
    {
        OBJECT_CODE,        //Code for function or temporary for code outside function
        OBJECT_INT,
        OBJECT_FLOAT,       //Not supported yet but go ahead and reserve ID
        OBJECT_STRING,
        OBJECT_BOOL,
        OBJECT_VAR,    
        OBJECT_BUILTIN,     //Functions defined by system (len, str, etc)
        OBJECT_LIST,
        OBJECT_DICT,
        OBJECT_TUPLE,
        OBJECT_SET,
        OBJECT_RANGE,
        OBJECT_NONE,            //Python None
        OBJECT_FREE,            //Free to garbage collect
        OBJECT_GLOBAL_VALUES,   //Global variable values
        OBJECT_GLOBAL_NAMES,    //Global variable names
    };


    //Parsing and execution tokens
    //============================
    enum TokenTypes
    {
        //Used in source parser in execute.c
        TOKEN_NONE,
        TOKEN_SYMBOL=TOKEN_NONE,    //Reuse in py_state_table when input is symbol instead of token
        
        //Paste token list from spreadsheet starting here:
    
        TOKEN_PERIOD,
        TOKEN_LPAREN,
        TOKEN_LSBRACKET,
        TOKEN_LCBRACKET,
        TOKEN_RPAREN,
        TOKEN_RSBRACKET,
        TOKEN_RCBRACKET,
        TOKEN_COLON,
        TOKEN_COMMA,
        TOKEN_EQ,
        TOKEN_EXP_EQ,
        TOKEN_MUL_EQ,
        TOKEN_DIV_EQ,
        TOKEN_MOD_EQ,
        TOKEN_ADD_EQ,
        TOKEN_SUB_EQ,
        TOKEN_LSHIFT_EQ,
        TOKEN_RSHIFT_EQ,
        TOKEN_AND_EQ,
        TOKEN_XOR_EQ,
        TOKEN_OR_EQ,
        TOKEN_EXP,
        TOKEN_INVERT,
        TOKEN_NEG,
        TOKEN_PLUS,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_MOD,
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT,
        TOKEN_AND,
        TOKEN_XOR,
        TOKEN_OR,
        TOKEN_IN,
        TOKEN_NOT_IN,
        TOKEN_LT,
        TOKEN_LE,
        TOKEN_GT,
        TOKEN_GE,
        TOKEN_NE,
        TOKEN_EQ_EQ,
        TOKEN_EXCLAM,
        TOKEN_BOOL_NOT,
        TOKEN_BOOL_AND,
        TOKEN_BOOL_OR,
        //Tokens starting here are bytecode - saves space to reuse same token range
        TOKEN_INT8,
        TOKEN_INT16,
        TOKEN_INT32,
        TOKEN_STRING,
        TOKEN_RETURN,
        TOKEN_FUNC,
        TOKEN_TUPLE,
        TOKEN_LIST,
        TOKEN_DICT,
        TOKEN_SET,
        TOKEN_NONE_OBJ,
        TOKEN_SLICE_INDEX,
        TOKEN_INDEX,
        TOKEN_BUILTIN_FUNC,
        TOKEN_GLOBAL,
        TOKEN_LOCAL,
        TOKEN_ATTRIBUTE,
        //Tokens starting here are for storing things like variable names on stack during compilation
        TOKEN_VAR_INFO,

    };


    //Built-in function IDs
    enum FunctionTypes
    {
        FUNC_ABS,
        FUNC_BIN,
        FUNC_CHR,
        FUNC_DICT,
        FUNC_DIVMOD,
        FUNC_HEX,
        FUNC_INPUT,
        FUNC_INT,
        FUNC_LEN,
        FUNC_LIST,
        FUNC_MAX,
        FUNC_MIN,
        FUNC_OCT,
        FUNC_ORD,
        FUNC_PRINT,
        FUNC_RANGE,
        FUNC_SET,
        FUNC_SORTED,
        FUNC_STR,
        FUNC_TUPLE, 
    };

    //Execution
    py_error_t py_run(uint8_t *bytecode);

#endif
