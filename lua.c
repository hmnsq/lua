/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ <signal.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


#எனில் !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#எனில் !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#எனில் !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


நிலையான lua_State *globalL = வெற்று;

நிலையான மாறிலி எழுத்து *progname = LUA_PROGNAME;


#எனில் defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
நிலையான வெறுமை setsignal (முழு sig, வெறுமை (*handler)(முழு)) {
  கட்டமைப்பு sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, வெற்று);
}

#இல்லையெனில்           /* }{ */

#define setsignal            சமிக்ஞை

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
நிலையான வெறுமை lstop (lua_State *L, lua_Debug *ar) {
  (வெறுமை)ar;  /* unused arg. */
  lua_sethook(L, வெற்று, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
நிலையான வெறுமை laction (முழு i) {
  முழு flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


நிலையான வெறுமை print_usage (மாறிலி எழுத்து *badoption) {
  lua_writestringerror("%s: ", progname);
  எனில் (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  இல்லையெனில்
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
நிலையான வெறுமை l_message (மாறிலி எழுத்து *pname, மாறிலி எழுத்து *செய்தி) {
  எனில் (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", செய்தி);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
நிலையான முழு report (lua_State *L, முழு status) {
  எனில் (status != LUA_OK) {
    மாறிலி எழுத்து *செய்தி = lua_tostring(L, -1);
    எனில் (செய்தி == வெற்று)
      செய்தி = "(error message not a string)";
    l_message(progname, செய்தி);
    lua_எடு(L, 1);  /* remove message */
  }
  திரும்பு status;
}


/*
** Message handler used to run all chunks
*/
நிலையான முழு msghandler (lua_State *L) {
  மாறிலி எழுத்து *செய்தி = lua_tostring(L, 1);
  எனில் (செய்தி == வெற்று) {  /* is error object not a string? */
    எனில் (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      திரும்பு 1;  /* that is the message */
    இல்லையெனில்
      செய்தி = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, செய்தி, 1);  /* append a standard traceback */
  திரும்பு 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
நிலையான முழு docall (lua_State *L, முழு narg, முழு nres) {
  முழு status;
  முழு base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_செருகு(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  திரும்பு status;
}


நிலையான வெறுமை print_version (வெறுமை) {
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
நிலையான வெறுமை createargtable (lua_State *L, எழுத்து **argv, முழு argc, முழு script) {
  முழு i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  க்கு (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


நிலையான முழு dochunk (lua_State *L, முழு status) {
  எனில் (status == LUA_OK) status = docall(L, 0, 0);
  திரும்பு report(L, status);
}


நிலையான முழு dofile (lua_State *L, மாறிலி எழுத்து *name) {
  திரும்பு dochunk(L, luaL_loadfilex(L, name, "bt"));
}


நிலையான முழு dostring (lua_State *L, மாறிலி எழுத்து *s, மாறிலி எழுத்து *name) {
  திரும்பு dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
நிலையான முழு dolibrary (lua_State *L, எழுத்து *globname) {
  முழு status;
  எழுத்து *suffix = வெற்று;
  எழுத்து *modname = strchr(globname, '=');
  எனில் (modname == வெற்று) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  இல்லையெனில் {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  எனில் (status == LUA_OK) {
    எனில் (suffix != வெற்று)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  திரும்பு report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
நிலையான முழு pushargs (lua_State *L) {
  முழு i, n;
  எனில் (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (முழு)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  க்கு (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  திரும்பு n;
}


நிலையான முழு handle_script (lua_State *L, எழுத்து **argv) {
  முழு status;
  மாறிலி எழுத்து *fname = argv[0];
  எனில் (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = வெற்று;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  எனில் (status == LUA_OK) {
    முழு n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  திரும்பு report(L, status);
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
நிலையான முழு collectargs (எழுத்து **argv, முழு *first) {
  முழு args = 0;
  முழு i;
  எனில் (argv[0] != வெற்று) {  /* is there a program name? */
    எனில் (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  இல்லையெனில் {  /* no program name */
    *first = -1;
    திரும்பு 0;
  }
  க்கு (i = 1; argv[i] != வெற்று; i++) {  /* handle arguments */
    *first = i;
    எனில் (argv[i][0] != '-')  /* not an option? */
        திரும்பு args;  /* stop handling options */
    தேர்வு (argv[i][1]) {  /* else check option */
      நிலை '-':  /* '--' */
        எனில் (argv[i][2] != '\0')  /* extra characters after '--'? */
          திரும்பு has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != வெற்று) ? i + 1 : 0;
        திரும்பு args;
      நிலை '\0':  /* '-' */
        திரும்பு args;  /* script "name" is '-' */
      நிலை 'E':
        எனில் (argv[i][2] != '\0')  /* extra characters? */
          திரும்பு has_error;  /* invalid option */
        args |= has_E;
        நிறுத்து;
      நிலை 'W':
        எனில் (argv[i][2] != '\0')  /* extra characters? */
          திரும்பு has_error;  /* invalid option */
        நிறுத்து;
      நிலை 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      நிலை 'v':
        எனில் (argv[i][2] != '\0')  /* extra characters? */
          திரும்பு has_error;  /* invalid option */
        args |= has_v;
        நிறுத்து;
      நிலை 'e':
        args |= has_e;  /* FALLTHROUGH */
      நிலை 'l':  /* both options need an argument */
        எனில் (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          எனில் (argv[i] == வெற்று || argv[i][0] == '-')
            திரும்பு has_error;  /* no next argument or it is another option */
        }
        நிறுத்து;
      இயல்பு:  /* invalid option */
        திரும்பு has_error;
    }
  }
  *first = 0;  /* no script name */
  திரும்பு args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
நிலையான முழு runargs (lua_State *L, எழுத்து **argv, முழு n) {
  முழு i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  க்கு (i = 1; i < n; i++) {
    முழு option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    தேர்வு (option) {
      நிலை 'e':  நிலை 'l': {
        முழு status;
        எழுத்து *extra = argv[i] + 2;  /* both options need an argument */
        எனில் (*extra == '\0') extra = argv[++i];
        lua_assert(extra != வெற்று);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        எனில் (status != LUA_OK) திரும்பு 0;
        நிறுத்து;
      }
      நிலை 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        நிறுத்து;
    }
  }
  திரும்பு 1;
}


நிலையான எழுத்து *(*l_getenv)(மாறிலி எழுத்து *name);

/* Function to ignore environment variables, used by option -E */
நிலையான எழுத்து *no_getenv (மாறிலி எழுத்து *name) {
  UNUSED(name);
  திரும்பு வெற்று;
}


நிலையான முழு handle_luainit (lua_State *L) {
  மாறிலி எழுத்து *name = "=" LUA_INITVARVERSION;
  மாறிலி எழுத்து *init = l_getenv(name + 1);
  எனில் (init == வெற்று) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  எனில் (init == வெற்று) திரும்பு LUA_OK;
  இல்லையெனில் எனில் (init[0] == '@')
    திரும்பு dofile(L, init+1);
  இல்லையெனில்
    திரும்பு dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#எனில் !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#எனில் !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#எனில் !defined(lua_stdin_is_tty)	/* { */

#எனில் defined(LUA_USE_POSIX)	/* { */

#அந்தர்பாவ <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#அல்லதெனில் defined(LUA_USE_WINDOWS)	/* }{ */

#அந்தர்பாவ <io.h>
#அந்தர்பாவ <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#இல்லையெனில்				/* }{ */

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

#எனில் !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#எனில் defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#அந்தர்பாவ <readline/readline.h>
#அந்தர்பாவ <readline/history.h>

#define lua_initreadline(L)  \
	((வெறுமை)L, *cast(மாறிலி எழுத்து **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((வெறுமை)buff, readline(prompt))
#define lua_saveline(வரி)	add_history(வரி)
#define lua_freeline(வரி)	விடுவி(வரி)

#இல்லையெனில்		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
வகைவரை எழுத்து *(*l_readlineT) (மாறிலி எழுத்து *prompt);
நிலையான l_readlineT l_readline = வெற்று;

/* pointer to 'add_history' function (if any) */
வகைவரை வெறுமை (*l_addhistT) (மாறிலி எழுத்து *string);
நிலையான l_addhistT l_addhist = வெற்று;


நிலையான எழுத்து *lua_readline (எழுத்து *buff, மாறிலி எழுத்து *prompt) {
  எனில் (l_readline != வெற்று)  /* is there a 'readline'? */
    திரும்பு (*l_readline)(prompt);  /* use it */
  இல்லையெனில் {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    திரும்பு fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


நிலையான வெறுமை lua_saveline (மாறிலி எழுத்து *வரி) {
  எனில் (l_addhist != வெற்று)  /* is there an 'add_history'? */
    (*l_addhist)(வரி);  /* use it */
  /* else nothing to be done */
}


நிலையான வெறுமை lua_freeline (எழுத்து *வரி) {
  எனில் (l_readline != வெற்று)  /* is there a 'readline'? */
    விடுவி(வரி);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#எனில் defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#அந்தர்பாவ <dlfcn.h>

நிலையான வெறுமை lua_initreadline (lua_State *L) {
  மாறிலி எழுத்து *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  வெறுமை *lib;  /* library handle */
  எனில் (rllib == வெற்று)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  எனில் (lib != வெற்று) {
    மாறிலி எழுத்து **name = cast(மாறிலி எழுத்து**, dlsym(lib, "rl_readline_name"));
    எனில் (name != வெற்று)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    எனில் (l_readline != வெற்று)  /* could load readline function? */
      திரும்பு;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#இல்லையெனில்		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((வெறுமை)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
நிலையான மாறிலி எழுத்து *get_prompt (lua_State *L, முழு firstline) {
  எனில் (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    திரும்பு (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  இல்லையெனில் {  /* apply 'tostring' over the value */
    மாறிலி எழுத்து *p = luaL_tolstring(L, -1, வெற்று);
    lua_remove(L, -2);  /* remove original value */
    திரும்பு p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(அளவு(EOFMARK)/அளவு(எழுத்து) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
நிலையான முழு incomplete (lua_State *L, முழு status) {
  எனில் (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    மாறிலி எழுத்து *செய்தி = lua_tolstring(L, -1, &lmsg);
    எனில் (lmsg >= marklen && strcmp(செய்தி + lmsg - marklen, EOFMARK) == 0)
      திரும்பு 1;
  }
  திரும்பு 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
நிலையான முழு pushline (lua_State *L, முழு firstline) {
  எழுத்து buffer[LUA_MAXINPUT];
  size_t l;
  மாறிலி எழுத்து *prmt = get_prompt(L, firstline);
  எழுத்து *b = lua_readline(buffer, prmt);
  lua_எடு(L, 1);  /* remove prompt */
  எனில் (b == வெற்று)
    திரும்பு 0;  /* no input */
  l = strlen(b);
  எனில் (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  திரும்பு 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
நிலையான முழு addreturn (lua_State *L) {
  மாறிலி எழுத்து *வரி = lua_tostring(L, -1);  /* original line */
  மாறிலி எழுத்து *retline = lua_pushfstring(L, "return %s;", வரி);
  முழு status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  எனில் (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  இல்லையெனில்
    lua_எடு(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  திரும்பு status;
}


நிலையான வெறுமை checklocal (மாறிலி எழுத்து *வரி) {
  நிலையான மாறிலி size_t szloc = அளவு("local") - 1;
  நிலையான மாறிலி எழுத்து space[] = " \t";
  வரி += strspn(வரி, space);  /* skip spaces */
  எனில் (strncmp(வரி, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(வரி + szloc)) != வெற்று) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
நிலையான முழு multiline (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *வரி = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(வரி);
  க்கு (;;) {  /* repeat until gets a complete statement */
    முழு status = luaL_loadbufferx(L, வரி, len, "=stdin", "t");  /* try it */
    எனில் (!incomplete(L, status) || !pushline(L, 0))
      திரும்பு status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_செருகு(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    வரி = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
நிலையான முழு loadline (lua_State *L) {
  மாறிலி எழுத்து *வரி;
  முழு status;
  lua_settop(L, 0);
  எனில் (!pushline(L, 1))
    திரும்பு -1;  /* no input */
  எனில் ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  வரி = lua_tostring(L, 1);
  எனில் (வரி[0] != '\0')  /* non empty? */
    lua_saveline(வரி);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  திரும்பு status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
நிலையான வெறுமை l_print (lua_State *L) {
  முழு n = lua_gettop(L);
  எனில் (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_செருகு(L, 1);
    எனில் (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
நிலையான வெறுமை doREPL (lua_State *L) {
  முழு status;
  மாறிலி எழுத்து *oldprogname = progname;
  progname = வெற்று;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  போது ((status = loadline(L)) != -1) {
    எனில் (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    எனில் (status == LUA_OK) l_print(L);
    இல்லையெனில் report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#எனில் !defined(luai_openlibs)
#எனில் defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#இல்லையெனில்
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
நிலையான முழு pmain (lua_State *L) {
  முழு argc = (முழு)lua_tointeger(L, 1);
  எழுத்து **argv = (எழுத்து **)lua_touserdata(L, 2);
  முழு script;
  முழு args = collectargs(argv, &script);
  முழு optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  எனில் (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    திரும்பு 0;
  }
  எனில் (args & has_v)  /* option '-v'? */
    print_version();
  எனில் (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  இல்லையெனில்
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  எனில் (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    திரும்பு 0;  /* error running LUA_INIT */
  எனில் (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    திரும்பு 0;  /* something failed */
  எனில் (script > 0) {  /* execute main script (if there is one) */
    எனில் (handle_script(L, argv + script) != LUA_OK)
      திரும்பு 0;  /* interrupt in case of error */
  }
  எனில் (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  இல்லையெனில் எனில் (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    எனில் (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    இல்லையெனில் dofile(L, வெற்று);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  திரும்பு 1;
}


முழு முதன்மை (முழு argc, எழுத்து **argv) {
  முழு status, result;
  lua_State *L = luaL_newstate();  /* create state */
  எனில் (L == வெற்று) {
    l_message(argv[0], "cannot create state: not enough memory");
    திரும்பு EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  திரும்பு (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

