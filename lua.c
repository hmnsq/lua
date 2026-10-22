/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ <signal.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


#જો !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#જો !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

/* Name of the environment variable with the name of the readline library */
#જો !defined(LUA_RLLIB_VAR)
#define LUA_RLLIB_VAR		"LUA_READLINELIB"
#endif


#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


સ્થિર lua_State *globalL = ખાલી;

સ્થિર અચલ અક્ષર *progname = LUA_PROGNAME;


#જો defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
સ્થિર શૂન્ય setsignal (પૂર્ણાંક sig, શૂન્ય (*handler)(પૂર્ણાંક)) {
  સંરચના sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ખાલી);
}

#નહીંતર           /* }{ */

#define setsignal            સંકેત

#endif                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
સ્થિર શૂન્ય lstop (lua_State *L, lua_Debug *ar) {
  (શૂન્ય)ar;  /* unused arg. */
  lua_sethook(L, ખાલી, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
સ્થિર શૂન્ય laction (પૂર્ણાંક i) {
  પૂર્ણાંક flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


સ્થિર શૂન્ય print_usage (અચલ અક્ષર *badoption) {
  lua_writestringerror("%s: ", progname);
  જો (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  નહીંતર
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
સ્થિર શૂન્ય l_message (અચલ અક્ષર *pname, અચલ અક્ષર *સંદેશ) {
  જો (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", સંદેશ);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
સ્થિર પૂર્ણાંક report (lua_State *L, પૂર્ણાંક status) {
  જો (status != LUA_OK) {
    અચલ અક્ષર *સંદેશ = lua_tostring(L, -1);
    જો (સંદેશ == ખાલી)
      સંદેશ = "(error message not a string)";
    l_message(progname, સંદેશ);
    lua_કાઢો(L, 1);  /* remove message */
  }
  પાછા status;
}


/*
** Message handler used to run all chunks
*/
સ્થિર પૂર્ણાંક msghandler (lua_State *L) {
  અચલ અક્ષર *સંદેશ = lua_tostring(L, 1);
  જો (સંદેશ == ખાલી) {  /* is error object not a string? */
    જો (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      પાછા 1;  /* that is the message */
    નહીંતર
      સંદેશ = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, સંદેશ, 1);  /* append a standard traceback */
  પાછા 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
સ્થિર પૂર્ણાંક docall (lua_State *L, પૂર્ણાંક narg, પૂર્ણાંક nres) {
  પૂર્ણાંક status;
  પૂર્ણાંક base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_ઉમેરો(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  પાછા status;
}


સ્થિર શૂન્ય print_version (શૂન્ય) {
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
સ્થિર શૂન્ય createargtable (lua_State *L, અક્ષર **argv, પૂર્ણાંક argc, પૂર્ણાંક script) {
  પૂર્ણાંક i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  માટે (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


સ્થિર પૂર્ણાંક dochunk (lua_State *L, પૂર્ણાંક status) {
  જો (status == LUA_OK) status = docall(L, 0, 0);
  પાછા report(L, status);
}


સ્થિર પૂર્ણાંક dofile (lua_State *L, અચલ અક્ષર *name) {
  પાછા dochunk(L, luaL_loadfilex(L, name, "bt"));
}


સ્થિર પૂર્ણાંક dostring (lua_State *L, અચલ અક્ષર *s, અચલ અક્ષર *name) {
  પાછા dochunk(L, luaL_loadbufferx(L, s, strlen(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
સ્થિર પૂર્ણાંક dolibrary (lua_State *L, અક્ષર *globname) {
  પૂર્ણાંક status;
  અક્ષર *suffix = ખાલી;
  અક્ષર *modname = strchr(globname, '=');
  જો (modname == ખાલી) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  નહીંતર {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);  /* call 'require(modname)' */
  જો (status == LUA_OK) {
    જો (suffix != ખાલી)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  પાછા report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
સ્થિર પૂર્ણાંક pushargs (lua_State *L) {
  પૂર્ણાંક i, n;
  જો (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (પૂર્ણાંક)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  માટે (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  પાછા n;
}


સ્થિર પૂર્ણાંક handle_script (lua_State *L, અક્ષર **argv) {
  પૂર્ણાંક status;
  અચલ અક્ષર *fname = argv[0];
  જો (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ખાલી;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  જો (status == LUA_OK) {
    પૂર્ણાંક n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  પાછા report(L, status);
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
સ્થિર પૂર્ણાંક collectargs (અક્ષર **argv, પૂર્ણાંક *first) {
  પૂર્ણાંક args = 0;
  પૂર્ણાંક i;
  જો (argv[0] != ખાલી) {  /* is there a program name? */
    જો (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  નહીંતર {  /* no program name */
    *first = -1;
    પાછા 0;
  }
  માટે (i = 1; argv[i] != ખાલી; i++) {  /* handle arguments */
    *first = i;
    જો (argv[i][0] != '-')  /* not an option? */
        પાછા args;  /* stop handling options */
    પસંદગી (argv[i][1]) {  /* else check option */
      સ્થિતિ '-':  /* '--' */
        જો (argv[i][2] != '\0')  /* extra characters after '--'? */
          પાછા has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != ખાલી) ? i + 1 : 0;
        પાછા args;
      સ્થિતિ '\0':  /* '-' */
        પાછા args;  /* script "name" is '-' */
      સ્થિતિ 'E':
        જો (argv[i][2] != '\0')  /* extra characters? */
          પાછા has_error;  /* invalid option */
        args |= has_E;
        તોડો;
      સ્થિતિ 'W':
        જો (argv[i][2] != '\0')  /* extra characters? */
          પાછા has_error;  /* invalid option */
        તોડો;
      સ્થિતિ 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      સ્થિતિ 'v':
        જો (argv[i][2] != '\0')  /* extra characters? */
          પાછા has_error;  /* invalid option */
        args |= has_v;
        તોડો;
      સ્થિતિ 'e':
        args |= has_e;  /* FALLTHROUGH */
      સ્થિતિ 'l':  /* both options need an argument */
        જો (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          જો (argv[i] == ખાલી || argv[i][0] == '-')
            પાછા has_error;  /* no next argument or it is another option */
        }
        તોડો;
      મૂળભૂત:  /* invalid option */
        પાછા has_error;
    }
  }
  *first = 0;  /* no script name */
  પાછા args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
સ્થિર પૂર્ણાંક runargs (lua_State *L, અક્ષર **argv, પૂર્ણાંક n) {
  પૂર્ણાંક i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  માટે (i = 1; i < n; i++) {
    પૂર્ણાંક option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    પસંદગી (option) {
      સ્થિતિ 'e':  સ્થિતિ 'l': {
        પૂર્ણાંક status;
        અક્ષર *extra = argv[i] + 2;  /* both options need an argument */
        જો (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ખાલી);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        જો (status != LUA_OK) પાછા 0;
        તોડો;
      }
      સ્થિતિ 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        તોડો;
    }
  }
  પાછા 1;
}


સ્થિર અક્ષર *(*l_getenv)(અચલ અક્ષર *name);

/* Function to ignore environment variables, used by option -E */
સ્થિર અક્ષર *no_getenv (અચલ અક્ષર *name) {
  UNUSED(name);
  પાછા ખાલી;
}


સ્થિર પૂર્ણાંક handle_luainit (lua_State *L) {
  અચલ અક્ષર *name = "=" LUA_INITVARVERSION;
  અચલ અક્ષર *init = l_getenv(name + 1);
  જો (init == ખાલી) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  જો (init == ખાલી) પાછા LUA_OK;
  નહીંતર જો (init[0] == '@')
    પાછા dofile(L, init+1);
  નહીંતર
    પાછા dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#જો !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#જો !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#જો !defined(lua_stdin_is_tty)	/* { */

#જો defined(LUA_USE_POSIX)	/* { */

#અન્તર્ભાવ <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#અથવાજો defined(LUA_USE_WINDOWS)	/* }{ */

#અન્તર્ભાવ <io.h>
#અન્તર્ભાવ <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#નહીંતર				/* }{ */

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

#જો !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#જો defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#અન્તર્ભાવ <readline/readline.h>
#અન્તર્ભાવ <readline/history.h>

#define lua_initreadline(L)  \
	((શૂન્ય)L, *cast(અચલ અક્ષર **, &rl_readline_name)="lua")
#define lua_readline(buff,prompt)	((શૂન્ય)buff, readline(prompt))
#define lua_saveline(લીટી)	add_history(લીટી)
#define lua_freeline(લીટી)	મુક્ત(લીટી)

#નહીંતર		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
પ્રકારવ્યાખ્યા અક્ષર *(*l_readlineT) (અચલ અક્ષર *prompt);
સ્થિર l_readlineT l_readline = ખાલી;

/* pointer to 'add_history' function (if any) */
પ્રકારવ્યાખ્યા શૂન્ય (*l_addhistT) (અચલ અક્ષર *string);
સ્થિર l_addhistT l_addhist = ખાલી;


સ્થિર અક્ષર *lua_readline (અક્ષર *buff, અચલ અક્ષર *prompt) {
  જો (l_readline != ખાલી)  /* is there a 'readline'? */
    પાછા (*l_readline)(prompt);  /* use it */
  નહીંતર {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    પાછા fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


સ્થિર શૂન્ય lua_saveline (અચલ અક્ષર *લીટી) {
  જો (l_addhist != ખાલી)  /* is there an 'add_history'? */
    (*l_addhist)(લીટી);  /* use it */
  /* else nothing to be done */
}


સ્થિર શૂન્ય lua_freeline (અક્ષર *લીટી) {
  જો (l_readline != ખાલી)  /* is there a 'readline'? */
    મુક્ત(લીટી);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#જો defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#અન્તર્ભાવ <dlfcn.h>

સ્થિર શૂન્ય lua_initreadline (lua_State *L) {
  અચલ અક્ષર *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  શૂન્ય *lib;  /* library handle */
  જો (rllib == ખાલી)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  જો (lib != ખાલી) {
    અચલ અક્ષર **name = cast(અચલ અક્ષર**, dlsym(lib, "rl_readline_name"));
    જો (name != ખાલી)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    જો (l_readline != ખાલી)  /* could load readline function? */
      પાછા;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#નહીંતર		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#define lua_initreadline(L)	((શૂન્ય)L)

#endif		/* } */

#endif				/* } */

#endif				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
સ્થિર અચલ અક્ષર *get_prompt (lua_State *L, પૂર્ણાંક firstline) {
  જો (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    પાછા (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  નહીંતર {  /* apply 'tostring' over the value */
    અચલ અક્ષર *p = luaL_tolstring(L, -1, ખાલી);
    lua_remove(L, -2);  /* remove original value */
    પાછા p;
  }
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(કદ(EOFMARK)/કદ(અક્ષર) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
સ્થિર પૂર્ણાંક incomplete (lua_State *L, પૂર્ણાંક status) {
  જો (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    અચલ અક્ષર *સંદેશ = lua_tolstring(L, -1, &lmsg);
    જો (lmsg >= marklen && strcmp(સંદેશ + lmsg - marklen, EOFMARK) == 0)
      પાછા 1;
  }
  પાછા 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
સ્થિર પૂર્ણાંક pushline (lua_State *L, પૂર્ણાંક firstline) {
  અક્ષર buffer[LUA_MAXINPUT];
  size_t l;
  અચલ અક્ષર *prmt = get_prompt(L, firstline);
  અક્ષર *b = lua_readline(buffer, prmt);
  lua_કાઢો(L, 1);  /* remove prompt */
  જો (b == ખાલી)
    પાછા 0;  /* no input */
  l = strlen(b);
  જો (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  પાછા 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
સ્થિર પૂર્ણાંક addreturn (lua_State *L) {
  અચલ અક્ષર *લીટી = lua_tostring(L, -1);  /* original line */
  અચલ અક્ષર *retline = lua_pushfstring(L, "return %s;", લીટી);
  પૂર્ણાંક status = luaL_loadbufferx(L, retline, strlen(retline), "=stdin", "t");
  જો (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  નહીંતર
    lua_કાઢો(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  પાછા status;
}


સ્થિર શૂન્ય checklocal (અચલ અક્ષર *લીટી) {
  સ્થિર અચલ size_t szloc = કદ("local") - 1;
  સ્થિર અચલ અક્ષર space[] = " \t";
  લીટી += strspn(લીટી, space);  /* skip spaces */
  જો (strncmp(લીટી, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(લીટી + szloc)) != ખાલી) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
સ્થિર પૂર્ણાંક multiline (lua_State *L) {
  size_t len;
  અચલ અક્ષર *લીટી = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(લીટી);
  માટે (;;) {  /* repeat until gets a complete statement */
    પૂર્ણાંક status = luaL_loadbufferx(L, લીટી, len, "=stdin", "t");  /* try it */
    જો (!incomplete(L, status) || !pushline(L, 0))
      પાછા status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_ઉમેરો(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    લીટી = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
સ્થિર પૂર્ણાંક loadline (lua_State *L) {
  અચલ અક્ષર *લીટી;
  પૂર્ણાંક status;
  lua_settop(L, 0);
  જો (!pushline(L, 1))
    પાછા -1;  /* no input */
  જો ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  લીટી = lua_tostring(L, 1);
  જો (લીટી[0] != '\0')  /* non empty? */
    lua_saveline(લીટી);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  પાછા status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
સ્થિર શૂન્ય l_print (lua_State *L) {
  પૂર્ણાંક n = lua_gettop(L);
  જો (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_ઉમેરો(L, 1);
    જો (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
સ્થિર શૂન્ય doREPL (lua_State *L) {
  પૂર્ણાંક status;
  અચલ અક્ષર *oldprogname = progname;
  progname = ખાલી;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  જ્યારે ((status = loadline(L)) != -1) {
    જો (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    જો (status == LUA_OK) l_print(L);
    નહીંતર report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#જો !defined(luai_openlibs)
#જો defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#નહીંતર
/* The default is to open all standard libraries */
#define luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#endif
#endif


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
સ્થિર પૂર્ણાંક pmain (lua_State *L) {
  પૂર્ણાંક argc = (પૂર્ણાંક)lua_tointeger(L, 1);
  અક્ષર **argv = (અક્ષર **)lua_touserdata(L, 2);
  પૂર્ણાંક script;
  પૂર્ણાંક args = collectargs(argv, &script);
  પૂર્ણાંક optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  જો (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    પાછા 0;
  }
  જો (args & has_v)  /* option '-v'? */
    print_version();
  જો (args & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  નહીંતર
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  જો (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    પાછા 0;  /* error running LUA_INIT */
  જો (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    પાછા 0;  /* something failed */
  જો (script > 0) {  /* execute main script (if there is one) */
    જો (handle_script(L, argv + script) != LUA_OK)
      પાછા 0;  /* interrupt in case of error */
  }
  જો (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  નહીંતર જો (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    જો (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    નહીંતર dofile(L, ખાલી);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  પાછા 1;
}


પૂર્ણાંક મુખ્ય (પૂર્ણાંક argc, અક્ષર **argv) {
  પૂર્ણાંક status, result;
  lua_State *L = luaL_newstate();  /* create state */
  જો (L == ખાલી) {
    l_message(argv[0], "cannot create state: not enough memory");
    પાછા EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  પાછા (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

