/*
** $Id: llimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#अगर_अघोषित llimits_h
#घोषणा llimits_h


#समावेश <limits.h>
#समावेश <stddef.h>


#समावेश "lua.h"


#घोषणा l_numbits(t)	cast_int(माप(t) * अक्षर_बिट)

/*
** 'l_mem' is a signed integer big enough to count the total memory
** used by Lua.  (It is signed due to the use of debt in several
** computations.) 'lu_mem' is a corresponding unsigned type.  Usually,
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#अगर defined(LUAI_MEM)		/* { external definitions? */
प्रकार LUAI_MEM l_mem;
प्रकार LUAI_UMEM lu_mem;
#या_अगर LUAI_IS32INT	/* }{ */
प्रकार सूचक_भेद_प्रकार l_mem;
प्रकार माप_प्रकार lu_mem;
#अन्यथा  /* 16-bit ints */	/* }{ */
प्रकार दीर्घ l_mem;
प्रकार अचिन्हित दीर्घ lu_mem;
#पूर्ण_अगर				/* } */

#घोषणा MAX_LMEM  \
	cast(l_mem, (cast(lu_mem, 1) << (l_numbits(l_mem) - 1)) - 1)


/* chars used as small naturals (so that 'char' is reserved for characters) */
प्रकार अचिन्हित अक्षर lu_byte;
प्रकार चिन्हित अक्षर ls_byte;


/* Type for thread status/error codes */
प्रकार lu_byte TStatus;

/* The C API still uses 'int' for status/error codes */
#घोषणा APIstatus(st)	cast_int(st)

/* maximum value for size_t */
#घोषणा MAX_SIZET	((माप_प्रकार)(~(माप_प्रकार)0))

/*
** Maximum size for strings and userdata visible for Lua; should be
** representable as a lua_Integer and as a size_t.
*/
#घोषणा MAX_आकार	(माप(माप_प्रकार) < माप(lua_Integer) ? MAX_SIZET \
			  : cast_sizet(LUA_MAXINTEGER))

/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#घोषणा ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#घोषणा LL(x)   (माप(x)/माप(अक्षर) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#अगर !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#समावेश <stdint.h>
#अगर defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#घोषणा L_P2I	uintptr_t
#अन्यथा  /* no 'intptr'? */
#घोषणा L_P2I	uintmax_t  /* use the largest available integer */
#पूर्ण_अगर
#अन्यथा  /* C89 option */
#घोषणा L_P2I	माप_प्रकार
#पूर्ण_अगर

#घोषणा point2uint(p)	cast_uint((L_P2I)(p) & अच_पूर्णांक_उच्च)



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
प्रकार LUAI_UACNUMBER l_uac_संख्या;
प्रकार LUAI_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#अगर defined LUAI_ASSERT
#अघोषित न_संशोधन
#समावेश <assert.h>
#घोषणा lua_assert(c)           निश्चित(c)
#घोषणा assert_code(c)		c
#पूर्ण_अगर

#अगर defined(lua_assert)
#अन्यथा
#घोषणा lua_assert(c)		((व्योम)0)
#घोषणा assert_code(c)		((व्योम)0)
#पूर्ण_अगर

#घोषणा check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#घोषणा lua_longassert(c)	assert_code((c) ? (व्योम)0 : lua_assert(0))


/* macro to avoid warnings about unused variables */
#अगर !defined(UNUSED)
#घोषणा UNUSED(x)	((व्योम)(x))
#पूर्ण_अगर


/* type casts (a macro highlights casts in the code) */
#घोषणा cast(t, exp)	((t)(exp))

#घोषणा cast_void(i)	cast(व्योम, (i))
#घोषणा cast_voidp(i)	cast(व्योम *, (i))
#घोषणा cast_अंक(i)	cast(lua_संख्या, (i))
#घोषणा cast_int(i)	cast(पूर्णांक, (i))
#घोषणा cast_short(i)	cast(लघु, (i))
#घोषणा cast_uint(i)	cast(अचिन्हित पूर्णांक, (i))
#घोषणा cast_byte(i)	cast(lu_byte, (i))
#घोषणा cast_uchar(i)	cast(अचिन्हित अक्षर, (i))
#घोषणा cast_char(i)	cast(अक्षर, (i))
#घोषणा cast_charp(i)	cast(अक्षर *, (i))
#घोषणा cast_sizet(i)	cast(माप_प्रकार, (i))
#घोषणा cast_Integer(i)	cast(lua_Integer, (i))
#घोषणा cast_Inst(i)	cast(Instruction, (i))


/* cast a signed lua_Integer to lua_Unsigned */
#अगर !defined(l_castS2U)
#घोषणा l_castS2U(i)	((lua_Unsigned)(i))
#पूर्ण_अगर

/*
** cast a lua_Unsigned to a signed lua_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#अगर !defined(l_castU2S)
#घोषणा l_castU2S(i)	((lua_Integer)(i))
#पूर्ण_अगर

/*
** cast a size_t to lua_Integer: These casts are always valid for
** sizes of Lua objects (see MAX_SIZE)
*/
#घोषणा cast_st2S(sz)	((lua_Integer)(sz))

