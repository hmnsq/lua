/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#شامل "lprefix.h"


#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل <signal.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


#اگر !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#اگر !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#اگر !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ایستا lua_State *globalL = پوچ;

ایستا ثابت نویسه *progname = LUA_PROGNAME;


#اگر defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ایستا تهی setsignal (صحیح sig, تهی (*handler)(صحیح)) {
  ساختار sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, پوچ);
}

#وگرنه           /* }{ */

#define setsignal            سیگنال

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ایستا تهی lstop (lua_State *L, lua_Debug *ar) {
  (تهی)ar;  /* unused arg. */
  lua_sethook(L, پوچ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ایستا تهی laction (صحیح i) {
  صحیح flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ایستا تهی print_usage (ثابت نویسه *badoption) {
  lua_writestringerror("%s: ", progname);
  اگر (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  وگرنه
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
ایستا تهی l_message (ثابت نویسه *pname, ثابت نویسه *پیام) {
  اگر (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", پیام);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ایستا صحیح report (lua_State *L, صحیح status) {
  اگر (status != LUA_OK) {
    ثابت نویسه *پیام = lua_tostring(L, -1);
    اگر (پیام == پوچ)
      پیام = "(error message not a string)";
    l_message(progname, پیام);
    lua_برداشت(L, 1);  /* remove message */
  }
  بازگشت status;
}


/*
** Message handler used to run all chunks
*/
ایستا صحیح msghandler (lua_State *L) {
  ثابت نویسه *پیام = lua_tostring(L, 1);
  اگر (پیام == پوچ) {  /* is error object not a string? */
    اگر (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      بازگشت 1;  /* that is the message */
    وگرنه
      پیام = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, پیام, 1);  /* append a standard traceback */
  بازگشت 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ایستا صحیح docall (lua_State *L, صحیح narg, صحیح nres) {
  صحیح status;
  صحیح base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_درج(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  بازگشت status;
}


ایستا تهی print_version (تهی) {
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
ایستا تهی createargtable (lua_State *L, نویسه **argv, صحیح argc, صحیح script) {
  صحیح i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  برای (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ایستا صحیح dochunk (lua_State *L, صحیح status) {
  اگر (status == LUA_OK) status = docall(L, 0, 0);
  بازگشت report(L, status);
}


ایستا صحیح dofile (lua_State *L, ثابت نویسه *name) {
  بازگشت dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ایستا صحیح dostring (lua_State *L, ثابت نویسه *s, ثابت نویسه *name) {
  بازگشت dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ایستا صحیح dolibrary (lua_State *L, نویسه *globname) {
  صحیح status;
  نویسه *suffix = پوچ;
  نویسه *modname = strchr(globname, '=');
  اگر (modname == پوچ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  وگرنه {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  اگر (status == LUA_OK) {
    اگر (suffix != پوچ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  بازگشت report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ایستا صحیح pushargs (lua_State *L) {
  صحیح i, n;
  اگر (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (صحیح)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  برای (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  بازگشت n;
}


ایستا صحیح handle_script (lua_State *L, نویسه **argv) {
  صحیح status;
  ثابت نویسه *fname = argv[0];
  اگر (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = پوچ;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  اگر (status == LUA_OK) {
    صحیح n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  بازگشت report(L, status);
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
ایستا صحیح collectargs (نویسه **argv, صحیح *first) {
  صحیح args = 0;
  صحیح i;
  اگر (argv[0] != پوچ) {  /* is there a program name? */
    اگر (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  وگرنه {  /* no program name */
    *first = -1;
    بازگشت 0;
  }
  برای (i = 1; argv[i] != پوچ; i++) {  /* handle arguments */
    *first = i;
    اگر (argv[i][0] != '-')  /* not an option? */
        بازگشت args;  /* stop handling options */
    انتخاب (argv[i][1]) {  /* else check option */
      حالت '-':  /* '--' */
        اگر (argv[i][2] != '\0')  /* extra characters after '--'? */
          بازگشت has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != پوچ) ? i + 1 : 0;
        بازگشت args;
      حالت '\0':  /* '-' */
        بازگشت args;  /* script "name" is '-' */
      حالت 'E':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          بازگشت has_error;  /* invalid option */
        args |= has_E;
        بشکن;
      حالت 'W':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          بازگشت has_error;  /* invalid option */
        بشکن;
      حالت 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      حالت 'v':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          بازگشت has_error;  /* invalid option */
        args |= has_v;
        بشکن;
      حالت 'e':
        args |= has_e;  /* FALLTHROUGH */
      حالت 'l':  /* both options need an argument */
        اگر (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          اگر (argv[i] == پوچ || argv[i][0] == '-')
            بازگشت has_error;  /* no next argument or it is another option */
        }
        بشکن;
      پیشفرض:  /* invalid option */
        بازگشت has_error;
    }
  }
  *first = 0;  /* no script name */
  بازگشت args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ایستا صحیح runargs (lua_State *L, نویسه **argv, صحیح n) {
  صحیح i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  برای (i = 1; i < n; i++) {
    صحیح option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    انتخاب (option) {
      حالت 'e':  حالت 'l': {
        صحیح status;
        نویسه *extra = argv[i] + 2;  /* both options need an argument */
        اگر (*extra == '\0') extra = argv[++i];
        lua_assert(extra != پوچ);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        اگر (status != LUA_OK) بازگشت 0;
        بشکن;
      }
      حالت 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        بشکن;
    }
  }
  بازگشت 1;
}


ایستا نویسه *(*l_getenv)(ثابت نویسه *name);

/* Function to ignore environment variables, used by option -E */
ایستا نویسه *no_getenv (ثابت نویسه *name) {
  UNUSED(name);
  بازگشت پوچ;
}


ایستا صحیح handle_luainit (lua_State *L) {
  ثابت نویسه *name = "=" LUA_INITVARVERSION;
  ثابت نویسه *init = l_getenv(name + 1);
  اگر (init == پوچ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  اگر (init == پوچ) بازگشت LUA_OK;
  وگرنه اگر (init[0] == '@')
    بازگشت dofile(L, init+1);
  وگرنه
    بازگشت dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#اگر !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#اگر !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#اگر !defined(lua_stdin_is_tty)	/* { */

#اگر defined(LUA_USE_POSIX)	/* { */

#شامل <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#وگرنهاگر defined(LUA_USE_WINDOWS)	/* }{ */

#شامل <io.h>
#شامل <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#وگرنه				/* }{ */

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

#اگر !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#اگر defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#شامل <readline/readline.h>
#شامل <readline/history.h>

#define lua_initreadline(L)  \
	((تهی)L, *cast(ثابت نویسه **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((تهی)buff, readline(prompt))
#define lua_saveline(سطر)	add_history(سطر)
#define lua_freeline(سطر)	آزاد(سطر)

#وگرنه		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
تعریفنوع نویسه *(*l_readlineT) (ثابت نویسه *prompt);
ایستا l_readlineT l_readline = پوچ;

/* pointer to 'add_history' function (if any) */
تعریفنوع تهی (*l_addhistT) (ثابت نویسه *string);
ایستا l_addhistT l_addhist = پوچ;


ایستا نویسه *lua_readline (نویسه *buff, ثابت نویسه *prompt) {
  اگر (l_readline != پوچ)  /* is there a 'readline'? */
    بازگشت (*l_readline)(prompt);  /* use it */
  وگرنه {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    بازگشت fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ایستا تهی lua_saveline (ثابت نویسه *سطر) {
  اگر (l_addhist != پوچ)  /* is there an 'add_history'? */
    (*l_addhist)(سطر);  /* use it */
  /* else nothing to be done */
}


ایستا تهی lua_freeline (نویسه *سطر) {
  اگر (l_readline != پوچ)  /* is there a 'readline'? */
    آزاد(سطر);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#اگر defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#شامل <dlfcn.h>

ایستا تهی lua_initreadline (lua_State *L) {
  ثابت نویسه *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  تهی *lib;  /* library handle */
  اگر (rllib == پوچ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  اگر (lib != پوچ) {
    ثابت نویسه **name = cast(ثابت نویسه**, dlsym(lib, "rl_readline_name"));
    اگر (name != پوچ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    اگر (l_readline != پوچ)  /* could load readline function? */
      بازگشت;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#وگرنه		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((تهی)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ایستا ثابت نویسه *get_prompt (lua_State *L, صحیح firstline) {
  اگر (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    بازگشت (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  وگرنه {  /* apply 'tostring' over the value */
    ثابت نویسه *p = luaL_tolstring(L, -1, پوچ);
    lua_remove(L, -2);  /* remove original value */
    بازگشت p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(اندازه(EOFMARK)/اندازه(نویسه) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ایستا صحیح incomplete (lua_State *L, صحیح status) {
  اگر (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    ثابت نویسه *پیام = lua_tolstring(L, -1, &lmsg);
    اگر (lmsg >= marklen && strcmp(پیام + lmsg - marklen, EOFMARK) == 0)
      بازگشت 1;
  }
  بازگشت 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ایستا صحیح pushline (lua_State *L, صحیح firstline) {
  نویسه buffer[LUA_MAXINPUT];
  size_t l;
  ثابت نویسه *prmt = get_prompt(L, firstline);
  نویسه *b = lua_readline(buffer, prmt);
  lua_برداشت(L, 1);  /* remove prompt */
  اگر (b == پوچ)
    بازگشت 0;  /* no input */
  l = strlen(b);
  اگر (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  بازگشت 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ایستا صحیح addreturn (lua_State *L) {
  ثابت نویسه *سطر = lua_tostring(L, -1);  /* original line */
  ثابت نویسه *retline = lua_pushfstring(L, "return %s;", سطر);
  صحیح status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  اگر (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  وگرنه
    lua_برداشت(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  بازگشت status;
}


ایستا تهی checklocal (ثابت نویسه *سطر) {
  ایستا ثابت size_t szloc = اندازه("local") - 1;
  ایستا ثابت نویسه space[] = " \t";
  سطر += strspn(سطر, space);  /* skip spaces */
  اگر (strncmp(سطر, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(سطر + szloc)) != پوچ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ایستا صحیح multiline (lua_State *L) {
  size_t len;
  ثابت نویسه *سطر = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(سطر);
  برای (;;) {  /* repeat until gets a complete statement */
    صحیح status = luaL_loadbufferx(L, سطر, len, "=stdin", "t");  /* try it */
    اگر (!incomplete(L, status) || !pushline(L, 0))
      بازگشت status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_درج(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    سطر = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ایستا صحیح loadline (lua_State *L) {
  ثابت نویسه *سطر;
  صحیح status;
  lua_settop(L, 0);
  اگر (!pushline(L, 1))
    بازگشت -1;  /* no input */
  اگر ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  سطر = lua_tostring(L, 1);
  اگر (سطر[0] != '\0')  /* non empty? */
    lua_saveline(سطر);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  بازگشت status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ایستا تهی l_print (lua_State *L) {
  صحیح n = lua_gettop(L);
  اگر (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_درج(L, 1);
    اگر (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ایستا تهی doREPL (lua_State *L) {
  صحیح status;
  ثابت نویسه *oldprogname = progname;
  progname = پوچ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  وقتی ((status = loadline(L)) != -1) {
    اگر (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    اگر (status == LUA_OK) l_print(L);
    وگرنه report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#اگر !defined(luai_openlibs)
#اگر defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#وگرنه
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ایستا صحیح pmain (lua_State *L) {
  صحیح argc = (صحیح)lua_tointeger(L, 1);
  نویسه **argv = (نویسه **)lua_touserdata(L, 2);
  صحیح script;
  صحیح args = collectargs(argv, &script);
  صحیح optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  اگر (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    بازگشت 0;
  }
  اگر (args & has_v)  /* option '-v'? */
    print_version();
  اگر (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  وگرنه
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  اگر (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    بازگشت 0;  /* error running LUA_INIT */
  اگر (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    بازگشت 0;  /* something failed */
  اگر (script > 0) {  /* execute main script (if there is one) */
    اگر (handle_script(L, argv + script) != LUA_OK)
      بازگشت 0;  /* interrupt in case of error */
  }
  اگر (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  وگرنه اگر (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    اگر (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    وگرنه dofile(L, پوچ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  بازگشت 1;
}


صحیح اصلی (صحیح argc, نویسه **argv) {
  صحیح status, result;
  lua_State *L = luaL_newstate();  /* create state */
  اگر (L == پوچ) {
    l_message(argv[0], "cannot create state: not enough memory");
    بازگشت EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  بازگشت (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

