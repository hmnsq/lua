/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#ifndef llex_h
#define llex_h

#अन्तर्भाव <limits.h>

#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#define FIRST_RESERVED	(अच_अक्षर_उच्च + 1)


#यदि !defined(LUA_ENV)
#define LUA_ENV		"_ENV"
#endif


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
गणना RESERVED {
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
#define अङ्क_RESERVED	(cast_int(TK_WHILE-FIRST_RESERVED + 1))


प्रकारनिर्देश सङ्घ {
  lua_सङ्ख्या r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


प्रकारनिर्देश संरचना Token {
  पूर्णाङ्क token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
प्रकारनिर्देश संरचना LexState {
  पूर्णाङ्क current;  /* current character (charint) */
  पूर्णाङ्क linenumber;  /* input line counter */
  पूर्णाङ्क lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  संरचना FuncState *fs;  /* current function (parser) */
  संरचना lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  Table *h;  /* to avoid collection/reuse strings */
  संरचना Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *source;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC शून्य luaX_init (lua_State *L);
LUAI_FUNC शून्य luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *source, पूर्णाङ्क firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, अचर वर्ण *str, माप_प्रकार l);
LUAI_FUNC शून्य luaX_next (LexState *ls);
LUAI_FUNC पूर्णाङ्क luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, अचर वर्ण *s);
LUAI_FUNC अचर वर्ण *luaX_token2str (LexState *ls, पूर्णाङ्क token);


#endif
