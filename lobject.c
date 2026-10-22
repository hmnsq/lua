/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#घोषणा lobject_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <float.h>
#समावेश <locale.h>
#समावेश <math.h>
#समावेश <stdarg.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lctype.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (अचिन्हित पूर्णांक x) {
  अटल स्थिर lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
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
  जबतक (x >= 256) { l += 8; x >>= 8; }
  वापस cast_byte(l + log_2[x]);
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
  अगर (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    वापस 0xFF;  /* return maximum value */
  अन्यथा {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    अगर (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      वापस cast_byte(p);
    }
    अन्यथा {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      अचिन्हित log = luaO_ceillog2(p + 1) - 5u;
      वापस cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  अगर (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  अगर (e >= 0) {
    अगर (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      वापस (x * m) << e;  /* order doesn't matter here */
    अन्यथा  /* real overflow */
      वापस MAX_LMEM;
  }
  अन्यथा {  /* negative exponent */
    e = -e;
    अगर (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      वापस (x * m) >> e;  /* multiplying first gives more precision */
    अन्यथा अगर ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      वापस (x >> e) * m;
    अन्यथा  /* real overflow */
      वापस MAX_LMEM;
  }
}


अटल lua_Integer intarith (lua_State *L, पूर्णांक op, lua_Integer v1,
                                                   lua_Integer v2) {
  चयन (op) {
    हाल LUA_OPADD: वापस intop(+, v1, v2);
    हाल LUA_OPSUB:वापस intop(-, v1, v2);
    हाल LUA_OPMUL:वापस intop(*, v1, v2);
    हाल LUA_OPMOD: वापस luaV_mod(L, v1, v2);
    हाल LUA_OPIDIV: वापस luaV_idiv(L, v1, v2);
    हाल LUA_OPBAND: वापस intop(&, v1, v2);
    हाल LUA_OPBOR: वापस intop(|, v1, v2);
    हाल LUA_OPBXOR: वापस intop(^, v1, v2);
    हाल LUA_OPSHL: वापस luaV_shiftl(v1, v2);
    हाल LUA_OPSHR: वापस luaV_shiftr(v1, v2);
    हाल LUA_OPUNM: वापस intop(-, 0, v1);
    हाल LUA_OPBNOT: वापस intop(^, ~l_castS2U(0), v1);
    शेष: lua_assert(0); वापस 0;
  }
}


अटल lua_संख्या numarith (lua_State *L, पूर्णांक op, lua_संख्या v1,
                                                  lua_संख्या v2) {
  चयन (op) {
    हाल LUA_OPADD: वापस luai_numadd(L, v1, v2);
    हाल LUA_OPSUB: वापस luai_numsub(L, v1, v2);
    हाल LUA_OPMUL: वापस luai_nummul(L, v1, v2);
    हाल LUA_OPDIV: वापस luai_numdiv(L, v1, v2);
    हाल LUA_OPPOW: वापस luai_numpow(L, v1, v2);
    हाल LUA_OPIDIV: वापस luai_numidiv(L, v1, v2);
    हाल LUA_OPUNM: वापस luai_numunm(L, v1);
    हाल LUA_OPMOD: वापस luaV_modf(L, v1, v2);
    शेष: lua_assert(0); वापस 0;
  }
}


पूर्णांक luaO_rawarith (lua_State *L, पूर्णांक op, स्थिर TValue *p1, स्थिर TValue *p2,
                   TValue *res) {
  चयन (op) {
    हाल LUA_OPBAND: हाल LUA_OPBOR: हाल LUA_OPBXOR:
    हाल LUA_OPSHL: हाल LUA_OPSHR:
    हाल LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      अगर (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        वापस 1;
      }
      अन्यथा वापस 0;  /* fail */
    }
    हाल LUA_OPDIV: हाल LUA_OPPOW: {  /* operate only on floats */
      lua_संख्या n1; lua_संख्या n2;
      अगर (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        वापस 1;
      }
      अन्यथा वापस 0;  /* fail */
    }
    शेष: {  /* other operations */
      lua_संख्या n1; lua_संख्या n2;
      अगर (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        वापस 1;
      }
      अन्यथा अगर (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        वापस 1;
      }
      अन्यथा वापस 0;  /* fail */
    }
  }
}


व्योम luaO_arith (lua_State *L, पूर्णांक op, स्थिर TValue *p1, स्थिर TValue *p2,
                 StkId res) {
  अगर (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (पूर्णांक c) {
  lua_assert(lisxdigit(c));
  अगर (lisdigit(c)) वापस cast_byte(c - '0');
  अन्यथा वापस cast_byte((ltolower(c) - 'a') + 10);
}


अटल पूर्णांक isneg (स्थिर अक्षर **s) {
  अगर (**s == '-') { (*s)++; वापस 1; }
  अन्यथा अगर (**s == '+') (*s)++;
  वापस 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#अगर !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#घोषणा MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
अटल lua_संख्या lua_strx2number (स्थिर अक्षर *s, अक्षर **endptr) {
  पूर्णांक dot = lua_getlocaledecpoint();
  lua_संख्या r = l_mathop(0.0);  /* result (accumulator) */
  पूर्णांक sigdig = 0;  /* number of significant digits */
  पूर्णांक nosigdig = 0;  /* number of non-significant digits */
  पूर्णांक e = 0;  /* exponent correction */
  पूर्णांक neg;  /* 1 if number is negative */
  पूर्णांक hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  जबतक (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  अगर (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    वापस l_mathop(0.0);  /* invalid format (no '0x') */
  क्रम (s += 2; ; s++) {  /* skip '0x' and read numeral */
    अगर (*s == dot) {
      अगर (hasdot) अवरोध;  /* second dot? stop loop */
      अन्यथा hasdot = 1;
    }
    अन्यथा अगर (lisxdigit(cast_uchar(*s))) {
      अगर (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      अन्यथा अगर (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      अन्यथा e++;  /* too many digits; ignore, but still count for exponent */
      अगर (hasdot) e--;  /* decimal digit? correct exponent */
    }
    अन्यथा अवरोध;  /* neither a dot nor a digit */
  }
  अगर (nosigdig + sigdig == 0)  /* no digits? */
    वापस l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  अगर (*s == 'p' || *s == 'P') {  /* exponent part? */
    पूर्णांक exp1 = 0;  /* exponent value */
    पूर्णांक neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    अगर (!lisdigit(cast_uchar(*s)))
      वापस l_mathop(0.0);  /* invalid; must have at least one digit */
    जबतक (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    अगर (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  अगर (neg) r = -r;
  वापस l_mathop(ldexp)(r, e);
}

#पूर्ण_अगर
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#अगर !defined (L_MAXLENNUM)
#घोषणा L_MAXLENNUM	200
#पूर्ण_अगर

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
अटल स्थिर अक्षर *l_str2dloc (स्थिर अक्षर *s, lua_संख्या *परिणाम, पूर्णांक mode) {
  अक्षर *endptr;
  *परिणाम = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  अगर (endptr == s) वापस शून्य;  /* nothing recognized? */
  जबतक (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  वापस (*endptr == '\0') ? endptr : शून्य;  /* OK iff no trailing chars */
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
अटल स्थिर अक्षर *l_str2d (स्थिर अक्षर *s, lua_संख्या *परिणाम) {
  स्थिर अक्षर *endptr;
  स्थिर अक्षर *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  पूर्णांक mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  अगर (mode == 'n')  /* reject 'inf' and 'nan' */
    वापस शून्य;
  endptr = l_str2dloc(s, परिणाम, mode);  /* try to convert */
  अगर (endptr == शून्य) {  /* failed? may be a different locale */
    अक्षर buff[L_MAXLENNUM + 1];
    स्थिर अक्षर *pdot = म_अक्षर(s, '.');
    अगर (pdot == शून्य || म_माप(s) > L_MAXLENNUM)
      वापस शून्य;  /* string too long or no dot; fail */
    म_नकल(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, परिणाम, mode);  /* try again */
    अगर (endptr != शून्य)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  वापस endptr;
}


#घोषणा MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#घोषणा MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

अटल स्थिर अक्षर *l_str2int (स्थिर अक्षर *s, lua_Integer *परिणाम) {
  lua_Unsigned a = 0;
  पूर्णांक empty = 1;
  पूर्णांक neg;
  जबतक (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  अगर (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    क्रम (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  अन्यथा {  /* decimal */
    क्रम (; lisdigit(cast_uchar(*s)); s++) {
      पूर्णांक d = *s - '0';
      अगर (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        वापस शून्य;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  जबतक (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  अगर (empty || *s != '\0') वापस शून्य;  /* something wrong in the numeral */
  अन्यथा {
    *परिणाम = l_castU2S((neg) ? 0u - a : a);
    वापस s;
  }
}


माप_प्रकार luaO_str2num (स्थिर अक्षर *s, TValue *o) {
  lua_Integer i; lua_संख्या n;
  स्थिर अक्षर *e;
  अगर ((e = l_str2int(s, &i)) != शून्य) {  /* try as an integer */
    setivalue(o, i);
  }
  अन्यथा अगर ((e = l_str2d(s, &n)) != शून्य) {  /* else try as a float */
    setfltvalue(o, n);
  }
  अन्यथा
    वापस 0;  /* conversion failed */
  वापस ct_diff2sz(e - s) + 1;  /* success; return string size */
}


पूर्णांक luaO_utf8esc (अक्षर *buff, l_uint32 x) {
  पूर्णांक n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  अगर (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  अन्यथा {  /* need continuation bytes */
    अचिन्हित पूर्णांक mfb = 0x3f;  /* maximum that fits in first byte */
    करो {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } जबतक (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  वापस n;
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
#अगर LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
#त्रुटि "invalid value for LUA_N2SBUFFSZ"
#पूर्ण_अगर


/*
** Convert a float to a string, adding it to a buffer. First try with
** a not too large number of digits, to avoid noise (for instance,
** 1.1 going to "1.1000000000000001"). If that lose precision, so
** that reading the result back gives a different number, then do the
** conversion again with extra precision. Moreover, if the numeral looks
** like an integer (without a decimal point or an exponent), add ".0" to
** its end.
*/
अटल पूर्णांक tostringbuffFloat (lua_संख्या n, अक्षर *buff) {
  /* first conversion */
  पूर्णांक len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_संख्या_FMT,
                            (LUAI_UACNUMBER)n);
  lua_संख्या check = lua_str2number(buff, शून्य);  /* read it back */
  अगर (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_संख्या_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  अगर (buff[म_अखोज(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  वापस len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
अचिन्हित luaO_tostringbuff (स्थिर TValue *obj, अक्षर *buff) {
  पूर्णांक len;
  lua_assert(ttisnumber(obj));
  अगर (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  अन्यथा
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  वापस cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
व्योम luaO_tostring (lua_State *L, TValue *obj) {
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
#घोषणा BUFVFS		cast_uint(LUA_IDSIZE + LUA_N2SBUFFSZ + 95)

/*
** Buffer used by 'luaO_pushvfstring'. 'err' signals an error while
** building result (memory error [1] or buffer overflow [2]).
*/
प्रकार काष्ठा BuffFS {
  lua_State *L;
  अक्षर *b;
  माप_प्रकार buffsize;
  माप_प्रकार blen;  /* length of string in 'buff' */
  पूर्णांक err;
  अक्षर space[BUFVFS];  /* initial buffer */
} BuffFS;


अटल व्योम initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = माप(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
अटल व्योम pushbuff (lua_State *L, व्योम *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  चयन (buff->err) {
    हाल 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      अवरोध;
    हाल 2:  /* length overflow: Add "..." at the end of result */
      अगर (buff->buffsize - buff->blen < 3)
        म_नकल(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      अन्यथा {  /* there is enough space left for the "..." */
        म_नकल(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    शेष: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->शीर्ष.p, ts);
      L->शीर्ष.p++;
    }
  }
}


अटल स्थिर अक्षर *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  स्थिर अक्षर *res;
  अगर (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = शून्य;  /* error message is on the top of the stack */
  अन्यथा
    res = getstr(tsvalue(s2v(L->शीर्ष.p - 1)));
  अगर (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  वापस res;
}


अटल व्योम addstr2buff (BuffFS *buff, स्थिर अक्षर *str, माप_प्रकार slen) {
  माप_प्रकार बायाँ = buff->buffsize - buff->blen;  /* space left in the buffer */
  अगर (buff->err)  /* do nothing else after an error */
    वापस;
  अगर (slen > बायाँ) {  /* new string doesn't fit into current buffer? */
    अगर (slen > ((MAX_आकार/2) - buff->blen)) {  /* overflow? */
      स_नकल(buff->b + buff->blen, str, बायाँ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      वापस;
    }
    अन्यथा {
      माप_प्रकार newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      अक्षर *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, शून्य, 0, newsize, अक्षर)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               अक्षर);
      अगर (newb == शून्य) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        वापस;
      }
      अगर (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
अटल व्योम addnum2buff (BuffFS *buff, TValue *अंक) {
  अक्षर numbuff[LUA_N2SBUFFSZ];
  अचिन्हित len = luaO_tostringbuff(अंक, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
स्थिर अक्षर *luaO_pushvfstring (lua_State *L, स्थिर अक्षर *fmt, बहु_सूची argp) {
  BuffFS buff;  /* holds last part of the result */
  स्थिर अक्षर *e;  /* points to next '%' */
  initbuff(L, &buff);
  जबतक ((e = म_अक्षर(fmt, '%')) != शून्य) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    चयन (*(e + 1)) {  /* conversion specifier */
      हाल 's': {  /* zero-terminated string */
        स्थिर अक्षर *s = बहु_तर्क(argp, अक्षर *);
        अगर (s == शून्य) s = "(null)";
        addstr2buff(&buff, s, म_माप(s));
        अवरोध;
      }
      हाल 'c': {  /* an 'int' as a character */
        अक्षर c = cast_char(बहु_तर्क(argp, पूर्णांक));
        addstr2buff(&buff, &c, माप(अक्षर));
        अवरोध;
      }
      हाल 'd': {  /* an 'int' */
        TValue अंक;
        setivalue(&अंक, बहु_तर्क(argp, पूर्णांक));
        addnum2buff(&buff, &अंक);
        अवरोध;
      }
      हाल 'I': {  /* a 'lua_Integer' */
        TValue अंक;
        setivalue(&अंक, cast_Integer(बहु_तर्क(argp, l_uacInt)));
        addnum2buff(&buff, &अंक);
        अवरोध;
      }
      हाल 'f': {  /* a 'lua_Number' */
        TValue अंक;
        setfltvalue(&अंक, cast_अंक(बहु_तर्क(argp, l_uac_संख्या)));
        addnum2buff(&buff, &अंक);
        अवरोध;
      }
      हाल 'p': {  /* a pointer */
        अक्षर bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        व्योम *p = बहु_तर्क(argp, व्योम *);
        पूर्णांक len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        अवरोध;
      }
      हाल 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        अक्षर bf[UTF8BUFFSZ];
        अचिन्हित दीर्घ arg = बहु_तर्क(argp, अचिन्हित दीर्घ);
        पूर्णांक len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        अवरोध;
      }
      हाल '%': {
        addstr2buff(&buff, "%", 1);
        अवरोध;
      }
      शेष: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        अवरोध;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, म_माप(fmt));  /* rest of 'fmt' */
  वापस clearbuff(&buff);  /* empty buffer into a new string */
}


स्थिर अक्षर *luaO_pushfstring (lua_State *L, स्थिर अक्षर *fmt, ...) {
  स्थिर अक्षर *संदेश;
  बहु_सूची argp;
  बहु_शुरू(argp, fmt);
  संदेश = luaO_pushvfstring(L, fmt, argp);
  बहु_पूर्ण(argp);
  अगर (संदेश == शून्य)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  वापस संदेश;
}

/* }================================================================== */


#घोषणा RETS	"..."
#घोषणा PRE	"[string \""
#घोषणा POS	"\"]"

#घोषणा addstr(a,b,l)	( स_नकल(a,b,(l) * माप(अक्षर)), a += (l) )

व्योम luaO_chunkid (अक्षर *out, स्थिर अक्षर *source, माप_प्रकार srclen) {
  माप_प्रकार bufflen = LUA_IDSIZE;  /* free space in buffer */
  अगर (*source == '=') {  /* 'literal' source */
    अगर (srclen <= bufflen)  /* small enough? */
      स_नकल(out, source + 1, srclen * माप(अक्षर));
    अन्यथा {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  अन्यथा अगर (*source == '@') {  /* file name */
    अगर (srclen <= bufflen)  /* small enough? */
      स_नकल(out, source + 1, srclen * माप(अक्षर));
    अन्यथा {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      स_नकल(out, source + 1 + srclen - bufflen, bufflen * माप(अक्षर));
    }
  }
  अन्यथा {  /* string; format as [string "source"] */
    स्थिर अक्षर *nl = म_अक्षर(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    अगर (srclen < bufflen && nl == शून्य) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    अन्यथा {
      अगर (nl != शून्य)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      अगर (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    स_नकल(out, POS, (LL(POS) + 1) * माप(अक्षर));
  }
}

