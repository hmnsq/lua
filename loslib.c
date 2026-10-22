/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#घोषणा loslib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <errno.h>
#समावेश <locale.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#अगर !defined(LUA_STRFTIMEOPTIONS)	/* { */

#अगर defined(LUA_USE_WINDOWS)
#घोषणा LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#या_अगर defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#घोषणा LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#अन्यथा  /* C99 specification */
#घोषणा LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#पूर्ण_अगर

#पूर्ण_अगर					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#अगर !defined(LUA_NUMTIME)	/* { */

#घोषणा l_timet			lua_Integer
#घोषणा l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#घोषणा l_gettime(L,arg)	luaL_checkinteger(L, arg)

#अन्यथा				/* }{ */

#घोषणा l_timet			lua_संख्या
#घोषणा l_pushtime(L,t)		lua_pushnumber(L,(lua_संख्या)(t))
#घोषणा l_gettime(L,arg)	luaL_checknumber(L, arg)

#पूर्ण_अगर				/* } */


#अगर !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#अगर defined(LUA_USE_POSIX)	/* { */

#घोषणा l_gmtime(t,r)		gmtime_r(t,r)
#घोषणा l_localtime(t,r)	localtime_r(t,r)

#अन्यथा				/* }{ */

/* ISO C definitions */
#घोषणा l_gmtime(t,r)		((व्योम)(r)->tm_sec, स_जमट(t))
#घोषणा l_localtime(t,r)	((व्योम)(r)->tm_sec, स_स्थानीय(t))

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#अगर !defined(lua_tmpnam)	/* { */

#अगर defined(LUA_USE_POSIX)	/* { */

#समावेश <unistd.h>

#घोषणा LUA_TMPNAMBUFSIZE	32

#अगर !defined(LUA_TMPNAMTEMPLATE)
#घोषणा LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#पूर्ण_अगर

#घोषणा lua_tmpnam(b,e) { \
        म_नकल(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        अगर (e != -1) बंद(e); \
        e = (e == -1); }

#अन्यथा				/* }{ */

/* ISO C definitions */
#घोषणा LUA_TMPNAMBUFSIZE	ब_क्षणिक
#घोषणा lua_tmpnam(b,e)		{ e = (क्षणिक(b) == शून्य); }

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */
/* }================================================================== */


#अगर !defined(l_system)
#अगर defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#घोषणा l_system(cmd) ((cmd) == शून्य ? 0 : -1)
#अन्यथा
#घोषणा l_system(cmd)	प्रणाली(cmd)  /* default definition */
#पूर्ण_अगर
#पूर्ण_अगर


अटल पूर्णांक os_execute (lua_State *L) {
  स्थिर अक्षर *cmd = luaL_optstring(L, 1, शून्य);
  पूर्णांक stat;
  त्रुटि_सं = 0;
  stat = l_system(cmd);
  अगर (cmd != शून्य)
    वापस luaL_execresult(L, stat);
  अन्यथा {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    वापस 1;
  }
}


अटल पूर्णांक os_remove (lua_State *L) {
  स्थिर अक्षर *filename = luaL_checkstring(L, 1);
  त्रुटि_सं = 0;
  वापस luaL_fileresult(L, हटाओ(filename) == 0, filename);
}


अटल पूर्णांक os_rename (lua_State *L) {
  स्थिर अक्षर *fromname = luaL_checkstring(L, 1);
  स्थिर अक्षर *toname = luaL_checkstring(L, 2);
  त्रुटि_सं = 0;
  वापस luaL_fileresult(L, नाम(fromname, toname) == 0, शून्य);
}


अटल पूर्णांक os_tmpname (lua_State *L) {
  अक्षर buff[LUA_TMPNAMBUFSIZE];
  पूर्णांक err;
  lua_tmpnam(buff, err);
  अगर (l_unlikely(err))
    वापस luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  वापस 1;
}


अटल पूर्णांक os_getenv (lua_State *L) {
  lua_pushstring(L, दो_पर्या(luaL_checkstring(L, 1)));  /* if NULL push nil */
  वापस 1;
}


