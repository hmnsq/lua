/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lprefix_h
#تعریف_کرو lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#اگر !defined(LUA_USE_C89)	/* { */

#اگر !defined(XOPEN_ماخذ)
#تعریف_کرو XOPEN_ماخذ           600
#وگرنہ_اگر XOPEN_ماخذ == 0
#تعریف_منسوخ XOPEN_ماخذ  /* use -D_XOPEN_SOURCE=0 to undefine it */
#اگر_ختم

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#اگر !defined(LUA_32_بٹ) && !defined(FILE_انحراف_بٹ)
#تعریف_کرو LARGEFILE_ماخذ       1
#تعریف_کرو FILE_انحراف_بٹ       64
#اگر_ختم

#اگر_ختم				/* } */


/*
** Windows stuff
*/
#اگر defined(_WIN32)	/* { */

#اگر !defined(_CRT_SECURE_NO_WARNINGS)
#تعریف_کرو _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#اگر_ختم

#اگر_ختم			/* } */

#اگر_ختم

