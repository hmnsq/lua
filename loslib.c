/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#ಯದಿ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#ಯದಿ defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#ನೋಚೇದ್ಯದಿ defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#ಅನ್ಯಥಾ  /* C99 specification */
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
#ಯದಿ !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#ಅನ್ಯಥಾ				/* }{ */

#define l_timet			lua_ಸಂಖ್ಯೆ
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_ಸಂಖ್ಯೆ)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#ಯದಿ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#ಯದಿ defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#ಅನ್ಯಥಾ				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((ಶೂನ್ಯ)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((ಶೂನ್ಯ)(r)->tm_sec, localtime(t))

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
#ಯದಿ !defined(lua_tmpnam)	/* { */

#ಯದಿ defined(LUA_USE_POSIX)	/* { */

#ಅನ್ತರ್ಭಾವ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#ಯದಿ !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        ಯದಿ (e != -1) close(e); \
        e = (e == -1); }

#ಅನ್ಯಥಾ				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == ರಿಕ್ತ); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#ಯದಿ !defined(l_system)
#ಯದಿ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == ರಿಕ್ತ ? 0 : -1)
#ಅನ್ಯಥಾ
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_execute (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *cmd = luaL_optstring(L, 1, ರಿಕ್ತ);
  ಪೂರ್ಣಾಙ್ಕ stat;
  errno = 0;
  stat = l_system(cmd);
  ಯದಿ (cmd != ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ luaL_execresult(L, stat);
  ಅನ್ಯಥಾ {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_remove (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *filename = luaL_checkstring(L, 1);
  errno = 0;
  ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, remove(filename) == 0, filename);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_rename (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *fromname = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *toname = luaL_checkstring(L, 2);
  errno = 0;
  ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, rename(fromname, toname) == 0, ರಿಕ್ತ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_tmpname (lua_State *L) {
  ವರ್ಣ buff[LUA_TMPNAMBUFSIZE];
  ಪೂರ್ಣಾಙ್ಕ err;
  lua_tmpnam(buff, err);
  ಯದಿ (l_unlikely(err))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_ಸಂಖ್ಯೆ)clock())/(lua_ಸಂಖ್ಯೆ)CLOCKS_PER_SEC);
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಸ್ಥಿರ ಶೂನ್ಯ setfield (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ, ಪೂರ್ಣಾಙ್ಕ delta) {
  #ಯದಿ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    ಯದಿ (l_unlikely(ಮೌಲ್ಯ > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", ಕೀಲಿ);
  #endif
  lua_pushinteger(L, (lua_Integer)ಮೌಲ್ಯ + delta);
  lua_setfield(L, -2, ಕೀಲಿ);
}


ಸ್ಥಿರ ಶೂನ್ಯ setboolfield (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ) {
  ಯದಿ (ಮೌಲ್ಯ < 0)  /* undefined? */
    ಪ್ರತ್ಯಾಗಮ;  /* does not set field */
  lua_pushboolean(L, ಮೌಲ್ಯ);
  lua_setfield(L, -2, ಕೀಲಿ);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ಸ್ಥಿರ ಶೂನ್ಯ setallfields (lua_State *L, ಸಂರಚನಾ tm *stm) {
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


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getboolfield (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ) {
  ಪೂರ್ಣಾಙ್ಕ res;
  res = (lua_getfield(L, -1, ಕೀಲಿ) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_ತೆಗೆ(L, 1);
  ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getfield (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ d, ಪೂರ್ಣಾಙ್ಕ delta) {
  ಪೂರ್ಣಾಙ್ಕ isnum;
  ಪೂರ್ಣಾಙ್ಕ t = lua_getfield(L, -1, ಕೀಲಿ);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  ಯದಿ (!isnum) {  /* field is not an integer? */
    ಯದಿ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "field '%s' is not an integer", ಕೀಲಿ);
    ಅನ್ಯಥಾ ಯದಿ (l_unlikely(d < 0))  /* absent field; no default? */
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "field '%s' missing in date table", ಕೀಲಿ);
    res = d;
  }
  ಅನ್ಯಥಾ {
    ಯದಿ (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "field '%s' is out-of-bound", ಕೀಲಿ);
    res -= delta;
  }
  lua_ತೆಗೆ(L, 1);
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)res;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *checkoption (lua_State *L, ಧ್ರುವ ವರ್ಣ *conv,
                                size_t convlen, ವರ್ಣ *buff) {
  ಧ್ರುವ ವರ್ಣ *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  ಪ್ರತಿ (; *option != '\0' && oplen <= convlen; option += oplen) {
    ಯದಿ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    ಅನ್ಯಥಾ ಯದಿ (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ಪ್ರತ್ಯಾಗಮ conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ಪ್ರತ್ಯಾಗಮ conv;  /* to avoid warnings */
}


ಸ್ಥಿರ time_t l_checktime (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ಪ್ರತ್ಯಾಗಮ (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_date (lua_State *L) {
  size_t slen;
  ಧ್ರುವ ವರ್ಣ *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ರಿಕ್ತ));
  ಧ್ರುವ ವರ್ಣ *se = s + slen;  /* 's' end */
  ಸಂರಚನಾ tm tmr, *stm;
  ಯದಿ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  ಅನ್ಯಥಾ
    stm = l_localtime(&t, &tmr);
  ಯದಿ (stm == ರಿಕ್ತ)  /* invalid date? */
    ಪ್ರತ್ಯಾಗಮ luaL_error(L,
                 "date result cannot be represented in this installation");
  ಯದಿ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  ಅನ್ಯಥಾ {
    ವರ್ಣ cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    ಯಾವತ್ (s < se) {
      ಯದಿ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      ಅನ್ಯಥಾ {
        size_t reslen;
        ವರ್ಣ *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_time_aux (lua_State *L, time_t t, ಪೂರ್ಣಾಙ್ಕ err) {
  ಯದಿ (err || t != (time_t)(l_timet)t)
    ಪ್ರತ್ಯಾಗಮ luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_time (lua_State *L) {
  ಯದಿ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ರಿಕ್ತ);  /* get current time; error if it is -1 */
    ಪ್ರತ್ಯಾಗಮ os_time_aux(L, t, (t == (time_t)(-1)));
  }
  ಅನ್ಯಥಾ {
    time_t t;
    ಸಂರಚನಾ tm ts;
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
    ಪ್ರತ್ಯಾಗಮ os_time_aux(L, t, ts.tm_wday == -1);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)difftime(t1, t2));
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_setlocale (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ರಿಕ್ತ};
  ಧ್ರುವ ವರ್ಣ *l = luaL_optstring(L, 1, ರಿಕ್ತ);
  ಪೂರ್ಣಾಙ್ಕ op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ os_exit (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ಯದಿ (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  ಅನ್ಯಥಾ
    status = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, 1, EXIT_SUCCESS);
  ಯದಿ (lua_toboolean(L, 2))
    lua_close(L);
  ಯದಿ (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg syslib[] = {
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
  {ರಿಕ್ತ, ರಿಕ್ತ}
};

/* }====================================================== */



LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ಪ್ರತ್ಯಾಗಮ 1;
}

