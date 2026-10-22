/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lstring_h
#నిర్వచించు lstring_h

#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#నిర్వచించు MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#ఒకవేళ !defined(LUAI_MAXSHORTLEN)
#నిర్వచించు LUAI_MAXSHORTLEN	40
#అయితే_ముగింపు


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#నిర్వచించు sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * పరిమాణం(అక్షరం))


#నిర్వచించు luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (పరిమాణం(s)/పరిమాణం(అక్షరం))-1))


/*
** test whether a string is a reserved word
*/
#నిర్వచించు isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#నిర్వచించు eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC గుర్తులేని luaS_hashlongstr (TString *ts);
LUAI_FUNC పూర్ణాంకం luaS_eqstr (TString *a, TString *b);
LUAI_FUNC శూన్య luaS_resize (lua_State *L, పూర్ణాంకం newsize);
LUAI_FUNC శూన్య luaS_clearcache (global_State *g);
LUAI_FUNC శూన్య luaS_init (lua_State *L);
LUAI_FUNC శూన్య luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              గుర్తులేని పొట్టి nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, స్థిరరాశి అక్షరం *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, స్థిరరాశి అక్షరం *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		స్థిరరాశి అక్షరం *s, size_t len, lua_Alloc falloc, శూన్య *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, పూర్ణాంకం రకం);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#అయితే_ముగింపు
