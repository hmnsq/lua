/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే llimits_h
#నిర్వచించు llimits_h


#అన్తర్భావ <limits.h>
#అన్తర్భావ <stddef.h>


#అన్తర్భావ "lua.h"


#నిర్వచించు l_numbits(t)	cast_int(పరిమాణం(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#ఒకవేళ defined(LUAI_MEM)		/* { external definitions? */
రకనిర్వచనం LUAI_MEM l_mem;
రకనిర్వచనం LUAI_UMEM lu_mem;
#లేదావేళ LUAI_IS32INT	/* }{ */
రకనిర్వచనం ptrdiff_t l_mem;
రకనిర్వచనం size_t lu_mem;
#లేకపోతే  /* 16-bit ints */	/* }{ */
రకనిర్వచనం దీర్ఘ l_mem;
రకనిర్వచనం గుర్తులేని దీర్ఘ lu_mem;
#అయితే_ముగింపు				/* } */

#నిర్వచించు MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
రకనిర్వచనం గుర్తులేని అక్షరం lu_byte;
రకనిర్వచనం గుర్తుగల అక్షరం ls_byte;


/* Type for thread status/error codes */
రకనిర్వచనం lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#నిర్వచించు APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#నిర్వచించు MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#నిర్వచించు MAX_కొలత	(పరిమాణం(size_t) < పరిమాణం(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#నిర్వచించు ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#నిర్వచించు LL(x)   (పరిమాణం(x)/పరిమాణం(అక్షరం) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#ఒకవేళ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#అన్తర్భావ <stdint.h>
#ఒకవేళ defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#నిర్వచించు L_P2I	uintptr_t
#లేకపోతే  /* no 'intptr'? */
#నిర్వచించు L_P2I	uintmax_t  /* use the largest available integer */
#అయితే_ముగింపు
#లేకపోతే  /* C89 option */
#నిర్వచించు L_P2I	size_t
#అయితే_ముగింపు

#నిర్వచించు point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
రకనిర్వచనం LUAI_UACNUMBER l_uac_సంఖ్య;
రకనిర్వచనం LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#ఒకవేళ defined LUAI_ASSERT
#నిర్వచనం_రద్దు NDEBUG
#అన్తర్భావ <assert.h>
#నిర్వచించు lua_assert(c)           ప్రతిజ్ఞా(c)
#నిర్వచించు assert_సంకేతం(c)		c
#అయితే_ముగింపు

#ఒకవేళ defined(lua_assert)
#లేకపోతే
#నిర్వచించు lua_assert(c)		((శూన్య)0)
#నిర్వచించు assert_సంకేతం(c)		((శూన్య)0)
#అయితే_ముగింపు

#నిర్వచించు check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#నిర్వచించు lua_longassert(c)	assert_సంకేతం((c) ? (శూన్య)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#ఒకవేళ !defined(వాడని)
#నిర్వచించు వాడని(x)	((శూన్య)(x))
#అయితే_ముగింపు


/* type casts (a macro highlights casts in the code) */
#నిర్వచించు cast(t, ఘాతాంకం)	((t)(ఘాతాంకం))

#నిర్వచించు cast_void(i)	cast(శూన్య, (i))
#నిర్వచించు cast_voidp(i)	cast(శూన్య *, (i))
#నిర్వచించు cast_అంకె(i)	cast(lua_సంఖ్య, (i))
#నిర్వచించు cast_int(i)	cast(పూర్ణాంకం, (i))
#నిర్వచించు cast_short(i)	cast(పొట్టి, (i))
#నిర్వచించు cast_uint(i)	cast(గుర్తులేని పూర్ణాంకం, (i))
#నిర్వచించు cast_byte(i)	cast(lu_byte, (i))
#నిర్వచించు cast_uchar(i)	cast(గుర్తులేని అక్షరం, (i))
#నిర్వచించు cast_char(i)	cast(అక్షరం, (i))
#నిర్వచించు cast_charp(i)	cast(అక్షరం *, (i))
#నిర్వచించు cast_sizet(i)	cast(size_t, (i))
#నిర్వచించు cast_Integer(i)	cast(lua_Integer, (i))
#నిర్వచించు cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#ఒకవేళ !defined(l_castS2U)
#నిర్వచించు l_castS2U(i)	((lua_Unsigned)(i))
#అయితే_ముగింపు

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#ఒకవేళ !defined(l_castU2S)
#నిర్వచించు l_castU2S(i)	((lua_Integer)(i))
#అయితే_ముగింపు

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#నిర్వచించు cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#నిర్వచించు ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#నిర్వచించు ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
రకనిర్వచనం శూన్య (*voidf)(శూన్య);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#ఒకవేళ defined(__GNUC__)
#నిర్వచించు cast_func(p) (__extension__ (voidf)(p))
#లేకపోతే
#నిర్వచించు cast_func(p) ((voidf)(p))
#అయితే_ముగింపు



