/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lctype_h
#घोषणा lctype_h

#समावेश "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#अगर !defined(LUA_USE_CTYPE)

#अगर 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#घोषणा LUA_USE_CTYPE	0
#अन्यथा
/* must use standard C ctype */
#घोषणा LUA_USE_CTYPE	1
#पूर्ण_अगर

#पूर्ण_अगर


#अगर !LUA_USE_CTYPE	/* { */

#समावेश <limits.h>

#समावेश "llimits.h"


#घोषणा ALPHABIT	0
#घोषणा DIGITBIT	1
#घोषणा PRINTBIT	2
#घोषणा SPACEBIT	3
#घोषणा XDIGITBIT	4


#घोषणा MASK(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#घोषणा testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#घोषणा lislalpha(c)	testprop(c, MASK(ALPHABIT))
#घोषणा lislalnum(c)	testprop(c, (MASK(ALPHABIT) | MASK(DIGITBIT)))
#घोषणा lisdigit(c)	testprop(c, MASK(DIGITBIT))
#घोषणा lisspace(c)	testprop(c, MASK(SPACEBIT))
#घोषणा lisprint(c)	testprop(c, MASK(PRINTBIT))
#घोषणा lisxdigit(c)	testprop(c, MASK(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#घोषणा ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(स्थिर lu_byte luai_ctype_[अच_अक्षर_उच्च + 2];)


#अन्यथा			/* }{ */

/*
** use standard C ctypes
*/

#समावेश <ctype.h>


#घोषणा lislalpha(c)	(है_अक्षर(c) || (c) == '_')
#घोषणा lislalnum(c)	(है_अक्षर_अंक(c) || (c) == '_')
#घोषणा lisdigit(c)	(है_अंक(c))
#घोषणा lisspace(c)	(है_खाली(c))
#घोषणा lisprint(c)	(है_छाप(c))
#घोषणा lisxdigit(c)	(है_षष्ठादशक(c))

#घोषणा ltolower(c)	(छोटे(c))

#पूर्ण_अगर			/* } */

#पूर्ण_अगर

