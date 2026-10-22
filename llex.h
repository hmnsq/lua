/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ifndef llex_h
#define llex_h

#شامل <limits.h>

#شامل "lobject.h"
#شامل "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#define FIRST_RESERVED	(UCHAR_MAX + 1)


#اگر !defined(LUA_ENV)
#define LUA_ENV		"_ENV"
#endif


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
شمارش RESERVED {
  /* terminal symbols denoted by reserved words */
  TK_AND = FIRST_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GLOBAL, TK_GOTO, TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR,
  TK_REPEAT, TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
  /* other terminal symbols */
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};

/* number of reserved words */
#define رقم_RESERVED	(cast_int(TK_WHILE-FIRST_RESERVED + 1))


تعریفنوع اجتماع {
  lua_عدد r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


تعریفنوع ساختار Token {
  صحیح token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
تعریفنوع ساختار LexState {
  صحیح current;  /* current character (charint) */
  صحیح linenumber;  /* input line counter */
  صحیح lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  ساختار FuncState *fs;  /* current function (parser) */
  ساختار lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  Table *h;  /* to avoid collection/reuse strings */
  ساختار Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *source;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC تهی luaX_init (lua_State *L);
LUAI_FUNC تهی luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *source, صحیح firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, ثابت نویسه *str, size_t l);
LUAI_FUNC تهی luaX_next (LexState *ls);
LUAI_FUNC صحیح luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, ثابت نویسه *s);
LUAI_FUNC ثابت نویسه *luaX_token2str (LexState *ls, صحیح token);


#endif
