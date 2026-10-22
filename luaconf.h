/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#अगर_अघोषित luaconf_h
#घोषणा luaconf_h

#समावेश <limits.h>
#समावेश <stddef.h>


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
#अगर !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#घोषणा LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#पूर्ण_अगर


#अगर defined(LUA_USE_WINDOWS)
#घोषणा LUA_DL_DLL	/* enable support for DLL */
#घोषणा LUA_USE_C89	/* broadly, Windows is C89 */
#पूर्ण_अगर


/*
** When POSIX DLL ('LUA_USE_DLOPEN') is enabled, the Lua stand-alone
** application will try to dynamically link a 'readline' facility
** for its REPL.  In that case, LUA_READLINELIB is the name of the
** library it will look for those facilities.  If lua.c cannot open
** the specified library, it will generate a warning and then run
** without 'readline'.  If that macro is not defined, lua.c will not
** use 'readline'.
*/
#अगर defined(LUA_USE_LINUX)
#घोषणा LUA_USE_POSIX
#घोषणा LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#अगर !defined(LUA_READLINELIB)
#घोषणा LUA_READLINELIB		"libreadline.so"
#पूर्ण_अगर
#पूर्ण_अगर


#अगर defined(LUA_USE_MACOSX)
#घोषणा LUA_USE_POSIX
#घोषणा LUA_USE_DLOPEN		/* macOS does not need -ldl */
#घोषणा LUA_USE_READLINE	/* needs an extra library: -lreadline */
#पूर्ण_अगर


#अगर defined(LUA_USE_IOS)
#घोषणा LUA_USE_POSIX
#घोषणा LUA_USE_DLOPEN
#पूर्ण_अगर


#अगर defined(LUA_USE_C89) && defined(LUA_USE_POSIX)
#त्रुटि "POSIX is not compatible with C89"
#पूर्ण_अगर


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#घोषणा LUAI_IS32INT	((अच_पूर्णांक_उच्च >> 30) >= 3)

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
#घोषणा LUA_INT_INT		1
#घोषणा LUA_INT_LONG		2
#घोषणा LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#घोषणा LUA_FLOAT_FLOAT		1
#घोषणा LUA_FLOAT_DOUBLE	2
#घोषणा LUA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#घोषणा LUA_INT_DEFAULT		LUA_INT_LONGLONG
#घोषणा LUA_FLOAT_DEFAULT	LUA_FLOAT_DOUBLE


/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
/* #define LUA_32BITS */


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#अगर defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#घोषणा LUA_C89_NUMBERS		1
#अन्यथा
#घोषणा LUA_C89_NUMBERS		0
#पूर्ण_अगर


#अगर defined(LUA_32BITS)	/* { */
/*
** 32-bit integers and 'float'
*/
#अगर LUAI_IS32INT  /* use 'int' if big enough */
#घोषणा LUA_INT_TYPE	LUA_INT_INT
#अन्यथा  /* otherwise use 'long' */
#घोषणा LUA_INT_TYPE	LUA_INT_LONG
#पूर्ण_अगर
#घोषणा LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#या_अगर LUA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#घोषणा LUA_INT_TYPE	LUA_INT_LONG
#घोषणा LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#अन्यथा		/* }{ */
/* use defaults */

#घोषणा LUA_INT_TYPE	LUA_INT_DEFAULT
#घोषणा LUA_FLOAT_TYPE	LUA_FLOAT_DEFAULT

#पूर्ण_अगर				/* } */


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
#घोषणा LUA_PATH_SEP            ";"
#घोषणा LUA_PATH_MARK           "?"
#घोषणा LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#घोषणा LUA_VDIR	LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#अगर defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#घोषणा LUA_LDIR	"!\\lua\\"
#घोषणा LUA_CDIR	"!\\"
#घोषणा LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"

#अगर !defined(LUA_PATH_DEFAULT)
#घोषणा LUA_PATH_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
		LUA_SHRDIR "?.lua;"  LUA_SHRDIR "?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#पूर्ण_अगर

#अगर !defined(LUA_CPATH_DEFAULT)
#घोषणा LUA_CPATH_DEFAULT \
		LUA_CDIR "?.dll;" \
		LUA_CDIR "..\\lib\\lua\\"  LUA_VDIR "\\?.dll;" \
		LUA_CDIR "loadall.dll;" ".\\?.dll"
#पूर्ण_अगर

#अन्यथा			/* }{ */

