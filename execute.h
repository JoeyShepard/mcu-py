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
    #define PY_FUNC_MAX_ARGS    255         //Limit so arg count fits in one byte
    #define PY_STACK_VAR_SIZE   3           //Size on stack of var info during compilation (excluding name pointer.) See enum VarFields below.
   
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
        INPUT_PERIOD,
        //Not in spreadsheet - must come at end
        INPUT_ERROR
    };

    //Operator precedence names for first five groups which require special handling
    enum PrecedenceNames
    {
        PREC_PERIOD,    // . for attribute
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
        FLAG_ADVANCE_STATE=     2,      //Move syntax state machine to next state?
        FLAG_VALUE=             4,      //Symbol being processed is alpha/num/hex/str?
        FLAG_DONE=              8,      //Done processing source?
        FLAG_COMMA_LAST=        0x10,   //Last symbol processed was a comma?
        FLAG_COLON_LAST=        0x20,   //Last symbol processed was a colon?
        FLAG_ATTRIBUTE=         0x40    //Last 
    };

    //Flags for 16 bits of metadata for each opening ( [ and {
    enum OpeningFlags
    {
        //lists - two bits for colon count
        FLAG_LIST_MASK=         0xC000,     //Two bits to count number of colons in list slice
        //dicts/sets - reuse same two bits as lists
        FLAG_DICT_SET_MASK=     0xC000,     //Two bits for dict/set parse state    
        //Used for functions, sets, lists, and dicts
        FLAG_FUNC_DEREF=        0x2000,     //Whether preceded by value which must be function or dereference 
        FLAG_COUNT_MASK=        0x1FFF,     //Count of commas encountered
        FLAG_SHIFT_COUNT=       14          //Shift 16 bit flag to access top two bits
    };

    enum DictSetParseStates
    {
        PARSE_BEGIN=            0x0000,
        PARSE_SET=              0x4000,
        PARSE_DICT_COMMA_NEXT=  0x8000,
        PARSE_DICT_COLON_NEXT=  0xC000,
    };

    //Compile stack is interleaved with different types so only pop data of the desired class
    enum PopClasses
    {
        POP_CLOSING_FOUND,      //Closing bracket hit - all operators including ( { [
        POP_END_LINE,           //End of line - all operators including ( { [
        POP_OPERATORS,          //Shunting Yard - operators not including ( { [
        POP_OPENINGS,           //Find opening brackets - ( { [
        POP_VAR_INFO,           //Variable information
    };

    enum VarFlags
    {
        FLAG_GLOBAL,            //Whether marked with global keyword
        FLAG_WRITTEN,           //Whether written to
        FLAG_READ,              //Whether read from
    };

    enum VarFields
    {
        VAR_TOKEN,              //1 byte
        VAR_FLAGS,              //1 byte
        VAR_SIZE,               //1 byte
        VAR_NAME                //sizeof(const char *) - large on x86 but small on systems were RAM is scarce
    };

#endif
