/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ifndef llex_h
#define llex_h

#অন্তর্ভাব <limits.h>

#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#define FIRST_RESERVED	(UCHAR_MAX + 1)


#যদি !defined(LUA_ENV)
#define LUA_ENV		"_ENV"
#endif


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
গণনা RESERVED {
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
#define অঙ্ক_RESERVED	(cast_int(TK_WHILE-FIRST_RESERVED + 1))


প্রকারনির্দেশ সঙ্ঘ {
  lua_সংখ্যা r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


প্রকারনির্দেশ সংরচনা Token {
  পূর্ণাঙ্ক token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
প্রকারনির্দেশ সংরচনা LexState {
  পূর্ণাঙ্ক current;  /* current character (charint) */
  পূর্ণাঙ্ক linenumber;  /* input line counter */
  পূর্ণাঙ্ক lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  সংরচনা FuncState *fs;  /* current function (parser) */
  সংরচনা lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  Table *h;  /* to avoid collection/reuse strings */
  সংরচনা Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *source;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC শূন্য luaX_init (lua_State *L);
LUAI_FUNC শূন্য luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *source, পূর্ণাঙ্ক firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, ধ্রুব বর্ণ *str, size_t l);
LUAI_FUNC শূন্য luaX_next (LexState *ls);
LUAI_FUNC পূর্ণাঙ্ক luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, ধ্রুব বর্ণ *s);
LUAI_FUNC ধ্রুব বর্ণ *luaX_token2str (LexState *ls, পূর্ণাঙ্ক token);


#endif
