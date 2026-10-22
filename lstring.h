/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h

#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"


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
#যদি !defined(LUAI_MAXSHORTLEN)
#define LUAI_MAXSHORTLEN	40
#endif


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#define sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * পরিমাণ(ৱর্ণ))


#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (পরিমাণ(s)/পরিমাণ(ৱর্ণ))-1))


/*
** test whether a string is a reserved word
*/
#define isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC unsigned luaS_hashlongstr (TString *ts);
LUAI_FUNC পূর্ণাঙ্ক luaS_eqstr (TString *a, TString *b);
LUAI_FUNC শূন্য luaS_resize (lua_State *L, পূর্ণাঙ্ক newsize);
LUAI_FUNC শূন্য luaS_clearcache (global_State *g);
LUAI_FUNC শূন্য luaS_init (lua_State *L);
LUAI_FUNC শূন্য luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              unsigned short nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ধ্রুৱ ৱর্ণ *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ধ্রুৱ ৱর্ণ *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ধ্রুৱ ৱর্ণ *s, size_t len, lua_Alloc falloc, শূন্য *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, পূর্ণাঙ্ক kind);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#endif
