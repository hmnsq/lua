/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lobject_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x) {
  ਸਥਿਰ ਅਚਲ lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  ਪੂਰਨ l = 0;
  x--;
  ਜਦੋਂ (x >= 256) { l += 8; x >>= 8; }
  ਵਾਪਸ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ p) {
  ਜੇ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ਵਾਪਸ 0xFF;  /* return maximum value */
  ਨਹੀਂਤਾਂ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ਜੇ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ਵਾਪਸ cast_byte(p);
    }
    ਨਹੀਂਤਾਂ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲਘੂਗਣਕ = luaO_ceillog2(p + 1) - 5u;
      ਵਾਪਸ cast_byte(((p >> ਲਘੂਗਣਕ) - 0x10) | ((ਲਘੂਗਣਕ + 1) << 4));
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
  ਪੂਰਨ m = p & 0xF;  /* mantissa */
  ਪੂਰਨ e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ਜੇ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ਜੇ (e >= 0) {
    ਜੇ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ਵਾਪਸ (x * m) << e;  /* order doesn't matter here */
    ਨਹੀਂਤਾਂ  /* real overflow */
      ਵਾਪਸ MAX_LMEM;
  }
  ਨਹੀਂਤਾਂ {  /* negative exponent */
    e = -e;
    ਜੇ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ਵਾਪਸ (x * m) >> e;  /* multiplying first gives more precision */
    ਨਹੀਂਤਾਂ ਜੇ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ਵਾਪਸ (x >> e) * m;
    ਨਹੀਂਤਾਂ  /* real overflow */
      ਵਾਪਸ MAX_LMEM;
  }
}


ਸਥਿਰ lua_Integer intarith (lua_State *L, ਪੂਰਨ op, lua_Integer v1,
                                                   lua_Integer v2) {
  ਚੋਣ (op) {
    ਹਾਲਤ LUA_OPADD: ਵਾਪਸ intop(+, v1, v2);
    ਹਾਲਤ LUA_OPSUB:ਵਾਪਸ intop(-, v1, v2);
    ਹਾਲਤ LUA_OPMUL:ਵਾਪਸ intop(*, v1, v2);
    ਹਾਲਤ LUA_OPMOD: ਵਾਪਸ luaV_mod(L, v1, v2);
    ਹਾਲਤ LUA_OPIDIV: ਵਾਪਸ luaV_idiv(L, v1, v2);
    ਹਾਲਤ LUA_OPBAND: ਵਾਪਸ intop(&, v1, v2);
    ਹਾਲਤ LUA_OPBOR: ਵਾਪਸ intop(|, v1, v2);
    ਹਾਲਤ LUA_OPBXOR: ਵਾਪਸ intop(^, v1, v2);
    ਹਾਲਤ LUA_OPSHL: ਵਾਪਸ luaV_shiftl(v1, v2);
    ਹਾਲਤ LUA_OPSHR: ਵਾਪਸ luaV_shiftr(v1, v2);
    ਹਾਲਤ LUA_OPUNM: ਵਾਪਸ intop(-, 0, v1);
    ਹਾਲਤ LUA_OPBNOT: ਵਾਪਸ intop(^, ~l_castS2U(0), v1);
    ਮੂਲ: lua_assert(0); ਵਾਪਸ 0;
  }
}


ਸਥਿਰ lua_ਸੰਖਿਆ numarith (lua_State *L, ਪੂਰਨ op, lua_ਸੰਖਿਆ v1,
                                                  lua_ਸੰਖਿਆ v2) {
  ਚੋਣ (op) {
    ਹਾਲਤ LUA_OPADD: ਵਾਪਸ luai_numadd(L, v1, v2);
    ਹਾਲਤ LUA_OPSUB: ਵਾਪਸ luai_numsub(L, v1, v2);
    ਹਾਲਤ LUA_OPMUL: ਵਾਪਸ luai_nummul(L, v1, v2);
    ਹਾਲਤ LUA_OPDIV: ਵਾਪਸ luai_numdiv(L, v1, v2);
    ਹਾਲਤ LUA_OPPOW: ਵਾਪਸ luai_numpow(L, v1, v2);
    ਹਾਲਤ LUA_OPIDIV: ਵਾਪਸ luai_numidiv(L, v1, v2);
    ਹਾਲਤ LUA_OPUNM: ਵਾਪਸ luai_numunm(L, v1);
    ਹਾਲਤ LUA_OPMOD: ਵਾਪਸ luaV_modf(L, v1, v2);
    ਮੂਲ: lua_assert(0); ਵਾਪਸ 0;
  }
}


