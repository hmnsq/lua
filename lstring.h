/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lstring_h
#घोषणा lstring_h

#समावेश "lgc.h"
#समावेश "lobject.h"
#समावेश "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#घोषणा MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#अगर !defined(LUAI_MAXSHORTLEN)
#घोषणा LUAI_MAXSHORTLEN	40
#पूर्ण_अगर


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#घोषणा sizestrshr(l)  \
	(दुरत्व(TString, contents) + ((l) + 1) * माप(अक्षर))


#घोषणा luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (माप(s)/माप(अक्षर))-1))


/*
** test whether a string is a reserved word
*/
#घोषणा isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#घोषणा eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC अचिन्हित luaS_hashlongstr (TString *ts);
LUAI_FUNC पूर्णांक luaS_eqstr (TString *a, TString *b);
LUAI_FUNC व्योम luaS_resize (lua_State *L, पूर्णांक newsize);
LUAI_FUNC व्योम luaS_clearcache (global_State *g);
LUAI_FUNC व्योम luaS_init (lua_State *L);
LUAI_FUNC व्योम luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, माप_प्रकार s,
                                              अचिन्हित लघु nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, स्थिर अक्षर *str, माप_प्रकार l);
LUAI_FUNC TString *luaS_new (lua_State *L, स्थिर अक्षर *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, माप_प्रकार l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		स्थिर अक्षर *s, माप_प्रकार len, lua_Alloc falloc, व्योम *ud);
LUAI_FUNC माप_प्रकार luaS_sizelngstr (माप_प्रकार len, पूर्णांक kind);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#पूर्ण_अगर
