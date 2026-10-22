/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_c

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


#ਜੇ !defined(LUA_PROGNAME)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_PROGNAME		"lua"
#ਜੇ_ਅੰਤ

#ਜੇ !defined(LUA_INIT_VAR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_INIT_VAR		"LUA_INIT"
#ਜੇ_ਅੰਤ

/* Name of the environment variable with the name of the readline library */
#ਜੇ !defined(LUA_RLLIB_VAR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RLLIB_VAR		"LUA_READLINELIB"
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ਸਥਿਰ lua_State *globalL = ਨਿਰਾ;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *progname = LUA_PROGNAME;


#ਜੇ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ਸਥਿਰ ਖਾਲੀ setsignal (ਪੂਰਨ sig, ਖਾਲੀ (*handler)(ਪੂਰਨ)) {
  ਬਣਤਰ sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ਨਿਰਾ);
}

#ਨਹੀਂਤਾਂ           /* }{ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setsignal            ਸੰਕੇਤ

#ਜੇ_ਅੰਤ                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ਸਥਿਰ ਖਾਲੀ lstop (lua_State *L, lua_Debug *ar) {
  (ਖਾਲੀ)ar;  /* unused arg. */
  lua_sethook(L, ਨਿਰਾ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ਸਥਿਰ ਖਾਲੀ laction (ਪੂਰਨ i) {
  ਪੂਰਨ flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ਸਥਿਰ ਖਾਲੀ print_usage (ਅਚਲ ਅੱਖਰ *badoption) {
  lua_writestringerror("%s: ", progname);
  ਜੇ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  ਨਹੀਂਤਾਂ
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
ਸਥਿਰ ਖਾਲੀ l_ਸੁਨੇਹਾ (ਅਚਲ ਅੱਖਰ *pname, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  ਜੇ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", ਸੁਨੇਹਾ);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ਸਥਿਰ ਪੂਰਨ report (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ) {
  ਜੇ (ਸਥਿਤੀ != LUA_OK) {
    ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = lua_tostring(L, -1);
    ਜੇ (ਸੁਨੇਹਾ == ਨਿਰਾ)
      ਸੁਨੇਹਾ = "(error message not a string)";
    l_ਸੁਨੇਹਾ(progname, ਸੁਨੇਹਾ);
    lua_ਕੱਢੋ(L, 1);  /* remove message */
  }
  ਵਾਪਸ ਸਥਿਤੀ;
}


/*
** Message handler used to run all chunks
*/
ਸਥਿਰ ਪੂਰਨ msghandler (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = lua_tostring(L, 1);
  ਜੇ (ਸੁਨੇਹਾ == ਨਿਰਾ) {  /* is error object not a string? */
    ਜੇ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ਵਾਪਸ 1;  /* that is the message */
    ਨਹੀਂਤਾਂ
      ਸੁਨੇਹਾ = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, ਸੁਨੇਹਾ, 1);  /* append a standard traceback */
  ਵਾਪਸ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ਸਥਿਰ ਪੂਰਨ docall (lua_State *L, ਪੂਰਨ narg, ਪੂਰਨ nres) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਪੂਰਨ ਅਧਾਰ = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_ਪਾਓ(L, ਅਧਾਰ);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  ਸਥਿਤੀ = lua_pcall(L, narg, nres, ਅਧਾਰ);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, ਅਧਾਰ);  /* remove message handler from the stack */
  ਵਾਪਸ ਸਥਿਤੀ;
}


ਸਥਿਰ ਖਾਲੀ print_ਸੰਸਕਰਣ (ਖਾਲੀ) {
  lua_writestring(LUA_COPYRIGHT, ਲੜੀ_ਲੰਬਾਈ(LUA_COPYRIGHT));
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
ਸਥਿਰ ਖਾਲੀ createargtable (lua_State *L, ਅੱਖਰ **argv, ਪੂਰਨ argc, ਪੂਰਨ script) {
  ਪੂਰਨ i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  ਲਈ (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ਸਥਿਰ ਪੂਰਨ dochunk (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ) {
  ਜੇ (ਸਥਿਤੀ == LUA_OK) ਸਥਿਤੀ = docall(L, 0, 0);
  ਵਾਪਸ report(L, ਸਥਿਤੀ);
}


ਸਥਿਰ ਪੂਰਨ dofile (lua_State *L, ਅਚਲ ਅੱਖਰ *name) {
  ਵਾਪਸ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ਸਥਿਰ ਪੂਰਨ dostring (lua_State *L, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *name) {
  ਵਾਪਸ dochunk(L, luaL_loadbufferx(L, s, ਲੜੀ_ਲੰਬਾਈ(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ਸਥਿਰ ਪੂਰਨ dolibrary (lua_State *L, ਅੱਖਰ *globname) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਅੱਖਰ *suffix = ਨਿਰਾ;
  ਅੱਖਰ *modname = strchr(globname, '=');
  ਜੇ (modname == ਨਿਰਾ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  ਨਹੀਂਤਾਂ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  ਸਥਿਤੀ = docall(L, 1, 1);  /* call 'require(modname)' */
  ਜੇ (ਸਥਿਤੀ == LUA_OK) {
    ਜੇ (suffix != ਨਿਰਾ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ਵਾਪਸ report(L, ਸਥਿਤੀ);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ਸਥਿਰ ਪੂਰਨ pushargs (lua_State *L) {
  ਪੂਰਨ i, n;
  ਜੇ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ਪੂਰਨ)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  ਲਈ (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ਵਾਪਸ n;
}


ਸਥਿਰ ਪੂਰਨ handle_script (lua_State *L, ਅੱਖਰ **argv) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਅਚਲ ਅੱਖਰ *fname = argv[0];
  ਜੇ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ਨਿਰਾ;  /* stdin */
  ਸਥਿਤੀ = luaL_loadfilex(L, fname, "bt");
  ਜੇ (ਸਥਿਤੀ == LUA_OK) {
    ਪੂਰਨ n = pushargs(L);  /* push arguments to script */
    ਸਥਿਤੀ = docall(L, n, LUA_MULTRET);
  }
  ਵਾਪਸ report(L, ਸਥਿਤੀ);
}


/* bits of various argument indicators in 'args' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_error	1	/* bad option */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_i		2	/* -i */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_v		4	/* -v */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_e		8	/* -e */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
ਸਥਿਰ ਪੂਰਨ collectargs (ਅੱਖਰ **argv, ਪੂਰਨ *ਪਹਿਲਾ) {
  ਪੂਰਨ ਦਲੀਲਾਂ = 0;
  ਪੂਰਨ i;
  ਜੇ (argv[0] != ਨਿਰਾ) {  /* is there a program name? */
    ਜੇ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  ਨਹੀਂਤਾਂ {  /* no program name */
    *ਪਹਿਲਾ = -1;
    ਵਾਪਸ 0;
  }
  ਲਈ (i = 1; argv[i] != ਨਿਰਾ; i++) {  /* handle arguments */
    *ਪਹਿਲਾ = i;
    ਜੇ (argv[i][0] != '-')  /* not an option? */
        ਵਾਪਸ ਦਲੀਲਾਂ;  /* stop handling options */
    ਚੋਣ (argv[i][1]) {  /* else check option */
      ਹਾਲਤ '-':  /* '--' */
        ਜੇ (argv[i][2] != '\0')  /* extra characters after '--'? */
          ਵਾਪਸ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *ਪਹਿਲਾ = (argv[i + 1] != ਨਿਰਾ) ? i + 1 : 0;
        ਵਾਪਸ ਦਲੀਲਾਂ;
      ਹਾਲਤ '\0':  /* '-' */
        ਵਾਪਸ ਦਲੀਲਾਂ;  /* script "name" is '-' */
      ਹਾਲਤ 'E':
        ਜੇ (argv[i][2] != '\0')  /* extra characters? */
          ਵਾਪਸ has_error;  /* invalid option */
        ਦਲੀਲਾਂ |= has_E;
        ਰੋਕੋ;
      ਹਾਲਤ 'W':
        ਜੇ (argv[i][2] != '\0')  /* extra characters? */
          ਵਾਪਸ has_error;  /* invalid option */
        ਰੋਕੋ;
      ਹਾਲਤ 'i':
        ਦਲੀਲਾਂ |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      ਹਾਲਤ 'v':
        ਜੇ (argv[i][2] != '\0')  /* extra characters? */
          ਵਾਪਸ has_error;  /* invalid option */
        ਦਲੀਲਾਂ |= has_v;
        ਰੋਕੋ;
      ਹਾਲਤ 'e':
        ਦਲੀਲਾਂ |= has_e;  /* FALLTHROUGH */
      ਹਾਲਤ 'l':  /* both options need an argument */
        ਜੇ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ਜੇ (argv[i] == ਨਿਰਾ || argv[i][0] == '-')
            ਵਾਪਸ has_error;  /* no next argument or it is another option */
        }
        ਰੋਕੋ;
      ਮੂਲ:  /* invalid option */
        ਵਾਪਸ has_error;
    }
  }
  *ਪਹਿਲਾ = 0;  /* no script name */
  ਵਾਪਸ ਦਲੀਲਾਂ;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ਸਥਿਰ ਪੂਰਨ runargs (lua_State *L, ਅੱਖਰ **argv, ਪੂਰਨ n) {
  ਪੂਰਨ i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  ਲਈ (i = 1; i < n; i++) {
    ਪੂਰਨ option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    ਚੋਣ (option) {
      ਹਾਲਤ 'e':  ਹਾਲਤ 'l': {
        ਪੂਰਨ ਸਥਿਤੀ;
        ਅੱਖਰ *extra = argv[i] + 2;  /* both options need an argument */
        ਜੇ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ਨਿਰਾ);
        ਸਥਿਤੀ = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ਜੇ (ਸਥਿਤੀ != LUA_OK) ਵਾਪਸ 0;
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        ਰੋਕੋ;
    }
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਅੱਖਰ *(*l_getenv)(ਅਚਲ ਅੱਖਰ *name);

/* Function to ignore environment variables, used by option -E */
ਸਥਿਰ ਅੱਖਰ *no_getenv (ਅਚਲ ਅੱਖਰ *name) {
  ਅਣਵਰਤਿਆ(name);
  ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ ਪੂਰਨ handle_luainit (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *name = "=" LUA_INITVARVERSION;
  ਅਚਲ ਅੱਖਰ *init = l_getenv(name + 1);
  ਜੇ (init == ਨਿਰਾ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ਜੇ (init == ਨਿਰਾ) ਵਾਪਸ LUA_OK;
  ਨਹੀਂਤਾਂ ਜੇ (init[0] == '@')
    ਵਾਪਸ dofile(L, init+1);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ਜੇ !defined(LUA_PROMPT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_PROMPT		"> "
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_PROMPT2		">> "
#ਜੇ_ਅੰਤ

#ਜੇ !defined(LUA_MAXINPUT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MAXINPUT		512
#ਜੇ_ਅੰਤ


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ਜੇ !defined(lua_stdin_is_tty)	/* { */

#ਜੇ defined(LUA_USE_POSIX)	/* { */

#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_stdin_is_tty()	isatty(0)

#ਜਾਂਜੇ defined(LUA_USE_WINDOWS)	/* }{ */

#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_stdin_is_tty()	_isatty(_fileno(stdin))

#ਨਹੀਂਤਾਂ				/* }{ */

/* ISO C definition */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_stdin_is_tty()	1  /* assume stdin is a tty */

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#ਜੇ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ਜੇ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ਅਨ੍ਤਰ੍ਭਾਵ <readline/readline.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <readline/history.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_initreadline(L)  \
	((ਖਾਲੀ)L, *cast(ਅਚਲ ਅੱਖਰ **, &rl_readline_name)="lua")
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_readline(buff,prompt)	((ਖਾਲੀ)buff, readline(prompt))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_saveline(ਸਤਰ)	ਜੋੜੋ_history(ਸਤਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_freeline(ਸਤਰ)	ਮੁਕਤ(ਸਤਰ)

#ਨਹੀਂਤਾਂ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ *(*l_readlineT) (ਅਚਲ ਅੱਖਰ *prompt);
ਸਥਿਰ l_readlineT l_readline = ਨਿਰਾ;

/* pointer to 'add_history' function (if any) */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*l_addhistT) (ਅਚਲ ਅੱਖਰ *string);
ਸਥਿਰ l_addhistT l_addhist = ਨਿਰਾ;


ਸਥਿਰ ਅੱਖਰ *lua_readline (ਅੱਖਰ *buff, ਅਚਲ ਅੱਖਰ *prompt) {
  ਜੇ (l_readline != ਨਿਰਾ)  /* is there a 'readline'? */
    ਵਾਪਸ (*l_readline)(prompt);  /* use it */
  ਨਹੀਂਤਾਂ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ਵਾਪਸ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ਸਥਿਰ ਖਾਲੀ lua_saveline (ਅਚਲ ਅੱਖਰ *ਸਤਰ) {
  ਜੇ (l_addhist != ਨਿਰਾ)  /* is there an 'add_history'? */
    (*l_addhist)(ਸਤਰ);  /* use it */
  /* else nothing to be done */
}


ਸਥਿਰ ਖਾਲੀ lua_freeline (ਅੱਖਰ *ਸਤਰ) {
  ਜੇ (l_readline != ਨਿਰਾ)  /* is there a 'readline'? */
    ਮੁਕਤ(ਸਤਰ);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ਜੇ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

ਸਥਿਰ ਖਾਲੀ lua_initreadline (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  ਖਾਲੀ *lib;  /* library handle */
  ਜੇ (rllib == ਨਿਰਾ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ਜੇ (lib != ਨਿਰਾ) {
    ਅਚਲ ਅੱਖਰ **name = cast(ਅਚਲ ਅੱਖਰ**, dlsym(lib, "rl_readline_name"));
    ਜੇ (name != ਨਿਰਾ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ਜੇ (l_readline != ਨਿਰਾ)  /* could load readline function? */
      ਵਾਪਸ;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#ਨਹੀਂਤਾਂ		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_initreadline(L)	((ਖਾਲੀ)L)

#ਜੇ_ਅੰਤ		/* } */

#ਜੇ_ਅੰਤ				/* } */

#ਜੇ_ਅੰਤ				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get_prompt (lua_State *L, ਪੂਰਨ firstline) {
  ਜੇ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ਵਾਪਸ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  ਨਹੀਂਤਾਂ {  /* apply 'tostring' over the value */
    ਅਚਲ ਅੱਖਰ *p = luaL_tolstring(L, -1, ਨਿਰਾ);
    lua_remove(L, -2);  /* remove original value */
    ਵਾਪਸ p;
  }
}

/* mark in error messages for incomplete statements */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOFMARK		"<eof>"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ marklen		(ਆਕਾਰ(EOFMARK)/ਆਕਾਰ(ਅੱਖਰ) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ਸਥਿਰ ਪੂਰਨ incomplete (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ) {
  ਜੇ (ਸਥਿਤੀ == LUA_ERRSYNTAX) {
    size_t lmsg;
    ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = lua_tolstring(L, -1, &lmsg);
    ਜੇ (lmsg >= marklen && strcmp(ਸੁਨੇਹਾ + lmsg - marklen, EOFMARK) == 0)
      ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ਸਥਿਰ ਪੂਰਨ pushline (lua_State *L, ਪੂਰਨ firstline) {
  ਅੱਖਰ ਬਫਰ[LUA_MAXINPUT];
  size_t l;
  ਅਚਲ ਅੱਖਰ *prmt = get_prompt(L, firstline);
  ਅੱਖਰ *b = lua_readline(ਬਫਰ, prmt);
  lua_ਕੱਢੋ(L, 1);  /* remove prompt */
  ਜੇ (b == ਨਿਰਾ)
    ਵਾਪਸ 0;  /* no input */
  l = ਲੜੀ_ਲੰਬਾਈ(b);
  ਜੇ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ਵਾਪਸ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ਸਥਿਰ ਪੂਰਨ addreturn (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਸਤਰ = lua_tostring(L, -1);  /* original line */
  ਅਚਲ ਅੱਖਰ *retline = lua_pushfstring(L, "return %s;", ਸਤਰ);
  ਪੂਰਨ ਸਥਿਤੀ = luaL_loadbufferx(L, retline, ਲੜੀ_ਲੰਬਾਈ(retline), "=stdin", "t");
  ਜੇ (ਸਥਿਤੀ == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  ਨਹੀਂਤਾਂ
    lua_ਕੱਢੋ(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ਵਾਪਸ ਸਥਿਤੀ;
}


ਸਥਿਰ ਖਾਲੀ checklocal (ਅਚਲ ਅੱਖਰ *ਸਤਰ) {
  ਸਥਿਰ ਅਚਲ size_t szloc = ਆਕਾਰ("local") - 1;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ space[] = " \t";
  ਸਤਰ += strspn(ਸਤਰ, space);  /* skip spaces */
  ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ਸਤਰ, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(ਸਤਰ + szloc)) != ਨਿਰਾ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ਸਥਿਰ ਪੂਰਨ multiline (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *ਸਤਰ = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(ਸਤਰ);
  ਲਈ (;;) {  /* repeat until gets a complete statement */
    ਪੂਰਨ ਸਥਿਤੀ = luaL_loadbufferx(L, ਸਤਰ, len, "=stdin", "t");  /* try it */
    ਜੇ (!incomplete(L, ਸਥਿਤੀ) || !pushline(L, 0))
      ਵਾਪਸ ਸਥਿਤੀ;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_ਪਾਓ(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    ਸਤਰ = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ਸਥਿਰ ਪੂਰਨ loadline (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਸਤਰ;
  ਪੂਰਨ ਸਥਿਤੀ;
  lua_settop(L, 0);
  ਜੇ (!pushline(L, 1))
    ਵਾਪਸ -1;  /* no input */
  ਜੇ ((ਸਥਿਤੀ = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    ਸਥਿਤੀ = multiline(L);  /* try as command, maybe with continuation lines */
  ਸਤਰ = lua_tostring(L, 1);
  ਜੇ (ਸਤਰ[0] != '\0')  /* non empty? */
    lua_saveline(ਸਤਰ);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ਵਾਪਸ ਸਥਿਤੀ;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ਸਥਿਰ ਖਾਲੀ l_print (lua_State *L) {
  ਪੂਰਨ n = lua_gettop(L);
  ਜੇ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_ਪਾਓ(L, 1);
    ਜੇ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_ਸੁਨੇਹਾ(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ਸਥਿਰ ਖਾਲੀ doREPL (lua_State *L) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਅਚਲ ਅੱਖਰ *oldprogname = progname;
  progname = ਨਿਰਾ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ਜਦੋਂ ((ਸਥਿਤੀ = loadline(L)) != -1) {
    ਜੇ (ਸਥਿਤੀ == LUA_OK)
      ਸਥਿਤੀ = docall(L, 0, LUA_MULTRET);
    ਜੇ (ਸਥਿਤੀ == LUA_OK) l_print(L);
    ਨਹੀਂਤਾਂ report(L, ਸਥਿਤੀ);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ਜੇ !defined(luai_openlibs)
#ਜੇ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#ਨਹੀਂਤਾਂ
/* The default is to open all standard libraries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ਸਥਿਰ ਪੂਰਨ pmain (lua_State *L) {
  ਪੂਰਨ argc = (ਪੂਰਨ)lua_tointeger(L, 1);
  ਅੱਖਰ **argv = (ਅੱਖਰ **)lua_touserdata(L, 2);
  ਪੂਰਨ script;
  ਪੂਰਨ ਦਲੀਲਾਂ = collectargs(argv, &script);
  ਪੂਰਨ optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ਜੇ (ਦਲੀਲਾਂ == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ਵਾਪਸ 0;
  }
  ਜੇ (ਦਲੀਲਾਂ & has_v)  /* option '-v'? */
    print_ਸੰਸਕਰਣ();
  ਜੇ (ਦਲੀਲਾਂ & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  ਨਹੀਂਤਾਂ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ਜੇ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ਵਾਪਸ 0;  /* error running LUA_INIT */
  ਜੇ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ਵਾਪਸ 0;  /* something failed */
  ਜੇ (script > 0) {  /* execute main script (if there is one) */
    ਜੇ (handle_script(L, argv + script) != LUA_OK)
      ਵਾਪਸ 0;  /* interrupt in case of error */
  }
  ਜੇ (ਦਲੀਲਾਂ & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  ਨਹੀਂਤਾਂ ਜੇ (script < 1 && !(ਦਲੀਲਾਂ & (has_e | has_v))) { /* no active option? */
    ਜੇ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_ਸੰਸਕਰਣ();
      doREPL(L);  /* do read-eval-print loop */
    }
    ਨਹੀਂਤਾਂ dofile(L, ਨਿਰਾ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ਵਾਪਸ 1;
}


ਪੂਰਨ ਮੁੱਖ (ਪੂਰਨ argc, ਅੱਖਰ **argv) {
  ਪੂਰਨ ਸਥਿਤੀ, ਨਤੀਜਾ;
  lua_State *L = luaL_newstate();  /* create state */
  ਜੇ (L == ਨਿਰਾ) {
    l_ਸੁਨੇਹਾ(argv[0], "cannot create state: not enough memory");
    ਵਾਪਸ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  ਸਥਿਤੀ = lua_pcall(L, 2, 1, 0);  /* do the call */
  ਨਤੀਜਾ = lua_toboolean(L, -1);  /* get result */
  report(L, ਸਥਿਤੀ);
  lua_close(L);
  ਵਾਪਸ (ਨਤੀਜਾ && ਸਥਿਤੀ == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

