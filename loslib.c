/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#నిర్వచించు loslib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <errno.h>
#అన్తర్భావ <locale.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#ఒకవేళ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#ఒకవేళ defined(LUA_USE_WINDOWS)
#నిర్వచించు LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#లేదావేళ defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#నిర్వచించు LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#లేకపోతే  /* C99 specification */
#నిర్వచించు LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#అయితే_ముగింపు

#అయితే_ముగింపు					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#ఒకవేళ !defined(LUA_NUMTIME)	/* { */

#నిర్వచించు l_timet			lua_Integer
#నిర్వచించు l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#నిర్వచించు l_gettime(L,arg)	luaL_checkinteger(L, arg)

#లేకపోతే				/* }{ */

#నిర్వచించు l_timet			lua_సంఖ్య
#నిర్వచించు l_pushtime(L,t)		lua_pushnumber(L,(lua_సంఖ్య)(t))
#నిర్వచించు l_gettime(L,arg)	luaL_checknumber(L, arg)

#అయితే_ముగింపు				/* } */


#ఒకవేళ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#ఒకవేళ defined(LUA_USE_POSIX)	/* { */

#నిర్వచించు l_gmtime(t,r)		gmtime_r(t,r)
#నిర్వచించు l_localtime(t,r)	localtime_r(t,r)

#లేకపోతే				/* }{ */

/* ISO C definitions */
#నిర్వచించు l_gmtime(t,r)		((శూన్య)(r)->tm_sec, gmtime(t))
#నిర్వచించు l_localtime(t,r)	((శూన్య)(r)->tm_sec, localtime(t))

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#ఒకవేళ !defined(lua_tmpnam)	/* { */

#ఒకవేళ defined(LUA_USE_POSIX)	/* { */

#అన్తర్భావ <unistd.h>

#నిర్వచించు LUA_TMPNAMBUFSIZE	32

#ఒకవేళ !defined(LUA_TMPNAMTEMPLATE)
#నిర్వచించు LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#అయితే_ముగింపు

#నిర్వచించు lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        ఒకవేళ (e != -1) close(e); \
        e = (e == -1); }

#లేకపోతే				/* }{ */

/* ISO C definitions */
#నిర్వచించు LUA_TMPNAMBUFSIZE	L_tmpnam
#నిర్వచించు lua_tmpnam(b,e)		{ e = (tmpnam(b) == ఖాళీ); }

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */
/* }================================================================== */


#ఒకవేళ !defined(l_system)
#ఒకవేళ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#నిర్వచించు l_system(cmd) ((cmd) == ఖాళీ ? 0 : -1)
#లేకపోతే
#నిర్వచించు l_system(cmd)	system(cmd)  /* default definition */
#అయితే_ముగింపు
#అయితే_ముగింపు


స్థిర పూర్ణాంకం os_execute (lua_State *L) {
  స్థిరరాశి అక్షరం *cmd = luaL_optstring(L, 1, ఖాళీ);
  పూర్ణాంకం stat;
  errno = 0;
  stat = l_system(cmd);
  ఒకవేళ (cmd != ఖాళీ)
    తిరిగి luaL_execresult(L, stat);
  లేకపోతే {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    తిరిగి 1;
  }
}


స్థిర పూర్ణాంకం os_remove (lua_State *L) {
  స్థిరరాశి అక్షరం *filename = luaL_checkstring(L, 1);
  errno = 0;
  తిరిగి luaL_fileresult(L, remove(filename) == 0, filename);
}


స్థిర పూర్ణాంకం os_rename (lua_State *L) {
  స్థిరరాశి అక్షరం *fromname = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *toname = luaL_checkstring(L, 2);
  errno = 0;
  తిరిగి luaL_fileresult(L, rename(fromname, toname) == 0, ఖాళీ);
}


