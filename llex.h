/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ llex_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ llex_h

#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਹਿਲਾ_RESERVED	(UCHAR_MAX + 1)


#ਜੇ !defined(LUA_ENV)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ENV		"_ENV"
#ਜੇ_ਅੰਤ


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
ਗਿਣਤੀ RESERVED {
  /* terminal symbols denoted by reserved words */
  TK_AND = ਪਹਿਲਾ_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_ਸਿਰਾ, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GLOBAL, TK_GOTO, TK_IF, TK_ਵਿੱਚ, TK_LOCAL, TK_NIL, TK_NOT, TK_OR,
  TK_REPEAT, TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
  /* other terminal symbols */
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};

/* number of reserved words */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_RESERVED	(cast_int(TK_WHILE-ਪਹਿਲਾ_RESERVED + 1))


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ {
  lua_ਸੰਖਿਆ r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Token {
  ਪੂਰਨ token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ LexState {
  ਪੂਰਨ current;  /* current character (charint) */
  ਪੂਰਨ linenumber;  /* input line counter */
  ਪੂਰਨ lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  ਬਣਤਰ FuncState *fs;  /* current function (parser) */
  ਬਣਤਰ lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  ਸਾਰਣੀ *h;  /* to avoid collection/reuse strings */
  ਬਣਤਰ Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *ਸਰੋਤ;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC ਖਾਲੀ luaX_init (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *ਸਰੋਤ, ਪੂਰਨ firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, ਅਚਲ ਅੱਖਰ *str, size_t l);
LUAI_FUNC ਖਾਲੀ lua_X_ਅਗਲਾ (LexState *ls);
LUAI_FUNC ਪੂਰਨ luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, ਅਚਲ ਅੱਖਰ *s);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaX_token2str (LexState *ls, ਪੂਰਨ token);


#ਜੇ_ਅੰਤ
