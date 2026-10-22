/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <ctype.h>
#شامل <errno.h>
#شامل <locale.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#اگر !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#اگر !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ایستا صحیح l_checkmode (ثابت نویسه *mode) {
  بازگشت (*mode != '\0' && strchr("rwa", *(mode++)) != پوچ &&
         (*mode != '+' || ((تهی)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#اگر !defined(l_popen)		/* { */

#اگر defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(پوچ), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#وگرنهاگر defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#اگر !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#وگرنه				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((تهی)c, (تهی)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((تهی)L, (تهی)file, -1)

#endif				/* } */

#endif				/* } */


#اگر !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#اگر !defined(l_getc)		/* { */

#اگر defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#وگرنه
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((تهی)0)
#define l_unlockfile(f)		((تهی)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#اگر !defined(l_fseek)		/* { */

#اگر defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#شامل <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#وگرنهاگر defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#وگرنه				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(اندازه(IO_PREFIX)/اندازه(نویسه) - 1)
#define IO_درون_داد	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


تعریفنوع luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == پوچ)


ایستا صحیح io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  اگر (p == پوچ)
    luaL_pushfail(L);  /* not a file */
  وگرنه اگر (isclosed(p))
    lua_pushliteral(L, "closed file");
  وگرنه
    lua_pushliteral(L, "file");
  بازگشت 1;
}


ایستا صحیح f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  وگرنه
    lua_pushfstring(L, "file (%p)", p->f);
  بازگشت 1;
}


ایستا FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  بازگشت p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ایستا LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, اندازه(LStream), 0);
  p->closef = پوچ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  بازگشت p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ایستا صحیح aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  فرار lua_CFunction cf = p->closef;
  p->closef = پوچ;  /* mark stream as closed */
  بازگشت (*cf)(L);  /* close it */
}


ایستا صحیح f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  بازگشت aux_close(L);
}


ایستا صحیح io_close (lua_State *L) {
  اگر (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  بازگشت f_close(L);
}


ایستا صحیح f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (!isclosed(p) && p->f != پوچ)
    aux_close(L);  /* ignore closed and incompletely open files */
  بازگشت 0;
}


/*
** function to close regular files
*/
ایستا صحیح io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  بازگشت luaL_fileresult(L, (fclose(p->f) == 0), پوچ);
}


ایستا LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = پوچ;
  p->closef = &io_fclose;
  بازگشت p;
}


