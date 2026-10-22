/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lprefix_h
#परिभाषय lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#यदि !defined(LUA_USE_C89)	/* { */

#यदि !defined(_XOPEN_SOURCE)
#परिभाषय _XOPEN_SOURCE           600
#नोचेद्यदि _XOPEN_SOURCE == 0
#अपरिभाषय _XOPEN_SOURCE  /* use -D_XOPEN_SOURCE=0 to undefine it */
#यदि_अन्तः

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#यदि !defined(LUA_32_द्वयङ्काः) && !defined(FILE_विस्थापनम्_द्वयङ्काः)
#परिभाषय _LARGEFILE_SOURCE       1
#परिभाषय FILE_विस्थापनम्_द्वयङ्काः       64
#यदि_अन्तः

#यदि_अन्तः				/* } */


/*
** Windows stuff
*/
#यदि defined(_WIN32)	/* { */

#यदि !defined(_CRT_SECURE_NO_WARNINGS)
#परिभाषय _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#यदि_अन्तः

#यदि_अन्तः			/* } */

#यदि_अन्तः

