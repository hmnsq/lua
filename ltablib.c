/*
** $Id: ltablib.c $
** Library for Table Manipulation
** See Copyright Notice in lua.h
*/

#تعریف_کرو ltablib_c
#تعریف_کرو LUA_LIB

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <stddef.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#تعریف_کرو TAB_R	1			/* read */
#تعریف_کرو TAB_W	2			/* write */
#تعریف_کرو TAB_L	4			/*لمبائی*/
#تعریف_کرو TAB_RW	(TAB_R | TAB_W)		/* read/write */


#تعریف_کرو aux_getn(L,n,w)	(checktab(L, n, (w) | TAB_L), luaL_len(L, n))


ساکن صحیح checkfield (lua_State *L, ثابت حرف *کلید, صحیح n) {
  lua_pushstring(L, کلید);
  واپس (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods).
*/
ساکن خالی checktab (lua_State *L, صحیح arg, صحیح what) {
  صحیح tp = lua_type(L, arg);
  اگر (tp != LUA_TTABLE) {  /* is it not a table? */
    صحیح n = 1;  /* number of elements to pop */
    اگر (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) ||  /* strings don't need '__len' to have a length */
             tp == LUA_TSTRING || checkfield(L, "__len", ++n))) {
      lua_نکالو(L, n);  /* pop metatable and tested metamethods */
    }
    ورنہ
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


ساکن صحیح tcreate (lua_State *L) {
  lua_Unsigned sizeseq = (lua_Unsigned)luaL_checkinteger(L, 1);
  lua_Unsigned sizerest = (lua_Unsigned)luaL_optinteger(L, 2, 0);
  luaL_argcheck(L, sizeseq <= cast_uint(زیادہ_سے_زیادہ_عدد), 1, "out of range");
  luaL_argcheck(L, sizerest <= cast_uint(زیادہ_سے_زیادہ_عدد), 2, "out of range");
  lua_createtable(L, cast_int(sizeseq), cast_int(sizerest));
  واپس 1;
}


ساکن صحیح tinsert (lua_State *L) {
  lua_Integer pos;  /* where to insert new element */
  lua_Integer e = aux_getn(L, 1, TAB_RW);
  e = luaL_intop(+, e, 1);  /* first empty element */
  انتخاب (lua_gettop(L)) {
    حالت 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      توڑو;
    }
    حالت 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      /* check whether 'pos' is in [1, e] */
      luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                       "position out of bounds");
      برائے (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      توڑو;
    }
    طےشدہ: {
      واپس luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  واپس 0;
}


ساکن صحیح tremove (lua_State *L) {
  lua_Integer سائز = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, سائز);
  اگر (pos != سائز)  /* validate 'pos' if given */
    /* check whether 'pos' is in [1, size + 1] */
    luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)سائز, 2,
                     "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  برائے ( ; pos < سائز; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* remove entry t[pos] */
  واپس 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
