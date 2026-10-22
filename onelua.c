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
#എങ്കിൽ 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#എങ്കിൽ 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#അന്തര്ഭാവ "lprefix.h"

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <float.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <setjmp.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#അന്തര്ഭാവ "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	സ്ഥിതം
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	സ്ഥിതം

/* core -- used by all */
#അന്തര്ഭാവ "lzio.c"
#അന്തര്ഭാവ "lctype.c"
#അന്തര്ഭാവ "lopcodes.c"
#അന്തര്ഭാവ "lmem.c"
#അന്തര്ഭാവ "lundump.c"
#അന്തര്ഭാവ "ldump.c"
#അന്തര്ഭാവ "lstate.c"
#അന്തര്ഭാവ "lgc.c"
#അന്തര്ഭാവ "llex.c"
#അന്തര്ഭാവ "lcode.c"
#അന്തര്ഭാവ "lparser.c"
#അന്തര്ഭാവ "ldebug.c"
#അന്തര്ഭാവ "lfunc.c"
#അന്തര്ഭാവ "lobject.c"
#അന്തര്ഭാവ "ltm.c"
#അന്തര്ഭാവ "lstring.c"
#അന്തര്ഭാവ "ltable.c"
#അന്തര്ഭാവ "ldo.c"
#അന്തര്ഭാവ "lvm.c"
#അന്തര്ഭാവ "lapi.c"

/* auxiliary library -- used by all */
#അന്തര്ഭാവ "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#അന്തര്ഭാവ "lbaselib.c"
#അന്തര്ഭാവ "lcorolib.c"
#അന്തര്ഭാവ "ldblib.c"
#അന്തര്ഭാവ "liolib.c"
#അന്തര്ഭാവ "lmathlib.c"
#അന്തര്ഭാവ "loadlib.c"
#അന്തര്ഭാവ "loslib.c"
#അന്തര്ഭാവ "lstrlib.c"
#അന്തര്ഭാവ "ltablib.c"
#അന്തര്ഭാവ "lutf8lib.c"
#അന്തര്ഭാവ "linit.c"
#endif

/* test library -- used only for internal development */
#എങ്കിൽ defined(LUA_DEBUG)
#അന്തര്ഭാവ "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#അന്തര്ഭാവ "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#അന്തര്ഭാവ "luac.c"
#endif
