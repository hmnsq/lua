/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#تعریف_کرو liolib_c
#تعریف_کرو LUA_LIB

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
#تعریف_کرو L_MODEEXT	"b"
#اگر_ختم

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ساکن صحیح l_checkmode (ثابت حرف *منوال) {
  واپس (*منوال != '\0' && strchr("rwa", *(منوال++)) != تہی &&
         (*منوال != '+' || ((خالی)(++منوال), 1)) &&  /* skip if char is '+' */
         (strspn(منوال, L_MODEEXT) == لڑی_لمبائی(منوال)));  /* check extensions */
}

#اگر_ختم

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#اگر !defined(l_popen)		/* { */

#اگر defined(LUA_USE_POSIX)	/* { */

#تعریف_کرو l_popen(L,c,m)		(fflush(تہی), popen(c,m))
#تعریف_کرو l_pclose(L,file)	(pclose(file))

#وگرنہ_اگر defined(LUA_USE_WINDOWS)	/* }{ */

#تعریف_کرو l_popen(L,c,m)		(_popen(c,m))
#تعریف_کرو l_pclose(L,file)	(_pclose(file))

#اگر !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#تعریف_کرو l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#اگر_ختم

#ورنہ				/* }{ */

/* ISO C definitions */
#تعریف_کرو l_popen(L,c,m)  \
	  ((خالی)c, (خالی)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#تعریف_کرو l_pclose(L,file)		((خالی)L, (خالی)file, -1)

#اگر_ختم				/* } */

#اگر_ختم				/* } */


#اگر !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#تعریف_کرو l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#اگر_ختم

/* }====================================================== */


#اگر !defined(l_getc)		/* { */

#اگر defined(LUA_USE_POSIX)
#تعریف_کرو l_getc(f)		getc_unlocked(f)
#تعریف_کرو l_lockfile(f)		flockfile(f)
#تعریف_کرو l_unlockfile(f)		funlockfile(f)
#ورنہ
#تعریف_کرو l_getc(f)		getc(f)
#تعریف_کرو l_lockfile(f)		((خالی)0)
#تعریف_کرو l_unlockfile(f)		((خالی)0)
#اگر_ختم

#اگر_ختم				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#اگر !defined(l_fseek)		/* { */

#اگر defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#شامل <sys/types.h>

#تعریف_کرو l_fseek(f,o,w)		fseeko(f,o,w)
#تعریف_کرو l_ftell(f)		ftello(f)
#تعریف_کرو l_seeknum		off_t

#وگرنہ_اگر defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#تعریف_کرو l_fseek(f,o,w)		_fseeki64(f,o,w)
#تعریف_کرو l_ftell(f)		_ftelli64(f)
#تعریف_کرو l_seeknum		__int64

#ورنہ				/* }{ */

/* ISO C definitions */
#تعریف_کرو l_fseek(f,o,w)		fseek(f,o,w)
#تعریف_کرو l_ftell(f)		ftell(f)
#تعریف_کرو l_seeknum		طویل

#اگر_ختم				/* } */

#اگر_ختم				/* } */

/* }====================================================== */



#تعریف_کرو IO_سابقہ	"_IO_"
#تعریف_کرو IOPREF_LEN	(حجم(IO_سابقہ)/حجم(حرف) - 1)
#تعریف_کرو IO_داخلہ	(IO_سابقہ "input")
#تعریف_کرو IO_OUTPUT	(IO_سابقہ "output")


قسمتعریف lua_L_دھارا LStream;


#تعریف_کرو tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#تعریف_کرو isclosed(p)	((p)->closef == تہی)


ساکن صحیح io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  اگر (p == تہی)
    luaL_pushfail(L);  /* not a file */
  ورنہ اگر (isclosed(p))
    lua_pushliteral(L, "closed file");
  ورنہ
    lua_pushliteral(L, "file");
  واپس 1;
}


ساکن صحیح f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ورنہ
    lua_pushfstring(L, "file (%p)", p->f);
  واپس 1;
}


ساکن FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  واپس p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ساکن LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, حجم(LStream), 0);
  p->closef = تہی;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  واپس p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ساکن صحیح aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  متلون lua_CFunction cf = p->closef;
  p->closef = تہی;  /* mark stream as closed */
  واپس (*cf)(L);  /* close it */
}


ساکن صحیح f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  واپس aux_close(L);
}


ساکن صحیح io_close (lua_State *L) {
  اگر (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  واپس f_close(L);
}


ساکن صحیح f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  اگر (!isclosed(p) && p->f != تہی)
    aux_close(L);  /* ignore closed and incompletely open files */
  واپس 0;
}


/*
** function to close regular files
*/
ساکن صحیح io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  واپس luaL_fileresult(L, (fclose(p->f) == 0), تہی);
}


ساکن LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = تہی;
  p->closef = &io_fclose;
  واپس p;
}


