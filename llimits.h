/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل llimits_h
#وصف_ڪريو llimits_h


#ضمن <limits.h>
#ضمن <stddef.h>


#ضمن "lua.h"


#وصف_ڪريو l_numbits(t)	cast_int(حجم(t) * CHAR_BIT)

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
تعريفنوع ڊگهو l_mem;
تعريفنوع بنانشان ڊگهو lu_mem;
#جيڪڏهن_پورو				/* } */

#وصف_ڪريو MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
تعريفنوع بنانشان حرف lu_byte;
تعريفنوع نشانوارو حرف ls_byte;


/* Type for thread status/error codes */
تعريفنوع lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#وصف_ڪريو APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#وصف_ڪريو MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#وصف_ڪريو MAX_سائيز	(حجم(size_t) < حجم(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#وصف_ڪريو ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#وصف_ڪريو LL(x)   (حجم(x)/حجم(حرف) - 1)


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
#وصف_ڪريو L_P2I	uintptr_t
#وإلا  /* no 'intptr'? */
#وصف_ڪريو L_P2I	uintmax_t  /* use the largest available integer */
#جيڪڏهن_پورو
#وإلا  /* C89 option */
#وصف_ڪريو L_P2I	size_t
#جيڪڏهن_پورو

#وصف_ڪريو point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
تعريفنوع LUAI_UACNUMBER l_uac_عدد;
تعريفنوع LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#إذا defined LUAI_ASSERT
#وصف_رد NDEBUG
#ضمن <assert.h>
#وصف_ڪريو lua_assert(c)           أكد(c)
#وصف_ڪريو assert_ڪوڊ(c)		c
#جيڪڏهن_پورو

#إذا defined(lua_assert)
#وإلا
#وصف_ڪريو lua_assert(c)		((فراغ)0)
#وصف_ڪريو assert_ڪوڊ(c)		((فراغ)0)
#جيڪڏهن_پورو

#وصف_ڪريو check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#وصف_ڪريو lua_longassert(c)	assert_ڪوڊ((c) ? (فراغ)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#إذا !defined(اڻ_استعمال_ٿيل)
#وصف_ڪريو اڻ_استعمال_ٿيل(x)	((فراغ)(x))
#جيڪڏهن_پورو


/* type casts (a macro highlights casts in the code) */
#وصف_ڪريو cast(t, قوت_نما)	((t)(قوت_نما))

#وصف_ڪريو cast_void(i)	cast(فراغ, (i))
#وصف_ڪريو cast_voidp(i)	cast(فراغ *, (i))
#وصف_ڪريو cast_انگ(i)	cast(lua_عدد, (i))
#وصف_ڪريو cast_int(i)	cast(صحيح, (i))
#وصف_ڪريو cast_short(i)	cast(ننڍو, (i))
#وصف_ڪريو cast_uint(i)	cast(بنانشان صحيح, (i))
#وصف_ڪريو cast_byte(i)	cast(lu_byte, (i))
#وصف_ڪريو cast_uchar(i)	cast(بنانشان حرف, (i))
#وصف_ڪريو cast_char(i)	cast(حرف, (i))
#وصف_ڪريو cast_charp(i)	cast(حرف *, (i))
#وصف_ڪريو cast_sizet(i)	cast(size_t, (i))
#وصف_ڪريو cast_Integer(i)	cast(lua_Integer, (i))
#وصف_ڪريو cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#إذا !defined(l_castS2U)
#وصف_ڪريو l_castS2U(i)	((lua_Unsigned)(i))
#جيڪڏهن_پورو

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#إذا !defined(l_castU2S)
#وصف_ڪريو l_castU2S(i)	((lua_Integer)(i))
#جيڪڏهن_پورو

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#وصف_ڪريو cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#وصف_ڪريو ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#وصف_ڪريو ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

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
#وصف_ڪريو cast_func(p) (__extension__ (voidf)(p))
#وإلا
#وصف_ڪريو cast_func(p) ((voidf)(p))
#جيڪڏهن_پورو



