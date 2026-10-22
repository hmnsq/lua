/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#घोषणा liolib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <locale.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#अगर !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#अगर !defined(L_MODEEXT)
#घोषणा L_MODEEXT	"b"
#पूर्ण_अगर

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
अटल पूर्णांक l_checkmode (स्थिर अक्षर *mode) {
  वापस (*mode != '\0' && म_अक्षर("rwa", *(mode++)) != शून्य &&
         (*mode != '+' || ((व्योम)(++mode), 1)) &&  /* skip if char is '+' */
         (म_अखोज(mode, L_MODEEXT) == म_माप(mode)));  /* check extensions */
}

#पूर्ण_अगर

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#अगर !defined(l_popen)		/* { */

#अगर defined(LUA_USE_POSIX)	/* { */

#घोषणा l_popen(L,c,m)		(ख_साफ(शून्य), popen(c,m))
#घोषणा l_pclose(L,file)	(pclose(file))

#या_अगर defined(LUA_USE_WINDOWS)	/* }{ */

#घोषणा l_popen(L,c,m)		(_popen(c,m))
#घोषणा l_pclose(L,file)	(_pclose(file))

#अगर !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#घोषणा l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#पूर्ण_अगर

#अन्यथा				/* }{ */

/* ISO C definitions */
#घोषणा l_popen(L,c,m)  \
	  ((व्योम)c, (व्योम)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (खाता*)0)
#घोषणा l_pclose(L,file)		((व्योम)L, (व्योम)file, -1)

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */


#अगर !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#घोषणा l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#पूर्ण_अगर

/* }====================================================== */


#अगर !defined(l_getc)		/* { */

#अगर defined(LUA_USE_POSIX)
#घोषणा l_getc(f)		getc_unlocked(f)
#घोषणा l_lockfile(f)		flockfile(f)
#घोषणा l_unlockfile(f)		funlockfile(f)
#अन्यथा
#घोषणा l_getc(f)		अ_लो(f)
#घोषणा l_lockfile(f)		((व्योम)0)
#घोषणा l_unlockfile(f)		((व्योम)0)
#पूर्ण_अगर

#पूर्ण_अगर				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#अगर !defined(l_fseek)		/* { */

#अगर defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#समावेश <sys/types.h>

#घोषणा l_fseek(f,o,w)		fseeko(f,o,w)
#घोषणा l_ftell(f)		ftello(f)
#घोषणा l_seeknum		off_t

#या_अगर defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#घोषणा l_fseek(f,o,w)		_fseeki64(f,o,w)
#घोषणा l_ftell(f)		_ftelli64(f)
#घोषणा l_seeknum		__int64

#अन्यथा				/* }{ */

/* ISO C definitions */
#घोषणा l_fseek(f,o,w)		ख_जाओ(f,o,w)
#घोषणा l_ftell(f)		ख_बताओ(f)
#घोषणा l_seeknum		दीर्घ

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */

/* }====================================================== */



#घोषणा IO_PREFIX	"_IO_"
#घोषणा IOPREF_LEN	(माप(IO_PREFIX)/माप(अक्षर) - 1)
#घोषणा IO_निवेश	(IO_PREFIX "input")
#घोषणा IO_OUTPUT	(IO_PREFIX "output")


प्रकार luaL_Stream LStream;


#घोषणा tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#घोषणा isclosed(p)	((p)->closef == शून्य)


अटल पूर्णांक io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  अगर (p == शून्य)
    luaL_pushfail(L);  /* not a file */
  अन्यथा अगर (isclosed(p))
    lua_pushliteral(L, "closed file");
  अन्यथा
    lua_pushliteral(L, "file");
  वापस 1;
}


अटल पूर्णांक f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  अगर (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  अन्यथा
    lua_pushfstring(L, "file (%p)", p->f);
  वापस 1;
}


अटल खाता *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  अगर (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  वापस p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
अटल LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, माप(LStream), 0);
  p->closef = शून्य;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  वापस p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
अटल पूर्णांक aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  अस्थिर lua_CFunction cf = p->closef;
  p->closef = शून्य;  /* mark stream as closed */
  वापस (*cf)(L);  /* close it */
}


अटल पूर्णांक f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  वापस aux_close(L);
}


