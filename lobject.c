/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <float.h>
#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lctype.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା x) {
  ସ୍ଥିର ଅଚଳ lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା l = 0;
  x--;
  ଯେତେବେଳେ (x >= 256) { l += 8; x >>= 8; }
  ଫେରନ୍ତୁ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା p) {
  ଯଦି (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ଫେରନ୍ତୁ 0xFF;  /* return maximum value */
  ଅନ୍ୟଥା {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ଯଦି (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ଫେରନ୍ତୁ cast_byte(p);
    }
    ଅନ୍ୟଥା {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      ଫେରନ୍ତୁ cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = p & 0xF;  /* mantissa */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ଯଦି (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ଯଦି (e >= 0) {
    ଯଦି (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ଫେରନ୍ତୁ (x * m) << e;  /* order doesn't matter here */
    ଅନ୍ୟଥା  /* real overflow */
      ଫେରନ୍ତୁ MAX_LMEM;
  }
  ଅନ୍ୟଥା {  /* negative exponent */
    e = -e;
    ଯଦି (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ଫେରନ୍ତୁ (x * m) >> e;  /* multiplying first gives more precision */
    ଅନ୍ୟଥା ଯଦି ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ଫେରନ୍ତୁ (x >> e) * m;
    ଅନ୍ୟଥା  /* real overflow */
      ଫେରନ୍ତୁ MAX_LMEM;
  }
}


ସ୍ଥିର lua_Integer intarith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, lua_Integer v1,
                                                   lua_Integer v2) {
  ବାଛ (op) {
    ସ୍ଥିତି LUA_OPADD: ଫେରନ୍ତୁ intop(+, v1, v2);
    ସ୍ଥିତି LUA_OPSUB:ଫେରନ୍ତୁ intop(-, v1, v2);
    ସ୍ଥିତି LUA_OPMUL:ଫେରନ୍ତୁ intop(*, v1, v2);
    ସ୍ଥିତି LUA_OPMOD: ଫେରନ୍ତୁ luaV_mod(L, v1, v2);
    ସ୍ଥିତି LUA_OPIDIV: ଫେରନ୍ତୁ luaV_idiv(L, v1, v2);
    ସ୍ଥିତି LUA_OPBAND: ଫେରନ୍ତୁ intop(&, v1, v2);
    ସ୍ଥିତି LUA_OPBOR: ଫେରନ୍ତୁ intop(|, v1, v2);
    ସ୍ଥିତି LUA_OPBXOR: ଫେରନ୍ତୁ intop(^, v1, v2);
    ସ୍ଥିତି LUA_OPSHL: ଫେରନ୍ତୁ luaV_shiftl(v1, v2);
    ସ୍ଥିତି LUA_OPSHR: ଫେରନ୍ତୁ luaV_shiftr(v1, v2);
    ସ୍ଥିତି LUA_OPUNM: ଫେରନ୍ତୁ intop(-, 0, v1);
    ସ୍ଥିତି LUA_OPBNOT: ଫେରନ୍ତୁ intop(^, ~l_castS2U(0), v1);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: lua_assert(0); ଫେରନ୍ତୁ 0;
  }
}


ସ୍ଥିର lua_ସଂଖ୍ୟା numarith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, lua_ସଂଖ୍ୟା v1,
                                                  lua_ସଂଖ୍ୟା v2) {
  ବାଛ (op) {
    ସ୍ଥିତି LUA_OPADD: ଫେରନ୍ତୁ luai_numadd(L, v1, v2);
    ସ୍ଥିତି LUA_OPSUB: ଫେରନ୍ତୁ luai_numsub(L, v1, v2);
    ସ୍ଥିତି LUA_OPMUL: ଫେରନ୍ତୁ luai_nummul(L, v1, v2);
    ସ୍ଥିତି LUA_OPDIV: ଫେରନ୍ତୁ luai_numdiv(L, v1, v2);
    ସ୍ଥିତି LUA_OPPOW: ଫେରନ୍ତୁ luai_numpow(L, v1, v2);
    ସ୍ଥିତି LUA_OPIDIV: ଫେରନ୍ତୁ luai_numidiv(L, v1, v2);
    ସ୍ଥିତି LUA_OPUNM: ଫେରନ୍ତୁ luai_numunm(L, v1);
    ସ୍ଥିତି LUA_OPMOD: ଫେରନ୍ତୁ luaV_modf(L, v1, v2);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: lua_assert(0); ଫେରନ୍ତୁ 0;
  }
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaO_rawarith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                   TValue *res) {
  ବାଛ (op) {
    ସ୍ଥିତି LUA_OPBAND: ସ୍ଥିତି LUA_OPBOR: ସ୍ଥିତି LUA_OPBXOR:
    ସ୍ଥିତି LUA_OPSHL: ସ୍ଥିତି LUA_OPSHR:
    ସ୍ଥିତି LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ଯଦି (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;  /* fail */
    }
    ସ୍ଥିତି LUA_OPDIV: ସ୍ଥିତି LUA_OPPOW: {  /* operate only on floats */
      lua_ସଂଖ୍ୟା n1; lua_ସଂଖ୍ୟା n2;
      ଯଦି (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;  /* fail */
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  /* other operations */
      lua_ସଂଖ୍ୟା n1; lua_ସଂଖ୍ୟା n2;
      ଯଦି (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା ଯଦି (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;  /* fail */
    }
  }
}


ଶୂନ୍ୟ luaO_arith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                 StkId res) {
  ଯଦି (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  lua_assert(lisxdigit(c));
  ଯଦି (lisdigit(c)) ଫେରନ୍ତୁ cast_byte(c - '0');
  ଅନ୍ୟଥା ଫେରନ୍ତୁ cast_byte((ltolower(c) - 'a') + 10);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isneg (ଅଚଳ ଅକ୍ଷର **s) {
  ଯଦି (**s == '-') { (*s)++; ଫେରନ୍ତୁ 1; }
  ଅନ୍ୟଥା ଯଦି (**s == '+') (*s)++;
  ଫେରନ୍ତୁ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ଯଦି !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
ସ୍ଥିର lua_ସଂଖ୍ୟା lua_strx2number (ଅଚଳ ଅକ୍ଷର *s, ଅକ୍ଷର **endptr) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dot = lua_getlocaledecpoint();
  lua_ସଂଖ୍ୟା r = l_mathop(0.0);  /* result (accumulator) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sigdig = 0;  /* number of significant digits */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nosigdig = 0;  /* number of non-significant digits */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା e = 0;  /* exponent correction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା neg;  /* 1 if number is negative */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ଯେତେବେଳେ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ଯଦି (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ଫେରନ୍ତୁ l_mathop(0.0);  /* invalid format (no '0x') */
  ପାଇଁ (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ଯଦି (*s == dot) {
      ଯଦି (hasdot) ଭାଙ୍ଗ;  /* second dot? stop loop */
      ଅନ୍ୟଥା hasdot = 1;
    }
    ଅନ୍ୟଥା ଯଦି (lisxdigit(cast_uchar(*s))) {
      ଯଦି (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      ଅନ୍ୟଥା ଯଦି (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      ଅନ୍ୟଥା e++;  /* too many digits; ignore, but still count for exponent */
      ଯଦି (hasdot) e--;  /* decimal digit? correct exponent */
    }
    ଅନ୍ୟଥା ଭାଙ୍ଗ;  /* neither a dot nor a digit */
  }
  ଯଦି (nosigdig + sigdig == 0)  /* no digits? */
    ଫେରନ୍ତୁ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ଯଦି (*s == 'p' || *s == 'P') {  /* exponent part? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା exp1 = 0;  /* exponent value */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ଯଦି (!lisdigit(cast_uchar(*s)))
      ଫେରନ୍ତୁ l_mathop(0.0);  /* invalid; must have at least one digit */
    ଯେତେବେଳେ (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ଯଦି (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ଯଦି (neg) r = -r;
  ଫେରନ୍ତୁ l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ଯଦି !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *l_str2dloc (ଅଚଳ ଅକ୍ଷର *s, lua_ସଂଖ୍ୟା *result, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
  ଅକ୍ଷର *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ଯଦି (endptr == s) ଫେରନ୍ତୁ ଖାଲି;  /* nothing recognized? */
  ଯେତେବେଳେ (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ଫେରନ୍ତୁ (*endptr == '\0') ? endptr : ଖାଲି;  /* OK iff no trailing chars */
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
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *l_str2d (ଅଚଳ ଅକ୍ଷର *s, lua_ସଂଖ୍ୟା *result) {
  ଅଚଳ ଅକ୍ଷର *endptr;
  ଅଚଳ ଅକ୍ଷର *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ଯଦି (mode == 'n')  /* reject 'inf' and 'nan' */
    ଫେରନ୍ତୁ ଖାଲି;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  ଯଦି (endptr == ଖାଲି) {  /* failed? may be a different locale */
    ଅକ୍ଷର buff[L_MAXLENNUM + 1];
    ଅଚଳ ଅକ୍ଷର *pdot = strchr(s, '.');
    ଯଦି (pdot == ଖାଲି || strlen(s) > L_MAXLENNUM)
      ଫେରନ୍ତୁ ଖାଲି;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    ଯଦି (endptr != ଖାଲି)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ଫେରନ୍ତୁ endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *l_str2int (ଅଚଳ ଅକ୍ଷର *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା empty = 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା neg;
  ଯେତେବେଳେ (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ଯଦି (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    ପାଇଁ (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  ଅନ୍ୟଥା {  /* decimal */
    ପାଇଁ (; lisdigit(cast_uchar(*s)); s++) {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା d = *s - '0';
      ଯଦି (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ଫେରନ୍ତୁ ଖାଲି;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  ଯେତେବେଳେ (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ଯଦି (empty || *s != '\0') ଫେରନ୍ତୁ ଖାଲି;  /* something wrong in the numeral */
  ଅନ୍ୟଥା {
    *result = l_castU2S((neg) ? 0u - a : a);
    ଫେରନ୍ତୁ s;
  }
}


size_t luaO_str2num (ଅଚଳ ଅକ୍ଷର *s, TValue *o) {
  lua_Integer i; lua_ସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *e;
  ଯଦି ((e = l_str2int(s, &i)) != ଖାଲି) {  /* try as an integer */
    setivalue(o, i);
  }
  ଅନ୍ୟଥା ଯଦି ((e = l_str2d(s, &n)) != ଖାଲି) {  /* else try as a float */
    setfltvalue(o, n);
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ 0;  /* conversion failed */
  ଫେରନ୍ତୁ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaO_utf8esc (ଅକ୍ଷର *buff, l_uint32 x) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ଯଦି (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  ଅନ୍ୟଥା {  /* need continuation bytes */
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mfb = 0x3f;  /* maximum that fits in first byte */
    କର {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ଯେତେବେଳେ (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ଫେରନ୍ତୁ n;
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
#ଯଦି LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tostringbuffFloat (lua_ସଂଖ୍ୟା n, ଅକ୍ଷର *buff) {
  /* first conversion */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ସଂଖ୍ୟା_FMT,
                            (LUAI_UACNUMBER)n);
  lua_ସଂଖ୍ୟା check = lua_str2number(buff, ଖାଲି);  /* read it back */
  ଯଦି (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_ସଂଖ୍ୟା_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ଯଦି (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  ଫେରନ୍ତୁ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (ଅଚଳ TValue *obj, ଅକ୍ଷର *buff) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
  lua_assert(ttisnumber(obj));
  ଯଦି (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  ଅନ୍ୟଥା
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ଫେରନ୍ତୁ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
ଶୂନ୍ୟ luaO_tostring (lua_State *L, TValue *obj) {
  ଅକ୍ଷର buff[LUA_N2SBUFFSZ];
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା BuffFS {
  lua_State *L;
  ଅକ୍ଷର *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
  ଅକ୍ଷର space[BUFVFS];  /* initial buffer */
} BuffFS;


ସ୍ଥିର ଶୂନ୍ୟ initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = ଆକାର(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
ସ୍ଥିର ଶୂନ୍ୟ pushbuff (lua_State *L, ଶୂନ୍ୟ *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  ବାଛ (buff->err) {
    ସ୍ଥିତି 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 2:  /* length overflow: Add "..." at the end of result */
      ଯଦି (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      ଅନ୍ୟଥା {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->ଶୀର୍ଷ.p, ts);
      L->ଶୀର୍ଷ.p++;
    }
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  ଅଚଳ ଅକ୍ଷର *res;
  ଯଦି (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = ଖାଲି;  /* error message is on the top of the stack */
  ଅନ୍ୟଥା
    res = getstr(tsvalue(s2v(L->ଶୀର୍ଷ.p - 1)));
  ଯଦି (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ଶୂନ୍ୟ addstr2buff (BuffFS *buff, ଅଚଳ ଅକ୍ଷର *str, size_t slen) {
  size_t ବାମ = buff->buffsize - buff->blen;  /* space left in the buffer */
  ଯଦି (buff->err)  /* do nothing else after an error */
    ଫେରନ୍ତୁ;
  ଯଦି (slen > ବାମ) {  /* new string doesn't fit into current buffer? */
    ଯଦି (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, ବାମ);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ଫେରନ୍ତୁ;
    }
    ଅନ୍ୟଥା {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      ଅକ୍ଷର *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, ଖାଲି, 0, newsize, ଅକ୍ଷର)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               ଅକ୍ଷର);
      ଯଦି (newb == ଖାଲି) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ଫେରନ୍ତୁ;
      }
      ଯଦି (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
ସ୍ଥିର ଶୂନ୍ୟ addnum2buff (BuffFS *buff, TValue *ଅଙ୍କ) {
  ଅକ୍ଷର numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(ଅଙ୍କ, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
ଅଚଳ ଅକ୍ଷର *luaO_pushvfstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  ଅଚଳ ଅକ୍ଷର *e;  /* points to next '%' */
  initbuff(L, &buff);
  ଯେତେବେଳେ ((e = strchr(fmt, '%')) != ଖାଲି) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    ବାଛ (*(e + 1)) {  /* conversion specifier */
      ସ୍ଥିତି 's': {  /* zero-terminated string */
        ଅଚଳ ଅକ୍ଷର *s = va_arg(argp, ଅକ୍ଷର *);
        ଯଦି (s == ଖାଲି) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'c': {  /* an 'int' as a character */
        ଅକ୍ଷର c = cast_char(va_arg(argp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
        addstr2buff(&buff, &c, ଆକାର(ଅକ୍ଷର));
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'd': {  /* an 'int' */
        TValue ଅଙ୍କ;
        setivalue(&ଅଙ୍କ, va_arg(argp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
        addnum2buff(&buff, &ଅଙ୍କ);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'I': {  /* a 'lua_Integer' */
        TValue ଅଙ୍କ;
        setivalue(&ଅଙ୍କ, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &ଅଙ୍କ);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'f': {  /* a 'lua_Number' */
        TValue ଅଙ୍କ;
        setfltvalue(&ଅଙ୍କ, cast_ଅଙ୍କ(va_arg(argp, l_uac_ସଂଖ୍ୟା)));
        addnum2buff(&buff, &ଅଙ୍କ);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'p': {  /* a pointer */
        ଅକ୍ଷର bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        ଶୂନ୍ୟ *p = va_arg(argp, ଶୂନ୍ୟ *);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        ଅକ୍ଷର bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '%': {
        addstr2buff(&buff, "%", 1);
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        ଭାଙ୍ଗ;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  ଫେରନ୍ତୁ clearbuff(&buff);  /* empty buffer into a new string */
}


ଅଚଳ ଅକ୍ଷର *luaO_pushfstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ;
  va_list argp;
  va_start(argp, fmt);
  ସନ୍ଦେଶ = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ଯଦି (ସନ୍ଦେଶ == ଖାଲି)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ଫେରନ୍ତୁ ସନ୍ଦେଶ;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * ଆକାର(ଅକ୍ଷର)), a += (l) )

ଶୂନ୍ୟ luaO_chunkid (ଅକ୍ଷର *out, ଅଚଳ ଅକ୍ଷର *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ଯଦି (*source == '=') {  /* 'literal' source */
    ଯଦି (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * ଆକାର(ଅକ୍ଷର));
    ଅନ୍ୟଥା {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  ଅନ୍ୟଥା ଯଦି (*source == '@') {  /* file name */
    ଯଦି (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * ଆକାର(ଅକ୍ଷର));
    ଅନ୍ୟଥା {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * ଆକାର(ଅକ୍ଷର));
    }
  }
  ଅନ୍ୟଥା {  /* string; format as [string "source"] */
    ଅଚଳ ଅକ୍ଷର *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ଯଦି (srclen < bufflen && nl == ଖାଲି) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    ଅନ୍ୟଥା {
      ଯଦି (nl != ଖାଲି)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      ଯଦି (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * ଆକାର(ଅକ୍ଷର));
  }
}

