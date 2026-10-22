/*
** $Id: llex.h $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే llex_h
#నిర్వచించు llex_h

#అన్తర్భావ <limits.h>

#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lzio.h"


/*
** Single-char tokens (terminal symbols) are represented by their own
** numeric code. Other tokens start at the following value.
*/
#నిర్వచించు మొదటి_RESERVED	(UCHAR_MAX + 1)


#ఒకవేళ !defined(LUA_ENV)
#నిర్వచించు LUA_ENV		"_ENV"
#అయితే_ముగింపు


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER RESERVED"
*/
గణన RESERVED {
  /* terminal symbols denoted by reserved words */
  TK_AND = మొదటి_RESERVED, TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_చివర, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GLOBAL, TK_GOTO, TK_IF, TK_లోపల, TK_LOCAL, TK_NIL, TK_NOT, TK_OR,
  TK_REPEAT, TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
  /* other terminal symbols */
  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};

/* number of reserved words */
#నిర్వచించు అంకె_RESERVED	(cast_int(TK_WHILE-మొదటి_RESERVED + 1))


రకనిర్వచనం సంఘం {
  lua_సంఖ్య r;
  lua_Integer i;
  TString *ts;
} SemInfo;  /* semantics information */


రకనిర్వచనం నిర్మాణం Token {
  పూర్ణాంకం token;
  SemInfo seminfo;
} Token;


/* state of the scanner plus state of the parser when shared by all
   functions */
రకనిర్వచనం నిర్మాణం LexState {
  పూర్ణాంకం current;  /* current character (charint) */
  పూర్ణాంకం linenumber;  /* input line counter */
  పూర్ణాంకం lastline;  /* line of last token 'consumed' */
  Token t;  /* current token */
  Token lookahead;  /* look ahead token */
  నిర్మాణం FuncState *fs;  /* current function (parser) */
  నిర్మాణం lua_State *L;
  ZIO *z;  /* input stream */
  Mbuffer *buff;  /* buffer for tokens */
  పట్టిక *h;  /* to avoid collection/reuse strings */
  నిర్మాణం Dyndata *dyd;  /* dynamic structures used by the parser */
  TString *ఆధారం;  /* current source name */
  TString *envn;  /* environment variable name */
  TString *brkn;  /* "break" name (used as a label) */
  TString *glbn;  /* "global" name (when not a reserved word) */
} LexState;


LUAI_FUNC శూన్య luaX_init (lua_State *L);
LUAI_FUNC శూన్య luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *ఆధారం, పూర్ణాంకం firstchar);
LUAI_FUNC TString *luaX_newstring (LexState *ls, స్థిరరాశి అక్షరం *str, size_t l);
LUAI_FUNC శూన్య lua_X_తదుపరి (LexState *ls);
LUAI_FUNC పూర్ణాంకం luaX_lookahead (LexState *ls);
LUAI_FUNC l_noret luaX_syntaxerror (LexState *ls, స్థిరరాశి అక్షరం *s);
LUAI_FUNC స్థిరరాశి అక్షరం *luaX_token2str (LexState *ls, పూర్ణాంకం token);


#అయితే_ముగింపు
