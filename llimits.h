/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده llimits_h
#تعریف_کن llimits_h


#ضمن <limits.h>
#ضمن <stddef.h>


#ضمن "lua.h"


#تعریف_کن l_numbits(t)	cast_int(حجم(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#إذا defined(LUAI_MEM)		/* { external definitions? */
تعريفنوع LUAI_MEM l_mem;
تعريفنوع LUAI_UMEM lu_mem;
#وإلاإذا LUAI_IS32INT	/* }{ */
تعريفنوع ptrdiff_t l_mem;
تعريفنوع size_t lu_mem;
#وإلا  /* 16-bit ints */	/* }{ */
تعريفنوع دراز l_mem;
تعريفنوع بیعلامت دراز lu_mem;
#پایان_اگر				/* } */

#تعریف_کن MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
تعريفنوع بیعلامت حرف lu_byte;
تعريفنوع علامتدار حرف ls_byte;


/* Type for thread status/error codes */
تعريفنوع lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#تعریف_کن APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#تعریف_کن MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#تعریف_کن MAX_اندازه	(حجم(size_t) < حجم(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#تعریف_کن ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#تعریف_کن LL(x)   (حجم(x)/حجم(حرف) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#إذا !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#ضمن <stdint.h>
#إذا defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#تعریف_کن L_P2I	uintptr_t
#وإلا  /* no 'intptr'? */
#تعریف_کن L_P2I	uintmax_t  /* use the largest available integer */
#پایان_اگر
#وإلا  /* C89 option */
#تعریف_کن L_P2I	size_t
#پایان_اگر

#تعریف_کن point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
تعريفنوع LUAI_UACNUMBER l_uac_عدد;
تعريفنوع LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#إذا defined LUAI_ASSERT
#لغو_تعریف NDEBUG
#ضمن <assert.h>
#تعریف_کن lua_assert(c)           أكد(c)
#تعریف_کن assert_کد(c)		c
#پایان_اگر

#إذا defined(lua_assert)
#وإلا
#تعریف_کن lua_assert(c)		((فراغ)0)
#تعریف_کن assert_کد(c)		((فراغ)0)
#پایان_اگر

#تعریف_کن check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#تعریف_کن lua_longassert(c)	assert_کد((c) ? (فراغ)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#إذا !defined(استفادهنشده)
#تعریف_کن استفادهنشده(x)	((فراغ)(x))
#پایان_اگر


/* type casts (a macro highlights casts in the code) */
#تعریف_کن cast(t, نمایی_تابع)	((t)(نمایی_تابع))

#تعریف_کن cast_void(i)	cast(فراغ, (i))
#تعریف_کن cast_voidp(i)	cast(فراغ *, (i))
#تعریف_کن cast_رقم(i)	cast(lua_عدد, (i))
#تعریف_کن cast_int(i)	cast(صحيح, (i))
#تعریف_کن cast_short(i)	cast(کوتاه, (i))
#تعریف_کن cast_uint(i)	cast(بیعلامت صحيح, (i))
#تعریف_کن cast_byte(i)	cast(lu_byte, (i))
#تعریف_کن cast_uchar(i)	cast(بیعلامت حرف, (i))
#تعریف_کن cast_char(i)	cast(حرف, (i))
#تعریف_کن cast_charp(i)	cast(حرف *, (i))
#تعریف_کن cast_sizet(i)	cast(size_t, (i))
#تعریف_کن cast_Integer(i)	cast(lua_Integer, (i))
#تعریف_کن cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#إذا !defined(l_castS2U)
#تعریف_کن l_castS2U(i)	((lua_Unsigned)(i))
#پایان_اگر

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#إذا !defined(l_castU2S)
#تعریف_کن l_castU2S(i)	((lua_Integer)(i))
#پایان_اگر

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#تعریف_کن cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#تعریف_کن ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#تعریف_کن ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
تعريفنوع فراغ (*voidf)(فراغ);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#إذا defined(__GNUC__)
#تعریف_کن cast_func(p) (__extension__ (voidf)(p))
#وإلا
#تعریف_کن cast_func(p) ((voidf)(p))
#پایان_اگر



