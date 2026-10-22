/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lobject_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <float.h>
#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lctype.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  ಪೂರ್ಣಾಂಕ l = 0;
  x--;
  ಆಗ (x >= 256) { l += 8; x >>= 8; }
  ಹಿಂತಿರುಗಿ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ p) {
  ಆದರೆ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ಹಿಂತಿರುಗಿ 0xFF;  /* return maximum value */
  ಇಲ್ಲದಿದ್ದರೆ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ಆದರೆ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ಹಿಂತಿರುಗಿ cast_byte(p);
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      ಚಿಹ್ನೆರಹಿತ ಲಘುಗಣಕ = luaO_ceillog2(p + 1) - 5u;
      ಹಿಂತಿರುಗಿ cast_byte(((p >> ಲಘುಗಣಕ) - 0x10) | ((ಲಘುಗಣಕ + 1) << 4));
    }
  }
}


/*
** Computes 'p' times 'x', where 'p' is a floating-point byte. Roughly,
** we have to multiply 'x' by the mantissa and then shift accordingly to
** the exponent.  If the exponent is positive, both the multiplication
** and the shift increase 'x', so we have to care only about overflows.
** For negative exponents, however, multiplying before the shift keeps
** more significant bits, as long as the multiplication does not
** overflow, so we check which order is best.
*/
l_mem luaO_applyparam (lu_byte p, l_mem x) {
  ಪೂರ್ಣಾಂಕ m = p & 0xF;  /* mantissa */
  ಪೂರ್ಣಾಂಕ e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ಆದರೆ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ಆದರೆ (e >= 0) {
    ಆದರೆ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ಹಿಂತಿರುಗಿ (x * m) << e;  /* order doesn't matter here */
    ಇಲ್ಲದಿದ್ದರೆ  /* real overflow */
      ಹಿಂತಿರುಗಿ MAX_LMEM;
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* negative exponent */
    e = -e;
    ಆದರೆ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ಹಿಂತಿರುಗಿ (x * m) >> e;  /* multiplying first gives more precision */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ಹಿಂತಿರುಗಿ (x >> e) * m;
    ಇಲ್ಲದಿದ್ದರೆ  /* real overflow */
      ಹಿಂತಿರುಗಿ MAX_LMEM;
  }
}


