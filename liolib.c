/*
** $Id: liolib.c $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#ଯଦି !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#ଯଦି !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା l_checkmode (ଅଚଳ ଅକ୍ଷର *mode) {
  ଫେରନ୍ତୁ (*mode != '\0' && strchr("rwa", *(mode++)) != ଖାଲି &&
         (*mode != '+' || ((ଶୂନ୍ୟ)(++mode), 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#ଯଦି !defined(l_popen)		/* { */

#ଯଦି defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(ଖାଲି), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#ନଚେତଯଦି defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#ଯଦି !defined(l_checkmodep)
/* Windows accepts "[rw][bt]?" as valid modes */
#define l_checkmodep(m)	((m[0] == 'r' || m[0] == 'w') && \
  (m[1] == '\0' || ((m[1] == 'b' || m[1] == 't') && m[2] == '\0')))
#endif

#ଅନ୍ୟଥା				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((ଶୂନ୍ୟ)c, (ଶୂନ୍ୟ)m, \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((ଶୂନ୍ୟ)L, (ଶୂନ୍ୟ)file, -1)

#endif				/* } */

#endif				/* } */


#ଯଦି !defined(l_checkmodep)
/* By default, Lua accepts only "r" or "w" as valid modes */
#define l_checkmodep(m)        ((m[0] == 'r' || m[0] == 'w') && m[1] == '\0')
#endif

/* }====================================================== */


#ଯଦି !defined(l_getc)		/* { */

#ଯଦି defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#ଅନ୍ୟଥା
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((ଶୂନ୍ୟ)0)
#define l_unlockfile(f)		((ଶୂନ୍ୟ)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#ଯଦି !defined(l_fseek)		/* { */

#ଯଦି defined(LUA_USE_POSIX) || defined(LUA_USE_OFF_T)	/* { */

#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#ନଚେତଯଦି defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#ଅନ୍ୟଥା				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */



#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(ଆକାର(IO_PREFIX)/ଆକାର(ଅକ୍ଷର) - 1)
#define IO_ନିବେଶ	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


