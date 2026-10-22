/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#నిర్వచించు lobject_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <float.h>
#అన్తర్భావ <locale.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lctype.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (గుర్తులేని పూర్ణాంకం x) {
  స్థిర స్థిరరాశి lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  పూర్ణాంకం l = 0;
  x--;
  అయితే (x >= 256) { l += 8; x >>= 8; }
  తిరిగి cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (గుర్తులేని పూర్ణాంకం p) {
  ఒకవేళ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    తిరిగి 0xFF;  /* return maximum value */
  లేకపోతే {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ఒకవేళ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      తిరిగి cast_byte(p);
    }
    లేకపోతే {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      గుర్తులేని సంవర్గమానం = luaO_ceillog2(p + 1) - 5u;
      తిరిగి cast_byte(((p >> సంవర్గమానం) - 0x10) | ((సంవర్గమానం + 1) << 4));
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
  పూర్ణాంకం m = p & 0xF;  /* mantissa */
  పూర్ణాంకం e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ఒకవేళ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ఒకవేళ (e >= 0) {
    ఒకవేళ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      తిరిగి (x * m) << e;  /* order doesn't matter here */
    లేకపోతే  /* real overflow */
      తిరిగి MAX_LMEM;
  }
  లేకపోతే {  /* negative exponent */
    e = -e;
    ఒకవేళ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      తిరిగి (x * m) >> e;  /* multiplying first gives more precision */
    లేకపోతే ఒకవేళ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      తిరిగి (x >> e) * m;
    లేకపోతే  /* real overflow */
      తిరిగి MAX_LMEM;
  }
}


స్థిర lua_Integer intarith (lua_State *L, పూర్ణాంకం op, lua_Integer v1,
                                                   lua_Integer v2) {
  ఎంపిక (op) {
    స్థితి LUA_OPADD: తిరిగి intop(+, v1, v2);
    స్థితి LUA_OPSUB:తిరిగి intop(-, v1, v2);
    స్థితి LUA_OPMUL:తిరిగి intop(*, v1, v2);
    స్థితి LUA_OPMOD: తిరిగి luaV_mod(L, v1, v2);
    స్థితి LUA_OPIDIV: తిరిగి luaV_idiv(L, v1, v2);
    స్థితి LUA_OPBAND: తిరిగి intop(&, v1, v2);
    స్థితి LUA_OPBOR: తిరిగి intop(|, v1, v2);
    స్థితి LUA_OPBXOR: తిరిగి intop(^, v1, v2);
    స్థితి LUA_OPSHL: తిరిగి luaV_shiftl(v1, v2);
    స్థితి LUA_OPSHR: తిరిగి luaV_shiftr(v1, v2);
    స్థితి LUA_OPUNM: తిరిగి intop(-, 0, v1);
    స్థితి LUA_OPBNOT: తిరిగి intop(^, ~l_castS2U(0), v1);
    అప్రమేయ: lua_assert(0); తిరిగి 0;
  }
}


స్థిర lua_సంఖ్య numarith (lua_State *L, పూర్ణాంకం op, lua_సంఖ్య v1,
                                                  lua_సంఖ్య v2) {
  ఎంపిక (op) {
    స్థితి LUA_OPADD: తిరిగి luai_numadd(L, v1, v2);
    స్థితి LUA_OPSUB: తిరిగి luai_numsub(L, v1, v2);
    స్థితి LUA_OPMUL: తిరిగి luai_nummul(L, v1, v2);
    స్థితి LUA_OPDIV: తిరిగి luai_numdiv(L, v1, v2);
    స్థితి LUA_OPPOW: తిరిగి luai_numpow(L, v1, v2);
    స్థితి LUA_OPIDIV: తిరిగి luai_numidiv(L, v1, v2);
    స్థితి LUA_OPUNM: తిరిగి luai_numunm(L, v1);
    స్థితి LUA_OPMOD: తిరిగి luaV_modf(L, v1, v2);
    అప్రమేయ: lua_assert(0); తిరిగి 0;
  }
}


పూర్ణాంకం luaO_rawarith (lua_State *L, పూర్ణాంకం op, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                   TValue *res) {
  ఎంపిక (op) {
    స్థితి LUA_OPBAND: స్థితి LUA_OPBOR: స్థితి LUA_OPBXOR:
    స్థితి LUA_OPSHL: స్థితి LUA_OPSHR:
    స్థితి LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ఒకవేళ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        తిరిగి 1;
      }
      లేకపోతే తిరిగి 0;  /* fail */
    }
    స్థితి LUA_OPDIV: స్థితి LUA_OPPOW: {  /* operate only on floats */
      lua_సంఖ్య n1; lua_సంఖ్య n2;
      ఒకవేళ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        తిరిగి 1;
      }
      లేకపోతే తిరిగి 0;  /* fail */
    }
    అప్రమేయ: {  /* other operations */
      lua_సంఖ్య n1; lua_సంఖ్య n2;
      ఒకవేళ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        తిరిగి 1;
      }
      లేకపోతే ఒకవేళ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        తిరిగి 1;
      }
      లేకపోతే తిరిగి 0;  /* fail */
    }
  }
}


