/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#اگر_غیر_تعریف_شدہ luaconf_h
#تعریف_کرو luaconf_h

#شامل <limits.h>
#شامل <stddef.h>


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
#اگر !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#تعریف_کرو LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#اگر_ختم


#اگر defined(LUA_USE_WINDOWS)
#تعریف_کرو LUA_DL_DLL	/* enable support for DLL */
#تعریف_کرو LUA_USE_C89	/* broadly, Windows is C89 */
#اگر_ختم


/*
** When POSIX DLL ('LUA_USE_DLOPEN') is enabled, the Lua stand-alone
** application will try to dynamically link a 'readline' facility
** for its REPL.  In that case, LUA_READLINELIB is the name of the
** library it will look for those facilities.  If lua.c cannot open
** the specified library, it will generate a warning and then run
** without 'readline'.  If that macro is not defined, lua.c will not
** use 'readline'.
*/
#اگر defined(LUA_USE_LINUX)
#تعریف_کرو LUA_USE_POSIX
#تعریف_کرو LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#اگر !defined(LUA_READLINELIB)
#تعریف_کرو LUA_READLINELIB		"libreadline.so"
#اگر_ختم
#اگر_ختم


#اگر defined(LUA_USE_MACOSX)
#تعریف_کرو LUA_USE_POSIX
#تعریف_کرو LUA_USE_DLOPEN		/* macOS does not need -ldl */
#تعریف_کرو LUA_USE_READLINE	/* needs an extra library: -lreadline */
#اگر_ختم


#اگر defined(LUA_USE_IOS)
#تعریف_کرو LUA_USE_POSIX
#تعریف_کرو LUA_USE_DLOPEN
#اگر_ختم


#اگر defined(LUA_USE_C89) && defined(LUA_USE_POSIX)
#غلطی "POSIX is not compatible with C89"
#اگر_ختم


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#تعریف_کرو LUAI_IS32INT	((UINT_MAX >> 30) >= 3)

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
#تعریف_کرو LUA_INT_INT		1
#تعریف_کرو LUA_INT_LONG		2
#تعریف_کرو LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#تعریف_کرو LUA_FLOAT_FLOAT		1
#تعریف_کرو LUA_FLOAT_DOUBLE	2
#تعریف_کرو LUA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#تعریف_کرو LUA_INT_DEFAULT		LUA_INT_LONGLONG
#تعریف_کرو LUA_FLOAT_DEFAULT	LUA_FLOAT_DOUBLE


/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
/* #define LUA_32BITS */


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#اگر defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#تعریف_کرو LUA_C89_NUMBERS		1
#ورنہ
#تعریف_کرو LUA_C89_NUMBERS		0
#اگر_ختم


#اگر defined(LUA_32_بٹ)	/* { */
/*
** 32-bit integers and 'float'
*/
#اگر LUAI_IS32INT  /* use 'int' if big enough */
#تعریف_کرو LUA_INT_TYPE	LUA_INT_INT
#ورنہ  /* otherwise use 'long' */
#تعریف_کرو LUA_INT_TYPE	LUA_INT_LONG
#اگر_ختم
#تعریف_کرو LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#وگرنہ_اگر LUA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#تعریف_کرو LUA_INT_TYPE	LUA_INT_LONG
#تعریف_کرو LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#ورنہ		/* }{ */
/* use defaults */

#تعریف_کرو LUA_INT_TYPE	LUA_INT_DEFAULT
#تعریف_کرو LUA_FLOAT_TYPE	LUA_FLOAT_DEFAULT

#اگر_ختم				/* } */


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
#تعریف_کرو LUA_راستہ_SEP            ";"
#تعریف_کرو LUA_راستہ_MARK           "?"
#تعریف_کرو LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#تعریف_کرو LUA_VDIR	LUA_نسخہ_MAJOR "." LUA_نسخہ_MINOR
#اگر defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#تعریف_کرو LUA_LDIR	"!\\lua\\"
#تعریف_کرو LUA_CDIR	"!\\"
#تعریف_کرو LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"

#اگر !defined(LUA_راستہ_DEFAULT)
#تعریف_کرو LUA_راستہ_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
		LUA_SHRDIR "?.lua;"  LUA_SHRDIR "?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#اگر_ختم

#اگر !defined(LUA_CPATH_DEFAULT)
#تعریف_کرو LUA_CPATH_DEFAULT \
		LUA_CDIR "?.dll;" \
		LUA_CDIR "..\\lib\\lua\\"  LUA_VDIR "\\?.dll;" \
		LUA_CDIR "loadall.dll;" ".\\?.dll"
#اگر_ختم

