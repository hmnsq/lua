/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#परिभाषय liolib_c
#परिभाषय LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <locale.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#यदि !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#यदि !defined(L_MODEEXT)
#परिभाषय L_MODEEXT	"b"
#यदि_अन्तः

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
स्थिर पूर्णाङ्क l_checkmode (ध्रुव वर्ण *बहुलकम्) {
  प्रत्यागम (*बहुलकम् != '\0' && strchr("rwa", *(बहुलकम्++)) != रिक्त &&
         (*बहुलकम् != '+' || ((शून्य)(++बहुलकम्), 1)) &&  /* skip if char is '+' */
         (strspn(बहुलकम्, L_MODEEXT) == वाक्यदैर्घ्यम्(बहुलकम्)));  /* check extensions */
}

#यदि_अन्तः

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#यदि !defined(l_popen)		/* { */

#यदि defined(LUA_USE_POSIX)	/* { */

#परिभाषय l_popen(L,c,m)		(fflush(रिक्त), popen(c,m))
#परिभाषय l_pclose(L,file)	(pclose(file))

#नोचेद्यदि defined(LUA_USE_WINDOWS)	/* }{ */

#परिभाषय l_popen(L,c,m)		(_popen(c,m))
#परिभाषय l_pclose(L,file)	(_pclose(file))

#यदि !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#परिभाषय l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#यदि_अन्तः

#अन्यथा				/* }{ */

/* ISO C definitions */
#परिभाषय l_popen(L,c,m)  \
	  ((शून्य)c, (शून्य)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#परिभाषय l_pclose(L,file)		((शून्य)L, (शून्य)file, -1)

#यदि_अन्तः				/* } */

#यदि_अन्तः				/* } */


#यदि !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#परिभाषय l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#यदि_अन्तः

/* }====================================================== */


#यदि !defined(l_getc)		/* { */

#यदि defined(LUA_USE_POSIX)
#परिभाषय l_getc(f)		getc_unlocked(f)
#परिभाषय l_lockfile(f)		flockfile(f)
#परिभाषय l_unlockfile(f)		funlockfile(f)
#अन्यथा
#परिभाषय l_getc(f)		getc(f)
#परिभाषय l_lockfile(f)		((शून्य)0)
#परिभाषय l_unlockfile(f)		((शून्य)0)
#यदि_अन्तः

#यदि_अन्तः				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#यदि !defined(l_fseek)		/* { */

#यदि defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#अन्तर्भाव <sys/types.h>

#परिभाषय l_fseek(f,o,w)		fseeko(f,o,w)
#परिभाषय l_ftell(f)		ftello(f)
#परिभाषय l_seeknum		off_t

#नोचेद्यदि defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#परिभाषय l_fseek(f,o,w)		_fseeki64(f,o,w)
#परिभाषय l_ftell(f)		_ftelli64(f)
#परिभाषय l_seeknum		__int64

#अन्यथा				/* }{ */

/* ISO C definitions */
#परिभाषय l_fseek(f,o,w)		fseek(f,o,w)
#परिभाषय l_ftell(f)		ftell(f)
#परिभाषय l_seeknum		दीर्घ

#यदि_अन्तः				/* } */

#यदि_अन्तः				/* } */

/* }====================================================== */



#परिभाषय IO_उपसर्गः	"_IO_"
#परिभाषय IOPREF_LEN	(परिमाण(IO_उपसर्गः)/परिमाण(वर्ण) - 1)
#परिभाषय IO_निवेशः	(IO_उपसर्गः "input")
#परिभाषय IO_OUTPUT	(IO_उपसर्गः "output")


प्रकारनिर्देश lua_L_प्रवाहः LStream;


#परिभाषय tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#परिभाषय isclosed(p)	((p)->closef == रिक्त)


स्थिर पूर्णाङ्क io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  यदि (p == रिक्त)
    luaL_pushfail(L);  /* not a file */
  अन्यथा यदि (isclosed(p))
    lua_pushliteral(L, "closed file");
  अन्यथा
    lua_pushliteral(L, "file");
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  अन्यथा
    lua_pushfstring(L, "file (%p)", p->f);
  प्रत्यागम 1;
}


स्थिर FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  प्रत्यागम p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
स्थिर LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, परिमाण(LStream), 0);
  p->closef = रिक्त;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  प्रत्यागम p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