अटल पूर्णांक io_close (lua_State *L) {
  अगर (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  वापस f_close(L);
}


अटल पूर्णांक f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  अगर (!isclosed(p) && p->f != शून्य)
    aux_close(L);  /* ignore closed and incompletely open files */
  वापस 0;
}


/*
** function to close regular files
*/
अटल पूर्णांक io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  वापस luaL_fileresult(L, (ख_बंद(p->f) == 0), शून्य);
}


अटल LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = शून्य;
  p->closef = &io_fclose;
  वापस p;
}


अटल व्योम opencheck (lua_State *L, स्थिर अक्षर *fname, स्थिर अक्षर *mode) {
  LStream *p = newfile(L);
  p->f = ख_खोलो(fname, mode);
  अगर (l_unlikely(p->f == शून्य))
    luaL_error(L, "cannot open file '%s' (%s)", fname, म_त्रुटि(त्रुटि_सं));
}


अटल पूर्णांक io_open (lua_State *L) {
  स्थिर अक्षर *filename = luaL_checkstring(L, 1);
  स्थिर अक्षर *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  स्थिर अक्षर *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = ख_खोलो(filename, mode);
  वापस (p->f == शून्य) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
अटल पूर्णांक io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  त्रुटि_सं = 0;
  वापस luaL_execresult(L, l_pclose(L, p->f));
}


अटल पूर्णांक io_popen (lua_State *L) {
  स्थिर अक्षर *filename = luaL_checkstring(L, 1);
  स्थिर अक्षर *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  त्रुटि_सं = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  वापस (p->f == शून्य) ? luaL_fileresult(L, 0, filename) : 1;
}


अटल पूर्णांक io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  त्रुटि_सं = 0;
  p->f = क्षणिक_ख();
  वापस (p->f == शून्य) ? luaL_fileresult(L, 0, शून्य) : 1;
}


