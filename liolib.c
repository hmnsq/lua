/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#എങ്കിൽ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#എങ്കിൽ !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
സ്ഥിതം പൂർണ്ണസംഖ്യ l_checkmode (സ്ഥിരം അക്ഷരം *mode) {
  തിരികെ (*mode != '\0' && strchr("rwa", *(mode++)) != ശൂന്യ &&
         (*mode != '+' || ((ശൂന്യം)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#എങ്കിൽ !defined(l_popen)		/* { */

#എങ്കിൽ defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(ശൂന്യ), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#അല്ലെങ്കിലെങ്കിൽ defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#എങ്കിൽ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#അല്ലെങ്കിൽ				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((ശൂന്യം)c, (ശൂന്യം)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((ശൂന്യം)L, (ശൂന്യം)file, -1)

#endif				/* } */

#endif				/* } */


#എങ്കിൽ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#എങ്കിൽ !defined(l_getc)		/* { */

#എങ്കിൽ defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#അല്ലെങ്കിൽ
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((ശൂന്യം)0)
#define l_unlockfile(f)		((ശൂന്യം)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#എങ്കിൽ !defined(l_fseek)		/* { */

#എങ്കിൽ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#അന്തര്ഭാവ <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#അല്ലെങ്കിലെങ്കിൽ defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#അല്ലെങ്കിൽ				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(വലുപ്പം(IO_PREFIX)/വലുപ്പം(അക്ഷരം) - 1)
#define IO_നിവേശം	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


തരനിർവചനം luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == ശൂന്യ)


സ്ഥിതം പൂർണ്ണസംഖ്യ io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  എങ്കിൽ (p == ശൂന്യ)
    luaL_pushfail(L);  /* not a file */
  അല്ലെങ്കിൽ എങ്കിൽ (isclosed(p))
    lua_pushliteral(L, "closed file");
  അല്ലെങ്കിൽ
    lua_pushliteral(L, "file");
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  എങ്കിൽ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  അല്ലെങ്കിൽ
    lua_pushfstring(L, "file (%p)", p->f);
  തിരികെ 1;
}


സ്ഥിതം FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  എങ്കിൽ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  തിരികെ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
സ്ഥിതം LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, വലുപ്പം(LStream), 0);
  p->closef = ശൂന്യ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  തിരികെ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  അസ്ഥിര lua_CFunction cf = p->closef;
  p->closef = ശൂന്യ;  /* mark stream as closed */
  തിരികെ (*cf)(L);  /* close it */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  തിരികെ aux_close(L);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_close (lua_State *L) {
  എങ്കിൽ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  തിരികെ f_close(L);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  എങ്കിൽ (!isclosed(p) && p->f != ശൂന്യ)
    aux_close(L);  /* ignore closed and incompletely open files */
  തിരികെ 0;
}


/*
** function to close regular files
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  തിരികെ luaL_fileresult(L, (fclose(p->f) == 0), ശൂന്യ);
}


സ്ഥിതം LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ശൂന്യ;
  p->closef = &io_fclose;
  തിരികെ p;
}


സ്ഥിതം ശൂന്യം opencheck (lua_State *L, സ്ഥിരം അക്ഷരം *fname, സ്ഥിരം അക്ഷരം *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  എങ്കിൽ (l_unlikely(p->f == ശൂന്യ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_open (lua_State *L) {
  സ്ഥിരം അക്ഷരം *filename = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  സ്ഥിരം അക്ഷരം *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  തിരികെ (p->f == ശൂന്യ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  തിരികെ luaL_execresult(L, l_pclose(L, p->f));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_popen (lua_State *L) {
  സ്ഥിരം അക്ഷരം *filename = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  തിരികെ (p->f == ശൂന്യ) ? luaL_fileresult(L, 0, filename) : 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  തിരികെ (p->f == ശൂന്യ) ? luaL_fileresult(L, 0, ശൂന്യ) : 1;
}


സ്ഥിതം FILE *getiofile (lua_State *L, സ്ഥിരം അക്ഷരം *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  എങ്കിൽ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  തിരികെ p->f;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ g_iofile (lua_State *L, സ്ഥിരം അക്ഷരം *f, സ്ഥിരം അക്ഷരം *mode) {
  എങ്കിൽ (!lua_isnoneornil(L, 1)) {
    സ്ഥിരം അക്ഷരം *filename = lua_tostring(L, 1);
    എങ്കിൽ (filename)
      opencheck(L, filename, mode);
    അല്ലെങ്കിൽ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_നിവേശം (lua_State *L) {
  തിരികെ g_iofile(L, IO_നിവേശം, "r");
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_output (lua_State *L) {
  തിരികെ g_iofile(L, IO_OUTPUT, "w");
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_readline (lua_State *L);


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
സ്ഥിതം ശൂന്യം aux_വരികൾ (lua_State *L, പൂർണ്ണസംഖ്യ toclose) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_വരികൾ (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_വരികൾ(L, 0);
  തിരികെ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ io_വരികൾ (lua_State *L) {
  പൂർണ്ണസംഖ്യ toclose;
  എങ്കിൽ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  എങ്കിൽ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_നിവേശം);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  അല്ലെങ്കിൽ {  /* open a new file */
    സ്ഥിരം അക്ഷരം *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_വരികൾ(L, toclose);  /* push iteration function */
  എങ്കിൽ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    തിരികെ 4;
  }
  അല്ലെങ്കിൽ
    തിരികെ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#എങ്കിൽ !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
