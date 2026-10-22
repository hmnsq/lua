/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <float.h>
#ضمن <locale.h>
#ضمن <math.h>
#ضمن <stdarg.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lctype.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned صحيح x) {
  ساكن ثابت lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  صحيح l = 0;
  x--;
  כלעוד (x >= 256) { l += 8; x >>= 8; }
  החזר cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned صحيح p) {
  אם (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    החזר 0xFF;  /* return maximum value */
  אחרת {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    אם (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      החזר cast_byte(p);
    }
    אחרת {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      החזר cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  صحيح m = p & 0xF;  /* mantissa */
  صحيح e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  אם (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  אם (e >= 0) {
    אם (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      החזר (x * m) << e;  /* order doesn't matter here */
    אחרת  /* real overflow */
      החזר MAX_LMEM;
  }
  אחרת {  /* negative exponent */
    e = -e;
    אם (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      החזר (x * m) >> e;  /* multiplying first gives more precision */
    אחרת אם ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      החזר (x >> e) * m;
    אחרת  /* real overflow */
      החזר MAX_LMEM;
  }
}


ساكن lua_Integer intarith (lua_State *L, صحيح op, lua_Integer v1,
                                                   lua_Integer v2) {
  بدل (op) {
    حالة LUA_OPADD: החזר intop(+, v1, v2);
    حالة LUA_OPSUB:החזר intop(-, v1, v2);
    حالة LUA_OPMUL:החזר intop(*, v1, v2);
    حالة LUA_OPMOD: החזר luaV_mod(L, v1, v2);
    حالة LUA_OPIDIV: החזר luaV_idiv(L, v1, v2);
    حالة LUA_OPBAND: החזר intop(&, v1, v2);
    حالة LUA_OPBOR: החזר intop(|, v1, v2);
    حالة LUA_OPBXOR: החזר intop(^, v1, v2);
    حالة LUA_OPSHL: החזר luaV_shiftl(v1, v2);
    حالة LUA_OPSHR: החזר luaV_shiftr(v1, v2);
    حالة LUA_OPUNM: החזר intop(-, 0, v1);
    حالة LUA_OPBNOT: החזר intop(^, ~l_castS2U(0), v1);
    افتراضي: lua_assert(0); החזר 0;
  }
}


ساكن lua_Number numarith (lua_State *L, صحيح op, lua_Number v1,
                                                  lua_Number v2) {
  بدل (op) {
    حالة LUA_OPADD: החזר luai_numadd(L, v1, v2);
    حالة LUA_OPSUB: החזר luai_numsub(L, v1, v2);
    حالة LUA_OPMUL: החזר luai_nummul(L, v1, v2);
    حالة LUA_OPDIV: החזר luai_numdiv(L, v1, v2);
    حالة LUA_OPPOW: החזר luai_numpow(L, v1, v2);
    حالة LUA_OPIDIV: החזר luai_numidiv(L, v1, v2);
    حالة LUA_OPUNM: החזר luai_numunm(L, v1);
    حالة LUA_OPMOD: החזר luaV_modf(L, v1, v2);
    افتراضي: lua_assert(0); החזר 0;
  }
}


صحيح luaO_rawarith (lua_State *L, صحيح op, ثابت TValue *p1, ثابت TValue *p2,
                   TValue *res) {
  بدل (op) {
    حالة LUA_OPBAND: حالة LUA_OPBOR: حالة LUA_OPBXOR:
    حالة LUA_OPSHL: حالة LUA_OPSHR:
    حالة LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      אם (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        החזר 1;
      }
      אחרת החזר 0;  /* fail */
    }
    حالة LUA_OPDIV: حالة LUA_OPPOW: {  /* operate only on floats */
      lua_Number n1; lua_Number n2;
      אם (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        החזר 1;
      }
      אחרת החזר 0;  /* fail */
    }
    افتراضي: {  /* other operations */
      lua_Number n1; lua_Number n2;
      אם (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        החזר 1;
      }
      אחרת אם (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        החזר 1;
      }
      אחרת החזר 0;  /* fail */
    }
  }
}


فراغ luaO_arith (lua_State *L, صحيح op, ثابت TValue *p1, ثابت TValue *p2,
                 StkId res) {
  אם (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (صحيح c) {
  lua_assert(lisxdigit(c));
  אם (lisdigit(c)) החזר cast_byte(c - '0');
  אחרת החזר cast_byte((ltolower(c) - 'a') + 10);
}


ساكن صحيح isneg (ثابت حرف **s) {
  אם (**s == '-') { (*s)++; החזר 1; }
  אחרת אם (**s == '+') (*s)++;
  החזר 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#אם !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ساكن lua_Number lua_strx2number (ثابت حرف *s, حرف **endptr) {
  صحيح dot = lua_getlocaledecpoint();
  lua_Number r = l_mathop(0.0);  /* result (accumulator) */
  صحيح sigdig = 0;  /* number of significant digits */
  صحيح nosigdig = 0;  /* number of non-significant digits */
  صحيح e = 0;  /* exponent correction */
  صحيح neg;  /* 1 if number is negative */
  صحيح hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  כלעוד (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  אם (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    החזר l_mathop(0.0);  /* invalid format (no '0x') */
  עבור (s += 2; ; s++) {  /* skip '0x' and read numeral */
    אם (*s == dot) {
      אם (hasdot) اكسر;  /* second dot? stop loop */
      אחרת hasdot = 1;
    }
    אחרת אם (lisxdigit(cast_uchar(*s))) {
      אם (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      אחרת אם (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      אחרת e++;  /* too many digits; ignore, but still count for exponent */
      אם (hasdot) e--;  /* decimal digit? correct exponent */
    }
    אחרת اكسر;  /* neither a dot nor a digit */
  }
  אם (nosigdig + sigdig == 0)  /* no digits? */
    החזר l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  אם (*s == 'p' || *s == 'P') {  /* exponent part? */
    صحيح exp1 = 0;  /* exponent value */
    صحيح neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    אם (!lisdigit(cast_uchar(*s)))
      החזר l_mathop(0.0);  /* invalid; must have at least one digit */
    כלעוד (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    אם (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  אם (neg) r = -r;
  החזר l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#אם !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ساكن ثابت حرف *l_str2dloc (ثابت حرف *s, lua_Number *result, صحيح mode) {
  حرف *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  אם (endptr == s) החזר عدم;  /* nothing recognized? */
  כלעוד (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  החזר (*endptr == '\0') ? endptr : عدم;  /* OK iff no trailing chars */
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
ساكن ثابت حرف *l_str2d (ثابت حرف *s, lua_Number *result) {
  ثابت حرف *endptr;
  ثابت حرف *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  صحيح mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  אם (mode == 'n')  /* reject 'inf' and 'nan' */
    החזר عدم;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  אם (endptr == عدم) {  /* failed? may be a different locale */
    حرف buff[L_MAXLENNUM + 1];
    ثابت حرف *pdot = strchr(s, '.');
    אם (pdot == عدم || strlen(s) > L_MAXLENNUM)
      החזר عدم;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    אם (endptr != عدم)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  החזר endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ساكن ثابت حرف *l_str2int (ثابت حرف *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  صحيح empty = 1;
  صحيح neg;
  כלעוד (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  אם (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    עבור (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  אחרת {  /* decimal */
    עבור (; lisdigit(cast_uchar(*s)); s++) {
      صحيح d = *s - '0';
      אם (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        החזר عدم;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  כלעוד (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  אם (empty || *s != '\0') החזר عدم;  /* something wrong in the numeral */
  אחרת {
    *result = l_castU2S((neg) ? 0u - a : a);
    החזר s;
  }
}


size_t luaO_str2num (ثابت حرف *s, TValue *o) {
  lua_Integer i; lua_Number n;
  ثابت حرف *e;
  אם ((e = l_str2int(s, &i)) != عدم) {  /* try as an integer */
    setivalue(o, i);
  }
  אחרת אם ((e = l_str2d(s, &n)) != عدم) {  /* else try as a float */
    setfltvalue(o, n);
  }
  אחרת
    החזר 0;  /* conversion failed */
  החזר ct_diff2sz(e - s) + 1;  /* success; return string size */
}


صحيح luaO_utf8esc (حرف *buff, l_uint32 x) {
  صحيح n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  אם (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  אחרת {  /* need continuation bytes */
    unsigned صحيح mfb = 0x3f;  /* maximum that fits in first byte */
    עשה {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } כלעוד (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  החזר n;
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
#אם LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
ساكن صحيح tostringbuffFloat (lua_Number n, حرف *buff) {
  /* first conversion */
  صحيح len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT,
                            (LUAI_UACNUMBER)n);
  lua_Number check = lua_str2number(buff, عدم);  /* read it back */
  אם (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  אם (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  החזר len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ثابت TValue *obj, حرف *buff) {
  صحيح len;
  lua_assert(ttisnumber(obj));
  אם (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  אחרת
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  החזר cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
فراغ luaO_tostring (lua_State *L, TValue *obj) {
  حرف buff[LUA_N2SBUFFSZ];
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
تعريفنوع بنية BuffFS {
  lua_State *L;
  حرف *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  صحيح err;
  حرف space[BUFVFS];  /* initial buffer */
} BuffFS;


ساكن فراغ initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = حجم(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ساكن فراغ pushbuff (lua_State *L, فراغ *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  بدل (buff->err) {
    حالة 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      اكسر;
    حالة 2:  /* length overflow: Add "..." at the end of result */
      אם (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      אחרת {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    افتراضي: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->top.p, ts);
      L->top.p++;
    }
  }
}


ساكن ثابت حرف *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ثابت حرف *res;
  אם (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = عدم;  /* error message is on the top of the stack */
  אחרת
    res = getstr(tsvalue(s2v(L->top.p - 1)));
  אם (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  החזר res;
}


ساكن فراغ addstr2buff (BuffFS *buff, ثابت حرف *str, size_t slen) {
  size_t left = buff->buffsize - buff->blen;  /* space left in the buffer */
  אם (buff->err)  /* do nothing else after an error */
    החזר;
  אם (slen > left) {  /* new string doesn't fit into current buffer? */
    אם (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, left);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      החזר;
    }
    אחרת {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      حرف *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, عدم, 0, newsize, حرف)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               حرف);
      אם (newb == عدم) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        החזר;
      }
      אם (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
ساكن فراغ addnum2buff (BuffFS *buff, TValue *num) {
  حرف numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(num, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ثابت حرف *luaO_pushvfstring (lua_State *L, ثابت حرف *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ثابت حرف *e;  /* points to next '%' */
  initbuff(L, &buff);
  כלעוד ((e = strchr(fmt, '%')) != عدم) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    بدل (*(e + 1)) {  /* conversion specifier */
      حالة 's': {  /* zero-terminated string */
        ثابت حرف *s = va_arg(argp, حرف *);
        אם (s == عدم) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        اكسر;
      }
      حالة 'c': {  /* an 'int' as a character */
        حرف c = cast_char(va_arg(argp, صحيح));
        addstr2buff(&buff, &c, حجم(حرف));
        اكسر;
      }
      حالة 'd': {  /* an 'int' */
        TValue num;
        setivalue(&num, va_arg(argp, صحيح));
        addnum2buff(&buff, &num);
        اكسر;
      }
      حالة 'I': {  /* a 'lua_Integer' */
        TValue num;
        setivalue(&num, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &num);
        اكسر;
      }
      حالة 'f': {  /* a 'lua_Number' */
        TValue num;
        setfltvalue(&num, cast_num(va_arg(argp, l_uacNumber)));
        addnum2buff(&buff, &num);
        اكسر;
      }
      حالة 'p': {  /* a pointer */
        حرف bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        فراغ *p = va_arg(argp, فراغ *);
        صحيح len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        اكسر;
      }
      حالة 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        حرف bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        صحيح len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        اكسر;
      }
      حالة '%': {
        addstr2buff(&buff, "%", 1);
        اكسر;
      }
      افتراضي: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        اكسر;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  החזר clearbuff(&buff);  /* empty buffer into a new string */
}


ثابت حرف *luaO_pushfstring (lua_State *L, ثابت حرف *fmt, ...) {
  ثابت حرف *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  אם (msg == عدم)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  החזר msg;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * حجم(حرف)), a += (l) )

فراغ luaO_chunkid (حرف *out, ثابت حرف *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  אם (*source == '=') {  /* 'literal' source */
    אם (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * حجم(حرف));
    אחרת {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  אחרת אם (*source == '@') {  /* file name */
    אם (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * حجم(حرف));
    אחרת {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * حجم(حرف));
    }
  }
  אחרת {  /* string; format as [string "source"] */
    ثابت حرف *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    אם (srclen < bufflen && nl == عدم) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    אחרת {
      אם (nl != عدم)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      אם (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * حجم(حرف));
  }
}

