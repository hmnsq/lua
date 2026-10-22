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
lu_byte luaO_ceillog2 (unsigned ціле x) {
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
  ціле l = 0;
  x--;
  поки (x >= 256) { l += 8; x >>= 8; }
  повернути cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned ціле p) {
  якщо (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    повернути 0xFF;  /* return maximum value */
  інакше {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    якщо (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      повернути cast_byte(p);
    }
    інакше {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      повернути cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  ціле m = p & 0xF;  /* mantissa */
  ціле e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  якщо (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  якщо (e >= 0) {
    якщо (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      повернути (x * m) << e;  /* order doesn't matter here */
    інакше  /* real overflow */
      повернути MAX_LMEM;
  }
  інакше {  /* negative exponent */
    e = -e;
    якщо (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      повернути (x * m) >> e;  /* multiplying first gives more precision */
    інакше якщо ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      повернути (x >> e) * m;
    інакше  /* real overflow */
      повернути MAX_LMEM;
  }
}


static lua_Integer intarith (lua_State *L, ціле op, lua_Integer v1,
                                                   lua_Integer v2) {
  switch (op) {
    case LUA_OPADD: повернути intop(+, v1, v2);
    case LUA_OPSUB:повернути intop(-, v1, v2);
    case LUA_OPMUL:повернути intop(*, v1, v2);
    case LUA_OPMOD: повернути luaV_mod(L, v1, v2);
    case LUA_OPIDIV: повернути luaV_idiv(L, v1, v2);
    case LUA_OPBAND: повернути intop(&, v1, v2);
    case LUA_OPBOR: повернути intop(|, v1, v2);
    case LUA_OPBXOR: повернути intop(^, v1, v2);
    case LUA_OPSHL: повернути luaV_shiftl(v1, v2);
    case LUA_OPSHR: повернути luaV_shiftr(v1, v2);
    case LUA_OPUNM: повернути intop(-, 0, v1);
    case LUA_OPBNOT: повернути intop(^, ~l_castS2U(0), v1);
    default: lua_assert(0); повернути 0;
  }
}


static lua_Number numarith (lua_State *L, ціле op, lua_Number v1,
                                                  lua_Number v2) {
  switch (op) {
    case LUA_OPADD: повернути luai_numadd(L, v1, v2);
    case LUA_OPSUB: повернути luai_numsub(L, v1, v2);
    case LUA_OPMUL: повернути luai_nummul(L, v1, v2);
    case LUA_OPDIV: повернути luai_numdiv(L, v1, v2);
    case LUA_OPPOW: повернути luai_numpow(L, v1, v2);
    case LUA_OPIDIV: повернути luai_numidiv(L, v1, v2);
    case LUA_OPUNM: повернути luai_numunm(L, v1);
    case LUA_OPMOD: повернути luaV_modf(L, v1, v2);
    default: lua_assert(0); повернути 0;
  }
}


ціле luaO_rawarith (lua_State *L, ціле op, const TValue *p1, const TValue *p2,
                   TValue *res) {
  switch (op) {
    case LUA_OPBAND: case LUA_OPBOR: case LUA_OPBXOR:
    case LUA_OPSHL: case LUA_OPSHR:
    case LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      якщо (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        повернути 1;
      }
      інакше повернути 0;  /* fail */
    }
    case LUA_OPDIV: case LUA_OPPOW: {  /* operate only on floats */
      lua_Number n1; lua_Number n2;
      якщо (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        повернути 1;
      }
      інакше повернути 0;  /* fail */
    }
    default: {  /* other operations */
      lua_Number n1; lua_Number n2;
      якщо (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        повернути 1;
      }
      інакше якщо (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        повернути 1;
      }
      інакше повернути 0;  /* fail */
    }
  }
}


void luaO_arith (lua_State *L, ціле op, const TValue *p1, const TValue *p2,
                 StkId res) {
  якщо (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (ціле c) {
  lua_assert(lisxdigit(c));
  якщо (lisdigit(c)) повернути cast_byte(c - '0');
  інакше повернути cast_byte((ltolower(c) - 'a') + 10);
}


static ціле isneg (const char **s) {
  якщо (**s == '-') { (*s)++; повернути 1; }
  інакше якщо (**s == '+') (*s)++;
  повернути 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#якщо !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
static lua_Number lua_strx2number (const char *s, char **endptr) {
  ціле dot = lua_getlocaledecpoint();
  lua_Number r = l_mathop(0.0);  /* result (accumulator) */
  ціле sigdig = 0;  /* number of significant digits */
  ціле nosigdig = 0;  /* number of non-significant digits */
  ціле e = 0;  /* exponent correction */
  ціле neg;  /* 1 if number is negative */
  ціле hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  поки (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  якщо (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    повернути l_mathop(0.0);  /* invalid format (no '0x') */
  для (s += 2; ; s++) {  /* skip '0x' and read numeral */
    якщо (*s == dot) {
      якщо (hasdot) break;  /* second dot? stop loop */
      інакше hasdot = 1;
    }
    інакше якщо (lisxdigit(cast_uchar(*s))) {
      якщо (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      інакше якщо (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      інакше e++;  /* too many digits; ignore, but still count for exponent */
      якщо (hasdot) e--;  /* decimal digit? correct exponent */
    }
    інакше break;  /* neither a dot nor a digit */
  }
  якщо (nosigdig + sigdig == 0)  /* no digits? */
    повернути l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  якщо (*s == 'p' || *s == 'P') {  /* exponent part? */
    ціле exp1 = 0;  /* exponent value */
    ціле neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    якщо (!lisdigit(cast_uchar(*s)))
      повернути l_mathop(0.0);  /* invalid; must have at least one digit */
    поки (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    якщо (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  якщо (neg) r = -r;
  повернути l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#якщо !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
static const char *l_str2dloc (const char *s, lua_Number *result, ціле mode) {
  char *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  якщо (endptr == s) повернути NULL;  /* nothing recognized? */
  поки (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  повернути (*endptr == '\0') ? endptr : NULL;  /* OK iff no trailing chars */
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
static const char *l_str2d (const char *s, lua_Number *result) {
  const char *endptr;
  const char *pmode = strpbrk(s, ".xXnN");  /* look for special chars */
  ціле mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  якщо (mode == 'n')  /* reject 'inf' and 'nan' */
    повернути NULL;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  якщо (endptr == NULL) {  /* failed? may be a different locale */
    char buff[L_MAXLENNUM + 1];
    const char *pdot = strchr(s, '.');
    якщо (pdot == NULL || strlen(s) > L_MAXLENNUM)
      повернути NULL;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    якщо (endptr != NULL)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  повернути endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

static const char *l_str2int (const char *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  ціле empty = 1;
  ціле neg;
  поки (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  якщо (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    для (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  інакше {  /* decimal */
    для (; lisdigit(cast_uchar(*s)); s++) {
      ціле d = *s - '0';
      якщо (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        повернути NULL;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  поки (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  якщо (empty || *s != '\0') повернути NULL;  /* something wrong in the numeral */
  інакше {
    *result = l_castU2S((neg) ? 0u - a : a);
    повернути s;
  }
}


size_t luaO_str2num (const char *s, TValue *o) {
  lua_Integer i; lua_Number n;
  const char *e;
  якщо ((e = l_str2int(s, &i)) != NULL) {  /* try as an integer */
    setivalue(o, i);
  }
  інакше якщо ((e = l_str2d(s, &n)) != NULL) {  /* else try as a float */
    setfltvalue(o, n);
  }
  інакше
    повернути 0;  /* conversion failed */
  повернути ct_diff2sz(e - s) + 1;  /* success; return string size */
}


ціле luaO_utf8esc (char *buff, l_uint32 x) {
  ціле n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  якщо (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  інакше {  /* need continuation bytes */
    unsigned ціле mfb = 0x3f;  /* maximum that fits in first byte */
    do {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } поки (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  повернути n;
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
#якщо LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
static ціле tostringbuffFloat (lua_Number n, char *buff) {
  /* first conversion */
  ціле len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT,
                            (LUAI_UACNUMBER)n);
  lua_Number check = lua_str2number(buff, NULL);  /* read it back */
  якщо (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  якщо (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  повернути len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (const TValue *obj, char *buff) {
  ціле len;
  lua_assert(ttisnumber(obj));
  якщо (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  інакше
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  повернути cast_uint(len);
}


/*
** Convert a number object to a Lua string, replacing the value at 'obj'
*/
void luaO_tostring (lua_State *L, TValue *obj) {
  char buff[LUA_N2SBUFFSZ];
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
typedef struct BuffFS {
  lua_State *L;
  char *b;
  size_t buffsize;
  size_t blen;  /* length of string in 'buff' */
  ціле err;
  char space[BUFVFS];  /* initial buffer */
} BuffFS;


static void initbuff (lua_State *L, BuffFS *buff) {
  buff->L = L;
  buff->b = buff->space;
  buff->buffsize = sizeof(buff->space);
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
    case 1:  /* memory error */
      luaD_throw(L, LUA_ERRMEM);
      break;
    case 2:  /* length overflow: Add "..." at the end of result */
      якщо (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      інакше {  /* there is enough space left for the "..." */
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


static const char *clearbuff (BuffFS *buff) {
  lua_State *L = buff->L;
  const char *res;
  якщо (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = NULL;  /* error message is on the top of the stack */
  інакше
    res = getstr(tsvalue(s2v(L->top.p - 1)));
  якщо (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  повернути res;
}


static void addstr2buff (BuffFS *buff, const char *str, size_t slen) {
  size_t left = buff->buffsize - buff->blen;  /* space left in the buffer */
  якщо (buff->err)  /* do nothing else after an error */
    повернути;
  якщо (slen > left) {  /* new string doesn't fit into current buffer? */
    якщо (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, left);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      повернути;
    }
    інакше {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      char *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, NULL, 0, newsize, char)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               char);
      якщо (newb == NULL) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        повернути;
      }
      якщо (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
  char numbuff[LUA_N2SBUFFSZ];
  unsigned len = luaO_tostringbuff(num, numbuff);
  addstr2buff(buff, numbuff, len);
}


/*
** this function handles only '%d', '%c', '%f', '%p', '%s', and '%%'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
const char *luaO_pushvfstring (lua_State *L, const char *fmt, va_list argp) {
  BuffFS buff;  /* holds last part of the result */
  const char *e;  /* points to next '%' */
  initbuff(L, &buff);
  поки ((e = strchr(fmt, '%')) != NULL) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    switch (*(e + 1)) {  /* conversion specifier */
      case 's': {  /* zero-terminated string */
        const char *s = va_arg(argp, char *);
        якщо (s == NULL) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        break;
      }
      case 'c': {  /* an 'int' as a character */
        char c = cast_char(va_arg(argp, ціле));
        addstr2buff(&buff, &c, sizeof(char));
        break;
      }
      case 'd': {  /* an 'int' */
        TValue num;
        setivalue(&num, va_arg(argp, ціле));
        addnum2buff(&buff, &num);
        break;
      }
      case 'I': {  /* a 'lua_Integer' */
        TValue num;
        setivalue(&num, cast_Integer(va_arg(argp, l_uacInt)));
        addnum2buff(&buff, &num);
        break;
      }
      case 'f': {  /* a 'lua_Number' */
        TValue num;
        setfltvalue(&num, cast_num(va_arg(argp, l_uacNumber)));
        addnum2buff(&buff, &num);
        break;
      }
      case 'p': {  /* a pointer */
        char bf[LUA_N2SBUFFSZ];  /* enough space for '%p' */
        void *p = va_arg(argp, void *);
        ціле len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        break;
      }
      case 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        char bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        ціле len = luaO_utf8esc(bf, cast(l_uint32, arg));
        addstr2buff(&buff, bf + UTF8BUFFSZ - len, cast_uint(len));
        break;
      }
      case '%': {
        addstr2buff(&buff, "%", 1);
        break;
      }
      default: {
        addstr2buff(&buff, e, 2);  /* keep unknown format in the result */
        break;
      }
    }
    fmt = e + 2;  /* skip '%' and the specifier */
  }
  addstr2buff(&buff, fmt, strlen(fmt));  /* rest of 'fmt' */
  повернути clearbuff(&buff);  /* empty buffer into a new string */
}


const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  якщо (msg == NULL)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  повернути msg;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * sizeof(char)), a += (l) )

void luaO_chunkid (char *out, const char *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  якщо (*source == '=') {  /* 'literal' source */
    якщо (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * sizeof(char));
    інакше {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  інакше якщо (*source == '@') {  /* file name */
    якщо (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * sizeof(char));
    інакше {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * sizeof(char));
    }
  }
  інакше {  /* string; format as [string "source"] */
    const char *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    якщо (srclen < bufflen && nl == NULL) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    інакше {
      якщо (nl != NULL)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      якщо (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * sizeof(char));
  }
}

