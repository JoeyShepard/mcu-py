#ifndef __GUARD_MCU_PY_TOKENS_PRIVATE
    #define __GUARD_MCU_PY_TOKENS_PRIVATE

    enum TokenTypes
    {
        //Used in source parser in execute.c
        TOKEN_NONE,
        TOKEN_SYMBOL=TOKEN_NONE,    //Reuse in py_state_table when input is symbol instead of token
        
        //Paste token list from spreadsheet starting here:
        
        TOKEN_PERIOD,
        TOKEN_LPAREN,
        TOKEN_LSBRACKET,
        TOKEN_LCBRACKET,
        TOKEN_RPAREN,
        TOKEN_RSBRACKET,
        TOKEN_RCBRACKET,
        TOKEN_COLON,
        TOKEN_COMMA,
        TOKEN_EQ,
        TOKEN_EXP_EQ,
        TOKEN_MUL_EQ,
        TOKEN_DIV_EQ,
        TOKEN_MOD_EQ,
        TOKEN_ADD_EQ,
        TOKEN_SUB_EQ,
        TOKEN_LSHIFT_EQ,
        TOKEN_RSHIFT_EQ,
        TOKEN_AND_EQ,
        TOKEN_XOR_EQ,
        TOKEN_OR_EQ,
        TOKEN_EXP,
        TOKEN_INVERT,
        TOKEN_NEG,
        TOKEN_PLUS,
        TOKEN_MUL,
        TOKEN_DIV,
        TOKEN_MOD,
        TOKEN_ADD,
        TOKEN_SUB,
        TOKEN_LSHIFT,
        TOKEN_RSHIFT,
        TOKEN_AND,
        TOKEN_XOR,
        TOKEN_OR,
        TOKEN_IN,
        TOKEN_NOT_IN,
        TOKEN_LT,
        TOKEN_LE,
        TOKEN_GT,
        TOKEN_GE,
        TOKEN_NE,
        TOKEN_EQ_EQ,
        TOKEN_EXCLAM,
        TOKEN_BOOL_NOT,
        TOKEN_BOOL_AND,
        TOKEN_BOOL_OR,
        //Tokens starting here are bytecode - saves space to reuse same token range
        TOKEN_INT8,
        TOKEN_INT16,
        TOKEN_INT32,
        TOKEN_STRING,
        TOKEN_RETURN,
        TOKEN_FUNC,
        TOKEN_TUPLE,
        TOKEN_LIST,
        TOKEN_DICT,
        TOKEN_SET,
        TOKEN_NONE_OBJ,
        TOKEN_SLICE_INDEX,
        TOKEN_INDEX,
        TOKEN_GLOBAL,
        TOKEN_LOCAL,
        TOKEN_ATTRIBUTE,
        TOKEN_BUILTIN,
        //Tokens starting here are built-in functions and keywords
        TOKEN_FUNC_ABS,
        TOKEN_FUNC_BIN,
        TOKEN_FUNC_CHR,
        TOKEN_FUNC_DICT,
        TOKEN_FUNC_DIVMOD,
        TOKEN_FUNC_HEX,
        TOKEN_FUNC_INPUT,
        TOKEN_FUNC_INT,
        TOKEN_FUNC_LEN,
        TOKEN_FUNC_LIST,
        TOKEN_FUNC_MAX,
        TOKEN_FUNC_MIN,
        TOKEN_FUNC_OCT,
        TOKEN_FUNC_ORD,
        TOKEN_FUNC_PRINT,
        TOKEN_FUNC_RANGE,
        TOKEN_FUNC_SET,
        TOKEN_FUNC_SORTED,
        TOKEN_FUNC_STR,
        TOKEN_FUNC_TUPLE,
        TOKEN_KEYWORD_BREAK,
        TOKEN_KEYWORD_CONTINUE,
        TOKEN_KEYWORD_DEF,
        TOKEN_KEYWORD_DEL,
        TOKEN_KEYWORD_ELIF,
        TOKEN_KEYWORD_ELSE,
        TOKEN_KEYWORD_FALSE,
        TOKEN_KEYWORD_FOR,
        TOKEN_KEYWORD_GLOBAL,
        TOKEN_KEYWORD_IF,
        TOKEN_KEYWORD_NONE,
        TOKEN_KEYWORD_PASS,
        TOKEN_KEYWORD_RETURN,
        TOKEN_KEYWORD_TRUE,
        TOKEN_KEYWORD_WHILE,
        //Tokens starting here are for storing things like variable names on stack during compilation
        TOKEN_VAR_INFO,

    };

#endif
