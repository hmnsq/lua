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

#অন্তর্ভাৱ "lprefix.h"

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <float.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <locale.h>
#অন্তর্ভাৱ <math.h>
#অন্তর্ভাৱ <setjmp.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#অন্তর্ভাৱ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	স্থির
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	স্থির

/* core -- used by all */
#অন্তর্ভাৱ "lzio.c"
#অন্তর্ভাৱ "lctype.c"
#অন্তর্ভাৱ "lopcodes.c"
#অন্তর্ভাৱ "lmem.c"
#অন্তর্ভাৱ "lundump.c"
#অন্তর্ভাৱ "ldump.c"
#অন্তর্ভাৱ "lstate.c"
#অন্তর্ভাৱ "lgc.c"
#অন্তর্ভাৱ "llex.c"
#অন্তর্ভাৱ "lcode.c"
#অন্তর্ভাৱ "lparser.c"
#অন্তর্ভাৱ "ldebug.c"
#অন্তর্ভাৱ "lfunc.c"
#অন্তর্ভাৱ "lobject.c"
#অন্তর্ভাৱ "ltm.c"
#অন্তর্ভাৱ "lstring.c"
#অন্তর্ভাৱ "ltable.c"
#অন্তর্ভাৱ "ldo.c"
#অন্তর্ভাৱ "lvm.c"
#অন্তর্ভাৱ "lapi.c"

/* auxiliary library -- used by all */
#অন্তর্ভাৱ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#অন্তর্ভাৱ "lbaselib.c"
#অন্তর্ভাৱ "lcorolib.c"
#অন্তর্ভাৱ "ldblib.c"
#অন্তর্ভাৱ "liolib.c"
#অন্তর্ভাৱ "lmathlib.c"
#অন্তর্ভাৱ "loadlib.c"
#অন্তর্ভাৱ "loslib.c"
#অন্তর্ভাৱ "lstrlib.c"
#অন্তর্ভাৱ "ltablib.c"
#অন্তর্ভাৱ "lutf8lib.c"
#অন্তর্ভাৱ "linit.c"
#endif

/* test library -- used only for internal development */
#যদি defined(LUA_DEBUG)
#অন্তর্ভাৱ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#অন্তর্ভাৱ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#অন্তর্ভাৱ "luac.c"
#endif