തരനിർവചനം ഘടന {
  FILE *f;  /* file being read */
  പൂർണ്ണസംഖ്യ c;  /* current character (look ahead) */
  പൂർണ്ണസംഖ്യ n;  /* number of elements in buffer 'buff' */
  അക്ഷരം buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ nextc (RN *rn) {
  എങ്കിൽ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    തിരികെ 0;  /* fail */
  }
  അല്ലെങ്കിൽ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    തിരികെ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ test2 (RN *rn, സ്ഥിരം അക്ഷരം *set) {
  എങ്കിൽ (rn->c == set[0] || rn->c == set[1])
    തിരികെ nextc(rn);
  അല്ലെങ്കിൽ തിരികെ 0;
}


/*
** Read a sequence of (hex)digits
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ readdigits (RN *rn, പൂർണ്ണസംഖ്യ hex) {
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  ആകുമ്പോൾ ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    എണ്ണം++;
  തിരികെ എണ്ണം;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ read_സംഖ്യ (lua_State *L, FILE *f) {
  RN rn;
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ hex = 0;
  അക്ഷരം decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  ചെയ്യുക { rn.c = l_getc(rn.f); } ആകുമ്പോൾ (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  എങ്കിൽ (test2(&rn, "00")) {
    എങ്കിൽ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    അല്ലെങ്കിൽ എണ്ണം = 1;  /* count initial '0' as a valid digit */
  }
  എണ്ണം += readdigits(&rn, hex);  /* integral part */
  എങ്കിൽ (test2(&rn, decp))  /* decimal point? */
    എണ്ണം += readdigits(&rn, hex);  /* fractional part */
  എങ്കിൽ (എണ്ണം > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  എങ്കിൽ (l_likely(lua_stringtonumber(L, rn.buff)))
    തിരികെ 1;  /* ok, it is a valid number */
  അല്ലെങ്കിൽ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   തിരികെ 0;  /* read fails */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ test_eof (lua_State *L, FILE *f) {
  പൂർണ്ണസംഖ്യ c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  തിരികെ (c != EOF);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ read_വരി (lua_State *L, FILE *f, പൂർണ്ണസംഖ്യ chop) {
  luaL_Buffer b;
  പൂർണ്ണസംഖ്യ c;
  luaL_buffinit(L, &b);
  ചെയ്യുക {  /* may need to read several chunks to get whole line */
    അക്ഷരം *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ആകുമ്പോൾ (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ആകുമ്പോൾ (c != EOF && c != '\n');  /* repeat until end of line */
  എങ്കിൽ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  തിരികെ (c == '\n' || lua_rawlen(L, -1) > 0);
}


സ്ഥിതം ശൂന്യം read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ചെയ്യുക {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    അക്ഷരം *p = luaL_prepbuffer(&b);
    nr = fread(p, വലുപ്പം(അക്ഷരം), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ആകുമ്പോൾ (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  അക്ഷരം *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, വലുപ്പം(അക്ഷരം), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  തിരികെ (nr > 0);  /* true iff read something */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ g_read (lua_State *L, FILE *f, പൂർണ്ണസംഖ്യ first) {
  പൂർണ്ണസംഖ്യ nargs = lua_gettop(L) - 1;
  പൂർണ്ണസംഖ്യ n, success;
  clearerr(f);
  errno = 0;
  എങ്കിൽ (nargs == 0) {  /* no arguments? */
    success = read_വരി(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  അല്ലെങ്കിൽ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    വേണ്ടി (n = first; nargs-- && success; n++) {
      എങ്കിൽ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      അല്ലെങ്കിൽ {
        സ്ഥിരം അക്ഷരം *p = luaL_checkstring(L, n);
        എങ്കിൽ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        തിരഞ്ഞെടുപ്പ് (*p) {
          അവസ്ഥ 'n':  /*സംഖ്യ*/
            success = read_സംഖ്യ(L, f);
            നിർത്തുക;
          അവസ്ഥ 'l':  /*വരി*/
            success = read_വരി(L, f, 1);
            നിർത്തുക;
          അവസ്ഥ 'L':  /* line with end-of-line */
            success = read_വരി(L, f, 0);
            നിർത്തുക;
          അവസ്ഥ 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            നിർത്തുക;
          സ്ഥിരസ്ഥിതി:
            തിരികെ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  എങ്കിൽ (ferror(f))
    തിരികെ luaL_fileresult(L, 0, ശൂന്യ);
  എങ്കിൽ (!success) {
    lua_എടുക്കുക(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  തിരികെ n - first;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_read (lua_State *L) {
  തിരികെ g_read(L, getiofile(L, IO_നിവേശം), 1);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_read (lua_State *L) {
  തിരികെ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = (പൂർണ്ണസംഖ്യ)lua_tointeger(L, lua_upvalueindex(2));
  എങ്കിൽ (isclosed(p))  /* file is already closed? */
    തിരികെ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  വേണ്ടി (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  എങ്കിൽ (lua_toboolean(L, -n))  /* read at least one value? */
    തിരികെ n;  /* return them */
  അല്ലെങ്കിൽ {  /* first result is false: EOF or error */
    എങ്കിൽ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      തിരികെ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    എങ്കിൽ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    തിരികെ 0;
  }
}

/* }====================================================== */


സ്ഥിതം പൂർണ്ണസംഖ്യ g_write (lua_State *L, FILE *f, പൂർണ്ണസംഖ്യ arg) {
  പൂർണ്ണസംഖ്യ nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  വേണ്ടി (; nargs--; arg++) {  /* for each argument */
    അക്ഷരം buff[LUA_N2SBUFFSZ];
    സ്ഥിരം അക്ഷരം *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    എങ്കിൽ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    അല്ലെങ്കിൽ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, വലുപ്പം(അക്ഷരം), len, f);
    totalbytes += numbytes;
    എങ്കിൽ (numbytes < len) {  /* write error? */
      പൂർണ്ണസംഖ്യ n = luaL_fileresult(L, 0, ശൂന്യ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      തിരികെ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  തിരികെ 1;  /* no errors; file handle already on stack top */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_write (lua_State *L) {
  തിരികെ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  തിരികെ g_write(L, f, 2);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_seek (lua_State *L) {
  സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം modenames[] = {"set", "cur", "end", ശൂന്യ};
  FILE *f = tofile(L);
  പൂർണ്ണസംഖ്യ op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  എങ്കിൽ (l_unlikely(op))
    തിരികെ luaL_fileresult(L, 0, ശൂന്യ);  /* error */
  അല്ലെങ്കിൽ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    തിരികെ 1;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_setvbuf (lua_State *L) {
  സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ mode[] = {_IONBF, _IOFBF, _IOLBF};
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം modenames[] = {"no", "full", "line", ശൂന്യ};
  FILE *f = tofile(L);
  പൂർണ്ണസംഖ്യ op = luaL_checkoption(L, 2, ശൂന്യ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  പൂർണ്ണസംഖ്യ res;
  errno = 0;
  res = setvbuf(f, ശൂന്യ, mode[op], (size_t)sz);
  തിരികെ luaL_fileresult(L, res == 0, ശൂന്യ);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  തിരികെ luaL_fileresult(L, fflush(f) == 0, ശൂന്യ);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ f_flush (lua_State *L) {
  തിരികെ aux_flush(L, tofile(L));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ io_flush (lua_State *L) {
  തിരികെ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
സ്ഥിതം സ്ഥിരം luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_നിവേശം},
  {"lines", io_വരികൾ},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ശൂന്യ, ശൂന്യ}
};


/*
** methods for file handles
*/
സ്ഥിതം സ്ഥിരം luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_വരികൾ},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ശൂന്യ, ശൂന്യ}
};


/*
** metamethods for file handles
*/
സ്ഥിതം സ്ഥിരം luaL_Reg metameth[] = {
  {"__index", ശൂന്യ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ശൂന്യ, ശൂന്യ}
};


സ്ഥിതം ശൂന്യം createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_എടുക്കുക(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  തിരികെ 2;
}


സ്ഥിതം ശൂന്യം createstdfile (lua_State *L, FILE *f, സ്ഥിരം അക്ഷരം *k,
                           സ്ഥിരം അക്ഷരം *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  എങ്കിൽ (k != ശൂന്യ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_നിവേശം, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ശൂന്യ, "stderr");
  തിരികെ 1;
}

