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


#නම් !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#නම් !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#නම් !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


static lua_State *globalL = NULL;

static const var_na *progname = LUA_PROGNAME;


#නම් defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
static void setsignal (පූර්ණ_සංඛ්‍යා sig, void (*handler)(පූර්ණ_සංඛ්‍යා)) {
  sa_nracanaa sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, NULL);
}

#නැතහොත්           /* }{ */

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
static void laction (පූර්ණ_සංඛ්‍යා i) {
  පූර්ණ_සංඛ්‍යා flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


static void print_usage (const var_na *badoption) {
  lua_writestringerror("%s: ", progname);
  නම් (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  නැතහොත්
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
  නම් (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
static පූර්ණ_සංඛ්‍යා report (lua_State *L, පූර්ණ_සංඛ්‍යා status) {
  නම් (status != LUA_OK) {
    const var_na *msg = lua_tostring(L, -1);
    නම් (msg == NULL)
      msg = "(error message not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  ආපසු status;
}


/*
** Message handler used to run all chunks
*/
static පූර්ණ_සංඛ්‍යා msghandler (lua_State *L) {
  const var_na *msg = lua_tostring(L, 1);
  නම් (msg == NULL) {  /* is error object not a string? */
    නම් (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ආපසු 1;  /* that is the message */
    නැතහොත්
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  ආපසු 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static පූර්ණ_සංඛ්‍යා docall (lua_State *L, පූර්ණ_සංඛ්‍යා narg, පූර්ණ_සංඛ්‍යා nres) {
  පූර්ණ_සංඛ්‍යා status;
  පූර්ණ_සංඛ්‍යා base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  ආපසු status;
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
static void createargtable (lua_State *L, var_na **argv, පූර්ණ_සංඛ්‍යා argc, පූර්ණ_සංඛ්‍යා script) {
  පූර්ණ_සංඛ්‍යා i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  සඳහා (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static පූර්ණ_සංඛ්‍යා dochunk (lua_State *L, පූර්ණ_සංඛ්‍යා status) {
  නම් (status == LUA_OK) status = docall(L, 0, 0);
  ආපසු report(L, status);
}


static පූර්ණ_සංඛ්‍යා dofile (lua_State *L, const var_na *name) {
  ආපසු dochunk(L, luaL_loadfilex(L, name, "bt"));
}


static පූර්ණ_සංඛ්‍යා dostring (lua_State *L, const var_na *s, const var_na *name) {
  ආපසු dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
static පූර්ණ_සංඛ්‍යා dolibrary (lua_State *L, var_na *globname) {
  පූර්ණ_සංඛ්‍යා status;
  var_na *suffix = NULL;
  var_na *modname = strchr(globname, '=');
  නම් (modname == NULL) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  නැතහොත් {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  නම් (status == LUA_OK) {
    නම් (suffix != NULL)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ආපසු report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static පූර්ණ_සංඛ්‍යා pushargs (lua_State *L) {
  පූර්ණ_සංඛ්‍යා i, n;
  නම් (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (පූර්ණ_සංඛ්‍යා)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  සඳහා (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ආපසු n;
}


static පූර්ණ_සංඛ්‍යා handle_script (lua_State *L, var_na **argv) {
  පූර්ණ_සංඛ්‍යා status;
  const var_na *fname = argv[0];
  නම් (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  නම් (status == LUA_OK) {
    පූර්ණ_සංඛ්‍යා n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  ආපසු report(L, status);
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
static පූර්ණ_සංඛ්‍යා collectargs (var_na **argv, පූර්ණ_සංඛ්‍යා *first) {
  පූර්ණ_සංඛ්‍යා args = 0;
  පූර්ණ_සංඛ්‍යා i;
  නම් (argv[0] != NULL) {  /* is there a program name? */
    නම් (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  නැතහොත් {  /* no program name */
    *first = -1;
    ආපසු 0;
  }
  සඳහා (i = 1; argv[i] != NULL; i++) {  /* handle arguments */
    *first = i;
    නම් (argv[i][0] != '-')  /* not an option? */
        ආපසු args;  /* stop handling options */
    switch (argv[i][1]) {  /* else check option */
      pak_sa '-':  /* '--' */
        නම් (argv[i][2] != '\0')  /* extra characters after '--'? */
          ආපසු has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != NULL) ? i + 1 : 0;
        ආපසු args;
      pak_sa '\0':  /* '-' */
        ආපසු args;  /* script "name" is '-' */
      pak_sa 'E':
        නම් (argv[i][2] != '\0')  /* extra characters? */
          ආපසු has_error;  /* invalid option */
        args |= has_E;
        bha_nga;
      pak_sa 'W':
        නම් (argv[i][2] != '\0')  /* extra characters? */
          ආපසු has_error;  /* invalid option */
        bha_nga;
      pak_sa 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      pak_sa 'v':
        නම් (argv[i][2] != '\0')  /* extra characters? */
          ආපසු has_error;  /* invalid option */
        args |= has_v;
        bha_nga;
      pak_sa 'e':
        args |= has_e;  /* FALLTHROUGH */
      pak_sa 'l':  /* both options need an argument */
        නම් (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          නම් (argv[i] == NULL || argv[i][0] == '-')
            ආපසු has_error;  /* no next argument or it is another option */
        }
        bha_nga;
      default:  /* invalid option */
        ආපසු has_error;
    }
  }
  *first = 0;  /* no script name */
  ආපසු args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static පූර්ණ_සංඛ්‍යා runargs (lua_State *L, var_na **argv, පූර්ණ_සංඛ්‍යා n) {
  පූර්ණ_සංඛ්‍යා i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  සඳහා (i = 1; i < n; i++) {
    පූර්ණ_සංඛ්‍යා option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    switch (option) {
      pak_sa 'e':  pak_sa 'l': {
        පූර්ණ_සංඛ්‍යා status;
        var_na *extra = argv[i] + 2;  /* both options need an argument */
        නම් (*extra == '\0') extra = argv[++i];
        lua_assert(extra != NULL);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        නම් (status != LUA_OK) ආපසු 0;
        bha_nga;
      }
      pak_sa 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        bha_nga;
    }
  }
  ආපසු 1;
}


static var_na *(*l_getenv)(const var_na *name);

/* Function to ignore environment variables, used by option -E */
static var_na *no_getenv (const var_na *name) {
  UNUSED(name);
  ආපසු NULL;
}


static පූර්ණ_සංඛ්‍යා handle_luainit (lua_State *L) {
  const var_na *name = "=" LUA_INITVARVERSION;
  const var_na *init = l_getenv(name + 1);
  නම් (init == NULL) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  නම් (init == NULL) ආපසු LUA_OK;
  නැතහොත් නම් (init[0] == '@')
    ආපසු dofile(L, init+1);
  නැතහොත්
    ආපසු dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#නම් !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#නම් !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#නම් !defined(lua_stdin_is_tty)	/* { */

#නම් defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#නැතහොත්				/* }{ */

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

#නම් !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#නම් defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#include <readline/readline.h>
#include <readline/history.h>

#define lua_initreadline(L)  \
	((void)L, *cast(const var_na **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((void)buff, readline(prompt))
#define lua_saveline(line)	add_history(line)
#define lua_freeline(line)	free(line)

#නැතහොත්		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
typedef var_na *(*l_readlineT) (const var_na *prompt);
static l_readlineT l_readline = NULL;

/* pointer to 'add_history' function (if any) */
typedef void (*l_addhistT) (const var_na *string);
static l_addhistT l_addhist = NULL;


static var_na *lua_readline (var_na *buff, const var_na *prompt) {
  නම් (l_readline != NULL)  /* is there a 'readline'? */
    ආපසු (*l_readline)(prompt);  /* use it */
  නැතහොත් {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ආපසු fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


static void lua_saveline (const var_na *line) {
  නම් (l_addhist != NULL)  /* is there an 'add_history'? */
    (*l_addhist)(line);  /* use it */
  /* else nothing to be done */
}


static void lua_freeline (var_na *line) {
  නම් (l_readline != NULL)  /* is there a 'readline'? */
    free(line);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#නම් defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#include <dlfcn.h>

static void lua_initreadline (lua_State *L) {
  const var_na *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  void *lib;  /* library handle */
  නම් (rllib == NULL)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  නම් (lib != NULL) {
    const var_na **name = cast(const var_na**, dlsym(lib, "rl_readline_name"));
    නම් (name != NULL)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    නම් (l_readline != NULL)  /* could load readline function? */
      ආපසු;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#නැතහොත්		/* }{ */
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
static const var_na *get_prompt (lua_State *L, පූර්ණ_සංඛ්‍යා firstline) {
  නම් (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ආපසු (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  නැතහොත් {  /* apply 'tostring' over the value */
    const var_na *p = luaL_tolstring(L, -1, NULL);
    lua_remove(L, -2);  /* remove original value */
    ආපසු p;
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
static පූර්ණ_සංඛ්‍යා incomplete (lua_State *L, පූර්ණ_සංඛ්‍යා status) {
  නම් (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const var_na *msg = lua_tolstring(L, -1, &lmsg);
    නම් (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
      ආපසු 1;
  }
  ආපසු 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static පූර්ණ_සංඛ්‍යා pushline (lua_State *L, පූර්ණ_සංඛ්‍යා firstline) {
  var_na buffer[LUA_MAXINPUT];
  size_t l;
  const var_na *prmt = get_prompt(L, firstline);
  var_na *b = lua_readline(buffer, prmt);
  lua_pop(L, 1);  /* remove prompt */
  නම් (b == NULL)
    ආපසු 0;  /* no input */
  l = strlen(b);
  නම් (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ආපසු 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static පූර්ණ_සංඛ්‍යා addreturn (lua_State *L) {
  const var_na *line = lua_tostring(L, -1);  /* original line */
  const var_na *retline = lua_pushfstring(L, "return %s;", line);
  පූර්ණ_සංඛ්‍යා status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  නම් (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  නැතහොත්
    lua_pop(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ආපසු status;
}


static void checklocal (const var_na *line) {
  static const size_t szloc = parimaa_na("local") - 1;
  static const var_na space[] = " \t";
  line += strspn(line, space);  /* skip spaces */
  නම් (strncmp(line, "local", szloc) == 0 &&  /* "local"? */
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
static පූර්ණ_සංඛ්‍යා multiline (lua_State *L) {
  size_t len;
  const var_na *line = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(line);
  සඳහා (;;) {  /* repeat until gets a complete statement */
    පූර්ණ_සංඛ්‍යා status = luaL_loadbufferx(L, line, len, "=stdin", "t");  /* try it */
    නම් (!incomplete(L, status) || !pushline(L, 0))
      ආපසු status;  /* should not or cannot try to add continuation line */
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
static පූර්ණ_සංඛ්‍යා loadline (lua_State *L) {
  const var_na *line;
  පූර්ණ_සංඛ්‍යා status;
  lua_settop(L, 0);
  නම් (!pushline(L, 1))
    ආපසු -1;  /* no input */
  නම් ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  line = lua_tostring(L, 1);
  නම් (line[0] != '\0')  /* non empty? */
    lua_saveline(line);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ආපසු status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
  පූර්ණ_සංඛ්‍යා n = lua_gettop(L);
  නම් (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    නම් (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL (lua_State *L) {
  පූර්ණ_සංඛ්‍යා status;
  const var_na *oldprogname = progname;
  progname = NULL;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  අතරතුර ((status = loadline(L)) != -1) {
    නම් (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    නම් (status == LUA_OK) l_print(L);
    නැතහොත් report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#නම් !defined(luai_openlibs)
#නම් defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#නැතහොත්
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static පූර්ණ_සංඛ්‍යා pmain (lua_State *L) {
  පූර්ණ_සංඛ්‍යා argc = (පූර්ණ_සංඛ්‍යා)lua_tointeger(L, 1);
  var_na **argv = (var_na **)lua_touserdata(L, 2);
  පූර්ණ_සංඛ්‍යා script;
  පූර්ණ_සංඛ්‍යා args = collectargs(argv, &script);
  පූර්ණ_සංඛ්‍යා optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  නම් (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ආපසු 0;
  }
  නම් (args & has_v)  /* option '-v'? */
    print_version();
  නම් (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  නැතහොත්
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  නම් (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ආපසු 0;  /* error running LUA_INIT */
  නම් (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ආපසු 0;  /* something failed */
  නම් (script > 0) {  /* execute main script (if there is one) */
    නම් (handle_script(L, argv + script) != LUA_OK)
      ආපසු 0;  /* interrupt in case of error */
  }
  නම් (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  නැතහොත් නම් (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    නම් (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    නැතහොත් dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ආපසු 1;
}


පූර්ණ_සංඛ්‍යා main (පූර්ණ_සංඛ්‍යා argc, var_na **argv) {
  පූර්ණ_සංඛ්‍යා status, result;
  lua_State *L = luaL_newstate();  /* create state */
  නම් (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    ආපසු EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  ආපසු (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

