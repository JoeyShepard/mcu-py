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

//Static function declarations
//============================
//TODO: arrange functions by group
static bool py_cstrcmp(const char *str1, uint16_t len1, const char *str2, uint16_t len2);
static uint8_t py_classify_input(const char input_char);
static uint8_t py_lookup_op(char op);
static uint8_t py_next_symbol(const char **text, uint16_t *len);
static int8_t py_find_symbol(const char *list, const char *symbol_begin, uint8_t symbol_len);
static uint8_t py_token_size(uint8_t token);
static py_error_t py_custom_push(const void *data, uint16_t data_size, uint16_t extra_size);
static bool py_custom_pop(uint8_t *data, uint8_t pop_class);
static uint8_t *py_peek_stack(uint8_t *obj, uint8_t pop_class);
static uint8_t *py_var_stack(const char *text, uint8_t symbol_len);
static uint8_t py_var_index(uint8_t *target_ptr);
static uint8_t *py_remove_stack(uint8_t *obj);
static uint8_t py_find_precedence(uint8_t token);
static uint8_t py_assign_op(uint8_t token);
static uint8_t py_compile_next_state(uint8_t state, uint8_t change);


//Compare counted strings
//Ok to pass in null-terminated string here as long as len1 and len2 don't include null terminator
static bool py_cstrcmp(const char *str1, uint16_t len1, const char *str2, uint16_t len2)
{
    //Counted strings must match in length - mismatch
    if (len1!=len2) return false;
    
    for (uint16_t i=0;i<len1;i++)
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

//Classify character of source
static uint8_t py_classify_input(const char input_char)
{ 
    //Error on input characters 127-255
    if (input_char>PY_CHAR_LAST_VALID) return INPUT_ERROR;

    //Classify input character
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
    //Should never happen but just in case
    return TOKEN_NONE;
}

static uint8_t py_next_symbol(const char **text, uint16_t *len)
{
    uint8_t symbol_state=SYMBOL_NONE;

    //Length of symbol - returned to caller
    *len=0;

    while (1)
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

                //Limit size of variable and function names as defined in custom.h
                if (*len==PY_MAX_SYMBOL_SIZE)
                {
                    return SYMBOL_ERROR; 
                }
            }
        }
    }
}

static int8_t py_find_symbol(const char *symbol_list, const char *symbol_begin, uint8_t symbol_len)
{
    //Look for symbol in symbol list
    int8_t symbol_id=0;
    while (*symbol_list)
    {
        uint8_t list_symbol_len=*symbol_list;
        if (py_cstrcmp(symbol_list+1,list_symbol_len,symbol_begin,symbol_len))
        {
            //Match found - return ID
            return symbol_id;
        }
        else
        {
            //Match not found - move to next symbol in list
            symbol_list+=list_symbol_len+1;
            symbol_id++;
        }
    }

    //Symbol not found
    return -1;
}

//Size on stack of any extra data after a token
//TODO: shrink
static uint8_t py_token_size(uint8_t token)
{
    switch (token)
    {
        case TOKEN_FUNC:
            return sizeof(uint8_t);
        case TOKEN_LPAREN:
        case TOKEN_LSBRACKET:
        case TOKEN_LCBRACKET:
        case TOKEN_TUPLE:
        case TOKEN_LIST:
        case TOKEN_DICT:
        case TOKEN_SET:
            return sizeof(uint16_t);
        case TOKEN_VAR_INFO:
            //-1 since PY_STACK_VAR_SIZE accounts for token
            return PY_STACK_VAR_SIZE+sizeof(const char **)-1;
        default:
            return 0;
    }
}

static py_error_t py_custom_push(const void *data, uint16_t data_size, uint16_t extra_size)
{
    if (py_free()<(data_size+extra_size)) return py_error_set(PY_ERROR_OUT_OF_MEM,0);
    
    //Decrease stack pointer
    py->sp-=(data_size+extra_size);

    for (uint16_t i=0;i<data_size;i++)
    {
        *(py->sp+i)=*(uint8_t *)(data+i);
    }

    //Increase count of items on stack
    py->sp_count++;

    return PY_ERROR_NONE;
}

