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
#ಯದಿ 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#ಯದಿ 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#ಅನ್ತರ್ಭಾವ "lprefix.h"

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <float.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <setjmp.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	ಸ್ಥಿರ
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	ಸ್ಥಿರ

/* core -- used by all */
#ಅನ್ತರ್ಭಾವ "lzio.c"
#ಅನ್ತರ್ಭಾವ "lctype.c"
#ಅನ್ತರ್ಭಾವ "lopcodes.c"
#ಅನ್ತರ್ಭಾವ "lmem.c"
#ಅನ್ತರ್ಭಾವ "lundump.c"
#ಅನ್ತರ್ಭಾವ "ldump.c"
#ಅನ್ತರ್ಭಾವ "lstate.c"
#ಅನ್ತರ್ಭಾವ "lgc.c"
#ಅನ್ತರ್ಭಾವ "llex.c"
#ಅನ್ತರ್ಭಾವ "lcode.c"
#ಅನ್ತರ್ಭಾವ "lparser.c"
#ಅನ್ತರ್ಭಾವ "ldebug.c"
#ಅನ್ತರ್ಭಾವ "lfunc.c"
#ಅನ್ತರ್ಭಾವ "lobject.c"
#ಅನ್ತರ್ಭಾವ "ltm.c"
#ಅನ್ತರ್ಭಾವ "lstring.c"
#ಅನ್ತರ್ಭಾವ "ltable.c"
#ಅನ್ತರ್ಭಾವ "ldo.c"
#ಅನ್ತರ್ಭಾವ "lvm.c"
#ಅನ್ತರ್ಭಾವ "lapi.c"

/* auxiliary library -- used by all */
#ಅನ್ತರ್ಭಾವ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#ಅನ್ತರ್ಭಾವ "lbaselib.c"
#ಅನ್ತರ್ಭಾವ "lcorolib.c"
#ಅನ್ತರ್ಭಾವ "ldblib.c"
#ಅನ್ತರ್ಭಾವ "liolib.c"
#ಅನ್ತರ್ಭಾವ "lmathlib.c"
#ಅನ್ತರ್ಭಾವ "loadlib.c"
#ಅನ್ತರ್ಭಾವ "loslib.c"
#ಅನ್ತರ್ಭಾವ "lstrlib.c"
#ಅನ್ತರ್ಭಾವ "ltablib.c"
#ಅನ್ತರ್ಭಾವ "lutf8lib.c"
#ಅನ್ತರ್ಭಾವ "linit.c"
#endif

/* test library -- used only for internal development */
#ಯದಿ defined(LUA_DEBUG)
#ಅನ್ತರ್ಭಾವ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#ಅನ್ತರ್ಭಾವ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#ಅನ್ತರ್ಭಾವ "luac.c"
#endif
