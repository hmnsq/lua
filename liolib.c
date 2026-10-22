/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#যদি !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#যদি !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
স্থির পূর্ণসংখ্যা l_checkmode (ধ্রুবক অক্ষর *mode) {
  ফেরত (*mode != '\0' && strchr("rwa", *(mode++)) != ফাঁকা &&
         (*mode != '+' || ((শূন্য)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#যদি !defined(l_popen)		/* { */

#যদি defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(ফাঁকা), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#নতুবা_যদি defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#যদি !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#অন্যথা				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((শূন্য)c, (শূন্য)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((শূন্য)L, (শূন্য)file, -1)

#endif				/* } */

#endif				/* } */


#যদি !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#যদি !defined(l_getc)		/* { */

#যদি defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#অন্যথা
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((শূন্য)0)
#define l_unlockfile(f)		((শূন্য)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#যদি !defined(l_fseek)		/* { */

#যদি defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#অন্তর্ভাব <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#নতুবা_যদি defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#অন্যথা				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(আকার(IO_PREFIX)/আকার(অক্ষর) - 1)
#define IO_প্রবেশ	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


প্রকারলেখ luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == ফাঁকা)


স্থির পূর্ণসংখ্যা io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  যদি (p == ফাঁকা)
    luaL_pushfail(L);  /* not a file */
  অন্যথা যদি (isclosed(p))
    lua_pushliteral(L, "closed file");
  অন্যথা
    lua_pushliteral(L, "file");
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  যদি (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  অন্যথা
    lua_pushfstring(L, "file (%p)", p->f);
  ফেরত 1;
}


স্থির FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  যদি (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ফেরত p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
স্থির LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, আকার(LStream), 0);
  p->closef = ফাঁকা;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ফেরত p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
স্থির পূর্ণসংখ্যা aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  অস্থির lua_CFunction cf = p->closef;
  p->closef = ফাঁকা;  /* mark stream as closed */
  ফেরত (*cf)(L);  /* close it */
}


স্থির পূর্ণসংখ্যা f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ফেরত aux_close(L);
}


স্থির পূর্ণসংখ্যা io_close (lua_State *L) {
  যদি (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ফেরত f_close(L);
}


স্থির পূর্ণসংখ্যা f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  যদি (!isclosed(p) && p->f != ফাঁকা)
    aux_close(L);  /* ignore closed and incompletely open files */
  ফেরত 0;
}


/*
** function to close regular files
*/
স্থির পূর্ণসংখ্যা io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ফেরত luaL_fileresult(L, (fclose(p->f) == 0), ফাঁকা);
}


স্থির LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ফাঁকা;
  p->closef = &io_fclose;
  ফেরত p;
}


স্থির শূন্য opencheck (lua_State *L, ধ্রুবক অক্ষর *fname, ধ্রুবক অক্ষর *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  যদি (l_unlikely(p->f == ফাঁকা))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


স্থির পূর্ণসংখ্যা io_open (lua_State *L) {
  ধ্রুবক অক্ষর *filename = luaL_checkstring(L, 1);
  ধ্রুবক অক্ষর *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ধ্রুবক অক্ষর *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  ফেরত (p->f == ফাঁকা) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
স্থির পূর্ণসংখ্যা io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ফেরত luaL_execresult(L, l_pclose(L, p->f));
}


স্থির পূর্ণসংখ্যা io_popen (lua_State *L) {
  ধ্রুবক অক্ষর *filename = luaL_checkstring(L, 1);
  ধ্রুবক অক্ষর *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  ফেরত (p->f == ফাঁকা) ? luaL_fileresult(L, 0, filename) : 1;
}


স্থির পূর্ণসংখ্যা io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ফেরত (p->f == ফাঁকা) ? luaL_fileresult(L, 0, ফাঁকা) : 1;
}


