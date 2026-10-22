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
#جيڪڏهن_وصف_نه_ٿيل MAKE_LIB
#جيڪڏهن_وصف_نه_ٿيل MAKE_LUAC
#جيڪڏهن_وصف_نه_ٿيل MAKE_LUA
#وصف_ڪريو MAKE_LUA
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#جيڪڏهن_پورو


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#إذا 0
#وصف_ڪريو LUA_USE_LINUX
#وصف_ڪريو LUA_USE_MACOSX
#وصف_ڪريو LUA_USE_POSIX
#جيڪڏهن_پورو


/*
** Other specific features
*/
#إذا 0
#وصف_ڪريو LUA_32_بٽ
#وصف_ڪريو LUA_USE_C89
#جيڪڏهن_پورو


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
#وصف_ڪريو LUA_CORE
#وصف_ڪريو LUA_LIB

#ضمن "luaconf.h"

/* do not export internal symbols */
#وصف_رد LUAI_FUNC
#وصف_رد LUAI_DDEC
#وصف_رد LUAI_DDEF
#وصف_ڪريو LUAI_FUNC	ساكن
#وصف_ڪريو LUAI_DDEC(def)	/*خالي_جايون*/
#وصف_ڪريو LUAI_DDEF	ساكن

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
#جيڪڏهن_وصف_نه_ٿيل MAKE_LUAC
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
#جيڪڏهن_پورو

/* test library -- used only for internal development */
#إذا defined(LUA_DEBUG)
#ضمن "ltests.c"
#جيڪڏهن_پورو

/* lua */
#جيڪڏهن_وصف_ٿيل MAKE_LUA
#ضمن "lua.c"
#جيڪڏهن_پورو

/* luac */
#جيڪڏهن_وصف_ٿيل MAKE_LUAC
#ضمن "luac.c"
#جيڪڏهن_پورو
