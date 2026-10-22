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
#जर !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#जर !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
स्थिर पूर्णांक l_checkmode (स्थिरांक अक्षर *mode) {
  परत (*mode != '\0' && म_अक्षर("rwa", *(mode++)) != रिक्त &&
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

#जर !defined(l_popen)		/* { */

#जर defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(ख_साफ(रिक्त), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#किंवाजर defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#जर !defined(l_checkmodep)
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


#जर !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#जर !defined(l_getc)		/* { */

#जर defined(LUA_USE_POSIX)
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

#जर !defined(l_fseek)		/* { */

#जर defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#अन्तर्भाव <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#किंवाजर defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
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
#define IOPREF_LEN	(आकार(IO_PREFIX)/आकार(अक्षर) - 1)
#define IO_आदान	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


प्रकारव्याख्या luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == रिक्त)


स्थिर पूर्णांक io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  जर (p == रिक्त)
    luaL_pushfail(L);  /* not a file */
  अन्यथा जर (isclosed(p))
    lua_pushliteral(L, "closed file");
  अन्यथा
    lua_pushliteral(L, "file");
  परत 1;
}


स्थिर पूर्णांक f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  जर (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  अन्यथा
    lua_pushfstring(L, "file (%p)", p->f);
  परत 1;
}


स्थिर खाता *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  जर (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  परत p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
स्थिर LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, आकार(LStream), 0);
  p->closef = रिक्त;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  परत p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
स्थिर पूर्णांक aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  अस्थिर lua_CFunction cf = p->closef;
  p->closef = रिक्त;  /* mark stream as closed */
  परत (*cf)(L);  /* close it */
}


स्थिर पूर्णांक f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  परत aux_close(L);
}


स्थिर पूर्णांक io_close (lua_State *L) {
  जर (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  परत f_close(L);
}


स्थिर पूर्णांक f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  जर (!isclosed(p) && p->f != रिक्त)
    aux_close(L);  /* ignore closed and incompletely open files */
  परत 0;
}


/*
** function to close regular files
*/
स्थिर पूर्णांक io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  परत luaL_fileresult(L, (ख_बंद(p->f) == 0), रिक्त);
}


स्थिर LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = रिक्त;
  p->closef = &io_fclose;
  परत p;
}


स्थिर शून्य opencheck (lua_State *L, स्थिरांक अक्षर *fname, स्थिरांक अक्षर *mode) {
  LStream *p = newfile(L);
  p->f = ख_खोलो(fname, mode);
  जर (l_unlikely(p->f == रिक्त))
    luaL_error(L, "cannot open file '%s' (%s)", fname, म_त्रुटि(त्रुटि_सं));
}


स्थिर पूर्णांक io_open (lua_State *L) {
  स्थिरांक अक्षर *filename = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  स्थिरांक अक्षर *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = ख_खोलो(filename, mode);
  परत (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
स्थिर पूर्णांक io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  परत luaL_execresult(L, l_pclose(L, p->f));
}


स्थिर पूर्णांक io_popen (lua_State *L) {
  स्थिरांक अक्षर *filename = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  परत (p->f == रिक्त) ? luaL_fileresult(L, 0, filename) : 1;
}


स्थिर पूर्णांक io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  त्रुटि_सं = 0;
  p->f = क्षणिक_ख();
  परत (p->f == रिक्त) ? luaL_fileresult(L, 0, रिक्त) : 1;
}