static bool py_custom_pop(uint8_t *data, uint8_t pop_class)
{
    //Find next object on stack that matches desired class
    uint8_t *obj=py_peek_stack(NULL,pop_class);

    //No matching object found - return
    if (obj==NULL) return false;

    //Copy found object
    uint8_t obj_size=py_token_size(obj[0])+1;
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
        new_obj=py->sp;
    }
    else new_obj=obj;

    //Search through stack
    while (1)
    {
        //Pointer reached end of stack - done searching
        if (new_obj==(uint8_t*)py+(py->mem_size)) return NULL;

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
                    case TOKEN_VAR_INFO:
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
                    case TOKEN_VAR_INFO:
                        //Not looking for these - keep searching
                        break;
                    case TOKEN_LPAREN:
                    case TOKEN_LSBRACKET:
                    case TOKEN_LCBRACKET:
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
                        break;
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
                        break;
                    default:
                        //Not looking for these - keep searching
                        break;
                }
                break;
            case POP_VAR_INFO:
                //Find variable info
                if (*new_obj==TOKEN_VAR_INFO)
                {
                    if (new_obj!=obj)
                    {
                        //Only return if different from starting object
                        return new_obj;
                    }
                }
                break;
            default:
                return NULL;
        }

        //Advance pointer
        new_obj+=py_token_size(*new_obj)+1;
    }
}

//TODO: only called once? inline?
static uint8_t *py_var_stack(const char *text, uint8_t symbol_len)
{
    uint8_t *obj_ptr=NULL;    
    while(1)
    {
        //Check if variable exists already
        obj_ptr=py_peek_stack(obj_ptr,POP_VAR_INFO);
        if (obj_ptr==NULL)
        {
            //No more variable objects on stack - create new below
            break;
        }
        else
        {
            //Variable found - check if name matches
            if (py_cstrcmp(text,symbol_len,*(const char **)(obj_ptr+VAR_NAME),obj_ptr[VAR_SIZE]))
            {
                //Name matches - record return pointer
                return obj_ptr;
            }
        }
    }

    //Variable not found - create new

    //Reserve 3 bytes for token, flags, and size of name plus extra space for pointer to word in source
    uint8_t stack_buffer[PY_STACK_VAR_SIZE];
    stack_buffer[VAR_TOKEN]=TOKEN_VAR_INFO;
    stack_buffer[VAR_FLAGS]=0;
    stack_buffer[VAR_SIZE]=symbol_len;
    py_custom_push(stack_buffer,PY_STACK_VAR_SIZE,sizeof(text));

    //Fill in pointer to word in source
    obj_ptr=py_peek_stack(NULL,POP_VAR_INFO);
    *((const char **)(obj_ptr+VAR_NAME))=text;

    //Return pointer to variable info which is now on top of stack
    return obj_ptr;
}

static uint8_t py_var_index(uint8_t *target_ptr)
{
    uint8_t *obj_ptr=NULL;    
    uint8_t ret_val=0;
    while(1)
    {
        //Check if variable exists already
        obj_ptr=py_peek_stack(obj_ptr,POP_VAR_INFO);
        if (obj_ptr==NULL)
        {
            return ret_val;
        }
        else if (obj_ptr==target_ptr) ret_val=0;
        else ret_val++;
    }
}

static uint8_t *py_remove_stack(uint8_t *obj)
{
    uint16_t obj_size=py_token_size(*obj)+1;
    for (uint8_t *copy_ptr=obj-1;copy_ptr>=py->sp;copy_ptr--)
    {
        *(copy_ptr+obj_size)=*copy_ptr; 
    }
    py->sp_count--;
    py->sp+=obj_size;
    return obj+obj_size;
}

static uint8_t py_find_precedence(uint8_t token)
{
    const uint8_t *table_ptr=py_op_precedence;
    uint8_t token_max=0;
    uint8_t precedence=0;

    while(1)
    {
        token_max+=*table_ptr;
        if (token<=token_max)
        {
            return precedence;
        }
        else
        {
            table_ptr++;
            precedence++;
        }
    }
}

