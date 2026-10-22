/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#यदि_अपरिभाषितम् luaconf_h
#परिभाषय luaconf_h

#अन्तर्भाव <limits.h>
#अन्तर्भाव <stddef.h>


/*
** ===================================================================
** General Configuration File for Lua
**
** Some definitions here can be changed externally, through the compiler
** (e.g., with '-D' options): They are commented out or protected
** by '#if !defined' guards. However, several other definitions
** should be changed directly here, either because they affect the
** Lua ABI (by making the changes here, you ensure that all software
** connected to Lua, such as C libraries, will be compiled with the same
** configuration); or because they are seldom changed.
**
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/


/*
** {====================================================================
** System Configuration: macros to adapt (if needed) Lua to some
** particular platform, for instance restricting it to C89.
** =====================================================================
*/

/*
@@ LUA_USE_C89 controls the use of non-ISO-C89 features.
** Define it if you want Lua to avoid the use of a few C99 features
** or Windows-specific features on Windows.
*/
/* #define LUA_USE_C89 */


/*
** By default, Lua on Windows use (some) specific Windows features
*/
#यदि !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#परिभाषय LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#यदि_अन्तः


#यदि defined(LUA_USE_WINDOWS)
#परिभाषय LUA_DL_DLL	/* enable support for DLL */
#परिभाषय LUA_USE_C89	/* broadly, Windows is C89 */
#यदि_अन्तः


/*
** When POSIX DLL ('LUA_USE_DLOPEN') is enabled, the Lua stand-alone
** application will try to dynamically link a 'readline' facility
** for its REPL.  In that case, LUA_READLINELIB is the name of the
** library it will look for those facilities.  If lua.c cannot open
** the specified library, it will generate a warning and then run
** without 'readline'.  If that macro is not defined, lua.c will not
** use 'readline'.
*/
#यदि defined(LUA_USE_LINUX)
#परिभाषय LUA_USE_POSIX
#परिभाषय LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#यदि !defined(LUA_READLINELIB)
#परिभाषय LUA_READLINELIB		"libreadline.so"
#यदि_अन्तः
#यदि_अन्तः


#यदि defined(LUA_USE_MACOSX)
#परिभाषय LUA_USE_POSIX
#परिभाषय LUA_USE_DLOPEN		/* macOS does not need -ldl */
#परिभाषय LUA_USE_READLINE	/* needs an extra library: -lreadline */
#यदि_अन्तः


#यदि defined(LUA_USE_IOS)
#परिभाषय LUA_USE_POSIX
#परिभाषय LUA_USE_DLOPEN
#यदि_अन्तः


#यदि defined(LUA_USE_C89) && defined(LUA_USE_POSIX)
#दोषः "POSIX is not compatible with C89"
#यदि_अन्तः


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#परिभाषय LUAI_IS32INT	((UINT_MAX >> 30) >= 3)

/* }================================================================== */



/*
** {==================================================================
** Configuration for Number types. These options should not be
** set externally, because any other code connected to Lua must
** use the same configuration.
** ===================================================================
*/

/*
@@ LUA_INT_TYPE defines the type for Lua integers.
@@ LUA_FLOAT_TYPE defines the type for Lua floats.
** Lua should work fine with any mix of these options supported
** by your C compiler. The usual configurations are 64-bit integers
** and 'double' (the default), 32-bit integers and 'float' (for
** restricted platforms), and 'long'/'double' (for C compilers not
** compliant with C99, which may not have support for 'long long').
*/

/* predefined options for LUA_INT_TYPE */
#परिभाषय LUA_INT_INT		1
#परिभाषय LUA_INT_LONG		2
#परिभाषय LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#परिभाषय LUA_FLOAT_FLOAT		1
#परिभाषय LUA_FLOAT_DOUBLE	2
#परिभाषय LUA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#परिभाषय LUA_INT_DEFAULT		LUA_INT_LONGLONG
#परिभाषय LUA_FLOAT_DEFAULT	LUA_FLOAT_DOUBLE


/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
/* #define LUA_32BITS */


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#यदि defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#परिभाषय LUA_C89_NUMBERS		1
#अन्यथा
#परिभाषय LUA_C89_NUMBERS		0
#यदि_अन्तः


#यदि defined(LUA_32_द्वयङ्काः)	/* { */
/*
** 32-bit integers and 'float'
*/
#यदि LUAI_IS32INT  /* use 'int' if big enough */
#परिभाषय LUA_INT_TYPE	LUA_INT_INT
#अन्यथा  /* otherwise use 'long' */
#परिभाषय LUA_INT_TYPE	LUA_INT_LONG
#यदि_अन्तः
#परिभाषय LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#नोचेद्यदि LUA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#परिभाषय LUA_INT_TYPE	LUA_INT_LONG
#परिभाषय LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#अन्यथा		/* }{ */
/* use defaults */

