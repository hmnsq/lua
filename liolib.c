/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ liolib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#ਜੇ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ਜੇ !defined(L_MODEEXT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_MODEEXT	"b"
#ਜੇ_ਅੰਤ

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ਸਥਿਰ ਪੂਰਨ l_checkmode (ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ਵਾਪਸ (*ਬਹੁਲਕ != '\0' && strchr("rwa", *(ਬਹੁਲਕ++)) != ਨਿਰਾ &&
         (*ਬਹੁਲਕ != '+' || ((ਖਾਲੀ)(++ਬਹੁਲਕ), 1)) &&  /* skip if char is '+' */
         (strspn(ਬਹੁਲਕ, L_MODEEXT) == ਲੜੀ_ਲੰਬਾਈ(ਬਹੁਲਕ)));  /* check extensions */
}

#ਜੇ_ਅੰਤ

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ਜੇ !defined(l_popen)		/* { */

#ਜੇ defined(LUA_USE_POSIX)	/* { */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_popen(L,c,m)		(fflush(ਨਿਰਾ), popen(c,m))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_pclose(L,file)	(pclose(file))

#ਜਾਂਜੇ defined(LUA_USE_WINDOWS)	/* }{ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_popen(L,c,m)		(_popen(c,m))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_pclose(L,file)	(_pclose(file))

#ਜੇ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ				/* }{ */

/* ISO C definitions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_popen(L,c,m)  \
	  ((ਖਾਲੀ)c, (ਖਾਲੀ)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_pclose(L,file)		((ਖਾਲੀ)L, (ਖਾਲੀ)file, -1)

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */


#ਜੇ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#ਜੇ_ਅੰਤ

/* }====================================================== */


#ਜੇ !defined(l_getc)		/* { */

#ਜੇ defined(LUA_USE_POSIX)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_getc(f)		getc_unlocked(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_lockfile(f)		flockfile(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_unlockfile(f)		funlockfile(f)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_getc(f)		getc(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_lockfile(f)		((ਖਾਲੀ)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_unlockfile(f)		((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ਜੇ !defined(l_fseek)		/* { */

#ਜੇ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_fseek(f,o,w)		fseeko(f,o,w)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_ftell(f)		ftello(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_seeknum		off_t

#ਜਾਂਜੇ defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_fseek(f,o,w)		_fseeki64(f,o,w)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_ftell(f)		_ftelli64(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_seeknum		__int64

#ਨਹੀਂਤਾਂ				/* }{ */

/* ISO C definitions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_fseek(f,o,w)		fseek(f,o,w)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_ftell(f)		ftell(f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_seeknum		ਲੰਮਾ

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */

/* }====================================================== */



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IO_ਅਗੇਤਰ	"_IO_"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IOPREF_LEN	(ਆਕਾਰ(IO_ਅਗੇਤਰ)/ਆਕਾਰ(ਅੱਖਰ) - 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IO_ਨਿਵੇਸ਼	(IO_ਅਗੇਤਰ "input")
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IO_OUTPUT	(IO_ਅਗੇਤਰ "output")


ਕਿਸਮਪਰਿਭਾਸ਼ਾ lua_L_ਧਾਰਾ LStream;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isclosed(p)	((p)->closef == ਨਿਰਾ)


ਸਥਿਰ ਪੂਰਨ io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ਜੇ (p == ਨਿਰਾ)
    luaL_pushfail(L);  /* not a file */
  ਨਹੀਂਤਾਂ ਜੇ (isclosed(p))
    lua_pushliteral(L, "closed file");
  ਨਹੀਂਤਾਂ
    lua_pushliteral(L, "file");
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ਜੇ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ਨਹੀਂਤਾਂ
    lua_pushfstring(L, "file (%p)", p->f);
  ਵਾਪਸ 1;
}


ਸਥਿਰ FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ਜੇ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ਵਾਪਸ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ਸਥਿਰ LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, ਆਕਾਰ(LStream), 0);
  p->closef = ਨਿਰਾ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ਵਾਪਸ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ਸਥਿਰ ਪੂਰਨ aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ਅਸਥਿਰ lua_CFunction cf = p->closef;
  p->closef = ਨਿਰਾ;  /* mark stream as closed */
  ਵਾਪਸ (*cf)(L);  /* close it */
}


ਸਥਿਰ ਪੂਰਨ f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ਵਾਪਸ aux_close(L);
}


ਸਥਿਰ ਪੂਰਨ io_close (lua_State *L) {
  ਜੇ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ਵਾਪਸ f_close(L);
}


ਸਥਿਰ ਪੂਰਨ f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ਜੇ (!isclosed(p) && p->f != ਨਿਰਾ)
    aux_close(L);  /* ignore closed and incompletely open files */
  ਵਾਪਸ 0;
}


