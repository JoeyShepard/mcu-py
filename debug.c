//Various debug functions including error messages which aren't included in final version
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "debug.h"
#include "globals.h"

//structs
struct DebugLookup
{
    const char *key;
    const char **value;
};

//Globals
//=======
bool debug_enabled;

static char debug_log_data[DEBUG_LOG_SIZE];

//execute.c
static const char *debug_interpreter_state[]=
{
    "STATE_NONE",
    "STATE_BEGIN",
    "STATE_LPAREN",
    "STATE_LSBRACKET",
    "STATE_LCBRACKET",
    "STATE_VAL",
    "STATE_REQ",
    "STATE_INV_NEG_PLUS",
    "STATE_NOT",
    "STATE_ERROR"
};

static const char *debug_symbol_type[]=
{
    "SYMBOL_NONE",
    "SYMBOL_ALPHA",
    "SYMBOL_NUM",
    "SYMBOL_ZERO", 
    "SYMBOL_HEX_PRE",
    "SYMBOL_HEX",
    "SYMBOL_STRING", 
    "SYMBOL_COMMENT",
    "SYMBOL_SPACE",
    "SYMBOL_OP",
    "SYMBOL_ERROR",
    "SYMBOL_END_LINE",
    "SYMBOL_END_ALL",
    //Don't remove! Marks last entry
    ""
};

static const char *debug_symbol_input_type[]=
{
    "INPUT_A_F",
    "INPUT_G_Z_",
    "INPUT_X",
    "INPUT_ZERO",
    "INPUT_NUM",
    "INPUT_QUOTE",
    "INPUT_SPACE",
    "INPUT_END_LINE",
    "INPUT_END_ALL",
    "INPUT_OTHER",
    "INPUT_HASH",
    "INPUT_OP",
    "INPUT_ERROR",
    //Don't remove! Marks last entry
    ""
};

static const char *debug_object_type[]=
{
    "OBJECT_TEMP",
    "OBJECT_INT",
    "OBJECT_STRING",
    "OBJECT_VAR",    
    "OBJECT_FUNC",   
    "OBJECT_BUILTIN",
    "OBJECT_LIST",
    "OBJECT_DICT",
    "OBJECT_TUPLE",
    "OBJECT_ITER",   
    "OBJECT_BOOL",
    "OBJECT_NONE",   
    "OBJECT_FREE",   
    "OBJECT_FLOAT",  
    "OBJECT_SET",     
    //Don't remove! Marks last entry
    ""
};

static const char *debug_token[]=
{
    "TOKEN_NONE",
    //Operators start here
    "TOKEN_LPAREN",
    "TOKEN_LSBRACKET",
    "TOKEN_LCBRACKET",
    "TOKEN_RPAREN",
    "TOKEN_RSBRACKET",
    "TOKEN_RCBRACKET",
    "TOKEN_COLON",
    "TOKEN_COMMA",
    "TOKEN_EQ",
    "TOKEN_EXP_EQ",
    "TOKEN_MUL_EQ",
    "TOKEN_DIV_EQ",
    "TOKEN_MOD_EQ",
    "TOKEN_ADD_EQ",
    "TOKEN_SUB_EQ",
    "TOKEN_LSHIFT_EQ",
    "TOKEN_RSHIFT_EQ",
    "TOKEN_AND_EQ",
    "TOKEN_XOR_EQ",
    "TOKEN_OR_EQ",
    "TOKEN_EXP",
    "TOKEN_INVERT",
    "TOKEN_NEG",
    "TOKEN_PLUS",
    "TOKEN_MUL",
    "TOKEN_DIV",
    "TOKEN_MOD",
    "TOKEN_ADD",
    "TOKEN_SUB",
    "TOKEN_LSHIFT",
    "TOKEN_RSHIFT",
    "TOKEN_AND",
    "TOKEN_XOR",
    "TOKEN_OR",
    "TOKEN_IN",
    "TOKEN_NOT_IN",
    "TOKEN_LT",
    "TOKEN_LE",
    "TOKEN_GT",
    "TOKEN_GE",
    "TOKEN_NE",
    "TOKEN_EQ_EQ",
    "TOKEN_EXCLAM",
    "TOKEN_BOOL_NOT",
    "TOKEN_BOOL_AND",
    "TOKEN_BOOL_OR",
    //Tokens below here are bytecode - saves space to reuse token range
    "TOKEN_INT8",
    "TOKEN_INT16",
    "TOKEN_INT32",
    "TOKEN_STRING",
    "TOKEN_RETURN",
    "TOKEN_FUNC",
    "TOKEN_TUPLE",
    "TOKEN_LIST",
    "TOKEN_DICT",
    "TOKEN_SET",
    "TOKEN_NONE_OBJ",
    "TOKEN_SLICE_INDEX",
    "TOKEN_INDEX",
    //Tokens starting here are for storing things like variable names on stack during compilation
    "TOKEN_VAR_NAME",


    //Don't remove! Marks last entry
    ""
};


static const char *debug_error[]=
{
    "PY_ERROR_NONE",
    "PY_ERROR_UNINITIALIZED",
    "PY_ERROR_UNDEFINED",
    "PY_ERROR_OUT_OF_MEM",
    "PY_ERROR_INPUT",
    "PY_ERROR_SYNTAX",
    "PY_ERROR_INTERNAL",
    "PY_ERROR_STACK_UNDERFLOW",
    "PY_ERROR_ELEMENT_OVERFLOW",
    "PY_ERROR_MISSING_ARG",
    "PY_ERROR_TOO_MANY_ARGS",

    //Don't remove! Marks last entry
    ""
};


