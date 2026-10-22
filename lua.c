/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#ضمن "lprefix.h"


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن <signal.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


#אם !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#אם !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#אם !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ساكن lua_State *globalL = عدم;

ساكن ثابت حرف *progname = LUA_PROGNAME;


#אם defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ساكن فراغ setsignal (صحيح sig, فراغ (*handler)(صحيح)) {
  بنية sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, عدم);
}

#אחרת           /* }{ */

#define setsignal            إشارة

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ساكن فراغ lstop (lua_State *L, lua_Debug *ar) {
  (فراغ)ar;  /* unused arg. */
  lua_sethook(L, عدم, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ساكن فراغ laction (صحيح i) {
  صحيح flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ساكن فراغ print_usage (ثابت حرف *badoption) {
  lua_writestringerror("%s: ", progname);
  אם (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  אחרת
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
ساكن فراغ l_message (ثابت حرف *pname, ثابت حرف *msg) {
  אם (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ساكن صحيح report (lua_State *L, صحيح status) {
  אם (status != LUA_OK) {
    ثابت حرف *msg = lua_tostring(L, -1);
    אם (msg == عدم)
      msg = "(error message not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  החזר status;
}


/*
** Message handler used to run all chunks
*/
ساكن صحيح msghandler (lua_State *L) {
  ثابت حرف *msg = lua_tostring(L, 1);
  אם (msg == عدم) {  /* is error object not a string? */
    אם (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      החזר 1;  /* that is the message */
    אחרת
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  החזר 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ساكن صحيح docall (lua_State *L, صحيح narg, صحيح nres) {
  صحيح status;
  صحيح base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  החזר status;
}


ساكن فراغ print_version (فراغ) {
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
ساكن فراغ createargtable (lua_State *L, حرف **argv, صحيح argc, صحيح script) {
  صحيح i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  עבור (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ساكن صحيح dochunk (lua_State *L, صحيح status) {
  אם (status == LUA_OK) status = docall(L, 0, 0);
  החזר report(L, status);
}


ساكن صحيح dofile (lua_State *L, ثابت حرف *name) {
  החזר dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ساكن صحيح dostring (lua_State *L, ثابت حرف *s, ثابت حرف *name) {
  החזר dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ساكن صحيح dolibrary (lua_State *L, حرف *globname) {
  صحيح status;
  حرف *suffix = عدم;
  حرف *modname = strchr(globname, '=');
  אם (modname == عدم) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  אחרת {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  אם (status == LUA_OK) {
    אם (suffix != عدم)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  החזר report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ساكن صحيح pushargs (lua_State *L) {
  صحيح i, n;
  אם (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (صحيح)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  עבור (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  החזר n;
}


ساكن صحيح handle_script (lua_State *L, حرف **argv) {
  صحيح status;
  ثابت حرف *fname = argv[0];
  אם (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = عدم;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  אם (status == LUA_OK) {
    صحيح n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  החזר report(L, status);
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
ساكن صحيح collectargs (حرف **argv, صحيح *first) {
  صحيح args = 0;
  صحيح i;
  אם (argv[0] != عدم) {  /* is there a program name? */
    אם (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  אחרת {  /* no program name */
    *first = -1;
    החזר 0;
  }
  עבור (i = 1; argv[i] != عدم; i++) {  /* handle arguments */
    *first = i;
    אם (argv[i][0] != '-')  /* not an option? */
        החזר args;  /* stop handling options */
    بدل (argv[i][1]) {  /* else check option */
      حالة '-':  /* '--' */
        אם (argv[i][2] != '\0')  /* extra characters after '--'? */
          החזר has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != عدم) ? i + 1 : 0;
        החזר args;
      حالة '\0':  /* '-' */
        החזר args;  /* script "name" is '-' */
      حالة 'E':
        אם (argv[i][2] != '\0')  /* extra characters? */
          החזר has_error;  /* invalid option */
        args |= has_E;
        اكسر;
      حالة 'W':
        אם (argv[i][2] != '\0')  /* extra characters? */
          החזר has_error;  /* invalid option */
        اكسر;
      حالة 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      حالة 'v':
        אם (argv[i][2] != '\0')  /* extra characters? */
          החזר has_error;  /* invalid option */
        args |= has_v;
        اكسر;
      حالة 'e':
        args |= has_e;  /* FALLTHROUGH */
      حالة 'l':  /* both options need an argument */
        אם (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          אם (argv[i] == عدم || argv[i][0] == '-')
            החזר has_error;  /* no next argument or it is another option */
        }
        اكسر;
      افتراضي:  /* invalid option */
        החזר has_error;
    }
  }
  *first = 0;  /* no script name */
  החזר args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ساكن صحيح runargs (lua_State *L, حرف **argv, صحيح n) {
  صحيح i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  עבור (i = 1; i < n; i++) {
    صحيح option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    بدل (option) {
      حالة 'e':  حالة 'l': {
        صحيح status;
        حرف *extra = argv[i] + 2;  /* both options need an argument */
        אם (*extra == '\0') extra = argv[++i];
        lua_assert(extra != عدم);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        אם (status != LUA_OK) החזר 0;
        اكسر;
      }
      حالة 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        اكسر;
    }
  }
  החזר 1;
}


ساكن حرف *(*l_getenv)(ثابت حرف *name);

/* Function to ignore environment variables, used by option -E */
ساكن حرف *no_getenv (ثابت حرف *name) {
  UNUSED(name);
  החזר عدم;
}


ساكن صحيح handle_luainit (lua_State *L) {
  ثابت حرف *name = "=" LUA_INITVARVERSION;
  ثابت حرف *init = l_getenv(name + 1);
  אם (init == عدم) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  אם (init == عدم) החזר LUA_OK;
  אחרת אם (init[0] == '@')
    החזר dofile(L, init+1);
  אחרת
    החזר dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#אם !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#אם !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#אם !defined(lua_stdin_is_tty)	/* { */

#אם defined(LUA_USE_POSIX)	/* { */

#ضمن <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#وإلاإذا defined(LUA_USE_WINDOWS)	/* }{ */

#ضمن <io.h>
#ضمن <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#אחרת				/* }{ */

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

#אם !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#אם defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ضمن <readline/readline.h>
#ضمن <readline/history.h>

#define lua_initreadline(L)  \
	((فراغ)L, *cast(ثابت حرف **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((فراغ)buff, readline(prompt))
#define lua_saveline(line)	add_history(line)
#define lua_freeline(line)	حرر(line)

#אחרת		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
تعريفنوع حرف *(*l_readlineT) (ثابت حرف *prompt);
ساكن l_readlineT l_readline = عدم;

/* pointer to 'add_history' function (if any) */
تعريفنوع فراغ (*l_addhistT) (ثابت حرف *string);
ساكن l_addhistT l_addhist = عدم;


ساكن حرف *lua_readline (حرف *buff, ثابت حرف *prompt) {
  אם (l_readline != عدم)  /* is there a 'readline'? */
    החזר (*l_readline)(prompt);  /* use it */
  אחרת {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    החזר fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ساكن فراغ lua_saveline (ثابت حرف *line) {
  אם (l_addhist != عدم)  /* is there an 'add_history'? */
    (*l_addhist)(line);  /* use it */
  /* else nothing to be done */
}


ساكن فراغ lua_freeline (حرف *line) {
  אם (l_readline != عدم)  /* is there a 'readline'? */
    حرر(line);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#אם defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ضمن <dlfcn.h>

ساكن فراغ lua_initreadline (lua_State *L) {
  ثابت حرف *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  فراغ *lib;  /* library handle */
  אם (rllib == عدم)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  אם (lib != عدم) {
    ثابت حرف **name = cast(ثابت حرف**, dlsym(lib, "rl_readline_name"));
    אם (name != عدم)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    אם (l_readline != عدم)  /* could load readline function? */
      החזר;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#אחרת		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((فراغ)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ساكن ثابت حرف *get_prompt (lua_State *L, صحيح firstline) {
  אם (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    החזר (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  אחרת {  /* apply 'tostring' over the value */
    ثابت حرف *p = luaL_tolstring(L, -1, عدم);
    lua_remove(L, -2);  /* remove original value */
    החזר p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(حجم(EOFMARK)/حجم(حرف) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ساكن صحيح incomplete (lua_State *L, صحيح status) {
  אם (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    ثابت حرف *msg = lua_tolstring(L, -1, &lmsg);
    אם (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
      החזר 1;
  }
  החזר 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ساكن صحيح pushline (lua_State *L, صحيح firstline) {
  حرف buffer[LUA_MAXINPUT];
  size_t l;
  ثابت حرف *prmt = get_prompt(L, firstline);
  حرف *b = lua_readline(buffer, prmt);
  lua_pop(L, 1);  /* remove prompt */
  אם (b == عدم)
    החזר 0;  /* no input */
  l = strlen(b);
  אם (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  החזר 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ساكن صحيح addreturn (lua_State *L) {
  ثابت حرف *line = lua_tostring(L, -1);  /* original line */
  ثابت حرف *retline = lua_pushfstring(L, "return %s;", line);
  صحيح status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  אם (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  אחרת
    lua_pop(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  החזר status;
}


ساكن فراغ checklocal (ثابت حرف *line) {
  ساكن ثابت size_t szloc = حجم("local") - 1;
  ساكن ثابت حرف space[] = " \t";
  line += strspn(line, space);  /* skip spaces */
  אם (strncmp(line, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(line + szloc)) != عدم) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ساكن صحيح multiline (lua_State *L) {
  size_t len;
  ثابت حرف *line = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(line);
  עבור (;;) {  /* repeat until gets a complete statement */
    صحيح status = luaL_loadbufferx(L, line, len, "=stdin", "t");  /* try it */
    אם (!incomplete(L, status) || !pushline(L, 0))
      החזר status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    line = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ساكن صحيح loadline (lua_State *L) {
  ثابت حرف *line;
  صحيح status;
  lua_settop(L, 0);
  אם (!pushline(L, 1))
    החזר -1;  /* no input */
  אם ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  line = lua_tostring(L, 1);
  אם (line[0] != '\0')  /* non empty? */
    lua_saveline(line);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  החזר status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ساكن فراغ l_print (lua_State *L) {
  صحيح n = lua_gettop(L);
  אם (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    אם (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ساكن فراغ doREPL (lua_State *L) {
  صحيح status;
  ثابت حرف *oldprogname = progname;
  progname = عدم;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  כלעוד ((status = loadline(L)) != -1) {
    אם (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    אם (status == LUA_OK) l_print(L);
    אחרת report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#אם !defined(luai_openlibs)
#אם defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#אחרת
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ساكن صحيح pmain (lua_State *L) {
  صحيح argc = (صحيح)lua_tointeger(L, 1);
  حرف **argv = (حرف **)lua_touserdata(L, 2);
  صحيح script;
  صحيح args = collectargs(argv, &script);
  صحيح optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  אם (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    החזר 0;
  }
  אם (args & has_v)  /* option '-v'? */
    print_version();
  אם (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  אחרת
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  אם (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    החזר 0;  /* error running LUA_INIT */
  אם (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    החזר 0;  /* something failed */
  אם (script > 0) {  /* execute main script (if there is one) */
    אם (handle_script(L, argv + script) != LUA_OK)
      החזר 0;  /* interrupt in case of error */
  }
  אם (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  אחרת אם (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    אם (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    אחרת dofile(L, عدم);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  החזר 1;
}


صحيح main (صحيح argc, حرف **argv) {
  صحيح status, result;
  lua_State *L = luaL_newstate();  /* create state */
  אם (L == عدم) {
    l_message(argv[0], "cannot create state: not enough memory");
    החזר EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  החזר (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

