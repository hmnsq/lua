/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#define ltablib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#define TAB_R	1			/* read */
#define TAB_W	2			/* write */
#define TAB_L	4			/* length */
#define TAB_RW	(TAB_R | TAB_W)		/* read/write */


#define aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


സ്ഥിതം പൂർണ്ണസംഖ്യ checkfield (lua_State *L, സ്ഥിരം അക്ഷരം *താക്കോൽ, പൂർണ്ണസംഖ്യ n) {
  lua_pushstring(L, താക്കോൽ);
  തിരികെ (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
സ്ഥിതം ശൂന്യം checktab (lua_State *L, പൂർണ്ണസംഖ്യ arg, പൂർണ്ണസംഖ്യ what) {
  പൂർണ്ണസംഖ്യ tp = lua_type(L, arg);
  എങ്കിൽ (tp != LUA_TTABLE) {  /* is it not a table? */
    പൂർണ്ണസംഖ്യ n = 1;  /* number of elements to pop */
    എങ്കിൽ (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_എടുക്കുക(L, n);  /* pop metatable and tested metamethods */
    }
    അല്ലെങ്കിൽ
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(INT_MAX), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(INT_MAX), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  തിരഞ്ഞെടുപ്പ് (lua_gettop(L)) {
    അവസ്ഥ 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      നിർത്തുക;
    }
    അവസ്ഥ 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      വേണ്ടി (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരികെ luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tremove (lua_State *L) {
  lua_Integer size = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, size);
  എങ്കിൽ (pos != size)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)size, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  വേണ്ടി ( ; pos < size; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  തിരികെ 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  പൂർണ്ണസംഖ്യ tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  എങ്കിൽ (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    എങ്കിൽ (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      വേണ്ടി (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    അല്ലെങ്കിൽ {
      വേണ്ടി (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  തിരികെ 1;
}


സ്ഥിതം ശൂന്യം addfield (lua_State *L, luaL_Buffer *b, lua_Integer i) {
  lua_geti(L, 1, i);
  എങ്കിൽ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tconcat (lua_State *L) {
  luaL_Buffer b;
  lua_Integer last = aux_getn(L, 1, TAB_R);
  size_t lsep;
  സ്ഥിരം അക്ഷരം *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  വേണ്ടി (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  എങ്കിൽ (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  തിരികെ 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

സ്ഥിതം പൂർണ്ണസംഖ്യ tpack (lua_State *L) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_തിരുകുക(L, 1);  /* put it at index 1 */
  വേണ്ടി (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  തിരികെ 1;  /* return table */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  എങ്കിൽ (i > e) തിരികെ 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  എങ്കിൽ (l_unlikely(n >= (unsigned പൂർണ്ണസംഖ്യ)INT_MAX  ||
                 !lua_checkstack(L, (പൂർണ്ണസംഖ്യ)(++n))))
    തിരികെ luaL_error(L, "too many results to unpack");
  വേണ്ടി (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  തിരികെ (പൂർണ്ണസംഖ്യ)n;
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
തരനിർവചനം unsigned പൂർണ്ണസംഖ്യ IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#define geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#define seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#എങ്കിൽ !defined(l_randomizePivot)
#define l_randomizePivot(L)	luaL_makeseed(L)
#endif					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#define RANLIMIT	100u


സ്ഥിതം ശൂന്യം set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sort_comp (lua_State *L, പൂർണ്ണസംഖ്യ a, പൂർണ്ണസംഖ്യ b) {
  എങ്കിൽ (lua_isnil(L, 2))  /* no function? */
    തിരികെ lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  അല്ലെങ്കിൽ {  /* function */
    പൂർണ്ണസംഖ്യ res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_എടുക്കുക(L, 1);          /* pop result */
    തിരികെ res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
സ്ഥിതം IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  വേണ്ടി (;;) {
    /* next loop: repeat ++i while a[i] < P */
    ആകുമ്പോൾ ((ശൂന്യം)geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      എങ്കിൽ (l_unlikely(i == up - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_എടുക്കുക(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    ആകുമ്പോൾ ((ശൂന്യം)geti(L, 1, --j), sort_comp(L, -3, -1)) {
      എങ്കിൽ (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_എടുക്കുക(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    എങ്കിൽ (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_എടുക്കുക(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, up - 1, i);
      തിരികെ i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
സ്ഥിതം IdxT choosePivot (IdxT lo, IdxT up, unsigned പൂർണ്ണസംഖ്യ rnd) {
  IdxT r4 = (up - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ up) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  തിരികെ p;
}


/*
** Quicksort algorithm (recursive function)
*/
സ്ഥിതം ശൂന്യം auxsort (lua_State *L, IdxT lo, IdxT up, unsigned rnd) {
  ആകുമ്പോൾ (lo < up) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, up);
    എങ്കിൽ (sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, up);  /* swap a[lo] - a[up] */
    അല്ലെങ്കിൽ
      lua_എടുക്കുക(L, 2);  /* remove both values */
    എങ്കിൽ (up - lo == 1)  /* only 2 elements? */
      തിരികെ;  /* already sorted */
    എങ്കിൽ (up - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    അല്ലെങ്കിൽ  /* for larger intervals, it is worth a random pivot */
      p = choosePivot(lo, up, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    എങ്കിൽ (sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    അല്ലെങ്കിൽ {
      lua_എടുക്കുക(L, 1);  /* remove a[lo] */
      geti(L, 1, up);
      എങ്കിൽ (sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, up);  /* swap a[up] - a[p] */
      അല്ലെങ്കിൽ
        lua_എടുക്കുക(L, 2);
    }
    എങ്കിൽ (up - lo == 2)  /* only 3 elements? */
      തിരികെ;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, up - 1);  /* push a[up - 1] */
    set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    എങ്കിൽ (p - lo < up - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    അല്ലെങ്കിൽ {
      auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    എങ്കിൽ ((up - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  എങ്കിൽ (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < INT_MAX, 1, "array too big");
    എങ്കിൽ (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  തിരികെ 0;
}

/* }====================================================== */


സ്ഥിതം സ്ഥിരം luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_table (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  തിരികെ 1;
}