static struct DebugLookup debug_lookup[]=
{
    {"interpreter state",   debug_interpreter_state},
    {"symbol type",         debug_symbol_type},
    {"symbol input type",   debug_symbol_input_type},
    {"object type",         debug_object_type},
    {"token",               debug_token},
    {"error",               debug_error},
    //Don't remove! Marks end of list.
    {"",0}
};

//Functions
const char *debug_value(const char *group, uint16_t value)
{
    const char **debug_messages=NULL;

    uint16_t index=0;
    while(true)
    {
        //Found end of list?
        if (debug_lookup[index].key[0]==0) break;
        
        //Found key?
        if (!strcmp(group,debug_lookup[index].key))
        {
            //Key found - return
            debug_messages=debug_lookup[index].value;
            break;
        }
        index++;
    }

    if (debug_messages==NULL)
    {
       printf("Error: debug type not found for debug_value - '%s'\n",group);
       exit(1);
    }

    /*
    //Figure out which group of debug messages to use
    if (!strcmp(group,"symbol type")) debug_messages=debug_symbol_type;
    else if (!strcmp(group,"symbol input type")) debug_messages=debug_symbol_input_type;
    else
    {
       printf("Error: debug type not found for debug_value - '%s'\n",group);
       exit(1);
    }
    */

    //Make sure index into debug messages is not past end
    for (int i;i<=value;i++)
    {

        /*
        printf("Testing - %d %d ",i,value);
        printf("%s ",debug_messages[i]);
        printf("%c \n",debug_messages[i][0]);
        */
        

        if (debug_messages[i][0]==0)
        {
            printf("Error: value %d out of range for debug type '%s' in debug_value.\n",value,group);
            exit(1);
        } 
    }

    return debug_messages[value];
}

//Wrapper for printf to hide debug messages when debugging turned off
int debug(const char *format,...)
{
    if (debug_enabled)
    {
        va_list args;
        va_start(args, format);
        vprintf(format,args);
        va_end(args);
    }
    return 0;
}

int debug_key()
{
    if (debug_enabled)
        return getchar();
    else
        return 0;
}

int debug_log(const char *msg)
{
    size_t log_len=strlen(debug_log_data);
    size_t msg_len=strlen(msg);

    if (log_len+msg_len+1 >= DEBUG_LOG_SIZE)
    {
        printf("Error: ran out of log memory. ");
        printf("Message: %s\n",debug_log_data);
        exit(1);
    }
    
    strcpy(debug_log_data+log_len,msg);
    return 0;
}

int debug_log_int(uint32_t num)
{
    //Max size of uint32_t and terminating 0 is 11 
    char buffer[11]; 
    sprintf(buffer,"%lu",(unsigned long)num);

    size_t log_len=strlen(debug_log_data);
    size_t num_len=strlen(buffer);

    if (log_len+num_len+1 >= DEBUG_LOG_SIZE)
    {
        printf("Error: ran out of log memory. ");
        printf("Message: %s\n",debug_log_data);
        exit(1);
    }
    
    strcpy(debug_log_data+log_len,buffer);
    return 0;
}

int debug_log_hex(uint32_t num)
{
    //Max size of hex (including 0x) uint32_t and terminating 0 is 11 
    char buffer[11]; 
    sprintf(buffer,"0x%lX",(unsigned long)num);

    size_t log_len=strlen(debug_log_data);
    size_t num_len=strlen(buffer);

    if (log_len+num_len+1 >= DEBUG_LOG_SIZE)
    {
        printf("Error: ran out of log memory. ");
        printf("Message: %s\n",debug_log_data);
        exit(1);
    }
    
    strcpy(debug_log_data+log_len,buffer);
    return 0;
}

int debug_print_log()
{
    //Limit output to size of debug_log_data which is defined as DEBUG_LOG_SIZE
    //Format needs 6 characters or so plus one for each digit of size, so 16 is up to 10MB
    char format[16];
    sprintf(format,"%%.%ds\n",DEBUG_LOG_SIZE);
    printf(format,debug_log_data);
    return 0;
}

int debug_reset_log()
{
    debug_log_data[0]=0;
    return 0;
}

int debug_stack()
{
    debug("Stack: %d bytes\n",py_mem_size-py_sp);
    if (py_sp_count>0)
    {
        uint16_t count=py_sp_count;
        uint8_t *ptr=py_tos;
        while(count>0)
        {
            uint8_t offset=0;
            debug(" %.2d: %.2X",py_sp_count-count,*ptr);
            switch (*ptr)
            {
                case TOKEN_LPAREN:
                case TOKEN_LSBRACKET:
                case TOKEN_LCBRACKET:
                    offset=2;
                    break;
            }
            for (int i=0;i<offset;i++)
            {
                debug(" %.2X",*(ptr+i+1));
            }

            uint8_t padding=16;
            for (int i=0;i<padding-7-offset*3;i++) debug(" ");
            debug(" %s\n",debug_value("token",*ptr));
            
            ptr+=offset+1;
            count--;
        }
    }
   

    return 0;
}
