/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#define ltablib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#define TAB_R	1			/* read */
#define TAB_W	2			/* write */
#define TAB_L	4			/*দৈর্ঘ্য*/
#define TAB_RW	(TAB_R | TAB_W)		/* read/write */


#define aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


স্থির পূর্ণসংখ্যা checkfield (lua_State *L, ধ্রুবক অক্ষর *চাবি, পূর্ণসংখ্যা n) {
  lua_pushstring(L, চাবি);
  ফেরত (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
স্থির শূন্য checktab (lua_State *L, পূর্ণসংখ্যা arg, পূর্ণসংখ্যা what) {
  পূর্ণসংখ্যা tp = lua_type(L, arg);
  যদি (tp != LUA_TTABLE) {  /* is it not a table? */
    পূর্ণসংখ্যা n = 1;  /* number of elements to pop */
    যদি (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_বের_করো(L, n);  /* pop metatable and tested metamethods */
    }
    অন্যথা
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


স্থির পূর্ণসংখ্যা tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(INT_MAX), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(INT_MAX), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  নির্বাচন (lua_gettop(L)) {
    ক্ষেত্র 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      বিরতি;
    }
    ক্ষেত্র 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      জন্য (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      ফেরত luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা tremove (lua_State *L) {
  lua_Integer আকার_2 = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, আকার_2);
  যদি (pos != আকার_2)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)আকার_2, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  জন্য ( ; pos < আকার_2; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  ফেরত 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
স্থির পূর্ণসংখ্যা tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  পূর্ণসংখ্যা tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  যদি (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    যদি (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      জন্য (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    অন্যথা {
      জন্য (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  ফেরত 1;
}


স্থির শূন্য addfield (lua_State *L, lua_L_বাফার *b, lua_Integer i) {
  lua_geti(L, 1, i);
  যদি (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


স্থির পূর্ণসংখ্যা tconcat (lua_State *L) {
  lua_L_বাফার b;
  lua_Integer last = aux_getn(L, 1, TAB_R);
  size_t lsep;
  ধ্রুবক অক্ষর *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  জন্য (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  যদি (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  ফেরত 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

স্থির পূর্ণসংখ্যা tpack (lua_State *L) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_ঢোকাও(L, 1);  /* put it at index 1 */
  জন্য (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  ফেরত 1;  /* return table */
}


স্থির পূর্ণসংখ্যা tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  যদি (i > e) ফেরত 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  যদি (l_unlikely(n >= (unsigned পূর্ণসংখ্যা)INT_MAX  ||
                 !lua_checkstack(L, (পূর্ণসংখ্যা)(++n))))
    ফেরত luaL_error(L, "too many results to unpack");
  জন্য (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  ফেরত (পূর্ণসংখ্যা)n;
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
প্রকারলেখ unsigned পূর্ণসংখ্যা IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#define geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#define seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#যদি !defined(l_randomizePivot)
#define l_randomizePivot(L)	luaL_makeseed(L)
#endif					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#define RANLIMIT	100u


স্থির শূন্য set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
স্থির পূর্ণসংখ্যা sort_comp (lua_State *L, পূর্ণসংখ্যা a, পূর্ণসংখ্যা b) {
  যদি (lua_isnil(L, 2))  /* no function? */
    ফেরত lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  অন্যথা {  /* function */
    পূর্ণসংখ্যা res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_বের_করো(L, 1);          /* pop result */
    ফেরত res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
স্থির IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  জন্য (;;) {
    /* next loop: repeat ++i while a[i] < P */
    যখন ((শূন্য)geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      যদি (l_unlikely(i == up - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_বের_করো(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    যখন ((শূন্য)geti(L, 1, --j), sort_comp(L, -3, -1)) {
      যদি (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_বের_করো(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    যদি (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_বের_করো(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, up - 1, i);
      ফেরত i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
স্থির IdxT choosePivot (IdxT lo, IdxT up, unsigned পূর্ণসংখ্যা rnd) {
  IdxT r4 = (up - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ up) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  ফেরত p;
}


/*
** Quicksort algorithm (recursive function)
*/
স্থির শূন্য auxsort (lua_State *L, IdxT lo, IdxT up, unsigned rnd) {
  যখন (lo < up) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, up);
    যদি (sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, up);  /* swap a[lo] - a[up] */
    অন্যথা
      lua_বের_করো(L, 2);  /* remove both values */
    যদি (up - lo == 1)  /* only 2 elements? */
      ফেরত;  /* already sorted */
    যদি (up - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    অন্যথা  /* for larger intervals, it is worth a random pivot */
      p = choosePivot(lo, up, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    যদি (sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    অন্যথা {
      lua_বের_করো(L, 1);  /* remove a[lo] */
      geti(L, 1, up);
      যদি (sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, up);  /* swap a[up] - a[p] */
      অন্যথা
        lua_বের_করো(L, 2);
    }
    যদি (up - lo == 2)  /* only 3 elements? */
      ফেরত;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, up - 1);  /* push a[up - 1] */
    set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    যদি (p - lo < up - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    অন্যথা {
      auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    যদি ((up - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


স্থির পূর্ণসংখ্যা sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  যদি (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < INT_MAX, 1, "array too big");
    যদি (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  ফেরত 0;
}

/* }====================================================== */


স্থির ধ্রুবক luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_table (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  ফেরত 1;
}

