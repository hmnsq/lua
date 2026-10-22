/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#ប្រសិនបើ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ប្រសិនបើ !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
static ចំនួនគត់ l_checkmode (const var_na *mode) {
  ត្រឡប់ (*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&
         (*mode != '+' || ((void)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ប្រសិនបើ !defined(l_popen)		/* { */

#ប្រសិនបើ defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(NULL), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#ប្រសិនបើ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#បើមិនដូច្នេះ				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((void)c, (void)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((void)L, (void)file, -1)

#endif				/* } */

#endif				/* } */


#ប្រសិនបើ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#ប្រសិនបើ !defined(l_getc)		/* { */

#ប្រសិនបើ defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#បើមិនដូច្នេះ
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((void)0)
#define l_unlockfile(f)		((void)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ប្រសិនបើ !defined(l_fseek)		/* { */

#ប្រសិនបើ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#include <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#elif defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#បើមិនដូច្នេះ				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(parimaa_na(IO_PREFIX)/parimaa_na(var_na) - 1)
#define IO_INPUT	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


typedef luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == NULL)


static ចំនួនគត់ io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ប្រសិនបើ (p == NULL)
    luaL_pushfail(L);  /* not a file */
  បើមិនដូច្នេះ ប្រសិនបើ (isclosed(p))
    lua_pushliteral(L, "closed file");
  បើមិនដូច្នេះ
    lua_pushliteral(L, "file");
  ត្រឡប់ 1;
}


static ចំនួនគត់ f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ប្រសិនបើ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  បើមិនដូច្នេះ
    lua_pushfstring(L, "file (%p)", p->f);
  ត្រឡប់ 1;
}


static FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ប្រសិនបើ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ត្រឡប់ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
static LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, parimaa_na(LStream), 0);
  p->closef = NULL;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ត្រឡប់ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
static ចំនួនគត់ aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ca_ncala lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  ត្រឡប់ (*cf)(L);  /* close it */
}


static ចំនួនគត់ f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ត្រឡប់ aux_close(L);
}


static ចំនួនគត់ io_close (lua_State *L) {
  ប្រសិនបើ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ត្រឡប់ f_close(L);
}


static ចំនួនគត់ f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ប្រសិនបើ (!isclosed(p) && p->f != NULL)
    aux_close(L);  /* ignore closed and incompletely open files */
  ត្រឡប់ 0;
}


/*
** function to close regular files
*/
static ចំនួនគត់ io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ត្រឡប់ luaL_fileresult(L, (fclose(p->f) == 0), NULL);
}


static LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = NULL;
  p->closef = &io_fclose;
  ត្រឡប់ p;
}