#परिभाषय LUA_INT_TYPE	LUA_INT_DEFAULT
#परिभाषय LUA_FLOAT_TYPE	LUA_FLOAT_DEFAULT

#यदि_अन्तः				/* } */


/* }================================================================== */



/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** LUA_PATH_SEP is the character that separates templates in a path.
** LUA_PATH_MARK is the string that marks the substitution points in a
** template.
** LUA_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#परिभाषय LUA_मार्गः_SEP            ";"
#परिभाषय LUA_मार्गः_MARK           "?"
#परिभाषय LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#परिभाषय LUA_VDIR	LUA_संस्करणम्_MAJOR "." LUA_संस्करणम्_MINOR
#यदि defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#परिभाषय LUA_LDIR	"!\\lua\\"
#परिभाषय LUA_CDIR	"!\\"
#परिभाषय LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"

#यदि !defined(LUA_मार्गः_DEFAULT)
#परिभाषय LUA_मार्गः_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
		LUA_SHRDIR "?.lua;"  LUA_SHRDIR "?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#यदि_अन्तः

#यदि !defined(LUA_CPATH_DEFAULT)
#परिभाषय LUA_CPATH_DEFAULT \
		LUA_CDIR "?.dll;" \
		LUA_CDIR "..\\lib\\lua\\"  LUA_VDIR "\\?.dll;" \
		LUA_CDIR "loadall.dll;" ".\\?.dll"
#यदि_अन्तः

#अन्यथा			/* }{ */

#परिभाषय LUA_मूलम्	"/usr/local/"
#परिभाषय LUA_LDIR	LUA_मूलम् "share/lua/" LUA_VDIR "/"
#परिभाषय LUA_CDIR	LUA_मूलम् "lib/lua/" LUA_VDIR "/"

#यदि !defined(LUA_मार्गः_DEFAULT)
#परिभाषय LUA_मार्गः_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#यदि_अन्तः

#यदि !defined(LUA_CPATH_DEFAULT)
#परिभाषय LUA_CPATH_DEFAULT \
		LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#यदि_अन्तः

#यदि_अन्तः			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#यदि !defined(LUA_DIRSEP)

#यदि defined(_WIN32)
#परिभाषय LUA_DIRSEP	"\\"
#अन्यथा
#परिभाषय LUA_DIRSEP	"/"
#यदि_अन्तः

#यदि_अन्तः


/*
** LUA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the suffix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#परिभाषय LUA_IGMARK		"-"

/* }================================================================== */


/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
@@ LUA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all auxiliary library functions.
@@ LUAMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** LUA_BUILD_AS_DLL to get it).
*/
#यदि defined(LUA_BUILD_AS_DLL)	/* { */

#यदि defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#परिभाषय LUA_API __declspec(dllexport)
#अन्यथा						/* }{ */
#परिभाषय LUA_API __declspec(dllimport)
#यदि_अन्तः						/* } */

#अन्यथा				/* }{ */

#परिभाषय LUA_API		बाह्य

#यदि_अन्तः				/* } */


/*
** More often than not the libs go together with the core.
*/
#परिभाषय LUALIB_API	LUA_API

#यदि defined(__cplusplus)
/* Lua uses the "C name" when calling open functions */
#परिभाषय LUAMOD_API	बाह्य "C"
#अन्यथा
#परिभाषय LUAMOD_API	LUA_API
#यदि_अन्तः

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_GLOBAL avoids 'global' being a reserved word
*/
#यदि !defined(LUA_COMPAT_GLOBAL)
#परिभाषय LUA_COMPAT_GLOBAL	1
#यदि_अन्तः


/*
@@ LUA_COMPAT_MATHLIB controls the presence of several deprecated
** functions in the mathematical library.
** (These functions were already officially removed in 5.3;
** nevertheless they are still available here.)
*/
/* #define LUA_COMPAT_MATHLIB */


/*
@@ The following macros supply trivial compatibility for some
** changes in the API. The macros themselves document how to
** change your code to avoid using them.
** (Once more, these macros were officially removed in 5.3, but they are
** still available here.)
*/
#परिभाषय lua_strlen(L,i)		lua_rawlen(L, (i))

#परिभाषय lua_objlen(L,i)		lua_rawlen(L, (i))

#परिभाषय lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#परिभाषय lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

/* }================================================================== */



