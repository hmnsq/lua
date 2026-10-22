/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#تعریف_کرو lobject_c
#تعریف_کرو LUA_CORE

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
lu_byte luaO_ceillog2 (بنانشانی صحيح x) {
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
  طالما (x >= 256) { l += 8; x >>= 8; }
  أرجع cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (بنانشانی صحيح p) {
  إذا (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    أرجع 0xFF;  /* return maximum value */
  وإلا {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    إذا (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      أرجع cast_byte(p);
    }
    وإلا {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      بنانشانی لوگارتھم = luaO_ceillog2(p + 1) - 5u;
      أرجع cast_byte(((p >> لوگارتھم) - 0x10) | ((لوگارتھم + 1) << 4));
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
  إذا (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  إذا (e >= 0) {
    إذا (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      أرجع (x * m) << e;  /* order doesn't matter here */
    وإلا  /* real overflow */
      أرجع MAX_LMEM;
  }
  وإلا {  /* negative exponent */
    e = -e;
    إذا (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      أرجع (x * m) >> e;  /* multiplying first gives more precision */
    وإلا إذا ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      أرجع (x >> e) * m;
    وإلا  /* real overflow */
      أرجع MAX_LMEM;
  }
}


ساكن lua_Integer intarith (lua_State *L, صحيح op, lua_Integer v1,
                                                   lua_Integer v2) {
  بدل (op) {
    حالة LUA_OPADD: أرجع intop(+, v1, v2);
    حالة LUA_OPSUB:أرجع intop(-, v1, v2);
    حالة LUA_OPMUL:أرجع intop(*, v1, v2);
    حالة LUA_OPMOD: أرجع luaV_mod(L, v1, v2);
    حالة LUA_OPIDIV: أرجع luaV_idiv(L, v1, v2);
    حالة LUA_OPBAND: أرجع intop(&, v1, v2);
    حالة LUA_OPBOR: أرجع intop(|, v1, v2);
    حالة LUA_OPBXOR: أرجع intop(^, v1, v2);
    حالة LUA_OPSHL: أرجع luaV_shiftl(v1, v2);
    حالة LUA_OPSHR: أرجع luaV_shiftr(v1, v2);
    حالة LUA_OPUNM: أرجع intop(-, 0, v1);
    حالة LUA_OPBNOT: أرجع intop(^, ~l_castS2U(0), v1);
    افتراضي: lua_assert(0); أرجع 0;
  }
}


ساكن lua_نمبر numarith (lua_State *L, صحيح op, lua_نمبر v1,
                                                  lua_نمبر v2) {
  بدل (op) {
    حالة LUA_OPADD: أرجع luai_numadd(L, v1, v2);
    حالة LUA_OPSUB: أرجع luai_numsub(L, v1, v2);
    حالة LUA_OPMUL: أرجع luai_nummul(L, v1, v2);
    حالة LUA_OPDIV: أرجع luai_numdiv(L, v1, v2);
    حالة LUA_OPPOW: أرجع luai_numpow(L, v1, v2);
    حالة LUA_OPIDIV: أرجع luai_numidiv(L, v1, v2);
    حالة LUA_OPUNM: أرجع luai_numunm(L, v1);
    حالة LUA_OPMOD: أرجع luaV_modf(L, v1, v2);
    افتراضي: lua_assert(0); أرجع 0;
  }
}


صحيح luaO_rawarith (lua_State *L, صحيح op, ثابت TValue *p1, ثابت TValue *p2,
                   TValue *res) {
  بدل (op) {
    حالة LUA_OPBAND: حالة LUA_OPBOR: حالة LUA_OPBXOR:
    حالة LUA_OPSHL: حالة LUA_OPSHR:
    حالة LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      إذا (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        أرجع 1;
      }
      وإلا أرجع 0;  /* fail */
    }
    حالة LUA_OPDIV: حالة LUA_OPPOW: {  /* operate only on floats */
      lua_نمبر n1; lua_نمبر n2;
      إذا (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        أرجع 1;
      }
      وإلا أرجع 0;  /* fail */
    }
    افتراضي: {  /* other operations */
      lua_نمبر n1; lua_نمبر n2;
      إذا (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        أرجع 1;
      }
      وإلا إذا (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        أرجع 1;
      }
      وإلا أرجع 0;  /* fail */
    }
  }
}


فراغ luaO_arith (lua_State *L, صحيح op, ثابت TValue *p1, ثابت TValue *p2,
                 StkId res) {
  إذا (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_جوڑو));
  }
}


lu_byte luaO_hexavalue (صحيح c) {
  lua_assert(lisxdigit(c));
  إذا (lisdigit(c)) أرجع cast_byte(c - '0');
  وإلا أرجع cast_byte((ltolower(c) - 'a') + 10);
}


ساكن صحيح isneg (ثابت حرف **s) {
  إذا (**s == '-') { (*s)++; أرجع 1; }
  وإلا إذا (**s == '+') (*s)++;
  أرجع 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#إذا !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#تعریف_کرو MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ساكن lua_نمبر lua_strx2number (ثابت حرف *s, حرف **endptr) {
  صحيح dot = lua_getlocaledecpoint();
  lua_نمبر r = l_mathop(0.0);  /* result (accumulator) */
  صحيح sigdig = 0;  /* number of significant digits */
  صحيح nosigdig = 0;  /* number of non-significant digits */
  صحيح e = 0;  /* exponent correction */
  صحيح neg;  /* 1 if number is negative */
  صحيح hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  طالما (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  إذا (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    أرجع l_mathop(0.0);  /* invalid format (no '0x') */
  لكل (s += 2; ; s++) {  /* skip '0x' and read numeral */
    إذا (*s == dot) {
      إذا (hasdot) اكسر;  /* second dot? stop loop */
      وإلا hasdot = 1;
    }
    وإلا إذا (lisxdigit(cast_uchar(*s))) {
      إذا (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      وإلا إذا (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      وإلا e++;  /* too many digits; ignore, but still count for exponent */
      إذا (hasdot) e--;  /* decimal digit? correct exponent */
    }
    وإلا اكسر;  /* neither a dot nor a digit */
  }
  إذا (nosigdig + sigdig == 0)  /* no digits? */
    أرجع l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  إذا (*s == 'p' || *s == 'P') {  /* exponent part? */
    صحيح exp1 = 0;  /* exponent value */
    صحيح neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    إذا (!lisdigit(cast_uchar(*s)))
      أرجع l_mathop(0.0);  /* invalid; must have at least one digit */
    طالما (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    إذا (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  إذا (neg) r = -r;
  أرجع l_mathop(ldexp)(r, e);
}

#جے_انت
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#إذا !defined (L_MAXLENNUM)
#تعریف_کرو L_MAXLENNUM	200
#جے_انت

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ساكن ثابت حرف *l_str2dloc (ثابت حرف *s, lua_نمبر *نتیجہ, صحيح منوال) {
  حرف *endptr;
  *نتیجہ = (منوال == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  إذا (endptr == s) أرجع عدم;  /* nothing recognized? */
  طالما (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  أرجع (*endptr == '\0') ? endptr : عدم;  /* OK iff no trailing chars */
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
ساكن ثابت حرف *l_str2d (ثابت حرف *s, lua_نمبر *نتیجہ) {
  ثابت حرف *endptr;
  ثابت حرف *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  صحيح منوال = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  إذا (منوال == 'n')  /* reject 'inf' and 'nan' */
    أرجع عدم;
  endptr = l_str2dloc(s, نتیجہ, منوال);  /* try to convert */
  إذا (endptr == عدم) {  /* failed? may be a different locale */
    حرف buff[L_MAXLENNUM + 1];
    ثابت حرف *pdot = strchr(s, '.');
    إذا (pdot == عدم || لڑی_لمبائی(s) > L_MAXLENNUM)
      أرجع عدم;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, نتیجہ, منوال);  /* try again */
    إذا (endptr != عدم)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  أرجع endptr;
}


#تعریف_کرو MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#تعریف_کرو MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ساكن ثابت حرف *l_str2int (ثابت حرف *s, lua_Integer *نتیجہ) {
  lua_Unsigned a = 0;
  صحيح خالی_تھاواں = 1;
  صحيح neg;
  طالما (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  إذا (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    لكل (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      خالی_تھاواں = 0;
    }
  }
  وإلا {  /* decimal */
    لكل (; lisdigit(cast_uchar(*s)); s++) {
      صحيح d = *s - '0';
      إذا (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        أرجع عدم;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      خالی_تھاواں = 0;
    }
  }
  طالما (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  إذا (خالی_تھاواں || *s != '\0') أرجع عدم;  /* something wrong in the numeral */
  وإلا {
    *نتیجہ = l_castU2S((neg) ? 0u - a : a);
    أرجع s;
  }
}


size_t luaO_str2num (ثابت حرف *s, TValue *o) {
  lua_Integer i; lua_نمبر n;
  ثابت حرف *e;
  إذا ((e = l_str2int(s, &i)) != عدم) {  /* try as an integer */
    setivalue(o, i);
  }
  وإلا إذا ((e = l_str2d(s, &n)) != عدم) {  /* else try as a float */
    setfltvalue(o, n);
  }
  وإلا
    أرجع 0;  /* conversion failed */
  أرجع ct_diff2sz(e - s) + 1;  /* success; return string size */
}


صحيح luaO_utf8esc (حرف *buff, l_uint32 x) {
  صحيح n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  إذا (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  وإلا {  /* need continuation bytes */
    بنانشانی صحيح mfb = 0x3f;  /* maximum that fits in first byte */
    افعل {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } طالما (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  أرجع n;
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
#إذا LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#غلطی "invalid value for LUA_N2SBUFFSZ"
#جے_انت


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
ساكن صحيح tostringbuffFloat (lua_نمبر n, حرف *buff) {
  /* first conversion */
  صحيح len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_نمبر_FMT,
                            (LUAI_UACNUMBER)n);
  lua_نمبر check = lua_str2number(buff, عدم);  /* read it back */
  إذا (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_نمبر_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  إذا (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  أرجع len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
بنانشانی luaO_tostringbuff (ثابت TValue *obj, حرف *buff) {
  صحيح len;
  lua_assert(ttisnumber(obj));
  إذا (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  وإلا
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  أرجع cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
فراغ luaO_tostring (lua_State *L, TValue *obj) {
  حرف buff[LUA_N2SBUFFSZ];
  بنانشانی len = luaO_tostringbuff(obj, buff);
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
#تعریف_کرو BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

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
      إذا (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      وإلا {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    افتراضي: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->چوٹی.p, ts);
      L->چوٹی.p++;
    }
  }
}


ساكن ثابت حرف *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ثابت حرف *res;
  إذا (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = عدم;  /* error message is on the top of the stack */
  وإلا
    res = getstr(tsvalue(s2v(L->چوٹی.p - 1)));
  إذا (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  أرجع res;
}


ساكن فراغ addstr2buff (BuffFS *buff, ثابت حرف *str, size_t slen) {
  size_t بایاں = buff->buffsize - buff->blen;  /* space left in the buffer */
  إذا (buff->err)  /* do nothing else after an error */
    أرجع;
  إذا (slen > بایاں) {  /* new string doesn't fit into current buffer? */
    إذا (slen > ((MAX_ناپ/2) - buff->blen)) {  /* overflow? */
      حافظہ_نقل(buff->b + buff->blen, str, بایاں);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      أرجع;
    }
    وإلا {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      حرف *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, عدم, 0, newsize, حرف)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               حرف);
      إذا (newb == عدم) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        أرجع;
      }
      إذا (buff->b == buff->space)  /* new buffer (not reallocated)? */
        حافظہ_نقل(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  حافظہ_نقل(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
ساكن فراغ addnum2buff (BuffFS *buff, TValue *عدد) {
  حرف numbuff[LUA_N2SBUFFSZ];
  بنانشانی len = luaO_tostringbuff(عدد, numbuff);
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
  طالما ((e = strchr(fmt, '%')) != عدم) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    بدل (*(e + 1)) {  /* conversion specifier */
      حالة 's': {  /* zero-terminated string */
        ثابت حرف *s = va_arg(argp, حرف *);
        إذا (s == عدم) s = "(null)";
        addstr2buff(&buff, s, لڑی_لمبائی(s));
        اكسر;
      }
      حالة 'c': {  /* an 'int' as a character */
        حرف c = cast_char(va_arg(argp, صحيح));
        addstr2buff(&buff, &c, حجم(حرف));
        اكسر;
      }
      حالة 'd': {  /* an 'int' */
        TValue عدد;
        setivalue(&عدد, va_arg(argp, صحيح));
        addnum2buff(&buff, &عدد);
        اكسر;
      }
      حالة 'I': {  /* a 'lua_Integer' */
        TValue عدد;
        setivalue(&عدد, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &عدد);
        اكسر;
      }
      حالة 'f': {  /* a 'lua_Number' */
        TValue عدد;
        setfltvalue(&عدد, cast_عدد(va_arg(argp, l_uac_نمبر)));
        addnum2buff(&buff, &عدد);
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
        بنانشانی لما arg = va_arg(argp, بنانشانی لما);
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
  addstr2buff(&buff, fmt, لڑی_لمبائی(fmt));  /* rest of 'fmt' */
  أرجع clearbuff(&buff);  /* empty buffer into a new string */
}


ثابت حرف *luaO_pushfstring (lua_State *L, ثابت حرف *fmt, ...) {
  ثابت حرف *پیغام;
  va_list argp;
  va_start(argp, fmt);
  پیغام = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  إذا (پیغام == عدم)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  أرجع پیغام;
}

/* }================================================================== */


#تعریف_کرو RETS	"..."
#تعریف_کرو PRE	"[string \""
#تعریف_کرو POS	"\"]"

#تعریف_کرو addstr(a,b,l)	( حافظہ_نقل(a,b,(l) * حجم(حرف)), a += (l) )

فراغ luaO_chunkid (حرف *باہرلا, ثابت حرف *سروت, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  إذا (*سروت == '=') {  /* 'literal' source */
    إذا (srclen <= bufflen)  /* small enough? */
      حافظہ_نقل(باہرلا, سروت + 1, srclen * حجم(حرف));
    وإلا {  /* truncate it */
      addstr(باہرلا, سروت + 1, bufflen - 1);
      *باہرلا = '\0';
    }
  }
  وإلا إذا (*سروت == '@') {  /* file name */
    إذا (srclen <= bufflen)  /* small enough? */
      حافظہ_نقل(باہرلا, سروت + 1, srclen * حجم(حرف));
    وإلا {  /* add '...' before rest of name */
      addstr(باہرلا, RETS, LL(RETS));
      bufflen -= LL(RETS);
      حافظہ_نقل(باہرلا, سروت + 1 + srclen - bufflen, bufflen * حجم(حرف));
    }
  }
  وإلا {  /* string; format as [string "source"] */
    ثابت حرف *nl = strchr(سروت, '\n');  /* find first new line (if any) */
    addstr(باہرلا, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    إذا (srclen < bufflen && nl == عدم) {  /* small one-line source? */
      addstr(باہرلا, سروت, srclen);  /* keep it */
    }
    وإلا {
      إذا (nl != عدم)
        srclen = ct_diff2sz(nl - سروت);  /* stop at first newline */
      إذا (srclen > bufflen) srclen = bufflen;
      addstr(باہرلا, سروت, srclen);
      addstr(باہرلا, RETS, LL(RETS));
    }
    حافظہ_نقل(باہرلا, POS, (LL(POS) + 1) * حجم(حرف));
  }
}

