/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#ifndef llimits_h
#define llimits_h


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stddef.h>


#അന്തര്ഭാവ "lua.h"


#define l_numbits(t)	cast_int(വലുപ്പം(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#എങ്കിൽ defined(LUAI_MEM)		/* { external definitions? */
തരനിർവചനം LUAI_MEM l_mem;
തരനിർവചനം LUAI_UMEM lu_mem;
#അല്ലെങ്കിലെങ്കിൽ LUAI_IS32INT	/* }{ */
തരനിർവചനം ptrdiff_t l_mem;
തരനിർവചനം size_t lu_mem;
#അല്ലെങ്കിൽ  /* 16-bit ints */	/* }{ */
തരനിർവചനം long l_mem;
തരനിർവചനം unsigned long lu_mem;
#endif				/* } */

#define MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
തരനിർവചനം unsigned അക്ഷരം lu_byte;
തരനിർവചനം signed അക്ഷരം ls_byte;


/* Type for thread status/error codes */
തരനിർവചനം lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#define APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#define MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#define MAX_SIZE	(വലുപ്പം(size_t) < വലുപ്പം(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#define ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#define LL(x)   (വലുപ്പം(x)/വലുപ്പം(അക്ഷരം) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#എങ്കിൽ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#അന്തര്ഭാവ <stdint.h>
#എങ്കിൽ defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#define L_P2I	uintptr_t
#അല്ലെങ്കിൽ  /* no 'intptr'? */
#define L_P2I	uintmax_t  /* use the largest available integer */
#endif
#അല്ലെങ്കിൽ  /* C89 option */
#define L_P2I	size_t
#endif

#define point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
തരനിർവചനം LUAI_UACNUMBER l_uac_സംഖ്യ;
തരനിർവചനം LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#എങ്കിൽ defined LUAI_ASSERT
#undef NDEBUG
#അന്തര്ഭാവ <assert.h>
#define lua_assert(c)           പ്രതിജ്ഞാ(c)
#define assert_code(c)		c
#endif

#എങ്കിൽ defined(lua_assert)
#അല്ലെങ്കിൽ
#define lua_assert(c)		((ശൂന്യം)0)
#define assert_code(c)		((ശൂന്യം)0)
#endif

#define check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#define lua_longassert(c)	assert_code((c) ? (ശൂന്യം)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#എങ്കിൽ !defined(UNUSED)
#define UNUSED(x)	((ശൂന്യം)(x))
#endif


/* type casts (a macro highlights casts in the code) */
#define cast(t, exp)	((t)(exp))

#define cast_void(i)	cast(ശൂന്യം, (i))
#define cast_voidp(i)	cast(ശൂന്യം *, (i))
#define cast_അക്കം(i)	cast(lua_സംഖ്യ, (i))
#define cast_int(i)	cast(പൂർണ്ണസംഖ്യ, (i))
#define cast_short(i)	cast(short, (i))
#define cast_uint(i)	cast(unsigned പൂർണ്ണസംഖ്യ, (i))
#define cast_byte(i)	cast(lu_byte, (i))
#define cast_uchar(i)	cast(unsigned അക്ഷരം, (i))
#define cast_char(i)	cast(അക്ഷരം, (i))
#define cast_charp(i)	cast(അക്ഷരം *, (i))
#define cast_sizet(i)	cast(size_t, (i))
#define cast_Integer(i)	cast(lua_Integer, (i))
#define cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#എങ്കിൽ !defined(l_castS2U)
#define l_castS2U(i)	((lua_Unsigned)(i))
#endif

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#എങ്കിൽ !defined(l_castU2S)
#define l_castU2S(i)	((lua_Integer)(i))
#endif

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#define cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#define ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#define ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
തരനിർവചനം ശൂന്യം (*voidf)(ശൂന്യം);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#എങ്കിൽ defined(__GNUC__)
#define cast_func(p) (__extension__ (voidf)(p))
#അല്ലെങ്കിൽ
#define cast_func(p) ((voidf)(p))
#endif



/*
** non-return type
*/
#എങ്കിൽ !defined(l_noret)

#എങ്കിൽ defined(__GNUC__)
#define l_noret		ശൂന്യം __attribute__((noreturn))
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER) && _MSC_VER >= 1200
#define l_noret		ശൂന്യം __declspec(noreturn)
#അല്ലെങ്കിൽ
#define l_noret		ശൂന്യം
#endif

#endif


/*
** Inline functions
*/
#എങ്കിൽ !defined(LUA_USE_C89)
#define l_inline	വരിയിൽ
#അല്ലെങ്കിലെങ്കിൽ defined(__GNUC__)
#define l_inline	__inline__
#അല്ലെങ്കിൽ
#define l_inline	/* empty */
#endif

#define l_sinline	സ്ഥിതം l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#എങ്കിൽ LUAI_IS32INT
തരനിർവചനം unsigned പൂർണ്ണസംഖ്യ l_uint32;
#അല്ലെങ്കിൽ
തരനിർവചനം unsigned long l_uint32;
#endif


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#എങ്കിൽ !defined(luai_numidiv)
#define luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#endif

/* float division */
#എങ്കിൽ !defined(luai_numdiv)
#define luai_numdiv(L,a,b)      ((ശൂന്യം)L, (a)/(b))
#endif

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
#എങ്കിൽ !defined(luai_nummod)
#define luai_nummod(L,a,b,m)  \
  { (ശൂന്യം)L; (m) = l_mathop(fmod)(a,b); \
    എങ്കിൽ (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#endif

/* exponentiation */
#എങ്കിൽ !defined(luai_numpow)
#define luai_numpow(L,a,b)  \
  ((ശൂന്യം)L, (b == 2) ? (a)*(a) : l_mathop(pow)(a,b))
#endif

/* the others are quite standard operations */
#എങ്കിൽ !defined(luai_numadd)
#define luai_numadd(L,a,b)      ((ശൂന്യം)L, (a)+(b))
#define luai_numsub(L,a,b)      ((ശൂന്യം)L, (a)-(b))
#define luai_nummul(L,a,b)      ((ശൂന്യം)L, (a)*(b))
#define luai_numunm(L,a)        ((ശൂന്യം)L, -(a))
#define luai_numeq(a,b)         ((a)==(b))
#define luai_numlt(a,b)         ((a)<(b))
#define luai_numle(a,b)         ((a)<=(b))
#define luai_numgt(a,b)         ((a)>(b))
#define luai_numge(a,b)         ((a)>=(b))
#define luai_numisnan(a)        (!luai_numeq((a), (a)))
#endif



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#define lua_numbertointeger(n,p) \
  ((n) >= (LUA_സംഖ്യ)(LUA_MININTEGER) && \
   (n) < -(LUA_സംഖ്യ)(LUA_MININTEGER) && \
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
#എങ്കിൽ !defined(LUAI_FUNC)

#എങ്കിൽ defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#define LUAI_FUNC	__attribute__((visibility("internal"))) ബാഹ്യ
#അല്ലെങ്കിൽ
#define LUAI_FUNC	ബാഹ്യ
#endif

#define LUAI_DDEC(dec)	LUAI_FUNC dec
#define LUAI_DDEF	/* empty */

#endif


/* Give these macros simpler names for internal use */
#define l_likely(x)	luai_likely(x)
#define l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#എങ്കിൽ !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), വലുപ്പം(അക്ഷരം), (l), stdout)
#endif

/* print a newline and flush the output */
#എങ്കിൽ !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#എങ്കിൽ !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */

#endif

