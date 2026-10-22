/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#જો !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#જો !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
સ્થિર પૂર્ણાંક l_checkmode (અચલ અક્ષર *mode) {
  પાછા (*mode != '\0' && strchr("rwa", *(mode++)) != ખાલી &&
         (*mode != '+' || ((શૂન્ય)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#જો !defined(l_popen)		/* { */

#જો defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(ખાલી), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#અથવાજો defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#જો !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#નહીંતર				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((શૂન્ય)c, (શૂન્ય)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((શૂન્ય)L, (શૂન્ય)file, -1)

#endif				/* } */

#endif				/* } */


#જો !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#જો !defined(l_getc)		/* { */

#જો defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#નહીંતર
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((શૂન્ય)0)
#define l_unlockfile(f)		((શૂન્ય)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#જો !defined(l_fseek)		/* { */

#જો defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#અન્તર્ભાવ <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#અથવાજો defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#નહીંતર				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(કદ(IO_PREFIX)/કદ(અક્ષર) - 1)
#define IO_આગત	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


પ્રકારવ્યાખ્યા luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == ખાલી)


સ્થિર પૂર્ણાંક io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  જો (p == ખાલી)
    luaL_pushfail(L);  /* not a file */
  નહીંતર જો (isclosed(p))
    lua_pushliteral(L, "closed file");
  નહીંતર
    lua_pushliteral(L, "file");
  પાછા 1;
}


સ્થિર પૂર્ણાંક f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  જો (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  નહીંતર
    lua_pushfstring(L, "file (%p)", p->f);
  પાછા 1;
}


સ્થિર FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  જો (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  પાછા p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
સ્થિર LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, કદ(LStream), 0);
  p->closef = ખાલી;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  પાછા p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
સ્થિર પૂર્ણાંક aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  અસ્થિર lua_CFunction cf = p->closef;
  p->closef = ખાલી;  /* mark stream as closed */
  પાછા (*cf)(L);  /* close it */
}


સ્થિર પૂર્ણાંક f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  પાછા aux_close(L);
}


સ્થિર પૂર્ણાંક io_close (lua_State *L) {
  જો (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  પાછા f_close(L);
}


સ્થિર પૂર્ણાંક f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  જો (!isclosed(p) && p->f != ખાલી)
    aux_close(L);  /* ignore closed and incompletely open files */
  પાછા 0;
}


/*
** function to close regular files
*/
સ્થિર પૂર્ણાંક io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  પાછા luaL_fileresult(L, (fclose(p->f) == 0), ખાલી);
}


સ્થિર LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ખાલી;
  p->closef = &io_fclose;
  પાછા p;
}


સ્થિર શૂન્ય opencheck (lua_State *L, અચલ અક્ષર *fname, અચલ અક્ષર *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  જો (l_unlikely(p->f == ખાલી))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


સ્થિર પૂર્ણાંક io_open (lua_State *L) {
  અચલ અક્ષર *filename = luaL_checkstring(L, 1);
  અચલ અક્ષર *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  અચલ અક્ષર *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  પાછા (p->f == ખાલી) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
સ્થિર પૂર્ણાંક io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  પાછા luaL_execresult(L, l_pclose(L, p->f));
}


સ્થિર પૂર્ણાંક io_popen (lua_State *L) {
  અચલ અક્ષર *filename = luaL_checkstring(L, 1);
  અચલ અક્ષર *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  પાછા (p->f == ખાલી) ? luaL_fileresult(L, 0, filename) : 1;
}


સ્થિર પૂર્ણાંક io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  પાછા (p->f == ખાલી) ? luaL_fileresult(L, 0, ખાલી) : 1;
}


