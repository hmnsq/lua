/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#تعریف_کرو lua_c

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
#تعریف_کرو LUA_PROGNAME		"lua"
#اگر_ختم

#اگر !defined(LUA_INIT_VAR)
#تعریف_کرو LUA_INIT_VAR		"LUA_INIT"
#اگر_ختم

/* Name of the environment variable with the name of the readline library */
#اگر !defined(LUA_RLLIB_VAR)
#تعریف_کرو LUA_RLLIB_VAR		"LUA_READLINELIB"
#اگر_ختم


#تعریف_کرو LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ساکن lua_State *globalL = تہی;

ساکن ثابت حرف *progname = LUA_PROGNAME;


#اگر defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ساکن خالی setsignal (صحیح sig, خالی (*handler)(صحیح)) {
  ساخت sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, تہی);
}

#ورنہ           /* }{ */

#تعریف_کرو setsignal            اشارہ

#اگر_ختم                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ساکن خالی lstop (lua_State *L, lua_Debug *ar) {
  (خالی)ar;  /* unused arg. */
  lua_sethook(L, تہی, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ساکن خالی laction (صحیح i) {
  صحیح flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ساکن خالی print_usage (ثابت حرف *badoption) {
  lua_writestringerror("%s: ", progname);
  اگر (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  ورنہ
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
ساکن خالی l_پیغام (ثابت حرف *pname, ثابت حرف *پیغام) {
  اگر (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", پیغام);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ساکن صحیح report (lua_State *L, صحیح حالت_2) {
  اگر (حالت_2 != LUA_OK) {
    ثابت حرف *پیغام = lua_tostring(L, -1);
    اگر (پیغام == تہی)
      پیغام = "(error message not a string)";
    l_پیغام(progname, پیغام);
    lua_نکالو(L, 1);  /* remove message */
  }
  واپس حالت_2;
}


/*
** Message handler used to run all chunks
*/
ساکن صحیح msghandler (lua_State *L) {
  ثابت حرف *پیغام = lua_tostring(L, 1);
  اگر (پیغام == تہی) {  /* is error object not a string? */
    اگر (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      واپس 1;  /* that is the message */
    ورنہ
      پیغام = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, پیغام, 1);  /* append a standard traceback */
  واپس 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ساکن صحیح docall (lua_State *L, صحیح narg, صحیح nres) {
  صحیح حالت_2;
  صحیح بنیاد = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_داخل_کرو(L, بنیاد);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  حالت_2 = lua_pcall(L, narg, nres, بنیاد);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, بنیاد);  /* remove message handler from the stack */
  واپس حالت_2;
}


ساکن خالی print_نسخہ (خالی) {
  lua_writestring(LUA_COPYRIGHT, لڑی_لمبائی(LUA_COPYRIGHT));
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
ساکن خالی createargtable (lua_State *L, حرف **argv, صحیح argc, صحیح script) {
  صحیح i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  برائے (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ساکن صحیح dochunk (lua_State *L, صحیح حالت_2) {
  اگر (حالت_2 == LUA_OK) حالت_2 = docall(L, 0, 0);
  واپس report(L, حالت_2);
}


ساکن صحیح dofile (lua_State *L, ثابت حرف *name) {
  واپس dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ساکن صحیح dostring (lua_State *L, ثابت حرف *s, ثابت حرف *name) {
  واپس dochunk(L, luaL_loadbufferx(L, s, لڑی_لمبائی(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ساکن صحیح dolibrary (lua_State *L, حرف *globname) {
  صحیح حالت_2;
  حرف *suffix = تہی;
  حرف *modname = strchr(globname, '=');
  اگر (modname == تہی) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  ورنہ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  حالت_2 = docall(L, 1, 1);  /* call 'require(modname)' */
  اگر (حالت_2 == LUA_OK) {
    اگر (suffix != تہی)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  واپس report(L, حالت_2);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ساکن صحیح pushargs (lua_State *L) {
  صحیح i, n;
  اگر (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (صحیح)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  برائے (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  واپس n;
}


ساکن صحیح handle_script (lua_State *L, حرف **argv) {
  صحیح حالت_2;
  ثابت حرف *fname = argv[0];
  اگر (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = تہی;  /* stdin */
  حالت_2 = luaL_loadfilex(L, fname, "bt");
  اگر (حالت_2 == LUA_OK) {
    صحیح n = pushargs(L);  /* push arguments to script */
    حالت_2 = docall(L, n, LUA_MULTRET);
  }
  واپس report(L, حالت_2);
}


/* bits of various argument indicators in 'args' */
#تعریف_کرو has_error	1	/* bad option */
#تعریف_کرو has_i		2	/* -i */
#تعریف_کرو has_v		4	/* -v */
#تعریف_کرو has_e		8	/* -e */
#تعریف_کرو has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
ساکن صحیح collectargs (حرف **argv, صحیح *پہلا) {
  صحیح دلائل = 0;
  صحیح i;
  اگر (argv[0] != تہی) {  /* is there a program name? */
    اگر (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  ورنہ {  /* no program name */
    *پہلا = -1;
    واپس 0;
  }
  برائے (i = 1; argv[i] != تہی; i++) {  /* handle arguments */
    *پہلا = i;
    اگر (argv[i][0] != '-')  /* not an option? */
        واپس دلائل;  /* stop handling options */
    انتخاب (argv[i][1]) {  /* else check option */
      حالت '-':  /* '--' */
        اگر (argv[i][2] != '\0')  /* extra characters after '--'? */
          واپس has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *پہلا = (argv[i + 1] != تہی) ? i + 1 : 0;
        واپس دلائل;
      حالت '\0':  /* '-' */
        واپس دلائل;  /* script "name" is '-' */
      حالت 'E':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          واپس has_error;  /* invalid option */
        دلائل |= has_E;
        توڑو;
      حالت 'W':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          واپس has_error;  /* invalid option */
        توڑو;
      حالت 'i':
        دلائل |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      حالت 'v':
        اگر (argv[i][2] != '\0')  /* extra characters? */
          واپس has_error;  /* invalid option */
        دلائل |= has_v;
        توڑو;
      حالت 'e':
        دلائل |= has_e;  /* FALLTHROUGH */
      حالت 'l':  /* both options need an argument */
        اگر (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          اگر (argv[i] == تہی || argv[i][0] == '-')
            واپس has_error;  /* no next argument or it is another option */
        }
        توڑو;
      طےشدہ:  /* invalid option */
        واپس has_error;
    }
  }
  *پہلا = 0;  /* no script name */
  واپس دلائل;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ساکن صحیح runargs (lua_State *L, حرف **argv, صحیح n) {
  صحیح i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  برائے (i = 1; i < n; i++) {
    صحیح option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    انتخاب (option) {
      حالت 'e':  حالت 'l': {
        صحیح حالت_2;
        حرف *extra = argv[i] + 2;  /* both options need an argument */
        اگر (*extra == '\0') extra = argv[++i];
        lua_assert(extra != تہی);
        حالت_2 = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        اگر (حالت_2 != LUA_OK) واپس 0;
        توڑو;
      }
      حالت 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        توڑو;
    }
  }
  واپس 1;
}


ساکن حرف *(*l_getenv)(ثابت حرف *name);

/* Function to ignore environment variables, used by option -E */
ساکن حرف *no_getenv (ثابت حرف *name) {
  غیر_مستعمل(name);
  واپس تہی;
}


ساکن صحیح handle_luainit (lua_State *L) {
  ثابت حرف *name = "=" LUA_INITVARVERSION;
  ثابت حرف *init = l_getenv(name + 1);
  اگر (init == تہی) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  اگر (init == تہی) واپس LUA_OK;
  ورنہ اگر (init[0] == '@')
    واپس dofile(L, init+1);
  ورنہ
    واپس dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#اگر !defined(LUA_PROMPT)
#تعریف_کرو LUA_PROMPT		"> "
#تعریف_کرو LUA_PROMPT2		">> "
#اگر_ختم

#اگر !defined(LUA_MAXINPUT)
#تعریف_کرو LUA_MAXINPUT		512
#اگر_ختم


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#اگر !defined(lua_stdin_is_tty)	/* { */

#اگر defined(LUA_USE_POSIX)	/* { */

#شامل <unistd.h>
#تعریف_کرو lua_stdin_is_tty()	isatty(0)

#وگرنہ_اگر defined(LUA_USE_WINDOWS)	/* }{ */

#شامل <io.h>
#شامل <windows.h>

#تعریف_کرو lua_stdin_is_tty()	_isatty(_fileno(stdin))

#ورنہ				/* }{ */

/* ISO C definition */
#تعریف_کرو lua_stdin_is_tty()	1  /* assume stdin is a tty */

#اگر_ختم				/* } */

#اگر_ختم				/* } */


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

#تعریف_کرو lua_initreadline(L)  \
	((خالی)L, *cast(ثابت حرف **, &rl_readline_name)="lua")
#تعریف_کرو lua_readline(buff,prompt)	((خالی)buff, readline(prompt))
#تعریف_کرو lua_saveline(سطر)	جوڑو_history(سطر)
#تعریف_کرو lua_freeline(سطر)	آزاد(سطر)

#ورنہ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
قسمتعریف حرف *(*l_readlineT) (ثابت حرف *prompt);
ساکن l_readlineT l_readline = تہی;

/* pointer to 'add_history' function (if any) */
قسمتعریف خالی (*l_addhistT) (ثابت حرف *string);
ساکن l_addhistT l_addhist = تہی;


ساکن حرف *lua_readline (حرف *buff, ثابت حرف *prompt) {
  اگر (l_readline != تہی)  /* is there a 'readline'? */
    واپس (*l_readline)(prompt);  /* use it */
  ورنہ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    واپس fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ساکن خالی lua_saveline (ثابت حرف *سطر) {
  اگر (l_addhist != تہی)  /* is there an 'add_history'? */
    (*l_addhist)(سطر);  /* use it */
  /* else nothing to be done */
}


ساکن خالی lua_freeline (حرف *سطر) {
  اگر (l_readline != تہی)  /* is there a 'readline'? */
    آزاد(سطر);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#اگر defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#شامل <dlfcn.h>

ساکن خالی lua_initreadline (lua_State *L) {
  ثابت حرف *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  خالی *lib;  /* library handle */
  اگر (rllib == تہی)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  اگر (lib != تہی) {
    ثابت حرف **name = cast(ثابت حرف**, dlsym(lib, "rl_readline_name"));
    اگر (name != تہی)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    اگر (l_readline != تہی)  /* could load readline function? */
      واپس;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#ورنہ		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#تعریف_کرو lua_initreadline(L)	((خالی)L)

#اگر_ختم		/* } */

#اگر_ختم				/* } */

#اگر_ختم				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ساکن ثابت حرف *get_prompt (lua_State *L, صحیح firstline) {
  اگر (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    واپس (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  ورنہ {  /* apply 'tostring' over the value */
    ثابت حرف *p = luaL_tolstring(L, -1, تہی);
    lua_remove(L, -2);  /* remove original value */
    واپس p;
  }
}

/* mark in error messages for incomplete statements */
#تعریف_کرو EOFMARK		"<eof>"
#تعریف_کرو marklen		(حجم(EOFMARK)/حجم(حرف) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ساکن صحیح incomplete (lua_State *L, صحیح حالت_2) {
  اگر (حالت_2 == LUA_ERRSYNTAX) {
    size_t lmsg;
    ثابت حرف *پیغام = lua_tolstring(L, -1, &lmsg);
    اگر (lmsg >= marklen && strcmp(پیغام + lmsg - marklen, EOFMARK) == 0)
      واپس 1;
  }
  واپس 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ساکن صحیح pushline (lua_State *L, صحیح firstline) {
  حرف بفر[LUA_MAXINPUT];
  size_t l;
  ثابت حرف *prmt = get_prompt(L, firstline);
  حرف *b = lua_readline(بفر, prmt);
  lua_نکالو(L, 1);  /* remove prompt */
  اگر (b == تہی)
    واپس 0;  /* no input */
  l = لڑی_لمبائی(b);
  اگر (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  واپس 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ساکن صحیح addreturn (lua_State *L) {
  ثابت حرف *سطر = lua_tostring(L, -1);  /* original line */
  ثابت حرف *retline = lua_pushfstring(L, "return %s;", سطر);
  صحیح حالت_2 = luaL_loadbufferx(L, retline, لڑی_لمبائی(retline), "=stdin", "t");
  اگر (حالت_2 == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  ورنہ
    lua_نکالو(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  واپس حالت_2;
}


ساکن خالی checklocal (ثابت حرف *سطر) {
  ساکن ثابت size_t szloc = حجم("local") - 1;
  ساکن ثابت حرف space[] = " \t";
  سطر += strspn(سطر, space);  /* skip spaces */
  اگر (لڑی_موازنہ(سطر, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(سطر + szloc)) != تہی) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ساکن صحیح multiline (lua_State *L) {
  size_t len;
  ثابت حرف *سطر = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(سطر);
  برائے (;;) {  /* repeat until gets a complete statement */
    صحیح حالت_2 = luaL_loadbufferx(L, سطر, len, "=stdin", "t");  /* try it */
    اگر (!incomplete(L, حالت_2) || !pushline(L, 0))
      واپس حالت_2;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_داخل_کرو(L, -2);  /* ...between the two lines */
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
ساکن صحیح loadline (lua_State *L) {
  ثابت حرف *سطر;
  صحیح حالت_2;
  lua_settop(L, 0);
  اگر (!pushline(L, 1))
    واپس -1;  /* no input */
  اگر ((حالت_2 = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    حالت_2 = multiline(L);  /* try as command, maybe with continuation lines */
  سطر = lua_tostring(L, 1);
  اگر (سطر[0] != '\0')  /* non empty? */
    lua_saveline(سطر);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  واپس حالت_2;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ساکن خالی l_print (lua_State *L) {
  صحیح n = lua_gettop(L);
  اگر (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_داخل_کرو(L, 1);
    اگر (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_پیغام(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ساکن خالی doREPL (lua_State *L) {
  صحیح حالت_2;
  ثابت حرف *oldprogname = progname;
  progname = تہی;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  جب ((حالت_2 = loadline(L)) != -1) {
    اگر (حالت_2 == LUA_OK)
      حالت_2 = docall(L, 0, LUA_MULTRET);
    اگر (حالت_2 == LUA_OK) l_print(L);
    ورنہ report(L, حالت_2);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#اگر !defined(luai_openlibs)
#اگر defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#تعریف_کرو luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#ورنہ
/* The default is to open all standard libraries */
#تعریف_کرو luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#اگر_ختم
#اگر_ختم


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ساکن صحیح pmain (lua_State *L) {
  صحیح argc = (صحیح)lua_tointeger(L, 1);
  حرف **argv = (حرف **)lua_touserdata(L, 2);
  صحیح script;
  صحیح دلائل = collectargs(argv, &script);
  صحیح optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  اگر (دلائل == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    واپس 0;
  }
  اگر (دلائل & has_v)  /* option '-v'? */
    print_نسخہ();
  اگر (دلائل & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  ورنہ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  اگر (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    واپس 0;  /* error running LUA_INIT */
  اگر (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    واپس 0;  /* something failed */
  اگر (script > 0) {  /* execute main script (if there is one) */
    اگر (handle_script(L, argv + script) != LUA_OK)
      واپس 0;  /* interrupt in case of error */
  }
  اگر (دلائل & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  ورنہ اگر (script < 1 && !(دلائل & (has_e | has_v))) { /* no active option? */
    اگر (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_نسخہ();
      doREPL(L);  /* do read-eval-print loop */
    }
    ورنہ dofile(L, تہی);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  واپس 1;
}


صحیح مین (صحیح argc, حرف **argv) {
  صحیح حالت_2, نتیجہ;
  lua_State *L = luaL_newstate();  /* create state */
  اگر (L == تہی) {
    l_پیغام(argv[0], "cannot create state: not enough memory");
    واپس EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  حالت_2 = lua_pcall(L, 2, 1, 0);  /* do the call */
  نتیجہ = lua_toboolean(L, -1);  /* get result */
  report(L, حالت_2);
  lua_close(L);
  واپس (نتیجہ && حالت_2 == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

