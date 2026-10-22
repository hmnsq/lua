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


#जर !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#जर !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#जर !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


स्थिर lua_State *globalL = रिक्त;

स्थिर स्थिरांक अक्षर *progname = LUA_PROGNAME;


#जर defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
स्थिर शून्य setsignal (पूर्णांक sig, शून्य (*handler)(पूर्णांक)) {
  संरचना sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, रिक्त);
}

#अन्यथा           /* }{ */

#define setsignal            संकेत

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
स्थिर शून्य laction (पूर्णांक i) {
  पूर्णांक flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, संक_पूर्व); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


स्थिर शून्य print_usage (स्थिरांक अक्षर *badoption) {
  lua_writestringerror("%s: ", progname);
  जर (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  अन्यथा
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
स्थिर शून्य l_message (स्थिरांक अक्षर *pname, स्थिरांक अक्षर *संदेश) {
  जर (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", संदेश);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
स्थिर पूर्णांक report (lua_State *L, पूर्णांक status) {
  जर (status != LUA_OK) {
    स्थिरांक अक्षर *संदेश = lua_tostring(L, -1);
    जर (संदेश == रिक्त)
      संदेश = "(error message not a string)";
    l_message(progname, संदेश);
    lua_काढा(L, 1);  /* remove message */
  }
  परत status;
}


/*
** Message handler used to run all chunks
*/
स्थिर पूर्णांक msghandler (lua_State *L) {
  स्थिरांक अक्षर *संदेश = lua_tostring(L, 1);
  जर (संदेश == रिक्त) {  /* is error object not a string? */
    जर (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      परत 1;  /* that is the message */
    अन्यथा
      संदेश = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, संदेश, 1);  /* append a standard traceback */
  परत 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
स्थिर पूर्णांक docall (lua_State *L, पूर्णांक narg, पूर्णांक nres) {
  पूर्णांक status;
  पूर्णांक base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_घाला(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(संक_विघ्न, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(संक_विघ्न, संक_पूर्व); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  परत status;
}


स्थिर शून्य print_version (शून्य) {
  lua_writestring(LUA_COPYRIGHT, म_माप(LUA_COPYRIGHT));
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
स्थिर शून्य createargtable (lua_State *L, अक्षर **argv, पूर्णांक argc, पूर्णांक script) {
  पूर्णांक i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  साठी (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


स्थिर पूर्णांक dochunk (lua_State *L, पूर्णांक status) {
  जर (status == LUA_OK) status = docall(L, 0, 0);
  परत report(L, status);
}


स्थिर पूर्णांक dofile (lua_State *L, स्थिरांक अक्षर *name) {
  परत dochunk(L, luaL_loadfilex(L, name, "bt"));
}


स्थिर पूर्णांक dostring (lua_State *L, स्थिरांक अक्षर *s, स्थिरांक अक्षर *name) {
  परत dochunk(L, luaL_loadbufferx(L, s, म_माप(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
स्थिर पूर्णांक dolibrary (lua_State *L, अक्षर *globname) {
  पूर्णांक status;
  अक्षर *suffix = रिक्त;
  अक्षर *modname = म_अक्षर(globname, '=');
  जर (modname == रिक्त) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = म_अक्षर(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  अन्यथा {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  जर (status == LUA_OK) {
    जर (suffix != रिक्त)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  परत report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
स्थिर पूर्णांक pushargs (lua_State *L) {
  पूर्णांक i, n;
  जर (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (पूर्णांक)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  साठी (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  परत n;
}


स्थिर पूर्णांक handle_script (lua_State *L, अक्षर **argv) {
  पूर्णांक status;
  स्थिरांक अक्षर *fname = argv[0];
  जर (म_भेद(fname, "-") == 0 && म_भेद(argv[-1], "--") != 0)
    fname = रिक्त;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  जर (status == LUA_OK) {
    पूर्णांक n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  परत report(L, status);
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
स्थिर पूर्णांक collectargs (अक्षर **argv, पूर्णांक *first) {
  पूर्णांक args = 0;
  पूर्णांक i;
  जर (argv[0] != रिक्त) {  /* is there a program name? */
    जर (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  अन्यथा {  /* no program name */
    *first = -1;
    परत 0;
  }
  साठी (i = 1; argv[i] != रिक्त; i++) {  /* handle arguments */
    *first = i;
    जर (argv[i][0] != '-')  /* not an option? */
        परत args;  /* stop handling options */
    निवड (argv[i][1]) {  /* else check option */
      स्थिती '-':  /* '--' */
        जर (argv[i][2] != '\0')  /* extra characters after '--'? */
          परत has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != रिक्त) ? i + 1 : 0;
        परत args;
      स्थिती '\0':  /* '-' */
        परत args;  /* script "name" is '-' */
      स्थिती 'E':
        जर (argv[i][2] != '\0')  /* extra characters? */
          परत has_error;  /* invalid option */
        args |= has_E;
        थांबा;
      स्थिती 'W':
        जर (argv[i][2] != '\0')  /* extra characters? */
          परत has_error;  /* invalid option */
        थांबा;
      स्थिती 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      स्थिती 'v':
        जर (argv[i][2] != '\0')  /* extra characters? */
          परत has_error;  /* invalid option */
        args |= has_v;
        थांबा;
      स्थिती 'e':
        args |= has_e;  /* FALLTHROUGH */
      स्थिती 'l':  /* both options need an argument */
        जर (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          जर (argv[i] == रिक्त || argv[i][0] == '-')
            परत has_error;  /* no next argument or it is another option */
        }
        थांबा;
      पूर्वनिर्धारित:  /* invalid option */
        परत has_error;
    }
  }
  *first = 0;  /* no script name */
  परत args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
स्थिर पूर्णांक runargs (lua_State *L, अक्षर **argv, पूर्णांक n) {
  पूर्णांक i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  साठी (i = 1; i < n; i++) {
    पूर्णांक option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    निवड (option) {
      स्थिती 'e':  स्थिती 'l': {
        पूर्णांक status;
        अक्षर *extra = argv[i] + 2;  /* both options need an argument */
        जर (*extra == '\0') extra = argv[++i];
        lua_assert(extra != रिक्त);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        जर (status != LUA_OK) परत 0;
        थांबा;
      }
      स्थिती 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        थांबा;
    }
  }
  परत 1;
}


स्थिर अक्षर *(*l_getenv)(स्थिरांक अक्षर *name);

/* Function to ignore environment variables, used by option -E */
स्थिर अक्षर *no_getenv (स्थिरांक अक्षर *name) {
  UNUSED(name);
  परत रिक्त;
}


स्थिर पूर्णांक handle_luainit (lua_State *L) {
  स्थिरांक अक्षर *name = "=" LUA_INITVARVERSION;
  स्थिरांक अक्षर *init = l_getenv(name + 1);
  जर (init == रिक्त) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  जर (init == रिक्त) परत LUA_OK;
  अन्यथा जर (init[0] == '@')
    परत dofile(L, init+1);
  अन्यथा
    परत dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#जर !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#जर !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#जर !defined(lua_stdin_is_tty)	/* { */

#जर defined(LUA_USE_POSIX)	/* { */

#अन्तर्भाव <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#किंवाजर defined(LUA_USE_WINDOWS)	/* }{ */

#अन्तर्भाव <io.h>
#अन्तर्भाव <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(मानक_निवेश))

#अन्यथा				/* }{ */

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

#जर !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#जर defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#अन्तर्भाव <readline/readline.h>
#अन्तर्भाव <readline/history.h>

#define lua_initreadline(L)  \
	((शून्य)L, *cast(स्थिरांक अक्षर **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((शून्य)buff, readline(prompt))
#define lua_saveline(ओळ)	add_history(ओळ)
#define lua_freeline(ओळ)	मुक्त(ओळ)

#अन्यथा		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
प्रकारव्याख्या अक्षर *(*l_readlineT) (स्थिरांक अक्षर *prompt);
स्थिर l_readlineT l_readline = रिक्त;

/* pointer to 'add_history' function (if any) */
प्रकारव्याख्या शून्य (*l_addhistT) (स्थिरांक अक्षर *string);
स्थिर l_addhistT l_addhist = रिक्त;


स्थिर अक्षर *lua_readline (अक्षर *buff, स्थिरांक अक्षर *prompt) {
  जर (l_readline != रिक्त)  /* is there a 'readline'? */
    परत (*l_readline)(prompt);  /* use it */
  अन्यथा {  /* emulate 'readline' over 'buff' */
    ख_माला_दो(prompt, मानक_निकास);
    ख_साफ(मानक_निकास);  /* show prompt */
    परत ख_माला_लो(buff, LUA_MAXINPUT, मानक_निवेश);  /* read line */
  }
}


स्थिर शून्य lua_saveline (स्थिरांक अक्षर *ओळ) {
  जर (l_addhist != रिक्त)  /* is there an 'add_history'? */
    (*l_addhist)(ओळ);  /* use it */
  /* else nothing to be done */
}


स्थिर शून्य lua_freeline (अक्षर *ओळ) {
  जर (l_readline != रिक्त)  /* is there a 'readline'? */
    मुक्त(ओळ);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#जर defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#अन्तर्भाव <dlfcn.h>

स्थिर शून्य lua_initreadline (lua_State *L) {
  स्थिरांक अक्षर *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  शून्य *lib;  /* library handle */
  जर (rllib == रिक्त)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  जर (lib != रिक्त) {
    स्थिरांक अक्षर **name = cast(स्थिरांक अक्षर**, dlsym(lib, "rl_readline_name"));
    जर (name != रिक्त)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    जर (l_readline != रिक्त)  /* could load readline function? */
      परत;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#अन्यथा		/* }{ */
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
स्थिर स्थिरांक अक्षर *get_prompt (lua_State *L, पूर्णांक firstline) {
  जर (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    परत (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  अन्यथा {  /* apply 'tostring' over the value */
    स्थिरांक अक्षर *p = luaL_tolstring(L, -1, रिक्त);
    lua_remove(L, -2);  /* remove original value */
    परत p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(आकार(EOFMARK)/आकार(अक्षर) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
स्थिर पूर्णांक incomplete (lua_State *L, पूर्णांक status) {
  जर (status == LUA_ERRSYNTAX) {
    माप_प्रकार lmsg;
    स्थिरांक अक्षर *संदेश = lua_tolstring(L, -1, &lmsg);
    जर (lmsg >= marklen && म_भेद(संदेश + lmsg - marklen, EOFMARK) == 0)
      परत 1;
  }
  परत 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
स्थिर पूर्णांक pushline (lua_State *L, पूर्णांक firstline) {
  अक्षर buffer[LUA_MAXINPUT];
  माप_प्रकार l;
  स्थिरांक अक्षर *prmt = get_prompt(L, firstline);
  अक्षर *b = lua_readline(buffer, prmt);
  lua_काढा(L, 1);  /* remove prompt */
  जर (b == रिक्त)
    परत 0;  /* no input */
  l = म_माप(b);
  जर (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  परत 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
स्थिर पूर्णांक addreturn (lua_State *L) {
  स्थिरांक अक्षर *ओळ = lua_tostring(L, -1);  /* original line */
  स्थिरांक अक्षर *retline = lua_pushfstring(L, "return %s;", ओळ);
  पूर्णांक status = luaL_loadbufferx(L, retline, म_माप(retline), "=stdin", "t");
  जर (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  अन्यथा
    lua_काढा(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  परत status;
}


स्थिर शून्य checklocal (स्थिरांक अक्षर *ओळ) {
  स्थिर स्थिरांक माप_प्रकार szloc = आकार("local") - 1;
  स्थिर स्थिरांक अक्षर space[] = " \t";
  ओळ += म_अखोज(ओळ, space);  /* skip spaces */
  जर (म_भेदन(ओळ, "local", szloc) == 0 &&  /* "local"? */
      म_अक्षर(space, *(ओळ + szloc)) != रिक्त) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
स्थिर पूर्णांक multiline (lua_State *L) {
  माप_प्रकार len;
  स्थिरांक अक्षर *ओळ = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(ओळ);
  साठी (;;) {  /* repeat until gets a complete statement */
    पूर्णांक status = luaL_loadbufferx(L, ओळ, len, "=stdin", "t");  /* try it */
    जर (!incomplete(L, status) || !pushline(L, 0))
      परत status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_घाला(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    ओळ = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
स्थिर पूर्णांक loadline (lua_State *L) {
  स्थिरांक अक्षर *ओळ;
  पूर्णांक status;
  lua_settop(L, 0);
  जर (!pushline(L, 1))
    परत -1;  /* no input */
  जर ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  ओळ = lua_tostring(L, 1);
  जर (ओळ[0] != '\0')  /* non empty? */
    lua_saveline(ओळ);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  परत status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
स्थिर शून्य l_print (lua_State *L) {
  पूर्णांक n = lua_gettop(L);
  जर (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_घाला(L, 1);
    जर (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
स्थिर शून्य doREPL (lua_State *L) {
  पूर्णांक status;
  स्थिरांक अक्षर *oldprogname = progname;
  progname = रिक्त;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  पर्यंत ((status = loadline(L)) != -1) {
    जर (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    जर (status == LUA_OK) l_print(L);
    अन्यथा report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#जर !defined(luai_openlibs)
#जर defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#अन्यथा
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
स्थिर पूर्णांक pmain (lua_State *L) {
  पूर्णांक argc = (पूर्णांक)lua_tointeger(L, 1);
  अक्षर **argv = (अक्षर **)lua_touserdata(L, 2);
  पूर्णांक script;
  पूर्णांक args = collectargs(argv, &script);
  पूर्णांक optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  जर (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    परत 0;
  }
  जर (args & has_v)  /* option '-v'? */
    print_version();
  जर (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  अन्यथा
    l_getenv = &दो_पर्या;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  जर (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    परत 0;  /* error running LUA_INIT */
  जर (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    परत 0;  /* something failed */
  जर (script > 0) {  /* execute main script (if there is one) */
    जर (handle_script(L, argv + script) != LUA_OK)
      परत 0;  /* interrupt in case of error */
  }
  जर (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  अन्यथा जर (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    जर (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    अन्यथा dofile(L, रिक्त);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  परत 1;
}


पूर्णांक main (पूर्णांक argc, अक्षर **argv) {
  पूर्णांक status, result;
  lua_State *L = luaL_newstate();  /* create state */
  जर (L == रिक्त) {
    l_message(argv[0], "cannot create state: not enough memory");
    परत निकास_त्रुटि;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  परत (result && status == LUA_OK) ? निकास_सफल : निकास_त्रुटि;
}

