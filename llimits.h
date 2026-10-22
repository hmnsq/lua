/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ llimits_h
#ವ್ಯಾಖ್ಯಾನಿಸು llimits_h


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>


#ಅನ್ತರ್ಭಾವ "lua.h"


#ವ್ಯಾಖ್ಯಾನಿಸು l_numbits(t)	cast_int(ಗಾತ್ರ(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#ಆದರೆ defined(LUAI_MEM)		/* { external definitions? */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUAI_MEM l_mem;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUAI_UMEM lu_mem;
#ಅಥವಾಆದರೆ LUAI_IS32INT	/* }{ */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ptrdiff_t l_mem;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ size_t lu_mem;
#ಇಲ್ಲದಿದ್ದರೆ  /* 16-bit ints */	/* }{ */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ l_mem;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ lu_mem;
#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ lu_byte;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ ls_byte;


/* Type for thread status/error codes */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#ವ್ಯಾಖ್ಯಾನಿಸು APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಗಾತ್ರ	(ಗಾತ್ರ(size_t) < ಗಾತ್ರ(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#ವ್ಯಾಖ್ಯಾನಿಸು LL(x)   (ಗಾತ್ರ(x)/ಗಾತ್ರ(ಅಕ್ಷರ) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#ಆದರೆ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಆದರೆ defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#ವ್ಯಾಖ್ಯಾನಿಸು L_P2I	uintptr_t
#ಇಲ್ಲದಿದ್ದರೆ  /* no 'intptr'? */
#ವ್ಯಾಖ್ಯಾನಿಸು L_P2I	uintmax_t  /* use the largest available integer */
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ  /* C89 option */
#ವ್ಯಾಖ್ಯಾನಿಸು L_P2I	size_t
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUAI_UACNUMBER l_uac_ಸಂಖ್ಯೆ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#ಆದರೆ defined LUAI_ASSERT
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು NDEBUG
#ಅನ್ತರ್ಭಾವ <assert.h>
#ವ್ಯಾಖ್ಯಾನಿಸು lua_assert(c)           ಪ್ರತಿಜ್ಞಾ(c)
#ವ್ಯಾಖ್ಯಾನಿಸು assert_ಸಂಕೇತ(c)		c
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(lua_assert)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು lua_assert(c)		((ಶೂನ್ಯ)0)
#ವ್ಯಾಖ್ಯಾನಿಸು assert_ಸಂಕೇತ(c)		((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#ವ್ಯಾಖ್ಯಾನಿಸು lua_longassert(c)	assert_ಸಂಕೇತ((c) ? (ಶೂನ್ಯ)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#ಆದರೆ !defined(ಬಳಸದ)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ(x)	((ಶೂನ್ಯ)(x))
#ಆದರೆ_ಅಂತ್ಯ


/* type casts (a macro highlights casts in the code) */
#ವ್ಯಾಖ್ಯಾನಿಸು cast(t, ಘಾತಾಂಕ)	((t)(ಘಾತಾಂಕ))

#ವ್ಯಾಖ್ಯಾನಿಸು cast_void(i)	cast(ಶೂನ್ಯ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_voidp(i)	cast(ಶೂನ್ಯ *, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_ಅಂಕೆ(i)	cast(lua_ಸಂಖ್ಯೆ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_int(i)	cast(ಪೂರ್ಣಾಂಕ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_short(i)	cast(ಗಿಡ್ಡ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_uint(i)	cast(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_byte(i)	cast(lu_byte, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_uchar(i)	cast(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_char(i)	cast(ಅಕ್ಷರ, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_charp(i)	cast(ಅಕ್ಷರ *, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_sizet(i)	cast(size_t, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_Integer(i)	cast(lua_Integer, (i))
#ವ್ಯಾಖ್ಯಾನಿಸು cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#ಆದರೆ !defined(l_castS2U)
#ವ್ಯಾಖ್ಯಾನಿಸು l_castS2U(i)	((lua_Unsigned)(i))
#ಆದರೆ_ಅಂತ್ಯ

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#ಆದರೆ !defined(l_castU2S)
#ವ್ಯಾಖ್ಯಾನಿಸು l_castU2S(i)	((lua_Integer)(i))
#ಆದರೆ_ಅಂತ್ಯ

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#ವ್ಯಾಖ್ಯಾನಿಸು ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*voidf)(ಶೂನ್ಯ);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#ಆದರೆ defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು cast_func(p) (__extension__ (voidf)(p))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು cast_func(p) ((voidf)(p))
#ಆದರೆ_ಅಂತ್ಯ