স্থির FILE *getiofile (lua_State *L, ধ্রুবক অক্ষর *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  যদি (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ফেরত p->f;
}


স্থির পূর্ণসংখ্যা g_iofile (lua_State *L, ধ্রুবক অক্ষর *f, ধ্রুবক অক্ষর *mode) {
  যদি (!lua_isnoneornil(L, 1)) {
    ধ্রুবক অক্ষর *filename = lua_tostring(L, 1);
    যদি (filename)
      opencheck(L, filename, mode);
    অন্যথা {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা io_প্রবেশ (lua_State *L) {
  ফেরত g_iofile(L, IO_প্রবেশ, "r");
}


স্থির পূর্ণসংখ্যা io_output (lua_State *L) {
  ফেরত g_iofile(L, IO_OUTPUT, "w");
}


স্থির পূর্ণসংখ্যা io_readline (lua_State *L);


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
স্থির শূন্য aux_সারিগুলি (lua_State *L, পূর্ণসংখ্যা toclose) {
  পূর্ণসংখ্যা n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


স্থির পূর্ণসংখ্যা f_সারিগুলি (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_সারিগুলি(L, 0);
  ফেরত 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
স্থির পূর্ণসংখ্যা io_সারিগুলি (lua_State *L) {
  পূর্ণসংখ্যা toclose;
  যদি (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  যদি (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_প্রবেশ);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  অন্যথা {  /* open a new file */
    ধ্রুবক অক্ষর *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_সারিগুলি(L, toclose);  /* push iteration function */
  যদি (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ফেরত 4;
  }
  অন্যথা
    ফেরত 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#যদি !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
প্রকারলেখ কাঠামো {
  FILE *f;  /* file being read */
  পূর্ণসংখ্যা c;  /* current character (look ahead) */
  পূর্ণসংখ্যা n;  /* number of elements in buffer 'buff' */
  অক্ষর buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
স্থির পূর্ণসংখ্যা nextc (RN *rn) {
  যদি (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ফেরত 0;  /* fail */
  }
  অন্যথা {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ফেরত 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
স্থির পূর্ণসংখ্যা test2 (RN *rn, ধ্রুবক অক্ষর *set) {
  যদি (rn->c == set[0] || rn->c == set[1])
    ফেরত nextc(rn);
  অন্যথা ফেরত 0;
}


/*
** Read a sequence of (hex)digits
*/
স্থির পূর্ণসংখ্যা readdigits (RN *rn, পূর্ণসংখ্যা hex) {
  পূর্ণসংখ্যা গুনতি = 0;
  যখন ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    গুনতি++;
  ফেরত গুনতি;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
স্থির পূর্ণসংখ্যা read_সংখ্যা (lua_State *L, FILE *f) {
  RN rn;
  পূর্ণসংখ্যা গুনতি = 0;
  পূর্ণসংখ্যা hex = 0;
  অক্ষর decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  করো { rn.c = l_getc(rn.f); } যখন (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  যদি (test2(&rn, "00")) {
    যদি (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    অন্যথা গুনতি = 1;  /* count initial '0' as a valid digit */
  }
  গুনতি += readdigits(&rn, hex);  /* integral part */
  যদি (test2(&rn, decp))  /* decimal point? */
    গুনতি += readdigits(&rn, hex);  /* fractional part */
  যদি (গুনতি > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  যদি (l_likely(lua_stringtonumber(L, rn.buff)))
    ফেরত 1;  /* ok, it is a valid number */
  অন্যথা {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ফেরত 0;  /* read fails */
  }
}


স্থির পূর্ণসংখ্যা test_eof (lua_State *L, FILE *f) {
  পূর্ণসংখ্যা c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ফেরত (c != EOF);
}


স্থির পূর্ণসংখ্যা read_সারি (lua_State *L, FILE *f, পূর্ণসংখ্যা chop) {
  lua_L_বাফার b;
  পূর্ণসংখ্যা c;
  luaL_buffinit(L, &b);
  করো {  /* may need to read several chunks to get whole line */
    অক্ষর *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    যখন (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } যখন (c != EOF && c != '\n');  /* repeat until end of line */
  যদি (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ফেরত (c == '\n' || lua_rawlen(L, -1) > 0);
}


স্থির শূন্য read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_বাফার b;
  luaL_buffinit(L, &b);
  করো {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    অক্ষর *p = luaL_prepbuffer(&b);
    nr = fread(p, আকার(অক্ষর), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } যখন (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


স্থির পূর্ণসংখ্যা read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  অক্ষর *p;
  lua_L_বাফার b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, আকার(অক্ষর), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ফেরত (nr > 0);  /* true iff read something */
}


স্থির পূর্ণসংখ্যা g_read (lua_State *L, FILE *f, পূর্ণসংখ্যা first) {
  পূর্ণসংখ্যা nargs = lua_gettop(L) - 1;
  পূর্ণসংখ্যা n, success;
  clearerr(f);
  errno = 0;
  যদি (nargs == 0) {  /* no arguments? */
    success = read_সারি(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  অন্যথা {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    জন্য (n = first; nargs-- && success; n++) {
      যদি (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      অন্যথা {
        ধ্রুবক অক্ষর *p = luaL_checkstring(L, n);
        যদি (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        নির্বাচন (*p) {
          ক্ষেত্র 'n':  /*সংখ্যা*/
            success = read_সংখ্যা(L, f);
            বিরতি;
          ক্ষেত্র 'l':  /*সারি*/
            success = read_সারি(L, f, 1);
            বিরতি;
          ক্ষেত্র 'L':  /* line with end-of-line */
            success = read_সারি(L, f, 0);
            বিরতি;
          ক্ষেত্র 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            বিরতি;
          পূর্বনির্ধারিত:
            ফেরত luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  যদি (ferror(f))
    ফেরত luaL_fileresult(L, 0, ফাঁকা);
  যদি (!success) {
    lua_বের_করো(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ফেরত n - first;
}


স্থির পূর্ণসংখ্যা io_read (lua_State *L) {
  ফেরত g_read(L, getiofile(L, IO_প্রবেশ), 1);
}


স্থির পূর্ণসংখ্যা f_read (lua_State *L) {
  ফেরত g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
স্থির পূর্ণসংখ্যা io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = (পূর্ণসংখ্যা)lua_tointeger(L, lua_upvalueindex(2));
  যদি (isclosed(p))  /* file is already closed? */
    ফেরত luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  জন্য (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  যদি (lua_toboolean(L, -n))  /* read at least one value? */
    ফেরত n;  /* return them */
  অন্যথা {  /* first result is false: EOF or error */
    যদি (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ফেরত luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    যদি (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ফেরত 0;
  }
}

/* }====================================================== */


স্থির পূর্ণসংখ্যা g_write (lua_State *L, FILE *f, পূর্ণসংখ্যা arg) {
  পূর্ণসংখ্যা nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  জন্য (; nargs--; arg++) {  /* for each argument */
    অক্ষর buff[LUA_N2SBUFFSZ];
    ধ্রুবক অক্ষর *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    যদি (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    অন্যথা  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, আকার(অক্ষর), len, f);
    totalbytes += numbytes;
    যদি (numbytes < len) {  /* write error? */
      পূর্ণসংখ্যা n = luaL_fileresult(L, 0, ফাঁকা);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ফেরত n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ফেরত 1;  /* no errors; file handle already on stack top */
}


স্থির পূর্ণসংখ্যা io_write (lua_State *L) {
  ফেরত g_write(L, getiofile(L, IO_OUTPUT), 1);
}


স্থির পূর্ণসংখ্যা f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ফেরত g_write(L, f, 2);
}


স্থির পূর্ণসংখ্যা f_seek (lua_State *L) {
  স্থির ধ্রুবক পূর্ণসংখ্যা mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  স্থির ধ্রুবক অক্ষর *ধ্রুবক modenames[] = {"set", "cur", "end", ফাঁকা};
  FILE *f = tofile(L);
  পূর্ণসংখ্যা op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  যদি (l_unlikely(op))
    ফেরত luaL_fileresult(L, 0, ফাঁকা);  /* error */
  অন্যথা {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ফেরত 1;
  }
}


স্থির পূর্ণসংখ্যা f_setvbuf (lua_State *L) {
  স্থির ধ্রুবক পূর্ণসংখ্যা mode[] = {_IONBF, _IOFBF, _IOLBF};
  স্থির ধ্রুবক অক্ষর *ধ্রুবক modenames[] = {"no", "full", "line", ফাঁকা};
  FILE *f = tofile(L);
  পূর্ণসংখ্যা op = luaL_checkoption(L, 2, ফাঁকা, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  পূর্ণসংখ্যা res;
  errno = 0;
  res = setvbuf(f, ফাঁকা, mode[op], (size_t)sz);
  ফেরত luaL_fileresult(L, res == 0, ফাঁকা);
}


স্থির পূর্ণসংখ্যা aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ফেরত luaL_fileresult(L, fflush(f) == 0, ফাঁকা);
}


স্থির পূর্ণসংখ্যা f_flush (lua_State *L) {
  ফেরত aux_flush(L, tofile(L));
}


স্থির পূর্ণসংখ্যা io_flush (lua_State *L) {
  ফেরত aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
স্থির ধ্রুবক luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_প্রবেশ},
  {"lines", io_সারিগুলি},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ফাঁকা, ফাঁকা}
};


/*
** methods for file handles
*/
স্থির ধ্রুবক luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_সারিগুলি},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ফাঁকা, ফাঁকা}
};


/*
** metamethods for file handles
*/
স্থির ধ্রুবক luaL_Reg metameth[] = {
  {"__index", ফাঁকা},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ফাঁকা, ফাঁকা}
};


স্থির শূন্য createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_বের_করো(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
স্থির পূর্ণসংখ্যা io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ফেরত 2;
}


স্থির শূন্য createstdfile (lua_State *L, FILE *f, ধ্রুবক অক্ষর *k,
                           ধ্রুবক অক্ষর *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  যদি (k != ফাঁকা) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API পূর্ণসংখ্যা luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_প্রবেশ, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ফাঁকা, "stderr");
  ফেরত 1;
}