#घोषणा LUA_मूल	"/usr/local/"
#घोषणा LUA_LDIR	LUA_मूल "share/lua/" LUA_VDIR "/"
#घोषणा LUA_CDIR	LUA_मूल "lib/lua/" LUA_VDIR "/"

#अगर !defined(LUA_PATH_DEFAULT)
#घोषणा LUA_PATH_DEFAULT  \
		LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
		LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#पूर्ण_अगर

#अगर !defined(LUA_CPATH_DEFAULT)
#घोषणा LUA_CPATH_DEFAULT \
		LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#पूर्ण_अगर

#पूर्ण_अगर			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#अगर !defined(LUA_DIRSEP)

#अगर defined(_WIN32)
#घोषणा LUA_DIRSEP	"\\"
#अन्यथा
#घोषणा LUA_DIRSEP	"/"
#पूर्ण_अगर

#पूर्ण_अगर


/*
** LUA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the suffix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#घोषणा LUA_IGMARK		"-"

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
#अगर defined(LUA_BUILD_AS_DLL)	/* { */

#अगर defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#घोषणा LUA_API __declspec(dllexport)
#अन्यथा						/* }{ */
#घोषणा LUA_API __declspec(dllimport)
#पूर्ण_अगर						/* } */

#अन्यथा				/* }{ */

#घोषणा LUA_API		बाह्य

#पूर्ण_अगर				/* } */


/*
** More often than not the libs go together with the core.
*/
#घोषणा LUALIB_API	LUA_API

#अगर defined(__cplusplus)
/* Lua uses the "C name" when calling open functions */
#घोषणा LUAMOD_API	बाह्य "C"
#अन्यथा
#घोषणा LUAMOD_API	LUA_API
#पूर्ण_अगर

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_GLOBAL avoids 'global' being a reserved word
*/
#अगर !defined(LUA_COMPAT_GLOBAL)
#घोषणा LUA_COMPAT_GLOBAL	1
#पूर्ण_अगर


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
#घोषणा lua_strlen(L,i)		lua_rawlen(L, (i))

#घोषणा lua_objlen(L,i)		lua_rawlen(L, (i))

#घोषणा lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#घोषणा lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

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

#घोषणा l_floor(x)		(l_mathop(न्यूनमान)(x))


/* now the variable definitions */

#अगर LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#घोषणा LUA_संख्या	भग्न

#घोषणा l_floatatt(n)		(FLT_##n)

#घोषणा LUAI_UACNUMBER	द्विगुन

#घोषणा LUA_संख्या_FRMLEN	""
#घोषणा LUA_संख्या_FMT		"%.7g"
#घोषणा LUA_संख्या_FMT_N	"%.9g"

#घोषणा l_mathop(op)		op##f

#घोषणा lua_str2number(s,p)	strtof((s), (p))


#या_अगर LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#घोषणा LUA_संख्या	दीर्घ द्विगुन

#घोषणा l_floatatt(n)		(LDBL_##n)

#घोषणा LUAI_UACNUMBER	दीर्घ द्विगुन

#घोषणा LUA_संख्या_FRMLEN	"L"
#घोषणा LUA_संख्या_FMT		"%.19Lg"
#घोषणा LUA_संख्या_FMT_N	"%.21Lg"

#घोषणा l_mathop(op)		op##l

#घोषणा lua_str2number(s,p)	strtold((s), (p))

#या_अगर LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#घोषणा LUA_संख्या	द्विगुन

#घोषणा l_floatatt(n)		(DBL_##n)

#घोषणा LUAI_UACNUMBER	द्विगुन

#घोषणा LUA_संख्या_FRMLEN	""
#घोषणा LUA_संख्या_FMT		"%.15g"
#घोषणा LUA_संख्या_FMT_N	"%.17g"

#घोषणा l_mathop(op)		op

#घोषणा lua_str2number(s,p)	म_से_भग्न((s), (p))

#अन्यथा						/* }{ */

#त्रुटि "numeric float type not defined"

#पूर्ण_अगर					/* } */



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

#घोषणा LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#घोषणा LUAI_UACINT		LUA_INTEGER

#घोषणा lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#घोषणा LUA_UNSIGNED		अचिन्हित LUAI_UACINT


/* now the variable definitions */

#अगर LUA_INT_TYPE == LUA_INT_INT		/* { int */

#घोषणा LUA_INTEGER		पूर्णांक
#घोषणा LUA_INTEGER_FRMLEN	""

#घोषणा LUA_MAXINTEGER		पूर्णांक_उच्च
#घोषणा LUA_MININTEGER		पूर्णांक_न्यून