ಸ್ಥಿರ lua_Integer intarith (lua_State *L, ಪೂರ್ಣಾಂಕ op, lua_Integer v1,
                                                   lua_Integer v2) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ LUA_OPADD: ಹಿಂತಿರುಗಿ intop(+, v1, v2);
    ಸ್ಥಿತಿ LUA_OPSUB:ಹಿಂತಿರುಗಿ intop(-, v1, v2);
    ಸ್ಥಿತಿ LUA_OPMUL:ಹಿಂತಿರುಗಿ intop(*, v1, v2);
    ಸ್ಥಿತಿ LUA_OPMOD: ಹಿಂತಿರುಗಿ luaV_mod(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPIDIV: ಹಿಂತಿರುಗಿ luaV_idiv(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPBAND: ಹಿಂತಿರುಗಿ intop(&, v1, v2);
    ಸ್ಥಿತಿ LUA_OPBOR: ಹಿಂತಿರುಗಿ intop(|, v1, v2);
    ಸ್ಥಿತಿ LUA_OPBXOR: ಹಿಂತಿರುಗಿ intop(^, v1, v2);
    ಸ್ಥಿತಿ LUA_OPSHL: ಹಿಂತಿರುಗಿ luaV_shiftl(v1, v2);
    ಸ್ಥಿತಿ LUA_OPSHR: ಹಿಂತಿರುಗಿ luaV_shiftr(v1, v2);
    ಸ್ಥಿತಿ LUA_OPUNM: ಹಿಂತಿರುಗಿ intop(-, 0, v1);
    ಸ್ಥಿತಿ LUA_OPBNOT: ಹಿಂತಿರುಗಿ intop(^, ~l_castS2U(0), v1);
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ numarith (lua_State *L, ಪೂರ್ಣಾಂಕ op, lua_ಸಂಖ್ಯೆ v1,
                                                  lua_ಸಂಖ್ಯೆ v2) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ LUA_OPADD: ಹಿಂತಿರುಗಿ luai_numadd(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPSUB: ಹಿಂತಿರುಗಿ luai_numsub(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPMUL: ಹಿಂತಿರುಗಿ luai_nummul(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPDIV: ಹಿಂತಿರುಗಿ luai_numdiv(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPPOW: ಹಿಂತಿರುಗಿ luai_numpow(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPIDIV: ಹಿಂತಿರುಗಿ luai_numidiv(L, v1, v2);
    ಸ್ಥಿತಿ LUA_OPUNM: ಹಿಂತಿರುಗಿ luai_numunm(L, v1);
    ಸ್ಥಿತಿ LUA_OPMOD: ಹಿಂತಿರುಗಿ luaV_modf(L, v1, v2);
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ಹಿಂತಿರುಗಿ 0;
  }
}


ಪೂರ್ಣಾಂಕ luaO_rawarith (lua_State *L, ಪೂರ್ಣಾಂಕ op, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                   TValue *res) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ LUA_OPBAND: ಸ್ಥಿತಿ LUA_OPBOR: ಸ್ಥಿತಿ LUA_OPBXOR:
    ಸ್ಥಿತಿ LUA_OPSHL: ಸ್ಥಿತಿ LUA_OPSHR:
    ಸ್ಥಿತಿ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ಆದರೆ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;  /* fail */
    }
    ಸ್ಥಿತಿ LUA_OPDIV: ಸ್ಥಿತಿ LUA_OPPOW: {  /* operate only on floats */
      lua_ಸಂಖ್ಯೆ n1; lua_ಸಂಖ್ಯೆ n2;
      ಆದರೆ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;  /* fail */
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* other operations */
      lua_ಸಂಖ್ಯೆ n1; lua_ಸಂಖ್ಯೆ n2;
      ಆದರೆ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;  /* fail */
    }
  }
}


ಶೂನ್ಯ luaO_arith (lua_State *L, ಪೂರ್ಣಾಂಕ op, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                 StkId res) {
  ಆದರೆ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ಕೂಡಿಸು));
  }
}


