#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "core.h"
#include "custom.h"
#include "debug.h"
#include "error.h"
#include "globals.h"

//Initialization
//==============
py_error_t py_init(uint8_t *mem, uint16_t size, void (*error_func)(uint8_t, uint16_t))
{
    //Make sure maximum symbol size defined in custom.h fits in one byte
    if (PY_MAX_SYMBOL_SIZE>=255) return PY_ERROR_MAX_SYMBOL_EXCEEDED;

    py=(struct py_struct *)mem;
    py_initialized=true;

    py->mem_size=size;
    py->error_func=error_func;
    py->sp=mem+size;
    py->sp_count=0;
    py->heap_begin=mem+sizeof(struct py_struct);
    *(uint16_t*)py->heap_begin=0;  //Linked list
    py->heap_ptr=py->heap_begin;

    //Add object to hold global variable values
    uint8_t *globals=py_allocate(0);
    uint8_t obj_type=OBJECT_GLOBAL_VALUES;
    py_append(globals,&obj_type,1);

    //Add object to hold global variable names
    globals=py_allocate(0);
    obj_type=OBJECT_GLOBAL_NAMES;
    py_append(globals,&obj_type,1);

    return py_error_set(PY_ERROR_NONE,0);
}


//Stack
//=====
py_error_t py_push(struct StackItem item)
{
    if (sizeof(struct StackItem)>py_free()) return py_error_set(PY_ERROR_OUT_OF_MEM,0);

    py->sp-=sizeof(struct StackItem);
    *(struct StackItem*)py->sp=item;
    py->sp_count++;

    return PY_ERROR_NONE;
}


//Memory management
//=================
uint8_t *py_allocate(uint16_t size)
{
    size+=sizeof(uint16_t);     //Two extra bytes for offset to next item
    if (size>py_free())
    {
        py_error_set(PY_ERROR_OUT_OF_MEM,0);
        return 0;
    }
    *(uint16_t*)(py->heap_ptr)=size;
    uint8_t *ret_val=py->heap_ptr;
    py->heap_ptr+=size;
    *(uint16_t*)(py->heap_ptr)=0;
    return ret_val;
}

py_error_t py_append(uint8_t *obj, const void *data, uint16_t size)
{
    //No need to add two bytes to size since end of list marker already exists
    if (size>py_free())
    {
        return py_error_set(PY_ERROR_OUT_OF_MEM,0);
    }
    uint8_t *data_dest=py->heap_ptr;
    for (uint16_t i=0;i<size;i++)
    {
        data_dest[i]=((uint8_t *)data)[i];
    }
    *(uint16_t *)(obj)+=size;
    py->heap_ptr+=size;
    *(uint16_t*)(py->heap_ptr)=0;
    return PY_ERROR_NONE;
}

uint16_t py_free()
{
    //Extra sizeof(uint16_t) for end address of heap linked list
    return (py->sp)-(py->heap_ptr)-sizeof(uint16_t);
}