સ્થિર FILE *getiofile (lua_State *L, અચલ અક્ષર *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  જો (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  પાછા p->f;
}


સ્થિર પૂર્ણાંક g_iofile (lua_State *L, અચલ અક્ષર *f, અચલ અક્ષર *mode) {
  જો (!lua_isnoneornil(L, 1)) {
    અચલ અક્ષર *filename = lua_tostring(L, 1);
    જો (filename)
      opencheck(L, filename, mode);
    નહીંતર {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  પાછા 1;
}


સ્થિર પૂર્ણાંક io_આગત (lua_State *L) {
  પાછા g_iofile(L, IO_આગત, "r");
}


સ્થિર પૂર્ણાંક io_output (lua_State *L) {
  પાછા g_iofile(L, IO_OUTPUT, "w");
}


સ્થિર પૂર્ણાંક io_readline (lua_State *L);


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
સ્થિર શૂન્ય aux_લીટીઓ (lua_State *L, પૂર્ણાંક toclose) {
  પૂર્ણાંક n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


સ્થિર પૂર્ણાંક f_લીટીઓ (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_લીટીઓ(L, 0);
  પાછા 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
સ્થિર પૂર્ણાંક io_લીટીઓ (lua_State *L) {
  પૂર્ણાંક toclose;
  જો (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  જો (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_આગત);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  નહીંતર {  /* open a new file */
    અચલ અક્ષર *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_લીટીઓ(L, toclose);  /* push iteration function */
  જો (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    પાછા 4;
  }
  નહીંતર
    પાછા 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#જો !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
પ્રકારવ્યાખ્યા સંરચના {
  FILE *f;  /* file being read */
  પૂર્ણાંક c;  /* current character (look ahead) */
  પૂર્ણાંક n;  /* number of elements in buffer 'buff' */
  અક્ષર buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
સ્થિર પૂર્ણાંક nextc (RN *rn) {
  જો (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    પાછા 0;  /* fail */
  }
  નહીંતર {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    પાછા 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
સ્થિર પૂર્ણાંક test2 (RN *rn, અચલ અક્ષર *set) {
  જો (rn->c == set[0] || rn->c == set[1])
    પાછા nextc(rn);
  નહીંતર પાછા 0;
}


/*
** Read a sequence of (hex)digits
*/
સ્થિર પૂર્ણાંક readdigits (RN *rn, પૂર્ણાંક hex) {
  પૂર્ણાંક ગણતરી = 0;
  જ્યારે ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    ગણતરી++;
  પાછા ગણતરી;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
સ્થિર પૂર્ણાંક read_સંખ્યા (lua_State *L, FILE *f) {
  RN rn;
  પૂર્ણાંક ગણતરી = 0;
  પૂર્ણાંક hex = 0;
  અક્ષર decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  કરો { rn.c = l_getc(rn.f); } જ્યારે (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  જો (test2(&rn, "00")) {
    જો (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    નહીંતર ગણતરી = 1;  /* count initial '0' as a valid digit */
  }
  ગણતરી += readdigits(&rn, hex);  /* integral part */
  જો (test2(&rn, decp))  /* decimal point? */
    ગણતરી += readdigits(&rn, hex);  /* fractional part */
  જો (ગણતરી > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  જો (l_likely(lua_stringtonumber(L, rn.buff)))
    પાછા 1;  /* ok, it is a valid number */
  નહીંતર {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   પાછા 0;  /* read fails */
  }
}


સ્થિર પૂર્ણાંક test_eof (lua_State *L, FILE *f) {
  પૂર્ણાંક c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  પાછા (c != EOF);
}


સ્થિર પૂર્ણાંક read_લીટી (lua_State *L, FILE *f, પૂર્ણાંક chop) {
  luaL_Buffer b;
  પૂર્ણાંક c;
  luaL_buffinit(L, &b);
  કરો {  /* may need to read several chunks to get whole line */
    અક્ષર *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    જ્યારે (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } જ્યારે (c != EOF && c != '\n');  /* repeat until end of line */
  જો (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  પાછા (c == '\n' || lua_rawlen(L, -1) > 0);
}


સ્થિર શૂન્ય read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  કરો {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    અક્ષર *p = luaL_prepbuffer(&b);
    nr = fread(p, કદ(અક્ષર), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } જ્યારે (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


સ્થિર પૂર્ણાંક read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  અક્ષર *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, કદ(અક્ષર), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  પાછા (nr > 0);  /* true iff read something */
}


સ્થિર પૂર્ણાંક g_read (lua_State *L, FILE *f, પૂર્ણાંક first) {
  પૂર્ણાંક nargs = lua_gettop(L) - 1;
  પૂર્ણાંક n, success;
  clearerr(f);
  errno = 0;
  જો (nargs == 0) {  /* no arguments? */
    success = read_લીટી(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  નહીંતર {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    માટે (n = first; nargs-- && success; n++) {
      જો (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      નહીંતર {
        અચલ અક્ષર *p = luaL_checkstring(L, n);
        જો (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        પસંદગી (*p) {
          સ્થિતિ 'n':  /*સંખ્યા*/
            success = read_સંખ્યા(L, f);
            તોડો;
          સ્થિતિ 'l':  /*લીટી*/
            success = read_લીટી(L, f, 1);
            તોડો;
          સ્થિતિ 'L':  /* line with end-of-line */
            success = read_લીટી(L, f, 0);
            તોડો;
          સ્થિતિ 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            તોડો;
          મૂળભૂત:
            પાછા luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  જો (ferror(f))
    પાછા luaL_fileresult(L, 0, ખાલી);
  જો (!success) {
    lua_કાઢો(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  પાછા n - first;
}


સ્થિર પૂર્ણાંક io_read (lua_State *L) {
  પાછા g_read(L, getiofile(L, IO_આગત), 1);
}


સ્થિર પૂર્ણાંક f_read (lua_State *L) {
  પાછા g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
સ્થિર પૂર્ણાંક io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  પૂર્ણાંક i;
  પૂર્ણાંક n = (પૂર્ણાંક)lua_tointeger(L, lua_upvalueindex(2));
  જો (isclosed(p))  /* file is already closed? */
    પાછા luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  માટે (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  જો (lua_toboolean(L, -n))  /* read at least one value? */
    પાછા n;  /* return them */
  નહીંતર {  /* first result is false: EOF or error */
    જો (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      પાછા luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    જો (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    પાછા 0;
  }
}

/* }====================================================== */


સ્થિર પૂર્ણાંક g_write (lua_State *L, FILE *f, પૂર્ણાંક arg) {
  પૂર્ણાંક nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  માટે (; nargs--; arg++) {  /* for each argument */
    અક્ષર buff[LUA_N2SBUFFSZ];
    અચલ અક્ષર *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    જો (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    નહીંતર  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, કદ(અક્ષર), len, f);
    totalbytes += numbytes;
    જો (numbytes < len) {  /* write error? */
      પૂર્ણાંક n = luaL_fileresult(L, 0, ખાલી);
      lua_pushinteger(L, cast_st2S(totalbytes));
      પાછા n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  પાછા 1;  /* no errors; file handle already on stack top */
}


સ્થિર પૂર્ણાંક io_write (lua_State *L) {
  પાછા g_write(L, getiofile(L, IO_OUTPUT), 1);
}


સ્થિર પૂર્ણાંક f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  પાછા g_write(L, f, 2);
}


સ્થિર પૂર્ણાંક f_seek (lua_State *L) {
  સ્થિર અચલ પૂર્ણાંક mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  સ્થિર અચલ અક્ષર *અચલ modenames[] = {"set", "cur", "end", ખાલી};
  FILE *f = tofile(L);
  પૂર્ણાંક op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  જો (l_unlikely(op))
    પાછા luaL_fileresult(L, 0, ખાલી);  /* error */
  નહીંતર {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    પાછા 1;
  }
}


સ્થિર પૂર્ણાંક f_setvbuf (lua_State *L) {
  સ્થિર અચલ પૂર્ણાંક mode[] = {_IONBF, _IOFBF, _IOLBF};
  સ્થિર અચલ અક્ષર *અચલ modenames[] = {"no", "full", "line", ખાલી};
  FILE *f = tofile(L);
  પૂર્ણાંક op = luaL_checkoption(L, 2, ખાલી, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  પૂર્ણાંક res;
  errno = 0;
  res = setvbuf(f, ખાલી, mode[op], (size_t)sz);
  પાછા luaL_fileresult(L, res == 0, ખાલી);
}


સ્થિર પૂર્ણાંક aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  પાછા luaL_fileresult(L, fflush(f) == 0, ખાલી);
}


સ્થિર પૂર્ણાંક f_flush (lua_State *L) {
  પાછા aux_flush(L, tofile(L));
}


સ્થિર પૂર્ણાંક io_flush (lua_State *L) {
  પાછા aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
સ્થિર અચલ luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_આગત},
  {"lines", io_લીટીઓ},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ખાલી, ખાલી}
};


/*
** methods for file handles
*/
સ્થિર અચલ luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_લીટીઓ},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ખાલી, ખાલી}
};


/*
** metamethods for file handles
*/
સ્થિર અચલ luaL_Reg metameth[] = {
  {"__index", ખાલી},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ખાલી, ખાલી}
};


સ્થિર શૂન્ય createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_કાઢો(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
સ્થિર પૂર્ણાંક io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  પાછા 2;
}


સ્થિર શૂન્ય createstdfile (lua_State *L, FILE *f, અચલ અક્ષર *k,
                           અચલ અક્ષર *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  જો (k != ખાલી) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API પૂર્ણાંક luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_આગત, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ખાલી, "stderr");
  પાછા 1;
}