ਪੂਰਨ luaO_rawarith (lua_State *L, ਪੂਰਨ op, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                   TValue *res) {
  ਚੋਣ (op) {
    ਹਾਲਤ LUA_OPBAND: ਹਾਲਤ LUA_OPBOR: ਹਾਲਤ LUA_OPBXOR:
    ਹਾਲਤ LUA_OPSHL: ਹਾਲਤ LUA_OPSHR:
    ਹਾਲਤ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ਜੇ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ ਵਾਪਸ 0;  /* fail */
    }
    ਹਾਲਤ LUA_OPDIV: ਹਾਲਤ LUA_OPPOW: {  /* operate only on floats */
      lua_ਸੰਖਿਆ n1; lua_ਸੰਖਿਆ n2;
      ਜੇ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ ਵਾਪਸ 0;  /* fail */
    }
    ਮੂਲ: {  /* other operations */
      lua_ਸੰਖਿਆ n1; lua_ਸੰਖਿਆ n2;
      ਜੇ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ ਜੇ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ ਵਾਪਸ 0;  /* fail */
    }
  }
}


ਖਾਲੀ luaO_arith (lua_State *L, ਪੂਰਨ op, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                 StkId res) {
  ਜੇ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ਜੋੜੋ));
  }
}


lu_byte luaO_hexavalue (ਪੂਰਨ c) {
  lua_assert(lisxdigit(c));
  ਜੇ (lisdigit(c)) ਵਾਪਸ cast_byte(c - '0');
  ਨਹੀਂਤਾਂ ਵਾਪਸ cast_byte((ltolower(c) - 'a') + 10);
}


