#ifndef __GUARD_MCU_PY_ERROR_PUBLIC
    #define __GUARD_MCU_PY_ERROR_PUBLIC

    //Constants
    enum py_error_types
    {
        PY_ERROR_NONE,                  //No error - all good
        PY_ERROR_UNINITIALIZED,         //Python system not initialized
        PY_ERROR_UNDEFINED,             //Symbol in source undefined
        PY_ERROR_OUT_OF_MEM,            //Out of heap memory passed in during initialization
        PY_ERROR_INPUT,                 //Invalid input such as character or invalid hex
        PY_ERROR_SYNTAX,                //Syntax error in source
        PY_ERROR_INTERNAL,              //Should never happen - problem is not from user's source 
        PY_ERROR_STACK_UNDERFLOW,   
        PY_ERROR_ELEMENT_OVERFLOW,      //More elements in list/dict/tuple in source than meta-data can accomodate
        PY_ERROR_MISSING_ARG,           //Missing func arg since default args not allowed ie f(x,)
        PY_ERROR_TOO_MANY_ARGS,         //Functions limited to 255 args
        PY_ERROR_MAX_SYMBOL_EXCEEDED,   //PY_MAX_SYMBOL_SIZE defined in custom.h must be 255 or less
        PY_ERROR_SYMBOL_TOO_LARGE,      //Symbol exceeds size defined by PY_MAX_SYMBOL_SIZE in custom.h
    };

#endif
