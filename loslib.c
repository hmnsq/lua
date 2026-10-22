/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

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
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#وگرنهاگر defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#وگرنه  /* C99 specification */
#define LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#endif

#endif					/* } */
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

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#وگرنه				/* }{ */

#define l_timet			lua_عدد
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_عدد)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#اگر !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#اگر defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#وگرنه				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((تهی)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((تهی)(r)->tm_sec, localtime(t))

#endif				/* } */

#endif				/* } */

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

#define LUA_TMPNAMBUFSIZE	32

#اگر !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        اگر (e != -1) close(e); \
        e = (e == -1); }

#وگرنه				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == پوچ); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#اگر !defined(l_system)
#اگر defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == پوچ ? 0 : -1)
#وگرنه
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


ایستا صحیح os_execute (lua_State *L) {
  ثابت نویسه *cmd = luaL_optstring(L, 1, پوچ);
  صحیح stat;
  errno = 0;
  stat = l_system(cmd);
  اگر (cmd != پوچ)
    بازگشت luaL_execresult(L, stat);
  وگرنه {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    بازگشت 1;
  }
}


ایستا صحیح os_remove (lua_State *L) {
  ثابت نویسه *filename = luaL_checkstring(L, 1);
  errno = 0;
  بازگشت luaL_fileresult(L, remove(filename) == 0, filename);
}


ایستا صحیح os_rename (lua_State *L) {
  ثابت نویسه *fromname = luaL_checkstring(L, 1);
  ثابت نویسه *toname = luaL_checkstring(L, 2);
  errno = 0;
  بازگشت luaL_fileresult(L, rename(fromname, toname) == 0, پوچ);
}


ایستا صحیح os_tmpname (lua_State *L) {
  نویسه buff[LUA_TMPNAMBUFSIZE];
  صحیح err;
  lua_tmpnam(buff, err);
  اگر (l_unlikely(err))
    بازگشت luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  بازگشت 1;
}


ایستا صحیح os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  بازگشت 1;
}


ایستا صحیح os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_عدد)clock())/(lua_عدد)CLOCKS_PER_SEC);
  بازگشت 1;
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
ایستا تهی setfield (lua_State *L, ثابت نویسه *کلید, صحیح مقدار, صحیح delta) {
  #اگر (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    اگر (l_unlikely(مقدار > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", کلید);
  #endif
  lua_pushinteger(L, (lua_Integer)مقدار + delta);
  lua_setfield(L, -2, کلید);
}


ایستا تهی setboolfield (lua_State *L, ثابت نویسه *کلید, صحیح مقدار) {
  اگر (مقدار < 0)  /* undefined? */
    بازگشت;  /* does not set field */
  lua_pushboolean(L, مقدار);
  lua_setfield(L, -2, کلید);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ایستا تهی setallfields (lua_State *L, ساختار tm *stm) {
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


ایستا صحیح getboolfield (lua_State *L, ثابت نویسه *کلید) {
  صحیح res;
  res = (lua_getfield(L, -1, کلید) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_برداشت(L, 1);
  بازگشت res;
}


ایستا صحیح getfield (lua_State *L, ثابت نویسه *کلید, صحیح d, صحیح delta) {
  صحیح isnum;
  صحیح t = lua_getfield(L, -1, کلید);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  اگر (!isnum) {  /* field is not an integer? */
    اگر (l_unlikely(t != LUA_TNIL))  /* some other value? */
      بازگشت luaL_error(L, "field '%s' is not an integer", کلید);
    وگرنه اگر (l_unlikely(d < 0))  /* absent field; no default? */
      بازگشت luaL_error(L, "field '%s' missing in date table", کلید);
    res = d;
  }
  وگرنه {
    اگر (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      بازگشت luaL_error(L, "field '%s' is out-of-bound", کلید);
    res -= delta;
  }
  lua_برداشت(L, 1);
  بازگشت (صحیح)res;
}


ایستا ثابت نویسه *checkoption (lua_State *L, ثابت نویسه *conv,
                                size_t convlen, نویسه *buff) {
  ثابت نویسه *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  برای (; *option != '\0' && oplen <= convlen; option += oplen) {
    اگر (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    وگرنه اگر (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      بازگشت conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  بازگشت conv;  /* to avoid warnings */
}


ایستا time_t l_checktime (lua_State *L, صحیح arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  بازگشت (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


ایستا صحیح os_date (lua_State *L) {
  size_t slen;
  ثابت نویسه *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(پوچ));
  ثابت نویسه *se = s + slen;  /* 's' end */
  ساختار tm tmr, *stm;
  اگر (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  وگرنه
    stm = l_localtime(&t, &tmr);
  اگر (stm == پوچ)  /* invalid date? */
    بازگشت luaL_error(L,
                 "date result cannot be represented in this installation");
  اگر (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  وگرنه {
    نویسه cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    وقتی (s < se) {
      اگر (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      وگرنه {
        size_t reslen;
        نویسه *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  بازگشت 1;
}


ایستا صحیح os_time_aux (lua_State *L, time_t t, صحیح err) {
  اگر (err || t != (time_t)(l_timet)t)
    بازگشت luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  بازگشت 1;
}


ایستا صحیح os_time (lua_State *L) {
  اگر (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(پوچ);  /* get current time; error if it is -1 */
    بازگشت os_time_aux(L, t, (t == (time_t)(-1)));
  }
  وگرنه {
    time_t t;
    ساختار tm ts;
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
    بازگشت os_time_aux(L, t, ts.tm_wday == -1);
  }
}


ایستا صحیح os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_عدد)difftime(t1, t2));
  بازگشت 1;
}

/* }====================================================== */


ایستا صحیح os_setlocale (lua_State *L) {
  ایستا ثابت صحیح cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ایستا ثابت نویسه *ثابت catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", پوچ};
  ثابت نویسه *l = luaL_optstring(L, 1, پوچ);
  صحیح op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  بازگشت 1;
}


ایستا صحیح os_exit (lua_State *L) {
  صحیح status;
  اگر (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  وگرنه
    status = (صحیح)luaL_optinteger(L, 1, EXIT_SUCCESS);
  اگر (lua_toboolean(L, 2))
    lua_close(L);
  اگر (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  بازگشت 0;
}


ایستا ثابت luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_time},
  {"tmpname", os_tmpname},
  {پوچ, پوچ}
};

/* }====================================================== */



LUAMOD_API صحیح luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  بازگشت 1;
}

