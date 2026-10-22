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
#ပေႃး !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ပေႃး !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
static puur_naa_nka l_checkmode (const var_na *mode) {
  ပွၵ်ႈ (*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&
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

#ပေႃး !defined(l_popen)		/* { */

#ပေႃး defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(NULL), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#ပေႃး !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#ပေႃးဢမ်ႇ				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((void)c, (void)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((void)L, (void)file, -1)

#endif				/* } */

#endif				/* } */


#ပေႃး !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#ပေႃး !defined(l_getc)		/* { */

#ပေႃး defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#ပေႃးဢမ်ႇ
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

#ပေႃး !defined(l_fseek)		/* { */

#ပေႃး defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

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

#ပေႃးဢမ်ႇ				/* }{ */

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


static puur_naa_nka io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ပေႃး (p == NULL)
    luaL_pushfail(L);  /* not a file */
  ပေႃးဢမ်ႇ ပေႃး (isclosed(p))
    lua_pushliteral(L, "closed file");
  ပေႃးဢမ်ႇ
    lua_pushliteral(L, "file");
  ပွၵ်ႈ 1;
}


static puur_naa_nka f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ပေႃး (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ပေႃးဢမ်ႇ
    lua_pushfstring(L, "file (%p)", p->f);
  ပွၵ်ႈ 1;
}


static FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ပေႃး (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ပွၵ်ႈ p->f;
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
  ပွၵ်ႈ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
static puur_naa_nka aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ca_ncala lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  ပွၵ်ႈ (*cf)(L);  /* close it */
}


static puur_naa_nka f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ပွၵ်ႈ aux_close(L);
}


static puur_naa_nka io_close (lua_State *L) {
  ပေႃး (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ပွၵ်ႈ f_close(L);
}


static puur_naa_nka f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ပေႃး (!isclosed(p) && p->f != NULL)
    aux_close(L);  /* ignore closed and incompletely open files */
  ပွၵ်ႈ 0;
}


/*
** function to close regular files
*/
static puur_naa_nka io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ပွၵ်ႈ luaL_fileresult(L, (fclose(p->f) == 0), NULL);
}


static LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = NULL;
  p->closef = &io_fclose;
  ပွၵ်ႈ p;
}


