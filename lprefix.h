/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lprefix_h
#ವ್ಯಾಖ್ಯಾನಿಸು lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#ಆದರೆ !defined(LUA_USE_C89)	/* { */

#ಆದರೆ !defined(XOPEN_ಮೂಲ)
#ವ್ಯಾಖ್ಯಾನಿಸು XOPEN_ಮೂಲ           600
#ಅಥವಾಆದರೆ XOPEN_ಮೂಲ == 0
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು XOPEN_ಮೂಲ  /* use -D_XOPEN_SOURCE=0 to undefine it */
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#ಆದರೆ !defined(LUA_32_ಬಿಟ್ಟುಗಳು) && !defined(FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು LARGEFILE_ಮೂಲ       1
#ವ್ಯಾಖ್ಯಾನಿಸು FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು       64
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** Windows stuff
*/
#ಆದರೆ defined(_WIN32)	/* { */

#ಆದರೆ !defined(_CRT_SECURE_NO_WARNINGS)
#ವ್ಯಾಖ್ಯಾನಿಸು _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ			/* } */

#ಆದರೆ_ಅಂತ್ಯ

