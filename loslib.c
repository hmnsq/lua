/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#എങ്കിൽ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#എങ്കിൽ defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#അല്ലെങ്കിലെങ്കിൽ defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#അല്ലെങ്കിൽ  /* C99 specification */
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
#എങ്കിൽ !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#അല്ലെങ്കിൽ				/* }{ */

#define l_timet			lua_സംഖ്യ
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_സംഖ്യ)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#എങ്കിൽ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#എങ്കിൽ defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#അല്ലെങ്കിൽ				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((ശൂന്യം)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((ശൂന്യം)(r)->tm_sec, localtime(t))

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
#എങ്കിൽ !defined(lua_tmpnam)	/* { */

#എങ്കിൽ defined(LUA_USE_POSIX)	/* { */

#അന്തര്ഭാവ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#എങ്കിൽ !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        എങ്കിൽ (e != -1) close(e); \
        e = (e == -1); }

#അല്ലെങ്കിൽ				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == ശൂന്യ); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#എങ്കിൽ !defined(l_system)
#എങ്കിൽ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == ശൂന്യ ? 0 : -1)
#അല്ലെങ്കിൽ
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


സ്ഥിതം പൂർണ്ണസംഖ്യ os_execute (lua_State *L) {
  സ്ഥിരം അക്ഷരം *cmd = luaL_optstring(L, 1, ശൂന്യ);
  പൂർണ്ണസംഖ്യ stat;
  errno = 0;
  stat = l_system(cmd);
  എങ്കിൽ (cmd != ശൂന്യ)
    തിരികെ luaL_execresult(L, stat);
  അല്ലെങ്കിൽ {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    തിരികെ 1;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_remove (lua_State *L) {
  സ്ഥിരം അക്ഷരം *filename = luaL_checkstring(L, 1);
  errno = 0;
  തിരികെ luaL_fileresult(L, remove(filename) == 0, filename);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_rename (lua_State *L) {
  സ്ഥിരം അക്ഷരം *fromname = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *toname = luaL_checkstring(L, 2);
  errno = 0;
  തിരികെ luaL_fileresult(L, rename(fromname, toname) == 0, ശൂന്യ);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_tmpname (lua_State *L) {
  അക്ഷരം buff[LUA_TMPNAMBUFSIZE];
  പൂർണ്ണസംഖ്യ err;
  lua_tmpnam(buff, err);
  എങ്കിൽ (l_unlikely(err))
    തിരികെ luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_സംഖ്യ)clock())/(lua_സംഖ്യ)CLOCKS_PER_SEC);
  തിരികെ 1;
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
സ്ഥിതം ശൂന്യം setfield (lua_State *L, സ്ഥിരം അക്ഷരം *താക്കോൽ, പൂർണ്ണസംഖ്യ മൂല്യം, പൂർണ്ണസംഖ്യ delta) {
  #എങ്കിൽ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    എങ്കിൽ (l_unlikely(മൂല്യം > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", താക്കോൽ);
  #endif
  lua_pushinteger(L, (lua_Integer)മൂല്യം + delta);
  lua_setfield(L, -2, താക്കോൽ);
}


സ്ഥിതം ശൂന്യം setboolfield (lua_State *L, സ്ഥിരം അക്ഷരം *താക്കോൽ, പൂർണ്ണസംഖ്യ മൂല്യം) {
  എങ്കിൽ (മൂല്യം < 0)  /* undefined? */
    തിരികെ;  /* does not set field */
  lua_pushboolean(L, മൂല്യം);
  lua_setfield(L, -2, താക്കോൽ);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
സ്ഥിതം ശൂന്യം setallfields (lua_State *L, ഘടന tm *stm) {
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


സ്ഥിതം പൂർണ്ണസംഖ്യ getboolfield (lua_State *L, സ്ഥിരം അക്ഷരം *താക്കോൽ) {
  പൂർണ്ണസംഖ്യ res;
  res = (lua_getfield(L, -1, താക്കോൽ) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_എടുക്കുക(L, 1);
  തിരികെ res;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ getfield (lua_State *L, സ്ഥിരം അക്ഷരം *താക്കോൽ, പൂർണ്ണസംഖ്യ d, പൂർണ്ണസംഖ്യ delta) {
  പൂർണ്ണസംഖ്യ isnum;
  പൂർണ്ണസംഖ്യ t = lua_getfield(L, -1, താക്കോൽ);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  എങ്കിൽ (!isnum) {  /* field is not an integer? */
    എങ്കിൽ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      തിരികെ luaL_error(L, "field '%s' is not an integer", താക്കോൽ);
    അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(d < 0))  /* absent field; no default? */
      തിരികെ luaL_error(L, "field '%s' missing in date table", താക്കോൽ);
    res = d;
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      തിരികെ luaL_error(L, "field '%s' is out-of-bound", താക്കോൽ);
    res -= delta;
  }
  lua_എടുക്കുക(L, 1);
  തിരികെ (പൂർണ്ണസംഖ്യ)res;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *checkoption (lua_State *L, സ്ഥിരം അക്ഷരം *conv,
                                size_t convlen, അക്ഷരം *buff) {
  സ്ഥിരം അക്ഷരം *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  വേണ്ടി (; *option != '\0' && oplen <= convlen; option += oplen) {
    എങ്കിൽ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    അല്ലെങ്കിൽ എങ്കിൽ (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      തിരികെ conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  തിരികെ conv;  /* to avoid warnings */
}


സ്ഥിതം time_t l_checktime (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  തിരികെ (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


സ്ഥിതം പൂർണ്ണസംഖ്യ os_date (lua_State *L) {
  size_t slen;
  സ്ഥിരം അക്ഷരം *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ശൂന്യ));
  സ്ഥിരം അക്ഷരം *se = s + slen;  /* 's' end */
  ഘടന tm tmr, *stm;
  എങ്കിൽ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  അല്ലെങ്കിൽ
    stm = l_localtime(&t, &tmr);
  എങ്കിൽ (stm == ശൂന്യ)  /* invalid date? */
    തിരികെ luaL_error(L,
                 "date result cannot be represented in this installation");
  എങ്കിൽ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  അല്ലെങ്കിൽ {
    അക്ഷരം cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    ആകുമ്പോൾ (s < se) {
      എങ്കിൽ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      അല്ലെങ്കിൽ {
        size_t reslen;
        അക്ഷരം *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_time_aux (lua_State *L, time_t t, പൂർണ്ണസംഖ്യ err) {
  എങ്കിൽ (err || t != (time_t)(l_timet)t)
    തിരികെ luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_time (lua_State *L) {
  എങ്കിൽ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ശൂന്യ);  /* get current time; error if it is -1 */
    തിരികെ os_time_aux(L, t, (t == (time_t)(-1)));
  }
  അല്ലെങ്കിൽ {
    time_t t;
    ഘടന tm ts;
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
    തിരികെ os_time_aux(L, t, ts.tm_wday == -1);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_സംഖ്യ)difftime(t1, t2));
  തിരികെ 1;
}

/* }====================================================== */


സ്ഥിതം പൂർണ്ണസംഖ്യ os_setlocale (lua_State *L) {
  സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ശൂന്യ};
  സ്ഥിരം അക്ഷരം *l = luaL_optstring(L, 1, ശൂന്യ);
  പൂർണ്ണസംഖ്യ op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ os_exit (lua_State *L) {
  പൂർണ്ണസംഖ്യ status;
  എങ്കിൽ (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  അല്ലെങ്കിൽ
    status = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, 1, EXIT_SUCCESS);
  എങ്കിൽ (lua_toboolean(L, 2))
    lua_close(L);
  എങ്കിൽ (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  തിരികെ 0;
}


സ്ഥിതം സ്ഥിരം luaL_Reg syslib[] = {
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
  {ശൂന്യ, ശൂന്യ}
};

/* }====================================================== */



LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  തിരികെ 1;
}

