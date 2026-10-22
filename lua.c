/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#وصف_ڪريو lua_c

#ضمن "lprefix.h"


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن <signal.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


#إذا !defined(LUA_PROGNAME)
#وصف_ڪريو LUA_PROGNAME		"lua"
#جيڪڏهن_پورو

#إذا !defined(LUA_INIT_VAR)
#وصف_ڪريو LUA_INIT_VAR		"LUA_INIT"
#جيڪڏهن_پورو

/* Name of the environment variable with the name of the readline library */
#إذا !defined(LUA_RLLIB_VAR)
#وصف_ڪريو LUA_RLLIB_VAR		"LUA_READLINELIB"
#جيڪڏهن_پورو


#وصف_ڪريو LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ساكن lua_State *globalL = عدم;

ساكن ثابت حرف *progname = LUA_PROGNAME;


#إذا defined(LUA_USE_POSIX)   /* { */

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

#وإلا           /* }{ */

#وصف_ڪريو setsignal            إشارة

#جيڪڏهن_پورو                               /* } */


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
  إذا (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  وإلا
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
ساكن فراغ l_پيغام (ثابت حرف *pname, ثابت حرف *نياپو) {
  إذا (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", نياپو);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ساكن صحيح report (lua_State *L, صحيح حالت) {
  إذا (حالت != LUA_OK) {
    ثابت حرف *نياپو = lua_tostring(L, -1);
    إذا (نياپو == عدم)
      نياپو = "(error message not a string)";
    l_پيغام(progname, نياپو);
    lua_ڪڍو(L, 1);  /* remove message */
  }
  أرجع حالت;
}


/*
** Message handler used to run all chunks
*/
ساكن صحيح msghandler (lua_State *L) {
  ثابت حرف *نياپو = lua_tostring(L, 1);
  إذا (نياپو == عدم) {  /* is error object not a string? */
    إذا (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      أرجع 1;  /* that is the message */
    وإلا
      نياپو = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, نياپو, 1);  /* append a standard traceback */
  أرجع 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ساكن صحيح docall (lua_State *L, صحيح narg, صحيح nres) {
  صحيح حالت;
  صحيح بنياد = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_وجهو(L, بنياد);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  حالت = lua_pcall(L, narg, nres, بنياد);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, بنياد);  /* remove message handler from the stack */
  أرجع حالت;
}


ساكن فراغ print_نسخو (فراغ) {
  lua_writestring(LUA_COPYRIGHT, لڙي_ڊيگهه(LUA_COPYRIGHT));
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
  لكل (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ساكن صحيح dochunk (lua_State *L, صحيح حالت) {
  إذا (حالت == LUA_OK) حالت = docall(L, 0, 0);
  أرجع report(L, حالت);
}


ساكن صحيح dofile (lua_State *L, ثابت حرف *name) {
  أرجع dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ساكن صحيح dostring (lua_State *L, ثابت حرف *s, ثابت حرف *name) {
  أرجع dochunk(L, luaL_loadbufferx(L, s, لڙي_ڊيگهه(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ساكن صحيح dolibrary (lua_State *L, حرف *globname) {
  صحيح حالت;
  حرف *suffix = عدم;
  حرف *modname = strchr(globname, '=');
  إذا (modname == عدم) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  وإلا {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  حالت = docall(L, 1, 1);  /* call 'require(modname)' */
  إذا (حالت == LUA_OK) {
    إذا (suffix != عدم)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  أرجع report(L, حالت);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ساكن صحيح pushargs (lua_State *L) {
  صحيح i, n;
  إذا (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (صحيح)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  لكل (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  أرجع n;
}


ساكن صحيح handle_script (lua_State *L, حرف **argv) {
  صحيح حالت;
  ثابت حرف *fname = argv[0];
  إذا (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = عدم;  /* stdin */
  حالت = luaL_loadfilex(L, fname, "bt");
  إذا (حالت == LUA_OK) {
    صحيح n = pushargs(L);  /* push arguments to script */
    حالت = docall(L, n, LUA_MULTRET);
  }
  أرجع report(L, حالت);
}


/* bits of various argument indicators in 'args' */
#وصف_ڪريو has_error	1	/* bad option */
#وصف_ڪريو has_i		2	/* -i */
#وصف_ڪريو has_v		4	/* -v */
#وصف_ڪريو has_e		8	/* -e */
#وصف_ڪريو has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
ساكن صحيح collectargs (حرف **argv, صحيح *پهريون) {
  صحيح دليلون = 0;
  صحيح i;
  إذا (argv[0] != عدم) {  /* is there a program name? */
    إذا (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  وإلا {  /* no program name */
    *پهريون = -1;
    أرجع 0;
  }
  لكل (i = 1; argv[i] != عدم; i++) {  /* handle arguments */
    *پهريون = i;
    إذا (argv[i][0] != '-')  /* not an option? */
        أرجع دليلون;  /* stop handling options */
    بدل (argv[i][1]) {  /* else check option */
      حالة '-':  /* '--' */
        إذا (argv[i][2] != '\0')  /* extra characters after '--'? */
          أرجع has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *پهريون = (argv[i + 1] != عدم) ? i + 1 : 0;
        أرجع دليلون;
      حالة '\0':  /* '-' */
        أرجع دليلون;  /* script "name" is '-' */
      حالة 'E':
        إذا (argv[i][2] != '\0')  /* extra characters? */
          أرجع has_error;  /* invalid option */
        دليلون |= has_E;
        اكسر;
      حالة 'W':
        إذا (argv[i][2] != '\0')  /* extra characters? */
          أرجع has_error;  /* invalid option */
        اكسر;
      حالة 'i':
        دليلون |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      حالة 'v':
        إذا (argv[i][2] != '\0')  /* extra characters? */
          أرجع has_error;  /* invalid option */
        دليلون |= has_v;
        اكسر;
      حالة 'e':
        دليلون |= has_e;  /* FALLTHROUGH */
      حالة 'l':  /* both options need an argument */
        إذا (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          إذا (argv[i] == عدم || argv[i][0] == '-')
            أرجع has_error;  /* no next argument or it is another option */
        }
        اكسر;
      افتراضي:  /* invalid option */
        أرجع has_error;
    }
  }
  *پهريون = 0;  /* no script name */
  أرجع دليلون;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ساكن صحيح runargs (lua_State *L, حرف **argv, صحيح n) {
  صحيح i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  لكل (i = 1; i < n; i++) {
    صحيح option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    بدل (option) {
      حالة 'e':  حالة 'l': {
        صحيح حالت;
        حرف *extra = argv[i] + 2;  /* both options need an argument */
        إذا (*extra == '\0') extra = argv[++i];
        lua_assert(extra != عدم);
        حالت = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        إذا (حالت != LUA_OK) أرجع 0;
        اكسر;
      }
      حالة 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        اكسر;
    }
  }
  أرجع 1;
}


ساكن حرف *(*l_getenv)(ثابت حرف *name);

/* Function to ignore environment variables, used by option -E */
ساكن حرف *no_getenv (ثابت حرف *name) {
  اڻ_استعمال_ٿيل(name);
  أرجع عدم;
}


ساكن صحيح handle_luainit (lua_State *L) {
  ثابت حرف *name = "=" LUA_INITVARVERSION;
  ثابت حرف *init = l_getenv(name + 1);
  إذا (init == عدم) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  إذا (init == عدم) أرجع LUA_OK;
  وإلا إذا (init[0] == '@')
    أرجع dofile(L, init+1);
  وإلا
    أرجع dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#إذا !defined(LUA_PROMPT)
#وصف_ڪريو LUA_PROMPT		"> "
#وصف_ڪريو LUA_PROMPT2		">> "
#جيڪڏهن_پورو

#إذا !defined(LUA_MAXINPUT)
#وصف_ڪريو LUA_MAXINPUT		512
#جيڪڏهن_پورو


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#إذا !defined(lua_stdin_is_tty)	/* { */

#إذا defined(LUA_USE_POSIX)	/* { */

#ضمن <unistd.h>
#وصف_ڪريو lua_stdin_is_tty()	isatty(0)

#وإلاإذا defined(LUA_USE_WINDOWS)	/* }{ */

#ضمن <io.h>
#ضمن <windows.h>

#وصف_ڪريو lua_stdin_is_tty()	_isatty(_fileno(stdin))

#وإلا				/* }{ */

/* ISO C definition */
#وصف_ڪريو lua_stdin_is_tty()	1  /* assume stdin is a tty */

#جيڪڏهن_پورو				/* } */

#جيڪڏهن_پورو				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#إذا !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#إذا defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ضمن <readline/readline.h>
#ضمن <readline/history.h>

#وصف_ڪريو lua_initreadline(L)  \
	((فراغ)L, *cast(ثابت حرف **, &rl_readline_name)="lua")
#وصف_ڪريو lua_readline(buff,prompt)	((فراغ)buff, readline(prompt))
#وصف_ڪريو lua_saveline(سٽ)	جوڙيو_history(سٽ)
#وصف_ڪريو lua_freeline(سٽ)	حرر(سٽ)

#وإلا		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
تعريفنوع حرف *(*l_readlineT) (ثابت حرف *prompt);
ساكن l_readlineT l_readline = عدم;

/* pointer to 'add_history' function (if any) */
تعريفنوع فراغ (*l_addhistT) (ثابت حرف *string);
ساكن l_addhistT l_addhist = عدم;


ساكن حرف *lua_readline (حرف *buff, ثابت حرف *prompt) {
  إذا (l_readline != عدم)  /* is there a 'readline'? */
    أرجع (*l_readline)(prompt);  /* use it */
  وإلا {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    أرجع fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ساكن فراغ lua_saveline (ثابت حرف *سٽ) {
  إذا (l_addhist != عدم)  /* is there an 'add_history'? */
    (*l_addhist)(سٽ);  /* use it */
  /* else nothing to be done */
}


ساكن فراغ lua_freeline (حرف *سٽ) {
  إذا (l_readline != عدم)  /* is there a 'readline'? */
    حرر(سٽ);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#إذا defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ضمن <dlfcn.h>

ساكن فراغ lua_initreadline (lua_State *L) {
  ثابت حرف *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  فراغ *lib;  /* library handle */
  إذا (rllib == عدم)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  إذا (lib != عدم) {
    ثابت حرف **name = cast(ثابت حرف**, dlsym(lib, "rl_readline_name"));
    إذا (name != عدم)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    إذا (l_readline != عدم)  /* could load readline function? */
      أرجع;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#وإلا		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#وصف_ڪريو lua_initreadline(L)	((فراغ)L)

#جيڪڏهن_پورو		/* } */

#جيڪڏهن_پورو				/* } */

#جيڪڏهن_پورو				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ساكن ثابت حرف *get_prompt (lua_State *L, صحيح firstline) {
  إذا (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    أرجع (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  وإلا {  /* apply 'tostring' over the value */
    ثابت حرف *p = luaL_tolstring(L, -1, عدم);
    lua_remove(L, -2);  /* remove original value */
    أرجع p;
  }
}

/* mark in error messages for incomplete statements */
#وصف_ڪريو EOFMARK		"<eof>"
#وصف_ڪريو marklen		(حجم(EOFMARK)/حجم(حرف) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ساكن صحيح incomplete (lua_State *L, صحيح حالت) {
  إذا (حالت == LUA_ERRSYNTAX) {
    size_t lmsg;
    ثابت حرف *نياپو = lua_tolstring(L, -1, &lmsg);
    إذا (lmsg >= marklen && strcmp(نياپو + lmsg - marklen, EOFMARK) == 0)
      أرجع 1;
  }
  أرجع 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ساكن صحيح pushline (lua_State *L, صحيح firstline) {
  حرف بفر[LUA_MAXINPUT];
  size_t l;
  ثابت حرف *prmt = get_prompt(L, firstline);
  حرف *b = lua_readline(بفر, prmt);
  lua_ڪڍو(L, 1);  /* remove prompt */
  إذا (b == عدم)
    أرجع 0;  /* no input */
  l = لڙي_ڊيگهه(b);
  إذا (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  أرجع 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ساكن صحيح addreturn (lua_State *L) {
  ثابت حرف *سٽ = lua_tostring(L, -1);  /* original line */
  ثابت حرف *retline = lua_pushfstring(L, "return %s;", سٽ);
  صحيح حالت = luaL_loadbufferx(L, retline, لڙي_ڊيگهه(retline), "=stdin", "t");
  إذا (حالت == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  وإلا
    lua_ڪڍو(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  أرجع حالت;
}


ساكن فراغ checklocal (ثابت حرف *سٽ) {
  ساكن ثابت size_t szloc = حجم("local") - 1;
  ساكن ثابت حرف space[] = " \t";
  سٽ += strspn(سٽ, space);  /* skip spaces */
  إذا (لڙي_ڀيٽ(سٽ, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(سٽ + szloc)) != عدم) {  /* followed by a space? */
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
  ثابت حرف *سٽ = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(سٽ);
  لكل (;;) {  /* repeat until gets a complete statement */
    صحيح حالت = luaL_loadbufferx(L, سٽ, len, "=stdin", "t");  /* try it */
    إذا (!incomplete(L, حالت) || !pushline(L, 0))
      أرجع حالت;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_وجهو(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    سٽ = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ساكن صحيح loadline (lua_State *L) {
  ثابت حرف *سٽ;
  صحيح حالت;
  lua_settop(L, 0);
  إذا (!pushline(L, 1))
    أرجع -1;  /* no input */
  إذا ((حالت = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    حالت = multiline(L);  /* try as command, maybe with continuation lines */
  سٽ = lua_tostring(L, 1);
  إذا (سٽ[0] != '\0')  /* non empty? */
    lua_saveline(سٽ);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  أرجع حالت;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ساكن فراغ l_print (lua_State *L) {
  صحيح n = lua_gettop(L);
  إذا (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_وجهو(L, 1);
    إذا (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_پيغام(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ساكن فراغ doREPL (lua_State *L) {
  صحيح حالت;
  ثابت حرف *oldprogname = progname;
  progname = عدم;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  طالما ((حالت = loadline(L)) != -1) {
    إذا (حالت == LUA_OK)
      حالت = docall(L, 0, LUA_MULTRET);
    إذا (حالت == LUA_OK) l_print(L);
    وإلا report(L, حالت);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#إذا !defined(luai_openlibs)
#إذا defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#وصف_ڪريو luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#وإلا
/* The default is to open all standard libraries */
#وصف_ڪريو luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#جيڪڏهن_پورو
#جيڪڏهن_پورو


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ساكن صحيح pmain (lua_State *L) {
  صحيح argc = (صحيح)lua_tointeger(L, 1);
  حرف **argv = (حرف **)lua_touserdata(L, 2);
  صحيح script;
  صحيح دليلون = collectargs(argv, &script);
  صحيح optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  إذا (دليلون == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    أرجع 0;
  }
  إذا (دليلون & has_v)  /* option '-v'? */
    print_نسخو();
  إذا (دليلون & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  وإلا
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  إذا (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    أرجع 0;  /* error running LUA_INIT */
  إذا (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    أرجع 0;  /* something failed */
  إذا (script > 0) {  /* execute main script (if there is one) */
    إذا (handle_script(L, argv + script) != LUA_OK)
      أرجع 0;  /* interrupt in case of error */
  }
  إذا (دليلون & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  وإلا إذا (script < 1 && !(دليلون & (has_e | has_v))) { /* no active option? */
    إذا (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_نسخو();
      doREPL(L);  /* do read-eval-print loop */
    }
    وإلا dofile(L, عدم);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  أرجع 1;
}


صحيح مکيه (صحيح argc, حرف **argv) {
  صحيح حالت, نتيجو;
  lua_State *L = luaL_newstate();  /* create state */
  إذا (L == عدم) {
    l_پيغام(argv[0], "cannot create state: not enough memory");
    أرجع EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  حالت = lua_pcall(L, 2, 1, 0);  /* do the call */
  نتيجو = lua_toboolean(L, -1);  /* get result */
  report(L, حالت);
  lua_close(L);
  أرجع (نتيجو && حالت == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

