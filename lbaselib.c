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


static จำนวนเต็ม luaB_print (lua_State *L) {
  จำนวนเต็ม n = lua_gettop(L);  /* number of arguments */
  จำนวนเต็ม i;
  สำหรับ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const var_na *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ถ้า (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  คืน 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static จำนวนเต็ม luaB_warn (lua_State *L) {
  จำนวนเต็ม n = lua_gettop(L);  /* number of arguments */
  จำนวนเต็ม i;
  luaL_checkstring(L, 1);  /* at least one argument */
  สำหรับ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  สำหรับ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  คืน 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const var_na *b_str2int (const var_na *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  จำนวนเต็ม neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ถ้า (*s == '-') { s++; neg = 1; }  /* handle sign */
  มิฉะนั้น ถ้า (*s == '+') s++;
  ถ้า (!isalnum(cast_uchar(*s)))  /* no digit? */
    คืน NULL;
  do {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    ถ้า (digit >= base) คืน NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } ขณะ (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  คืน s;
}


static จำนวนเต็ม luaB_tonumber (lua_State *L) {
  ถ้า (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ถ้า (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      คืน 1;
    }
    มิฉะนั้น {
      size_t l;
      const var_na *s = lua_tolstring(L, 1, &l);
      ถ้า (s != NULL && lua_stringtonumber(L, s) == l + 1)
        คืน 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  มิฉะนั้น {
    size_t l;
    const var_na *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    ถ้า (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      คืน 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  คืน 1;
}


static จำนวนเต็ม luaB_error (lua_State *L) {
  จำนวนเต็ม level = (จำนวนเต็ม)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ถ้า (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  คืน lua_error(L);
}


static จำนวนเต็ม luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ถ้า (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    คืน 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  คืน 1;  /* returns either __metatable field (if present) or metatable */
}


static จำนวนเต็ม luaB_setmetatable (lua_State *L) {
  จำนวนเต็ม t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ถ้า (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    คืน luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  คืน 1;
}


static จำนวนเต็ม luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  คืน 1;
}


static จำนวนเต็ม luaB_rawlen (lua_State *L) {
  จำนวนเต็ม t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  คืน 1;
}


static จำนวนเต็ม luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  คืน 1;
}

static จำนวนเต็ม luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  คืน 1;
}


static จำนวนเต็ม pushmode (lua_State *L, จำนวนเต็ม oldmode) {
  ถ้า (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  มิฉะนั้น
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  คืน 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { ถ้า (res == -1) bha_nga; }

static จำนวนเต็ม luaB_collectgarbage (lua_State *L) {
  static const var_na *const opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", NULL};
  static const var_na optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  จำนวนเต็ม o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    pak_sa LUA_GCCOUNT: {
      จำนวนเต็ม k = lua_gc(L, o);
      จำนวนเต็ม b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_Number)k + ((lua_Number)b/1024));
      คืน 1;
    }
    pak_sa LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      จำนวนเต็ม res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      คืน 1;
    }
    pak_sa LUA_GCISRUNNING: {
      จำนวนเต็ม res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      คืน 1;
    }
    pak_sa LUA_GCGEN: {
      คืน pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCINC: {
      คืน pushmode(L, lua_gc(L, o));
    }
    pak_sa LUA_GCPARAM: {
      static const var_na *const params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", NULL};
      static const var_na pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      จำนวนเต็ม p = pnum[luaL_checkoption(L, 2, NULL, params)];
      lua_Integer value = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (จำนวนเต็ม)value));
      คืน 1;
    }
    default: {
      จำนวนเต็ม res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      คืน 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  คืน 1;
}


static จำนวนเต็ม luaB_type (lua_State *L) {
  จำนวนเต็ม t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  คืน 1;
}


static จำนวนเต็ม luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ถ้า (lua_next(L, 1))
    คืน 2;
  มิฉะนั้น {
    lua_pushnil(L);
    คืน 1;
  }
}


static จำนวนเต็ม pairscont (lua_State *L, จำนวนเต็ม status, lua_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  คืน 4;  /* __pairs did all the work, just return its results */
}

static จำนวนเต็ม luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  ถ้า (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  มิฉะนั้น {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  คืน 4;
}


/*
** Traversal function for 'ipairs'
*/
static จำนวนเต็ม ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  คืน (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static จำนวนเต็ม luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  คืน 3;
}


static จำนวนเต็ม load_aux (lua_State *L, จำนวนเต็ม status, จำนวนเต็ม envidx) {
  ถ้า (l_likely(status == LUA_OK)) {
    ถ้า (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ถ้า (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    คืน 1;
  }
  มิฉะนั้น {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_insert(L, -2);  /* put before error message */
    คืน 2;  /* return fail plus error message */
  }
}


static const var_na *getMode (lua_State *L, จำนวนเต็ม idx) {
  const var_na *mode = luaL_optstring(L, idx, NULL);
  ถ้า (mode != NULL && strchr(mode, 'B') != NULL) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  คืน mode;
}


static จำนวนเต็ม luaB_loadfile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  const var_na *mode = getMode(L, 2);
  จำนวนเต็ม env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  จำนวนเต็ม status = luaL_loadfilex(L, fname, mode);
  คืน load_aux(L, status, env);
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
  จำนวนเต็ม *firstcall = cast(จำนวนเต็ม *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ถ้า (*firstcall)
    *firstcall = 0;
  มิฉะนั้น
    lua_pop(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ถ้า (lua_isnil(L, -1)) {
    *size = 0;
    คืน NULL;
  }
  มิฉะนั้น ถ้า (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  คืน lua_tolstring(L, -1, size);
}


static จำนวนเต็ม luaB_load (lua_State *L) {
  จำนวนเต็ม status;
  size_t l;
  const var_na *s = lua_tolstring(L, 1, &l);
  const var_na *mode = getMode(L, 3);
  จำนวนเต็ม env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ถ้า (s != NULL) {  /* loading a string? */
    const var_na *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  มิฉะนั้น {  /* loading from a reader function */
    จำนวนเต็ม firstcall = 1;  /* userdata for generic_reader */
    const var_na *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  คืน load_aux(L, status, env);
}

/* }====================================================== */


static จำนวนเต็ม dofilecont (lua_State *L, จำนวนเต็ม d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'lua_Kfunction' prototype */
  คืน lua_gettop(L) - 1;
}


static จำนวนเต็ม luaB_dofile (lua_State *L) {
  const var_na *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  ถ้า (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    คืน lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  คืน dofilecont(L, 0, 0);
}


static จำนวนเต็ม luaB_assert (lua_State *L) {
  ถ้า (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    คืน lua_gettop(L);  /* return all arguments */
  มิฉะนั้น {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    คืน luaB_error(L);  /* call 'error' */
  }
}


static จำนวนเต็ม luaB_select (lua_State *L) {
  จำนวนเต็ม n = lua_gettop(L);
  ถ้า (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    คืน 1;
  }
  มิฉะนั้น {
    lua_Integer i = luaL_checkinteger(L, 1);
    ถ้า (i < 0) i = n + i;
    มิฉะนั้น ถ้า (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    คืน n - (จำนวนเต็ม)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static จำนวนเต็ม finishpcall (lua_State *L, จำนวนเต็ม status, lua_KContext extra) {
  ถ้า (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    คืน 2;  /* return false, msg */
  }
  มิฉะนั้น
    คืน lua_gettop(L) - (จำนวนเต็ม)extra;  /* return all results */
}


static จำนวนเต็ม luaB_pcall (lua_State *L) {
  จำนวนเต็ม status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  คืน finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static จำนวนเต็ม luaB_xpcall (lua_State *L) {
  จำนวนเต็ม status;
  จำนวนเต็ม n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  คืน finishpcall(L, status, 2);
}


static จำนวนเต็ม luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  คืน 1;
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


LUAMOD_API จำนวนเต็ม luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  คืน 1;
}

