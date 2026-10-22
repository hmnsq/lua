/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lctype_h
#నిర్వచించు lctype_h

#అన్తర్భావ "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
*/

#ఒకవేళ !defined(LUA_USE_CTYPE)

#ఒకవేళ 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#నిర్వచించు LUA_USE_CTYPE	0
#లేకపోతే
/* must use standard C ctype */
#నిర్వచించు LUA_USE_CTYPE	1
#అయితే_ముగింపు

#అయితే_ముగింపు


#ఒకవేళ !LUA_USE_CTYPE	/* { */

#అన్తర్భావ <limits.h>

#అన్తర్భావ "llimits.h"


#నిర్వచించు ALPHABIT	0
#నిర్వచించు DIGITBIT	1
#నిర్వచించు PRINTBIT	2
#నిర్వచించు SPACEBIT	3
#నిర్వచించు XDIGITBIT	4


#నిర్వచించు ముసుగు(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#నిర్వచించు testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#నిర్వచించు lislalpha(c)	testprop(c, ముసుగు(ALPHABIT))
#నిర్వచించు lislalnum(c)	testprop(c, (ముసుగు(ALPHABIT) | ముసుగు(DIGITBIT)))
#నిర్వచించు lisdigit(c)	testprop(c, ముసుగు(DIGITBIT))
#నిర్వచించు lisspace(c)	testprop(c, ముసుగు(SPACEBIT))
#నిర్వచించు lisprint(c)	testprop(c, ముసుగు(PRINTBIT))
#నిర్వచించు lisxdigit(c)	testprop(c, ముసుగు(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
*/
#నిర్వచించు ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) */
LUAI_DDEC(స్థిరరాశి lu_byte luai_ctype_[UCHAR_MAX + 2];)


#లేకపోతే			/* }{ */

/*
** use standard C ctypes
*/

#అన్తర్భావ <ctype.h>


#నిర్వచించు lislalpha(c)	(అక్షరమా(c) || (c) == '_')
#నిర్వచించు lislalnum(c)	(అక్షరమా_అంకెనా(c) || (c) == '_')
#నిర్వచించు lisdigit(c)	(అంకెనా(c))
#నిర్వచించు lisspace(c)	(ఖాళీనా(c))
#నిర్వచించు lisprint(c)	(isprint(c))
#నిర్వచించు lisxdigit(c)	(isxdigit(c))

#నిర్వచించు ltolower(c)	(tolower(c))

#అయితే_ముగింపు			/* } */

#అయితే_ముగింపు

