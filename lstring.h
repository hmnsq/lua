/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lstring_h
#ವ್ಯಾಖ್ಯಾನಿಸು lstring_h

#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEMERRMSG       "not enough memory"


/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#ಆದರೆ !defined(LUAI_MAXSHORTLEN)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXSHORTLEN	40
#ಆದರೆ_ಅಂತ್ಯ


/*
** Size of a short TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#ವ್ಯಾಖ್ಯಾನಿಸು sizestrshr(l)  \
	(offsetof(TString, contents) + ((l) + 1) * ಗಾತ್ರ(ಅಕ್ಷರ))


#ವ್ಯಾಖ್ಯಾನಿಸು luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (ಗಾತ್ರ(s)/ಗಾತ್ರ(ಅಕ್ಷರ))-1))


/*
** test whether a string is a reserved word
*/
#ವ್ಯಾಖ್ಯಾನಿಸು isreserved(s)	(strisshr(s) && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#ವ್ಯಾಖ್ಯಾನಿಸು eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC ಚಿಹ್ನೆರಹಿತ luaS_hashlongstr (TString *ts);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaS_eqstr (TString *a, TString *b);
LUAI_FUNC ಶೂನ್ಯ luaS_resize (lua_State *L, ಪೂರ್ಣಾಂಕ newsize);
LUAI_FUNC ಶೂನ್ಯ luaS_clearcache (global_State *g);
LUAI_FUNC ಶೂನ್ಯ luaS_init (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s,
                                              ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);
LUAI_FUNC TString *luaS_newextlstr (lua_State *L,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud);
LUAI_FUNC size_t luaS_sizelngstr (size_t len, ಪೂರ್ಣಾಂಕ ಬಗೆ);
LUAI_FUNC TString *luaS_normstr (lua_State *L, TString *ts);

#ಆದರೆ_ಅಂತ್ಯ
