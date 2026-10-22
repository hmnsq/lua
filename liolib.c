/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#నిర్వచించు liolib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <locale.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#ఒకవేళ !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ఒకవేళ !defined(L_MODEEXT)
#నిర్వచించు L_MODEEXT	"b"
#అయితే_ముగింపు

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
స్థిర పూర్ణాంకం l_checkmode (స్థిరరాశి అక్షరం *బహుళకం) {
  తిరిగి (*బహుళకం != '\0' && strchr("rwa", *(బహుళకం++)) != ఖాళీ &&
         (*బహుళకం != '+' || ((శూన్య)(++బహుళకం), 1)) &&  /* skip if char is '+' */
         (strspn(బహుళకం, L_MODEEXT) == అక్షరశ్రేణి_పొడవు(బహుళకం)));  /* check extensions */
}

#అయితే_ముగింపు

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ఒకవేళ !defined(l_popen)		/* { */

#ఒకవేళ defined(LUA_USE_POSIX)	/* { */

#నిర్వచించు l_popen(L,c,m)		(fflush(ఖాళీ), popen(c,m))
#నిర్వచించు l_pclose(L,file)	(pclose(file))

#లేదావేళ defined(LUA_USE_WINDOWS)	/* }{ */

#నిర్వచించు l_popen(L,c,m)		(_popen(c,m))
#నిర్వచించు l_pclose(L,file)	(_pclose(file))

#ఒకవేళ !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#నిర్వచించు l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#అయితే_ముగింపు

#లేకపోతే				/* }{ */

/* ISO C definitions */
#నిర్వచించు l_popen(L,c,m)  \
	  ((శూన్య)c, (శూన్య)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#నిర్వచించు l_pclose(L,file)		((శూన్య)L, (శూన్య)file, -1)

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */


#ఒకవేళ !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#నిర్వచించు l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#అయితే_ముగింపు

/* }====================================================== */


#ఒకవేళ !defined(l_getc)		/* { */

#ఒకవేళ defined(LUA_USE_POSIX)
#నిర్వచించు l_getc(f)		getc_unlocked(f)
#నిర్వచించు l_lockfile(f)		flockfile(f)
#నిర్వచించు l_unlockfile(f)		funlockfile(f)
#లేకపోతే
#నిర్వచించు l_getc(f)		getc(f)
#నిర్వచించు l_lockfile(f)		((శూన్య)0)
#నిర్వచించు l_unlockfile(f)		((శూన్య)0)
#అయితే_ముగింపు

#అయితే_ముగింపు				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ఒకవేళ !defined(l_fseek)		/* { */

#ఒకవేళ defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#అన్తర్భావ <sys/types.h>

#నిర్వచించు l_fseek(f,o,w)		fseeko(f,o,w)
#నిర్వచించు l_ftell(f)		ftello(f)
#నిర్వచించు l_seeknum		off_t

#లేదావేళ defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#నిర్వచించు l_fseek(f,o,w)		_fseeki64(f,o,w)
#నిర్వచించు l_ftell(f)		_ftelli64(f)
#నిర్వచించు l_seeknum		__int64

#లేకపోతే				/* }{ */

/* ISO C definitions */
#నిర్వచించు l_fseek(f,o,w)		fseek(f,o,w)
#నిర్వచించు l_ftell(f)		ftell(f)
#నిర్వచించు l_seeknum		దీర్ఘ

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */

/* }====================================================== */



#నిర్వచించు IO_ఉపసర్గ	"_IO_"
#నిర్వచించు IOPREF_LEN	(పరిమాణం(IO_ఉపసర్గ)/పరిమాణం(అక్షరం) - 1)
#నిర్వచించు IO_ఆదానం	(IO_ఉపసర్గ "input")
#నిర్వచించు IO_OUTPUT	(IO_ఉపసర్గ "output")


రకనిర్వచనం lua_L_ప్రవాహం LStream;


#నిర్వచించు tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#నిర్వచించు isclosed(p)	((p)->closef == ఖాళీ)


స్థిర పూర్ణాంకం io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ఒకవేళ (p == ఖాళీ)
    luaL_pushfail(L);  /* not a file */
  లేకపోతే ఒకవేళ (isclosed(p))
    lua_pushliteral(L, "closed file");
  లేకపోతే
    lua_pushliteral(L, "file");
  తిరిగి 1;
}


స్థిర పూర్ణాంకం f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ఒకవేళ (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  లేకపోతే
    lua_pushfstring(L, "file (%p)", p->f);
  తిరిగి 1;
}


