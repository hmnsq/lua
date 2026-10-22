/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lctype_h
#تعریف_کړه lctype_h

#ضمن "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#إذا !defined(LUA_USE_CTYPE)

#إذا 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#تعریف_کړه LUA_USE_CTYPE	0
#وإلا
/* must use standard C ctype */
#تعریف_کړه LUA_USE_CTYPE	1
#د_که_پای

#د_که_پای


#إذا !LUA_USE_CTYPE	/* { */

#ضمن <limits.h>

#ضمن "llimits.h"


#تعریف_کړه ALPHABIT	0
#تعریف_کړه DIGITBIT	1
#تعریف_کړه PRINTBIT	2
#تعریف_کړه SPACEBIT	3
#تعریف_کړه XDIGITBIT	4


#تعریف_کړه ماسک(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#تعریف_کړه testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#تعریف_کړه lislalpha(c)	testprop(c, ماسک(ALPHABIT))
#تعریف_کړه lislalnum(c)	testprop(c, (ماسک(ALPHABIT) | ماسک(DIGITBIT)))
#تعریف_کړه lisdigit(c)	testprop(c, ماسک(DIGITBIT))
#تعریف_کړه lisspace(c)	testprop(c, ماسک(SPACEBIT))
#تعریف_کړه lisprint(c)	testprop(c, ماسک(PRINTBIT))
#تعریف_کړه lisxdigit(c)	testprop(c, ماسک(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#تعریف_کړه ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ثابت lu_byte luai_ctype_[UCHAR_MAX + 2];)


#وإلا			/* }{ */

/*
** use standard C ctypes
*/

#ضمن <ctype.h>


#تعریف_کړه lislalpha(c)	(که_توری(c) || (c) == '_')
#تعریف_کړه lislalnum(c)	(که_توری_یا_عدد(c) || (c) == '_')
#تعریف_کړه lisdigit(c)	(که_عدد(c))
#تعریف_کړه lisspace(c)	(که_تشه(c))
#تعریف_کړه lisprint(c)	(isprint(c))
#تعریف_کړه lisxdigit(c)	(isxdigit(c))

#تعریف_کړه ltolower(c)	(tolower(c))

#د_که_پای			/* } */

#د_که_پای

