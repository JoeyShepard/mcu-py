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

        //TODO: remove
        /*
        debug(">>>Input char: %c(%d). Length: %d. Type: %d - %s\n",input_char,input_char,*len,char_type,debug_value("symbol input type",char_type));
        debug(">>>Old symbol state: %d - %s. New symbol state: %d - %s\n",symbol_state,debug_value("symbol type",symbol_state),new_symbol_state,debug_value("symbol type",new_symbol_state));
        debug(">>>Look up: [%d][%d/2] = [%d][%d]\n",char_type,symbol_state,char_type,symbol_state/2);
        debug_key();
        */


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

static py_error_t py_token_push(uint8_t token)
{
    if (py_free==0) return py_error_set(PY_ERROR_OUT_OF_MEM,0);

    py_sp-=1;
    *py_tos=token;
    py_sp_count++;

    return PY_ERROR_NONE;
}

static py_error_t py_uint16_push(uint16_t data)
{
    //TODO: handle in function like pop and return line number?
    if (py_free<2) return py_error_set(PY_ERROR_OUT_OF_MEM,0);

    py_sp-=2;
    *(uint16_t *)py_tos=data;
    py_sp_count++;

    return PY_ERROR_NONE;
}

static uint8_t py_token_pop()
{
    if (py_sp_count==0) 
    {
        py_error_set(PY_ERROR_STACK_UNDERFLOW,0);
        return 0;
    }

    uint8_t token=*py_tos;
    py_sp+=1;
    py_sp_count--;
    return token;
}

