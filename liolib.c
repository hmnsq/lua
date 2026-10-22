/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

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
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
स्थिर पूर्णाङ्क l_checkmode (अचर वर्ण *mode) {
  फर्काऊ (*mode != '\0' && म_अक्षर("rwa", *(mode++)) != रिक्त &&
         (*mode != '+' || ((शून्य)(++mode), 1)) &&  /* skip if char is '+' */
         (म_अखोज(mode, L_MODEEXT) == म_माप(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#यदि !defined(l_popen)		/* { */

#यदि defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(ख_साफ(रिक्त), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#अथवायदि defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#यदि !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#अन्यथा				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((शून्य)c, (शून्य)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (खाता*)0)
#define l_pclose(L,file)		((शून्य)L, (शून्य)file, -1)

#endif				/* } */

#endif				/* } */


#यदि !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#यदि !defined(l_getc)		/* { */

#यदि defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#अन्यथा
#define l_getc(f)		अ_लो(f)
#define l_lockfile(f)		((शून्य)0)
#define l_unlockfile(f)		((शून्य)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#यदि !defined(l_fseek)		/* { */

#यदि defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#अन्तर्भाव <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#अथवायदि defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#अन्यथा				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		ख_जाओ(f,o,w)
#define l_ftell(f)		ख_बताओ(f)
#define l_seeknum		दीर्घ

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(परिमाण(IO_PREFIX)/परिमाण(वर्ण) - 1)
#define IO_आगत	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


प्रकारनिर्देश luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == रिक्त)


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
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  अन्यथा
    lua_pushfstring(L, "file (%p)", p->f);
  फर्काऊ 1;
}


स्थिर खाता *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  फर्काऊ p->f;
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
  फर्काऊ p;
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
  फर्काऊ (*cf)(L);  /* close it */
}


स्थिर पूर्णाङ्क f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  फर्काऊ aux_close(L);
}


स्थिर पूर्णाङ्क io_close (lua_State *L) {
  यदि (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  फर्काऊ f_close(L);
}


स्थिर पूर्णाङ्क f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  यदि (!isclosed(p) && p->f != रिक्त)
    aux_close(L);  /* ignore closed and incompletely open files */
  फर्काऊ 0;
}


/*
** function to close regular files
*/
स्थिर पूर्णाङ्क io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  फर्काऊ luaL_fileresult(L, (ख_बंद(p->f) == 0), रिक्त);
}


स्थिर LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = रिक्त;
  p->closef = &io_fclose;
  फर्काऊ p;
}


स्थिर शून्य opencheck (lua_State *L, अचर वर्ण *fname, अचर वर्ण *mode) {
  LStream *p = newfile(L);
  p->f = ख_खोलो(fname, mode);
  यदि (l_unlikely(p->f == रिक्त))
    luaL_error(L, "cannot open file '%s' (%s)", fname, म_त्रुटि(त्रुटि_सं));
}


