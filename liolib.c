/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#ಯದಿ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ಯದಿ !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ l_checkmode (ಧ್ರುವ ವರ್ಣ *mode) {
  ಪ್ರತ್ಯಾಗಮ (*mode != '\0' && strchr("rwa", *(mode++)) != ರಿಕ್ತ &&
         (*mode != '+' || ((ಶೂನ್ಯ)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ಯದಿ !defined(l_popen)		/* { */

#ಯದಿ defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(ರಿಕ್ತ), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#ನೋಚೇದ್ಯದಿ defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#ಯದಿ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#ಅನ್ಯಥಾ				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((ಶೂನ್ಯ)c, (ಶೂನ್ಯ)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((ಶೂನ್ಯ)L, (ಶೂನ್ಯ)file, -1)

#endif				/* } */

#endif				/* } */


#ಯದಿ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#ಯದಿ !defined(l_getc)		/* { */

#ಯದಿ defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#ಅನ್ಯಥಾ
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((ಶೂನ್ಯ)0)
#define l_unlockfile(f)		((ಶೂನ್ಯ)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ಯದಿ !defined(l_fseek)		/* { */

#ಯದಿ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#ಅನ್ತರ್ಭಾವ <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#ನೋಚೇದ್ಯದಿ defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#ಅನ್ಯಥಾ				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(ಪರಿಮಾಣ(IO_PREFIX)/ಪರಿಮಾಣ(ವರ್ಣ) - 1)
#define IO_ಒಳಹರಿವು	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


ಪ್ರಕಾರನಿರ್ದೇಶ luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == ರಿಕ್ತ)


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ಯದಿ (p == ರಿಕ್ತ)
    luaL_pushfail(L);  /* not a file */
  ಅನ್ಯಥಾ ಯದಿ (isclosed(p))
    lua_pushliteral(L, "closed file");
  ಅನ್ಯಥಾ
    lua_pushliteral(L, "file");
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ಯದಿ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ಅನ್ಯಥಾ
    lua_pushfstring(L, "file (%p)", p->f);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ಯದಿ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ಪ್ರತ್ಯಾಗಮ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ಸ್ಥಿರ LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, ಪರಿಮಾಣ(LStream), 0);
  p->closef = ರಿಕ್ತ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ಪ್ರತ್ಯಾಗಮ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ಚಞ್ಚಲ lua_CFunction cf = p->closef;
  p->closef = ರಿಕ್ತ;  /* mark stream as closed */
  ಪ್ರತ್ಯಾಗಮ (*cf)(L);  /* close it */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ಪ್ರತ್ಯಾಗಮ aux_close(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_close (lua_State *L) {
  ಯದಿ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ಪ್ರತ್ಯಾಗಮ f_close(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ಯದಿ (!isclosed(p) && p->f != ರಿಕ್ತ)
    aux_close(L);  /* ignore closed and incompletely open files */
  ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** function to close regular files
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, (fclose(p->f) == 0), ರಿಕ್ತ);
}


ಸ್ಥಿರ LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ರಿಕ್ತ;
  p->closef = &io_fclose;
  ಪ್ರತ್ಯಾಗಮ p;
}


ಸ್ಥಿರ ಶೂನ್ಯ opencheck (lua_State *L, ಧ್ರುವ ವರ್ಣ *fname, ಧ್ರುವ ವರ್ಣ *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  ಯದಿ (l_unlikely(p->f == ರಿಕ್ತ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_open (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *filename = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ಧ್ರುವ ವರ್ಣ *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  ಪ್ರತ್ಯಾಗಮ (p->f == ರಿಕ್ತ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ಪ್ರತ್ಯಾಗಮ luaL_execresult(L, l_pclose(L, p->f));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_popen (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *filename = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  ಪ್ರತ್ಯಾಗಮ (p->f == ರಿಕ್ತ) ? luaL_fileresult(L, 0, filename) : 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ಪ್ರತ್ಯಾಗಮ (p->f == ರಿಕ್ತ) ? luaL_fileresult(L, 0, ರಿಕ್ತ) : 1;
}


ಸ್ಥಿರ FILE *getiofile (lua_State *L, ಧ್ರುವ ವರ್ಣ *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ಯದಿ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ಪ್ರತ್ಯಾಗಮ p->f;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ g_iofile (lua_State *L, ಧ್ರುವ ವರ್ಣ *f, ಧ್ರುವ ವರ್ಣ *mode) {
  ಯದಿ (!lua_isnoneornil(L, 1)) {
    ಧ್ರುವ ವರ್ಣ *filename = lua_tostring(L, 1);
    ಯದಿ (filename)
      opencheck(L, filename, mode);
    ಅನ್ಯಥಾ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_ಒಳಹರಿವು (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ g_iofile(L, IO_ಒಳಹರಿವು, "r");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_output (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ g_iofile(L, IO_OUTPUT, "w");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_readline (lua_State *L);


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
ಸ್ಥಿರ ಶೂನ್ಯ aux_ಸಾಲುಗಳು (lua_State *L, ಪೂರ್ಣಾಙ್ಕ toclose) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_ಸಾಲುಗಳು (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ಸಾಲುಗಳು(L, 0);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_ಸಾಲುಗಳು (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ toclose;
  ಯದಿ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ಯದಿ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_ಒಳಹರಿವು);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ಅನ್ಯಥಾ {  /* open a new file */
    ಧ್ರುವ ವರ್ಣ *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ಸಾಲುಗಳು(L, toclose);  /* push iteration function */
  ಯದಿ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ಪ್ರತ್ಯಾಗಮ 4;
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ಯದಿ !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  FILE *f;  /* file being read */
  ಪೂರ್ಣಾಙ್ಕ c;  /* current character (look ahead) */
  ಪೂರ್ಣಾಙ್ಕ n;  /* number of elements in buffer 'buff' */
  ವರ್ಣ buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nextc (RN *rn) {
  ಯದಿ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ಪ್ರತ್ಯಾಗಮ 0;  /* fail */
  }
  ಅನ್ಯಥಾ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test2 (RN *rn, ಧ್ರುವ ವರ್ಣ *set) {
  ಯದಿ (rn->c == set[0] || rn->c == set[1])
    ಪ್ರತ್ಯಾಗಮ nextc(rn);
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** Read a sequence of (hex)digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ readdigits (RN *rn, ಪೂರ್ಣಾಙ್ಕ hex) {
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಯಾವತ್ ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    ಎಣಿಕೆ++;
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_ಸಂಖ್ಯೆ (lua_State *L, FILE *f) {
  RN rn;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಙ್ಕ hex = 0;
  ವರ್ಣ decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  ಕುರು { rn.c = l_getc(rn.f); } ಯಾವತ್ (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ಯದಿ (test2(&rn, "00")) {
    ಯದಿ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ಅನ್ಯಥಾ ಎಣಿಕೆ = 1;  /* count initial '0' as a valid digit */
  }
  ಎಣಿಕೆ += readdigits(&rn, hex);  /* integral part */
  ಯದಿ (test2(&rn, decp))  /* decimal point? */
    ಎಣಿಕೆ += readdigits(&rn, hex);  /* fractional part */
  ಯದಿ (ಎಣಿಕೆ > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ಯದಿ (l_likely(lua_stringtonumber(L, rn.buff)))
    ಪ್ರತ್ಯಾಗಮ 1;  /* ok, it is a valid number */
  ಅನ್ಯಥಾ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ಪ್ರತ್ಯಾಗಮ 0;  /* read fails */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_eof (lua_State *L, FILE *f) {
  ಪೂರ್ಣಾಙ್ಕ c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ಪ್ರತ್ಯಾಗಮ (c != EOF);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_ಸಾಲು (lua_State *L, FILE *f, ಪೂರ್ಣಾಙ್ಕ chop) {
  luaL_Buffer b;
  ಪೂರ್ಣಾಙ್ಕ c;
  luaL_buffinit(L, &b);
  ಕುರು {  /* may need to read several chunks to get whole line */
    ವರ್ಣ *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ಯಾವತ್ (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ಯಾವತ್ (c != EOF && c != '\n');  /* repeat until end of line */
  ಯದಿ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ಪ್ರತ್ಯಾಗಮ (c == '\n' || lua_rawlen(L, -1) > 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ಕುರು {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    ವರ್ಣ *p = luaL_prepbuffer(&b);
    nr = fread(p, ಪರಿಮಾಣ(ವರ್ಣ), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ಯಾವತ್ (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  ವರ್ಣ *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, ಪರಿಮಾಣ(ವರ್ಣ), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ಪ್ರತ್ಯಾಗಮ (nr > 0);  /* true iff read something */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ g_read (lua_State *L, FILE *f, ಪೂರ್ಣಾಙ್ಕ first) {
  ಪೂರ್ಣಾಙ್ಕ nargs = lua_gettop(L) - 1;
  ಪೂರ್ಣಾಙ್ಕ n, success;
  clearerr(f);
  errno = 0;
  ಯದಿ (nargs == 0) {  /* no arguments? */
    success = read_ಸಾಲು(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  ಅನ್ಯಥಾ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    ಪ್ರತಿ (n = first; nargs-- && success; n++) {
      ಯದಿ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ಅನ್ಯಥಾ {
        ಧ್ರುವ ವರ್ಣ *p = luaL_checkstring(L, n);
        ಯದಿ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        ವಿಕಲ್ಪ (*p) {
          ಪಕ್ಷ 'n':  /*ಸಂಖ್ಯೆ*/
            success = read_ಸಂಖ್ಯೆ(L, f);
            ಭಙ್ಗ;
          ಪಕ್ಷ 'l':  /*ಸಾಲು*/
            success = read_ಸಾಲು(L, f, 1);
            ಭಙ್ಗ;
          ಪಕ್ಷ 'L':  /* line with end-of-line */
            success = read_ಸಾಲು(L, f, 0);
            ಭಙ್ಗ;
          ಪಕ್ಷ 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            ಭಙ್ಗ;
          ಸ್ವಭಾವ:
            ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ಯದಿ (ferror(f))
    ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, 0, ರಿಕ್ತ);
  ಯದಿ (!success) {
    lua_ತೆಗೆ(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ಪ್ರತ್ಯಾಗಮ n - first;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_read (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ g_read(L, getiofile(L, IO_ಒಳಹರಿವು), 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_read (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, lua_upvalueindex(2));
  ಯದಿ (isclosed(p))  /* file is already closed? */
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  ಪ್ರತಿ (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ಯದಿ (lua_toboolean(L, -n))  /* read at least one value? */
    ಪ್ರತ್ಯಾಗಮ n;  /* return them */
  ಅನ್ಯಥಾ {  /* first result is false: EOF or error */
    ಯದಿ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ಯದಿ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ g_write (lua_State *L, FILE *f, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಪೂರ್ಣಾಙ್ಕ nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  ಪ್ರತಿ (; nargs--; arg++) {  /* for each argument */
    ವರ್ಣ buff[LUA_N2SBUFFSZ];
    ಧ್ರುವ ವರ್ಣ *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ಯದಿ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ಅನ್ಯಥಾ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, ಪರಿಮಾಣ(ವರ್ಣ), len, f);
    totalbytes += numbytes;
    ಯದಿ (numbytes < len) {  /* write error? */
      ಪೂರ್ಣಾಙ್ಕ n = luaL_fileresult(L, 0, ರಿಕ್ತ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ಪ್ರತ್ಯಾಗಮ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ಪ್ರತ್ಯಾಗಮ 1;  /* no errors; file handle already on stack top */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_write (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ಪ್ರತ್ಯಾಗಮ g_write(L, f, 2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_seek (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ modenames[] = {"set", "cur", "end", ರಿಕ್ತ};
  FILE *f = tofile(L);
  ಪೂರ್ಣಾಙ್ಕ op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  ಯದಿ (l_unlikely(op))
    ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, 0, ರಿಕ್ತ);  /* error */
  ಅನ್ಯಥಾ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_setvbuf (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ mode[] = {_IONBF, _IOFBF, _IOLBF};
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ modenames[] = {"no", "full", "line", ರಿಕ್ತ};
  FILE *f = tofile(L);
  ಪೂರ್ಣಾಙ್ಕ op = luaL_checkoption(L, 2, ರಿಕ್ತ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  ಪೂರ್ಣಾಙ್ಕ res;
  errno = 0;
  res = setvbuf(f, ರಿಕ್ತ, mode[op], (size_t)sz);
  ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, res == 0, ರಿಕ್ತ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, fflush(f) == 0, ರಿಕ್ತ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ f_flush (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ aux_flush(L, tofile(L));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_flush (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ಸ್ಥಿರ ಧ್ರುವ luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_ಒಳಹರಿವು},
  {"lines", io_ಸಾಲುಗಳು},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


/*
** methods for file handles
*/
ಸ್ಥಿರ ಧ್ರುವ luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ಸಾಲುಗಳು},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


/*
** metamethods for file handles
*/
ಸ್ಥಿರ ಧ್ರುವ luaL_Reg metameth[] = {
  {"__index", ರಿಕ್ತ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


ಸ್ಥಿರ ಶೂನ್ಯ createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_ತೆಗೆ(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ಪ್ರತ್ಯಾಗಮ 2;
}


ಸ್ಥಿರ ಶೂನ್ಯ createstdfile (lua_State *L, FILE *f, ಧ್ರುವ ವರ್ಣ *k,
                           ಧ್ರುವ ವರ್ಣ *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ಯದಿ (k != ರಿಕ್ತ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_ಒಳಹರಿವು, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ರಿಕ್ತ, "stderr");
  ಪ್ರತ್ಯಾಗಮ 1;
}

