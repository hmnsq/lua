/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lctype_h
#تعریف_کرو lctype_h

#شامل "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#اگر !defined(LUA_USE_CTYPE)

#اگر 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#تعریف_کرو LUA_USE_CTYPE	0
#ورنہ
/* must use standard C ctype */
#تعریف_کرو LUA_USE_CTYPE	1
#اگر_ختم

#اگر_ختم


#اگر !LUA_USE_CTYPE	/* { */

#شامل <limits.h>

#شامل "llimits.h"


#تعریف_کرو ALPHABIT	0
#تعریف_کرو DIGITBIT	1
#تعریف_کرو PRINTBIT	2
#تعریف_کرو SPACEBIT	3
#تعریف_کرو XDIGITBIT	4


#تعریف_کرو نقاب(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#تعریف_کرو testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#تعریف_کرو lislalpha(c)	testprop(c, نقاب(ALPHABIT))
#تعریف_کرو lislalnum(c)	testprop(c, (نقاب(ALPHABIT) | نقاب(DIGITBIT)))
#تعریف_کرو lisdigit(c)	testprop(c, نقاب(DIGITBIT))
#تعریف_کرو lisspace(c)	testprop(c, نقاب(SPACEBIT))
#تعریف_کرو lisprint(c)	testprop(c, نقاب(PRINTBIT))
#تعریف_کرو lisxdigit(c)	testprop(c, نقاب(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#تعریف_کرو ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ثابت lu_byte luai_ctype_[UCHAR_MAX + 2];)


#ورنہ			/* }{ */

/*
** use standard C ctypes
*/

#شامل <ctype.h>


#تعریف_کرو lislalpha(c)	(حرف_ہے(c) || (c) == '_')
#تعریف_کرو lislalnum(c)	(حرف_یا_ہندسہ(c) || (c) == '_')
#تعریف_کرو lisdigit(c)	(ہندسہ_ہے(c))
#تعریف_کرو lisspace(c)	(خلا_ہے(c))
#تعریف_کرو lisprint(c)	(isprint(c))
#تعریف_کرو lisxdigit(c)	(isxdigit(c))

#تعریف_کرو ltolower(c)	(tolower(c))

#اگر_ختم			/* } */

#اگر_ختم

