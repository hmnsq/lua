/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lstring_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lstring_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#ਜੇ !defined(LUAI_MAXSHORTLEN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXSHORTLEN	40
#ਜੇ_ਅੰਤ


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * ਆਕਾਰ(ਅੱਖਰ))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (ਆਕਾਰ(s)/ਆਕਾਰ(ਅੱਖਰ))-1))


/*
** test whether a string is a reserved word
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC ਬਿਨਾਨਿਸ਼ਾਨੀ luaS_hashlongstr (TString *ts);
LUAI_FUNC ਪੂਰਨ luaS_eqstr (TString *a, TString *b);
LUAI_FUNC ਖਾਲੀ luaS_resize (lua_State *L, ਪੂਰਨ newsize);
LUAI_FUNC ਖਾਲੀ luaS_clearcache (global_State *g);
LUAI_FUNC ਖਾਲੀ luaS_init (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ਅਚਲ ਅੱਖਰ *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ਅਚਲ ਅੱਖਰ *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ਅਚਲ ਅੱਖਰ *s, size_t len, lua_Alloc falloc, ਖਾਲੀ *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, ਪੂਰਨ ਕਿਸਮ);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#ਜੇ_ਅੰਤ
