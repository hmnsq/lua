/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#ifndef llimits_h
#define llimits_h


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>


#ଅନ୍ତର୍ଭାଵ "lua.h"


#define l_numbits(t)	cast_int(ଆକାର(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#ଯଦି defined(LUAI_MEM)		/* { external definitions? */
ପ୍ରକାରବ୍ୟାଖ୍ୟା LUAI_MEM l_mem;
ପ୍ରକାରବ୍ୟାଖ୍ୟା LUAI_UMEM lu_mem;
#ନଚେତଯଦି LUAI_IS32INT	/* }{ */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ptrdiff_t l_mem;
ପ୍ରକାରବ୍ୟାଖ୍ୟା size_t lu_mem;
#ଅନ୍ୟଥା  /* 16-bit ints */	/* }{ */
ପ୍ରକାରବ୍ୟାଖ୍ୟା long l_mem;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long lu_mem;
#endif				/* } */

#define MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର lu_byte;
ପ୍ରକାରବ୍ୟାଖ୍ୟା signed ଅକ୍ଷର ls_byte;


/* Type for thread status/error codes */
ପ୍ରକାରବ୍ୟାଖ୍ୟା lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#define APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#define MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#define MAX_SIZE	(ଆକାର(size_t) < ଆକାର(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#define ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#define LL(x)   (ଆକାର(x)/ଆକାର(ଅକ୍ଷର) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#ଯଦି !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଯଦି defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#define L_P2I	uintptr_t
#ଅନ୍ୟଥା  /* no 'intptr'? */
#define L_P2I	uintmax_t  /* use the largest available integer */
#endif
#ଅନ୍ୟଥା  /* C89 option */
#define L_P2I	size_t
#endif

#define point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
ପ୍ରକାରବ୍ୟାଖ୍ୟା LUAI_UACNUMBER l_uac_ସଂଖ୍ୟା;
ପ୍ରକାରବ୍ୟାଖ୍ୟା LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#ଯଦି defined LUAI_ASSERT
#undef NDEBUG
#ଅନ୍ତର୍ଭାଵ <assert.h>
#define lua_assert(c)           ପ୍ରତିଜ୍ଞା(c)
#define assert_code(c)		c
#endif

#ଯଦି defined(lua_assert)
#ଅନ୍ୟଥା
#define lua_assert(c)		((ଶୂନ୍ୟ)0)
#define assert_code(c)		((ଶୂନ୍ୟ)0)
#endif

#define check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#define lua_longassert(c)	assert_code((c) ? (ଶୂନ୍ୟ)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#ଯଦି !defined(UNUSED)
#define UNUSED(x)	((ଶୂନ୍ୟ)(x))
#endif


/* type casts (a macro highlights casts in the code) */
#define cast(t, exp)	((t)(exp))

#define cast_void(i)	cast(ଶୂନ୍ୟ, (i))
#define cast_voidp(i)	cast(ଶୂନ୍ୟ *, (i))
#define cast_ଅଙ୍କ(i)	cast(lua_ସଂଖ୍ୟା, (i))
#define cast_int(i)	cast(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, (i))
#define cast_short(i)	cast(short, (i))
#define cast_uint(i)	cast(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, (i))
#define cast_byte(i)	cast(lu_byte, (i))
#define cast_uchar(i)	cast(unsigned ଅକ୍ଷର, (i))
#define cast_char(i)	cast(ଅକ୍ଷର, (i))
#define cast_charp(i)	cast(ଅକ୍ଷର *, (i))
#define cast_sizet(i)	cast(size_t, (i))
#define cast_Integer(i)	cast(lua_Integer, (i))
#define cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#ଯଦି !defined(l_castS2U)
#define l_castS2U(i)	((lua_Unsigned)(i))
#endif

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#ଯଦି !defined(l_castU2S)
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ (*voidf)(ଶୂନ୍ୟ);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#ଯଦି defined(__GNUC__)
#define cast_func(p) (__extension__ (voidf)(p))
#ଅନ୍ୟଥା
#define cast_func(p) ((voidf)(p))
#endif



/*
** non-return type
*/
#ଯଦି !defined(l_noret)

#ଯଦି defined(__GNUC__)
#define l_noret		ଶୂନ୍ୟ __attribute__((noreturn))
#ନଚେତଯଦି defined(_MSC_VER) && _MSC_VER >= 1200
#define l_noret		ଶୂନ୍ୟ __declspec(noreturn)
#ଅନ୍ୟଥା
#define l_noret		ଶୂନ୍ୟ
#endif

#endif


/*
** Inline functions
*/
#ଯଦି !defined(LUA_USE_C89)
#define l_inline	ଧାଡ଼ିଭିତରେ
#ନଚେତଯଦି defined(__GNUC__)
#define l_inline	__inline__
#ଅନ୍ୟଥା
#define l_inline	/* empty */
#endif

#define l_sinline	ସ୍ଥିର l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#ଯଦି LUAI_IS32INT
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା l_uint32;
#ଅନ୍ୟଥା
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long l_uint32;
#endif


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#ଯଦି !defined(luai_numidiv)
#define luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#endif

/* float division */
#ଯଦି !defined(luai_numdiv)
#define luai_numdiv(L,a,b)      ((ଶୂନ୍ୟ)L, (a)/(b))
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
#ଯଦି !defined(luai_nummod)
#define luai_nummod(L,a,b,m)  \
  { (ଶୂନ୍ୟ)L; (m) = l_mathop(fmod)(a,b); \
    ଯଦି (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#endif

/* exponentiation */
#ଯଦି !defined(luai_numpow)
#define luai_numpow(L,a,b)  \
  ((ଶୂନ୍ୟ)L, (b == 2) ? (a)*(a) : l_mathop(pow)(a,b))
#endif

/* the others are quite standard operations */
#ଯଦି !defined(luai_numadd)
#define luai_numadd(L,a,b)      ((ଶୂନ୍ୟ)L, (a)+(b))
#define luai_numsub(L,a,b)      ((ଶୂନ୍ୟ)L, (a)-(b))
#define luai_nummul(L,a,b)      ((ଶୂନ୍ୟ)L, (a)*(b))
#define luai_numunm(L,a)        ((ଶୂନ୍ୟ)L, -(a))
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
  ((n) >= (LUA_ସଂଖ୍ୟା)(LUA_MININTEGER) && \
   (n) < -(LUA_ସଂଖ୍ୟା)(LUA_MININTEGER) && \
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
#ଯଦି !defined(LUAI_FUNC)

#ଯଦି defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#define LUAI_FUNC	__attribute__((visibility("internal"))) ବାହ୍ୟ
#ଅନ୍ୟଥା
#define LUAI_FUNC	ବାହ୍ୟ
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
#ଯଦି !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), ଆକାର(ଅକ୍ଷର), (l), stdout)
#endif

/* print a newline and flush the output */
#ଯଦି !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#ଯଦି !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */

#endif

