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
#اگر 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#اگر 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#شامل "lprefix.h"

#شامل <assert.h>
#شامل <ctype.h>
#شامل <errno.h>
#شامل <float.h>
#شامل <limits.h>
#شامل <locale.h>
#شامل <math.h>
#شامل <setjmp.h>
#شامل <signal.h>
#شامل <stdarg.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#شامل "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	ایستا
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	ایستا

/* core -- used by all */
#شامل "lzio.c"
#شامل "lctype.c"
#شامل "lopcodes.c"
#شامل "lmem.c"
#شامل "lundump.c"
#شامل "ldump.c"
#شامل "lstate.c"
#شامل "lgc.c"
#شامل "llex.c"
#شامل "lcode.c"
#شامل "lparser.c"
#شامل "ldebug.c"
#شامل "lfunc.c"
#شامل "lobject.c"
#شامل "ltm.c"
#شامل "lstring.c"
#شامل "ltable.c"
#شامل "ldo.c"
#شامل "lvm.c"
#شامل "lapi.c"

/* auxiliary library -- used by all */
#شامل "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#شامل "lbaselib.c"
#شامل "lcorolib.c"
#شامل "ldblib.c"
#شامل "liolib.c"
#شامل "lmathlib.c"
#شامل "loadlib.c"
#شامل "loslib.c"
#شامل "lstrlib.c"
#شامل "ltablib.c"
#شامل "lutf8lib.c"
#شامل "linit.c"
#endif

/* test library -- used only for internal development */
#اگر defined(LUA_DEBUG)
#شامل "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#شامل "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#شامل "luac.c"
#endif
