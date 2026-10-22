/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು liolib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

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
#ಆದರೆ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ಆದರೆ !defined(L_MODEEXT)
#ವ್ಯಾಖ್ಯಾನಿಸು L_MODEEXT	"b"
#ಆದರೆ_ಅಂತ್ಯ

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ l_checkmode (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ಹಿಂತಿರುಗಿ (*ಬಹುಲಕ != '\0' && strchr("rwa", *(ಬಹುಲಕ++)) != ಖಾಲಿ &&
         (*ಬಹುಲಕ != '+' || ((ಶೂನ್ಯ)(++ಬಹುಲಕ), 1)) &&  /* skip if char is '+' */
         (strspn(ಬಹುಲಕ, L_MODEEXT) == ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಹುಲಕ)));  /* check extensions */
}

#ಆದರೆ_ಅಂತ್ಯ

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ಆದರೆ !defined(l_popen)		/* { */

#ಆದರೆ defined(LUA_USE_POSIX)	/* { */

#ವ್ಯಾಖ್ಯಾನಿಸು l_popen(L,c,m)		(fflush(ಖಾಲಿ), popen(c,m))
#ವ್ಯಾಖ್ಯಾನಿಸು l_pclose(L,file)	(pclose(file))

#ಅಥವಾಆದರೆ defined(LUA_USE_WINDOWS)	/* }{ */

#ವ್ಯಾಖ್ಯಾನಿಸು l_popen(L,c,m)		(_popen(c,m))
#ವ್ಯಾಖ್ಯಾನಿಸು l_pclose(L,file)	(_pclose(file))

#ಆದರೆ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#ವ್ಯಾಖ್ಯಾನಿಸು l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

