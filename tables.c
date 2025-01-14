//Tables used in execute.c for parsing

//Auto-generated tables from spreadsheet - DO NOT MODIFY!

#include <stdint.h>
#include "core.h"
#include "execute.h"
#include "tables.h"

//Paste tables from spreadsheet starting here:

const char *py_commands=
"\x3""int"
"\x3""hex"
"\x5""strip"
"\x6""rstrip"
"\x6""lstrip"
"\x5""split"
"\x3""for"
"\x5""while"
"\x4""True"
"\x5""False"
"\x2""if"
"\x4""else"
"\x4""elif"
"\x5""break"
"\x8""continue"
"\x4""join"
"\x3""len"
"\x3""def"
"\x3""del"
"\x6""return"
"\x4""None"
"\x4""pass"
"\x3""str"
"\x5""float"
"\x5""index"
"\x5""upper"
"\x5""lower"
"\x4""find"
"\x7""replace"
"\x7""reverse"
"\x4""sort"
"\x6""remove"
"\x6""insert"
"\x3""pop"
"\x5""items"
"\x4""keys"
"\x6""values"
"\x5""print"
"\x3""chr"
"\x3""ord"
"\x3""min"
"\x3""max"
"\x5""range"
"\x4""free"
"\x7""delvars"
"\x7""deldefs"
"\0";

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
(INPUT_OP<<4)|6,              //40-45
(INPUT_ERROR<<4)|1,           //46
(INPUT_OP<<4)|1,              //47
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

const char *py_op_lookup_keys="([{)]}:,=~*/%+-&^|!<>";

const uint8_t py_op_lookup_vals[]=
{
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

const char *py_op_lookup_keys_long[]={"\x3""not", "\x3""and", "\x2""or", "\x2""in"};

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

const uint8_t py_op_precedence[]={3, 3, 1, 1, 12, 1, 3, 3, 2, 2, 1, 1, 1, 9, 1, 1, 1};

const uint8_t py_state_table[]=
{
1,(STATE_ERROR<<4)|STATE_VAL,(STATE_NONE<<4)|STATE_VAL,                         //SYMBOL
1,(STATE_NONE<<4)|STATE_LPAREN,(STATE_NONE<<4)|STATE_NONE,                      //TOKEN_LPAREN
1,(STATE_ERROR<<4)|STATE_LSBRACKET,(STATE_INV_NEG_PLUS<<4)|STATE_LCBRACKET,     //TOKEN_LSBRACKET
1,(STATE_ERROR<<4)|STATE_LCBRACKET,(STATE_INV_NEG_PLUS<<4)|STATE_VAL,           //TOKEN_LCBRACKET
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LPAREN,                       //TOKEN_RPAREN
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LSBRACKET,                    //TOKEN_RSBRACKET
1,(STATE_VAL<<4)|STATE_ERROR,(STATE_VAL<<4)|STATE_LCBRACKET,                    //TOKEN_RCBRACKET
14,(STATE_BEGIN<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                      //TOKEN_COLON
1,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_EXP
3,(STATE_ERROR<<4)|STATE_INV_NEG_PLUS,(STATE_NONE<<4)|STATE_VAL,                //TOKEN_INVERT
3,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_MUL
2,(STATE_REQ<<4)|STATE_INV_NEG_PLUS,(STATE_NONE<<4)|STATE_VAL,                  //TOKEN_ADD
13,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                        //TOKEN_LSHIFT
1,(STATE_NONE<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_NONE,                       //TOKEN_EXCLAM
1,(STATE_ERROR<<4)|STATE_NOT,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_BOOL_NOT
2,(STATE_REQ<<4)|STATE_ERROR,(STATE_NONE<<4)|STATE_VAL,                         //TOKEN_BOOL_AND
};


