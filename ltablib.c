/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#परिभाषय ltablib_c
#परिभाषय LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#परिभाषय TAB_R	1			/* read */
#परिभाषय TAB_W	2			/* write */
#परिभाषय TAB_L	4			/*दीर्घता*/
#परिभाषय TAB_RW	(TAB_R | TAB_W)		/* read/write */


#परिभाषय aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


स्थिर पूर्णाङ्क checkfield (lua_State *L, ध्रुव वर्ण *कुञ्चिका, पूर्णाङ्क n) {
  lua_pushstring(L, कुञ्चिका);
  प्रत्यागम (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
स्थिर शून्य checktab (lua_State *L, पूर्णाङ्क arg, पूर्णाङ्क what) {
  पूर्णाङ्क tp = lua_type(L, arg);
  यदि (tp != LUA_TTABLE) {  /* is it not a table? */
    पूर्णाङ्क n = 1;  /* number of elements to pop */
    यदि (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_निष्कासय(L, n);  /* pop metatable and tested metamethods */
    }
    अन्यथा
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


स्थिर पूर्णाङ्क tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(अधिकतम_पूर्णाङ्कः), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(अधिकतम_पूर्णाङ्कः), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  विकल्प (lua_gettop(L)) {
    पक्ष 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      भङ्ग;
    }
    पक्ष 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      प्रति (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      भङ्ग;
    }
    स्वभाव: {
      प्रत्यागम luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क tremove (lua_State *L) {
  lua_Integer आकार = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, आकार);
  यदि (pos != आकार)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)आकार, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  प्रति ( ; pos < आकार; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  प्रत्यागम 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
स्थिर पूर्णाङ्क tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  पूर्णाङ्क tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  यदि (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    यदि (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      प्रति (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    अन्यथा {
      प्रति (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  प्रत्यागम 1;
}


स्थिर शून्य addfield (lua_State *L, lua_L_बफर *b, lua_Integer i) {
  lua_geti(L, 1, i);
  यदि (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


स्थिर पूर्णाङ्क tconcat (lua_State *L) {
  lua_L_बफर b;
  lua_Integer चरमः = aux_getn(L, 1, TAB_R);
  size_t lsep;
  ध्रुव वर्ण *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  चरमः = luaL_optinteger(L, 4, चरमः);
  luaL_buffinit(L, &b);
  प्रति (; i < चरमः; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  यदि (i == चरमः)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  प्रत्यागम 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

स्थिर पूर्णाङ्क tpack (lua_State *L) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_निवेशय(L, 1);  /* put it at index 1 */
  प्रति (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  प्रत्यागम 1;  /* return table */
}


स्थिर पूर्णाङ्क tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  यदि (i > e) प्रत्यागम 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  यदि (l_unlikely(n >= (अचिह्नित पूर्णाङ्क)अधिकतम_पूर्णाङ्कः  ||
                 !lua_checkstack(L, (पूर्णाङ्क)(++n))))
    प्रत्यागम luaL_error(L, "too many results to unpack");
  प्रति (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  प्रत्यागम (पूर्णाङ्क)n;
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
प्रकारनिर्देश अचिह्नित पूर्णाङ्क IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#परिभाषय geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#परिभाषय seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#यदि !defined(l_randomizePivot)
#परिभाषय l_randomizePivot(L)	luaL_makeseed(L)
#यदि_अन्तः					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#परिभाषय RANLIMIT	100u


स्थिर शून्य set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
स्थिर पूर्णाङ्क क्रमय_comp (lua_State *L, पूर्णाङ्क a, पूर्णाङ्क b) {
  यदि (lua_isnil(L, 2))  /* no function? */
    प्रत्यागम lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  अन्यथा {  /* function */
    पूर्णाङ्क res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_निष्कासय(L, 1);          /* pop result */
    प्रत्यागम res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
स्थिर IdxT partition (lua_State *L, IdxT lo, IdxT ऊर्ध्वम्) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = ऊर्ध्वम् - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  प्रति (;;) {
    /* next loop: repeat ++i while a[i] < P */
    यावत् ((शून्य)geti(L, 1, ++i), क्रमय_comp(L, -1, -2)) {
      यदि (l_unlikely(i == ऊर्ध्वम् - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_निष्कासय(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    यावत् ((शून्य)geti(L, 1, --j), क्रमय_comp(L, -3, -1)) {
      यदि (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_निष्कासय(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    यदि (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_निष्कासय(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, ऊर्ध्वम् - 1, i);
      प्रत्यागम i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
स्थिर IdxT वृणु_Pivot (IdxT lo, IdxT ऊर्ध्वम्, अचिह्नित पूर्णाङ्क rnd) {
  IdxT r4 = (ऊर्ध्वम् - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ ऊर्ध्वम्) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= ऊर्ध्वम् - r4);
  प्रत्यागम p;
}


/*
** Quicksort algorithm (recursive function)
*/
स्थिर शून्य auxsort (lua_State *L, IdxT lo, IdxT ऊर्ध्वम्, अचिह्नित rnd) {
  यावत् (lo < ऊर्ध्वम्) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, ऊर्ध्वम्);
    यदि (क्रमय_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, ऊर्ध्वम्);  /* swap a[lo] - a[up] */
    अन्यथा
      lua_निष्कासय(L, 2);  /* remove both values */
    यदि (ऊर्ध्वम् - lo == 1)  /* only 2 elements? */
      प्रत्यागम;  /* already sorted */
    यदि (ऊर्ध्वम् - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + ऊर्ध्वम्)/2;  /* middle element is a good pivot */
    अन्यथा  /* for larger intervals, it is worth a random pivot */
      p = वृणु_Pivot(lo, ऊर्ध्वम्, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    यदि (क्रमय_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    अन्यथा {
      lua_निष्कासय(L, 1);  /* remove a[lo] */
      geti(L, 1, ऊर्ध्वम्);
      यदि (क्रमय_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, ऊर्ध्वम्);  /* swap a[up] - a[p] */
      अन्यथा
        lua_निष्कासय(L, 2);
    }
    यदि (ऊर्ध्वम् - lo == 2)  /* only 3 elements? */
      प्रत्यागम;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, ऊर्ध्वम् - 1);  /* push a[up - 1] */
    set2(L, p, ऊर्ध्वम् - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, ऊर्ध्वम्);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    यदि (p - lo < ऊर्ध्वम् - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    अन्यथा {
      auxsort(L, p + 1, ऊर्ध्वम्, rnd);  /* call recursively for upper interval */
      n = ऊर्ध्वम् - p;  /* size of smaller interval */
      ऊर्ध्वम् = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    यदि ((ऊर्ध्वम् - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


स्थिर पूर्णाङ्क क्रमय (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  यदि (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < अधिकतम_पूर्णाङ्कः, 1, "array too big");
    यदि (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  प्रत्यागम 0;
}

/* }====================================================== */


स्थिर ध्रुव luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", क्रमय},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णाङ्क luaopen_सारणी (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  प्रत्यागम 1;
}

