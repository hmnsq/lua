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
#که_نه_تعریف_شوی MAKE_LIB
#که_نه_تعریف_شوی MAKE_LUAC
#که_نه_تعریف_شوی MAKE_LUA
#تعریف_کړه MAKE_LUA
#د_که_پای
#د_که_پای
#د_که_پای


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#إذا 0
#تعریف_کړه LUA_USE_LINUX
#تعریف_کړه LUA_USE_MACOSX
#تعریف_کړه LUA_USE_POSIX
#د_که_پای


/*
** Other specific features
*/
#إذا 0
#تعریف_کړه LUA_32_بټونه
#تعریف_کړه LUA_USE_C89
#د_که_پای


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
#تعریف_کړه LUA_CORE
#تعریف_کړه LUA_LIB

#ضمن "luaconf.h"

/* do not export internal symbols */
#تعریف_لغوه LUAI_FUNC
#تعریف_لغوه LUAI_DDEC
#تعریف_لغوه LUAI_DDEF
#تعریف_کړه LUAI_FUNC	ساكن
#تعریف_کړه LUAI_DDEC(def)	/*تشه*/
#تعریف_کړه LUAI_DDEF	ساكن

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
#که_نه_تعریف_شوی MAKE_LUAC
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
#د_که_پای

/* test library -- used only for internal development */
#إذا defined(LUA_DEBUG)
#ضمن "ltests.c"
#د_که_پای

/* lua */
#که_تعریف_شوی MAKE_LUA
#ضمن "lua.c"
#د_که_پای

/* luac */
#که_تعریف_شوی MAKE_LUAC
#ضمن "luac.c"
#د_که_پای
