//#include <stdint.h>

#ifndef __GUARD_MCU_PY_ERROR
    #define __GUARD_MCU_PY_ERROR

    //Constants
    #define PY_ERROR_NONE               0
    #define PY_ERROR_UNINITIALIZED      1
    #define PY_ERROR_UNDEFINED          2

    #define MAX_SYMBOL_SIZE             10      //Symbols in source code
    
    //structs
    struct py_settings_struct
    {
        char initialized; //Global so initialized to 0
        unsigned char *mem;
    };
    
    //Aliases into allocated memory
    //=============================
    //(values not exposed to user can be stored in allocated memory)
    //Better to store there so allocated memory can be reused by C
    #define PY_MEM_SIZE             0                                               //2 bytes: size of passed in memory
    #define PY_MEM_ERROR_FUNC_PTR   (PY_MEM_SIZE+(int)sizeof(uint16_t))             //Depends on architecture
    #define PY_MEM_ERROR_NUM        (PY_MEM_ERROR_FUNC_PTR+(int)sizeof(void *))     //1 byte: error code
    #define PY_MEM_NEXT             (PY_MEM_ERROR_NUM+(int)sizeof(unsigned char))    

    #define _py_mem_size            *(uint16_t*)(py_settings.mem+PY_MEM_SIZE)
    #define _py_error_func_ptr      *(void **)(py_settings.mem+PY_MEM_ERROR_FUNC_PTR)
    #define _py_error_func          ((void (*)())(_py_error_func_ptr))
    #define py_error_num            py_settings.mem[PY_MEM_ERROR_NUM] 

    //Globals (see globals.c)
    extern struct py_settings_struct py_settings;

    //Globals (see tables.c)
    extern const char *py_commands;

    //Functions
    //=========
    //core.c
    unsigned char py_allocate(unsigned int size);
    unsigned char py_error_set(unsigned char error_num);
    unsigned char py_init(unsigned char *mem, unsigned int size, void (*error_func)());

    //execute.c
    unsigned char py_execute(const char *text);
    unsigned char _py_find_symbol(const char *text);

#endif