#घोषणा LUA_MAXUNSIGNED		अच_पूर्णांक_उच्च

#या_अगर LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#घोषणा LUA_INTEGER		दीर्घ
#घोषणा LUA_INTEGER_FRMLEN	"l"

#घोषणा LUA_MAXINTEGER		दीर्घ_उच्च
#घोषणा LUA_MININTEGER		दीर्घ_न्यून

#घोषणा LUA_MAXUNSIGNED		अच_दीर्घ_उच्च

#या_अगर LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#अगर defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#घोषणा LUA_INTEGER		दीर्घ दीर्घ
#घोषणा LUA_INTEGER_FRMLEN	"ll"

#घोषणा LUA_MAXINTEGER		LLONG_MAX
#घोषणा LUA_MININTEGER		LLONG_MIN

#घोषणा LUA_MAXUNSIGNED		ULLONG_MAX

#या_अगर defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#घोषणा LUA_INTEGER		__int64
#घोषणा LUA_INTEGER_FRMLEN	"I64"

#घोषणा LUA_MAXINTEGER		_I64_MAX
#घोषणा LUA_MININTEGER		_I64_MIN

#घोषणा LUA_MAXUNSIGNED		_UI64_MAX

#अन्यथा				/* }{ */

#त्रुटि "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' क्रम details)"

#पूर्ण_अगर				/* } */

#अन्यथा				/* }{ */

#त्रुटि "numeric integer type not defined"

#पूर्ण_अगर				/* } */

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
#अगर !defined(LUA_USE_C89)
#घोषणा l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#अन्यथा
#घोषणा l_sprintf(s,sz,f,i)	((व्योम)(sz), प्र_लिखो(s,f,i))
#पूर्ण_अगर


/*
@@ lua_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#अगर !defined(LUA_USE_C89)
#घोषणा lua_strx2number(s,p)		lua_str2number(s,p)
#पूर्ण_अगर


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#घोषणा lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#अगर !defined(LUA_USE_C89)
#घोषणा lua_number2strx(L,b,sz,f,n)  \
	((व्योम)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#पूर्ण_अगर


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#अगर defined(LUA_USE_C89) || (defined(विशाल_मान) && !defined(HUGE_VALF))
#अघोषित l_mathop  /* variants not available */
#अघोषित lua_str2number
#घोषणा l_mathop(op)		(lua_संख्या)op  /* no variant */
#घोषणा lua_str2number(s,p)	((lua_संख्या)म_से_भग्न((s), (p)))
#पूर्ण_अगर


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#घोषणा LUA_KCONTEXT	सूचक_भेद_प्रकार

#अगर !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#समावेश <stdint.h>
#अगर defined(INTPTR_MAX)  /* even in C99 this type is optional */
#अघोषित LUA_KCONTEXT
#घोषणा LUA_KCONTEXT	intptr_t
#पूर्ण_अगर
#पूर्ण_अगर


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#अगर !defined(lua_getlocaledecpoint)
#घोषणा lua_getlocaledecpoint()		(क्षेत्र_बदलो()->decimal_point[0])
#पूर्ण_अगर


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#अगर !defined(luai_likely)

#अगर !defined(LUA_NOBUILTIN) && defined(__GNUC__) && (__GNUC__ >= 3)
#घोषणा luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#घोषणा luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#अन्यथा
#घोषणा luai_likely(x)		(x)
#घोषणा luai_unlikely(x)	(x)
#पूर्ण_अगर

#पूर्ण_अगर



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
#घोषणा LUA_EXTRASPACE		(माप(व्योम *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#घोषणा LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#घोषणा LUAL_BUFFERSIZE   ((पूर्णांक)(16 * माप(व्योम*) * माप(lua_संख्या)))


/*
@@ LUAI_MAXALIGN defines fields that ensure proper alignment for
** memory areas offered by Lua (e.g., userdata memory).
** Add fields to it if you need alignment for non-ISO objects.
*/
#अगर defined(LLONG_MAX)
/* use ISO C99 stuff */
#घोषणा LUAI_MAXALIGN दीर्घ द्विगुन u; व्योम *s; दीर्घ दीर्घ l
#अन्यथा
/* use only C89 stuff */
#घोषणा LUAI_MAXALIGN  lua_संख्या n; द्विगुन u; व्योम *s; lua_Integer i; दीर्घ l
#पूर्ण_अगर

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/



#पूर्ण_अगर

