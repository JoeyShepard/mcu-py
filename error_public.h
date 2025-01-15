#ifndef __GUARD_MCU_PY_ERROR_PUBLIC
    #define __GUARD_MCU_PY_ERROR_PUBLIC

    //Constants
    enum py_error_types
    {
        PY_ERROR_NONE,
        PY_ERROR_UNINITIALIZED,     //Python system not initialized
        PY_ERROR_UNDEFINED,         //Symbol in source undefined
        PY_ERROR_OUT_OF_MEM,        //Out of heap memory passed in during initialization
        PY_ERROR_INPUT,             //Invalid input such as character or invalid hex
        PY_ERROR_SYNTAX,            //Syntax error in source
        PY_ERROR_INTERNAL,          //Should never happen - problem is not from user's source 
        PY_ERROR_STACK_UNDERFLOW,   
        PY_ERROR_ELEMENT_OVERFLOW,  //More elements in list/dict/tuple in source than meta-data can hold
        PY_ERROR_MISSING_ARG,       //Missing func arg since default args not allowed ie f(x,)
        PY_ERROR_TOO_MANY_ARGS      //Functions limited to 255 args
    };

#endif
