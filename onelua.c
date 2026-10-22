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
#ଯଦି 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#ଯଦି 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#ଅନ୍ତର୍ଭାଵ "lprefix.h"

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <float.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	ସ୍ଥିର
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	ସ୍ଥିର

/* core -- used by all */
#ଅନ୍ତର୍ଭାଵ "lzio.c"
#ଅନ୍ତର୍ଭାଵ "lctype.c"
#ଅନ୍ତର୍ଭାଵ "lopcodes.c"
#ଅନ୍ତର୍ଭାଵ "lmem.c"
#ଅନ୍ତର୍ଭାଵ "lundump.c"
#ଅନ୍ତର୍ଭାଵ "ldump.c"
#ଅନ୍ତର୍ଭାଵ "lstate.c"
#ଅନ୍ତର୍ଭାଵ "lgc.c"
#ଅନ୍ତର୍ଭାଵ "llex.c"
#ଅନ୍ତର୍ଭାଵ "lcode.c"
#ଅନ୍ତର୍ଭାଵ "lparser.c"
#ଅନ୍ତର୍ଭାଵ "ldebug.c"
#ଅନ୍ତର୍ଭାଵ "lfunc.c"
#ଅନ୍ତର୍ଭାଵ "lobject.c"
#ଅନ୍ତର୍ଭାଵ "ltm.c"
#ଅନ୍ତର୍ଭାଵ "lstring.c"
#ଅନ୍ତର୍ଭାଵ "ltable.c"
#ଅନ୍ତର୍ଭାଵ "ldo.c"
#ଅନ୍ତର୍ଭାଵ "lvm.c"
#ଅନ୍ତର୍ଭାଵ "lapi.c"

/* auxiliary library -- used by all */
#ଅନ୍ତର୍ଭାଵ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#ଅନ୍ତର୍ଭାଵ "lbaselib.c"
#ଅନ୍ତର୍ଭାଵ "lcorolib.c"
#ଅନ୍ତର୍ଭାଵ "ldblib.c"
#ଅନ୍ତର୍ଭାଵ "liolib.c"
#ଅନ୍ତର୍ଭାଵ "lmathlib.c"
#ଅନ୍ତର୍ଭାଵ "loadlib.c"
#ଅନ୍ତର୍ଭାଵ "loslib.c"
#ଅନ୍ତର୍ଭାଵ "lstrlib.c"
#ଅନ୍ତର୍ଭାଵ "ltablib.c"
#ଅନ୍ତର୍ଭାଵ "lutf8lib.c"
#ଅନ୍ତର୍ଭାଵ "linit.c"
#endif

/* test library -- used only for internal development */
#ଯଦି defined(LUA_DEBUG)
#ଅନ୍ତର୍ଭାଵ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#ଅନ୍ତର୍ଭାଵ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#ଅନ୍ତର୍ଭାଵ "luac.c"
#endif