स्थिर खाता *getiofile (lua_State *L, स्थिरांक अक्षर *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  जर (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  परत p->f;
}


स्थिर पूर्णांक g_iofile (lua_State *L, स्थिरांक अक्षर *f, स्थिरांक अक्षर *mode) {
  जर (!lua_isnoneornil(L, 1)) {
    स्थिरांक अक्षर *filename = lua_tostring(L, 1);
    जर (filename)
      opencheck(L, filename, mode);
    अन्यथा {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  परत 1;
}


स्थिर पूर्णांक io_आदान (lua_State *L) {
  परत g_iofile(L, IO_आदान, "r");
}


स्थिर पूर्णांक io_output (lua_State *L) {
  परत g_iofile(L, IO_OUTPUT, "w");
}


स्थिर पूर्णांक io_readline (lua_State *L);


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
स्थिर शून्य aux_ओळी (lua_State *L, पूर्णांक toclose) {
  पूर्णांक n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


स्थिर पूर्णांक f_ओळी (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ओळी(L, 0);
  परत 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
स्थिर पूर्णांक io_ओळी (lua_State *L) {
  पूर्णांक toclose;
  जर (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  जर (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_आदान);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  अन्यथा {  /* open a new file */
    स्थिरांक अक्षर *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ओळी(L, toclose);  /* push iteration function */
  जर (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    परत 4;
  }
  अन्यथा
    परत 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#जर !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
प्रकारव्याख्या संरचना {
  खाता *f;  /* file being read */
  पूर्णांक c;  /* current character (look ahead) */
  पूर्णांक n;  /* number of elements in buffer 'buff' */
  अक्षर buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
स्थिर पूर्णांक nextc (RN *rn) {
  जर (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    परत 0;  /* fail */
  }
  अन्यथा {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    परत 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
स्थिर पूर्णांक test2 (RN *rn, स्थिरांक अक्षर *set) {
  जर (rn->c == set[0] || rn->c == set[1])
    परत nextc(rn);
  अन्यथा परत 0;
}


/*
** Read a sequence of (hex)digits
*/
स्थिर पूर्णांक readdigits (RN *rn, पूर्णांक hex) {
  पूर्णांक मोजणी = 0;
  पर्यंत ((hex ? है_षष्ठादशक(rn->c) : है_अंक(rn->c)) && nextc(rn))
    मोजणी++;
  परत मोजणी;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
स्थिर पूर्णांक read_संख्या (lua_State *L, खाता *f) {
  RN rn;
  पूर्णांक मोजणी = 0;
  पूर्णांक hex = 0;
  अक्षर decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  करा { rn.c = l_getc(rn.f); } पर्यंत (है_खाली(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  जर (test2(&rn, "00")) {
    जर (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    अन्यथा मोजणी = 1;  /* count initial '0' as a valid digit */
  }
  मोजणी += readdigits(&rn, hex);  /* integral part */
  जर (test2(&rn, decp))  /* decimal point? */
    मोजणी += readdigits(&rn, hex);  /* fractional part */
  जर (मोजणी > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  अक्षर_वापस(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  जर (l_likely(lua_stringtonumber(L, rn.buff)))
    परत 1;  /* ok, it is a valid number */
  अन्यथा {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   परत 0;  /* read fails */
  }
}


स्थिर पूर्णांक test_eof (lua_State *L, खाता *f) {
  पूर्णांक c = अ_लो(f);
  अक्षर_वापस(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  परत (c != खातापूर्ण);
}


स्थिर पूर्णांक read_ओळ (lua_State *L, खाता *f, पूर्णांक chop) {
  luaL_Buffer b;
  पूर्णांक c;
  luaL_buffinit(L, &b);
  करा {  /* may need to read several chunks to get whole line */
    अक्षर *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    अचिन्हित i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    पर्यंत (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != खातापूर्ण && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } पर्यंत (c != खातापूर्ण && c != '\n');  /* repeat until end of line */
  जर (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  परत (c == '\n' || lua_rawlen(L, -1) > 0);
}


स्थिर शून्य read_all (lua_State *L, खाता *f) {
  माप_प्रकार nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  करा {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    अक्षर *p = luaL_prepbuffer(&b);
    nr = ख_पढ़ो(p, आकार(अक्षर), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } पर्यंत (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


स्थिर पूर्णांक read_chars (lua_State *L, खाता *f, माप_प्रकार n) {
  माप_प्रकार nr;  /* number of chars actually read */
  अक्षर *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = ख_पढ़ो(p, आकार(अक्षर), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  परत (nr > 0);  /* true iff read something */
}


स्थिर पूर्णांक g_read (lua_State *L, खाता *f, पूर्णांक first) {
  पूर्णांक nargs = lua_gettop(L) - 1;
  पूर्णांक n, success;
  साफ_त्रुटि(f);
  त्रुटि_सं = 0;
  जर (nargs == 0) {  /* no arguments? */
    success = read_ओळ(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  अन्यथा {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    साठी (n = first; nargs-- && success; n++) {
      जर (lua_type(L, n) == LUA_TNUMBER) {
        माप_प्रकार l = (माप_प्रकार)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      अन्यथा {
        स्थिरांक अक्षर *p = luaL_checkstring(L, n);
        जर (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        निवड (*p) {
          स्थिती 'n':  /*संख्या*/
            success = read_संख्या(L, f);
            थांबा;
          स्थिती 'l':  /*ओळ*/
            success = read_ओळ(L, f, 1);
            थांबा;
          स्थिती 'L':  /* line with end-of-line */
            success = read_ओळ(L, f, 0);
            थांबा;
          स्थिती 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            थांबा;
          पूर्वनिर्धारित:
            परत luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  जर (ख_त्रुटि(f))
    परत luaL_fileresult(L, 0, रिक्त);
  जर (!success) {
    lua_काढा(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  परत n - first;
}


स्थिर पूर्णांक io_read (lua_State *L) {
  परत g_read(L, getiofile(L, IO_आदान), 1);
}


स्थिर पूर्णांक f_read (lua_State *L) {
  परत g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
स्थिर पूर्णांक io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  पूर्णांक i;
  पूर्णांक n = (पूर्णांक)lua_tointeger(L, lua_upvalueindex(2));
  जर (isclosed(p))  /* file is already closed? */
    परत luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  साठी (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  जर (lua_toboolean(L, -n))  /* read at least one value? */
    परत n;  /* return them */
  अन्यथा {  /* first result is false: EOF or error */
    जर (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      परत luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    जर (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    परत 0;
  }
}

/* }====================================================== */


स्थिर पूर्णांक g_write (lua_State *L, खाता *f, पूर्णांक arg) {
  पूर्णांक nargs = lua_gettop(L) - arg;
  माप_प्रकार totalbytes = 0;  /* total number of bytes written */
  त्रुटि_सं = 0;
  साठी (; nargs--; arg++) {  /* for each argument */
    अक्षर buff[LUA_N2SBUFFSZ];
    स्थिरांक अक्षर *s;
    माप_प्रकार numbytes;  /* bytes written in one call to 'fwrite' */
    माप_प्रकार len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    जर (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    अन्यथा  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = ख_डालो(s, आकार(अक्षर), len, f);
    totalbytes += numbytes;
    जर (numbytes < len) {  /* write error? */
      पूर्णांक n = luaL_fileresult(L, 0, रिक्त);
      lua_pushinteger(L, cast_st2S(totalbytes));
      परत n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  परत 1;  /* no errors; file handle already on stack top */
}


स्थिर पूर्णांक io_write (lua_State *L) {
  परत g_write(L, getiofile(L, IO_OUTPUT), 1);
}


स्थिर पूर्णांक f_write (lua_State *L) {
  खाता *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  परत g_write(L, f, 2);
}


स्थिर पूर्णांक f_seek (lua_State *L) {
  स्थिर स्थिरांक पूर्णांक mode[] = {शुरू_से, प्रस्तुत_से, अंत_से};
  स्थिर स्थिरांक अक्षर *स्थिरांक modenames[] = {"set", "cur", "end", रिक्त};
  खाता *f = tofile(L);
  पूर्णांक op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  त्रुटि_सं = 0;
  op = l_fseek(f, offset, mode[op]);
  जर (l_unlikely(op))
    परत luaL_fileresult(L, 0, रिक्त);  /* error */
  अन्यथा {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    परत 1;
  }
}


स्थिर पूर्णांक f_setvbuf (lua_State *L) {
  स्थिर स्थिरांक पूर्णांक mode[] = {_IONBF, _IOFBF, _IOLBF};
  स्थिर स्थिरांक अक्षर *स्थिरांक modenames[] = {"no", "full", "line", रिक्त};
  खाता *f = tofile(L);
  पूर्णांक op = luaL_checkoption(L, 2, रिक्त, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  पूर्णांक res;
  त्रुटि_सं = 0;
  res = रखो_भबफ(f, रिक्त, mode[op], (माप_प्रकार)sz);
  परत luaL_fileresult(L, res == 0, रिक्त);
}


स्थिर पूर्णांक aux_flush (lua_State *L, खाता *f) {
  त्रुटि_सं = 0;
  परत luaL_fileresult(L, ख_साफ(f) == 0, रिक्त);
}


स्थिर पूर्णांक f_flush (lua_State *L) {
  परत aux_flush(L, tofile(L));
}


स्थिर पूर्णांक io_flush (lua_State *L) {
  परत aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
स्थिर स्थिरांक luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_आदान},
  {"lines", io_ओळी},
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
स्थिर स्थिरांक luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ओळी},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {रिक्त, रिक्त}
};


/*
** metamethods for file handles
*/
स्थिर स्थिरांक luaL_Reg metameth[] = {
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
  lua_काढा(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
स्थिर पूर्णांक io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  परत 2;
}


स्थिर शून्य createstdfile (lua_State *L, खाता *f, स्थिरांक अक्षर *k,
                           स्थिरांक अक्षर *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  जर (k != रिक्त) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API पूर्णांक luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, मानक_निवेश, IO_आदान, "stdin");
  createstdfile(L, मानक_निकास, IO_OUTPUT, "stdout");
  createstdfile(L, मानक_त्रुटि, रिक्त, "stderr");
  परत 1;
}

