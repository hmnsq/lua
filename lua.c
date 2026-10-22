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


#ប្រសិនបើ !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#ប្រសិនបើ !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#ប្រសិនបើ !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


static lua_State *globalL = NULL;

static const var_na *progname = LUA_PROGNAME;


#ប្រសិនបើ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
static void setsignal (ចំនួនគត់ sig, void (*handler)(ចំនួនគត់)) {
  sa_nracanaa sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, NULL);
}

#បើមិនដូច្នេះ           /* }{ */

#define setsignal            sa_nkeeta

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
static void laction (ចំនួនគត់ i) {
  ចំនួនគត់ flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


static void print_usage (const var_na *badoption) {
  lua_writestringerror("%s: ", progname);
  ប្រសិនបើ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  បើមិនដូច្នេះ
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
static void l_message (const var_na *pname, const var_na *msg) {
  ប្រសិនបើ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
static ចំនួនគត់ report (lua_State *L, ចំនួនគត់ status) {
  ប្រសិនបើ (status != LUA_OK) {
    const var_na *msg = lua_tostring(L, -1);
    ប្រសិនបើ (msg == NULL)
      msg = "(error message not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  ត្រឡប់ status;
}


/*
** Message handler used to run all chunks
*/
static ចំនួនគត់ msghandler (lua_State *L) {
  const var_na *msg = lua_tostring(L, 1);
  ប្រសិនបើ (msg == NULL) {  /* is error object not a string? */
    ប្រសិនបើ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ត្រឡប់ 1;  /* that is the message */
    បើមិនដូច្នេះ
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  ត្រឡប់ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static ចំនួនគត់ docall (lua_State *L, ចំនួនគត់ narg, ចំនួនគត់ nres) {
  ចំនួនគត់ status;
  ចំនួនគត់ base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  ត្រឡប់ status;
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
static void createargtable (lua_State *L, var_na **argv, ចំនួនគត់ argc, ចំនួនគត់ script) {
  ចំនួនគត់ i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  សម្រាប់ (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static ចំនួនគត់ dochunk (lua_State *L, ចំនួនគត់ status) {
  ប្រសិនបើ (status == LUA_OK) status = docall(L, 0, 0);
  ត្រឡប់ report(L, status);
}


static ចំនួនគត់ dofile (lua_State *L, const var_na *name) {
  ត្រឡប់ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


static ចំនួនគត់ dostring (lua_State *L, const var_na *s, const var_na *name) {
  ត្រឡប់ dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
static ចំនួនគត់ dolibrary (lua_State *L, var_na *globname) {
  ចំនួនគត់ status;
  var_na *suffix = NULL;
  var_na *modname = strchr(globname, '=');
  ប្រសិនបើ (modname == NULL) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  បើមិនដូច្នេះ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  ប្រសិនបើ (status == LUA_OK) {
    ប្រសិនបើ (suffix != NULL)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ត្រឡប់ report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static ចំនួនគត់ pushargs (lua_State *L) {
  ចំនួនគត់ i, n;
  ប្រសិនបើ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ចំនួនគត់)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  សម្រាប់ (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ត្រឡប់ n;
}


static ចំនួនគត់ handle_script (lua_State *L, var_na **argv) {
  ចំនួនគត់ status;
  const var_na *fname = argv[0];
  ប្រសិនបើ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  ប្រសិនបើ (status == LUA_OK) {
    ចំនួនគត់ n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  ត្រឡប់ report(L, status);
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
static ចំនួនគត់ collectargs (var_na **argv, ចំនួនគត់ *first) {
  ចំនួនគត់ args = 0;
  ចំនួនគត់ i;
  ប្រសិនបើ (argv[0] != NULL) {  /* is there a program name? */
    ប្រសិនបើ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  បើមិនដូច្នេះ {  /* no program name */
    *first = -1;
    ត្រឡប់ 0;
  }
  សម្រាប់ (i = 1; argv[i] != NULL; i++) {  /* handle arguments */
    *first = i;
    ប្រសិនបើ (argv[i][0] != '-')  /* not an option? */
        ត្រឡប់ args;  /* stop handling options */
    switch (argv[i][1]) {  /* else check option */
      pak_sa '-':  /* '--' */
        ប្រសិនបើ (argv[i][2] != '\0')  /* extra characters after '--'? */
          ត្រឡប់ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != NULL) ? i + 1 : 0;
        ត្រឡប់ args;
      pak_sa '\0':  /* '-' */
        ត្រឡប់ args;  /* script "name" is '-' */
      pak_sa 'E':
        ប្រសិនបើ (argv[i][2] != '\0')  /* extra characters? */
          ត្រឡប់ has_error;  /* invalid option */
        args |= has_E;
        bha_nga;
      pak_sa 'W':
        ប្រសិនបើ (argv[i][2] != '\0')  /* extra characters? */
          ត្រឡប់ has_error;  /* invalid option */
        bha_nga;
      pak_sa 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      pak_sa 'v':
        ប្រសិនបើ (argv[i][2] != '\0')  /* extra characters? */
          ត្រឡប់ has_error;  /* invalid option */
        args |= has_v;
        bha_nga;
      pak_sa 'e':
        args |= has_e;  /* FALLTHROUGH */
      pak_sa 'l':  /* both options need an argument */
        ប្រសិនបើ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ប្រសិនបើ (argv[i] == NULL || argv[i][0] == '-')
            ត្រឡប់ has_error;  /* no next argument or it is another option */
        }
        bha_nga;
      default:  /* invalid option */
        ត្រឡប់ has_error;
    }
  }
  *first = 0;  /* no script name */
  ត្រឡប់ args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static ចំនួនគត់ runargs (lua_State *L, var_na **argv, ចំនួនគត់ n) {
  ចំនួនគត់ i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  សម្រាប់ (i = 1; i < n; i++) {
    ចំនួនគត់ option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    switch (option) {
      pak_sa 'e':  pak_sa 'l': {
        ចំនួនគត់ status;
        var_na *extra = argv[i] + 2;  /* both options need an argument */
        ប្រសិនបើ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != NULL);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ប្រសិនបើ (status != LUA_OK) ត្រឡប់ 0;
        bha_nga;
      }
      pak_sa 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        bha_nga;
    }
  }
  ត្រឡប់ 1;
}


static var_na *(*l_getenv)(const var_na *name);

/* Function to ignore environment variables, used by option -E */
static var_na *no_getenv (const var_na *name) {
  UNUSED(name);
  ត្រឡប់ NULL;
}


static ចំនួនគត់ handle_luainit (lua_State *L) {
  const var_na *name = "=" LUA_INITVARVERSION;
  const var_na *init = l_getenv(name + 1);
  ប្រសិនបើ (init == NULL) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ប្រសិនបើ (init == NULL) ត្រឡប់ LUA_OK;
  បើមិនដូច្នេះ ប្រសិនបើ (init[0] == '@')
    ត្រឡប់ dofile(L, init+1);
  បើមិនដូច្នេះ
    ត្រឡប់ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ប្រសិនបើ !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#ប្រសិនបើ !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ប្រសិនបើ !defined(lua_stdin_is_tty)	/* { */

#ប្រសិនបើ defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#បើមិនដូច្នេះ				/* }{ */

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

#ប្រសិនបើ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ប្រសិនបើ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#include <readline/readline.h>
#include <readline/history.h>

#define lua_initreadline(L)  \
	((void)L, *cast(const var_na **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((void)buff, readline(prompt))
#define lua_saveline(line)	add_history(line)
#define lua_freeline(line)	free(line)

#បើមិនដូច្នេះ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
typedef var_na *(*l_readlineT) (const var_na *prompt);
static l_readlineT l_readline = NULL;

/* pointer to 'add_history' function (if any) */
typedef void (*l_addhistT) (const var_na *string);
static l_addhistT l_addhist = NULL;


static var_na *lua_readline (var_na *buff, const var_na *prompt) {
  ប្រសិនបើ (l_readline != NULL)  /* is there a 'readline'? */
    ត្រឡប់ (*l_readline)(prompt);  /* use it */
  បើមិនដូច្នេះ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ត្រឡប់ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


static void lua_saveline (const var_na *line) {
  ប្រសិនបើ (l_addhist != NULL)  /* is there an 'add_history'? */
    (*l_addhist)(line);  /* use it */
  /* else nothing to be done */
}


static void lua_freeline (var_na *line) {
  ប្រសិនបើ (l_readline != NULL)  /* is there a 'readline'? */
    free(line);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ប្រសិនបើ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#include <dlfcn.h>

static void lua_initreadline (lua_State *L) {
  const var_na *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  void *lib;  /* library handle */
  ប្រសិនបើ (rllib == NULL)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ប្រសិនបើ (lib != NULL) {
    const var_na **name = cast(const var_na**, dlsym(lib, "rl_readline_name"));
    ប្រសិនបើ (name != NULL)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ប្រសិនបើ (l_readline != NULL)  /* could load readline function? */
      ត្រឡប់;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#បើមិនដូច្នេះ		/* }{ */
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
static const var_na *get_prompt (lua_State *L, ចំនួនគត់ firstline) {
  ប្រសិនបើ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ត្រឡប់ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  បើមិនដូច្នេះ {  /* apply 'tostring' over the value */
    const var_na *p = luaL_tolstring(L, -1, NULL);
    lua_remove(L, -2);  /* remove original value */
    ត្រឡប់ p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(parimaa_na(EOFMARK)/parimaa_na(var_na) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static ចំនួនគត់ incomplete (lua_State *L, ចំនួនគត់ status) {
  ប្រសិនបើ (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const var_na *msg = lua_tolstring(L, -1, &lmsg);
    ប្រសិនបើ (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
      ត្រឡប់ 1;
  }
  ត្រឡប់ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static ចំនួនគត់ pushline (lua_State *L, ចំនួនគត់ firstline) {
  var_na buffer[LUA_MAXINPUT];
  size_t l;
  const var_na *prmt = get_prompt(L, firstline);
  var_na *b = lua_readline(buffer, prmt);
  lua_pop(L, 1);  /* remove prompt */
  ប្រសិនបើ (b == NULL)
    ត្រឡប់ 0;  /* no input */
  l = strlen(b);
  ប្រសិនបើ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ត្រឡប់ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static ចំនួនគត់ addreturn (lua_State *L) {
  const var_na *line = lua_tostring(L, -1);  /* original line */
  const var_na *retline = lua_pushfstring(L, "return %s;", line);
  ចំនួនគត់ status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  ប្រសិនបើ (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  បើមិនដូច្នេះ
    lua_pop(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ត្រឡប់ status;
}


static void checklocal (const var_na *line) {
  static const size_t szloc = parimaa_na("local") - 1;
  static const var_na space[] = " \t";
  line += strspn(line, space);  /* skip spaces */
  ប្រសិនបើ (strncmp(line, "local", szloc) == 0 &&  /* "local"? */
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
static ចំនួនគត់ multiline (lua_State *L) {
  size_t len;
  const var_na *line = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(line);
  សម្រាប់ (;;) {  /* repeat until gets a complete statement */
    ចំនួនគត់ status = luaL_loadbufferx(L, line, len, "=stdin", "t");  /* try it */
    ប្រសិនបើ (!incomplete(L, status) || !pushline(L, 0))
      ត្រឡប់ status;  /* should not or cannot try to add continuation line */
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
static ចំនួនគត់ loadline (lua_State *L) {
  const var_na *line;
  ចំនួនគត់ status;
  lua_settop(L, 0);
  ប្រសិនបើ (!pushline(L, 1))
    ត្រឡប់ -1;  /* no input */
  ប្រសិនបើ ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  line = lua_tostring(L, 1);
  ប្រសិនបើ (line[0] != '\0')  /* non empty? */
    lua_saveline(line);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ត្រឡប់ status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
  ចំនួនគត់ n = lua_gettop(L);
  ប្រសិនបើ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    ប្រសិនបើ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL (lua_State *L) {
  ចំនួនគត់ status;
  const var_na *oldprogname = progname;
  progname = NULL;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ខណៈពេល ((status = loadline(L)) != -1) {
    ប្រសិនបើ (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    ប្រសិនបើ (status == LUA_OK) l_print(L);
    បើមិនដូច្នេះ report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ប្រសិនបើ !defined(luai_openlibs)
#ប្រសិនបើ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#បើមិនដូច្នេះ
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static ចំនួនគត់ pmain (lua_State *L) {
  ចំនួនគត់ argc = (ចំនួនគត់)lua_tointeger(L, 1);
  var_na **argv = (var_na **)lua_touserdata(L, 2);
  ចំនួនគត់ script;
  ចំនួនគត់ args = collectargs(argv, &script);
  ចំនួនគត់ optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ប្រសិនបើ (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ត្រឡប់ 0;
  }
  ប្រសិនបើ (args & has_v)  /* option '-v'? */
    print_version();
  ប្រសិនបើ (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  បើមិនដូច្នេះ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ប្រសិនបើ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ត្រឡប់ 0;  /* error running LUA_INIT */
  ប្រសិនបើ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ត្រឡប់ 0;  /* something failed */
  ប្រសិនបើ (script > 0) {  /* execute main script (if there is one) */
    ប្រសិនបើ (handle_script(L, argv + script) != LUA_OK)
      ត្រឡប់ 0;  /* interrupt in case of error */
  }
  ប្រសិនបើ (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  បើមិនដូច្នេះ ប្រសិនបើ (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    ប្រសិនបើ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    បើមិនដូច្នេះ dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ត្រឡប់ 1;
}


ចំនួនគត់ main (ចំនួនគត់ argc, var_na **argv) {
  ចំនួនគត់ status, result;
  lua_State *L = luaL_newstate();  /* create state */
  ប្រសិនបើ (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    ត្រឡប់ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  ត្រឡប់ (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