ساکن صحیح tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  صحیح tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  checktab(L, 1, TAB_R);
  checktab(L, tt, TAB_W);
  اگر (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    اگر (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      برائے (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    ورنہ {
      برائے (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  واپس 1;
}


ساکن خالی addfield (lua_State *L, lua_L_بفر *b, lua_Integer i) {
  lua_geti(L, 1, i);
  اگر (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  luaL_typename(L, -1), (LUAI_UACINT)i);
  luaL_addvalue(b);
}


ساکن صحیح tconcat (lua_State *L) {
  lua_L_بفر b;
  lua_Integer آخری = aux_getn(L, 1, TAB_R);
  size_t lsep;
  ثابت حرف *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  آخری = luaL_optinteger(L, 4, آخری);
  luaL_buffinit(L, &b);
  برائے (; i < آخری; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  اگر (i == آخری)  /* add last value (if interval was not empty) */
    addfield(L, &b, i);
  luaL_pushresult(&b);
  واپس 1;
}


/*
** {======================================================
** Pack/unpack
** =======================================================
*/

ساکن صحیح tpack (lua_State *L) {
  صحیح i;
  صحیح n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_داخل_کرو(L, 1);  /* put it at index 1 */
  برائے (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  واپس 1;  /* return table */
}


ساکن صحیح tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer len = aux_getn(L, 1, TAB_R);
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, len);
  اگر (i > e) واپس 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  اگر (l_unlikely(n >= (غیرعلامتی صحیح)زیادہ_سے_زیادہ_عدد  ||
                 !lua_checkstack(L, (صحیح)(++n))))
    واپس luaL_error(L, "too many results to unpack");
  برائے (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  واپس (صحیح)n;
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
قسمتعریف غیرعلامتی صحیح IdxT;


/* Versions of lua_seti/lua_geti specialized for IdxT */
#تعریف_کرو geti(L,idt,idx)	lua_geti(L, idt, l_castU2S(idx))
#تعریف_کرو seti(L,idt,idx)	lua_seti(L, idt, l_castU2S(idx))


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'sort' detects a big imbalance in the result
** of a partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#اگر !defined(l_randomize_محور)
#تعریف_کرو l_randomize_محور(L)	luaL_makeseed(L)
#اگر_ختم					/* } */


/* arrays larger than 'RANLIMIT' may use randomized pivots */
#تعریف_کرو RANLIMIT	100u


ساکن خالی set2 (lua_State *L, IdxT i, IdxT j) {
  seti(L, 1, i);
  seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the sort).
*/
ساکن صحیح ترتیب_دو_comp (lua_State *L, صحیح a, صحیح b) {
  اگر (lua_isnil(L, 2))  /* no function? */
    واپس lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  ورنہ {  /* function */
    صحیح res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_نکالو(L, 1);          /* pop result */
    واپس res;
  }
}


/*
** Does the partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
ساکن IdxT تقسیم (lua_State *L, IdxT lo, IdxT اوپر) {
  IdxT i = lo;  /* will be incremented before first use */
  IdxT j = اوپر - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  برائے (;;) {
    /* next loop: repeat ++i while a[i] < P */
    جب ((خالی)geti(L, 1, ++i), ترتیب_دو_comp(L, -1, -2)) {
      اگر (l_unlikely(i == اوپر - 1))  /* a[up - 1] < P == a[up - 1] */
        luaL_error(L, "invalid order function for sorting");
      lua_نکالو(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P  (a) */
    /* next loop: repeat --j while P < a[j] */
    جب ((خالی)geti(L, 1, --j), ترتیب_دو_comp(L, -3, -1)) {
      اگر (l_unlikely(j < i))  /* j <= i - 1 and a[j] > P, contradicts (a) */
        luaL_error(L, "invalid order function for sorting");
      lua_نکالو(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    اگر (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_نکالو(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      set2(L, اوپر - 1, i);
      واپس i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
ساکن IdxT انتخاب_محور (IdxT lo, IdxT اوپر, غیرعلامتی صحیح rnd) {
  IdxT r4 = (اوپر - lo) / 4;  /* range/4 */
  IdxT p = (rnd ^ lo ^ اوپر) % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= اوپر - r4);
  واپس p;
}


/*
** Quicksort algorithm (recursive function)
*/
ساکن خالی auxsort (lua_State *L, IdxT lo, IdxT اوپر, غیرعلامتی rnd) {
  جب (lo < اوپر) {  /* loop for tail recursion */
    IdxT p;  /* Pivot index */
    IdxT n;  /* to be used later */
    /* sort elements 'lo', 'p', and 'up' */
    geti(L, 1, lo);
    geti(L, 1, اوپر);
    اگر (ترتیب_دو_comp(L, -1, -2))  /* a[up] < a[lo]? */
      set2(L, lo, اوپر);  /* swap a[lo] - a[up] */
    ورنہ
      lua_نکالو(L, 2);  /* remove both values */
    اگر (اوپر - lo == 1)  /* only 2 elements? */
      واپس;  /* already sorted */
    اگر (اوپر - lo < RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + اوپر)/2;  /* middle element is a good pivot */
    ورنہ  /* for larger intervals, it is worth a random pivot */
      p = انتخاب_محور(lo, اوپر, rnd);
    geti(L, 1, p);
    geti(L, 1, lo);
    اگر (ترتیب_دو_comp(L, -2, -1))  /* a[p] < a[lo]? */
      set2(L, p, lo);  /* swap a[p] - a[lo] */
    ورنہ {
      lua_نکالو(L, 1);  /* remove a[lo] */
      geti(L, 1, اوپر);
      اگر (ترتیب_دو_comp(L, -1, -2))  /* a[up] < a[p]? */
        set2(L, p, اوپر);  /* swap a[up] - a[p] */
      ورنہ
        lua_نکالو(L, 2);
    }
    اگر (اوپر - lo == 2)  /* only 3 elements? */
      واپس;  /* already sorted */
    geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    geti(L, 1, اوپر - 1);  /* push a[up - 1] */
    set2(L, p, اوپر - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = تقسیم(L, lo, اوپر);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    اگر (p - lo < اوپر - p) {  /* lower interval is smaller? */
      auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    ورنہ {
      auxsort(L, p + 1, اوپر, rnd);  /* call recursively for upper interval */
      n = اوپر - p;  /* size of smaller interval */
      اوپر = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    اگر ((اوپر - lo) / 128 > n) /* partition too imbalanced? */
      rnd = l_randomize_محور(L);  /* try a new randomization */
  }  /* tail call auxsort(L, lo, up, rnd) */
}


ساکن صحیح ترتیب_دو (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  اگر (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < زیادہ_سے_زیادہ_عدد, 1, "array too big");
    اگر (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    auxsort(L, 1, (IdxT)n, 0);
  }
  واپس 0;
}

/* }====================================================== */


ساکن ثابت luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"create", tcreate},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", ترتیب_دو},
  {تہی, تہی}
};


LUAMOD_API صحیح luaopen_جدول (lua_State *L) {
  luaL_newlib(L, tab_funcs);
  واپس 1;
}

