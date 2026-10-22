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


static ཧྲིལ luaB_print (lua_State *L) {
  ཧྲིལ n = lua_gettop(L);  /* number of arguments */
  ཧྲིལ i;
  དོན་ལུ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const var_na *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    གལ་སྲིད (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  ལོག 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static ཧྲིལ luaB_warn (lua_State *L) {
  ཧྲིལ n = lua_gettop(L);  /* number of arguments */
  ཧྲིལ i;
  luaL_checkstring(L, 1);  /* at least one argument */
  དོན་ལུ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  དོན་ལུ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ལོག 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const var_na *b_str2int (const var_na *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  ཧྲིལ neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  གལ་སྲིད (*s == '-') { s++; neg = 1; }  /* handle sign */
  དེ་མིན གལ་སྲིད (*s == '+') s++;
  གལ་སྲིད (!isalnum(cast_uchar(*s)))  /* no digit? */
    ལོག NULL;
  do {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    གལ་སྲིད (digit >= base) ལོག NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } སྐབས (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ལོག s;
}


static ཧྲིལ luaB_tonumber (lua_State *L) {
  གལ་སྲིད (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    གལ་སྲིད (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ལོག 1;
    }
    དེ་མིན {
      size_t l;
      const var_na *s = lua_tolstring(L, 1, &l);
      གལ་སྲིད (s != NULL && lua_stringtonumber(L, s) == l + 1)
        ལོག 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  དེ་མིན {
    size_t l;
    const var_na *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    གལ་སྲིད (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      ལོག 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ལོག 1;
}


static ཧྲིལ luaB_error (lua_State *L) {
  ཧྲིལ level = (ཧྲིལ)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  གལ་སྲིད (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ལོག lua_error(L);
}


static ཧྲིལ luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  གལ་སྲིད (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ལོག 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ལོག 1;  /* returns either __metatable field (if present) or metatable */
}


static ཧྲིལ luaB_setmetatable (lua_State *L) {
  ཧྲིལ t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  གལ་སྲིད (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ལོག luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ལོག 1;
}


static ཧྲིལ luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ལོག 1;
}


static ཧྲིལ luaB_rawlen (lua_State *L) {
  ཧྲིལ t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ལོག 1;
}


static ཧྲིལ luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ལོག 1;
}

static ཧྲིལ luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ལོག 1;
}


static ཧྲིལ pushmode (lua_State *L, ཧྲིལ oldmode) {
  གལ་སྲིད (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  དེ་མིན
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ལོག 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { གལ་སྲིད (res == -1) bha_nga; }

static ཧྲིལ luaB_collectgarbage (lua_State *L) {
  static const var_na *const opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", NULL};
  static const var_na optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  ཧྲིལ o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    pak_sa LUA_GCCOUNT: {
      ཧྲིལ k = lua_gc(L, o);
      ཧྲིལ b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_Number)k + ((lua_Number)b/1024));
      ལོག 1;
    }
    pak_sa LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      ཧྲིལ res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ལོག 1;
    }
    pak_sa LUA_GCISRUNNING: {
      ཧྲིལ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ལོག 1;
    }
    pak_sa LUA_GCGEN: {
      ལོག pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCINC: {
      ལོག pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCPARAM: {
      static const var_na *const params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", NULL};
      static const var_na pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      ཧྲིལ p = pnum[luaL_checkoption(L, 2, NULL, params)];
      lua_Integer value = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (ཧྲིལ)value));
      ལོག 1;
    }
    default: {
      ཧྲིལ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ལོག 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ལོག 1;
}


static ཧྲིལ luaB_type (lua_State *L) {
  ཧྲིལ t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ལོག 1;
}


static ཧྲིལ luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  གལ་སྲིད (lua_next(L, 1))
    ལོག 2;
  དེ་མིན {
    lua_pushnil(L);
    ལོག 1;
  }
}


static ཧྲིལ pairscont (lua_State *L, ཧྲིལ status, lua_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  ལོག 4;  /* __pairs did all the work, just return its results */
}

static ཧྲིལ luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  གལ་སྲིད (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  དེ་མིན {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ལོག 4;
}


/*
** Traversal function for 'ipairs'
*/
static ཧྲིལ ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ལོག (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static ཧྲིལ luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ལོག 3;
}


static ཧྲིལ load_aux (lua_State *L, ཧྲིལ status, ཧྲིལ envidx) {
  གལ་སྲིད (l_likely(status == LUA_OK)) {
    གལ་སྲིད (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      གལ་སྲིད (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    ལོག 1;
  }
  དེ་མིན {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_insert(L, -2);  /* put before error message */
    ལོག 2;  /* return fail plus error message */
  }
}


static const var_na *getMode (lua_State *L, ཧྲིལ idx) {
  const var_na *mode = luaL_optstring(L, idx, NULL);
  གལ་སྲིད (mode != NULL && strchr(mode, 'B') != NULL) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ལོག mode;
}


static ཧྲིལ luaB_loadfile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  const var_na *mode = getMode(L, 2);
  ཧྲིལ env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  ཧྲིལ status = luaL_loadfilex(L, fname, mode);
  ལོག load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
static const var_na *generic_reader (lua_State *L, void *ud, size_t *size) {
  ཧྲིལ *firstcall = cast(ཧྲིལ *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  གལ་སྲིད (*firstcall)
    *firstcall = 0;
  དེ་མིན
    lua_pop(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  གལ་སྲིད (lua_isnil(L, -1)) {
    *size = 0;
    ལོག NULL;
  }
  དེ་མིན གལ་སྲིད (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ལོག lua_tolstring(L, -1, size);
}


static ཧྲིལ luaB_load (lua_State *L) {
  ཧྲིལ status;
  size_t l;
  const var_na *s = lua_tolstring(L, 1, &l);
  const var_na *mode = getMode(L, 3);
  ཧྲིལ env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  གལ་སྲིད (s != NULL) {  /* loading a string? */
    const var_na *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  དེ་མིན {  /* loading from a reader function */
    ཧྲིལ firstcall = 1;  /* userdata for generic_reader */
    const var_na *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  ལོག load_aux(L, status, env);
}

/* }====================================================== */


static ཧྲིལ dofilecont (lua_State *L, ཧྲིལ d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  ལོག lua_gettop(L) - 1;
}


static ཧྲིལ luaB_dofile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  གལ་སྲིད (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ལོག lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ལོག dofilecont(L, 0, 0);
}


static ཧྲིལ luaB_assert (lua_State *L) {
  གལ་སྲིད (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ལོག lua_gettop(L);  /* return all arguments */
  དེ་མིན {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ལོག luaB_error(L);  /* call 'error' */
  }
}


static ཧྲིལ luaB_select (lua_State *L) {
  ཧྲིལ n = lua_gettop(L);
  གལ་སྲིད (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ལོག 1;
  }
  དེ་མིན {
    lua_Integer i = luaL_checkinteger(L, 1);
    གལ་སྲིད (i < 0) i = n + i;
    དེ་མིན གལ་སྲིད (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ལོག n - (ཧྲིལ)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static ཧྲིལ finishpcall (lua_State *L, ཧྲིལ status, lua_KContext extra) {
  གལ་སྲིད (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ལོག 2;  /* return false, msg */
  }
  དེ་མིན
    ལོག lua_gettop(L) - (ཧྲིལ)extra;  /* return all results */
}


static ཧྲིལ luaB_pcall (lua_State *L) {
  ཧྲིལ status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ལོག finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static ཧྲིལ luaB_xpcall (lua_State *L) {
  ཧྲིལ status;
  ཧྲིལ n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ལོག finishpcall(L, status, 2);
}


static ཧྲིལ luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  ལོག 1;
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


LUAMOD_API ཧྲིལ luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  ལོག 1;
}

