/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltablib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAB_R	1			/* read */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAB_W	2			/* write */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAB_L	4			/*ਲੰਮਾਈ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TAB_RW	(TAB_R | TAB_W)		/* read/write */


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


ਸਥਿਰ ਪੂਰਨ checkfield (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ, ਪੂਰਨ n) {
  lua_pushstring(L, ਕੁੰਜੀ);
  ਵਾਪਸ (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
ਸਥਿਰ ਖਾਲੀ checktab (lua_State *L, ਪੂਰਨ arg, ਪੂਰਨ what) {
  ਪੂਰਨ tp = lua_type(L, arg);
  ਜੇ (tp != LUA_TTABLE) {  /* is it not a table? */
    ਪੂਰਨ n = 1;  /* number of elements to pop */
    ਜੇ (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_ਕੱਢੋ(L, n);  /* pop metatable and tested metamethods */
    }
    ਨਹੀਂਤਾਂ
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


ਸਥਿਰ ਪੂਰਨ tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  ਚੋਣ (lua_gettop(L)) {
    ਹਾਲਤ 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      ਰੋਕੋ;
    }
    ਹਾਲਤ 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      ਲਈ (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਵਾਪਸ luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ tremove (lua_State *L) {
  lua_Integer ਮਾਪ = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, ਮਾਪ);
  ਜੇ (pos != ਮਾਪ)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)ਮਾਪ, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  ਲਈ ( ; pos < ਮਾਪ; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  ਵਾਪਸ 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
ਸਥਿਰ ਪੂਰਨ tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  ਪੂਰਨ tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  ਜੇ (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    ਜੇ (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      ਲਈ (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    ਨਹੀਂਤਾਂ {
      ਲਈ (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਖਾਲੀ addfield (lua_State *L, lua_L_ਬਫਰ *b, lua_Integer i) {
  lua_geti(L, 1, i);
  ਜੇ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


ਸਥਿਰ ਪੂਰਨ tconcat (lua_State *L) {
  lua_L_ਬਫਰ b;
  lua_Integer ਆਖਰੀ = aux_getn(L, 1, TAB_R);
  size_t lsep;
  ਅਚਲ ਅੱਖਰ *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  ਆਖਰੀ = luaL_optinteger(L, 4, ਆਖਰੀ);
  luaL_buffinit(L, &b);
  ਲਈ (; i < ਆਖਰੀ; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  ਜੇ (i == ਆਖਰੀ)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  ਵਾਪਸ 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

ਸਥਿਰ ਪੂਰਨ tpack (lua_State *L) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_ਪਾਓ(L, 1);  /* put it at index 1 */
  ਲਈ (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  ਵਾਪਸ 1;  /* return table */
}


ਸਥਿਰ ਪੂਰਨ tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  ਜੇ (i > e) ਵਾਪਸ 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  ਜੇ (l_unlikely(n >= (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ  ||
                 !lua_checkstack(L, (ਪੂਰਨ)(++n))))
    ਵਾਪਸ luaL_error(L, "too many results to unpack");
  ਲਈ (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  ਵਾਪਸ (ਪੂਰਨ)n;
}

/* }====================================================== */



/*
** {======================================================
** Quicksort
** (based on 'Algorithms in MODULA-3', Robert Sedgewick;
**  Addison-Wesley, 1993.)
** =======================================================
*/


/*
** Type for array indices. These indices are always limited by INT_MAX,
** so it is safe to cast them to lua_Integer even for Lua 32 bits.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#ਜੇ !defined(l_randomize_ਧੁਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_randomize_ਧੁਰਾ(L)	luaL_makeseed(L)
#ਜੇ_ਅੰਤ					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RANLIMIT	100u


ਸਥਿਰ ਖਾਲੀ set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
ਸਥਿਰ ਪੂਰਨ ਛਾਂਟੋ_comp (lua_State *L, ਪੂਰਨ a, ਪੂਰਨ b) {
  ਜੇ (lua_isnil(L, 2))  /* no function? */
    ਵਾਪਸ lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  ਨਹੀਂਤਾਂ {  /* function */
    ਪੂਰਨ res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_ਕੱਢੋ(L, 1);          /* pop result */
    ਵਾਪਸ res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
ਸਥਿਰ IdxT ਹਿੱਸੇ_ਕਰੋ (lua_State *L, IdxT lo, IdxT ਉੱਪਰ) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = ਉੱਪਰ - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  ਲਈ (;;) {
    /* next loop: repeat ++i while a[i] < P */
    ਜਦੋਂ ((ਖਾਲੀ)geti(L, 1, ++i), ਛਾਂਟੋ_comp(L, -1, -2)) {
      ਜੇ (l_unlikely(i == ਉੱਪਰ - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_ਕੱਢੋ(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    ਜਦੋਂ ((ਖਾਲੀ)geti(L, 1, --j), ਛਾਂਟੋ_comp(L, -3, -1)) {
      ਜੇ (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_ਕੱਢੋ(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    ਜੇ (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_ਕੱਢੋ(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, ਉੱਪਰ - 1, i);
      ਵਾਪਸ i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
ਸਥਿਰ IdxT ਚੁਣੋ_ਧੁਰਾ (IdxT lo, IdxT ਉੱਪਰ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ rnd) {
  IdxT r4 = (ਉੱਪਰ - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ ਉੱਪਰ) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= ਉੱਪਰ - r4);
  ਵਾਪਸ p;
}


/*
** Quicksort algorithm (recursive function)
*/
ਸਥਿਰ ਖਾਲੀ auxsort (lua_State *L, IdxT lo, IdxT ਉੱਪਰ, ਬਿਨਾਨਿਸ਼ਾਨੀ rnd) {
  ਜਦੋਂ (lo < ਉੱਪਰ) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, ਉੱਪਰ);
    ਜੇ (ਛਾਂਟੋ_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, ਉੱਪਰ);  /* swap a[lo] - a[up] */
    ਨਹੀਂਤਾਂ
      lua_ਕੱਢੋ(L, 2);  /* remove both values */
    ਜੇ (ਉੱਪਰ - lo == 1)  /* only 2 elements? */
      ਵਾਪਸ;  /* already sorted */
    ਜੇ (ਉੱਪਰ - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + ਉੱਪਰ)/2;  /* middle element is a good pivot */
    ਨਹੀਂਤਾਂ  /* for larger intervals, it is worth a random pivot */
      p = ਚੁਣੋ_ਧੁਰਾ(lo, ਉੱਪਰ, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    ਜੇ (ਛਾਂਟੋ_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    ਨਹੀਂਤਾਂ {
      lua_ਕੱਢੋ(L, 1);  /* remove a[lo] */
      geti(L, 1, ਉੱਪਰ);
      ਜੇ (ਛਾਂਟੋ_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, ਉੱਪਰ);  /* swap a[up] - a[p] */
      ਨਹੀਂਤਾਂ
        lua_ਕੱਢੋ(L, 2);
    }
    ਜੇ (ਉੱਪਰ - lo == 2)  /* only 3 elements? */
      ਵਾਪਸ;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, ਉੱਪਰ - 1);  /* push a[up - 1] */
    set2(L, p, ਉੱਪਰ - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = ਹਿੱਸੇ_ਕਰੋ(L, lo, ਉੱਪਰ);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    ਜੇ (p - lo < ਉੱਪਰ - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    ਨਹੀਂਤਾਂ {
      auxsort(L, p + 1, ਉੱਪਰ, rnd);  /* call recursively for upper interval */
      n = ਉੱਪਰ - p;  /* size of smaller interval */
      ਉੱਪਰ = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    ਜੇ ((ਉੱਪਰ - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomize_ਧੁਰਾ(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


ਸਥਿਰ ਪੂਰਨ ਛਾਂਟੋ (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  ਜੇ (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, 1, "array too big");
    ਜੇ (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  ਵਾਪਸ 0;
}

/* }====================================================== */


ਸਥਿਰ ਅਚਲ luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", ਛਾਂਟੋ},
  {ਨਿਰਾ, ਨਿਰਾ}
};


LUAMOD_API ਪੂਰਨ luaopen_ਸਾਰਣੀ (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  ਵਾਪਸ 1;
}

