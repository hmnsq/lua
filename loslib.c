/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#ଯଦି !defined(LUA_STRFTIMEOPTIONS)	/* { */

#ଯଦି defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#ନଚେତଯଦି defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#ଅନ୍ୟଥା  /* C99 specification */
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
#ଯଦି !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#ଅନ୍ୟଥା				/* }{ */

#define l_timet			lua_ସଂଖ୍ୟା
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_ସଂଖ୍ୟା)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#ଯଦି !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#ଯଦି defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#ଅନ୍ୟଥା				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((ଶୂନ୍ୟ)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((ଶୂନ୍ୟ)(r)->tm_sec, localtime(t))

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
#ଯଦି !defined(lua_tmpnam)	/* { */

#ଯଦି defined(LUA_USE_POSIX)	/* { */

#ଅନ୍ତର୍ଭାଵ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#ଯଦି !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        ଯଦି (e != -1) close(e); \
        e = (e == -1); }

#ଅନ୍ୟଥା				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == ଖାଲି); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#ଯଦି !defined(l_system)
#ଯଦି defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == ଖାଲି ? 0 : -1)
#ଅନ୍ୟଥା
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_execute (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *cmd = luaL_optstring(L, 1, ଖାଲି);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat;
  errno = 0;
  stat = l_system(cmd);
  ଯଦି (cmd != ଖାଲି)
    ଫେରନ୍ତୁ luaL_execresult(L, stat);
  ଅନ୍ୟଥା {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ଫେରନ୍ତୁ 1;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_remove (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *filename = luaL_checkstring(L, 1);
  errno = 0;
  ଫେରନ୍ତୁ luaL_fileresult(L, remove(filename) == 0, filename);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_rename (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *fromname = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *toname = luaL_checkstring(L, 2);
  errno = 0;
  ଫେରନ୍ତୁ luaL_fileresult(L, rename(fromname, toname) == 0, ଖାଲି);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_tmpname (lua_State *L) {
  ଅକ୍ଷର buff[LUA_TMPNAMBUFSIZE];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
  lua_tmpnam(buff, err);
  ଯଦି (l_unlikely(err))
    ଫେରନ୍ତୁ luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_ସଂଖ୍ୟା)clock())/(lua_ସଂଖ୍ୟା)CLOCKS_PER_SEC);
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର ଶୂନ୍ୟ setfield (lua_State *L, ଅଚଳ ଅକ୍ଷର *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta) {
  #ଯଦି (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    ଯଦି (l_unlikely(ମୂଲ୍ୟ > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", ଚାବି);
  #endif
  lua_pushinteger(L, (lua_Integer)ମୂଲ୍ୟ + delta);
  lua_setfield(L, -2, ଚାବି);
}


ସ୍ଥିର ଶୂନ୍ୟ setboolfield (lua_State *L, ଅଚଳ ଅକ୍ଷର *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ) {
  ଯଦି (ମୂଲ୍ୟ < 0)  /* undefined? */
    ଫେରନ୍ତୁ;  /* does not set field */
  lua_pushboolean(L, ମୂଲ୍ୟ);
  lua_setfield(L, -2, ଚାବି);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ସ୍ଥିର ଶୂନ୍ୟ setallfields (lua_State *L, ସଂରଚନା tm *stm) {
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


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getboolfield (lua_State *L, ଅଚଳ ଅକ୍ଷର *ଚାବି) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  res = (lua_getfield(L, -1, ଚାବି) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_କାଢ଼(L, 1);
  ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getfield (lua_State *L, ଅଚଳ ଅକ୍ଷର *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା d, ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = lua_getfield(L, -1, ଚାବି);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  ଯଦି (!isnum) {  /* field is not an integer? */
    ଯଦି (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ଫେରନ୍ତୁ luaL_error(L, "field '%s' is not an integer", ଚାବି);
    ଅନ୍ୟଥା ଯଦି (l_unlikely(d < 0))  /* absent field; no default? */
      ଫେରନ୍ତୁ luaL_error(L, "field '%s' missing in date table", ଚାବି);
    res = d;
  }
  ଅନ୍ୟଥା {
    ଯଦି (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      ଫେରନ୍ତୁ luaL_error(L, "field '%s' is out-of-bound", ଚାବି);
    res -= delta;
  }
  lua_କାଢ଼(L, 1);
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)res;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *checkoption (lua_State *L, ଅଚଳ ଅକ୍ଷର *conv,
                                size_t convlen, ଅକ୍ଷର *buff) {
  ଅଚଳ ଅକ୍ଷର *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  ପାଇଁ (; *option != '\0' && oplen <= convlen; option += oplen) {
    ଯଦି (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    ଅନ୍ୟଥା ଯଦି (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ଫେରନ୍ତୁ conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ଫେରନ୍ତୁ conv;  /* to avoid warnings */
}


ସ୍ଥିର time_t l_checktime (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ଫେରନ୍ତୁ (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_date (lua_State *L) {
  size_t slen;
  ଅଚଳ ଅକ୍ଷର *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ଖାଲି));
  ଅଚଳ ଅକ୍ଷର *se = s + slen;  /* 's' end */
  ସଂରଚନା tm tmr, *stm;
  ଯଦି (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  ଅନ୍ୟଥା
    stm = l_localtime(&t, &tmr);
  ଯଦି (stm == ଖାଲି)  /* invalid date? */
    ଫେରନ୍ତୁ luaL_error(L,
                 "date result cannot be represented in this installation");
  ଯଦି (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  ଅନ୍ୟଥା {
    ଅକ୍ଷର cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    ଯେତେବେଳେ (s < se) {
      ଯଦି (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      ଅନ୍ୟଥା {
        size_t reslen;
        ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_time_aux (lua_State *L, time_t t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା err) {
  ଯଦି (err || t != (time_t)(l_timet)t)
    ଫେରନ୍ତୁ luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_time (lua_State *L) {
  ଯଦି (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ଖାଲି);  /* get current time; error if it is -1 */
    ଫେରନ୍ତୁ os_time_aux(L, t, (t == (time_t)(-1)));
  }
  ଅନ୍ୟଥା {
    time_t t;
    ସଂରଚନା tm ts;
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
    ଫେରନ୍ତୁ os_time_aux(L, t, ts.tm_wday == -1);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_ସଂଖ୍ୟା)difftime(t1, t2));
  ଫେରନ୍ତୁ 1;
}

/* }====================================================== */


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_setlocale (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ଖାଲି};
  ଅଚଳ ଅକ୍ଷର *l = luaL_optstring(L, 1, ଖାଲି);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_exit (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ଯଦି (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  ଅନ୍ୟଥା
    status = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, 1, EXIT_SUCCESS);
  ଯଦି (lua_toboolean(L, 2))
    lua_close(L);
  ଯଦି (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ଅଚଳ luaL_Reg syslib[] = {
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
  {ଖାଲି, ଖାଲି}
};

/* }====================================================== */



LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ଫେରନ୍ତୁ 1;
}