ਸਥਿਰ ਪੂਰਨ isneg (ਅਚਲ ਅੱਖਰ **s) {
  ਜੇ (**s == '-') { (*s)++; ਵਾਪਸ 1; }
  ਨਹੀਂਤਾਂ ਜੇ (**s == '+') (*s)++;
  ਵਾਪਸ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ਜੇ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ਸਥਿਰ lua_ਸੰਖਿਆ lua_strx2number (ਅਚਲ ਅੱਖਰ *s, ਅੱਖਰ **endptr) {
  ਪੂਰਨ dot = lua_getlocaledecpoint();
  lua_ਸੰਖਿਆ r = l_mathop(0.0);  /* result (accumulator) */
  ਪੂਰਨ sigdig = 0;  /* number of significant digits */
  ਪੂਰਨ nosigdig = 0;  /* number of non-significant digits */
  ਪੂਰਨ e = 0;  /* exponent correction */
  ਪੂਰਨ neg;  /* 1 if number is negative */
  ਪੂਰਨ hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ਜਦੋਂ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ਜੇ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ਵਾਪਸ l_mathop(0.0);  /* invalid format (no '0x') */
  ਲਈ (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ਜੇ (*s == dot) {
      ਜੇ (hasdot) ਰੋਕੋ;  /* second dot? stop loop */
      ਨਹੀਂਤਾਂ hasdot = 1;
    }
    ਨਹੀਂਤਾਂ ਜੇ (lisxdigit(cast_uchar(*s))) {
      ਜੇ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      ਨਹੀਂਤਾਂ ਜੇ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      ਨਹੀਂਤਾਂ e++;  /* too many digits; ignore, but still count for exponent */
      ਜੇ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    ਨਹੀਂਤਾਂ ਰੋਕੋ;  /* neither a dot nor a digit */
  }
  ਜੇ (nosigdig + sigdig == 0)  /* no digits? */
    ਵਾਪਸ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ਜੇ (*s == 'p' || *s == 'P') {  /* exponent part? */
    ਪੂਰਨ exp1 = 0;  /* exponent value */
    ਪੂਰਨ neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ਜੇ (!lisdigit(cast_uchar(*s)))
      ਵਾਪਸ l_mathop(0.0);  /* invalid; must have at least one digit */
    ਜਦੋਂ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ਜੇ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ਜੇ (neg) r = -r;
  ਵਾਪਸ l_mathop(ldexp)(r, e);
}

#ਜੇ_ਅੰਤ
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ਜੇ !defined (L_MAXLENNUM)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_MAXLENNUM	200
#ਜੇ_ਅੰਤ

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *l_str2dloc (ਅਚਲ ਅੱਖਰ *s, lua_ਸੰਖਿਆ *ਨਤੀਜਾ, ਪੂਰਨ ਬਹੁਲਕ) {
  ਅੱਖਰ *endptr;
  *ਨਤੀਜਾ = (ਬਹੁਲਕ == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ਜੇ (endptr == s) ਵਾਪਸ ਨਿਰਾ;  /* nothing recognized? */
  ਜਦੋਂ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ਵਾਪਸ (*endptr == '\0') ? endptr : ਨਿਰਾ;  /* OK iff no trailing chars */
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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *l_str2d (ਅਚਲ ਅੱਖਰ *s, lua_ਸੰਖਿਆ *ਨਤੀਜਾ) {
  ਅਚਲ ਅੱਖਰ *endptr;
  ਅਚਲ ਅੱਖਰ *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ਪੂਰਨ ਬਹੁਲਕ = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ਜੇ (ਬਹੁਲਕ == 'n')  /* reject 'inf' and 'nan' */
    ਵਾਪਸ ਨਿਰਾ;
  endptr = l_str2dloc(s, ਨਤੀਜਾ, ਬਹੁਲਕ);  /* try to convert */
  ਜੇ (endptr == ਨਿਰਾ) {  /* failed? may be a different locale */
    ਅੱਖਰ buff[L_MAXLENNUM + 1];
    ਅਚਲ ਅੱਖਰ *pdot = strchr(s, '.');
    ਜੇ (pdot == ਨਿਰਾ || ਲੜੀ_ਲੰਬਾਈ(s) > L_MAXLENNUM)
      ਵਾਪਸ ਨਿਰਾ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, ਨਤੀਜਾ, ਬਹੁਲਕ);  /* try again */
    ਜੇ (endptr != ਨਿਰਾ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ਵਾਪਸ endptr;
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *l_str2int (ਅਚਲ ਅੱਖਰ *s, lua_Integer *ਨਤੀਜਾ) {
  lua_Unsigned a = 0;
  ਪੂਰਨ ਖਾਲੀ_ਥਾਵਾਂ = 1;
  ਪੂਰਨ neg;
  ਜਦੋਂ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ਜੇ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    ਲਈ (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      ਖਾਲੀ_ਥਾਵਾਂ = 0;
    }
  }
  ਨਹੀਂਤਾਂ {  /* decimal */
    ਲਈ (; lisdigit(cast_uchar(*s)); s++) {
      ਪੂਰਨ d = *s - '0';
      ਜੇ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ਵਾਪਸ ਨਿਰਾ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      ਖਾਲੀ_ਥਾਵਾਂ = 0;
    }
  }
  ਜਦੋਂ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ਜੇ (ਖਾਲੀ_ਥਾਵਾਂ || *s != '\0') ਵਾਪਸ ਨਿਰਾ;  /* something wrong in the numeral */
  ਨਹੀਂਤਾਂ {
    *ਨਤੀਜਾ = l_castU2S((neg) ? 0u - a : a);
    ਵਾਪਸ s;
  }
}


size_t luaO_str2num (ਅਚਲ ਅੱਖਰ *s, TValue *o) {
  lua_Integer i; lua_ਸੰਖਿਆ n;
  ਅਚਲ ਅੱਖਰ *e;
  ਜੇ ((e = l_str2int(s, &i)) != ਨਿਰਾ) {  /* try as an integer */
    setivalue(o, i);
  }
  ਨਹੀਂਤਾਂ ਜੇ ((e = l_str2d(s, &n)) != ਨਿਰਾ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;  /* conversion failed */
  ਵਾਪਸ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ਪੂਰਨ luaO_utf8esc (ਅੱਖਰ *buff, l_uint32 x) {
  ਪੂਰਨ n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ਜੇ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  ਨਹੀਂਤਾਂ {  /* need continuation bytes */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ mfb = 0x3f;  /* maximum that fits in first byte */
    ਕਰੋ {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ਜਦੋਂ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ਵਾਪਸ n;
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
#ਜੇ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#ਗਲਤੀ "invalid value for LUA_N2SBUFFSZ"
#ਜੇ_ਅੰਤ


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
ਸਥਿਰ ਪੂਰਨ tostringbuffFloat (lua_ਸੰਖਿਆ n, ਅੱਖਰ *buff) {
  /* first conversion */
  ਪੂਰਨ len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ਸੰਖਿਆ_FMT,
                            (LUAI_UACNUMBER)n);
  lua_ਸੰਖਿਆ check = lua_str2number(buff, ਨਿਰਾ);  /* read it back */
  ਜੇ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ਸੰਖਿਆ_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ਜੇ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  ਵਾਪਸ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
ਬਿਨਾਨਿਸ਼ਾਨੀ luaO_tostringbuff (ਅਚਲ TValue *obj, ਅੱਖਰ *buff) {
  ਪੂਰਨ len;
  lua_assert(ttisnumber(obj));
  ਜੇ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  ਨਹੀਂਤਾਂ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ਵਾਪਸ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
ਖਾਲੀ luaO_tostring (lua_State *L, TValue *obj) {
  ਅੱਖਰ buff[LUA_N2SBUFFSZ];
  ਬਿਨਾਨਿਸ਼ਾਨੀ len = luaO_tostringbuff(obj, buff);
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

/*
** Buffer used by 'luaO_pushvfstring'. 'err' signals an error while
** building result (memory error [1] or buffer overflow [2]).
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BuffFS {
  lua_State *L;
  ਅੱਖਰ *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ਪੂਰਨ err;
  ਅੱਖਰ space[BUFVFS];  /* initial buffer */
} BuffFS;


ਸਥਿਰ ਖਾਲੀ initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = ਆਕਾਰ(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ਸਥਿਰ ਖਾਲੀ pushbuff (lua_State *L, ਖਾਲੀ *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  ਚੋਣ (buff->err) {
    ਹਾਲਤ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ਰੋਕੋ;
    ਹਾਲਤ 2:  /* length overflow: Add "..." at the end of result */
      ਜੇ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      ਨਹੀਂਤਾਂ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    ਮੂਲ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->ਸਿਖਰ.p, ts);
      L->ਸਿਖਰ.p++;
    }
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ਅਚਲ ਅੱਖਰ *res;
  ਜੇ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ਨਿਰਾ;  /* error message is on the top of the stack */
  ਨਹੀਂਤਾਂ
    res = getstr(tsvalue(s2v(L->ਸਿਖਰ.p - 1)));
  ਜੇ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ਵਾਪਸ res;
}


ਸਥਿਰ ਖਾਲੀ addstr2buff (BuffFS *buff, ਅਚਲ ਅੱਖਰ *str, size_t slen) {
  size_t ਖੱਬਾ = buff->buffsize - buff->blen;  /* space left in the buffer */
  ਜੇ (buff->err)  /* do nothing else after an error */
    ਵਾਪਸ;
  ਜੇ (slen > ਖੱਬਾ) {  /* new string doesn't fit into current buffer? */
    ਜੇ (slen > ((MAX_ਮਾਪ/2) - buff->blen)) {  /* overflow? */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(buff->b + buff->blen, str, ਖੱਬਾ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ਵਾਪਸ;
    }
    ਨਹੀਂਤਾਂ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      ਅੱਖਰ *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ਨਿਰਾ, 0, newsize, ਅੱਖਰ)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               ਅੱਖਰ);
      ਜੇ (newb == ਨਿਰਾ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ਵਾਪਸ;
      }
      ਜੇ (buff->b == buff->space)  /* new buffer (not reallocated)? */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
ਸਥਿਰ ਖਾਲੀ addnum2buff (BuffFS *buff, TValue *ਅੰਕ) {
  ਅੱਖਰ numbuff[LUA_N2SBUFFSZ];
  ਬਿਨਾਨਿਸ਼ਾਨੀ len = luaO_tostringbuff(ਅੰਕ, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ਅਚਲ ਅੱਖਰ *luaO_pushvfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ਅਚਲ ਅੱਖਰ *e;  /* points to next '%' */
  initbuff(L, &buff);
  ਜਦੋਂ ((e = strchr(fmt, '%')) != ਨਿਰਾ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    ਚੋਣ (*(e + 1)) {  /* conversion specifier */
      ਹਾਲਤ 's': {  /* zero-terminated string */
        ਅਚਲ ਅੱਖਰ *s = va_arg(argp, ਅੱਖਰ *);
        ਜੇ (s == ਨਿਰਾ) s = "(null)";
        addstr2buff(&buff, s, ਲੜੀ_ਲੰਬਾਈ(s));
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'c': {  /* an 'int' as a character */
        ਅੱਖਰ c = cast_char(va_arg(argp, ਪੂਰਨ));
        addstr2buff(&buff, &c, ਆਕਾਰ(ਅੱਖਰ));
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'd': {  /* an 'int' */
        TValue ਅੰਕ;
        setivalue(&ਅੰਕ, va_arg(argp, ਪੂਰਨ));
        addnum2buff(&buff, &ਅੰਕ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'I': {  /* a 'lua_Integer' */
        TValue ਅੰਕ;
        setivalue(&ਅੰਕ, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &ਅੰਕ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'f': {  /* a 'lua_Number' */
        TValue ਅੰਕ;
        setfltvalue(&ਅੰਕ, cast_ਅੰਕ(va_arg(argp, l_uac_ਸੰਖਿਆ)));
        addnum2buff(&buff, &ਅੰਕ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'p': {  /* a pointer */
        ਅੱਖਰ bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        ਖਾਲੀ *p = va_arg(argp, ਖਾਲੀ *);
        ਪੂਰਨ len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        ਅੱਖਰ bf[UTF8BUFFSZ];
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ arg = va_arg(argp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ);
        ਪੂਰਨ len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ਰੋਕੋ;
      }
      ਹਾਲਤ '%': {
        addstr2buff(&buff, "%", 1);
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ਰੋਕੋ;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, ਲੜੀ_ਲੰਬਾਈ(fmt));  /* rest of 'fmt' */
  ਵਾਪਸ clearbuff(&buff);  /* empty buffer into a new string */
}


ਅਚਲ ਅੱਖਰ *luaO_pushfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
  va_list argp;
  va_start(argp, fmt);
  ਸੁਨੇਹਾ = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ਜੇ (ਸੁਨੇਹਾ == ਨਿਰਾ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ਵਾਪਸ ਸੁਨੇਹਾ;
}

/* }================================================================== */


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RETS	"..."
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRE	"[string \""
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POS	"\"]"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ addstr(a,b,l)	( ਸਮ੍ਰਿਤੀ_ਨਕਲ(a,b,(l) * ਆਕਾਰ(ਅੱਖਰ)), a += (l) )

ਖਾਲੀ luaO_chunkid (ਅੱਖਰ *ਬਾਹਰਲਾ, ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ਜੇ (*ਸਰੋਤ == '=') {  /* 'literal' source */
    ਜੇ (srclen <= bufflen)  /* small enough? */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, ਸਰੋਤ + 1, srclen * ਆਕਾਰ(ਅੱਖਰ));
    ਨਹੀਂਤਾਂ {  /* truncate it */
      addstr(ਬਾਹਰਲਾ, ਸਰੋਤ + 1, bufflen - 1);
      *ਬਾਹਰਲਾ = '\0';
    }
  }
  ਨਹੀਂਤਾਂ ਜੇ (*ਸਰੋਤ == '@') {  /* file name */
    ਜੇ (srclen <= bufflen)  /* small enough? */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, ਸਰੋਤ + 1, srclen * ਆਕਾਰ(ਅੱਖਰ));
    ਨਹੀਂਤਾਂ {  /* add '...' before rest of name */
      addstr(ਬਾਹਰਲਾ, RETS, LL(RETS));
      bufflen -= LL(RETS);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, ਸਰੋਤ + 1 + srclen - bufflen, bufflen * ਆਕਾਰ(ਅੱਖਰ));
    }
  }
  ਨਹੀਂਤਾਂ {  /* string; format as [string "source"] */
    ਅਚਲ ਅੱਖਰ *nl = strchr(ਸਰੋਤ, '\n');  /* find first new line (if any) */
    addstr(ਬਾਹਰਲਾ, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ਜੇ (srclen < bufflen && nl == ਨਿਰਾ) {  /* small one-line source? */
      addstr(ਬਾਹਰਲਾ, ਸਰੋਤ, srclen);  /* keep it */
    }
    ਨਹੀਂਤਾਂ {
      ਜੇ (nl != ਨਿਰਾ)
        srclen = ct_diff2sz(nl - ਸਰੋਤ);  /* stop at first newline */
      ਜੇ (srclen > bufflen) srclen = bufflen;
      addstr(ਬਾਹਰਲਾ, ਸਰੋਤ, srclen);
      addstr(ਬਾਹਰਲਾ, RETS, LL(RETS));
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ, POS, (LL(POS) + 1) * ਆਕਾਰ(ਅੱਖਰ));
  }
}

