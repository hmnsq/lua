/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ luaconf_h
#ವ್ಯಾಖ್ಯಾನಿಸು luaconf_h

#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>


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
#ಆದರೆ !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ defined(LUA_USE_WINDOWS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DL_DLL	/* enable support for DLL */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_C89	/* broadly, Windows is C89 */
#ಆದರೆ_ಅಂತ್ಯ


/*
** When POSIX DLL ('LUA_USE_DLOPEN') is enabled, the Lua stand-alone
** application will try to dynamically link a 'readline' facility
** for its REPL.  In that case, LUA_READLINELIB is the name of the
** library it will look for those facilities.  If lua.c cannot open
** the specified library, it will generate a warning and then run
** without 'readline'.  If that macro is not defined, lua.c will not
** use 'readline'.
*/
#ಆದರೆ defined(LUA_USE_LINUX)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_POSIX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#ಆದರೆ !defined(LUA_READLINELIB)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_READLINELIB		"libreadline.so"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ defined(LUA_USE_MACOSX)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_POSIX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_DLOPEN		/* macOS does not need -ldl */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_READLINE	/* needs an extra library: -lreadline */
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ defined(LUA_USE_IOS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_POSIX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_DLOPEN
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ defined(LUA_USE_C89) && defined(LUA_USE_POSIX)
#ದೋಷ "POSIX is not compatible with C89"
#ಆದರೆ_ಅಂತ್ಯ


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_IS32INT	((UINT_MAX >> 30) >= 3)

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
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_INT		1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_LONG		2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_FLOAT		1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_DOUBLE	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_DEFAULT		LUA_INT_LONGLONG
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_DEFAULT	LUA_FLOAT_DOUBLE


/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
/* #define LUA_32BITS */


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#ಆದರೆ defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_C89_NUMBERS		1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_C89_NUMBERS		0
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ defined(LUA_32_ಬಿಟ್ಟುಗಳು)	/* { */
/*
** 32-bit integers and 'float'
*/
#ಆದರೆ LUAI_IS32INT  /* use 'int' if big enough */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_TYPE	LUA_INT_INT
#ಇಲ್ಲದಿದ್ದರೆ  /* otherwise use 'long' */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_TYPE	LUA_INT_LONG
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#ಅಥವಾಆದರೆ LUA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_TYPE	LUA_INT_LONG
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#ಇಲ್ಲದಿದ್ದರೆ		/* }{ */
/* use defaults */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INT_TYPE	LUA_INT_DEFAULT
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOAT_TYPE	LUA_FLOAT_DEFAULT

#ಆದರೆ_ಅಂತ್ಯ				/* } */


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
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಮಾರ್ಗ_SEP            ";"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಮಾರ್ಗ_MARK           "?"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VDIR	LUA_ಆವೃತ್ತಿ_MAJOR "." LUA_ಆವೃತ್ತಿ_MINOR
#ಆದರೆ defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LDIR	"!\\lua\\"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CDIR	"!\\"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"

#ಆದರೆ !defined(LUA_ಮಾರ್ಗ_DEFAULT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಮಾರ್ಗ_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
		LUA_SHRDIR "?.lua;"  LUA_SHRDIR "?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_CPATH_DEFAULT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CPATH_DEFAULT \
		LUA_CDIR "?.dll;" \
		LUA_CDIR "..\\lib\\lua\\"  LUA_VDIR "\\?.dll;" \
		LUA_CDIR "loadall.dll;" ".\\?.dll"
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ			/* }{ */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಬೇರು	"/usr/local/"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LDIR	LUA_ಬೇರು "share/lua/" LUA_VDIR "/"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CDIR	LUA_ಬೇರು "lib/lua/" LUA_VDIR "/"

#ಆದರೆ !defined(LUA_ಮಾರ್ಗ_DEFAULT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಮಾರ್ಗ_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_CPATH_DEFAULT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CPATH_DEFAULT \
		LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#ಆದರೆ !defined(LUA_DIRSEP)

#ಆದರೆ defined(_WIN32)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DIRSEP	"\\"
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DIRSEP	"/"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ


/*
** LUA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the suffix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_IGMARK		"-"

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
#ಆದರೆ defined(LUA_BUILD_AS_DLL)	/* { */

#ಆದರೆ defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_API __declspec(dllexport)
#ಇಲ್ಲದಿದ್ದರೆ						/* }{ */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_API __declspec(dllimport)
#ಆದರೆ_ಅಂತ್ಯ						/* } */

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_API		ಬಾಹ್ಯ

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** More often than not the libs go together with the core.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUALIB_API	LUA_API

#ಆದರೆ defined(__cplusplus)
/* Lua uses the "C name" when calling open functions */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAMOD_API	ಬಾಹ್ಯ "C"
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUAMOD_API	LUA_API
#ಆದರೆ_ಅಂತ್ಯ

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_GLOBAL avoids 'global' being a reserved word
*/
#ಆದರೆ !defined(LUA_COMPAT_GLOBAL)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COMPAT_GLOBAL	1
#ಆದರೆ_ಅಂತ್ಯ


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
#ವ್ಯಾಖ್ಯಾನಿಸು lua_strlen(L,i)		lua_rawlen(L, (i))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_objlen(L,i)		lua_rawlen(L, (i))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

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

#ವ್ಯಾಖ್ಯಾನಿಸು l_floor(x)		(l_mathop(floor)(x))


/* now the variable definitions */

#ಆದರೆ LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ	ದಶಮಾಂಶ

#ವ್ಯಾಖ್ಯಾನಿಸು l_floatatt(n)		(FLT_##n)

#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_UACNUMBER	ದ್ವಿಗುಣ

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FRMLEN	""
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT		"%.7g"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT_N	"%.9g"

#ವ್ಯಾಖ್ಯಾನಿಸು l_mathop(op)		op##f

#ವ್ಯಾಖ್ಯಾನಿಸು lua_str2number(s,p)	strtof((s), (p))


#ಅಥವಾಆದರೆ LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ	ದೀರ್ಘ ದ್ವಿಗುಣ

#ವ್ಯಾಖ್ಯಾನಿಸು l_floatatt(n)		(LDBL_##n)

#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_UACNUMBER	ದೀರ್ಘ ದ್ವಿಗುಣ

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FRMLEN	"L"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT		"%.19Lg"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT_N	"%.21Lg"

#ವ್ಯಾಖ್ಯಾನಿಸು l_mathop(op)		op##l

#ವ್ಯಾಖ್ಯಾನಿಸು lua_str2number(s,p)	strtold((s), (p))

#ಅಥವಾಆದರೆ LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ	ದ್ವಿಗುಣ

#ವ್ಯಾಖ್ಯಾನಿಸು l_floatatt(n)		(DBL_##n)

#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_UACNUMBER	ದ್ವಿಗುಣ

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FRMLEN	""
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT		"%.15g"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಸಂಖ್ಯೆ_FMT_N	"%.17g"

#ವ್ಯಾಖ್ಯಾನಿಸು l_mathop(op)		op

#ವ್ಯಾಖ್ಯಾನಿಸು lua_str2number(s,p)	strtod((s), (p))

#ಇಲ್ಲದಿದ್ದರೆ						/* }{ */

#ದೋಷ "numeric float type not defined"

#ಆದರೆ_ಅಂತ್ಯ					/* } */



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

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_UACINT		LUA_INTEGER

#ವ್ಯಾಖ್ಯಾನಿಸು lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_UNSIGNED		ಚಿಹ್ನೆರಹಿತ LUAI_UACINT


/* now the variable definitions */

#ಆದರೆ LUA_INT_TYPE == LUA_INT_INT		/* { int */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER		ಪೂರ್ಣಾಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER_FRMLEN	""

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXINTEGER		ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MININTEGER		INT_MIN

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXUNSIGNED		UINT_MAX

#ಅಥವಾಆದರೆ LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER		ದೀರ್ಘ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER_FRMLEN	"l"

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXINTEGER		LONG_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MININTEGER		LONG_MIN

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXUNSIGNED		ULONG_MAX

#ಅಥವಾಆದರೆ LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#ಆದರೆ defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER		ದೀರ್ಘ ದೀರ್ಘ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER_FRMLEN	"ll"

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXINTEGER		LLONG_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MININTEGER		LLONG_MIN

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXUNSIGNED		ULLONG_MAX

#ಅಥವಾಆದರೆ defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER		__int64
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INTEGER_FRMLEN	"I64"

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXINTEGER		_I64_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MININTEGER		_I64_MIN

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXUNSIGNED		_UI64_MAX

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

#ದೋಷ "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' ಗಾಗಿ details)"

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

#ದೋಷ "numeric integer type not defined"

#ಆದರೆ_ಅಂತ್ಯ				/* } */

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
#ಆದರೆ !defined(LUA_USE_C89)
#ವ್ಯಾಖ್ಯಾನಿಸು l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_sprintf(s,sz,f,i)	((ಶೂನ್ಯ)(sz), sprintf(s,f,i))
#ಆದರೆ_ಅಂತ್ಯ


/*
@@ lua_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#ಆದರೆ !defined(LUA_USE_C89)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_strx2number(s,p)		lua_str2number(s,p)
#ಆದರೆ_ಅಂತ್ಯ


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#ಆದರೆ !defined(LUA_USE_C89)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_number2strx(L,b,sz,f,n)  \
	((ಶೂನ್ಯ)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#ಆದರೆ_ಅಂತ್ಯ


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#ಆದರೆ defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು l_mathop  /* variants not available */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು lua_str2number
#ವ್ಯಾಖ್ಯಾನಿಸು l_mathop(op)		(lua_ಸಂಖ್ಯೆ)op  /* no variant */
#ವ್ಯಾಖ್ಯಾನಿಸು lua_str2number(s,p)	((lua_ಸಂಖ್ಯೆ)strtod((s), (p)))
#ಆದರೆ_ಅಂತ್ಯ


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_KCONTEXT	ptrdiff_t

#ಆದರೆ !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಆದರೆ defined(INTPTR_MAX)  /* even in C99 this type is optional */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LUA_KCONTEXT
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_KCONTEXT	intptr_t
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#ಆದರೆ !defined(lua_getlocaledecpoint)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_getlocaledecpoint()		(localeconv()->decimal_point[0])
#ಆದರೆ_ಅಂತ್ಯ


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#ಆದರೆ !defined(luai_likely)

#ಆದರೆ !defined(LUA_NOBUILTIN) && defined(__GNUC__) && (__GNUC__ >= 3)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು luai_likely(x)		(x)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_unlikely(x)	(x)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ



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
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_EXTRASPACE		(ಗಾತ್ರ(ಶೂನ್ಯ *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUAL_BUFFERSIZE   ((ಪೂರ್ಣಾಂಕ)(16 * ಗಾತ್ರ(ಶೂನ್ಯ*) * ಗಾತ್ರ(lua_ಸಂಖ್ಯೆ)))


/*
@@ LUAI_MAXALIGN defines fields that ensure proper alignment for
** memory areas offered by Lua (e.g., userdata memory).
** Add fields to it if you need alignment for non-ISO objects.
*/
#ಆದರೆ defined(LLONG_MAX)
/* use ISO C99 stuff */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXALIGN ದೀರ್ಘ ದ್ವಿಗುಣ u; ಶೂನ್ಯ *s; ದೀರ್ಘ ದೀರ್ಘ l
#ಇಲ್ಲದಿದ್ದರೆ
/* use only C89 stuff */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXALIGN  lua_ಸಂಖ್ಯೆ n; ದ್ವಿಗುಣ u; ಶೂನ್ಯ *s; lua_Integer i; ದೀರ್ಘ l
#ಆದರೆ_ಅಂತ್ಯ

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/



#ಆದರೆ_ಅಂತ್ಯ

