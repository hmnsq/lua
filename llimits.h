/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ llimits_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ llimits_h


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>


#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_numbits(t)	cast_int(ਆਕਾਰ(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#ਜੇ defined(LUAI_MEM)		/* { external definitions? */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUAI_MEM l_mem;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUAI_UMEM lu_mem;
#ਜਾਂਜੇ LUAI_IS32INT	/* }{ */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ptrdiff_t l_mem;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ size_t lu_mem;
#ਨਹੀਂਤਾਂ  /* 16-bit ints */	/* }{ */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ l_mem;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ lu_mem;
#ਜੇ_ਅੰਤ				/* } */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ lu_byte;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ ls_byte;


/* Type for thread status/error codes */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਮਾਪ	(ਆਕਾਰ(size_t) < ਆਕਾਰ(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LL(x)   (ਆਕਾਰ(x)/ਆਕਾਰ(ਅੱਖਰ) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#ਜੇ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਜੇ defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_P2I	uintptr_t
#ਨਹੀਂਤਾਂ  /* no 'intptr'? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_P2I	uintmax_t  /* use the largest available integer */
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ  /* C89 option */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_P2I	size_t
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUAI_UACNUMBER l_uac_ਸੰਖਿਆ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#ਜੇ defined LUAI_ASSERT
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ NDEBUG
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_assert(c)           ਪ੍ਰਤਿਜ੍ਞਾ(c)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ assert_ਕੋਡ(c)		c
#ਜੇ_ਅੰਤ

#ਜੇ defined(lua_assert)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_assert(c)		((ਖਾਲੀ)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ assert_ਕੋਡ(c)		((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_longassert(c)	assert_ਕੋਡ((c) ? (ਖਾਲੀ)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#ਜੇ !defined(ਅਣਵਰਤਿਆ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ(x)	((ਖਾਲੀ)(x))
#ਜੇ_ਅੰਤ


/* type casts (a macro highlights casts in the code) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast(t, ਘਾਤਾਂਕ)	((t)(ਘਾਤਾਂਕ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_void(i)	cast(ਖਾਲੀ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_voidp(i)	cast(ਖਾਲੀ *, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_ਅੰਕ(i)	cast(lua_ਸੰਖਿਆ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_int(i)	cast(ਪੂਰਨ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_short(i)	cast(ਛੋਟਾ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_uint(i)	cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_byte(i)	cast(lu_byte, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_uchar(i)	cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_char(i)	cast(ਅੱਖਰ, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_charp(i)	cast(ਅੱਖਰ *, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_sizet(i)	cast(size_t, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_Integer(i)	cast(lua_Integer, (i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#ਜੇ !defined(l_castS2U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_castS2U(i)	((lua_Unsigned)(i))
#ਜੇ_ਅੰਤ

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#ਜੇ !defined(l_castU2S)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_castU2S(i)	((lua_Integer)(i))
#ਜੇ_ਅੰਤ

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*voidf)(ਖਾਲੀ);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_func(p) (__extension__ (voidf)(p))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_func(p) ((voidf)(p))
#ਜੇ_ਅੰਤ



/*
** non-return type
*/
#ਜੇ !defined(l_noret)

#ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_noret		ਖਾਲੀ __attribute__((noreturn))
#ਜਾਂਜੇ defined(_MSC_VER) && _MSC_VER >= 1200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_noret		ਖਾਲੀ __declspec(noreturn)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_noret		ਖਾਲੀ
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ


/*
** Inline functions
*/
#ਜੇ !defined(LUA_USE_C89)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_inline	ਸਤਰਅੰਦਰ
#ਜਾਂਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_inline	__inline__
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_inline	/*ਖਾਲੀ_ਥਾਵਾਂ*/
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_sinline	ਸਥਿਰ l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#ਜੇ LUAI_IS32INT
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ l_uint32;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ l_uint32;
#ਜੇ_ਅੰਤ


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#ਜੇ !defined(luai_numidiv)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#ਜੇ_ਅੰਤ

/* float division */
#ਜੇ !defined(luai_numdiv)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numdiv(L,a,b)      ((ਖਾਲੀ)L, (a)/(b))
#ਜੇ_ਅੰਤ

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
#ਜੇ !defined(luai_nummod)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_nummod(L,a,b,m)  \
  { (ਖਾਲੀ)L; (m) = l_mathop(fmod)(a,b); \
    ਜੇ (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#ਜੇ_ਅੰਤ

/* exponentiation */
#ਜੇ !defined(luai_numpow)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numpow(L,a,b)  \
  ((ਖਾਲੀ)L, (b == 2) ? (a)*(a) : l_mathop(ਘਾਤ)(a,b))
#ਜੇ_ਅੰਤ

/* the others are quite standard operations */
#ਜੇ !defined(luai_numadd)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numadd(L,a,b)      ((ਖਾਲੀ)L, (a)+(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numsub(L,a,b)      ((ਖਾਲੀ)L, (a)-(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_nummul(L,a,b)      ((ਖਾਲੀ)L, (a)*(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numunm(L,a)        ((ਖਾਲੀ)L, -(a))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numeq(a,b)         ((a)==(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numlt(a,b)         ((a)<(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numle(a,b)         ((a)<=(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numgt(a,b)         ((a)>(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numge(a,b)         ((a)>=(b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_numisnan(a)        (!luai_numeq((a), (a)))
#ਜੇ_ਅੰਤ



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_numbertointeger(n,p) \
  ((n) >= (LUA_ਸੰਖਿਆ)(LUA_MININTEGER) && \
   (n) < -(LUA_ਸੰਖਿਆ)(LUA_MININTEGER) && \
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
#ਜੇ !defined(LUAI_FUNC)

#ਜੇ defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_FUNC	__attribute__((visibility("internal"))) ਬਾਹਰੀ
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_FUNC	ਬਾਹਰੀ
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_DDEC(dec)	LUAI_FUNC dec
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_DDEF	/*ਖਾਲੀ_ਥਾਵਾਂ*/

#ਜੇ_ਅੰਤ


/* Give these macros simpler names for internal use */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_likely(x)	luai_likely(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#ਜੇ !defined(lua_writestring)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_writestring(s,l)   fwrite((s), ਆਕਾਰ(ਅੱਖਰ), (l), stdout)
#ਜੇ_ਅੰਤ

/* print a newline and flush the output */
#ਜੇ !defined(lua_writeline)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#ਜੇ_ਅੰਤ

/* print an error message */
#ਜੇ !defined(lua_writestringerror)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#ਜੇ_ਅੰਤ

/* }================================================================== */

#ਜੇ_ਅੰਤ

