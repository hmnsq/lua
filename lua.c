/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव <signal.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


#जइ !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#जइ !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#जइ !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


स्थिर lua_State *globalL = रिक्त;

स्थिर ध्रुव वर्ण *progname = LUA_PROGNAME;


#जइ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
स्थिर शून्य setsignal (पूर्णाङ्क sig, शून्य (*handler)(पूर्णाङ्क)) {
  संरचना sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, रिक्त);
}

#अण्णहा           /* }{ */

#define setsignal            सङ्केत

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
स्थिर शून्य lstop (lua_State *L, lua_Debug *ar) {
  (शून्य)ar;  /* unused arg. */
  lua_sethook(L, रिक्त, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
स्थिर शून्य laction (पूर्णाङ्क i) {
  पूर्णाङ्क flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


स्थिर शून्य print_usage (ध्रुव वर्ण *badoption) {
  lua_writestringerror("%s: ", progname);
  जइ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  अण्णहा
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
स्थिर शून्य l_message (ध्रुव वर्ण *pname, ध्रुव वर्ण *सन्देशः) {
  जइ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", सन्देशः);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
स्थिर पूर्णाङ्क report (lua_State *L, पूर्णाङ्क status) {
  जइ (status != LUA_OK) {
    ध्रुव वर्ण *सन्देशः = lua_tostring(L, -1);
    जइ (सन्देशः == रिक्त)
      सन्देशः = "(error message not a string)";
    l_message(progname, सन्देशः);
    lua_निष्कासय(L, 1);  /* remove message */
  }
  प्रत्यागम status;
}


/*
** Message handler used to run all chunks
*/
स्थिर पूर्णाङ्क msghandler (lua_State *L) {
  ध्रुव वर्ण *सन्देशः = lua_tostring(L, 1);
  जइ (सन्देशः == रिक्त) {  /* is error object not a string? */
    जइ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      प्रत्यागम 1;  /* that is the message */
    अण्णहा
      सन्देशः = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, सन्देशः, 1);  /* append a standard traceback */
  प्रत्यागम 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
स्थिर पूर्णाङ्क docall (lua_State *L, पूर्णाङ्क narg, पूर्णाङ्क nres) {
  पूर्णाङ्क status;
  पूर्णाङ्क base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_निवेशय(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  प्रत्यागम status;
}


स्थिर शून्य print_version (शून्य) {
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
स्थिर शून्य createargtable (lua_State *L, वर्ण **argv, पूर्णाङ्क argc, पूर्णाङ्क script) {
  पूर्णाङ्क i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  प्रति (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


स्थिर पूर्णाङ्क dochunk (lua_State *L, पूर्णाङ्क status) {
  जइ (status == LUA_OK) status = docall(L, 0, 0);
  प्रत्यागम report(L, status);
}


स्थिर पूर्णाङ्क dofile (lua_State *L, ध्रुव वर्ण *name) {
  प्रत्यागम dochunk(L, luaL_loadfilex(L, name, "bt"));
}


स्थिर पूर्णाङ्क dostring (lua_State *L, ध्रुव वर्ण *s, ध्रुव वर्ण *name) {
  प्रत्यागम dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
स्थिर पूर्णाङ्क dolibrary (lua_State *L, वर्ण *globname) {
  पूर्णाङ्क status;
  वर्ण *suffix = रिक्त;
  वर्ण *modname = strchr(globname, '=');
  जइ (modname == रिक्त) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  अण्णहा {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  जइ (status == LUA_OK) {
    जइ (suffix != रिक्त)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  प्रत्यागम report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
स्थिर पूर्णाङ्क pushargs (lua_State *L) {
  पूर्णाङ्क i, n;
  जइ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (पूर्णाङ्क)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  प्रति (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  प्रत्यागम n;
}


स्थिर पूर्णाङ्क handle_script (lua_State *L, वर्ण **argv) {
  पूर्णाङ्क status;
  ध्रुव वर्ण *fname = argv[0];
  जइ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = रिक्त;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  जइ (status == LUA_OK) {
    पूर्णाङ्क n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  प्रत्यागम report(L, status);
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
स्थिर पूर्णाङ्क collectargs (वर्ण **argv, पूर्णाङ्क *first) {
  पूर्णाङ्क args = 0;
  पूर्णाङ्क i;
  जइ (argv[0] != रिक्त) {  /* is there a program name? */
    जइ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  अण्णहा {  /* no program name */
    *first = -1;
    प्रत्यागम 0;
  }
  प्रति (i = 1; argv[i] != रिक्त; i++) {  /* handle arguments */
    *first = i;
    जइ (argv[i][0] != '-')  /* not an option? */
        प्रत्यागम args;  /* stop handling options */
    विकल्प (argv[i][1]) {  /* else check option */
      पक्ष '-':  /* '--' */
        जइ (argv[i][2] != '\0')  /* extra characters after '--'? */
          प्रत्यागम has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != रिक्त) ? i + 1 : 0;
        प्रत्यागम args;
      पक्ष '\0':  /* '-' */
        प्रत्यागम args;  /* script "name" is '-' */
      पक्ष 'E':
        जइ (argv[i][2] != '\0')  /* extra characters? */
          प्रत्यागम has_error;  /* invalid option */
        args |= has_E;
        भङ्ग;
      पक्ष 'W':
        जइ (argv[i][2] != '\0')  /* extra characters? */
          प्रत्यागम has_error;  /* invalid option */
        भङ्ग;
      पक्ष 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      पक्ष 'v':
        जइ (argv[i][2] != '\0')  /* extra characters? */
          प्रत्यागम has_error;  /* invalid option */
        args |= has_v;
        भङ्ग;
      पक्ष 'e':
        args |= has_e;  /* FALLTHROUGH */
      पक्ष 'l':  /* both options need an argument */
        जइ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          जइ (argv[i] == रिक्त || argv[i][0] == '-')
            प्रत्यागम has_error;  /* no next argument or it is another option */
        }
        भङ्ग;
      स्वभाव:  /* invalid option */
        प्रत्यागम has_error;
    }
  }
  *first = 0;  /* no script name */
  प्रत्यागम args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
स्थिर पूर्णाङ्क runargs (lua_State *L, वर्ण **argv, पूर्णाङ्क n) {
  पूर्णाङ्क i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  प्रति (i = 1; i < n; i++) {
    पूर्णाङ्क option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    विकल्प (option) {
      पक्ष 'e':  पक्ष 'l': {
        पूर्णाङ्क status;
        वर्ण *extra = argv[i] + 2;  /* both options need an argument */
        जइ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != रिक्त);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        जइ (status != LUA_OK) प्रत्यागम 0;
        भङ्ग;
      }
      पक्ष 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        भङ्ग;
    }
  }
  प्रत्यागम 1;
}


स्थिर वर्ण *(*l_getenv)(ध्रुव वर्ण *name);

/* Function to ignore environment variables, used by option -E */
स्थिर वर्ण *no_getenv (ध्रुव वर्ण *name) {
  UNUSED(name);
  प्रत्यागम रिक्त;
}


स्थिर पूर्णाङ्क handle_luainit (lua_State *L) {
  ध्रुव वर्ण *name = "=" LUA_INITVARVERSION;
  ध्रुव वर्ण *init = l_getenv(name + 1);
  जइ (init == रिक्त) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  जइ (init == रिक्त) प्रत्यागम LUA_OK;
  अण्णहा जइ (init[0] == '@')
    प्रत्यागम dofile(L, init+1);
  अण्णहा
    प्रत्यागम dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#जइ !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#जइ !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#जइ !defined(lua_stdin_is_tty)	/* { */

#जइ defined(LUA_USE_POSIX)	/* { */

#अन्तर्भाव <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#नोचेद्यदि defined(LUA_USE_WINDOWS)	/* }{ */

#अन्तर्भाव <io.h>
#अन्तर्भाव <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#अण्णहा				/* }{ */

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

#जइ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#जइ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#अन्तर्भाव <readline/readline.h>
#अन्तर्भाव <readline/history.h>

#define lua_initreadline(L)  \
	((शून्य)L, *cast(ध्रुव वर्ण **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((शून्य)buff, readline(prompt))
#define lua_saveline(पङ्क्तिः)	add_history(पङ्क्तिः)
#define lua_freeline(पङ्क्तिः)	मुक्त(पङ्क्तिः)

#अण्णहा		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
प्रकारनिर्देश वर्ण *(*l_readlineT) (ध्रुव वर्ण *prompt);
स्थिर l_readlineT l_readline = रिक्त;

/* pointer to 'add_history' function (if any) */
प्रकारनिर्देश शून्य (*l_addhistT) (ध्रुव वर्ण *string);
स्थिर l_addhistT l_addhist = रिक्त;


स्थिर वर्ण *lua_readline (वर्ण *buff, ध्रुव वर्ण *prompt) {
  जइ (l_readline != रिक्त)  /* is there a 'readline'? */
    प्रत्यागम (*l_readline)(prompt);  /* use it */
  अण्णहा {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    प्रत्यागम fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


स्थिर शून्य lua_saveline (ध्रुव वर्ण *पङ्क्तिः) {
  जइ (l_addhist != रिक्त)  /* is there an 'add_history'? */
    (*l_addhist)(पङ्क्तिः);  /* use it */
  /* else nothing to be done */
}


स्थिर शून्य lua_freeline (वर्ण *पङ्क्तिः) {
  जइ (l_readline != रिक्त)  /* is there a 'readline'? */
    मुक्त(पङ्क्तिः);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#जइ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#अन्तर्भाव <dlfcn.h>

स्थिर शून्य lua_initreadline (lua_State *L) {
  ध्रुव वर्ण *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  शून्य *lib;  /* library handle */
  जइ (rllib == रिक्त)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  जइ (lib != रिक्त) {
    ध्रुव वर्ण **name = cast(ध्रुव वर्ण**, dlsym(lib, "rl_readline_name"));
    जइ (name != रिक्त)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    जइ (l_readline != रिक्त)  /* could load readline function? */
      प्रत्यागम;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#अण्णहा		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((शून्य)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
स्थिर ध्रुव वर्ण *get_prompt (lua_State *L, पूर्णाङ्क firstline) {
  जइ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    प्रत्यागम (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  अण्णहा {  /* apply 'tostring' over the value */
    ध्रुव वर्ण *p = luaL_tolstring(L, -1, रिक्त);
    lua_remove(L, -2);  /* remove original value */
    प्रत्यागम p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(परिमाण(EOFMARK)/परिमाण(वर्ण) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
स्थिर पूर्णाङ्क incomplete (lua_State *L, पूर्णाङ्क status) {
  जइ (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    ध्रुव वर्ण *सन्देशः = lua_tolstring(L, -1, &lmsg);
    जइ (lmsg >= marklen && strcmp(सन्देशः + lmsg - marklen, EOFMARK) == 0)
      प्रत्यागम 1;
  }
  प्रत्यागम 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
स्थिर पूर्णाङ्क pushline (lua_State *L, पूर्णाङ्क firstline) {
  वर्ण buffer[LUA_MAXINPUT];
  size_t l;
  ध्रुव वर्ण *prmt = get_prompt(L, firstline);
  वर्ण *b = lua_readline(buffer, prmt);
  lua_निष्कासय(L, 1);  /* remove prompt */
  जइ (b == रिक्त)
    प्रत्यागम 0;  /* no input */
  l = strlen(b);
  जइ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  प्रत्यागम 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
स्थिर पूर्णाङ्क addreturn (lua_State *L) {
  ध्रुव वर्ण *पङ्क्तिः = lua_tostring(L, -1);  /* original line */
  ध्रुव वर्ण *retline = lua_pushfstring(L, "return %s;", पङ्क्तिः);
  पूर्णाङ्क status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  जइ (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  अण्णहा
    lua_निष्कासय(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  प्रत्यागम status;
}


स्थिर शून्य checklocal (ध्रुव वर्ण *पङ्क्तिः) {
  स्थिर ध्रुव size_t szloc = परिमाण("local") - 1;
  स्थिर ध्रुव वर्ण space[] = " \t";
  पङ्क्तिः += strspn(पङ्क्तिः, space);  /* skip spaces */
  जइ (strncmp(पङ्क्तिः, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(पङ्क्तिः + szloc)) != रिक्त) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
स्थिर पूर्णाङ्क multiline (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *पङ्क्तिः = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(पङ्क्तिः);
  प्रति (;;) {  /* repeat until gets a complete statement */
    पूर्णाङ्क status = luaL_loadbufferx(L, पङ्क्तिः, len, "=stdin", "t");  /* try it */
    जइ (!incomplete(L, status) || !pushline(L, 0))
      प्रत्यागम status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_निवेशय(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    पङ्क्तिः = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
स्थिर पूर्णाङ्क loadline (lua_State *L) {
  ध्रुव वर्ण *पङ्क्तिः;
  पूर्णाङ्क status;
  lua_settop(L, 0);
  जइ (!pushline(L, 1))
    प्रत्यागम -1;  /* no input */
  जइ ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  पङ्क्तिः = lua_tostring(L, 1);
  जइ (पङ्क्तिः[0] != '\0')  /* non empty? */
    lua_saveline(पङ्क्तिः);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  प्रत्यागम status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
स्थिर शून्य l_print (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);
  जइ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_निवेशय(L, 1);
    जइ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
स्थिर शून्य doREPL (lua_State *L) {
  पूर्णाङ्क status;
  ध्रुव वर्ण *oldprogname = progname;
  progname = रिक्त;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  जाव ((status = loadline(L)) != -1) {
    जइ (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    जइ (status == LUA_OK) l_print(L);
    अण्णहा report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#जइ !defined(luai_openlibs)
#जइ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#अण्णहा
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
स्थिर पूर्णाङ्क pmain (lua_State *L) {
  पूर्णाङ्क argc = (पूर्णाङ्क)lua_tointeger(L, 1);
  वर्ण **argv = (वर्ण **)lua_touserdata(L, 2);
  पूर्णाङ्क script;
  पूर्णाङ्क args = collectargs(argv, &script);
  पूर्णाङ्क optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  जइ (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    प्रत्यागम 0;
  }
  जइ (args & has_v)  /* option '-v'? */
    print_version();
  जइ (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  अण्णहा
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  जइ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    प्रत्यागम 0;  /* error running LUA_INIT */
  जइ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    प्रत्यागम 0;  /* something failed */
  जइ (script > 0) {  /* execute main script (if there is one) */
    जइ (handle_script(L, argv + script) != LUA_OK)
      प्रत्यागम 0;  /* interrupt in case of error */
  }
  जइ (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  अण्णहा जइ (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    जइ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    अण्णहा dofile(L, रिक्त);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  प्रत्यागम 1;
}


पूर्णाङ्क main (पूर्णाङ्क argc, वर्ण **argv) {
  पूर्णाङ्क status, result;
  lua_State *L = luaL_newstate();  /* create state */
  जइ (L == रिक्त) {
    l_message(argv[0], "cannot create state: not enough memory");
    प्रत्यागम EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  प्रत्यागम (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