/*
** non-return type
*/
#إذا !defined(l_noret)

#إذا defined(__GNUC__)
#تعریف_کن l_noret		فراغ __attribute__((noreturn))
#وإلاإذا defined(_MSC_VER) && _MSC_VER >= 1200
#تعریف_کن l_noret		فراغ __declspec(noreturn)
#وإلا
#تعریف_کن l_noret		فراغ
#پایان_اگر

#پایان_اگر


/*
** Inline functions
*/
#إذا !defined(LUA_USE_C89)
#تعریف_کن l_inline	مضمن
#وإلاإذا defined(__GNUC__)
#تعریف_کن l_inline	__inline__
#وإلا
#تعریف_کن l_inline	/*خالی*/
#پایان_اگر

#تعریف_کن l_sinline	ساكن l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#إذا LUAI_IS32INT
تعريفنوع بیعلامت صحيح l_uint32;
#وإلا
تعريفنوع بیعلامت دراز l_uint32;
#پایان_اگر


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#إذا !defined(luai_numidiv)
#تعریف_کن luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#پایان_اگر

/* float division */
#إذا !defined(luai_numdiv)
#تعریف_کن luai_numdiv(L,a,b)      ((فراغ)L, (a)/(b))
#پایان_اگر

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
#إذا !defined(luai_nummod)
#تعریف_کن luai_nummod(L,a,b,m)  \
  { (فراغ)L; (m) = l_mathop(fmod)(a,b); \
    إذا (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#پایان_اگر

/* exponentiation */
#إذا !defined(luai_numpow)
#تعریف_کن luai_numpow(L,a,b)  \
  ((فراغ)L, (b == 2) ? (a)*(a) : l_mathop(توان)(a,b))
#پایان_اگر

/* the others are quite standard operations */
#إذا !defined(luai_numadd)
#تعریف_کن luai_numadd(L,a,b)      ((فراغ)L, (a)+(b))
#تعریف_کن luai_numsub(L,a,b)      ((فراغ)L, (a)-(b))
#تعریف_کن luai_nummul(L,a,b)      ((فراغ)L, (a)*(b))
#تعریف_کن luai_numunm(L,a)        ((فراغ)L, -(a))
#تعریف_کن luai_numeq(a,b)         ((a)==(b))
#تعریف_کن luai_numlt(a,b)         ((a)<(b))
#تعریف_کن luai_numle(a,b)         ((a)<=(b))
#تعریف_کن luai_numgt(a,b)         ((a)>(b))
#تعریف_کن luai_numge(a,b)         ((a)>=(b))
#تعریف_کن luai_numisnan(a)        (!luai_numeq((a), (a)))
#پایان_اگر



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#تعریف_کن lua_numbertointeger(n,p) \
  ((n) >= (LUA_عدد)(LUA_MININTEGER) && \
   (n) < -(LUA_عدد)(LUA_MININTEGER) && \
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
#إذا !defined(LUAI_FUNC)

#إذا defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#تعریف_کن LUAI_FUNC	__attribute__((visibility("internal"))) خارجي
#وإلا
#تعریف_کن LUAI_FUNC	خارجي
#پایان_اگر

#تعریف_کن LUAI_DDEC(dec)	LUAI_FUNC dec
#تعریف_کن LUAI_DDEF	/*خالی*/

#پایان_اگر


/* Give these macros simpler names for internal use */
#تعریف_کن l_likely(x)	luai_likely(x)
#تعریف_کن l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#إذا !defined(lua_writestring)
#تعریف_کن lua_writestring(s,l)   fwrite((s), حجم(حرف), (l), stdout)
#پایان_اگر

/* print a newline and flush the output */
#إذا !defined(lua_writeline)
#تعریف_کن lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#پایان_اگر

/* print an error message */
#إذا !defined(lua_writestringerror)
#تعریف_کن lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#پایان_اگر

/* }================================================================== */

#پایان_اگر

