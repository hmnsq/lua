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
#எனில் 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#எனில் 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#அந்தர்பாவ "lprefix.h"

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <float.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <setjmp.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#அந்தர்பாவ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	நிலையான
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	நிலையான

/* core -- used by all */
#அந்தர்பாவ "lzio.c"
#அந்தர்பாவ "lctype.c"
#அந்தர்பாவ "lopcodes.c"
#அந்தர்பாவ "lmem.c"
#அந்தர்பாவ "lundump.c"
#அந்தர்பாவ "ldump.c"
#அந்தர்பாவ "lstate.c"
#அந்தர்பாவ "lgc.c"
#அந்தர்பாவ "llex.c"
#அந்தர்பாவ "lcode.c"
#அந்தர்பாவ "lparser.c"
#அந்தர்பாவ "ldebug.c"
#அந்தர்பாவ "lfunc.c"
#அந்தர்பாவ "lobject.c"
#அந்தர்பாவ "ltm.c"
#அந்தர்பாவ "lstring.c"
#அந்தர்பாவ "ltable.c"
#அந்தர்பாவ "ldo.c"
#அந்தர்பாவ "lvm.c"
#அந்தர்பாவ "lapi.c"

/* auxiliary library -- used by all */
#அந்தர்பாவ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#அந்தர்பாவ "lbaselib.c"
#அந்தர்பாவ "lcorolib.c"
#அந்தர்பாவ "ldblib.c"
#அந்தர்பாவ "liolib.c"
#அந்தர்பாவ "lmathlib.c"
#அந்தர்பாவ "loadlib.c"
#அந்தர்பாவ "loslib.c"
#அந்தர்பாவ "lstrlib.c"
#அந்தர்பாவ "ltablib.c"
#அந்தர்பாவ "lutf8lib.c"
#அந்தர்பாவ "linit.c"
#endif

/* test library -- used only for internal development */
#எனில் defined(LUA_DEBUG)
#அந்தர்பாவ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#அந்தர்பாவ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#அந்தர்பாவ "luac.c"
#endif