/*
** non-return type
*/
#إذا !defined(l_noret)

#إذا defined(__GNUC__)
#وصف_ڪريو l_noret		فراغ __attribute__((noreturn))
#وإلاإذا defined(_MSC_VER) && _MSC_VER >= 1200
#وصف_ڪريو l_noret		فراغ __declspec(noreturn)
#وإلا
#وصف_ڪريو l_noret		فراغ
#جيڪڏهن_پورو

#جيڪڏهن_پورو


/*
** Inline functions
*/
#إذا !defined(LUA_USE_C89)
#وصف_ڪريو l_inline	مضمن
#وإلاإذا defined(__GNUC__)
#وصف_ڪريو l_inline	__inline__
#وإلا
#وصف_ڪريو l_inline	/*خالي_جايون*/
#جيڪڏهن_پورو

#وصف_ڪريو l_sinline	ساكن l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#إذا LUAI_IS32INT
تعريفنوع بنانشان صحيح l_uint32;
#وإلا
تعريفنوع بنانشان ڊگهو l_uint32;
#جيڪڏهن_پورو


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#إذا !defined(luai_numidiv)
#وصف_ڪريو luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#جيڪڏهن_پورو

/* float division */
#إذا !defined(luai_numdiv)
#وصف_ڪريو luai_numdiv(L,a,b)      ((فراغ)L, (a)/(b))
#جيڪڏهن_پورو

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
#وصف_ڪريو luai_nummod(L,a,b,m)  \
  { (فراغ)L; (m) = l_mathop(fmod)(a,b); \
    إذا (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#جيڪڏهن_پورو

/* exponentiation */
#إذا !defined(luai_numpow)
#وصف_ڪريو luai_numpow(L,a,b)  \
  ((فراغ)L, (b == 2) ? (a)*(a) : l_mathop(قوت)(a,b))
#جيڪڏهن_پورو

/* the others are quite standard operations */
#إذا !defined(luai_numadd)
#وصف_ڪريو luai_numadd(L,a,b)      ((فراغ)L, (a)+(b))
#وصف_ڪريو luai_numsub(L,a,b)      ((فراغ)L, (a)-(b))
#وصف_ڪريو luai_nummul(L,a,b)      ((فراغ)L, (a)*(b))
#وصف_ڪريو luai_numunm(L,a)        ((فراغ)L, -(a))
#وصف_ڪريو luai_numeq(a,b)         ((a)==(b))
#وصف_ڪريو luai_numlt(a,b)         ((a)<(b))
#وصف_ڪريو luai_numle(a,b)         ((a)<=(b))
#وصف_ڪريو luai_numgt(a,b)         ((a)>(b))
#وصف_ڪريو luai_numge(a,b)         ((a)>=(b))
#وصف_ڪريو luai_numisnan(a)        (!luai_numeq((a), (a)))
#جيڪڏهن_پورو



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#وصف_ڪريو lua_numbertointeger(n,p) \
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
#وصف_ڪريو LUAI_FUNC	__attribute__((visibility("internal"))) خارجي
#وإلا
#وصف_ڪريو LUAI_FUNC	خارجي
#جيڪڏهن_پورو

#وصف_ڪريو LUAI_DDEC(dec)	LUAI_FUNC dec
#وصف_ڪريو LUAI_DDEF	/*خالي_جايون*/

#جيڪڏهن_پورو


/* Give these macros simpler names for internal use */
#وصف_ڪريو l_likely(x)	luai_likely(x)
#وصف_ڪريو l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#إذا !defined(lua_writestring)
#وصف_ڪريو lua_writestring(s,l)   fwrite((s), حجم(حرف), (l), stdout)
#جيڪڏهن_پورو

/* print a newline and flush the output */
#إذا !defined(lua_writeline)
#وصف_ڪريو lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#جيڪڏهن_پورو

/* print an error message */
#إذا !defined(lua_writestringerror)
#وصف_ڪريو lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#جيڪڏهن_پورو

/* }================================================================== */

#جيڪڏهن_پورو