/* ISO C definitions */
#ವ್ಯಾಖ್ಯಾನಿಸು l_popen(L,c,m)  \
	  ((ಶೂನ್ಯ)c, (ಶೂನ್ಯ)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#ವ್ಯಾಖ್ಯಾನಿಸು l_pclose(L,file)		((ಶೂನ್ಯ)L, (ಶೂನ್ಯ)file, -1)

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */


#ಆದರೆ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#ವ್ಯಾಖ್ಯಾನಿಸು l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#ಆದರೆ_ಅಂತ್ಯ

/* }====================================================== */


#ಆದರೆ !defined(l_getc)		/* { */

#ಆದರೆ defined(LUA_USE_POSIX)
#ವ್ಯಾಖ್ಯಾನಿಸು l_getc(f)		getc_unlocked(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_lockfile(f)		flockfile(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_unlockfile(f)		funlockfile(f)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_getc(f)		getc(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_lockfile(f)		((ಶೂನ್ಯ)0)
#ವ್ಯಾಖ್ಯಾನಿಸು l_unlockfile(f)		((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ಆದರೆ !defined(l_fseek)		/* { */

#ಆದರೆ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ವ್ಯಾಖ್ಯಾನಿಸು l_fseek(f,o,w)		fseeko(f,o,w)
#ವ್ಯಾಖ್ಯಾನಿಸು l_ftell(f)		ftello(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_seeknum		off_t

#ಅಥವಾಆದರೆ defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#ವ್ಯಾಖ್ಯಾನಿಸು l_fseek(f,o,w)		_fseeki64(f,o,w)
#ವ್ಯಾಖ್ಯಾನಿಸು l_ftell(f)		_ftelli64(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_seeknum		__int64

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

/* ISO C definitions */
#ವ್ಯಾಖ್ಯಾನಿಸು l_fseek(f,o,w)		fseek(f,o,w)
#ವ್ಯಾಖ್ಯಾನಿಸು l_ftell(f)		ftell(f)
#ವ್ಯಾಖ್ಯಾನಿಸು l_seeknum		ದೀರ್ಘ

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */

/* }====================================================== */



#ವ್ಯಾಖ್ಯಾನಿಸು IO_ಪೂರ್ವಪ್ರತ್ಯಯ	"_IO_"
#ವ್ಯಾಖ್ಯಾನಿಸು IOPREF_LEN	(ಗಾತ್ರ(IO_ಪೂರ್ವಪ್ರತ್ಯಯ)/ಗಾತ್ರ(ಅಕ್ಷರ) - 1)
#ವ್ಯಾಖ್ಯಾನಿಸು IO_ಒಳಹರಿವು	(IO_ಪೂರ್ವಪ್ರತ್ಯಯ "input")
#ವ್ಯಾಖ್ಯಾನಿಸು IO_OUTPUT	(IO_ಪೂರ್ವಪ್ರತ್ಯಯ "output")


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ lua_L_ಹರಿವು LStream;


#ವ್ಯಾಖ್ಯಾನಿಸು tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#ವ್ಯಾಖ್ಯಾನಿಸು isclosed(p)	((p)->closef == ಖಾಲಿ)


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ಆದರೆ (p == ಖಾಲಿ)
    luaL_pushfail(L);  /* not a file */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isclosed(p))
    lua_pushliteral(L, "closed file");
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushliteral(L, "file");
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ಆದರೆ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushfstring(L, "file (%p)", p->f);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ಆದರೆ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ಹಿಂತಿರುಗಿ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ಸ್ಥಿರ LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, ಗಾತ್ರ(LStream), 0);
  p->closef = ಖಾಲಿ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ಹಿಂತಿರುಗಿ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ಅಸ್ಥಿರ lua_CFunction cf = p->closef;
  p->closef = ಖಾಲಿ;  /* mark stream as closed */
  ಹಿಂತಿರುಗಿ (*cf)(L);  /* close it */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ಹಿಂತಿರುಗಿ aux_close(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_close (lua_State *L) {
  ಆದರೆ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ಹಿಂತಿರುಗಿ f_close(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ಆದರೆ (!isclosed(p) && p->f != ಖಾಲಿ)
    aux_close(L);  /* ignore closed and incompletely open files */
  ಹಿಂತಿರುಗಿ 0;
}


/*
** function to close regular files
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ಹಿಂತಿರುಗಿ luaL_fileresult(L, (fclose(p->f) == 0), ಖಾಲಿ);
}


ಸ್ಥಿರ LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ಖಾಲಿ;
  p->closef = &io_fclose;
  ಹಿಂತಿರುಗಿ p;
}


ಸ್ಥಿರ ಶೂನ್ಯ opencheck (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  LStream *p = newfile(L);
  p->f = fopen(fname, ಬಹುಲಕ);
  ಆದರೆ (l_unlikely(p->f == ಖಾಲಿ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_open (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *md = ಬಹುಲಕ;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, ಬಹುಲಕ);
  ಹಿಂತಿರುಗಿ (p->f == ಖಾಲಿ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ಹಿಂತಿರುಗಿ luaL_execresult(L, l_pclose(L, p->f));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_popen (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(ಬಹುಲಕ), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, ಬಹುಲಕ);
  p->closef = &io_pclose;
  ಹಿಂತಿರುಗಿ (p->f == ಖಾಲಿ) ? luaL_fileresult(L, 0, filename) : 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ಹಿಂತಿರುಗಿ (p->f == ಖಾಲಿ) ? luaL_fileresult(L, 0, ಖಾಲಿ) : 1;
}


ಸ್ಥಿರ FILE *getiofile (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ಆದರೆ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ಹಿಂತಿರುಗಿ p->f;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ g_iofile (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *f, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ಆದರೆ (!lua_isnoneornil(L, 1)) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = lua_tostring(L, 1);
    ಆದರೆ (filename)
      opencheck(L, filename, ಬಹುಲಕ);
    ಇಲ್ಲದಿದ್ದರೆ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_ಒಳಹರಿವು (lua_State *L) {
  ಹಿಂತಿರುಗಿ g_iofile(L, IO_ಒಳಹರಿವು, "r");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_output (lua_State *L) {
  ಹಿಂತಿರುಗಿ g_iofile(L, IO_OUTPUT, "w");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
ಸ್ಥಿರ ಶೂನ್ಯ aux_ಸಾಲುಗಳು (lua_State *L, ಪೂರ್ಣಾಂಕ toclose) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_ಸಾಲುಗಳು (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ಸಾಲುಗಳು(L, 0);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_ಸಾಲುಗಳು (lua_State *L) {
  ಪೂರ್ಣಾಂಕ toclose;
  ಆದರೆ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ಆದರೆ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_ಒಳಹರಿವು);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* open a new file */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ಸಾಲುಗಳು(L, toclose);  /* push iteration function */
  ಆದರೆ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ಹಿಂತಿರುಗಿ 4;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ಆದರೆ !defined (L_MAXLENNUM)
#ವ್ಯಾಖ್ಯಾನಿಸು L_MAXLENNUM     200
#ಆದರೆ_ಅಂತ್ಯ


/* auxiliary structure used by 'read_number' */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  FILE *f;  /* file being read */
  ಪೂರ್ಣಾಂಕ c;  /* current character (look ahead) */
  ಪೂರ್ಣಾಂಕ n;  /* number of elements in buffer 'buff' */
  ಅಕ್ಷರ buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nextc (RN *rn) {
  ಆದರೆ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ಹಿಂತಿರುಗಿ 0;  /* fail */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ಹಿಂತಿರುಗಿ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test2 (RN *rn, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗಣ) {
  ಆದರೆ (rn->c == ಗಣ[0] || rn->c == ಗಣ[1])
    ಹಿಂತಿರುಗಿ nextc(rn);
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;
}


/*
** Read a sequence of (hex)digits
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readdigits (RN *rn, ಪೂರ್ಣಾಂಕ hex) {
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಆಗ ((hex ? isxdigit(rn->c) : ಅಂಕಿಯೇ(rn->c)) && nextc(rn))
    ಎಣಿಕೆ_2++;
  ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_ಸಂಖ್ಯೆ (lua_State *L, FILE *f) {
  RN rn;
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಪೂರ್ಣಾಂಕ hex = 0;
  ಅಕ್ಷರ decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  ಮಾಡು { rn.c = l_getc(rn.f); } ಆಗ (ಖಾಲಿಯೇ(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ಆದರೆ (test2(&rn, "00")) {
    ಆದರೆ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ಇಲ್ಲದಿದ್ದರೆ ಎಣಿಕೆ_2 = 1;  /* count initial '0' as a valid digit */
  }
  ಎಣಿಕೆ_2 += readdigits(&rn, hex);  /* integral part */
  ಆದರೆ (test2(&rn, decp))  /* decimal point? */
    ಎಣಿಕೆ_2 += readdigits(&rn, hex);  /* fractional part */
  ಆದರೆ (ಎಣಿಕೆ_2 > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ಆದರೆ (l_likely(lua_stringtonumber(L, rn.buff)))
    ಹಿಂತಿರುಗಿ 1;  /* ok, it is a valid number */
  ಇಲ್ಲದಿದ್ದರೆ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ಹಿಂತಿರುಗಿ 0;  /* read fails */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_eof (lua_State *L, FILE *f) {
  ಪೂರ್ಣಾಂಕ c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ಹಿಂತಿರುಗಿ (c != EOF);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_ಸಾಲು (lua_State *L, FILE *f, ಪೂರ್ಣಾಂಕ chop) {
  lua_L_ಬಫರ್ b;
  ಪೂರ್ಣಾಂಕ c;
  luaL_buffinit(L, &b);
  ಮಾಡು {  /* may need to read several chunks to get whole line */
    ಅಕ್ಷರ *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    ಚಿಹ್ನೆರಹಿತ i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ಆಗ (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ಆಗ (c != EOF && c != '\n');  /* repeat until end of line */
  ಆದರೆ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ಹಿಂತಿರುಗಿ (c == '\n' || lua_rawlen(L, -1) > 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_ಬಫರ್ b;
  luaL_buffinit(L, &b);
  ಮಾಡು {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    ಅಕ್ಷರ *p = luaL_prepbuffer(&b);
    nr = fread(p, ಗಾತ್ರ(ಅಕ್ಷರ), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ಆಗ (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  ಅಕ್ಷರ *p;
  lua_L_ಬಫರ್ b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, ಗಾತ್ರ(ಅಕ್ಷರ), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ಹಿಂತಿರುಗಿ (nr > 0);  /* true iff read something */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ g_read (lua_State *L, FILE *f, ಪೂರ್ಣಾಂಕ ಮೊದಲ) {
  ಪೂರ್ಣಾಂಕ nargs = lua_gettop(L) - 1;
  ಪೂರ್ಣಾಂಕ n, success;
  clearerr(f);
  errno = 0;
  ಆದರೆ (nargs == 0) {  /* no arguments? */
    success = read_ಸಾಲು(L, f, 1);
    n = ಮೊದಲ + 1;  /* to return 1 result */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    ಗಾಗಿ (n = ಮೊದಲ; nargs-- && success; n++) {
      ಆದರೆ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ಇಲ್ಲದಿದ್ದರೆ {
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = luaL_checkstring(L, n);
        ಆದರೆ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        ಆಯ್ಕೆ (*p) {
          ಸ್ಥಿತಿ 'n':  /*ಸಂಖ್ಯೆ*/
            success = read_ಸಂಖ್ಯೆ(L, f);
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 'l':  /*ಸಾಲು*/
            success = read_ಸಾಲು(L, f, 1);
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 'L':  /* line with end-of-line */
            success = read_ಸಾಲು(L, f, 0);
            ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            ನಿಲ್ಲಿಸು;
          ಪೂರ್ವನಿಯೋಜಿತ:
            ಹಿಂತಿರುಗಿ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ಆದರೆ (ferror(f))
    ಹಿಂತಿರುಗಿ luaL_fileresult(L, 0, ಖಾಲಿ);
  ಆದರೆ (!success) {
    lua_ತೆಗೆ(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ಹಿಂತಿರುಗಿ n - ಮೊದಲ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_read (lua_State *L) {
  ಹಿಂತಿರುಗಿ g_read(L, getiofile(L, IO_ಒಳಹರಿವು), 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_read (lua_State *L) {
  ಹಿಂತಿರುಗಿ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, lua_upvalueindex(2));
  ಆದರೆ (isclosed(p))  /* file is already closed? */
    ಹಿಂತಿರುಗಿ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  ಗಾಗಿ (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ಆದರೆ (lua_toboolean(L, -n))  /* read at least one value? */
    ಹಿಂತಿರುಗಿ n;  /* return them */
  ಇಲ್ಲದಿದ್ದರೆ {  /* first result is false: EOF or error */
    ಆದರೆ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ಹಿಂತಿರುಗಿ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ಆದರೆ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ಹಿಂತಿರುಗಿ 0;
  }
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ g_write (lua_State *L, FILE *f, ಪೂರ್ಣಾಂಕ arg) {
  ಪೂರ್ಣಾಂಕ nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  ಗಾಗಿ (; nargs--; arg++) {  /* for each argument */
    ಅಕ್ಷರ buff[LUA_N2SBUFFSZ];
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ಆದರೆ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ಇಲ್ಲದಿದ್ದರೆ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, ಗಾತ್ರ(ಅಕ್ಷರ), len, f);
    totalbytes += numbytes;
    ಆದರೆ (numbytes < len) {  /* write error? */
      ಪೂರ್ಣಾಂಕ n = luaL_fileresult(L, 0, ಖಾಲಿ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ಹಿಂತಿರುಗಿ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ಹಿಂತಿರುಗಿ 1;  /* no errors; file handle already on stack top */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_write (lua_State *L) {
  ಹಿಂತಿರುಗಿ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ಹಿಂತಿರುಗಿ g_write(L, f, 2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_seek (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಬಹುಲಕ[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ modenames[] = {"set", "cur", "end", ಖಾಲಿ};
  FILE *f = tofile(L);
  ಪೂರ್ಣಾಂಕ op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum ಸ್ಥಾನಾಂತರ = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)ಸ್ಥಾನಾಂತರ == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, ಸ್ಥಾನಾಂತರ, ಬಹುಲಕ[op]);
  ಆದರೆ (l_unlikely(op))
    ಹಿಂತಿರುಗಿ luaL_fileresult(L, 0, ಖಾಲಿ);  /* error */
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ಹಿಂತಿರುಗಿ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_setvbuf (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಬಹುಲಕ[] = {_IONBF, _IOFBF, _IOLBF};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ modenames[] = {"no", "full", "line", ಖಾಲಿ};
  FILE *f = tofile(L);
  ಪೂರ್ಣಾಂಕ op = luaL_checkoption(L, 2, ಖಾಲಿ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  ಪೂರ್ಣಾಂಕ res;
  errno = 0;
  res = setvbuf(f, ಖಾಲಿ, ಬಹುಲಕ[op], (size_t)sz);
  ಹಿಂತಿರುಗಿ luaL_fileresult(L, res == 0, ಖಾಲಿ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ಹಿಂತಿರುಗಿ luaL_fileresult(L, fflush(f) == 0, ಖಾಲಿ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ f_flush (lua_State *L) {
  ಹಿಂತಿರುಗಿ aux_flush(L, tofile(L));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_flush (lua_State *L) {
  ಹಿಂತಿರುಗಿ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg iolib[] = {
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
  {ಖಾಲಿ, ಖಾಲಿ}
};


/*
** methods for file handles
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ಸಾಲುಗಳು},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ಖಾಲಿ, ಖಾಲಿ}
};


/*
** metamethods for file handles
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg metameth[] = {
  {"__index", ಖಾಲಿ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ಖಾಲಿ, ಖಾಲಿ}
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ಹಿಂತಿರುಗಿ 2;
}


ಸ್ಥಿರ ಶೂನ್ಯ createstdfile (lua_State *L, FILE *f, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k,
                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ಆದರೆ (k != ಖಾಲಿ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_ಒಳಹರಿವು, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ಖಾಲಿ, "stderr");
  ಹಿಂತಿರುಗಿ 1;
}

