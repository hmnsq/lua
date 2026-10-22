/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <float.h>
#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lctype.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned পূর্ণাঙ্ক x) {
  স্থির ধ্রুব lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  পূর্ণাঙ্ক l = 0;
  x--;
  যাবত্ (x >= 256) { l += 8; x >>= 8; }
  প্রত্যাগম cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned পূর্ণাঙ্ক p) {
  যদি (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    প্রত্যাগম 0xFF;  /* return maximum value */
  অন্যথা {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    যদি (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      প্রত্যাগম cast_byte(p);
    }
    অন্যথা {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      প্রত্যাগম cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  পূর্ণাঙ্ক m = p & 0xF;  /* mantissa */
  পূর্ণাঙ্ক e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  যদি (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  যদি (e >= 0) {
    যদি (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      প্রত্যাগম (x * m) << e;  /* order doesn't matter here */
    অন্যথা  /* real overflow */
      প্রত্যাগম MAX_LMEM;
  }
  অন্যথা {  /* negative exponent */
    e = -e;
    যদি (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      প্রত্যাগম (x * m) >> e;  /* multiplying first gives more precision */
    অন্যথা যদি ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      প্রত্যাগম (x >> e) * m;
    অন্যথা  /* real overflow */
      প্রত্যাগম MAX_LMEM;
  }
}


স্থির lua_Integer intarith (lua_State *L, পূর্ণাঙ্ক op, lua_Integer v1,
                                                   lua_Integer v2) {
  বিকল্প (op) {
    পক্ষ LUA_OPADD: প্রত্যাগম intop(+, v1, v2);
    পক্ষ LUA_OPSUB:প্রত্যাগম intop(-, v1, v2);
    পক্ষ LUA_OPMUL:প্রত্যাগম intop(*, v1, v2);
    পক্ষ LUA_OPMOD: প্রত্যাগম luaV_mod(L, v1, v2);
    পক্ষ LUA_OPIDIV: প্রত্যাগম luaV_idiv(L, v1, v2);
    পক্ষ LUA_OPBAND: প্রত্যাগম intop(&, v1, v2);
    পক্ষ LUA_OPBOR: প্রত্যাগম intop(|, v1, v2);
    পক্ষ LUA_OPBXOR: প্রত্যাগম intop(^, v1, v2);
    পক্ষ LUA_OPSHL: প্রত্যাগম luaV_shiftl(v1, v2);
    পক্ষ LUA_OPSHR: প্রত্যাগম luaV_shiftr(v1, v2);
    পক্ষ LUA_OPUNM: প্রত্যাগম intop(-, 0, v1);
    পক্ষ LUA_OPBNOT: প্রত্যাগম intop(^, ~l_castS2U(0), v1);
    স্বভাব: lua_assert(0); প্রত্যাগম 0;
  }
}


স্থির lua_সংখ্যা numarith (lua_State *L, পূর্ণাঙ্ক op, lua_সংখ্যা v1,
                                                  lua_সংখ্যা v2) {
  বিকল্প (op) {
    পক্ষ LUA_OPADD: প্রত্যাগম luai_numadd(L, v1, v2);
    পক্ষ LUA_OPSUB: প্রত্যাগম luai_numsub(L, v1, v2);
    পক্ষ LUA_OPMUL: প্রত্যাগম luai_nummul(L, v1, v2);
    পক্ষ LUA_OPDIV: প্রত্যাগম luai_numdiv(L, v1, v2);
    পক্ষ LUA_OPPOW: প্রত্যাগম luai_numpow(L, v1, v2);
    পক্ষ LUA_OPIDIV: প্রত্যাগম luai_numidiv(L, v1, v2);
    পক্ষ LUA_OPUNM: প্রত্যাগম luai_numunm(L, v1);
    পক্ষ LUA_OPMOD: প্রত্যাগম luaV_modf(L, v1, v2);
    স্বভাব: lua_assert(0); প্রত্যাগম 0;
  }
}


পূর্ণাঙ্ক luaO_rawarith (lua_State *L, পূর্ণাঙ্ক op, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                   TValue *res) {
  বিকল্প (op) {
    পক্ষ LUA_OPBAND: পক্ষ LUA_OPBOR: পক্ষ LUA_OPBXOR:
    পক্ষ LUA_OPSHL: পক্ষ LUA_OPSHR:
    পক্ষ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      যদি (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        প্রত্যাগম 1;
      }
      অন্যথা প্রত্যাগম 0;  /* fail */
    }
    পক্ষ LUA_OPDIV: পক্ষ LUA_OPPOW: {  /* operate only on floats */
      lua_সংখ্যা n1; lua_সংখ্যা n2;
      যদি (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        প্রত্যাগম 1;
      }
      অন্যথা প্রত্যাগম 0;  /* fail */
    }
    স্বভাব: {  /* other operations */
      lua_সংখ্যা n1; lua_সংখ্যা n2;
      যদি (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        প্রত্যাগম 1;
      }
      অন্যথা যদি (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        প্রত্যাগম 1;
      }
      অন্যথা প্রত্যাগম 0;  /* fail */
    }
  }
}


শূন্য luaO_arith (lua_State *L, পূর্ণাঙ্ক op, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                 StkId res) {
  যদি (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (পূর্ণাঙ্ক c) {
  lua_assert(lisxdigit(c));
  যদি (lisdigit(c)) প্রত্যাগম cast_byte(c - '0');
  অন্যথা প্রত্যাগম cast_byte((ltolower(c) - 'a') + 10);
}


স্থির পূর্ণাঙ্ক isneg (ধ্রুব বর্ণ **s) {
  যদি (**s == '-') { (*s)++; প্রত্যাগম 1; }
  অন্যথা যদি (**s == '+') (*s)++;
  প্রত্যাগম 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#যদি !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
স্থির lua_সংখ্যা lua_strx2number (ধ্রুব বর্ণ *s, বর্ণ **endptr) {
  পূর্ণাঙ্ক dot = lua_getlocaledecpoint();
  lua_সংখ্যা r = l_mathop(0.0);  /* result (accumulator) */
  পূর্ণাঙ্ক sigdig = 0;  /* number of significant digits */
  পূর্ণাঙ্ক nosigdig = 0;  /* number of non-significant digits */
  পূর্ণাঙ্ক e = 0;  /* exponent correction */
  পূর্ণাঙ্ক neg;  /* 1 if number is negative */
  পূর্ণাঙ্ক hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  যাবত্ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  যদি (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    প্রত্যাগম l_mathop(0.0);  /* invalid format (no '0x') */
  প্রতি (s += 2; ; s++) {  /* skip '0x' and read numeral */
    যদি (*s == dot) {
      যদি (hasdot) ভঙ্গ;  /* second dot? stop loop */
      অন্যথা hasdot = 1;
    }
    অন্যথা যদি (lisxdigit(cast_uchar(*s))) {
      যদি (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      অন্যথা যদি (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      অন্যথা e++;  /* too many digits; ignore, but still count for exponent */
      যদি (hasdot) e--;  /* decimal digit? correct exponent */
    }
    অন্যথা ভঙ্গ;  /* neither a dot nor a digit */
  }
  যদি (nosigdig + sigdig == 0)  /* no digits? */
    প্রত্যাগম l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  যদি (*s == 'p' || *s == 'P') {  /* exponent part? */
    পূর্ণাঙ্ক exp1 = 0;  /* exponent value */
    পূর্ণাঙ্ক neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    যদি (!lisdigit(cast_uchar(*s)))
      প্রত্যাগম l_mathop(0.0);  /* invalid; must have at least one digit */
    যাবত্ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    যদি (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  যদি (neg) r = -r;
  প্রত্যাগম l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#যদি !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
স্থির ধ্রুব বর্ণ *l_str2dloc (ধ্রুব বর্ণ *s, lua_সংখ্যা *result, পূর্ণাঙ্ক mode) {
  বর্ণ *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  যদি (endptr == s) প্রত্যাগম রিক্ত;  /* nothing recognized? */
  যাবত্ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  প্রত্যাগম (*endptr == '\0') ? endptr : রিক্ত;  /* OK iff no trailing chars */
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
স্থির ধ্রুব বর্ণ *l_str2d (ধ্রুব বর্ণ *s, lua_সংখ্যা *result) {
  ধ্রুব বর্ণ *endptr;
  ধ্রুব বর্ণ *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  পূর্ণাঙ্ক mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  যদি (mode == 'n')  /* reject 'inf' and 'nan' */
    প্রত্যাগম রিক্ত;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  যদি (endptr == রিক্ত) {  /* failed? may be a different locale */
    বর্ণ buff[L_MAXLENNUM + 1];
    ধ্রুব বর্ণ *pdot = strchr(s, '.');
    যদি (pdot == রিক্ত || strlen(s) > L_MAXLENNUM)
      প্রত্যাগম রিক্ত;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    যদি (endptr != রিক্ত)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  প্রত্যাগম endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

স্থির ধ্রুব বর্ণ *l_str2int (ধ্রুব বর্ণ *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  পূর্ণাঙ্ক empty = 1;
  পূর্ণাঙ্ক neg;
  যাবত্ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  যদি (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    প্রতি (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  অন্যথা {  /* decimal */
    প্রতি (; lisdigit(cast_uchar(*s)); s++) {
      পূর্ণাঙ্ক d = *s - '0';
      যদি (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        প্রত্যাগম রিক্ত;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  যাবত্ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  যদি (empty || *s != '\0') প্রত্যাগম রিক্ত;  /* something wrong in the numeral */
  অন্যথা {
    *result = l_castU2S((neg) ? 0u - a : a);
    প্রত্যাগম s;
  }
}


size_t luaO_str2num (ধ্রুব বর্ণ *s, TValue *o) {
  lua_Integer i; lua_সংখ্যা n;
  ধ্রুব বর্ণ *e;
  যদি ((e = l_str2int(s, &i)) != রিক্ত) {  /* try as an integer */
    setivalue(o, i);
  }
  অন্যথা যদি ((e = l_str2d(s, &n)) != রিক্ত) {  /* else try as a float */
    setfltvalue(o, n);
  }
  অন্যথা
    প্রত্যাগম 0;  /* conversion failed */
  প্রত্যাগম ct_diff2sz(e - s) + 1;  /* success; return string size */
}


পূর্ণাঙ্ক luaO_utf8esc (বর্ণ *buff, l_uint32 x) {
  পূর্ণাঙ্ক n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  যদি (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  অন্যথা {  /* need continuation bytes */
    unsigned পূর্ণাঙ্ক mfb = 0x3f;  /* maximum that fits in first byte */
    কুরু {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } যাবত্ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  প্রত্যাগম n;
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
#যদি LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
স্থির পূর্ণাঙ্ক tostringbuffFloat (lua_সংখ্যা n, বর্ণ *buff) {
  /* first conversion */
  পূর্ণাঙ্ক len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_সংখ্যা_FMT,
                            (LUAI_UACNUMBER)n);
  lua_সংখ্যা check = lua_str2number(buff, রিক্ত);  /* read it back */
  যদি (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_সংখ্যা_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  যদি (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  প্রত্যাগম len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ধ্রুব TValue *obj, বর্ণ *buff) {
  পূর্ণাঙ্ক len;
  lua_assert(ttisnumber(obj));
  যদি (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  অন্যথা
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  প্রত্যাগম cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
শূন্য luaO_tostring (lua_State *L, TValue *obj) {
  বর্ণ buff[LUA_N2SBUFFSZ];
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
প্রকারনির্দেশ সংরচনা BuffFS {
  lua_State *L;
  বর্ণ *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  পূর্ণাঙ্ক err;
  বর্ণ space[BUFVFS];  /* initial buffer */
} BuffFS;


স্থির শূন্য initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = পরিমাণ(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
স্থির শূন্য pushbuff (lua_State *L, শূন্য *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  বিকল্প (buff->err) {
    পক্ষ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ভঙ্গ;
    পক্ষ 2:  /* length overflow: Add "..." at the end of result */
      যদি (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      অন্যথা {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    স্বভাব: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->শীর্ষ.p, ts);
      L->শীর্ষ.p++;
    }
  }
}


স্থির ধ্রুব বর্ণ *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ধ্রুব বর্ণ *res;
  যদি (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = রিক্ত;  /* error message is on the top of the stack */
  অন্যথা
    res = getstr(tsvalue(s2v(L->শীর্ষ.p - 1)));
  যদি (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  প্রত্যাগম res;
}


স্থির শূন্য addstr2buff (BuffFS *buff, ধ্রুব বর্ণ *str, size_t slen) {
  size_t বাম = buff->buffsize - buff->blen;  /* space left in the buffer */
  যদি (buff->err)  /* do nothing else after an error */
    প্রত্যাগম;
  যদি (slen > বাম) {  /* new string doesn't fit into current buffer? */
    যদি (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, বাম);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      প্রত্যাগম;
    }
    অন্যথা {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      বর্ণ *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, রিক্ত, 0, newsize, বর্ণ)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               বর্ণ);
      যদি (newb == রিক্ত) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        প্রত্যাগম;
      }
      যদি (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
স্থির শূন্য addnum2buff (BuffFS *buff, TValue *অঙ্ক) {
  বর্ণ numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(অঙ্ক, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ধ্রুব বর্ণ *luaO_pushvfstring (lua_State *L, ধ্রুব বর্ণ *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ধ্রুব বর্ণ *e;  /* points to next '%' */
  initbuff(L, &buff);
  যাবত্ ((e = strchr(fmt, '%')) != রিক্ত) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    বিকল্প (*(e + 1)) {  /* conversion specifier */
      পক্ষ 's': {  /* zero-terminated string */
        ধ্রুব বর্ণ *s = va_arg(argp, বর্ণ *);
        যদি (s == রিক্ত) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        ভঙ্গ;
      }
      পক্ষ 'c': {  /* an 'int' as a character */
        বর্ণ c = cast_char(va_arg(argp, পূর্ণাঙ্ক));
        addstr2buff(&buff, &c, পরিমাণ(বর্ণ));
        ভঙ্গ;
      }
      পক্ষ 'd': {  /* an 'int' */
        TValue অঙ্ক;
        setivalue(&অঙ্ক, va_arg(argp, পূর্ণাঙ্ক));
        addnum2buff(&buff, &অঙ্ক);
        ভঙ্গ;
      }
      পক্ষ 'I': {  /* a 'lua_Integer' */
        TValue অঙ্ক;
        setivalue(&অঙ্ক, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &অঙ্ক);
        ভঙ্গ;
      }
      পক্ষ 'f': {  /* a 'lua_Number' */
        TValue অঙ্ক;
        setfltvalue(&অঙ্ক, cast_অঙ্ক(va_arg(argp, l_uac_সংখ্যা)));
        addnum2buff(&buff, &অঙ্ক);
        ভঙ্গ;
      }
      পক্ষ 'p': {  /* a pointer */
        বর্ণ bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        শূন্য *p = va_arg(argp, শূন্য *);
        পূর্ণাঙ্ক len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ভঙ্গ;
      }
      পক্ষ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        বর্ণ bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        পূর্ণাঙ্ক len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ভঙ্গ;
      }
      পক্ষ '%': {
        addstr2buff(&buff, "%", 1);
        ভঙ্গ;
      }
      স্বভাব: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ভঙ্গ;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  প্রত্যাগম clearbuff(&buff);  /* empty buffer into a new string */
}


ধ্রুব বর্ণ *luaO_pushfstring (lua_State *L, ধ্রুব বর্ণ *fmt, ...) {
  ধ্রুব বর্ণ *বার্তা;
  va_list argp;
  va_start(argp, fmt);
  বার্তা = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  যদি (বার্তা == রিক্ত)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  প্রত্যাগম বার্তা;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * পরিমাণ(বর্ণ)), a += (l) )

শূন্য luaO_chunkid (বর্ণ *out, ধ্রুব বর্ণ *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  যদি (*source == '=') {  /* 'literal' source */
    যদি (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * পরিমাণ(বর্ণ));
    অন্যথা {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  অন্যথা যদি (*source == '@') {  /* file name */
    যদি (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * পরিমাণ(বর্ণ));
    অন্যথা {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * পরিমাণ(বর্ণ));
    }
  }
  অন্যথা {  /* string; format as [string "source"] */
    ধ্রুব বর্ণ *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    যদি (srclen < bufflen && nl == রিক্ত) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    অন্যথা {
      যদি (nl != রিক্ত)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      যদি (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * পরিমাণ(বর্ণ));
  }
}

