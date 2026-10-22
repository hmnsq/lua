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


static puur_naa_nka luaB_print (lua_State *L) {
  puur_naa_nka n = lua_gettop(L);  /* number of arguments */
  puur_naa_nka i;
  ꦏꦁꦒꦺꦴ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const var_na *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ꦪꦼꦤ꧀ (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  ꦮꦭꦶ 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static puur_naa_nka luaB_warn (lua_State *L) {
  puur_naa_nka n = lua_gettop(L);  /* number of arguments */
  puur_naa_nka i;
  luaL_checkstring(L, 1);  /* at least one argument */
  ꦏꦁꦒꦺꦴ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  ꦏꦁꦒꦺꦴ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ꦮꦭꦶ 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const var_na *b_str2int (const var_na *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  puur_naa_nka neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ꦪꦼꦤ꧀ (*s == '-') { s++; neg = 1; }  /* handle sign */
  ꦭꦶꦪ ꦪꦼꦤ꧀ (*s == '+') s++;
  ꦪꦼꦤ꧀ (!isalnum(cast_uchar(*s)))  /* no digit? */
    ꦮꦭꦶ NULL;
  do {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    ꦪꦼꦤ꧀ (digit >= base) ꦮꦭꦶ NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } ꦱꦼꦢꦼꦭ (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ꦮꦭꦶ s;
}


static puur_naa_nka luaB_tonumber (lua_State *L) {
  ꦪꦼꦤ꧀ (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ꦪꦼꦤ꧀ (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ꦮꦭꦶ 1;
    }
    ꦭꦶꦪ {
      size_t l;
      const var_na *s = lua_tolstring(L, 1, &l);
      ꦪꦼꦤ꧀ (s != NULL && lua_stringtonumber(L, s) == l + 1)
        ꦮꦭꦶ 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  ꦭꦶꦪ {
    size_t l;
    const var_na *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    ꦪꦼꦤ꧀ (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      ꦮꦭꦶ 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_error (lua_State *L) {
  puur_naa_nka level = (puur_naa_nka)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ꦪꦼꦤ꧀ (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ꦮꦭꦶ lua_error(L);
}


static puur_naa_nka luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ꦪꦼꦤ꧀ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ꦮꦭꦶ 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ꦮꦭꦶ 1;  /* returns either __metatable field (if present) or metatable */
}


static puur_naa_nka luaB_setmetatable (lua_State *L) {
  puur_naa_nka t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ꦪꦼꦤ꧀ (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ꦮꦭꦶ luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_rawlen (lua_State *L) {
  puur_naa_nka t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ꦮꦭꦶ 1;
}

static puur_naa_nka luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ꦮꦭꦶ 1;
}


static puur_naa_nka pushmode (lua_State *L, puur_naa_nka oldmode) {
  ꦪꦼꦤ꧀ (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  ꦭꦶꦪ
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ꦮꦭꦶ 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { ꦪꦼꦤ꧀ (res == -1) bha_nga; }

static puur_naa_nka luaB_collectgarbage (lua_State *L) {
  static const var_na *const opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", NULL};
  static const var_na optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  puur_naa_nka o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    pak_sa LUA_GCCOUNT: {
      puur_naa_nka k = lua_gc(L, o);
      puur_naa_nka b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_Number)k + ((lua_Number)b/1024));
      ꦮꦭꦶ 1;
    }
    pak_sa LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      puur_naa_nka res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ꦮꦭꦶ 1;
    }
    pak_sa LUA_GCISRUNNING: {
      puur_naa_nka res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ꦮꦭꦶ 1;
    }
    pak_sa LUA_GCGEN: {
      ꦮꦭꦶ pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCINC: {
      ꦮꦭꦶ pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCPARAM: {
      static const var_na *const params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", NULL};
      static const var_na pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      puur_naa_nka p = pnum[luaL_checkoption(L, 2, NULL, params)];
      lua_Integer value = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (puur_naa_nka)value));
      ꦮꦭꦶ 1;
    }
    default: {
      puur_naa_nka res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ꦮꦭꦶ 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_type (lua_State *L) {
  puur_naa_nka t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ꦪꦼꦤ꧀ (lua_next(L, 1))
    ꦮꦭꦶ 2;
  ꦭꦶꦪ {
    lua_pushnil(L);
    ꦮꦭꦶ 1;
  }
}


static puur_naa_nka pairscont (lua_State *L, puur_naa_nka status, lua_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  ꦮꦭꦶ 4;  /* __pairs did all the work, just return its results */
}

static puur_naa_nka luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  ꦪꦼꦤ꧀ (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  ꦭꦶꦪ {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ꦮꦭꦶ 4;
}


/*
** Traversal function for 'ipairs'
*/
static puur_naa_nka ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ꦮꦭꦶ (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static puur_naa_nka luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ꦮꦭꦶ 3;
}


static puur_naa_nka load_aux (lua_State *L, puur_naa_nka status, puur_naa_nka envidx) {
  ꦪꦼꦤ꧀ (l_likely(status == LUA_OK)) {
    ꦪꦼꦤ꧀ (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ꦪꦼꦤ꧀ (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    ꦮꦭꦶ 1;
  }
  ꦭꦶꦪ {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_insert(L, -2);  /* put before error message */
    ꦮꦭꦶ 2;  /* return fail plus error message */
  }
}


static const var_na *getMode (lua_State *L, puur_naa_nka idx) {
  const var_na *mode = luaL_optstring(L, idx, NULL);
  ꦪꦼꦤ꧀ (mode != NULL && strchr(mode, 'B') != NULL) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ꦮꦭꦶ mode;
}


static puur_naa_nka luaB_loadfile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  const var_na *mode = getMode(L, 2);
  puur_naa_nka env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  puur_naa_nka status = luaL_loadfilex(L, fname, mode);
  ꦮꦭꦶ load_aux(L, status, env);
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
  puur_naa_nka *firstcall = cast(puur_naa_nka *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ꦪꦼꦤ꧀ (*firstcall)
    *firstcall = 0;
  ꦭꦶꦪ
    lua_pop(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ꦪꦼꦤ꧀ (lua_isnil(L, -1)) {
    *size = 0;
    ꦮꦭꦶ NULL;
  }
  ꦭꦶꦪ ꦪꦼꦤ꧀ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ꦮꦭꦶ lua_tolstring(L, -1, size);
}


static puur_naa_nka luaB_load (lua_State *L) {
  puur_naa_nka status;
  size_t l;
  const var_na *s = lua_tolstring(L, 1, &l);
  const var_na *mode = getMode(L, 3);
  puur_naa_nka env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ꦪꦼꦤ꧀ (s != NULL) {  /* loading a string? */
    const var_na *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  ꦭꦶꦪ {  /* loading from a reader function */
    puur_naa_nka firstcall = 1;  /* userdata for generic_reader */
    const var_na *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  ꦮꦭꦶ load_aux(L, status, env);
}

/* }====================================================== */


static puur_naa_nka dofilecont (lua_State *L, puur_naa_nka d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  ꦮꦭꦶ lua_gettop(L) - 1;
}


static puur_naa_nka luaB_dofile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  ꦪꦼꦤ꧀ (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ꦮꦭꦶ lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ꦮꦭꦶ dofilecont(L, 0, 0);
}


static puur_naa_nka luaB_assert (lua_State *L) {
  ꦪꦼꦤ꧀ (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ꦮꦭꦶ lua_gettop(L);  /* return all arguments */
  ꦭꦶꦪ {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ꦮꦭꦶ luaB_error(L);  /* call 'error' */
  }
}


static puur_naa_nka luaB_select (lua_State *L) {
  puur_naa_nka n = lua_gettop(L);
  ꦪꦼꦤ꧀ (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ꦮꦭꦶ 1;
  }
  ꦭꦶꦪ {
    lua_Integer i = luaL_checkinteger(L, 1);
    ꦪꦼꦤ꧀ (i < 0) i = n + i;
    ꦭꦶꦪ ꦪꦼꦤ꧀ (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ꦮꦭꦶ n - (puur_naa_nka)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static puur_naa_nka finishpcall (lua_State *L, puur_naa_nka status, lua_KContext extra) {
  ꦪꦼꦤ꧀ (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ꦮꦭꦶ 2;  /* return false, msg */
  }
  ꦭꦶꦪ
    ꦮꦭꦶ lua_gettop(L) - (puur_naa_nka)extra;  /* return all results */
}


static puur_naa_nka luaB_pcall (lua_State *L) {
  puur_naa_nka status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ꦮꦭꦶ finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static puur_naa_nka luaB_xpcall (lua_State *L) {
  puur_naa_nka status;
  puur_naa_nka n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ꦮꦭꦶ finishpcall(L, status, 2);
}


static puur_naa_nka luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  ꦮꦭꦶ 1;
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


LUAMOD_API puur_naa_nka luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  ꦮꦭꦶ 1;
}

