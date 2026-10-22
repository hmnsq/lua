/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lctype_h
#परिभाषय lctype_h

#अन्तर्भाव "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#यदि !defined(LUA_USE_CTYPE)

#यदि 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#परिभाषय LUA_USE_CTYPE	0
#अन्यथा
/* must use standard C ctype */
#परिभाषय LUA_USE_CTYPE	1
#यदि_अन्तः

#यदि_अन्तः


#यदि !LUA_USE_CTYPE	/* { */

#अन्तर्भाव <limits.h>

#अन्तर्भाव "llimits.h"


#परिभाषय ALPHABIT	0
#परिभाषय DIGITBIT	1
#परिभाषय PRINTBIT	2
#परिभाषय SPACEBIT	3
#परिभाषय XDIGITBIT	4


#परिभाषय आवरणम्(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#परिभाषय testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#परिभाषय lislalpha(c)	testprop(c, आवरणम्(ALPHABIT))
#परिभाषय lislalnum(c)	testprop(c, (आवरणम्(ALPHABIT) | आवरणम्(DIGITBIT)))
#परिभाषय lisdigit(c)	testprop(c, आवरणम्(DIGITBIT))
#परिभाषय lisspace(c)	testprop(c, आवरणम्(SPACEBIT))
#परिभाषय lisprint(c)	testprop(c, आवरणम्(PRINTBIT))
#परिभाषय lisxdigit(c)	testprop(c, आवरणम्(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#परिभाषय ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(ध्रुव lu_byte luai_ctype_[UCHAR_MAX + 2];)


#अन्यथा			/* }{ */

/*
** use standard C ctypes
*/

#अन्तर्भाव <ctype.h>


#परिभाषय lislalpha(c)	(अक्षरम्_वा(c) || (c) == '_')
#परिभाषय lislalnum(c)	(अक्षरम्_अङ्कः_वा(c) || (c) == '_')
#परिभाषय lisdigit(c)	(अङ्कः_वा(c))
#परिभाषय lisspace(c)	(रिक्तम्_वा(c))
#परिभाषय lisprint(c)	(isprint(c))
#परिभाषय lisxdigit(c)	(isxdigit(c))

#परिभाषय ltolower(c)	(tolower(c))

#यदि_अन्तः			/* } */

#यदि_अन्तः

