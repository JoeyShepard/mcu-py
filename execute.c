#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "core.h"
#include "custom.h"
#include "debug.h"
#include "error.h"
#include "execute.h"
#include "globals.h"
#include "tables.h"

//Compare counted strings
//Ok to pass in null-terminated string here as long as len1 and len2 don't include null terminator
static bool py_cstrcmp(const char *str1, uint16_t len1, const char *str2, uint16_t len2)
{
    //Counted strings must match in length - mismatch
    if (len1!=len2) return false;
    
    for (uint16_t i;i<len1;i++)
    {
        //Hit null terminator so return mismatch since at least one string is counted and therefore not null terminated
        //Othwerwise, ok to pass in null-terminated string as long as len1 and len2 don't include null terminator
        if ((!str1[i])||(!str2[i])) return false;

        //Mismatch found
        if (str1[i]!=str2[i]) return false;
    }
    //All characters match
    return true;
}

//TODO: used?
//Check if character is in string
static bool py_strchr(const char *str, char ch, size_t size)
{
    for (uint16_t i=0;i<size;i++)
    {
        if (str[i]==ch) return true;
    }
    return false;
}

//Classify character of source
static uint8_t py_classify_input(const char input_char)
{ 
    //Error on input characters 127-255
    if (input_char>PY_CHAR_LAST_VALID) return INPUT_ERROR;

    //Classify input character
    uint8_t char_type=0;
    uint16_t total=0;
    const uint8_t *table=py_state_chars;
    while(1)
    {
        total+=(*table)&PY_CHAR_MASK;   //Count of range in lower 4 bits of table entry
        if (input_char<total)
        {
            //Character class found
            return (*table)>>4;         //Character class in upper 4 bits of table entry
            break;
        }
        table++;
    }
}

static uint8_t py_lookup_op(char op)
{
    const char *op_list=py_op_lookup_keys;
   
    while (*op_list)
    {
        if (*op_list==op)
        {
            return py_op_lookup_vals[op_list-py_op_lookup_keys];
        }
        op_list++;
    }
}

static uint8_t py_next_symbol(const char **text, uint16_t *len)
{
    bool processing=true;
    uint8_t symbol_state=SYMBOL_NONE;

    //Length of symbol - returned to caller
    *len=0;

    while (processing)
    {
        //Get next character of symbol
        unsigned char input_char=*(*text+*len);

        //Classify next character for input into symbol state machine
        uint8_t char_type=py_classify_input(input_char);

        //Error on characters marked error in character table
        if (char_type==INPUT_ERROR) return SYMBOL_ERROR;

        //Look up symbol state given current state and input character
        uint8_t new_symbol_state=py_symbol_state[char_type][symbol_state/2];
        if ((symbol_state&1)==0) new_symbol_state&=PY_SYMBOL_MASK;
        else new_symbol_state>>=4;

        //Error if lookup table returns error state
        if (new_symbol_state==SYMBOL_ERROR) return SYMBOL_ERROR;

        //Process new symbol state
        if (new_symbol_state!=symbol_state)
        {
            //Symbol state of new character differs from current state
            if (symbol_state==SYMBOL_NONE)
            {
                //Found beginning of symbol
                *len=1;
                symbol_state=new_symbol_state;

                //Return immediately for states like END_LINE, END_ALL
                if (new_symbol_state>=SYMBOL_RETURN)
                {
                    return symbol_state;
                }
            }
            else
            {
                //State changed after beginning of symbol - probably ready to return
                if ((new_symbol_state==SYMBOL_HEX_PRE)||(new_symbol_state==SYMBOL_HEX))
                {
                    //Account for a few exceptions - state change here is not end of symbol so keep going
                    (*len)++;
                    symbol_state=new_symbol_state;
                }
                else
                {
                    //End of symbol found - return
                    if (symbol_state==SYMBOL_ZERO)
                    {
                        //Correction for 0 when it's a number rather than hex prefix
                        return SYMBOL_NUM;
                    }
                    else if (symbol_state==SYMBOL_STRING)
                    {
                        //Extend symbol to include closing quote
                        (*len)++;
                    }
                    return symbol_state;
                }
            }
        }
        else
        {
            //New symbol state is same as old
            if (symbol_state==SYMBOL_NONE) 
            {
                //Beginning of symbol not found yet - advance source pointer
                (*text)++; 
            }
            else 
            {
                //Beginning of symbol found previously - advance length but not source pointer
                (*len)++;
            }
        }
    }
}

