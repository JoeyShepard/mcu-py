//Tables used in execute.c for parsing

//Auto-generated tables from spreadsheet - DO NOT MODIFY!

#include <stdint.h>
#include "core.h"
#include "execute.h"
#include "tables.h"

//Paste tables from spreadsheet starting here:

const char py_keywords[]=
"\x3""abs"
"\x3""bin"
"\x3""chr"
"\x4""dict"
"\x6""divmod"
"\x3""hex"
"\x5""input"
"\x3""int"
"\x3""len"
"\x4""list"
"\x3""max"
"\x3""min"
"\x3""oct"
"\x3""ord"
"\x5""print"
"\x5""range"
"\x3""set"
"\x6""sorted"
"\x3""str"
"\x5""tuple"
"\x5""break"
"\x8""continue"
"\x3""def"
"\x3""del"
"\x4""elif"
"\x4""else"
"\x5""False"
"\x3""for"
"\x6""global"
"\x2""if"
"\x4""None"
"\x4""pass"
"\x6""return"
"\x4""True"
"\x5""while"
"\0";

const uint8_t py_keyword_token[]={
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
};

const uint8_t py_keyword_class[]={
COMPILE_BUILTIN,              //abs()
COMPILE_BUILTIN,              //bin()
COMPILE_BUILTIN,              //chr()
COMPILE_BUILTIN,              //dict()
COMPILE_BUILTIN,              //divmod()
COMPILE_BUILTIN,              //hex()
COMPILE_BUILTIN,              //input()
COMPILE_BUILTIN,              //int()
COMPILE_BUILTIN,              //len()
COMPILE_BUILTIN,              //list()
COMPILE_BUILTIN,              //max()
COMPILE_BUILTIN,              //min()
COMPILE_BUILTIN,              //oct()
COMPILE_BUILTIN,              //ord()
COMPILE_BUILTIN,              //print()
COMPILE_BUILTIN,              //range()
COMPILE_BUILTIN,              //set()
COMPILE_BUILTIN,              //sorted()
COMPILE_BUILTIN,              //str()
COMPILE_BUILTIN,              //tuple()
COMPILE_NO_ARG,               //break
COMPILE_NO_ARG,               //continue
COMPILE_DEF,                  //def
COMPILE_ARG_REQ,              //del
COMPILE_IFWHILE,              //elif
COMPILE_ELSE,                 //else
COMPILE_VALUE,                //False
COMPILE_FOR,                  //for
COMPILE_ARG_REQ,              //global
COMPILE_IFWHILE,              //if
COMPILE_VALUE,                //None
COMPILE_NO_ARG,               //pass
COMPILE_ARG_REQ,              //return
COMPILE_VALUE,                //True
COMPILE_IFWHILE,              //while
};

