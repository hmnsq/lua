/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <locale.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#إذا !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#إذا !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ساكن صحيح l_checkmode (ثابت حرف *mode) {
  أرجع (*mode != '\0' && strchr("rwa", *(mode++)) != عدم &&
         (*mode != '+' || ((فراغ)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#إذا !defined(l_popen)		/* { */

#إذا defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(عدم), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#وإلاإذا defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#إذا !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#وإلا				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((فراغ)c, (فراغ)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((فراغ)L, (فراغ)file, -1)

#endif				/* } */

#endif				/* } */


#إذا !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#إذا !defined(l_getc)		/* { */

#إذا defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#وإلا
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((فراغ)0)
#define l_unlockfile(f)		((فراغ)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#إذا !defined(l_fseek)		/* { */

#إذا defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#ضمن <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#وإلاإذا defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#وإلا				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(حجم(IO_PREFIX)/حجم(حرف) - 1)
#define IO_شغرو	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


تعريفنوع luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == عدم)


ساكن صحيح io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  إذا (p == عدم)
    luaL_pushfail(L);  /* not a file */
  وإلا إذا (isclosed(p))
    lua_pushliteral(L, "closed file");
  وإلا
    lua_pushliteral(L, "file");
  أرجع 1;
}


ساكن صحيح f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  إذا (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  وإلا
    lua_pushfstring(L, "file (%p)", p->f);
  أرجع 1;
}


ساكن FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  إذا (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  أرجع p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ساكن LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, حجم(LStream), 0);
  p->closef = عدم;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  أرجع p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ساكن صحيح aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  متطاير lua_CFunction cf = p->closef;
  p->closef = عدم;  /* mark stream as closed */
  أرجع (*cf)(L);  /* close it */
}


ساكن صحيح f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  أرجع aux_close(L);
}


ساكن صحيح io_close (lua_State *L) {
  إذا (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  أرجع f_close(L);
}


ساكن صحيح f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  إذا (!isclosed(p) && p->f != عدم)
    aux_close(L);  /* ignore closed and incompletely open files */
  أرجع 0;
}


/*
** function to close regular files
*/
ساكن صحيح io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  أرجع luaL_fileresult(L, (fclose(p->f) == 0), عدم);
}


ساكن LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = عدم;
  p->closef = &io_fclose;
  أرجع p;
}


ساكن فراغ opencheck (lua_State *L, ثابت حرف *fname, ثابت حرف *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  إذا (l_unlikely(p->f == عدم))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ساكن صحيح io_open (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  ثابت حرف *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ثابت حرف *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  أرجع (p->f == عدم) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ساكن صحيح io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  أرجع luaL_execresult(L, l_pclose(L, p->f));
}


ساكن صحيح io_popen (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  ثابت حرف *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  أرجع (p->f == عدم) ? luaL_fileresult(L, 0, filename) : 1;
}


ساكن صحيح io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  أرجع (p->f == عدم) ? luaL_fileresult(L, 0, عدم) : 1;
}