స్థిర పూర్ణాంకం os_tmpname (lua_State *L) {
  అక్షరం buff[LUA_TMPNAMBUFSIZE];
  పూర్ణాంకం err;
  lua_tmpnam(buff, err);
  ఒకవేళ (l_unlikely(err))
    తిరిగి luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_సంఖ్య)clock())/(lua_సంఖ్య)CLOCKS_PER_SEC);
  తిరిగి 1;
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
స్థిర శూన్య setfield (lua_State *L, స్థిరరాశి అక్షరం *కీలకం, పూర్ణాంకం విలువ, పూర్ణాంకం delta) {
  #ఒకవేళ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= గరిష్ఠ_పూర్ణాంకం)
    ఒకవేళ (l_unlikely(విలువ > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", కీలకం);
  #అయితే_ముగింపు
  lua_pushinteger(L, (lua_Integer)విలువ + delta);
  lua_setfield(L, -2, కీలకం);
}


స్థిర శూన్య setboolfield (lua_State *L, స్థిరరాశి అక్షరం *కీలకం, పూర్ణాంకం విలువ) {
  ఒకవేళ (విలువ < 0)  /* undefined? */
    తిరిగి;  /* does not set field */
  lua_pushboolean(L, విలువ);
  lua_setfield(L, -2, కీలకం);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
స్థిర శూన్య setallfields (lua_State *L, నిర్మాణం tm *stm) {
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


స్థిర పూర్ణాంకం getboolfield (lua_State *L, స్థిరరాశి అక్షరం *కీలకం) {
  పూర్ణాంకం res;
  res = (lua_getfield(L, -1, కీలకం) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_తీయి(L, 1);
  తిరిగి res;
}


స్థిర పూర్ణాంకం getfield (lua_State *L, స్థిరరాశి అక్షరం *కీలకం, పూర్ణాంకం d, పూర్ణాంకం delta) {
  పూర్ణాంకం isnum;
  పూర్ణాంకం t = lua_getfield(L, -1, కీలకం);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  ఒకవేళ (!isnum) {  /* field is not an integer? */
    ఒకవేళ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      తిరిగి luaL_error(L, "field '%s' is not an integer", కీలకం);
    లేకపోతే ఒకవేళ (l_unlikely(d < 0))  /* absent field; no default? */
      తిరిగి luaL_error(L, "field '%s' missing in date table", కీలకం);
    res = d;
  }
  లేకపోతే {
    ఒకవేళ (!(res >= 0 ? res - delta <= గరిష్ఠ_పూర్ణాంకం : INT_MIN + delta <= res))
      తిరిగి luaL_error(L, "field '%s' is out-of-bound", కీలకం);
    res -= delta;
  }
  lua_తీయి(L, 1);
  తిరిగి (పూర్ణాంకం)res;
}


స్థిర స్థిరరాశి అక్షరం *checkoption (lua_State *L, స్థిరరాశి అక్షరం *conv,
                                size_t convlen, అక్షరం *buff) {
  స్థిరరాశి అక్షరం *option = LUA_STRFTIMEOPTIONS;
  గుర్తులేని oplen = 1;  /* length of options being checked */
  కొరకు (; *option != '\0' && oplen <= convlen; option += oplen) {
    ఒకవేళ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    లేకపోతే ఒకవేళ (memcmp(conv, option, oplen) == 0) {  /* match? */
      జ్ఞాపకం_నకలు(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      తిరిగి conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  తిరిగి conv;  /* to avoid warnings */
}


స్థిర time_t l_checktime (lua_State *L, పూర్ణాంకం arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  తిరిగి (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#నిర్వచించు SIZETIMEFMT	250


స్థిర పూర్ణాంకం os_date (lua_State *L) {
  size_t slen;
  స్థిరరాశి అక్షరం *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ఖాళీ));
  స్థిరరాశి అక్షరం *se = s + slen;  /* 's' end */
  నిర్మాణం tm tmr, *stm;
  ఒకవేళ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  లేకపోతే
    stm = l_localtime(&t, &tmr);
  ఒకవేళ (stm == ఖాళీ)  /* invalid date? */
    తిరిగి luaL_error(L,
                 "date result cannot be represented in this installation");
  ఒకవేళ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  లేకపోతే {
    అక్షరం cc[4];  /* buffer for individual conversion specifiers */
    lua_L_బఫరు b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    అయితే (s < se) {
      ఒకవేళ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      లేకపోతే {
        size_t reslen;
        అక్షరం *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం os_సమయం_aux (lua_State *L, time_t t, పూర్ణాంకం err) {
  ఒకవేళ (err || t != (time_t)(l_timet)t)
    తిరిగి luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం os_సమయం (lua_State *L) {
  ఒకవేళ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ఖాళీ);  /* get current time; error if it is -1 */
    తిరిగి os_సమయం_aux(L, t, (t == (time_t)(-1)));
  }
  లేకపోతే {
    time_t t;
    నిర్మాణం tm ts;
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
    తిరిగి os_సమయం_aux(L, t, ts.tm_wday == -1);
  }
}


స్థిర పూర్ణాంకం os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_సంఖ్య)difftime(t1, t2));
  తిరిగి 1;
}

/* }====================================================== */


స్థిర పూర్ణాంకం os_setlocale (lua_State *L) {
  స్థిర స్థిరరాశి పూర్ణాంకం cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ఖాళీ};
  స్థిరరాశి అక్షరం *l = luaL_optstring(L, 1, ఖాళీ);
  పూర్ణాంకం op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం os_exit (lua_State *L) {
  పూర్ణాంకం స్థితి_2;
  ఒకవేళ (lua_isboolean(L, 1))
    స్థితి_2 = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  లేకపోతే
    స్థితి_2 = (పూర్ణాంకం)luaL_optinteger(L, 1, EXIT_SUCCESS);
  ఒకవేళ (lua_toboolean(L, 2))
    lua_close(L);
  ఒకవేళ (L) నిష్క్రమించు(స్థితి_2);  /* 'if' to avoid warnings for unreachable 'return' */
  తిరిగి 0;
}


స్థిర స్థిరరాశి luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_సమయం},
  {"tmpname", os_tmpname},
  {ఖాళీ, ఖాళీ}
};

/* }====================================================== */



LUAMOD_API పూర్ణాంకం luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  తిరిగి 1;
}

