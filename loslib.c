/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#परिभाषय loslib_c
#परिभाषय LUA_LIB

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
#यदि !defined(LUA_STRFTIMEOPTIONS)	/* { */

#यदि defined(LUA_USE_WINDOWS)
#परिभाषय LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#नोचेद्यदि defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#परिभाषय LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#अन्यथा  /* C99 specification */
#परिभाषय LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#यदि_अन्तः

#यदि_अन्तः					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#यदि !defined(LUA_NUMTIME)	/* { */

#परिभाषय l_timet			lua_Integer
#परिभाषय l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#परिभाषय l_gettime(L,arg)	luaL_checkinteger(L, arg)

#अन्यथा				/* }{ */

#परिभाषय l_timet			lua_सङ्ख्या
#परिभाषय l_pushtime(L,t)		lua_pushnumber(L,(lua_सङ्ख्या)(t))
#परिभाषय l_gettime(L,arg)	luaL_checknumber(L, arg)

#यदि_अन्तः				/* } */


#यदि !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#यदि defined(LUA_USE_POSIX)	/* { */

#परिभाषय l_gmtime(t,r)		gmtime_r(t,r)
#परिभाषय l_localtime(t,r)	localtime_r(t,r)

#अन्यथा				/* }{ */

/* ISO C definitions */
#परिभाषय l_gmtime(t,r)		((शून्य)(r)->tm_sec, gmtime(t))
#परिभाषय l_localtime(t,r)	((शून्य)(r)->tm_sec, localtime(t))

#यदि_अन्तः				/* } */

#यदि_अन्तः				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#यदि !defined(lua_tmpnam)	/* { */

#यदि defined(LUA_USE_POSIX)	/* { */

#अन्तर्भाव <unistd.h>

#परिभाषय LUA_TMPNAMBUFSIZE	32

#यदि !defined(LUA_TMPNAMTEMPLATE)
#परिभाषय LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#यदि_अन्तः

#परिभाषय lua_tmpnam(b,e) { \
        वाक्यं_प्रतिलिख(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        यदि (e != -1) close(e); \
        e = (e == -1); }

#अन्यथा				/* }{ */

/* ISO C definitions */
#परिभाषय LUA_TMPNAMBUFSIZE	L_tmpnam
#परिभाषय lua_tmpnam(b,e)		{ e = (tmpnam(b) == रिक्त); }

#यदि_अन्तः				/* } */

#यदि_अन्तः				/* } */
/* }================================================================== */


#यदि !defined(l_system)
#यदि defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#परिभाषय l_system(cmd) ((cmd) == रिक्त ? 0 : -1)
#अन्यथा
#परिभाषय l_system(cmd)	system(cmd)  /* default definition */
#यदि_अन्तः
#यदि_अन्तः


स्थिर पूर्णाङ्क os_execute (lua_State *L) {
  ध्रुव वर्ण *cmd = luaL_optstring(L, 1, रिक्त);
  पूर्णाङ्क stat;
  errno = 0;
  stat = l_system(cmd);
  यदि (cmd != रिक्त)
    प्रत्यागम luaL_execresult(L, stat);
  अन्यथा {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    प्रत्यागम 1;
  }
}


स्थिर पूर्णाङ्क os_remove (lua_State *L) {
  ध्रुव वर्ण *filename = luaL_checkstring(L, 1);
  errno = 0;
  प्रत्यागम luaL_fileresult(L, remove(filename) == 0, filename);
}


स्थिर पूर्णाङ्क os_rename (lua_State *L) {
  ध्रुव वर्ण *fromname = luaL_checkstring(L, 1);
  ध्रुव वर्ण *toname = luaL_checkstring(L, 2);
  errno = 0;
  प्रत्यागम luaL_fileresult(L, rename(fromname, toname) == 0, रिक्त);
}


