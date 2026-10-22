/*
** $Id: lmathlib.c $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <float.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <time.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


स्थिर पूर्णांक math_abs (lua_State *L) {
  जर (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    जर (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  अन्यथा
    lua_pushnumber(L, l_mathop(भ_असल)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_atan (lua_State *L) {
  lua_संख्या y = luaL_checknumber(L, 1);
  lua_संख्या x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  परत 1;
}


स्थिर पूर्णांक math_toint (lua_State *L) {
  पूर्णांक valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  जर (l_likely(valid))
    lua_pushinteger(L, n);
  अन्यथा {
    luaL_checkany(L, 1);
    luaL_pushfail(L);  /* value is not convertible to integer */
  }
  परत 1;
}


स्थिर शून्य pushnumint (lua_State *L, lua_संख्या d) {
  lua_Integer n;
  जर (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  अन्यथा
    lua_pushnumber(L, d);  /* result is float */
}


स्थिर पूर्णांक math_floor (lua_State *L) {
  जर (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  अन्यथा {
    lua_संख्या d = l_mathop(न्यूनमान)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  परत 1;
}


स्थिर पूर्णांक math_ceil (lua_State *L) {
  जर (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceiling */
  अन्यथा {
    lua_संख्या d = l_mathop(उच्चमान)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  परत 1;
}


स्थिर पूर्णांक math_fmod (lua_State *L) {
  जर (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    जर ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    अन्यथा
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  अन्यथा
    lua_pushnumber(L, l_mathop(भ_शेष)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  परत 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
स्थिर पूर्णांक math_modf (lua_State *L) {
  जर (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  अन्यथा {
    lua_संख्या n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_संख्या ip = (n < 0) ? l_mathop(उच्चमान)(n) : l_mathop(न्यूनमान)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  परत 2;
}


स्थिर पूर्णांक math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(वर्ग_मूल)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  परत 1;
}


स्थिर पूर्णांक math_log (lua_State *L) {
  lua_संख्या x = luaL_checknumber(L, 1);
  lua_संख्या res;
  जर (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  अन्यथा {
    lua_संख्या base = luaL_checknumber(L, 2);
#जर !defined(LUA_USE_C89)
    जर (base == l_mathop(2.0))
      res = l_mathop(log2)(x);
    अन्यथा
#endif
    जर (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    अन्यथा
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  परत 1;
}


स्थिर पूर्णांक math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  परत 1;
}


स्थिर पूर्णांक math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  परत 1;
}


स्थिर पूर्णांक math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  परत 1;
}


स्थिर पूर्णांक math_frexp (lua_State *L) {
  lua_संख्या x = luaL_checknumber(L, 1);
  पूर्णांक ep;
  lua_pushnumber(L, l_mathop(frexp)(x, &ep));
  lua_pushinteger(L, ep);
  परत 2;
}


स्थिर पूर्णांक math_ldexp (lua_State *L) {
  lua_संख्या x = luaL_checknumber(L, 1);
  पूर्णांक ep = (पूर्णांक)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  परत 1;
}


स्थिर पूर्णांक math_min (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक imin = 1;  /* index of current minimum value */
  पूर्णांक i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  साठी (i = 2; i <= n; i++) {
    जर (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  परत 1;
}


स्थिर पूर्णांक math_max (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक imax = 1;  /* index of current maximum value */
  पूर्णांक i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  साठी (i = 2; i <= n; i++) {
    जर (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  परत 1;
}


स्थिर पूर्णांक math_type (lua_State *L) {
  जर (lua_type(L, 1) == LUA_TNUMBER)
    lua_pushstring(L, (lua_isinteger(L, 1)) ? "integer" : "float");
  अन्यथा {
    luaL_checkany(L, 1);
    luaL_pushfail(L);
  }
  परत 1;
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

#जर FIGS > 64
/* there are only 64 random bits; use them all */
#undef FIGS
#define FIGS	64
#endif


/*
** LUA_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#जर !defined(LUA_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#जर ((अच_दीर्घ_उच्च >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#define Rand64		अचिन्हित दीर्घ
#define SRand64		दीर्घ

#किंवाजर !defined(LUA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#define Rand64		अचिन्हित दीर्घ दीर्घ
#define SRand64		दीर्घ दीर्घ

#किंवाजर ((LUA_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'lua_Unsigned' has at least 64 bits */
#define Rand64		lua_Unsigned
#define SRand64		lua_Integer

#endif

#endif


#जर defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
स्थिर Rand64 rotl (Rand64 x, पूर्णांक n) {
  परत (x << n) | (trim64(x) >> (64 - n));
}

स्थिर Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  परत res;
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

स्थिर lua_संख्या I2d (Rand64 x) {
  SRand64 sx = (SRand64)(trim64(x) >> shift64_FIG);
  lua_संख्या res = (lua_संख्या)(sx) * scaleFIG;
  जर (sx < 0)
    res += l_mathop(1.0);  /* correct the two's complement if negative */
  lua_assert(0 <= res && res < 1);
  परत res;
}

/* convert a 'Rand64' to a 'lua_Unsigned' */
#define I2UInt(x)	((lua_Unsigned)trim64(x))

/* convert a 'lua_Unsigned' to a 'Rand64' */
#define Int2I(x)	((Rand64)(x))


#अन्यथा	/* no 'Rand64'   }{ */

/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
प्रकारव्याख्या संरचना Rand64 {
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
स्थिर Rand64 packI (l_uint32 h, l_uint32 l) {
  Rand64 result;
  result.h = h;
  result.l = l;
  परत result;
}

/* return i << n */
स्थिर Rand64 Ishl (Rand64 i, पूर्णांक n) {
  lua_assert(n > 0 && n < 32);
  परत packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
स्थिर शून्य Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
स्थिर Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 result = packI(i1.h + i2.h, i1.l + i2.l);
  जर (trim32(result.l) < trim32(i1.l))  /* carry? */
    result.h++;
  परत result;
}

/* return i * 5 */
स्थिर Rand64 times5 (Rand64 i) {
  परत Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
स्थिर Rand64 times9 (Rand64 i) {
  परत Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
स्थिर Rand64 rotl (Rand64 i, पूर्णांक n) {
  lua_assert(n > 0 && n < 32);
  परत packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
स्थिर Rand64 rotl1 (Rand64 i, पूर्णांक n) {
  lua_assert(n > 32 && n < 64);
  n = 64 - n;
  परत packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
स्थिर Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  परत res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#define UONE		((l_uint32)1)


#जर FIGS <= 32

/* 2^(-FIGS) */
#define scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
स्थिर lua_संख्या I2d (Rand64 x) {
  lua_संख्या h = (lua_संख्या)(trim32(x.h) >> (32 - FIGS));
  परत h * scaleFIG;
}

#अन्यथा	/* 32 < FIGS <= 64 */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#define scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#define shift_खालचा	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#define shiftHI		((lua_संख्या)(UONE << (FIGS - 33)) * l_mathop(2.0))


स्थिर lua_संख्या I2d (Rand64 x) {
  lua_संख्या h = (lua_संख्या)trim32(x.h) * shiftHI;
  lua_संख्या l = (lua_संख्या)(trim32(x.l) >> shift_खालचा);
  परत (h + l) * scaleFIG;
}

#endif


/* convert a 'Rand64' to a 'lua_Unsigned' */
स्थिर lua_Unsigned I2UInt (Rand64 x) {
  परत (((lua_Unsigned)trim32(x.h) << 31) << 1) | (lua_Unsigned)trim32(x.l);
}

/* convert a 'lua_Unsigned' to a 'Rand64' */
स्थिर Rand64 Int2I (lua_Unsigned n) {
  परत packI((l_uint32)((n >> 31) >> 1), (l_uint32)n);
}

#endif  /* } */


/*
** A state uses four 'Rand64' values.
*/
प्रकारव्याख्या संरचना {
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
स्थिर lua_Unsigned project (lua_Unsigned ran, lua_Unsigned n,
                             RanState *state) {
  lua_Unsigned lim = n;  /* to compute the Mersenne number */
  पूर्णांक sh;  /* how much to spread bits to the right in 'lim' */
  /* spread '1' bits in 'lim' until it becomes a Mersenne number */
  साठी (sh = 1; (lim & (lim + 1)) != 0; sh *= 2)
    lim |= (lim >> sh);  /* spread '1's to the right */
  पर्यंत ((ran &= lim) > n)  /* project 'ran' into [0..lim] and test */
    ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
  परत ran;
}


स्थिर पूर्णांक math_random (lua_State *L) {
  lua_Integer खालचा, up;
  lua_Unsigned p;
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  निवड (lua_gettop(L)) {  /* check number of arguments */
    स्थिती 0: {  /* no arguments */
      lua_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      परत 1;
    }
    स्थिती 1: {  /* only upper limit */
      खालचा = 1;
      up = luaL_checkinteger(L, 1);
      जर (up == 0) {  /* single 0 as argument? */
        lua_pushinteger(L, l_castU2S(I2UInt(rv)));  /* full random integer */
        परत 1;
      }
      थांबा;
    }
    स्थिती 2: {  /* lower and upper limits */
      खालचा = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      थांबा;
    }
    पूर्वनिर्धारित: परत luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, खालचा <= up, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), l_castS2U(up) - l_castS2U(खालचा), state);
  lua_pushinteger(L, l_castU2S(p + l_castS2U(खालचा)));
  परत 1;
}


स्थिर शून्य setseed (lua_State *L, Rand64 *state,
                     lua_Unsigned n1, lua_Unsigned n2) {
  पूर्णांक i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  साठी (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  lua_pushinteger(L, l_castU2S(n1));
  lua_pushinteger(L, l_castU2S(n2));
}


स्थिर पूर्णांक math_randomseed (lua_State *L) {
  RanState *state = (RanState *)lua_touserdata(L, lua_upvalueindex(1));
  lua_Unsigned n1, n2;
  जर (lua_isnone(L, 1)) {
    n1 = luaL_makeseed(L);  /* "random" seed */
    n2 = I2UInt(nextrand(state->s));  /* in case seed is not that random... */
  }
  अन्यथा {
    n1 = l_castS2U(luaL_checkinteger(L, 1));
    n2 = l_castS2U(luaL_optinteger(L, 2, 0));
  }
  setseed(L, state->s, n1, n2);
  परत 2;  /* return seeds */
}


स्थिर स्थिरांक luaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {रिक्त, रिक्त}
};


/*
** Register the random functions and initialize their state.
*/
स्थिर शून्य setrandfunc (lua_State *L) {
  RanState *state = (RanState *)lua_newuserdatauv(L, आकार(RanState), 0);
  setseed(L, state->s, luaL_makeseed(L), 0);  /* initialize with random seed */
  lua_काढा(L, 2);  /* remove pushed seeds */
  luaL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#जर defined(LUA_COMPAT_MATHLIB)

स्थिर पूर्णांक math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  परत 1;
}

स्थिर पूर्णांक math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  परत 1;
}

स्थिर पूर्णांक math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  परत 1;
}

स्थिर पूर्णांक math_pow (lua_State *L) {
  lua_संख्या x = luaL_checknumber(L, 1);
  lua_संख्या y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(घात)(x, y));
  परत 1;
}

स्थिर पूर्णांक math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  परत 1;
}

#endif
/* }================================================================== */



स्थिर स्थिरांक luaL_Reg mathlib[] = {
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
#जर defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh", math_cosh},
  {"sinh", math_sinh},
  {"tanh", math_tanh},
  {"pow", math_pow},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"random", रिक्त},
  {"randomseed", रिक्त},
  {"pi", रिक्त},
  {"huge", रिक्त},
  {"maxinteger", रिक्त},
  {"mininteger", रिक्त},
  {रिक्त, रिक्त}
};


/*
** Open math library
*/
LUAMOD_API पूर्णांक luaopen_math (lua_State *L) {
  luaL_newlib(L, mathlib);
  lua_pushnumber(L, PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_संख्या)विशाल_मान);
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, LUA_MAXINTEGER);
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, LUA_MININTEGER);
  lua_setfield(L, -2, "mininteger");
  setrandfunc(L);
  परत 1;
}

