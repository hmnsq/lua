/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#ifndef lctype_h
#define lctype_h

#अन्तर्भाव "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#यदि !defined(LUA_USE_CTYPE)

#यदि 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#define LUA_USE_CTYPE	0
#अन्यथा
/* must use standard C ctype */
#define LUA_USE_CTYPE	1
#endif

#endif


#यदि !LUA_USE_CTYPE	/* { */

#अन्तर्भाव <limits.h>

#अन्तर्भाव "llimits.h"


#define ALPHABIT	0
#define DIGITBIT	1
#define PRINTBIT	2
#define SPACEBIT	3
#define XDIGITBIT	4


#define MASK(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#define testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#define lislalpha(c)	testprop(c, MASK(ALPHABIT))
#define lislalnum(c)	testprop(c, (MASK(ALPHABIT) | MASK(DIGITBIT)))
#define lisdigit(c)	testprop(c, MASK(DIGITBIT))
#define lisspace(c)	testprop(c, MASK(SPACEBIT))
#define lisprint(c)	testprop(c, MASK(PRINTBIT))
#define lisxdigit(c)	testprop(c, MASK(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#define ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(अचर lu_byte luai_ctype_[अच_अक्षर_उच्च + 2];)


#अन्यथा			/* }{ */

/*
** use standard C ctypes
*/

#अन्तर्भाव <ctype.h>


#define lislalpha(c)	(है_अक्षर(c) || (c) == '_')
#define lislalnum(c)	(है_अक्षर_अंक(c) || (c) == '_')
#define lisdigit(c)	(है_अंक(c))
#define lisspace(c)	(है_खाली(c))
#define lisprint(c)	(है_छाप(c))
#define lisxdigit(c)	(है_षष्ठादशक(c))

#define ltolower(c)	(छोटे(c))

#endif			/* } */

#endif

