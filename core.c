#include <stdio.h>

#include <stdbool.h>
#include <stdint.h>
#include "core.h"
#include "error.h"
#include "globals.h"

//Initialization
//==============
py_error_t py_init(uint8_t *mem, uint16_t size, void (*error_func)(uint8_t, uint16_t))
{
    py_settings.mem=mem;
    py_settings.initialized=true;

    py_mem_size=size;
    py_error_func_ptr=error_func;
    py_sp=size;
    py_sp_count=0;
    *(uint16_t*)py_heap_begin=0;  //Linked list
    py_heap_ptr=PY_MEM_HEAP_BEGIN;

    return py_error_set(PY_ERROR_NONE,0);
}


//Stack
//=====
//Could combine with general purpose multi-byte push in execute.c but speed boost should be worth increased size
py_error_t py_push(struct StackItem item)
{
    if (sizeof(struct StackItem)>py_free) return py_error_set(PY_ERROR_OUT_OF_MEM,0);

    py_sp-=sizeof(struct StackItem);
    *(struct StackItem*)py_tos=item;
    py_sp_count++;

    return PY_ERROR_NONE;
}


//Memory management
//=================
uint16_t py_allocate(uint16_t size)
{
    size+=sizeof(uint16_t);     //Two extra bytes for offset to next item
    if (size>py_free)
    {
        py_error_set(PY_ERROR_OUT_OF_MEM,0);
        return 0;
    }
    *(uint16_t*)(py_heap_current)=size;
    uint16_t ret_val=py_heap_ptr;
    py_heap_ptr+=size;
    *(uint16_t*)(py_heap_current)=0;
    return ret_val;
}


