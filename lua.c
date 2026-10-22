/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#నిర్వచించు lua_c

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ <signal.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


#ఒకవేళ !defined(LUA_PROGNAME)
#నిర్వచించు LUA_PROGNAME		"lua"
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_INIT_VAR)
#నిర్వచించు LUA_INIT_VAR		"LUA_INIT"
#అయితే_ముగింపు

/* Name of the environment variable with the name of the readline library */
#ఒకవేళ !defined(LUA_RLLIB_VAR)
#నిర్వచించు LUA_RLLIB_VAR		"LUA_READLINELIB"
#అయితే_ముగింపు


#నిర్వచించు LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


స్థిర lua_State *globalL = ఖాళీ;

స్థిర స్థిరరాశి అక్షరం *progname = LUA_PROGNAME;


#ఒకవేళ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
స్థిర శూన్య setsignal (పూర్ణాంకం sig, శూన్య (*handler)(పూర్ణాంకం)) {
  నిర్మాణం sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ఖాళీ);
}

#లేకపోతే           /* }{ */

#నిర్వచించు setsignal            సంకేతం

#అయితే_ముగింపు                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
స్థిర శూన్య lstop (lua_State *L, lua_Debug *ar) {
  (శూన్య)ar;  /* unused arg. */
  lua_sethook(L, ఖాళీ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
స్థిర శూన్య laction (పూర్ణాంకం i) {
  పూర్ణాంకం flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


స్థిర శూన్య print_usage (స్థిరరాశి అక్షరం *badoption) {
  lua_writestringerror("%s: ", progname);
  ఒకవేళ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  లేకపోతే
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
స్థిర శూన్య l_సందేశం (స్థిరరాశి అక్షరం *pname, స్థిరరాశి అక్షరం *సందేశం) {
  ఒకవేళ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", సందేశం);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
స్థిర పూర్ణాంకం report (lua_State *L, పూర్ణాంకం స్థితి_2) {
  ఒకవేళ (స్థితి_2 != LUA_OK) {
    స్థిరరాశి అక్షరం *సందేశం = lua_tostring(L, -1);
    ఒకవేళ (సందేశం == ఖాళీ)
      సందేశం = "(error message not a string)";
    l_సందేశం(progname, సందేశం);
    lua_తీయి(L, 1);  /* remove message */
  }
  తిరిగి స్థితి_2;
}


/*
** Message handler used to run all chunks
*/
స్థిర పూర్ణాంకం msghandler (lua_State *L) {
  స్థిరరాశి అక్షరం *సందేశం = lua_tostring(L, 1);
  ఒకవేళ (సందేశం == ఖాళీ) {  /* is error object not a string? */
    ఒకవేళ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      తిరిగి 1;  /* that is the message */
    లేకపోతే
      సందేశం = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, సందేశం, 1);  /* append a standard traceback */
  తిరిగి 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
స్థిర పూర్ణాంకం docall (lua_State *L, పూర్ణాంకం narg, పూర్ణాంకం nres) {
  పూర్ణాంకం స్థితి_2;
  పూర్ణాంకం పునాది = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_చొప్పించు(L, పునాది);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  స్థితి_2 = lua_pcall(L, narg, nres, పునాది);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, పునాది);  /* remove message handler from the stack */
  తిరిగి స్థితి_2;
}


స్థిర శూన్య print_సంస్కరణ (శూన్య) {
  lua_writestring(LUA_COPYRIGHT, అక్షరశ్రేణి_పొడవు(LUA_COPYRIGHT));
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
స్థిర శూన్య createargtable (lua_State *L, అక్షరం **argv, పూర్ణాంకం argc, పూర్ణాంకం script) {
  పూర్ణాంకం i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  కొరకు (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


స్థిర పూర్ణాంకం dochunk (lua_State *L, పూర్ణాంకం స్థితి_2) {
  ఒకవేళ (స్థితి_2 == LUA_OK) స్థితి_2 = docall(L, 0, 0);
  తిరిగి report(L, స్థితి_2);
}


స్థిర పూర్ణాంకం dofile (lua_State *L, స్థిరరాశి అక్షరం *name) {
  తిరిగి dochunk(L, luaL_loadfilex(L, name, "bt"));
}


స్థిర పూర్ణాంకం dostring (lua_State *L, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *name) {
  తిరిగి dochunk(L, luaL_loadbufferx(L, s, అక్షరశ్రేణి_పొడవు(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
స్థిర పూర్ణాంకం dolibrary (lua_State *L, అక్షరం *globname) {
  పూర్ణాంకం స్థితి_2;
  అక్షరం *suffix = ఖాళీ;
  అక్షరం *modname = strchr(globname, '=');
  ఒకవేళ (modname == ఖాళీ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  లేకపోతే {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  స్థితి_2 = docall(L, 1, 1);  /* call 'require(modname)' */
  ఒకవేళ (స్థితి_2 == LUA_OK) {
    ఒకవేళ (suffix != ఖాళీ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  తిరిగి report(L, స్థితి_2);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
స్థిర పూర్ణాంకం pushargs (lua_State *L) {
  పూర్ణాంకం i, n;
  ఒకవేళ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (పూర్ణాంకం)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  కొరకు (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  తిరిగి n;
}


స్థిర పూర్ణాంకం handle_script (lua_State *L, అక్షరం **argv) {
  పూర్ణాంకం స్థితి_2;
  స్థిరరాశి అక్షరం *fname = argv[0];
  ఒకవేళ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ఖాళీ;  /* stdin */
  స్థితి_2 = luaL_loadfilex(L, fname, "bt");
  ఒకవేళ (స్థితి_2 == LUA_OK) {
    పూర్ణాంకం n = pushargs(L);  /* push arguments to script */
    స్థితి_2 = docall(L, n, LUA_MULTRET);
  }
  తిరిగి report(L, స్థితి_2);
}


/* bits of various argument indicators in 'args' */
#నిర్వచించు has_error	1	/* bad option */
#నిర్వచించు has_i		2	/* -i */
#నిర్వచించు has_v		4	/* -v */
#నిర్వచించు has_e		8	/* -e */
#నిర్వచించు has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
స్థిర పూర్ణాంకం collectargs (అక్షరం **argv, పూర్ణాంకం *మొదటి) {
  పూర్ణాంకం వాదనలు = 0;
  పూర్ణాంకం i;
  ఒకవేళ (argv[0] != ఖాళీ) {  /* is there a program name? */
    ఒకవేళ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  లేకపోతే {  /* no program name */
    *మొదటి = -1;
    తిరిగి 0;
  }
  కొరకు (i = 1; argv[i] != ఖాళీ; i++) {  /* handle arguments */
    *మొదటి = i;
    ఒకవేళ (argv[i][0] != '-')  /* not an option? */
        తిరిగి వాదనలు;  /* stop handling options */
    ఎంపిక (argv[i][1]) {  /* else check option */
      స్థితి '-':  /* '--' */
        ఒకవేళ (argv[i][2] != '\0')  /* extra characters after '--'? */
          తిరిగి has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *మొదటి = (argv[i + 1] != ఖాళీ) ? i + 1 : 0;
        తిరిగి వాదనలు;
      స్థితి '\0':  /* '-' */
        తిరిగి వాదనలు;  /* script "name" is '-' */
      స్థితి 'E':
        ఒకవేళ (argv[i][2] != '\0')  /* extra characters? */
          తిరిగి has_error;  /* invalid option */
        వాదనలు |= has_E;
        ఆపు;
      స్థితి 'W':
        ఒకవేళ (argv[i][2] != '\0')  /* extra characters? */
          తిరిగి has_error;  /* invalid option */
        ఆపు;
      స్థితి 'i':
        వాదనలు |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      స్థితి 'v':
        ఒకవేళ (argv[i][2] != '\0')  /* extra characters? */
          తిరిగి has_error;  /* invalid option */
        వాదనలు |= has_v;
        ఆపు;
      స్థితి 'e':
        వాదనలు |= has_e;  /* FALLTHROUGH */
      స్థితి 'l':  /* both options need an argument */
        ఒకవేళ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ఒకవేళ (argv[i] == ఖాళీ || argv[i][0] == '-')
            తిరిగి has_error;  /* no next argument or it is another option */
        }
        ఆపు;
      అప్రమేయ:  /* invalid option */
        తిరిగి has_error;
    }
  }
  *మొదటి = 0;  /* no script name */
  తిరిగి వాదనలు;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
స్థిర పూర్ణాంకం runargs (lua_State *L, అక్షరం **argv, పూర్ణాంకం n) {
  పూర్ణాంకం i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  కొరకు (i = 1; i < n; i++) {
    పూర్ణాంకం option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    ఎంపిక (option) {
      స్థితి 'e':  స్థితి 'l': {
        పూర్ణాంకం స్థితి_2;
        అక్షరం *extra = argv[i] + 2;  /* both options need an argument */
        ఒకవేళ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ఖాళీ);
        స్థితి_2 = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ఒకవేళ (స్థితి_2 != LUA_OK) తిరిగి 0;
        ఆపు;
      }
      స్థితి 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        ఆపు;
    }
  }
  తిరిగి 1;
}


స్థిర అక్షరం *(*l_getenv)(స్థిరరాశి అక్షరం *name);

/* Function to ignore environment variables, used by option -E */
స్థిర అక్షరం *no_getenv (స్థిరరాశి అక్షరం *name) {
  వాడని(name);
  తిరిగి ఖాళీ;
}


స్థిర పూర్ణాంకం handle_luainit (lua_State *L) {
  స్థిరరాశి అక్షరం *name = "=" LUA_INITVARVERSION;
  స్థిరరాశి అక్షరం *init = l_getenv(name + 1);
  ఒకవేళ (init == ఖాళీ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ఒకవేళ (init == ఖాళీ) తిరిగి LUA_OK;
  లేకపోతే ఒకవేళ (init[0] == '@')
    తిరిగి dofile(L, init+1);
  లేకపోతే
    తిరిగి dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ఒకవేళ !defined(LUA_PROMPT)
#నిర్వచించు LUA_PROMPT		"> "
#నిర్వచించు LUA_PROMPT2		">> "
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_MAXINPUT)
#నిర్వచించు LUA_MAXINPUT		512
#అయితే_ముగింపు


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ఒకవేళ !defined(lua_stdin_is_tty)	/* { */

#ఒకవేళ defined(LUA_USE_POSIX)	/* { */

#అన్తర్భావ <unistd.h>
#నిర్వచించు lua_stdin_is_tty()	isatty(0)

#లేదావేళ defined(LUA_USE_WINDOWS)	/* }{ */

#అన్తర్భావ <io.h>
#అన్తర్భావ <windows.h>

#నిర్వచించు lua_stdin_is_tty()	_isatty(_fileno(stdin))

#లేకపోతే				/* }{ */

/* ISO C definition */
#నిర్వచించు lua_stdin_is_tty()	1  /* assume stdin is a tty */

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#ఒకవేళ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ఒకవేళ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#అన్తర్భావ <readline/readline.h>
#అన్తర్భావ <readline/history.h>

#నిర్వచించు lua_initreadline(L)  \
	((శూన్య)L, *cast(స్థిరరాశి అక్షరం **, &rl_readline_name)="lua")
#నిర్వచించు lua_readline(buff,prompt)	((శూన్య)buff, readline(prompt))
#నిర్వచించు lua_saveline(పంక్తి)	కూడు_history(పంక్తి)
#నిర్వచించు lua_freeline(పంక్తి)	విడుదల(పంక్తి)

#లేకపోతే		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
రకనిర్వచనం అక్షరం *(*l_readlineT) (స్థిరరాశి అక్షరం *prompt);
స్థిర l_readlineT l_readline = ఖాళీ;

/* pointer to 'add_history' function (if any) */
రకనిర్వచనం శూన్య (*l_addhistT) (స్థిరరాశి అక్షరం *string);
స్థిర l_addhistT l_addhist = ఖాళీ;


స్థిర అక్షరం *lua_readline (అక్షరం *buff, స్థిరరాశి అక్షరం *prompt) {
  ఒకవేళ (l_readline != ఖాళీ)  /* is there a 'readline'? */
    తిరిగి (*l_readline)(prompt);  /* use it */
  లేకపోతే {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    తిరిగి fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


స్థిర శూన్య lua_saveline (స్థిరరాశి అక్షరం *పంక్తి) {
  ఒకవేళ (l_addhist != ఖాళీ)  /* is there an 'add_history'? */
    (*l_addhist)(పంక్తి);  /* use it */
  /* else nothing to be done */
}


స్థిర శూన్య lua_freeline (అక్షరం *పంక్తి) {
  ఒకవేళ (l_readline != ఖాళీ)  /* is there a 'readline'? */
    విడుదల(పంక్తి);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ఒకవేళ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#అన్తర్భావ <dlfcn.h>

స్థిర శూన్య lua_initreadline (lua_State *L) {
  స్థిరరాశి అక్షరం *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  శూన్య *lib;  /* library handle */
  ఒకవేళ (rllib == ఖాళీ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ఒకవేళ (lib != ఖాళీ) {
    స్థిరరాశి అక్షరం **name = cast(స్థిరరాశి అక్షరం**, dlsym(lib, "rl_readline_name"));
    ఒకవేళ (name != ఖాళీ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ఒకవేళ (l_readline != ఖాళీ)  /* could load readline function? */
      తిరిగి;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#లేకపోతే		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#నిర్వచించు lua_initreadline(L)	((శూన్య)L)

#అయితే_ముగింపు		/* } */

#అయితే_ముగింపు				/* } */

#అయితే_ముగింపు				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
స్థిర స్థిరరాశి అక్షరం *get_prompt (lua_State *L, పూర్ణాంకం firstline) {
  ఒకవేళ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    తిరిగి (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  లేకపోతే {  /* apply 'tostring' over the value */
    స్థిరరాశి అక్షరం *p = luaL_tolstring(L, -1, ఖాళీ);
    lua_remove(L, -2);  /* remove original value */
    తిరిగి p;
  }
}

/* mark in error messages for incomplete statements */
#నిర్వచించు EOFMARK		"<eof>"
#నిర్వచించు marklen		(పరిమాణం(EOFMARK)/పరిమాణం(అక్షరం) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
స్థిర పూర్ణాంకం incomplete (lua_State *L, పూర్ణాంకం స్థితి_2) {
  ఒకవేళ (స్థితి_2 == LUA_ERRSYNTAX) {
    size_t lmsg;
    స్థిరరాశి అక్షరం *సందేశం = lua_tolstring(L, -1, &lmsg);
    ఒకవేళ (lmsg >= marklen && strcmp(సందేశం + lmsg - marklen, EOFMARK) == 0)
      తిరిగి 1;
  }
  తిరిగి 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
స్థిర పూర్ణాంకం pushline (lua_State *L, పూర్ణాంకం firstline) {
  అక్షరం బఫరు[LUA_MAXINPUT];
  size_t l;
  స్థిరరాశి అక్షరం *prmt = get_prompt(L, firstline);
  అక్షరం *b = lua_readline(బఫరు, prmt);
  lua_తీయి(L, 1);  /* remove prompt */
  ఒకవేళ (b == ఖాళీ)
    తిరిగి 0;  /* no input */
  l = అక్షరశ్రేణి_పొడవు(b);
  ఒకవేళ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  తిరిగి 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
స్థిర పూర్ణాంకం addreturn (lua_State *L) {
  స్థిరరాశి అక్షరం *పంక్తి = lua_tostring(L, -1);  /* original line */
  స్థిరరాశి అక్షరం *retline = lua_pushfstring(L, "return %s;", పంక్తి);
  పూర్ణాంకం స్థితి_2 = luaL_loadbufferx(L, retline, అక్షరశ్రేణి_పొడవు(retline), "=stdin", "t");
  ఒకవేళ (స్థితి_2 == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  లేకపోతే
    lua_తీయి(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  తిరిగి స్థితి_2;
}


స్థిర శూన్య checklocal (స్థిరరాశి అక్షరం *పంక్తి) {
  స్థిర స్థిరరాశి size_t szloc = పరిమాణం("local") - 1;
  స్థిర స్థిరరాశి అక్షరం space[] = " \t";
  పంక్తి += strspn(పంక్తి, space);  /* skip spaces */
  ఒకవేళ (అక్షరశ్రేణి_పోలిక(పంక్తి, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(పంక్తి + szloc)) != ఖాళీ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
స్థిర పూర్ణాంకం multiline (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *పంక్తి = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(పంక్తి);
  కొరకు (;;) {  /* repeat until gets a complete statement */
    పూర్ణాంకం స్థితి_2 = luaL_loadbufferx(L, పంక్తి, len, "=stdin", "t");  /* try it */
    ఒకవేళ (!incomplete(L, స్థితి_2) || !pushline(L, 0))
      తిరిగి స్థితి_2;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_చొప్పించు(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    పంక్తి = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
స్థిర పూర్ణాంకం loadline (lua_State *L) {
  స్థిరరాశి అక్షరం *పంక్తి;
  పూర్ణాంకం స్థితి_2;
  lua_settop(L, 0);
  ఒకవేళ (!pushline(L, 1))
    తిరిగి -1;  /* no input */
  ఒకవేళ ((స్థితి_2 = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    స్థితి_2 = multiline(L);  /* try as command, maybe with continuation lines */
  పంక్తి = lua_tostring(L, 1);
  ఒకవేళ (పంక్తి[0] != '\0')  /* non empty? */
    lua_saveline(పంక్తి);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  తిరిగి స్థితి_2;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
స్థిర శూన్య l_print (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);
  ఒకవేళ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_చొప్పించు(L, 1);
    ఒకవేళ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_సందేశం(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
స్థిర శూన్య doREPL (lua_State *L) {
  పూర్ణాంకం స్థితి_2;
  స్థిరరాశి అక్షరం *oldprogname = progname;
  progname = ఖాళీ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  అయితే ((స్థితి_2 = loadline(L)) != -1) {
    ఒకవేళ (స్థితి_2 == LUA_OK)
      స్థితి_2 = docall(L, 0, LUA_MULTRET);
    ఒకవేళ (స్థితి_2 == LUA_OK) l_print(L);
    లేకపోతే report(L, స్థితి_2);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ఒకవేళ !defined(luai_openlibs)
#ఒకవేళ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#నిర్వచించు luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#లేకపోతే
/* The default is to open all standard libraries */
#నిర్వచించు luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#అయితే_ముగింపు
#అయితే_ముగింపు


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
స్థిర పూర్ణాంకం pmain (lua_State *L) {
  పూర్ణాంకం argc = (పూర్ణాంకం)lua_tointeger(L, 1);
  అక్షరం **argv = (అక్షరం **)lua_touserdata(L, 2);
  పూర్ణాంకం script;
  పూర్ణాంకం వాదనలు = collectargs(argv, &script);
  పూర్ణాంకం optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ఒకవేళ (వాదనలు == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    తిరిగి 0;
  }
  ఒకవేళ (వాదనలు & has_v)  /* option '-v'? */
    print_సంస్కరణ();
  ఒకవేళ (వాదనలు & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  లేకపోతే
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ఒకవేళ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    తిరిగి 0;  /* error running LUA_INIT */
  ఒకవేళ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    తిరిగి 0;  /* something failed */
  ఒకవేళ (script > 0) {  /* execute main script (if there is one) */
    ఒకవేళ (handle_script(L, argv + script) != LUA_OK)
      తిరిగి 0;  /* interrupt in case of error */
  }
  ఒకవేళ (వాదనలు & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  లేకపోతే ఒకవేళ (script < 1 && !(వాదనలు & (has_e | has_v))) { /* no active option? */
    ఒకవేళ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_సంస్కరణ();
      doREPL(L);  /* do read-eval-print loop */
    }
    లేకపోతే dofile(L, ఖాళీ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  తిరిగి 1;
}


పూర్ణాంకం ప్రధాన (పూర్ణాంకం argc, అక్షరం **argv) {
  పూర్ణాంకం స్థితి_2, ఫలితం;
  lua_State *L = luaL_newstate();  /* create state */
  ఒకవేళ (L == ఖాళీ) {
    l_సందేశం(argv[0], "cannot create state: not enough memory");
    తిరిగి EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  స్థితి_2 = lua_pcall(L, 2, 1, 0);  /* do the call */
  ఫలితం = lua_toboolean(L, -1);  /* get result */
  report(L, స్థితి_2);
  lua_close(L);
  తిరిగి (ఫలితం && స్థితి_2 == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

