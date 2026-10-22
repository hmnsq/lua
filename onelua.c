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
#यदि 0
#define LUA_USE_LINUX
#define LUA_USE_MACOSX
#define LUA_USE_POSIX
#endif


/*
** Other specific features
*/
#यदि 0
#define LUA_32BITS
#define LUA_USE_C89
#endif


/* no need to change anything below this line ----------------------------- */

#अन्तर्भाव "lprefix.h"

#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <float.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <locale.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <setjmp.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>

/* setup for luaconf.h */
#define LUA_CORE
#define LUA_LIB

#अन्तर्भाव "luaconf.h"

/* do not export internal symbols */
#undef LUAI_FUNC
#undef LUAI_DDEC
#undef LUAI_DDEF
#define LUAI_FUNC	स्थिर
#define LUAI_DDEC(def)	/* empty */
#define LUAI_DDEF	स्थिर

/* core -- used by all */
#अन्तर्भाव "lzio.c"
#अन्तर्भाव "lctype.c"
#अन्तर्भाव "lopcodes.c"
#अन्तर्भाव "lmem.c"
#अन्तर्भाव "lundump.c"
#अन्तर्भाव "ldump.c"
#अन्तर्भाव "lstate.c"
#अन्तर्भाव "lgc.c"
#अन्तर्भाव "llex.c"
#अन्तर्भाव "lcode.c"
#अन्तर्भाव "lparser.c"
#अन्तर्भाव "ldebug.c"
#अन्तर्भाव "lfunc.c"
#अन्तर्भाव "lobject.c"
#अन्तर्भाव "ltm.c"
#अन्तर्भाव "lstring.c"
#अन्तर्भाव "ltable.c"
#अन्तर्भाव "ldo.c"
#अन्तर्भाव "lvm.c"
#अन्तर्भाव "lapi.c"

/* auxiliary library -- used by all */
#अन्तर्भाव "lauxlib.c"

/* standard library  -- not used by luac */
#ifndef MAKE_LUAC
#अन्तर्भाव "lbaselib.c"
#अन्तर्भाव "lcorolib.c"
#अन्तर्भाव "ldblib.c"
#अन्तर्भाव "liolib.c"
#अन्तर्भाव "lmathlib.c"
#अन्तर्भाव "loadlib.c"
#अन्तर्भाव "loslib.c"
#अन्तर्भाव "lstrlib.c"
#अन्तर्भाव "ltablib.c"
#अन्तर्भाव "lutf8lib.c"
#अन्तर्भाव "linit.c"
#endif

/* test library -- used only for internal development */
#यदि defined(LUA_DEBUG)
#अन्तर्भाव "ltests.c"
#endif

/* lua */
#ifdef MAKE_LUA
#अन्तर्भाव "lua.c"
#endif

/* luac */
#ifdef MAKE_LUAC
#अन्तर्भाव "luac.c"
#endif
