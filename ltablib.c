/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#define ltablib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#define TAB_R	1			/* read */
#define TAB_W	2			/* write */
#define TAB_L	4			/* length */
#define TAB_RW	(TAB_R | TAB_W)		/* read/write */


#define aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkfield (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ n) {
  lua_pushstring(L, ಕೀಲಿ);
  ಪ್ರತ್ಯಾಗಮ (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
ಸ್ಥಿರ ಶೂನ್ಯ checktab (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಪೂರ್ಣಾಙ್ಕ what) {
  ಪೂರ್ಣಾಙ್ಕ tp = lua_type(L, arg);
  ಯದಿ (tp != LUA_TTABLE) {  /* is it not a table? */
    ಪೂರ್ಣಾಙ್ಕ n = 1;  /* number of elements to pop */
    ಯದಿ (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_ತೆಗೆ(L, n);  /* pop metatable and tested metamethods */
    }
    ಅನ್ಯಥಾ
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(INT_MAX), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(INT_MAX), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  ವಿಕಲ್ಪ (lua_gettop(L)) {
    ಪಕ್ಷ 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      ಪ್ರತಿ (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tremove (lua_State *L) {
  lua_Integer size = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, size);
  ಯದಿ (pos != size)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)size, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  ಪ್ರತಿ ( ; pos < size; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  ಪೂರ್ಣಾಙ್ಕ tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  ಯದಿ (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    ಯದಿ (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      ಪ್ರತಿ (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    ಅನ್ಯಥಾ {
      ಪ್ರತಿ (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ addfield (lua_State *L, luaL_Buffer *b, lua_Integer i) {
  lua_geti(L, 1, i);
  ಯದಿ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tconcat (lua_State *L) {
  luaL_Buffer b;
  lua_Integer last = aux_getn(L, 1, TAB_R);
  size_t lsep;
  ಧ್ರುವ ವರ್ಣ *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  ಪ್ರತಿ (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  ಯದಿ (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tpack (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_ಸೇರಿಸು(L, 1);  /* put it at index 1 */
  ಪ್ರತಿ (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  ಪ್ರತ್ಯಾಗಮ 1;  /* return table */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  ಯದಿ (i > e) ಪ್ರತ್ಯಾಗಮ 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  ಯದಿ (l_unlikely(n >= (unsigned ಪೂರ್ಣಾಙ್ಕ)INT_MAX  ||
                 !lua_checkstack(L, (ಪೂರ್ಣಾಙ್ಕ)(++n))))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "too many results to unpack");
  ಪ್ರತಿ (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)n;
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
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ಪೂರ್ಣಾಙ್ಕ IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#define geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#define seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#ಯದಿ !defined(l_randomizePivot)
#define l_randomizePivot(L)	luaL_makeseed(L)
#endif					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#define RANLIMIT	100u


ಸ್ಥಿರ ಶೂನ್ಯ set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sort_comp (lua_State *L, ಪೂರ್ಣಾಙ್ಕ a, ಪೂರ್ಣಾಙ್ಕ b) {
  ಯದಿ (lua_isnil(L, 2))  /* no function? */
    ಪ್ರತ್ಯಾಗಮ lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  ಅನ್ಯಥಾ {  /* function */
    ಪೂರ್ಣಾಙ್ಕ res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_ತೆಗೆ(L, 1);          /* pop result */
    ಪ್ರತ್ಯಾಗಮ res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
ಸ್ಥಿರ IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  ಪ್ರತಿ (;;) {
    /* next loop: repeat ++i while a[i] < P */
    ಯಾವತ್ ((ಶೂನ್ಯ)geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      ಯದಿ (l_unlikely(i == up - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_ತೆಗೆ(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    ಯಾವತ್ ((ಶೂನ್ಯ)geti(L, 1, --j), sort_comp(L, -3, -1)) {
      ಯದಿ (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_ತೆಗೆ(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    ಯದಿ (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_ತೆಗೆ(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, up - 1, i);
      ಪ್ರತ್ಯಾಗಮ i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
ಸ್ಥಿರ IdxT choosePivot (IdxT lo, IdxT up, unsigned ಪೂರ್ಣಾಙ್ಕ rnd) {
  IdxT r4 = (up - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ up) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  ಪ್ರತ್ಯಾಗಮ p;
}


/*
** Quicksort algorithm (recursive function)
*/
ಸ್ಥಿರ ಶೂನ್ಯ auxsort (lua_State *L, IdxT lo, IdxT up, unsigned rnd) {
  ಯಾವತ್ (lo < up) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, up);
    ಯದಿ (sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, up);  /* swap a[lo] - a[up] */
    ಅನ್ಯಥಾ
      lua_ತೆಗೆ(L, 2);  /* remove both values */
    ಯದಿ (up - lo == 1)  /* only 2 elements? */
      ಪ್ರತ್ಯಾಗಮ;  /* already sorted */
    ಯದಿ (up - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    ಅನ್ಯಥಾ  /* for larger intervals, it is worth a random pivot */
      p = choosePivot(lo, up, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    ಯದಿ (sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    ಅನ್ಯಥಾ {
      lua_ತೆಗೆ(L, 1);  /* remove a[lo] */
      geti(L, 1, up);
      ಯದಿ (sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, up);  /* swap a[up] - a[p] */
      ಅನ್ಯಥಾ
        lua_ತೆಗೆ(L, 2);
    }
    ಯದಿ (up - lo == 2)  /* only 3 elements? */
      ಪ್ರತ್ಯಾಗಮ;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, up - 1);  /* push a[up - 1] */
    set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    ಯದಿ (p - lo < up - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    ಅನ್ಯಥಾ {
      auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    ಯದಿ ((up - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  ಯದಿ (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < INT_MAX, 1, "array too big");
    ಯದಿ (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* }====================================================== */


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_table (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  ಪ್ರತ್ಯಾಗಮ 1;
}