स्थिर पूर्णाङ्क aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  चञ्चल lua_CFunction cf = p->closef;
  p->closef = रिक्त;  /* mark stream as closed */
  प्रत्यागम (*cf)(L);  /* close it */
}


स्थिर पूर्णाङ्क f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  प्रत्यागम aux_close(L);
}


स्थिर पूर्णाङ्क io_close (lua_State *L) {
  यदि (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  प्रत्यागम f_close(L);
}


स्थिर पूर्णाङ्क f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (!isclosed(p) && p->f != रिक्त)
    aux_close(L);  /* ignore closed and incompletely open files */
  प्रत्यागम 0;
}


/*
** function to close regular files
*/
स्थिर पूर्णाङ्क io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  प्रत्यागम luaL_fileresult(L, (fclose(p->f) == 0), रिक्त);
}


स्थिर LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = रिक्त;
  p->closef = &io_fclose;
  प्रत्यागम p;
}


स्थिर शून्य opencheck (lua_State *L, ध्रुव वर्ण *fname, ध्रुव वर्ण *बहुलकम्) {
  LStream *p = newfile(L);
  p->f = fopen(fname, बहुलकम्);
  यदि (l_unlikely(p->f == रिक्त))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


स्थिर पूर्णाङ्क io_open (lua_State *L) {
  ध्रुव वर्ण *filename = luaL_checkstring(L, 1);
  ध्रुव वर्ण *बहुलकम् = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ध्रुव वर्ण *md = बहुलकम्;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, बहुलकम्);
  प्रत्यागम (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
स्थिर पूर्णाङ्क io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  प्रत्यागम luaL_execresult(L, l_pclose(L, p->f));
}


स्थिर पूर्णाङ्क io_popen (lua_State *L) {
  ध्रुव वर्ण *filename = luaL_checkstring(L, 1);
  ध्रुव वर्ण *बहुलकम् = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(बहुलकम्), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, बहुलकम्);
  p->closef = &io_pclose;
  प्रत्यागम (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


स्थिर पूर्णाङ्क io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  प्रत्यागम (p->f == रिक्त) ? luaL_fileresult(L, 0, रिक्त) : 1;
}


स्थिर FILE *getiofile (lua_State *L, ध्रुव वर्ण *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  यदि (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  प्रत्यागम p->f;
}


स्थिर पूर्णाङ्क g_iofile (lua_State *L, ध्रुव वर्ण *f, ध्रुव वर्ण *बहुलकम्) {
  यदि (!lua_isnoneornil(L, 1)) {
    ध्रुव वर्ण *filename = lua_tostring(L, 1);
    यदि (filename)
      opencheck(L, filename, बहुलकम्);
    अन्यथा {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क io_निवेशः (lua_State *L) {
  प्रत्यागम g_iofile(L, IO_निवेशः, "r");
}


स्थिर पूर्णाङ्क io_output (lua_State *L) {
  प्रत्यागम g_iofile(L, IO_OUTPUT, "w");
}


स्थिर पूर्णाङ्क io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#परिभाषय MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
स्थिर शून्य aux_पङ्क्तयः (lua_State *L, पूर्णाङ्क toclose) {
  पूर्णाङ्क n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


स्थिर पूर्णाङ्क f_पङ्क्तयः (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_पङ्क्तयः(L, 0);
  प्रत्यागम 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
स्थिर पूर्णाङ्क io_पङ्क्तयः (lua_State *L) {
  पूर्णाङ्क toclose;
  यदि (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  यदि (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_निवेशः);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  अन्यथा {  /* open a new file */
    ध्रुव वर्ण *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_पङ्क्तयः(L, toclose);  /* push iteration function */
  यदि (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    प्रत्यागम 4;
  }
  अन्यथा
    प्रत्यागम 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#यदि !defined (L_MAXLENNUM)
#परिभाषय L_MAXLENNUM     200
#यदि_अन्तः


/* auxiliary structure used by 'read_number' */
प्रकारनिर्देश संरचना {
  FILE *f;  /* file being read */
  पूर्णाङ्क c;  /* current character (look ahead) */
  पूर्णाङ्क n;  /* number of elements in buffer 'buff' */
  वर्ण buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
स्थिर पूर्णाङ्क nextc (RN *rn) {
  यदि (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    प्रत्यागम 0;  /* fail */
  }
  अन्यथा {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    प्रत्यागम 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
स्थिर पूर्णाङ्क test2 (RN *rn, ध्रुव वर्ण *समुच्चयः) {
  यदि (rn->c == समुच्चयः[0] || rn->c == समुच्चयः[1])
    प्रत्यागम nextc(rn);
  अन्यथा प्रत्यागम 0;
}


/*
** Read a sequence of (hex)digits
*/
स्थिर पूर्णाङ्क readdigits (RN *rn, पूर्णाङ्क hex) {
  पूर्णाङ्क गणनम् = 0;
  यावत् ((hex ? isxdigit(rn->c) : अङ्कः_वा(rn->c)) && nextc(rn))
    गणनम्++;
  प्रत्यागम गणनम्;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
स्थिर पूर्णाङ्क read_सङ्ख्या (lua_State *L, FILE *f) {
  RN rn;
  पूर्णाङ्क गणनम् = 0;
  पूर्णाङ्क hex = 0;
  वर्ण decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  कुरु { rn.c = l_getc(rn.f); } यावत् (रिक्तम्_वा(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  यदि (test2(&rn, "00")) {
    यदि (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    अन्यथा गणनम् = 1;  /* count initial '0' as a valid digit */
  }
  गणनम् += readdigits(&rn, hex);  /* integral part */
  यदि (test2(&rn, decp))  /* decimal point? */
    गणनम् += readdigits(&rn, hex);  /* fractional part */
  यदि (गणनम् > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  यदि (l_likely(lua_stringtonumber(L, rn.buff)))
    प्रत्यागम 1;  /* ok, it is a valid number */
  अन्यथा {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   प्रत्यागम 0;  /* read fails */
  }
}


स्थिर पूर्णाङ्क test_eof (lua_State *L, FILE *f) {
  पूर्णाङ्क c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  प्रत्यागम (c != EOF);
}


स्थिर पूर्णाङ्क read_पङ्क्तिः (lua_State *L, FILE *f, पूर्णाङ्क chop) {
  lua_L_बफर b;
  पूर्णाङ्क c;
  luaL_buffinit(L, &b);
  कुरु {  /* may need to read several chunks to get whole line */
    वर्ण *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    अचिह्नित i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    यावत् (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } यावत् (c != EOF && c != '\n');  /* repeat until end of line */
  यदि (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  प्रत्यागम (c == '\n' || lua_rawlen(L, -1) > 0);
}


स्थिर शून्य read_all (lua_State *L, FILE *f) {
  size_t nr;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  कुरु {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    वर्ण *p = luaL_prepbuffer(&b);
    nr = fread(p, परिमाण(वर्ण), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } यावत् (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


स्थिर पूर्णाङ्क read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  वर्ण *p;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, परिमाण(वर्ण), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  प्रत्यागम (nr > 0);  /* true iff read something */
}


स्थिर पूर्णाङ्क g_read (lua_State *L, FILE *f, पूर्णाङ्क first) {
  पूर्णाङ्क nargs = lua_gettop(L) - 1;
  पूर्णाङ्क n, success;
  clearerr(f);
  errno = 0;
  यदि (nargs == 0) {  /* no arguments? */
    success = read_पङ्क्तिः(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  अन्यथा {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    प्रति (n = first; nargs-- && success; n++) {
      यदि (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      अन्यथा {
        ध्रुव वर्ण *p = luaL_checkstring(L, n);
        यदि (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        विकल्प (*p) {
          पक्ष 'n':  /*सङ्ख्या*/
            success = read_सङ्ख्या(L, f);
            भङ्ग;
          पक्ष 'l':  /*पङ्क्तिः*/
            success = read_पङ्क्तिः(L, f, 1);
            भङ्ग;
          पक्ष 'L':  /* line with end-of-line */
            success = read_पङ्क्तिः(L, f, 0);
            भङ्ग;
          पक्ष 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            भङ्ग;
          स्वभाव:
            प्रत्यागम luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  यदि (ferror(f))
    प्रत्यागम luaL_fileresult(L, 0, रिक्त);
  यदि (!success) {
    lua_निष्कासय(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  प्रत्यागम n - first;
}


स्थिर पूर्णाङ्क io_read (lua_State *L) {
  प्रत्यागम g_read(L, getiofile(L, IO_निवेशः), 1);
}


स्थिर पूर्णाङ्क f_read (lua_State *L) {
  प्रत्यागम g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
स्थिर पूर्णाङ्क io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  पूर्णाङ्क i;
  पूर्णाङ्क n = (पूर्णाङ्क)lua_tointeger(L, lua_upvalueindex(2));
  यदि (isclosed(p))  /* file is already closed? */
    प्रत्यागम luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  प्रति (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  यदि (lua_toboolean(L, -n))  /* read at least one value? */
    प्रत्यागम n;  /* return them */
  अन्यथा {  /* first result is false: EOF or error */
    यदि (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      प्रत्यागम luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    यदि (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    प्रत्यागम 0;
  }
}

/* }====================================================== */


स्थिर पूर्णाङ्क g_write (lua_State *L, FILE *f, पूर्णाङ्क arg) {
  पूर्णाङ्क nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  प्रति (; nargs--; arg++) {  /* for each argument */
    वर्ण buff[LUA_N2SBUFFSZ];
    ध्रुव वर्ण *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    यदि (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    अन्यथा  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, परिमाण(वर्ण), len, f);
    totalbytes += numbytes;
    यदि (numbytes < len) {  /* write error? */
      पूर्णाङ्क n = luaL_fileresult(L, 0, रिक्त);
      lua_pushinteger(L, cast_st2S(totalbytes));
      प्रत्यागम n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  प्रत्यागम 1;  /* no errors; file handle already on stack top */
}


स्थिर पूर्णाङ्क io_write (lua_State *L) {
  प्रत्यागम g_write(L, getiofile(L, IO_OUTPUT), 1);
}


स्थिर पूर्णाङ्क f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  प्रत्यागम g_write(L, f, 2);
}


स्थिर पूर्णाङ्क f_seek (lua_State *L) {
  स्थिर ध्रुव पूर्णाङ्क बहुलकम्[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  स्थिर ध्रुव वर्ण *ध्रुव modenames[] = {"set", "cur", "end", रिक्त};
  FILE *f = tofile(L);
  पूर्णाङ्क op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum विस्थापनम् = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)विस्थापनम् == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, विस्थापनम्, बहुलकम्[op]);
  यदि (l_unlikely(op))
    प्रत्यागम luaL_fileresult(L, 0, रिक्त);  /* error */
  अन्यथा {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    प्रत्यागम 1;
  }
}


स्थिर पूर्णाङ्क f_setvbuf (lua_State *L) {
  स्थिर ध्रुव पूर्णाङ्क बहुलकम्[] = {_IONBF, _IOFBF, _IOLBF};
  स्थिर ध्रुव वर्ण *ध्रुव modenames[] = {"no", "full", "line", रिक्त};
  FILE *f = tofile(L);
  पूर्णाङ्क op = luaL_checkoption(L, 2, रिक्त, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  पूर्णाङ्क res;
  errno = 0;
  res = setvbuf(f, रिक्त, बहुलकम्[op], (size_t)sz);
  प्रत्यागम luaL_fileresult(L, res == 0, रिक्त);
}


स्थिर पूर्णाङ्क aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  प्रत्यागम luaL_fileresult(L, fflush(f) == 0, रिक्त);
}


स्थिर पूर्णाङ्क f_flush (lua_State *L) {
  प्रत्यागम aux_flush(L, tofile(L));
}


स्थिर पूर्णाङ्क io_flush (lua_State *L) {
  प्रत्यागम aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
स्थिर ध्रुव luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_निवेशः},
  {"lines", io_पङ्क्तयः},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {रिक्त, रिक्त}
};


/*
** methods for file handles
*/
स्थिर ध्रुव luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_पङ्क्तयः},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {रिक्त, रिक्त}
};


/*
** metamethods for file handles
*/
स्थिर ध्रुव luaL_Reg metameth[] = {
  {"__index", रिक्त},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {रिक्त, रिक्त}
};


स्थिर शून्य createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_निष्कासय(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
स्थिर पूर्णाङ्क io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  प्रत्यागम 2;
}


स्थिर शून्य createstdfile (lua_State *L, FILE *f, ध्रुव वर्ण *k,
                           ध्रुव वर्ण *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  यदि (k != रिक्त) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API पूर्णाङ्क luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_निवेशः, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, रिक्त, "stderr");
  प्रत्यागम 1;
}

