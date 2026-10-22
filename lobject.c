/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

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
lu_byte luaO_ceillog2 (unsigned ಪೂರ್ಣಾಙ್ಕ x) {
  ಸ್ಥಿರ ಧ್ರುವ lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  ಪೂರ್ಣಾಙ್ಕ l = 0;
  x--;
  ಯಾವತ್ (x >= 256) { l += 8; x >>= 8; }
  ಪ್ರತ್ಯಾಗಮ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned ಪೂರ್ಣಾಙ್ಕ p) {
  ಯದಿ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ಪ್ರತ್ಯಾಗಮ 0xFF;  /* return maximum value */
  ಅನ್ಯಥಾ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ಯದಿ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ಪ್ರತ್ಯಾಗಮ cast_byte(p);
    }
    ಅನ್ಯಥಾ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      ಪ್ರತ್ಯಾಗಮ cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  ಪೂರ್ಣಾಙ್ಕ m = p & 0xF;  /* mantissa */
  ಪೂರ್ಣಾಙ್ಕ e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ಯದಿ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ಯದಿ (e >= 0) {
    ಯದಿ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ಪ್ರತ್ಯಾಗಮ (x * m) << e;  /* order doesn't matter here */
    ಅನ್ಯಥಾ  /* real overflow */
      ಪ್ರತ್ಯಾಗಮ MAX_LMEM;
  }
  ಅನ್ಯಥಾ {  /* negative exponent */
    e = -e;
    ಯದಿ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ಪ್ರತ್ಯಾಗಮ (x * m) >> e;  /* multiplying first gives more precision */
    ಅನ್ಯಥಾ ಯದಿ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ಪ್ರತ್ಯಾಗಮ (x >> e) * m;
    ಅನ್ಯಥಾ  /* real overflow */
      ಪ್ರತ್ಯಾಗಮ MAX_LMEM;
  }
}


