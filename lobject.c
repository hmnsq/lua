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
lu_byte luaO_ceillog2 (अचिन्हित पूर्णांक x) {
  स्थिर स्थिरांक lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  पूर्णांक l = 0;
  x--;
  पर्यंत (x >= 256) { l += 8; x >>= 8; }
  परत cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (अचिन्हित पूर्णांक p) {
  जर (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    परत 0xFF;  /* return maximum value */
  अन्यथा {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    जर (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      परत cast_byte(p);
    }
    अन्यथा {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      अचिन्हित log = luaO_ceillog2(p + 1) - 5u;
      परत cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  पूर्णांक m = p & 0xF;  /* mantissa */
  पूर्णांक e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  जर (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  जर (e >= 0) {
    जर (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      परत (x * m) << e;  /* order doesn't matter here */
    अन्यथा  /* real overflow */
      परत MAX_LMEM;
  }
  अन्यथा {  /* negative exponent */
    e = -e;
    जर (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      परत (x * m) >> e;  /* multiplying first gives more precision */
    अन्यथा जर ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      परत (x >> e) * m;
    अन्यथा  /* real overflow */
      परत MAX_LMEM;
  }
}


स्थिर lua_Integer intarith (lua_State *L, पूर्णांक op, lua_Integer v1,
                                                   lua_Integer v2) {
  निवड (op) {
    स्थिती LUA_OPADD: परत intop(+, v1, v2);
    स्थिती LUA_OPSUB:परत intop(-, v1, v2);
    स्थिती LUA_OPMUL:परत intop(*, v1, v2);
    स्थिती LUA_OPMOD: परत luaV_mod(L, v1, v2);
    स्थिती LUA_OPIDIV: परत luaV_idiv(L, v1, v2);
    स्थिती LUA_OPBAND: परत intop(&, v1, v2);
    स्थिती LUA_OPBOR: परत intop(|, v1, v2);
    स्थिती LUA_OPBXOR: परत intop(^, v1, v2);
    स्थिती LUA_OPSHL: परत luaV_shiftl(v1, v2);
    स्थिती LUA_OPSHR: परत luaV_shiftr(v1, v2);
    स्थिती LUA_OPUNM: परत intop(-, 0, v1);
    स्थिती LUA_OPBNOT: परत intop(^, ~l_castS2U(0), v1);
    पूर्वनिर्धारित: lua_assert(0); परत 0;
  }
}


स्थिर lua_संख्या numarith (lua_State *L, पूर्णांक op, lua_संख्या v1,
                                                  lua_संख्या v2) {
  निवड (op) {
    स्थिती LUA_OPADD: परत luai_numadd(L, v1, v2);
    स्थिती LUA_OPSUB: परत luai_numsub(L, v1, v2);
    स्थिती LUA_OPMUL: परत luai_nummul(L, v1, v2);
    स्थिती LUA_OPDIV: परत luai_numdiv(L, v1, v2);
    स्थिती LUA_OPPOW: परत luai_numpow(L, v1, v2);
    स्थिती LUA_OPIDIV: परत luai_numidiv(L, v1, v2);
    स्थिती LUA_OPUNM: परत luai_numunm(L, v1);
    स्थिती LUA_OPMOD: परत luaV_modf(L, v1, v2);
    पूर्वनिर्धारित: lua_assert(0); परत 0;
  }
}


पूर्णांक luaO_rawarith (lua_State *L, पूर्णांक op, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                   TValue *res) {
  निवड (op) {
    स्थिती LUA_OPBAND: स्थिती LUA_OPBOR: स्थिती LUA_OPBXOR:
    स्थिती LUA_OPSHL: स्थिती LUA_OPSHR:
    स्थिती LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      जर (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        परत 1;
      }
      अन्यथा परत 0;  /* fail */
    }
    स्थिती LUA_OPDIV: स्थिती LUA_OPPOW: {  /* operate only on floats */
      lua_संख्या n1; lua_संख्या n2;
      जर (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        परत 1;
      }
      अन्यथा परत 0;  /* fail */
    }
    पूर्वनिर्धारित: {  /* other operations */
      lua_संख्या n1; lua_संख्या n2;
      जर (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        परत 1;
      }
      अन्यथा जर (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        परत 1;
      }
      अन्यथा परत 0;  /* fail */
    }
  }
}


शून्य luaO_arith (lua_State *L, पूर्णांक op, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                 StkId res) {
  जर (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (पूर्णांक c) {
  lua_assert(lisxdigit(c));
  जर (lisdigit(c)) परत cast_byte(c - '0');
  अन्यथा परत cast_byte((ltolower(c) - 'a') + 10);
}


स्थिर पूर्णांक isneg (स्थिरांक अक्षर **s) {
  जर (**s == '-') { (*s)++; परत 1; }
  अन्यथा जर (**s == '+') (*s)++;
  परत 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#जर !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
स्थिर lua_संख्या lua_strx2number (स्थिरांक अक्षर *s, अक्षर **endptr) {
  पूर्णांक dot = lua_getlocaledecpoint();
  lua_संख्या r = l_mathop(0.0);  /* result (accumulator) */
  पूर्णांक sigdig = 0;  /* number of significant digits */
  पूर्णांक nosigdig = 0;  /* number of non-significant digits */
  पूर्णांक e = 0;  /* exponent correction */
  पूर्णांक neg;  /* 1 if number is negative */
  पूर्णांक hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  पर्यंत (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  जर (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    परत l_mathop(0.0);  /* invalid format (no '0x') */
  साठी (s += 2; ; s++) {  /* skip '0x' and read numeral */
    जर (*s == dot) {
      जर (hasdot) थांबा;  /* second dot? stop loop */
      अन्यथा hasdot = 1;
    }
    अन्यथा जर (lisxdigit(cast_uchar(*s))) {
      जर (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      अन्यथा जर (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      अन्यथा e++;  /* too many digits; ignore, but still count for exponent */
      जर (hasdot) e--;  /* decimal digit? correct exponent */
    }
    अन्यथा थांबा;  /* neither a dot nor a digit */
  }
  जर (nosigdig + sigdig == 0)  /* no digits? */
    परत l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  जर (*s == 'p' || *s == 'P') {  /* exponent part? */
    पूर्णांक exp1 = 0;  /* exponent value */
    पूर्णांक neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    जर (!lisdigit(cast_uchar(*s)))
      परत l_mathop(0.0);  /* invalid; must have at least one digit */
    पर्यंत (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    जर (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  जर (neg) r = -r;
  परत l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#जर !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
स्थिर स्थिरांक अक्षर *l_str2dloc (स्थिरांक अक्षर *s, lua_संख्या *result, पूर्णांक mode) {
  अक्षर *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  जर (endptr == s) परत रिक्त;  /* nothing recognized? */
  पर्यंत (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  परत (*endptr == '\0') ? endptr : रिक्त;  /* OK iff no trailing chars */
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
स्थिर स्थिरांक अक्षर *l_str2d (स्थिरांक अक्षर *s, lua_संख्या *result) {
  स्थिरांक अक्षर *endptr;
  स्थिरांक अक्षर *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  पूर्णांक mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  जर (mode == 'n')  /* reject 'inf' and 'nan' */
    परत रिक्त;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  जर (endptr == रिक्त) {  /* failed? may be a different locale */
    अक्षर buff[L_MAXLENNUM + 1];
    स्थिरांक अक्षर *pdot = म_अक्षर(s, '.');
    जर (pdot == रिक्त || म_माप(s) > L_MAXLENNUM)
      परत रिक्त;  /* string too long or no dot; fail */
    म_नकल(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    जर (endptr != रिक्त)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  परत endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

स्थिर स्थिरांक अक्षर *l_str2int (स्थिरांक अक्षर *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  पूर्णांक empty = 1;
  पूर्णांक neg;
  पर्यंत (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  जर (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    साठी (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  अन्यथा {  /* decimal */
    साठी (; lisdigit(cast_uchar(*s)); s++) {
      पूर्णांक d = *s - '0';
      जर (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        परत रिक्त;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  पर्यंत (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  जर (empty || *s != '\0') परत रिक्त;  /* something wrong in the numeral */
  अन्यथा {
    *result = l_castU2S((neg) ? 0u - a : a);
    परत s;
  }
}


माप_प्रकार luaO_str2num (स्थिरांक अक्षर *s, TValue *o) {
  lua_Integer i; lua_संख्या n;
  स्थिरांक अक्षर *e;
  जर ((e = l_str2int(s, &i)) != रिक्त) {  /* try as an integer */
    setivalue(o, i);
  }
  अन्यथा जर ((e = l_str2d(s, &n)) != रिक्त) {  /* else try as a float */
    setfltvalue(o, n);
  }
  अन्यथा
    परत 0;  /* conversion failed */
  परत ct_diff2sz(e - s) + 1;  /* success; return string size */
}


पूर्णांक luaO_utf8esc (अक्षर *buff, l_uint32 x) {
  पूर्णांक n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  जर (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  अन्यथा {  /* need continuation bytes */
    अचिन्हित पूर्णांक mfb = 0x3f;  /* maximum that fits in first byte */
    करा {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } पर्यंत (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  परत n;
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
#जर LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
स्थिर पूर्णांक tostringbuffFloat (lua_संख्या n, अक्षर *buff) {
  /* first conversion */
  पूर्णांक len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_संख्या_FMT,
                            (LUAI_UACNUMBER)n);
  lua_संख्या check = lua_str2number(buff, रिक्त);  /* read it back */
  जर (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_संख्या_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  जर (buff[म_अखोज(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  परत len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
अचिन्हित luaO_tostringbuff (स्थिरांक TValue *obj, अक्षर *buff) {
  पूर्णांक len;
  lua_assert(ttisnumber(obj));
  जर (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  अन्यथा
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  परत cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
शून्य luaO_tostring (lua_State *L, TValue *obj) {
  अक्षर buff[LUA_N2SBUFFSZ];
  अचिन्हित len = luaO_tostringbuff(obj, buff);
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
प्रकारव्याख्या संरचना BuffFS {
  lua_State *L;
  अक्षर *b;
  माप_प्रकार buffsize;
  माप_प्रकार blen;  /* length of string in 'buff' */
  पूर्णांक err;
  अक्षर space[BUFVFS];  /* initial buffer */
} BuffFS;


स्थिर शून्य initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = आकार(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
स्थिर शून्य pushbuff (lua_State *L, शून्य *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  निवड (buff->err) {
    स्थिती 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      थांबा;
    स्थिती 2:  /* length overflow: Add "..." at the end of result */
      जर (buff->buffsize - buff->blen < 3)
        म_नकल(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      अन्यथा {  /* there is enough space left for the "..." */
        म_नकल(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    पूर्वनिर्धारित: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->शीर्ष.p, ts);
      L->शीर्ष.p++;
    }
  }
}


स्थिर स्थिरांक अक्षर *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  स्थिरांक अक्षर *res;
  जर (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = रिक्त;  /* error message is on the top of the stack */
  अन्यथा
    res = getstr(tsvalue(s2v(L->शीर्ष.p - 1)));
  जर (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  परत res;
}


स्थिर शून्य addstr2buff (BuffFS *buff, स्थिरांक अक्षर *str, माप_प्रकार slen) {
  माप_प्रकार डावा = buff->buffsize - buff->blen;  /* space left in the buffer */
  जर (buff->err)  /* do nothing else after an error */
    परत;
  जर (slen > डावा) {  /* new string doesn't fit into current buffer? */
    जर (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      स_नकल(buff->b + buff->blen, str, डावा);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      परत;
    }
    अन्यथा {
      माप_प्रकार newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      अक्षर *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, रिक्त, 0, newsize, अक्षर)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               अक्षर);
      जर (newb == रिक्त) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        परत;
      }
      जर (buff->b == buff->space)  /* new buffer (not reallocated)? */
        स_नकल(newb, buff->b, buff->blen);  /* copy previous content */
      buff->b = newb;  /* set new (larger) buffer... */
      buff->buffsize = newsize;  /* ...and its new size */
    }
  }
  स_नकल(buff->b + buff->blen, str, slen);  /* copy new content */
  buff->blen += slen;
}


/*
** Add a numeral to the buffer.
*/
स्थिर शून्य addnum2buff (BuffFS *buff, TValue *अंक) {
  अक्षर numbuff[LUA_N2SBUFFSZ];
  अचिन्हित len = luaO_tostringbuff(अंक, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
स्थिरांक अक्षर *luaO_pushvfstring (lua_State *L, स्थिरांक अक्षर *fmt, बहु_सूची argp) {
  BuffFS buff;  /* holds last part of the result */
  स्थिरांक अक्षर *e;  /* points to next '%' */
  initbuff(L, &buff);
  पर्यंत ((e = म_अक्षर(fmt, '%')) != रिक्त) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    निवड (*(e + 1)) {  /* conversion specifier */
      स्थिती 's': {  /* zero-terminated string */
        स्थिरांक अक्षर *s = बहु_तर्क(argp, अक्षर *);
        जर (s == रिक्त) s = "(null)";
        addstr2buff(&buff, s, म_माप(s));
        थांबा;
      }
      स्थिती 'c': {  /* an 'int' as a character */
        अक्षर c = cast_char(बहु_तर्क(argp, पूर्णांक));
        addstr2buff(&buff, &c, आकार(अक्षर));
        थांबा;
      }
      स्थिती 'd': {  /* an 'int' */
        TValue अंक;
        setivalue(&अंक, बहु_तर्क(argp, पूर्णांक));
        addnum2buff(&buff, &अंक);
        थांबा;
      }
      स्थिती 'I': {  /* a 'lua_Integer' */
        TValue अंक;
        setivalue(&अंक, cast_Integer(बहु_तर्क(argp, l_uacInt)));
        addnum2buff(&buff, &अंक);
        थांबा;
      }
      स्थिती 'f': {  /* a 'lua_Number' */
        TValue अंक;
        setfltvalue(&अंक, cast_अंक(बहु_तर्क(argp, l_uac_संख्या)));
        addnum2buff(&buff, &अंक);
        थांबा;
      }
      स्थिती 'p': {  /* a pointer */
        अक्षर bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        शून्य *p = बहु_तर्क(argp, शून्य *);
        पूर्णांक len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        थांबा;
      }
      स्थिती 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        अक्षर bf[UTF8BUFFSZ];
        अचिन्हित दीर्घ arg = बहु_तर्क(argp, अचिन्हित दीर्घ);
        पूर्णांक len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        थांबा;
      }
      स्थिती '%': {
        addstr2buff(&buff, "%", 1);
        थांबा;
      }
      पूर्वनिर्धारित: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        थांबा;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, म_माप(fmt));  /* rest of 'fmt' */
  परत clearbuff(&buff);  /* empty buffer into a new string */
}


स्थिरांक अक्षर *luaO_pushfstring (lua_State *L, स्थिरांक अक्षर *fmt, ...) {
  स्थिरांक अक्षर *संदेश;
  बहु_सूची argp;
  बहु_शुरू(argp, fmt);
  संदेश = luaO_pushvfstring(L, fmt, argp);
  बहु_पूर्ण(argp);
  जर (संदेश == रिक्त)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  परत संदेश;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( स_नकल(a,b,(l) * आकार(अक्षर)), a += (l) )

शून्य luaO_chunkid (अक्षर *out, स्थिरांक अक्षर *source, माप_प्रकार srclen) {
  माप_प्रकार bufflen = LUA_IDSIZE;  /* free space in buffer */
  जर (*source == '=') {  /* 'literal' source */
    जर (srclen <= bufflen)  /* small enough? */
      स_नकल(out, source + 1, srclen * आकार(अक्षर));
    अन्यथा {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  अन्यथा जर (*source == '@') {  /* file name */
    जर (srclen <= bufflen)  /* small enough? */
      स_नकल(out, source + 1, srclen * आकार(अक्षर));
    अन्यथा {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      स_नकल(out, source + 1 + srclen - bufflen, bufflen * आकार(अक्षर));
    }
  }
  अन्यथा {  /* string; format as [string "source"] */
    स्थिरांक अक्षर *nl = म_अक्षर(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    जर (srclen < bufflen && nl == रिक्त) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    अन्यथा {
      जर (nl != रिक्त)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      जर (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    स_नकल(out, POS, (LL(POS) + 1) * आकार(अक्षर));
  }
}

