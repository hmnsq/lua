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
#ifndef MAKE_LIB
#ifndef MAKE_LUAC
#ifndef MAKE_LUA
#define MAKE_LUA
#endif
#endif
#endif


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#જો 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#જો 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#અન્તર્ભાવ "lprefix.h"

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <float.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <setjmp.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#અન્તર્ભાવ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	સ્થિર
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	સ્થિર

/* core -- used by all */
#અન્તર્ભાવ "lzio.c"
#અન્તર્ભાવ "lctype.c"
#અન્તર્ભાવ "lopcodes.c"
#અન્તર્ભાવ "lmem.c"
#અન્તર્ભાવ "lundump.c"
#અન્તર્ભાવ "ldump.c"
#અન્તર્ભાવ "lstate.c"
#અન્તર્ભાવ "lgc.c"
#અન્તર્ભાવ "llex.c"
#અન્તર્ભાવ "lcode.c"
#અન્તર્ભાવ "lparser.c"
#અન્તર્ભાવ "ldebug.c"
#અન્તર્ભાવ "lfunc.c"
#અન્તર્ભાવ "lobject.c"
#અન્તર્ભાવ "ltm.c"
#અન્તર્ભાવ "lstring.c"
#અન્તર્ભાવ "ltable.c"
#અન્તર્ભાવ "ldo.c"
#અન્તર્ભાવ "lvm.c"
#અન્તર્ભાવ "lapi.c"

/* auxiliary library -- used by all */
#અન્તર્ભાવ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#અન્તર્ભાવ "lbaselib.c"
#અન્તર્ભાવ "lcorolib.c"
#અન્તર્ભાવ "ldblib.c"
#અન્તર્ભાવ "liolib.c"
#અન્તર્ભાવ "lmathlib.c"
#અન્તર્ભાવ "loadlib.c"
#અન્તર્ભાવ "loslib.c"
#અન્તર્ભાવ "lstrlib.c"
#અન્તર્ભાવ "ltablib.c"
#અન્તર્ભાવ "lutf8lib.c"
#અન્તર્ભાવ "linit.c"
#endif

/* test library -- used only for internal development */
#જો defined(LUA_DEBUG)
#અન્તર્ભાવ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#અન્તર્ભાવ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#અન્તર્ભાવ "luac.c"
#endif
