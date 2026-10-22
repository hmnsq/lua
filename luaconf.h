/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#నిర్వచితం_కాకపోతే luaconf_h
#నిర్వచించు luaconf_h

#అన్తర్భావ <limits.h>
#అన్తర్భావ <stddef.h>


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
#ఒకవేళ !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#నిర్వచించు LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#అయితే_ముగింపు


#ఒకవేళ defined(LUA_USE_WINDOWS)
#నిర్వచించు LUA_DL_DLL	/* enable support for DLL */
#నిర్వచించు LUA_USE_C89	/* broadly, Windows is C89 */
#అయితే_ముగింపు


/*
** When POSIX DLL ('LUA_USE_DLOPEN') is enabled, the Lua stand-alone
** application will try to dynamically link a 'readline' facility
** for its REPL.  In that case, LUA_READLINELIB is the name of the
** library it will look for those facilities.  If lua.c cannot open
** the specified library, it will generate a warning and then run
** without 'readline'.  If that macro is not defined, lua.c will not
** use 'readline'.
*/
#ఒకవేళ defined(LUA_USE_LINUX)
#నిర్వచించు LUA_USE_POSIX
#నిర్వచించు LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#ఒకవేళ !defined(LUA_READLINELIB)
#నిర్వచించు LUA_READLINELIB		"libreadline.so"
#అయితే_ముగింపు
#అయితే_ముగింపు


#ఒకవేళ defined(LUA_USE_MACOSX)
#నిర్వచించు LUA_USE_POSIX
#నిర్వచించు LUA_USE_DLOPEN		/* macOS does not need -ldl */
#నిర్వచించు LUA_USE_READLINE	/* needs an extra library: -lreadline */
#అయితే_ముగింపు


#ఒకవేళ defined(LUA_USE_IOS)
#నిర్వచించు LUA_USE_POSIX
#నిర్వచించు LUA_USE_DLOPEN
#అయితే_ముగింపు


#ఒకవేళ defined(LUA_USE_C89) && defined(LUA_USE_POSIX)
#దోషం "POSIX is not compatible with C89"
#అయితే_ముగింపు


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#నిర్వచించు LUAI_IS32INT	((UINT_MAX >> 30) >= 3)

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
#నిర్వచించు LUA_INT_INT		1
#నిర్వచించు LUA_INT_LONG		2
#నిర్వచించు LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#నిర్వచించు LUA_FLOAT_FLOAT		1
#నిర్వచించు LUA_FLOAT_DOUBLE	2
#నిర్వచించు LUA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#నిర్వచించు LUA_INT_DEFAULT		LUA_INT_LONGLONG
#నిర్వచించు LUA_FLOAT_DEFAULT	LUA_FLOAT_DOUBLE


/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
/* #define LUA_32BITS */


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#ఒకవేళ defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#నిర్వచించు LUA_C89_NUMBERS		1
#లేకపోతే
#నిర్వచించు LUA_C89_NUMBERS		0
#అయితే_ముగింపు


#ఒకవేళ defined(LUA_32_బిట్లు)	/* { */
/*
** 32-bit integers and 'float'
*/
#ఒకవేళ LUAI_IS32INT  /* use 'int' if big enough */
#నిర్వచించు LUA_INT_TYPE	LUA_INT_INT
#లేకపోతే  /* otherwise use 'long' */
#నిర్వచించు LUA_INT_TYPE	LUA_INT_LONG
#అయితే_ముగింపు
#నిర్వచించు LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#లేదావేళ LUA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#నిర్వచించు LUA_INT_TYPE	LUA_INT_LONG
#నిర్వచించు LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#లేకపోతే		/* }{ */
/* use defaults */

#నిర్వచించు LUA_INT_TYPE	LUA_INT_DEFAULT
#నిర్వచించు LUA_FLOAT_TYPE	LUA_FLOAT_DEFAULT

#అయితే_ముగింపు				/* } */


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
#నిర్వచించు LUA_మార్గం_SEP            ";"
#నిర్వచించు LUA_మార్గం_MARK           "?"
#నిర్వచించు LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#నిర్వచించు LUA_VDIR	LUA_సంస్కరణ_MAJOR "." LUA_సంస్కరణ_MINOR
#ఒకవేళ defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#నిర్వచించు LUA_LDIR	"!\\lua\\"
#నిర్వచించు LUA_CDIR	"!\\"
#నిర్వచించు LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"

#ఒకవేళ !defined(LUA_మార్గం_DEFAULT)
#నిర్వచించు LUA_మార్గం_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
		LUA_SHRDIR "?.lua;"  LUA_SHRDIR "?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_CPATH_DEFAULT)
#నిర్వచించు LUA_CPATH_DEFAULT \
		LUA_CDIR "?.dll;" \
		LUA_CDIR "..\\lib\\lua\\"  LUA_VDIR "\\?.dll;" \
		LUA_CDIR "loadall.dll;" ".\\?.dll"
#అయితే_ముగింపు

#లేకపోతే			/* }{ */

