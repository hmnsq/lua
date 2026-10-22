/*
** $Id: lmathlib.c $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <float.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <time.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


സ്ഥിതം പൂർണ്ണസംഖ്യ math_abs (lua_State *L) {
  എങ്കിൽ (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    എങ്കിൽ (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  അല്ലെങ്കിൽ
    lua_pushnumber(L, l_mathop(fabs)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_atan (lua_State *L) {
  lua_സംഖ്യ y = luaL_checknumber(L, 1);
  lua_സംഖ്യ x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_toint (lua_State *L) {
  പൂർണ്ണസംഖ്യ valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  എങ്കിൽ (l_likely(valid))
    lua_pushinteger(L, n);
  അല്ലെങ്കിൽ {
    luaL_checkany(L, 1);
    luaL_pushfail(L);  /* value is not convertible to integer */
  }
  തിരികെ 1;
}


സ്ഥിതം ശൂന്യം pushnumint (lua_State *L, lua_സംഖ്യ d) {
  lua_Integer n;
  എങ്കിൽ (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  അല്ലെങ്കിൽ
    lua_pushnumber(L, d);  /* result is float */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_floor (lua_State *L) {
  എങ്കിൽ (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  അല്ലെങ്കിൽ {
    lua_സംഖ്യ d = l_mathop(floor)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_ceil (lua_State *L) {
  എങ്കിൽ (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceiling */
  അല്ലെങ്കിൽ {
    lua_സംഖ്യ d = l_mathop(ceil)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_fmod (lua_State *L) {
  എങ്കിൽ (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    എങ്കിൽ ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    അല്ലെങ്കിൽ
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  അല്ലെങ്കിൽ
    lua_pushnumber(L, l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  തിരികെ 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ math_modf (lua_State *L) {
  എങ്കിൽ (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  അല്ലെങ്കിൽ {
    lua_സംഖ്യ n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_സംഖ്യ ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  തിരികെ 2;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(sqrt)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_log (lua_State *L) {
  lua_സംഖ്യ x = luaL_checknumber(L, 1);
  lua_സംഖ്യ res;
  എങ്കിൽ (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  അല്ലെങ്കിൽ {
    lua_സംഖ്യ base = luaL_checknumber(L, 2);
#എങ്കിൽ !defined(LUA_USE_C89)
    എങ്കിൽ (base == l_mathop(2.0))
      res = l_mathop(log2)(x);
    അല്ലെങ്കിൽ
#endif
    എങ്കിൽ (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    അല്ലെങ്കിൽ
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_frexp (lua_State *L) {
  lua_സംഖ്യ x = luaL_checknumber(L, 1);
  പൂർണ്ണസംഖ്യ ep;
  lua_pushnumber(L, l_mathop(frexp)(x, &ep));
  lua_pushinteger(L, ep);
  തിരികെ 2;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_ldexp (lua_State *L) {
  lua_സംഖ്യ x = luaL_checknumber(L, 1);
  പൂർണ്ണസംഖ്യ ep = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_min (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  പൂർണ്ണസംഖ്യ imin = 1;  /* index of current minimum value */
  പൂർണ്ണസംഖ്യ i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  വേണ്ടി (i = 2; i <= n; i++) {
    എങ്കിൽ (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_max (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  പൂർണ്ണസംഖ്യ imax = 1;  /* index of current maximum value */
  പൂർണ്ണസംഖ്യ i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  വേണ്ടി (i = 2; i <= n; i++) {
    എങ്കിൽ (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_type (lua_State *L) {
  എങ്കിൽ (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushstring(L, (lua_isinteger(L, 1)) ? "integer" : "float");
  അല്ലെങ്കിൽ {
    luaL_checkany(L, 1);
    luaL_pushfail(L);
  }
  തിരികെ 1;
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

#എങ്കിൽ FIGS > 64
/* there are only 64 random bits; use them all */
#undef FIGS
#define FIGS	64
#endif


/*
** LUA_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#എങ്കിൽ !defined(LUA_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#എങ്കിൽ ((ULONG_MAX >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#define Rand64		unsigned long
#define SRand64		long

#അല്ലെങ്കിലെങ്കിൽ !defined(LUA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#define Rand64		unsigned long long
#define SRand64		long long

#അല്ലെങ്കിലെങ്കിൽ ((LUA_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'lua_Unsigned' has at least 64 bits */
#define Rand64		lua_Unsigned
#define SRand64		lua_Integer

#endif

#endif


#എങ്കിൽ defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
സ്ഥിതം Rand64 rotl (Rand64 x, പൂർണ്ണസംഖ്യ n) {
  തിരികെ (x << n) | (trim64(x) >> (64 - n));
}

സ്ഥിതം Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  തിരികെ res;
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

സ്ഥിതം lua_സംഖ്യ I2d (Rand64 x) {
  SRand64 sx = (SRand64)(trim64(x) >> shift64_FIG);
  lua_സംഖ്യ res = (lua_സംഖ്യ)(sx) * scaleFIG;
  എങ്കിൽ (sx < 0)
    res += l_mathop(1.0);  /* correct the two's complement if negative */
  lua_assert(0 <= res && res < 1);
  തിരികെ res;
}

/* convert a 'Rand64' to a 'lua_Unsigned' */
#define I2UInt(x)	((lua_Unsigned)trim64(x))

/* convert a 'lua_Unsigned' to a 'Rand64' */
#define Int2I(x)	((Rand64)(x))


#അല്ലെങ്കിൽ	/* no 'Rand64'   }{ */

/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
തരനിർവചനം ഘടന Rand64 {
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
സ്ഥിതം Rand64 packI (l_uint32 h, l_uint32 l) {
  Rand64 result;
  result.h = h;
  result.l = l;
  തിരികെ result;
}

/* return i << n */
സ്ഥിതം Rand64 Ishl (Rand64 i, പൂർണ്ണസംഖ്യ n) {
  lua_assert(n > 0 && n < 32);
  തിരികെ packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
സ്ഥിതം ശൂന്യം Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
സ്ഥിതം Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 result = packI(i1.h + i2.h, i1.l + i2.l);
  എങ്കിൽ (trim32(result.l) < trim32(i1.l))  /* carry? */
    result.h++;
  തിരികെ result;
}

/* return i * 5 */
സ്ഥിതം Rand64 times5 (Rand64 i) {
  തിരികെ Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
സ്ഥിതം Rand64 times9 (Rand64 i) {
  തിരികെ Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
സ്ഥിതം Rand64 rotl (Rand64 i, പൂർണ്ണസംഖ്യ n) {
  lua_assert(n > 0 && n < 32);
  തിരികെ packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
സ്ഥിതം Rand64 rotl1 (Rand64 i, പൂർണ്ണസംഖ്യ n) {
  lua_assert(n > 32 && n < 64);
  n = 64 - n;
  തിരികെ packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
സ്ഥിതം Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  തിരികെ res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#define UONE		((l_uint32)1)


#എങ്കിൽ FIGS <= 32

/* 2^(-FIGS) */
#define scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
സ്ഥിതം lua_സംഖ്യ I2d (Rand64 x) {
  lua_സംഖ്യ h = (lua_സംഖ്യ)(trim32(x.h) >> (32 - FIGS));
  തിരികെ h * scaleFIG;
}

#അല്ലെങ്കിൽ	/* 32 < FIGS <= 64 */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#define scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#define shift_താഴെയുള്ള	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#define shiftHI		((lua_സംഖ്യ)(UONE << (FIGS - 33)) * l_mathop(2.0))


സ്ഥിതം lua_സംഖ്യ I2d (Rand64 x) {
  lua_സംഖ്യ h = (lua_സംഖ്യ)trim32(x.h) * shiftHI;
  lua_സംഖ്യ l = (lua_സംഖ്യ)(trim32(x.l) >> shift_താഴെയുള്ള);
  തിരികെ (h + l) * scaleFIG;
}

#endif


/* convert a 'Rand64' to a 'lua_Unsigned' */
സ്ഥിതം lua_Unsigned I2UInt (Rand64 x) {
  തിരികെ (((lua_Unsigned)trim32(x.h) << 31) << 1) | (lua_Unsigned)trim32(x.l);
}

/* convert a 'lua_Unsigned' to a 'Rand64' */
സ്ഥിതം Rand64 Int2I (lua_Unsigned n) {
  തിരികെ packI((l_uint32)((n >> 31) >> 1), (l_uint32)n);
}

#endif  /* } */


/*
** A state uses four 'Rand64' values.
*/
തരനിർവചനം ഘടന {
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
സ്ഥിതം lua_Unsigned project (lua_Unsigned ran, lua_Unsigned n,
                             RanState *state) {
  lua_Unsigned lim = n;  /* to compute the Mersenne number */
  പൂർണ്ണസംഖ്യ sh;  /* how much to spread bits to the right in 'lim' */
  /* spread '1' bits in 'lim' until it becomes a Mersenne number */
  വേണ്ടി (sh = 1; (lim & (lim + 1)) != 0; sh *= 2)
    lim |= (lim >> sh);  /* spread '1's to the right */
  ആകുമ്പോൾ ((ran &= lim) > n)  /* project 'ran' into [0..lim] and test */
    ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
  തിരികെ ran;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_random (lua_State *L) {
  lua_Integer താഴെയുള്ള, up;
  lua_Unsigned p;
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  തിരഞ്ഞെടുപ്പ് (lua_gettop(L)) {  /* check number of arguments */
    അവസ്ഥ 0: {  /* no arguments */
      lua_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      തിരികെ 1;
    }
    അവസ്ഥ 1: {  /* only upper limit */
      താഴെയുള്ള = 1;
      up = luaL_checkinteger(L, 1);
      എങ്കിൽ (up == 0) {  /* single 0 as argument? */
        lua_pushinteger(L, l_castU2S(I2UInt(rv)));  /* full random integer */
        തിരികെ 1;
      }
      നിർത്തുക;
    }
    അവസ്ഥ 2: {  /* lower and upper limits */
      താഴെയുള്ള = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: തിരികെ luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, താഴെയുള്ള <= up, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), l_castS2U(up) - l_castS2U(താഴെയുള്ള), state);
  lua_pushinteger(L, l_castU2S(p + l_castS2U(താഴെയുള്ള)));
  തിരികെ 1;
}


സ്ഥിതം ശൂന്യം setseed (lua_State *L, Rand64 *state,
                     lua_Unsigned n1, lua_Unsigned n2) {
  പൂർണ്ണസംഖ്യ i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  വേണ്ടി (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  lua_pushinteger(L, l_castU2S(n1));
  lua_pushinteger(L, l_castU2S(n2));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ math_randomseed (lua_State *L) {
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  lua_Unsigned n1, n2;
  എങ്കിൽ (lua_isnone(L, 1)) {
    n1 = luaL_makeseed(L);  /* "random" seed */
    n2 = I2UInt(nextrand(state->s));  /* in case seed is not that random... */
  }
  അല്ലെങ്കിൽ {
    n1 = l_castS2U(luaL_checkinteger(L, 1));
    n2 = l_castS2U(luaL_optinteger(L, 2, 0));
  }
  setseed(L, state->s, n1, n2);
  തിരികെ 2;  /* return seeds */
}


സ്ഥിതം സ്ഥിരം luaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {ശൂന്യ, ശൂന്യ}
};


/*
** Register the random functions and initialize their state.
*/
സ്ഥിതം ശൂന്യം setrandfunc (lua_State *L) {
  RanState *state = (RanState *)lua_newuserdatauv(L, വലുപ്പം(RanState), 0);
  setseed(L, state->s, luaL_makeseed(L), 0);  /* initialize with random seed */
  lua_എടുക്കുക(L, 2);  /* remove pushed seeds */
  luaL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#എങ്കിൽ defined(LUA_COMPAT_MATHLIB)

സ്ഥിതം പൂർണ്ണസംഖ്യ math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ math_pow (lua_State *L) {
  lua_സംഖ്യ x = luaL_checknumber(L, 1);
  lua_സംഖ്യ y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(pow)(x, y));
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  തിരികെ 1;
}

#endif
/* }================================================================== */



സ്ഥിതം സ്ഥിരം luaL_Reg mathlib[] = {
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
#എങ്കിൽ defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh", math_cosh},
  {"sinh", math_sinh},
  {"tanh", math_tanh},
  {"pow", math_pow},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"random", ശൂന്യ},
  {"randomseed", ശൂന്യ},
  {"pi", ശൂന്യ},
  {"huge", ശൂന്യ},
  {"maxinteger", ശൂന്യ},
  {"mininteger", ശൂന്യ},
  {ശൂന്യ, ശൂന്യ}
};


/*
** Open math library
*/
LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_math (lua_State *L) {
  luaL_newlib(L, mathlib);
  lua_pushnumber(L, PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_സംഖ്യ)HUGE_VAL);
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, LUA_MAXINTEGER);
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, LUA_MININTEGER);
  lua_setfield(L, -2, "mininteger");
  setrandfunc(L);
  തിരികെ 1;
}

