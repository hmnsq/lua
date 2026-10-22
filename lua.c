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


#만약 !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#만약 !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#만약 !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;


#만약 defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
static void setsignal (정수 sig, void (*handler)(정수)) {
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, NULL);
}

#아니면           /* }{ */

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
static void laction (정수 i) {
  정수 flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


static void print_usage (const char *badoption) {
  lua_writestringerror("%s: ", progname);
  만약 (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  아니면
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
  만약 (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
static 정수 report (lua_State *L, 정수 status) {
  만약 (status != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    만약 (msg == NULL)
      msg = "(error message not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  반환 status;
}


/*
** Message handler used to run all chunks
*/
static 정수 msghandler (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  만약 (msg == NULL) {  /* is error object not a string? */
    만약 (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      반환 1;  /* that is the message */
    아니면
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  반환 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static 정수 docall (lua_State *L, 정수 narg, 정수 nres) {
  정수 status;
  정수 base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  반환 status;
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
static void createargtable (lua_State *L, char **argv, 정수 argc, 정수 script) {
  정수 i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  위해 (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static 정수 dochunk (lua_State *L, 정수 status) {
  만약 (status == LUA_OK) status = docall(L, 0, 0);
  반환 report(L, status);
}


static 정수 dofile (lua_State *L, const char *name) {
  반환 dochunk(L, luaL_loadfilex(L, name, "bt"));
}


static 정수 dostring (lua_State *L, const char *s, const char *name) {
  반환 dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
static 정수 dolibrary (lua_State *L, char *globname) {
  정수 status;
  char *suffix = NULL;
  char *modname = strchr(globname, '=');
  만약 (modname == NULL) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  아니면 {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  만약 (status == LUA_OK) {
    만약 (suffix != NULL)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  반환 report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static 정수 pushargs (lua_State *L) {
  정수 i, n;
  만약 (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (정수)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  위해 (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  반환 n;
}


static 정수 handle_script (lua_State *L, char **argv) {
  정수 status;
  const char *fname = argv[0];
  만약 (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  만약 (status == LUA_OK) {
    정수 n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  반환 report(L, status);
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
static 정수 collectargs (char **argv, 정수 *first) {
  정수 args = 0;
  정수 i;
  만약 (argv[0] != NULL) {  /* is there a program name? */
    만약 (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  아니면 {  /* no program name */
    *first = -1;
    반환 0;
  }
  위해 (i = 1; argv[i] != NULL; i++) {  /* handle arguments */
    *first = i;
    만약 (argv[i][0] != '-')  /* not an option? */
        반환 args;  /* stop handling options */
    switch (argv[i][1]) {  /* else check option */
      case '-':  /* '--' */
        만약 (argv[i][2] != '\0')  /* extra characters after '--'? */
          반환 has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != NULL) ? i + 1 : 0;
        반환 args;
      case '\0':  /* '-' */
        반환 args;  /* script "name" is '-' */
      case 'E':
        만약 (argv[i][2] != '\0')  /* extra characters? */
          반환 has_error;  /* invalid option */
        args |= has_E;
        break;
      case 'W':
        만약 (argv[i][2] != '\0')  /* extra characters? */
          반환 has_error;  /* invalid option */
        break;
      case 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      case 'v':
        만약 (argv[i][2] != '\0')  /* extra characters? */
          반환 has_error;  /* invalid option */
        args |= has_v;
        break;
      case 'e':
        args |= has_e;  /* FALLTHROUGH */
      case 'l':  /* both options need an argument */
        만약 (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          만약 (argv[i] == NULL || argv[i][0] == '-')
            반환 has_error;  /* no next argument or it is another option */
        }
        break;
      default:  /* invalid option */
        반환 has_error;
    }
  }
  *first = 0;  /* no script name */
  반환 args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static 정수 runargs (lua_State *L, char **argv, 정수 n) {
  정수 i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  위해 (i = 1; i < n; i++) {
    정수 option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    switch (option) {
      case 'e':  case 'l': {
        정수 status;
        char *extra = argv[i] + 2;  /* both options need an argument */
        만약 (*extra == '\0') extra = argv[++i];
        lua_assert(extra != NULL);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        만약 (status != LUA_OK) 반환 0;
        break;
      }
      case 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        break;
    }
  }
  반환 1;
}


static char *(*l_getenv)(const char *name);

/* Function to ignore environment variables, used by option -E */
static char *no_getenv (const char *name) {
  UNUSED(name);
  반환 NULL;
}


static 정수 handle_luainit (lua_State *L) {
  const char *name = "=" LUA_INITVARVERSION;
  const char *init = l_getenv(name + 1);
  만약 (init == NULL) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  만약 (init == NULL) 반환 LUA_OK;
  아니면 만약 (init[0] == '@')
    반환 dofile(L, init+1);
  아니면
    반환 dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#만약 !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#만약 !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#만약 !defined(lua_stdin_is_tty)	/* { */

#만약 defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#아니면				/* }{ */

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

#만약 !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#만약 defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#include <readline/readline.h>
#include <readline/history.h>

#define lua_initreadline(L)  \
	((void)L, *cast(const char **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((void)buff, readline(prompt))
#define lua_saveline(line)	add_history(line)
#define lua_freeline(line)	free(line)

#아니면		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
typedef char *(*l_readlineT) (const char *prompt);
static l_readlineT l_readline = NULL;

/* pointer to 'add_history' function (if any) */
typedef void (*l_addhistT) (const char *string);
static l_addhistT l_addhist = NULL;


static char *lua_readline (char *buff, const char *prompt) {
  만약 (l_readline != NULL)  /* is there a 'readline'? */
    반환 (*l_readline)(prompt);  /* use it */
  아니면 {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    반환 fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


static void lua_saveline (const char *line) {
  만약 (l_addhist != NULL)  /* is there an 'add_history'? */
    (*l_addhist)(line);  /* use it */
  /* else nothing to be done */
}


static void lua_freeline (char *line) {
  만약 (l_readline != NULL)  /* is there a 'readline'? */
    free(line);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#만약 defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#include <dlfcn.h>

static void lua_initreadline (lua_State *L) {
  const char *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  void *lib;  /* library handle */
  만약 (rllib == NULL)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  만약 (lib != NULL) {
    const char **name = cast(const char**, dlsym(lib, "rl_readline_name"));
    만약 (name != NULL)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    만약 (l_readline != NULL)  /* could load readline function? */
      반환;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#아니면		/* }{ */
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
static const char *get_prompt (lua_State *L, 정수 firstline) {
  만약 (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    반환 (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  아니면 {  /* apply 'tostring' over the value */
    const char *p = luaL_tolstring(L, -1, NULL);
    lua_remove(L, -2);  /* remove original value */
    반환 p;
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
static 정수 incomplete (lua_State *L, 정수 status) {
  만약 (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    만약 (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
      반환 1;
  }
  반환 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static 정수 pushline (lua_State *L, 정수 firstline) {
  char buffer[LUA_MAXINPUT];
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  char *b = lua_readline(buffer, prmt);
  lua_pop(L, 1);  /* remove prompt */
  만약 (b == NULL)
    반환 0;  /* no input */
  l = strlen(b);
  만약 (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  반환 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static 정수 addreturn (lua_State *L) {
  const char *line = lua_tostring(L, -1);  /* original line */
  const char *retline = lua_pushfstring(L, "return %s;", line);
  정수 status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  만약 (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  아니면
    lua_pop(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  반환 status;
}


static void checklocal (const char *line) {
  static const size_t szloc = sizeof("local") - 1;
  static const char space[] = " \t";
  line += strspn(line, space);  /* skip spaces */
  만약 (strncmp(line, "local", szloc) == 0 &&  /* "local"? */
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
static 정수 multiline (lua_State *L) {
  size_t len;
  const char *line = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(line);
  위해 (;;) {  /* repeat until gets a complete statement */
    정수 status = luaL_loadbufferx(L, line, len, "=stdin", "t");  /* try it */
    만약 (!incomplete(L, status) || !pushline(L, 0))
      반환 status;  /* should not or cannot try to add continuation line */
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
static 정수 loadline (lua_State *L) {
  const char *line;
  정수 status;
  lua_settop(L, 0);
  만약 (!pushline(L, 1))
    반환 -1;  /* no input */
  만약 ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  line = lua_tostring(L, 1);
  만약 (line[0] != '\0')  /* non empty? */
    lua_saveline(line);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  반환 status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
  정수 n = lua_gettop(L);
  만약 (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    만약 (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL (lua_State *L) {
  정수 status;
  const char *oldprogname = progname;
  progname = NULL;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  동안 ((status = loadline(L)) != -1) {
    만약 (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    만약 (status == LUA_OK) l_print(L);
    아니면 report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#만약 !defined(luai_openlibs)
#만약 defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#아니면
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static 정수 pmain (lua_State *L) {
  정수 argc = (정수)lua_tointeger(L, 1);
  char **argv = (char **)lua_touserdata(L, 2);
  정수 script;
  정수 args = collectargs(argv, &script);
  정수 optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  만약 (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    반환 0;
  }
  만약 (args & has_v)  /* option '-v'? */
    print_version();
  만약 (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  아니면
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  만약 (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    반환 0;  /* error running LUA_INIT */
  만약 (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    반환 0;  /* something failed */
  만약 (script > 0) {  /* execute main script (if there is one) */
    만약 (handle_script(L, argv + script) != LUA_OK)
      반환 0;  /* interrupt in case of error */
  }
  만약 (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  아니면 만약 (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    만약 (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    아니면 dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  반환 1;
}


정수 main (정수 argc, char **argv) {
  정수 status, result;
  lua_State *L = luaL_newstate();  /* create state */
  만약 (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    반환 EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  반환 (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