/*
** non-return type
*/
#ఒకవేళ !defined(l_noret)

#ఒకవేళ defined(__GNUC__)
#నిర్వచించు l_noret		శూన్య __attribute__((noreturn))
#లేదావేళ defined(_MSC_VER) && _MSC_VER >= 1200
#నిర్వచించు l_noret		శూన్య __declspec(noreturn)
#లేకపోతే
#నిర్వచించు l_noret		శూన్య
#అయితే_ముగింపు

#అయితే_ముగింపు


/*
** Inline functions
*/
#ఒకవేళ !defined(LUA_USE_C89)
#నిర్వచించు l_inline	అంతఃవరుస
#లేదావేళ defined(__GNUC__)
#నిర్వచించు l_inline	__inline__
#లేకపోతే
#నిర్వచించు l_inline	/*ఖాళీలు*/
#అయితే_ముగింపు

#నిర్వచించు l_sinline	స్థిర l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#ఒకవేళ LUAI_IS32INT
రకనిర్వచనం గుర్తులేని పూర్ణాంకం l_uint32;
#లేకపోతే
రకనిర్వచనం గుర్తులేని దీర్ఘ l_uint32;
#అయితే_ముగింపు


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#ఒకవేళ !defined(luai_numidiv)
#నిర్వచించు luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#అయితే_ముగింపు

/* float division */
#ఒకవేళ !defined(luai_numdiv)
#నిర్వచించు luai_numdiv(L,a,b)      ((శూన్య)L, (a)/(b))
#అయితే_ముగింపు

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
#ఒకవేళ !defined(luai_nummod)
#నిర్వచించు luai_nummod(L,a,b,m)  \
  { (శూన్య)L; (m) = l_mathop(fmod)(a,b); \
    ఒకవేళ (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#అయితే_ముగింపు

/* exponentiation */
#ఒకవేళ !defined(luai_numpow)
#నిర్వచించు luai_numpow(L,a,b)  \
  ((శూన్య)L, (b == 2) ? (a)*(a) : l_mathop(ఘాతం)(a,b))
#అయితే_ముగింపు

/* the others are quite standard operations */
#ఒకవేళ !defined(luai_numadd)
#నిర్వచించు luai_numadd(L,a,b)      ((శూన్య)L, (a)+(b))
#నిర్వచించు luai_numsub(L,a,b)      ((శూన్య)L, (a)-(b))
#నిర్వచించు luai_nummul(L,a,b)      ((శూన్య)L, (a)*(b))
#నిర్వచించు luai_numunm(L,a)        ((శూన్య)L, -(a))
#నిర్వచించు luai_numeq(a,b)         ((a)==(b))
#నిర్వచించు luai_numlt(a,b)         ((a)<(b))
#నిర్వచించు luai_numle(a,b)         ((a)<=(b))
#నిర్వచించు luai_numgt(a,b)         ((a)>(b))
#నిర్వచించు luai_numge(a,b)         ((a)>=(b))
#నిర్వచించు luai_numisnan(a)        (!luai_numeq((a), (a)))
#అయితే_ముగింపు



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#నిర్వచించు lua_numbertointeger(n,p) \
  ((n) >= (LUA_సంఖ్య)(LUA_MININTEGER) && \
   (n) < -(LUA_సంఖ్య)(LUA_MININTEGER) && \
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
#ఒకవేళ !defined(LUAI_FUNC)

#ఒకవేళ defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#నిర్వచించు LUAI_FUNC	__attribute__((visibility("internal"))) బాహ్య
#లేకపోతే
#నిర్వచించు LUAI_FUNC	బాహ్య
#అయితే_ముగింపు

#నిర్వచించు LUAI_DDEC(dec)	LUAI_FUNC dec
#నిర్వచించు LUAI_DDEF	/*ఖాళీలు*/

#అయితే_ముగింపు


/* Give these macros simpler names for internal use */
#నిర్వచించు l_likely(x)	luai_likely(x)
#నిర్వచించు l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#ఒకవేళ !defined(lua_writestring)
#నిర్వచించు lua_writestring(s,l)   fwrite((s), పరిమాణం(అక్షరం), (l), stdout)
#అయితే_ముగింపు

/* print a newline and flush the output */
#ఒకవేళ !defined(lua_writeline)
#నిర్వచించు lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#అయితే_ముగింపు

/* print an error message */
#ఒకవేళ !defined(lua_writestringerror)
#నిర్వచించు lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#అయితే_ముగింపు

/* }================================================================== */

#అయితే_ముగింపు

