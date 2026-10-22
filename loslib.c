/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ loslib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#ਜੇ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#ਜੇ defined(LUA_USE_WINDOWS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#ਜਾਂਜੇ defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#ਨਹੀਂਤਾਂ  /* C99 specification */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#ਜੇ !defined(LUA_NUMTIME)	/* { */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_timet			lua_Integer
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gettime(L,arg)	luaL_checkinteger(L, arg)

#ਨਹੀਂਤਾਂ				/* }{ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_timet			lua_ਸੰਖਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_pushtime(L,t)		lua_pushnumber(L,(lua_ਸੰਖਿਆ)(t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gettime(L,arg)	luaL_checknumber(L, arg)

#ਜੇ_ਅੰਤ				/* } */


#ਜੇ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#ਜੇ defined(LUA_USE_POSIX)	/* { */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gmtime(t,r)		gmtime_r(t,r)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_localtime(t,r)	localtime_r(t,r)

#ਨਹੀਂਤਾਂ				/* }{ */

/* ISO C definitions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gmtime(t,r)		((ਖਾਲੀ)(r)->tm_sec, gmtime(t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_localtime(t,r)	((ਖਾਲੀ)(r)->tm_sec, localtime(t))

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#ਜੇ !defined(lua_tmpnam)	/* { */

#ਜੇ defined(LUA_USE_POSIX)	/* { */

#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TMPNAMBUFSIZE	32

#ਜੇ !defined(LUA_TMPNAMTEMPLATE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        ਜੇ (e != -1) close(e); \
        e = (e == -1); }

#ਨਹੀਂਤਾਂ				/* }{ */

/* ISO C definitions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TMPNAMBUFSIZE	L_tmpnam
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_tmpnam(b,e)		{ e = (tmpnam(b) == ਨਿਰਾ); }

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */
/* }================================================================== */


#ਜੇ !defined(l_system)
#ਜੇ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_system(cmd) ((cmd) == ਨਿਰਾ ? 0 : -1)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_system(cmd)	system(cmd)  /* default definition */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਪੂਰਨ os_execute (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *cmd = luaL_optstring(L, 1, ਨਿਰਾ);
  ਪੂਰਨ stat;
  errno = 0;
  stat = l_system(cmd);
  ਜੇ (cmd != ਨਿਰਾ)
    ਵਾਪਸ luaL_execresult(L, stat);
  ਨਹੀਂਤਾਂ {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ਵਾਪਸ 1;
  }
}


ਸਥਿਰ ਪੂਰਨ os_remove (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *filename = luaL_checkstring(L, 1);
  errno = 0;
  ਵਾਪਸ luaL_fileresult(L, remove(filename) == 0, filename);
}


ਸਥਿਰ ਪੂਰਨ os_rename (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *fromname = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *toname = luaL_checkstring(L, 2);
  errno = 0;
  ਵਾਪਸ luaL_fileresult(L, rename(fromname, toname) == 0, ਨਿਰਾ);
}


