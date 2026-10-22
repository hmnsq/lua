/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#எனில் !defined(LUA_STRFTIMEOPTIONS)	/* { */

#எனில் defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#அல்லதெனில் defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#இல்லையெனில்  /* C99 specification */
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
#எனில் !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#இல்லையெனில்				/* }{ */

#define l_timet			lua_எண்
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_எண்)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#எனில் !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#எனில் defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#இல்லையெனில்				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((வெறுமை)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((வெறுமை)(r)->tm_sec, localtime(t))

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
#எனில் !defined(lua_tmpnam)	/* { */

#எனில் defined(LUA_USE_POSIX)	/* { */

#அந்தர்பாவ <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#எனில் !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        எனில் (e != -1) close(e); \
        e = (e == -1); }

#இல்லையெனில்				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == வெற்று); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#எனில் !defined(l_system)
#எனில் defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == வெற்று ? 0 : -1)
#இல்லையெனில்
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


நிலையான முழு os_execute (lua_State *L) {
  மாறிலி எழுத்து *cmd = luaL_optstring(L, 1, வெற்று);
  முழு stat;
  errno = 0;
  stat = l_system(cmd);
  எனில் (cmd != வெற்று)
    திரும்பு luaL_execresult(L, stat);
  இல்லையெனில் {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    திரும்பு 1;
  }
}


நிலையான முழு os_remove (lua_State *L) {
  மாறிலி எழுத்து *filename = luaL_checkstring(L, 1);
  errno = 0;
  திரும்பு luaL_fileresult(L, remove(filename) == 0, filename);
}


நிலையான முழு os_rename (lua_State *L) {
  மாறிலி எழுத்து *fromname = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *toname = luaL_checkstring(L, 2);
  errno = 0;
  திரும்பு luaL_fileresult(L, rename(fromname, toname) == 0, வெற்று);
}


நிலையான முழு os_tmpname (lua_State *L) {
  எழுத்து buff[LUA_TMPNAMBUFSIZE];
  முழு err;
  lua_tmpnam(buff, err);
  எனில் (l_unlikely(err))
    திரும்பு luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  திரும்பு 1;
}


நிலையான முழு os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  திரும்பு 1;
}


நிலையான முழு os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_எண்)clock())/(lua_எண்)CLOCKS_PER_SEC);
  திரும்பு 1;
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
நிலையான வெறுமை setfield (lua_State *L, மாறிலி எழுத்து *சாவி, முழு மதிப்பு, முழு delta) {
  #எனில் (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    எனில் (l_unlikely(மதிப்பு > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", சாவி);
  #endif
  lua_pushinteger(L, (lua_Integer)மதிப்பு + delta);
  lua_setfield(L, -2, சாவி);
}


நிலையான வெறுமை setboolfield (lua_State *L, மாறிலி எழுத்து *சாவி, முழு மதிப்பு) {
  எனில் (மதிப்பு < 0)  /* undefined? */
    திரும்பு;  /* does not set field */
  lua_pushboolean(L, மதிப்பு);
  lua_setfield(L, -2, சாவி);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
நிலையான வெறுமை setallfields (lua_State *L, கட்டமைப்பு tm *stm) {
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


நிலையான முழு getboolfield (lua_State *L, மாறிலி எழுத்து *சாவி) {
  முழு res;
  res = (lua_getfield(L, -1, சாவி) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_எடு(L, 1);
  திரும்பு res;
}


நிலையான முழு getfield (lua_State *L, மாறிலி எழுத்து *சாவி, முழு d, முழு delta) {
  முழு isnum;
  முழு t = lua_getfield(L, -1, சாவி);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  எனில் (!isnum) {  /* field is not an integer? */
    எனில் (l_unlikely(t != LUA_TNIL))  /* some other value? */
      திரும்பு luaL_error(L, "field '%s' is not an integer", சாவி);
    இல்லையெனில் எனில் (l_unlikely(d < 0))  /* absent field; no default? */
      திரும்பு luaL_error(L, "field '%s' missing in date table", சாவி);
    res = d;
  }
  இல்லையெனில் {
    எனில் (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      திரும்பு luaL_error(L, "field '%s' is out-of-bound", சாவி);
    res -= delta;
  }
  lua_எடு(L, 1);
  திரும்பு (முழு)res;
}


நிலையான மாறிலி எழுத்து *checkoption (lua_State *L, மாறிலி எழுத்து *conv,
                                size_t convlen, எழுத்து *buff) {
  மாறிலி எழுத்து *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  க்கு (; *option != '\0' && oplen <= convlen; option += oplen) {
    எனில் (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    இல்லையெனில் எனில் (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      திரும்பு conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  திரும்பு conv;  /* to avoid warnings */
}


நிலையான time_t l_checktime (lua_State *L, முழு arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  திரும்பு (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


நிலையான முழு os_date (lua_State *L) {
  size_t slen;
  மாறிலி எழுத்து *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(வெற்று));
  மாறிலி எழுத்து *se = s + slen;  /* 's' end */
  கட்டமைப்பு tm tmr, *stm;
  எனில் (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  இல்லையெனில்
    stm = l_localtime(&t, &tmr);
  எனில் (stm == வெற்று)  /* invalid date? */
    திரும்பு luaL_error(L,
                 "date result cannot be represented in this installation");
  எனில் (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  இல்லையெனில் {
    எழுத்து cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    போது (s < se) {
      எனில் (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      இல்லையெனில் {
        size_t reslen;
        எழுத்து *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  திரும்பு 1;
}


நிலையான முழு os_time_aux (lua_State *L, time_t t, முழு err) {
  எனில் (err || t != (time_t)(l_timet)t)
    திரும்பு luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  திரும்பு 1;
}


நிலையான முழு os_time (lua_State *L) {
  எனில் (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(வெற்று);  /* get current time; error if it is -1 */
    திரும்பு os_time_aux(L, t, (t == (time_t)(-1)));
  }
  இல்லையெனில் {
    time_t t;
    கட்டமைப்பு tm ts;
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
    திரும்பு os_time_aux(L, t, ts.tm_wday == -1);
  }
}


நிலையான முழு os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_எண்)difftime(t1, t2));
  திரும்பு 1;
}

/* }====================================================== */


நிலையான முழு os_setlocale (lua_State *L) {
  நிலையான மாறிலி முழு cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  நிலையான மாறிலி எழுத்து *மாறிலி catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", வெற்று};
  மாறிலி எழுத்து *l = luaL_optstring(L, 1, வெற்று);
  முழு op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  திரும்பு 1;
}


நிலையான முழு os_exit (lua_State *L) {
  முழு status;
  எனில் (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  இல்லையெனில்
    status = (முழு)luaL_optinteger(L, 1, EXIT_SUCCESS);
  எனில் (lua_toboolean(L, 2))
    lua_close(L);
  எனில் (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  திரும்பு 0;
}


நிலையான மாறிலி luaL_Reg syslib[] = {
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
  {வெற்று, வெற்று}
};

/* }====================================================== */



LUAMOD_API முழு luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  திரும்பு 1;
}

