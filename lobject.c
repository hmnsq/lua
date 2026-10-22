/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <float.h>
#अन्तर्भाव <locale.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lctype.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned पूर्णाङ्क x) {
  स्थिर ध्रुव lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  पूर्णाङ्क l = 0;
  x--;
  याव (x >= 256) { l += 8; x >>= 8; }
  पटिनिवत्त cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned पूर्णाङ्क p) {
  सचे (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    पटिनिवत्त 0xFF;  /* return maximum value */
  अञ्ञथा {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    सचे (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      पटिनिवत्त cast_byte(p);
    }
    अञ्ञथा {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      पटिनिवत्त cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  पूर्णाङ्क m = p & 0xF;  /* mantissa */
  पूर्णाङ्क e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  सचे (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  सचे (e >= 0) {
    सचे (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      पटिनिवत्त (x * m) << e;  /* order doesn't matter here */
    अञ्ञथा  /* real overflow */
      पटिनिवत्त MAX_LMEM;
  }
  अञ्ञथा {  /* negative exponent */
    e = -e;
    सचे (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      पटिनिवत्त (x * m) >> e;  /* multiplying first gives more precision */
    अञ्ञथा सचे ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      पटिनिवत्त (x >> e) * m;
    अञ्ञथा  /* real overflow */
      पटिनिवत्त MAX_LMEM;
  }
}


स्थिर lua_Integer intarith (lua_State *L, पूर्णाङ्क op, lua_Integer v1,
                                                   lua_Integer v2) {
  विकल्प (op) {
    पक्ष LUA_OPADD: पटिनिवत्त intop(+, v1, v2);
    पक्ष LUA_OPSUB:पटिनिवत्त intop(-, v1, v2);
    पक्ष LUA_OPMUL:पटिनिवत्त intop(*, v1, v2);
    पक्ष LUA_OPMOD: पटिनिवत्त luaV_mod(L, v1, v2);
    पक्ष LUA_OPIDIV: पटिनिवत्त luaV_idiv(L, v1, v2);
    पक्ष LUA_OPBAND: पटिनिवत्त intop(&, v1, v2);
    पक्ष LUA_OPBOR: पटिनिवत्त intop(|, v1, v2);
    पक्ष LUA_OPBXOR: पटिनिवत्त intop(^, v1, v2);
    पक्ष LUA_OPSHL: पटिनिवत्त luaV_shiftl(v1, v2);
    पक्ष LUA_OPSHR: पटिनिवत्त luaV_shiftr(v1, v2);
    पक्ष LUA_OPUNM: पटिनिवत्त intop(-, 0, v1);
    पक्ष LUA_OPBNOT: पटिनिवत्त intop(^, ~l_castS2U(0), v1);
    स्वभाव: lua_assert(0); पटिनिवत्त 0;
  }
}


स्थिर lua_सङ्ख्या numarith (lua_State *L, पूर्णाङ्क op, lua_सङ्ख्या v1,
                                                  lua_सङ्ख्या v2) {
  विकल्प (op) {
    पक्ष LUA_OPADD: पटिनिवत्त luai_numadd(L, v1, v2);
    पक्ष LUA_OPSUB: पटिनिवत्त luai_numsub(L, v1, v2);
    पक्ष LUA_OPMUL: पटिनिवत्त luai_nummul(L, v1, v2);
    पक्ष LUA_OPDIV: पटिनिवत्त luai_numdiv(L, v1, v2);
    पक्ष LUA_OPPOW: पटिनिवत्त luai_numpow(L, v1, v2);
    पक्ष LUA_OPIDIV: पटिनिवत्त luai_numidiv(L, v1, v2);
    पक्ष LUA_OPUNM: पटिनिवत्त luai_numunm(L, v1);
    पक्ष LUA_OPMOD: पटिनिवत्त luaV_modf(L, v1, v2);
    स्वभाव: lua_assert(0); पटिनिवत्त 0;
  }
}


पूर्णाङ्क luaO_rawarith (lua_State *L, पूर्णाङ्क op, ध्रुव TValue *p1, ध्रुव TValue *p2,
                   TValue *res) {
  विकल्प (op) {
    पक्ष LUA_OPBAND: पक्ष LUA_OPBOR: पक्ष LUA_OPBXOR:
    पक्ष LUA_OPSHL: पक्ष LUA_OPSHR:
    पक्ष LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      सचे (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        पटिनिवत्त 1;
      }
      अञ्ञथा पटिनिवत्त 0;  /* fail */
    }
    पक्ष LUA_OPDIV: पक्ष LUA_OPPOW: {  /* operate only on floats */
      lua_सङ्ख्या n1; lua_सङ्ख्या n2;
      सचे (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        पटिनिवत्त 1;
      }
      अञ्ञथा पटिनिवत्त 0;  /* fail */
    }
    स्वभाव: {  /* other operations */
      lua_सङ्ख्या n1; lua_सङ्ख्या n2;
      सचे (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        पटिनिवत्त 1;
      }
      अञ्ञथा सचे (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        पटिनिवत्त 1;
      }
      अञ्ञथा पटिनिवत्त 0;  /* fail */
    }
  }
}


शून्य luaO_arith (lua_State *L, पूर्णाङ्क op, ध्रुव TValue *p1, ध्रुव TValue *p2,
                 StkId res) {
  सचे (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (पूर्णाङ्क c) {
  lua_assert(lisxdigit(c));
  सचे (lisdigit(c)) पटिनिवत्त cast_byte(c - '0');
  अञ्ञथा पटिनिवत्त cast_byte((ltolower(c) - 'a') + 10);
}


स्थिर पूर्णाङ्क isneg (ध्रुव वर्ण **s) {
  सचे (**s == '-') { (*s)++; पटिनिवत्त 1; }
  अञ्ञथा सचे (**s == '+') (*s)++;
  पटिनिवत्त 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#सचे !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
स्थिर lua_सङ्ख्या lua_strx2number (ध्रुव वर्ण *s, वर्ण **endptr) {
  पूर्णाङ्क dot = lua_getlocaledecpoint();
  lua_सङ्ख्या r = l_mathop(0.0);  /* result (accumulator) */
  पूर्णाङ्क sigdig = 0;  /* number of significant digits */
  पूर्णाङ्क nosigdig = 0;  /* number of non-significant digits */
  पूर्णाङ्क e = 0;  /* exponent correction */
  पूर्णाङ्क neg;  /* 1 if number is negative */
  पूर्णाङ्क hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  याव (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  सचे (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    पटिनिवत्त l_mathop(0.0);  /* invalid format (no '0x') */
  अत्थाय (s += 2; ; s++) {  /* skip '0x' and read numeral */
    सचे (*s == dot) {
      सचे (hasdot) भङ्ग;  /* second dot? stop loop */
      अञ्ञथा hasdot = 1;
    }
    अञ्ञथा सचे (lisxdigit(cast_uchar(*s))) {
      सचे (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      अञ्ञथा सचे (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      अञ्ञथा e++;  /* too many digits; ignore, but still count for exponent */
      सचे (hasdot) e--;  /* decimal digit? correct exponent */
    }
    अञ्ञथा भङ्ग;  /* neither a dot nor a digit */
  }
  सचे (nosigdig + sigdig == 0)  /* no digits? */
    पटिनिवत्त l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  सचे (*s == 'p' || *s == 'P') {  /* exponent part? */
    पूर्णाङ्क exp1 = 0;  /* exponent value */
    पूर्णाङ्क neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    सचे (!lisdigit(cast_uchar(*s)))
      पटिनिवत्त l_mathop(0.0);  /* invalid; must have at least one digit */
    याव (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    सचे (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  सचे (neg) r = -r;
  पटिनिवत्त l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#सचे !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
स्थिर ध्रुव वर्ण *l_str2dloc (ध्रुव वर्ण *s, lua_सङ्ख्या *result, पूर्णाङ्क mode) {
  वर्ण *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  सचे (endptr == s) पटिनिवत्त रिक्त;  /* nothing recognized? */
  याव (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  पटिनिवत्त (*endptr == '\0') ? endptr : रिक्त;  /* OK iff no trailing chars */
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
स्थिर ध्रुव वर्ण *l_str2d (ध्रुव वर्ण *s, lua_सङ्ख्या *result) {
  ध्रुव वर्ण *endptr;
  ध्रुव वर्ण *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  पूर्णाङ्क mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  सचे (mode == 'n')  /* reject 'inf' and 'nan' */
    पटिनिवत्त रिक्त;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  सचे (endptr == रिक्त) {  /* failed? may be a different locale */
    वर्ण buff[L_MAXLENNUM + 1];
    ध्रुव वर्ण *pdot = strchr(s, '.');
    सचे (pdot == रिक्त || strlen(s) > L_MAXLENNUM)
      पटिनिवत्त रिक्त;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    सचे (endptr != रिक्त)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  पटिनिवत्त endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

स्थिर ध्रुव वर्ण *l_str2int (ध्रुव वर्ण *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  पूर्णाङ्क empty = 1;
  पूर्णाङ्क neg;
  याव (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  सचे (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    अत्थाय (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  अञ्ञथा {  /* decimal */
    अत्थाय (; lisdigit(cast_uchar(*s)); s++) {
      पूर्णाङ्क d = *s - '0';
      सचे (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        पटिनिवत्त रिक्त;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  याव (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  सचे (empty || *s != '\0') पटिनिवत्त रिक्त;  /* something wrong in the numeral */
  अञ्ञथा {
    *result = l_castU2S((neg) ? 0u - a : a);
    पटिनिवत्त s;
  }
}


size_t luaO_str2num (ध्रुव वर्ण *s, TValue *o) {
  lua_Integer i; lua_सङ्ख्या n;
  ध्रुव वर्ण *e;
  सचे ((e = l_str2int(s, &i)) != रिक्त) {  /* try as an integer */
    setivalue(o, i);
  }
  अञ्ञथा सचे ((e = l_str2d(s, &n)) != रिक्त) {  /* else try as a float */
    setfltvalue(o, n);
  }
  अञ्ञथा
    पटिनिवत्त 0;  /* conversion failed */
  पटिनिवत्त ct_diff2sz(e - s) + 1;  /* success; return string size */
}


पूर्णाङ्क luaO_utf8esc (वर्ण *buff, l_uint32 x) {
  पूर्णाङ्क n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  सचे (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  अञ्ञथा {  /* need continuation bytes */
    unsigned पूर्णाङ्क mfb = 0x3f;  /* maximum that fits in first byte */
    करोहि {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } याव (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  पटिनिवत्त n;
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
#सचे LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
स्थिर पूर्णाङ्क tostringbuffFloat (lua_सङ्ख्या n, वर्ण *buff) {
  /* first conversion */
  पूर्णाङ्क len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_सङ्ख्या_FMT,
                            (LUAI_UACNUMBER)n);
  lua_सङ्ख्या check = lua_str2number(buff, रिक्त);  /* read it back */
  सचे (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_सङ्ख्या_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  सचे (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  पटिनिवत्त len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ध्रुव TValue *obj, वर्ण *buff) {
  पूर्णाङ्क len;
  lua_assert(ttisnumber(obj));
  सचे (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  अञ्ञथा
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  पटिनिवत्त cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
शून्य luaO_tostring (lua_State *L, TValue *obj) {
  वर्ण buff[LUA_N2SBUFFSZ];
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
प्रकारनिर्देश संरचना BuffFS {
  lua_State *L;
  वर्ण *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  पूर्णाङ्क err;
  वर्ण space[BUFVFS];  /* initial buffer */
} BuffFS;


स्थिर शून्य initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = परिमाण(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
स्थिर शून्य pushbuff (lua_State *L, शून्य *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  विकल्प (buff->err) {
    पक्ष 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      भङ्ग;
    पक्ष 2:  /* length overflow: Add "..." at the end of result */
      सचे (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      अञ्ञथा {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    स्वभाव: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->शिखरम्.p, ts);
      L->शिखरम्.p++;
    }
  }
}


स्थिर ध्रुव वर्ण *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ध्रुव वर्ण *res;
  सचे (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = रिक्त;  /* error message is on the top of the stack */
  अञ्ञथा
    res = getstr(tsvalue(s2v(L->शिखरम्.p - 1)));
  सचे (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  पटिनिवत्त res;
}


स्थिर शून्य addstr2buff (BuffFS *buff, ध्रुव वर्ण *str, size_t slen) {
  size_t वामः = buff->buffsize - buff->blen;  /* space left in the buffer */
  सचे (buff->err)  /* do nothing else after an error */
    पटिनिवत्त;
  सचे (slen > वामः) {  /* new string doesn't fit into current buffer? */
    सचे (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, वामः);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      पटिनिवत्त;
    }
    अञ्ञथा {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      वर्ण *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, रिक्त, 0, newsize, वर्ण)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               वर्ण);
      सचे (newb == रिक्त) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        पटिनिवत्त;
      }
      सचे (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
स्थिर शून्य addnum2buff (BuffFS *buff, TValue *अङ्कः) {
  वर्ण numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(अङ्कः, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ध्रुव वर्ण *luaO_pushvfstring (lua_State *L, ध्रुव वर्ण *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ध्रुव वर्ण *e;  /* points to next '%' */
  initbuff(L, &buff);
  याव ((e = strchr(fmt, '%')) != रिक्त) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    विकल्प (*(e + 1)) {  /* conversion specifier */
      पक्ष 's': {  /* zero-terminated string */
        ध्रुव वर्ण *s = va_arg(argp, वर्ण *);
        सचे (s == रिक्त) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        भङ्ग;
      }
      पक्ष 'c': {  /* an 'int' as a character */
        वर्ण c = cast_char(va_arg(argp, पूर्णाङ्क));
        addstr2buff(&buff, &c, परिमाण(वर्ण));
        भङ्ग;
      }
      पक्ष 'd': {  /* an 'int' */
        TValue अङ्कः;
        setivalue(&अङ्कः, va_arg(argp, पूर्णाङ्क));
        addnum2buff(&buff, &अङ्कः);
        भङ्ग;
      }
      पक्ष 'I': {  /* a 'lua_Integer' */
        TValue अङ्कः;
        setivalue(&अङ्कः, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &अङ्कः);
        भङ्ग;
      }
      पक्ष 'f': {  /* a 'lua_Number' */
        TValue अङ्कः;
        setfltvalue(&अङ्कः, cast_अङ्कः(va_arg(argp, l_uac_सङ्ख्या)));
        addnum2buff(&buff, &अङ्कः);
        भङ्ग;
      }
      पक्ष 'p': {  /* a pointer */
        वर्ण bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        शून्य *p = va_arg(argp, शून्य *);
        पूर्णाङ्क len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        भङ्ग;
      }
      पक्ष 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        वर्ण bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        पूर्णाङ्क len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        भङ्ग;
      }
      पक्ष '%': {
        addstr2buff(&buff, "%", 1);
        भङ्ग;
      }
      स्वभाव: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        भङ्ग;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  पटिनिवत्त clearbuff(&buff);  /* empty buffer into a new string */
}


ध्रुव वर्ण *luaO_pushfstring (lua_State *L, ध्रुव वर्ण *fmt, ...) {
  ध्रुव वर्ण *सन्देशः;
  va_list argp;
  va_start(argp, fmt);
  सन्देशः = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  सचे (सन्देशः == रिक्त)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  पटिनिवत्त सन्देशः;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * परिमाण(वर्ण)), a += (l) )

शून्य luaO_chunkid (वर्ण *out, ध्रुव वर्ण *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  सचे (*source == '=') {  /* 'literal' source */
    सचे (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * परिमाण(वर्ण));
    अञ्ञथा {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  अञ्ञथा सचे (*source == '@') {  /* file name */
    सचे (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * परिमाण(वर्ण));
    अञ्ञथा {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * परिमाण(वर्ण));
    }
  }
  अञ्ञथा {  /* string; format as [string "source"] */
    ध्रुव वर्ण *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    सचे (srclen < bufflen && nl == रिक्त) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    अञ्ञथा {
      सचे (nl != रिक्त)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      सचे (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * परिमाण(वर्ण));
  }
}

