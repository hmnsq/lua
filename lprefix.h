/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف lprefix_h
#تعریف_کرو lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#إذا !defined(LUA_USE_C89)	/* { */

#إذا !defined(XOPEN_سروت)
#تعریف_کرو XOPEN_سروت           600
#وإلاإذا XOPEN_سروت == 0
#تعریف_رد XOPEN_سروت  /* use -D_XOPEN_SOURCE=0 to undefine it */
#جے_انت

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#إذا !defined(LUA_32_بٹ) && !defined(FILE_وتھ_بٹ)
#تعریف_کرو LARGEFILE_سروت       1
#تعریف_کرو FILE_وتھ_بٹ       64
#جے_انت

#جے_انت				/* } */


/*
** Windows stuff
*/
#إذا defined(_WIN32)	/* { */

#إذا !defined(_CRT_SECURE_NO_WARNINGS)
#تعریف_کرو _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#جے_انت

#جے_انت			/* } */

#جے_انت

