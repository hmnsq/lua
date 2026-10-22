/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#تعریف_کن loslib_c
#تعریف_کن LUA_LIB

#ضمن "lprefix.h"


#ضمن <errno.h>
#ضمن <locale.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#إذا !defined(LUA_STRFTIMEOPTIONS)	/* { */

#إذا defined(LUA_USE_WINDOWS)
#تعریف_کن LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#وإلاإذا defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#تعریف_کن LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#وإلا  /* C99 specification */
#تعریف_کن LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#پایان_اگر

#پایان_اگر					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#إذا !defined(LUA_NUMTIME)	/* { */

#تعریف_کن l_timet			lua_Integer
#تعریف_کن l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#تعریف_کن l_gettime(L,arg)	luaL_checkinteger(L, arg)

#وإلا				/* }{ */

#تعریف_کن l_timet			lua_عدد
#تعریف_کن l_pushtime(L,t)		lua_pushnumber(L,(lua_عدد)(t))
#تعریف_کن l_gettime(L,arg)	luaL_checknumber(L, arg)

#پایان_اگر				/* } */


#إذا !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#إذا defined(LUA_USE_POSIX)	/* { */

#تعریف_کن l_gmtime(t,r)		gmtime_r(t,r)
#تعریف_کن l_localtime(t,r)	localtime_r(t,r)

#وإلا				/* }{ */

/* ISO C definitions */
#تعریف_کن l_gmtime(t,r)		((فراغ)(r)->tm_sec, gmtime(t))
#تعریف_کن l_localtime(t,r)	((فراغ)(r)->tm_sec, localtime(t))

#پایان_اگر				/* } */

#پایان_اگر				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#إذا !defined(lua_tmpnam)	/* { */

#إذا defined(LUA_USE_POSIX)	/* { */

#ضمن <unistd.h>

#تعریف_کن LUA_TMPNAMBUFSIZE	32

#إذا !defined(LUA_TMPNAMTEMPLATE)
#تعریف_کن LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#پایان_اگر

#تعریف_کن lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        إذا (e != -1) close(e); \
        e = (e == -1); }

#وإلا				/* }{ */

/* ISO C definitions */
#تعریف_کن LUA_TMPNAMBUFSIZE	L_tmpnam
#تعریف_کن lua_tmpnam(b,e)		{ e = (tmpnam(b) == عدم); }

#پایان_اگر				/* } */

#پایان_اگر				/* } */
/* }================================================================== */


#إذا !defined(l_system)
#إذا defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#تعریف_کن l_system(cmd) ((cmd) == عدم ? 0 : -1)
#وإلا
#تعریف_کن l_system(cmd)	system(cmd)  /* default definition */
#پایان_اگر
#پایان_اگر


ساكن صحيح os_execute (lua_State *L) {
  ثابت حرف *cmd = luaL_optstring(L, 1, عدم);
  صحيح stat;
  errno = 0;
  stat = l_system(cmd);
  إذا (cmd != عدم)
    أرجع luaL_execresult(L, stat);
  وإلا {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    أرجع 1;
  }
}


ساكن صحيح os_remove (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  errno = 0;
  أرجع luaL_fileresult(L, remove(filename) == 0, filename);
}


ساكن صحيح os_rename (lua_State *L) {
  ثابت حرف *fromname = luaL_checkstring(L, 1);
  ثابت حرف *toname = luaL_checkstring(L, 2);
  errno = 0;
  أرجع luaL_fileresult(L, rename(fromname, toname) == 0, عدم);
}


ساكن صحيح os_tmpname (lua_State *L) {
  حرف buff[LUA_TMPNAMBUFSIZE];
  صحيح err;
  lua_tmpnam(buff, err);
  إذا (l_unlikely(err))
    أرجع luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  أرجع 1;
}


ساكن صحيح os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  أرجع 1;
}


ساكن صحيح os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_عدد)clock())/(lua_عدد)CLOCKS_PER_SEC);
  أرجع 1;
}


/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

