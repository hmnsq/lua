/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lprefix_h
#تعریف_کن lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#إذا !defined(LUA_USE_C89)	/* { */

#إذا !defined(XOPEN_منبع)
#تعریف_کن XOPEN_منبع           600
#وإلاإذا XOPEN_منبع == 0
#لغو_تعریف XOPEN_منبع  /* use -D_XOPEN_SOURCE=0 to undefine it */
#پایان_اگر

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#إذا !defined(LUA_32_بیتها) && !defined(FILE_جابه_جایی_بیتها)
#تعریف_کن LARGEFILE_منبع       1
#تعریف_کن FILE_جابه_جایی_بیتها       64
#پایان_اگر

#پایان_اگر				/* } */


/*
** Windows stuff
*/
#إذا defined(_WIN32)	/* { */

#إذا !defined(_CRT_SECURE_NO_WARNINGS)
#تعریف_کن _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#پایان_اگر

#پایان_اگر			/* } */

#پایان_اگر

