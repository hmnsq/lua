/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <float.h>
#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lctype.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned પૂર્ણાંક x) {
  સ્થિર અચલ lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  પૂર્ણાંક l = 0;
  x--;
  જ્યારે (x >= 256) { l += 8; x >>= 8; }
  પાછા cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned પૂર્ણાંક p) {
  જો (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    પાછા 0xFF;  /* return maximum value */
  નહીંતર {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    જો (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      પાછા cast_byte(p);
    }
    નહીંતર {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      પાછા cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  પૂર્ણાંક m = p & 0xF;  /* mantissa */
  પૂર્ણાંક e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  જો (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  જો (e >= 0) {
    જો (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      પાછા (x * m) << e;  /* order doesn't matter here */
    નહીંતર  /* real overflow */
      પાછા MAX_LMEM;
  }
  નહીંતર {  /* negative exponent */
    e = -e;
    જો (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      પાછા (x * m) >> e;  /* multiplying first gives more precision */
    નહીંતર જો ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      પાછા (x >> e) * m;
    નહીંતર  /* real overflow */
      પાછા MAX_LMEM;
  }
}


સ્થિર lua_Integer intarith (lua_State *L, પૂર્ણાંક op, lua_Integer v1,
                                                   lua_Integer v2) {
  પસંદગી (op) {
    સ્થિતિ LUA_OPADD: પાછા intop(+, v1, v2);
    સ્થિતિ LUA_OPSUB:પાછા intop(-, v1, v2);
    સ્થિતિ LUA_OPMUL:પાછા intop(*, v1, v2);
    સ્થિતિ LUA_OPMOD: પાછા luaV_mod(L, v1, v2);
    સ્થિતિ LUA_OPIDIV: પાછા luaV_idiv(L, v1, v2);
    સ્થિતિ LUA_OPBAND: પાછા intop(&, v1, v2);
    સ્થિતિ LUA_OPBOR: પાછા intop(|, v1, v2);
    સ્થિતિ LUA_OPBXOR: પાછા intop(^, v1, v2);
    સ્થિતિ LUA_OPSHL: પાછા luaV_shiftl(v1, v2);
    સ્થિતિ LUA_OPSHR: પાછા luaV_shiftr(v1, v2);
    સ્થિતિ LUA_OPUNM: પાછા intop(-, 0, v1);
    સ્થિતિ LUA_OPBNOT: પાછા intop(^, ~l_castS2U(0), v1);
    મૂળભૂત: lua_assert(0); પાછા 0;
  }
}


સ્થિર lua_સંખ્યા numarith (lua_State *L, પૂર્ણાંક op, lua_સંખ્યા v1,
                                                  lua_સંખ્યા v2) {
  પસંદગી (op) {
    સ્થિતિ LUA_OPADD: પાછા luai_numadd(L, v1, v2);
    સ્થિતિ LUA_OPSUB: પાછા luai_numsub(L, v1, v2);
    સ્થિતિ LUA_OPMUL: પાછા luai_nummul(L, v1, v2);
    સ્થિતિ LUA_OPDIV: પાછા luai_numdiv(L, v1, v2);
    સ્થિતિ LUA_OPPOW: પાછા luai_numpow(L, v1, v2);
    સ્થિતિ LUA_OPIDIV: પાછા luai_numidiv(L, v1, v2);
    સ્થિતિ LUA_OPUNM: પાછા luai_numunm(L, v1);
    સ્થિતિ LUA_OPMOD: પાછા luaV_modf(L, v1, v2);
    મૂળભૂત: lua_assert(0); પાછા 0;
  }
}


પૂર્ણાંક luaO_rawarith (lua_State *L, પૂર્ણાંક op, અચલ TValue *p1, અચલ TValue *p2,
                   TValue *res) {
  પસંદગી (op) {
    સ્થિતિ LUA_OPBAND: સ્થિતિ LUA_OPBOR: સ્થિતિ LUA_OPBXOR:
    સ્થિતિ LUA_OPSHL: સ્થિતિ LUA_OPSHR:
    સ્થિતિ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      જો (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        પાછા 1;
      }
      નહીંતર પાછા 0;  /* fail */
    }
    સ્થિતિ LUA_OPDIV: સ્થિતિ LUA_OPPOW: {  /* operate only on floats */
      lua_સંખ્યા n1; lua_સંખ્યા n2;
      જો (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        પાછા 1;
      }
      નહીંતર પાછા 0;  /* fail */
    }
    મૂળભૂત: {  /* other operations */
      lua_સંખ્યા n1; lua_સંખ્યા n2;
      જો (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        પાછા 1;
      }
      નહીંતર જો (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        પાછા 1;
      }
      નહીંતર પાછા 0;  /* fail */
    }
  }
}


શૂન્ય luaO_arith (lua_State *L, પૂર્ણાંક op, અચલ TValue *p1, અચલ TValue *p2,
                 StkId res) {
  જો (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (પૂર્ણાંક c) {
  lua_assert(lisxdigit(c));
  જો (lisdigit(c)) પાછા cast_byte(c - '0');
  નહીંતર પાછા cast_byte((ltolower(c) - 'a') + 10);
}


સ્થિર પૂર્ણાંક isneg (અચલ અક્ષર **s) {
  જો (**s == '-') { (*s)++; પાછા 1; }
  નહીંતર જો (**s == '+') (*s)++;
  પાછા 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#જો !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
સ્થિર lua_સંખ્યા lua_strx2number (અચલ અક્ષર *s, અક્ષર **endptr) {
  પૂર્ણાંક dot = lua_getlocaledecpoint();
  lua_સંખ્યા r = l_mathop(0.0);  /* result (accumulator) */
  પૂર્ણાંક sigdig = 0;  /* number of significant digits */
  પૂર્ણાંક nosigdig = 0;  /* number of non-significant digits */
  પૂર્ણાંક e = 0;  /* exponent correction */
  પૂર્ણાંક neg;  /* 1 if number is negative */
  પૂર્ણાંક hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  જ્યારે (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  જો (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    પાછા l_mathop(0.0);  /* invalid format (no '0x') */
  માટે (s += 2; ; s++) {  /* skip '0x' and read numeral */
    જો (*s == dot) {
      જો (hasdot) તોડો;  /* second dot? stop loop */
      નહીંતર hasdot = 1;
    }
    નહીંતર જો (lisxdigit(cast_uchar(*s))) {
      જો (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      નહીંતર જો (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      નહીંતર e++;  /* too many digits; ignore, but still count for exponent */
      જો (hasdot) e--;  /* decimal digit? correct exponent */
    }
    નહીંતર તોડો;  /* neither a dot nor a digit */
  }
  જો (nosigdig + sigdig == 0)  /* no digits? */
    પાછા l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  જો (*s == 'p' || *s == 'P') {  /* exponent part? */
    પૂર્ણાંક exp1 = 0;  /* exponent value */
    પૂર્ણાંક neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    જો (!lisdigit(cast_uchar(*s)))
      પાછા l_mathop(0.0);  /* invalid; must have at least one digit */
    જ્યારે (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    જો (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  જો (neg) r = -r;
  પાછા l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#જો !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
સ્થિર અચલ અક્ષર *l_str2dloc (અચલ અક્ષર *s, lua_સંખ્યા *result, પૂર્ણાંક mode) {
  અક્ષર *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  જો (endptr == s) પાછા ખાલી;  /* nothing recognized? */
  જ્યારે (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  પાછા (*endptr == '\0') ? endptr : ખાલી;  /* OK iff no trailing chars */
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
સ્થિર અચલ અક્ષર *l_str2d (અચલ અક્ષર *s, lua_સંખ્યા *result) {
  અચલ અક્ષર *endptr;
  અચલ અક્ષર *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  પૂર્ણાંક mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  જો (mode == 'n')  /* reject 'inf' and 'nan' */
    પાછા ખાલી;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  જો (endptr == ખાલી) {  /* failed? may be a different locale */
    અક્ષર buff[L_MAXLENNUM + 1];
    અચલ અક્ષર *pdot = strchr(s, '.');
    જો (pdot == ખાલી || strlen(s) > L_MAXLENNUM)
      પાછા ખાલી;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    જો (endptr != ખાલી)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  પાછા endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

સ્થિર અચલ અક્ષર *l_str2int (અચલ અક્ષર *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  પૂર્ણાંક empty = 1;
  પૂર્ણાંક neg;
  જ્યારે (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  જો (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    માટે (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  નહીંતર {  /* decimal */
    માટે (; lisdigit(cast_uchar(*s)); s++) {
      પૂર્ણાંક d = *s - '0';
      જો (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        પાછા ખાલી;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  જ્યારે (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  જો (empty || *s != '\0') પાછા ખાલી;  /* something wrong in the numeral */
  નહીંતર {
    *result = l_castU2S((neg) ? 0u - a : a);
    પાછા s;
  }
}


size_t luaO_str2num (અચલ અક્ષર *s, TValue *o) {
  lua_Integer i; lua_સંખ્યા n;
  અચલ અક્ષર *e;
  જો ((e = l_str2int(s, &i)) != ખાલી) {  /* try as an integer */
    setivalue(o, i);
  }
  નહીંતર જો ((e = l_str2d(s, &n)) != ખાલી) {  /* else try as a float */
    setfltvalue(o, n);
  }
  નહીંતર
    પાછા 0;  /* conversion failed */
  પાછા ct_diff2sz(e - s) + 1;  /* success; return string size */
}


પૂર્ણાંક luaO_utf8esc (અક્ષર *buff, l_uint32 x) {
  પૂર્ણાંક n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  જો (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  નહીંતર {  /* need continuation bytes */
    unsigned પૂર્ણાંક mfb = 0x3f;  /* maximum that fits in first byte */
    કરો {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } જ્યારે (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  પાછા n;
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
#જો LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
સ્થિર પૂર્ણાંક tostringbuffFloat (lua_સંખ્યા n, અક્ષર *buff) {
  /* first conversion */
  પૂર્ણાંક len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_સંખ્યા_FMT,
                            (LUAI_UACNUMBER)n);
  lua_સંખ્યા check = lua_str2number(buff, ખાલી);  /* read it back */
  જો (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_સંખ્યા_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  જો (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  પાછા len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (અચલ TValue *obj, અક્ષર *buff) {
  પૂર્ણાંક len;
  lua_assert(ttisnumber(obj));
  જો (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  નહીંતર
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  પાછા cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
શૂન્ય luaO_tostring (lua_State *L, TValue *obj) {
  અક્ષર buff[LUA_N2SBUFFSZ];
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
પ્રકારવ્યાખ્યા સંરચના BuffFS {
  lua_State *L;
  અક્ષર *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  પૂર્ણાંક err;
  અક્ષર space[BUFVFS];  /* initial buffer */
} BuffFS;


સ્થિર શૂન્ય initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = કદ(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
સ્થિર શૂન્ય pushbuff (lua_State *L, શૂન્ય *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  પસંદગી (buff->err) {
    સ્થિતિ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      તોડો;
    સ્થિતિ 2:  /* length overflow: Add "..." at the end of result */
      જો (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      નહીંતર {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    મૂળભૂત: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->ટોચ.p, ts);
      L->ટોચ.p++;
    }
  }
}


સ્થિર અચલ અક્ષર *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  અચલ અક્ષર *res;
  જો (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ખાલી;  /* error message is on the top of the stack */
  નહીંતર
    res = getstr(tsvalue(s2v(L->ટોચ.p - 1)));
  જો (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  પાછા res;
}


સ્થિર શૂન્ય addstr2buff (BuffFS *buff, અચલ અક્ષર *str, size_t slen) {
  size_t ડાબું = buff->buffsize - buff->blen;  /* space left in the buffer */
  જો (buff->err)  /* do nothing else after an error */
    પાછા;
  જો (slen > ડાબું) {  /* new string doesn't fit into current buffer? */
    જો (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, ડાબું);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      પાછા;
    }
    નહીંતર {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      અક્ષર *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ખાલી, 0, newsize, અક્ષર)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               અક્ષર);
      જો (newb == ખાલી) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        પાછા;
      }
      જો (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
સ્થિર શૂન્ય addnum2buff (BuffFS *buff, TValue *અંક) {
  અક્ષર numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(અંક, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
અચલ અક્ષર *luaO_pushvfstring (lua_State *L, અચલ અક્ષર *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  અચલ અક્ષર *e;  /* points to next '%' */
  initbuff(L, &buff);
  જ્યારે ((e = strchr(fmt, '%')) != ખાલી) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    પસંદગી (*(e + 1)) {  /* conversion specifier */
      સ્થિતિ 's': {  /* zero-terminated string */
        અચલ અક્ષર *s = va_arg(argp, અક્ષર *);
        જો (s == ખાલી) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        તોડો;
      }
      સ્થિતિ 'c': {  /* an 'int' as a character */
        અક્ષર c = cast_char(va_arg(argp, પૂર્ણાંક));
        addstr2buff(&buff, &c, કદ(અક્ષર));
        તોડો;
      }
      સ્થિતિ 'd': {  /* an 'int' */
        TValue અંક;
        setivalue(&અંક, va_arg(argp, પૂર્ણાંક));
        addnum2buff(&buff, &અંક);
        તોડો;
      }
      સ્થિતિ 'I': {  /* a 'lua_Integer' */
        TValue અંક;
        setivalue(&અંક, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &અંક);
        તોડો;
      }
      સ્થિતિ 'f': {  /* a 'lua_Number' */
        TValue અંક;
        setfltvalue(&અંક, cast_અંક(va_arg(argp, l_uac_સંખ્યા)));
        addnum2buff(&buff, &અંક);
        તોડો;
      }
      સ્થિતિ 'p': {  /* a pointer */
        અક્ષર bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        શૂન્ય *p = va_arg(argp, શૂન્ય *);
        પૂર્ણાંક len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        તોડો;
      }
      સ્થિતિ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        અક્ષર bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        પૂર્ણાંક len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        તોડો;
      }
      સ્થિતિ '%': {
        addstr2buff(&buff, "%", 1);
        તોડો;
      }
      મૂળભૂત: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        તોડો;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  પાછા clearbuff(&buff);  /* empty buffer into a new string */
}


અચલ અક્ષર *luaO_pushfstring (lua_State *L, અચલ અક્ષર *fmt, ...) {
  અચલ અક્ષર *સંદેશ;
  va_list argp;
  va_start(argp, fmt);
  સંદેશ = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  જો (સંદેશ == ખાલી)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  પાછા સંદેશ;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * કદ(અક્ષર)), a += (l) )

શૂન્ય luaO_chunkid (અક્ષર *out, અચલ અક્ષર *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  જો (*source == '=') {  /* 'literal' source */
    જો (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * કદ(અક્ષર));
    નહીંતર {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  નહીંતર જો (*source == '@') {  /* file name */
    જો (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * કદ(અક્ષર));
    નહીંતર {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * કદ(અક્ષર));
    }
  }
  નહીંતર {  /* string; format as [string "source"] */
    અચલ અક્ષર *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    જો (srclen < bufflen && nl == ખાલી) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    નહીંતર {
      જો (nl != ખાલી)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      જો (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * કદ(અક્ષર));
  }
}