/*
** function to close regular files
*/
ਸਥਿਰ ਪੂਰਨ io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ਵਾਪਸ luaL_fileresult(L, (fclose(p->f) == 0), ਨਿਰਾ);
}


ਸਥਿਰ LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ਨਿਰਾ;
  p->closef = &io_fclose;
  ਵਾਪਸ p;
}


ਸਥਿਰ ਖਾਲੀ opencheck (lua_State *L, ਅਚਲ ਅੱਖਰ *fname, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  LStream *p = newfile(L);
  p->f = fopen(fname, ਬਹੁਲਕ);
  ਜੇ (l_unlikely(p->f == ਨਿਰਾ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ਸਥਿਰ ਪੂਰਨ io_open (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *filename = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ਅਚਲ ਅੱਖਰ *md = ਬਹੁਲਕ;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, ਬਹੁਲਕ);
  ਵਾਪਸ (p->f == ਨਿਰਾ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ਸਥਿਰ ਪੂਰਨ io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ਵਾਪਸ luaL_execresult(L, l_pclose(L, p->f));
}


ਸਥਿਰ ਪੂਰਨ io_popen (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *filename = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(ਬਹੁਲਕ), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, ਬਹੁਲਕ);
  p->closef = &io_pclose;
  ਵਾਪਸ (p->f == ਨਿਰਾ) ? luaL_fileresult(L, 0, filename) : 1;
}


ਸਥਿਰ ਪੂਰਨ io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ਵਾਪਸ (p->f == ਨਿਰਾ) ? luaL_fileresult(L, 0, ਨਿਰਾ) : 1;
}


ਸਥਿਰ FILE *getiofile (lua_State *L, ਅਚਲ ਅੱਖਰ *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ਜੇ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ਵਾਪਸ p->f;
}


ਸਥਿਰ ਪੂਰਨ g_iofile (lua_State *L, ਅਚਲ ਅੱਖਰ *f, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ਜੇ (!lua_isnoneornil(L, 1)) {
    ਅਚਲ ਅੱਖਰ *filename = lua_tostring(L, 1);
    ਜੇ (filename)
      opencheck(L, filename, ਬਹੁਲਕ);
    ਨਹੀਂਤਾਂ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ io_ਨਿਵੇਸ਼ (lua_State *L) {
  ਵਾਪਸ g_iofile(L, IO_ਨਿਵੇਸ਼, "r");
}


ਸਥਿਰ ਪੂਰਨ io_output (lua_State *L) {
  ਵਾਪਸ g_iofile(L, IO_OUTPUT, "w");
}


ਸਥਿਰ ਪੂਰਨ io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
ਸਥਿਰ ਖਾਲੀ aux_ਸਤਰਾਂ (lua_State *L, ਪੂਰਨ toclose) {
  ਪੂਰਨ n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ਸਥਿਰ ਪੂਰਨ f_ਸਤਰਾਂ (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ਸਤਰਾਂ(L, 0);
  ਵਾਪਸ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ਸਥਿਰ ਪੂਰਨ io_ਸਤਰਾਂ (lua_State *L) {
  ਪੂਰਨ toclose;
  ਜੇ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ਜੇ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_ਨਿਵੇਸ਼);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ਨਹੀਂਤਾਂ {  /* open a new file */
    ਅਚਲ ਅੱਖਰ *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ਸਤਰਾਂ(L, toclose);  /* push iteration function */
  ਜੇ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ਵਾਪਸ 4;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ਜੇ !defined (L_MAXLENNUM)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_MAXLENNUM     200
#ਜੇ_ਅੰਤ


/* auxiliary structure used by 'read_number' */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  FILE *f;  /* file being read */
  ਪੂਰਨ c;  /* current character (look ahead) */
  ਪੂਰਨ n;  /* number of elements in buffer 'buff' */
  ਅੱਖਰ buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ਸਥਿਰ ਪੂਰਨ nextc (RN *rn) {
  ਜੇ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ਵਾਪਸ 0;  /* fail */
  }
  ਨਹੀਂਤਾਂ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ਵਾਪਸ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ਸਥਿਰ ਪੂਰਨ test2 (RN *rn, ਅਚਲ ਅੱਖਰ *ਸੈੱਟ) {
  ਜੇ (rn->c == ਸੈੱਟ[0] || rn->c == ਸੈੱਟ[1])
    ਵਾਪਸ nextc(rn);
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;
}


/*
** Read a sequence of (hex)digits
*/
ਸਥਿਰ ਪੂਰਨ readdigits (RN *rn, ਪੂਰਨ hex) {
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਜਦੋਂ ((hex ? isxdigit(rn->c) : ਅੰਕ_ਹੈ(rn->c)) && nextc(rn))
    ਗਿਣਤੀ_2++;
  ਵਾਪਸ ਗਿਣਤੀ_2;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ਸਥਿਰ ਪੂਰਨ read_ਸੰਖਿਆ (lua_State *L, FILE *f) {
  RN rn;
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਪੂਰਨ hex = 0;
  ਅੱਖਰ decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  ਕਰੋ { rn.c = l_getc(rn.f); } ਜਦੋਂ (ਖਾਲੀ_ਥਾਂ_ਹੈ(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ਜੇ (test2(&rn, "00")) {
    ਜੇ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ਨਹੀਂਤਾਂ ਗਿਣਤੀ_2 = 1;  /* count initial '0' as a valid digit */
  }
  ਗਿਣਤੀ_2 += readdigits(&rn, hex);  /* integral part */
  ਜੇ (test2(&rn, decp))  /* decimal point? */
    ਗਿਣਤੀ_2 += readdigits(&rn, hex);  /* fractional part */
  ਜੇ (ਗਿਣਤੀ_2 > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ਜੇ (l_likely(lua_stringtonumber(L, rn.buff)))
    ਵਾਪਸ 1;  /* ok, it is a valid number */
  ਨਹੀਂਤਾਂ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ਵਾਪਸ 0;  /* read fails */
  }
}


ਸਥਿਰ ਪੂਰਨ test_eof (lua_State *L, FILE *f) {
  ਪੂਰਨ c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ਵਾਪਸ (c != EOF);
}


ਸਥਿਰ ਪੂਰਨ read_ਸਤਰ (lua_State *L, FILE *f, ਪੂਰਨ chop) {
  lua_L_ਬਫਰ b;
  ਪੂਰਨ c;
  luaL_buffinit(L, &b);
  ਕਰੋ {  /* may need to read several chunks to get whole line */
    ਅੱਖਰ *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ਜਦੋਂ (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ਜਦੋਂ (c != EOF && c != '\n');  /* repeat until end of line */
  ਜੇ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ਵਾਪਸ (c == '\n' || lua_rawlen(L, -1) > 0);
}


ਸਥਿਰ ਖਾਲੀ read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_ਬਫਰ b;
  luaL_buffinit(L, &b);
  ਕਰੋ {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    ਅੱਖਰ *p = luaL_prepbuffer(&b);
    nr = fread(p, ਆਕਾਰ(ਅੱਖਰ), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ਜਦੋਂ (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ਸਥਿਰ ਪੂਰਨ read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  ਅੱਖਰ *p;
  lua_L_ਬਫਰ b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, ਆਕਾਰ(ਅੱਖਰ), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ਵਾਪਸ (nr > 0);  /* true iff read something */
}


ਸਥਿਰ ਪੂਰਨ g_read (lua_State *L, FILE *f, ਪੂਰਨ ਪਹਿਲਾ) {
  ਪੂਰਨ nargs = lua_gettop(L) - 1;
  ਪੂਰਨ n, success;
  clearerr(f);
  errno = 0;
  ਜੇ (nargs == 0) {  /* no arguments? */
    success = read_ਸਤਰ(L, f, 1);
    n = ਪਹਿਲਾ + 1;  /* to return 1 result */
  }
  ਨਹੀਂਤਾਂ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    ਲਈ (n = ਪਹਿਲਾ; nargs-- && success; n++) {
      ਜੇ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ਨਹੀਂਤਾਂ {
        ਅਚਲ ਅੱਖਰ *p = luaL_checkstring(L, n);
        ਜੇ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        ਚੋਣ (*p) {
          ਹਾਲਤ 'n':  /*ਸੰਖਿਆ*/
            success = read_ਸੰਖਿਆ(L, f);
            ਰੋਕੋ;
          ਹਾਲਤ 'l':  /*ਸਤਰ*/
            success = read_ਸਤਰ(L, f, 1);
            ਰੋਕੋ;
          ਹਾਲਤ 'L':  /* line with end-of-line */
            success = read_ਸਤਰ(L, f, 0);
            ਰੋਕੋ;
          ਹਾਲਤ 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            ਰੋਕੋ;
          ਮੂਲ:
            ਵਾਪਸ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ਜੇ (ferror(f))
    ਵਾਪਸ luaL_fileresult(L, 0, ਨਿਰਾ);
  ਜੇ (!success) {
    lua_ਕੱਢੋ(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ਵਾਪਸ n - ਪਹਿਲਾ;
}


ਸਥਿਰ ਪੂਰਨ io_read (lua_State *L) {
  ਵਾਪਸ g_read(L, getiofile(L, IO_ਨਿਵੇਸ਼), 1);
}


ਸਥਿਰ ਪੂਰਨ f_read (lua_State *L) {
  ਵਾਪਸ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ਸਥਿਰ ਪੂਰਨ io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  ਪੂਰਨ i;
  ਪੂਰਨ n = (ਪੂਰਨ)lua_tointeger(L, lua_upvalueindex(2));
  ਜੇ (isclosed(p))  /* file is already closed? */
    ਵਾਪਸ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  ਲਈ (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ਜੇ (lua_toboolean(L, -n))  /* read at least one value? */
    ਵਾਪਸ n;  /* return them */
  ਨਹੀਂਤਾਂ {  /* first result is false: EOF or error */
    ਜੇ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ਵਾਪਸ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ਜੇ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ਵਾਪਸ 0;
  }
}

/* }====================================================== */


ਸਥਿਰ ਪੂਰਨ g_write (lua_State *L, FILE *f, ਪੂਰਨ arg) {
  ਪੂਰਨ nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  ਲਈ (; nargs--; arg++) {  /* for each argument */
    ਅੱਖਰ buff[LUA_N2SBUFFSZ];
    ਅਚਲ ਅੱਖਰ *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ਜੇ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ਨਹੀਂਤਾਂ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, ਆਕਾਰ(ਅੱਖਰ), len, f);
    totalbytes += numbytes;
    ਜੇ (numbytes < len) {  /* write error? */
      ਪੂਰਨ n = luaL_fileresult(L, 0, ਨਿਰਾ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ਵਾਪਸ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ਵਾਪਸ 1;  /* no errors; file handle already on stack top */
}


ਸਥਿਰ ਪੂਰਨ io_write (lua_State *L) {
  ਵਾਪਸ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ਸਥਿਰ ਪੂਰਨ f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ਵਾਪਸ g_write(L, f, 2);
}


ਸਥਿਰ ਪੂਰਨ f_seek (lua_State *L) {
  ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਬਹੁਲਕ[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ modenames[] = {"set", "cur", "end", ਨਿਰਾ};
  FILE *f = tofile(L);
  ਪੂਰਨ op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum ਵਿੱਥ = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)ਵਿੱਥ == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, ਵਿੱਥ, ਬਹੁਲਕ[op]);
  ਜੇ (l_unlikely(op))
    ਵਾਪਸ luaL_fileresult(L, 0, ਨਿਰਾ);  /* error */
  ਨਹੀਂਤਾਂ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ਵਾਪਸ 1;
  }
}


ਸਥਿਰ ਪੂਰਨ f_setvbuf (lua_State *L) {
  ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਬਹੁਲਕ[] = {_IONBF, _IOFBF, _IOLBF};
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ modenames[] = {"no", "full", "line", ਨਿਰਾ};
  FILE *f = tofile(L);
  ਪੂਰਨ op = luaL_checkoption(L, 2, ਨਿਰਾ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  ਪੂਰਨ res;
  errno = 0;
  res = setvbuf(f, ਨਿਰਾ, ਬਹੁਲਕ[op], (size_t)sz);
  ਵਾਪਸ luaL_fileresult(L, res == 0, ਨਿਰਾ);
}


ਸਥਿਰ ਪੂਰਨ aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ਵਾਪਸ luaL_fileresult(L, fflush(f) == 0, ਨਿਰਾ);
}


ਸਥਿਰ ਪੂਰਨ f_flush (lua_State *L) {
  ਵਾਪਸ aux_flush(L, tofile(L));
}


ਸਥਿਰ ਪੂਰਨ io_flush (lua_State *L) {
  ਵਾਪਸ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ਸਥਿਰ ਅਚਲ luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_ਨਿਵੇਸ਼},
  {"lines", io_ਸਤਰਾਂ},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ਨਿਰਾ, ਨਿਰਾ}
};


/*
** methods for file handles
*/
ਸਥਿਰ ਅਚਲ luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ਸਤਰਾਂ},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ਨਿਰਾ, ਨਿਰਾ}
};


/*
** metamethods for file handles
*/
ਸਥਿਰ ਅਚਲ luaL_Reg metameth[] = {
  {"__index", ਨਿਰਾ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ਨਿਰਾ, ਨਿਰਾ}
};


ਸਥਿਰ ਖਾਲੀ createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_ਕੱਢੋ(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ਸਥਿਰ ਪੂਰਨ io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ਵਾਪਸ 2;
}


ਸਥਿਰ ਖਾਲੀ createstdfile (lua_State *L, FILE *f, ਅਚਲ ਅੱਖਰ *k,
                           ਅਚਲ ਅੱਖਰ *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ਜੇ (k != ਨਿਰਾ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API ਪੂਰਨ luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_ਨਿਵੇਸ਼, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ਨਿਰਾ, "stderr");
  ਵਾਪਸ 1;
}