static uint16_t py_uint16_pop()
{
    if (py_sp_count==0) 
    {
        py_error_set(PY_ERROR_STACK_UNDERFLOW,0);
        return 0;
    }

    uint16_t ret_val=*(uint16_t *)py_tos;
    py_sp+=2;
    py_sp_count--;
    return ret_val;
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

//Execute Python source passed in as a string
py_error_t py_execute(const char *text)
{
    //Can't set error_num if uninitialized since stored in passed-in memory so return error code but don't set py_error_num
    if (!py_settings.initialized) return PY_ERROR_UNINITIALIZED;

    //Debugging
    {
        /*
        debug("Pre-heap: %u. Free: %u\n",PY_MEM_HEAP_BEGIN,py_free);
        py_allocate(5);
        debug("Pre-heap: %u. Free: %u\n",PY_MEM_HEAP_BEGIN,py_free);
        py_allocate(7);
        debug("Pre-heap: %u. Free: %u\n",PY_MEM_HEAP_BEGIN,py_free);
        struct StackItem temp_stack_item;
        py_push(temp_stack_item);
        debug("Pre-heap: %u. Free: %u\n",PY_MEM_HEAP_BEGIN,py_free);
        py_push_custom(0x42,3);
        debug("Pre-heap: %u. Free: %u\n",PY_MEM_HEAP_BEGIN,py_free);

        uint16_t temp=*(uint16_t*)py_heap_begin;
        debug("Linked list sizes: %d ",temp);
        temp=*(uint16_t*)(py_heap_begin+temp);
        debug("%d ",temp);
        temp=*(uint16_t*)(py_heap_begin+temp);
        debug("%d ",temp);

        debug_key();
        */

        debug("%s\n",text);
    }

    struct SymbolType
    {
        uint8_t symbol;
        uint8_t token;
    } symbol_queue[3];
    
    const char *text_original=text;
    uint16_t symbol_len;
    uint8_t interpreter_state;
    uint8_t last_interpreter_state;
    uint8_t exec_flags=FLAG_RESET_STATE;
    //Used for state machine and precedence lookups
    const uint8_t *table_ptr;
    uint8_t token_max;
    uint8_t stack_token;
    uint8_t stack_token_precedence;

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
        }

        //Get next symbol in source
        symbol_queue[0].symbol=py_next_symbol(&text,&symbol_len);
        if (symbol_queue[0].symbol==SYMBOL_ERROR) return py_error_set(PY_ERROR_INPUT,text_original-text);

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

                        //debug("Combo %d found! %s + %s = %s\n",i,debug_value("token",symbol_queue[0].token),debug_value("token",symbol_queue[1].token),debug_value("token",py_op_combos[i][2]));

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

        /*
        //Debugging
        {
            debug("Queue: ");
            for (int i=2;i>=0;i--)
            {
                debug("%s",debug_value("symbol type",symbol_queue[i].symbol));
                if (symbol_queue[i].symbol==SYMBOL_OP) debug(" (%s)",debug_value("token",symbol_queue[i].token));
                if (i==0) debug("\n");
                else debug(", ");
            }
            debug("Flag value: %d\n",(exec_flags&FLAG_VALUE)==FLAG_VALUE);
        }
        //Debugging
        */

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

            //Debugging
            /*
            {
                if (exec_flags&FLAG_ADVANCE_STATE)
                {
                    debug("Stack(%d):",py_sp_count);
                    uint8_t count=py_sp_count;
                    uint8_t *ptr=py_tos;
                    for (int j=0;j<count;j++)
                    {
                        debug("%s ",debug_value("token",*ptr));
                        if (py_find_precedence(*ptr)==PREC_OPENING)
                        {
                            debug("(%X) ",*(uint16_t *)(ptr+1));
                            count--; 
                            ptr+=2;
                        }
                        ptr++;
                    }
                    debug("\n");
                }
            }
            */
            //Debugging


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

                        //debug("Starting state: %s, Token: %s, Result: ",debug_value("interpreter state",interpreter_state),debug_value("token",input_token));

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
                                if (interpreter_state==STATE_ERROR) return py_error_set(PY_ERROR_INPUT,text_original-text);

                                //Account for unary + and -
                                if (interpreter_state==STATE_INV_NEG_PLUS)
                                {
                                    if (input_token==TOKEN_ADD) input_token=TOKEN_PLUS;
                                    if (input_token==TOKEN_SUB) input_token=TOKEN_NEG;
                                }

                                //debug("%s\n",debug_value("interpreter state",interpreter_state));

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
               
                //Symbol ready to be compiled
                int num=0;
                const char *num_ptr=text;
                switch (input_symbol)
                {
                    case SYMBOL_END_LINE:
                        //Done processing line
                        exec_flags|=FLAG_RESET_STATE;
                        //Fallthrough
                    case SYMBOL_END_ALL:
                        //Make sure expression didn't end with calculation pending like 2+
                        if (interpreter_state==STATE_REQ) return py_error_set(PY_ERROR_SYNTAX,text_original-text);

                        uint8_t count=py_sp_count;
                        for (int j=0;j<count;j++)
                        {
                            stack_token=py_token_pop();

                            //Debugging
                            debug("%s ",debug_value("token",stack_token));
                            
                            if (py_find_precedence(stack_token)==PREC_OPENING)
                            {
                                //If there is a ( [ or { at the end, error on missing } ] or )
                                return py_error_set(PY_ERROR_SYNTAX,text_original-text);
                            }
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

                        //Debugging
                        {
                            debug("HEX: 0x%X\n",num);
                        }
                        //Debugging

                        break;
                    case SYMBOL_NUM:
                        for (int i=0;i<symbol_len;i++)
                        {
                            num*=10;
                            num+=(*num_ptr)-'0';
                            num_ptr++;
                        }

                        //Debugging
                        {
                            debug("NUM: %d\n",num);
                        }
                        //Debugging

                        break;
                    case SYMBOL_STRING:
                        break;
                        
                        
                        
                        //Debugging
                        {
                            /*
                            debug("%s\n",text);
                            debug("%s: ",debug_value("symbol type",input_symbol));
                            for (int i=0;i<symbol_len;i++)
                            {
                                debug("%c",text[i]);
                            }
                            debug("\n");
                            debug_key();
                            */

                            for (int i=0;i<symbol_len;i++)
                            {
                                debug("%c",text[i]);
                            }
                            debug(" ");
                        }
                        //Debugging

                        //Compile value



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
                            if (last_interpreter_state==STATE_VAL) py_uint16_push(FLAG_FUNC_DEREF);
                            else py_uint16_push(0);
                            py_token_push(input_token);
                        }
                        else if (input_token_precedence==PREC_CLOSING)
                        {
                            //Closing parentheses or brackets: ) ] }

                            //debug("\nClosing: %s\n",debug_value("token",input_token));

                            uint8_t count=py_sp_count;
                            for (int j=0;j<count;j++)
                            {
                                stack_token=py_token_pop();
                                stack_token_precedence=py_find_precedence(stack_token);
                                if (stack_token_precedence==PREC_OPENING)
                                {
                                    //Ensure ( matches ), [ matches ], { matches }
                                    if (input_token!=stack_token+3) return py_error_set(PY_ERROR_SYNTAX,text_original-text);
                                    else
                                    {
                                        //Found match - process meta data
                                        uint16_t stack_meta_data=py_uint16_pop();
                                        count--;
                                        break;
                                    }
                                }
                                else 
                                {
                                    //Compile operator
                                    //debug("Searching and popped: %s\n ",debug_value("token",stack_token));
                                    debug("%s ",debug_value("token",stack_token));
                                }
                            }
                        }
                        else if (input_token_precedence==PREC_COLON)
                        {
                            //Colon
                        }
                        else if (input_token_precedence==PREC_COMMA)
                        {
                            //Comma
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
                                uint8_t count=py_sp_count;
                                for (int j=0;j<count;j++)
                                {
                                    stack_token=*py_tos;
                                    stack_token_precedence=py_find_precedence(stack_token);
                                    
                                    if (stack_token_precedence==PREC_OPENING)
                                    {
                                        //Done looping since hit ( [ or {
                                        break;
                                    }
                                    else if (((token_left_assoc==true)&&(stack_token_precedence<=input_token_precedence))||
                                            ((token_left_assoc==false)&&(stack_token_precedence<input_token_precedence)))
                                    {
                                        //stack_token already recorded above before pop
                                        py_token_pop(); 
                                        //debug("Lower: %s\n",debug_value("token",stack_token));
                                        debug("%s ",debug_value("token",stack_token));
                                    }
                                    else 
                                    {
                                        //Done looping
                                        break;
                                    }
                                }
                            }
                            py_token_push(input_token);
                           
                        }
                        
                        //Debugging
                        {
                            /*
                            debug("%s\n",text);
                            debug("token: %s\n",debug_value("token",input_token));
                            debug_key();
                            */

                            //debug("%s (%d) ",debug_value("token",input_token),precedence);
                        }
                        //Debugging

                        break;
                }



                /*
                //Debugging
                {
                    if ((input_symbol!=SYMBOL_SPACE)&&(input_symbol!=SYMBOL_NONE))
                    {
                        if (input_symbol==SYMBOL_OP)
                        {
                            debug("> Processed token: %s\n",debug_value("token",input_token));
                        }
                        else
                        {
                            debug("> Processed %s: ",debug_value("symbol type",input_symbol));
                            for (int i=0;i<symbol_len;i++) debug("%c",text[i]);
                            debug("\n");
                        }
                    }
                }
                //Debugging
                */
            }
        }

        //Debugging
        /*
        {
            debug("\n%s\n",text);
            debug("Type: %d - %s. Length: %d. Symbol: ",symbol_queue[0].symbol,debug_value("symbol type",symbol_queue[0].symbol),symbol_len);
            for (int i=0;i<symbol_len;i++)
            {
                debug("%c",text[i]);
            }
            debug("\n");

            if (symbol_queue[0].symbol==SYMBOL_OP)
            {
                debug(" - Token: %d",symbol_queue[0].token);
                debug(" (%s)",debug_value("token",symbol_queue[0].token));
                debug("\n");
            }
            debug_key();
        }
        */
        //Debugging

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

    } while(symbol_queue[0].symbol!=SYMBOL_END_ALL);

    //int symbol_id=py_find_symbol(text);

    //if (symbol_id==-1) return py_error_set(PY_ERROR_UNDEFINED);

    return PY_ERROR_NONE;
}
