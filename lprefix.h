/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lprefix_h
#घोषणा lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#अगर !defined(LUA_USE_C89)	/* { */

#अगर !defined(_XOPEN_SOURCE)
#घोषणा _XOPEN_SOURCE           600
#या_अगर _XOPEN_SOURCE == 0
#अघोषित _XOPEN_SOURCE  /* use -D_XOPEN_SOURCE=0 to undefine it */
#पूर्ण_अगर

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#अगर !defined(LUA_32BITS) && !defined(_FILE_OFFSET_BITS)
#घोषणा _LARGEFILE_SOURCE       1
#घोषणा _FILE_OFFSET_BITS       64
#पूर्ण_अगर

#पूर्ण_अगर				/* } */


/*
** Windows stuff
*/
#अगर defined(_WIN32)	/* { */

#अगर !defined(_CRT_SECURE_NO_WARNINGS)
#घोषणा _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#पूर्ण_अगर

#पूर्ण_अगर			/* } */

#पूर्ण_अगर

