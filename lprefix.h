/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lprefix_h
#تعریف_کړه lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#إذا !defined(LUA_USE_C89)	/* { */

#إذا !defined(XOPEN_سرچینه)
#تعریف_کړه XOPEN_سرچینه           600
#وإلاإذا XOPEN_سرچینه == 0
#تعریف_لغوه XOPEN_سرچینه  /* use -D_XOPEN_SOURCE=0 to undefine it */
#د_که_پای

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#إذا !defined(LUA_32_بټونه) && !defined(FILE_دننه_واټن_بټونه)
#تعریف_کړه LARGEFILE_سرچینه       1
#تعریف_کړه FILE_دننه_واټن_بټونه       64
#د_که_پای

#د_که_پای				/* } */


/*
** Windows stuff
*/
#إذا defined(_WIN32)	/* { */

#إذا !defined(_CRT_SECURE_NO_WARNINGS)
#تعریف_کړه _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#د_که_پای

#د_که_پای			/* } */

#د_که_پای

