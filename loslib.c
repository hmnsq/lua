/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#જો !defined(LUA_STRFTIMEOPTIONS)	/* { */

#જો defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#અથવાજો defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#નહીંતર  /* C99 specification */
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
#જો !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#નહીંતર				/* }{ */

#define l_timet			lua_સંખ્યા
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_સંખ્યા)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#જો !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#જો defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#નહીંતર				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((શૂન્ય)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((શૂન્ય)(r)->tm_sec, localtime(t))

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
#જો !defined(lua_tmpnam)	/* { */

#જો defined(LUA_USE_POSIX)	/* { */

#અન્તર્ભાવ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#જો !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        જો (e != -1) close(e); \
        e = (e == -1); }

#નહીંતર				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == ખાલી); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#જો !defined(l_system)
#જો defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == ખાલી ? 0 : -1)
#નહીંતર
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


સ્થિર પૂર્ણાંક os_execute (lua_State *L) {
  અચલ અક્ષર *cmd = luaL_optstring(L, 1, ખાલી);
  પૂર્ણાંક stat;
  errno = 0;
  stat = l_system(cmd);
  જો (cmd != ખાલી)
    પાછા luaL_execresult(L, stat);
  નહીંતર {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    પાછા 1;
  }
}


સ્થિર પૂર્ણાંક os_remove (lua_State *L) {
  અચલ અક્ષર *filename = luaL_checkstring(L, 1);
  errno = 0;
  પાછા luaL_fileresult(L, remove(filename) == 0, filename);
}


સ્થિર પૂર્ણાંક os_rename (lua_State *L) {
  અચલ અક્ષર *fromname = luaL_checkstring(L, 1);
  અચલ અક્ષર *toname = luaL_checkstring(L, 2);
  errno = 0;
  પાછા luaL_fileresult(L, rename(fromname, toname) == 0, ખાલી);
}


સ્થિર પૂર્ણાંક os_tmpname (lua_State *L) {
  અક્ષર buff[LUA_TMPNAMBUFSIZE];
  પૂર્ણાંક err;
  lua_tmpnam(buff, err);
  જો (l_unlikely(err))
    પાછા luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  પાછા 1;
}


સ્થિર પૂર્ણાંક os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  પાછા 1;
}


સ્થિર પૂર્ણાંક os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_સંખ્યા)clock())/(lua_સંખ્યા)CLOCKS_PER_SEC);
  પાછા 1;
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
સ્થિર શૂન્ય setfield (lua_State *L, અચલ અક્ષર *ચાવી, પૂર્ણાંક કિંમત, પૂર્ણાંક delta) {
  #જો (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    જો (l_unlikely(કિંમત > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", ચાવી);
  #endif
  lua_pushinteger(L, (lua_Integer)કિંમત + delta);
  lua_setfield(L, -2, ચાવી);
}


સ્થિર શૂન્ય setboolfield (lua_State *L, અચલ અક્ષર *ચાવી, પૂર્ણાંક કિંમત) {
  જો (કિંમત < 0)  /* undefined? */
    પાછા;  /* does not set field */
  lua_pushboolean(L, કિંમત);
  lua_setfield(L, -2, ચાવી);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
સ્થિર શૂન્ય setallfields (lua_State *L, સંરચના tm *stm) {
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


સ્થિર પૂર્ણાંક getboolfield (lua_State *L, અચલ અક્ષર *ચાવી) {
  પૂર્ણાંક res;
  res = (lua_getfield(L, -1, ચાવી) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_કાઢો(L, 1);
  પાછા res;
}


સ્થિર પૂર્ણાંક getfield (lua_State *L, અચલ અક્ષર *ચાવી, પૂર્ણાંક d, પૂર્ણાંક delta) {
  પૂર્ણાંક isnum;
  પૂર્ણાંક t = lua_getfield(L, -1, ચાવી);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  જો (!isnum) {  /* field is not an integer? */
    જો (l_unlikely(t != LUA_TNIL))  /* some other value? */
      પાછા luaL_error(L, "field '%s' is not an integer", ચાવી);
    નહીંતર જો (l_unlikely(d < 0))  /* absent field; no default? */
      પાછા luaL_error(L, "field '%s' missing in date table", ચાવી);
    res = d;
  }
  નહીંતર {
    જો (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      પાછા luaL_error(L, "field '%s' is out-of-bound", ચાવી);
    res -= delta;
  }
  lua_કાઢો(L, 1);
  પાછા (પૂર્ણાંક)res;
}


સ્થિર અચલ અક્ષર *checkoption (lua_State *L, અચલ અક્ષર *conv,
                                size_t convlen, અક્ષર *buff) {
  અચલ અક્ષર *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  માટે (; *option != '\0' && oplen <= convlen; option += oplen) {
    જો (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    નહીંતર જો (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      પાછા conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  પાછા conv;  /* to avoid warnings */
}


સ્થિર time_t l_checktime (lua_State *L, પૂર્ણાંક arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  પાછા (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


સ્થિર પૂર્ણાંક os_date (lua_State *L) {
  size_t slen;
  અચલ અક્ષર *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ખાલી));
  અચલ અક્ષર *se = s + slen;  /* 's' end */
  સંરચના tm tmr, *stm;
  જો (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  નહીંતર
    stm = l_localtime(&t, &tmr);
  જો (stm == ખાલી)  /* invalid date? */
    પાછા luaL_error(L,
                 "date result cannot be represented in this installation");
  જો (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  નહીંતર {
    અક્ષર cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    જ્યારે (s < se) {
      જો (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      નહીંતર {
        size_t reslen;
        અક્ષર *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક os_time_aux (lua_State *L, time_t t, પૂર્ણાંક err) {
  જો (err || t != (time_t)(l_timet)t)
    પાછા luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  પાછા 1;
}


સ્થિર પૂર્ણાંક os_time (lua_State *L) {
  જો (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ખાલી);  /* get current time; error if it is -1 */
    પાછા os_time_aux(L, t, (t == (time_t)(-1)));
  }
  નહીંતર {
    time_t t;
    સંરચના tm ts;
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
    પાછા os_time_aux(L, t, ts.tm_wday == -1);
  }
}


સ્થિર પૂર્ણાંક os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_સંખ્યા)difftime(t1, t2));
  પાછા 1;
}

/* }====================================================== */


સ્થિર પૂર્ણાંક os_setlocale (lua_State *L) {
  સ્થિર અચલ પૂર્ણાંક cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  સ્થિર અચલ અક્ષર *અચલ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ખાલી};
  અચલ અક્ષર *l = luaL_optstring(L, 1, ખાલી);
  પૂર્ણાંક op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  પાછા 1;
}


સ્થિર પૂર્ણાંક os_exit (lua_State *L) {
  પૂર્ણાંક status;
  જો (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  નહીંતર
    status = (પૂર્ણાંક)luaL_optinteger(L, 1, EXIT_SUCCESS);
  જો (lua_toboolean(L, 2))
    lua_close(L);
  જો (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  પાછા 0;
}


સ્થિર અચલ luaL_Reg syslib[] = {
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
  {ખાલી, ખાલી}
};

/* }====================================================== */



LUAMOD_API પૂર્ણાંક luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  પાછા 1;
}

