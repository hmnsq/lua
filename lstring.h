/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h

#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#define MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#यदि !defined(LUAI_MAXSHORTLEN)
#define LUAI_MAXSHORTLEN	40
#endif


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#define sizestrshr(l)  \
	(दुरत्व(TString, contents) + ((l) + 1) * परिमाण(वर्ण))


#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (परिमाण(s)/परिमाण(वर्ण))-1))


/*
** test whether a string is a reserved word
*/
#define isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC अचिन्हित luaS_hashlongstr (TString *ts);
LUAI_FUNC पूर्णाङ्क luaS_eqstr (TString *a, TString *b);
LUAI_FUNC शून्य luaS_resize (lua_State *L, पूर्णाङ्क newsize);
LUAI_FUNC शून्य luaS_clearcache (global_State *g);
LUAI_FUNC शून्य luaS_init (lua_State *L);
LUAI_FUNC शून्य luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, माप_प्रकार s,
                                              अचिन्हित लघु nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ध्रुव वर्ण *str, माप_प्रकार l);
LUAI_FUNC TString *luaS_new (lua_State *L, ध्रुव वर्ण *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, माप_प्रकार l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ध्रुव वर्ण *s, माप_प्रकार len, lua_Alloc falloc, शून्य *ud);
LUAI_FUNC माप_प्रकार luaS_sizelngstr (माप_प्रकार len, पूर्णाङ्क kind);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#endif
