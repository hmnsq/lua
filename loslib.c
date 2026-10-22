/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <locale.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <time.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lauxlib.h"
#অন্তর্ভাৱ "lualib.h"
#অন্তর্ভাৱ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#যদি !defined(LUA_STRFTIMEOPTIONS)	/* { */

#যদি defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#নোচেদ্যদি defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#অন্যথা  /* C99 specification */
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
#যদি !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#অন্যথা				/* }{ */

#define l_timet			lua_সংখ্যা
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_সংখ্যা)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#যদি !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#যদি defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#অন্যথা				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((শূন্য)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((শূন্য)(r)->tm_sec, localtime(t))

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
#যদি !defined(lua_tmpnam)	/* { */

#যদি defined(LUA_USE_POSIX)	/* { */

#অন্তর্ভাৱ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#যদি !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        যদি (e != -1) close(e); \
        e = (e == -1); }

#অন্যথা				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == রিক্ত); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#যদি !defined(l_system)
#যদি defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == রিক্ত ? 0 : -1)
#অন্যথা
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


স্থির পূর্ণাঙ্ক os_execute (lua_State *L) {
  ধ্রুৱ ৱর্ণ *cmd = luaL_optstring(L, 1, রিক্ত);
  পূর্ণাঙ্ক stat;
  errno = 0;
  stat = l_system(cmd);
  যদি (cmd != রিক্ত)
    ঘূৰাই luaL_execresult(L, stat);
  অন্যথা {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ঘূৰাই 1;
  }
}


স্থির পূর্ণাঙ্ক os_remove (lua_State *L) {
  ধ্রুৱ ৱর্ণ *filename = luaL_checkstring(L, 1);
  errno = 0;
  ঘূৰাই luaL_fileresult(L, remove(filename) == 0, filename);
}


স্থির পূর্ণাঙ্ক os_rename (lua_State *L) {
  ধ্রুৱ ৱর্ণ *fromname = luaL_checkstring(L, 1);
  ধ্রুৱ ৱর্ণ *toname = luaL_checkstring(L, 2);
  errno = 0;
  ঘূৰাই luaL_fileresult(L, rename(fromname, toname) == 0, রিক্ত);
}


স্থির পূর্ণাঙ্ক os_tmpname (lua_State *L) {
  ৱর্ণ buff[LUA_TMPNAMBUFSIZE];
  পূর্ণাঙ্ক err;
  lua_tmpnam(buff, err);
  যদি (l_unlikely(err))
    ঘূৰাই luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_সংখ্যা)clock())/(lua_সংখ্যা)CLOCKS_PER_SEC);
  ঘূৰাই 1;
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
স্থির শূন্য setfield (lua_State *L, ধ্রুৱ ৱর্ণ *চাবি, পূর্ণাঙ্ক মান, পূর্ণাঙ্ক delta) {
  #যদি (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    যদি (l_unlikely(মান > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", চাবি);
  #endif
  lua_pushinteger(L, (lua_Integer)মান + delta);
  lua_setfield(L, -2, চাবি);
}


