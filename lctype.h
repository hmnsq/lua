/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lctype_h
#تعریف_کن lctype_h

#ضمن "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#إذا !defined(LUA_USE_CTYPE)

#إذا 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#تعریف_کن LUA_USE_CTYPE	0
#وإلا
/* must use standard C ctype */
#تعریف_کن LUA_USE_CTYPE	1
#پایان_اگر

#پایان_اگر


#إذا !LUA_USE_CTYPE	/* { */

#ضمن <limits.h>

#ضمن "llimits.h"


#تعریف_کن ALPHABIT	0
#تعریف_کن DIGITBIT	1
#تعریف_کن PRINTBIT	2
#تعریف_کن SPACEBIT	3
#تعریف_کن XDIGITBIT	4


#تعریف_کن ماسک(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#تعریف_کن testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#تعریف_کن lislalpha(c)	testprop(c, ماسک(ALPHABIT))
#تعریف_کن lislalnum(c)	testprop(c, (ماسک(ALPHABIT) | ماسک(DIGITBIT)))
#تعریف_کن lisdigit(c)	testprop(c, ماسک(DIGITBIT))
#تعریف_کن lisspace(c)	testprop(c, ماسک(SPACEBIT))
#تعریف_کن lisprint(c)	testprop(c, ماسک(PRINTBIT))
#تعریف_کن lisxdigit(c)	testprop(c, ماسک(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#تعریف_کن ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ثابت lu_byte luai_ctype_[UCHAR_MAX + 2];)


#وإلا			/* }{ */

/*
** use standard C ctypes
*/

#ضمن <ctype.h>


#تعریف_کن lislalpha(c)	(آیا_حرف(c) || (c) == '_')
#تعریف_کن lislalnum(c)	(آیا_حرف_یا_رقم(c) || (c) == '_')
#تعریف_کن lisdigit(c)	(آیا_رقم(c))
#تعریف_کن lisspace(c)	(آیا_فاصله(c))
#تعریف_کن lisprint(c)	(isprint(c))
#تعریف_کن lisxdigit(c)	(isxdigit(c))

#تعریف_کن ltolower(c)	(tolower(c))

#پایان_اگر			/* } */

#پایان_اگر