/* UNFINISHED! */
static int16_t py_find_symbol(const char *symbol_begin,uint8_t symbol_len)
{
    //Find length of symbol
    const char *symbol_len_ptr=symbol_begin;
    
    //Look for symbol in tables
    const char *commands_ptr=py_commands;
    uint16_t command_id=0;
    while (*commands_ptr)
    {
        //Compare counted string lengths
        if (*commands_ptr!=symbol_len)
        {
            //Length of symbol doesn't match length of command - skip
            commands_ptr+=*commands_ptr+1;
            command_id++;
        }
        else
        {
            //TODO: replace with py_cstrcmp

            //Length of symbol matches length of command - compare letters
            bool match=true;
            for (int i=0;i<symbol_len;i++)
            {
                if (commands_ptr[i+1]!=symbol_begin[i])
                {
                    //Words don't match - skip command
                    match=false;
                    commands_ptr+=symbol_len+1;
                    command_id++;
                    break;
                }
            }
            //Loop finished - symbol matches command. Return ID of command.
            if (match) return command_id;
        }
    }

    //TODO: change return from -1 since type is uint8_t

    //Symbol not found
    return -1;
}

//Size on stack of any extra data after a token
static uint8_t py_token_size(uint8_t token)
{
    switch (token)
    {
        case TOKEN_FUNC:
            return sizeof(uint8_t);
        case TOKEN_LPAREN:
        case TOKEN_LSBRACKET:
        case TOKEN_LCBRACKET:
            return sizeof(uint16_t);
        default:
            return 0;
    }
}

static py_error_t py_custom_push(const void *data, uint16_t data_size)
{
    if (py_free<data_size) return py_error_set(PY_ERROR_OUT_OF_MEM,0);

    py_sp-=data_size;
    for (uint16_t i=0;i<data_size;i++)
    {
        *(py_tos+i)=*(uint8_t *)(data+i);
    }
    py_sp_count++;

    return PY_ERROR_NONE;
}

static bool py_custom_pop(uint8_t *data, uint8_t pop_class)
{
    //Find next object on stack that matches desired class
    uint8_t *obj=py_peek_stack(NULL,pop_class);

    //No matching object found - return
    if (obj==NULL) return false;

    //Copy found object
    uint8_t obj_size=py_token_size(data[0])+1;
    for (int16_t i=0;i<obj_size;i++)
    {
        data[i]=obj[i];
    }

    //Remove object from stack
    py_remove_stack(obj);

    //Matching object found - return true
    return true;
}

static uint8_t *py_peek_stack(uint8_t *obj, uint8_t pop_class)
{
    uint8_t *new_obj;
    if (obj==NULL)
    {
        //Haven't started searching yet so start at top of stack
        new_obj=py_tos;
    }
    else new_obj=obj;

    //Search through stack
    while (1)
    {
        //Pointer reached end of stack - done searching
        if (new_obj==py_settings.mem+py_mem_size) return NULL;

        //TODO: use token ranges instead of switch? look up table?
        //Return address if token correct type
        switch (pop_class)
        {
            case POP_CLOSING_FOUND:
                //Closing bracket hit - all operators including ( { [
                //Fallthrough
            case POP_END_LINE:
                //End of line - all operators including ( { [
                switch (*new_obj)
                {
                    case TOKEN_VAR_NAME:
                        //Not looking for these - keep searching
                        break;
                    default:
                        //All others are a match
                        if (new_obj!=obj)
                        {
                            //Only return if different from starting object
                            return new_obj;
                        }
                }
                break;
            case POP_OPERATORS:
                //Shunting Yard - operators not including ( { [
                switch (*new_obj)
                {
                    case TOKEN_VAR_NAME:
                        //Not looking for these - keep searching
                        break;
                    case TOKEN_LPAREN:
                    case TOKEN_LSBRACKET:
                    case TOKEN_LCBRACKET:
                    case TOKEN_COLON:
                        //Done searching
                        return NULL;
                        break;
                    default:
                        //All others are a match
                        if (new_obj!=obj)
                        {
                            //Only return if different from starting object
                            return new_obj;
                        }
                }
                break;
            case POP_OPENINGS:
                //Find opening brackets - ( { [
                switch (*new_obj)
                {
                    case TOKEN_LPAREN:
                    case TOKEN_LSBRACKET:
                    case TOKEN_LCBRACKET:
                        //Only these are a match
                        if (new_obj!=obj)
                        {
                            //Only return if different from starting object
                            return new_obj;
                        }
                    default:
                        //Not looking for these - keep searching
                        break;
                }
                break;
            default:
                return NULL;
        }

        //Advance pointer
        new_obj+=py_token_size(*new_obj)+1;
    }
}

