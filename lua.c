/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ <signal.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


#എങ്കിൽ !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#എങ്കിൽ !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#എങ്കിൽ !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


സ്ഥിതം lua_State *globalL = ശൂന്യ;

സ്ഥിതം സ്ഥിരം അക്ഷരം *progname = LUA_PROGNAME;


#എങ്കിൽ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
സ്ഥിതം ശൂന്യം setsignal (പൂർണ്ണസംഖ്യ sig, ശൂന്യം (*handler)(പൂർണ്ണസംഖ്യ)) {
  ഘടന sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ശൂന്യ);
}

#അല്ലെങ്കിൽ           /* }{ */

#define setsignal            സൂചന

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
സ്ഥിതം ശൂന്യം lstop (lua_State *L, lua_Debug *ar) {
  (ശൂന്യം)ar;  /* unused arg. */
  lua_sethook(L, ശൂന്യ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
സ്ഥിതം ശൂന്യം laction (പൂർണ്ണസംഖ്യ i) {
  പൂർണ്ണസംഖ്യ flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


സ്ഥിതം ശൂന്യം print_usage (സ്ഥിരം അക്ഷരം *badoption) {
  lua_writestringerror("%s: ", progname);
  എങ്കിൽ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  അല്ലെങ്കിൽ
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
സ്ഥിതം ശൂന്യം l_message (സ്ഥിരം അക്ഷരം *pname, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  എങ്കിൽ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", സന്ദേശം);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ report (lua_State *L, പൂർണ്ണസംഖ്യ status) {
  എങ്കിൽ (status != LUA_OK) {
    സ്ഥിരം അക്ഷരം *സന്ദേശം = lua_tostring(L, -1);
    എങ്കിൽ (സന്ദേശം == ശൂന്യ)
      സന്ദേശം = "(error message not a string)";
    l_message(progname, സന്ദേശം);
    lua_എടുക്കുക(L, 1);  /* remove message */
  }
  തിരികെ status;
}


/*
** Message handler used to run all chunks
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ msghandler (lua_State *L) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം = lua_tostring(L, 1);
  എങ്കിൽ (സന്ദേശം == ശൂന്യ) {  /* is error object not a string? */
    എങ്കിൽ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      തിരികെ 1;  /* that is the message */
    അല്ലെങ്കിൽ
      സന്ദേശം = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, സന്ദേശം, 1);  /* append a standard traceback */
  തിരികെ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ docall (lua_State *L, പൂർണ്ണസംഖ്യ narg, പൂർണ്ണസംഖ്യ nres) {
  പൂർണ്ണസംഖ്യ status;
  പൂർണ്ണസംഖ്യ base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_തിരുകുക(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  തിരികെ status;
}


സ്ഥിതം ശൂന്യം print_version (ശൂന്യം) {
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
സ്ഥിതം ശൂന്യം createargtable (lua_State *L, അക്ഷരം **argv, പൂർണ്ണസംഖ്യ argc, പൂർണ്ണസംഖ്യ script) {
  പൂർണ്ണസംഖ്യ i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  വേണ്ടി (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


സ്ഥിതം പൂർണ്ണസംഖ്യ dochunk (lua_State *L, പൂർണ്ണസംഖ്യ status) {
  എങ്കിൽ (status == LUA_OK) status = docall(L, 0, 0);
  തിരികെ report(L, status);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ dofile (lua_State *L, സ്ഥിരം അക്ഷരം *name) {
  തിരികെ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ dostring (lua_State *L, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *name) {
  തിരികെ dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dolibrary (lua_State *L, അക്ഷരം *globname) {
  പൂർണ്ണസംഖ്യ status;
  അക്ഷരം *suffix = ശൂന്യ;
  അക്ഷരം *modname = strchr(globname, '=');
  എങ്കിൽ (modname == ശൂന്യ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  അല്ലെങ്കിൽ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  എങ്കിൽ (status == LUA_OK) {
    എങ്കിൽ (suffix != ശൂന്യ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  തിരികെ report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pushargs (lua_State *L) {
  പൂർണ്ണസംഖ്യ i, n;
  എങ്കിൽ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (പൂർണ്ണസംഖ്യ)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  വേണ്ടി (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  തിരികെ n;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ handle_script (lua_State *L, അക്ഷരം **argv) {
  പൂർണ്ണസംഖ്യ status;
  സ്ഥിരം അക്ഷരം *fname = argv[0];
  എങ്കിൽ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ശൂന്യ;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  എങ്കിൽ (status == LUA_OK) {
    പൂർണ്ണസംഖ്യ n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  തിരികെ report(L, status);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ collectargs (അക്ഷരം **argv, പൂർണ്ണസംഖ്യ *first) {
  പൂർണ്ണസംഖ്യ args = 0;
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ (argv[0] != ശൂന്യ) {  /* is there a program name? */
    എങ്കിൽ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  അല്ലെങ്കിൽ {  /* no program name */
    *first = -1;
    തിരികെ 0;
  }
  വേണ്ടി (i = 1; argv[i] != ശൂന്യ; i++) {  /* handle arguments */
    *first = i;
    എങ്കിൽ (argv[i][0] != '-')  /* not an option? */
        തിരികെ args;  /* stop handling options */
    തിരഞ്ഞെടുപ്പ് (argv[i][1]) {  /* else check option */
      അവസ്ഥ '-':  /* '--' */
        എങ്കിൽ (argv[i][2] != '\0')  /* extra characters after '--'? */
          തിരികെ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != ശൂന്യ) ? i + 1 : 0;
        തിരികെ args;
      അവസ്ഥ '\0':  /* '-' */
        തിരികെ args;  /* script "name" is '-' */
      അവസ്ഥ 'E':
        എങ്കിൽ (argv[i][2] != '\0')  /* extra characters? */
          തിരികെ has_error;  /* invalid option */
        args |= has_E;
        നിർത്തുക;
      അവസ്ഥ 'W':
        എങ്കിൽ (argv[i][2] != '\0')  /* extra characters? */
          തിരികെ has_error;  /* invalid option */
        നിർത്തുക;
      അവസ്ഥ 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      അവസ്ഥ 'v':
        എങ്കിൽ (argv[i][2] != '\0')  /* extra characters? */
          തിരികെ has_error;  /* invalid option */
        args |= has_v;
        നിർത്തുക;
      അവസ്ഥ 'e':
        args |= has_e;  /* FALLTHROUGH */
      അവസ്ഥ 'l':  /* both options need an argument */
        എങ്കിൽ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          എങ്കിൽ (argv[i] == ശൂന്യ || argv[i][0] == '-')
            തിരികെ has_error;  /* no next argument or it is another option */
        }
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:  /* invalid option */
        തിരികെ has_error;
    }
  }
  *first = 0;  /* no script name */
  തിരികെ args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ runargs (lua_State *L, അക്ഷരം **argv, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  വേണ്ടി (i = 1; i < n; i++) {
    പൂർണ്ണസംഖ്യ option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    തിരഞ്ഞെടുപ്പ് (option) {
      അവസ്ഥ 'e':  അവസ്ഥ 'l': {
        പൂർണ്ണസംഖ്യ status;
        അക്ഷരം *extra = argv[i] + 2;  /* both options need an argument */
        എങ്കിൽ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ശൂന്യ);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        എങ്കിൽ (status != LUA_OK) തിരികെ 0;
        നിർത്തുക;
      }
      അവസ്ഥ 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        നിർത്തുക;
    }
  }
  തിരികെ 1;
}


സ്ഥിതം അക്ഷരം *(*l_getenv)(സ്ഥിരം അക്ഷരം *name);

/* Function to ignore environment variables, used by option -E */
സ്ഥിതം അക്ഷരം *no_getenv (സ്ഥിരം അക്ഷരം *name) {
  UNUSED(name);
  തിരികെ ശൂന്യ;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ handle_luainit (lua_State *L) {
  സ്ഥിരം അക്ഷരം *name = "=" LUA_INITVARVERSION;
  സ്ഥിരം അക്ഷരം *init = l_getenv(name + 1);
  എങ്കിൽ (init == ശൂന്യ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  എങ്കിൽ (init == ശൂന്യ) തിരികെ LUA_OK;
  അല്ലെങ്കിൽ എങ്കിൽ (init[0] == '@')
    തിരികെ dofile(L, init+1);
  അല്ലെങ്കിൽ
    തിരികെ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#എങ്കിൽ !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#എങ്കിൽ !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#എങ്കിൽ !defined(lua_stdin_is_tty)	/* { */

#എങ്കിൽ defined(LUA_USE_POSIX)	/* { */

#അന്തര്ഭാവ <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#അല്ലെങ്കിലെങ്കിൽ defined(LUA_USE_WINDOWS)	/* }{ */

#അന്തര്ഭാവ <io.h>
#അന്തര്ഭാവ <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#അല്ലെങ്കിൽ				/* }{ */

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

#എങ്കിൽ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#എങ്കിൽ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#അന്തര്ഭാവ <readline/readline.h>
#അന്തര്ഭാവ <readline/history.h>

#define lua_initreadline(L)  \
	((ശൂന്യം)L, *cast(സ്ഥിരം അക്ഷരം **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((ശൂന്യം)buff, readline(prompt))
#define lua_saveline(വരി)	add_history(വരി)
#define lua_freeline(വരി)	സ്വതന്ത്രം(വരി)

#അല്ലെങ്കിൽ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
തരനിർവചനം അക്ഷരം *(*l_readlineT) (സ്ഥിരം അക്ഷരം *prompt);
സ്ഥിതം l_readlineT l_readline = ശൂന്യ;

/* pointer to 'add_history' function (if any) */
തരനിർവചനം ശൂന്യം (*l_addhistT) (സ്ഥിരം അക്ഷരം *string);
സ്ഥിതം l_addhistT l_addhist = ശൂന്യ;


സ്ഥിതം അക്ഷരം *lua_readline (അക്ഷരം *buff, സ്ഥിരം അക്ഷരം *prompt) {
  എങ്കിൽ (l_readline != ശൂന്യ)  /* is there a 'readline'? */
    തിരികെ (*l_readline)(prompt);  /* use it */
  അല്ലെങ്കിൽ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    തിരികെ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


സ്ഥിതം ശൂന്യം lua_saveline (സ്ഥിരം അക്ഷരം *വരി) {
  എങ്കിൽ (l_addhist != ശൂന്യ)  /* is there an 'add_history'? */
    (*l_addhist)(വരി);  /* use it */
  /* else nothing to be done */
}


സ്ഥിതം ശൂന്യം lua_freeline (അക്ഷരം *വരി) {
  എങ്കിൽ (l_readline != ശൂന്യ)  /* is there a 'readline'? */
    സ്വതന്ത്രം(വരി);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#എങ്കിൽ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#അന്തര്ഭാവ <dlfcn.h>

സ്ഥിതം ശൂന്യം lua_initreadline (lua_State *L) {
  സ്ഥിരം അക്ഷരം *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  ശൂന്യം *lib;  /* library handle */
  എങ്കിൽ (rllib == ശൂന്യ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  എങ്കിൽ (lib != ശൂന്യ) {
    സ്ഥിരം അക്ഷരം **name = cast(സ്ഥിരം അക്ഷരം**, dlsym(lib, "rl_readline_name"));
    എങ്കിൽ (name != ശൂന്യ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    എങ്കിൽ (l_readline != ശൂന്യ)  /* could load readline function? */
      തിരികെ;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#അല്ലെങ്കിൽ		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((ശൂന്യം)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *get_prompt (lua_State *L, പൂർണ്ണസംഖ്യ firstline) {
  എങ്കിൽ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    തിരികെ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  അല്ലെങ്കിൽ {  /* apply 'tostring' over the value */
    സ്ഥിരം അക്ഷരം *p = luaL_tolstring(L, -1, ശൂന്യ);
    lua_remove(L, -2);  /* remove original value */
    തിരികെ p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(വലുപ്പം(EOFMARK)/വലുപ്പം(അക്ഷരം) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ incomplete (lua_State *L, പൂർണ്ണസംഖ്യ status) {
  എങ്കിൽ (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    സ്ഥിരം അക്ഷരം *സന്ദേശം = lua_tolstring(L, -1, &lmsg);
    എങ്കിൽ (lmsg >= marklen && strcmp(സന്ദേശം + lmsg - marklen, EOFMARK) == 0)
      തിരികെ 1;
  }
  തിരികെ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pushline (lua_State *L, പൂർണ്ണസംഖ്യ firstline) {
  അക്ഷരം buffer[LUA_MAXINPUT];
  size_t l;
  സ്ഥിരം അക്ഷരം *prmt = get_prompt(L, firstline);
  അക്ഷരം *b = lua_readline(buffer, prmt);
  lua_എടുക്കുക(L, 1);  /* remove prompt */
  എങ്കിൽ (b == ശൂന്യ)
    തിരികെ 0;  /* no input */
  l = strlen(b);
  എങ്കിൽ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  തിരികെ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ addreturn (lua_State *L) {
  സ്ഥിരം അക്ഷരം *വരി = lua_tostring(L, -1);  /* original line */
  സ്ഥിരം അക്ഷരം *retline = lua_pushfstring(L, "return %s;", വരി);
  പൂർണ്ണസംഖ്യ status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  എങ്കിൽ (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  അല്ലെങ്കിൽ
    lua_എടുക്കുക(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  തിരികെ status;
}


സ്ഥിതം ശൂന്യം checklocal (സ്ഥിരം അക്ഷരം *വരി) {
  സ്ഥിതം സ്ഥിരം size_t szloc = വലുപ്പം("local") - 1;
  സ്ഥിതം സ്ഥിരം അക്ഷരം space[] = " \t";
  വരി += strspn(വരി, space);  /* skip spaces */
  എങ്കിൽ (strncmp(വരി, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(വരി + szloc)) != ശൂന്യ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiline (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *വരി = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(വരി);
  വേണ്ടി (;;) {  /* repeat until gets a complete statement */
    പൂർണ്ണസംഖ്യ status = luaL_loadbufferx(L, വരി, len, "=stdin", "t");  /* try it */
    എങ്കിൽ (!incomplete(L, status) || !pushline(L, 0))
      തിരികെ status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_തിരുകുക(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    വരി = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ loadline (lua_State *L) {
  സ്ഥിരം അക്ഷരം *വരി;
  പൂർണ്ണസംഖ്യ status;
  lua_settop(L, 0);
  എങ്കിൽ (!pushline(L, 1))
    തിരികെ -1;  /* no input */
  എങ്കിൽ ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  വരി = lua_tostring(L, 1);
  എങ്കിൽ (വരി[0] != '\0')  /* non empty? */
    lua_saveline(വരി);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  തിരികെ status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
സ്ഥിതം ശൂന്യം l_print (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);
  എങ്കിൽ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_തിരുകുക(L, 1);
    എങ്കിൽ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
സ്ഥിതം ശൂന്യം doREPL (lua_State *L) {
  പൂർണ്ണസംഖ്യ status;
  സ്ഥിരം അക്ഷരം *oldprogname = progname;
  progname = ശൂന്യ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ആകുമ്പോൾ ((status = loadline(L)) != -1) {
    എങ്കിൽ (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    എങ്കിൽ (status == LUA_OK) l_print(L);
    അല്ലെങ്കിൽ report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#എങ്കിൽ !defined(luai_openlibs)
#എങ്കിൽ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#അല്ലെങ്കിൽ
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pmain (lua_State *L) {
  പൂർണ്ണസംഖ്യ argc = (പൂർണ്ണസംഖ്യ)lua_tointeger(L, 1);
  അക്ഷരം **argv = (അക്ഷരം **)lua_touserdata(L, 2);
  പൂർണ്ണസംഖ്യ script;
  പൂർണ്ണസംഖ്യ args = collectargs(argv, &script);
  പൂർണ്ണസംഖ്യ optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  എങ്കിൽ (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    തിരികെ 0;
  }
  എങ്കിൽ (args & has_v)  /* option '-v'? */
    print_version();
  എങ്കിൽ (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  അല്ലെങ്കിൽ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  എങ്കിൽ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    തിരികെ 0;  /* error running LUA_INIT */
  എങ്കിൽ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    തിരികെ 0;  /* something failed */
  എങ്കിൽ (script > 0) {  /* execute main script (if there is one) */
    എങ്കിൽ (handle_script(L, argv + script) != LUA_OK)
      തിരികെ 0;  /* interrupt in case of error */
  }
  എങ്കിൽ (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  അല്ലെങ്കിൽ എങ്കിൽ (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    എങ്കിൽ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    അല്ലെങ്കിൽ dofile(L, ശൂന്യ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  തിരികെ 1;
}


പൂർണ്ണസംഖ്യ പ്രധാന (പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) {
  പൂർണ്ണസംഖ്യ status, result;
  lua_State *L = luaL_newstate();  /* create state */
  എങ്കിൽ (L == ശൂന്യ) {
    l_message(argv[0], "cannot create state: not enough memory");
    തിരികെ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  തിരികെ (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

