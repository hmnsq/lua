/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <float.h>
#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lctype.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned പൂർണ്ണസംഖ്യ x) {
  സ്ഥിതം സ്ഥിരം lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  പൂർണ്ണസംഖ്യ l = 0;
  x--;
  ആകുമ്പോൾ (x >= 256) { l += 8; x >>= 8; }
  തിരികെ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned പൂർണ്ണസംഖ്യ p) {
  എങ്കിൽ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    തിരികെ 0xFF;  /* return maximum value */
  അല്ലെങ്കിൽ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    എങ്കിൽ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      തിരികെ cast_byte(p);
    }
    അല്ലെങ്കിൽ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      തിരികെ cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  പൂർണ്ണസംഖ്യ m = p & 0xF;  /* mantissa */
  പൂർണ്ണസംഖ്യ e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  എങ്കിൽ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  എങ്കിൽ (e >= 0) {
    എങ്കിൽ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      തിരികെ (x * m) << e;  /* order doesn't matter here */
    അല്ലെങ്കിൽ  /* real overflow */
      തിരികെ MAX_LMEM;
  }
  അല്ലെങ്കിൽ {  /* negative exponent */
    e = -e;
    എങ്കിൽ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      തിരികെ (x * m) >> e;  /* multiplying first gives more precision */
    അല്ലെങ്കിൽ എങ്കിൽ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      തിരികെ (x >> e) * m;
    അല്ലെങ്കിൽ  /* real overflow */
      തിരികെ MAX_LMEM;
  }
}


സ്ഥിതം lua_Integer intarith (lua_State *L, പൂർണ്ണസംഖ്യ op, lua_Integer v1,
                                                   lua_Integer v2) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ LUA_OPADD: തിരികെ intop(+, v1, v2);
    അവസ്ഥ LUA_OPSUB:തിരികെ intop(-, v1, v2);
    അവസ്ഥ LUA_OPMUL:തിരികെ intop(*, v1, v2);
    അവസ്ഥ LUA_OPMOD: തിരികെ luaV_mod(L, v1, v2);
    അവസ്ഥ LUA_OPIDIV: തിരികെ luaV_idiv(L, v1, v2);
    അവസ്ഥ LUA_OPBAND: തിരികെ intop(&, v1, v2);
    അവസ്ഥ LUA_OPBOR: തിരികെ intop(|, v1, v2);
    അവസ്ഥ LUA_OPBXOR: തിരികെ intop(^, v1, v2);
    അവസ്ഥ LUA_OPSHL: തിരികെ luaV_shiftl(v1, v2);
    അവസ്ഥ LUA_OPSHR: തിരികെ luaV_shiftr(v1, v2);
    അവസ്ഥ LUA_OPUNM: തിരികെ intop(-, 0, v1);
    അവസ്ഥ LUA_OPBNOT: തിരികെ intop(^, ~l_castS2U(0), v1);
    സ്ഥിരസ്ഥിതി: lua_assert(0); തിരികെ 0;
  }
}


സ്ഥിതം lua_സംഖ്യ numarith (lua_State *L, പൂർണ്ണസംഖ്യ op, lua_സംഖ്യ v1,
                                                  lua_സംഖ്യ v2) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ LUA_OPADD: തിരികെ luai_numadd(L, v1, v2);
    അവസ്ഥ LUA_OPSUB: തിരികെ luai_numsub(L, v1, v2);
    അവസ്ഥ LUA_OPMUL: തിരികെ luai_nummul(L, v1, v2);
    അവസ്ഥ LUA_OPDIV: തിരികെ luai_numdiv(L, v1, v2);
    അവസ്ഥ LUA_OPPOW: തിരികെ luai_numpow(L, v1, v2);
    അവസ്ഥ LUA_OPIDIV: തിരികെ luai_numidiv(L, v1, v2);
    അവസ്ഥ LUA_OPUNM: തിരികെ luai_numunm(L, v1);
    അവസ്ഥ LUA_OPMOD: തിരികെ luaV_modf(L, v1, v2);
    സ്ഥിരസ്ഥിതി: lua_assert(0); തിരികെ 0;
  }
}


