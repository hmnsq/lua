/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lctype_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lctype_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#ਜੇ !defined(LUA_USE_CTYPE)

#ਜੇ 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_CTYPE	0
#ਨਹੀਂਤਾਂ
/* must use standard C ctype */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_CTYPE	1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ


#ਜੇ !LUA_USE_CTYPE	/* { */

#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALPHABIT	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIGITBIT	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRINTBIT	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPACEBIT	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XDIGITBIT	4


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਨਕਾਬ(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lislalpha(c)	testprop(c, ਨਕਾਬ(ALPHABIT))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lislalnum(c)	testprop(c, (ਨਕਾਬ(ALPHABIT) | ਨਕਾਬ(DIGITBIT)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisdigit(c)	testprop(c, ਨਕਾਬ(DIGITBIT))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisspace(c)	testprop(c, ਨਕਾਬ(SPACEBIT))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisprint(c)	testprop(c, ਨਕਾਬ(PRINTBIT))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisxdigit(c)	testprop(c, ਨਕਾਬ(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ਅਚਲ lu_byte luai_ctype_[UCHAR_MAX + 2];)


#ਨਹੀਂਤਾਂ			/* }{ */

/*
** use standard C ctypes
*/

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lislalpha(c)	(ਅੱਖਰ_ਹੈ(c) || (c) == '_')
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lislalnum(c)	(ਅੱਖਰ_ਜਾਂ_ਅੰਕ(c) || (c) == '_')
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisdigit(c)	(ਅੰਕ_ਹੈ(c))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisspace(c)	(ਖਾਲੀ_ਥਾਂ_ਹੈ(c))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisprint(c)	(isprint(c))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lisxdigit(c)	(isxdigit(c))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltolower(c)	(tolower(c))

#ਜੇ_ਅੰਤ			/* } */

#ਜੇ_ਅੰਤ

