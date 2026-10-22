/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ <signal.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


#ಯದಿ !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#ಯದಿ !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#ಯದಿ !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ಸ್ಥಿರ lua_State *globalL = ರಿಕ್ತ;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *progname = LUA_PROGNAME;


#ಯದಿ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setsignal (ಪೂರ್ಣಾಙ್ಕ sig, ಶೂನ್ಯ (*handler)(ಪೂರ್ಣಾಙ್ಕ)) {
  ಸಂರಚನಾ sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ರಿಕ್ತ);
}

#ಅನ್ಯಥಾ           /* }{ */

#define setsignal            ಸಙ್ಕೇತ

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ lstop (lua_State *L, lua_Debug *ar) {
  (ಶೂನ್ಯ)ar;  /* unused arg. */
  lua_sethook(L, ರಿಕ್ತ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ laction (ಪೂರ್ಣಾಙ್ಕ i) {
  ಪೂರ್ಣಾಙ್ಕ flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ಸ್ಥಿರ ಶೂನ್ಯ print_usage (ಧ್ರುವ ವರ್ಣ *badoption) {
  lua_writestringerror("%s: ", progname);
  ಯದಿ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಶೂನ್ಯ l_message (ಧ್ರುವ ವರ್ಣ *pname, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
  ಯದಿ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", ಸಂದೇಶ);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ report (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status) {
  ಯದಿ (status != LUA_OK) {
    ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = lua_tostring(L, -1);
    ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ)
      ಸಂದೇಶ = "(error message not a string)";
    l_message(progname, ಸಂದೇಶ);
    lua_ತೆಗೆ(L, 1);  /* remove message */
  }
  ಪ್ರತ್ಯಾಗಮ status;
}


/*
** Message handler used to run all chunks
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ msghandler (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = lua_tostring(L, 1);
  ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ) {  /* is error object not a string? */
    ಯದಿ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ಪ್ರತ್ಯಾಗಮ 1;  /* that is the message */
    ಅನ್ಯಥಾ
      ಸಂದೇಶ = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, ಸಂದೇಶ, 1);  /* append a standard traceback */
  ಪ್ರತ್ಯಾಗಮ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ docall (lua_State *L, ಪೂರ್ಣಾಙ್ಕ narg, ಪೂರ್ಣಾಙ್ಕ nres) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ಪೂರ್ಣಾಙ್ಕ base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_ಸೇರಿಸು(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  ಪ್ರತ್ಯಾಗಮ status;
}


ಸ್ಥಿರ ಶೂನ್ಯ print_version (ಶೂನ್ಯ) {
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
ಸ್ಥಿರ ಶೂನ್ಯ createargtable (lua_State *L, ವರ್ಣ **argv, ಪೂರ್ಣಾಙ್ಕ argc, ಪೂರ್ಣಾಙ್ಕ script) {
  ಪೂರ್ಣಾಙ್ಕ i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  ಪ್ರತಿ (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dochunk (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status) {
  ಯದಿ (status == LUA_OK) status = docall(L, 0, 0);
  ಪ್ರತ್ಯಾಗಮ report(L, status);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dofile (lua_State *L, ಧ್ರುವ ವರ್ಣ *name) {
  ಪ್ರತ್ಯಾಗಮ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dostring (lua_State *L, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *name) {
  ಪ್ರತ್ಯಾಗಮ dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dolibrary (lua_State *L, ವರ್ಣ *globname) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ವರ್ಣ *suffix = ರಿಕ್ತ;
  ವರ್ಣ *modname = strchr(globname, '=');
  ಯದಿ (modname == ರಿಕ್ತ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  ಅನ್ಯಥಾ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  ಯದಿ (status == LUA_OK) {
    ಯದಿ (suffix != ರಿಕ್ತ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ಪ್ರತ್ಯಾಗಮ report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pushargs (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ i, n;
  ಯದಿ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ಪೂರ್ಣಾಙ್ಕ)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  ಪ್ರತಿ (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ಪ್ರತ್ಯಾಗಮ n;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_script (lua_State *L, ವರ್ಣ **argv) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ಧ್ರುವ ವರ್ಣ *fname = argv[0];
  ಯದಿ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ರಿಕ್ತ;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  ಯದಿ (status == LUA_OK) {
    ಪೂರ್ಣಾಙ್ಕ n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  ಪ್ರತ್ಯಾಗಮ report(L, status);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ collectargs (ವರ್ಣ **argv, ಪೂರ್ಣಾಙ್ಕ *first) {
  ಪೂರ್ಣಾಙ್ಕ args = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ (argv[0] != ರಿಕ್ತ) {  /* is there a program name? */
    ಯದಿ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  ಅನ್ಯಥಾ {  /* no program name */
    *first = -1;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿ (i = 1; argv[i] != ರಿಕ್ತ; i++) {  /* handle arguments */
    *first = i;
    ಯದಿ (argv[i][0] != '-')  /* not an option? */
        ಪ್ರತ್ಯಾಗಮ args;  /* stop handling options */
    ವಿಕಲ್ಪ (argv[i][1]) {  /* else check option */
      ಪಕ್ಷ '-':  /* '--' */
        ಯದಿ (argv[i][2] != '\0')  /* extra characters after '--'? */
          ಪ್ರತ್ಯಾಗಮ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != ರಿಕ್ತ) ? i + 1 : 0;
        ಪ್ರತ್ಯಾಗಮ args;
      ಪಕ್ಷ '\0':  /* '-' */
        ಪ್ರತ್ಯಾಗಮ args;  /* script "name" is '-' */
      ಪಕ್ಷ 'E':
        ಯದಿ (argv[i][2] != '\0')  /* extra characters? */
          ಪ್ರತ್ಯಾಗಮ has_error;  /* invalid option */
        args |= has_E;
        ಭಙ್ಗ;
      ಪಕ್ಷ 'W':
        ಯದಿ (argv[i][2] != '\0')  /* extra characters? */
          ಪ್ರತ್ಯಾಗಮ has_error;  /* invalid option */
        ಭಙ್ಗ;
      ಪಕ್ಷ 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      ಪಕ್ಷ 'v':
        ಯದಿ (argv[i][2] != '\0')  /* extra characters? */
          ಪ್ರತ್ಯಾಗಮ has_error;  /* invalid option */
        args |= has_v;
        ಭಙ್ಗ;
      ಪಕ್ಷ 'e':
        args |= has_e;  /* FALLTHROUGH */
      ಪಕ್ಷ 'l':  /* both options need an argument */
        ಯದಿ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ಯದಿ (argv[i] == ರಿಕ್ತ || argv[i][0] == '-')
            ಪ್ರತ್ಯಾಗಮ has_error;  /* no next argument or it is another option */
        }
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:  /* invalid option */
        ಪ್ರತ್ಯಾಗಮ has_error;
    }
  }
  *first = 0;  /* no script name */
  ಪ್ರತ್ಯಾಗಮ args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ runargs (lua_State *L, ವರ್ಣ **argv, ಪೂರ್ಣಾಙ್ಕ n) {
  ಪೂರ್ಣಾಙ್ಕ i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  ಪ್ರತಿ (i = 1; i < n; i++) {
    ಪೂರ್ಣಾಙ್ಕ option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    ವಿಕಲ್ಪ (option) {
      ಪಕ್ಷ 'e':  ಪಕ್ಷ 'l': {
        ಪೂರ್ಣಾಙ್ಕ status;
        ವರ್ಣ *extra = argv[i] + 2;  /* both options need an argument */
        ಯದಿ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ರಿಕ್ತ);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ಯದಿ (status != LUA_OK) ಪ್ರತ್ಯಾಗಮ 0;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ವರ್ಣ *(*l_getenv)(ಧ್ರುವ ವರ್ಣ *name);

/* Function to ignore environment variables, used by option -E */
ಸ್ಥಿರ ವರ್ಣ *no_getenv (ಧ್ರುವ ವರ್ಣ *name) {
  UNUSED(name);
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_luainit (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *name = "=" LUA_INITVARVERSION;
  ಧ್ರುವ ವರ್ಣ *init = l_getenv(name + 1);
  ಯದಿ (init == ರಿಕ್ತ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ಯದಿ (init == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ LUA_OK;
  ಅನ್ಯಥಾ ಯದಿ (init[0] == '@')
    ಪ್ರತ್ಯಾಗಮ dofile(L, init+1);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ಯದಿ !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#ಯದಿ !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ಯದಿ !defined(lua_stdin_is_tty)	/* { */

#ಯದಿ defined(LUA_USE_POSIX)	/* { */

#ಅನ್ತರ್ಭಾವ <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#ನೋಚೇದ್ಯದಿ defined(LUA_USE_WINDOWS)	/* }{ */

#ಅನ್ತರ್ಭಾವ <io.h>
#ಅನ್ತರ್ಭಾವ <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#ಅನ್ಯಥಾ				/* }{ */

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

#ಯದಿ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ಯದಿ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ಅನ್ತರ್ಭಾವ <readline/readline.h>
#ಅನ್ತರ್ಭಾವ <readline/history.h>

#define lua_initreadline(L)  \
	((ಶೂನ್ಯ)L, *cast(ಧ್ರುವ ವರ್ಣ **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((ಶೂನ್ಯ)buff, readline(prompt))
#define lua_saveline(ಸಾಲು)	add_history(ಸಾಲು)
#define lua_freeline(ಸಾಲು)	ಮುಕ್ತ(ಸಾಲು)

#ಅನ್ಯಥಾ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
ಪ್ರಕಾರನಿರ್ದೇಶ ವರ್ಣ *(*l_readlineT) (ಧ್ರುವ ವರ್ಣ *prompt);
ಸ್ಥಿರ l_readlineT l_readline = ರಿಕ್ತ;

/* pointer to 'add_history' function (if any) */
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ (*l_addhistT) (ಧ್ರುವ ವರ್ಣ *string);
ಸ್ಥಿರ l_addhistT l_addhist = ರಿಕ್ತ;


ಸ್ಥಿರ ವರ್ಣ *lua_readline (ವರ್ಣ *buff, ಧ್ರುವ ವರ್ಣ *prompt) {
  ಯದಿ (l_readline != ರಿಕ್ತ)  /* is there a 'readline'? */
    ಪ್ರತ್ಯಾಗಮ (*l_readline)(prompt);  /* use it */
  ಅನ್ಯಥಾ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ಪ್ರತ್ಯಾಗಮ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ lua_saveline (ಧ್ರುವ ವರ್ಣ *ಸಾಲು) {
  ಯದಿ (l_addhist != ರಿಕ್ತ)  /* is there an 'add_history'? */
    (*l_addhist)(ಸಾಲು);  /* use it */
  /* else nothing to be done */
}


ಸ್ಥಿರ ಶೂನ್ಯ lua_freeline (ವರ್ಣ *ಸಾಲು) {
  ಯದಿ (l_readline != ರಿಕ್ತ)  /* is there a 'readline'? */
    ಮುಕ್ತ(ಸಾಲು);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ಯದಿ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ಅನ್ತರ್ಭಾವ <dlfcn.h>

ಸ್ಥಿರ ಶೂನ್ಯ lua_initreadline (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  ಶೂನ್ಯ *lib;  /* library handle */
  ಯದಿ (rllib == ರಿಕ್ತ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ಯದಿ (lib != ರಿಕ್ತ) {
    ಧ್ರುವ ವರ್ಣ **name = cast(ಧ್ರುವ ವರ್ಣ**, dlsym(lib, "rl_readline_name"));
    ಯದಿ (name != ರಿಕ್ತ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ಯದಿ (l_readline != ರಿಕ್ತ)  /* could load readline function? */
      ಪ್ರತ್ಯಾಗಮ;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#ಅನ್ಯಥಾ		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((ಶೂನ್ಯ)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *get_prompt (lua_State *L, ಪೂರ್ಣಾಙ್ಕ firstline) {
  ಯದಿ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ಪ್ರತ್ಯಾಗಮ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  ಅನ್ಯಥಾ {  /* apply 'tostring' over the value */
    ಧ್ರುವ ವರ್ಣ *p = luaL_tolstring(L, -1, ರಿಕ್ತ);
    lua_remove(L, -2);  /* remove original value */
    ಪ್ರತ್ಯಾಗಮ p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(ಪರಿಮಾಣ(EOFMARK)/ಪರಿಮಾಣ(ವರ್ಣ) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ incomplete (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status) {
  ಯದಿ (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = lua_tolstring(L, -1, &lmsg);
    ಯದಿ (lmsg >= marklen && strcmp(ಸಂದೇಶ + lmsg - marklen, EOFMARK) == 0)
      ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pushline (lua_State *L, ಪೂರ್ಣಾಙ್ಕ firstline) {
  ವರ್ಣ buffer[LUA_MAXINPUT];
  size_t l;
  ಧ್ರುವ ವರ್ಣ *prmt = get_prompt(L, firstline);
  ವರ್ಣ *b = lua_readline(buffer, prmt);
  lua_ತೆಗೆ(L, 1);  /* remove prompt */
  ಯದಿ (b == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ 0;  /* no input */
  l = strlen(b);
  ಯದಿ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ addreturn (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *ಸಾಲು = lua_tostring(L, -1);  /* original line */
  ಧ್ರುವ ವರ್ಣ *retline = lua_pushfstring(L, "return %s;", ಸಾಲು);
  ಪೂರ್ಣಾಙ್ಕ status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  ಯದಿ (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  ಅನ್ಯಥಾ
    lua_ತೆಗೆ(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ಪ್ರತ್ಯಾಗಮ status;
}


ಸ್ಥಿರ ಶೂನ್ಯ checklocal (ಧ್ರುವ ವರ್ಣ *ಸಾಲು) {
  ಸ್ಥಿರ ಧ್ರುವ size_t szloc = ಪರಿಮಾಣ("local") - 1;
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ space[] = " \t";
  ಸಾಲು += strspn(ಸಾಲು, space);  /* skip spaces */
  ಯದಿ (strncmp(ಸಾಲು, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(ಸಾಲು + szloc)) != ರಿಕ್ತ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiline (lua_State *L) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *ಸಾಲು = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(ಸಾಲು);
  ಪ್ರತಿ (;;) {  /* repeat until gets a complete statement */
    ಪೂರ್ಣಾಙ್ಕ status = luaL_loadbufferx(L, ಸಾಲು, len, "=stdin", "t");  /* try it */
    ಯದಿ (!incomplete(L, status) || !pushline(L, 0))
      ಪ್ರತ್ಯಾಗಮ status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_ಸೇರಿಸು(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    ಸಾಲು = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadline (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *ಸಾಲು;
  ಪೂರ್ಣಾಙ್ಕ status;
  lua_settop(L, 0);
  ಯದಿ (!pushline(L, 1))
    ಪ್ರತ್ಯಾಗಮ -1;  /* no input */
  ಯದಿ ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  ಸಾಲು = lua_tostring(L, 1);
  ಯದಿ (ಸಾಲು[0] != '\0')  /* non empty? */
    lua_saveline(ಸಾಲು);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ಪ್ರತ್ಯಾಗಮ status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ಸ್ಥಿರ ಶೂನ್ಯ l_print (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);
  ಯದಿ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_ಸೇರಿಸು(L, 1);
    ಯದಿ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ಸ್ಥಿರ ಶೂನ್ಯ doREPL (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ಧ್ರುವ ವರ್ಣ *oldprogname = progname;
  progname = ರಿಕ್ತ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ಯಾವತ್ ((status = loadline(L)) != -1) {
    ಯದಿ (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    ಯದಿ (status == LUA_OK) l_print(L);
    ಅನ್ಯಥಾ report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ಯದಿ !defined(luai_openlibs)
#ಯದಿ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#ಅನ್ಯಥಾ
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pmain (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ argc = (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, 1);
  ವರ್ಣ **argv = (ವರ್ಣ **)lua_touserdata(L, 2);
  ಪೂರ್ಣಾಙ್ಕ script;
  ಪೂರ್ಣಾಙ್ಕ args = collectargs(argv, &script);
  ಪೂರ್ಣಾಙ್ಕ optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ಯದಿ (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ (args & has_v)  /* option '-v'? */
    print_version();
  ಯದಿ (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  ಅನ್ಯಥಾ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ಯದಿ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ಪ್ರತ್ಯಾಗಮ 0;  /* error running LUA_INIT */
  ಯದಿ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ಪ್ರತ್ಯಾಗಮ 0;  /* something failed */
  ಯದಿ (script > 0) {  /* execute main script (if there is one) */
    ಯದಿ (handle_script(L, argv + script) != LUA_OK)
      ಪ್ರತ್ಯಾಗಮ 0;  /* interrupt in case of error */
  }
  ಯದಿ (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  ಅನ್ಯಥಾ ಯದಿ (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    ಯದಿ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    ಅನ್ಯಥಾ dofile(L, ರಿಕ್ತ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಪೂರ್ಣಾಙ್ಕ main (ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) {
  ಪೂರ್ಣಾಙ್ಕ status, result;
  lua_State *L = luaL_newstate();  /* create state */
  ಯದಿ (L == ರಿಕ್ತ) {
    l_message(argv[0], "cannot create state: not enough memory");
    ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  ಪ್ರತ್ಯಾಗಮ (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

