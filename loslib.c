/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#تعریف_کرو loslib_c
#تعریف_کرو LUA_LIB

#شامل "lprefix.h"


#شامل <errno.h>
#شامل <locale.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#اگر !defined(LUA_STRFTIMEOPTIONS)	/* { */

#اگر defined(LUA_USE_WINDOWS)
#تعریف_کرو LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#وگرنہ_اگر defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#تعریف_کرو LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#ورنہ  /* C99 specification */
#تعریف_کرو LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#اگر_ختم

#اگر_ختم					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#اگر !defined(LUA_NUMTIME)	/* { */

#تعریف_کرو l_timet			lua_Integer
#تعریف_کرو l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#تعریف_کرو l_gettime(L,arg)	luaL_checkinteger(L, arg)

#ورنہ				/* }{ */

#تعریف_کرو l_timet			lua_نمبر
#تعریف_کرو l_pushtime(L,t)		lua_pushnumber(L,(lua_نمبر)(t))
#تعریف_کرو l_gettime(L,arg)	luaL_checknumber(L, arg)

#اگر_ختم				/* } */


#اگر !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#اگر defined(LUA_USE_POSIX)	/* { */

#تعریف_کرو l_gmtime(t,r)		gmtime_r(t,r)
#تعریف_کرو l_localtime(t,r)	localtime_r(t,r)

#ورنہ				/* }{ */

/* ISO C definitions */
#تعریف_کرو l_gmtime(t,r)		((خالی)(r)->tm_sec, gmtime(t))
#تعریف_کرو l_localtime(t,r)	((خالی)(r)->tm_sec, localtime(t))

#اگر_ختم				/* } */

#اگر_ختم				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#اگر !defined(lua_tmpnam)	/* { */

#اگر defined(LUA_USE_POSIX)	/* { */

#شامل <unistd.h>

#تعریف_کرو LUA_TMPNAMBUFSIZE	32

#اگر !defined(LUA_TMPNAMTEMPLATE)
#تعریف_کرو LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#اگر_ختم

#تعریف_کرو lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        اگر (e != -1) close(e); \
        e = (e == -1); }

#ورنہ				/* }{ */

/* ISO C definitions */
#تعریف_کرو LUA_TMPNAMBUFSIZE	L_tmpnam
#تعریف_کرو lua_tmpnam(b,e)		{ e = (tmpnam(b) == تہی); }

#اگر_ختم				/* } */

#اگر_ختم				/* } */
/* }================================================================== */


#اگر !defined(l_system)
#اگر defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#تعریف_کرو l_system(cmd) ((cmd) == تہی ? 0 : -1)
#ورنہ
#تعریف_کرو l_system(cmd)	system(cmd)  /* default definition */
#اگر_ختم
#اگر_ختم


ساکن صحیح os_execute (lua_State *L) {
  ثابت حرف *cmd = luaL_optstring(L, 1, تہی);
  صحیح stat;
  errno = 0;
  stat = l_system(cmd);
  اگر (cmd != تہی)
    واپس luaL_execresult(L, stat);
  ورنہ {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    واپس 1;
  }
}


ساکن صحیح os_remove (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  errno = 0;
  واپس luaL_fileresult(L, remove(filename) == 0, filename);
}


ساکن صحیح os_rename (lua_State *L) {
  ثابت حرف *fromname = luaL_checkstring(L, 1);
  ثابت حرف *toname = luaL_checkstring(L, 2);
  errno = 0;
  واپس luaL_fileresult(L, rename(fromname, toname) == 0, تہی);
}


ساکن صحیح os_tmpname (lua_State *L) {
  حرف buff[LUA_TMPNAMBUFSIZE];
  صحیح err;
  lua_tmpnam(buff, err);
  اگر (l_unlikely(err))
    واپس luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  واپس 1;
}


ساکن صحیح os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  واپس 1;
}


