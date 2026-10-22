/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lprefix_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#ਜੇ !defined(LUA_USE_C89)	/* { */

#ਜੇ !defined(XOPEN_ਸਰੋਤ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XOPEN_ਸਰੋਤ           600
#ਜਾਂਜੇ XOPEN_ਸਰੋਤ == 0
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ XOPEN_ਸਰੋਤ  /* use -D_XOPEN_SOURCE=0 to undefine it */
#ਜੇ_ਅੰਤ

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#ਜੇ !defined(LUA_32_ਬਿੱਟ) && !defined(FILE_ਵਿੱਥ_ਬਿੱਟ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LARGEFILE_ਸਰੋਤ       1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FILE_ਵਿੱਥ_ਬਿੱਟ       64
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ				/* } */


/*
** Windows stuff
*/
#ਜੇ defined(_WIN32)	/* { */

#ਜੇ !defined(_CRT_SECURE_NO_WARNINGS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ			/* } */

#ਜੇ_ਅੰਤ