ਸਥਿਰ ਪੂਰਨ os_tmpname (lua_State *L) {
  ਅੱਖਰ buff[LUA_TMPNAMBUFSIZE];
  ਪੂਰਨ err;
  lua_tmpnam(buff, err);
  ਜੇ (l_unlikely(err))
    ਵਾਪਸ luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_ਸੰਖਿਆ)clock())/(lua_ਸੰਖਿਆ)CLOCKS_PER_SEC);
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਖਾਲੀ setfield (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ, ਪੂਰਨ ਮੁੱਲ, ਪੂਰਨ delta) {
  #ਜੇ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)
    ਜੇ (l_unlikely(ਮੁੱਲ > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", ਕੁੰਜੀ);
  #ਜੇ_ਅੰਤ
  lua_pushinteger(L, (lua_Integer)ਮੁੱਲ + delta);
  lua_setfield(L, -2, ਕੁੰਜੀ);
}


ਸਥਿਰ ਖਾਲੀ setboolfield (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ, ਪੂਰਨ ਮੁੱਲ) {
  ਜੇ (ਮੁੱਲ < 0)  /* undefined? */
    ਵਾਪਸ;  /* does not set field */
  lua_pushboolean(L, ਮੁੱਲ);
  lua_setfield(L, -2, ਕੁੰਜੀ);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ਸਥਿਰ ਖਾਲੀ setallfields (lua_State *L, ਬਣਤਰ tm *stm) {
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


ਸਥਿਰ ਪੂਰਨ getboolfield (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ) {
  ਪੂਰਨ res;
  res = (lua_getfield(L, -1, ਕੁੰਜੀ) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_ਕੱਢੋ(L, 1);
  ਵਾਪਸ res;
}


ਸਥਿਰ ਪੂਰਨ getfield (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ, ਪੂਰਨ d, ਪੂਰਨ delta) {
  ਪੂਰਨ isnum;
  ਪੂਰਨ t = lua_getfield(L, -1, ਕੁੰਜੀ);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  ਜੇ (!isnum) {  /* field is not an integer? */
    ਜੇ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ਵਾਪਸ luaL_error(L, "field '%s' is not an integer", ਕੁੰਜੀ);
    ਨਹੀਂਤਾਂ ਜੇ (l_unlikely(d < 0))  /* absent field; no default? */
      ਵਾਪਸ luaL_error(L, "field '%s' missing in date table", ਕੁੰਜੀ);
    res = d;
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (!(res >= 0 ? res - delta <= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ : INT_MIN + delta <= res))
      ਵਾਪਸ luaL_error(L, "field '%s' is out-of-bound", ਕੁੰਜੀ);
    res -= delta;
  }
  lua_ਕੱਢੋ(L, 1);
  ਵਾਪਸ (ਪੂਰਨ)res;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *checkoption (lua_State *L, ਅਚਲ ਅੱਖਰ *conv,
                                size_t convlen, ਅੱਖਰ *buff) {
  ਅਚਲ ਅੱਖਰ *option = LUA_STRFTIMEOPTIONS;
  ਬਿਨਾਨਿਸ਼ਾਨੀ oplen = 1;  /* length of options being checked */
  ਲਈ (; *option != '\0' && oplen <= convlen; option += oplen) {
    ਜੇ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    ਨਹੀਂਤਾਂ ਜੇ (memcmp(conv, option, oplen) == 0) {  /* match? */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ਵਾਪਸ conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ਵਾਪਸ conv;  /* to avoid warnings */
}


ਸਥਿਰ time_t l_checktime (lua_State *L, ਪੂਰਨ arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ਵਾਪਸ (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZETIMEFMT	250


ਸਥਿਰ ਪੂਰਨ os_date (lua_State *L) {
  size_t slen;
  ਅਚਲ ਅੱਖਰ *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ਨਿਰਾ));
  ਅਚਲ ਅੱਖਰ *se = s + slen;  /* 's' end */
  ਬਣਤਰ tm tmr, *stm;
  ਜੇ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  ਨਹੀਂਤਾਂ
    stm = l_localtime(&t, &tmr);
  ਜੇ (stm == ਨਿਰਾ)  /* invalid date? */
    ਵਾਪਸ luaL_error(L,
                 "date result cannot be represented in this installation");
  ਜੇ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  ਨਹੀਂਤਾਂ {
    ਅੱਖਰ cc[4];  /* buffer for individual conversion specifiers */
    lua_L_ਬਫਰ b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    ਜਦੋਂ (s < se) {
      ਜੇ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      ਨਹੀਂਤਾਂ {
        size_t reslen;
        ਅੱਖਰ *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ os_ਸਮਾਂ_aux (lua_State *L, time_t t, ਪੂਰਨ err) {
  ਜੇ (err || t != (time_t)(l_timet)t)
    ਵਾਪਸ luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ os_ਸਮਾਂ (lua_State *L) {
  ਜੇ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ਨਿਰਾ);  /* get current time; error if it is -1 */
    ਵਾਪਸ os_ਸਮਾਂ_aux(L, t, (t == (time_t)(-1)));
  }
  ਨਹੀਂਤਾਂ {
    time_t t;
    ਬਣਤਰ tm ts;
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
    ਵਾਪਸ os_ਸਮਾਂ_aux(L, t, ts.tm_wday == -1);
  }
}


ਸਥਿਰ ਪੂਰਨ os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_ਸੰਖਿਆ)difftime(t1, t2));
  ਵਾਪਸ 1;
}

/* }====================================================== */


ਸਥਿਰ ਪੂਰਨ os_setlocale (lua_State *L) {
  ਸਥਿਰ ਅਚਲ ਪੂਰਨ cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ਨਿਰਾ};
  ਅਚਲ ਅੱਖਰ *l = luaL_optstring(L, 1, ਨਿਰਾ);
  ਪੂਰਨ op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ os_exit (lua_State *L) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਜੇ (lua_isboolean(L, 1))
    ਸਥਿਤੀ = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  ਨਹੀਂਤਾਂ
    ਸਥਿਤੀ = (ਪੂਰਨ)luaL_optinteger(L, 1, EXIT_SUCCESS);
  ਜੇ (lua_toboolean(L, 2))
    lua_close(L);
  ਜੇ (L) ਬਾਹਰ_ਨਿਕਲੋ(ਸਥਿਤੀ);  /* 'if' to avoid warnings for unreachable 'return' */
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਅਚਲ luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_ਸਮਾਂ},
  {"tmpname", os_tmpname},
  {ਨਿਰਾ, ਨਿਰਾ}
};

/* }====================================================== */



LUAMOD_API ਪੂਰਨ luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ਵਾਪਸ 1;
}

