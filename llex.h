/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ llex_h
#ವ್ಯಾಖ್ಯಾನಿಸು llex_h

#ಅನ್ತರ್ಭಾವ <limits.h>

#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಮೊದಲ_RESERVED	(UCHAR_MAX + 1)


#ಆದರೆ !defined(LUA_ENV)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ENV		"_ENV"
#ಆದರೆ_ಅಂತ್ಯ


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
ಎಣಿಕೆ RESERVED {
  /* terminal symbols denoted by reserved words */
  TK_AND = ಮೊದಲ_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_ಕೊನೆ, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GLOBAL, TK_GOTO, TK_IF, TK_ಒಳ, TK_LOCAL, TK_NIL, TK_NOT, TK_OR,
  TK_REPEAT, TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
  /* other terminal symbols */
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};

/* number of reserved words */
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_RESERVED	(cast_int(TK_WHILE-ಮೊದಲ_RESERVED + 1))


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ {
  lua_ಸಂಖ್ಯೆ r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Token {
  ಪೂರ್ಣಾಂಕ token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ LexState {
  ಪೂರ್ಣಾಂಕ current;  /* current character (charint) */
  ಪೂರ್ಣಾಂಕ linenumber;  /* input line counter */
  ಪೂರ್ಣಾಂಕ lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  ರಚನೆ FuncState *fs;  /* current function (parser) */
  ರಚನೆ lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  ಕೋಷ್ಟಕ *h;  /* to avoid collection/reuse strings */
  ರಚನೆ Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *ಮೂಲ;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC ಶೂನ್ಯ luaX_init (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *ಮೂಲ, ಪೂರ್ಣಾಂಕ firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l);
LUAI_FUNC ಶೂನ್ಯ lua_X_ಮುಂದಿನ (LexState *ls);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaX_token2str (LexState *ls, ಪೂರ್ಣಾಂಕ token);


#ಆದರೆ_ಅಂತ್ಯ
