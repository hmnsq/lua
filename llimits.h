/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ llimits_h
#تعریف_کرو llimits_h


#شامل <limits.h>
#شامل <stddef.h>


#شامل "lua.h"


#تعریف_کرو l_numbits(t)	cast_int(حجم(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#اگر defined(LUAI_MEM)		/* { external definitions? */
قسمتعریف LUAI_MEM l_mem;
قسمتعریف LUAI_UMEM lu_mem;
#وگرنہ_اگر LUAI_IS32INT	/* }{ */
قسمتعریف ptrdiff_t l_mem;
قسمتعریف size_t lu_mem;
#ورنہ  /* 16-bit ints */	/* }{ */
قسمتعریف طویل l_mem;
قسمتعریف غیرعلامتی طویل lu_mem;
#اگر_ختم				/* } */

#تعریف_کرو MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
قسمتعریف غیرعلامتی حرف lu_byte;
قسمتعریف علامتی حرف ls_byte;


/* Type for thread status/error codes */
قسمتعریف lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#تعریف_کرو APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#تعریف_کرو MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#تعریف_کرو MAX_سائز	(حجم(size_t) < حجم(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#تعریف_کرو ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#تعریف_کرو LL(x)   (حجم(x)/حجم(حرف) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#اگر !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#شامل <stdint.h>
#اگر defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#تعریف_کرو L_P2I	uintptr_t
#ورنہ  /* no 'intptr'? */
#تعریف_کرو L_P2I	uintmax_t  /* use the largest available integer */
#اگر_ختم
#ورنہ  /* C89 option */
#تعریف_کرو L_P2I	size_t
#اگر_ختم

#تعریف_کرو point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
قسمتعریف LUAI_UACNUMBER l_uac_نمبر;
قسمتعریف LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#اگر defined LUAI_ASSERT
#تعریف_منسوخ NDEBUG
#شامل <assert.h>
#تعریف_کرو lua_assert(c)           أكد(c)
#تعریف_کرو assert_رمز(c)		c
#اگر_ختم

#اگر defined(lua_assert)
#ورنہ
#تعریف_کرو lua_assert(c)		((خالی)0)
#تعریف_کرو assert_رمز(c)		((خالی)0)
#اگر_ختم

#تعریف_کرو check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#تعریف_کرو lua_longassert(c)	assert_رمز((c) ? (خالی)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#اگر !defined(غیر_مستعمل)
#تعریف_کرو غیر_مستعمل(x)	((خالی)(x))
#اگر_ختم


/* type casts (a macro highlights casts in the code) */
#تعریف_کرو cast(t, قوت_نما)	((t)(قوت_نما))

#تعریف_کرو cast_void(i)	cast(خالی, (i))
#تعریف_کرو cast_voidp(i)	cast(خالی *, (i))
#تعریف_کرو cast_عدد(i)	cast(lua_نمبر, (i))
#تعریف_کرو cast_int(i)	cast(صحیح, (i))
#تعریف_کرو cast_short(i)	cast(مختصر, (i))
#تعریف_کرو cast_uint(i)	cast(غیرعلامتی صحیح, (i))
#تعریف_کرو cast_byte(i)	cast(lu_byte, (i))
#تعریف_کرو cast_uchar(i)	cast(غیرعلامتی حرف, (i))
#تعریف_کرو cast_char(i)	cast(حرف, (i))
#تعریف_کرو cast_charp(i)	cast(حرف *, (i))
#تعریف_کرو cast_sizet(i)	cast(size_t, (i))
#تعریف_کرو cast_Integer(i)	cast(lua_Integer, (i))
#تعریف_کرو cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#اگر !defined(l_castS2U)
#تعریف_کرو l_castS2U(i)	((lua_Unsigned)(i))
#اگر_ختم

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#اگر !defined(l_castU2S)
#تعریف_کرو l_castU2S(i)	((lua_Integer)(i))
#اگر_ختم

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#تعریف_کرو cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#تعریف_کرو ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#تعریف_کرو ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
قسمتعریف خالی (*voidf)(خالی);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#اگر defined(__GNUC__)
#تعریف_کرو cast_func(p) (__extension__ (voidf)(p))
#ورنہ
#تعریف_کرو cast_func(p) ((voidf)(p))
#اگر_ختم



/*
** non-return type
*/
#اگر !defined(l_noret)

#اگر defined(__GNUC__)
#تعریف_کرو l_noret		خالی __attribute__((noreturn))
#وگرنہ_اگر defined(_MSC_VER) && _MSC_VER >= 1200
#تعریف_کرو l_noret		خالی __declspec(noreturn)
#ورنہ
#تعریف_کرو l_noret		خالی
#اگر_ختم

#اگر_ختم