अटल पूर्णांक os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_संख्या)घड़ी())/(lua_संख्या)CLOCKS_PER_SEC);
  वापस 1;
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
अटल व्योम setfield (lua_State *L, स्थिर अक्षर *कुंजी, पूर्णांक मूल्य, पूर्णांक delta) {
  #अगर (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= पूर्णांक_उच्च)
    अगर (l_unlikely(मूल्य > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", कुंजी);
  #पूर्ण_अगर
  lua_pushinteger(L, (lua_Integer)मूल्य + delta);
  lua_setfield(L, -2, कुंजी);
}


अटल व्योम setboolfield (lua_State *L, स्थिर अक्षर *कुंजी, पूर्णांक मूल्य) {
  अगर (मूल्य < 0)  /* undefined? */
    वापस;  /* does not set field */
  lua_pushboolean(L, मूल्य);
  lua_setfield(L, -2, कुंजी);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
अटल व्योम setallfields (lua_State *L, काष्ठा पंचांग *stm) {
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


अटल पूर्णांक getboolfield (lua_State *L, स्थिर अक्षर *कुंजी) {
  पूर्णांक res;
  res = (lua_getfield(L, -1, कुंजी) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_निकालो(L, 1);
  वापस res;
}


अटल पूर्णांक getfield (lua_State *L, स्थिर अक्षर *कुंजी, पूर्णांक d, पूर्णांक delta) {
  पूर्णांक isnum;
  पूर्णांक t = lua_getfield(L, -1, कुंजी);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  अगर (!isnum) {  /* field is not an integer? */
    अगर (l_unlikely(t != LUA_TNIL))  /* some other value? */
      वापस luaL_error(L, "field '%s' is not an integer", कुंजी);
    अन्यथा अगर (l_unlikely(d < 0))  /* absent field; no default? */
      वापस luaL_error(L, "field '%s' missing in date table", कुंजी);
    res = d;
  }
  अन्यथा {
    अगर (!(res >= 0 ? res - delta <= पूर्णांक_उच्च : पूर्णांक_न्यून + delta <= res))
      वापस luaL_error(L, "field '%s' is out-of-bound", कुंजी);
    res -= delta;
  }
  lua_निकालो(L, 1);
  वापस (पूर्णांक)res;
}


अटल स्थिर अक्षर *checkoption (lua_State *L, स्थिर अक्षर *conv,
                                माप_प्रकार convlen, अक्षर *buff) {
  स्थिर अक्षर *option = LUA_STRFTIMEOPTIONS;
  अचिन्हित oplen = 1;  /* length of options being checked */
  क्रम (; *option != '\0' && oplen <= convlen; option += oplen) {
    अगर (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    अन्यथा अगर (स_भेद(conv, option, oplen) == 0) {  /* match? */
      स_नकल(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      वापस conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  वापस conv;  /* to avoid warnings */
}


अटल समय_प्रकार l_checktime (lua_State *L, पूर्णांक arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (समय_प्रकार)t == t, arg, "time out-of-bounds");
  वापस (समय_प्रकार)t;
}


/* maximum size for an individual 'strftime' item */
#घोषणा SIZETIMEFMT	250


अटल पूर्णांक os_date (lua_State *L) {
  माप_प्रकार slen;
  स्थिर अक्षर *s = luaL_optlstring(L, 1, "%c", &slen);
  समय_प्रकार t = luaL_opt(L, l_checktime, 2, समय(शून्य));
  स्थिर अक्षर *se = s + slen;  /* 's' end */
  काष्ठा पंचांग tmr, *stm;
  अगर (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  अन्यथा
    stm = l_localtime(&t, &tmr);
  अगर (stm == शून्य)  /* invalid date? */
    वापस luaL_error(L,
                 "date result cannot be represented in this installation");
  अगर (म_भेद(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  अन्यथा {
    अक्षर cc[4];  /* buffer for individual conversion specifiers */
    lua_L_बफर b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    जबतक (s < se) {
      अगर (*s != '%')  /* not a conversion specifier? */
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
  वापस 1;
}


अटल पूर्णांक os_time_aux (lua_State *L, समय_प्रकार t, पूर्णांक err) {
  अगर (err || t != (समय_प्रकार)(l_timet)t)
    वापस luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  वापस 1;
}


अटल पूर्णांक os_time (lua_State *L) {
  अगर (lua_isnoneornil(L, 1)) {  /* called without args? */
    समय_प्रकार t = समय(शून्य);  /* get current time; error if it is -1 */
    वापस os_time_aux(L, t, (t == (समय_प्रकार)(-1)));
  }
  अन्यथा {
    समय_प्रकार t;
    काष्ठा पंचांग ts;
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
    वापस os_time_aux(L, t, ts.tm_wday == -1);
  }
}


अटल पूर्णांक os_difftime (lua_State *L) {
  समय_प्रकार t1 = l_checktime(L, 1);
  समय_प्रकार t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_संख्या)स_अंतर(t1, t2));
  वापस 1;
}

/* }====================================================== */


अटल पूर्णांक os_setlocale (lua_State *L) {
  अटल स्थिर पूर्णांक cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  अटल स्थिर अक्षर *स्थिर catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", शून्य};
  स्थिर अक्षर *l = luaL_optstring(L, 1, शून्य);
  पूर्णांक op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, रखो_क्षेत्र(cat[op], l));
  वापस 1;
}


अटल पूर्णांक os_exit (lua_State *L) {
  पूर्णांक status;
  अगर (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? निकास_सफल : निकास_त्रुटि);
  अन्यथा
    status = (पूर्णांक)luaL_optinteger(L, 1, निकास_सफल);
  अगर (lua_toboolean(L, 2))
    lua_close(L);
  अगर (L) निकास(status);  /* 'if' to avoid warnings for unreachable 'return' */
  वापस 0;
}


अटल स्थिर luaL_Reg syslib[] = {
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
  {शून्य, शून्य}
};

/* }====================================================== */



LUAMOD_API पूर्णांक luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  वापस 1;
}