ساکن خالی opencheck (lua_State *L, ثابت حرف *fname, ثابت حرف *منوال) {
  LStream *p = newfile(L);
  p->f = fopen(fname, منوال);
  اگر (l_unlikely(p->f == تہی))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ساکن صحیح io_open (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  ثابت حرف *منوال = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ثابت حرف *md = منوال;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, منوال);
  واپس (p->f == تہی) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ساکن صحیح io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  واپس luaL_execresult(L, l_pclose(L, p->f));
}


ساکن صحیح io_popen (lua_State *L) {
  ثابت حرف *filename = luaL_checkstring(L, 1);
  ثابت حرف *منوال = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(منوال), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, منوال);
  p->closef = &io_pclose;
  واپس (p->f == تہی) ? luaL_fileresult(L, 0, filename) : 1;
}


ساکن صحیح io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  واپس (p->f == تہی) ? luaL_fileresult(L, 0, تہی) : 1;
}


ساکن FILE *getiofile (lua_State *L, ثابت حرف *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  اگر (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  واپس p->f;
}


ساکن صحیح g_iofile (lua_State *L, ثابت حرف *f, ثابت حرف *منوال) {
  اگر (!lua_isnoneornil(L, 1)) {
    ثابت حرف *filename = lua_tostring(L, 1);
    اگر (filename)
      opencheck(L, filename, منوال);
    ورنہ {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  واپس 1;
}


ساکن صحیح io_داخلہ (lua_State *L) {
  واپس g_iofile(L, IO_داخلہ, "r");
}


ساکن صحیح io_output (lua_State *L) {
  واپس g_iofile(L, IO_OUTPUT, "w");
}


ساکن صحیح io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#تعریف_کرو MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
ساکن خالی aux_سطریں (lua_State *L, صحیح toclose) {
  صحیح n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ساکن صحیح f_سطریں (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_سطریں(L, 0);
  واپس 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ساکن صحیح io_سطریں (lua_State *L) {
  صحیح toclose;
  اگر (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  اگر (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_داخلہ);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ورنہ {  /* open a new file */
    ثابت حرف *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_سطریں(L, toclose);  /* push iteration function */
  اگر (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    واپس 4;
  }
  ورنہ
    واپس 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#اگر !defined (L_MAXLENNUM)
#تعریف_کرو L_MAXLENNUM     200
#اگر_ختم


/* auxiliary structure used by 'read_number' */
قسمتعریف ساخت {
  FILE *f;  /* file being read */
  صحیح c;  /* current character (look ahead) */
  صحیح n;  /* number of elements in buffer 'buff' */
  حرف buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ساکن صحیح nextc (RN *rn) {
  اگر (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    واپس 0;  /* fail */
  }
  ورنہ {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    واپس 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ساکن صحیح test2 (RN *rn, ثابت حرف *سیٹ) {
  اگر (rn->c == سیٹ[0] || rn->c == سیٹ[1])
    واپس nextc(rn);
  ورنہ واپس 0;
}


/*
** Read a sequence of (hex)digits
*/
ساکن صحیح readdigits (RN *rn, صحیح hex) {
  صحیح گنتی = 0;
  جب ((hex ? isxdigit(rn->c) : ہندسہ_ہے(rn->c)) && nextc(rn))
    گنتی++;
  واپس گنتی;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ساکن صحیح read_نمبر (lua_State *L, FILE *f) {
  RN rn;
  صحیح گنتی = 0;
  صحیح hex = 0;
  حرف decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  کرو { rn.c = l_getc(rn.f); } جب (خلا_ہے(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  اگر (test2(&rn, "00")) {
    اگر (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ورنہ گنتی = 1;  /* count initial '0' as a valid digit */
  }
  گنتی += readdigits(&rn, hex);  /* integral part */
  اگر (test2(&rn, decp))  /* decimal point? */
    گنتی += readdigits(&rn, hex);  /* fractional part */
  اگر (گنتی > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  اگر (l_likely(lua_stringtonumber(L, rn.buff)))
    واپس 1;  /* ok, it is a valid number */
  ورنہ {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   واپس 0;  /* read fails */
  }
}


ساکن صحیح test_eof (lua_State *L, FILE *f) {
  صحیح c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  واپس (c != EOF);
}


ساکن صحیح read_سطر (lua_State *L, FILE *f, صحیح chop) {
  lua_L_بفر b;
  صحیح c;
  luaL_buffinit(L, &b);
  کرو {  /* may need to read several chunks to get whole line */
    حرف *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    غیرعلامتی i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    جب (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } جب (c != EOF && c != '\n');  /* repeat until end of line */
  اگر (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  واپس (c == '\n' || lua_rawlen(L, -1) > 0);
}


ساکن خالی read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  کرو {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    حرف *p = luaL_prepbuffer(&b);
    nr = fread(p, حجم(حرف), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } جب (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ساکن صحیح read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  حرف *p;
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, حجم(حرف), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  واپس (nr > 0);  /* true iff read something */
}


ساکن صحیح g_read (lua_State *L, FILE *f, صحیح پہلا) {
  صحیح nargs = lua_gettop(L) - 1;
  صحیح n, success;
  clearerr(f);
  errno = 0;
  اگر (nargs == 0) {  /* no arguments? */
    success = read_سطر(L, f, 1);
    n = پہلا + 1;  /* to return 1 result */
  }
  ورنہ {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    برائے (n = پہلا; nargs-- && success; n++) {
      اگر (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ورنہ {
        ثابت حرف *p = luaL_checkstring(L, n);
        اگر (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        انتخاب (*p) {
          حالت 'n':  /*نمبر*/
            success = read_نمبر(L, f);
            توڑو;
          حالت 'l':  /*سطر*/
            success = read_سطر(L, f, 1);
            توڑو;
          حالت 'L':  /* line with end-of-line */
            success = read_سطر(L, f, 0);
            توڑو;
          حالت 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            توڑو;
          طےشدہ:
            واپس luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  اگر (ferror(f))
    واپس luaL_fileresult(L, 0, تہی);
  اگر (!success) {
    lua_نکالو(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  واپس n - پہلا;
}


ساکن صحیح io_read (lua_State *L) {
  واپس g_read(L, getiofile(L, IO_داخلہ), 1);
}


ساکن صحیح f_read (lua_State *L) {
  واپس g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ساکن صحیح io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  صحیح i;
  صحیح n = (صحیح)lua_tointeger(L, lua_upvalueindex(2));
  اگر (isclosed(p))  /* file is already closed? */
    واپس luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  برائے (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  اگر (lua_toboolean(L, -n))  /* read at least one value? */
    واپس n;  /* return them */
  ورنہ {  /* first result is false: EOF or error */
    اگر (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      واپس luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    اگر (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    واپس 0;
  }
}

/* }====================================================== */


ساکن صحیح g_write (lua_State *L, FILE *f, صحیح arg) {
  صحیح nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  برائے (; nargs--; arg++) {  /* for each argument */
    حرف buff[LUA_N2SBUFFSZ];
    ثابت حرف *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    اگر (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ورنہ  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, حجم(حرف), len, f);
    totalbytes += numbytes;
    اگر (numbytes < len) {  /* write error? */
      صحیح n = luaL_fileresult(L, 0, تہی);
      lua_pushinteger(L, cast_st2S(totalbytes));
      واپس n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  واپس 1;  /* no errors; file handle already on stack top */
}


ساکن صحیح io_write (lua_State *L) {
  واپس g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ساکن صحیح f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  واپس g_write(L, f, 2);
}


ساکن صحیح f_seek (lua_State *L) {
  ساکن ثابت صحیح منوال[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ساکن ثابت حرف *ثابت modenames[] = {"set", "cur", "end", تہی};
  FILE *f = tofile(L);
  صحیح op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum انحراف = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)انحراف == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, انحراف, منوال[op]);
  اگر (l_unlikely(op))
    واپس luaL_fileresult(L, 0, تہی);  /* error */
  ورنہ {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    واپس 1;
  }
}


ساکن صحیح f_setvbuf (lua_State *L) {
  ساکن ثابت صحیح منوال[] = {_IONBF, _IOFBF, _IOLBF};
  ساکن ثابت حرف *ثابت modenames[] = {"no", "full", "line", تہی};
  FILE *f = tofile(L);
  صحیح op = luaL_checkoption(L, 2, تہی, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  صحیح res;
  errno = 0;
  res = setvbuf(f, تہی, منوال[op], (size_t)sz);
  واپس luaL_fileresult(L, res == 0, تہی);
}


ساکن صحیح aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  واپس luaL_fileresult(L, fflush(f) == 0, تہی);
}


ساکن صحیح f_flush (lua_State *L) {
  واپس aux_flush(L, tofile(L));
}


ساکن صحیح io_flush (lua_State *L) {
  واپس aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ساکن ثابت luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_داخلہ},
  {"lines", io_سطریں},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {تہی, تہی}
};


/*
** methods for file handles
*/
ساکن ثابت luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_سطریں},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {تہی, تہی}
};


/*
** metamethods for file handles
*/
ساکن ثابت luaL_Reg metameth[] = {
  {"__index", تہی},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {تہی, تہی}
};


ساکن خالی createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_نکالو(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ساکن صحیح io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  واپس 2;
}


ساکن خالی createstdfile (lua_State *L, FILE *f, ثابت حرف *k,
                           ثابت حرف *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  اگر (k != تہی) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API صحیح luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_داخلہ, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, تہی, "stderr");
  واپس 1;
}

