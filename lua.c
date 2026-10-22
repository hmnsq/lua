/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ <signal.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


#ଯଦି !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#ଯଦି !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#ଯଦି !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ସ୍ଥିର lua_State *globalL = ଖାଲି;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *progname = LUA_PROGNAME;


#ଯଦି defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ସ୍ଥିର ଶୂନ୍ୟ setsignal (ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, ଶୂନ୍ୟ (*handler)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)) {
  ସଂରଚନା sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ଖାଲି);
}

#ଅନ୍ୟଥା           /* }{ */

#define setsignal            ସଙ୍କେତ

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ସ୍ଥିର ଶୂନ୍ୟ lstop (lua_State *L, lua_Debug *ar) {
  (ଶୂନ୍ୟ)ar;  /* unused arg. */
  lua_sethook(L, ଖାଲି, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ସ୍ଥିର ଶୂନ୍ୟ laction (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ସ୍ଥିର ଶୂନ୍ୟ print_usage (ଅଚଳ ଅକ୍ଷର *badoption) {
  lua_writestringerror("%s: ", progname);
  ଯଦି (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  ଅନ୍ୟଥା
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
ସ୍ଥିର ଶୂନ୍ୟ l_message (ଅଚଳ ଅକ୍ଷର *pname, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  ଯଦି (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", ସନ୍ଦେଶ);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା report (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status) {
  ଯଦି (status != LUA_OK) {
    ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = lua_tostring(L, -1);
    ଯଦି (ସନ୍ଦେଶ == ଖାଲି)
      ସନ୍ଦେଶ = "(error message not a string)";
    l_message(progname, ସନ୍ଦେଶ);
    lua_କାଢ଼(L, 1);  /* remove message */
  }
  ଫେରନ୍ତୁ status;
}


/*
** Message handler used to run all chunks
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା msghandler (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = lua_tostring(L, 1);
  ଯଦି (ସନ୍ଦେଶ == ଖାଲି) {  /* is error object not a string? */
    ଯଦି (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ଫେରନ୍ତୁ 1;  /* that is the message */
    ଅନ୍ୟଥା
      ସନ୍ଦେଶ = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, ସନ୍ଦେଶ, 1);  /* append a standard traceback */
  ଫେରନ୍ତୁ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା docall (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_ଭର୍ତ୍ତି(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  ଫେରନ୍ତୁ status;
}


ସ୍ଥିର ଶୂନ୍ୟ print_version (ଶୂନ୍ୟ) {
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
ସ୍ଥିର ଶୂନ୍ୟ createargtable (lua_State *L, ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା script) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  ପାଇଁ (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dochunk (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status) {
  ଯଦି (status == LUA_OK) status = docall(L, 0, 0);
  ଫେରନ୍ତୁ report(L, status);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dofile (lua_State *L, ଅଚଳ ଅକ୍ଷର *name) {
  ଫେରନ୍ତୁ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dostring (lua_State *L, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *name) {
  ଫେରନ୍ତୁ dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dolibrary (lua_State *L, ଅକ୍ଷର *globname) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ଅକ୍ଷର *suffix = ଖାଲି;
  ଅକ୍ଷର *modname = strchr(globname, '=');
  ଯଦି (modname == ଖାଲି) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  ଅନ୍ୟଥା {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  ଯଦି (status == LUA_OK) {
    ଯଦି (suffix != ଖାଲି)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ଫେରନ୍ତୁ report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pushargs (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
  ଯଦି (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  ପାଇଁ (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ଫେରନ୍ତୁ n;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_script (lua_State *L, ଅକ୍ଷର **argv) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ଅଚଳ ଅକ୍ଷର *fname = argv[0];
  ଯଦି (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ଖାଲି;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  ଯଦି (status == LUA_OK) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  ଫେରନ୍ତୁ report(L, status);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା collectargs (ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *first) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା args = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି (argv[0] != ଖାଲି) {  /* is there a program name? */
    ଯଦି (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  ଅନ୍ୟଥା {  /* no program name */
    *first = -1;
    ଫେରନ୍ତୁ 0;
  }
  ପାଇଁ (i = 1; argv[i] != ଖାଲି; i++) {  /* handle arguments */
    *first = i;
    ଯଦି (argv[i][0] != '-')  /* not an option? */
        ଫେରନ୍ତୁ args;  /* stop handling options */
    ବାଛ (argv[i][1]) {  /* else check option */
      ସ୍ଥିତି '-':  /* '--' */
        ଯଦି (argv[i][2] != '\0')  /* extra characters after '--'? */
          ଫେରନ୍ତୁ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != ଖାଲି) ? i + 1 : 0;
        ଫେରନ୍ତୁ args;
      ସ୍ଥିତି '\0':  /* '-' */
        ଫେରନ୍ତୁ args;  /* script "name" is '-' */
      ସ୍ଥିତି 'E':
        ଯଦି (argv[i][2] != '\0')  /* extra characters? */
          ଫେରନ୍ତୁ has_error;  /* invalid option */
        args |= has_E;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 'W':
        ଯଦି (argv[i][2] != '\0')  /* extra characters? */
          ଫେରନ୍ତୁ has_error;  /* invalid option */
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      ସ୍ଥିତି 'v':
        ଯଦି (argv[i][2] != '\0')  /* extra characters? */
          ଫେରନ୍ତୁ has_error;  /* invalid option */
        args |= has_v;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 'e':
        args |= has_e;  /* FALLTHROUGH */
      ସ୍ଥିତି 'l':  /* both options need an argument */
        ଯଦି (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ଯଦି (argv[i] == ଖାଲି || argv[i][0] == '-')
            ଫେରନ୍ତୁ has_error;  /* no next argument or it is another option */
        }
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* invalid option */
        ଫେରନ୍ତୁ has_error;
    }
  }
  *first = 0;  /* no script name */
  ଫେରନ୍ତୁ args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା runargs (lua_State *L, ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  ପାଇଁ (i = 1; i < n; i++) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    ବାଛ (option) {
      ସ୍ଥିତି 'e':  ସ୍ଥିତି 'l': {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
        ଅକ୍ଷର *extra = argv[i] + 2;  /* both options need an argument */
        ଯଦି (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ଖାଲି);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ଯଦି (status != LUA_OK) ଫେରନ୍ତୁ 0;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଅକ୍ଷର *(*l_getenv)(ଅଚଳ ଅକ୍ଷର *name);

/* Function to ignore environment variables, used by option -E */
ସ୍ଥିର ଅକ୍ଷର *no_getenv (ଅଚଳ ଅକ୍ଷର *name) {
  UNUSED(name);
  ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_luainit (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *name = "=" LUA_INITVARVERSION;
  ଅଚଳ ଅକ୍ଷର *init = l_getenv(name + 1);
  ଯଦି (init == ଖାଲି) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ଯଦି (init == ଖାଲି) ଫେରନ୍ତୁ LUA_OK;
  ଅନ୍ୟଥା ଯଦି (init[0] == '@')
    ଫେରନ୍ତୁ dofile(L, init+1);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ଯଦି !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#ଯଦି !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ଯଦି !defined(lua_stdin_is_tty)	/* { */

#ଯଦି defined(LUA_USE_POSIX)	/* { */

#ଅନ୍ତର୍ଭାଵ <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#ନଚେତଯଦି defined(LUA_USE_WINDOWS)	/* }{ */

#ଅନ୍ତର୍ଭାଵ <io.h>
#ଅନ୍ତର୍ଭାଵ <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#ଅନ୍ୟଥା				/* }{ */

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

#ଯଦି !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ଯଦି defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ଅନ୍ତର୍ଭାଵ <readline/readline.h>
#ଅନ୍ତର୍ଭାଵ <readline/history.h>

#define lua_initreadline(L)  \
	((ଶୂନ୍ୟ)L, *cast(ଅଚଳ ଅକ୍ଷର **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((ଶୂନ୍ୟ)buff, readline(prompt))
#define lua_saveline(ଧାଡ଼ି)	add_history(ଧାଡ଼ି)
#define lua_freeline(ଧାଡ଼ି)	ମୁକ୍ତ(ଧାଡ଼ି)

#ଅନ୍ୟଥା		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅକ୍ଷର *(*l_readlineT) (ଅଚଳ ଅକ୍ଷର *prompt);
ସ୍ଥିର l_readlineT l_readline = ଖାଲି;

/* pointer to 'add_history' function (if any) */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ (*l_addhistT) (ଅଚଳ ଅକ୍ଷର *string);
ସ୍ଥିର l_addhistT l_addhist = ଖାଲି;


ସ୍ଥିର ଅକ୍ଷର *lua_readline (ଅକ୍ଷର *buff, ଅଚଳ ଅକ୍ଷର *prompt) {
  ଯଦି (l_readline != ଖାଲି)  /* is there a 'readline'? */
    ଫେରନ୍ତୁ (*l_readline)(prompt);  /* use it */
  ଅନ୍ୟଥା {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ଫେରନ୍ତୁ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ lua_saveline (ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି) {
  ଯଦି (l_addhist != ଖାଲି)  /* is there an 'add_history'? */
    (*l_addhist)(ଧାଡ଼ି);  /* use it */
  /* else nothing to be done */
}


ସ୍ଥିର ଶୂନ୍ୟ lua_freeline (ଅକ୍ଷର *ଧାଡ଼ି) {
  ଯଦି (l_readline != ଖାଲି)  /* is there a 'readline'? */
    ମୁକ୍ତ(ଧାଡ଼ି);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ଯଦି defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ଅନ୍ତର୍ଭାଵ <dlfcn.h>

ସ୍ଥିର ଶୂନ୍ୟ lua_initreadline (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  ଶୂନ୍ୟ *lib;  /* library handle */
  ଯଦି (rllib == ଖାଲି)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ଯଦି (lib != ଖାଲି) {
    ଅଚଳ ଅକ୍ଷର **name = cast(ଅଚଳ ଅକ୍ଷର**, dlsym(lib, "rl_readline_name"));
    ଯଦି (name != ଖାଲି)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ଯଦି (l_readline != ଖାଲି)  /* could load readline function? */
      ଫେରନ୍ତୁ;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#ଅନ୍ୟଥା		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((ଶୂନ୍ୟ)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get_prompt (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstline) {
  ଯଦି (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ଫେରନ୍ତୁ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  ଅନ୍ୟଥା {  /* apply 'tostring' over the value */
    ଅଚଳ ଅକ୍ଷର *p = luaL_tolstring(L, -1, ଖାଲି);
    lua_remove(L, -2);  /* remove original value */
    ଫେରନ୍ତୁ p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(ଆକାର(EOFMARK)/ଆକାର(ଅକ୍ଷର) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା incomplete (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status) {
  ଯଦି (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = lua_tolstring(L, -1, &lmsg);
    ଯଦି (lmsg >= marklen && strcmp(ସନ୍ଦେଶ + lmsg - marklen, EOFMARK) == 0)
      ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pushline (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstline) {
  ଅକ୍ଷର buffer[LUA_MAXINPUT];
  size_t l;
  ଅଚଳ ଅକ୍ଷର *prmt = get_prompt(L, firstline);
  ଅକ୍ଷର *b = lua_readline(buffer, prmt);
  lua_କାଢ଼(L, 1);  /* remove prompt */
  ଯଦି (b == ଖାଲି)
    ଫେରନ୍ତୁ 0;  /* no input */
  l = strlen(b);
  ଯଦି (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ଫେରନ୍ତୁ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା addreturn (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି = lua_tostring(L, -1);  /* original line */
  ଅଚଳ ଅକ୍ଷର *retline = lua_pushfstring(L, "return %s;", ଧାଡ଼ି);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  ଯଦି (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  ଅନ୍ୟଥା
    lua_କାଢ଼(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ଫେରନ୍ତୁ status;
}


ସ୍ଥିର ଶୂନ୍ୟ checklocal (ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି) {
  ସ୍ଥିର ଅଚଳ size_t szloc = ଆକାର("local") - 1;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର space[] = " \t";
  ଧାଡ଼ି += strspn(ଧାଡ଼ି, space);  /* skip spaces */
  ଯଦି (strncmp(ଧାଡ଼ି, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(ଧାଡ଼ି + szloc)) != ଖାଲି) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiline (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(ଧାଡ଼ି);
  ପାଇଁ (;;) {  /* repeat until gets a complete statement */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = luaL_loadbufferx(L, ଧାଡ଼ି, len, "=stdin", "t");  /* try it */
    ଯଦି (!incomplete(L, status) || !pushline(L, 0))
      ଫେରନ୍ତୁ status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_ଭର୍ତ୍ତି(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    ଧାଡ଼ି = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loadline (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  lua_settop(L, 0);
  ଯଦି (!pushline(L, 1))
    ଫେରନ୍ତୁ -1;  /* no input */
  ଯଦି ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  ଧାଡ଼ି = lua_tostring(L, 1);
  ଯଦି (ଧାଡ଼ି[0] != '\0')  /* non empty? */
    lua_saveline(ଧାଡ଼ି);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ଫେରନ୍ତୁ status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ସ୍ଥିର ଶୂନ୍ୟ l_print (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);
  ଯଦି (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_ଭର୍ତ୍ତି(L, 1);
    ଯଦି (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ସ୍ଥିର ଶୂନ୍ୟ doREPL (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ଅଚଳ ଅକ୍ଷର *oldprogname = progname;
  progname = ଖାଲି;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ଯେତେବେଳେ ((status = loadline(L)) != -1) {
    ଯଦି (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    ଯଦି (status == LUA_OK) l_print(L);
    ଅନ୍ୟଥା report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ଯଦି !defined(luai_openlibs)
#ଯଦି defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#ଅନ୍ୟଥା
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pmain (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, 1);
  ଅକ୍ଷର **argv = (ଅକ୍ଷର **)lua_touserdata(L, 2);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା script;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା args = collectargs(argv, &script);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ଯଦି (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି (args & has_v)  /* option '-v'? */
    print_version();
  ଯଦି (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  ଅନ୍ୟଥା
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ଯଦି (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ଫେରନ୍ତୁ 0;  /* error running LUA_INIT */
  ଯଦି (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ଫେରନ୍ତୁ 0;  /* something failed */
  ଯଦି (script > 0) {  /* execute main script (if there is one) */
    ଯଦି (handle_script(L, argv + script) != LUA_OK)
      ଫେରନ୍ତୁ 0;  /* interrupt in case of error */
  }
  ଯଦି (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  ଅନ୍ୟଥା ଯଦି (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    ଯଦି (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    ଅନ୍ୟଥା dofile(L, ଖାଲି);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ଫେରନ୍ତୁ 1;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, result;
  lua_State *L = luaL_newstate();  /* create state */
  ଯଦି (L == ଖାଲି) {
    l_message(argv[0], "cannot create state: not enough memory");
    ଫେରନ୍ତୁ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  ଫେରନ୍ତୁ (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

