/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <float.h>
#شامل <locale.h>
#شامل <math.h>
#شامل <stdarg.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lctype.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned صحیح x) {
  ایستا ثابت lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  صحیح l = 0;
  x--;
  وقتی (x >= 256) { l += 8; x >>= 8; }
  بازگشت cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned صحیح p) {
  اگر (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    بازگشت 0xFF;  /* return maximum value */
  وگرنه {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    اگر (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      بازگشت cast_byte(p);
    }
    وگرنه {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      بازگشت cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  صحیح m = p & 0xF;  /* mantissa */
  صحیح e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  اگر (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  اگر (e >= 0) {
    اگر (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      بازگشت (x * m) << e;  /* order doesn't matter here */
    وگرنه  /* real overflow */
      بازگشت MAX_LMEM;
  }
  وگرنه {  /* negative exponent */
    e = -e;
    اگر (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      بازگشت (x * m) >> e;  /* multiplying first gives more precision */
    وگرنه اگر ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      بازگشت (x >> e) * m;
    وگرنه  /* real overflow */
      بازگشت MAX_LMEM;
  }
}


ایستا lua_Integer intarith (lua_State *L, صحیح op, lua_Integer v1,
                                                   lua_Integer v2) {
  انتخاب (op) {
    حالت LUA_OPADD: بازگشت intop(+, v1, v2);
    حالت LUA_OPSUB:بازگشت intop(-, v1, v2);
    حالت LUA_OPMUL:بازگشت intop(*, v1, v2);
    حالت LUA_OPMOD: بازگشت luaV_mod(L, v1, v2);
    حالت LUA_OPIDIV: بازگشت luaV_idiv(L, v1, v2);
    حالت LUA_OPBAND: بازگشت intop(&, v1, v2);
    حالت LUA_OPBOR: بازگشت intop(|, v1, v2);
    حالت LUA_OPBXOR: بازگشت intop(^, v1, v2);
    حالت LUA_OPSHL: بازگشت luaV_shiftl(v1, v2);
    حالت LUA_OPSHR: بازگشت luaV_shiftr(v1, v2);
    حالت LUA_OPUNM: بازگشت intop(-, 0, v1);
    حالت LUA_OPBNOT: بازگشت intop(^, ~l_castS2U(0), v1);
    پیشفرض: lua_assert(0); بازگشت 0;
  }
}


ایستا lua_عدد numarith (lua_State *L, صحیح op, lua_عدد v1,
                                                  lua_عدد v2) {
  انتخاب (op) {
    حالت LUA_OPADD: بازگشت luai_numadd(L, v1, v2);
    حالت LUA_OPSUB: بازگشت luai_numsub(L, v1, v2);
    حالت LUA_OPMUL: بازگشت luai_nummul(L, v1, v2);
    حالت LUA_OPDIV: بازگشت luai_numdiv(L, v1, v2);
    حالت LUA_OPPOW: بازگشت luai_numpow(L, v1, v2);
    حالت LUA_OPIDIV: بازگشت luai_numidiv(L, v1, v2);
    حالت LUA_OPUNM: بازگشت luai_numunm(L, v1);
    حالت LUA_OPMOD: بازگشت luaV_modf(L, v1, v2);
    پیشفرض: lua_assert(0); بازگشت 0;
  }
}


صحیح luaO_rawarith (lua_State *L, صحیح op, ثابت TValue *p1, ثابت TValue *p2,
                   TValue *res) {
  انتخاب (op) {
    حالت LUA_OPBAND: حالت LUA_OPBOR: حالت LUA_OPBXOR:
    حالت LUA_OPSHL: حالت LUA_OPSHR:
    حالت LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      اگر (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        بازگشت 1;
      }
      وگرنه بازگشت 0;  /* fail */
    }
    حالت LUA_OPDIV: حالت LUA_OPPOW: {  /* operate only on floats */
      lua_عدد n1; lua_عدد n2;
      اگر (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        بازگشت 1;
      }
      وگرنه بازگشت 0;  /* fail */
    }
    پیشفرض: {  /* other operations */
      lua_عدد n1; lua_عدد n2;
      اگر (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        بازگشت 1;
      }
      وگرنه اگر (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        بازگشت 1;
      }
      وگرنه بازگشت 0;  /* fail */
    }
  }
}


تهی luaO_arith (lua_State *L, صحیح op, ثابت TValue *p1, ثابت TValue *p2,
                 StkId res) {
  اگر (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (صحیح c) {
  lua_assert(lisxdigit(c));
  اگر (lisdigit(c)) بازگشت cast_byte(c - '0');
  وگرنه بازگشت cast_byte((ltolower(c) - 'a') + 10);
}


ایستا صحیح isneg (ثابت نویسه **s) {
  اگر (**s == '-') { (*s)++; بازگشت 1; }
  وگرنه اگر (**s == '+') (*s)++;
  بازگشت 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#اگر !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ایستا lua_عدد lua_strx2number (ثابت نویسه *s, نویسه **endptr) {
  صحیح dot = lua_getlocaledecpoint();
  lua_عدد r = l_mathop(0.0);  /* result (accumulator) */
  صحیح sigdig = 0;  /* number of significant digits */
  صحیح nosigdig = 0;  /* number of non-significant digits */
  صحیح e = 0;  /* exponent correction */
  صحیح neg;  /* 1 if number is negative */
  صحیح hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  وقتی (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  اگر (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    بازگشت l_mathop(0.0);  /* invalid format (no '0x') */
  برای (s += 2; ; s++) {  /* skip '0x' and read numeral */
    اگر (*s == dot) {
      اگر (hasdot) بشکن;  /* second dot? stop loop */
      وگرنه hasdot = 1;
    }
    وگرنه اگر (lisxdigit(cast_uchar(*s))) {
      اگر (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      وگرنه اگر (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      وگرنه e++;  /* too many digits; ignore, but still count for exponent */
      اگر (hasdot) e--;  /* decimal digit? correct exponent */
    }
    وگرنه بشکن;  /* neither a dot nor a digit */
  }
  اگر (nosigdig + sigdig == 0)  /* no digits? */
    بازگشت l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  اگر (*s == 'p' || *s == 'P') {  /* exponent part? */
    صحیح exp1 = 0;  /* exponent value */
    صحیح neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    اگر (!lisdigit(cast_uchar(*s)))
      بازگشت l_mathop(0.0);  /* invalid; must have at least one digit */
    وقتی (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    اگر (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  اگر (neg) r = -r;
  بازگشت l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#اگر !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ایستا ثابت نویسه *l_str2dloc (ثابت نویسه *s, lua_عدد *result, صحیح mode) {
  نویسه *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  اگر (endptr == s) بازگشت پوچ;  /* nothing recognized? */
  وقتی (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  بازگشت (*endptr == '\0') ? endptr : پوچ;  /* OK iff no trailing chars */
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
ایستا ثابت نویسه *l_str2d (ثابت نویسه *s, lua_عدد *result) {
  ثابت نویسه *endptr;
  ثابت نویسه *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  صحیح mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  اگر (mode == 'n')  /* reject 'inf' and 'nan' */
    بازگشت پوچ;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  اگر (endptr == پوچ) {  /* failed? may be a different locale */
    نویسه buff[L_MAXLENNUM + 1];
    ثابت نویسه *pdot = strchr(s, '.');
    اگر (pdot == پوچ || strlen(s) > L_MAXLENNUM)
      بازگشت پوچ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    اگر (endptr != پوچ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  بازگشت endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ایستا ثابت نویسه *l_str2int (ثابت نویسه *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  صحیح empty = 1;
  صحیح neg;
  وقتی (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  اگر (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    برای (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  وگرنه {  /* decimal */
    برای (; lisdigit(cast_uchar(*s)); s++) {
      صحیح d = *s - '0';
      اگر (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        بازگشت پوچ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  وقتی (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  اگر (empty || *s != '\0') بازگشت پوچ;  /* something wrong in the numeral */
  وگرنه {
    *result = l_castU2S((neg) ? 0u - a : a);
    بازگشت s;
  }
}


size_t luaO_str2num (ثابت نویسه *s, TValue *o) {
  lua_Integer i; lua_عدد n;
  ثابت نویسه *e;
  اگر ((e = l_str2int(s, &i)) != پوچ) {  /* try as an integer */
    setivalue(o, i);
  }
  وگرنه اگر ((e = l_str2d(s, &n)) != پوچ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  وگرنه
    بازگشت 0;  /* conversion failed */
  بازگشت ct_diff2sz(e - s) + 1;  /* success; return string size */
}


صحیح luaO_utf8esc (نویسه *buff, l_uint32 x) {
  صحیح n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  اگر (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  وگرنه {  /* need continuation bytes */
    unsigned صحیح mfb = 0x3f;  /* maximum that fits in first byte */
    انجامبده {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } وقتی (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  بازگشت n;
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
#اگر LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
ایستا صحیح tostringbuffFloat (lua_عدد n, نویسه *buff) {
  /* first conversion */
  صحیح len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_عدد_FMT,
                            (LUAI_UACNUMBER)n);
  lua_عدد check = lua_str2number(buff, پوچ);  /* read it back */
  اگر (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_عدد_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  اگر (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  بازگشت len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ثابت TValue *obj, نویسه *buff) {
  صحیح len;
  lua_assert(ttisnumber(obj));
  اگر (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  وگرنه
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  بازگشت cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
تهی luaO_tostring (lua_State *L, TValue *obj) {
  نویسه buff[LUA_N2SBUFFSZ];
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
تعریفنوع ساختار BuffFS {
  lua_State *L;
  نویسه *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  صحیح err;
  نویسه space[BUFVFS];  /* initial buffer */
} BuffFS;


ایستا تهی initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = اندازه(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ایستا تهی pushbuff (lua_State *L, تهی *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  انتخاب (buff->err) {
    حالت 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      بشکن;
    حالت 2:  /* length overflow: Add "..." at the end of result */
      اگر (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      وگرنه {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    پیشفرض: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->بالاترین.p, ts);
      L->بالاترین.p++;
    }
  }
}


ایستا ثابت نویسه *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ثابت نویسه *res;
  اگر (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = پوچ;  /* error message is on the top of the stack */
  وگرنه
    res = getstr(tsvalue(s2v(L->بالاترین.p - 1)));
  اگر (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  بازگشت res;
}


ایستا تهی addstr2buff (BuffFS *buff, ثابت نویسه *str, size_t slen) {
  size_t چپ = buff->buffsize - buff->blen;  /* space left in the buffer */
  اگر (buff->err)  /* do nothing else after an error */
    بازگشت;
  اگر (slen > چپ) {  /* new string doesn't fit into current buffer? */
    اگر (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, چپ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      بازگشت;
    }
    وگرنه {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      نویسه *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, پوچ, 0, newsize, نویسه)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               نویسه);
      اگر (newb == پوچ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        بازگشت;
      }
      اگر (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
ایستا تهی addnum2buff (BuffFS *buff, TValue *رقم) {
  نویسه numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(رقم, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ثابت نویسه *luaO_pushvfstring (lua_State *L, ثابت نویسه *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ثابت نویسه *e;  /* points to next '%' */
  initbuff(L, &buff);
  وقتی ((e = strchr(fmt, '%')) != پوچ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    انتخاب (*(e + 1)) {  /* conversion specifier */
      حالت 's': {  /* zero-terminated string */
        ثابت نویسه *s = va_arg(argp, نویسه *);
        اگر (s == پوچ) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        بشکن;
      }
      حالت 'c': {  /* an 'int' as a character */
        نویسه c = cast_char(va_arg(argp, صحیح));
        addstr2buff(&buff, &c, اندازه(نویسه));
        بشکن;
      }
      حالت 'd': {  /* an 'int' */
        TValue رقم;
        setivalue(&رقم, va_arg(argp, صحیح));
        addnum2buff(&buff, &رقم);
        بشکن;
      }
      حالت 'I': {  /* a 'lua_Integer' */
        TValue رقم;
        setivalue(&رقم, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &رقم);
        بشکن;
      }
      حالت 'f': {  /* a 'lua_Number' */
        TValue رقم;
        setfltvalue(&رقم, cast_رقم(va_arg(argp, l_uac_عدد)));
        addnum2buff(&buff, &رقم);
        بشکن;
      }
      حالت 'p': {  /* a pointer */
        نویسه bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        تهی *p = va_arg(argp, تهی *);
        صحیح len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        بشکن;
      }
      حالت 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        نویسه bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        صحیح len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        بشکن;
      }
      حالت '%': {
        addstr2buff(&buff, "%", 1);
        بشکن;
      }
      پیشفرض: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        بشکن;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  بازگشت clearbuff(&buff);  /* empty buffer into a new string */
}


ثابت نویسه *luaO_pushfstring (lua_State *L, ثابت نویسه *fmt, ...) {
  ثابت نویسه *پیام;
  va_list argp;
  va_start(argp, fmt);
  پیام = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  اگر (پیام == پوچ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  بازگشت پیام;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * اندازه(نویسه)), a += (l) )

تهی luaO_chunkid (نویسه *out, ثابت نویسه *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  اگر (*source == '=') {  /* 'literal' source */
    اگر (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * اندازه(نویسه));
    وگرنه {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  وگرنه اگر (*source == '@') {  /* file name */
    اگر (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * اندازه(نویسه));
    وگرنه {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * اندازه(نویسه));
    }
  }
  وگرنه {  /* string; format as [string "source"] */
    ثابت نویسه *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    اگر (srclen < bufflen && nl == پوچ) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    وگرنه {
      اگر (nl != پوچ)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      اگر (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * اندازه(نویسه));
  }
}

