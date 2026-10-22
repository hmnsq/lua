/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#अगर_अघोषित llex_h
#घोषणा llex_h

#समावेश <limits.h>

#समावेश "lobject.h"
#समावेश "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#घोषणा FIRST_RESERVED	(अच_अक्षर_उच्च + 1)


#अगर !defined(LUA_ENV)
#घोषणा LUA_ENV		"_ENV"
#पूर्ण_अगर


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
क्रमागत RESERVED {
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
#घोषणा अंक_RESERVED	(cast_int(TK_WHILE-FIRST_RESERVED + 1))


प्रकार जोड़ {
  lua_संख्या r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


प्रकार काष्ठा Token {
  पूर्णांक token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
प्रकार काष्ठा LexState {
  पूर्णांक current;  /* current character (charint) */
  पूर्णांक linenumber;  /* input line counter */
  पूर्णांक lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  काष्ठा FuncState *fs;  /* current function (parser) */
  काष्ठा lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  Table *h;  /* to avoid collection/reuse strings */
  काष्ठा Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *source;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC व्योम luaX_init (lua_State *L);
LUAI_FUNC व्योम luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *source, पूर्णांक firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, स्थिर अक्षर *str, माप_प्रकार l);
LUAI_FUNC व्योम lua_X_अग्र (LexState *ls);
LUAI_FUNC पूर्णांक luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, स्थिर अक्षर *s);
LUAI_FUNC स्थिर अक्षर *luaX_token2str (LexState *ls, पूर्णांक token);


#पूर्ण_अगर