const uint8_t py_symbol_state[][PY_SYMBOL_ROW_SIZE]={
{SYMBOL_ALPHA|SYMBOL_ALPHA<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_HEX|SYMBOL_HEX<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_ALPHA}, //A_F
{SYMBOL_ALPHA|SYMBOL_ALPHA<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_ALPHA}, //G_Z_
{SYMBOL_ALPHA|SYMBOL_ALPHA<<4, SYMBOL_ERROR|SYMBOL_HEX_PRE<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_ALPHA}, //X
{SYMBOL_ZERO|SYMBOL_ALPHA<<4, SYMBOL_NUM|SYMBOL_ERROR<<4, SYMBOL_HEX|SYMBOL_HEX<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_ZERO}, //ZERO
{SYMBOL_NUM|SYMBOL_ALPHA<<4, SYMBOL_NUM|SYMBOL_ERROR<<4, SYMBOL_HEX|SYMBOL_HEX<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_NUM}, //NUM
{SYMBOL_STRING|SYMBOL_STRING<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_NONE|SYMBOL_COMMENT<<4, SYMBOL_STRING}, //QUOTE
{SYMBOL_SPACE|SYMBOL_SPACE<<4, SYMBOL_SPACE|SYMBOL_SPACE<<4, SYMBOL_ERROR|SYMBOL_SPACE<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_SPACE}, //SPACE
{SYMBOL_END_LINE|SYMBOL_END_LINE<<4, SYMBOL_END_LINE|SYMBOL_END_LINE<<4, SYMBOL_ERROR|SYMBOL_END_LINE<<4, SYMBOL_STRING|SYMBOL_END_LINE<<4, SYMBOL_END_LINE}, //END_LINE
{SYMBOL_END_ALL|SYMBOL_END_ALL<<4, SYMBOL_END_ALL|SYMBOL_END_ALL<<4, SYMBOL_ERROR|SYMBOL_END_ALL<<4, SYMBOL_ERROR|SYMBOL_END_ALL<<4, SYMBOL_END_ALL}, //END_ALL
{SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_ERROR|SYMBOL_ERROR<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_ERROR}, //OTHER
{SYMBOL_COMMENT|SYMBOL_COMMENT<<4, SYMBOL_COMMENT|SYMBOL_COMMENT<<4, SYMBOL_ERROR|SYMBOL_COMMENT<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_COMMENT}, //HASH
{SYMBOL_OP|SYMBOL_OP<<4, SYMBOL_OP|SYMBOL_OP<<4, SYMBOL_ERROR|SYMBOL_OP<<4, SYMBOL_STRING|SYMBOL_COMMENT<<4, SYMBOL_OP}, //OP
};

const uint8_t py_state_chars[]={
(INPUT_END_ALL<<4)|1,         //0
(INPUT_ERROR<<4)|8,           //1-8
(INPUT_SPACE<<4)|1,           //9
(INPUT_END_LINE<<4)|1,        //10
(INPUT_ERROR<<4)|2,           //11-12
(INPUT_END_LINE<<4)|1,        //13
(INPUT_ERROR<<4)|15,          //14-28
(INPUT_ERROR<<4)|3,           //29-31
(INPUT_SPACE<<4)|1,           //32
(INPUT_OP<<4)|1,              //33
(INPUT_OTHER<<4)|1,           //34
(INPUT_HASH<<4)|1,            //35
(INPUT_OTHER<<4)|1,           //36
(INPUT_OP<<4)|2,              //37-38
(INPUT_QUOTE<<4)|1,           //39
(INPUT_OP<<4)|8,              //40-47
(INPUT_ZERO<<4)|1,            //48
(INPUT_NUM<<4)|9,             //49-57
(INPUT_OP<<4)|1,              //58
(INPUT_END_LINE<<4)|1,        //59
(INPUT_OP<<4)|3,              //60-62
(INPUT_OTHER<<4)|2,           //63-64
(INPUT_A_F<<4)|6,             //65-70
(INPUT_G_Z_<<4)|15,           //71-85
(INPUT_G_Z_<<4)|5,            //86-90
(INPUT_OP<<4)|1,              //91
(INPUT_OTHER<<4)|1,           //92
(INPUT_OP<<4)|2,              //93-94
(INPUT_G_Z_<<4)|1,            //95
(INPUT_OTHER<<4)|1,           //96
(INPUT_A_F<<4)|6,             //97-102
(INPUT_G_Z_<<4)|15,           //103-117
(INPUT_G_Z_<<4)|2,            //118-119
(INPUT_X<<4)|1,               //120
(INPUT_G_Z_<<4)|2,            //121-122
(INPUT_OP<<4)|4,              //123-126
};

const char py_op_lookup_keys[]=".([{)]}:,=~*/%+-&^|!<>";

const uint8_t py_op_lookup_vals[]=
{
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
TOKEN_INVERT,
TOKEN_MUL,
TOKEN_DIV,
TOKEN_MOD,
TOKEN_ADD,
TOKEN_SUB,
TOKEN_AND,
TOKEN_XOR,
TOKEN_OR,
TOKEN_EXCLAM,
TOKEN_LT,
TOKEN_GT,
};