#ورنہ			/* }{ */

#تعریف_کرو LUA_جڑ	"/usr/local/"
#تعریف_کرو LUA_LDIR	LUA_جڑ "share/lua/" LUA_VDIR "/"
#تعریف_کرو LUA_CDIR	LUA_جڑ "lib/lua/" LUA_VDIR "/"

#اگر !defined(LUA_راستہ_DEFAULT)
#تعریف_کرو LUA_راستہ_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#اگر_ختم

#اگر !defined(LUA_CPATH_DEFAULT)
#تعریف_کرو LUA_CPATH_DEFAULT \
		LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#اگر_ختم

#اگر_ختم			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#اگر !defined(LUA_DIRSEP)

#اگر defined(_WIN32)
#تعریف_کرو LUA_DIRSEP	"\\"
#ورنہ
#تعریف_کرو LUA_DIRSEP	"/"
#اگر_ختم

#اگر_ختم


/*
** LUA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the suffix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#تعریف_کرو LUA_IGMARK		"-"

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
#اگر defined(LUA_BUILD_AS_DLL)	/* { */

#اگر defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#تعریف_کرو LUA_API __declspec(dllexport)
#ورنہ						/* }{ */
#تعریف_کرو LUA_API __declspec(dllimport)
#اگر_ختم						/* } */

#ورنہ				/* }{ */

#تعریف_کرو LUA_API		خارجی

#اگر_ختم				/* } */


/*
** More often than not the libs go together with the core.
*/
#تعریف_کرو LUALIB_API	LUA_API

#اگر defined(__cplusplus)
/* Lua uses the "C name" when calling open functions */
#تعریف_کرو LUAMOD_API	خارجی "C"
#ورنہ
#تعریف_کرو LUAMOD_API	LUA_API
#اگر_ختم

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_GLOBAL avoids 'global' being a reserved word
*/
#اگر !defined(LUA_COMPAT_GLOBAL)
#تعریف_کرو LUA_COMPAT_GLOBAL	1
#اگر_ختم


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
#تعریف_کرو lua_strlen(L,i)		lua_rawlen(L, (i))

#تعریف_کرو lua_objlen(L,i)		lua_rawlen(L, (i))

#تعریف_کرو lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#تعریف_کرو lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

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

#تعریف_کرو l_floor(x)		(l_mathop(floor)(x))


/* now the variable definitions */

#اگر LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#تعریف_کرو LUA_نمبر	اعشاری

#تعریف_کرو l_floatatt(n)		(FLT_##n)

#تعریف_کرو LUAI_UACNUMBER	دوہرا

#تعریف_کرو LUA_نمبر_FRMLEN	""
#تعریف_کرو LUA_نمبر_FMT		"%.7g"
#تعریف_کرو LUA_نمبر_FMT_N	"%.9g"

#تعریف_کرو l_mathop(op)		op##f

#تعریف_کرو lua_str2number(s,p)	strtof((s), (p))


#وگرنہ_اگر LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#تعریف_کرو LUA_نمبر	طویل دوہرا

#تعریف_کرو l_floatatt(n)		(LDBL_##n)

#تعریف_کرو LUAI_UACNUMBER	طویل دوہرا

#تعریف_کرو LUA_نمبر_FRMLEN	"L"
#تعریف_کرو LUA_نمبر_FMT		"%.19Lg"
#تعریف_کرو LUA_نمبر_FMT_N	"%.21Lg"

#تعریف_کرو l_mathop(op)		op##l

#تعریف_کرو lua_str2number(s,p)	strtold((s), (p))

#وگرنہ_اگر LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#تعریف_کرو LUA_نمبر	دوہرا

#تعریف_کرو l_floatatt(n)		(DBL_##n)

#تعریف_کرو LUAI_UACNUMBER	دوہرا

#تعریف_کرو LUA_نمبر_FRMLEN	""
#تعریف_کرو LUA_نمبر_FMT		"%.15g"
#تعریف_کرو LUA_نمبر_FMT_N	"%.17g"

#تعریف_کرو l_mathop(op)		op

#تعریف_کرو lua_str2number(s,p)	strtod((s), (p))

#ورنہ						/* }{ */

#غلطی "numeric float type not defined"

#اگر_ختم					/* } */



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

#تعریف_کرو LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#تعریف_کرو LUAI_UACINT		LUA_INTEGER

#تعریف_کرو lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#تعریف_کرو LUA_UNSIGNED		غیرعلامتی LUAI_UACINT


/* now the variable definitions */

#اگر LUA_INT_TYPE == LUA_INT_INT		/* { int */

