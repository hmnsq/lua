/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#include "lprefix.h"


#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#အကယ်၍ !defined(LUA_STRFTIMEOPTIONS)	/* { */

#အကယ်၍ defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */
#elif defined(LUA_USE_C89)  /* C89 (only 1-char options) */
#define LUA_STRFTIMEOPTIONS  "aAbBcdHIjmMpSUwWxXyYZ%"
#မဟုတ်ပါက  /* C99 specification */
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
#အကယ်၍ !defined(LUA_NUMTIME)	/* { */

#define l_timet			lua_Integer
#define l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))
#define l_gettime(L,arg)	luaL_checkinteger(L, arg)

#မဟုတ်ပါက				/* }{ */

#define l_timet			lua_Number
#define l_pushtime(L,t)		lua_pushnumber(L,(lua_Number)(t))
#define l_gettime(L,arg)	luaL_checknumber(L, arg)

#endif				/* } */


#အကယ်၍ !defined(l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#အကယ်၍ defined(LUA_USE_POSIX)	/* { */

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#မဟုတ်ပါက				/* }{ */

/* ISO C definitions */
#define l_gmtime(t,r)		((void)(r)->tm_sec, gmtime(t))
#define l_localtime(t,r)	((void)(r)->tm_sec, localtime(t))

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
#အကယ်၍ !defined(lua_tmpnam)	/* { */

#အကယ်၍ defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#အကယ်၍ !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        အကယ်၍ (e != -1) close(e); \
        e = (e == -1); }

#မဟုတ်ပါက				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == NULL); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */


#အကယ်၍ !defined(l_system)
#အကယ်၍ defined(LUA_USE_IOS)
/* Despite claiming to be ISO C, iOS does not implement 'system'. */
#define l_system(cmd) ((cmd) == NULL ? 0 : -1)
#မဟုတ်ပါက
#define l_system(cmd)	system(cmd)  /* default definition */
#endif
#endif


static ကိန်းပြည့် os_execute (lua_State *L) {
  const var_na *cmd = luaL_optstring(L, 1, NULL);
  ကိန်းပြည့် stat;
  errno = 0;
  stat = l_system(cmd);
  အကယ်၍ (cmd != NULL)
    ပြန်ပေး luaL_execresult(L, stat);
  မဟုတ်ပါက {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    ပြန်ပေး 1;
  }
}


static ကိန်းပြည့် os_remove (lua_State *L) {
  const var_na *filename = luaL_checkstring(L, 1);
  errno = 0;
  ပြန်ပေး luaL_fileresult(L, remove(filename) == 0, filename);
}


static ကိန်းပြည့် os_rename (lua_State *L) {
  const var_na *fromname = luaL_checkstring(L, 1);
  const var_na *toname = luaL_checkstring(L, 2);
  errno = 0;
  ပြန်ပေး luaL_fileresult(L, rename(fromname, toname) == 0, NULL);
}


static ကိန်းပြည့် os_tmpname (lua_State *L) {
  var_na buff[LUA_TMPNAMBUFSIZE];
  ကိန်းပြည့် err;
  lua_tmpnam(buff, err);
  အကယ်၍ (l_unlikely(err))
    ပြန်ပေး luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  ပြန်ပေး 1;
}


static ကိန်းပြည့် os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_Number)clock())/(lua_Number)CLOCKS_PER_SEC);
  ပြန်ပေး 1;
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
static void setfield (lua_State *L, const var_na *key, ကိန်းပြည့် value, ကိန်းပြည့် delta) {
  #အကယ်၍ (defined(LUA_NUMTIME) && LUA_MAXINTEGER <= INT_MAX)
    အကယ်၍ (l_unlikely(value > LUA_MAXINTEGER - delta))
      luaL_error(L, "field '%s' is out-of-bound", key);
  #endif
  lua_pushinteger(L, (lua_Integer)value + delta);
  lua_setfield(L, -2, key);
}


