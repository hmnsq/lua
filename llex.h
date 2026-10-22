/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ llex_h
#تعریف_کرو llex_h

#شامل <limits.h>

#شامل "lobject.h"
#شامل "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#تعریف_کرو پہلا_RESERVED	(UCHAR_MAX + 1)


#اگر !defined(LUA_ENV)
#تعریف_کرو LUA_ENV		"_ENV"
#اگر_ختم


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
شمار RESERVED {
  /* terminal symbols denoted by reserved words */
  TK_AND = پہلا_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_انتہا, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GLOBAL, TK_GOTO, TK_IF, TK_اندر, TK_LOCAL, TK_NIL, TK_NOT, TK_OR,
  TK_REPEAT, TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
  /* other terminal symbols */
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};

/* number of reserved words */
#تعریف_کرو عدد_RESERVED	(cast_int(TK_WHILE-پہلا_RESERVED + 1))


قسمتعریف اتحاد {
  lua_نمبر r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


قسمتعریف ساخت Token {
  صحیح token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
قسمتعریف ساخت LexState {
  صحیح current;  /* current character (charint) */
  صحیح linenumber;  /* input line counter */
  صحیح lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  ساخت FuncState *fs;  /* current function (parser) */
  ساخت lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  جدول *h;  /* to avoid collection/reuse strings */
  ساخت Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *ماخذ;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC خالی luaX_init (lua_State *L);
LUAI_FUNC خالی luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *ماخذ, صحیح firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, ثابت حرف *str, size_t l);
LUAI_FUNC خالی lua_X_اگلا (LexState *ls);
LUAI_FUNC صحیح luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, ثابت حرف *s);
LUAI_FUNC ثابت حرف *luaX_token2str (LexState *ls, صحیح token);


#اگر_ختم