/*
** About the overflow check: an overflow cannot occur when time
** is represented by a lua_Integer, because either lua_Integer is
** large enough to represent all int fields or it is not large enough
** to represent a time that cause a field to overflow.  However, if
** times are represented as doubles and lua_Integer is int, then the
** time 0x1.e1853b0d184f6p+55 would cause an overflow when adding 1900
** to compute the year.
*/
ساكن فراغ setfield (lua_State *L, ثابت حرف *کلید, صحيح مقدار, صحيح delta) {
  #إذا (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= بیشترین_صحیح)
    إذا (l_unlikely(مقدار > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", کلید);
  #پایان_اگر
  lua_pushinteger(L, (lua_Integer)مقدار + delta);
  lua_setfield(L, -2, کلید);
}


ساكن فراغ setboolfield (lua_State *L, ثابت حرف *کلید, صحيح مقدار) {
  إذا (مقدار < 0)  /* undefined? */
    أرجع;  /* does not set field */
  lua_pushboolean(L, مقدار);
  lua_setfield(L, -2, کلید);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ساكن فراغ setallfields (lua_State *L, بنية tm *stm) {
  setfield(L, "year", stm->tm_year, 1900);
  setfield(L, "month", stm->tm_mon, 1);
  setfield(L, "day", stm->tm_mday, 0);
  setfield(L, "hour", stm->tm_hour, 0);
  setfield(L, "min", stm->tm_min, 0);
  setfield(L, "sec", stm->tm_sec, 0);
  setfield(L, "yday", stm->tm_yday, 1);
  setfield(L, "wday", stm->tm_wday, 1);
  setboolfield(L, "isdst", stm->tm_isdst);
}


ساكن صحيح getboolfield (lua_State *L, ثابت حرف *کلید) {
  صحيح res;
  res = (lua_getfield(L, -1, کلید) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_برداشت(L, 1);
  أرجع res;
}


ساكن صحيح getfield (lua_State *L, ثابت حرف *کلید, صحيح d, صحيح delta) {
  صحيح isnum;
  صحيح t = lua_getfield(L, -1, کلید);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  إذا (!isnum) {  /* field is not an integer? */
    إذا (l_unlikely(t != LUA_TNIL))  /* some other value? */
      أرجع luaL_error(L, "field '%s' is not an integer", کلید);
    وإلا إذا (l_unlikely(d < 0))  /* absent field; no default? */
      أرجع luaL_error(L, "field '%s' missing in date table", کلید);
    res = d;
  }
  وإلا {
    إذا (!(res >= 0 ? res - delta <= بیشترین_صحیح : INT_MIN + delta <= res))
      أرجع luaL_error(L, "field '%s' is out-of-bound", کلید);
    res -= delta;
  }
  lua_برداشت(L, 1);
  أرجع (صحيح)res;
}


ساكن ثابت حرف *checkoption (lua_State *L, ثابت حرف *conv,
                                size_t convlen, حرف *buff) {
  ثابت حرف *option = LUA_STRFTIMEOPTIONS;
  بیعلامت oplen = 1;  /* length of options being checked */
  لكل (; *option != '\0' && oplen <= convlen; option += oplen) {
    إذا (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    وإلا إذا (memcmp(conv, option, oplen) == 0) {  /* match? */
      کپی_حافظه(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      أرجع conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  أرجع conv;  /* to avoid warnings */
}


ساكن time_t l_checktime (lua_State *L, صحيح arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  أرجع (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#تعریف_کن SIZETIMEFMT	250


ساكن صحيح os_date (lua_State *L) {
  size_t slen;
  ثابت حرف *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(عدم));
  ثابت حرف *se = s + slen;  /* 's' end */
  بنية tm tmr, *stm;
  إذا (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  وإلا
    stm = l_localtime(&t, &tmr);
  إذا (stm == عدم)  /* invalid date? */
    أرجع luaL_error(L,
                 "date result cannot be represented in this installation");
  إذا (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  وإلا {
    حرف cc[4];  /* buffer for individual conversion specifiers */
    lua_L_میانگیر b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    طالما (s < se) {
      إذا (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      وإلا {
        size_t reslen;
        حرف *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  أرجع 1;
}


ساكن صحيح os_زمان_aux (lua_State *L, time_t t, صحيح err) {
  إذا (err || t != (time_t)(l_timet)t)
    أرجع luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  أرجع 1;
}


ساكن صحيح os_زمان (lua_State *L) {
  إذا (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(عدم);  /* get current time; error if it is -1 */
    أرجع os_زمان_aux(L, t, (t == (time_t)(-1)));
  }
  وإلا {
    time_t t;
    بنية tm ts;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);  /* make sure table is at the top */
    ts.tm_year = getfield(L, "year", -1, 1900);
    ts.tm_mon = getfield(L, "month", -1, 1);
    ts.tm_mday = getfield(L, "day", -1, 0);
    ts.tm_hour = getfield(L, "hour", 12, 0);
    ts.tm_min = getfield(L, "min", 0, 0);
    ts.tm_sec = getfield(L, "sec", 0, 0);
    ts.tm_isdst = getboolfield(L, "isdst");
    ts.tm_wday = -1;  /* if call succeeds, it will set this value */
    t = mktime(&ts);
    setallfields(L, &ts);  /* update fields with normalized values */
    /* error if tm_wday was not set */
    أرجع os_زمان_aux(L, t, ts.tm_wday == -1);
  }
}


ساكن صحيح os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_عدد)difftime(t1, t2));
  أرجع 1;
}

/* }====================================================== */


ساكن صحيح os_setlocale (lua_State *L) {
  ساكن ثابت صحيح cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ساكن ثابت حرف *ثابت catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", عدم};
  ثابت حرف *l = luaL_optstring(L, 1, عدم);
  صحيح op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  أرجع 1;
}


ساكن صحيح os_exit (lua_State *L) {
  صحيح وضعیت;
  إذا (lua_isboolean(L, 1))
    وضعیت = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  وإلا
    وضعیت = (صحيح)luaL_optinteger(L, 1, EXIT_SUCCESS);
  إذا (lua_toboolean(L, 2))
    lua_close(L);
  إذا (L) خروج(وضعیت);  /* 'if' to avoid warnings for unreachable 'return' */
  أرجع 0;
}


ساكن ثابت luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_زمان},
  {"tmpname", os_tmpname},
  {عدم, عدم}
};

/* }====================================================== */



LUAMOD_API صحيح luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  أرجع 1;
}