శూన్య luaO_arith (lua_State *L, పూర్ణాంకం op, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                 StkId res) {
  ఒకవేళ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_కూడు));
  }
}


lu_byte luaO_hexavalue (పూర్ణాంకం c) {
  lua_assert(lisxdigit(c));
  ఒకవేళ (lisdigit(c)) తిరిగి cast_byte(c - '0');
  లేకపోతే తిరిగి cast_byte((ltolower(c) - 'a') + 10);
}


స్థిర పూర్ణాంకం isneg (స్థిరరాశి అక్షరం **s) {
  ఒకవేళ (**s == '-') { (*s)++; తిరిగి 1; }
  లేకపోతే ఒకవేళ (**s == '+') (*s)++;
  తిరిగి 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ఒకవేళ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#నిర్వచించు MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
స్థిర lua_సంఖ్య lua_strx2number (స్థిరరాశి అక్షరం *s, అక్షరం **endptr) {
  పూర్ణాంకం dot = lua_getlocaledecpoint();
  lua_సంఖ్య r = l_mathop(0.0);  /* result (accumulator) */
  పూర్ణాంకం sigdig = 0;  /* number of significant digits */
  పూర్ణాంకం nosigdig = 0;  /* number of non-significant digits */
  పూర్ణాంకం e = 0;  /* exponent correction */
  పూర్ణాంకం neg;  /* 1 if number is negative */
  పూర్ణాంకం hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  అయితే (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ఒకవేళ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    తిరిగి l_mathop(0.0);  /* invalid format (no '0x') */
  కొరకు (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ఒకవేళ (*s == dot) {
      ఒకవేళ (hasdot) ఆపు;  /* second dot? stop loop */
      లేకపోతే hasdot = 1;
    }
    లేకపోతే ఒకవేళ (lisxdigit(cast_uchar(*s))) {
      ఒకవేళ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      లేకపోతే ఒకవేళ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      లేకపోతే e++;  /* too many digits; ignore, but still count for exponent */
      ఒకవేళ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    లేకపోతే ఆపు;  /* neither a dot nor a digit */
  }
  ఒకవేళ (nosigdig + sigdig == 0)  /* no digits? */
    తిరిగి l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ఒకవేళ (*s == 'p' || *s == 'P') {  /* exponent part? */
    పూర్ణాంకం exp1 = 0;  /* exponent value */
    పూర్ణాంకం neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ఒకవేళ (!lisdigit(cast_uchar(*s)))
      తిరిగి l_mathop(0.0);  /* invalid; must have at least one digit */
    అయితే (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ఒకవేళ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ఒకవేళ (neg) r = -r;
  తిరిగి l_mathop(ldexp)(r, e);
}

#అయితే_ముగింపు
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ఒకవేళ !defined (L_MAXLENNUM)
#నిర్వచించు L_MAXLENNUM	200
#అయితే_ముగింపు

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
స్థిర స్థిరరాశి అక్షరం *l_str2dloc (స్థిరరాశి అక్షరం *s, lua_సంఖ్య *ఫలితం, పూర్ణాంకం బహుళకం) {
  అక్షరం *endptr;
  *ఫలితం = (బహుళకం == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ఒకవేళ (endptr == s) తిరిగి ఖాళీ;  /* nothing recognized? */
  అయితే (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  తిరిగి (*endptr == '\0') ? endptr : ఖాళీ;  /* OK iff no trailing chars */
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
స్థిర స్థిరరాశి అక్షరం *l_str2d (స్థిరరాశి అక్షరం *s, lua_సంఖ్య *ఫలితం) {
  స్థిరరాశి అక్షరం *endptr;
  స్థిరరాశి అక్షరం *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  పూర్ణాంకం బహుళకం = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ఒకవేళ (బహుళకం == 'n')  /* reject 'inf' and 'nan' */
    తిరిగి ఖాళీ;
  endptr = l_str2dloc(s, ఫలితం, బహుళకం);  /* try to convert */
  ఒకవేళ (endptr == ఖాళీ) {  /* failed? may be a different locale */
    అక్షరం buff[L_MAXLENNUM + 1];
    స్థిరరాశి అక్షరం *pdot = strchr(s, '.');
    ఒకవేళ (pdot == ఖాళీ || అక్షరశ్రేణి_పొడవు(s) > L_MAXLENNUM)
      తిరిగి ఖాళీ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, ఫలితం, బహుళకం);  /* try again */
    ఒకవేళ (endptr != ఖాళీ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  తిరిగి endptr;
}


#నిర్వచించు MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#నిర్వచించు MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

స్థిర స్థిరరాశి అక్షరం *l_str2int (స్థిరరాశి అక్షరం *s, lua_Integer *ఫలితం) {
  lua_Unsigned a = 0;
  పూర్ణాంకం ఖాళీలు = 1;
  పూర్ణాంకం neg;
  అయితే (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ఒకవేళ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    కొరకు (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      ఖాళీలు = 0;
    }
  }
  లేకపోతే {  /* decimal */
    కొరకు (; lisdigit(cast_uchar(*s)); s++) {
      పూర్ణాంకం d = *s - '0';
      ఒకవేళ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        తిరిగి ఖాళీ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      ఖాళీలు = 0;
    }
  }
  అయితే (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ఒకవేళ (ఖాళీలు || *s != '\0') తిరిగి ఖాళీ;  /* something wrong in the numeral */
  లేకపోతే {
    *ఫలితం = l_castU2S((neg) ? 0u - a : a);
    తిరిగి s;
  }
}


size_t luaO_str2num (స్థిరరాశి అక్షరం *s, TValue *o) {
  lua_Integer i; lua_సంఖ్య n;
  స్థిరరాశి అక్షరం *e;
  ఒకవేళ ((e = l_str2int(s, &i)) != ఖాళీ) {  /* try as an integer */
    setivalue(o, i);
  }
  లేకపోతే ఒకవేళ ((e = l_str2d(s, &n)) != ఖాళీ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  లేకపోతే
    తిరిగి 0;  /* conversion failed */
  తిరిగి ct_diff2sz(e - s) + 1;  /* success; return string size */
}


పూర్ణాంకం luaO_utf8esc (అక్షరం *buff, l_uint32 x) {
  పూర్ణాంకం n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ఒకవేళ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  లేకపోతే {  /* need continuation bytes */
    గుర్తులేని పూర్ణాంకం mfb = 0x3f;  /* maximum that fits in first byte */
    చేయి {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } అయితే (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  తిరిగి n;
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
#ఒకవేళ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#దోషం "invalid value for LUA_N2SBUFFSZ"
#అయితే_ముగింపు


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
స్థిర పూర్ణాంకం tostringbuffFloat (lua_సంఖ్య n, అక్షరం *buff) {
  /* first conversion */
  పూర్ణాంకం len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_సంఖ్య_FMT,
                            (LUAI_UACNUMBER)n);
  lua_సంఖ్య check = lua_str2number(buff, ఖాళీ);  /* read it back */
  ఒకవేళ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_సంఖ్య_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ఒకవేళ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  తిరిగి len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
గుర్తులేని luaO_tostringbuff (స్థిరరాశి TValue *obj, అక్షరం *buff) {
  పూర్ణాంకం len;
  lua_assert(ttisnumber(obj));
  ఒకవేళ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  లేకపోతే
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  తిరిగి cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
శూన్య luaO_tostring (lua_State *L, TValue *obj) {
  అక్షరం buff[LUA_N2SBUFFSZ];
  గుర్తులేని len = luaO_tostringbuff(obj, buff);
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
#నిర్వచించు BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

/*
** Buffer used by 'luaO_pushvfstring'. 'err' signals an error while
** building result (memory error [1] or buffer overflow [2]).
*/
రకనిర్వచనం నిర్మాణం BuffFS {
  lua_State *L;
  అక్షరం *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  పూర్ణాంకం err;
  అక్షరం space[BUFVFS];  /* initial buffer */
} BuffFS;


స్థిర శూన్య initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = పరిమాణం(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
స్థిర శూన్య pushbuff (lua_State *L, శూన్య *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  ఎంపిక (buff->err) {
    స్థితి 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ఆపు;
    స్థితి 2:  /* length overflow: Add "..." at the end of result */
      ఒకవేళ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      లేకపోతే {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    అప్రమేయ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->శిఖరం.p, ts);
      L->శిఖరం.p++;
    }
  }
}


స్థిర స్థిరరాశి అక్షరం *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  స్థిరరాశి అక్షరం *res;
  ఒకవేళ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ఖాళీ;  /* error message is on the top of the stack */
  లేకపోతే
    res = getstr(tsvalue(s2v(L->శిఖరం.p - 1)));
  ఒకవేళ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  తిరిగి res;
}


స్థిర శూన్య addstr2buff (BuffFS *buff, స్థిరరాశి అక్షరం *str, size_t slen) {
  size_t ఎడమ = buff->buffsize - buff->blen;  /* space left in the buffer */
  ఒకవేళ (buff->err)  /* do nothing else after an error */
    తిరిగి;
  ఒకవేళ (slen > ఎడమ) {  /* new string doesn't fit into current buffer? */
    ఒకవేళ (slen > ((MAX_కొలత/2) - buff->blen)) {  /* overflow? */
      జ్ఞాపకం_నకలు(buff->b + buff->blen, str, ఎడమ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      తిరిగి;
    }
    లేకపోతే {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      అక్షరం *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ఖాళీ, 0, newsize, అక్షరం)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               అక్షరం);
      ఒకవేళ (newb == ఖాళీ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        తిరిగి;
      }
      ఒకవేళ (buff->b == buff->space)  /* new buffer (not reallocated)? */
        జ్ఞాపకం_నకలు(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  జ్ఞాపకం_నకలు(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
స్థిర శూన్య addnum2buff (BuffFS *buff, TValue *అంకె) {
  అక్షరం numbuff[LUA_N2SBUFFSZ];
  గుర్తులేని len = luaO_tostringbuff(అంకె, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
స్థిరరాశి అక్షరం *luaO_pushvfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  స్థిరరాశి అక్షరం *e;  /* points to next '%' */
  initbuff(L, &buff);
  అయితే ((e = strchr(fmt, '%')) != ఖాళీ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    ఎంపిక (*(e + 1)) {  /* conversion specifier */
      స్థితి 's': {  /* zero-terminated string */
        స్థిరరాశి అక్షరం *s = va_arg(argp, అక్షరం *);
        ఒకవేళ (s == ఖాళీ) s = "(null)";
        addstr2buff(&buff, s, అక్షరశ్రేణి_పొడవు(s));
        ఆపు;
      }
      స్థితి 'c': {  /* an 'int' as a character */
        అక్షరం c = cast_char(va_arg(argp, పూర్ణాంకం));
        addstr2buff(&buff, &c, పరిమాణం(అక్షరం));
        ఆపు;
      }
      స్థితి 'd': {  /* an 'int' */
        TValue అంకె;
        setivalue(&అంకె, va_arg(argp, పూర్ణాంకం));
        addnum2buff(&buff, &అంకె);
        ఆపు;
      }
      స్థితి 'I': {  /* a 'lua_Integer' */
        TValue అంకె;
        setivalue(&అంకె, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &అంకె);
        ఆపు;
      }
      స్థితి 'f': {  /* a 'lua_Number' */
        TValue అంకె;
        setfltvalue(&అంకె, cast_అంకె(va_arg(argp, l_uac_సంఖ్య)));
        addnum2buff(&buff, &అంకె);
        ఆపు;
      }
      స్థితి 'p': {  /* a pointer */
        అక్షరం bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        శూన్య *p = va_arg(argp, శూన్య *);
        పూర్ణాంకం len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ఆపు;
      }
      స్థితి 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        అక్షరం bf[UTF8BUFFSZ];
        గుర్తులేని దీర్ఘ arg = va_arg(argp, గుర్తులేని దీర్ఘ);
        పూర్ణాంకం len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ఆపు;
      }
      స్థితి '%': {
        addstr2buff(&buff, "%", 1);
        ఆపు;
      }
      అప్రమేయ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ఆపు;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, అక్షరశ్రేణి_పొడవు(fmt));  /* rest of 'fmt' */
  తిరిగి clearbuff(&buff);  /* empty buffer into a new string */
}


స్థిరరాశి అక్షరం *luaO_pushfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...) {
  స్థిరరాశి అక్షరం *సందేశం;
  va_list argp;
  va_start(argp, fmt);
  సందేశం = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ఒకవేళ (సందేశం == ఖాళీ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  తిరిగి సందేశం;
}

/* }================================================================== */


#నిర్వచించు RETS	"..."
#నిర్వచించు PRE	"[string \""
#నిర్వచించు POS	"\"]"

#నిర్వచించు addstr(a,b,l)	( జ్ఞాపకం_నకలు(a,b,(l) * పరిమాణం(అక్షరం)), a += (l) )

శూన్య luaO_chunkid (అక్షరం *బహిర్గతం, స్థిరరాశి అక్షరం *ఆధారం, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ఒకవేళ (*ఆధారం == '=') {  /* 'literal' source */
    ఒకవేళ (srclen <= bufflen)  /* small enough? */
      జ్ఞాపకం_నకలు(బహిర్గతం, ఆధారం + 1, srclen * పరిమాణం(అక్షరం));
    లేకపోతే {  /* truncate it */
      addstr(బహిర్గతం, ఆధారం + 1, bufflen - 1);
      *బహిర్గతం = '\0';
    }
  }
  లేకపోతే ఒకవేళ (*ఆధారం == '@') {  /* file name */
    ఒకవేళ (srclen <= bufflen)  /* small enough? */
      జ్ఞాపకం_నకలు(బహిర్గతం, ఆధారం + 1, srclen * పరిమాణం(అక్షరం));
    లేకపోతే {  /* add '...' before rest of name */
      addstr(బహిర్గతం, RETS, LL(RETS));
      bufflen -= LL(RETS);
      జ్ఞాపకం_నకలు(బహిర్గతం, ఆధారం + 1 + srclen - bufflen, bufflen * పరిమాణం(అక్షరం));
    }
  }
  లేకపోతే {  /* string; format as [string "source"] */
    స్థిరరాశి అక్షరం *nl = strchr(ఆధారం, '\n');  /* find first new line (if any) */
    addstr(బహిర్గతం, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ఒకవేళ (srclen < bufflen && nl == ఖాళీ) {  /* small one-line source? */
      addstr(బహిర్గతం, ఆధారం, srclen);  /* keep it */
    }
    లేకపోతే {
      ఒకవేళ (nl != ఖాళీ)
        srclen = ct_diff2sz(nl - ఆధారం);  /* stop at first newline */
      ఒకవేళ (srclen > bufflen) srclen = bufflen;
      addstr(బహిర్గతం, ఆధారం, srclen);
      addstr(బహిర్గతం, RETS, LL(RETS));
    }
    జ్ఞాపకం_నకలు(బహిర్గతం, POS, (LL(POS) + 1) * పరిమాణం(అక్షరం));
  }
}

