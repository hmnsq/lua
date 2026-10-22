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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAKE_LIB
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAKE_LUAC
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAKE_LUA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAKE_LUA
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_LINUX
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_MACOSX
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_POSIX
#ਜੇ_ਅੰਤ


/*
** Other specific features
*/
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_32_ਬਿੱਟ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_C89
#ਜੇ_ਅੰਤ


/* no need to change anything below this line ----------------------------- */

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>

/* setup for luaconf.h */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "luaconf.h"

/* do not export internal symbols */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUAI_FUNC
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUAI_DDEC
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUAI_DDEF
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_FUNC	ਸਥਿਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_DDEC(def)	/*ਖਾਲੀ_ਥਾਵਾਂ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_DDEF	ਸਥਿਰ

/* core -- used by all */
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lctype.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lundump.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldump.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "llex.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lcode.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.c"

/* auxiliary library -- used by all */
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.c"

/* standard library  -- not used by luac */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAKE_LUAC
#ਅਨ੍ਤਰ੍ਭਾਵ "lbaselib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lcorolib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldblib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "liolib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmathlib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "loadlib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "loslib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstrlib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltablib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "lutf8lib.c"
#ਅਨ੍ਤਰ੍ਭਾਵ "linit.c"
#ਜੇ_ਅੰਤ

/* test library -- used only for internal development */
#ਜੇ defined(LUA_DEBUG)
#ਅਨ੍ਤਰ੍ਭਾਵ "ltests.c"
#ਜੇ_ਅੰਤ

/* lua */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKE_LUA
#ਅਨ੍ਤਰ੍ਭਾਵ "lua.c"
#ਜੇ_ਅੰਤ

/* luac */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKE_LUAC
#ਅਨ੍ਤਰ੍ਭਾਵ "luac.c"
#ਜੇ_ਅੰਤ
