/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् llimits_h
#परिभाषय llimits_h


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stddef.h>


#अन्तर्भाव "lua.h"


#परिभाषय l_numbits(t)	cast_int(परिमाण(t) * CHAR_BIT)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#यदि defined(LUAI_MEM)		/* { external definitions? */
प्रकारनिर्देश LUAI_MEM l_mem;
प्रकारनिर्देश LUAI_UMEM lu_mem;
#नोचेद्यदि LUAI_IS32INT	/* }{ */
प्रकारनिर्देश ptrdiff_t l_mem;
प्रकारनिर्देश size_t lu_mem;
#अन्यथा  /* 16-bit ints */	/* }{ */
प्रकारनिर्देश दीर्घ l_mem;
प्रकारनिर्देश अचिह्नित दीर्घ lu_mem;
#यदि_अन्तः				/* } */

#परिभाषय MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
प्रकारनिर्देश अचिह्नित वर्ण lu_byte;
प्रकारनिर्देश चिह्नित वर्ण ls_byte;


/* Type for thread status/error codes */
प्रकारनिर्देश lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#परिभाषय APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#परिभाषय MAX_SIZET	((size_t)(~(size_t)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#परिभाषय MAX_आकार	(परिमाण(size_t) < परिमाण(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#परिभाषय ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#परिभाषय LL(x)   (परिमाण(x)/परिमाण(वर्ण) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#यदि !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#अन्तर्भाव <stdint.h>
#यदि defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#परिभाषय L_P2I	uintptr_t
#अन्यथा  /* no 'intptr'? */
#परिभाषय L_P2I	uintmax_t  /* use the largest available integer */
#यदि_अन्तः
#अन्यथा  /* C89 option */
#परिभाषय L_P2I	size_t
#यदि_अन्तः

#परिभाषय point2uint(p)	cast_uint((L_P2I)(p) & UINT_MAX)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
प्रकारनिर्देश LUAI_UACNUMBER l_uac_सङ्ख्या;
प्रकारनिर्देश LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#यदि defined LUAI_ASSERT
#अपरिभाषय NDEBUG
#अन्तर्भाव <assert.h>
#परिभाषय lua_assert(c)           प्रतिज्ञा(c)
#परिभाषय assert_संहिता(c)		c
#यदि_अन्तः

#यदि defined(lua_assert)
#अन्यथा
#परिभाषय lua_assert(c)		((शून्य)0)
#परिभाषय assert_संहिता(c)		((शून्य)0)
#यदि_अन्तः

#परिभाषय check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#परिभाषय lua_longassert(c)	assert_संहिता((c) ? (शून्य)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#यदि !defined(अप्रयुक्तम्)
#परिभाषय अप्रयुक्तम्(x)	((शून्य)(x))
#यदि_अन्तः


/* type casts (a macro highlights casts in the code) */
#परिभाषय cast(t, घातांकः)	((t)(घातांकः))

#परिभाषय cast_void(i)	cast(शून्य, (i))
#परिभाषय cast_voidp(i)	cast(शून्य *, (i))
#परिभाषय cast_अङ्कः(i)	cast(lua_सङ्ख्या, (i))
#परिभाषय cast_int(i)	cast(पूर्णाङ्क, (i))
#परिभाषय cast_short(i)	cast(ह्रस्व, (i))
#परिभाषय cast_uint(i)	cast(अचिह्नित पूर्णाङ्क, (i))
#परिभाषय cast_byte(i)	cast(lu_byte, (i))
#परिभाषय cast_uchar(i)	cast(अचिह्नित वर्ण, (i))
#परिभाषय cast_char(i)	cast(वर्ण, (i))
#परिभाषय cast_charp(i)	cast(वर्ण *, (i))
#परिभाषय cast_sizet(i)	cast(size_t, (i))
#परिभाषय cast_Integer(i)	cast(lua_Integer, (i))
#परिभाषय cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#यदि !defined(l_castS2U)
#परिभाषय l_castS2U(i)	((lua_Unsigned)(i))
#यदि_अन्तः

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#यदि !defined(l_castU2S)
#परिभाषय l_castU2S(i)	((lua_Integer)(i))
#यदि_अन्तः

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#परिभाषय cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#परिभाषय ct_diff2sz(df)	((size_t)(df))