static void opencheck (lua_State *L, const var_na *fname, const var_na *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  ပေႃး (l_unlikely(p->f == NULL))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


static puur_naa_nka io_open (lua_State *L) {
  const var_na *filename = luaL_checkstring(L, 1);
  const var_na *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  const var_na *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  ပွၵ်ႈ (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
static puur_naa_nka io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ပွၵ်ႈ luaL_execresult(L, l_pclose(L, p->f));
}


static puur_naa_nka io_popen (lua_State *L) {
  const var_na *filename = luaL_checkstring(L, 1);
  const var_na *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  ပွၵ်ႈ (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


static puur_naa_nka io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ပွၵ်ႈ (p->f == NULL) ? luaL_fileresult(L, 0, NULL) : 1;
}


static FILE *getiofile (lua_State *L, const var_na *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ပေႃး (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ပွၵ်ႈ p->f;
}


static puur_naa_nka g_iofile (lua_State *L, const var_na *f, const var_na *mode) {
  ပေႃး (!lua_isnoneornil(L, 1)) {
    const var_na *filename = lua_tostring(L, 1);
    ပေႃး (filename)
      opencheck(L, filename, mode);
    ပေႃးဢမ်ႇ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ပွၵ်ႈ 1;
}


static puur_naa_nka io_input (lua_State *L) {
  ပွၵ်ႈ g_iofile(L, IO_INPUT, "r");
}


static puur_naa_nka io_output (lua_State *L) {
  ပွၵ်ႈ g_iofile(L, IO_OUTPUT, "w");
}


static puur_naa_nka io_readline (lua_State *L);


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
static void aux_lines (lua_State *L, puur_naa_nka toclose) {
  puur_naa_nka n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


static puur_naa_nka f_lines (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_lines(L, 0);
  ပွၵ်ႈ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
static puur_naa_nka io_lines (lua_State *L) {
  puur_naa_nka toclose;
  ပေႃး (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ပေႃး (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_INPUT);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ပေႃးဢမ်ႇ {  /* open a new file */
    const var_na *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_lines(L, toclose);  /* push iteration function */
  ပေႃး (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ပွၵ်ႈ 4;
  }
  ပေႃးဢမ်ႇ
    ပွၵ်ႈ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ပေႃး !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
typedef sa_nracanaa {
  FILE *f;  /* file being read */
  puur_naa_nka c;  /* current character (look ahead) */
  puur_naa_nka n;  /* number of elements in buffer 'buff' */
  var_na buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
static puur_naa_nka nextc (RN *rn) {
  ပေႃး (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ပွၵ်ႈ 0;  /* fail */
  }
  ပေႃးဢမ်ႇ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ပွၵ်ႈ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
static puur_naa_nka test2 (RN *rn, const var_na *set) {
  ပေႃး (rn->c == set[0] || rn->c == set[1])
    ပွၵ်ႈ nextc(rn);
  ပေႃးဢမ်ႇ ပွၵ်ႈ 0;
}


/*
** Read a sequence of (hex)digits
*/
static puur_naa_nka readdigits (RN *rn, puur_naa_nka hex) {
  puur_naa_nka count = 0;
  ၶၢဝ်း ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    count++;
  ပွၵ်ႈ count;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
static puur_naa_nka read_number (lua_State *L, FILE *f) {
  RN rn;
  puur_naa_nka count = 0;
  puur_naa_nka hex = 0;
  var_na decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  do { rn.c = l_getc(rn.f); } ၶၢဝ်း (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ပေႃး (test2(&rn, "00")) {
    ပေႃး (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ပေႃးဢမ်ႇ count = 1;  /* count initial '0' as a valid digit */
  }
  count += readdigits(&rn, hex);  /* integral part */
  ပေႃး (test2(&rn, decp))  /* decimal point? */
    count += readdigits(&rn, hex);  /* fractional part */
  ပေႃး (count > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ပေႃး (l_likely(lua_stringtonumber(L, rn.buff)))
    ပွၵ်ႈ 1;  /* ok, it is a valid number */
  ပေႃးဢမ်ႇ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ပွၵ်ႈ 0;  /* read fails */
  }
}


static puur_naa_nka test_eof (lua_State *L, FILE *f) {
  puur_naa_nka c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ပွၵ်ႈ (c != EOF);
}


static puur_naa_nka read_line (lua_State *L, FILE *f, puur_naa_nka chop) {
  luaL_Buffer b;
  puur_naa_nka c;
  luaL_buffinit(L, &b);
  do {  /* may need to read several chunks to get whole line */
    var_na *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ၶၢဝ်း (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ၶၢဝ်း (c != EOF && c != '\n');  /* repeat until end of line */
  ပေႃး (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ပွၵ်ႈ (c == '\n' || lua_rawlen(L, -1) > 0);
}


static void read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    var_na *p = luaL_prepbuffer(&b);
    nr = fread(p, parimaa_na(var_na), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ၶၢဝ်း (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


static puur_naa_nka read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  var_na *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, parimaa_na(var_na), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ပွၵ်ႈ (nr > 0);  /* true iff read something */
}


static puur_naa_nka g_read (lua_State *L, FILE *f, puur_naa_nka first) {
  puur_naa_nka nargs = lua_gettop(L) - 1;
  puur_naa_nka n, success;
  clearerr(f);
  errno = 0;
  ပေႃး (nargs == 0) {  /* no arguments? */
    success = read_line(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  ပေႃးဢမ်ႇ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      ပေႃး (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ပေႃးဢမ်ႇ {
        const var_na *p = luaL_checkstring(L, n);
        ပေႃး (*p == '*') p++;  /* skip optional '*' (for compatibility) */
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
            ပွၵ်ႈ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ပေႃး (ferror(f))
    ပွၵ်ႈ luaL_fileresult(L, 0, NULL);
  ပေႃး (!success) {
    lua_pop(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ပွၵ်ႈ n - first;
}


static puur_naa_nka io_read (lua_State *L) {
  ပွၵ်ႈ g_read(L, getiofile(L, IO_INPUT), 1);
}


static puur_naa_nka f_read (lua_State *L) {
  ပွၵ်ႈ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
static puur_naa_nka io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  puur_naa_nka i;
  puur_naa_nka n = (puur_naa_nka)lua_tointeger(L, lua_upvalueindex(2));
  ပေႃး (isclosed(p))  /* file is already closed? */
    ပွၵ်ႈ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  for (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ပေႃး (lua_toboolean(L, -n))  /* read at least one value? */
    ပွၵ်ႈ n;  /* return them */
  ပေႃးဢမ်ႇ {  /* first result is false: EOF or error */
    ပေႃး (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ပွၵ်ႈ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ပေႃး (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ပွၵ်ႈ 0;
  }
}

/* }====================================================== */


static puur_naa_nka g_write (lua_State *L, FILE *f, puur_naa_nka arg) {
  puur_naa_nka nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  for (; nargs--; arg++) {  /* for each argument */
    var_na buff[LUA_N2SBUFFSZ];
    const var_na *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ပေႃး (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ပေႃးဢမ်ႇ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, parimaa_na(var_na), len, f);
    totalbytes += numbytes;
    ပေႃး (numbytes < len) {  /* write error? */
      puur_naa_nka n = luaL_fileresult(L, 0, NULL);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ပွၵ်ႈ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ပွၵ်ႈ 1;  /* no errors; file handle already on stack top */
}


static puur_naa_nka io_write (lua_State *L) {
  ပွၵ်ႈ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


static puur_naa_nka f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ပွၵ်ႈ g_write(L, f, 2);
}


static puur_naa_nka f_seek (lua_State *L) {
  static const puur_naa_nka mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const var_na *const modenames[] = {"set", "cur", "end", NULL};
  FILE *f = tofile(L);
  puur_naa_nka op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  ပေႃး (l_unlikely(op))
    ပွၵ်ႈ luaL_fileresult(L, 0, NULL);  /* error */
  ပေႃးဢမ်ႇ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ပွၵ်ႈ 1;
  }
}


static puur_naa_nka f_setvbuf (lua_State *L) {
  static const puur_naa_nka mode[] = {_IONBF, _IOFBF, _IOLBF};
  static const var_na *const modenames[] = {"no", "full", "line", NULL};
  FILE *f = tofile(L);
  puur_naa_nka op = luaL_checkoption(L, 2, NULL, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  puur_naa_nka res;
  errno = 0;
  res = setvbuf(f, NULL, mode[op], (size_t)sz);
  ပွၵ်ႈ luaL_fileresult(L, res == 0, NULL);
}


static puur_naa_nka aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ပွၵ်ႈ luaL_fileresult(L, fflush(f) == 0, NULL);
}


static puur_naa_nka f_flush (lua_State *L) {
  ပွၵ်ႈ aux_flush(L, tofile(L));
}


static puur_naa_nka io_flush (lua_State *L) {
  ပွၵ်ႈ aux_flush(L, getiofile(L, IO_OUTPUT));
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
static puur_naa_nka io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ပွၵ်ႈ 2;
}


static void createstdfile (lua_State *L, FILE *f, const var_na *k,
                           const var_na *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ပေႃး (k != NULL) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API puur_naa_nka luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_INPUT, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, NULL, "stderr");
  ပွၵ်ႈ 1;
}