static uint8_t *py_remove_stack(uint8_t *obj)
{
    uint16_t obj_size=py_token_size(*obj)+1;
    for (uint8_t *copy_ptr=obj-1;copy_ptr>=py_tos;copy_ptr--)
    {
        *(copy_ptr+obj_size)=*copy_ptr; 
    }
    py_sp_count--;
    py_sp+=obj_size;
    return obj+obj_size;
}

static uint8_t py_find_precedence(uint8_t token)
{
    const char *table_ptr=py_op_precedence;
    uint8_t token_max=0;
    uint8_t precedence=0;

    while(1)
    {
        token_max+=*table_ptr;
        if (token<=token_max) return precedence;
        else
        {
            table_ptr++;
            precedence++;
        }
    }
}

static py_error_t py_append(uint8_t *obj, const void *data, uint16_t size)
{
    //No need to add two bytes to size since end of list marker already exists
    if (size>py_free)
    {
        return py_error_set(PY_ERROR_OUT_OF_MEM,0);
    }
    uint8_t *data_dest=py_heap_current;
    for (uint16_t i=0;i<size;i++)
    {
        *data_dest=*(uint8_t *)data;
        data++;
        data_dest++;
    }
    *(uint16_t *)(obj)+=size;
    py_heap_ptr+=size;
    *(uint16_t*)(py_heap_current)=0;
    return PY_ERROR_NONE;
}

