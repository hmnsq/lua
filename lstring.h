/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lstring_h
#परिभाषय lstring_h

#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#परिभाषय MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#यदि !defined(LUAI_MAXSHORTLEN)
#परिभाषय LUAI_MAXSHORTLEN	40
#यदि_अन्तः


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#परिभाषय sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * परिमाण(वर्ण))


#परिभाषय luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (परिमाण(s)/परिमाण(वर्ण))-1))


/*
** test whether a string is a reserved word
*/
#परिभाषय isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#परिभाषय eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC अचिह्नित luaS_hashlongstr (TString *ts);
LUAI_FUNC पूर्णाङ्क luaS_eqstr (TString *a, TString *b);
LUAI_FUNC शून्य luaS_resize (lua_State *L, पूर्णाङ्क newsize);
LUAI_FUNC शून्य luaS_clearcache (global_State *g);
LUAI_FUNC शून्य luaS_init (lua_State *L);
LUAI_FUNC शून्य luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              अचिह्नित ह्रस्व nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ध्रुव वर्ण *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ध्रुव वर्ण *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ध्रुव वर्ण *s, size_t len, lua_Alloc falloc, शून्य *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, पूर्णाङ्क विधा);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#यदि_अन्तः
