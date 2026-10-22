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
#যদি 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#যদি 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#অন্তর্ভাব "lprefix.h"

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <float.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <setjmp.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#অন্তর্ভাব "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	স্থির
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	স্থির

/* core -- used by all */
#অন্তর্ভাব "lzio.c"
#অন্তর্ভাব "lctype.c"
#অন্তর্ভাব "lopcodes.c"
#অন্তর্ভাব "lmem.c"
#অন্তর্ভাব "lundump.c"
#অন্তর্ভাব "ldump.c"
#অন্তর্ভাব "lstate.c"
#অন্তর্ভাব "lgc.c"
#অন্তর্ভাব "llex.c"
#অন্তর্ভাব "lcode.c"
#অন্তর্ভাব "lparser.c"
#অন্তর্ভাব "ldebug.c"
#অন্তর্ভাব "lfunc.c"
#অন্তর্ভাব "lobject.c"
#অন্তর্ভাব "ltm.c"
#অন্তর্ভাব "lstring.c"
#অন্তর্ভাব "ltable.c"
#অন্তর্ভাব "ldo.c"
#অন্তর্ভাব "lvm.c"
#অন্তর্ভাব "lapi.c"

/* auxiliary library -- used by all */
#অন্তর্ভাব "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#অন্তর্ভাব "lbaselib.c"
#অন্তর্ভাব "lcorolib.c"
#অন্তর্ভাব "ldblib.c"
#অন্তর্ভাব "liolib.c"
#অন্তর্ভাব "lmathlib.c"
#অন্তর্ভাব "loadlib.c"
#অন্তর্ভাব "loslib.c"
#অন্তর্ভাব "lstrlib.c"
#অন্তর্ভাব "ltablib.c"
#অন্তর্ভাব "lutf8lib.c"
#অন্তর্ভাব "linit.c"
#endif

/* test library -- used only for internal development */
#যদি defined(LUA_DEBUG)
#অন্তর্ভাব "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#অন্তর্ভাব "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#অন্তর্ভাব "luac.c"
#endif
