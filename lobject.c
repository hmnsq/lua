/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#تعریف_کرو lobject_c
#تعریف_کرو LUA_CORE

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
lu_byte luaO_ceillog2 (غیرعلامتی صحیح x) {
  ساکن ثابت lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
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
  جب (x >= 256) { l += 8; x >>= 8; }
  واپس cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (غیرعلامتی صحیح p) {
  اگر (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    واپس 0xFF;  /* return maximum value */
  ورنہ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    اگر (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      واپس cast_byte(p);
    }
    ورنہ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      غیرعلامتی لوگارتھم = luaO_ceillog2(p + 1) - 5u;
      واپس cast_byte(((p >> لوگارتھم) - 0x10) | ((لوگارتھم + 1) << 4));
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
      واپس (x * m) << e;  /* order doesn't matter here */
    ورنہ  /* real overflow */
      واپس MAX_LMEM;
  }
  ورنہ {  /* negative exponent */
    e = -e;
    اگر (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      واپس (x * m) >> e;  /* multiplying first gives more precision */
    ورنہ اگر ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      واپس (x >> e) * m;
    ورنہ  /* real overflow */
      واپس MAX_LMEM;
  }
}


ساکن lua_Integer intarith (lua_State *L, صحیح op, lua_Integer v1,
                                                   lua_Integer v2) {
  انتخاب (op) {
    حالت LUA_OPADD: واپس intop(+, v1, v2);
    حالت LUA_OPSUB:واپس intop(-, v1, v2);
    حالت LUA_OPMUL:واپس intop(*, v1, v2);
    حالت LUA_OPMOD: واپس luaV_mod(L, v1, v2);
    حالت LUA_OPIDIV: واپس luaV_idiv(L, v1, v2);
    حالت LUA_OPBAND: واپس intop(&, v1, v2);
    حالت LUA_OPBOR: واپس intop(|, v1, v2);
    حالت LUA_OPBXOR: واپس intop(^, v1, v2);
    حالت LUA_OPSHL: واپس luaV_shiftl(v1, v2);
    حالت LUA_OPSHR: واپس luaV_shiftr(v1, v2);
    حالت LUA_OPUNM: واپس intop(-, 0, v1);
    حالت LUA_OPBNOT: واپس intop(^, ~l_castS2U(0), v1);
    طےشدہ: lua_assert(0); واپس 0;
  }
}


ساکن lua_نمبر numarith (lua_State *L, صحیح op, lua_نمبر v1,
                                                  lua_نمبر v2) {
  انتخاب (op) {
    حالت LUA_OPADD: واپس luai_numadd(L, v1, v2);
    حالت LUA_OPSUB: واپس luai_numsub(L, v1, v2);
    حالت LUA_OPMUL: واپس luai_nummul(L, v1, v2);
    حالت LUA_OPDIV: واپس luai_numdiv(L, v1, v2);
    حالت LUA_OPPOW: واپس luai_numpow(L, v1, v2);
    حالت LUA_OPIDIV: واپس luai_numidiv(L, v1, v2);
    حالت LUA_OPUNM: واپس luai_numunm(L, v1);
    حالت LUA_OPMOD: واپس luaV_modf(L, v1, v2);
    طےشدہ: lua_assert(0); واپس 0;
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
        واپس 1;
      }
      ورنہ واپس 0;  /* fail */
    }
    حالت LUA_OPDIV: حالت LUA_OPPOW: {  /* operate only on floats */
      lua_نمبر n1; lua_نمبر n2;
      اگر (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        واپس 1;
      }
      ورنہ واپس 0;  /* fail */
    }
    طےشدہ: {  /* other operations */
      lua_نمبر n1; lua_نمبر n2;
      اگر (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        واپس 1;
      }
      ورنہ اگر (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        واپس 1;
      }
      ورنہ واپس 0;  /* fail */
    }
  }
}


خالی luaO_arith (lua_State *L, صحیح op, ثابت TValue *p1, ثابت TValue *p2,
                 StkId res) {
  اگر (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_جوڑو));
  }
}


