/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


static 整数 luaB_print (lua_State *L) {
  整数 n = lua_gettop(L);  /* number of arguments */
  整数 i;
  为 (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const char *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    如果 (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  返回 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static 整数 luaB_warn (lua_State *L) {
  整数 n = lua_gettop(L);  /* number of arguments */
  整数 i;
  luaL_checkstring(L, 1);  /* at least one argument */
  为 (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  为 (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  返回 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  整数 neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  如果 (*s == '-') { s++; neg = 1; }  /* handle sign */
  否则 如果 (*s == '+') s++;
  如果 (!isalnum(cast_uchar(*s)))  /* no digit? */
    返回 NULL;
  do {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    如果 (digit >= base) 返回 NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } 当 (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  返回 s;
}


static 整数 luaB_tonumber (lua_State *L) {
  如果 (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    如果 (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      返回 1;
    }
    否则 {
      size_t l;
      const char *s = lua_tolstring(L, 1, &l);
      如果 (s != NULL && lua_stringtonumber(L, s) == l + 1)
        返回 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  否则 {
    size_t l;
    const char *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    如果 (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      返回 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  返回 1;
}


static 整数 luaB_error (lua_State *L) {
  整数 level = (整数)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  如果 (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  返回 lua_error(L);
}


static 整数 luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  如果 (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    返回 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  返回 1;  /* returns either __metatable field (if present) or metatable */
}


static 整数 luaB_setmetatable (lua_State *L) {
  整数 t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  如果 (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    返回 luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  返回 1;
}


static 整数 luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  返回 1;
}


static 整数 luaB_rawlen (lua_State *L) {
  整数 t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  返回 1;
}


static 整数 luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  返回 1;
}

static 整数 luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  返回 1;
}


static 整数 pushmode (lua_State *L, 整数 oldmode) {
  如果 (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  否则
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  返回 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { 如果 (res == -1) break; }

static 整数 luaB_collectgarbage (lua_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", NULL};
  static const char optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  整数 o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    case LUA_GCCOUNT: {
      整数 k = lua_gc(L, o);
      整数 b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_Number)k + ((lua_Number)b/1024));
      返回 1;
    }
    case LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      整数 res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      返回 1;
    }
    case LUA_GCISRUNNING: {
      整数 res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      返回 1;
    }
    case LUA_GCGEN: {
      返回 pushmode(L, lua_gc(L, o));
    }
    case LUA_GCINC: {
      返回 pushmode(L, lua_gc(L, o));
    }
    case LUA_GCPARAM: {
      static const char *const params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", NULL};
      static const char pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      整数 p = pnum[luaL_checkoption(L, 2, NULL, params)];
      lua_Integer value = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (整数)value));
      返回 1;
    }
    default: {
      整数 res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      返回 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  返回 1;
}


static 整数 luaB_type (lua_State *L) {
  整数 t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  返回 1;
}


static 整数 luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  如果 (lua_next(L, 1))
    返回 2;
  否则 {
    lua_pushnil(L);
    返回 1;
  }
}


static 整数 pairscont (lua_State *L, 整数 status, lua_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  返回 4;  /* __pairs did all the work, just return its results */
}

static 整数 luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  如果 (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  否则 {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  返回 4;
}


/*
** Traversal function for 'ipairs'
*/
static 整数 ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  返回 (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static 整数 luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  返回 3;
}


static 整数 load_aux (lua_State *L, 整数 status, 整数 envidx) {
  如果 (l_likely(status == LUA_OK)) {
    如果 (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      如果 (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    返回 1;
  }
  否则 {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_insert(L, -2);  /* put before error message */
    返回 2;  /* return fail plus error message */
  }
}


static const char *getMode (lua_State *L, 整数 idx) {
  const char *mode = luaL_optstring(L, idx, NULL);
  如果 (mode != NULL && strchr(mode, 'B') != NULL) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  返回 mode;
}


static 整数 luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = getMode(L, 2);
  整数 env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  整数 status = luaL_loadfilex(L, fname, mode);
  返回 load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
  整数 *firstcall = cast(整数 *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  如果 (*firstcall)
    *firstcall = 0;
  否则
    lua_pop(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  如果 (lua_isnil(L, -1)) {
    *size = 0;
    返回 NULL;
  }
  否则 如果 (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  返回 lua_tolstring(L, -1, size);
}


static 整数 luaB_load (lua_State *L) {
  整数 status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = getMode(L, 3);
  整数 env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  如果 (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  否则 {  /* loading from a reader function */
    整数 firstcall = 1;  /* userdata for generic_reader */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  返回 load_aux(L, status, env);
}

/* }====================================================== */


static 整数 dofilecont (lua_State *L, 整数 d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  返回 lua_gettop(L) - 1;
}


static 整数 luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  如果 (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    返回 lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  返回 dofilecont(L, 0, 0);
}


static 整数 luaB_assert (lua_State *L) {
  如果 (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    返回 lua_gettop(L);  /* return all arguments */
  否则 {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    返回 luaB_error(L);  /* call 'error' */
  }
}


static 整数 luaB_select (lua_State *L) {
  整数 n = lua_gettop(L);
  如果 (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    返回 1;
  }
  否则 {
    lua_Integer i = luaL_checkinteger(L, 1);
    如果 (i < 0) i = n + i;
    否则 如果 (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    返回 n - (整数)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static 整数 finishpcall (lua_State *L, 整数 status, lua_KContext extra) {
  如果 (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    返回 2;  /* return false, msg */
  }
  否则
    返回 lua_gettop(L) - (整数)extra;  /* return all results */
}


static 整数 luaB_pcall (lua_State *L) {
  整数 status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  返回 finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static 整数 luaB_xpcall (lua_State *L) {
  整数 status;
  整数 n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  返回 finishpcall(L, status, 2);
}


static 整数 luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  返回 1;
}


static const luaL_Reg base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"warn", luaB_warn},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},
  /* placeholders */
  {LUA_GNAME, NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};


LUAMOD_API 整数 luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  返回 1;
}

