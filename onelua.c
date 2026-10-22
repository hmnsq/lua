/*
** Lua core, libraries, and interpreter in a single file.
** Compiling just this file generates a complete Lua stand-alone
** program:
**
** $ gcc -O2 -std=c99 -o lua onelua.c -lm
**
** or (for C89)
**
** $ gcc -O2 -std=c89 -DLUA_USE_C89 -o lua onelua.c -lm
**
** or (for Linux)
**
** gcc -O2 -o lua -DLUA_USE_LINUX -Wl,-E onelua.c -lm -ldl
**
*/

/* default is to build the full interpreter */
#నిర్వచితం_కాకపోతే MAKE_LIB
#నిర్వచితం_కాకపోతే MAKE_LUAC
#నిర్వచితం_కాకపోతే MAKE_LUA
#నిర్వచించు MAKE_LUA
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#ఒకవేళ 0
#నిర్వచించు LUA_USE_LINUX
#నిర్వచించు LUA_USE_MACOSX
#నిర్వచించు LUA_USE_POSIX
#అయితే_ముగింపు


/*
** Other specific features
*/
#ఒకవేళ 0
#నిర్వచించు LUA_32_బిట్లు
#నిర్వచించు LUA_USE_C89
#అయితే_ముగింపు


/* no need to change anything below this line ----------------------------- */

#అన్తర్భావ "lprefix.h"

#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <float.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <locale.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <setjmp.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>

/* setup for luaconf.h */
#నిర్వచించు LUA_CORE
#నిర్వచించు LUA_LIB

#అన్తర్భావ "luaconf.h"

/* do not export internal symbols */
#నిర్వచనం_రద్దు LUAI_FUNC
#నిర్వచనం_రద్దు LUAI_DDEC
#నిర్వచనం_రద్దు LUAI_DDEF
#నిర్వచించు LUAI_FUNC	స్థిర
#నిర్వచించు LUAI_DDEC(def)	/*ఖాళీలు*/
#నిర్వచించు LUAI_DDEF	స్థిర

/* core -- used by all */
#అన్తర్భావ "lzio.c"
#అన్తర్భావ "lctype.c"
#అన్తర్భావ "lopcodes.c"
#అన్తర్భావ "lmem.c"
#అన్తర్భావ "lundump.c"
#అన్తర్భావ "ldump.c"
#అన్తర్భావ "lstate.c"
#అన్తర్భావ "lgc.c"
#అన్తర్భావ "llex.c"
#అన్తర్భావ "lcode.c"
#అన్తర్భావ "lparser.c"
#అన్తర్భావ "ldebug.c"
#అన్తర్భావ "lfunc.c"
#అన్తర్భావ "lobject.c"
#అన్తర్భావ "ltm.c"
#అన్తర్భావ "lstring.c"
#అన్తర్భావ "ltable.c"
#అన్తర్భావ "ldo.c"
#అన్తర్భావ "lvm.c"
#అన్తర్భావ "lapi.c"

/* auxiliary library -- used by all */
#అన్తర్భావ "lauxlib.c"

/* standard library  -- not used by luac */
#నిర్వచితం_కాకపోతే MAKE_LUAC
#అన్తర్భావ "lbaselib.c"
#అన్తర్భావ "lcorolib.c"
#అన్తర్భావ "ldblib.c"
#అన్తర్భావ "liolib.c"
#అన్తర్భావ "lmathlib.c"
#అన్తర్భావ "loadlib.c"
#అన్తర్భావ "loslib.c"
#అన్తర్భావ "lstrlib.c"
#అన్తర్భావ "ltablib.c"
#అన్తర్భావ "lutf8lib.c"
#అన్తర్భావ "linit.c"
#అయితే_ముగింపు

/* test library -- used only for internal development */
#ఒకవేళ defined(LUA_DEBUG)
#అన్తర్భావ "ltests.c"
#అయితే_ముగింపు

/* lua */
#నిర్వచితమైతే MAKE_LUA
#అన్తర్భావ "lua.c"
#అయితే_ముగింపు

/* luac */
#నిర్వచితమైతే MAKE_LUAC
#అన్తర్భావ "luac.c"
#అయితే_ముగింపు