/*
** {==================================================================
** Configuration for Numbers (low-level part).
** Change these definitions if no predefined LUA_FLOAT_* / LUA_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ LUAI_UACNUMBER is the result of a 'default argument promotion'
@@ over a floating number.
@@ l_floatatt(x) corrects float attribute 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ LUA_NUMBER_FRMLEN is the length modifier for writing floats.
@@ LUA_NUMBER_FMT is the format for writing floats with the maximum
** number of digits that respects tostring(tonumber(numeral)) == numeral.
** (That would be floor(log10(2^n)), where n is the number of bits in
** the float mantissa.)
@@ LUA_NUMBER_FMT_N is the format for writing floats with the minimum
** number of digits that ensures tonumber(tostring(number)) == number.
** (That would be LUA_NUMBER_FMT+2.)
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ lua_str2number converts a decimal numeral to a number.
*/


/* The following definition is good for most cases here */

#परिभाषय l_floor(x)		(l_mathop(floor)(x))


/* now the variable definitions */

#यदि LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#परिभाषय LUA_सङ्ख्या	भिन्न

#परिभाषय l_floatatt(n)		(FLT_##n)

#परिभाषय LUAI_UACNUMBER	द्विगुण

#परिभाषय LUA_सङ्ख्या_FRMLEN	""
#परिभाषय LUA_सङ्ख्या_FMT		"%.7g"
#परिभाषय LUA_सङ्ख्या_FMT_N	"%.9g"

#परिभाषय l_mathop(op)		op##f

#परिभाषय lua_str2number(s,p)	strtof((s), (p))


#नोचेद्यदि LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#परिभाषय LUA_सङ्ख्या	दीर्घ द्विगुण

#परिभाषय l_floatatt(n)		(LDBL_##n)

#परिभाषय LUAI_UACNUMBER	दीर्घ द्विगुण

#परिभाषय LUA_सङ्ख्या_FRMLEN	"L"
#परिभाषय LUA_सङ्ख्या_FMT		"%.19Lg"
#परिभाषय LUA_सङ्ख्या_FMT_N	"%.21Lg"

#परिभाषय l_mathop(op)		op##l

#परिभाषय lua_str2number(s,p)	strtold((s), (p))

#नोचेद्यदि LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#परिभाषय LUA_सङ्ख्या	द्विगुण

#परिभाषय l_floatatt(n)		(DBL_##n)

#परिभाषय LUAI_UACNUMBER	द्विगुण

#परिभाषय LUA_सङ्ख्या_FRMLEN	""
#परिभाषय LUA_सङ्ख्या_FMT		"%.15g"
#परिभाषय LUA_सङ्ख्या_FMT_N	"%.17g"

#परिभाषय l_mathop(op)		op

#परिभाषय lua_str2number(s,p)	strtod((s), (p))

#अन्यथा						/* }{ */

#दोषः "numeric float type not defined"

#यदि_अन्तः					/* } */



/*
@@ LUA_UNSIGNED is the unsigned version of LUA_INTEGER.
@@ LUAI_UACINT is the result of a 'default argument promotion'
@@ over a LUA_INTEGER.
@@ LUA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ LUA_INTEGER_FMT is the format for writing integers.
@@ LUA_MAXINTEGER is the maximum value for a LUA_INTEGER.
@@ LUA_MININTEGER is the minimum value for a LUA_INTEGER.
@@ LUA_MAXUNSIGNED is the maximum value for a LUA_UNSIGNED.
@@ lua_integer2str converts an integer to a string.
*/


/* The following definitions are good for most cases here */

#परिभाषय LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#परिभाषय LUAI_UACINT		LUA_INTEGER

#परिभाषय lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#परिभाषय LUA_UNSIGNED		अचिह्नित LUAI_UACINT


/* now the variable definitions */

#यदि LUA_INT_TYPE == LUA_INT_INT		/* { int */

#परिभाषय LUA_INTEGER		पूर्णाङ्क
#परिभाषय LUA_INTEGER_FRMLEN	""

#परिभाषय LUA_MAXINTEGER		अधिकतम_पूर्णाङ्कः
#परिभाषय LUA_MININTEGER		INT_MIN

#परिभाषय LUA_MAXUNSIGNED		UINT_MAX

#नोचेद्यदि LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#परिभाषय LUA_INTEGER		दीर्घ
#परिभाषय LUA_INTEGER_FRMLEN	"l"

#परिभाषय LUA_MAXINTEGER		LONG_MAX
#परिभाषय LUA_MININTEGER		LONG_MIN

#परिभाषय LUA_MAXUNSIGNED		ULONG_MAX