#నిర్వచించు LUA_మూలం	"/usr/local/"
#నిర్వచించు LUA_LDIR	LUA_మూలం "share/lua/" LUA_VDIR "/"
#నిర్వచించు LUA_CDIR	LUA_మూలం "lib/lua/" LUA_VDIR "/"

#ఒకవేళ !defined(LUA_మార్గం_DEFAULT)
#నిర్వచించు LUA_మార్గం_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_CPATH_DEFAULT)
#నిర్వచించు LUA_CPATH_DEFAULT \
		LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#అయితే_ముగింపు

#అయితే_ముగింపు			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#ఒకవేళ !defined(LUA_DIRSEP)

#ఒకవేళ defined(_WIN32)
#నిర్వచించు LUA_DIRSEP	"\\"
#లేకపోతే
#నిర్వచించు LUA_DIRSEP	"/"
#అయితే_ముగింపు

#అయితే_ముగింపు


/*
** LUA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the suffix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#నిర్వచించు LUA_IGMARK		"-"

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
#ఒకవేళ defined(LUA_BUILD_AS_DLL)	/* { */

#ఒకవేళ defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#నిర్వచించు LUA_API __declspec(dllexport)
#లేకపోతే						/* }{ */
#నిర్వచించు LUA_API __declspec(dllimport)
#అయితే_ముగింపు						/* } */

#లేకపోతే				/* }{ */

#నిర్వచించు LUA_API		బాహ్య

#అయితే_ముగింపు				/* } */


/*
** More often than not the libs go together with the core.
*/
#నిర్వచించు LUALIB_API	LUA_API

#ఒకవేళ defined(__cplusplus)
/* Lua uses the "C name" when calling open functions */
#నిర్వచించు LUAMOD_API	బాహ్య "C"
#లేకపోతే
#నిర్వచించు LUAMOD_API	LUA_API
#అయితే_ముగింపు

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_GLOBAL avoids 'global' being a reserved word
*/
#ఒకవేళ !defined(LUA_COMPAT_GLOBAL)
#నిర్వచించు LUA_COMPAT_GLOBAL	1
#అయితే_ముగింపు


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
#నిర్వచించు lua_strlen(L,i)		lua_rawlen(L, (i))

#నిర్వచించు lua_objlen(L,i)		lua_rawlen(L, (i))

#నిర్వచించు lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#నిర్వచించు lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

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

#నిర్వచించు l_floor(x)		(l_mathop(floor)(x))


/* now the variable definitions */

#ఒకవేళ LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#నిర్వచించు LUA_సంఖ్య	దశాంశం

#నిర్వచించు l_floatatt(n)		(FLT_##n)

#నిర్వచించు LUAI_UACNUMBER	ద్విగుణ

#నిర్వచించు LUA_సంఖ్య_FRMLEN	""
#నిర్వచించు LUA_సంఖ్య_FMT		"%.7g"
#నిర్వచించు LUA_సంఖ్య_FMT_N	"%.9g"

#నిర్వచించు l_mathop(op)		op##f

#నిర్వచించు lua_str2number(s,p)	strtof((s), (p))


#లేదావేళ LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#నిర్వచించు LUA_సంఖ్య	దీర్ఘ ద్విగుణ

#నిర్వచించు l_floatatt(n)		(LDBL_##n)

#నిర్వచించు LUAI_UACNUMBER	దీర్ఘ ద్విగుణ

#నిర్వచించు LUA_సంఖ్య_FRMLEN	"L"
#నిర్వచించు LUA_సంఖ్య_FMT		"%.19Lg"
#నిర్వచించు LUA_సంఖ్య_FMT_N	"%.21Lg"

#నిర్వచించు l_mathop(op)		op##l

#నిర్వచించు lua_str2number(s,p)	strtold((s), (p))

#లేదావేళ LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#నిర్వచించు LUA_సంఖ్య	ద్విగుణ

#నిర్వచించు l_floatatt(n)		(DBL_##n)

#నిర్వచించు LUAI_UACNUMBER	ద్విగుణ

#నిర్వచించు LUA_సంఖ్య_FRMLEN	""
#నిర్వచించు LUA_సంఖ్య_FMT		"%.15g"
#నిర్వచించు LUA_సంఖ్య_FMT_N	"%.17g"

#నిర్వచించు l_mathop(op)		op

#నిర్వచించు lua_str2number(s,p)	strtod((s), (p))

#లేకపోతే						/* }{ */

#దోషం "numeric float type not defined"

#అయితే_ముగింపు					/* } */



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

#నిర్వచించు LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#నిర్వచించు LUAI_UACINT		LUA_INTEGER

#నిర్వచించు lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#నిర్వచించు LUA_UNSIGNED		గుర్తులేని LUAI_UACINT


/* now the variable definitions */

#ఒకవేళ LUA_INT_TYPE == LUA_INT_INT		/* { int */

#నిర్వచించు LUA_INTEGER		పూర్ణాంకం
#నిర్వచించు LUA_INTEGER_FRMLEN	""