ساکن صحیح os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_نمبر)clock())/(lua_نمبر)CLOCKS_PER_SEC);
  واپس 1;
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
ساکن خالی setfield (lua_State *L, ثابت حرف *کلید, صحیح قدر, صحیح delta) {
  #اگر (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= زیادہ_سے_زیادہ_عدد)
    اگر (l_unlikely(قدر > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", کلید);
  #اگر_ختم
  lua_pushinteger(L, (lua_Integer)قدر + delta);
  lua_setfield(L, -2, کلید);
}


ساکن خالی setboolfield (lua_State *L, ثابت حرف *کلید, صحیح قدر) {
  اگر (قدر < 0)  /* undefined? */
    واپس;  /* does not set field */
  lua_pushboolean(L, قدر);
  lua_setfield(L, -2, کلید);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ساکن خالی setallfields (lua_State *L, ساخت tm *stm) {
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


ساکن صحیح getboolfield (lua_State *L, ثابت حرف *کلید) {
  صحیح res;
  res = (lua_getfield(L, -1, کلید) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_نکالو(L, 1);
  واپس res;
}


ساکن صحیح getfield (lua_State *L, ثابت حرف *کلید, صحیح d, صحیح delta) {
  صحیح isnum;
  صحیح t = lua_getfield(L, -1, کلید);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  اگر (!isnum) {  /* field is not an integer? */
    اگر (l_unlikely(t != LUA_TNIL))  /* some other value? */
      واپس luaL_error(L, "field '%s' is not an integer", کلید);
    ورنہ اگر (l_unlikely(d < 0))  /* absent field; no default? */
      واپس luaL_error(L, "field '%s' missing in date table", کلید);
    res = d;
  }
  ورنہ {
    اگر (!(res >= 0 ? res - delta <= زیادہ_سے_زیادہ_عدد : INT_MIN + delta <= res))
      واپس luaL_error(L, "field '%s' is out-of-bound", کلید);
    res -= delta;
  }
  lua_نکالو(L, 1);
  واپس (صحیح)res;
}


ساکن ثابت حرف *checkoption (lua_State *L, ثابت حرف *conv,
                                size_t convlen, حرف *buff) {
  ثابت حرف *option = LUA_STRFTIMEOPTIONS;
  غیرعلامتی oplen = 1;  /* length of options being checked */
  برائے (; *option != '\0' && oplen <= convlen; option += oplen) {
    اگر (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    ورنہ اگر (memcmp(conv, option, oplen) == 0) {  /* match? */
      حافظہ_نقل(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      واپس conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  واپس conv;  /* to avoid warnings */
}


ساکن time_t l_checktime (lua_State *L, صحیح arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  واپس (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#تعریف_کرو SIZETIMEFMT	250


ساکن صحیح os_date (lua_State *L) {
  size_t slen;
  ثابت حرف *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(تہی));
  ثابت حرف *se = s + slen;  /* 's' end */
  ساخت tm tmr, *stm;
  اگر (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  ورنہ
    stm = l_localtime(&t, &tmr);
  اگر (stm == تہی)  /* invalid date? */
    واپس luaL_error(L,
                 "date result cannot be represented in this installation");
  اگر (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  ورنہ {
    حرف cc[4];  /* buffer for individual conversion specifiers */
    lua_L_بفر b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    جب (s < se) {
      اگر (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      ورنہ {
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
  واپس 1;
}


ساکن صحیح os_وقت_aux (lua_State *L, time_t t, صحیح err) {
  اگر (err || t != (time_t)(l_timet)t)
    واپس luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  واپس 1;
}


ساکن صحیح os_وقت (lua_State *L) {
  اگر (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(تہی);  /* get current time; error if it is -1 */
    واپس os_وقت_aux(L, t, (t == (time_t)(-1)));
  }
  ورنہ {
    time_t t;
    ساخت tm ts;
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
    واپس os_وقت_aux(L, t, ts.tm_wday == -1);
  }
}


ساکن صحیح os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_نمبر)difftime(t1, t2));
  واپس 1;
}

/* }====================================================== */


ساکن صحیح os_setlocale (lua_State *L) {
  ساکن ثابت صحیح cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ساکن ثابت حرف *ثابت catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", تہی};
  ثابت حرف *l = luaL_optstring(L, 1, تہی);
  صحیح op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  واپس 1;
}


ساکن صحیح os_exit (lua_State *L) {
  صحیح حالت_2;
  اگر (lua_isboolean(L, 1))
    حالت_2 = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  ورنہ
    حالت_2 = (صحیح)luaL_optinteger(L, 1, EXIT_SUCCESS);
  اگر (lua_toboolean(L, 2))
    lua_close(L);
  اگر (L) باہر_نکلو(حالت_2);  /* 'if' to avoid warnings for unreachable 'return' */
  واپس 0;
}


ساکن ثابت luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_وقت},
  {"tmpname", os_tmpname},
  {تہی, تہی}
};

/* }====================================================== */



LUAMOD_API صحیح luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  واپس 1;
}