static void setboolfield (lua_State *L, const var_na *key, ကိန်းပြည့် value) {
  အကယ်၍ (value < 0)  /* undefined? */
    ပြန်ပေး;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
static void setallfields (lua_State *L, sa_nracanaa tm *stm) {
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


static ကိန်းပြည့် getboolfield (lua_State *L, const var_na *key) {
  ကိန်းပြည့် res;
  res = (lua_getfield(L, -1, key) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  ပြန်ပေး res;
}


static ကိန်းပြည့် getfield (lua_State *L, const var_na *key, ကိန်းပြည့် d, ကိန်းပြည့် delta) {
  ကိန်းပြည့် isnum;
  ကိန်းပြည့် t = lua_getfield(L, -1, key);  /* get field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  အကယ်၍ (!isnum) {  /* field is not an integer? */
    အကယ်၍ (l_unlikely(t != LUA_TNIL))  /* some other value? */
      ပြန်ပေး luaL_error(L, "field '%s' is not an integer", key);
    မဟုတ်ပါက အကယ်၍ (l_unlikely(d < 0))  /* absent field; no default? */
      ပြန်ပေး luaL_error(L, "field '%s' missing in date table", key);
    res = d;
  }
  မဟုတ်ပါက {
    အကယ်၍ (!(res >= 0 ? res - delta <= INT_MAX : INT_MIN + delta <= res))
      ပြန်ပေး luaL_error(L, "field '%s' is out-of-bound", key);
    res -= delta;
  }
  lua_pop(L, 1);
  ပြန်ပေး (ကိန်းပြည့်)res;
}


static const var_na *checkoption (lua_State *L, const var_na *conv,
                                size_t convlen, var_na *buff) {
  const var_na *option = LUA_STRFTIMEOPTIONS;
  unsigned oplen = 1;  /* length of options being checked */
  အတွက် (; *option != '\0' && oplen <= convlen; option += oplen) {
    အကယ်၍ (*option == '|')  /* next block? */
      oplen++;  /* will check options with next length (+1) */
    မဟုတ်ပါက အကယ်၍ (memcmp(conv, option, oplen) == 0) {  /* match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      ပြန်ပေး conv + oplen;  /* return next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  ပြန်ပေး conv;  /* to avoid warnings */
}


static time_t l_checktime (lua_State *L, ကိန်းပြည့် arg) {
  l_timet t = l_gettime(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  ပြန်ပေး (time_t)t;
}


/* maximum size for an individual 'strftime' item */
#define SIZETIMEFMT	250


static ကိန်းပြည့် os_date (lua_State *L) {
  size_t slen;
  const var_na *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, l_checktime, 2, time(NULL));
  const var_na *se = s + slen;  /* 's' end */
  sa_nracanaa tm tmr, *stm;
  အကယ်၍ (*s == '!') {  /* UTC? */
    stm = l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  မဟုတ်ပါက
    stm = l_localtime(&t, &tmr);
  အကယ်၍ (stm == NULL)  /* invalid date? */
    ပြန်ပေး luaL_error(L,
                 "date result cannot be represented in this installation");
  အကယ်၍ (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setallfields(L, stm);
  }
  မဟုတ်ပါက {
    var_na cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    နေစဉ် (s < se) {
      အကယ်၍ (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      မဟုတ်ပါက {
        size_t reslen;
        var_na *buff = luaL_prepbuffsize(&b, SIZETIMEFMT);
        s++;  /* skip '%' */
        /* copy specifier to 'cc' */
        s = checkoption(L, s, ct_diff2sz(se - s), cc + 1);
        reslen = strftime(buff, SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် os_time_aux (lua_State *L, time_t t, ကိန်းပြည့် err) {
  အကယ်၍ (err || t != (time_t)(l_timet)t)
    ပြန်ပေး luaL_error(L,
                  "time result cannot be represented in this installation");
  l_pushtime(L, t);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် os_time (lua_State *L) {
  အကယ်၍ (lua_isnoneornil(L, 1)) {  /* called without args? */
    time_t t = time(NULL);  /* get current time; error if it is -1 */
    ပြန်ပေး os_time_aux(L, t, (t == (time_t)(-1)));
  }
  မဟုတ်ပါက {
    time_t t;
    sa_nracanaa tm ts;
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
    ပြန်ပေး os_time_aux(L, t, ts.tm_wday == -1);
  }
}


static ကိန်းပြည့် os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_Number)difftime(t1, t2));
  ပြန်ပေး 1;
}

/* }====================================================== */


static ကိန်းပြည့် os_setlocale (lua_State *L) {
  static const ကိန်းပြည့် cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  static const var_na *const catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", NULL};
  const var_na *l = luaL_optstring(L, 1, NULL);
  ကိန်းပြည့် op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် os_exit (lua_State *L) {
  ကိန်းပြည့် status;
  အကယ်၍ (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  မဟုတ်ပါက
    status = (ကိန်းပြည့်)luaL_optinteger(L, 1, EXIT_SUCCESS);
  အကယ်၍ (lua_toboolean(L, 2))
    lua_close(L);
  အကယ်၍ (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  ပြန်ပေး 0;
}


static const luaL_Reg syslib[] = {
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
  {NULL, NULL}
};

/* }====================================================== */



LUAMOD_API ကိန်းပြည့် luaopen_os (lua_State *L) {
  luaL_newlib(L, syslib);
  ပြန်ပေး 1;
}

