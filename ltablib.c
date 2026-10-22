/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#घोषणा ltablib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <limits.h>
#समावेश <stddef.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#घोषणा TAB_R	1			/* read */
#घोषणा TAB_W	2			/* write */
#घोषणा TAB_L	4			/*लंबाई*/
#घोषणा TAB_RW	(TAB_R | TAB_W)		/* read/write */


#घोषणा aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


अटल पूर्णांक checkfield (lua_State *L, स्थिर अक्षर *कुंजी, पूर्णांक n) {
  lua_pushstring(L, कुंजी);
  वापस (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
अटल व्योम checktab (lua_State *L, पूर्णांक arg, पूर्णांक what) {
  पूर्णांक tp = lua_type(L, arg);
  अगर (tp != LUA_TTABLE) {  /* is it not a table? */
    पूर्णांक n = 1;  /* number of elements to pop */
    अगर (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_निकालो(L, n);  /* pop metatable and tested metamethods */
    }
    अन्यथा
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


अटल पूर्णांक tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(पूर्णांक_उच्च), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(पूर्णांक_उच्च), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  वापस 1;
}


अटल पूर्णांक tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  चयन (lua_gettop(L)) {
    हाल 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      अवरोध;
    }
    हाल 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      क्रम (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      अवरोध;
    }
    शेष: {
      वापस luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  वापस 0;
}


अटल पूर्णांक tremove (lua_State *L) {
  lua_Integer आकार = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, आकार);
  अगर (pos != आकार)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)आकार, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  क्रम ( ; pos < आकार; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  वापस 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
अटल पूर्णांक tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  पूर्णांक tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  अगर (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    अगर (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      क्रम (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    अन्यथा {
      क्रम (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  वापस 1;
}


अटल व्योम addfield (lua_State *L, lua_L_बफर *b, lua_Integer i) {
  lua_geti(L, 1, i);
  अगर (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


अटल पूर्णांक tconcat (lua_State *L) {
  lua_L_बफर b;
  lua_Integer last = aux_getn(L, 1, TAB_R);
  माप_प्रकार lsep;
  स्थिर अक्षर *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  क्रम (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  अगर (i == last)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  वापस 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

अटल पूर्णांक tpack (lua_State *L) {
  पूर्णांक i;
  पूर्णांक n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_जोड़ो(L, 1);  /* put it at index 1 */
  क्रम (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  वापस 1;  /* return table */
}


अटल पूर्णांक tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  अगर (i > e) वापस 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  अगर (l_unlikely(n >= (अचिन्हित पूर्णांक)पूर्णांक_उच्च  ||
                 !lua_checkstack(L, (पूर्णांक)(++n))))
    वापस luaL_error(L, "too many results to unpack");
  क्रम (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  वापस (पूर्णांक)n;
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
प्रकार अचिन्हित पूर्णांक IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#घोषणा geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#घोषणा seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#अगर !defined(l_randomizePivot)
#घोषणा l_randomizePivot(L)	luaL_makeseed(L)
#पूर्ण_अगर					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#घोषणा RANLIMIT	100u


अटल व्योम set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
अटल पूर्णांक sort_comp (lua_State *L, पूर्णांक a, पूर्णांक b) {
  अगर (lua_isnil(L, 2))  /* no function? */
    वापस lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  अन्यथा {  /* function */
    पूर्णांक res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_निकालो(L, 1);          /* pop result */
    वापस res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
अटल IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  क्रम (;;) {
    /* next loop: repeat ++i while a[i] < P */
    जबतक ((व्योम)geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      अगर (l_unlikely(i == up - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_निकालो(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    जबतक ((व्योम)geti(L, 1, --j), sort_comp(L, -3, -1)) {
      अगर (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_निकालो(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    अगर (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_निकालो(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, up - 1, i);
      वापस i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
अटल IdxT choosePivot (IdxT lo, IdxT up, अचिन्हित पूर्णांक rnd) {
  IdxT r4 = (up - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ up) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  वापस p;
}


/*
** Quicksort algorithm (recursive function)
*/
अटल व्योम auxsort (lua_State *L, IdxT lo, IdxT up, अचिन्हित rnd) {
  जबतक (lo < up) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, up);
    अगर (sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, up);  /* swap a[lo] - a[up] */
    अन्यथा
      lua_निकालो(L, 2);  /* remove both values */
    अगर (up - lo == 1)  /* only 2 elements? */
      वापस;  /* already sorted */
    अगर (up - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    अन्यथा  /* for larger intervals, it is worth a random pivot */
      p = choosePivot(lo, up, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    अगर (sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    अन्यथा {
      lua_निकालो(L, 1);  /* remove a[lo] */
      geti(L, 1, up);
      अगर (sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, up);  /* swap a[up] - a[p] */
      अन्यथा
        lua_निकालो(L, 2);
    }
    अगर (up - lo == 2)  /* only 3 elements? */
      वापस;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, up - 1);  /* push a[up - 1] */
    set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    अगर (p - lo < up - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    अन्यथा {
      auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    अगर ((up - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomizePivot(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


अटल पूर्णांक sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  अगर (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < पूर्णांक_उच्च, 1, "array too big");
    अगर (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  वापस 0;
}

/* }====================================================== */


अटल स्थिर luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {शून्य, शून्य}
};


LUAMOD_API पूर्णांक luaopen_table (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  वापस 1;
}