अटल खाता *getiofile (lua_State *L, स्थिर अक्षर *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  अगर (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  वापस p->f;
}


अटल पूर्णांक g_iofile (lua_State *L, स्थिर अक्षर *f, स्थिर अक्षर *mode) {
  अगर (!lua_isnoneornil(L, 1)) {
    स्थिर अक्षर *filename = lua_tostring(L, 1);
    अगर (filename)
      opencheck(L, filename, mode);
    अन्यथा {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  वापस 1;
}


अटल पूर्णांक io_निवेश (lua_State *L) {
  वापस g_iofile(L, IO_निवेश, "r");
}


अटल पूर्णांक io_output (lua_State *L) {
  वापस g_iofile(L, IO_OUTPUT, "w");
}


अटल पूर्णांक io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#घोषणा MAXARGLINE	250

/*
** Auxiliary function to create the iteration function for 'lines'.
** The iteration function is a closure over 'io_readline', with
** the following upvalues:
** 1) The file being read (first value in the stack)
** 2) the number of arguments to read
** 3) a boolean, true iff file has to be closed when finished ('toclose')
** *) a variable number of format arguments (rest of the stack)
*/
अटल व्योम aux_पंक्तियाँ (lua_State *L, पूर्णांक toclose) {
  पूर्णांक n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


अटल पूर्णांक f_पंक्तियाँ (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_पंक्तियाँ(L, 0);
  वापस 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
अटल पूर्णांक io_पंक्तियाँ (lua_State *L) {
  पूर्णांक toclose;
  अगर (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  अगर (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_निवेश);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  अन्यथा {  /* open a new file */
    स्थिर अक्षर *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_पंक्तियाँ(L, toclose);  /* push iteration function */
  अगर (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    वापस 4;
  }
  अन्यथा
    वापस 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#अगर !defined (L_MAXLENNUM)
#घोषणा L_MAXLENNUM     200
#पूर्ण_अगर


/* auxiliary structure used by 'read_number' */
प्रकार काष्ठा {
  खाता *f;  /* file being read */
  पूर्णांक c;  /* current character (look ahead) */
  पूर्णांक n;  /* number of elements in buffer 'buff' */
  अक्षर buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
अटल पूर्णांक nextc (RN *rn) {
  अगर (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    वापस 0;  /* fail */
  }
  अन्यथा {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    वापस 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
अटल पूर्णांक test2 (RN *rn, स्थिर अक्षर *set) {
  अगर (rn->c == set[0] || rn->c == set[1])
    वापस nextc(rn);
  अन्यथा वापस 0;
}


/*
** Read a sequence of (hex)digits
*/
अटल पूर्णांक readdigits (RN *rn, पूर्णांक hex) {
  पूर्णांक गिनती = 0;
  जबतक ((hex ? है_षष्ठादशक(rn->c) : है_अंक(rn->c)) && nextc(rn))
    गिनती++;
  वापस गिनती;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
अटल पूर्णांक read_संख्या (lua_State *L, खाता *f) {
  RN rn;
  पूर्णांक गिनती = 0;
  पूर्णांक hex = 0;
  अक्षर decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  करो { rn.c = l_getc(rn.f); } जबतक (है_खाली(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  अगर (test2(&rn, "00")) {
    अगर (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    अन्यथा गिनती = 1;  /* count initial '0' as a valid digit */
  }
  गिनती += readdigits(&rn, hex);  /* integral part */
  अगर (test2(&rn, decp))  /* decimal point? */
    गिनती += readdigits(&rn, hex);  /* fractional part */
  अगर (गिनती > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  अक्षर_वापस(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  अगर (l_likely(lua_stringtonumber(L, rn.buff)))
    वापस 1;  /* ok, it is a valid number */
  अन्यथा {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   वापस 0;  /* read fails */
  }
}


अटल पूर्णांक test_eof (lua_State *L, खाता *f) {
  पूर्णांक c = अ_लो(f);
  अक्षर_वापस(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  वापस (c != खातापूर्ण);
}


अटल पूर्णांक read_पंक्ति (lua_State *L, खाता *f, पूर्णांक chop) {
  lua_L_बफर b;
  पूर्णांक c;
  luaL_buffinit(L, &b);
  करो {  /* may need to read several chunks to get whole line */
    अक्षर *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    अचिन्हित i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    जबतक (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != खातापूर्ण && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } जबतक (c != खातापूर्ण && c != '\n');  /* repeat until end of line */
  अगर (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  वापस (c == '\n' || lua_rawlen(L, -1) > 0);
}


अटल व्योम read_all (lua_State *L, खाता *f) {
  माप_प्रकार nr;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  करो {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    अक्षर *p = luaL_prepbuffer(&b);
    nr = ख_पढ़ो(p, माप(अक्षर), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } जबतक (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


अटल पूर्णांक read_chars (lua_State *L, खाता *f, माप_प्रकार n) {
  माप_प्रकार nr;  /* number of chars actually read */
  अक्षर *p;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = ख_पढ़ो(p, माप(अक्षर), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  वापस (nr > 0);  /* true iff read something */
}


अटल पूर्णांक g_read (lua_State *L, खाता *f, पूर्णांक first) {
  पूर्णांक nargs = lua_gettop(L) - 1;
  पूर्णांक n, success;
  साफ_त्रुटि(f);
  त्रुटि_सं = 0;
  अगर (nargs == 0) {  /* no arguments? */
    success = read_पंक्ति(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  अन्यथा {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    क्रम (n = first; nargs-- && success; n++) {
      अगर (lua_type(L, n) == LUA_TNUMBER) {
        माप_प्रकार l = (माप_प्रकार)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      अन्यथा {
        स्थिर अक्षर *p = luaL_checkstring(L, n);
        अगर (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        चयन (*p) {
          हाल 'n':  /*संख्या*/
            success = read_संख्या(L, f);
            अवरोध;
          हाल 'l':  /*पंक्ति*/
            success = read_पंक्ति(L, f, 1);
            अवरोध;
          हाल 'L':  /* line with end-of-line */
            success = read_पंक्ति(L, f, 0);
            अवरोध;
          हाल 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            अवरोध;
          शेष:
            वापस luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  अगर (ख_त्रुटि(f))
    वापस luaL_fileresult(L, 0, शून्य);
  अगर (!success) {
    lua_निकालो(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  वापस n - first;
}


अटल पूर्णांक io_read (lua_State *L) {
  वापस g_read(L, getiofile(L, IO_निवेश), 1);
}


अटल पूर्णांक f_read (lua_State *L) {
  वापस g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
अटल पूर्णांक io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  पूर्णांक i;
  पूर्णांक n = (पूर्णांक)lua_tointeger(L, lua_upvalueindex(2));
  अगर (isclosed(p))  /* file is already closed? */
    वापस luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  क्रम (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  अगर (lua_toboolean(L, -n))  /* read at least one value? */
    वापस n;  /* return them */
  अन्यथा {  /* first result is false: EOF or error */
    अगर (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      वापस luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    अगर (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    वापस 0;
  }
}

/* }====================================================== */


अटल पूर्णांक g_write (lua_State *L, खाता *f, पूर्णांक arg) {
  पूर्णांक nargs = lua_gettop(L) - arg;
  माप_प्रकार totalbytes = 0;  /* total number of bytes written */
  त्रुटि_सं = 0;
  क्रम (; nargs--; arg++) {  /* for each argument */
    अक्षर buff[LUA_N2SBUFFSZ];
    स्थिर अक्षर *s;
    माप_प्रकार numbytes;  /* bytes written in one call to 'fwrite' */
    माप_प्रकार len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    अगर (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    अन्यथा  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = ख_डालो(s, माप(अक्षर), len, f);
    totalbytes += numbytes;
    अगर (numbytes < len) {  /* write error? */
      पूर्णांक n = luaL_fileresult(L, 0, शून्य);
      lua_pushinteger(L, cast_st2S(totalbytes));
      वापस n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  वापस 1;  /* no errors; file handle already on stack top */
}


अटल पूर्णांक io_write (lua_State *L) {
  वापस g_write(L, getiofile(L, IO_OUTPUT), 1);
}


अटल पूर्णांक f_write (lua_State *L) {
  खाता *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  वापस g_write(L, f, 2);
}


अटल पूर्णांक f_seek (lua_State *L) {
  अटल स्थिर पूर्णांक mode[] = {शुरू_से, प्रस्तुत_से, अंत_से};
  अटल स्थिर अक्षर *स्थिर modenames[] = {"set", "cur", "end", शून्य};
  खाता *f = tofile(L);
  पूर्णांक op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  त्रुटि_सं = 0;
  op = l_fseek(f, offset, mode[op]);
  अगर (l_unlikely(op))
    वापस luaL_fileresult(L, 0, शून्य);  /* error */
  अन्यथा {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    वापस 1;
  }
}


अटल पूर्णांक f_setvbuf (lua_State *L) {
  अटल स्थिर पूर्णांक mode[] = {_IONBF, _IOFBF, _IOLBF};
  अटल स्थिर अक्षर *स्थिर modenames[] = {"no", "full", "line", शून्य};
  खाता *f = tofile(L);
  पूर्णांक op = luaL_checkoption(L, 2, शून्य, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  पूर्णांक res;
  त्रुटि_सं = 0;
  res = रखो_भबफ(f, शून्य, mode[op], (माप_प्रकार)sz);
  वापस luaL_fileresult(L, res == 0, शून्य);
}


अटल पूर्णांक aux_flush (lua_State *L, खाता *f) {
  त्रुटि_सं = 0;
  वापस luaL_fileresult(L, ख_साफ(f) == 0, शून्य);
}


अटल पूर्णांक f_flush (lua_State *L) {
  वापस aux_flush(L, tofile(L));
}


अटल पूर्णांक io_flush (lua_State *L) {
  वापस aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
अटल स्थिर luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_निवेश},
  {"lines", io_पंक्तियाँ},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {शून्य, शून्य}
};


/*
** methods for file handles
*/
अटल स्थिर luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_पंक्तियाँ},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {शून्य, शून्य}
};


/*
** metamethods for file handles
*/
अटल स्थिर luaL_Reg metameth[] = {
  {"__index", शून्य},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {शून्य, शून्य}
};


अटल व्योम createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_निकालो(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
अटल पूर्णांक io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  वापस 2;
}


अटल व्योम createstdfile (lua_State *L, खाता *f, स्थिर अक्षर *k,
                           स्थिर अक्षर *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  अगर (k != शून्य) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API पूर्णांक luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, मानक_निवेश, IO_निवेश, "stdin");
  createstdfile(L, मानक_निकास, IO_OUTPUT, "stdout");
  createstdfile(L, मानक_त्रुटि, शून्य, "stderr");
  वापस 1;
}