ପ୍ରକାରବ୍ୟାଖ୍ୟା luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == ଖାଲି)


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  ଯଦି (p == ଖାଲି)
    luaL_pushfail(L);  /* not a file */
  ଅନ୍ୟଥା ଯଦି (isclosed(p))
    lua_pushliteral(L, "closed file");
  ଅନ୍ୟଥା
    lua_pushliteral(L, "file");
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  ଯଦି (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  ଅନ୍ୟଥା
    lua_pushfstring(L, "file (%p)", p->f);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  ଯଦି (l_unlikely(isclosed(p)))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  ଫେରନ୍ତୁ p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
ସ୍ଥିର LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, ଆକାର(LStream), 0);
  p->closef = ଖାଲି;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  ଫେରନ୍ତୁ p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  ଅସ୍ଥିର lua_CFunction cf = p->closef;
  p->closef = ଖାଲି;  /* mark stream as closed */
  ଫେରନ୍ତୁ (*cf)(L);  /* close it */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  ଫେରନ୍ତୁ aux_close(L);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_close (lua_State *L) {
  ଯଦି (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use default output */
  ଫେରନ୍ତୁ f_close(L);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_gc (lua_State *L) {
  LStream *p = tolstream(L);
  ଯଦି (!isclosed(p) && p->f != ଖାଲି)
    aux_close(L);  /* ignore closed and incompletely open files */
  ଫେରନ୍ତୁ 0;
}


/*
** function to close regular files
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ଫେରନ୍ତୁ luaL_fileresult(L, (fclose(p->f) == 0), ଖାଲି);
}


ସ୍ଥିର LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = ଖାଲି;
  p->closef = &io_fclose;
  ଫେରନ୍ତୁ p;
}


ସ୍ଥିର ଶୂନ୍ୟ opencheck (lua_State *L, ଅଚଳ ଅକ୍ଷର *fname, ଅଚଳ ଅକ୍ଷର *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  ଯଦି (l_unlikely(p->f == ଖାଲି))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_open (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *filename = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  ଅଚଳ ଅକ୍ଷର *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  errno = 0;
  p->f = fopen(filename, mode);
  ଫେରନ୍ତୁ (p->f == ଖାଲି) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  errno = 0;
  ଫେରନ୍ତୁ luaL_execresult(L, l_pclose(L, p->f));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_popen (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *filename = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  luaL_argcheck(L, l_checkmodep(mode), 2, "invalid mode");
  errno = 0;
  p->f = l_popen(L, filename, mode);
  p->closef = &io_pclose;
  ଫେରନ୍ତୁ (p->f == ଖାଲି) ? luaL_fileresult(L, 0, filename) : 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  errno = 0;
  p->f = tmpfile();
  ଫେରନ୍ତୁ (p->f == ଖାଲି) ? luaL_fileresult(L, 0, ଖାଲି) : 1;
}


ସ୍ଥିର FILE *getiofile (lua_State *L, ଅଚଳ ଅକ୍ଷର *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  ଯଦି (l_unlikely(isclosed(p)))
    luaL_error(L, "default %s file is closed", findex + IOPREF_LEN);
  ଫେରନ୍ତୁ p->f;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା g_iofile (lua_State *L, ଅଚଳ ଅକ୍ଷର *f, ଅଚଳ ଅକ୍ଷର *mode) {
  ଯଦି (!lua_isnoneornil(L, 1)) {
    ଅଚଳ ଅକ୍ଷର *filename = lua_tostring(L, 1);
    ଯଦି (filename)
      opencheck(L, filename, mode);
    ଅନ୍ୟଥା {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_ନିବେଶ (lua_State *L) {
  ଫେରନ୍ତୁ g_iofile(L, IO_ନିବେଶ, "r");
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_output (lua_State *L) {
  ଫେରନ୍ତୁ g_iofile(L, IO_OUTPUT, "w");
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_readline (lua_State *L);


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
ସ୍ଥିର ଶୂନ୍ୟ aux_ଧାଡ଼ିଗୁଡ଼ିକ (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା toclose) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushvalue(L, 1);  /* file */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 3);  /* move the three values to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_ଧାଡ଼ିଗୁଡ଼ିକ (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_ଧାଡ଼ିଗୁଡ଼ିକ(L, 0);
  ଫେରନ୍ତୁ 1;
}


/*
** Return an iteration function for 'io.lines'. If file has to be
** closed, also returns the file itself as a second result (to be
** closed as the state at the exit of a generic for).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_ଧାଡ଼ିଗୁଡ଼ିକ (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା toclose;
  ଯଦି (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  ଯଦି (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_ନିବେଶ);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  ଅନ୍ୟଥା {  /* open a new file */
    ଅଚଳ ଅକ୍ଷର *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_ଧାଡ଼ିଗୁଡ଼ିକ(L, toclose);  /* push iteration function */
  ଯଦି (toclose) {
    lua_pushnil(L);  /* state */
    lua_pushnil(L);  /* control */
    lua_pushvalue(L, 1);  /* file is the to-be-closed variable (4th result) */
    ଫେରନ୍ତୁ 4;
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#ଯଦି !defined (L_MAXLENNUM)
#define L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'read_number' */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
  FILE *f;  /* file being read */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;  /* current character (look ahead) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;  /* number of elements in buffer 'buff' */
  ଅକ୍ଷର buff[L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextc (RN *rn) {
  ଯଦି (l_unlikely(rn->n >= L_MAXLENNUM)) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    ଫେରନ୍ତୁ 0;  /* fail */
  }
  ଅନ୍ୟଥା {
    rn->buff[rn->n++] = cast_char(rn->c);  /* save current char */
    rn->c = l_getc(rn->f);  /* read next one */
    ଫେରନ୍ତୁ 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test2 (RN *rn, ଅଚଳ ଅକ୍ଷର *set) {
  ଯଦି (rn->c == set[0] || rn->c == set[1])
    ଫେରନ୍ତୁ nextc(rn);
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;
}


/*
** Read a sequence of (hex)digits
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା readdigits (RN *rn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ଯେତେବେଳେ ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && nextc(rn))
    ଗଣନା_2++;
  ଫେରନ୍ତୁ ଗଣନା_2;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_ସଂଖ୍ୟା (lua_State *L, FILE *f) {
  RN rn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex = 0;
  ଅକ୍ଷର decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  l_lockfile(rn.f);
  କର { rn.c = l_getc(rn.f); } ଯେତେବେଳେ (isspace(rn.c));  /* skip spaces */
  test2(&rn, "-+");  /* optional sign */
  ଯଦି (test2(&rn, "00")) {
    ଯଦି (test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    ଅନ୍ୟଥା ଗଣନା_2 = 1;  /* count initial '0' as a valid digit */
  }
  ଗଣନା_2 += readdigits(&rn, hex);  /* integral part */
  ଯଦି (test2(&rn, decp))  /* decimal point? */
    ଗଣନା_2 += readdigits(&rn, hex);  /* fractional part */
  ଯଦି (ଗଣନା_2 > 0 && test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    test2(&rn, "-+");  /* exponent sign */
    readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  ଯଦି (l_likely(lua_stringtonumber(L, rn.buff)))
    ଫେରନ୍ତୁ 1;  /* ok, it is a valid number */
  ଅନ୍ୟଥା {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   ଫେରନ୍ତୁ 0;  /* read fails */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_eof (lua_State *L, FILE *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  ଫେରନ୍ତୁ (c != EOF);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_ଧାଡ଼ି (lua_State *L, FILE *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chop) {
  luaL_Buffer b;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  luaL_buffinit(L, &b);
  କର {  /* may need to read several chunks to get whole line */
    ଅକ୍ଷର *buff = luaL_prepbuffer(&b);  /* preallocate buffer space */
    unsigned i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    ଯେତେବେଳେ (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = cast_char(c);  /* read up to end of line or buffer limit */
    l_unlockfile(f);
    luaL_addsize(&b, i);
  } ଯେତେବେଳେ (c != EOF && c != '\n');  /* repeat until end of line */
  ଯଦି (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, '\n');  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  ଫେରନ୍ତୁ (c == '\n' || lua_rawlen(L, -1) > 0);
}


ସ୍ଥିର ଶୂନ୍ୟ read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  କର {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    ଅକ୍ଷର *p = luaL_prepbuffer(&b);
    nr = fread(p, ଆକାର(ଅକ୍ଷର), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } ଯେତେବେଳେ (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  ଅକ୍ଷର *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  nr = fread(p, ଆକାର(ଅକ୍ଷର), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  ଫେରନ୍ତୁ (nr > 0);  /* true iff read something */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା g_read (lua_State *L, FILE *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = lua_gettop(L) - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, success;
  clearerr(f);
  errno = 0;
  ଯଦି (nargs == 0) {  /* no arguments? */
    success = read_ଧାଡ଼ି(L, f, 1);
    n = first + 1;  /* to return 1 result */
  }
  ଅନ୍ୟଥା {
    /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    ପାଇଁ (n = first; nargs-- && success; n++) {
      ଯଦି (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      ଅନ୍ୟଥା {
        ଅଚଳ ଅକ୍ଷର *p = luaL_checkstring(L, n);
        ଯଦି (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        ବାଛ (*p) {
          ସ୍ଥିତି 'n':  /*ସଂଖ୍ୟା*/
            success = read_ସଂଖ୍ୟା(L, f);
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 'l':  /*ଧାଡ଼ି*/
            success = read_ଧାଡ଼ି(L, f, 1);
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 'L':  /* line with end-of-line */
            success = read_ଧାଡ଼ି(L, f, 0);
            ଭାଙ୍ଗ;
          ସ୍ଥିତି 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            ଭାଙ୍ଗ;
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            ଫେରନ୍ତୁ luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  ଯଦି (ferror(f))
    ଫେରନ୍ତୁ luaL_fileresult(L, 0, ଖାଲି);
  ଯଦି (!success) {
    lua_କାଢ଼(L, 1);  /* remove last result */
    luaL_pushfail(L);  /* push nil instead */
  }
  ଫେରନ୍ତୁ n - first;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_read (lua_State *L) {
  ଫେରନ୍ତୁ g_read(L, getiofile(L, IO_ନିବେଶ), 1);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_read (lua_State *L) {
  ଫେରନ୍ତୁ g_read(L, tofile(L), 2);
}


/*
** Iteration function for 'lines'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, lua_upvalueindex(2));
  ଯଦି (isclosed(p))  /* file is already closed? */
    ଫେରନ୍ତୁ luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  ପାଇଁ (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  ଯଦି (lua_toboolean(L, -n))  /* read at least one value? */
    ଫେରନ୍ତୁ n;  /* return them */
  ଅନ୍ୟଥା {  /* first result is false: EOF or error */
    ଯଦି (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      ଫେରନ୍ତୁ luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    ଯଦି (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);  /* clear stack */
      lua_pushvalue(L, lua_upvalueindex(1));  /* push file at index 1 */
      aux_close(L);  /* close it */
    }
    ଫେରନ୍ତୁ 0;
  }
}

/* }====================================================== */


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା g_write (lua_State *L, FILE *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = lua_gettop(L) - arg;
  size_t totalbytes = 0;  /* total number of bytes written */
  errno = 0;
  ପାଇଁ (; nargs--; arg++) {  /* for each argument */
    ଅକ୍ଷର buff[LUA_N2SBUFFSZ];
    ଅଚଳ ଅକ୍ଷର *s;
    size_t numbytes;  /* bytes written in one call to 'fwrite' */
    size_t len = lua_numbertocstring(L, arg, buff);  /* try as a number */
    ଯଦି (len > 0) {  /* did conversion work (value was a number)? */
      s = buff;
      len--;
    }
    ଅନ୍ୟଥା  /* must be a string */
      s = luaL_checklstring(L, arg, &len);
    numbytes = fwrite(s, ଆକାର(ଅକ୍ଷର), len, f);
    totalbytes += numbytes;
    ଯଦି (numbytes < len) {  /* write error? */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = luaL_fileresult(L, 0, ଖାଲି);
      lua_pushinteger(L, cast_st2S(totalbytes));
      ଫେରନ୍ତୁ n + 1;  /* return fail, error msg., error code, and counter */
    }
  }
  ଫେରନ୍ତୁ 1;  /* no errors; file handle already on stack top */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_write (lua_State *L) {
  ଫେରନ୍ତୁ g_write(L, getiofile(L, IO_OUTPUT), 1);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  ଫେରନ୍ତୁ g_write(L, f, 2);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_seek (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ modenames[] = {"set", "cur", "end", ଖାଲି};
  FILE *f = tofile(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  errno = 0;
  op = l_fseek(f, offset, mode[op]);
  ଯଦି (l_unlikely(op))
    ଫେରନ୍ତୁ luaL_fileresult(L, 0, ଖାଲି);  /* error */
  ଅନ୍ୟଥା {
    lua_pushinteger(L, (lua_Integer)l_ftell(f));
    ଫେରନ୍ତୁ 1;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_setvbuf (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode[] = {_IONBF, _IOFBF, _IOLBF};
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ modenames[] = {"no", "full", "line", ଖାଲି};
  FILE *f = tofile(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = luaL_checkoption(L, 2, ଖାଲି, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  errno = 0;
  res = setvbuf(f, ଖାଲି, mode[op], (size_t)sz);
  ଫେରନ୍ତୁ luaL_fileresult(L, res == 0, ଖାଲି);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା aux_flush (lua_State *L, FILE *f) {
  errno = 0;
  ଫେରନ୍ତୁ luaL_fileresult(L, fflush(f) == 0, ଖାଲି);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_flush (lua_State *L) {
  ଫେରନ୍ତୁ aux_flush(L, tofile(L));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_flush (lua_State *L) {
  ଫେରନ୍ତୁ aux_flush(L, getiofile(L, IO_OUTPUT));
}


/*
** functions for 'io' library
*/
ସ୍ଥିର ଅଚଳ luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_ନିବେଶ},
  {"lines", io_ଧାଡ଼ିଗୁଡ଼ିକ},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {ଖାଲି, ଖାଲି}
};


/*
** methods for file handles
*/
ସ୍ଥିର ଅଚଳ luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_ଧାଡ଼ିଗୁଡ଼ିକ},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {ଖାଲି, ଖାଲି}
};


/*
** metamethods for file handles
*/
ସ୍ଥିର ଅଚଳ luaL_Reg metameth[] = {
  {"__index", ଖାଲି},  /* placeholder */
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {ଖାଲି, ଖାଲି}
};


ସ୍ଥିର ଶୂନ୍ୟ createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* metatable for file handles */
  luaL_setfuncs(L, metameth, 0);  /* add metamethods to new metatable */
  luaL_newlibtable(L, meth);  /* create method table */
  luaL_setfuncs(L, meth, 0);  /* add file methods to method table */
  lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
  lua_କାଢ଼(L, 1);  /* pop metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_noclose (lua_State *L) {
  LStream *p = tolstream(L);
  p->closef = &io_noclose;  /* keep file opened */
  luaL_pushfail(L);
  lua_pushliteral(L, "cannot close standard file");
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ଶୂନ୍ୟ createstdfile (lua_State *L, FILE *f, ଅଚଳ ଅକ୍ଷର *k,
                           ଅଚଳ ଅକ୍ଷର *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  ଯଦି (k != ଖାଲି) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_io (lua_State *L) {
  luaL_newlib(L, iolib);  /* new module */
  createmeta(L);
  /* create (and set) default files */
  createstdfile(L, stdin, IO_ନିବେଶ, "stdin");
  createstdfile(L, stdout, IO_OUTPUT, "stdout");
  createstdfile(L, stderr, ଖାଲି, "stderr");
  ଫେରନ୍ତୁ 1;
}

