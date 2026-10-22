/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <errno.h>
#अन्तर्भाव <locale.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#जर !defined(LUA_STRFTIMEOPTIONS)	/* { */

#जर defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#किंवाजर defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#अन्यथा  /* C99 specification */
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
#जर !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#अन्यथा				/* }{ */

#define l_timet			lua_संख्या
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_संख्या)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#जर !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#जर defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#अन्यथा				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((शून्य)(r)->tm_sec, स_जमट(t))
#define l_localtime(t,r)	((शून्य)(r)->tm_sec, स_स्थानीय(t))

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
#जर !defined(lua_tmpnam)	/* { */

#जर defined(LUA_USE_POSIX)	/* { */

#अन्तर्भाव <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#जर !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        म_नकल(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        जर (e != -1) बंद(e); \
        e = (e == -1); }

#अन्यथा				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	ब_क्षणिक
#define lua_tmpnam(b,e)		{ e = (क्षणिक(b) == रिक्त); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#जर !defined(l_system)
#जर defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == रिक्त ? 0 : -1)
#अन्यथा
#define l_system(cmd)	प्रणाली(cmd)  /* default definition */
#endif
#endif


स्थिर पूर्णांक os_execute (lua_State *L) {
  स्थिरांक अक्षर *cmd = luaL_optstring(L, 1, रिक्त);
  पूर्णांक stat;
  त्रुटि_सं = 0;
  stat = l_system(cmd);
  जर (cmd != रिक्त)
    परत luaL_execresult(L, stat);
  अन्यथा {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    परत 1;
  }
}


स्थिर पूर्णांक os_remove (lua_State *L) {
  स्थिरांक अक्षर *filename = luaL_checkstring(L, 1);
  त्रुटि_सं = 0;
  परत luaL_fileresult(L, हटाओ(filename) == 0, filename);
}


स्थिर पूर्णांक os_rename (lua_State *L) {
  स्थिरांक अक्षर *fromname = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *toname = luaL_checkstring(L, 2);
  त्रुटि_सं = 0;
  परत luaL_fileresult(L, नाम(fromname, toname) == 0, रिक्त);
}


स्थिर पूर्णांक os_tmpname (lua_State *L) {
  अक्षर buff[LUA_TMPNAMBUFSIZE];
  पूर्णांक err;
  lua_tmpnam(buff, err);
  जर (l_unlikely(err))
    परत luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  परत 1;
}


स्थिर पूर्णांक os_getenv (lua_State *L) {
  lua_pushstring(L, दो_पर्या(luaL_checkstring(L, 1)));  /* if NULL push nil */
  परत 1;
}


