/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <float.h>
#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lctype.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned முழு x) {
  நிலையான மாறிலி lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  முழு l = 0;
  x--;
  போது (x >= 256) { l += 8; x >>= 8; }
  திரும்பு cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned முழு p) {
  எனில் (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    திரும்பு 0xFF;  /* return maximum value */
  இல்லையெனில் {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    எனில் (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      திரும்பு cast_byte(p);
    }
    இல்லையெனில் {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      திரும்பு cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  முழு m = p & 0xF;  /* mantissa */
  முழு e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  எனில் (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  எனில் (e >= 0) {
    எனில் (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      திரும்பு (x * m) << e;  /* order doesn't matter here */
    இல்லையெனில்  /* real overflow */
      திரும்பு MAX_LMEM;
  }
  இல்லையெனில் {  /* negative exponent */
    e = -e;
    எனில் (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      திரும்பு (x * m) >> e;  /* multiplying first gives more precision */
    இல்லையெனில் எனில் ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      திரும்பு (x >> e) * m;
    இல்லையெனில்  /* real overflow */
      திரும்பு MAX_LMEM;
  }
}


நிலையான lua_Integer intarith (lua_State *L, முழு op, lua_Integer v1,
                                                   lua_Integer v2) {
  தேர்வு (op) {
    நிலை LUA_OPADD: திரும்பு intop(+, v1, v2);
    நிலை LUA_OPSUB:திரும்பு intop(-, v1, v2);
    நிலை LUA_OPMUL:திரும்பு intop(*, v1, v2);
    நிலை LUA_OPMOD: திரும்பு luaV_mod(L, v1, v2);
    நிலை LUA_OPIDIV: திரும்பு luaV_idiv(L, v1, v2);
    நிலை LUA_OPBAND: திரும்பு intop(&, v1, v2);
    நிலை LUA_OPBOR: திரும்பு intop(|, v1, v2);
    நிலை LUA_OPBXOR: திரும்பு intop(^, v1, v2);
    நிலை LUA_OPSHL: திரும்பு luaV_shiftl(v1, v2);
    நிலை LUA_OPSHR: திரும்பு luaV_shiftr(v1, v2);
    நிலை LUA_OPUNM: திரும்பு intop(-, 0, v1);
    நிலை LUA_OPBNOT: திரும்பு intop(^, ~l_castS2U(0), v1);
    இயல்பு: lua_assert(0); திரும்பு 0;
  }
}


நிலையான lua_எண் numarith (lua_State *L, முழு op, lua_எண் v1,
                                                  lua_எண் v2) {
  தேர்வு (op) {
    நிலை LUA_OPADD: திரும்பு luai_numadd(L, v1, v2);
    நிலை LUA_OPSUB: திரும்பு luai_numsub(L, v1, v2);
    நிலை LUA_OPMUL: திரும்பு luai_nummul(L, v1, v2);
    நிலை LUA_OPDIV: திரும்பு luai_numdiv(L, v1, v2);
    நிலை LUA_OPPOW: திரும்பு luai_numpow(L, v1, v2);
    நிலை LUA_OPIDIV: திரும்பு luai_numidiv(L, v1, v2);
    நிலை LUA_OPUNM: திரும்பு luai_numunm(L, v1);
    நிலை LUA_OPMOD: திரும்பு luaV_modf(L, v1, v2);
    இயல்பு: lua_assert(0); திரும்பு 0;
  }
}


முழு luaO_rawarith (lua_State *L, முழு op, மாறிலி TValue *p1, மாறிலி TValue *p2,
                   TValue *res) {
  தேர்வு (op) {
    நிலை LUA_OPBAND: நிலை LUA_OPBOR: நிலை LUA_OPBXOR:
    நிலை LUA_OPSHL: நிலை LUA_OPSHR:
    நிலை LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      எனில் (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        திரும்பு 1;
      }
      இல்லையெனில் திரும்பு 0;  /* fail */
    }
    நிலை LUA_OPDIV: நிலை LUA_OPPOW: {  /* operate only on floats */
      lua_எண் n1; lua_எண் n2;
      எனில் (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        திரும்பு 1;
      }
      இல்லையெனில் திரும்பு 0;  /* fail */
    }
    இயல்பு: {  /* other operations */
      lua_எண் n1; lua_எண் n2;
      எனில் (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        திரும்பு 1;
      }
      இல்லையெனில் எனில் (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        திரும்பு 1;
      }
      இல்லையெனில் திரும்பு 0;  /* fail */
    }
  }
}


வெறுமை luaO_arith (lua_State *L, முழு op, மாறிலி TValue *p1, மாறிலி TValue *p2,
                 StkId res) {
  எனில் (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (முழு c) {
  lua_assert(lisxdigit(c));
  எனில் (lisdigit(c)) திரும்பு cast_byte(c - '0');
  இல்லையெனில் திரும்பு cast_byte((ltolower(c) - 'a') + 10);
}


நிலையான முழு isneg (மாறிலி எழுத்து **s) {
  எனில் (**s == '-') { (*s)++; திரும்பு 1; }
  இல்லையெனில் எனில் (**s == '+') (*s)++;
  திரும்பு 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#எனில் !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
நிலையான lua_எண் lua_strx2number (மாறிலி எழுத்து *s, எழுத்து **endptr) {
  முழு dot = lua_getlocaledecpoint();
  lua_எண் r = l_mathop(0.0);  /* result (accumulator) */
  முழு sigdig = 0;  /* number of significant digits */
  முழு nosigdig = 0;  /* number of non-significant digits */
  முழு e = 0;  /* exponent correction */
  முழு neg;  /* 1 if number is negative */
  முழு hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  போது (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  எனில் (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    திரும்பு l_mathop(0.0);  /* invalid format (no '0x') */
  க்கு (s += 2; ; s++) {  /* skip '0x' and read numeral */
    எனில் (*s == dot) {
      எனில் (hasdot) நிறுத்து;  /* second dot? stop loop */
      இல்லையெனில் hasdot = 1;
    }
    இல்லையெனில் எனில் (lisxdigit(cast_uchar(*s))) {
      எனில் (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      இல்லையெனில் எனில் (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      இல்லையெனில் e++;  /* too many digits; ignore, but still count for exponent */
      எனில் (hasdot) e--;  /* decimal digit? correct exponent */
    }
    இல்லையெனில் நிறுத்து;  /* neither a dot nor a digit */
  }
  எனில் (nosigdig + sigdig == 0)  /* no digits? */
    திரும்பு l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  எனில் (*s == 'p' || *s == 'P') {  /* exponent part? */
    முழு exp1 = 0;  /* exponent value */
    முழு neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    எனில் (!lisdigit(cast_uchar(*s)))
      திரும்பு l_mathop(0.0);  /* invalid; must have at least one digit */
    போது (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    எனில் (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  எனில் (neg) r = -r;
  திரும்பு l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#எனில் !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
நிலையான மாறிலி எழுத்து *l_str2dloc (மாறிலி எழுத்து *s, lua_எண் *result, முழு mode) {
  எழுத்து *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  எனில் (endptr == s) திரும்பு வெற்று;  /* nothing recognized? */
  போது (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  திரும்பு (*endptr == '\0') ? endptr : வெற்று;  /* OK iff no trailing chars */
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
நிலையான மாறிலி எழுத்து *l_str2d (மாறிலி எழுத்து *s, lua_எண் *result) {
  மாறிலி எழுத்து *endptr;
  மாறிலி எழுத்து *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  முழு mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  எனில் (mode == 'n')  /* reject 'inf' and 'nan' */
    திரும்பு வெற்று;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  எனில் (endptr == வெற்று) {  /* failed? may be a different locale */
    எழுத்து buff[L_MAXLENNUM + 1];
    மாறிலி எழுத்து *pdot = strchr(s, '.');
    எனில் (pdot == வெற்று || strlen(s) > L_MAXLENNUM)
      திரும்பு வெற்று;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    எனில் (endptr != வெற்று)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  திரும்பு endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

நிலையான மாறிலி எழுத்து *l_str2int (மாறிலி எழுத்து *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  முழு empty = 1;
  முழு neg;
  போது (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  எனில் (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    க்கு (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  இல்லையெனில் {  /* decimal */
    க்கு (; lisdigit(cast_uchar(*s)); s++) {
      முழு d = *s - '0';
      எனில் (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        திரும்பு வெற்று;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  போது (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  எனில் (empty || *s != '\0') திரும்பு வெற்று;  /* something wrong in the numeral */
  இல்லையெனில் {
    *result = l_castU2S((neg) ? 0u - a : a);
    திரும்பு s;
  }
}


size_t luaO_str2num (மாறிலி எழுத்து *s, TValue *o) {
  lua_Integer i; lua_எண் n;
  மாறிலி எழுத்து *e;
  எனில் ((e = l_str2int(s, &i)) != வெற்று) {  /* try as an integer */
    setivalue(o, i);
  }
  இல்லையெனில் எனில் ((e = l_str2d(s, &n)) != வெற்று) {  /* else try as a float */
    setfltvalue(o, n);
  }
  இல்லையெனில்
    திரும்பு 0;  /* conversion failed */
  திரும்பு ct_diff2sz(e - s) + 1;  /* success; return string size */
}


முழு luaO_utf8esc (எழுத்து *buff, l_uint32 x) {
  முழு n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  எனில் (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  இல்லையெனில் {  /* need continuation bytes */
    unsigned முழு mfb = 0x3f;  /* maximum that fits in first byte */
    செய் {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } போது (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  திரும்பு n;
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
#எனில் LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
நிலையான முழு tostringbuffFloat (lua_எண் n, எழுத்து *buff) {
  /* first conversion */
  முழு len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_எண்_FMT,
                            (LUAI_UACNUMBER)n);
  lua_எண் check = lua_str2number(buff, வெற்று);  /* read it back */
  எனில் (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_எண்_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  எனில் (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  திரும்பு len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (மாறிலி TValue *obj, எழுத்து *buff) {
  முழு len;
  lua_assert(ttisnumber(obj));
  எனில் (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  இல்லையெனில்
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  திரும்பு cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
வெறுமை luaO_tostring (lua_State *L, TValue *obj) {
  எழுத்து buff[LUA_N2SBUFFSZ];
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
வகைவரை கட்டமைப்பு BuffFS {
  lua_State *L;
  எழுத்து *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  முழு err;
  எழுத்து space[BUFVFS];  /* initial buffer */
} BuffFS;


நிலையான வெறுமை initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = அளவு(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
நிலையான வெறுமை pushbuff (lua_State *L, வெறுமை *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  தேர்வு (buff->err) {
    நிலை 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      நிறுத்து;
    நிலை 2:  /* length overflow: Add "..." at the end of result */
      எனில் (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      இல்லையெனில் {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    இயல்பு: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->உச்சி.p, ts);
      L->உச்சி.p++;
    }
  }
}


நிலையான மாறிலி எழுத்து *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  மாறிலி எழுத்து *res;
  எனில் (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = வெற்று;  /* error message is on the top of the stack */
  இல்லையெனில்
    res = getstr(tsvalue(s2v(L->உச்சி.p - 1)));
  எனில் (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  திரும்பு res;
}


நிலையான வெறுமை addstr2buff (BuffFS *buff, மாறிலி எழுத்து *str, size_t slen) {
  size_t இடது = buff->buffsize - buff->blen;  /* space left in the buffer */
  எனில் (buff->err)  /* do nothing else after an error */
    திரும்பு;
  எனில் (slen > இடது) {  /* new string doesn't fit into current buffer? */
    எனில் (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, இடது);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      திரும்பு;
    }
    இல்லையெனில் {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      எழுத்து *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, வெற்று, 0, newsize, எழுத்து)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               எழுத்து);
      எனில் (newb == வெற்று) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        திரும்பு;
      }
      எனில் (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
நிலையான வெறுமை addnum2buff (BuffFS *buff, TValue *இலக்கம்) {
  எழுத்து numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(இலக்கம், numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
மாறிலி எழுத்து *luaO_pushvfstring (lua_State *L, மாறிலி எழுத்து *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  மாறிலி எழுத்து *e;  /* points to next '%' */
  initbuff(L, &buff);
  போது ((e = strchr(fmt, '%')) != வெற்று) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    தேர்வு (*(e + 1)) {  /* conversion specifier */
      நிலை 's': {  /* zero-terminated string */
        மாறிலி எழுத்து *s = va_arg(argp, எழுத்து *);
        எனில் (s == வெற்று) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        நிறுத்து;
      }
      நிலை 'c': {  /* an 'int' as a character */
        எழுத்து c = cast_char(va_arg(argp, முழு));
        addstr2buff(&buff, &c, அளவு(எழுத்து));
        நிறுத்து;
      }
      நிலை 'd': {  /* an 'int' */
        TValue இலக்கம்;
        setivalue(&இலக்கம், va_arg(argp, முழு));
        addnum2buff(&buff, &இலக்கம்);
        நிறுத்து;
      }
      நிலை 'I': {  /* a 'lua_Integer' */
        TValue இலக்கம்;
        setivalue(&இலக்கம், cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &இலக்கம்);
        நிறுத்து;
      }
      நிலை 'f': {  /* a 'lua_Number' */
        TValue இலக்கம்;
        setfltvalue(&இலக்கம், cast_இலக்கம்(va_arg(argp, l_uac_எண்)));
        addnum2buff(&buff, &இலக்கம்);
        நிறுத்து;
      }
      நிலை 'p': {  /* a pointer */
        எழுத்து bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        வெறுமை *p = va_arg(argp, வெறுமை *);
        முழு len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        நிறுத்து;
      }
      நிலை 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        எழுத்து bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        முழு len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        நிறுத்து;
      }
      நிலை '%': {
        addstr2buff(&buff, "%", 1);
        நிறுத்து;
      }
      இயல்பு: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        நிறுத்து;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  திரும்பு clearbuff(&buff);  /* empty buffer into a new string */
}


மாறிலி எழுத்து *luaO_pushfstring (lua_State *L, மாறிலி எழுத்து *fmt, ...) {
  மாறிலி எழுத்து *செய்தி;
  va_list argp;
  va_start(argp, fmt);
  செய்தி = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  எனில் (செய்தி == வெற்று)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  திரும்பு செய்தி;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * அளவு(எழுத்து)), a += (l) )

வெறுமை luaO_chunkid (எழுத்து *out, மாறிலி எழுத்து *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  எனில் (*source == '=') {  /* 'literal' source */
    எனில் (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * அளவு(எழுத்து));
    இல்லையெனில் {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  இல்லையெனில் எனில் (*source == '@') {  /* file name */
    எனில் (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * அளவு(எழுத்து));
    இல்லையெனில் {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * அளவு(எழுத்து));
    }
  }
  இல்லையெனில் {  /* string; format as [string "source"] */
    மாறிலி எழுத்து *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    எனில் (srclen < bufflen && nl == வெற்று) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    இல்லையெனில் {
      எனில் (nl != வெற்று)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      எனில் (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * அளவு(எழுத்து));
  }
}

