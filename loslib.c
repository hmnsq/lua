/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು loslib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

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
#ಆದರೆ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#ಆದರೆ defined(LUA_USE_WINDOWS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#ಅಥವಾಆದರೆ defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#ಇಲ್ಲದಿದ್ದರೆ  /* C99 specification */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_STRFTIMEOPTIONS  "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

/*
** type to represent time_t in Lua
*/
#ಆದರೆ !defined(LUA_NUMTIME)	/* { */

#ವ್ಯಾಖ್ಯಾನಿಸು l_timet			lua_Integer
#ವ್ಯಾಖ್ಯಾನಿಸು l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#ವ್ಯಾಖ್ಯಾನಿಸು l_gettime(L,arg)	luaL_checkinteger(L, arg)

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

#ವ್ಯಾಖ್ಯಾನಿಸು l_timet			lua_ಸಂಖ್ಯೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_pushtime(L,t)		lua_pushnumber(L,(lua_ಸಂಖ್ಯೆ)(t))
#ವ್ಯಾಖ್ಯಾನಿಸು l_gettime(L,arg)	luaL_checknumber(L, arg)

#ಆದರೆ_ಅಂತ್ಯ				/* } */


#ಆದರೆ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#ಆದರೆ defined(LUA_USE_POSIX)	/* { */

#ವ್ಯಾಖ್ಯಾನಿಸು l_gmtime(t,r)		gmtime_r(t,r)
#ವ್ಯಾಖ್ಯಾನಿಸು l_localtime(t,r)	localtime_r(t,r)

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

/* ISO C definitions */
#ವ್ಯಾಖ್ಯಾನಿಸು l_gmtime(t,r)		((ಶೂನ್ಯ)(r)->tm_sec, gmtime(t))
#ವ್ಯಾಖ್ಯಾನಿಸು l_localtime(t,r)	((ಶೂನ್ಯ)(r)->tm_sec, localtime(t))

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#ಆದರೆ !defined(lua_tmpnam)	/* { */

#ಆದರೆ defined(LUA_USE_POSIX)	/* { */

#ಅನ್ತರ್ಭಾವ <unistd.h>

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TMPNAMBUFSIZE	32

#ಆದರೆ !defined(LUA_TMPNAMTEMPLATE)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        ಆದರೆ (e != -1) close(e); \
        e = (e == -1); }

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

/* ISO C definitions */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TMPNAMBUFSIZE	L_tmpnam
#ವ್ಯಾಖ್ಯಾನಿಸು lua_tmpnam(b,e)		{ e = (tmpnam(b) == ಖಾಲಿ); }

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */
/* }================================================================== */