//Execution
//=========
//bytecode - pointer to beginning of object which starts with:
//- uint16_t: distance to next object
//- uint8_t: enum ObjectTypes - should be OBJECT_CODE
py_error_t py_run(uint8_t *bytecode)
{
    //Debugging
    /*
    {
        for (int i=0;i<20;i++)
        {
            debug("%d\n",*(bytecode+i));
        }
        debug_key();
    }*/

    //Distance to next object
    uint16_t obj_size=*(uint16_t *)bytecode-sizeof(uint16_t);
    bytecode+=sizeof(uint16_t);

    //Debugging
    {
        debug("RUN: size:%u\n",obj_size);
        debug("%s\n",debug_value("object type",*bytecode));
    }
    //Debugging
    
    //Skip over object type
    bytecode+=sizeof(uint8_t);

    //Loop through bytecode
    uint8_t op;
    do 
    {
        op=*bytecode;
        bytecode++;

        //Debugging
        debug("%d - %s (",op,debug_value("token",op));
        //Debugging

        switch (op)
        {
            case TOKEN_EQ:
                break;
            case TOKEN_EXP_EQ:
                break;
            case TOKEN_MUL_EQ:
                break;
            case TOKEN_DIV_EQ:
                break;
            case TOKEN_MOD_EQ:
                break;
            case TOKEN_ADD_EQ:
                break;
            case TOKEN_SUB_EQ:
                break;
            case TOKEN_LSHIFT_EQ:
                break;
            case TOKEN_RSHIFT_EQ:
                break;
            case TOKEN_AND_EQ:
                break;
            case TOKEN_XOR_EQ:
                break;
            case TOKEN_OR_EQ:
                break;
            case TOKEN_EXP:
                break;
            case TOKEN_INVERT:
                break;
            case TOKEN_NEG:
                break;
            case TOKEN_PLUS:
                break;
            case TOKEN_MUL:
                break;
            case TOKEN_DIV:
                break;
            case TOKEN_MOD:
                break;
            case TOKEN_ADD:
                break;
            case TOKEN_SUB:
                break;
            case TOKEN_LSHIFT:
                break;
            case TOKEN_RSHIFT:
                break;
            case TOKEN_AND:
                break;
            case TOKEN_XOR:
                break;
            case TOKEN_OR:
                break;
            case TOKEN_IN:
                break;
            case TOKEN_NOT_IN:
                break;
            case TOKEN_LT:
                break;
            case TOKEN_LE:
                break;
            case TOKEN_GT:
                break;
            case TOKEN_GE:
                break;
            case TOKEN_NE:
                break;
            case TOKEN_EQ_EQ:
                break;
            case TOKEN_EXCLAM:
                break;
            case TOKEN_BOOL_NOT:
                break;
            case TOKEN_BOOL_AND:
                break;
            case TOKEN_BOOL_OR:
                break;
            //Tokens below here are bytecode - saves space to reuse token range
            case TOKEN_INT8:
                debug("%d",*bytecode);
                bytecode++;
                break;
            case TOKEN_INT16:
                debug("%d",*(uint16_t *)bytecode);
                bytecode+=2;
                break;
            case TOKEN_INT32:
                debug("%d",*(uint32_t *)bytecode);
                bytecode+=4;
                break;
            case TOKEN_STRING:
                debug("<<");
                uint16_t str_size=*(uint16_t *)bytecode;
                bytecode+=2;
                for (uint16_t i=0;i<str_size;i++)
                {
                    debug("%c",*bytecode);
                    bytecode++;
                }
                debug(">>");
                break;
            case TOKEN_RETURN:
                break;
            case TOKEN_FUNC:
                debug("%d",*bytecode);
                bytecode++;
                break;
            case TOKEN_TUPLE:
                debug("%d",*(uint16_t *)bytecode);
                bytecode+=2;
                break;
            case TOKEN_LIST:
                debug("%d",*(uint16_t *)bytecode);
                bytecode+=2;
                break;
            case TOKEN_DICT:
                debug("%d",*(uint16_t *)bytecode);
                bytecode+=2;
                break;
            case TOKEN_SET:
                debug("%d",*(uint16_t *)bytecode);
                bytecode+=2;
                break;
            case TOKEN_NONE_OBJ:
                break;
            case TOKEN_SLICE_INDEX:
                break;
            case TOKEN_INDEX:
                break;
            case TOKEN_BUILTIN_FUNC:
                debug("%s",debug_value("builtin",*bytecode));
                bytecode++;
                break;
            case TOKEN_GLOBAL:
                debug("%d",*bytecode);
                bytecode++;
                break;
            case TOKEN_LOCAL:
                debug("%d",*bytecode);
                bytecode++;
                break;
            case TOKEN_ATTRIBUTE:
                uint8_t attribute_len=*bytecode;
                bytecode++;
                debug_cstr(bytecode,attribute_len);
                bytecode+=attribute_len;
                break;

                //TODO:
                //put pointer to builtin in stack item if running on architecture where it fits
                //if with sizeof will optimize out

                break;
        } //switch(op)

        debug(")\n");

    } while (op!=TOKEN_RETURN);

    return PY_ERROR_NONE;
}