स्थिर पूर्णांक os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_संख्या)घड़ी())/(lua_संख्या)CLOCKS_PER_SEC);
  परत 1;
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
स्थिर शून्य setfield (lua_State *L, स्थिरांक अक्षर *किल्ली, पूर्णांक मूल्य, पूर्णांक delta) {
  #जर (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= पूर्णांक_उच्च)
    जर (l_unlikely(मूल्य > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", किल्ली);
  #endif
  lua_pushinteger(L, (lua_Integer)मूल्य + delta);
  lua_setfield(L, -2, किल्ली);
}


स्थिर शून्य setboolfield (lua_State *L, स्थिरांक अक्षर *किल्ली, पूर्णांक मूल्य) {
  जर (मूल्य < 0)  /* undefined? */
    परत;  /* does not set field */
  lua_pushboolean(L, मूल्य);
  lua_setfield(L, -2, किल्ली);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
स्थिर शून्य setallfields (lua_State *L, संरचना पंचांग *stm) {
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


स्थिर पूर्णांक getboolfield (lua_State *L, स्थिरांक अक्षर *किल्ली) {
  पूर्णांक res;
  res = (lua_getfield(L, -1, किल्ली) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_काढा(L, 1);
  परत res;
}


स्थिर पूर्णांक getfield (lua_State *L, स्थिरांक अक्षर *किल्ली, पूर्णांक d, पूर्णांक delta) {
  पूर्णांक isnum;
  पूर्णांक t = lua_getfield(L, -1, किल्ली);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  जर (!isnum) {  /* field is not an integer? */
    जर (l_unlikely(t != LUA_TNIL))  /* some other value? */
      परत luaL_error(L, "field '%s' is not an integer", किल्ली);
    अन्यथा जर (l_unlikely(d < 0))  /* absent field; no default? */
      परत luaL_error(L, "field '%s' missing in date table", किल्ली);
    res = d;
  }
  अन्यथा {
    जर (!(res >= 0 ? res - delta <= पूर्णांक_उच्च : पूर्णांक_न्यून + delta <= res))
      परत luaL_error(L, "field '%s' is out-of-bound", किल्ली);
    res -= delta;
  }
  lua_काढा(L, 1);
  परत (पूर्णांक)res;
}


स्थिर स्थिरांक अक्षर *checkoption (lua_State *L, स्थिरांक अक्षर *conv,
                                माप_प्रकार convlen, अक्षर *buff) {
  स्थिरांक अक्षर *option = LUA_STRFTIMEOPTIONS;
  अचिन्हित oplen = 1;  /* length of options being checked */
  साठी (; *option != '\0' && oplen <= convlen; option += oplen) {
    जर (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    अन्यथा जर (स_भेद(conv, option, oplen) == 0) {  /* match? */
      स_नकल(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      परत conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  परत conv;  /* to avoid warnings */
}


स्थिर समय_प्रकार l_checktime (lua_State *L, पूर्णांक arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (समय_प्रकार)t == t, arg, "time out-of-bounds");
  परत (समय_प्रकार)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


स्थिर पूर्णांक os_date (lua_State *L) {
  माप_प्रकार slen;
  स्थिरांक अक्षर *s = luaL_optlstring(L, 1, "%c", &slen);
  समय_प्रकार t = luaL_opt(L, l_checktime, 2, समय(रिक्त));
  स्थिरांक अक्षर *se = s + slen;  /* 's' end */
  संरचना पंचांग tmr, *stm;
  जर (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  अन्यथा
    stm = l_localtime(&t, &tmr);
  जर (stm == रिक्त)  /* invalid date? */
    परत luaL_error(L,
                 "date result cannot be represented in this installation");
  जर (म_भेद(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  अन्यथा {
    अक्षर cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    पर्यंत (s < se) {
      जर (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      अन्यथा {
        माप_प्रकार reslen;
        अक्षर *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = स_माला(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  परत 1;
}


स्थिर पूर्णांक os_time_aux (lua_State *L, समय_प्रकार t, पूर्णांक err) {
  जर (err || t != (समय_प्रकार)(l_timet)t)
    परत luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  परत 1;
}


स्थिर पूर्णांक os_time (lua_State *L) {
  जर (lua_isnoneornil(L, 1)) {  /* called without args? */
    समय_प्रकार t = समय(रिक्त);  /* get current time; error if it is -1 */
    परत os_time_aux(L, t, (t == (समय_प्रकार)(-1)));
  }
  अन्यथा {
    समय_प्रकार t;
    संरचना पंचांग ts;
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
    t = स_गढ़ो(&ts);
    setallfields(L, &ts);  /* update fields with normalized values */
    /* error if tm_wday was not set */
    परत os_time_aux(L, t, ts.tm_wday == -1);
  }
}


स्थिर पूर्णांक os_difftime (lua_State *L) {
  समय_प्रकार t1 = l_checktime(L, 1);
  समय_प्रकार t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_संख्या)स_अंतर(t1, t2));
  परत 1;
}

/* }====================================================== */


स्थिर पूर्णांक os_setlocale (lua_State *L) {
  स्थिर स्थिरांक पूर्णांक cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  स्थिर स्थिरांक अक्षर *स्थिरांक catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", रिक्त};
  स्थिरांक अक्षर *l = luaL_optstring(L, 1, रिक्त);
  पूर्णांक op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, रखो_क्षेत्र(cat[op], l));
  परत 1;
}


स्थिर पूर्णांक os_exit (lua_State *L) {
  पूर्णांक status;
  जर (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? निकास_सफल : निकास_त्रुटि);
  अन्यथा
    status = (पूर्णांक)luaL_optinteger(L, 1, निकास_सफल);
  जर (lua_toboolean(L, 2))
    lua_close(L);
  जर (L) निकास(status);  /* 'if' to avoid warnings for unreachable 'return' */
  परत 0;
}


स्थिर स्थिरांक luaL_Reg syslib[] = {
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
  {रिक्त, रिक्त}
};

/* }====================================================== */



LUAMOD_API पूर्णांक luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  परत 1;
}

