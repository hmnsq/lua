/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


#якщо !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#якщо !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#якщо !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;


#якщо defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
static void setsignal (ціле sig, void (*handler)(ціле)) {
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, NULL);
}

#інакше           /* }{ */

#define setsignal            signal

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
static void laction (ціле i) {
  ціле flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


static void print_usage (const char *badoption) {
  lua_writestringerror("%s: ", progname);
  якщо (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  інакше
    lua_writestringerror("unrecognized option '%s'\n", badoption);
  lua_writestringerror(
  "usage: %s [options] [script [args]]\n"
  "Available options are:\n"
  "  -e stat   execute string 'stat'\n"
  "  -i        enter interactive mode after executing 'script'\n"
  "  -l mod    require library 'mod' into global 'mod'\n"
  "  -l g=mod  require library 'mod' into global 'g'\n"
  "  -v        show version information\n"
  "  -E        ignore environment variables\n"
  "  -W        turn warnings on\n"
  "  --        stop handling options\n"
  "  -         stop handling options and execute stdin\n"
  ,
  progname);
}


/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message (const char *pname, const char *msg) {
  якщо (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
static ціле report (lua_State *L, ціле status) {
  якщо (status != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    якщо (msg == NULL)
      msg = "(error message not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  повернути status;
}


/*
** Message handler used to run all chunks
*/
static ціле msghandler (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  якщо (msg == NULL) {  /* is error object not a string? */
    якщо (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      повернути 1;  /* that is the message */
    інакше
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  повернути 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static ціле docall (lua_State *L, ціле narg, ціле nres) {
  ціле status;
  ціле base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  повернути status;
}


static void print_version (void) {
  lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
  lua_writeline();
}


/*
** Create the 'arg' table, which stores all arguments from the
** command line ('argv'). It should be aligned so that, at index 0,
** it has 'argv[script]', which is the script name. The arguments
** to the script (everything after 'script') go to positive indices;
** other arguments (before the script name) go to negative indices.
** If there is no script name, assume interpreter's name as base.
** (If there is no interpreter's name either, 'script' is -1, so
** table sizes are zero.)
*/
static void createargtable (lua_State *L, char **argv, ціле argc, ціле script) {
  ціле i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  для (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static ціле dochunk (lua_State *L, ціле status) {
  якщо (status == LUA_OK) status = docall(L, 0, 0);
  повернути report(L, status);
}


static ціле dofile (lua_State *L, const char *name) {
  повернути dochunk(L, luaL_loadfilex(L, name, "bt"));
}


static ціле dostring (lua_State *L, const char *s, const char *name) {
  повернути dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
static ціле dolibrary (lua_State *L, char *globname) {
  ціле status;
  char *suffix = NULL;
  char *modname = strchr(globname, '=');
  якщо (modname == NULL) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  інакше {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  якщо (status == LUA_OK) {
    якщо (suffix != NULL)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  повернути report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static ціле pushargs (lua_State *L) {
  ціле i, n;
  якщо (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ціле)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  для (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  повернути n;
}


static ціле handle_script (lua_State *L, char **argv) {
  ціле status;
  const char *fname = argv[0];
  якщо (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  якщо (status == LUA_OK) {
    ціле n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  повернути report(L, status);
}


/* bits of various argument indicators in 'args' */
#define has_error	1	/* bad option */
#define has_i		2	/* -i */
#define has_v		4	/* -v */
#define has_e		8	/* -e */
#define has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
static ціле collectargs (char **argv, ціле *first) {
  ціле args = 0;
  ціле i;
  якщо (argv[0] != NULL) {  /* is there a program name? */
    якщо (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  інакше {  /* no program name */
    *first = -1;
    повернути 0;
  }
  для (i = 1; argv[i] != NULL; i++) {  /* handle arguments */
    *first = i;
    якщо (argv[i][0] != '-')  /* not an option? */
        повернути args;  /* stop handling options */
    switch (argv[i][1]) {  /* else check option */
      case '-':  /* '--' */
        якщо (argv[i][2] != '\0')  /* extra characters after '--'? */
          повернути has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != NULL) ? i + 1 : 0;
        повернути args;
      case '\0':  /* '-' */
        повернути args;  /* script "name" is '-' */
      case 'E':
        якщо (argv[i][2] != '\0')  /* extra characters? */
          повернути has_error;  /* invalid option */
        args |= has_E;
        break;
      case 'W':
        якщо (argv[i][2] != '\0')  /* extra characters? */
          повернути has_error;  /* invalid option */
        break;
      case 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      case 'v':
        якщо (argv[i][2] != '\0')  /* extra characters? */
          повернути has_error;  /* invalid option */
        args |= has_v;
        break;
      case 'e':
        args |= has_e;  /* FALLTHROUGH */
      case 'l':  /* both options need an argument */
        якщо (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          якщо (argv[i] == NULL || argv[i][0] == '-')
            повернути has_error;  /* no next argument or it is another option */
        }
        break;
      default:  /* invalid option */
        повернути has_error;
    }
  }
  *first = 0;  /* no script name */
  повернути args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static ціле runargs (lua_State *L, char **argv, ціле n) {
  ціле i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  для (i = 1; i < n; i++) {
    ціле option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    switch (option) {
      case 'e':  case 'l': {
        ціле status;
        char *extra = argv[i] + 2;  /* both options need an argument */
        якщо (*extra == '\0') extra = argv[++i];
        lua_assert(extra != NULL);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        якщо (status != LUA_OK) повернути 0;
        break;
      }
      case 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        break;
    }
  }
  повернути 1;
}


static char *(*l_getenv)(const char *name);

/* Function to ignore environment variables, used by option -E */
static char *no_getenv (const char *name) {
  UNUSED(name);
  повернути NULL;
}


static ціле handle_luainit (lua_State *L) {
  const char *name = "=" LUA_INITVARVERSION;
  const char *init = l_getenv(name + 1);
  якщо (init == NULL) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  якщо (init == NULL) повернути LUA_OK;
  інакше якщо (init[0] == '@')
    повернути dofile(L, init+1);
  інакше
    повернути dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#якщо !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#якщо !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#якщо !defined(lua_stdin_is_tty)	/* { */

#якщо defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#інакше				/* }{ */

/* ISO C definition */
#define lua_stdin_is_tty()	1  /* assume stdin is a tty */

#endif				/* } */

#endif				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#якщо !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#якщо defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#include <readline/readline.h>
#include <readline/history.h>

#define lua_initreadline(L)  \
	((void)L, *cast(const char **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((void)buff, readline(prompt))
#define lua_saveline(line)	add_history(line)
#define lua_freeline(line)	free(line)

#інакше		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
typedef char *(*l_readlineT) (const char *prompt);
static l_readlineT l_readline = NULL;

/* pointer to 'add_history' function (if any) */
typedef void (*l_addhistT) (const char *string);
static l_addhistT l_addhist = NULL;


static char *lua_readline (char *buff, const char *prompt) {
  якщо (l_readline != NULL)  /* is there a 'readline'? */
    повернути (*l_readline)(prompt);  /* use it */
  інакше {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    повернути fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


static void lua_saveline (const char *line) {
  якщо (l_addhist != NULL)  /* is there an 'add_history'? */
    (*l_addhist)(line);  /* use it */
  /* else nothing to be done */
}


static void lua_freeline (char *line) {
  якщо (l_readline != NULL)  /* is there a 'readline'? */
    free(line);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#якщо defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#include <dlfcn.h>

static void lua_initreadline (lua_State *L) {
  const char *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  void *lib;  /* library handle */
  якщо (rllib == NULL)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  якщо (lib != NULL) {
    const char **name = cast(const char**, dlsym(lib, "rl_readline_name"));
    якщо (name != NULL)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    якщо (l_readline != NULL)  /* could load readline function? */
      повернути;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#інакше		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((void)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
static const char *get_prompt (lua_State *L, ціле firstline) {
  якщо (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    повернути (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  інакше {  /* apply 'tostring' over the value */
    const char *p = luaL_tolstring(L, -1, NULL);
    lua_remove(L, -2);  /* remove original value */
    повернути p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static ціле incomplete (lua_State *L, ціле status) {
  якщо (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    якщо (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
      повернути 1;
  }
  повернути 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static ціле pushline (lua_State *L, ціле firstline) {
  char buffer[LUA_MAXINPUT];
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  char *b = lua_readline(buffer, prmt);
  lua_pop(L, 1);  /* remove prompt */
  якщо (b == NULL)
    повернути 0;  /* no input */
  l = strlen(b);
  якщо (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  повернути 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static ціле addreturn (lua_State *L) {
  const char *line = lua_tostring(L, -1);  /* original line */
  const char *retline = lua_pushfstring(L, "return %s;", line);
  ціле status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  якщо (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  інакше
    lua_pop(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  повернути status;
}


static void checklocal (const char *line) {
  static const size_t szloc = sizeof("local") - 1;
  static const char space[] = " \t";
  line += strspn(line, space);  /* skip spaces */
  якщо (strncmp(line, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(line + szloc)) != NULL) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
static ціле multiline (lua_State *L) {
  size_t len;
  const char *line = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(line);
  для (;;) {  /* repeat until gets a complete statement */
    ціле status = luaL_loadbufferx(L, line, len, "=stdin", "t");  /* try it */
    якщо (!incomplete(L, status) || !pushline(L, 0))
      повернути status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    line = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static ціле loadline (lua_State *L) {
  const char *line;
  ціле status;
  lua_settop(L, 0);
  якщо (!pushline(L, 1))
    повернути -1;  /* no input */
  якщо ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  line = lua_tostring(L, 1);
  якщо (line[0] != '\0')  /* non empty? */
    lua_saveline(line);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  повернути status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
  ціле n = lua_gettop(L);
  якщо (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    якщо (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL (lua_State *L) {
  ціле status;
  const char *oldprogname = progname;
  progname = NULL;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  поки ((status = loadline(L)) != -1) {
    якщо (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    якщо (status == LUA_OK) l_print(L);
    інакше report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#якщо !defined(luai_openlibs)
#якщо defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#інакше
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static ціле pmain (lua_State *L) {
  ціле argc = (ціле)lua_tointeger(L, 1);
  char **argv = (char **)lua_touserdata(L, 2);
  ціле script;
  ціле args = collectargs(argv, &script);
  ціле optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  якщо (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    повернути 0;
  }
  якщо (args & has_v)  /* option '-v'? */
    print_version();
  якщо (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  інакше
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  якщо (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    повернути 0;  /* error running LUA_INIT */
  якщо (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    повернути 0;  /* something failed */
  якщо (script > 0) {  /* execute main script (if there is one) */
    якщо (handle_script(L, argv + script) != LUA_OK)
      повернути 0;  /* interrupt in case of error */
  }
  якщо (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  інакше якщо (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    якщо (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    інакше dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  повернути 1;
}


ціле main (ціле argc, char **argv) {
  ціле status, result;
  lua_State *L = luaL_newstate();  /* create state */
  якщо (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    повернути EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  повернути (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

