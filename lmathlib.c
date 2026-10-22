/*
** $Id: lmathlib.c $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <float.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <time.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_abs (lua_State *L) {
  ଯଦି (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    ଯଦି (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  ଅନ୍ୟଥା
    lua_pushnumber(L, l_mathop(fabs)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_atan (lua_State *L) {
  lua_ସଂଖ୍ୟା y = luaL_checknumber(L, 1);
  lua_ସଂଖ୍ୟା x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_toint (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  ଯଦି (l_likely(valid))
    lua_pushinteger(L, n);
  ଅନ୍ୟଥା {
    luaL_checkany(L, 1);
    luaL_pushfail(L);  /* value is not convertible to integer */
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଶୂନ୍ୟ pushnumint (lua_State *L, lua_ସଂଖ୍ୟା d) {
  lua_Integer n;
  ଯଦି (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  ଅନ୍ୟଥା
    lua_pushnumber(L, d);  /* result is float */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_floor (lua_State *L) {
  ଯଦି (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  ଅନ୍ୟଥା {
    lua_ସଂଖ୍ୟା d = l_mathop(floor)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_ceil (lua_State *L) {
  ଯଦି (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceiling */
  ଅନ୍ୟଥା {
    lua_ସଂଖ୍ୟା d = l_mathop(ceil)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_fmod (lua_State *L) {
  ଯଦି (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    ଯଦି ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    ଅନ୍ୟଥା
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  ଅନ୍ୟଥା
    lua_pushnumber(L, l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  ଫେରନ୍ତୁ 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_modf (lua_State *L) {
  ଯଦି (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  ଅନ୍ୟଥା {
    lua_ସଂଖ୍ୟା n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_ସଂଖ୍ୟା ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(sqrt)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_log (lua_State *L) {
  lua_ସଂଖ୍ୟା x = luaL_checknumber(L, 1);
  lua_ସଂଖ୍ୟା res;
  ଯଦି (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  ଅନ୍ୟଥା {
    lua_ସଂଖ୍ୟା base = luaL_checknumber(L, 2);
#ଯଦି !defined(LUA_USE_C89)
    ଯଦି (base == l_mathop(2.0))
      res = l_mathop(log2)(x);
    ଅନ୍ୟଥା
#endif
    ଯଦି (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    ଅନ୍ୟଥା
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_frexp (lua_State *L) {
  lua_ସଂଖ୍ୟା x = luaL_checknumber(L, 1);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ep;
  lua_pushnumber(L, l_mathop(frexp)(x, &ep));
  lua_pushinteger(L, ep);
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_ldexp (lua_State *L) {
  lua_ସଂଖ୍ୟା x = luaL_checknumber(L, 1);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ep = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_min (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା imin = 1;  /* index of current minimum value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  ପାଇଁ (i = 2; i <= n; i++) {
    ଯଦି (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_max (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା imax = 1;  /* index of current maximum value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  ପାଇଁ (i = 2; i <= n; i++) {
    ଯଦି (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_type (lua_State *L) {
  ଯଦି (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushstring(L, (lua_isinteger(L, 1)) ? "integer" : "float");
  ଅନ୍ୟଥା {
    luaL_checkany(L, 1);
    luaL_pushfail(L);
  }
  ଫେରନ୍ତୁ 1;
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
#define FIGS	l_floatatt(MANT_DIG)

#ଯଦି FIGS > 64
/* there are only 64 random bits; use them all */
#undef FIGS
#define FIGS	64
#endif


/*
** LUA_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#ଯଦି !defined(LUA_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#ଯଦି ((ULONG_MAX >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#define Rand64		unsigned long
#define SRand64		long

#ନଚେତଯଦି !defined(LUA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#define Rand64		unsigned long long
#define SRand64		long long

#ନଚେତଯଦି ((LUA_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'lua_Unsigned' has at least 64 bits */
#define Rand64		lua_Unsigned
#define SRand64		lua_Integer

#endif

#endif


#ଯଦି defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
ସ୍ଥିର Rand64 rotl (Rand64 x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଫେରନ୍ତୁ (x << n) | (trim64(x) >> (64 - n));
}

ସ୍ଥିର Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  ଫେରନ୍ତୁ res;
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
#define shift64_FIG	(64 - FIGS)

/* 2^(-FIGS) == 2^-1 / 2^(FIGS-1) */
#define scaleFIG	(l_mathop(0.5) / ((Rand64)1 << (FIGS - 1)))

ସ୍ଥିର lua_ସଂଖ୍ୟା I2d (Rand64 x) {
  SRand64 sx = (SRand64)(trim64(x) >> shift64_FIG);
  lua_ସଂଖ୍ୟା res = (lua_ସଂଖ୍ୟା)(sx) * scaleFIG;
  ଯଦି (sx < 0)
    res += l_mathop(1.0);  /* correct the two's complement if negative */
  lua_assert(0 <= res && res < 1);
  ଫେରନ୍ତୁ res;
}

/* convert a 'Rand64' to a 'lua_Unsigned' */
#define I2UInt(x)	((lua_Unsigned)trim64(x))

/* convert a 'lua_Unsigned' to a 'Rand64' */
#define Int2I(x)	((Rand64)(x))


#ଅନ୍ୟଥା	/* no 'Rand64'   }{ */

/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା Rand64 {
  l_uint32 h;  /* higher half */
  l_uint32 l;  /* lower half */
} Rand64;


/*
** If 'l_uint32' has more than 32 bits, the extra bits do not interfere
** with the 32 initial bits, except in a right shift and comparisons.
** Moreover, the final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim32(x)	((x) & 0xffffffffu)


/*
** basic operations on 'Rand64' values
*/

/* build a new Rand64 value */
ସ୍ଥିର Rand64 packI (l_uint32 h, l_uint32 l) {
  Rand64 result;
  result.h = h;
  result.l = l;
  ଫେରନ୍ତୁ result;
}

/* return i << n */
ସ୍ଥିର Rand64 Ishl (Rand64 i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  lua_assert(n > 0 && n < 32);
  ଫେରନ୍ତୁ packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
ସ୍ଥିର ଶୂନ୍ୟ Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
ସ୍ଥିର Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 result = packI(i1.h + i2.h, i1.l + i2.l);
  ଯଦି (trim32(result.l) < trim32(i1.l))  /* carry? */
    result.h++;
  ଫେରନ୍ତୁ result;
}

/* return i * 5 */
ସ୍ଥିର Rand64 times5 (Rand64 i) {
  ଫେରନ୍ତୁ Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
ସ୍ଥିର Rand64 times9 (Rand64 i) {
  ଫେରନ୍ତୁ Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
ସ୍ଥିର Rand64 rotl (Rand64 i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  lua_assert(n > 0 && n < 32);
  ଫେରନ୍ତୁ packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
ସ୍ଥିର Rand64 rotl1 (Rand64 i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  lua_assert(n > 32 && n < 64);
  n = 64 - n;
  ଫେରନ୍ତୁ packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
ସ୍ଥିର Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  ଫେରନ୍ତୁ res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#define UONE		((l_uint32)1)


#ଯଦି FIGS <= 32

/* 2^(-FIGS) */
#define scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
ସ୍ଥିର lua_ସଂଖ୍ୟା I2d (Rand64 x) {
  lua_ସଂଖ୍ୟା h = (lua_ସଂଖ୍ୟା)(trim32(x.h) >> (32 - FIGS));
  ଫେରନ୍ତୁ h * scaleFIG;
}

#ଅନ୍ୟଥା	/* 32 < FIGS <= 64 */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#define scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#define shift_ତଳ	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#define shiftHI		((lua_ସଂଖ୍ୟା)(UONE << (FIGS - 33)) * l_mathop(2.0))


ସ୍ଥିର lua_ସଂଖ୍ୟା I2d (Rand64 x) {
  lua_ସଂଖ୍ୟା h = (lua_ସଂଖ୍ୟା)trim32(x.h) * shiftHI;
  lua_ସଂଖ୍ୟା l = (lua_ସଂଖ୍ୟା)(trim32(x.l) >> shift_ତଳ);
  ଫେରନ୍ତୁ (h + l) * scaleFIG;
}

#endif


/* convert a 'Rand64' to a 'lua_Unsigned' */
ସ୍ଥିର lua_Unsigned I2UInt (Rand64 x) {
  ଫେରନ୍ତୁ (((lua_Unsigned)trim32(x.h) << 31) << 1) | (lua_Unsigned)trim32(x.l);
}

/* convert a 'lua_Unsigned' to a 'Rand64' */
ସ୍ଥିର Rand64 Int2I (lua_Unsigned n) {
  ଫେରନ୍ତୁ packI((l_uint32)((n >> 31) >> 1), (l_uint32)n);
}

#endif  /* } */


/*
** A state uses four 'Rand64' values.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
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
ସ୍ଥିର lua_Unsigned project (lua_Unsigned ran, lua_Unsigned n,
                             RanState *state) {
  lua_Unsigned lim = n;  /* to compute the Mersenne number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sh;  /* how much to spread bits to the right in 'lim' */
  /* spread '1' bits in 'lim' until it becomes a Mersenne number */
  ପାଇଁ (sh = 1; (lim & (lim + 1)) != 0; sh *= 2)
    lim |= (lim >> sh);  /* spread '1's to the right */
  ଯେତେବେଳେ ((ran &= lim) > n)  /* project 'ran' into [0..lim] and test */
    ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
  ଫେରନ୍ତୁ ran;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_random (lua_State *L) {
  lua_Integer ତଳ, up;
  lua_Unsigned p;
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  ବାଛ (lua_gettop(L)) {  /* check number of arguments */
    ସ୍ଥିତି 0: {  /* no arguments */
      lua_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି 1: {  /* only upper limit */
      ତଳ = 1;
      up = luaL_checkinteger(L, 1);
      ଯଦି (up == 0) {  /* single 0 as argument? */
        lua_pushinteger(L, l_castU2S(I2UInt(rv)));  /* full random integer */
        ଫେରନ୍ତୁ 1;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 2: {  /* lower and upper limits */
      ତଳ = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, ତଳ <= up, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), l_castS2U(up) - l_castS2U(ତଳ), state);
  lua_pushinteger(L, l_castU2S(p + l_castS2U(ତଳ)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଶୂନ୍ୟ setseed (lua_State *L, Rand64 *state,
                     lua_Unsigned n1, lua_Unsigned n2) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  ପାଇଁ (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  lua_pushinteger(L, l_castU2S(n1));
  lua_pushinteger(L, l_castU2S(n2));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_randomseed (lua_State *L) {
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  lua_Unsigned n1, n2;
  ଯଦି (lua_isnone(L, 1)) {
    n1 = luaL_makeseed(L);  /* "random" seed */
    n2 = I2UInt(nextrand(state->s));  /* in case seed is not that random... */
  }
  ଅନ୍ୟଥା {
    n1 = l_castS2U(luaL_checkinteger(L, 1));
    n2 = l_castS2U(luaL_optinteger(L, 2, 0));
  }
  setseed(L, state->s, n1, n2);
  ଫେରନ୍ତୁ 2;  /* return seeds */
}


ସ୍ଥିର ଅଚଳ luaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {ଖାଲି, ଖାଲି}
};


/*
** Register the random functions and initialize their state.
*/
ସ୍ଥିର ଶୂନ୍ୟ setrandfunc (lua_State *L) {
  RanState *state = (RanState *)lua_newuserdatauv(L, ଆକାର(RanState), 0);
  setseed(L, state->s, luaL_makeseed(L), 0);  /* initialize with random seed */
  lua_କାଢ଼(L, 2);  /* remove pushed seeds */
  luaL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#ଯଦି defined(LUA_COMPAT_MATHLIB)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_pow (lua_State *L) {
  lua_ସଂଖ୍ୟା x = luaL_checknumber(L, 1);
  lua_ସଂଖ୍ୟା y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(pow)(x, y));
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  ଫେରନ୍ତୁ 1;
}

#endif
/* }================================================================== */



ସ୍ଥିର ଅଚଳ luaL_Reg mathlib[] = {
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
#ଯଦି defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh", math_cosh},
  {"sinh", math_sinh},
  {"tanh", math_tanh},
  {"pow", math_pow},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"random", ଖାଲି},
  {"randomseed", ଖାଲି},
  {"pi", ଖାଲି},
  {"huge", ଖାଲି},
  {"maxinteger", ଖାଲି},
  {"mininteger", ଖାଲି},
  {ଖାଲି, ଖାଲି}
};


/*
** Open math library
*/
LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_math (lua_State *L) {
  luaL_newlib(L, mathlib);
  lua_pushnumber(L, PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_ସଂଖ୍ୟା)HUGE_VAL);
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, LUA_MAXINTEGER);
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, LUA_MININTEGER);
  lua_setfield(L, -2, "mininteger");
  setrandfunc(L);
  ଫେରନ୍ତୁ 1;
}

