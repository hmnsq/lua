/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#నిర్వచించు ltablib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#నిర్వచించు TAB_R	1			/* read */
#నిర్వచించు TAB_W	2			/* write */
#నిర్వచించు TAB_L	4			/*నిడివి*/
#నిర్వచించు TAB_RW	(TAB_R | TAB_W)		/* read/write */


#నిర్వచించు aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


స్థిర పూర్ణాంకం checkfield (lua_State *L, స్థిరరాశి అక్షరం *కీలకం, పూర్ణాంకం n) {
  lua_pushstring(L, కీలకం);
  తిరిగి (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
స్థిర శూన్య checktab (lua_State *L, పూర్ణాంకం arg, పూర్ణాంకం what) {
  పూర్ణాంకం tp = lua_type(L, arg);
  ఒకవేళ (tp != LUA_TTABLE) {  /* is it not a table? */
    పూర్ణాంకం n = 1;  /* number of elements to pop */
    ఒకవేళ (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_తీయి(L, n);  /* pop metatable and tested metamethods */
    }
    లేకపోతే
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


స్థిర పూర్ణాంకం tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(గరిష్ఠ_పూర్ణాంకం), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(గరిష్ఠ_పూర్ణాంకం), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  ఎంపిక (lua_gettop(L)) {
    స్థితి 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      ఆపు;
    }
    స్థితి 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      కొరకు (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      ఆపు;
    }
    అప్రమేయ: {
      తిరిగి luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  తిరిగి 0;
}


స్థిర పూర్ణాంకం tremove (lua_State *L) {
  lua_Integer కొలత = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, కొలత);
  ఒకవేళ (pos != కొలత)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)కొలత, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  కొరకు ( ; pos < కొలత; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  తిరిగి 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
స్థిర పూర్ణాంకం tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  పూర్ణాంకం tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  ఒకవేళ (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    ఒకవేళ (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      కొరకు (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    లేకపోతే {
      కొరకు (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  తిరిగి 1;
}


స్థిర శూన్య addfield (lua_State *L, lua_L_బఫరు *b, lua_Integer i) {
  lua_geti(L, 1, i);
  ఒకవేళ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


స్థిర పూర్ణాంకం tconcat (lua_State *L) {
  lua_L_బఫరు b;
  lua_Integer చివరి = aux_getn(L, 1, TAB_R);
  size_t lsep;
  స్థిరరాశి అక్షరం *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  చివరి = luaL_optinteger(L, 4, చివరి);
  luaL_buffinit(L, &b);
  కొరకు (; i < చివరి; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  ఒకవేళ (i == చివరి)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  తిరిగి 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

స్థిర పూర్ణాంకం tpack (lua_State *L) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_చొప్పించు(L, 1);  /* put it at index 1 */
  కొరకు (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  తిరిగి 1;  /* return table */
}


స్థిర పూర్ణాంకం tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  ఒకవేళ (i > e) తిరిగి 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  ఒకవేళ (l_unlikely(n >= (గుర్తులేని పూర్ణాంకం)గరిష్ఠ_పూర్ణాంకం  ||
                 !lua_checkstack(L, (పూర్ణాంకం)(++n))))
    తిరిగి luaL_error(L, "too many results to unpack");
  కొరకు (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  తిరిగి (పూర్ణాంకం)n;
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
రకనిర్వచనం గుర్తులేని పూర్ణాంకం IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#నిర్వచించు geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#నిర్వచించు seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#ఒకవేళ !defined(l_randomize_ఇరుసు)
#నిర్వచించు l_randomize_ఇరుసు(L)	luaL_makeseed(L)
#అయితే_ముగింపు					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#నిర్వచించు RANLIMIT	100u


స్థిర శూన్య set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
స్థిర పూర్ణాంకం క్రమీకరించు_comp (lua_State *L, పూర్ణాంకం a, పూర్ణాంకం b) {
  ఒకవేళ (lua_isnil(L, 2))  /* no function? */
    తిరిగి lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  లేకపోతే {  /* function */
    పూర్ణాంకం res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_తీయి(L, 1);          /* pop result */
    తిరిగి res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
స్థిర IdxT విభజన (lua_State *L, IdxT lo, IdxT పైకి) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = పైకి - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  కొరకు (;;) {
    /* next loop: repeat ++i while a[i] < P */
    అయితే ((శూన్య)geti(L, 1, ++i), క్రమీకరించు_comp(L, -1, -2)) {
      ఒకవేళ (l_unlikely(i == పైకి - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_తీయి(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    అయితే ((శూన్య)geti(L, 1, --j), క్రమీకరించు_comp(L, -3, -1)) {
      ఒకవేళ (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_తీయి(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    ఒకవేళ (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_తీయి(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, పైకి - 1, i);
      తిరిగి i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
స్థిర IdxT ఎంచుకో_ఇరుసు (IdxT lo, IdxT పైకి, గుర్తులేని పూర్ణాంకం rnd) {
  IdxT r4 = (పైకి - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ పైకి) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= పైకి - r4);
  తిరిగి p;
}


/*
** Quicksort algorithm (recursive function)
*/
స్థిర శూన్య auxsort (lua_State *L, IdxT lo, IdxT పైకి, గుర్తులేని rnd) {
  అయితే (lo < పైకి) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, పైకి);
    ఒకవేళ (క్రమీకరించు_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, పైకి);  /* swap a[lo] - a[up] */
    లేకపోతే
      lua_తీయి(L, 2);  /* remove both values */
    ఒకవేళ (పైకి - lo == 1)  /* only 2 elements? */
      తిరిగి;  /* already sorted */
    ఒకవేళ (పైకి - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + పైకి)/2;  /* middle element is a good pivot */
    లేకపోతే  /* for larger intervals, it is worth a random pivot */
      p = ఎంచుకో_ఇరుసు(lo, పైకి, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    ఒకవేళ (క్రమీకరించు_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    లేకపోతే {
      lua_తీయి(L, 1);  /* remove a[lo] */
      geti(L, 1, పైకి);
      ఒకవేళ (క్రమీకరించు_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, పైకి);  /* swap a[up] - a[p] */
      లేకపోతే
        lua_తీయి(L, 2);
    }
    ఒకవేళ (పైకి - lo == 2)  /* only 3 elements? */
      తిరిగి;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, పైకి - 1);  /* push a[up - 1] */
    set2(L, p, పైకి - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = విభజన(L, lo, పైకి);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    ఒకవేళ (p - lo < పైకి - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    లేకపోతే {
      auxsort(L, p + 1, పైకి, rnd);  /* call recursively for upper interval */
      n = పైకి - p;  /* size of smaller interval */
      పైకి = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    ఒకవేళ ((పైకి - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomize_ఇరుసు(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


స్థిర పూర్ణాంకం క్రమీకరించు (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  ఒకవేళ (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < గరిష్ఠ_పూర్ణాంకం, 1, "array too big");
    ఒకవేళ (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  తిరిగి 0;
}

/* }====================================================== */


స్థిర స్థిరరాశి luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", క్రమీకరించు},
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_పట్టిక (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  తిరిగి 1;
}