स्थिर पूर्णाङ्क io_open (lua_State *L) {
  अचर वर्ण *filename = luaL_checkstring(L, 1);
  अचर वर्ण *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  अचर वर्ण *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = ख_खोलो(filename, mode);
  फर्काऊ (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
स्थिर पूर्णाङ्क io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  फर्काऊ luaL_execresult(L, l_pclose(L, p->f));
}


स्थिर पूर्णाङ्क io_popen (lua_State *L) {
  अचर वर्ण *filename = luaL_checkstring(L, 1);
  अचर वर्ण *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  फर्काऊ (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


स्थिर पूर्णाङ्क io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  त्रुटि_सं = 0;
  p->f = क्षणिक_ख();
  फर्काऊ (p->f == रिक्त) ? luaL_fileresult(L, 0, रिक्त) : 1;
}


स्थिर खाता *getiofile (lua_State *L, अचर वर्ण *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  यदि (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  फर्काऊ p->f;
}


स्थिर पूर्णाङ्क g_iofile (lua_State *L, अचर वर्ण *f, अचर वर्ण *mode) {
  यदि (!lua_isnoneornil(L, 1)) {
    अचर वर्ण *filename = lua_tostring(L, 1);
    यदि (filename)
      opencheck(L, filename, mode);
    अन्यथा {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क io_आगत (lua_State *L) {
  फर्काऊ g_iofile(L, IO_आगत, "r");
}


स्थिर पूर्णाङ्क io_output (lua_State *L) {
  फर्काऊ g_iofile(L, IO_OUTPUT, "w");
}


स्थिर पूर्णाङ्क io_readline (lua_State *L);


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
स्थिर शून्य aux_पङ्क्तिहरू (lua_State *L, पूर्णाङ्क toclose) {
  पूर्णाङ्क n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


स्थिर पूर्णाङ्क f_पङ्क्तिहरू (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_पङ्क्तिहरू(L, 0);
  फर्काऊ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
स्थिर पूर्णाङ्क io_पङ्क्तिहरू (lua_State *L) {
  पूर्णाङ्क toclose;
  यदि (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  यदि (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_आगत);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  अन्यथा {  /* open a new file */
    अचर वर्ण *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_पङ्क्तिहरू(L, toclose);  /* push iteration function */
  यदि (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    फर्काऊ 4;
  }
  अन्यथा
    फर्काऊ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#यदि !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
प्रकारनिर्देश संरचना {
  खाता *f;  /* file being read */
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
    फर्काऊ 0;  /* fail */
  }
  अन्यथा {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    फर्काऊ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
स्थिर पूर्णाङ्क test2 (RN *rn, अचर वर्ण *set) {
  यदि (rn->c == set[0] || rn->c == set[1])
    फर्काऊ nextc(rn);
  अन्यथा फर्काऊ 0;
}


/*
** Read a sequence of (hex)digits
*/
स्थिर पूर्णाङ्क readdigits (RN *rn, पूर्णाङ्क hex) {
  पूर्णाङ्क गन्ती = 0;
  जबसम्म ((hex ? है_षष्ठादशक(rn->c) : है_अंक(rn->c)) && nextc(rn))
    गन्ती++;
  फर्काऊ गन्ती;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
स्थिर पूर्णाङ्क read_सङ्ख्या (lua_State *L, खाता *f) {
  RN rn;
  पूर्णाङ्क गन्ती = 0;
  पूर्णाङ्क hex = 0;
  वर्ण decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  गर { rn.c = l_getc(rn.f); } जबसम्म (है_खाली(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  यदि (test2(&rn, "00")) {
    यदि (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    अन्यथा गन्ती = 1;  /* count initial '0' as a valid digit */
  }
  गन्ती += readdigits(&rn, hex);  /* integral part */
  यदि (test2(&rn, decp))  /* decimal point? */
    गन्ती += readdigits(&rn, hex);  /* fractional part */
  यदि (गन्ती > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  अक्षर_वापस(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  यदि (l_likely(lua_stringtonumber(L, rn.buff)))
    फर्काऊ 1;  /* ok, it is a valid number */
  अन्यथा {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   फर्काऊ 0;  /* read fails */
  }
}


स्थिर पूर्णाङ्क test_eof (lua_State *L, खाता *f) {
  पूर्णाङ्क c = अ_लो(f);
  अक्षर_वापस(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  फर्काऊ (c != खातापूर्ण);
}


स्थिर पूर्णाङ्क read_पङ्क्ति (lua_State *L, खाता *f, पूर्णाङ्क chop) {
  luaL_Buffer b;
  पूर्णाङ्क c;
  luaL_buffinit(L, &b);
  गर {  /* may need to read several chunks to get whole line */
    वर्ण *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    अचिन्हित i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    जबसम्म (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != खातापूर्ण && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } जबसम्म (c != खातापूर्ण && c != '\n');  /* repeat until end of line */
  यदि (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  फर्काऊ (c == '\n' || lua_rawlen(L, -1) > 0);
}


स्थिर शून्य read_all (lua_State *L, खाता *f) {
  माप_प्रकार nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  गर {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    वर्ण *p = luaL_prepbuffer(&b);
    nr = ख_पढ़ो(p, परिमाण(वर्ण), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } जबसम्म (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


स्थिर पूर्णाङ्क read_chars (lua_State *L, खाता *f, माप_प्रकार n) {
  माप_प्रकार nr;  /* number of chars actually read */
  वर्ण *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = ख_पढ़ो(p, परिमाण(वर्ण), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  फर्काऊ (nr > 0);  /* true iff read something */
}


स्थिर पूर्णाङ्क g_read (lua_State *L, खाता *f, पूर्णाङ्क first) {
  पूर्णाङ्क nargs = lua_gettop(L) - 1;
  पूर्णाङ्क n, success;
  साफ_त्रुटि(f);
  त्रुटि_सं = 0;
  यदि (nargs == 0) {  /* no arguments? */
    success = read_पङ्क्ति(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  अन्यथा {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    लागि (n = first; nargs-- && success; n++) {
      यदि (lua_type(L, n) == LUA_TNUMBER) {
        माप_प्रकार l = (माप_प्रकार)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      अन्यथा {
        अचर वर्ण *p = luaL_checkstring(L, n);
        यदि (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        विकल्प (*p) {
          पक्ष 'n':  /*सङ्ख्या*/
            success = read_सङ्ख्या(L, f);
            रोक;
          पक्ष 'l':  /*पङ्क्ति*/
            success = read_पङ्क्ति(L, f, 1);
            रोक;
          पक्ष 'L':  /* line with end-of-line */
            success = read_पङ्क्ति(L, f, 0);
            रोक;
          पक्ष 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            रोक;
          स्वभाव:
            फर्काऊ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  यदि (ख_त्रुटि(f))
    फर्काऊ luaL_fileresult(L, 0, रिक्त);
  यदि (!success) {
    lua_झिक(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  फर्काऊ n - first;
}


स्थिर पूर्णाङ्क io_read (lua_State *L) {
  फर्काऊ g_read(L, getiofile(L, IO_आगत), 1);
}


स्थिर पूर्णाङ्क f_read (lua_State *L) {
  फर्काऊ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
स्थिर पूर्णाङ्क io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  पूर्णाङ्क i;
  पूर्णाङ्क n = (पूर्णाङ्क)lua_tointeger(L, lua_upvalueindex(2));
  यदि (isclosed(p))  /* file is already closed? */
    फर्काऊ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  लागि (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  यदि (lua_toboolean(L, -n))  /* read at least one value? */
    फर्काऊ n;  /* return them */
  अन्यथा {  /* first result is false: EOF or error */
    यदि (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      फर्काऊ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    यदि (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    फर्काऊ 0;
  }
}

/* }====================================================== */


स्थिर पूर्णाङ्क g_write (lua_State *L, खाता *f, पूर्णाङ्क arg) {
  पूर्णाङ्क nargs = lua_gettop(L) - arg;
  माप_प्रकार totalbytes = 0;  /* total number of bytes written */
  त्रुटि_सं = 0;
  लागि (; nargs--; arg++) {  /* for each argument */
    वर्ण buff[LUA_N2SBUFFSZ];
    अचर वर्ण *s;
    माप_प्रकार numbytes;  /* bytes written in one call to 'fwrite' */
    माप_प्रकार len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    यदि (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    अन्यथा  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = ख_डालो(s, परिमाण(वर्ण), len, f);
    totalbytes += numbytes;
    यदि (numbytes < len) {  /* write error? */
      पूर्णाङ्क n = luaL_fileresult(L, 0, रिक्त);
      lua_pushinteger(L, cast_st2S(totalbytes));
      फर्काऊ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  फर्काऊ 1;  /* no errors; file handle already on stack top */
}


स्थिर पूर्णाङ्क io_write (lua_State *L) {
  फर्काऊ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


स्थिर पूर्णाङ्क f_write (lua_State *L) {
  खाता *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  फर्काऊ g_write(L, f, 2);
}


स्थिर पूर्णाङ्क f_seek (lua_State *L) {
  स्थिर अचर पूर्णाङ्क mode[] = {शुरू_से, प्रस्तुत_से, अंत_से};
  स्थिर अचर वर्ण *अचर modenames[] = {"set", "cur", "end", रिक्त};
  खाता *f = tofile(L);
  पूर्णाङ्क op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  त्रुटि_सं = 0;
  op = l_fseek(f, offset, mode[op]);
  यदि (l_unlikely(op))
    फर्काऊ luaL_fileresult(L, 0, रिक्त);  /* error */
  अन्यथा {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    फर्काऊ 1;
  }
}


स्थिर पूर्णाङ्क f_setvbuf (lua_State *L) {
  स्थिर अचर पूर्णाङ्क mode[] = {_IONBF, _IOFBF, _IOLBF};
  स्थिर अचर वर्ण *अचर modenames[] = {"no", "full", "line", रिक्त};
  खाता *f = tofile(L);
  पूर्णाङ्क op = luaL_checkoption(L, 2, रिक्त, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  पूर्णाङ्क res;
  त्रुटि_सं = 0;
  res = रखो_भबफ(f, रिक्त, mode[op], (माप_प्रकार)sz);
  फर्काऊ luaL_fileresult(L, res == 0, रिक्त);
}


स्थिर पूर्णाङ्क aux_flush (lua_State *L, खाता *f) {
  त्रुटि_सं = 0;
  फर्काऊ luaL_fileresult(L, ख_साफ(f) == 0, रिक्त);
}


स्थिर पूर्णाङ्क f_flush (lua_State *L) {
  फर्काऊ aux_flush(L, tofile(L));
}


स्थिर पूर्णाङ्क io_flush (lua_State *L) {
  फर्काऊ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
स्थिर अचर luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_आगत},
  {"lines", io_पङ्क्तिहरू},
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
स्थिर अचर luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_पङ्क्तिहरू},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {रिक्त, रिक्त}
};


/*
** metamethods for file handles
*/
स्थिर अचर luaL_Reg metameth[] = {
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
  lua_झिक(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
स्थिर पूर्णाङ्क io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  फर्काऊ 2;
}


स्थिर शून्य createstdfile (lua_State *L, खाता *f, अचर वर्ण *k,
                           अचर वर्ण *fname) {
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
  createstdfile(L, मानक_निवेश, IO_आगत, "stdin");
  createstdfile(L, मानक_निकास, IO_OUTPUT, "stdout");
  createstdfile(L, मानक_त्रुटि, रिक्त, "stderr");
  फर्काऊ 1;
}