ایستا تهی opencheck (lua_State *L, ثابت نویسه *fname, ثابت نویسه *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  اگر (l_unlikely(p->f == پوچ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ایستا صحیح io_open (lua_State *L) {
  ثابت نویسه *filename = luaL_checkstring(L, 1);
  ثابت نویسه *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ثابت نویسه *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  بازگشت (p->f == پوچ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ایستا صحیح io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  بازگشت luaL_execresult(L, l_pclose(L, p->f));
}


ایستا صحیح io_popen (lua_State *L) {
  ثابت نویسه *filename = luaL_checkstring(L, 1);
  ثابت نویسه *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  بازگشت (p->f == پوچ) ? luaL_fileresult(L, 0, filename) : 1;
}


ایستا صحیح io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  بازگشت (p->f == پوچ) ? luaL_fileresult(L, 0, پوچ) : 1;
}


ایستا FILE *getiofile (lua_State *L, ثابت نویسه *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  اگر (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  بازگشت p->f;
}


ایستا صحیح g_iofile (lua_State *L, ثابت نویسه *f, ثابت نویسه *mode) {
  اگر (!lua_isnoneornil(L, 1)) {
    ثابت نویسه *filename = lua_tostring(L, 1);
    اگر (filename)
      opencheck(L, filename, mode);
    وگرنه {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  بازگشت 1;
}


ایستا صحیح io_درون_داد (lua_State *L) {
  بازگشت g_iofile(L, IO_درون_داد, "r");
}


ایستا صحیح io_output (lua_State *L) {
  بازگشت g_iofile(L, IO_OUTPUT, "w");
}


ایستا صحیح io_readline (lua_State *L);


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
ایستا تهی aux_سطرها (lua_State *L, صحیح toclose) {
  صحیح n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ایستا صحیح f_سطرها (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_سطرها(L, 0);
  بازگشت 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ایستا صحیح io_سطرها (lua_State *L) {
  صحیح toclose;
  اگر (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  اگر (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_درون_داد);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  وگرنه {  /* open a new file */
    ثابت نویسه *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_سطرها(L, toclose);  /* push iteration function */
  اگر (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    بازگشت 4;
  }
  وگرنه
    بازگشت 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#اگر !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
تعریفنوع ساختار {
  FILE *f;  /* file being read */
  صحیح c;  /* current character (look ahead) */
  صحیح n;  /* number of elements in buffer 'buff' */
  نویسه buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ایستا صحیح nextc (RN *rn) {
  اگر (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    بازگشت 0;  /* fail */
  }
  وگرنه {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    بازگشت 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ایستا صحیح test2 (RN *rn, ثابت نویسه *set) {
  اگر (rn->c == set[0] || rn->c == set[1])
    بازگشت nextc(rn);
  وگرنه بازگشت 0;
}


/*
** Read a sequence of (hex)digits
*/
ایستا صحیح readdigits (RN *rn, صحیح hex) {
  صحیح شمار = 0;
  وقتی ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    شمار++;
  بازگشت شمار;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ایستا صحیح read_عدد (lua_State *L, FILE *f) {
  RN rn;
  صحیح شمار = 0;
  صحیح hex = 0;
  نویسه decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  انجامبده { rn.c = l_getc(rn.f); } وقتی (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  اگر (test2(&rn, "00")) {
    اگر (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    وگرنه شمار = 1;  /* count initial '0' as a valid digit */
  }
  شمار += readdigits(&rn, hex);  /* integral part */
  اگر (test2(&rn, decp))  /* decimal point? */
    شمار += readdigits(&rn, hex);  /* fractional part */
  اگر (شمار > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  اگر (l_likely(lua_stringtonumber(L, rn.buff)))
    بازگشت 1;  /* ok, it is a valid number */
  وگرنه {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   بازگشت 0;  /* read fails */
  }
}


ایستا صحیح test_eof (lua_State *L, FILE *f) {
  صحیح c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  بازگشت (c != EOF);
}


ایستا صحیح read_سطر (lua_State *L, FILE *f, صحیح chop) {
  luaL_Buffer b;
  صحیح c;
  luaL_buffinit(L, &b);
  انجامبده {  /* may need to read several chunks to get whole line */
    نویسه *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    وقتی (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } وقتی (c != EOF && c != '\n');  /* repeat until end of line */
  اگر (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  بازگشت (c == '\n' || lua_rawlen(L, -1) > 0);
}


ایستا تهی read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  انجامبده {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    نویسه *p = luaL_prepbuffer(&b);
    nr = fread(p, اندازه(نویسه), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } وقتی (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ایستا صحیح read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  نویسه *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, اندازه(نویسه), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  بازگشت (nr > 0);  /* true iff read something */
}


ایستا صحیح g_read (lua_State *L, FILE *f, صحیح first) {
  صحیح nargs = lua_gettop(L) - 1;
  صحیح n, success;
  clearerr(f);
  errno = 0;
  اگر (nargs == 0) {  /* no arguments? */
    success = read_سطر(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  وگرنه {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    برای (n = first; nargs-- && success; n++) {
      اگر (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      وگرنه {
        ثابت نویسه *p = luaL_checkstring(L, n);
        اگر (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        انتخاب (*p) {
          حالت 'n':  /*عدد*/
            success = read_عدد(L, f);
            بشکن;
          حالت 'l':  /*سطر*/
            success = read_سطر(L, f, 1);
            بشکن;
          حالت 'L':  /* line with end-of-line */
            success = read_سطر(L, f, 0);
            بشکن;
          حالت 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            بشکن;
          پیشفرض:
            بازگشت luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  اگر (ferror(f))
    بازگشت luaL_fileresult(L, 0, پوچ);
  اگر (!success) {
    lua_برداشت(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  بازگشت n - first;
}


ایستا صحیح io_read (lua_State *L) {
  بازگشت g_read(L, getiofile(L, IO_درون_داد), 1);
}


ایستا صحیح f_read (lua_State *L) {
  بازگشت g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ایستا صحیح io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  صحیح i;
  صحیح n = (صحیح)lua_tointeger(L, lua_upvalueindex(2));
  اگر (isclosed(p))  /* file is already closed? */
    بازگشت luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  برای (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  اگر (lua_toboolean(L, -n))  /* read at least one value? */
    بازگشت n;  /* return them */
  وگرنه {  /* first result is false: EOF or error */
    اگر (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      بازگشت luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    اگر (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    بازگشت 0;
  }
}

/* }====================================================== */


ایستا صحیح g_write (lua_State *L, FILE *f, صحیح arg) {
  صحیح nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  برای (; nargs--; arg++) {  /* for each argument */
    نویسه buff[LUA_N2SBUFFSZ];
    ثابت نویسه *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    اگر (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    وگرنه  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, اندازه(نویسه), len, f);
    totalbytes += numbytes;
    اگر (numbytes < len) {  /* write error? */
      صحیح n = luaL_fileresult(L, 0, پوچ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      بازگشت n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  بازگشت 1;  /* no errors; file handle already on stack top */
}


ایستا صحیح io_write (lua_State *L) {
  بازگشت g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ایستا صحیح f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  بازگشت g_write(L, f, 2);
}


ایستا صحیح f_seek (lua_State *L) {
  ایستا ثابت صحیح mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ایستا ثابت نویسه *ثابت modenames[] = {"set", "cur", "end", پوچ};
  FILE *f = tofile(L);
  صحیح op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  اگر (l_unlikely(op))
    بازگشت luaL_fileresult(L, 0, پوچ);  /* error */
  وگرنه {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    بازگشت 1;
  }
}


ایستا صحیح f_setvbuf (lua_State *L) {
  ایستا ثابت صحیح mode[] = {_IONBF, _IOFBF, _IOLBF};
  ایستا ثابت نویسه *ثابت modenames[] = {"no", "full", "line", پوچ};
  FILE *f = tofile(L);
  صحیح op = luaL_checkoption(L, 2, پوچ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  صحیح res;
  errno = 0;
  res = setvbuf(f, پوچ, mode[op], (size_t)sz);
  بازگشت luaL_fileresult(L, res == 0, پوچ);
}


ایستا صحیح aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  بازگشت luaL_fileresult(L, fflush(f) == 0, پوچ);
}


ایستا صحیح f_flush (lua_State *L) {
  بازگشت aux_flush(L, tofile(L));
}


ایستا صحیح io_flush (lua_State *L) {
  بازگشت aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ایستا ثابت luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_درون_داد},
  {"lines", io_سطرها},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {پوچ, پوچ}
};


/*
** methods for file handles
*/
ایستا ثابت luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_سطرها},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {پوچ, پوچ}
};


/*
** metamethods for file handles
*/
ایستا ثابت luaL_Reg metameth[] = {
  {"__index", پوچ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {پوچ, پوچ}
};


ایستا تهی createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_برداشت(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ایستا صحیح io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  بازگشت 2;
}


ایستا تهی createstdfile (lua_State *L, FILE *f, ثابت نویسه *k,
                           ثابت نویسه *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  اگر (k != پوچ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API صحیح luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_درون_داد, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, پوچ, "stderr");
  بازگشت 1;
}

