#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "execute_public.h"

#ifndef __GUARD_MCU_PY_EXECUTE_PRIVATE
    #define __GUARD_MCU_PY_EXECUTE_PRIVATE

    //Constants
    //=========
    #define PY_CHAR_LAST_VALID  126         //Characters in source after this are invalid
    #define PY_CHAR_MASK        0xF         //Lower 4 bits of entry in py_state_chars is size of range
    #define PY_SYMBOL_MASK      0xF         //Lookup in py_state_symbols stored 4 bits each
   
    //Interpreter states
    #define STATE_ENTRY_SIZE    3           //Size in bytes of entry in py_state_table in tables.c
    enum InterpreterStates
    {
        STATE_NONE,
        STATE_BEGIN,
        STATE_LPAREN,
        STATE_LSBRACKET,
        STATE_LCBRACKET,
        STATE_VAL,
        STATE_REQ,
        STATE_INV_NEG_PLUS,
        STATE_NOT,
        STATE_ERROR
    };
    
    enum SymbolTypes
    {
        //Must be in same order as spreadsheet
        SYMBOL_NONE,                //used in state machine table but not returned by py_next_symbol
        SYMBOL_ALPHA,               //alpha, num, underscore
        SYMBOL_NUM,                 //0-9 (no floats yet)
        SYMBOL_ZERO,                //leading 0 so just 0 or hex
        SYMBOL_HEX_PRE,             //leading 0x only
        SYMBOL_HEX,                 //leading 0x with at least one valid hex digit
        SYMBOL_STRING,              //delimited by ' but not "
        SYMBOL_COMMENT,             //comment characters - ignore
        SYMBOL_SPACE,               //one or more spaces to differentiate eg == and = =
        //Not in spreadsheet - must come at end of enum here
        SYMBOL_RETURN,              //symbols from here on return immediately when parsed
        SYMBOL_OP=SYMBOL_RETURN,    //all operators: +, -, (, ), [, ] etc
        SYMBOL_ERROR,               //invalid state - throw error
        SYMBOL_END_LINE,            //end of current line in source
        SYMBOL_END_ALL              //end of source
    };

    enum SymbolInputs
    {
        //Must be in same order as spreadsheet
        INPUT_A_F,          //A-F, a-f
        INPUT_G_Z_,         //G-Z, g-z except x, underscore
        INPUT_X,            //lowercase x
        INPUT_ZERO,         //leading 0
        INPUT_NUM,          //1-9 (no floats for now)
        INPUT_QUOTE,        //only '
        INPUT_SPACE,
        INPUT_END_LINE,     //end of line but not end of source
        INPUT_END_ALL,      //end of source
        INPUT_OTHER,        //printable characters with no function: $, @, ? etc
        INPUT_HASH,         //#
        INPUT_OP,           //all operators: +, -, (, ), [, ] etc
        //Not in spreadsheet - must come at end
        INPUT_ERROR
    };

    //Operator precedence names for first five groups which require special handling
    enum PrecedenceNames
    {
        PREC_OPENING,   // ( [ {
        PREC_CLOSING,   // ) ] }
        PREC_COLON,     // : 
        PREC_COMMA,     // ,
        PREC_ASSIGN     // = += -= etc
    };

    //Binary flags for py_execute
    enum ExecuteFlags
    {
        FLAG_RESET_STATE=       1,      //Reset state at beginning of each line of source
        FLAG_ADVANCE_STATE=     2,      //Move syntax state machine to next state
        FLAG_VALUE=             4,      //Whether symbol being processed is alpha/num/hex/str or not
        FLAG_IN_FUNC=           8,      //Whether code is being compiled in a function or not
    };

    //Flags for 16 bits of metadata for each opening ( [ and {
    enum OpeningFlags
    {
        FLAG_FUNC_DEREF=        0x8000, //Whether preceded by value which is then function or dereference 
    };

    //Functions
    //=========

    //TODO: arrange functions by group
    //Static functions - not used outside of execute.c
    static bool py_cstrcmp(const char *str1, uint16_t len1, const char *str2, uint16_t len2);
    static bool py_strchr(const char *str, char ch, size_t size);
    static uint8_t py_classify_input(const char input_char);
    static uint8_t py_lookup_op(char op);
    static uint8_t py_next_symbol(const char **text, uint16_t *len);
    static int16_t py_find_symbol(const char *symbol_begin, uint8_t symbol_len);
    static py_error_t py_token_push(uint8_t token);
    static py_error_t py_uint16_push(uint16_t data);
    static uint8_t py_token_pop();
    static uint16_t py_uint16_pop();
    static uint8_t py_find_precedence(uint8_t token);
    //TODO: move to core.c?
    static py_error_t py_append(uint8_t *obj, void *data, uint16_t data_size);

#endif
