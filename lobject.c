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
lu_byte luaO_ceillog2 (unsigned পূর্ণসংখ্যা x) {
  স্থির ধ্রুবক lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  পূর্ণসংখ্যা l = 0;
  x--;
  যখন (x >= 256) { l += 8; x >>= 8; }
  ফেরত cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned পূর্ণসংখ্যা p) {
  যদি (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ফেরত 0xFF;  /* return maximum value */
  অন্যথা {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    যদি (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ফেরত cast_byte(p);
    }
    অন্যথা {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      ফেরত cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  পূর্ণসংখ্যা m = p & 0xF;  /* mantissa */
  পূর্ণসংখ্যা e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  যদি (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  যদি (e >= 0) {
    যদি (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ফেরত (x * m) << e;  /* order doesn't matter here */
    অন্যথা  /* real overflow */
      ফেরত MAX_LMEM;
  }
  অন্যথা {  /* negative exponent */
    e = -e;
    যদি (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ফেরত (x * m) >> e;  /* multiplying first gives more precision */
    অন্যথা যদি ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ফেরত (x >> e) * m;
    অন্যথা  /* real overflow */
      ফেরত MAX_LMEM;
  }
}


স্থির lua_Integer intarith (lua_State *L, পূর্ণসংখ্যা op, lua_Integer v1,
                                                   lua_Integer v2) {
  নির্বাচন (op) {
    ক্ষেত্র LUA_OPADD: ফেরত intop(+, v1, v2);
    ক্ষেত্র LUA_OPSUB:ফেরত intop(-, v1, v2);
    ক্ষেত্র LUA_OPMUL:ফেরত intop(*, v1, v2);
    ক্ষেত্র LUA_OPMOD: ফেরত luaV_mod(L, v1, v2);
    ক্ষেত্র LUA_OPIDIV: ফেরত luaV_idiv(L, v1, v2);
    ক্ষেত্র LUA_OPBAND: ফেরত intop(&, v1, v2);
    ক্ষেত্র LUA_OPBOR: ফেরত intop(|, v1, v2);
    ক্ষেত্র LUA_OPBXOR: ফেরত intop(^, v1, v2);
    ক্ষেত্র LUA_OPSHL: ফেরত luaV_shiftl(v1, v2);
    ক্ষেত্র LUA_OPSHR: ফেরত luaV_shiftr(v1, v2);
    ক্ষেত্র LUA_OPUNM: ফেরত intop(-, 0, v1);
    ক্ষেত্র LUA_OPBNOT: ফেরত intop(^, ~l_castS2U(0), v1);
    পূর্বনির্ধারিত: lua_assert(0); ফেরত 0;
  }
}


স্থির lua_সংখ্যা numarith (lua_State *L, পূর্ণসংখ্যা op, lua_সংখ্যা v1,
                                                  lua_সংখ্যা v2) {
  নির্বাচন (op) {
    ক্ষেত্র LUA_OPADD: ফেরত luai_numadd(L, v1, v2);
    ক্ষেত্র LUA_OPSUB: ফেরত luai_numsub(L, v1, v2);
    ক্ষেত্র LUA_OPMUL: ফেরত luai_nummul(L, v1, v2);
    ক্ষেত্র LUA_OPDIV: ফেরত luai_numdiv(L, v1, v2);
    ক্ষেত্র LUA_OPPOW: ফেরত luai_numpow(L, v1, v2);
    ক্ষেত্র LUA_OPIDIV: ফেরত luai_numidiv(L, v1, v2);
    ক্ষেত্র LUA_OPUNM: ফেরত luai_numunm(L, v1);
    ক্ষেত্র LUA_OPMOD: ফেরত luaV_modf(L, v1, v2);
    পূর্বনির্ধারিত: lua_assert(0); ফেরত 0;
  }
}


পূর্ণসংখ্যা luaO_rawarith (lua_State *L, পূর্ণসংখ্যা op, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                   TValue *res) {
  নির্বাচন (op) {
    ক্ষেত্র LUA_OPBAND: ক্ষেত্র LUA_OPBOR: ক্ষেত্র LUA_OPBXOR:
    ক্ষেত্র LUA_OPSHL: ক্ষেত্র LUA_OPSHR:
    ক্ষেত্র LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      যদি (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ফেরত 1;
      }
      অন্যথা ফেরত 0;  /* fail */
    }
    ক্ষেত্র LUA_OPDIV: ক্ষেত্র LUA_OPPOW: {  /* operate only on floats */
      lua_সংখ্যা n1; lua_সংখ্যা n2;
      যদি (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ফেরত 1;
      }
      অন্যথা ফেরত 0;  /* fail */
    }
    পূর্বনির্ধারিত: {  /* other operations */
      lua_সংখ্যা n1; lua_সংখ্যা n2;
      যদি (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ফেরত 1;
      }
      অন্যথা যদি (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ফেরত 1;
      }
      অন্যথা ফেরত 0;  /* fail */
    }
  }
}


শূন্য luaO_arith (lua_State *L, পূর্ণসংখ্যা op, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                 StkId res) {
  যদি (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (পূর্ণসংখ্যা c) {
  lua_assert(lisxdigit(c));
  যদি (lisdigit(c)) ফেরত cast_byte(c - '0');
  অন্যথা ফেরত cast_byte((ltolower(c) - 'a') + 10);
}


স্থির পূর্ণসংখ্যা isneg (ধ্রুবক অক্ষর **s) {
  যদি (**s == '-') { (*s)++; ফেরত 1; }
  অন্যথা যদি (**s == '+') (*s)++;
  ফেরত 0;
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
স্থির lua_সংখ্যা lua_strx2number (ধ্রুবক অক্ষর *s, অক্ষর **endptr) {
  পূর্ণসংখ্যা dot = lua_getlocaledecpoint();
  lua_সংখ্যা r = l_mathop(0.0);  /* result (accumulator) */
  পূর্ণসংখ্যা sigdig = 0;  /* number of significant digits */
  পূর্ণসংখ্যা nosigdig = 0;  /* number of non-significant digits */
  পূর্ণসংখ্যা e = 0;  /* exponent correction */
  পূর্ণসংখ্যা neg;  /* 1 if number is negative */
  পূর্ণসংখ্যা hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  যখন (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  যদি (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ফেরত l_mathop(0.0);  /* invalid format (no '0x') */
  জন্য (s += 2; ; s++) {  /* skip '0x' and read numeral */
    যদি (*s == dot) {
      যদি (hasdot) বিরতি;  /* second dot? stop loop */
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
    অন্যথা বিরতি;  /* neither a dot nor a digit */
  }
  যদি (nosigdig + sigdig == 0)  /* no digits? */
    ফেরত l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  যদি (*s == 'p' || *s == 'P') {  /* exponent part? */
    পূর্ণসংখ্যা exp1 = 0;  /* exponent value */
    পূর্ণসংখ্যা neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    যদি (!lisdigit(cast_uchar(*s)))
      ফেরত l_mathop(0.0);  /* invalid; must have at least one digit */
    যখন (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    যদি (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  যদি (neg) r = -r;
  ফেরত l_mathop(ldexp)(r, e);
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
স্থির ধ্রুবক অক্ষর *l_str2dloc (ধ্রুবক অক্ষর *s, lua_সংখ্যা *ফলাফল, পূর্ণসংখ্যা mode) {
  অক্ষর *endptr;
  *ফলাফল = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  যদি (endptr == s) ফেরত ফাঁকা;  /* nothing recognized? */
  যখন (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ফেরত (*endptr == '\0') ? endptr : ফাঁকা;  /* OK iff no trailing chars */
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
স্থির ধ্রুবক অক্ষর *l_str2d (ধ্রুবক অক্ষর *s, lua_সংখ্যা *ফলাফল) {
  ধ্রুবক অক্ষর *endptr;
  ধ্রুবক অক্ষর *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  পূর্ণসংখ্যা mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  যদি (mode == 'n')  /* reject 'inf' and 'nan' */
    ফেরত ফাঁকা;
  endptr = l_str2dloc(s, ফলাফল, mode);  /* try to convert */
  যদি (endptr == ফাঁকা) {  /* failed? may be a different locale */
    অক্ষর buff[L_MAXLENNUM + 1];
    ধ্রুবক অক্ষর *pdot = strchr(s, '.');
    যদি (pdot == ফাঁকা || strlen(s) > L_MAXLENNUM)
      ফেরত ফাঁকা;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, ফলাফল, mode);  /* try again */
    যদি (endptr != ফাঁকা)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ফেরত endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

স্থির ধ্রুবক অক্ষর *l_str2int (ধ্রুবক অক্ষর *s, lua_Integer *ফলাফল) {
  lua_Unsigned a = 0;
  পূর্ণসংখ্যা empty = 1;
  পূর্ণসংখ্যা neg;
  যখন (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  যদি (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    জন্য (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  অন্যথা {  /* decimal */
    জন্য (; lisdigit(cast_uchar(*s)); s++) {
      পূর্ণসংখ্যা d = *s - '0';
      যদি (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ফেরত ফাঁকা;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  যখন (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  যদি (empty || *s != '\0') ফেরত ফাঁকা;  /* something wrong in the numeral */
  অন্যথা {
    *ফলাফল = l_castU2S((neg) ? 0u - a : a);
    ফেরত s;
  }
}


size_t luaO_str2num (ধ্রুবক অক্ষর *s, TValue *o) {
  lua_Integer i; lua_সংখ্যা n;
  ধ্রুবক অক্ষর *e;
  যদি ((e = l_str2int(s, &i)) != ফাঁকা) {  /* try as an integer */
    setivalue(o, i);
  }
  অন্যথা যদি ((e = l_str2d(s, &n)) != ফাঁকা) {  /* else try as a float */
    setfltvalue(o, n);
  }
  অন্যথা
    ফেরত 0;  /* conversion failed */
  ফেরত ct_diff2sz(e - s) + 1;  /* success; return string size */
}


পূর্ণসংখ্যা luaO_utf8esc (অক্ষর *buff, l_uint32 x) {
  পূর্ণসংখ্যা n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  যদি (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  অন্যথা {  /* need continuation bytes */
    unsigned পূর্ণসংখ্যা mfb = 0x3f;  /* maximum that fits in first byte */
    করো {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } যখন (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ফেরত n;
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
স্থির পূর্ণসংখ্যা tostringbuffFloat (lua_সংখ্যা n, অক্ষর *buff) {
  /* first conversion */
  পূর্ণসংখ্যা len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_সংখ্যা_FMT,
                            (LUAI_UACNUMBER)n);
  lua_সংখ্যা check = lua_str2number(buff, ফাঁকা);  /* read it back */
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
  ফেরত len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ধ্রুবক TValue *obj, অক্ষর *buff) {
  পূর্ণসংখ্যা len;
  lua_assert(ttisnumber(obj));
  যদি (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  অন্যথা
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ফেরত cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
শূন্য luaO_tostring (lua_State *L, TValue *obj) {
  অক্ষর buff[LUA_N2SBUFFSZ];
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
প্রকারলেখ কাঠামো BuffFS {
  lua_State *L;
  অক্ষর *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  পূর্ণসংখ্যা err;
  অক্ষর space[BUFVFS];  /* initial buffer */
} BuffFS;


স্থির শূন্য initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = আকার(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
স্থির শূন্য pushbuff (lua_State *L, শূন্য *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  নির্বাচন (buff->err) {
    ক্ষেত্র 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      বিরতি;
    ক্ষেত্র 2:  /* length overflow: Add "..." at the end of result */
      যদি (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      অন্যথা {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    পূর্বনির্ধারিত: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->শীর্ষ.p, ts);
      L->শীর্ষ.p++;
    }
  }
}


স্থির ধ্রুবক অক্ষর *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ধ্রুবক অক্ষর *res;
  যদি (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ফাঁকা;  /* error message is on the top of the stack */
  অন্যথা
    res = getstr(tsvalue(s2v(L->শীর্ষ.p - 1)));
  যদি (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ফেরত res;
}


স্থির শূন্য addstr2buff (BuffFS *buff, ধ্রুবক অক্ষর *str, size_t slen) {
  size_t বাম = buff->buffsize - buff->blen;  /* space left in the buffer */
  যদি (buff->err)  /* do nothing else after an error */
    ফেরত;
  যদি (slen > বাম) {  /* new string doesn't fit into current buffer? */
    যদি (slen > ((MAX_আকার/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, বাম);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ফেরত;
    }
    অন্যথা {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      অক্ষর *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ফাঁকা, 0, newsize, অক্ষর)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               অক্ষর);
      যদি (newb == ফাঁকা) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ফেরত;
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
  অক্ষর numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(অঙ্ক, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ধ্রুবক অক্ষর *luaO_pushvfstring (lua_State *L, ধ্রুবক অক্ষর *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ধ্রুবক অক্ষর *e;  /* points to next '%' */
  initbuff(L, &buff);
  যখন ((e = strchr(fmt, '%')) != ফাঁকা) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    নির্বাচন (*(e + 1)) {  /* conversion specifier */
      ক্ষেত্র 's': {  /* zero-terminated string */
        ধ্রুবক অক্ষর *s = va_arg(argp, অক্ষর *);
        যদি (s == ফাঁকা) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        বিরতি;
      }
      ক্ষেত্র 'c': {  /* an 'int' as a character */
        অক্ষর c = cast_char(va_arg(argp, পূর্ণসংখ্যা));
        addstr2buff(&buff, &c, আকার(অক্ষর));
        বিরতি;
      }
      ক্ষেত্র 'd': {  /* an 'int' */
        TValue অঙ্ক;
        setivalue(&অঙ্ক, va_arg(argp, পূর্ণসংখ্যা));
        addnum2buff(&buff, &অঙ্ক);
        বিরতি;
      }
      ক্ষেত্র 'I': {  /* a 'lua_Integer' */
        TValue অঙ্ক;
        setivalue(&অঙ্ক, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &অঙ্ক);
        বিরতি;
      }
      ক্ষেত্র 'f': {  /* a 'lua_Number' */
        TValue অঙ্ক;
        setfltvalue(&অঙ্ক, cast_অঙ্ক(va_arg(argp, l_uac_সংখ্যা)));
        addnum2buff(&buff, &অঙ্ক);
        বিরতি;
      }
      ক্ষেত্র 'p': {  /* a pointer */
        অক্ষর bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        শূন্য *p = va_arg(argp, শূন্য *);
        পূর্ণসংখ্যা len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        বিরতি;
      }
      ক্ষেত্র 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        অক্ষর bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        পূর্ণসংখ্যা len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        বিরতি;
      }
      ক্ষেত্র '%': {
        addstr2buff(&buff, "%", 1);
        বিরতি;
      }
      পূর্বনির্ধারিত: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        বিরতি;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  ফেরত clearbuff(&buff);  /* empty buffer into a new string */
}


ধ্রুবক অক্ষর *luaO_pushfstring (lua_State *L, ধ্রুবক অক্ষর *fmt, ...) {
  ধ্রুবক অক্ষর *বার্তা;
  va_list argp;
  va_start(argp, fmt);
  বার্তা = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  যদি (বার্তা == ফাঁকা)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ফেরত বার্তা;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * আকার(অক্ষর)), a += (l) )

শূন্য luaO_chunkid (অক্ষর *out, ধ্রুবক অক্ষর *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  যদি (*source == '=') {  /* 'literal' source */
    যদি (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * আকার(অক্ষর));
    অন্যথা {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  অন্যথা যদি (*source == '@') {  /* file name */
    যদি (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * আকার(অক্ষর));
    অন্যথা {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * আকার(অক্ষর));
    }
  }
  অন্যথা {  /* string; format as [string "source"] */
    ধ্রুবক অক্ষর *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    যদি (srclen < bufflen && nl == ফাঁকা) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    অন্যথা {
      যদি (nl != ফাঁকা)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      যদি (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * আকার(অক্ষর));
  }
}