const char py_op_lookup_keys_long[][5]={"\x3""not", "\x3""and", "\x2""or", "\x2""in"};

const uint8_t py_op_lookup_vals_long[]={TOKEN_BOOL_NOT, TOKEN_BOOL_AND, TOKEN_BOOL_OR, TOKEN_IN};

const uint8_t py_op_combos[][PY_OP_COMBOS_SIZE]=
{
{TOKEN_EQ, TOKEN_EQ, TOKEN_EQ_EQ},
{TOKEN_EXP, TOKEN_EQ, TOKEN_EXP_EQ},
{TOKEN_MUL, TOKEN_EQ, TOKEN_MUL_EQ},
{TOKEN_DIV, TOKEN_EQ, TOKEN_DIV_EQ},
{TOKEN_MOD, TOKEN_EQ, TOKEN_MOD_EQ},
{TOKEN_ADD, TOKEN_EQ, TOKEN_ADD_EQ},
{TOKEN_SUB, TOKEN_EQ, TOKEN_SUB_EQ},
{TOKEN_LSHIFT, TOKEN_EQ, TOKEN_LSHIFT_EQ},
{TOKEN_RSHIFT, TOKEN_EQ, TOKEN_RSHIFT_EQ},
{TOKEN_AND, TOKEN_EQ, TOKEN_AND_EQ},
{TOKEN_XOR, TOKEN_EQ, TOKEN_XOR_EQ},
{TOKEN_OR, TOKEN_EQ, TOKEN_OR_EQ},
{TOKEN_LT, TOKEN_EQ, TOKEN_LE},
{TOKEN_GT, TOKEN_EQ, TOKEN_GE},
{TOKEN_EXCLAM, TOKEN_EQ, TOKEN_NE},
{TOKEN_MUL, TOKEN_MUL, TOKEN_EXP},
{TOKEN_LT, TOKEN_LT, TOKEN_LSHIFT},
{TOKEN_GT, TOKEN_GT, TOKEN_RSHIFT},
};

const uint8_t py_op_precedence[]={1, 3, 3, 1, 1, 12, 1, 3, 3, 2, 2, 1, 1, 1, 9, 1, 1, 1};

