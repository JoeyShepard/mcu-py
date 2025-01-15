#include <stdint.h>
#include "core_public.h"

#ifndef __GUARD_MCU_PY_CORE_PRIVATE
    #define __GUARD_MCU_PY_CORE_PRIVATE
   
    //Aliases into allocated memory
    //=============================
    //(values not exposed to user can be stored in allocated memory)
    //Better to store there so allocated memory can be reused by C
    #define PY_MEM_SIZE                     0                                               
    #define PY_MEM_SIZE_OFFSET              (uint16_t)sizeof(uint16_t)
    #define PY_MEM_ERROR_FUNC_PTR           (PY_MEM_SIZE+PY_MEM_SIZE_OFFSET)
    #define PY_MEM_ERROR_FUNC_PTR_OFFSET    (uint16_t)sizeof(void *)
    #define PY_MEM_ERROR_NUM                (PY_MEM_ERROR_FUNC_PTR+PY_MEM_ERROR_FUNC_PTR_OFFSET)
    #define PY_MEM_ERROR_NUM_OFFSET         (uint16_t)sizeof(uint8_t)
    #define PY_MEM_SP                       (PY_MEM_ERROR_NUM+PY_MEM_ERROR_NUM_OFFSET)
    #define PY_MEM_SP_OFFSET                (uint16_t)sizeof(uint16_t)
    #define PY_MEM_SP_COUNT                 (PY_MEM_SP+PY_MEM_SP_OFFSET)
    #define PY_MEM_SP_COUNT_OFFSET          (uint16_t)sizeof(uint8_t)
    #define PY_MEM_HEAP_PTR                 (PY_MEM_SP_COUNT+PY_MEM_SP_COUNT_OFFSET)
    #define PY_MEM_HEAP_PTR_OFFSET          (uint16_t)sizeof(uint16_t)

    //Heap starts after end of all other variables
    #define PY_MEM_HEAP_BEGIN       (PY_MEM_HEAP_PTR+PY_MEM_HEAP_PTR_OFFSET)

    #define py_mem_size             *(uint16_t*)(py_settings.mem+PY_MEM_SIZE)
    #define py_error_func_ptr       *(void **)(py_settings.mem+PY_MEM_ERROR_FUNC_PTR)
    #define py_error_func           ((void (*)(uint8_t,uint16_t))(py_error_func_ptr))
    #define py_error_num            py_settings.mem[PY_MEM_ERROR_NUM] 
    #define py_sp                   *(uint16_t*)(py_settings.mem+PY_MEM_SP)
    #define py_sp_count             (py_settings.mem[PY_MEM_SP_COUNT])
    #define py_tos                  (py_settings.mem+py_sp)
    //Relative to beginning of memory not to beginning of heap!
    #define py_heap_ptr             *(uint16_t*)(py_settings.mem+PY_MEM_HEAP_PTR)
    #define py_heap_current         (py_settings.mem+py_heap_ptr)
    //Absolute address of beginning of heap
    #define py_heap_begin           (py_settings.mem+PY_MEM_HEAP_BEGIN)
    #define py_free                 (py_sp-py_heap_ptr-(uint16_t)sizeof(uint16_t))

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


    //Objects
    //=======
    enum ObjectTypes
    {
        OBJECT_TEMP,        //Temp space for code executed outside of function
        OBJECT_INT,
        OBJECT_STRING,
        OBJECT_VAR,    
        OBJECT_FUNC,        //User-defined function
        OBJECT_BUILTIN,     //Functions defined by system (len, str, etc)
        OBJECT_LIST,
        OBJECT_DICT,
        OBJECT_TUPLE,
        OBJECT_ITER,        //Iterator - range, list, etc for for loop
        OBJECT_BOOL,
        OBJECT_NONE,        //Python None
        OBJECT_FREE,        //Free to garbage collect
        OBJECT_FLOAT,       //Not supported yet but go ahead and reserve ID
        OBJECT_SET          //Not supported yet but go ahead and reserve ID
    };


    //Parsing and execution tokens
    //============================
    enum TokenTypes
    {
        //Used in source parser in execute.c
        TOKEN_NONE,
        TOKEN_SYMBOL=TOKEN_NONE,    //Reuse in py_state_table when input is symbol instead of token
        
        //Paste token list from spreadsheet starting here:
    
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
        TOKEN_MAKE_LIST,
        TOKEN_RETURN,
        TOKEN_FUNC,
        //Tokens starting here are for storing things like variable names on stack during compilation
        TOKEN_VAR_NAME,

    };

    //Execution
    py_error_t py_run(uint8_t *bytecode);

#endif