//Look up op like + from assignment like +=
static uint8_t py_assign_op(uint8_t token)
{
    switch(token)
    {
        case TOKEN_EXP_EQ:      return TOKEN_EXP;
        case TOKEN_MUL_EQ:      return TOKEN_MUL;
        case TOKEN_DIV_EQ:      return TOKEN_DIV;
        case TOKEN_MOD_EQ:      return TOKEN_MOD;
        case TOKEN_ADD_EQ:      return TOKEN_ADD;
        case TOKEN_SUB_EQ:      return TOKEN_SUB;
        case TOKEN_LSHIFT_EQ:   return TOKEN_LSHIFT;
        case TOKEN_RSHIFT_EQ:   return TOKEN_RSHIFT;
        case TOKEN_AND_EQ:      return TOKEN_AND;
        case TOKEN_XOR_EQ:      return TOKEN_XOR;
        case TOKEN_OR_EQ:       return TOKEN_OR;
        default:                return TOKEN_NONE;
    }
}

static uint8_t py_compile_next_state(uint8_t state, uint8_t change)
{
    if (state==COMPILE_ERROR) return COMPILE_ERROR;
    
}

//Execute Python source passed in as a string
py_error_t py_execute(const char *text)
{
    //Can't set error_num if uninitialized since stored in passed-in memory so return error code but don't set py_error_num
    if (!py_initialized) return PY_ERROR_UNINITIALIZED;

    /*
    //Debugging
    {
        uint8_t *debug_ptr=py_peek_stack(NULL,POP_VAR_INFO);
        debug("No vars. Should be null: %p\n",debug_ptr);

        debug_ptr=py_var_stack("abc",3);
        debug("abc: %p\n",debug_ptr);
        debug_ptr=py_peek_stack(NULL,POP_VAR_INFO);
        debug("abc: %p\n",debug_ptr);
        uint8_t var_index=py_var_index(debug_ptr);
        debug("abc rank: %d\n",var_index);
        
        debug_ptr=py_var_stack("foo",3);
        debug("foo: %p\n",debug_ptr);
        debug_ptr=py_peek_stack(NULL,POP_VAR_INFO);
        debug("foo: %p\n",debug_ptr);
        var_index=py_var_index(debug_ptr);
        debug("foo rank: %d\n",var_index);

        debug_ptr=py_peek_stack(NULL,POP_VAR_INFO);
        debug_ptr=py_peek_stack(debug_ptr,POP_VAR_INFO);
        debug("abc: %p\n",debug_ptr);
        debug_ptr=py_var_stack("abc",3);
        debug("abc: %p\n",debug_ptr);
        var_index=py_var_index(debug_ptr);
        debug("abc rank: %d\n",var_index);


        exit(1);
    }
    //Debugging
    */

    //Local variables reset for each line
    struct SymbolType
    {
        uint8_t symbol;
        uint8_t token;
    } symbol_queue[3];
    uint8_t interpreter_state=STATE_BEGIN;          //Assigned immediately below but assign here too to appease linter
    uint8_t last_interpreter_state=STATE_NONE;      //Assigned immediately below but assign here too to appease linter
    uint8_t exec_flags=FLAG_RESET_STATE;
    uint16_t line_number=0;
    uint16_t top_comma_count;
    uint8_t compile_state;
    uint8_t *compile_target=NULL;

    //Local variables used temporarily in code
    uint16_t symbol_len;
    const uint8_t *table_ptr;       //Used for state machine and precedence lookups
    uint8_t token_max;
    uint8_t stack_token_precedence;
    uint8_t *obj_ptr;
    //TODO: explain size. should be big enough for biggest possible.
    //      3 - 8 bits for token and 16 bits for extra data for enum OpeningFlags for ( [ {
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
            compile_state=COMPILE_BEGIN;
            if (compile_target==NULL)
            {
                //Allocate temp mem for code outside of function
                compile_target=py_allocate(0);
                stack_buffer[0]=OBJECT_CODE;
                py_append(compile_target,stack_buffer,1);
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
                        //Determine next interpreter state
                        if ((interpreter_state==STATE_BEGIN)&&
                            ((input_token==TOKEN_RPAREN)||(input_token==TOKEN_RCBRACKET)))
                        {
                            //Exceptions to look up table:
                            //  ) after , for (1,)
                            //  } after , for {1:2,} which is just {1:2}
                            last_interpreter_state=interpreter_state;
                            interpreter_state=STATE_VAL;
                        }
                        else if ((interpreter_state==STATE_BEGIN)&&(input_token==TOKEN_COLON))
                        {
                            //Exception to look up table - colon after colon for [::]
                            last_interpreter_state=interpreter_state;
                            interpreter_state=STATE_BEGIN;
                        }
                        else
                        {
                            //Look up next interpreter state from table 
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
                        }
                        break;
                }

                //Account for period as attribute operator - anything other than alpha symbol is error
                if ((exec_flags&FLAG_ATTRIBUTE)&&
                    (!((input_symbol==SYMBOL_ALPHA)||(input_symbol==SYMBOL_SPACE))))
                {
                    return py_error_set(PY_ERROR_SYNTAX,0);
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

                        //Check if top level is a tuple is 1,2 rather than (1,2)
                        if (top_comma_count>0)
                        {
                            if (exec_flags&FLAG_COMMA_LAST)
                            {
                                //Tuple with empty argument like 1,
                                top_comma_count--;
                            }
                            stack_buffer[0]=TOKEN_TUPLE;
                            *(uint16_t*)(stack_buffer+1)=top_comma_count+1;
                            py_append(compile_target,stack_buffer,3);
                        }

                        if (input_symbol==SYMBOL_END_ALL)
                        {
                            //Compile return token since done processing
                            stack_buffer[0]=TOKEN_RETURN;
                            py_append(compile_target,stack_buffer,1);
                            exec_flags|=FLAG_DONE;
                        }
                        break;
                    case SYMBOL_ALPHA:
                        if (exec_flags&FLAG_ATTRIBUTE)
                        {
                            //Symbol is attribute following period
                            //Copy name of symbol into bytecode - takes more space but simplifies interpeter
                            //Could keep list of attribute names and assign index but then need to search
                            //list of names for lookup and then adjust index if no more uses of that attribute
                            //(which may be a different class using the same name) but then would also need
                            //reference counter for each attribute name. Less complicated to copy name.
                            stack_buffer[0]=TOKEN_ATTRIBUTE;
                            stack_buffer[1]=symbol_len;
                            py_append(compile_target,stack_buffer,2);
                            py_append(compile_target,text,symbol_len);
                            exec_flags&=~FLAG_ATTRIBUTE;
                        }
                        else
                        {
                            //Check if alpha symbol is built-in Python function
                            
                            //int8_t symbol_id=py_find_symbol(py_functions,text,symbol_len);
                            int8_t symbol_id=-1;

                            if (symbol_id!=-1)
                            {
                                /*
                                //Built-in function
                                stack_buffer[0]=TOKEN_BUILTIN_FUNC;
                                stack_buffer[1]=symbol_id;
                                py_append(compile_target,stack_buffer,2);
                                */
                            }
                            else
                            {
                                //Check if alpha symbol is keyword like break or def
                                int8_t symbol_id=py_find_symbol(py_keywords,text,symbol_len);
                                if (symbol_id!=-1)
                                {
                                    //Keyword

                                    //Debugging
                                    {
                                        debug("KEYWORD: ");
                                        debug_cstr(text,symbol_len);
                                        debug("\n");
                                    }
                                    //Debugging

                                }
                                else
                                {
                                    //Not built-in function or keyword so must be variable or user-defined function
                                    //Push pointer in source to name for now and decide if local or global at end
                                    //Note, two passes are necessary here since a variable read or passed to a function
                                    //may be local or global. Writing to the variable if it wasn't declared with global
                                    //means it's local whether the write occurs in source before or after the read.

                                    //Find var if it exists on stack and create it if it doesn't exist
                                    obj_ptr=py_var_stack(text,symbol_len);

                                    //Generate bytecode for global variable by default and record index to var info on stack.
                                    //After current function or top-level code is processed, do second pass to change globals
                                    //to locals as needed and change indexes from pointing to var info on stack to pointing to
                                    //locals or globals at runtime.
                                    uint8_t var_index=py_var_index(obj_ptr);
                                    stack_buffer[0]=TOKEN_GLOBAL;
                                    stack_buffer[1]=var_index;
                                    py_append(compile_target,stack_buffer,2);
                                }
                            }
                        }
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
                            stack_buffer[0]=TOKEN_INT8;
                            data_size=1;
                        }
                        else if ((num>=INT16_MIN)&&(num<=INT16_MAX))
                        {
                            stack_buffer[0]=TOKEN_INT16;
                            data_size=2;
                        }
                        else 
                        {
                            stack_buffer[0]=TOKEN_INT32;
                            data_size=4;
                        }
                        py_append(compile_target,stack_buffer,1);
                        py_append(compile_target,&num,data_size);
                        break;
                    case SYMBOL_STRING:
                        //Compile value
                        stack_buffer[0]=TOKEN_STRING;
                        py_append(compile_target,stack_buffer,1);
                        *(uint16_t *)stack_buffer=symbol_len-2;         //-2 to length to discard quotes
                        py_append(compile_target,stack_buffer,2);       //Write length of string
                        py_append(compile_target,text+1,symbol_len-2);  //Write string data. +1 to start after first quote
                        break;
                    case SYMBOL_OP:
                        //Shunting yard algorithm
                        ; //Needed before declaration below by older C standard used by msp430-gcc
                        uint8_t input_token_precedence=py_find_precedence(input_token);
                        bool token_left_assoc;
                        
                        //Check for operators requiring special handling
                        if (input_token_precedence==PREC_PERIOD)
                        {
                            //Period for attribute such as method or member

                            //Set flag so next alpha symbol can be treated as attribute
                            exec_flags|=FLAG_ATTRIBUTE;
                        }
                        else if (input_token_precedence==PREC_OPENING)
                        {
                            //Opening parentheses or brackets: ( [ {

                            //Push token and 16 bit value for extra data
                            stack_buffer[0]=input_token;
                            if (last_interpreter_state==STATE_VAL)
                            {
                                if (input_token==TOKEN_LCBRACKET)
                                {
                                    //Error - neither set nor dictionary can index with {}, ie x{1} or x{1:2}
                                    return py_error_set(PY_ERROR_SYNTAX,0);
                                }
                                else
                                {
                                    //Value precedes ( or [ so function or list/dict indexing
                                    *(uint16_t *)(stack_buffer+1)=FLAG_FUNC_DEREF;
                                }
                            }
                            else 
                            {
                                //No value preceding ( or [ so not function or list/dict indexing
                                *(uint16_t *)(stack_buffer+1)=0;
                            }
                            py_custom_push(stack_buffer,3,0);
                        }
                        else if (input_token_precedence==PREC_CLOSING)
                        {
                            //Closing parentheses or brackets: ) ] }

                            //Empty pending operators until opening ( [ { found
                            while(1)
                            {
                                if (py_custom_pop(stack_buffer,POP_CLOSING_FOUND)==false)
                                {
                                    //Error - no matching bracket found! ie 1+2)
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
                                        //Found matching ( [ { on stack - figure out object type and element count
                                        uint16_t element_data=*(uint16_t *)(stack_buffer+1);
                                        uint16_t element_count=element_data&FLAG_COUNT_MASK;
                                        
                                        //debug("%X %X\n",element_data,element_count);

                                        if (stack_buffer[0]==TOKEN_LPAREN)
                                        {
                                            //Found ( so must be function, tuple, or expression

                                            //Adjust element count
                                            if (last_interpreter_state!=STATE_LPAREN)
                                            {
                                                //Parentheses not empty - increment element count
                                                element_count++;
                                            }
                                            if (exec_flags&FLAG_COMMA_LAST)
                                            {
                                                //Empty element at end like (1,) so decrease count
                                                element_count--;
                                            }
                                            
                                            //Preset element count since used by most options below
                                            *(uint16_t *)(stack_buffer+1)=element_count;

                                            if (element_data&FLAG_FUNC_DEREF)
                                            {
                                                //Function
                                                if (exec_flags&FLAG_COMMA_LAST)
                                                {
                                                    //No support for default args so last arg can't be empty
                                                    //Could add support for default args here
                                                    return py_error_set(PY_ERROR_MISSING_ARG,0);
                                                }
                                                if (element_count>PY_FUNC_MAX_ARGS)
                                                {
                                                    //Limit max function arg count so it fits in one byte
                                                    return py_error_set(PY_ERROR_TOO_MANY_ARGS,0);
                                                }
                                                stack_buffer[0]=TOKEN_FUNC;
                                                stack_buffer[1]=element_count;
                                                py_append(compile_target,stack_buffer,2);
                                            }
                                            else
                                            {
                                                //Tuple or expression
                                                if ((element_count==1)&&(!(exec_flags&FLAG_COMMA_LAST)))
                                                {
                                                    //Expression - only one element and no empty arg like (1,)
                                                    //No special handling
                                                }
                                                else
                                                {
                                                    //Tuple - empty () or one element with comma (1,) or more than one element
                                                    stack_buffer[0]=TOKEN_TUPLE;
                                                    py_append(compile_target,stack_buffer,3);
                                                }
                                            }
                                        }
                                        else if (stack_buffer[0]==TOKEN_LSBRACKET)
                                        {
                                            //Found [ so must be list or indexing of list or dict
                                            if (element_data&FLAG_LIST_MASK)
                                            {
                                                //Slice since at least one colon
                                                if ((element_data&FLAG_FUNC_DEREF)==false)
                                                {
                                                    //Error - slice must be attached to value like x[:] not [:] alone
                                                    return py_error_set(PY_ERROR_SYNTAX,0);
                                                }

                                                //Count of colons in slice
                                                element_count=2-((element_data&FLAG_LIST_MASK)>>FLAG_SHIFT_COUNT);
                                                if (exec_flags&FLAG_COLON_LAST) element_count++;
                                               
                                                //Insert None if any slice numbers missing at end since [::] == [None:None:None] 
                                                stack_buffer[0]=TOKEN_NONE_OBJ;
                                                for (int i=0;i<element_count;i++)
                                                {
                                                    py_append(compile_target,stack_buffer,1);
                                                }

                                                //Slice
                                                stack_buffer[0]=TOKEN_SLICE_INDEX;
                                                py_append(compile_target,stack_buffer,1);
                                            }
                                            else
                                            {
                                                //No colons so list or indexing of list or dict

                                                //Adjust element count
                                                element_count++;    //no check for [] since interpreter state machine catches
                                                if (exec_flags&FLAG_COMMA_LAST)
                                                {
                                                    //Empty element at end like [1,2,] so decrease count
                                                    element_count--;
                                                }

                                                //Element count used for dict and set below
                                                *(uint16_t *)(stack_buffer+1)=element_count;

                                                if (element_data&FLAG_FUNC_DEREF)
                                                {
                                                    //Indexing - either single element like x[1] or tuple like x[1,2] or x[1,]
                                                    if ((element_count==1)&&(!(exec_flags&FLAG_COMMA_LAST)))
                                                    {
                                                        //Single element indexing like x[1] - no need to convert to tuple
                                                        //Index token written below
                                                    }
                                                    else
                                                    {
                                                        //Tuple indexing - create tuple then index
                                                        stack_buffer[0]=TOKEN_TUPLE;
                                                        py_append(compile_target,stack_buffer,3);
                                                    }
                                                    //Index in both if and else cases above
                                                    stack_buffer[0]=TOKEN_INDEX;
                                                    py_append(compile_target,stack_buffer,1);
                                                }
                                                else
                                                {
                                                    //List
                                                    stack_buffer[0]=TOKEN_LIST;
                                                    py_append(compile_target,stack_buffer,3);
                                                }
                                            }
                                        }
                                        else if (stack_buffer[0]==TOKEN_LCBRACKET)
                                        {
                                            //Found { so must be dict or set

                                            //Adjust element count
                                            if (last_interpreter_state!=STATE_LCBRACKET)
                                            {
                                                //Brackets not empty - increment element count
                                                element_count++;
                                            }
                                            if (exec_flags&FLAG_COMMA_LAST)
                                            {
                                                //Empty element at end like {1:2,} so decrease count
                                                element_count--;
                                            }

                                            //Element count used for dict and set below
                                            *(uint16_t *)(stack_buffer+1)=element_count;
                                            
                                            //Determine if set or dictionary
                                            uint16_t temp_flags=element_data&FLAG_DICT_SET_MASK;
                                            if (temp_flags==PARSE_BEGIN)
                                            {
                                                if (element_count==0)
                                                {
                                                    //Empty dictionary, ie {}
                                                    stack_buffer[0]=TOKEN_DICT;
                                                }
                                                else
                                                {
                                                    //Set with one item so no commas, ie {1}
                                                    stack_buffer[0]=TOKEN_SET;
                                                }
                                            }
                                            else if (temp_flags==PARSE_SET)
                                            {
                                                //Set with at least one comma, ie {1,2}
                                                stack_buffer[0]=TOKEN_SET;
                                            }
                                            else if (temp_flags==PARSE_DICT_COMMA_NEXT)
                                            {
                                                if (exec_flags&FLAG_COLON_LAST)
                                                {
                                                    //Error - comma expected next but nothing after colon, ie {1:}
                                                    return py_error_set(PY_ERROR_SYNTAX,0);
                                                }
                                                //Dictionary, ie {1:2}
                                                stack_buffer[0]=TOKEN_DICT;
                                            }
                                            else if (temp_flags==PARSE_DICT_COLON_NEXT)
                                            {
                                                if ((exec_flags&FLAG_COMMA_LAST)==false)
                                                {
                                                    //Error - missing colon, ie {1:2,3}
                                                    return py_error_set(PY_ERROR_SYNTAX,0);
                                                }

                                                //Dictionary with empty element at end, ie {1:2,}
                                                stack_buffer[0]=TOKEN_DICT;
                                            }
                                            py_append(compile_target,stack_buffer,3);
                                        }
                                        break;
                                    }
                                }
                                else 
                                {
                                    //Compile operator - no special handling
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
                                if (py_custom_pop(stack_buffer,POP_OPERATORS)==false)
                                {
                                    //Reached end of stack - no more operators left to pop and compile
                                    break;
                                }

                                //Compile operator
                                py_append(compile_target,stack_buffer,1);
                            }
                            
                            //Find ( { [ to edit its attached flags and comma count
                            obj_ptr=py_peek_stack(NULL,POP_OPENINGS); 

                            //Used for comma and colon
                            uint16_t *element_data=(uint16_t *)(obj_ptr+1);

                            //Increase element count
                            if (input_token_precedence==PREC_COMMA)
                            {
                                //Find ( [ or { on stack and increase comma count in data stored on stack following it
                                if (obj_ptr==NULL) 
                                {
                                    //No ( [ { on stack so use count of top-level commas ie 1,2 without parentheses
                                    element_data=&top_comma_count;
                                }
                                else
                                {
                                    if (*obj_ptr==TOKEN_LSBRACKET)
                                    {
                                        //Found [ so make sure not slice, ie [1:2, 
                                        if ((*element_data)&FLAG_LIST_MASK)
                                        {
                                            //FLAG_LIST_MASK masks 2 bits holding count of colons encountered - should be 0
                                            return py_error_set(PY_ERROR_SYNTAX,0);
                                        }
                                    }
                                    else if (*obj_ptr==TOKEN_LCBRACKET)
                                    {
                                        //Found { so process state change for dict parsing
                                        uint16_t temp_flags=((*element_data)&FLAG_DICT_SET_MASK);
                                        if (temp_flags==PARSE_BEGIN) temp_flags=PARSE_SET;
                                        else if (temp_flags==PARSE_DICT_COMMA_NEXT) temp_flags=PARSE_DICT_COLON_NEXT;
                                        else if (temp_flags==PARSE_DICT_COLON_NEXT)
                                        {
                                            //Error - expecting colon but found comma, ie {1:2,3,
                                            return py_error_set(PY_ERROR_SYNTAX,0);
                                        }
                                        else
                                        {
                                            //No handling for PARSE_SET since no change on comma
                                        }

                                        //Write modified flags back
                                        *element_data=(((*element_data)&~FLAG_DICT_SET_MASK)|temp_flags);
                                    }
                                    else
                                    {
                                        //No extra processing for (
                                    }
                                }

                                //Increase count and leave other flags intact
                                uint16_t temp_count=((*element_data)&FLAG_COUNT_MASK)+1;
                                if (temp_count>FLAG_COUNT_MASK) return py_error_set(PY_ERROR_ELEMENT_OVERFLOW,0); 
                                *element_data=(((*element_data)&~FLAG_COUNT_MASK)|temp_count);
                            }
                            else if (input_token_precedence==PREC_COLON)
                            {
                                if ((obj_ptr==NULL)||
                                    (*obj_ptr==TOKEN_LPAREN))
                                {
                                    //No [ { on stack or found ( which doesn't allow colon 
                                    return py_error_set(PY_ERROR_SYNTAX,0);
                                }
                                
                                if (*obj_ptr==TOKEN_LSBRACKET)
                                {
                                    //Found [ so process as slice
                                    if ((*element_data)&FLAG_COUNT_MASK)
                                    {
                                        //Error - no colons allowed if commas have appeared, ie [1,2:3
                                        return py_error_set(PY_ERROR_SYNTAX,0);
                                    }

                                    //Increase count of colons encountered
                                    if (((*element_data)&FLAG_LIST_MASK)==(2<<FLAG_SHIFT_COUNT))
                                    {
                                        //Error - slice already has two colons, can't add third, ie [::: 
                                        return py_error_set(PY_ERROR_SYNTAX,0);
                                    }

                                    //Increase colon count by one
                                    *element_data+=(1<<FLAG_SHIFT_COUNT);
                                    
                                    //Insert None as necessary as [:] and [::] == [None:None:None]
                                    if ((last_interpreter_state==STATE_LSBRACKET)||
                                        (exec_flags&FLAG_COLON_LAST))
                                    {
                                        stack_buffer[0]=TOKEN_NONE_OBJ;
                                        py_append(compile_target,stack_buffer,1);
                                    }
                                }
                                else if (*obj_ptr==TOKEN_LCBRACKET)
                                {
                                    //Found { so process state change for dict parsing
                                    uint16_t temp_flags=((*element_data)&FLAG_DICT_SET_MASK);
                                    if (temp_flags==PARSE_BEGIN) temp_flags=PARSE_DICT_COMMA_NEXT;
                                    else if (temp_flags==PARSE_SET)
                                    {
                                        //Error - no colon allowed in set, ie {1,2:
                                        return py_error_set(PY_ERROR_SYNTAX,0);
                                    }
                                    else if (temp_flags==PARSE_DICT_COLON_NEXT) temp_flags=PARSE_DICT_COMMA_NEXT;
                                    else if (temp_flags==PARSE_DICT_COMMA_NEXT)
                                    {
                                        //Error - expecting comma but found colon, ie {1:2:
                                        return py_error_set(PY_ERROR_SYNTAX,0);
                                    }

                                    //Write modified flags back
                                    *element_data=(((*element_data)&~FLAG_DICT_SET_MASK)|temp_flags);
                                }
                            }
                        }
                        else if (input_token_precedence==PREC_ASSIGN)
                        {
                            //Assigning values: = += -= etc
                            if (py_custom_pop(stack_buffer, POP_END_LINE))
                            {
                                //Error: ( [ { or operator left on stack
                                return py_error_set(PY_ERROR_SYNTAX,0);
                            }
                            if (last_interpreter_state!=STATE_VAL)
                            {
                                //Error: no value to assign to or incomplete like 2+
                                return py_error_set(PY_ERROR_SYNTAX,0);
                            }
                        }
                        else
                        {
                            //All other operators - no special handling
                            if (py->sp_count!=0)
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
                            py_custom_push(&input_token,1,0);
                        }
                        break;
                }   //switch for compiled values and operators processed by Shunting Yard

                if (input_symbol!=SYMBOL_SPACE)
                {
                    //Record whether last symbol was comma to recognize trailing comma, ie (1) vs (1,)
                    if ((input_symbol==SYMBOL_OP)&&(input_token==TOKEN_COMMA)) exec_flags|=FLAG_COMMA_LAST;
                    else exec_flags&=~FLAG_COMMA_LAST;
                    //Record whether last symbol was colon to flag error on {1:}
                    //Note this was previously handled by interpreter state machine but added exception for
                    //STATE_BEGIN which is generated by both comma and colon. {1:2,} is not an error but no 
                    //way to tell that from {1:} which is an error.
                    if ((input_symbol==SYMBOL_OP)&&(input_token==TOKEN_COLON)) exec_flags|=FLAG_COLON_LAST;
                    else exec_flags&=~FLAG_COLON_LAST;
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
