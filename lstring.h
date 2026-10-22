/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lstring_h
#تعریف_کن lstring_h

#ضمن "lgc.h"
#ضمن "lobject.h"
#ضمن "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#تعریف_کن MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#إذا !defined(LUAI_MAXSHORTLEN)
#تعریف_کن LUAI_MAXSHORTLEN	40
#پایان_اگر


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#تعریف_کن sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * حجم(حرف))


#تعریف_کن luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (حجم(s)/حجم(حرف))-1))


/*
** test whether a string is a reserved word
*/
#تعریف_کن isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#تعریف_کن eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC بیعلامت luaS_hashlongstr (TString *ts);
LUAI_FUNC صحيح luaS_eqstr (TString *a, TString *b);
LUAI_FUNC فراغ luaS_resize (lua_State *L, صحيح newsize);
LUAI_FUNC فراغ luaS_clearcache (global_State *g);
LUAI_FUNC فراغ luaS_init (lua_State *L);
LUAI_FUNC فراغ luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              بیعلامت کوتاه nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ثابت حرف *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ثابت حرف *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ثابت حرف *s, size_t len, lua_Alloc falloc, فراغ *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, صحيح نوع);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#پایان_اگر