പൂർണ്ണസംഖ്യ luaO_rawarith (lua_State *L, പൂർണ്ണസംഖ്യ op, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                   TValue *res) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ LUA_OPBAND: അവസ്ഥ LUA_OPBOR: അവസ്ഥ LUA_OPBXOR:
    അവസ്ഥ LUA_OPSHL: അവസ്ഥ LUA_OPSHR:
    അവസ്ഥ LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      എങ്കിൽ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ തിരികെ 0;  /* fail */
    }
    അവസ്ഥ LUA_OPDIV: അവസ്ഥ LUA_OPPOW: {  /* operate only on floats */
      lua_സംഖ്യ n1; lua_സംഖ്യ n2;
      എങ്കിൽ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ തിരികെ 0;  /* fail */
    }
    സ്ഥിരസ്ഥിതി: {  /* other operations */
      lua_സംഖ്യ n1; lua_സംഖ്യ n2;
      എങ്കിൽ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ എങ്കിൽ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ തിരികെ 0;  /* fail */
    }
  }
}


ശൂന്യം luaO_arith (lua_State *L, പൂർണ്ണസംഖ്യ op, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                 StkId res) {
  എങ്കിൽ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (പൂർണ്ണസംഖ്യ c) {
  lua_assert(lisxdigit(c));
  എങ്കിൽ (lisdigit(c)) തിരികെ cast_byte(c - '0');
  അല്ലെങ്കിൽ തിരികെ cast_byte((ltolower(c) - 'a') + 10);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ isneg (സ്ഥിരം അക്ഷരം **s) {
  എങ്കിൽ (**s == '-') { (*s)++; തിരികെ 1; }
  അല്ലെങ്കിൽ എങ്കിൽ (**s == '+') (*s)++;
  തിരികെ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#എങ്കിൽ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
സ്ഥിതം lua_സംഖ്യ lua_strx2number (സ്ഥിരം അക്ഷരം *s, അക്ഷരം **endptr) {
  പൂർണ്ണസംഖ്യ dot = lua_getlocaledecpoint();
  lua_സംഖ്യ r = l_mathop(0.0);  /* result (accumulator) */
  പൂർണ്ണസംഖ്യ sigdig = 0;  /* number of significant digits */
  പൂർണ്ണസംഖ്യ nosigdig = 0;  /* number of non-significant digits */
  പൂർണ്ണസംഖ്യ e = 0;  /* exponent correction */
  പൂർണ്ണസംഖ്യ neg;  /* 1 if number is negative */
  പൂർണ്ണസംഖ്യ hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ആകുമ്പോൾ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  എങ്കിൽ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    തിരികെ l_mathop(0.0);  /* invalid format (no '0x') */
  വേണ്ടി (s += 2; ; s++) {  /* skip '0x' and read numeral */
    എങ്കിൽ (*s == dot) {
      എങ്കിൽ (hasdot) നിർത്തുക;  /* second dot? stop loop */
      അല്ലെങ്കിൽ hasdot = 1;
    }
    അല്ലെങ്കിൽ എങ്കിൽ (lisxdigit(cast_uchar(*s))) {
      എങ്കിൽ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      അല്ലെങ്കിൽ എങ്കിൽ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      അല്ലെങ്കിൽ e++;  /* too many digits; ignore, but still count for exponent */
      എങ്കിൽ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    അല്ലെങ്കിൽ നിർത്തുക;  /* neither a dot nor a digit */
  }
  എങ്കിൽ (nosigdig + sigdig == 0)  /* no digits? */
    തിരികെ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  എങ്കിൽ (*s == 'p' || *s == 'P') {  /* exponent part? */
    പൂർണ്ണസംഖ്യ exp1 = 0;  /* exponent value */
    പൂർണ്ണസംഖ്യ neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    എങ്കിൽ (!lisdigit(cast_uchar(*s)))
      തിരികെ l_mathop(0.0);  /* invalid; must have at least one digit */
    ആകുമ്പോൾ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    എങ്കിൽ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  എങ്കിൽ (neg) r = -r;
  തിരികെ l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#എങ്കിൽ !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *l_str2dloc (സ്ഥിരം അക്ഷരം *s, lua_സംഖ്യ *result, പൂർണ്ണസംഖ്യ mode) {
  അക്ഷരം *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  എങ്കിൽ (endptr == s) തിരികെ ശൂന്യ;  /* nothing recognized? */
  ആകുമ്പോൾ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  തിരികെ (*endptr == '\0') ? endptr : ശൂന്യ;  /* OK iff no trailing chars */
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
സ്ഥിതം സ്ഥിരം അക്ഷരം *l_str2d (സ്ഥിരം അക്ഷരം *s, lua_സംഖ്യ *result) {
  സ്ഥിരം അക്ഷരം *endptr;
  സ്ഥിരം അക്ഷരം *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  പൂർണ്ണസംഖ്യ mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  എങ്കിൽ (mode == 'n')  /* reject 'inf' and 'nan' */
    തിരികെ ശൂന്യ;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  എങ്കിൽ (endptr == ശൂന്യ) {  /* failed? may be a different locale */
    അക്ഷരം buff[L_MAXLENNUM + 1];
    സ്ഥിരം അക്ഷരം *pdot = strchr(s, '.');
    എങ്കിൽ (pdot == ശൂന്യ || strlen(s) > L_MAXLENNUM)
      തിരികെ ശൂന്യ;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    എങ്കിൽ (endptr != ശൂന്യ)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  തിരികെ endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

സ്ഥിതം സ്ഥിരം അക്ഷരം *l_str2int (സ്ഥിരം അക്ഷരം *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  പൂർണ്ണസംഖ്യ empty = 1;
  പൂർണ്ണസംഖ്യ neg;
  ആകുമ്പോൾ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  എങ്കിൽ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    വേണ്ടി (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  അല്ലെങ്കിൽ {  /* decimal */
    വേണ്ടി (; lisdigit(cast_uchar(*s)); s++) {
      പൂർണ്ണസംഖ്യ d = *s - '0';
      എങ്കിൽ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        തിരികെ ശൂന്യ;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  ആകുമ്പോൾ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  എങ്കിൽ (empty || *s != '\0') തിരികെ ശൂന്യ;  /* something wrong in the numeral */
  അല്ലെങ്കിൽ {
    *result = l_castU2S((neg) ? 0u - a : a);
    തിരികെ s;
  }
}


size_t luaO_str2num (സ്ഥിരം അക്ഷരം *s, TValue *o) {
  lua_Integer i; lua_സംഖ്യ n;
  സ്ഥിരം അക്ഷരം *e;
  എങ്കിൽ ((e = l_str2int(s, &i)) != ശൂന്യ) {  /* try as an integer */
    setivalue(o, i);
  }
  അല്ലെങ്കിൽ എങ്കിൽ ((e = l_str2d(s, &n)) != ശൂന്യ) {  /* else try as a float */
    setfltvalue(o, n);
  }
  അല്ലെങ്കിൽ
    തിരികെ 0;  /* conversion failed */
  തിരികെ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


പൂർണ്ണസംഖ്യ luaO_utf8esc (അക്ഷരം *buff, l_uint32 x) {
  പൂർണ്ണസംഖ്യ n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  എങ്കിൽ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  അല്ലെങ്കിൽ {  /* need continuation bytes */
    unsigned പൂർണ്ണസംഖ്യ mfb = 0x3f;  /* maximum that fits in first byte */
    ചെയ്യുക {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ആകുമ്പോൾ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  തിരികെ n;
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
#എങ്കിൽ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
സ്ഥിതം പൂർണ്ണസംഖ്യ tostringbuffFloat (lua_സംഖ്യ n, അക്ഷരം *buff) {
  /* first conversion */
  പൂർണ്ണസംഖ്യ len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_സംഖ്യ_FMT,
                            (LUAI_UACNUMBER)n);
  lua_സംഖ്യ check = lua_str2number(buff, ശൂന്യ);  /* read it back */
  എങ്കിൽ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_സംഖ്യ_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  എങ്കിൽ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  തിരികെ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (സ്ഥിരം TValue *obj, അക്ഷരം *buff) {
  പൂർണ്ണസംഖ്യ len;
  lua_assert(ttisnumber(obj));
  എങ്കിൽ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  അല്ലെങ്കിൽ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  തിരികെ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
ശൂന്യം luaO_tostring (lua_State *L, TValue *obj) {
  അക്ഷരം buff[LUA_N2SBUFFSZ];
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
തരനിർവചനം ഘടന BuffFS {
  lua_State *L;
  അക്ഷരം *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  പൂർണ്ണസംഖ്യ err;
  അക്ഷരം space[BUFVFS];  /* initial buffer */
} BuffFS;


സ്ഥിതം ശൂന്യം initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = വലുപ്പം(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
സ്ഥിതം ശൂന്യം pushbuff (lua_State *L, ശൂന്യം *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  തിരഞ്ഞെടുപ്പ് (buff->err) {
    അവസ്ഥ 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      നിർത്തുക;
    അവസ്ഥ 2:  /* length overflow: Add "..." at the end of result */
      എങ്കിൽ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      അല്ലെങ്കിൽ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    സ്ഥിരസ്ഥിതി: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->മുകൾ.p, ts);
      L->മുകൾ.p++;
    }
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  സ്ഥിരം അക്ഷരം *res;
  എങ്കിൽ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ശൂന്യ;  /* error message is on the top of the stack */
  അല്ലെങ്കിൽ
    res = getstr(tsvalue(s2v(L->മുകൾ.p - 1)));
  എങ്കിൽ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  തിരികെ res;
}


സ്ഥിതം ശൂന്യം addstr2buff (BuffFS *buff, സ്ഥിരം അക്ഷരം *str, size_t slen) {
  size_t ഇടത് = buff->buffsize - buff->blen;  /* space left in the buffer */
  എങ്കിൽ (buff->err)  /* do nothing else after an error */
    തിരികെ;
  എങ്കിൽ (slen > ഇടത്) {  /* new string doesn't fit into current buffer? */
    എങ്കിൽ (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, ഇടത്);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      തിരികെ;
    }
    അല്ലെങ്കിൽ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      അക്ഷരം *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ശൂന്യ, 0, newsize, അക്ഷരം)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               അക്ഷരം);
      എങ്കിൽ (newb == ശൂന്യ) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        തിരികെ;
      }
      എങ്കിൽ (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
സ്ഥിതം ശൂന്യം addnum2buff (BuffFS *buff, TValue *അക്കം) {
  അക്ഷരം numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(അക്കം, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
സ്ഥിരം അക്ഷരം *luaO_pushvfstring (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  സ്ഥിരം അക്ഷരം *e;  /* points to next '%' */
  initbuff(L, &buff);
  ആകുമ്പോൾ ((e = strchr(fmt, '%')) != ശൂന്യ) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    തിരഞ്ഞെടുപ്പ് (*(e + 1)) {  /* conversion specifier */
      അവസ്ഥ 's': {  /* zero-terminated string */
        സ്ഥിരം അക്ഷരം *s = va_arg(argp, അക്ഷരം *);
        എങ്കിൽ (s == ശൂന്യ) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        നിർത്തുക;
      }
      അവസ്ഥ 'c': {  /* an 'int' as a character */
        അക്ഷരം c = cast_char(va_arg(argp, പൂർണ്ണസംഖ്യ));
        addstr2buff(&buff, &c, വലുപ്പം(അക്ഷരം));
        നിർത്തുക;
      }
      അവസ്ഥ 'd': {  /* an 'int' */
        TValue അക്കം;
        setivalue(&അക്കം, va_arg(argp, പൂർണ്ണസംഖ്യ));
        addnum2buff(&buff, &അക്കം);
        നിർത്തുക;
      }
      അവസ്ഥ 'I': {  /* a 'lua_Integer' */
        TValue അക്കം;
        setivalue(&അക്കം, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &അക്കം);
        നിർത്തുക;
      }
      അവസ്ഥ 'f': {  /* a 'lua_Number' */
        TValue അക്കം;
        setfltvalue(&അക്കം, cast_അക്കം(va_arg(argp, l_uac_സംഖ്യ)));
        addnum2buff(&buff, &അക്കം);
        നിർത്തുക;
      }
      അവസ്ഥ 'p': {  /* a pointer */
        അക്ഷരം bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        ശൂന്യം *p = va_arg(argp, ശൂന്യം *);
        പൂർണ്ണസംഖ്യ len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        നിർത്തുക;
      }
      അവസ്ഥ 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        അക്ഷരം bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        പൂർണ്ണസംഖ്യ len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        നിർത്തുക;
      }
      അവസ്ഥ '%': {
        addstr2buff(&buff, "%", 1);
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        നിർത്തുക;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  തിരികെ clearbuff(&buff);  /* empty buffer into a new string */
}


സ്ഥിരം അക്ഷരം *luaO_pushfstring (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം;
  va_list argp;
  va_start(argp, fmt);
  സന്ദേശം = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  എങ്കിൽ (സന്ദേശം == ശൂന്യ)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  തിരികെ സന്ദേശം;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * വലുപ്പം(അക്ഷരം)), a += (l) )

ശൂന്യം luaO_chunkid (അക്ഷരം *out, സ്ഥിരം അക്ഷരം *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  എങ്കിൽ (*source == '=') {  /* 'literal' source */
    എങ്കിൽ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * വലുപ്പം(അക്ഷരം));
    അല്ലെങ്കിൽ {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  അല്ലെങ്കിൽ എങ്കിൽ (*source == '@') {  /* file name */
    എങ്കിൽ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * വലുപ്പം(അക്ഷരം));
    അല്ലെങ്കിൽ {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * വലുപ്പം(അക്ഷരം));
    }
  }
  അല്ലെങ്കിൽ {  /* string; format as [string "source"] */
    സ്ഥിരം അക്ഷരം *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    എങ്കിൽ (srclen < bufflen && nl == ശൂന്യ) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    അല്ലെങ്കിൽ {
      എങ്കിൽ (nl != ശൂന്യ)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      എങ്കിൽ (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * വലുപ്പം(അക്ഷരം));
  }
}