স্থির শূন্য setboolfield (lua_State *L, ধ্রুৱ ৱর্ণ *চাবি, পূর্ণাঙ্ক মান) {
  যদি (মান < 0)  /* undefined? */
    ঘূৰাই;  /* does not set field */
  lua_pushboolean(L, মান);
  lua_setfield(L, -2, চাবি);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
স্থির শূন্য setallfields (lua_State *L, সংরচনা tm *stm) {
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


স্থির পূর্ণাঙ্ক getboolfield (lua_State *L, ধ্রুৱ ৱর্ণ *চাবি) {
  পূর্ণাঙ্ক res;
  res = (lua_getfield(L, -1, চাবি) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_বেৰ_কৰো(L, 1);
  ঘূৰাই res;
}


স্থির পূর্ণাঙ্ক getfield (lua_State *L, ধ্রুৱ ৱর্ণ *চাবি, পূর্ণাঙ্ক d, পূর্ণাঙ্ক delta) {
  পূর্ণাঙ্ক isnum;
  পূর্ণাঙ্ক t = lua_getfield(L, -1, চাবি);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  যদি (!isnum) {  /* field is not an integer? */
    যদি (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ঘূৰাই luaL_error(L, "field '%s' is not an integer", চাবি);
    অন্যথা যদি (l_unlikely(d < 0))  /* absent field; no default? */
      ঘূৰাই luaL_error(L, "field '%s' missing in date table", চাবি);
    res = d;
  }
  অন্যথা {
    যদি (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      ঘূৰাই luaL_error(L, "field '%s' is out-of-bound", চাবি);
    res -= delta;
  }
  lua_বেৰ_কৰো(L, 1);
  ঘূৰাই (পূর্ণাঙ্ক)res;
}


স্থির ধ্রুৱ ৱর্ণ *checkoption (lua_State *L, ধ্রুৱ ৱর্ণ *conv,
                                size_t convlen, ৱর্ণ *buff) {
  ধ্রুৱ ৱর্ণ *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  বাবে (; *option != '\0' && oplen <= convlen; option += oplen) {
    যদি (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    অন্যথা যদি (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ঘূৰাই conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ঘূৰাই conv;  /* to avoid warnings */
}


স্থির time_t l_checktime (lua_State *L, পূর্ণাঙ্ক arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ঘূৰাই (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


স্থির পূর্ণাঙ্ক os_date (lua_State *L) {
  size_t slen;
  ধ্রুৱ ৱর্ণ *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(রিক্ত));
  ধ্রুৱ ৱর্ণ *se = s + slen;  /* 's' end */
  সংরচনা tm tmr, *stm;
  যদি (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  অন্যথা
    stm = l_localtime(&t, &tmr);
  যদি (stm == রিক্ত)  /* invalid date? */
    ঘূৰাই luaL_error(L,
                 "date result cannot be represented in this installation");
  যদি (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  অন্যথা {
    ৱর্ণ cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    যেতিয়ালৈ (s < se) {
      যদি (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      অন্যথা {
        size_t reslen;
        ৱর্ণ *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক os_time_aux (lua_State *L, time_t t, পূর্ণাঙ্ক err) {
  যদি (err || t != (time_t)(l_timet)t)
    ঘূৰাই luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক os_time (lua_State *L) {
  যদি (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(রিক্ত);  /* get current time; error if it is -1 */
    ঘূৰাই os_time_aux(L, t, (t == (time_t)(-1)));
  }
  অন্যথা {
    time_t t;
    সংরচনা tm ts;
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
    ঘূৰাই os_time_aux(L, t, ts.tm_wday == -1);
  }
}


স্থির পূর্ণাঙ্ক os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_সংখ্যা)difftime(t1, t2));
  ঘূৰাই 1;
}

/* }====================================================== */


স্থির পূর্ণাঙ্ক os_setlocale (lua_State *L) {
  স্থির ধ্রুৱ পূর্ণাঙ্ক cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", রিক্ত};
  ধ্রুৱ ৱর্ণ *l = luaL_optstring(L, 1, রিক্ত);
  পূর্ণাঙ্ক op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক os_exit (lua_State *L) {
  পূর্ণাঙ্ক status;
  যদি (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  অন্যথা
    status = (পূর্ণাঙ্ক)luaL_optinteger(L, 1, EXIT_SUCCESS);
  যদি (lua_toboolean(L, 2))
    lua_close(L);
  যদি (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  ঘূৰাই 0;
}


স্থির ধ্রুৱ luaL_Reg syslib[] = {
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
  {রিক্ত, রিক্ত}
};

/* }====================================================== */



LUAMOD_API পূর্ণাঙ্ক luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ঘূৰাই 1;
}

