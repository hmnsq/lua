/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#எனில் !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#எனில் !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
நிலையான முழு l_checkmode (மாறிலி எழுத்து *mode) {
  திரும்பு (*mode != '\0' && strchr("rwa", *(mode++)) != வெற்று &&
         (*mode != '+' || ((வெறுமை)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#எனில் !defined(l_popen)		/* { */

#எனில் defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(வெற்று), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#அல்லதெனில் defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#எனில் !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#இல்லையெனில்				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((வெறுமை)c, (வெறுமை)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((வெறுமை)L, (வெறுமை)file, -1)

#endif				/* } */

#endif				/* } */


#எனில் !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#எனில் !defined(l_getc)		/* { */

#எனில் defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#இல்லையெனில்
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((வெறுமை)0)
#define l_unlockfile(f)		((வெறுமை)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#எனில் !defined(l_fseek)		/* { */

#எனில் defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#அந்தர்பாவ <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#அல்லதெனில் defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#இல்லையெனில்				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(அளவு(IO_PREFIX)/அளவு(எழுத்து) - 1)
#define IO_உள்ளீடு	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


வகைவரை luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == வெற்று)


நிலையான முழு io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  எனில் (p == வெற்று)
    luaL_pushfail(L);  /* not a file */
  இல்லையெனில் எனில் (isclosed(p))
    lua_pushliteral(L, "closed file");
  இல்லையெனில்
    lua_pushliteral(L, "file");
  திரும்பு 1;
}


நிலையான முழு f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  எனில் (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  இல்லையெனில்
    lua_pushfstring(L, "file (%p)", p->f);
  திரும்பு 1;
}


நிலையான FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  எனில் (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  திரும்பு p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
நிலையான LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, அளவு(LStream), 0);
  p->closef = வெற்று;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  திரும்பு p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
நிலையான முழு aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  நிலையற்ற lua_CFunction cf = p->closef;
  p->closef = வெற்று;  /* mark stream as closed */
  திரும்பு (*cf)(L);  /* close it */
}


நிலையான முழு f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  திரும்பு aux_close(L);
}


நிலையான முழு io_close (lua_State *L) {
  எனில் (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  திரும்பு f_close(L);
}


நிலையான முழு f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  எனில் (!isclosed(p) && p->f != வெற்று)
    aux_close(L);  /* ignore closed and incompletely open files */
  திரும்பு 0;
}


/*
** function to close regular files
*/
நிலையான முழு io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  திரும்பு luaL_fileresult(L, (fclose(p->f) == 0), வெற்று);
}


நிலையான LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = வெற்று;
  p->closef = &io_fclose;
  திரும்பு p;
}


நிலையான வெறுமை opencheck (lua_State *L, மாறிலி எழுத்து *fname, மாறிலி எழுத்து *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  எனில் (l_unlikely(p->f == வெற்று))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


நிலையான முழு io_open (lua_State *L) {
  மாறிலி எழுத்து *filename = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  மாறிலி எழுத்து *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  திரும்பு (p->f == வெற்று) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
நிலையான முழு io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  திரும்பு luaL_execresult(L, l_pclose(L, p->f));
}


நிலையான முழு io_popen (lua_State *L) {
  மாறிலி எழுத்து *filename = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  திரும்பு (p->f == வெற்று) ? luaL_fileresult(L, 0, filename) : 1;
}


நிலையான முழு io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  திரும்பு (p->f == வெற்று) ? luaL_fileresult(L, 0, வெற்று) : 1;
}