స్థిర FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ఒకవేళ (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  తిరిగి p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
స్థిర LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, పరిమాణం(LStream), 0);
  p->closef = ఖాళీ;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  తిరిగి p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
స్థిర పూర్ణాంకం aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  అస్థిర lua_CFunction cf = p->closef;
  p->closef = ఖాళీ;  /* mark stream as closed */
  తిరిగి (*cf)(L);  /* close it */
}


స్థిర పూర్ణాంకం f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  తిరిగి aux_close(L);
}


స్థిర పూర్ణాంకం io_close (lua_State *L) {
  ఒకవేళ (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  తిరిగి f_close(L);
}


స్థిర పూర్ణాంకం f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ఒకవేళ (!isclosed(p) && p->f != ఖాళీ)
    aux_close(L);  /* ignore closed and incompletely open files */
  తిరిగి 0;
}


/*
** function to close regular files
*/
స్థిర పూర్ణాంకం io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  తిరిగి luaL_fileresult(L, (fclose(p->f) == 0), ఖాళీ);
}


స్థిర LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ఖాళీ;
  p->closef = &io_fclose;
  తిరిగి p;
}


స్థిర శూన్య opencheck (lua_State *L, స్థిరరాశి అక్షరం *fname, స్థిరరాశి అక్షరం *బహుళకం) {
  LStream *p = newfile(L);
  p->f = fopen(fname, బహుళకం);
  ఒకవేళ (l_unlikely(p->f == ఖాళీ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


స్థిర పూర్ణాంకం io_open (lua_State *L) {
  స్థిరరాశి అక్షరం *filename = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *బహుళకం = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  స్థిరరాశి అక్షరం *md = బహుళకం;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, బహుళకం);
  తిరిగి (p->f == ఖాళీ) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
స్థిర పూర్ణాంకం io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  తిరిగి luaL_execresult(L, l_pclose(L, p->f));
}


స్థిర పూర్ణాంకం io_popen (lua_State *L) {
  స్థిరరాశి అక్షరం *filename = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *బహుళకం = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(బహుళకం), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, బహుళకం);
  p->closef = &io_pclose;
  తిరిగి (p->f == ఖాళీ) ? luaL_fileresult(L, 0, filename) : 1;
}


స్థిర పూర్ణాంకం io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  తిరిగి (p->f == ఖాళీ) ? luaL_fileresult(L, 0, ఖాళీ) : 1;
}


