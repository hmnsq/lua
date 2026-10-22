/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lprefix_h
#وصف_ڪريو lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#إذا !defined(LUA_USE_C89)	/* { */

#إذا !defined(XOPEN_ذريعو)
#وصف_ڪريو XOPEN_ذريعو           600
#وإلاإذا XOPEN_ذريعو == 0
#وصف_رد XOPEN_ذريعو  /* use -D_XOPEN_SOURCE=0 to undefine it */
#جيڪڏهن_پورو

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#إذا !defined(LUA_32_بٽ) && !defined(FILE_وٿي_بٽ)
#وصف_ڪريو LARGEFILE_ذريعو       1
#وصف_ڪريو FILE_وٿي_بٽ       64
#جيڪڏهن_پورو

#جيڪڏهن_پورو				/* } */


/*
** Windows stuff
*/
#إذا defined(_WIN32)	/* { */

#إذا !defined(_CRT_SECURE_NO_WARNINGS)
#وصف_ڪريو _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#جيڪڏهن_پورو

#جيڪڏهن_پورو			/* } */

#جيڪڏهن_پورو