#ಆದರೆ !defined(l_system)
#ಆದರೆ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#ವ್ಯಾಖ್ಯಾನಿಸು l_system(cmd) ((cmd) == ಖಾಲಿ ? 0 : -1)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_system(cmd)	system(cmd)  /* default definition */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_execute (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd = luaL_optstring(L, 1, ಖಾಲಿ);
  ಪೂರ್ಣಾಂಕ stat;
  errno = 0;
  stat = l_system(cmd);
  ಆದರೆ (cmd != ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ luaL_execresult(L, stat);
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ಹಿಂತಿರುಗಿ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_remove (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = luaL_checkstring(L, 1);
  errno = 0;
  ಹಿಂತಿರುಗಿ luaL_fileresult(L, remove(filename) == 0, filename);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_rename (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fromname = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *toname = luaL_checkstring(L, 2);
  errno = 0;
  ಹಿಂತಿರುಗಿ luaL_fileresult(L, rename(fromname, toname) == 0, ಖಾಲಿ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_tmpname (lua_State *L) {
  ಅಕ್ಷರ buff[LUA_TMPNAMBUFSIZE];
  ಪೂರ್ಣಾಂಕ err;
  lua_tmpnam(buff, err);
  ಆದರೆ (l_unlikely(err))
    ಹಿಂತಿರುಗಿ luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_ಸಂಖ್ಯೆ)clock())/(lua_ಸಂಖ್ಯೆ)CLOCKS_PER_SEC);
  ಹಿಂತಿರುಗಿ 1;
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
ಸ್ಥಿರ ಶೂನ್ಯ setfield (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ delta) {
  #ಆದರೆ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)
    ಆದರೆ (l_unlikely(ಮೌಲ್ಯ > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", ಕೀಲಿ);
  #ಆದರೆ_ಅಂತ್ಯ
  lua_pushinteger(L, (lua_Integer)ಮೌಲ್ಯ + delta);
  lua_setfield(L, -2, ಕೀಲಿ);
}


ಸ್ಥಿರ ಶೂನ್ಯ setboolfield (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ) {
  ಆದರೆ (ಮೌಲ್ಯ < 0)  /* undefined? */
    ಹಿಂತಿರುಗಿ;  /* does not set field */
  lua_pushboolean(L, ಮೌಲ್ಯ);
  lua_setfield(L, -2, ಕೀಲಿ);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
ಸ್ಥಿರ ಶೂನ್ಯ setallfields (lua_State *L, ರಚನೆ tm *stm) {
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


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getboolfield (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ) {
  ಪೂರ್ಣಾಂಕ res;
  res = (lua_getfield(L, -1, ಕೀಲಿ) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_ತೆಗೆ(L, 1);
  ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getfield (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ d, ಪೂರ್ಣಾಂಕ delta) {
  ಪೂರ್ಣಾಂಕ isnum;
  ಪೂರ್ಣಾಂಕ t = lua_getfield(L, -1, ಕೀಲಿ);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  ಆದರೆ (!isnum) {  /* field is not an integer? */
    ಆದರೆ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ಹಿಂತಿರುಗಿ luaL_error(L, "field '%s' is not an integer", ಕೀಲಿ);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(d < 0))  /* absent field; no default? */
      ಹಿಂತಿರುಗಿ luaL_error(L, "field '%s' missing in date table", ಕೀಲಿ);
    res = d;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (!(res >= 0 ? res - delta <= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ : INT_MIN + delta <= res))
      ಹಿಂತಿರುಗಿ luaL_error(L, "field '%s' is out-of-bound", ಕೀಲಿ);
    res -= delta;
  }
  lua_ತೆಗೆ(L, 1);
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)res;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *checkoption (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *conv,
                                size_t convlen, ಅಕ್ಷರ *buff) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *option = LUA_STRFTIMEOPTIONS;
  ಚಿಹ್ನೆರಹಿತ oplen = 1;  /* length of options being checked */
  ಗಾಗಿ (; *option != '\0' && oplen <= convlen; option += oplen) {
    ಆದರೆ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (memcmp(conv, option, oplen) == 0) {  /* match? */
      ಸ್ಮರಣೆ_ನಕಲು(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ಹಿಂತಿರುಗಿ conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ಹಿಂತಿರುಗಿ conv;  /* to avoid warnings */
}


ಸ್ಥಿರ time_t l_checktime (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ಹಿಂತಿರುಗಿ (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#ವ್ಯಾಖ್ಯಾನಿಸು SIZETIMEFMT	250


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_date (lua_State *L) {
  size_t slen;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(ಖಾಲಿ));
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *se = s + slen;  /* 's' end */
  ರಚನೆ tm tmr, *stm;
  ಆದರೆ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    stm = l_localtime(&t, &tmr);
  ಆದರೆ (stm == ಖಾಲಿ)  /* invalid date? */
    ಹಿಂತಿರುಗಿ luaL_error(L,
                 "date result cannot be represented in this installation");
  ಆದರೆ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಅಕ್ಷರ cc[4];  /* buffer for individual conversion specifiers */
    lua_L_ಬಫರ್ b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    ಆಗ (s < se) {
      ಆದರೆ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      ಇಲ್ಲದಿದ್ದರೆ {
        size_t reslen;
        ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_ಸಮಯ_aux (lua_State *L, time_t t, ಪೂರ್ಣಾಂಕ err) {
  ಆದರೆ (err || t != (time_t)(l_timet)t)
    ಹಿಂತಿರುಗಿ luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_ಸಮಯ (lua_State *L) {
  ಆದರೆ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(ಖಾಲಿ);  /* get current time; error if it is -1 */
    ಹಿಂತಿರುಗಿ os_ಸಮಯ_aux(L, t, (t == (time_t)(-1)));
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    time_t t;
    ರಚನೆ tm ts;
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
    ಹಿಂತಿರುಗಿ os_ಸಮಯ_aux(L, t, ts.tm_wday == -1);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)difftime(t1, t2));
  ಹಿಂತಿರುಗಿ 1;
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_setlocale (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", ಖಾಲಿ};
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *l = luaL_optstring(L, 1, ಖಾಲಿ);
  ಪೂರ್ಣಾಂಕ op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ os_exit (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಆದರೆ (lua_isboolean(L, 1))
    ಸ್ಥಿತಿ_2 = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿತಿ_2 = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, 1, EXIT_SUCCESS);
  ಆದರೆ (lua_toboolean(L, 2))
    lua_close(L);
  ಆದರೆ (L) ನಿರ್ಗಮಿಸು(ಸ್ಥಿತಿ_2);  /* 'if' to avoid warnings for unreachable 'return' */
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_ಸಮಯ},
  {"tmpname", os_tmpname},
  {ಖಾಲಿ, ಖಾಲಿ}
};

/* }====================================================== */



LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ಹಿಂತಿರುಗಿ 1;
}

