/*
** $Id: lobject.c $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#define lobject_c
#define LUA_CORE

#include "lprefix.h"


#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lctype.h"
#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "lvm.h"


/*
** Computes ceil(log2(x)), which is the smallest integer n such that
** x <= (1 << n).
*/
lu_byte luaO_ceillog2 (unsigned ចំនួនគត់ x) {
  static const lu_byte log_2[256] = {  /* log_2[i - 1] = ceil(log2(i)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  ចំនួនគត់ l = 0;
  x--;
  ខណៈពេល (x >= 256) { l += 8; x >>= 8; }
  ត្រឡប់ cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned ចំនួនគត់ p) {
  ប្រសិនបើ (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    ត្រឡប់ 0xFF;  /* return maximum value */
  បើមិនដូច្នេះ {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    ប្រសិនបើ (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      ត្រឡប់ cast_byte(p);
    }
    បើមិនដូច្នេះ {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      ត្រឡប់ cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  ចំនួនគត់ m = p & 0xF;  /* mantissa */
  ចំនួនគត់ e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  ប្រសិនបើ (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  ប្រសិនបើ (e >= 0) {
    ប្រសិនបើ (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      ត្រឡប់ (x * m) << e;  /* order doesn't matter here */
    បើមិនដូច្នេះ  /* real overflow */
      ត្រឡប់ MAX_LMEM;
  }
  បើមិនដូច្នេះ {  /* negative exponent */
    e = -e;
    ប្រសិនបើ (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      ត្រឡប់ (x * m) >> e;  /* multiplying first gives more precision */
    បើមិនដូច្នេះ ប្រសិនបើ ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      ត្រឡប់ (x >> e) * m;
    បើមិនដូច្នេះ  /* real overflow */
      ត្រឡប់ MAX_LMEM;
  }
}


static lua_Integer intarith (lua_State *L, ចំនួនគត់ op, lua_Integer v1,
                                                   lua_Integer v2) {
  switch (op) {
    pak_sa LUA_OPADD: ត្រឡប់ intop(+, v1, v2);
    pak_sa LUA_OPSUB:ត្រឡប់ intop(-, v1, v2);
    pak_sa LUA_OPMUL:ត្រឡប់ intop(*, v1, v2);
    pak_sa LUA_OPMOD: ត្រឡប់ luaV_mod(L, v1, v2);
    pak_sa LUA_OPIDIV: ត្រឡប់ luaV_idiv(L, v1, v2);
    pak_sa LUA_OPBAND: ត្រឡប់ intop(&, v1, v2);
    pak_sa LUA_OPBOR: ត្រឡប់ intop(|, v1, v2);
    pak_sa LUA_OPBXOR: ត្រឡប់ intop(^, v1, v2);
    pak_sa LUA_OPSHL: ត្រឡប់ luaV_shiftl(v1, v2);
    pak_sa LUA_OPSHR: ត្រឡប់ luaV_shiftr(v1, v2);
    pak_sa LUA_OPUNM: ត្រឡប់ intop(-, 0, v1);
    pak_sa LUA_OPBNOT: ត្រឡប់ intop(^, ~l_castS2U(0), v1);
    default: lua_assert(0); ត្រឡប់ 0;
  }
}


static lua_Number numarith (lua_State *L, ចំនួនគត់ op, lua_Number v1,
                                                  lua_Number v2) {
  switch (op) {
    pak_sa LUA_OPADD: ត្រឡប់ luai_numadd(L, v1, v2);
    pak_sa LUA_OPSUB: ត្រឡប់ luai_numsub(L, v1, v2);
    pak_sa LUA_OPMUL: ត្រឡប់ luai_nummul(L, v1, v2);
    pak_sa LUA_OPDIV: ត្រឡប់ luai_numdiv(L, v1, v2);
    pak_sa LUA_OPPOW: ត្រឡប់ luai_numpow(L, v1, v2);
    pak_sa LUA_OPIDIV: ត្រឡប់ luai_numidiv(L, v1, v2);
    pak_sa LUA_OPUNM: ត្រឡប់ luai_numunm(L, v1);
    pak_sa LUA_OPMOD: ត្រឡប់ luaV_modf(L, v1, v2);
    default: lua_assert(0); ត្រឡប់ 0;
  }
}


ចំនួនគត់ luaO_rawarith (lua_State *L, ចំនួនគត់ op, const TValue *p1, const TValue *p2,
                   TValue *res) {
  switch (op) {
    pak_sa LUA_OPBAND: pak_sa LUA_OPBOR: pak_sa LUA_OPBXOR:
    pak_sa LUA_OPSHL: pak_sa LUA_OPSHR:
    pak_sa LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      ប្រសិនបើ (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ ត្រឡប់ 0;  /* fail */
    }
    pak_sa LUA_OPDIV: pak_sa LUA_OPPOW: {  /* operate only on floats */
      lua_Number n1; lua_Number n2;
      ប្រសិនបើ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ ត្រឡប់ 0;  /* fail */
    }
    default: {  /* other operations */
      lua_Number n1; lua_Number n2;
      ប្រសិនបើ (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ ប្រសិនបើ (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ ត្រឡប់ 0;  /* fail */
    }
  }
}


void luaO_arith (lua_State *L, ចំនួនគត់ op, const TValue *p1, const TValue *p2,
                 StkId res) {
  ប្រសិនបើ (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (ចំនួនគត់ c) {
  lua_assert(lisxdigit(c));
  ប្រសិនបើ (lisdigit(c)) ត្រឡប់ cast_byte(c - '0');
  បើមិនដូច្នេះ ត្រឡប់ cast_byte((ltolower(c) - 'a') + 10);
}


static ចំនួនគត់ isneg (const var_na **s) {
  ប្រសិនបើ (**s == '-') { (*s)++; ត្រឡប់ 1; }
  បើមិនដូច្នេះ ប្រសិនបើ (**s == '+') (*s)++;
  ត្រឡប់ 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#ប្រសិនបើ !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
static lua_Number lua_strx2number (const var_na *s, var_na **endptr) {
  ចំនួនគត់ dot = lua_getlocaledecpoint();
  lua_Number r = l_mathop(0.0);  /* result (accumulator) */
  ចំនួនគត់ sigdig = 0;  /* number of significant digits */
  ចំនួនគត់ nosigdig = 0;  /* number of non-significant digits */
  ចំនួនគត់ e = 0;  /* exponent correction */
  ចំនួនគត់ neg;  /* 1 if number is negative */
  ចំនួនគត់ hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  ខណៈពេល (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  ប្រសិនបើ (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    ត្រឡប់ l_mathop(0.0);  /* invalid format (no '0x') */
  សម្រាប់ (s += 2; ; s++) {  /* skip '0x' and read numeral */
    ប្រសិនបើ (*s == dot) {
      ប្រសិនបើ (hasdot) bha_nga;  /* second dot? stop loop */
      បើមិនដូច្នេះ hasdot = 1;
    }
    បើមិនដូច្នេះ ប្រសិនបើ (lisxdigit(cast_uchar(*s))) {
      ប្រសិនបើ (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      បើមិនដូច្នេះ ប្រសិនបើ (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      បើមិនដូច្នេះ e++;  /* too many digits; ignore, but still count for exponent */
      ប្រសិនបើ (hasdot) e--;  /* decimal digit? correct exponent */
    }
    បើមិនដូច្នេះ bha_nga;  /* neither a dot nor a digit */
  }
  ប្រសិនបើ (nosigdig + sigdig == 0)  /* no digits? */
    ត្រឡប់ l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  ប្រសិនបើ (*s == 'p' || *s == 'P') {  /* exponent part? */
    ចំនួនគត់ exp1 = 0;  /* exponent value */
    ចំនួនគត់ neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    ប្រសិនបើ (!lisdigit(cast_uchar(*s)))
      ត្រឡប់ l_mathop(0.0);  /* invalid; must have at least one digit */
    ខណៈពេល (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    ប្រសិនបើ (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  ប្រសិនបើ (neg) r = -r;
  ត្រឡប់ l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#ប្រសិនបើ !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
static const var_na *l_str2dloc (const var_na *s, lua_Number *result, ចំនួនគត់ mode) {
  var_na *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  ប្រសិនបើ (endptr == s) ត្រឡប់ NULL;  /* nothing recognized? */
  ខណៈពេល (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  ត្រឡប់ (*endptr == '\0') ? endptr : NULL;  /* OK iff no trailing chars */
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
static const var_na *l_str2d (const var_na *s, lua_Number *result) {
  const var_na *endptr;
  const var_na *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ចំនួនគត់ mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  ប្រសិនបើ (mode == 'n')  /* reject 'inf' and 'nan' */
    ត្រឡប់ NULL;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  ប្រសិនបើ (endptr == NULL) {  /* failed? may be a different locale */
    var_na buff[L_MAXLENNUM + 1];
    const var_na *pdot = strchr(s, '.');
    ប្រសិនបើ (pdot == NULL || strlen(s) > L_MAXLENNUM)
      ត្រឡប់ NULL;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    ប្រសិនបើ (endptr != NULL)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  ត្រឡប់ endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

static const var_na *l_str2int (const var_na *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  ចំនួនគត់ empty = 1;
  ចំនួនគត់ neg;
  ខណៈពេល (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  ប្រសិនបើ (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    សម្រាប់ (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  បើមិនដូច្នេះ {  /* decimal */
    សម្រាប់ (; lisdigit(cast_uchar(*s)); s++) {
      ចំនួនគត់ d = *s - '0';
      ប្រសិនបើ (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        ត្រឡប់ NULL;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  ខណៈពេល (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  ប្រសិនបើ (empty || *s != '\0') ត្រឡប់ NULL;  /* something wrong in the numeral */
  បើមិនដូច្នេះ {
    *result = l_castU2S((neg) ? 0u - a : a);
    ត្រឡប់ s;
  }
}


size_t luaO_str2num (const var_na *s, TValue *o) {
  lua_Integer i; lua_Number n;
  const var_na *e;
  ប្រសិនបើ ((e = l_str2int(s, &i)) != NULL) {  /* try as an integer */
    setivalue(o, i);
  }
  បើមិនដូច្នេះ ប្រសិនបើ ((e = l_str2d(s, &n)) != NULL) {  /* else try as a float */
    setfltvalue(o, n);
  }
  បើមិនដូច្នេះ
    ត្រឡប់ 0;  /* conversion failed */
  ត្រឡប់ ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ចំនួនគត់ luaO_utf8esc (var_na *buff, l_uint32 x) {
  ចំនួនគត់ n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  ប្រសិនបើ (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  បើមិនដូច្នេះ {  /* need continuation bytes */
    unsigned ចំនួនគត់ mfb = 0x3f;  /* maximum that fits in first byte */
    do {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } ខណៈពេល (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  ត្រឡប់ n;
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
#ប្រសិនបើ LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
static ចំនួនគត់ tostringbuffFloat (lua_Number n, var_na *buff) {
  /* first conversion */
  ចំនួនគត់ len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT,
                            (LUAI_UACNUMBER)n);
  lua_Number check = lua_str2number(buff, NULL);  /* read it back */
  ប្រសិនបើ (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  ប្រសិនបើ (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  ត្រឡប់ len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (const TValue *obj, var_na *buff) {
  ចំនួនគត់ len;
  lua_assert(ttisnumber(obj));
  ប្រសិនបើ (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  បើមិនដូច្នេះ
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  ត្រឡប់ cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
void luaO_tostring (lua_State *L, TValue *obj) {
  var_na buff[LUA_N2SBUFFSZ];
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
typedef sa_nracanaa BuffFS {
  lua_State *L;
  var_na *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ចំនួនគត់ err;
  var_na space[BUFVFS];  /* initial buffer */
} BuffFS;


static void initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = parimaa_na(buff->space);
  buff->blen = 0;
  buff->err = 0;
}


/*
** Push final result from 'luaO_pushvfstring'. This function may raise
** errors explicitly or through memory errors, so it must run protected.
*/
static void pushbuff (lua_State *L, void *ud) {
  BuffFS *buff = cast(BuffFS*, ud);
  switch (buff->err) {
    pak_sa 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      bha_nga;
    pak_sa 2:  /* length overflow: Add "..." at the end of result */
      ប្រសិនបើ (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      បើមិនដូច្នេះ {  /* there is enough space left for the "..." */
        strcpy(buff->b + buff->blen, "...");
        buff->blen += 3;
      }
      /* FALLTHROUGH */
    default: {  /* no errors, but it can raise one creating the new string */
      TString *ts = luaS_newlstr(L, buff->b, buff->blen);
      setsvalue2s(L, L->top.p, ts);
      L->top.p++;
    }
  }
}


static const var_na *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  const var_na *res;
  ប្រសិនបើ (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = NULL;  /* error message is on the top of the stack */
  បើមិនដូច្នេះ
    res = getstr(tsvalue(s2v(L->top.p - 1)));
  ប្រសិនបើ (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  ត្រឡប់ res;
}


static void addstr2buff (BuffFS *buff, const var_na *str, size_t slen) {
  size_t left = buff->buffsize - buff->blen;  /* space left in the buffer */
  ប្រសិនបើ (buff->err)  /* do nothing else after an error */
    ត្រឡប់;
  ប្រសិនបើ (slen > left) {  /* new string doesn't fit into current buffer? */
    ប្រសិនបើ (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, left);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      ត្រឡប់;
    }
    បើមិនដូច្នេះ {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      var_na *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, NULL, 0, newsize, var_na)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               var_na);
      ប្រសិនបើ (newb == NULL) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        ត្រឡប់;
      }
      ប្រសិនបើ (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
static void addnum2buff (BuffFS *buff, TValue *num) {
  var_na numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(num, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
const var_na *luaO_pushvfstring (lua_State *L, const var_na *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  const var_na *e;  /* points to next '%' */
  initbuff(L, &buff);
  ខណៈពេល ((e = strchr(fmt, '%')) != NULL) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    switch (*(e + 1)) {  /* conversion specifier */
      pak_sa 's': {  /* zero-terminated string */
        const var_na *s = va_arg(argp, var_na *);
        ប្រសិនបើ (s == NULL) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        bha_nga;
      }
      pak_sa 'c': {  /* an 'int' as a character */
        var_na c = cast_char(va_arg(argp, ចំនួនគត់));
        addstr2buff(&buff, &c, parimaa_na(var_na));
        bha_nga;
      }
      pak_sa 'd': {  /* an 'int' */
        TValue num;
        setivalue(&num, va_arg(argp, ចំនួនគត់));
        addnum2buff(&buff, &num);
        bha_nga;
      }
      pak_sa 'I': {  /* a 'lua_Integer' */
        TValue num;
        setivalue(&num, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &num);
        bha_nga;
      }
      pak_sa 'f': {  /* a 'lua_Number' */
        TValue num;
        setfltvalue(&num, cast_num(va_arg(argp, l_uacNumber)));
        addnum2buff(&buff, &num);
        bha_nga;
      }
      pak_sa 'p': {  /* a pointer */
        var_na bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        void *p = va_arg(argp, void *);
        ចំនួនគត់ len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        bha_nga;
      }
      pak_sa 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        var_na bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        ចំនួនគត់ len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        bha_nga;
      }
      pak_sa '%': {
        addstr2buff(&buff, "%", 1);
        bha_nga;
      }
      default: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        bha_nga;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  ត្រឡប់ clearbuff(&buff);  /* empty buffer into a new string */
}


const var_na *luaO_pushfstring (lua_State *L, const var_na *fmt, ...) {
  const var_na *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  ប្រសិនបើ (msg == NULL)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  ត្រឡប់ msg;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * parimaa_na(var_na)), a += (l) )

void luaO_chunkid (var_na *out, const var_na *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  ប្រសិនបើ (*source == '=') {  /* 'literal' source */
    ប្រសិនបើ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * parimaa_na(var_na));
    បើមិនដូច្នេះ {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  បើមិនដូច្នេះ ប្រសិនបើ (*source == '@') {  /* file name */
    ប្រសិនបើ (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * parimaa_na(var_na));
    បើមិនដូច្នេះ {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * parimaa_na(var_na));
    }
  }
  បើមិនដូច្នេះ {  /* string; format as [string "source"] */
    const var_na *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    ប្រសិនបើ (srclen < bufflen && nl == NULL) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    បើមិនដូច្នេះ {
      ប្រសិនបើ (nl != NULL)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      ប្រសិនបើ (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * parimaa_na(var_na));
  }
}

