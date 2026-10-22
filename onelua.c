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
#𐡄𐡍 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#𐡄𐡍 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#ضمن "lprefix.h"

#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <float.h>
#ضمن <limits.h>
#ضمن <locale.h>
#ضمن <math.h>
#ضمن <setjmp.h>
#ضمن <signal.h>
#ضمن <stdarg.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#ضمن "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	ساكن
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	ساكن

/* core -- used by all */
#ضمن "lzio.c"
#ضمن "lctype.c"
#ضمن "lopcodes.c"
#ضمن "lmem.c"
#ضمن "lundump.c"
#ضمن "ldump.c"
#ضمن "lstate.c"
#ضمن "lgc.c"
#ضمن "llex.c"
#ضمن "lcode.c"
#ضمن "lparser.c"
#ضمن "ldebug.c"
#ضمن "lfunc.c"
#ضمن "lobject.c"
#ضمن "ltm.c"
#ضمن "lstring.c"
#ضمن "ltable.c"
#ضمن "ldo.c"
#ضمن "lvm.c"
#ضمن "lapi.c"

/* auxiliary library -- used by all */
#ضمن "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#ضمن "lbaselib.c"
#ضمن "lcorolib.c"
#ضمن "ldblib.c"
#ضمن "liolib.c"
#ضمن "lmathlib.c"
#ضمن "loadlib.c"
#ضمن "loslib.c"
#ضمن "lstrlib.c"
#ضمن "ltablib.c"
#ضمن "lutf8lib.c"
#ضمن "linit.c"
#endif

/* test library -- used only for internal development */
#𐡄𐡍 defined(LUA_DEBUG)
#ضمن "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#ضمن "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#ضمن "luac.c"
#endif
