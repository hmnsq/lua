/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lctype_h
#ವ್ಯಾಖ್ಯಾನಿಸು lctype_h

#ಅನ್ತರ್ಭಾವ "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#ಆದರೆ !defined(LUA_USE_CTYPE)

#ಆದರೆ 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_CTYPE	0
#ಇಲ್ಲದಿದ್ದರೆ
/* must use standard C ctype */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_CTYPE	1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ !LUA_USE_CTYPE	/* { */

#ಅನ್ತರ್ಭಾವ <limits.h>

#ಅನ್ತರ್ಭಾವ "llimits.h"


#ವ್ಯಾಖ್ಯಾನಿಸು ALPHABIT	0
#ವ್ಯಾಖ್ಯಾನಿಸು DIGITBIT	1
#ವ್ಯಾಖ್ಯಾನಿಸು PRINTBIT	2
#ವ್ಯಾಖ್ಯಾನಿಸು SPACEBIT	3
#ವ್ಯಾಖ್ಯಾನಿಸು XDIGITBIT	4


#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಖವಾಡ(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lislalpha(c)	testprop(c, ಮುಖವಾಡ(ALPHABIT))
#ವ್ಯಾಖ್ಯಾನಿಸು lislalnum(c)	testprop(c, (ಮುಖವಾಡ(ALPHABIT) | ಮುಖವಾಡ(DIGITBIT)))
#ವ್ಯಾಖ್ಯಾನಿಸು lisdigit(c)	testprop(c, ಮುಖವಾಡ(DIGITBIT))
#ವ್ಯಾಖ್ಯಾನಿಸು lisspace(c)	testprop(c, ಮುಖವಾಡ(SPACEBIT))
#ವ್ಯಾಖ್ಯಾನಿಸು lisprint(c)	testprop(c, ಮುಖವಾಡ(PRINTBIT))
#ವ್ಯಾಖ್ಯಾನಿಸು lisxdigit(c)	testprop(c, ಮುಖವಾಡ(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ಸ್ಥಿರಾಂಕ lu_byte luai_ctype_[UCHAR_MAX + 2];)


#ಇಲ್ಲದಿದ್ದರೆ			/* }{ */

/*
** use standard C ctypes
*/

#ಅನ್ತರ್ಭಾವ <ctype.h>


#ವ್ಯಾಖ್ಯಾನಿಸು lislalpha(c)	(ಅಕ್ಷರವೇ(c) || (c) == '_')
#ವ್ಯಾಖ್ಯಾನಿಸು lislalnum(c)	(ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(c) || (c) == '_')
#ವ್ಯಾಖ್ಯಾನಿಸು lisdigit(c)	(ಅಂಕಿಯೇ(c))
#ವ್ಯಾಖ್ಯಾನಿಸು lisspace(c)	(ಖಾಲಿಯೇ(c))
#ವ್ಯಾಖ್ಯಾನಿಸು lisprint(c)	(isprint(c))
#ವ್ಯಾಖ್ಯಾನಿಸು lisxdigit(c)	(isxdigit(c))

#ವ್ಯಾಖ್ಯಾನಿಸು ltolower(c)	(tolower(c))

#ಆದರೆ_ಅಂತ್ಯ			/* } */

#ಆದರೆ_ಅಂತ್ಯ

