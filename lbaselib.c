/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_print (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ଅଚଳ ଅକ୍ଷର *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ଯଦି (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_କାଢ଼(L, 1);  /* pop result */
  }
  lua_writeline();
  ଫେରନ୍ତୁ 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_warn (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_checkstring(L, 1);  /* at least one argument */
  ପାଇଁ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  ପାଇଁ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ଫେରନ୍ତୁ 0;
}


#define SPACECHARS	" \f\n\r\t\v"

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *b_str2int (ଅଚଳ ଅକ୍ଷର *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ଯଦି (*s == '-') { s++; neg = 1; }  /* handle sign */
  ଅନ୍ୟଥା ଯଦି (*s == '+') s++;
  ଯଦି (!isalnum(cast_uchar(*s)))  /* no digit? */
    ଫେରନ୍ତୁ ଖାଲି;
  କର {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    ଯଦି (digit >= base) ଫେରନ୍ତୁ ଖାଲି;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } ଯେତେବେଳେ (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ଫେରନ୍ତୁ s;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_tonumber (lua_State *L) {
  ଯଦି (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ଯଦି (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ଫେରନ୍ତୁ 1;
    }
    ଅନ୍ୟଥା {
      size_t l;
      ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, 1, &l);
      ଯଦି (s != ଖାଲି && lua_stringtonumber(L, s) == l + 1)
        ଫେରନ୍ତୁ 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  ଅନ୍ୟଥା {
    size_t l;
    ଅଚଳ ଅକ୍ଷର *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    ଯଦି (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      ଫେରନ୍ତୁ 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_error (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ଯଦି (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ଫେରନ୍ତୁ lua_error(L);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ଯଦି (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ଫେରନ୍ତୁ 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ଫେରନ୍ତୁ 1;  /* returns either __metatable field (if present) or metatable */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_setmetatable (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ଯଦି (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ଫେରନ୍ତୁ luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_rawlen (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pushmode (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldmode) {
  ଯଦି (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  ଅନ୍ୟଥା
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ଫେରନ୍ତୁ 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { ଯଦି (res == -1) ଭାଙ୍ଗ; }

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_collectgarbage (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ଖାଲି};
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  ବାଛ (o) {
    ସ୍ଥିତି LUA_GCCOUNT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = lua_gc(L, o);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_ସଂଖ୍ୟା)k + ((lua_ସଂଖ୍ୟା)b/1024));
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି LUA_GCISRUNNING: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି LUA_GCGEN: {
      ଫେରନ୍ତୁ pushmode(L, lua_gc(L, o));
    }
    ସ୍ଥିତି LUA_GCINC: {
      ଫେରନ୍ତୁ pushmode(L, lua_gc(L, o));
    }
    ସ୍ଥିତି LUA_GCPARAM: {
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ଖାଲି};
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା p = pnum[luaL_checkoption(L, 2, ଖାଲି, params)];
      lua_Integer ମୂଲ୍ୟ = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ମୂଲ୍ୟ));
      ଫେରନ୍ତୁ 1;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ଫେରନ୍ତୁ 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_type (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ଯଦି (lua_next(L, 1))
    ଫେରନ୍ତୁ 2;
  ଅନ୍ୟଥା {
    lua_pushnil(L);
    ଫେରନ୍ତୁ 1;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pairscont (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, lua_KContext k) {
  (ଶୂନ୍ୟ)L; (ଶୂନ୍ୟ)status; (ଶୂନ୍ୟ)k;  /* unused */
  ଫେରନ୍ତୁ 4;  /* __pairs did all the work, just return its results */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  ଯଦି (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  ଅନ୍ୟଥା {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ଫେରନ୍ତୁ 4;
}


/*
** Traversal function for 'ipairs'
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ଫେରନ୍ତୁ (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ଫେରନ୍ତୁ 3;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା load_aux (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, ପୂର୍ଣ୍ଣସଂଖ୍ୟା envidx) {
  ଯଦି (l_likely(status == LUA_OK)) {
    ଯଦି (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ଯଦି (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_କାଢ଼(L, 1);  /* remove 'env' if not used by previous call */
    }
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_ଭର୍ତ୍ତି(L, -2);  /* put before error message */
    ଫେରନ୍ତୁ 2;  /* return fail plus error message */
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getMode (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ ଅକ୍ଷର *mode = luaL_optstring(L, idx, ଖାଲି);
  ଯଦି (mode != ଖାଲି && strchr(mode, 'B') != ଖାଲି) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ଫେରନ୍ତୁ mode;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_loadfile (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *fname = luaL_optstring(L, 1, ଖାଲି);
  ଅଚଳ ଅକ୍ଷର *mode = getMode(L, 2);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = luaL_loadfilex(L, fname, mode);
  ଫେରନ୍ତୁ load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *generic_reader (lua_State *L, ଶୂନ୍ୟ *ud, size_t *size) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *firstcall = cast(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ଯଦି (*firstcall)
    *firstcall = 0;
  ଅନ୍ୟଥା
    lua_କାଢ଼(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ଯଦି (lua_isnil(L, -1)) {
    *size = 0;
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଅନ୍ୟଥା ଯଦି (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ଫେରନ୍ତୁ lua_tolstring(L, -1, size);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_load (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  size_t l;
  ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, 1, &l);
  ଅଚଳ ଅକ୍ଷର *mode = getMode(L, 3);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ଯଦି (s != ଖାଲି) {  /* loading a string? */
    ଅଚଳ ଅକ୍ଷର *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  ଅନ୍ୟଥା {  /* loading from a reader function */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstcall = 1;  /* userdata for generic_reader */
    ଅଚଳ ଅକ୍ଷର *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  ଫେରନ୍ତୁ load_aux(L, status, env);
}

/* }====================================================== */


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dofilecont (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା d1, lua_KContext d2) {
  (ଶୂନ୍ୟ)d1;  (ଶୂନ୍ୟ)d2;  /* only to match 'lua_Kfunction' prototype */
  ଫେରନ୍ତୁ lua_gettop(L) - 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_dofile (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *fname = luaL_optstring(L, 1, ଖାଲି);
  lua_settop(L, 1);
  ଯଦି (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ଫେରନ୍ତୁ lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ଫେରନ୍ତୁ dofilecont(L, 0, 0);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_assert (lua_State *L) {
  ଯଦି (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ଫେରନ୍ତୁ lua_gettop(L);  /* return all arguments */
  ଅନ୍ୟଥା {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ଫେରନ୍ତୁ luaB_error(L);  /* call 'error' */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_select (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);
  ଯଦି (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {
    lua_Integer i = luaL_checkinteger(L, 1);
    ଯଦି (i < 0) i = n + i;
    ଅନ୍ୟଥା ଯଦି (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ଫେରନ୍ତୁ n - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା finishpcall (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, lua_KContext extra) {
  ଯଦି (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ଫେରନ୍ତୁ 2;  /* return false, msg */
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ lua_gettop(L) - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)extra;  /* return all results */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_pcall (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_ଭର୍ତ୍ତି(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ଫେରନ୍ତୁ finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_xpcall (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ଫେରନ୍ତୁ finishpcall(L, status, 2);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ଖାଲି);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଅଚଳ luaL_Reg base_funcs[] = {
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
  {LUA_GNAME, ଖାଲି},
  {"_VERSION", ଖାଲି},
  {ଖାଲି, ଖାଲି}
};


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  ଫେରନ୍ତୁ 1;
}

