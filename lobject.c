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
lu_byte luaO_ceillog2 (unsigned 整数 x) {
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
  整数 l = 0;
  x--;
  当 (x >= 256) { l += 8; x >>= 8; }
  返回 cast_byte(l + log_2[x]);
}

/*
** Encodes 'p'% as a floating-point byte, represented as (eeeexxxx).
** The exponent is represented using excess-7. Mimicking IEEE 754, the
** representation normalizes the number when possible, assuming an extra
** 1 before the mantissa (xxxx) and adding one to the exponent (eeee)
** to signal that. So, the real value is (1xxxx) * 2^(eeee - 7 - 1) if
** eeee != 0, and (xxxx) * 2^-7 otherwise (subnormal numbers).
*/
lu_byte luaO_codeparam (unsigned 整数 p) {
  如果 (p >= (cast(lu_mem, 0x1F) << (0xF - 7 - 1)) * 100u)  /* overflow? */
    返回 0xFF;  /* return maximum value */
  否则 {
    p = (cast(l_uint32, p) * 128 + 99) / 100;  /* round up the division */
    如果 (p < 0x10) {  /* subnormal number? */
      /* exponent bits are already zero; nothing else to do */
      返回 cast_byte(p);
    }
    否则 {  /* p >= 0x10 implies ceil(log2(p + 1)) >= 5 */
      /* preserve 5 bits in 'p' */
      unsigned log = luaO_ceillog2(p + 1) - 5u;
      返回 cast_byte(((p >> log) - 0x10) | ((log + 1) << 4));
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
  整数 m = p & 0xF;  /* mantissa */
  整数 e = (p >> 4);  /* exponent */
  lua_assert(x >= 0);
  如果 (e > 0) {  /* normalized? */
    e--;  /* correct exponent */
    m += 0x10;  /* correct mantissa; maximum value is 0x1F */
  }
  e -= 7;  /* correct excess-7 */
  如果 (e >= 0) {
    如果 (x < (MAX_LMEM / 0x1F) >> e)  /* no overflow? */
      返回 (x * m) << e;  /* order doesn't matter here */
    否则  /* real overflow */
      返回 MAX_LMEM;
  }
  否则 {  /* negative exponent */
    e = -e;
    如果 (x < MAX_LMEM / 0x1F)  /* multiplication cannot overflow? */
      返回 (x * m) >> e;  /* multiplying first gives more precision */
    否则 如果 ((x >> e) <  MAX_LMEM / 0x1F)  /* cannot overflow after shift? */
      返回 (x >> e) * m;
    否则  /* real overflow */
      返回 MAX_LMEM;
  }
}


static lua_Integer intarith (lua_State *L, 整数 op, lua_Integer v1,
                                                   lua_Integer v2) {
  switch (op) {
    case LUA_OPADD: 返回 intop(+, v1, v2);
    case LUA_OPSUB:返回 intop(-, v1, v2);
    case LUA_OPMUL:返回 intop(*, v1, v2);
    case LUA_OPMOD: 返回 luaV_mod(L, v1, v2);
    case LUA_OPIDIV: 返回 luaV_idiv(L, v1, v2);
    case LUA_OPBAND: 返回 intop(&, v1, v2);
    case LUA_OPBOR: 返回 intop(|, v1, v2);
    case LUA_OPBXOR: 返回 intop(^, v1, v2);
    case LUA_OPSHL: 返回 luaV_shiftl(v1, v2);
    case LUA_OPSHR: 返回 luaV_shiftr(v1, v2);
    case LUA_OPUNM: 返回 intop(-, 0, v1);
    case LUA_OPBNOT: 返回 intop(^, ~l_castS2U(0), v1);
    default: lua_assert(0); 返回 0;
  }
}


static lua_Number numarith (lua_State *L, 整数 op, lua_Number v1,
                                                  lua_Number v2) {
  switch (op) {
    case LUA_OPADD: 返回 luai_numadd(L, v1, v2);
    case LUA_OPSUB: 返回 luai_numsub(L, v1, v2);
    case LUA_OPMUL: 返回 luai_nummul(L, v1, v2);
    case LUA_OPDIV: 返回 luai_numdiv(L, v1, v2);
    case LUA_OPPOW: 返回 luai_numpow(L, v1, v2);
    case LUA_OPIDIV: 返回 luai_numidiv(L, v1, v2);
    case LUA_OPUNM: 返回 luai_numunm(L, v1);
    case LUA_OPMOD: 返回 luaV_modf(L, v1, v2);
    default: lua_assert(0); 返回 0;
  }
}


整数 luaO_rawarith (lua_State *L, 整数 op, const TValue *p1, const TValue *p2,
                   TValue *res) {
  switch (op) {
    case LUA_OPBAND: case LUA_OPBOR: case LUA_OPBXOR:
    case LUA_OPSHL: case LUA_OPSHR:
    case LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      如果 (tointegerns(p1, &i1) && tointegerns(p2, &i2)) {
        setivalue(res, intarith(L, op, i1, i2));
        返回 1;
      }
      否则 返回 0;  /* fail */
    }
    case LUA_OPDIV: case LUA_OPPOW: {  /* operate only on floats */
      lua_Number n1; lua_Number n2;
      如果 (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        返回 1;
      }
      否则 返回 0;  /* fail */
    }
    default: {  /* other operations */
      lua_Number n1; lua_Number n2;
      如果 (ttisinteger(p1) && ttisinteger(p2)) {
        setivalue(res, intarith(L, op, ivalue(p1), ivalue(p2)));
        返回 1;
      }
      否则 如果 (tonumberns(p1, n1) && tonumberns(p2, n2)) {
        setfltvalue(res, numarith(L, op, n1, n2));
        返回 1;
      }
      否则 返回 0;  /* fail */
    }
  }
}


void luaO_arith (lua_State *L, 整数 op, const TValue *p1, const TValue *p2,
                 StkId res) {
  如果 (!luaO_rawarith(L, op, p1, p2, s2v(res))) {
    /* could not perform raw operation; try metamethod */
    luaT_trybinTM(L, p1, p2, res, cast(TMS, (op - LUA_OPADD) + TM_ADD));
  }
}


lu_byte luaO_hexavalue (整数 c) {
  lua_assert(lisxdigit(c));
  如果 (lisdigit(c)) 返回 cast_byte(c - '0');
  否则 返回 cast_byte((ltolower(c) - 'a') + 10);
}


static 整数 isneg (const char **s) {
  如果 (**s == '-') { (*s)++; 返回 1; }
  否则 如果 (**s == '+') (*s)++;
  返回 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#如果 !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define MAXSIGDIG	30

/*
** convert a hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
static lua_Number lua_strx2number (const char *s, char **endptr) {
  整数 dot = lua_getlocaledecpoint();
  lua_Number r = l_mathop(0.0);  /* result (accumulator) */
  整数 sigdig = 0;  /* number of significant digits */
  整数 nosigdig = 0;  /* number of non-significant digits */
  整数 e = 0;  /* exponent correction */
  整数 neg;  /* 1 if number is negative */
  整数 hasdot = 0;  /* true after seen a dot */
  *endptr = cast_charp(s);  /* nothing is valid yet */
  当 (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);  /* check sign */
  如果 (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* check '0x' */
    返回 l_mathop(0.0);  /* invalid format (no '0x') */
  为 (s += 2; ; s++) {  /* skip '0x' and read numeral */
    如果 (*s == dot) {
      如果 (hasdot) break;  /* second dot? stop loop */
      否则 hasdot = 1;
    }
    否则 如果 (lisxdigit(cast_uchar(*s))) {
      如果 (sigdig == 0 && *s == '0')  /* non-significant digit (zero)? */
        nosigdig++;
      否则 如果 (++sigdig <= MAXSIGDIG)  /* can read it without overflow? */
          r = (r * l_mathop(16.0)) + luaO_hexavalue(*s);
      否则 e++;  /* too many digits; ignore, but still count for exponent */
      如果 (hasdot) e--;  /* decimal digit? correct exponent */
    }
    否则 break;  /* neither a dot nor a digit */
  }
  如果 (nosigdig + sigdig == 0)  /* no digits? */
    返回 l_mathop(0.0);  /* invalid format */
  *endptr = cast_charp(s);  /* valid up to here */
  e *= 4;  /* each digit multiplies/divides value by 2^4 */
  如果 (*s == 'p' || *s == 'P') {  /* exponent part? */
    整数 exp1 = 0;  /* exponent value */
    整数 neg1;  /* exponent sign */
    s++;  /* skip 'p' */
    neg1 = isneg(&s);  /* sign */
    如果 (!lisdigit(cast_uchar(*s)))
      返回 l_mathop(0.0);  /* invalid; must have at least one digit */
    当 (lisdigit(cast_uchar(*s)))  /* read exponent */
      exp1 = exp1 * 10 + *(s++) - '0';
    如果 (neg1) exp1 = -exp1;
    e += exp1;
    *endptr = cast_charp(s);  /* valid up to here */
  }
  如果 (neg) r = -r;
  返回 l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral to be converted to a number */
#如果 !defined (L_MAXLENNUM)
#define L_MAXLENNUM	200
#endif

/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL on
** fail or the address of the ending '\0' on success. ('mode' == 'x')
** means a hexadecimal numeral.
*/
static const char *l_str2dloc (const char *s, lua_Number *result, 整数 mode) {
  char *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  如果 (endptr == s) 返回 NULL;  /* nothing recognized? */
  当 (lisspace(cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  返回 (*endptr == '\0') ? endptr : NULL;  /* OK iff no trailing chars */
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
  整数 mode = pmode ? ltolower(cast_uchar(*pmode)) : 0;
  如果 (mode == 'n')  /* reject 'inf' and 'nan' */
    返回 NULL;
  endptr = l_str2dloc(s, result, mode);  /* try to convert */
  如果 (endptr == NULL) {  /* failed? may be a different locale */
    char buff[L_MAXLENNUM + 1];
    const char *pdot = strchr(s, '.');
    如果 (pdot == NULL || strlen(s) > L_MAXLENNUM)
      返回 NULL;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = l_str2dloc(buff, result, mode);  /* try again */
    如果 (endptr != NULL)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  返回 endptr;
}


#define MAXBY10		cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define MAXLASTD	cast_int(LUA_MAXINTEGER % 10)

static const char *l_str2int (const char *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  整数 empty = 1;
  整数 neg;
  当 (lisspace(cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = isneg(&s);
  如果 (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    为 (; lisxdigit(cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  否则 {  /* decimal */
    为 (; lisdigit(cast_uchar(*s)); s++) {
      整数 d = *s - '0';
      如果 (a >= MAXBY10 && (a > MAXBY10 || d > MAXLASTD + neg))  /* overflow? */
        返回 NULL;  /* do not accept it (as integer) */
      a = a * 10 + cast_uint(d);
      empty = 0;
    }
  }
  当 (lisspace(cast_uchar(*s))) s++;  /* skip trailing spaces */
  如果 (empty || *s != '\0') 返回 NULL;  /* something wrong in the numeral */
  否则 {
    *result = l_castU2S((neg) ? 0u - a : a);
    返回 s;
  }
}


size_t luaO_str2num (const char *s, TValue *o) {
  lua_Integer i; lua_Number n;
  const char *e;
  如果 ((e = l_str2int(s, &i)) != NULL) {  /* try as an integer */
    setivalue(o, i);
  }
  否则 如果 ((e = l_str2d(s, &n)) != NULL) {  /* else try as a float */
    setfltvalue(o, n);
  }
  否则
    返回 0;  /* conversion failed */
  返回 ct_diff2sz(e - s) + 1;  /* success; return string size */
}


整数 luaO_utf8esc (char *buff, l_uint32 x) {
  整数 n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x7FFFFFFFu);
  如果 (x < 0x80)  /* ASCII? */
    buff[UTF8BUFFSZ - 1] = cast_char(x);
  否则 {  /* need continuation bytes */
    unsigned 整数 mfb = 0x3f;  /* maximum that fits in first byte */
    do {  /* add continuation bytes */
      buff[UTF8BUFFSZ - (n++)] = cast_char(0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } 当 (x > mfb);  /* still needs continuation byte? */
    buff[UTF8BUFFSZ - n] = cast_char((~mfb << 1) | x);  /* add first byte */
  }
  返回 n;
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
#如果 LUA_N2SBUFFSZ < (20 + l_floatatt(DIG))
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
static 整数 tostringbuffFloat (lua_Number n, char *buff) {
  /* first conversion */
  整数 len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT,
                            (LUAI_UACNUMBER)n);
  lua_Number check = lua_str2number(buff, NULL);  /* read it back */
  如果 (check != n) {  /* not enough precision? */
    /* convert again with more precision */
    len = l_sprintf(buff, LUA_N2SBUFFSZ, LUA_NUMBER_FMT_N,
                          (LUAI_UACNUMBER)n);
  }
  /* looks like an integer? */
  如果 (buff[strspn(buff, "-0123456789")] == '\0') {
    buff[len++] = lua_getlocaledecpoint();
    buff[len++] = '0';  /* adds '.0' to result */
  }
  返回 len;
}


/*
** Convert a number object to a string, adding it to a buffer.
*/
unsigned luaO_tostringbuff (const TValue *obj, char *buff) {
  整数 len;
  lua_assert(ttisnumber(obj));
  如果 (ttisinteger(obj))
    len = lua_integer2str(buff, LUA_N2SBUFFSZ, ivalue(obj));
  否则
    len = tostringbuffFloat(fltvalue(obj), buff);
  lua_assert(len < LUA_N2SBUFFSZ);
  返回 cast_uint(len);
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
  整数 err;
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
      如果 (buff->buffsize - buff->blen < 3)
        strcpy(buff->b + buff->blen - 3, "...");  /* 'blen' must be > 3 */
      否则 {  /* there is enough space left for the "..." */
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
  如果 (luaD_rawrunprotected(L, pushbuff, buff) != LUA_OK)  /* errors? */
    res = NULL;  /* error message is on the top of the stack */
  否则
    res = getstr(tsvalue(s2v(L->top.p - 1)));
  如果 (buff->b != buff->space)  /* using dynamic buffer? */
    luaM_freearray(L, buff->b, buff->buffsize);  /* free it */
  返回 res;
}


static void addstr2buff (BuffFS *buff, const char *str, size_t slen) {
  size_t left = buff->buffsize - buff->blen;  /* space left in the buffer */
  如果 (buff->err)  /* do nothing else after an error */
    返回;
  如果 (slen > left) {  /* new string doesn't fit into current buffer? */
    如果 (slen > ((MAX_SIZE/2) - buff->blen)) {  /* overflow? */
      memcpy(buff->b + buff->blen, str, left);  /* copy what it can */
      buff->blen = buff->buffsize;
      buff->err = 2;  /* doesn't add anything else */
      返回;
    }
    否则 {
      size_t newsize = buff->buffsize + slen;  /* limited to MAX_SIZE/2 */
      char *newb =
        (buff->b == buff->space)  /* still using static space? */
        ? luaM_reallocvector(buff->L, NULL, 0, newsize, char)
        : luaM_reallocvector(buff->L, buff->b, buff->buffsize, newsize,
                                                               char);
      如果 (newb == NULL) {  /* allocation error? */
        buff->err = 1;  /* signal a memory error */
        返回;
      }
      如果 (buff->b == buff->space)  /* new buffer (not reallocated)? */
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
  当 ((e = strchr(fmt, '%')) != NULL) {
    addstr2buff(&buff, fmt, ct_diff2sz(e - fmt));  /* add 'fmt' up to '%' */
    switch (*(e + 1)) {  /* conversion specifier */
      case 's': {  /* zero-terminated string */
        const char *s = va_arg(argp, char *);
        如果 (s == NULL) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        break;
      }
      case 'c': {  /* an 'int' as a character */
        char c = cast_char(va_arg(argp, 整数));
        addstr2buff(&buff, &c, sizeof(char));
        break;
      }
      case 'd': {  /* an 'int' */
        TValue num;
        setivalue(&num, va_arg(argp, 整数));
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
        整数 len = lua_pointer2str(bf, LUA_N2SBUFFSZ, p);
        addstr2buff(&buff, bf, cast_uint(len));
        break;
      }
      case 'U': {  /* an 'unsigned long' as a UTF-8 sequence */
        char bf[UTF8BUFFSZ];
        unsigned long arg = va_arg(argp, unsigned long);
        整数 len = luaO_utf8esc(bf, cast(l_uint32, arg));
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
  返回 clearbuff(&buff);  /* empty buffer into a new string */
}


const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  如果 (msg == NULL)  /* error? */
    luaD_throw(L, LUA_ERRMEM);
  返回 msg;
}

/* }================================================================== */


#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * sizeof(char)), a += (l) )

void luaO_chunkid (char *out, const char *source, size_t srclen) {
  size_t bufflen = LUA_IDSIZE;  /* free space in buffer */
  如果 (*source == '=') {  /* 'literal' source */
    如果 (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * sizeof(char));
    否则 {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  否则 如果 (*source == '@') {  /* file name */
    如果 (srclen <= bufflen)  /* small enough? */
      memcpy(out, source + 1, srclen * sizeof(char));
    否则 {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * sizeof(char));
    }
  }
  否则 {  /* string; format as [string "source"] */
    const char *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* save space for prefix+suffix+'\0' */
    如果 (srclen < bufflen && nl == NULL) {  /* small one-line source? */
      addstr(out, source, srclen);  /* keep it */
    }
    否则 {
      如果 (nl != NULL)
        srclen = ct_diff2sz(nl - source);  /* stop at first newline */
      如果 (srclen > bufflen) srclen = bufflen;
      addstr(out, source, srclen);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * sizeof(char));
  }
}