स्थिर पूर्णाङ्क os_tmpname (lua_State *L) {
  वर्ण buff[LUA_TMPNAMBUFSIZE];
  पूर्णाङ्क err;
  lua_tmpnam(buff, err);
  यदि (l_unlikely(err))
    प्रत्यागम luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_सङ्ख्या)clock())/(lua_सङ्ख्या)CLOCKS_PER_SEC);
  प्रत्यागम 1;
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
स्थिर शून्य setfield (lua_State *L, ध्रुव वर्ण *कुञ्चिका, पूर्णाङ्क मानम्, पूर्णाङ्क delta) {
  #यदि (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= अधिकतम_पूर्णाङ्कः)
    यदि (l_unlikely(मानम् > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", कुञ्चिका);
  #यदि_अन्तः
  lua_pushinteger(L, (lua_Integer)मानम् + delta);
  lua_setfield(L, -2, कुञ्चिका);
}


स्थिर शून्य setboolfield (lua_State *L, ध्रुव वर्ण *कुञ्चिका, पूर्णाङ्क मानम्) {
  यदि (मानम् < 0)  /* undefined? */
    प्रत्यागम;  /* does not set field */
  lua_pushboolean(L, मानम्);
  lua_setfield(L, -2, कुञ्चिका);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
स्थिर शून्य setallfields (lua_State *L, संरचना tm *stm) {
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


स्थिर पूर्णाङ्क getboolfield (lua_State *L, ध्रुव वर्ण *कुञ्चिका) {
  पूर्णाङ्क res;
  res = (lua_getfield(L, -1, कुञ्चिका) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_निष्कासय(L, 1);
  प्रत्यागम res;
}


स्थिर पूर्णाङ्क getfield (lua_State *L, ध्रुव वर्ण *कुञ्चिका, पूर्णाङ्क d, पूर्णाङ्क delta) {
  पूर्णाङ्क isnum;
  पूर्णाङ्क t = lua_getfield(L, -1, कुञ्चिका);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  यदि (!isnum) {  /* field is not an integer? */
    यदि (l_unlikely(t != LUA_TNIL))  /* some other value? */
      प्रत्यागम luaL_error(L, "field '%s' is not an integer", कुञ्चिका);
    अन्यथा यदि (l_unlikely(d < 0))  /* absent field; no default? */
      प्रत्यागम luaL_error(L, "field '%s' missing in date table", कुञ्चिका);
    res = d;
  }
  अन्यथा {
    यदि (!(res >= 0 ? res - delta <= अधिकतम_पूर्णाङ्कः : INT_MIN + delta <= res))
      प्रत्यागम luaL_error(L, "field '%s' is out-of-bound", कुञ्चिका);
    res -= delta;
  }
  lua_निष्कासय(L, 1);
  प्रत्यागम (पूर्णाङ्क)res;
}


स्थिर ध्रुव वर्ण *checkoption (lua_State *L, ध्रुव वर्ण *conv,
                                size_t convlen, वर्ण *buff) {
  ध्रुव वर्ण *option = LUA_STRFTIMEOPTIONS;
  अचिह्नित oplen = 1;  /* length of options being checked */
  प्रति (; *option != '\0' && oplen <= convlen; option += oplen) {
    यदि (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    अन्यथा यदि (memcmp(conv, option, oplen) == 0) {  /* match? */
      स्मृतिं_प्रतिलिख(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      प्रत्यागम conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  प्रत्यागम conv;  /* to avoid warnings */
}


स्थिर time_t l_checktime (lua_State *L, पूर्णाङ्क arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  प्रत्यागम (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#परिभाषय SIZETIMEFMT	250


स्थिर पूर्णाङ्क os_date (lua_State *L) {
  size_t slen;
  ध्रुव वर्ण *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(रिक्त));
  ध्रुव वर्ण *se = s + slen;  /* 's' end */
  संरचना tm tmr, *stm;
  यदि (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  अन्यथा
    stm = l_localtime(&t, &tmr);
  यदि (stm == रिक्त)  /* invalid date? */
    प्रत्यागम luaL_error(L,
                 "date result cannot be represented in this installation");
  यदि (वाक्यं_तुलय(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  अन्यथा {
    वर्ण cc[4];  /* buffer for individual conversion specifiers */
    lua_L_बफर b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    यावत् (s < se) {
      यदि (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      अन्यथा {
        size_t reslen;
        वर्ण *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क os_time_aux (lua_State *L, time_t t, पूर्णाङ्क err) {
  यदि (err || t != (time_t)(l_timet)t)
    प्रत्यागम luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क os_time (lua_State *L) {
  यदि (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(रिक्त);  /* get current time; error if it is -1 */
    प्रत्यागम os_time_aux(L, t, (t == (time_t)(-1)));
  }
  अन्यथा {
    time_t t;
    संरचना tm ts;
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
    प्रत्यागम os_time_aux(L, t, ts.tm_wday == -1);
  }
}


स्थिर पूर्णाङ्क os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_सङ्ख्या)difftime(t1, t2));
  प्रत्यागम 1;
}

/* }====================================================== */


स्थिर पूर्णाङ्क os_setlocale (lua_State *L) {
  स्थिर ध्रुव पूर्णाङ्क cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  स्थिर ध्रुव वर्ण *ध्रुव catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", रिक्त};
  ध्रुव वर्ण *l = luaL_optstring(L, 1, रिक्त);
  पूर्णाङ्क op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क os_exit (lua_State *L) {
  पूर्णाङ्क स्थितिः;
  यदि (lua_isboolean(L, 1))
    स्थितिः = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  अन्यथा
    स्थितिः = (पूर्णाङ्क)luaL_optinteger(L, 1, EXIT_SUCCESS);
  यदि (lua_toboolean(L, 2))
    lua_close(L);
  यदि (L) निर्गच्छ(स्थितिः);  /* 'if' to avoid warnings for unreachable 'return' */
  प्रत्यागम 0;
}


स्थिर ध्रुव luaL_Reg syslib[] = {
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



LUAMOD_API पूर्णाङ्क luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  प्रत्यागम 1;
}