lu_byte luaO_hexavalue (ಪೂರ್ಣಾಂಕ c) {
  lua_assert(lisxdigit(c));
  ಆದರೆ (lisdigit(c)) ಹಿಂತಿರುಗಿ cast_byte(c - '0');
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ cast_byte((ltolower(c) - 'a') + 10);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isneg (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **s) {
  ಆದರೆ (**s == '-') { (*s)++; ಹಿಂತಿರುಗಿ 1; }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**s == '+') (*s)++;
  ಹಿಂತಿರುಗಿ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ಆದರೆ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ lua_strx2number (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಅಕ್ಷರ **endptr) {
  ಪೂರ್ಣಾಂಕ dot = lua_getlocaledecpoint();
  lua_ಸಂಖ್ಯೆ r = l_mathop(0.0);  /* result (accumulator) */
  ಪೂರ್ಣಾಂಕ sigdig = 0;  /* number of significant digits */
  ಪೂರ್ಣಾಂಕ nosigdig = 0;  /* number of non-significant digits */
  ಪೂರ್ಣಾಂಕ e = 0;  /* exponent correction */
  ಪೂರ್ಣಾಂಕ neg;  /* 1 if number is negative */
  ಪೂರ್ಣಾಂಕ hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ಆಗ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ಆದರೆ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ಹಿಂತಿರುಗಿ l_mathop(0.0);  /* invalid format (no '0x') */
  ಗಾಗಿ (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ಆದರೆ (*s == dot) {
      ಆದರೆ (hasdot) ನಿಲ್ಲಿಸು;  /* second dot? stop loop */
      ಇಲ್ಲದಿದ್ದರೆ hasdot = 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lisxdigit(cast_uchar(*s))) {
      ಆದರೆ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      ಇಲ್ಲದಿದ್ದರೆ e++;  /* too many digits; ignore, but still count for exponent */
      ಆದರೆ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    ಇಲ್ಲದಿದ್ದರೆ ನಿಲ್ಲಿಸು;  /* neither a dot nor a digit */
  }
  ಆದರೆ (nosigdig + sigdig == 0)  /* no digits? */
    ಹಿಂತಿರುಗಿ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ಆದರೆ (*s == 'p' || *s == 'P') {  /* exponent part? */
    ಪೂರ್ಣಾಂಕ exp1 = 0;  /* exponent value */
    ಪೂರ್ಣಾಂಕ neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ಆದರೆ (!lisdigit(cast_uchar(*s)))
      ಹಿಂತಿರುಗಿ l_mathop(0.0);  /* invalid; must have at least one digit */
    ಆಗ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ಆದರೆ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ಆದರೆ (neg) r = -r;
  ಹಿಂತಿರುಗಿ l_mathop(ldexp)(r, e);
}

#ಆದರೆ_ಅಂತ್ಯ
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ಆದರೆ !defined (L_MAXLENNUM)
#ವ್ಯಾಖ್ಯಾನಿಸು L_MAXLENNUM	200
#ಆದರೆ_ಅಂತ್ಯ

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *l_str2dloc (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, lua_ಸಂಖ್ಯೆ *ಫಲಿತಾಂಶ, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
  ಅಕ್ಷರ *endptr;
  *ಫಲಿತಾಂಶ = (ಬಹುಲಕ == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ಆದರೆ (endptr == s) ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* nothing recognized? */
  ಆಗ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ಹಿಂತಿರುಗಿ (*endptr == '\0') ? endptr : ಖಾಲಿ;  /* OK iff no trailing chars */
}


/*
** Convert string 's' to a Lua number (put in 'result') handling the
** current locale.
** This function accepts both the current locale or a dot as the radix
** mark. If the conversion fails, it may mean number has a dot but
** locale accepts something else. In that case, the code copies 's'
** to a buffer (because 's' is read-only), changes the dot to the
** current locale radix mark, and tries to convert again.
** The variable 'mode' checks for special characters in the string:
** - 'n' means 'inf' or 'nan' (which should be rejected)
** - 'x' means a hexadecimal numeral
** - '.' just optimizes the search for the common case (no special chars)
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *l_str2d (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, lua_ಸಂಖ್ಯೆ *ಫಲಿತಾಂಶ) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *endptr;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ಪೂರ್ಣಾಂಕ ಬಹುಲಕ = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ಆದರೆ (ಬಹುಲಕ == 'n')  /* reject 'inf' and 'nan' */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  endptr = l_str2dloc(s, ಫಲಿತಾಂಶ, ಬಹುಲಕ);  /* try to convert */
  ಆದರೆ (endptr == ಖಾಲಿ) {  /* failed? may be a different locale */
    ಅಕ್ಷರ buff[L_MAXLENNUM + 1];
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pdot = strchr(s, '.');
    ಆದರೆ (pdot == ಖಾಲಿ || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) > L_MAXLENNUM)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, ಫಲಿತಾಂಶ, ಬಹುಲಕ);  /* try again */
    ಆದರೆ (endptr != ಖಾಲಿ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ಹಿಂತಿರುಗಿ endptr;
}


#ವ್ಯಾಖ್ಯಾನಿಸು MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#ವ್ಯಾಖ್ಯಾನಿಸು MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *l_str2int (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, lua_Integer *ಫಲಿತಾಂಶ) {
  lua_Unsigned a = 0;
  ಪೂರ್ಣಾಂಕ ಖಾಲಿಗಳು = 1;
  ಪೂರ್ಣಾಂಕ neg;
  ಆಗ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ಆದರೆ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    ಗಾಗಿ (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      ಖಾಲಿಗಳು = 0;
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* decimal */
    ಗಾಗಿ (; lisdigit(cast_uchar(*s)); s++) {
      ಪೂರ್ಣಾಂಕ d = *s - '0';
      ಆದರೆ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      ಖಾಲಿಗಳು = 0;
    }
  }
  ಆಗ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ಆದರೆ (ಖಾಲಿಗಳು || *s != '\0') ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* something wrong in the numeral */
  ಇಲ್ಲದಿದ್ದರೆ {
    *ಫಲಿತಾಂಶ = l_castU2S((neg) ? 0u - a : a);
    ಹಿಂತಿರುಗಿ s;
  }
}


size_t luaO_str2num (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, TValue *o) {
  lua_Integer i; lua_ಸಂಖ್ಯೆ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e;
  ಆದರೆ ((e = l_str2int(s, &i)) != ಖಾಲಿ) {  /* try as an integer */
    setivalue(o, i);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((e = l_str2d(s, &n)) != ಖಾಲಿ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;  /* conversion failed */
  ಹಿಂತಿರುಗಿ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ಪೂರ್ಣಾಂಕ luaO_utf8esc (ಅಕ್ಷರ *buff, l_uint32 x) {
  ಪೂರ್ಣಾಂಕ n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ಆದರೆ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  ಇಲ್ಲದಿದ್ದರೆ {  /* need continuation bytes */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ mfb = 0x3f;  /* maximum that fits in first byte */
    ಮಾಡು {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ಆಗ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ಹಿಂತಿರುಗಿ n;
}


/*
** The size of the buffer for the conversion of a number to a string
** 'LUA_N2SBUFFSZ' must be enough to accommodate both LUA_INTEGER_FMT
** and LUA_NUMBER_FMT.  For a long long int, this is 19 digits plus a
** sign and a final '\0', adding to 21. For a long double, it can go to
** a sign, the dot, an exponent letter, an exponent sign, 4 exponent
** digits, the final '\0', plus the significant digits, which are
** approximately the *_DIG attribute.
*/
#ಆದರೆ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#ದೋಷ "invalid value for LUA_N2SBUFFSZ"
#ಆದರೆ_ಅಂತ್ಯ


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tostringbuffFloat (lua_ಸಂಖ್ಯೆ n, ಅಕ್ಷರ *buff) {
  /* first conversion */
  ಪೂರ್ಣಾಂಕ len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ಸಂಖ್ಯೆ_FMT,
                            (LUAI_UACNUMBER)n);
  lua_ಸಂಖ್ಯೆ check = lua_str2number(buff, ಖಾಲಿ);  /* read it back */
  ಆದರೆ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ಸಂಖ್ಯೆ_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ಆದರೆ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  ಹಿಂತಿರುಗಿ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
ಚಿಹ್ನೆರಹಿತ luaO_tostringbuff (ಸ್ಥಿರಾಂಕ TValue *obj, ಅಕ್ಷರ *buff) {
  ಪೂರ್ಣಾಂಕ len;
  lua_assert(ttisnumber(obj));
  ಆದರೆ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  ಇಲ್ಲದಿದ್ದರೆ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ಹಿಂತಿರುಗಿ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
ಶೂನ್ಯ luaO_tostring (lua_State *L, TValue *obj) {
  ಅಕ್ಷರ buff[LUA_N2SBUFFSZ];
  ಚಿಹ್ನೆರಹಿತ len = luaO_tostringbuff(obj, buff);
  setsvalue(L, obj, luaS_newlstr(L, buff, len));
}




/*
** {==================================================================
** 'luaO_pushvfstring'
** ===================================================================
*/

/*
** Size for buffer space used by 'luaO_pushvfstring'. It should be
** (LUA_IDSIZE + LUA_N2SBUFFSZ) + a minimal space for basic messages,
** so that 'luaG_addinfo' can work directly on the static buffer.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

/*
** Buffer used by 'luaO_pushvfstring'. 'err' signals an error while
** building result (memory error [1] or buffer overflow [2]).
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BuffFS {
  lua_State *L;
  ಅಕ್ಷರ *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ಪೂರ್ಣಾಂಕ err;
  ಅಕ್ಷರ space[BUFVFS];  /* initial buffer */
} BuffFS;


ಸ್ಥಿರ ಶೂನ್ಯ initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = ಗಾತ್ರ(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pushbuff (lua_State *L, ಶೂನ್ಯ *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  ಆಯ್ಕೆ (buff->err) {
    ಸ್ಥಿತಿ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:  /* length overflow: Add "..." at the end of result */
      ಆದರೆ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      ಇಲ್ಲದಿದ್ದರೆ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->ತುದಿ.p, ts);
      L->ತುದಿ.p++;
    }
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res;
  ಆದರೆ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ಖಾಲಿ;  /* error message is on the top of the stack */
  ಇಲ್ಲದಿದ್ದರೆ
    res = getstr(tsvalue(s2v(L->ತುದಿ.p - 1)));
  ಆದರೆ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಶೂನ್ಯ addstr2buff (BuffFS *buff, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t slen) {
  size_t ಎಡ = buff->buffsize - buff->blen;  /* space left in the buffer */
  ಆದರೆ (buff->err)  /* do nothing else after an error */
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (slen > ಎಡ) {  /* new string doesn't fit into current buffer? */
    ಆದರೆ (slen > ((MAX_ಗಾತ್ರ/2) - buff->blen)) {  /* overflow? */
      ಸ್ಮರಣೆ_ನಕಲು(buff->b + buff->blen, str, ಎಡ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ಹಿಂತಿರುಗಿ;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      ಅಕ್ಷರ *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ಖಾಲಿ, 0, newsize, ಅಕ್ಷರ)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               ಅಕ್ಷರ);
      ಆದರೆ (newb == ಖಾಲಿ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ಹಿಂತಿರುಗಿ;
      }
      ಆದರೆ (buff->b == buff->space)  /* new buffer (not reallocated)? */
        ಸ್ಮರಣೆ_ನಕಲು(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  ಸ್ಮರಣೆ_ನಕಲು(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
ಸ್ಥಿರ ಶೂನ್ಯ addnum2buff (BuffFS *buff, TValue *ಅಂಕೆ) {
  ಅಕ್ಷರ numbuff[LUA_N2SBUFFSZ];
  ಚಿಹ್ನೆರಹಿತ len = luaO_tostringbuff(ಅಂಕೆ, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaO_pushvfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e;  /* points to next '%' */
  initbuff(L, &buff);
  ಆಗ ((e = strchr(fmt, '%')) != ಖಾಲಿ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    ಆಯ್ಕೆ (*(e + 1)) {  /* conversion specifier */
      ಸ್ಥಿತಿ 's': {  /* zero-terminated string */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = va_arg(argp, ಅಕ್ಷರ *);
        ಆದರೆ (s == ಖಾಲಿ) s = "(null)";
        addstr2buff(&buff, s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s));
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'c': {  /* an 'int' as a character */
        ಅಕ್ಷರ c = cast_char(va_arg(argp, ಪೂರ್ಣಾಂಕ));
        addstr2buff(&buff, &c, ಗಾತ್ರ(ಅಕ್ಷರ));
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'd': {  /* an 'int' */
        TValue ಅಂಕೆ;
        setivalue(&ಅಂಕೆ, va_arg(argp, ಪೂರ್ಣಾಂಕ));
        addnum2buff(&buff, &ಅಂಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'I': {  /* a 'lua_Integer' */
        TValue ಅಂಕೆ;
        setivalue(&ಅಂಕೆ, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &ಅಂಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'f': {  /* a 'lua_Number' */
        TValue ಅಂಕೆ;
        setfltvalue(&ಅಂಕೆ, cast_ಅಂಕೆ(va_arg(argp, l_uac_ಸಂಖ್ಯೆ)));
        addnum2buff(&buff, &ಅಂಕೆ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'p': {  /* a pointer */
        ಅಕ್ಷರ bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        ಶೂನ್ಯ *p = va_arg(argp, ಶೂನ್ಯ *);
        ಪೂರ್ಣಾಂಕ len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        ಅಕ್ಷರ bf[UTF8BUFFSZ];
        ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ arg = va_arg(argp, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ);
        ಪೂರ್ಣಾಂಕ len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '%': {
        addstr2buff(&buff, "%", 1);
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ನಿಲ್ಲಿಸು;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(fmt));  /* rest of 'fmt' */
  ಹಿಂತಿರುಗಿ clearbuff(&buff);  /* empty buffer into a new string */
}


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaO_pushfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
  va_list argp;
  va_start(argp, fmt);
  ಸಂದೇಶ = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ಹಿಂತಿರುಗಿ ಸಂದೇಶ;
}

/* }================================================================== */


#ವ್ಯಾಖ್ಯಾನಿಸು RETS	"..."
#ವ್ಯಾಖ್ಯಾನಿಸು PRE	"[string \""
#ವ್ಯಾಖ್ಯಾನಿಸು POS	"\"]"

#ವ್ಯಾಖ್ಯಾನಿಸು addstr(a,b,l)	( ಸ್ಮರಣೆ_ನಕಲು(a,b,(l) * ಗಾತ್ರ(ಅಕ್ಷರ)), a += (l) )

ಶೂನ್ಯ luaO_chunkid (ಅಕ್ಷರ *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ಆದರೆ (*ಮೂಲ == '=') {  /* 'literal' source */
    ಆದರೆ (srclen <= bufflen)  /* small enough? */
      ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು, ಮೂಲ + 1, srclen * ಗಾತ್ರ(ಅಕ್ಷರ));
    ಇಲ್ಲದಿದ್ದರೆ {  /* truncate it */
      addstr(ಹೊರಹೋಗು, ಮೂಲ + 1, bufflen - 1);
      *ಹೊರಹೋಗು = '\0';
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ಮೂಲ == '@') {  /* file name */
    ಆದರೆ (srclen <= bufflen)  /* small enough? */
      ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು, ಮೂಲ + 1, srclen * ಗಾತ್ರ(ಅಕ್ಷರ));
    ಇಲ್ಲದಿದ್ದರೆ {  /* add '...' before rest of name */
      addstr(ಹೊರಹೋಗು, RETS, LL(RETS));
      bufflen -= LL(RETS);
      ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು, ಮೂಲ + 1 + srclen - bufflen, bufflen * ಗಾತ್ರ(ಅಕ್ಷರ));
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* string; format as [string "source"] */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nl = strchr(ಮೂಲ, '\n');  /* find first new line (if any) */
    addstr(ಹೊರಹೋಗು, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ಆದರೆ (srclen < bufflen && nl == ಖಾಲಿ) {  /* small one-line source? */
      addstr(ಹೊರಹೋಗು, ಮೂಲ, srclen);  /* keep it */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (nl != ಖಾಲಿ)
        srclen = ct_diff2sz(nl - ಮೂಲ);  /* stop at first newline */
      ಆದರೆ (srclen > bufflen) srclen = bufflen;
      addstr(ಹೊರಹೋಗು, ಮೂಲ, srclen);
      addstr(ಹೊರಹೋಗು, RETS, LL(RETS));
    }
    ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು, POS, (LL(POS) + 1) * ಗಾತ್ರ(ಅಕ್ಷರ));
  }
}

