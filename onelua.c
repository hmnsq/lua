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
#अगर_अघोषित MAKE_LIB
#अगर_अघोषित MAKE_LUAC
#अगर_अघोषित MAKE_LUA
#घोषणा MAKE_LUA
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर


/*
** Choose suitable platform-specific features. Default is no
** platform-specific features. Some of these options may need extra
** libraries such as -ldl -lreadline -lncurses
*/
#अगर 0
#घोषणा LUA_USE_LINUX
#घोषणा LUA_USE_MACOSX
#घोषणा LUA_USE_POSIX
#पूर्ण_अगर


/*
** Other specific features
*/
#अगर 0
#घोषणा LUA_32BITS
#घोषणा LUA_USE_C89
#पूर्ण_अगर


/* no need to change anything below this line ----------------------------- */

#समावेश "lprefix.h"

#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <float.h>
#समावेश <limits.h>
#समावेश <locale.h>
#समावेश <math.h>
#समावेश <setjmp.h>
#समावेश <signal.h>
#समावेश <stdarg.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>

/* setup for luaconf.h */
#घोषणा LUA_CORE
#घोषणा LUA_LIB

#समावेश "luaconf.h"

/* do not export internal symbols */
#अघोषित LUAI_FUNC
#अघोषित LUAI_DDEC
#अघोषित LUAI_DDEF
#घोषणा LUAI_FUNC	अटल
#घोषणा LUAI_DDEC(def)	/* empty */
#घोषणा LUAI_DDEF	अटल

/* core -- used by all */
#समावेश "lzio.c"
#समावेश "lctype.c"
#समावेश "lopcodes.c"
#समावेश "lmem.c"
#समावेश "lundump.c"
#समावेश "ldump.c"
#समावेश "lstate.c"
#समावेश "lgc.c"
#समावेश "llex.c"
#समावेश "lcode.c"
#समावेश "lparser.c"
#समावेश "ldebug.c"
#समावेश "lfunc.c"
#समावेश "lobject.c"
#समावेश "ltm.c"
#समावेश "lstring.c"
#समावेश "ltable.c"
#समावेश "ldo.c"
#समावेश "lvm.c"
#समावेश "lapi.c"

/* auxiliary library -- used by all */
#समावेश "lauxlib.c"

/* standard library  -- not used by luac */
#अगर_अघोषित MAKE_LUAC
#समावेश "lbaselib.c"
#समावेश "lcorolib.c"
#समावेश "ldblib.c"
#समावेश "liolib.c"
#समावेश "lmathlib.c"
#समावेश "loadlib.c"
#समावेश "loslib.c"
#समावेश "lstrlib.c"
#समावेश "ltablib.c"
#समावेश "lutf8lib.c"
#समावेश "linit.c"
#पूर्ण_अगर

/* test library -- used only for internal development */
#अगर defined(LUA_DEBUG)
#समावेश "ltests.c"
#पूर्ण_अगर

/* lua */
#अगर_घोषित MAKE_LUA
#समावेश "lua.c"
#पूर्ण_अगर

/* luac */
#अगर_घोषित MAKE_LUAC
#समावेश "luac.c"
#पूर्ण_अगर