const uint8_t py_state_table[]=
{
1,(STATE_ERROR<<4)|STATE_VAL,(STATE_NONE<<4)|STATE_VAL,                         //SYMBOL
1,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_PERIOD
1,(STATE_NONE<<4)|STATE_LPAREN,(STATE_NONE<<4)|STATE_NONE,                      //TOKEN_LPAREN
1,(STATE_ERROR<<4)|STATE_LSBRACKET,(STATE_INV_NEG_PLUS<<4)|STATE_LCBRACKET,     //TOKEN_LSBRACKET
1,(STATE_ERROR<<4)|STATE_LCBRACKET,(STATE_INV_NEG_PLUS<<4)|STATE_VAL,           //TOKEN_LCBRACKET
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LPAREN,                       //TOKEN_RPAREN
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_BEGIN,                        //TOKEN_RSBRACKET
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LCBRACKET,                    //TOKEN_RCBRACKET
1,(STATE_BEGIN<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LSBRACKET,                  //TOKEN_COLON
13,(STATE_BEGIN<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                      //TOKEN_COMMA
1,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_EXP
3,(STATE_ERROR<<4)|STATE_INV_NEG_PLUS,(STATE_NONE<<4)|STATE_VAL,                //TOKEN_INVERT
3,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_MUL
2,(STATE_REQ<<4)|STATE_INV_NEG_PLUS,(STATE_NONE<<4)|STATE_VAL,                  //TOKEN_ADD
13,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                        //TOKEN_LSHIFT
1,(STATE_NONE<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_NONE,                       //TOKEN_EXCLAM
1,(STATE_ERROR<<4)|STATE_NOT,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_BOOL_NOT
2,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_BOOL_AND
};

const uint8_t py_compile_state[]={
COMPILE_LEFT_EXPRESSION, COMPILE_ERROR, 0xCE,                                   //COMPILE_BEGIN
COMPILE_LEFT_EXPRESSION, COMPILE_ASSIGN, 0x2,                                   //COMPILE_LEFT_EXPRESSION
COMPILE_RIGHT_EXPRESSION, COMPILE_ERROR, 0xCE,                                  //COMPILE_ASSIGN
COMPILE_RIGHT_EXPRESSION, COMPILE_ERROR, 0x2,                                   //COMPILE_RIGHT_EXPRESSION
COMPILE_FOR_EXPRESSION, COMPILE_ERROR, 0xCE,                                    //COMPILE_FOR
COMPILE_FOR_EXPRESSION, COMPILE_ERROR, COMPILE_FOR_IN, 0x24, 0x0,               //COMPILE_FOR_EXPRESSION
COMPILE_FOR_IN_EXPRESSION, COMPILE_ERROR, 0xCE,                                 //COMPILE_FOR_IN
COMPILE_FOR_IN_EXPRESSION, COMPILE_ERROR, COMPILE_FOR_COLON, 0x84, 0x0,         //COMPILE_FOR_IN_EXPRESSION
COMPILE_ERROR, 0, 0x0,                                                          //COMPILE_FOR_COLON
COMPILE_IFWHILE_EXPRESSION, COMPILE_ERROR, 0xCE,                                //COMPILE_IFWHILE
COMPILE_IFWHILE_EXPRESSION, COMPILE_ERROR, COMPILE_IFWHILE_COLON, 0x84, 0x0,    //COMPILE_IFWHILE_EXPRESSION
COMPILE_ERROR, 0, 0x0,                                                          //COMPILE_IFWHILE_COLON
COMPILE_ERROR, COMPILE_ELSE_COLON, 0x8,                                         //COMPILE_ELSE
COMPILE_ERROR, 0, 0x0,                                                          //COMPILE_ELSE_COLON
COMPILE_ERROR, COMPILE_DEF_ALPHA, 0x10,                                         //COMPILE_DEF
COMPILE_ERROR, COMPILE_DEF_LPAREN, 0x20,                                        //COMPILE_DEF_ALPHA
COMPILE_ERROR, COMPILE_DEF_VAR, 0x10,                                           //COMPILE_DEF_LPAREN
COMPILE_ERROR, COMPILE_DEF_RPAREN, COMPILE_DEF_COMMA, 0x0, 0x90,                //COMPILE_DEF_VAR
COMPILE_ERROR, COMPILE_DEF_VAR, 0x10,                                           //COMPILE_DEF_COMMA
COMPILE_ERROR, COMPILE_DEF_COLON, 0x8,                                          //COMPILE_DEF_RPAREN
COMPILE_ERROR, 0, 0x0,                                                          //COMPILE_DEF_COLON
COMPILE_ERROR, 0, 0x0,                                                          //COMPILE_NO_ARG
COMPILE_ARG_OPT_EXP, COMPILE_ERROR, 0xCE,                                       //COMPILE_ARG_OPT
COMPILE_ARG_OPT_EXP, COMPILE_ERROR, 0x2,                                        //COMPILE_ARG_OPT_EXP
COMPILE_ERROR, COMPILE_ARG_REQ_VAR, 0x10,                                       //COMPILE_ARG_REQ
COMPILE_ERROR, COMPILE_ARG_REQ_COMMA, 0x80,                                     //COMPILE_ARG_REQ_VAR
COMPILE_ERROR, COMPILE_ARG_REQ_VAR, 0x10,                                       //COMPILE_ARG_REQ_COMMA
};

const uint32_t py_compile_variant_count=0x204A0;

const uint32_t py_compile_done=0x54FD6F4;

