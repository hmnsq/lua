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
#جے_بنا_تعریف MAKE_LIB
#جے_بنا_تعریف MAKE_LUAC
#جے_بنا_تعریف MAKE_LUA
#تعریف_کرو MAKE_LUA
#جے_انت
#جے_انت
#جے_انت


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#إذا 0
#تعریف_کرو LUA_USE_LINUX
#تعریف_کرو LUA_USE_MACOSX
#تعریف_کرو LUA_USE_POSIX
#جے_انت


/*
** Other specific features
*/
#إذا 0
#تعریف_کرو LUA_32_بٹ
#تعریف_کرو LUA_USE_C89
#جے_انت


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
#تعریف_کرو LUA_CORE
#تعریف_کرو LUA_LIB

#ضمن "luaconf.h"

/* do not export internal symbols */
#تعریف_رد LUAI_FUNC
#تعریف_رد LUAI_DDEC
#تعریف_رد LUAI_DDEF
#تعریف_کرو LUAI_FUNC	ساكن
#تعریف_کرو LUAI_DDEC(def)	/*خالی_تھاواں*/
#تعریف_کرو LUAI_DDEF	ساكن

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
#جے_بنا_تعریف MAKE_LUAC
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
#جے_انت

/* test library -- used only for internal development */
#إذا defined(LUA_DEBUG)
#ضمن "ltests.c"
#جے_انت

/* lua */
#جے_تعریف_شدہ MAKE_LUA
#ضمن "lua.c"
#جے_انت

/* luac */
#جے_تعریف_شدہ MAKE_LUAC
#ضمن "luac.c"
#جے_انت