/* Cast a ptrdiff_t to size_t, when it is known that the minuend
** comes from the subtrahend (the base)
*/
#घोषणा ct_diff2sz(df)	((माप_प्रकार)(df))

/* ptrdiff_t to lua_Integer */
#घोषणा ct_diff2S(df)	cast_st2S(ct_diff2sz(df))

/*
** Special type equivalent to '(void*)' for functions (to suppress some
** warnings when converting function pointers)
*/
प्रकार व्योम (*voidf)(व्योम);

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#अगर defined(__GNUC__)
#घोषणा cast_func(p) (__extension__ (voidf)(p))
#अन्यथा
#घोषणा cast_func(p) ((voidf)(p))
#पूर्ण_अगर



/*
** non-return type
*/
#अगर !defined(l_noret)

#अगर defined(__GNUC__)
#घोषणा l_noret		व्योम __attribute__((noreturn))
#या_अगर defined(_MSC_VER) && _MSC_VER >= 1200
#घोषणा l_noret		व्योम __declspec(noreturn)
#अन्यथा
#घोषणा l_noret		व्योम
#पूर्ण_अगर

#पूर्ण_अगर


/*
** Inline functions
*/
#अगर !defined(LUA_USE_C89)
#घोषणा l_inline	अंतरभूत
#या_अगर defined(__GNUC__)
#घोषणा l_inline	__inline__
#अन्यथा
#घोषणा l_inline	/* empty */
#पूर्ण_अगर

#घोषणा l_sinline	अटल l_inline


/*
** An unsigned with (at least) 4 bytes
*/
#अगर LUAI_IS32INT
प्रकार अचिन्हित पूर्णांक l_uint32;
#अन्यथा
प्रकार अचिन्हित दीर्घ l_uint32;
#पूर्ण_अगर


/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#अगर !defined(luai_numidiv)
#घोषणा luai_numidiv(L,a,b)     l_floor(luai_numdiv(L,a,b))
#पूर्ण_अगर

/* float division */
#अगर !defined(luai_numdiv)
#घोषणा luai_numdiv(L,a,b)      ((व्योम)L, (a)/(b))
#पूर्ण_अगर

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
#अगर !defined(luai_nummod)
#घोषणा luai_nummod(L,a,b,m)  \
  { (व्योम)L; (m) = l_mathop(भ_शेष)(a,b); \
    अगर (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#पूर्ण_अगर

/* exponentiation */
#अगर !defined(luai_numpow)
#घोषणा luai_numpow(L,a,b)  \
  ((व्योम)L, (b == 2) ? (a)*(a) : l_mathop(घात)(a,b))
#पूर्ण_अगर

/* the others are quite standard operations */
#अगर !defined(luai_numadd)
#घोषणा luai_numadd(L,a,b)      ((व्योम)L, (a)+(b))
#घोषणा luai_numsub(L,a,b)      ((व्योम)L, (a)-(b))
#घोषणा luai_nummul(L,a,b)      ((व्योम)L, (a)*(b))
#घोषणा luai_numunm(L,a)        ((व्योम)L, -(a))
#घोषणा luai_numeq(a,b)         ((a)==(b))
#घोषणा luai_numlt(a,b)         ((a)<(b))
#घोषणा luai_numle(a,b)         ((a)<=(b))
#घोषणा luai_numgt(a,b)         ((a)>(b))
#घोषणा luai_numge(a,b)         ((a)>=(b))
#घोषणा luai_numisnan(a)        (!luai_numeq((a), (a)))
#पूर्ण_अगर



/*
** lua_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if the float is not within the range of
** a lua_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#घोषणा lua_numbertointeger(n,p) \
  ((n) >= (LUA_संख्या)(LUA_MININTEGER) && \
   (n) < -(LUA_संख्या)(LUA_MININTEGER) && \
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
#अगर !defined(LUAI_FUNC)

#अगर defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    (defined(__ELF__) || defined(__MACH__))
#घोषणा LUAI_FUNC	__attribute__((visibility("internal"))) बाह्य
#अन्यथा
#घोषणा LUAI_FUNC	बाह्य
#पूर्ण_अगर

#घोषणा LUAI_DDEC(dec)	LUAI_FUNC dec
#घोषणा LUAI_DDEF	/* empty */

#पूर्ण_अगर


/* Give these macros simpler names for internal use */
#घोषणा l_likely(x)	luai_likely(x)
#घोषणा l_unlikely(x)	luai_unlikely(x)

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#अगर !defined(lua_writestring)
#घोषणा lua_writestring(s,l)   ख_डालो((s), माप(अक्षर), (l), मानक_निकास)
#पूर्ण_अगर

/* print a newline and flush the output */
#अगर !defined(lua_writeline)
#घोषणा lua_writeline()        (lua_writestring("\n", 1), ख_साफ(मानक_निकास))
#पूर्ण_अगर

/* print an error message */
#अगर !defined(lua_writestringerror)
#घोषणा lua_writestringerror(s,p) \
        (ख_लिखो(मानक_त्रुटि, (s), (p)), ख_साफ(मानक_त्रुटि))
#पूर्ण_अगर

/* }================================================================== */

#पूर्ण_अगर