ಸ್ಥಿರ lua_Integer intarith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, lua_Integer v1,
                                                   lua_Integer v2) {
  ವಿಕಲ್ಪ (op) {
    ಪಕ್ಷ LUA_OPADD: ಪ್ರತ್ಯಾಗಮ intop(+, v1, v2);
    ಪಕ್ಷ LUA_OPSUB:ಪ್ರತ್ಯಾಗಮ intop(-, v1, v2);
    ಪಕ್ಷ LUA_OPMUL:ಪ್ರತ್ಯಾಗಮ intop(*, v1, v2);
    ಪಕ್ಷ LUA_OPMOD: ಪ್ರತ್ಯಾಗಮ luaV_mod(L, v1, v2);
    ಪಕ್ಷ LUA_OPIDIV: ಪ್ರತ್ಯಾಗಮ luaV_idiv(L, v1, v2);
    ಪಕ್ಷ LUA_OPBAND: ಪ್ರತ್ಯಾಗಮ intop(&, v1, v2);
    ಪಕ್ಷ LUA_OPBOR: ಪ್ರತ್ಯಾಗಮ intop(|, v1, v2);
    ಪಕ್ಷ LUA_OPBXOR: ಪ್ರತ್ಯಾಗಮ intop(^, v1, v2);
    ಪಕ್ಷ LUA_OPSHL: ಪ್ರತ್ಯಾಗಮ luaV_shiftl(v1, v2);
    ಪಕ್ಷ LUA_OPSHR: ಪ್ರತ್ಯಾಗಮ luaV_shiftr(v1, v2);
    ಪಕ್ಷ LUA_OPUNM: ಪ್ರತ್ಯಾಗಮ intop(-, 0, v1);
    ಪಕ್ಷ LUA_OPBNOT: ಪ್ರತ್ಯಾಗಮ intop(^, ~l_castS2U(0), v1);
    ಸ್ವಭಾವ: lua_assert(0); ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ numarith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, lua_ಸಂಖ್ಯೆ v1,
                                                  lua_ಸಂಖ್ಯೆ v2) {
  ವಿಕಲ್ಪ (op) {
    ಪಕ್ಷ LUA_OPADD: ಪ್ರತ್ಯಾಗಮ luai_numadd(L, v1, v2);
    ಪಕ್ಷ LUA_OPSUB: ಪ್ರತ್ಯಾಗಮ luai_numsub(L, v1, v2);
    ಪಕ್ಷ LUA_OPMUL: ಪ್ರತ್ಯಾಗಮ luai_nummul(L, v1, v2);
    ಪಕ್ಷ LUA_OPDIV: ಪ್ರತ್ಯಾಗಮ luai_numdiv(L, v1, v2);
    ಪಕ್ಷ LUA_OPPOW: ಪ್ರತ್ಯಾಗಮ luai_numpow(L, v1, v2);
    ಪಕ್ಷ LUA_OPIDIV: ಪ್ರತ್ಯಾಗಮ luai_numidiv(L, v1, v2);
    ಪಕ್ಷ LUA_OPUNM: ಪ್ರತ್ಯಾಗಮ luai_numunm(L, v1);
    ಪಕ್ಷ LUA_OPMOD: ಪ್ರತ್ಯಾಗಮ luaV_modf(L, v1, v2);
    ಸ್ವಭಾವ: lua_assert(0); ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಪೂರ್ಣಾಙ್ಕ luaO_rawarith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                   TValue *res) {
  ವಿಕಲ್ಪ (op) {
    ಪಕ್ಷ LUA_OPBAND: ಪಕ್ಷ LUA_OPBOR: ಪಕ್ಷ LUA_OPBXOR:
    ಪಕ್ಷ LUA_OPSHL: ಪಕ್ಷ LUA_OPSHR:
    ಪಕ್ಷ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ಯದಿ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;  /* fail */
    }
    ಪಕ್ಷ LUA_OPDIV: ಪಕ್ಷ LUA_OPPOW: {  /* operate only on floats */
      lua_ಸಂಖ್ಯೆ n1; lua_ಸಂಖ್ಯೆ n2;
      ಯದಿ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;  /* fail */
    }
    ಸ್ವಭಾವ: {  /* other operations */
      lua_ಸಂಖ್ಯೆ n1; lua_ಸಂಖ್ಯೆ n2;
      ಯದಿ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ ಯದಿ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;  /* fail */
    }
  }
}


ಶೂನ್ಯ luaO_arith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                 StkId res) {
  ಯದಿ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (ಪೂರ್ಣಾಙ್ಕ c) {
  lua_assert(lisxdigit(c));
  ಯದಿ (lisdigit(c)) ಪ್ರತ್ಯಾಗಮ cast_byte(c - '0');
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ cast_byte((ltolower(c) - 'a') + 10);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isneg (ಧ್ರುವ ವರ್ಣ **s) {
  ಯದಿ (**s == '-') { (*s)++; ಪ್ರತ್ಯಾಗಮ 1; }
  ಅನ್ಯಥಾ ಯದಿ (**s == '+') (*s)++;
  ಪ್ರತ್ಯಾಗಮ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ಯದಿ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ lua_strx2number (ಧ್ರುವ ವರ್ಣ *s, ವರ್ಣ **endptr) {
  ಪೂರ್ಣಾಙ್ಕ dot = lua_getlocaledecpoint();
  lua_ಸಂಖ್ಯೆ r = l_mathop(0.0);  /* result (accumulator) */
  ಪೂರ್ಣಾಙ್ಕ sigdig = 0;  /* number of significant digits */
  ಪೂರ್ಣಾಙ್ಕ nosigdig = 0;  /* number of non-significant digits */
  ಪೂರ್ಣಾಙ್ಕ e = 0;  /* exponent correction */
  ಪೂರ್ಣಾಙ್ಕ neg;  /* 1 if number is negative */
  ಪೂರ್ಣಾಙ್ಕ hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ಯಾವತ್ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ಯದಿ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ಪ್ರತ್ಯಾಗಮ l_mathop(0.0);  /* invalid format (no '0x') */
  ಪ್ರತಿ (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ಯದಿ (*s == dot) {
      ಯದಿ (hasdot) ಭಙ್ಗ;  /* second dot? stop loop */
      ಅನ್ಯಥಾ hasdot = 1;
    }
    ಅನ್ಯಥಾ ಯದಿ (lisxdigit(cast_uchar(*s))) {
      ಯದಿ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      ಅನ್ಯಥಾ ಯದಿ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      ಅನ್ಯಥಾ e++;  /* too many digits; ignore, but still count for exponent */
      ಯದಿ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    ಅನ್ಯಥಾ ಭಙ್ಗ;  /* neither a dot nor a digit */
  }
  ಯದಿ (nosigdig + sigdig == 0)  /* no digits? */
    ಪ್ರತ್ಯಾಗಮ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ಯದಿ (*s == 'p' || *s == 'P') {  /* exponent part? */
    ಪೂರ್ಣಾಙ್ಕ exp1 = 0;  /* exponent value */
    ಪೂರ್ಣಾಙ್ಕ neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ಯದಿ (!lisdigit(cast_uchar(*s)))
      ಪ್ರತ್ಯಾಗಮ l_mathop(0.0);  /* invalid; must have at least one digit */
    ಯಾವತ್ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ಯದಿ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ಯದಿ (neg) r = -r;
  ಪ್ರತ್ಯಾಗಮ l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ಯದಿ !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *l_str2dloc (ಧ್ರುವ ವರ್ಣ *s, lua_ಸಂಖ್ಯೆ *result, ಪೂರ್ಣಾಙ್ಕ mode) {
  ವರ್ಣ *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ಯದಿ (endptr == s) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* nothing recognized? */
  ಯಾವತ್ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ಪ್ರತ್ಯಾಗಮ (*endptr == '\0') ? endptr : ರಿಕ್ತ;  /* OK iff no trailing chars */
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
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *l_str2d (ಧ್ರುವ ವರ್ಣ *s, lua_ಸಂಖ್ಯೆ *result) {
  ಧ್ರುವ ವರ್ಣ *endptr;
  ಧ್ರುವ ವರ್ಣ *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ಪೂರ್ಣಾಙ್ಕ mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ಯದಿ (mode == 'n')  /* reject 'inf' and 'nan' */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  ಯದಿ (endptr == ರಿಕ್ತ) {  /* failed? may be a different locale */
    ವರ್ಣ buff[L_MAXLENNUM + 1];
    ಧ್ರುವ ವರ್ಣ *pdot = strchr(s, '.');
    ಯದಿ (pdot == ರಿಕ್ತ || strlen(s) > L_MAXLENNUM)
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    ಯದಿ (endptr != ರಿಕ್ತ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ಪ್ರತ್ಯಾಗಮ endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *l_str2int (ಧ್ರುವ ವರ್ಣ *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  ಪೂರ್ಣಾಙ್ಕ empty = 1;
  ಪೂರ್ಣಾಙ್ಕ neg;
  ಯಾವತ್ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ಯದಿ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    ಪ್ರತಿ (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  ಅನ್ಯಥಾ {  /* decimal */
    ಪ್ರತಿ (; lisdigit(cast_uchar(*s)); s++) {
      ಪೂರ್ಣಾಙ್ಕ d = *s - '0';
      ಯದಿ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  ಯಾವತ್ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ಯದಿ (empty || *s != '\0') ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* something wrong in the numeral */
  ಅನ್ಯಥಾ {
    *result = l_castU2S((neg) ? 0u - a : a);
    ಪ್ರತ್ಯಾಗಮ s;
  }
}


size_t luaO_str2num (ಧ್ರುವ ವರ್ಣ *s, TValue *o) {
  lua_Integer i; lua_ಸಂಖ್ಯೆ n;
  ಧ್ರುವ ವರ್ಣ *e;
  ಯದಿ ((e = l_str2int(s, &i)) != ರಿಕ್ತ) {  /* try as an integer */
    setivalue(o, i);
  }
  ಅನ್ಯಥಾ ಯದಿ ((e = l_str2d(s, &n)) != ರಿಕ್ತ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ 0;  /* conversion failed */
  ಪ್ರತ್ಯಾಗಮ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ಪೂರ್ಣಾಙ್ಕ luaO_utf8esc (ವರ್ಣ *buff, l_uint32 x) {
  ಪೂರ್ಣಾಙ್ಕ n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ಯದಿ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  ಅನ್ಯಥಾ {  /* need continuation bytes */
    unsigned ಪೂರ್ಣಾಙ್ಕ mfb = 0x3f;  /* maximum that fits in first byte */
    ಕುರು {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ಯಾವತ್ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ಪ್ರತ್ಯಾಗಮ n;
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
#ಯದಿ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#error "invalid value for LUA_N2SBUFFSZ"
#endif


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tostringbuffFloat (lua_ಸಂಖ್ಯೆ n, ವರ್ಣ *buff) {
  /* first conversion */
  ಪೂರ್ಣಾಙ್ಕ len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ಸಂಖ್ಯೆ_FMT,
                            (LUAI_UACNUMBER)n);
  lua_ಸಂಖ್ಯೆ check = lua_str2number(buff, ರಿಕ್ತ);  /* read it back */
  ಯದಿ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ಸಂಖ್ಯೆ_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ಯದಿ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  ಪ್ರತ್ಯಾಗಮ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ಧ್ರುವ TValue *obj, ವರ್ಣ *buff) {
  ಪೂರ್ಣಾಙ್ಕ len;
  lua_assert(ttisnumber(obj));
  ಯದಿ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  ಅನ್ಯಥಾ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ಪ್ರತ್ಯಾಗಮ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
ಶೂನ್ಯ luaO_tostring (lua_State *L, TValue *obj) {
  ವರ್ಣ buff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(obj, buff);
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
#define BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

/*
** Buffer used by 'luaO_pushvfstring'. 'err' signals an error while
** building result (memory error [1] or buffer overflow [2]).
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ BuffFS {
  lua_State *L;
  ವರ್ಣ *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ಪೂರ್ಣಾಙ್ಕ err;
  ವರ್ಣ space[BUFVFS];  /* initial buffer */
} BuffFS;


ಸ್ಥಿರ ಶೂನ್ಯ initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = ಪರಿಮಾಣ(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pushbuff (lua_State *L, ಶೂನ್ಯ *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  ವಿಕಲ್ಪ (buff->err) {
    ಪಕ್ಷ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ಭಙ್ಗ;
    ಪಕ್ಷ 2:  /* length overflow: Add "..." at the end of result */
      ಯದಿ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      ಅನ್ಯಥಾ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    ಸ್ವಭಾವ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->ತುದಿ.p, ts);
      L->ತುದಿ.p++;
    }
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ಧ್ರುವ ವರ್ಣ *res;
  ಯದಿ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ರಿಕ್ತ;  /* error message is on the top of the stack */
  ಅನ್ಯಥಾ
    res = getstr(tsvalue(s2v(L->ತುದಿ.p - 1)));
  ಯದಿ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಶೂನ್ಯ addstr2buff (BuffFS *buff, ಧ್ರುವ ವರ್ಣ *str, size_t slen) {
  size_t ಎಡ = buff->buffsize - buff->blen;  /* space left in the buffer */
  ಯದಿ (buff->err)  /* do nothing else after an error */
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (slen > ಎಡ) {  /* new string doesn't fit into current buffer? */
    ಯದಿ (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, ಎಡ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಅನ್ಯಥಾ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      ವರ್ಣ *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ರಿಕ್ತ, 0, newsize, ವರ್ಣ)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               ವರ್ಣ);
      ಯದಿ (newb == ರಿಕ್ತ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ಪ್ರತ್ಯಾಗಮ;
      }
      ಯದಿ (buff->b == buff->space)  /* new buffer (not reallocated)? */
        memcpy(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  memcpy(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
ಸ್ಥಿರ ಶೂನ್ಯ addnum2buff (BuffFS *buff, TValue *ಅಂಕೆ) {
  ವರ್ಣ numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(ಅಂಕೆ, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ಧ್ರುವ ವರ್ಣ *luaO_pushvfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ಧ್ರುವ ವರ್ಣ *e;  /* points to next '%' */
  initbuff(L, &buff);
  ಯಾವತ್ ((e = strchr(fmt, '%')) != ರಿಕ್ತ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    ವಿಕಲ್ಪ (*(e + 1)) {  /* conversion specifier */
      ಪಕ್ಷ 's': {  /* zero-terminated string */
        ಧ್ರುವ ವರ್ಣ *s = va_arg(argp, ವರ್ಣ *);
        ಯದಿ (s == ರಿಕ್ತ) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'c': {  /* an 'int' as a character */
        ವರ್ಣ c = cast_char(va_arg(argp, ಪೂರ್ಣಾಙ್ಕ));
        addstr2buff(&buff, &c, ಪರಿಮಾಣ(ವರ್ಣ));
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'd': {  /* an 'int' */
        TValue ಅಂಕೆ;
        setivalue(&ಅಂಕೆ, va_arg(argp, ಪೂರ್ಣಾಙ್ಕ));
        addnum2buff(&buff, &ಅಂಕೆ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'I': {  /* a 'lua_Integer' */
        TValue ಅಂಕೆ;
        setivalue(&ಅಂಕೆ, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &ಅಂಕೆ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'f': {  /* a 'lua_Number' */
        TValue ಅಂಕೆ;
        setfltvalue(&ಅಂಕೆ, cast_ಅಂಕೆ(va_arg(argp, l_uac_ಸಂಖ್ಯೆ)));
        addnum2buff(&buff, &ಅಂಕೆ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'p': {  /* a pointer */
        ವರ್ಣ bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        ಶೂನ್ಯ *p = va_arg(argp, ಶೂನ್ಯ *);
        ಪೂರ್ಣಾಙ್ಕ len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        ವರ್ಣ bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        ಪೂರ್ಣಾಙ್ಕ len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ '%': {
        addstr2buff(&buff, "%", 1);
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ಭಙ್ಗ;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  ಪ್ರತ್ಯಾಗಮ clearbuff(&buff);  /* empty buffer into a new string */
}


ಧ್ರುವ ವರ್ಣ *luaO_pushfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;
  va_list argp;
  va_start(argp, fmt);
  ಸಂದೇಶ = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ಪ್ರತ್ಯಾಗಮ ಸಂದೇಶ;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * ಪರಿಮಾಣ(ವರ್ಣ)), a += (l) )

ಶೂನ್ಯ luaO_chunkid (ವರ್ಣ *out, ಧ್ರುವ ವರ್ಣ *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ಯದಿ (*source == '=') {  /* 'literal' source */
    ಯದಿ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * ಪರಿಮಾಣ(ವರ್ಣ));
    ಅನ್ಯಥಾ {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  ಅನ್ಯಥಾ ಯದಿ (*source == '@') {  /* file name */
    ಯದಿ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * ಪರಿಮಾಣ(ವರ್ಣ));
    ಅನ್ಯಥಾ {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * ಪರಿಮಾಣ(ವರ್ಣ));
    }
  }
  ಅನ್ಯಥಾ {  /* string; format as [string "source"] */
    ಧ್ರುವ ವರ್ಣ *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ಯದಿ (srclen < bufflen && nl == ರಿಕ್ತ) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    ಅನ್ಯಥಾ {
      ಯದಿ (nl != ರಿಕ್ತ)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      ಯದಿ (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * ಪರಿಮಾಣ(ವರ್ಣ));
  }
}