/* ptrdiff_t to lua_Integer */
#परिभाषय ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
प्रकारनिर्देश शून्य (*voidf)(शून्य);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#यदि defined(__GNUC__)
#परिभाषय cast_func(p) (__extension__ (voidf)(p))
#अन्यथा
#परिभाषय cast_func(p) ((voidf)(p))
#यदि_अन्तः



/*
** non-return type
*/
#यदि !defined(l_noret)

#यदि defined(__GNUC__)
#परिभाषय l_noret		शून्य __attribute__((noreturn))
#नोचेद्यदि defined(_MSC_VER) && _MSC_VER >= 1200
#परिभाषय l_noret		शून्य __declspec(noreturn)
#अन्यथा
#परिभाषय l_noret		शून्य
#यदि_अन्तः

#यदि_अन्तः


/*
** Inline functions
*/
#यदि !defined(LUA_USE_C89)
#परिभाषय l_inline	अन्तःस्थ
#नोचेद्यदि defined(__GNUC__)
#परिभाषय l_inline	__inline__
#अन्यथा
#परिभाषय l_inline	/*रिक्तानि*/
#यदि_अन्तः

#परिभाषय l_sinline	स्थिर l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#यदि LUAI_IS32INT
प्रकारनिर्देश अचिह्नित पूर्णाङ्क l_uint32;
#अन्यथा
प्रकारनिर्देश अचिह्नित दीर्घ l_uint32;
#यदि_अन्तः


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#यदि !defined(luai_numidiv)
#परिभाषय luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#यदि_अन्तः

/* float division */
#यदि !defined(luai_numdiv)
#परिभाषय luai_numdiv(L,a,b)      ((शून्य)L, (a)/(b))
#यदि_अन्तः

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
#यदि !defined(luai_nummod)
#परिभाषय luai_nummod(L,a,b,m)  \
  { (शून्य)L; (m) = l_mathop(fmod)(a,b); \
    यदि (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#यदि_अन्तः

/* exponentiation */
#यदि !defined(luai_numpow)
#परिभाषय luai_numpow(L,a,b)  \
  ((शून्य)L, (b == 2) ? (a)*(a) : l_mathop(घातः)(a,b))
#यदि_अन्तः

/* the others are quite standard operations */
#यदि !defined(luai_numadd)
#परिभाषय luai_numadd(L,a,b)      ((शून्य)L, (a)+(b))
#परिभाषय luai_numsub(L,a,b)      ((शून्य)L, (a)-(b))
#परिभाषय luai_nummul(L,a,b)      ((शून्य)L, (a)*(b))
#परिभाषय luai_numunm(L,a)        ((शून्य)L, -(a))
#परिभाषय luai_numeq(a,b)         ((a)==(b))
#परिभाषय luai_numlt(a,b)         ((a)<(b))
#परिभाषय luai_numle(a,b)         ((a)<=(b))
#परिभाषय luai_numgt(a,b)         ((a)>(b))
#परिभाषय luai_numge(a,b)         ((a)>=(b))
#परिभाषय luai_numisnan(a)        (!luai_numeq((a), (a)))
#यदि_अन्तः



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#परिभाषय lua_numbertointeger(n,p) \
  ((n) >= (LUA_सङ्ख्या)(LUA_MININTEGER) && \
   (n) < -(LUA_सङ्ख्या)(LUA_MININTEGER) && \
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
#यदि !defined(LUAI_FUNC)

#यदि defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#परिभाषय LUAI_FUNC	__attribute__((visibility("internal"))) बाह्य
#अन्यथा
#परिभाषय LUAI_FUNC	बाह्य
#यदि_अन्तः

#परिभाषय LUAI_DDEC(dec)	LUAI_FUNC dec
#परिभाषय LUAI_DDEF	/*रिक्तानि*/

#यदि_अन्तः


/* Give these macros simpler names for internal use */
#परिभाषय l_likely(x)	luai_likely(x)
#परिभाषय l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#यदि !defined(lua_writestring)
#परिभाषय lua_writestring(s,l)   fwrite((s), परिमाण(वर्ण), (l), stdout)
#यदि_अन्तः

/* print a newline and flush the output */
#यदि !defined(lua_writeline)
#परिभाषय lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#यदि_अन्तः

/* print an error message */
#यदि !defined(lua_writestringerror)
#परिभाषय lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#यदि_अन्तः

/* }================================================================== */

#यदि_अन्तः

