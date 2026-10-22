/*
** $Id: lprefix.h $
** Definitions for Lua code that must come before any other header file
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lprefix_h
#నిర్వచించు lprefix_h


/*
** Allows POSIX/XSI stuff
*/
#ఒకవేళ !defined(LUA_USE_C89)	/* { */

#ఒకవేళ !defined(XOPEN_ఆధారం)
#నిర్వచించు XOPEN_ఆధారం           600
#లేదావేళ XOPEN_ఆధారం == 0
#నిర్వచనం_రద్దు XOPEN_ఆధారం  /* use -D_XOPEN_SOURCE=0 to undefine it */
#అయితే_ముగింపు

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#ఒకవేళ !defined(LUA_32_బిట్లు) && !defined(FILE_స్థానభ్రంశం_బిట్లు)
#నిర్వచించు LARGEFILE_ఆధారం       1
#నిర్వచించు FILE_స్థానభ్రంశం_బిట్లు       64
#అయితే_ముగింపు

#అయితే_ముగింపు				/* } */


/*
** Windows stuff
*/
#ఒకవేళ defined(_WIN32)	/* { */

#ఒకవేళ !defined(_CRT_SECURE_NO_WARNINGS)
#నిర్వచించు _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#అయితే_ముగింపు

#అయితే_ముగింపు			/* } */

#అయితే_ముగింపు