நிலையான FILE *getiofile (lua_State *L, மாறிலி எழுத்து *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  எனில் (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  திரும்பு p->f;
}


நிலையான முழு g_iofile (lua_State *L, மாறிலி எழுத்து *f, மாறிலி எழுத்து *mode) {
  எனில் (!lua_isnoneornil(L, 1)) {
    மாறிலி எழுத்து *filename = lua_tostring(L, 1);
    எனில் (filename)
      opencheck(L, filename, mode);
    இல்லையெனில் {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  திரும்பு 1;
}


நிலையான முழு io_உள்ளீடு (lua_State *L) {
  திரும்பு g_iofile(L, IO_உள்ளீடு, "r");
}


நிலையான முழு io_output (lua_State *L) {
  திரும்பு g_iofile(L, IO_OUTPUT, "w");
}


நிலையான முழு io_readline (lua_State *L);


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
நிலையான வெறுமை aux_வரிகள் (lua_State *L, முழு toclose) {
  முழு n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


நிலையான முழு f_வரிகள் (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_வரிகள்(L, 0);
  திரும்பு 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
நிலையான முழு io_வரிகள் (lua_State *L) {
  முழு toclose;
  எனில் (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  எனில் (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_உள்ளீடு);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  இல்லையெனில் {  /* open a new file */
    மாறிலி எழுத்து *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_வரிகள்(L, toclose);  /* push iteration function */
  எனில் (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    திரும்பு 4;
  }
  இல்லையெனில்
    திரும்பு 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#எனில் !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
வகைவரை கட்டமைப்பு {
  FILE *f;  /* file being read */
  முழு c;  /* current character (look ahead) */
  முழு n;  /* number of elements in buffer 'buff' */
  எழுத்து buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
நிலையான முழு nextc (RN *rn) {
  எனில் (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    திரும்பு 0;  /* fail */
  }
  இல்லையெனில் {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    திரும்பு 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
நிலையான முழு test2 (RN *rn, மாறிலி எழுத்து *set) {
  எனில் (rn->c == set[0] || rn->c == set[1])
    திரும்பு nextc(rn);
  இல்லையெனில் திரும்பு 0;
}


/*
** Read a sequence of (hex)digits
*/
நிலையான முழு readdigits (RN *rn, முழு hex) {
  முழு மொத்தம் = 0;
  போது ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    மொத்தம்++;
  திரும்பு மொத்தம்;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
நிலையான முழு read_எண் (lua_State *L, FILE *f) {
  RN rn;
  முழு மொத்தம் = 0;
  முழு hex = 0;
  எழுத்து decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  செய் { rn.c = l_getc(rn.f); } போது (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  எனில் (test2(&rn, "00")) {
    எனில் (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    இல்லையெனில் மொத்தம் = 1;  /* count initial '0' as a valid digit */
  }
  மொத்தம் += readdigits(&rn, hex);  /* integral part */
  எனில் (test2(&rn, decp))  /* decimal point? */
    மொத்தம் += readdigits(&rn, hex);  /* fractional part */
  எனில் (மொத்தம் > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  எனில் (l_likely(lua_stringtonumber(L, rn.buff)))
    திரும்பு 1;  /* ok, it is a valid number */
  இல்லையெனில் {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   திரும்பு 0;  /* read fails */
  }
}


நிலையான முழு test_eof (lua_State *L, FILE *f) {
  முழு c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  திரும்பு (c != EOF);
}


நிலையான முழு read_வரி (lua_State *L, FILE *f, முழு chop) {
  luaL_Buffer b;
  முழு c;
  luaL_buffinit(L, &b);
  செய் {  /* may need to read several chunks to get whole line */
    எழுத்து *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    போது (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } போது (c != EOF && c != '\n');  /* repeat until end of line */
  எனில் (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  திரும்பு (c == '\n' || lua_rawlen(L, -1) > 0);
}


நிலையான வெறுமை read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  செய் {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    எழுத்து *p = luaL_prepbuffer(&b);
    nr = fread(p, அளவு(எழுத்து), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } போது (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


நிலையான முழு read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  எழுத்து *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, அளவு(எழுத்து), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  திரும்பு (nr > 0);  /* true iff read something */
}


நிலையான முழு g_read (lua_State *L, FILE *f, முழு first) {
  முழு nargs = lua_gettop(L) - 1;
  முழு n, success;
  clearerr(f);
  errno = 0;
  எனில் (nargs == 0) {  /* no arguments? */
    success = read_வரி(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  இல்லையெனில் {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    க்கு (n = first; nargs-- && success; n++) {
      எனில் (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      இல்லையெனில் {
        மாறிலி எழுத்து *p = luaL_checkstring(L, n);
        எனில் (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        தேர்வு (*p) {
          நிலை 'n':  /*எண்*/
            success = read_எண்(L, f);
            நிறுத்து;
          நிலை 'l':  /*வரி*/
            success = read_வரி(L, f, 1);
            நிறுத்து;
          நிலை 'L':  /* line with end-of-line */
            success = read_வரி(L, f, 0);
            நிறுத்து;
          நிலை 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            நிறுத்து;
          இயல்பு:
            திரும்பு luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  எனில் (ferror(f))
    திரும்பு luaL_fileresult(L, 0, வெற்று);
  எனில் (!success) {
    lua_எடு(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  திரும்பு n - first;
}


நிலையான முழு io_read (lua_State *L) {
  திரும்பு g_read(L, getiofile(L, IO_உள்ளீடு), 1);
}


நிலையான முழு f_read (lua_State *L) {
  திரும்பு g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
நிலையான முழு io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  முழு i;
  முழு n = (முழு)lua_tointeger(L, lua_upvalueindex(2));
  எனில் (isclosed(p))  /* file is already closed? */
    திரும்பு luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  க்கு (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  எனில் (lua_toboolean(L, -n))  /* read at least one value? */
    திரும்பு n;  /* return them */
  இல்லையெனில் {  /* first result is false: EOF or error */
    எனில் (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      திரும்பு luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    எனில் (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    திரும்பு 0;
  }
}

/* }====================================================== */


நிலையான முழு g_write (lua_State *L, FILE *f, முழு arg) {
  முழு nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  க்கு (; nargs--; arg++) {  /* for each argument */
    எழுத்து buff[LUA_N2SBUFFSZ];
    மாறிலி எழுத்து *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    எனில் (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    இல்லையெனில்  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, அளவு(எழுத்து), len, f);
    totalbytes += numbytes;
    எனில் (numbytes < len) {  /* write error? */
      முழு n = luaL_fileresult(L, 0, வெற்று);
      lua_pushinteger(L, cast_st2S(totalbytes));
      திரும்பு n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  திரும்பு 1;  /* no errors; file handle already on stack top */
}


நிலையான முழு io_write (lua_State *L) {
  திரும்பு g_write(L, getiofile(L, IO_OUTPUT), 1);
}


நிலையான முழு f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  திரும்பு g_write(L, f, 2);
}


நிலையான முழு f_seek (lua_State *L) {
  நிலையான மாறிலி முழு mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  நிலையான மாறிலி எழுத்து *மாறிலி modenames[] = {"set", "cur", "end", வெற்று};
  FILE *f = tofile(L);
  முழு op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  எனில் (l_unlikely(op))
    திரும்பு luaL_fileresult(L, 0, வெற்று);  /* error */
  இல்லையெனில் {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    திரும்பு 1;
  }
}


நிலையான முழு f_setvbuf (lua_State *L) {
  நிலையான மாறிலி முழு mode[] = {_IONBF, _IOFBF, _IOLBF};
  நிலையான மாறிலி எழுத்து *மாறிலி modenames[] = {"no", "full", "line", வெற்று};
  FILE *f = tofile(L);
  முழு op = luaL_checkoption(L, 2, வெற்று, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  முழு res;
  errno = 0;
  res = setvbuf(f, வெற்று, mode[op], (size_t)sz);
  திரும்பு luaL_fileresult(L, res == 0, வெற்று);
}


நிலையான முழு aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  திரும்பு luaL_fileresult(L, fflush(f) == 0, வெற்று);
}


நிலையான முழு f_flush (lua_State *L) {
  திரும்பு aux_flush(L, tofile(L));
}


நிலையான முழு io_flush (lua_State *L) {
  திரும்பு aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
நிலையான மாறிலி luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_உள்ளீடு},
  {"lines", io_வரிகள்},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {வெற்று, வெற்று}
};


/*
** methods for file handles
*/
நிலையான மாறிலி luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_வரிகள்},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {வெற்று, வெற்று}
};


/*
** metamethods for file handles
*/
நிலையான மாறிலி luaL_Reg metameth[] = {
  {"__index", வெற்று},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {வெற்று, வெற்று}
};


நிலையான வெறுமை createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_எடு(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
நிலையான முழு io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  திரும்பு 2;
}


நிலையான வெறுமை createstdfile (lua_State *L, FILE *f, மாறிலி எழுத்து *k,
                           மாறிலி எழுத்து *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  எனில் (k != வெற்று) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API முழு luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_உள்ளீடு, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, வெற்று, "stderr");
  திரும்பு 1;
}