ساكن FILE *getiofile (lua_State *L, ثابت حرف *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  إذا (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  أرجع p->f;
}


ساكن صحيح g_iofile (lua_State *L, ثابت حرف *f, ثابت حرف *mode) {
  إذا (!lua_isnoneornil(L, 1)) {
    ثابت حرف *filename = lua_tostring(L, 1);
    إذا (filename)
      opencheck(L, filename, mode);
    وإلا {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  أرجع 1;
}


ساكن صحيح io_شغرو (lua_State *L) {
  أرجع g_iofile(L, IO_شغرو, "r");
}


ساكن صحيح io_output (lua_State *L) {
  أرجع g_iofile(L, IO_OUTPUT, "w");
}


ساكن صحيح io_readline (lua_State *L);


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
ساكن فراغ aux_ليوك (lua_State *L, صحيح toclose) {
  صحيح n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ساكن صحيح f_ليوك (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ليوك(L, 0);
  أرجع 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ساكن صحيح io_ليوك (lua_State *L) {
  صحيح toclose;
  إذا (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  إذا (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_شغرو);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  وإلا {  /* open a new file */
    ثابت حرف *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ليوك(L, toclose);  /* push iteration function */
  إذا (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    أرجع 4;
  }
  وإلا
    أرجع 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#إذا !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
تعريفنوع بنية {
  FILE *f;  /* file being read */
  صحيح c;  /* current character (look ahead) */
  صحيح n;  /* number of elements in buffer 'buff' */
  حرف buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ساكن صحيح nextc (RN *rn) {
  إذا (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    أرجع 0;  /* fail */
  }
  وإلا {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    أرجع 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ساكن صحيح test2 (RN *rn, ثابت حرف *set) {
  إذا (rn->c == set[0] || rn->c == set[1])
    أرجع nextc(rn);
  وإلا أرجع 0;
}


/*
** Read a sequence of (hex)digits
*/
ساكن صحيح readdigits (RN *rn, صحيح hex) {
  صحيح كدي = 0;
  طالما ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    كدي++;
  أرجع كدي;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ساكن صحيح read_لمب (lua_State *L, FILE *f) {
  RN rn;
  صحيح كدي = 0;
  صحيح hex = 0;
  حرف decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  افعل { rn.c = l_getc(rn.f); } طالما (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  إذا (test2(&rn, "00")) {
    إذا (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    وإلا كدي = 1;  /* count initial '0' as a valid digit */
  }
  كدي += readdigits(&rn, hex);  /* integral part */
  إذا (test2(&rn, decp))  /* decimal point? */
    كدي += readdigits(&rn, hex);  /* fractional part */
  إذا (كدي > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  إذا (l_likely(lua_stringtonumber(L, rn.buff)))
    أرجع 1;  /* ok, it is a valid number */
  وإلا {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   أرجع 0;  /* read fails */
  }
}


ساكن صحيح test_eof (lua_State *L, FILE *f) {
  صحيح c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  أرجع (c != EOF);
}


ساكن صحيح read_لي (lua_State *L, FILE *f, صحيح chop) {
  luaL_Buffer b;
  صحيح c;
  luaL_buffinit(L, &b);
  افعل {  /* may need to read several chunks to get whole line */
    حرف *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    طالما (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } طالما (c != EOF && c != '\n');  /* repeat until end of line */
  إذا (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  أرجع (c == '\n' || lua_rawlen(L, -1) > 0);
}


ساكن فراغ read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  افعل {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    حرف *p = luaL_prepbuffer(&b);
    nr = fread(p, حجم(حرف), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } طالما (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ساكن صحيح read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  حرف *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, حجم(حرف), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  أرجع (nr > 0);  /* true iff read something */
}


ساكن صحيح g_read (lua_State *L, FILE *f, صحيح first) {
  صحيح nargs = lua_gettop(L) - 1;
  صحيح n, success;
  clearerr(f);
  errno = 0;
  إذا (nargs == 0) {  /* no arguments? */
    success = read_لي(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  وإلا {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    لكل (n = first; nargs-- && success; n++) {
      إذا (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      وإلا {
        ثابت حرف *p = luaL_checkstring(L, n);
        إذا (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        بدل (*p) {
          حالة 'n':  /*لمب*/
            success = read_لمب(L, f);
            اكسر;
          حالة 'l':  /*لي*/
            success = read_لي(L, f, 1);
            اكسر;
          حالة 'L':  /* line with end-of-line */
            success = read_لي(L, f, 0);
            اكسر;
          حالة 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            اكسر;
          افتراضي:
            أرجع luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  إذا (ferror(f))
    أرجع luaL_fileresult(L, 0, عدم);
  إذا (!success) {
    lua_چرو(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  أرجع n - first;
}


ساكن صحيح io_read (lua_State *L) {
  أرجع g_read(L, getiofile(L, IO_شغرو), 1);
}


ساكن صحيح f_read (lua_State *L) {
  أرجع g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ساكن صحيح io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  صحيح i;
  صحيح n = (صحيح)lua_tointeger(L, lua_upvalueindex(2));
  إذا (isclosed(p))  /* file is already closed? */
    أرجع luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  لكل (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  إذا (lua_toboolean(L, -n))  /* read at least one value? */
    أرجع n;  /* return them */
  وإلا {  /* first result is false: EOF or error */
    إذا (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      أرجع luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    إذا (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    أرجع 0;
  }
}

/* }====================================================== */


ساكن صحيح g_write (lua_State *L, FILE *f, صحيح arg) {
  صحيح nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  لكل (; nargs--; arg++) {  /* for each argument */
    حرف buff[LUA_N2SBUFFSZ];
    ثابت حرف *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    إذا (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    وإلا  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, حجم(حرف), len, f);
    totalbytes += numbytes;
    إذا (numbytes < len) {  /* write error? */
      صحيح n = luaL_fileresult(L, 0, عدم);
      lua_pushinteger(L, cast_st2S(totalbytes));
      أرجع n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  أرجع 1;  /* no errors; file handle already on stack top */
}


ساكن صحيح io_write (lua_State *L) {
  أرجع g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ساكن صحيح f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  أرجع g_write(L, f, 2);
}


ساكن صحيح f_seek (lua_State *L) {
  ساكن ثابت صحيح mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ساكن ثابت حرف *ثابت modenames[] = {"set", "cur", "end", عدم};
  FILE *f = tofile(L);
  صحيح op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  إذا (l_unlikely(op))
    أرجع luaL_fileresult(L, 0, عدم);  /* error */
  وإلا {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    أرجع 1;
  }
}


ساكن صحيح f_setvbuf (lua_State *L) {
  ساكن ثابت صحيح mode[] = {_IONBF, _IOFBF, _IOLBF};
  ساكن ثابت حرف *ثابت modenames[] = {"no", "full", "line", عدم};
  FILE *f = tofile(L);
  صحيح op = luaL_checkoption(L, 2, عدم, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  صحيح res;
  errno = 0;
  res = setvbuf(f, عدم, mode[op], (size_t)sz);
  أرجع luaL_fileresult(L, res == 0, عدم);
}


ساكن صحيح aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  أرجع luaL_fileresult(L, fflush(f) == 0, عدم);
}


ساكن صحيح f_flush (lua_State *L) {
  أرجع aux_flush(L, tofile(L));
}


ساكن صحيح io_flush (lua_State *L) {
  أرجع aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ساكن ثابت luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_شغرو},
  {"lines", io_ليوك},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {عدم, عدم}
};


/*
** methods for file handles
*/
ساكن ثابت luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ليوك},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {عدم, عدم}
};


/*
** metamethods for file handles
*/
ساكن ثابت luaL_Reg metameth[] = {
  {"__index", عدم},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {عدم, عدم}
};


ساكن فراغ createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_چرو(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ساكن صحيح io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  أرجع 2;
}


ساكن فراغ createstdfile (lua_State *L, FILE *f, ثابت حرف *k,
                           ثابت حرف *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  إذا (k != عدم) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API صحيح luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_شغرو, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, عدم, "stderr");
  أرجع 1;
}

