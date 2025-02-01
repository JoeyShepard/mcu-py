#include <stdint.h>
#include "core_public.h"
#include "tokens.h"

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

    //Execution
    py_error_t py_run(uint8_t *bytecode);

#endif