/*
** non-return type
*/
#ಆದರೆ !defined(l_noret)

#ಆದರೆ defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು l_noret		ಶೂನ್ಯ __attribute__((noreturn))
#ಅಥವಾಆದರೆ defined(_MSC_VER) && _MSC_VER >= 1200
#ವ್ಯಾಖ್ಯಾನಿಸು l_noret		ಶೂನ್ಯ __declspec(noreturn)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_noret		ಶೂನ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ


/*
** Inline functions
*/
#ಆದರೆ !defined(LUA_USE_C89)
#ವ್ಯಾಖ್ಯಾನಿಸು l_inline	ಸಾಲೊಳಗೆ
#ಅಥವಾಆದರೆ defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು l_inline	__inline__
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_inline	/*ಖಾಲಿಗಳು*/
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು l_sinline	ಸ್ಥಿರ l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#ಆದರೆ LUAI_IS32INT
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ l_uint32;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ l_uint32;
#ಆದರೆ_ಅಂತ್ಯ


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#ಆದರೆ !defined(luai_numidiv)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#ಆದರೆ_ಅಂತ್ಯ

/* float division */
#ಆದರೆ !defined(luai_numdiv)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numdiv(L,a,b)      ((ಶೂನ್ಯ)L, (a)/(b))
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ !defined(luai_nummod)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_nummod(L,a,b,m)  \
  { (ಶೂನ್ಯ)L; (m) = l_mathop(fmod)(a,b); \
    ಆದರೆ (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#ಆದರೆ_ಅಂತ್ಯ

/* exponentiation */
#ಆದರೆ !defined(luai_numpow)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numpow(L,a,b)  \
  ((ಶೂನ್ಯ)L, (b == 2) ? (a)*(a) : l_mathop(ಘಾತ)(a,b))
#ಆದರೆ_ಅಂತ್ಯ

/* the others are quite standard operations */
#ಆದರೆ !defined(luai_numadd)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numadd(L,a,b)      ((ಶೂನ್ಯ)L, (a)+(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numsub(L,a,b)      ((ಶೂನ್ಯ)L, (a)-(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_nummul(L,a,b)      ((ಶೂನ್ಯ)L, (a)*(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numunm(L,a)        ((ಶೂನ್ಯ)L, -(a))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numeq(a,b)         ((a)==(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numlt(a,b)         ((a)<(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numle(a,b)         ((a)<=(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numgt(a,b)         ((a)>(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numge(a,b)         ((a)>=(b))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_numisnan(a)        (!luai_numeq((a), (a)))
#ಆದರೆ_ಅಂತ್ಯ



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lua_numbertointeger(n,p) \
  ((n) >= (LUA_ಸಂಖ್ಯೆ)(LUA_MININTEGER) && \
   (n) < -(LUA_ಸಂಖ್ಯೆ)(LUA_MININTEGER) && \
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
#ಆದರೆ !defined(LUAI_FUNC)

#ಆದರೆ defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_FUNC	__attribute__((visibility("internal"))) ಬಾಹ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_FUNC	ಬಾಹ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_DDEC(dec)	LUAI_FUNC dec
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_DDEF	/*ಖಾಲಿಗಳು*/

#ಆದರೆ_ಅಂತ್ಯ


/* Give these macros simpler names for internal use */
#ವ್ಯಾಖ್ಯಾನಿಸು l_likely(x)	luai_likely(x)
#ವ್ಯಾಖ್ಯಾನಿಸು l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#ಆದರೆ !defined(lua_writestring)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_writestring(s,l)   fwrite((s), ಗಾತ್ರ(ಅಕ್ಷರ), (l), stdout)
#ಆದರೆ_ಅಂತ್ಯ

/* print a newline and flush the output */
#ಆದರೆ !defined(lua_writeline)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#ಆದರೆ_ಅಂತ್ಯ

/* print an error message */
#ಆದರೆ !defined(lua_writestringerror)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#ಆದರೆ_ಅಂತ್ಯ

/* }================================================================== */

#ಆದರೆ_ಅಂತ್ಯ