స్థిర FILE *getiofile (lua_State *L, స్థిరరాశి అక్షరం *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ఒకవేళ (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  తిరిగి p->f;
}


స్థిర పూర్ణాంకం g_iofile (lua_State *L, స్థిరరాశి అక్షరం *f, స్థిరరాశి అక్షరం *బహుళకం) {
  ఒకవేళ (!lua_isnoneornil(L, 1)) {
    స్థిరరాశి అక్షరం *filename = lua_tostring(L, 1);
    ఒకవేళ (filename)
      opencheck(L, filename, బహుళకం);
    లేకపోతే {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం io_ఆదానం (lua_State *L) {
  తిరిగి g_iofile(L, IO_ఆదానం, "r");
}


స్థిర పూర్ణాంకం io_output (lua_State *L) {
  తిరిగి g_iofile(L, IO_OUTPUT, "w");
}


స్థిర పూర్ణాంకం io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#నిర్వచించు MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
స్థిర శూన్య aux_పంక్తులు (lua_State *L, పూర్ణాంకం toclose) {
  పూర్ణాంకం n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


స్థిర పూర్ణాంకం f_పంక్తులు (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_పంక్తులు(L, 0);
  తిరిగి 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
స్థిర పూర్ణాంకం io_పంక్తులు (lua_State *L) {
  పూర్ణాంకం toclose;
  ఒకవేళ (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ఒకవేళ (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_ఆదానం);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  లేకపోతే {  /* open a new file */
    స్థిరరాశి అక్షరం *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_పంక్తులు(L, toclose);  /* push iteration function */
  ఒకవేళ (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    తిరిగి 4;
  }
  లేకపోతే
    తిరిగి 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ఒకవేళ !defined (L_MAXLENNUM)
#నిర్వచించు L_MAXLENNUM     200
#అయితే_ముగింపు


/* auxiliary structure used by 'read_number' */
రకనిర్వచనం నిర్మాణం {
  FILE *f;  /* file being read */
  పూర్ణాంకం c;  /* current character (look ahead) */
  పూర్ణాంకం n;  /* number of elements in buffer 'buff' */
  అక్షరం buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
స్థిర పూర్ణాంకం nextc (RN *rn) {
  ఒకవేళ (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    తిరిగి 0;  /* fail */
  }
  లేకపోతే {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    తిరిగి 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
స్థిర పూర్ణాంకం test2 (RN *rn, స్థిరరాశి అక్షరం *సమితి) {
  ఒకవేళ (rn->c == సమితి[0] || rn->c == సమితి[1])
    తిరిగి nextc(rn);
  లేకపోతే తిరిగి 0;
}


/*
** Read a sequence of (hex)digits
*/
స్థిర పూర్ణాంకం readdigits (RN *rn, పూర్ణాంకం hex) {
  పూర్ణాంకం లెక్క = 0;
  అయితే ((hex ? isxdigit(rn->c) : అంకెనా(rn->c)) && nextc(rn))
    లెక్క++;
  తిరిగి లెక్క;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
స్థిర పూర్ణాంకం read_సంఖ్య (lua_State *L, FILE *f) {
  RN rn;
  పూర్ణాంకం లెక్క = 0;
  పూర్ణాంకం hex = 0;
  అక్షరం decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  చేయి { rn.c = l_getc(rn.f); } అయితే (ఖాళీనా(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ఒకవేళ (test2(&rn, "00")) {
    ఒకవేళ (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    లేకపోతే లెక్క = 1;  /* count initial '0' as a valid digit */
  }
  లెక్క += readdigits(&rn, hex);  /* integral part */
  ఒకవేళ (test2(&rn, decp))  /* decimal point? */
    లెక్క += readdigits(&rn, hex);  /* fractional part */
  ఒకవేళ (లెక్క > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ఒకవేళ (l_likely(lua_stringtonumber(L, rn.buff)))
    తిరిగి 1;  /* ok, it is a valid number */
  లేకపోతే {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   తిరిగి 0;  /* read fails */
  }
}


స్థిర పూర్ణాంకం test_eof (lua_State *L, FILE *f) {
  పూర్ణాంకం c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  తిరిగి (c != EOF);
}


స్థిర పూర్ణాంకం read_పంక్తి (lua_State *L, FILE *f, పూర్ణాంకం chop) {
  lua_L_బఫరు b;
  పూర్ణాంకం c;
  luaL_buffinit(L, &b);
  చేయి {  /* may need to read several chunks to get whole line */
    అక్షరం *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    గుర్తులేని i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    అయితే (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } అయితే (c != EOF && c != '\n');  /* repeat until end of line */
  ఒకవేళ (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  తిరిగి (c == '\n' || lua_rawlen(L, -1) > 0);
}


స్థిర శూన్య read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_బఫరు b;
  luaL_buffinit(L, &b);
  చేయి {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    అక్షరం *p = luaL_prepbuffer(&b);
    nr = fread(p, పరిమాణం(అక్షరం), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } అయితే (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


స్థిర పూర్ణాంకం read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  అక్షరం *p;
  lua_L_బఫరు b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, పరిమాణం(అక్షరం), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  తిరిగి (nr > 0);  /* true iff read something */
}


స్థిర పూర్ణాంకం g_read (lua_State *L, FILE *f, పూర్ణాంకం మొదటి) {
  పూర్ణాంకం nargs = lua_gettop(L) - 1;
  పూర్ణాంకం n, success;
  clearerr(f);
  errno = 0;
  ఒకవేళ (nargs == 0) {  /* no arguments? */
    success = read_పంక్తి(L, f, 1);
    n = మొదటి + 1;  /* to return 1 result */
  }
  లేకపోతే {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    కొరకు (n = మొదటి; nargs-- && success; n++) {
      ఒకవేళ (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      లేకపోతే {
        స్థిరరాశి అక్షరం *p = luaL_checkstring(L, n);
        ఒకవేళ (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        ఎంపిక (*p) {
          స్థితి 'n':  /*సంఖ్య*/
            success = read_సంఖ్య(L, f);
            ఆపు;
          స్థితి 'l':  /*పంక్తి*/
            success = read_పంక్తి(L, f, 1);
            ఆపు;
          స్థితి 'L':  /* line with end-of-line */
            success = read_పంక్తి(L, f, 0);
            ఆపు;
          స్థితి 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            ఆపు;
          అప్రమేయ:
            తిరిగి luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ఒకవేళ (ferror(f))
    తిరిగి luaL_fileresult(L, 0, ఖాళీ);
  ఒకవేళ (!success) {
    lua_తీయి(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  తిరిగి n - మొదటి;
}


స్థిర పూర్ణాంకం io_read (lua_State *L) {
  తిరిగి g_read(L, getiofile(L, IO_ఆదానం), 1);
}


స్థిర పూర్ణాంకం f_read (lua_State *L) {
  తిరిగి g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
స్థిర పూర్ణాంకం io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  పూర్ణాంకం i;
  పూర్ణాంకం n = (పూర్ణాంకం)lua_tointeger(L, lua_upvalueindex(2));
  ఒకవేళ (isclosed(p))  /* file is already closed? */
    తిరిగి luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  కొరకు (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ఒకవేళ (lua_toboolean(L, -n))  /* read at least one value? */
    తిరిగి n;  /* return them */
  లేకపోతే {  /* first result is false: EOF or error */
    ఒకవేళ (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      తిరిగి luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ఒకవేళ (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    తిరిగి 0;
  }
}

/* }====================================================== */


స్థిర పూర్ణాంకం g_write (lua_State *L, FILE *f, పూర్ణాంకం arg) {
  పూర్ణాంకం nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  కొరకు (; nargs--; arg++) {  /* for each argument */
    అక్షరం buff[LUA_N2SBUFFSZ];
    స్థిరరాశి అక్షరం *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ఒకవేళ (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    లేకపోతే  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, పరిమాణం(అక్షరం), len, f);
    totalbytes += numbytes;
    ఒకవేళ (numbytes < len) {  /* write error? */
      పూర్ణాంకం n = luaL_fileresult(L, 0, ఖాళీ);
      lua_pushinteger(L, cast_st2S(totalbytes));
      తిరిగి n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  తిరిగి 1;  /* no errors; file handle already on stack top */
}


స్థిర పూర్ణాంకం io_write (lua_State *L) {
  తిరిగి g_write(L, getiofile(L, IO_OUTPUT), 1);
}


స్థిర పూర్ణాంకం f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  తిరిగి g_write(L, f, 2);
}


స్థిర పూర్ణాంకం f_seek (lua_State *L) {
  స్థిర స్థిరరాశి పూర్ణాంకం బహుళకం[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి modenames[] = {"set", "cur", "end", ఖాళీ};
  FILE *f = tofile(L);
  పూర్ణాంకం op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum స్థానభ్రంశం = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)స్థానభ్రంశం == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, స్థానభ్రంశం, బహుళకం[op]);
  ఒకవేళ (l_unlikely(op))
    తిరిగి luaL_fileresult(L, 0, ఖాళీ);  /* error */
  లేకపోతే {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    తిరిగి 1;
  }
}


స్థిర పూర్ణాంకం f_setvbuf (lua_State *L) {
  స్థిర స్థిరరాశి పూర్ణాంకం బహుళకం[] = {_IONBF, _IOFBF, _IOLBF};
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి modenames[] = {"no", "full", "line", ఖాళీ};
  FILE *f = tofile(L);
  పూర్ణాంకం op = luaL_checkoption(L, 2, ఖాళీ, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  పూర్ణాంకం res;
  errno = 0;
  res = setvbuf(f, ఖాళీ, బహుళకం[op], (size_t)sz);
  తిరిగి luaL_fileresult(L, res == 0, ఖాళీ);
}


స్థిర పూర్ణాంకం aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  తిరిగి luaL_fileresult(L, fflush(f) == 0, ఖాళీ);
}


స్థిర పూర్ణాంకం f_flush (lua_State *L) {
  తిరిగి aux_flush(L, tofile(L));
}


స్థిర పూర్ణాంకం io_flush (lua_State *L) {
  తిరిగి aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
స్థిర స్థిరరాశి luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_ఆదానం},
  {"lines", io_పంక్తులు},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ఖాళీ, ఖాళీ}
};


/*
** methods for file handles
*/
స్థిర స్థిరరాశి luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_పంక్తులు},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ఖాళీ, ఖాళీ}
};


/*
** metamethods for file handles
*/
స్థిర స్థిరరాశి luaL_Reg metameth[] = {
  {"__index", ఖాళీ},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ఖాళీ, ఖాళీ}
};


స్థిర శూన్య createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_తీయి(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
స్థిర పూర్ణాంకం io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  తిరిగి 2;
}


స్థిర శూన్య createstdfile (lua_State *L, FILE *f, స్థిరరాశి అక్షరం *k,
                           స్థిరరాశి అక్షరం *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ఒకవేళ (k != ఖాళీ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API పూర్ణాంకం luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_ఆదానం, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ఖాళీ, "stderr");
  తిరిగి 1;
}

