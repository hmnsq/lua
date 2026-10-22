/*
** $Id: lmathlib.c $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#تعریف_کرو lmathlib_c
#تعریف_کرو LUA_LIB

#شامل "lprefix.h"


#شامل <float.h>
#شامل <limits.h>
#شامل <math.h>
#شامل <stdlib.h>
#شامل <time.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


#تعریف_منسوخ PI
#تعریف_کرو PI	(l_mathop(3.141592653589793238462643383279502884))


ساکن صحیح math_abs (lua_State *L) {
  اگر (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    اگر (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  ورنہ
    lua_pushnumber(L, l_mathop(fabs)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(جیب)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(جیب_تمام)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_atan (lua_State *L) {
  lua_نمبر y = luaL_checknumber(L, 1);
  lua_نمبر x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  واپس 1;
}


ساکن صحیح math_toint (lua_State *L) {
  صحیح valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  اگر (l_likely(valid))
    lua_pushinteger(L, n);
  ورنہ {
    luaL_checkany(L, 1);
    luaL_pushfail(L);  /* value is not convertible to integer */
  }
  واپس 1;
}


ساکن خالی pushnumint (lua_State *L, lua_نمبر d) {
  lua_Integer n;
  اگر (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  ورنہ
    lua_pushnumber(L, d);  /* result is float */
}


ساکن صحیح math_floor (lua_State *L) {
  اگر (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  ورنہ {
    lua_نمبر d = l_mathop(floor)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  واپس 1;
}


ساکن صحیح math_ceil (lua_State *L) {
  اگر (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceiling */
  ورنہ {
    lua_نمبر d = l_mathop(ceil)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  واپس 1;
}


ساکن صحیح math_fmod (lua_State *L) {
  اگر (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    اگر ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    ورنہ
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  ورنہ
    lua_pushnumber(L, l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  واپس 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
ساکن صحیح math_modf (lua_State *L) {
  اگر (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  ورنہ {
    lua_نمبر n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_نمبر ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  واپس 2;
}


ساکن صحیح math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(جذر)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  واپس 1;
}


ساکن صحیح math_log (lua_State *L) {
  lua_نمبر x = luaL_checknumber(L, 1);
  lua_نمبر res;
  اگر (lua_isnoneornil(L, 2))
    res = l_mathop(لوگارتھم)(x);
  ورنہ {
    lua_نمبر بنیاد = luaL_checknumber(L, 2);
#اگر !defined(LUA_USE_C89)
    اگر (بنیاد == l_mathop(2.0))
      res = l_mathop(log2)(x);
    ورنہ
#اگر_ختم
    اگر (بنیاد == l_mathop(10.0))
      res = l_mathop(log10)(x);
    ورنہ
      res = l_mathop(لوگارتھم)(x)/l_mathop(لوگارتھم)(بنیاد);
  }
  lua_pushnumber(L, res);
  واپس 1;
}


ساکن صحیح math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(قوت_نما)(luaL_checknumber(L, 1)));
  واپس 1;
}


ساکن صحیح math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  واپس 1;
}


ساکن صحیح math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  واپس 1;
}


ساکن صحیح math_frexp (lua_State *L) {
  lua_نمبر x = luaL_checknumber(L, 1);
  صحیح ep;
  lua_pushnumber(L, l_mathop(frexp)(x, &ep));
  lua_pushinteger(L, ep);
  واپس 2;
}


ساکن صحیح math_ldexp (lua_State *L) {
  lua_نمبر x = luaL_checknumber(L, 1);
  صحیح ep = (صحیح)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  واپس 1;
}


ساکن صحیح math_min (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح imin = 1;  /* index of current minimum value */
  صحیح i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  برائے (i = 2; i <= n; i++) {
    اگر (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  واپس 1;
}


ساکن صحیح math_max (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح imax = 1;  /* index of current maximum value */
  صحیح i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  برائے (i = 2; i <= n; i++) {
    اگر (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  واپس 1;
}


ساکن صحیح math_type (lua_State *L) {
  اگر (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushstring(L, (lua_isinteger(L, 1)) ? "integer" : "float");
  ورنہ {
    luaL_checkany(L, 1);
    luaL_pushfail(L);
  }
  واپس 1;
}



/*
** {==================================================================
** Pseudo-Random Number Generator based on 'xoshiro256**'.
** ===================================================================
*/

/*
** This code uses lots of shifts. ISO C does not allow shifts greater
** than or equal to the width of the type being shifted, so some shifts
** are written in convoluted ways to match that restriction. For
** preprocessor tests, it assumes a width of 32 bits, so the maximum
** shift there is 31 bits.
*/


/* number of binary digits in the mantissa of a float */
#تعریف_کرو FIGS	l_floatatt(MANT_DIG)

#اگر FIGS > 64
/* there are only 64 random bits; use them all */
#تعریف_منسوخ FIGS
#تعریف_کرو FIGS	64
#اگر_ختم


/*
** LUA_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#اگر !defined(LUA_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#اگر ((ULONG_MAX >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#تعریف_کرو Rand64		غیرعلامتی طویل
#تعریف_کرو SRand64		طویل

#وگرنہ_اگر !defined(LUA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#تعریف_کرو Rand64		غیرعلامتی طویل طویل
#تعریف_کرو SRand64		طویل طویل

#وگرنہ_اگر ((LUA_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'lua_Unsigned' has at least 64 bits */
#تعریف_کرو Rand64		lua_Unsigned
#تعریف_کرو SRand64		lua_Integer

#اگر_ختم

#اگر_ختم


#اگر defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#تعریف_کرو trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
ساکن Rand64 rotl (Rand64 x, صحیح n) {
  واپس (x << n) | (trim64(x) >> (64 - n));
}

ساکن Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  واپس res;
}


/*
** Convert bits from a random integer into a float in the
** interval [0,1), getting the higher FIG bits from the
** random unsigned integer and converting that to a float.
** Some old Microsoft compilers cannot cast an unsigned long
** to a floating-point number, so we use a signed long as an
** intermediary. When lua_Number is float or double, the shift ensures
** that 'sx' is non negative; in that case, a good compiler will remove
** the correction.
*/

/* must throw out the extra (64 - FIGS) bits */
#تعریف_کرو shift64_FIG	(64 - FIGS)

/* 2^(-FIGS) == 2^-1 / 2^(FIGS-1) */
#تعریف_کرو scaleFIG	(l_mathop(0.5) / ((Rand64)1 << (FIGS - 1)))

ساکن lua_نمبر I2d (Rand64 x) {
  SRand64 sx = (SRand64)(trim64(x) >> shift64_FIG);
  lua_نمبر res = (lua_نمبر)(sx) * scaleFIG;
  اگر (sx < 0)
    res += l_mathop(1.0);  /* correct the two's complement if negative */
  lua_assert(0 <= res && res < 1);
  واپس res;
}

/* convert a 'Rand64' to a 'lua_Unsigned' */
#تعریف_کرو I2UInt(x)	((lua_Unsigned)trim64(x))

/* convert a 'lua_Unsigned' to a 'Rand64' */
#تعریف_کرو Int2I(x)	((Rand64)(x))


#ورنہ	/* no 'Rand64'   }{ */

/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
قسمتعریف ساخت Rand64 {
  l_uint32 h;  /* higher half */
  l_uint32 l;  /* lower half */
} Rand64;


/*
** If 'l_uint32' has more than 32 bits, the extra bits do not interfere
** with the 32 initial bits, except in a right shift and comparisons.
** Moreover, the final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#تعریف_کرو trim32(x)	((x) & 0xffffffffu)


/*
** basic operations on 'Rand64' values
*/

/* build a new Rand64 value */
ساکن Rand64 packI (l_uint32 h, l_uint32 l) {
  Rand64 نتیجہ;
  نتیجہ.h = h;
  نتیجہ.l = l;
  واپس نتیجہ;
}

/* return i << n */
ساکن Rand64 Ishl (Rand64 i, صحیح n) {
  lua_assert(n > 0 && n < 32);
  واپس packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
ساکن خالی Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
ساکن Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 نتیجہ = packI(i1.h + i2.h, i1.l + i2.l);
  اگر (trim32(نتیجہ.l) < trim32(i1.l))  /* carry? */
    نتیجہ.h++;
  واپس نتیجہ;
}

/* return i * 5 */
ساکن Rand64 times5 (Rand64 i) {
  واپس Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
ساکن Rand64 times9 (Rand64 i) {
  واپس Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
ساکن Rand64 rotl (Rand64 i, صحیح n) {
  lua_assert(n > 0 && n < 32);
  واپس packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
ساکن Rand64 rotl1 (Rand64 i, صحیح n) {
  lua_assert(n > 32 && n < 64);
  n = 64 - n;
  واپس packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
ساکن Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  واپس res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#تعریف_کرو UONE		((l_uint32)1)


#اگر FIGS <= 32

/* 2^(-FIGS) */
#تعریف_کرو scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
ساکن lua_نمبر I2d (Rand64 x) {
  lua_نمبر h = (lua_نمبر)(trim32(x.h) >> (32 - FIGS));
  واپس h * scaleFIG;
}

#ورنہ	/* 32 < FIGS <= 64 */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#تعریف_کرو scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#تعریف_کرو سرکاؤ_کم	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#تعریف_کرو سرکاؤ_HI		((lua_نمبر)(UONE << (FIGS - 33)) * l_mathop(2.0))


ساکن lua_نمبر I2d (Rand64 x) {
  lua_نمبر h = (lua_نمبر)trim32(x.h) * سرکاؤ_HI;
  lua_نمبر l = (lua_نمبر)(trim32(x.l) >> سرکاؤ_کم);
  واپس (h + l) * scaleFIG;
}

#اگر_ختم


/* convert a 'Rand64' to a 'lua_Unsigned' */
ساکن lua_Unsigned I2UInt (Rand64 x) {
  واپس (((lua_Unsigned)trim32(x.h) << 31) << 1) | (lua_Unsigned)trim32(x.l);
}

/* convert a 'lua_Unsigned' to a 'Rand64' */
ساکن Rand64 Int2I (lua_Unsigned n) {
  واپس packI((l_uint32)((n >> 31) >> 1), (l_uint32)n);
}

#اگر_ختم  /* } */


/*
** A state uses four 'Rand64' values.
*/
قسمتعریف ساخت {
  Rand64 s[4];
} RanState;


/*
** Project the random integer 'ran' into the interval [0, n].
** Because 'ran' has 2^B possible values, the projection can only be
** uniform when the size of the interval is a power of 2 (exact
** division). So, to get a uniform projection into [0, n], we
** first compute 'lim', the smallest Mersenne number not smaller than
** 'n'. We then project 'ran' into the interval [0, lim].  If the result
** is inside [0, n], we are done. Otherwise, we try with another 'ran',
** until we have a result inside the interval.
*/
ساکن lua_Unsigned project (lua_Unsigned ran, lua_Unsigned n,
                             RanState *state) {
  lua_Unsigned lim = n;  /* to compute the Mersenne number */
  صحیح sh;  /* how much to spread bits to the right in 'lim' */
  /* spread '1' bits in 'lim' until it becomes a Mersenne number */
  برائے (sh = 1; (lim & (lim + 1)) != 0; sh *= 2)
    lim |= (lim >> sh);  /* spread '1's to the right */
  جب ((ran &= lim) > n)  /* project 'ran' into [0..lim] and test */
    ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
  واپس ran;
}


ساکن صحیح math_random (lua_State *L) {
  lua_Integer کم, اوپر;
  lua_Unsigned p;
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  انتخاب (lua_gettop(L)) {  /* check number of arguments */
    حالت 0: {  /* no arguments */
      lua_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      واپس 1;
    }
    حالت 1: {  /* only upper limit */
      کم = 1;
      اوپر = luaL_checkinteger(L, 1);
      اگر (اوپر == 0) {  /* single 0 as argument? */
        lua_pushinteger(L, l_castU2S(I2UInt(rv)));  /* full random integer */
        واپس 1;
      }
      توڑو;
    }
    حالت 2: {  /* lower and upper limits */
      کم = luaL_checkinteger(L, 1);
      اوپر = luaL_checkinteger(L, 2);
      توڑو;
    }
    طےشدہ: واپس luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, کم <= اوپر, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), l_castS2U(اوپر) - l_castS2U(کم), state);
  lua_pushinteger(L, l_castU2S(p + l_castS2U(کم)));
  واپس 1;
}


ساکن خالی setseed (lua_State *L, Rand64 *state,
                     lua_Unsigned n1, lua_Unsigned n2) {
  صحیح i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  برائے (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  lua_pushinteger(L, l_castU2S(n1));
  lua_pushinteger(L, l_castU2S(n2));
}


ساکن صحیح math_randomseed (lua_State *L) {
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  lua_Unsigned n1, n2;
  اگر (lua_isnone(L, 1)) {
    n1 = luaL_makeseed(L);  /* "random" seed */
    n2 = I2UInt(nextrand(state->s));  /* in case seed is not that random... */
  }
  ورنہ {
    n1 = l_castS2U(luaL_checkinteger(L, 1));
    n2 = l_castS2U(luaL_optinteger(L, 2, 0));
  }
  setseed(L, state->s, n1, n2);
  واپس 2;  /* return seeds */
}


ساکن ثابت luaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {تہی, تہی}
};


/*
** Register the random functions and initialize their state.
*/
ساکن خالی setrandfunc (lua_State *L) {
  RanState *state = (RanState *)lua_newuserdatauv(L, حجم(RanState), 0);
  setseed(L, state->s, luaL_makeseed(L), 0);  /* initialize with random seed */
  lua_نکالو(L, 2);  /* remove pushed seeds */
  luaL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#اگر defined(LUA_COMPAT_MATHLIB)

ساکن صحیح math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  واپس 1;
}

ساکن صحیح math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  واپس 1;
}

ساکن صحیح math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  واپس 1;
}

ساکن صحیح math_pow (lua_State *L) {
  lua_نمبر x = luaL_checknumber(L, 1);
  lua_نمبر y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(قوت)(x, y));
  واپس 1;
}

ساکن صحیح math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  واپس 1;
}

#اگر_ختم
/* }================================================================== */



ساکن ثابت luaL_Reg mathlib[] = {
  {"abs", math_abs},
  {"acos", math_acos},
  {"asin", math_asin},
  {"atan", math_atan},
  {"ceil", math_ceil},
  {"cos", math_cos},
  {"deg", math_deg},
  {"exp", math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod", math_fmod},
  {"frexp", math_frexp},
  {"ult", math_ult},
  {"ldexp", math_ldexp},
  {"log", math_log},
  {"max", math_max},
  {"min", math_min},
  {"modf", math_modf},
  {"rad", math_rad},
  {"sin", math_sin},
  {"sqrt", math_sqrt},
  {"tan", math_tan},
  {"type", math_type},
#اگر defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh", math_cosh},
  {"sinh", math_sinh},
  {"tanh", math_tanh},
  {"pow", math_pow},
  {"log10", math_log10},
#اگر_ختم
  /* placeholders */
  {"random", تہی},
  {"randomseed", تہی},
  {"pi", تہی},
  {"huge", تہی},
  {"maxinteger", تہی},
  {"mininteger", تہی},
  {تہی, تہی}
};


/*
** Open math library
*/
LUAMOD_API صحیح luaopen_math (lua_State *L) {
  luaL_newlib(L, mathlib);
  lua_pushnumber(L, PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_نمبر)HUGE_VAL);
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, LUA_MAXINTEGER);
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, LUA_MININTEGER);
  lua_setfield(L, -2, "mininteger");
  setrandfunc(L);
  واپس 1;
}