lu_byte luaO_hexavalue (صحیح c) {
  lua_assert(lisxdigit(c));
  اگر (lisdigit(c)) واپس cast_byte(c - '0');
  ورنہ واپس cast_byte((ltolower(c) - 'a') + 10);
}


ساکن صحیح isneg (ثابت حرف **s) {
  اگر (**s == '-') { (*s)++; واپس 1; }
  ورنہ اگر (**s == '+') (*s)++;
  واپس 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#اگر !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#تعریف_کرو MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ساکن lua_نمبر lua_strx2number (ثابت حرف *s, حرف **endptr) {
  صحیح dot = lua_getlocaledecpoint();
  lua_نمبر r = l_mathop(0.0);  /* result (accumulator) */
  صحیح sigdig = 0;  /* number of significant digits */
  صحیح nosigdig = 0;  /* number of non-significant digits */
  صحیح e = 0;  /* exponent correction */
  صحیح neg;  /* 1 if number is negative */
  صحیح hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  جب (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  اگر (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    واپس l_mathop(0.0);  /* invalid format (no '0x') */
  برائے (s += 2; ; s++) {  /* skip '0x' and read numeral */
    اگر (*s == dot) {
      اگر (hasdot) توڑو;  /* second dot? stop loop */
      ورنہ hasdot = 1;
    }
    ورنہ اگر (lisxdigit(cast_uchar(*s))) {
      اگر (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      ورنہ اگر (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      ورنہ e++;  /* too many digits; ignore, but still count for exponent */
      اگر (hasdot) e--;  /* decimal digit? correct exponent */
    }
    ورنہ توڑو;  /* neither a dot nor a digit */
  }
  اگر (nosigdig + sigdig == 0)  /* no digits? */
    واپس l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  اگر (*s == 'p' || *s == 'P') {  /* exponent part? */
    صحیح exp1 = 0;  /* exponent value */
    صحیح neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    اگر (!lisdigit(cast_uchar(*s)))
      واپس l_mathop(0.0);  /* invalid; must have at least one digit */
    جب (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    اگر (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  اگر (neg) r = -r;
  واپس l_mathop(ldexp)(r, e);
}

#اگر_ختم
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#اگر !defined (L_MAXLENNUM)
#تعریف_کرو L_MAXLENNUM	200
#اگر_ختم

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ساکن ثابت حرف *l_str2dloc (ثابت حرف *s, lua_نمبر *نتیجہ, صحیح منوال) {
  حرف *endptr;
  *نتیجہ = (منوال == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  اگر (endptr == s) واپس تہی;  /* nothing recognized? */
  جب (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  واپس (*endptr == '\0') ? endptr : تہی;  /* OK iff no trailing chars */
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
ساکن ثابت حرف *l_str2d (ثابت حرف *s, lua_نمبر *نتیجہ) {
  ثابت حرف *endptr;
  ثابت حرف *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  صحیح منوال = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  اگر (منوال == 'n')  /* reject 'inf' and 'nan' */
    واپس تہی;
  endptr = l_str2dloc(s, نتیجہ, منوال);  /* try to convert */
  اگر (endptr == تہی) {  /* failed? may be a different locale */
    حرف buff[L_MAXLENNUM + 1];
    ثابت حرف *pdot = strchr(s, '.');
    اگر (pdot == تہی || لڑی_لمبائی(s) > L_MAXLENNUM)
      واپس تہی;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, نتیجہ, منوال);  /* try again */
    اگر (endptr != تہی)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  واپس endptr;
}


#تعریف_کرو MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#تعریف_کرو MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ساکن ثابت حرف *l_str2int (ثابت حرف *s, lua_Integer *نتیجہ) {
  lua_Unsigned a = 0;
  صحیح خالی_خانے = 1;
  صحیح neg;
  جب (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  اگر (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    برائے (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      خالی_خانے = 0;
    }
  }
  ورنہ {  /* decimal */
    برائے (; lisdigit(cast_uchar(*s)); s++) {
      صحیح d = *s - '0';
      اگر (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        واپس تہی;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      خالی_خانے = 0;
    }
  }
  جب (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  اگر (خالی_خانے || *s != '\0') واپس تہی;  /* something wrong in the numeral */
  ورنہ {
    *نتیجہ = l_castU2S((neg) ? 0u - a : a);
    واپس s;
  }
}


size_t luaO_str2num (ثابت حرف *s, TValue *o) {
  lua_Integer i; lua_نمبر n;
  ثابت حرف *e;
  اگر ((e = l_str2int(s, &i)) != تہی) {  /* try as an integer */
    setivalue(o, i);
  }
  ورنہ اگر ((e = l_str2d(s, &n)) != تہی) {  /* else try as a float */
    setfltvalue(o, n);
  }
  ورنہ
    واپس 0;  /* conversion failed */
  واپس ct_diff2sz(e - s) + 1;  /* success; return string size */
}


صحیح luaO_utf8esc (حرف *buff, l_uint32 x) {
  صحیح n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  اگر (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  ورنہ {  /* need continuation bytes */
    غیرعلامتی صحیح mfb = 0x3f;  /* maximum that fits in first byte */
    کرو {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } جب (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  واپس n;
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
#غلطی "invalid value for LUA_N2SBUFFSZ"
#اگر_ختم


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
ساکن صحیح tostringbuffFloat (lua_نمبر n, حرف *buff) {
  /* first conversion */
  صحیح len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_نمبر_FMT,
                            (LUAI_UACNUMBER)n);
  lua_نمبر check = lua_str2number(buff, تہی);  /* read it back */
  اگر (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_نمبر_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  اگر (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  واپس len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
غیرعلامتی luaO_tostringbuff (ثابت TValue *obj, حرف *buff) {
  صحیح len;
  lua_assert(ttisnumber(obj));
  اگر (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  ورنہ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  واپس cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
خالی luaO_tostring (lua_State *L, TValue *obj) {
  حرف buff[LUA_N2SBUFFSZ];
  غیرعلامتی len = luaO_tostringbuff(obj, buff);
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
قسمتعریف ساخت BuffFS {
  lua_State *L;
  حرف *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  صحیح err;
  حرف space[BUFVFS];  /* initial buffer */
} BuffFS;


ساکن خالی initbuff (lua_State *L, BuffFS *buff) {
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
ساکن خالی pushbuff (lua_State *L, خالی *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  انتخاب (buff->err) {
    حالت 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      توڑو;
    حالت 2:  /* length overflow: Add "..." at the end of result */
      اگر (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      ورنہ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    طےشدہ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->چوٹی.p, ts);
      L->چوٹی.p++;
    }
  }
}


ساکن ثابت حرف *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ثابت حرف *res;
  اگر (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = تہی;  /* error message is on the top of the stack */
  ورنہ
    res = getstr(tsvalue(s2v(L->چوٹی.p - 1)));
  اگر (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  واپس res;
}


ساکن خالی addstr2buff (BuffFS *buff, ثابت حرف *str, size_t slen) {
  size_t بایاں = buff->buffsize - buff->blen;  /* space left in the buffer */
  اگر (buff->err)  /* do nothing else after an error */
    واپس;
  اگر (slen > بایاں) {  /* new string doesn't fit into current buffer? */
    اگر (slen > ((MAX_سائز/2) - buff->blen)) {  /* overflow? */
      حافظہ_نقل(buff->b + buff->blen, str, بایاں);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      واپس;
    }
    ورنہ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      حرف *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, تہی, 0, newsize, حرف)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               حرف);
      اگر (newb == تہی) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        واپس;
      }
      اگر (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
ساکن خالی addnum2buff (BuffFS *buff, TValue *عدد) {
  حرف numbuff[LUA_N2SBUFFSZ];
  غیرعلامتی len = luaO_tostringbuff(عدد, numbuff);
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
  جب ((e = strchr(fmt, '%')) != تہی) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    انتخاب (*(e + 1)) {  /* conversion specifier */
      حالت 's': {  /* zero-terminated string */
        ثابت حرف *s = va_arg(argp, حرف *);
        اگر (s == تہی) s = "(null)";
        addstr2buff(&buff, s, لڑی_لمبائی(s));
        توڑو;
      }
      حالت 'c': {  /* an 'int' as a character */
        حرف c = cast_char(va_arg(argp, صحیح));
        addstr2buff(&buff, &c, حجم(حرف));
        توڑو;
      }
      حالت 'd': {  /* an 'int' */
        TValue عدد;
        setivalue(&عدد, va_arg(argp, صحیح));
        addnum2buff(&buff, &عدد);
        توڑو;
      }
      حالت 'I': {  /* a 'lua_Integer' */
        TValue عدد;
        setivalue(&عدد, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &عدد);
        توڑو;
      }
      حالت 'f': {  /* a 'lua_Number' */
        TValue عدد;
        setfltvalue(&عدد, cast_عدد(va_arg(argp, l_uac_نمبر)));
        addnum2buff(&buff, &عدد);
        توڑو;
      }
      حالت 'p': {  /* a pointer */
        حرف bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        خالی *p = va_arg(argp, خالی *);
        صحیح len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        توڑو;
      }
      حالت 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        حرف bf[UTF8BUFFSZ];
        غیرعلامتی طویل arg = va_arg(argp, غیرعلامتی طویل);
        صحیح len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        توڑو;
      }
      حالت '%': {
        addstr2buff(&buff, "%", 1);
        توڑو;
      }
      طےشدہ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        توڑو;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, لڑی_لمبائی(fmt));  /* rest of 'fmt' */
  واپس clearbuff(&buff);  /* empty buffer into a new string */
}


ثابت حرف *luaO_pushfstring (lua_State *L, ثابت حرف *fmt, ...) {
  ثابت حرف *پیغام;
  va_list argp;
  va_start(argp, fmt);
  پیغام = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  اگر (پیغام == تہی)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  واپس پیغام;
}

/* }================================================================== */


#تعریف_کرو RETS	"..."
#تعریف_کرو PRE	"[string \""
#تعریف_کرو POS	"\"]"

#تعریف_کرو addstr(a,b,l)	( حافظہ_نقل(a,b,(l) * حجم(حرف)), a += (l) )

خالی luaO_chunkid (حرف *برآمد, ثابت حرف *ماخذ, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  اگر (*ماخذ == '=') {  /* 'literal' source */
    اگر (srclen <= bufflen)  /* small enough? */
      حافظہ_نقل(برآمد, ماخذ + 1, srclen * حجم(حرف));
    ورنہ {  /* truncate it */
      addstr(برآمد, ماخذ + 1, bufflen - 1);
      *برآمد = '\0';
    }
  }
  ورنہ اگر (*ماخذ == '@') {  /* file name */
    اگر (srclen <= bufflen)  /* small enough? */
      حافظہ_نقل(برآمد, ماخذ + 1, srclen * حجم(حرف));
    ورنہ {  /* add '...' before rest of name */
      addstr(برآمد, RETS, LL(RETS));
      bufflen -= LL(RETS);
      حافظہ_نقل(برآمد, ماخذ + 1 + srclen - bufflen, bufflen * حجم(حرف));
    }
  }
  ورنہ {  /* string; format as [string "source"] */
    ثابت حرف *nl = strchr(ماخذ, '\n');  /* find first new line (if any) */
    addstr(برآمد, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    اگر (srclen < bufflen && nl == تہی) {  /* small one-line source? */
      addstr(برآمد, ماخذ, srclen);  /* keep it */
    }
    ورنہ {
      اگر (nl != تہی)
        srclen = ct_diff2sz(nl - ماخذ);  /* stop at first newline */
      اگر (srclen > bufflen) srclen = bufflen;
      addstr(برآمد, ماخذ, srclen);
      addstr(برآمد, RETS, LL(RETS));
    }
    حافظہ_نقل(برآمد, POS, (LL(POS) + 1) * حجم(حرف));
  }
}