#नोचेद्यदि LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#यदि defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#परिभाषय LUA_INTEGER		दीर्घ दीर्घ
#परिभाषय LUA_INTEGER_FRMLEN	"ll"

#परिभाषय LUA_MAXINTEGER		LLONG_MAX
#परिभाषय LUA_MININTEGER		LLONG_MIN

#परिभाषय LUA_MAXUNSIGNED		ULLONG_MAX

#नोचेद्यदि defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#परिभाषय LUA_INTEGER		__int64
#परिभाषय LUA_INTEGER_FRMLEN	"I64"

#परिभाषय LUA_MAXINTEGER		_I64_MAX
#परिभाषय LUA_MININTEGER		_I64_MIN

#परिभाषय LUA_MAXUNSIGNED		_UI64_MAX

#अन्यथा				/* }{ */

#दोषः "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' प्रति details)"

#यदि_अन्तः				/* } */

#अन्यथा				/* }{ */

#दोषः "numeric integer type not defined"

#यदि_अन्तः				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in Lua have only one format item.)
*/
#यदि !defined(LUA_USE_C89)
#परिभाषय l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#अन्यथा
#परिभाषय l_sprintf(s,sz,f,i)	((शून्य)(sz), sprintf(s,f,i))
#यदि_अन्तः


/*
@@ lua_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#यदि !defined(LUA_USE_C89)
#परिभाषय lua_strx2number(s,p)		lua_str2number(s,p)
#यदि_अन्तः


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#परिभाषय lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#यदि !defined(LUA_USE_C89)
#परिभाषय lua_number2strx(L,b,sz,f,n)  \
	((शून्य)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#यदि_अन्तः


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#यदि defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#अपरिभाषय l_mathop  /* variants not available */
#अपरिभाषय lua_str2number
#परिभाषय l_mathop(op)		(lua_सङ्ख्या)op  /* no variant */
#परिभाषय lua_str2number(s,p)	((lua_सङ्ख्या)strtod((s), (p)))
#यदि_अन्तः


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#परिभाषय LUA_KCONTEXT	ptrdiff_t

#यदि !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#अन्तर्भाव <stdint.h>
#यदि defined(INTPTR_MAX)  /* even in C99 this type is optional */
#अपरिभाषय LUA_KCONTEXT
#परिभाषय LUA_KCONTEXT	intptr_t
#यदि_अन्तः
#यदि_अन्तः


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#यदि !defined(lua_getlocaledecpoint)
#परिभाषय lua_getlocaledecpoint()		(localeconv()->decimal_point[0])
#यदि_अन्तः


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#यदि !defined(luai_likely)

#यदि !defined(LUA_NOBUILTIN) && defined(__GNUC__) && (__GNUC__ >= 3)
#परिभाषय luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#परिभाषय luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#अन्यथा
#परिभाषय luai_likely(x)		(x)
#परिभाषय luai_unlikely(x)	(x)
#यदि_अन्तः

#यदि_अन्तः



/* }================================================================== */


/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
@@ LUA_NOCVTN2S/LUA_NOCVTS2N control how Lua performs some
** coercions. Define LUA_NOCVTN2S to turn off automatic coercion from
** numbers to strings. Define LUA_NOCVTS2N to turn off automatic
** coercion from strings to numbers.
*/
/* #define LUA_NOCVTN2S */
/* #define LUA_NOCVTS2N */


/*
@@ LUA_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
/* #define LUA_USE_APICHECK */

/* }================================================================== */


/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile Lua and when you compile code that links to
** Lua).
** =====================================================================
*/

/*
@@ LUA_EXTRASPACE defines the size of a raw memory area associated with
** a Lua state with very fast access.
** CHANGE it if you need a different size.
*/
#परिभाषय LUA_EXTRASPACE		(परिमाण(शून्य *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#परिभाषय LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#परिभाषय LUAL_BUFFERSIZE   ((पूर्णाङ्क)(16 * परिमाण(शून्य*) * परिमाण(lua_सङ्ख्या)))


/*
@@ LUAI_MAXALIGN defines fields that ensure proper alignment for
** memory areas offered by Lua (e.g., userdata memory).
** Add fields to it if you need alignment for non-ISO objects.
*/
#यदि defined(LLONG_MAX)
/* use ISO C99 stuff */
#परिभाषय LUAI_MAXALIGN दीर्घ द्विगुण u; शून्य *s; दीर्घ दीर्घ l
#अन्यथा
/* use only C89 stuff */
#परिभाषय LUAI_MAXALIGN  lua_सङ्ख्या n; द्विगुण u; शून्य *s; lua_Integer i; दीर्घ l
#यदि_अन्तः

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/



#यदि_अन्तः

