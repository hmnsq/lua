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
#اگر_غیر_تعریف_شدہ MAKE_LIB
#اگر_غیر_تعریف_شدہ MAKE_LUAC
#اگر_غیر_تعریف_شدہ MAKE_LUA
#تعریف_کرو MAKE_LUA
#اگر_ختم
#اگر_ختم
#اگر_ختم


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#اگر 0
#تعریف_کرو LUA_USE_LINUX
#تعریف_کرو LUA_USE_MACOSX
#تعریف_کرو LUA_USE_POSIX
#اگر_ختم


/*
** Other specific features
*/
#اگر 0
#تعریف_کرو LUA_32_بٹ
#تعریف_کرو LUA_USE_C89
#اگر_ختم


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
#تعریف_کرو LUA_CORE
#تعریف_کرو LUA_LIB

#شامل "luaconf.h"

/* do not export internal symbols */
#تعریف_منسوخ LUAI_FUNC
#تعریف_منسوخ LUAI_DDEC
#تعریف_منسوخ LUAI_DDEF
#تعریف_کرو LUAI_FUNC	ساکن
#تعریف_کرو LUAI_DDEC(def)	/*خالی_خانے*/
#تعریف_کرو LUAI_DDEF	ساکن

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
#اگر_غیر_تعریف_شدہ MAKE_LUAC
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
#اگر_ختم

/* test library -- used only for internal development */
#اگر defined(LUA_DEBUG)
#شامل "ltests.c"
#اگر_ختم

/* lua */
#اگر_تعریف_شدہ MAKE_LUA
#شامل "lua.c"
#اگر_ختم

/* luac */
#اگر_تعریف_شدہ MAKE_LUAC
#شامل "luac.c"
#اگر_ختم