static void opencheck (lua_State *L, const var_na *fname, const var_na *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  ប្រសិនបើ (l_unlikely(p->f == NULL))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


static ចំនួនគត់ io_open (lua_State *L) {
  const var_na *filename = luaL_checkstring(L, 1);
  const var_na *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  const var_na *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  ត្រឡប់ (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
static ចំនួនគត់ io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ត្រឡប់ luaL_execresult(L, l_pclose(L, p->f));
}


static ចំនួនគត់ io_popen (lua_State *L) {
  const var_na *filename = luaL_checkstring(L, 1);
  const var_na *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  ត្រឡប់ (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


static ចំនួនគត់ io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ត្រឡប់ (p->f == NULL) ? luaL_fileresult(L, 0, NULL) : 1;
}


static FILE *getiofile (lua_State *L, const var_na *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ប្រសិនបើ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ត្រឡប់ p->f;
}


static ចំនួនគត់ g_iofile (lua_State *L, const var_na *f, const var_na *mode) {
  ប្រសិនបើ (!lua_isnoneornil(L, 1)) {
    const var_na *filename = lua_tostring(L, 1);
    ប្រសិនបើ (filename)
      opencheck(L, filename, mode);
    បើមិនដូច្នេះ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ត្រឡប់ 1;
}


static ចំនួនគត់ io_input (lua_State *L) {
  ត្រឡប់ g_iofile(L, IO_INPUT, "r");
}


static ចំនួនគត់ io_output (lua_State *L) {
  ត្រឡប់ g_iofile(L, IO_OUTPUT, "w");
}


static ចំនួនគត់ io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#define MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
static void aux_lines (lua_State *L, ចំនួនគត់ toclose) {
  ចំនួនគត់ n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


static ចំនួនគត់ f_lines (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_lines(L, 0);
  ត្រឡប់ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
static ចំនួនគត់ io_lines (lua_State *L) {
  ចំនួនគត់ toclose;
  ប្រសិនបើ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ប្រសិនបើ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_INPUT);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  បើមិនដូច្នេះ {  /* open a new file */
    const var_na *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_lines(L, toclose);  /* push iteration function */
  ប្រសិនបើ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ត្រឡប់ 4;
  }
  បើមិនដូច្នេះ
    ត្រឡប់ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ប្រសិនបើ !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
typedef sa_nracanaa {
  FILE *f;  /* file being read */
  ចំនួនគត់ c;  /* current character (look ahead) */
  ចំនួនគត់ n;  /* number of elements in buffer 'buff' */
  var_na buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
static ចំនួនគត់ nextc (RN *rn) {
  ប្រសិនបើ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ត្រឡប់ 0;  /* fail */
  }
  បើមិនដូច្នេះ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ត្រឡប់ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
static ចំនួនគត់ test2 (RN *rn, const var_na *set) {
  ប្រសិនបើ (rn->c == set[0] || rn->c == set[1])
    ត្រឡប់ nextc(rn);
  បើមិនដូច្នេះ ត្រឡប់ 0;
}


/*
** Read a sequence of (hex)digits
*/
static ចំនួនគត់ readdigits (RN *rn, ចំនួនគត់ hex) {
  ចំនួនគត់ count = 0;
  ខណៈពេល ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    count++;
  ត្រឡប់ count;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
static ចំនួនគត់ read_number (lua_State *L, FILE *f) {
  RN rn;
  ចំនួនគត់ count = 0;
  ចំនួនគត់ hex = 0;
  var_na decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  do { rn.c = l_getc(rn.f); } ខណៈពេល (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ប្រសិនបើ (test2(&rn, "00")) {
    ប្រសិនបើ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    បើមិនដូច្នេះ count = 1;  /* count initial '0' as a valid digit */
  }
  count += readdigits(&rn, hex);  /* integral part */
  ប្រសិនបើ (test2(&rn, decp))  /* decimal point? */
    count += readdigits(&rn, hex);  /* fractional part */
  ប្រសិនបើ (count > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ប្រសិនបើ (l_likely(lua_stringtonumber(L, rn.buff)))
    ត្រឡប់ 1;  /* ok, it is a valid number */
  បើមិនដូច្នេះ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ត្រឡប់ 0;  /* read fails */
  }
}


static ចំនួនគត់ test_eof (lua_State *L, FILE *f) {
  ចំនួនគត់ c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ត្រឡប់ (c != EOF);
}


static ចំនួនគត់ read_line (lua_State *L, FILE *f, ចំនួនគត់ chop) {
  luaL_Buffer b;
  ចំនួនគត់ c;
  luaL_buffinit(L, &b);
  do {  /* may need to read several chunks to get whole line */
    var_na *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ខណៈពេល (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ខណៈពេល (c != EOF && c != '\n');  /* repeat until end of line */
  ប្រសិនបើ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ត្រឡប់ (c == '\n' || lua_rawlen(L, -1) > 0);
}


static void read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    var_na *p = luaL_prepbuffer(&b);
    nr = fread(p, parimaa_na(var_na), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ខណៈពេល (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


static ចំនួនគត់ read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  var_na *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, parimaa_na(var_na), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ត្រឡប់ (nr > 0);  /* true iff read something */
}


static ចំនួនគត់ g_read (lua_State *L, FILE *f, ចំនួនគត់ first) {
  ចំនួនគត់ nargs = lua_gettop(L) - 1;
  ចំនួនគត់ n, success;
  clearerr(f);
  errno = 0;
  ប្រសិនបើ (nargs == 0) {  /* no arguments? */
    success = read_line(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  បើមិនដូច្នេះ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    សម្រាប់ (n = first; nargs-- && success; n++) {
      ប្រសិនបើ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      បើមិនដូច្នេះ {
        const var_na *p = luaL_checkstring(L, n);
        ប្រសិនបើ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        switch (*p) {
          pak_sa 'n':  /* number */
            success = read_number(L, f);
            bha_nga;
          pak_sa 'l':  /* line */
            success = read_line(L, f, 1);
            bha_nga;
          pak_sa 'L':  /* line with end-of-line */
            success = read_line(L, f, 0);
            bha_nga;
          pak_sa 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            bha_nga;
          default:
            ត្រឡប់ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ប្រសិនបើ (ferror(f))
    ត្រឡប់ luaL_fileresult(L, 0, NULL);
  ប្រសិនបើ (!success) {
    lua_pop(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ត្រឡប់ n - first;
}


static ចំនួនគត់ io_read (lua_State *L) {
  ត្រឡប់ g_read(L, getiofile(L, IO_INPUT), 1);
}


static ចំនួនគត់ f_read (lua_State *L) {
  ត្រឡប់ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
static ចំនួនគត់ io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  ចំនួនគត់ i;
  ចំនួនគត់ n = (ចំនួនគត់)lua_tointeger(L, lua_upvalueindex(2));
  ប្រសិនបើ (isclosed(p))  /* file is already closed? */
    ត្រឡប់ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  សម្រាប់ (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ប្រសិនបើ (lua_toboolean(L, -n))  /* read at least one value? */
    ត្រឡប់ n;  /* return them */
  បើមិនដូច្នេះ {  /* first result is false: EOF or error */
    ប្រសិនបើ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ត្រឡប់ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ប្រសិនបើ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ត្រឡប់ 0;
  }
}

/* }====================================================== */


static ចំនួនគត់ g_write (lua_State *L, FILE *f, ចំនួនគត់ arg) {
  ចំនួនគត់ nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  សម្រាប់ (; nargs--; arg++) {  /* for each argument */
    var_na buff[LUA_N2SBUFFSZ];
    const var_na *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ប្រសិនបើ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    បើមិនដូច្នេះ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, parimaa_na(var_na), len, f);
    totalbytes += numbytes;
    ប្រសិនបើ (numbytes < len) {  /* write error? */
      ចំនួនគត់ n = luaL_fileresult(L, 0, NULL);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ត្រឡប់ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ត្រឡប់ 1;  /* no errors; file handle already on stack top */
}


static ចំនួនគត់ io_write (lua_State *L) {
  ត្រឡប់ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


static ចំនួនគត់ f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ត្រឡប់ g_write(L, f, 2);
}


static ចំនួនគត់ f_seek (lua_State *L) {
  static const ចំនួនគត់ mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const var_na *const modenames[] = {"set", "cur", "end", NULL};
  FILE *f = tofile(L);
  ចំនួនគត់ op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  ប្រសិនបើ (l_unlikely(op))
    ត្រឡប់ luaL_fileresult(L, 0, NULL);  /* error */
  បើមិនដូច្នេះ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ត្រឡប់ 1;
  }
}


static ចំនួនគត់ f_setvbuf (lua_State *L) {
  static const ចំនួនគត់ mode[] = {_IONBF, _IOFBF, _IOLBF};
  static const var_na *const modenames[] = {"no", "full", "line", NULL};
  FILE *f = tofile(L);
  ចំនួនគត់ op = luaL_checkoption(L, 2, NULL, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  ចំនួនគត់ res;
  errno = 0;
  res = setvbuf(f, NULL, mode[op], (size_t)sz);
  ត្រឡប់ luaL_fileresult(L, res == 0, NULL);
}


static ចំនួនគត់ aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ត្រឡប់ luaL_fileresult(L, fflush(f) == 0, NULL);
}


static ចំនួនគត់ f_flush (lua_State *L) {
  ត្រឡប់ aux_flush(L, tofile(L));
}


static ចំនួនគត់ io_flush (lua_State *L) {
  ត្រឡប់ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
static const luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_input},
  {"lines", io_lines},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {NULL, NULL}
};


/*
** methods for file handles
*/
static const luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_lines},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {NULL, NULL}
};


/*
** metamethods for file handles
*/
static const luaL_Reg metameth[] = {
  {"__index", NULL},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {NULL, NULL}
};


static void createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_pop(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static ចំនួនគត់ io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ត្រឡប់ 2;
}


static void createstdfile (lua_State *L, FILE *f, const var_na *k,
                           const var_na *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ប្រសិនបើ (k != NULL) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API ចំនួនគត់ luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_INPUT, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, NULL, "stderr");
  ត្រឡប់ 1;
}