#تعریف_کرو LUA_INTEGER		صحیح
#تعریف_کرو LUA_INTEGER_FRMLEN	""

#تعریف_کرو LUA_MAXINTEGER		زیادہ_سے_زیادہ_عدد
#تعریف_کرو LUA_MININTEGER		INT_MIN

#تعریف_کرو LUA_MAXUNSIGNED		UINT_MAX

#وگرنہ_اگر LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#تعریف_کرو LUA_INTEGER		طویل
#تعریف_کرو LUA_INTEGER_FRMLEN	"l"

#تعریف_کرو LUA_MAXINTEGER		LONG_MAX
#تعریف_کرو LUA_MININTEGER		LONG_MIN

#تعریف_کرو LUA_MAXUNSIGNED		ULONG_MAX

#وگرنہ_اگر LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#اگر defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#تعریف_کرو LUA_INTEGER		طویل طویل
#تعریف_کرو LUA_INTEGER_FRMLEN	"ll"

#تعریف_کرو LUA_MAXINTEGER		LLONG_MAX
#تعریف_کرو LUA_MININTEGER		LLONG_MIN

#تعریف_کرو LUA_MAXUNSIGNED		ULLONG_MAX

#وگرنہ_اگر defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#تعریف_کرو LUA_INTEGER		__int64
#تعریف_کرو LUA_INTEGER_FRMLEN	"I64"

#تعریف_کرو LUA_MAXINTEGER		_I64_MAX
#تعریف_کرو LUA_MININTEGER		_I64_MIN

#تعریف_کرو LUA_MAXUNSIGNED		_UI64_MAX

#ورنہ				/* }{ */

#غلطی "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' برائے details)"

#اگر_ختم				/* } */

#ورنہ				/* }{ */

#غلطی "numeric integer type not defined"

#اگر_ختم				/* } */

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
#اگر !defined(LUA_USE_C89)
#تعریف_کرو l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#ورنہ
#تعریف_کرو l_sprintf(s,sz,f,i)	((خالی)(sz), sprintf(s,f,i))
#اگر_ختم


/*
@@ lua_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#اگر !defined(LUA_USE_C89)
#تعریف_کرو lua_strx2number(s,p)		lua_str2number(s,p)
#اگر_ختم


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#تعریف_کرو lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#اگر !defined(LUA_USE_C89)
#تعریف_کرو lua_number2strx(L,b,sz,f,n)  \
	((خالی)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#اگر_ختم


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#اگر defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#تعریف_منسوخ l_mathop  /* variants not available */
#تعریف_منسوخ lua_str2number
#تعریف_کرو l_mathop(op)		(lua_نمبر)op  /* no variant */
#تعریف_کرو lua_str2number(s,p)	((lua_نمبر)strtod((s), (p)))
#اگر_ختم


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#تعریف_کرو LUA_KCONTEXT	ptrdiff_t

#اگر !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#شامل <stdint.h>
#اگر defined(INTPTR_MAX)  /* even in C99 this type is optional */
#تعریف_منسوخ LUA_KCONTEXT
#تعریف_کرو LUA_KCONTEXT	intptr_t
#اگر_ختم
#اگر_ختم


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#اگر !defined(lua_getlocaledecpoint)
#تعریف_کرو lua_getlocaledecpoint()		(localeconv()->decimal_point[0])
#اگر_ختم


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#اگر !defined(luai_likely)

#اگر !defined(LUA_NOBUILTIN) && defined(__GNUC__) && (__GNUC__ >= 3)
#تعریف_کرو luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#تعریف_کرو luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#ورنہ
#تعریف_کرو luai_likely(x)		(x)
#تعریف_کرو luai_unlikely(x)	(x)
#اگر_ختم

#اگر_ختم



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
#تعریف_کرو LUA_EXTRASPACE		(حجم(خالی *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#تعریف_کرو LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#تعریف_کرو LUAL_BUFFERSIZE   ((صحیح)(16 * حجم(خالی*) * حجم(lua_نمبر)))


/*
@@ LUAI_MAXALIGN defines fields that ensure proper alignment for
** memory areas offered by Lua (e.g., userdata memory).
** Add fields to it if you need alignment for non-ISO objects.
*/
#اگر defined(LLONG_MAX)
/* use ISO C99 stuff */
#تعریف_کرو LUAI_MAXALIGN طویل دوہرا u; خالی *s; طویل طویل l
#ورنہ
/* use only C89 stuff */
#تعریف_کرو LUAI_MAXALIGN  lua_نمبر n; دوہرا u; خالی *s; lua_Integer i; طویل l
#اگر_ختم

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/



#اگر_ختم