/*
** Inline functions
*/
#اگر !defined(LUA_USE_C89)
#تعریف_کرو l_inline	اندرونی
#وگرنہ_اگر defined(__GNUC__)
#تعریف_کرو l_inline	__inline__
#ورنہ
#تعریف_کرو l_inline	/*خالی_خانے*/
#اگر_ختم

#تعریف_کرو l_sinline	ساکن l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#اگر LUAI_IS32INT
قسمتعریف غیرعلامتی صحیح l_uint32;
#ورنہ
قسمتعریف غیرعلامتی طویل l_uint32;
#اگر_ختم


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#اگر !defined(luai_numidiv)
#تعریف_کرو luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#اگر_ختم

/* float division */
#اگر !defined(luai_numdiv)
#تعریف_کرو luai_numdiv(L,a,b)      ((خالی)L, (a)/(b))
#اگر_ختم

/*
** modulo: defined as 'a - floor(a/b)*b'; the direct computation
** using this definition has several problems with rounding errors,
** so it is better to use 'fmod'. 'fmod' gives the result of
** 'a - trunc(a/b)*b', and therefore must be corrected when
** 'trunc(a/b) ~= floor(a/b)'. That happens when the division has a
** non-integer negative result: non-integer result is equivalent to
** a non-zero remainder 'm'; negative result is equivalent to 'a' and
** 'b' with different signs, or 'm' and 'b' with different signs
** (as the result 'm' of 'fmod' has the same sign of 'a').
*/
#اگر !defined(luai_nummod)
#تعریف_کرو luai_nummod(L,a,b,m)  \
  { (خالی)L; (m) = l_mathop(fmod)(a,b); \
    اگر (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#اگر_ختم

/* exponentiation */
#اگر !defined(luai_numpow)
#تعریف_کرو luai_numpow(L,a,b)  \
  ((خالی)L, (b == 2) ? (a)*(a) : l_mathop(قوت)(a,b))
#اگر_ختم

/* the others are quite standard operations */
#اگر !defined(luai_numadd)
#تعریف_کرو luai_numadd(L,a,b)      ((خالی)L, (a)+(b))
#تعریف_کرو luai_numsub(L,a,b)      ((خالی)L, (a)-(b))
#تعریف_کرو luai_nummul(L,a,b)      ((خالی)L, (a)*(b))
#تعریف_کرو luai_numunm(L,a)        ((خالی)L, -(a))
#تعریف_کرو luai_numeq(a,b)         ((a)==(b))
#تعریف_کرو luai_numlt(a,b)         ((a)<(b))
#تعریف_کرو luai_numle(a,b)         ((a)<=(b))
#تعریف_کرو luai_numgt(a,b)         ((a)>(b))
#تعریف_کرو luai_numge(a,b)         ((a)>=(b))
#تعریف_کرو luai_numisnan(a)        (!luai_numeq((a), (a)))
#اگر_ختم



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#تعریف_کرو lua_numbertointeger(n,p) \
  ((n) >= (LUA_نمبر)(LUA_MININTEGER) && \
   (n) < -(LUA_نمبر)(LUA_MININTEGER) && \
      (*(p) = (LUA_INTEGER)(n), 1))



/*
** LUAI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
** LUAI_DDEF and LUAI_DDEC are marks for all extern (const) variables,
** none of which to be exported to outside modules (LUAI_DDEF for
** definitions and LUAI_DDEC for declarations).
** Elf and MACH/gcc (versions 3.2 and later) mark them as "hidden" to
** optimize access when Lua is compiled as a shared library. Not all elf
** targets support this attribute. Unfortunately, gcc does not offer
** a way to check whether the target offers that support, and those
** without support give a warning about it. To avoid these warnings,
** change to the default definition.
*/
#اگر !defined(LUAI_FUNC)

#اگر defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#تعریف_کرو LUAI_FUNC	__attribute__((visibility("internal"))) خارجی
#ورنہ
#تعریف_کرو LUAI_FUNC	خارجی
#اگر_ختم

#تعریف_کرو LUAI_DDEC(dec)	LUAI_FUNC dec
#تعریف_کرو LUAI_DDEF	/*خالی_خانے*/

#اگر_ختم


/* Give these macros simpler names for internal use */
#تعریف_کرو l_likely(x)	luai_likely(x)
#تعریف_کرو l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#اگر !defined(lua_writestring)
#تعریف_کرو lua_writestring(s,l)   fwrite((s), حجم(حرف), (l), stdout)
#اگر_ختم

/* print a newline and flush the output */
#اگر !defined(lua_writeline)
#تعریف_کرو lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#اگر_ختم

/* print an error message */
#اگر !defined(lua_writestringerror)
#تعریف_کرو lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#اگر_ختم

/* }================================================================== */

#اگر_ختم

