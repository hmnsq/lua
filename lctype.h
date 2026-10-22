/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lctype_h
#وصف_ڪريو lctype_h

#ضمن "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#إذا !defined(LUA_USE_CTYPE)

#إذا 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#وصف_ڪريو LUA_USE_CTYPE	0
#وإلا
/* must use standard C ctype */
#وصف_ڪريو LUA_USE_CTYPE	1
#جيڪڏهن_پورو

#جيڪڏهن_پورو


#إذا !LUA_USE_CTYPE	/* { */

#ضمن <limits.h>

#ضمن "llimits.h"


#وصف_ڪريو ALPHABIT	0
#وصف_ڪريو DIGITBIT	1
#وصف_ڪريو PRINTBIT	2
#وصف_ڪريو SPACEBIT	3
#وصف_ڪريو XDIGITBIT	4


#وصف_ڪريو نقاب(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#وصف_ڪريو testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#وصف_ڪريو lislalpha(c)	testprop(c, نقاب(ALPHABIT))
#وصف_ڪريو lislalnum(c)	testprop(c, (نقاب(ALPHABIT) | نقاب(DIGITBIT)))
#وصف_ڪريو lisdigit(c)	testprop(c, نقاب(DIGITBIT))
#وصف_ڪريو lisspace(c)	testprop(c, نقاب(SPACEBIT))
#وصف_ڪريو lisprint(c)	testprop(c, نقاب(PRINTBIT))
#وصف_ڪريو lisxdigit(c)	testprop(c, نقاب(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#وصف_ڪريو ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ثابت lu_byte luai_ctype_[UCHAR_MAX + 2];)


#وإلا			/* }{ */

/*
** use standard C ctypes
*/

#ضمن <ctype.h>


#وصف_ڪريو lislalpha(c)	(اکر_آهي(c) || (c) == '_')
#وصف_ڪريو lislalnum(c)	(اکر_يا_انگ(c) || (c) == '_')
#وصف_ڪريو lisdigit(c)	(انگ_آهي(c))
#وصف_ڪريو lisspace(c)	(خال_آهي(c))
#وصف_ڪريو lisprint(c)	(isprint(c))
#وصف_ڪريو lisxdigit(c)	(isxdigit(c))

#وصف_ڪريو ltolower(c)	(tolower(c))

#جيڪڏهن_پورو			/* } */

#جيڪڏهن_پورو