//Execute Python source passed in as a string
py_error_t py_execute(const char *text)
{
    //Can't set error_num if uninitialized since stored in passed-in memory so return error code but don't set py_error_num
    if (!py_settings.initialized) return PY_ERROR_UNINITIALIZED;

    //Local variables reset for each line
    struct SymbolType
    {
        uint8_t symbol;
        uint8_t token;
    } symbol_queue[3];
    uint8_t interpreter_state;
    uint8_t last_interpreter_state;
    uint8_t exec_flags=FLAG_RESET_STATE;
    uint16_t line_number=0;
    uint8_t *compile_target=NULL;
    uint16_t top_comma_count;

    //Local variables used temporarily in code
    uint16_t symbol_len;
    const uint8_t *table_ptr;       //Used for state machine and precedence lookups
    uint8_t append_data[2];         //Buffer for data passed to py_append
    uint8_t token_max;
    uint8_t stack_token_precedence;
    uint8_t *obj_ptr;
    //TODO: explain size. should be big enough for biggest possible.
    //      3 - token and 16 bit metadata for ( [ {
    uint8_t stack_buffer[3];

    do
    {
        //Reset state at beginning of every line
        if (exec_flags&FLAG_RESET_STATE)
        {
            symbol_queue[0].symbol=SYMBOL_NONE;
            symbol_queue[1].symbol=SYMBOL_NONE;
            symbol_queue[2].symbol=SYMBOL_NONE;
            interpreter_state=STATE_BEGIN;
            last_interpreter_state=STATE_NONE;
            exec_flags&=~FLAG_RESET_STATE;
            line_number++;
            top_comma_count=0;
            if (compile_target==NULL)
            {
                //Allocate temp mem for code outside of function
                compile_target=py_allocate(0);
                append_data[0]=OBJECT_TEMP;
                py_append(compile_target,append_data,1);
            }
        }

        //Get next symbol in source
        symbol_queue[0].symbol=py_next_symbol(&text,&symbol_len);
        if (symbol_queue[0].symbol==SYMBOL_ERROR) return py_error_set(PY_ERROR_INPUT,line_number);

        //Reclassify operator words to SYMBOL_OP: and, in, or, not
        if (symbol_queue[0].symbol==SYMBOL_ALPHA)
        {
            for (uint16_t i=0;i<PY_OP_LOOKUP_KEYS_LONG_COUNT;i++)
            {
                //Compare symbol to symbol in list of word operators
                //Note py_cstrcmp is for counted strings but works with null-terminated strings too
                if (py_cstrcmp(text,symbol_len,py_op_lookup_keys_long[i]+1,py_op_lookup_keys_long[i][0]))
                {
                    //Symbol found
                    symbol_queue[0].symbol=SYMBOL_OP;
                    symbol_queue[0].token=py_op_lookup_vals_long[i];

                    //Account for 'not in'
                    if ((symbol_queue[0].token==TOKEN_IN)&&
                        (symbol_queue[1].symbol==SYMBOL_SPACE)&&
                        (symbol_queue[2].symbol==SYMBOL_OP)&&
                        (symbol_queue[2].token==TOKEN_BOOL_NOT))
                    {
                        symbol_queue[0].token=TOKEN_NOT_IN;
                        symbol_queue[1].symbol=SYMBOL_NONE;
                        symbol_queue[2].symbol=SYMBOL_NONE;
                    }
                    break;
                }
            }
        }
        else if (symbol_queue[0].symbol==SYMBOL_OP)
        {
            //Look up token for single-character operators (word operators handled above)
            symbol_queue[0].token=py_lookup_op(*text);
        }

        //Recognize combined operators like >> and +=
        if ((symbol_queue[0].symbol==SYMBOL_OP)&&(symbol_queue[1].symbol==SYMBOL_OP))
        {
            for (uint16_t i=0;i<PY_OP_COMBOS_COUNT;i++)
            {
                //TODO: magic numbers
                if (py_op_combos[i][0]==symbol_queue[1].token)
                {
                    if (py_op_combos[i][1]==symbol_queue[0].token)
                    {
                        //Combination found
                        symbol_queue[0].token=py_op_combos[i][2];
                        symbol_queue[1]=symbol_queue[2];
                        symbol_queue[2].symbol=SYMBOL_NONE;
                        break;
                    }
                }
            }
        }

        //TODO: size payoff?
        //Mark values (alpha, num, etc) for processing - simplifies logic below
        switch (symbol_queue[0].symbol)
        {
            case SYMBOL_ALPHA:
            case SYMBOL_HEX:
            case SYMBOL_NUM:
            case SYMBOL_STRING:
                exec_flags|=FLAG_VALUE;
                break;
            default:
                exec_flags&=~FLAG_VALUE;
                break;
        }

        //Advance interpreter state
        uint8_t input_token, input_symbol;
        for (int index=2;index>=0;index--)
        { 
            //Always process the symbol at position 2 which is the end of the queue since it's dropped below
            //Process postion 1 and 0 if at end of line/source or symbol encountered
            if ((index==2)||
            (symbol_queue[0].symbol==SYMBOL_END_LINE)||
            (symbol_queue[0].symbol==SYMBOL_END_ALL)||
            (exec_flags&FLAG_VALUE))
            {
                input_symbol=symbol_queue[index].symbol;
                input_token=symbol_queue[index].token;
                exec_flags|=FLAG_ADVANCE_STATE;
            }
            else exec_flags&=~FLAG_ADVANCE_STATE;

            //Look up new interpreter state in table
            if (exec_flags&FLAG_ADVANCE_STATE)
            {
                switch (input_symbol)
                {
                    case SYMBOL_ALPHA: 
                    case SYMBOL_NUM: 
                    case SYMBOL_HEX: 
                    case SYMBOL_STRING: 
                        //Interpreter state machine handles all tokens so these four symbols are treated as a token
                        input_token=TOKEN_SYMBOL;
                        //Fallthrough
                    case SYMBOL_OP:
                        //Look up next state from table 
                        if ((interpreter_state==STATE_BEGIN)&&(input_token==TOKEN_RPAREN))
                        {
                            //Exception to look up table - ) after , for (1,)
                            last_interpreter_state=interpreter_state;
                            interpreter_state=STATE_VAL;
                        }
                        table_ptr=py_state_table;
                        token_max=*table_ptr;
                        while(1)
                        {
                            if (input_token<token_max)
                            {
                                //Table entry found - process 
                                last_interpreter_state=interpreter_state;
                                uint8_t rep_match1=*(table_ptr+2)&0xF;
                                uint8_t rep_match2=*(table_ptr+2)>>4;
                                uint8_t default_val=*(table_ptr+1)&0xF;
                                uint8_t rep_val=*(table_ptr+1)>>4;
                                if (interpreter_state==rep_match1) interpreter_state=rep_val;
                                else if (interpreter_state==rep_match2) interpreter_state=rep_val;
                                else interpreter_state=default_val;
                                
                                //Handle syntax errors
                                if (interpreter_state==STATE_ERROR) return py_error_set(PY_ERROR_INPUT,line_number);

                                //Account for unary + and -
                                if (interpreter_state==STATE_INV_NEG_PLUS)
                                {
                                    if (input_token==TOKEN_ADD) input_token=TOKEN_PLUS;
                                    if (input_token==TOKEN_SUB) input_token=TOKEN_NEG;
                                }
                                break;
                            }
                            else
                            {
                                //No match found - advance to next entry
                                table_ptr+=STATE_ENTRY_SIZE;
                                token_max+=*table_ptr;
                            }
                        }
                        break;
                }
             
                //Values ready to be compiled, operators ready to be processed by Shunting Yard
                int32_t num=0;
                const char *num_ptr=text;
                switch (input_symbol)
                {
                    case SYMBOL_END_LINE:
                        //Done processing line
                        exec_flags|=FLAG_RESET_STATE;
                        //Fallthrough
                    case SYMBOL_END_ALL:
                        //Make sure expression didn't end with calculation pending like 2+
                        if (interpreter_state==STATE_REQ) return py_error_set(PY_ERROR_SYNTAX,line_number);
                       
                        //Compile pending operators
                        while(1)
                        {
                            if (py_custom_pop(stack_buffer, POP_END_LINE)==false)
                            {
                                //Reached end of stack - no more operators left to pop and compile
                                break;
                            }
                            if (py_find_precedence(*stack_buffer)==PREC_OPENING)
                            {
                                //If there is a ( [ or { at the end, error on missing } ] or )
                                return py_error_set(PY_ERROR_SYNTAX,line_number);
                            }

                            //Compile operator
                            py_append(compile_target,stack_buffer,1);
                        }
                        if (input_symbol==SYMBOL_END_ALL)
                        {
                            append_data[0]=TOKEN_RETURN;
                            py_append(compile_target,append_data,1);
                            exec_flags|=FLAG_DONE;
                        }
                        break;
                    case SYMBOL_ALPHA:
                        break;
                    case SYMBOL_HEX:
                        num_ptr+=2;
                        for (int i=0;i<symbol_len-2;i++)
                        {
                            num<<=4;
                            if ((*num_ptr>='0')&&(*num_ptr<='9')) num+=(*num_ptr)-'0';
                            else if ((*num_ptr>='a')&&(*num_ptr<='z')) num+=(*num_ptr)-'a'+10;
                            else if ((*num_ptr>='A')&&(*num_ptr<='Z')) num+=(*num_ptr)-'A'+10;
                            num_ptr++;
                        }
                        //Fallthrough
                    case SYMBOL_NUM:
                        if (input_symbol==SYMBOL_NUM)
                        {
                            for (int i=0;i<symbol_len;i++)
                            {
                                num*=10;
                                num+=(*num_ptr)-'0';
                                num_ptr++;
                            }
                        }
                        
                        uint16_t data_size;
                        if ((num>=INT8_MIN)&&(num<=INT8_MAX))
                        {
                            append_data[0]=TOKEN_INT8;
                            data_size=1;
                        }
                        else if ((num>=INT16_MIN)&&(num<=INT16_MAX))
                        {
                            append_data[0]=TOKEN_INT16;
                            data_size=2;
                        }
                        else 
                        {
                            append_data[0]=TOKEN_INT32;
                            data_size=4;
                        }
                        py_append(compile_target,append_data,1);
                        py_append(compile_target,&num,data_size);
                        break;
                    case SYMBOL_STRING:
                        //Compile value
                        append_data[0]=TOKEN_STRING;
                        py_append(compile_target,append_data,1);
                        *(uint16_t *)append_data=symbol_len-2;          //-2 to length to discard quotes
                        py_append(compile_target,append_data,2);        //Write length of string
                        py_append(compile_target,text+1,symbol_len-2);  //Write string datae. +1 to start after first quote
                        break;
                    case SYMBOL_OP:
                        //Shunting yard algorithm
                        uint8_t input_token_precedence=py_find_precedence(input_token);
                        bool token_left_assoc;
                       
                        //Check for operators requiring special handling
                        if (input_token_precedence==PREC_OPENING)
                        {
                            //Opening parentheses or brackets: ( [ {

                            //Push token and 16 bit value for metadata
                            stack_buffer[0]=input_token;
                            if (last_interpreter_state==STATE_VAL) *(uint16_t *)(stack_buffer+1)=FLAG_FUNC_DEREF;
                            else *(uint16_t *)(stack_buffer+1)=0;
                            py_custom_push(stack_buffer,3);
                        }
                        else if (input_token_precedence==PREC_CLOSING)
                        {
                            //Closing parentheses or brackets: ) ] }

                            //Empty pending operators until opening ( [ { found
                            while(1)
                            {
                                if (py_custom_pop(stack_buffer,POP_CLOSING_FOUND)==false)
                                {
                                    //No matching bracket found! ie 1+2)
                                    return py_error_set(PY_ERROR_SYNTAX,line_number);
                                }
                                stack_token_precedence=py_find_precedence(stack_buffer[0]);
                                if (stack_token_precedence==PREC_OPENING)
                                {
                                    //Ensure ( matches ), [ matches ], { matches }
                                    if (input_token!=stack_buffer[0]+3)
                                    {
                                        return py_error_set(PY_ERROR_SYNTAX,line_number);
                                    }
                                    else
                                    {
                                        //Found match - figure out object type and element count
                                        uint16_t element_data=*(uint16_t *)(stack_buffer+1);
                                        if (stack_buffer[0]==TOKEN_LPAREN)
                                        {
                                            //Found ( so must be function, tuple, or expression
                                            if ((element_data&FLAG_COUNT_MASK)>0)
                                            {
                                                //At least one comma found so function or tuple
                                                if (element_data&FLAG_FUNC_DEREF)
                                                {
                                                    //Function
                                                    if (exec_flags&FLAG_COMMA_LAST)
                                                    {
                                                        //No support for default args so last arg can't be empty
                                                        return py_error_set(PY_ERROR_MISSING_ARG,0);
                                                    }
                                                    if ((element_data&FLAG_COUNT_MASK)>PY_FUNC_MAX_ARGS)
                                                    {
                                                        //Limit max function arg count so it fits in one byte
                                                        return py_error_set(PY_ERROR_TOO_MANY_ARGS,0);
                                                    }
                                                    stack_buffer[0]=TOKEN_FUNC;
                                                    stack_buffer[1]=element_data&0xFF;
                                                    py_append(compile_target,stack_buffer,2);
                                                }
                                                else
                                                {
                                                    //Tuple
                                                }
                                            }
                                            else
                                            {
                                                //Expression - no special processing
                                            }
                                        }
                                        else
                                        {
                                            //TODO: [ and {
                                        }
                                        break;
                                    }
                                }
                                else 
                                {
                                    //Compile operator
                                    py_append(compile_target,stack_buffer,1);
                                }
                            }
                        }
                        else if ((input_token_precedence==PREC_COMMA)||
                                (input_token_precedence==PREC_COLON))
                        {
                            //Compile pending operators
                            while(1)
                            {
                                if (py_custom_pop(stack_buffer, POP_OPERATORS)==false)
                                {
                                    //Reached end of stack - no more operators left to pop and compile
                                    break;
                                }

                                //Compile operator
                                py_append(compile_target,stack_buffer,1);
                            }

                            //Increase element count
                            if (input_token_precedence==PREC_COMMA)
                            {
                                //Find ( [ or { on stack and increase comma count in data stored on stack following it
                                uint16_t *element_count;
                                obj_ptr=py_peek_stack(NULL,POP_OPENINGS); 
                                if (obj_ptr==NULL) 
                                {
                                    //No ( [ { on stack so use count of top-level commas ie 1,2 without parentheses
                                    element_count=&top_comma_count;
                                }
                                else
                                {
                                    //Found ( [ { on stack - use count field of data following
                                    element_count=(uint16_t *)(obj_ptr+1);
                                }

                                //Increase count and leave other flags intact
                                uint16_t temp_count=((*element_count)&FLAG_COUNT_MASK)+1;
                                if (temp_count>FLAG_COUNT_MASK) return py_error_set(PY_ERROR_ELEMENT_OVERFLOW,0); 
                                *element_count=((*element_count&~FLAG_COUNT_MASK)|temp_count);
                            }
                        }
                        else if (input_token_precedence==PREC_ASSIGN)
                        {
                            //Assigning values: = += -= etc
                        }
                        else
                        {
                            //All other operators - no special handling
                            if (py_sp_count!=0)
                            {
                                //Operator associativity
                                switch (input_token)
                                {
                                    case TOKEN_EXP:
                                    case TOKEN_INVERT:
                                    case TOKEN_NEG:
                                    case TOKEN_PLUS:
                                    case TOKEN_BOOL_NOT:
                                        token_left_assoc=false;
                                        break;
                                    default:
                                        token_left_assoc=true;
                                        break;
                                }
                               
                                //Pop operators from stack
                                obj_ptr=NULL;
                                while(1)
                                {
                                    obj_ptr=py_peek_stack(obj_ptr,POP_OPERATORS);
                                    if (obj_ptr==NULL)
                                    {
                                        //Done - reached ( { [ : or end of stack
                                        break;
                                    }
                                    stack_token_precedence=py_find_precedence(*obj_ptr);
                                    if (((token_left_assoc==true)&&(stack_token_precedence<=input_token_precedence))||
                                            ((token_left_assoc==false)&&(stack_token_precedence<input_token_precedence)))
                                    {
                                        //Compile token
                                        py_append(compile_target,obj_ptr,1);
                                        
                                        //Remove token from stack
                                        obj_ptr=py_remove_stack(obj_ptr);
                                    }
                                    else 
                                    {
                                        //Done - reached operator of higher precedence
                                        break;
                                    }
                                }
                            }
                            py_custom_push(&input_token,1);
                        }
                        break;
                }   //switch for compiled values and operators processed by Shunting Yard

                //Record whether last symbol was comma to recognize trailing comma, ie (1) vs (1,)
                if (input_symbol!=SYMBOL_SPACE)
                {
                    if ((input_symbol==SYMBOL_OP)&&(input_token==TOKEN_COMMA)) exec_flags|=FLAG_COMMA_LAST;
                    else exec_flags&=~FLAG_COMMA_LAST;
                }

            } //for - loop through tokens in symbol_queue   
        } //if - processed next interpreter state

        //Make room in queue for next symbol
        if (exec_flags&FLAG_VALUE)
        {
            //Queue was flushed above
            symbol_queue[0].symbol=SYMBOL_NONE;
            symbol_queue[1].symbol=SYMBOL_NONE;
        }
        symbol_queue[2]=symbol_queue[1];
        symbol_queue[1]=symbol_queue[0];

        //Advance source pointer by size of symbol
        text+=symbol_len;

    } while(!(exec_flags&FLAG_DONE));

    //Run compiled code if anything compiled outside of a function
    if (compile_target!=NULL)
    {
        py_run(compile_target);
    }

    return PY_ERROR_NONE;
}