#నిర్వచించు LUA_MAXINTEGER		గరిష్ఠ_పూర్ణాంకం
#నిర్వచించు LUA_MININTEGER		INT_MIN

#నిర్వచించు LUA_MAXUNSIGNED		UINT_MAX

#లేదావేళ LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#నిర్వచించు LUA_INTEGER		దీర్ఘ
#నిర్వచించు LUA_INTEGER_FRMLEN	"l"

#నిర్వచించు LUA_MAXINTEGER		LONG_MAX
#నిర్వచించు LUA_MININTEGER		LONG_MIN

#నిర్వచించు LUA_MAXUNSIGNED		ULONG_MAX

#లేదావేళ LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#ఒకవేళ defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#నిర్వచించు LUA_INTEGER		దీర్ఘ దీర్ఘ
#నిర్వచించు LUA_INTEGER_FRMLEN	"ll"

#నిర్వచించు LUA_MAXINTEGER		LLONG_MAX
#నిర్వచించు LUA_MININTEGER		LLONG_MIN

#నిర్వచించు LUA_MAXUNSIGNED		ULLONG_MAX

#లేదావేళ defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#నిర్వచించు LUA_INTEGER		__int64
#నిర్వచించు LUA_INTEGER_FRMLEN	"I64"

#నిర్వచించు LUA_MAXINTEGER		_I64_MAX
#నిర్వచించు LUA_MININTEGER		_I64_MIN

#నిర్వచించు LUA_MAXUNSIGNED		_UI64_MAX

#లేకపోతే				/* }{ */

#దోషం "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' కొరకు details)"

#అయితే_ముగింపు				/* } */

#లేకపోతే				/* }{ */

#దోషం "numeric integer type not defined"

#అయితే_ముగింపు				/* } */

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
#ఒకవేళ !defined(LUA_USE_C89)
#నిర్వచించు l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#లేకపోతే
#నిర్వచించు l_sprintf(s,sz,f,i)	((శూన్య)(sz), sprintf(s,f,i))
#అయితే_ముగింపు


/*
@@ lua_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#ఒకవేళ !defined(LUA_USE_C89)
#నిర్వచించు lua_strx2number(s,p)		lua_str2number(s,p)
#అయితే_ముగింపు


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#నిర్వచించు lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#ఒకవేళ !defined(LUA_USE_C89)
#నిర్వచించు lua_number2strx(L,b,sz,f,n)  \
	((శూన్య)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#అయితే_ముగింపు


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#ఒకవేళ defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#నిర్వచనం_రద్దు l_mathop  /* variants not available */
#నిర్వచనం_రద్దు lua_str2number
#నిర్వచించు l_mathop(op)		(lua_సంఖ్య)op  /* no variant */
#నిర్వచించు lua_str2number(s,p)	((lua_సంఖ్య)strtod((s), (p)))
#అయితే_ముగింపు


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#నిర్వచించు LUA_KCONTEXT	ptrdiff_t

#ఒకవేళ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#అన్తర్భావ <stdint.h>
#ఒకవేళ defined(INTPTR_MAX)  /* even in C99 this type is optional */
#నిర్వచనం_రద్దు LUA_KCONTEXT
#నిర్వచించు LUA_KCONTEXT	intptr_t
#అయితే_ముగింపు
#అయితే_ముగింపు


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#ఒకవేళ !defined(lua_getlocaledecpoint)
#నిర్వచించు lua_getlocaledecpoint()		(localeconv()->decimal_point[0])
#అయితే_ముగింపు


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#ఒకవేళ !defined(luai_likely)

#ఒకవేళ !defined(LUA_NOBUILTIN) && defined(__GNUC__) && (__GNUC__ >= 3)
#నిర్వచించు luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#నిర్వచించు luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#లేకపోతే
#నిర్వచించు luai_likely(x)		(x)
#నిర్వచించు luai_unlikely(x)	(x)
#అయితే_ముగింపు

#అయితే_ముగింపు



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
#నిర్వచించు LUA_EXTRASPACE		(పరిమాణం(శూన్య *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#నిర్వచించు LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#నిర్వచించు LUAL_BUFFERSIZE   ((పూర్ణాంకం)(16 * పరిమాణం(శూన్య*) * పరిమాణం(lua_సంఖ్య)))


/*
@@ LUAI_MAXALIGN defines fields that ensure proper alignment for
** memory areas offered by Lua (e.g., userdata memory).
** Add fields to it if you need alignment for non-ISO objects.
*/
#ఒకవేళ defined(LLONG_MAX)
/* use ISO C99 stuff */
#నిర్వచించు LUAI_MAXALIGN దీర్ఘ ద్విగుణ u; శూన్య *s; దీర్ఘ దీర్ఘ l
#లేకపోతే
/* use only C89 stuff */
#నిర్వచించు LUAI_MAXALIGN  lua_సంఖ్య n; ద్విగుణ u; శూన్య *s; lua_Integer i; దీర్ఘ l
#అయితే_ముగింపు

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/



#అయితే_ముగింపు

