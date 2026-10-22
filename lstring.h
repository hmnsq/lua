/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lstring_h
#تعریف_کرو lstring_h

#شامل "lgc.h"
#شامل "lobject.h"
#شامل "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#تعریف_کرو MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#اگر !defined(LUAI_MAXSHORTLEN)
#تعریف_کرو LUAI_MAXSHORTLEN	40
#اگر_ختم


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#تعریف_کرو sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * حجم(حرف))


#تعریف_کرو luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (حجم(s)/حجم(حرف))-1))


/*
** test whether a string is a reserved word
*/
#تعریف_کرو isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#تعریف_کرو eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC غیرعلامتی luaS_hashlongstr (TString *ts);
LUAI_FUNC صحیح luaS_eqstr (TString *a, TString *b);
LUAI_FUNC خالی luaS_resize (lua_State *L, صحیح newsize);
LUAI_FUNC خالی luaS_clearcache (global_State *g);
LUAI_FUNC خالی luaS_init (lua_State *L);
LUAI_FUNC خالی luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              غیرعلامتی مختصر nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ثابت حرف *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ثابت حرف *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ثابت حرف *s, size_t len, lua_Alloc falloc, خالی *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, صحیح قسم);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#اگر_ختم
