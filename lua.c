/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#घोषणा lua_c

#समावेश "lprefix.h"


#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश <signal.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


#अगर !defined(LUA_PROGNAME)
#घोषणा LUA_PROGNAME		"lua"
#पूर्ण_अगर

#अगर !defined(LUA_INIT_VAR)
#घोषणा LUA_INIT_VAR		"LUA_INIT"
#पूर्ण_अगर

/* Name of the environment variable with the name of the readline library */
#अगर !defined(LUA_RLLIB_VAR)
#घोषणा LUA_RLLIB_VAR		"LUA_READLINELIB"
#पूर्ण_अगर


#घोषणा LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


अटल lua_State *globalL = शून्य;

अटल स्थिर अक्षर *progname = LUA_PROGNAME;


#अगर defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
अटल व्योम setsignal (पूर्णांक sig, व्योम (*handler)(पूर्णांक)) {
  काष्ठा sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, शून्य);
}

#अन्यथा           /* }{ */

#घोषणा setsignal            संकेत

#पूर्ण_अगर                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
अटल व्योम lstop (lua_State *L, lua_Debug *ar) {
  (व्योम)ar;  /* unused arg. */
  lua_sethook(L, शून्य, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
अटल व्योम laction (पूर्णांक i) {
  पूर्णांक flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, संक_पूर्व); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


अटल व्योम print_usage (स्थिर अक्षर *badoption) {
  lua_writestringerror("%s: ", progname);
  अगर (badoption[1] == 'e' || badoption[1] == 'l')
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
अटल व्योम l_message (स्थिर अक्षर *pname, स्थिर अक्षर *संदेश) {
  अगर (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", संदेश);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
अटल पूर्णांक report (lua_State *L, पूर्णांक status) {
  अगर (status != LUA_OK) {
    स्थिर अक्षर *संदेश = lua_tostring(L, -1);
    अगर (संदेश == शून्य)
      संदेश = "(error message not a string)";
    l_message(progname, संदेश);
    lua_निकालो(L, 1);  /* remove message */
  }
  वापस status;
}


/*
** Message handler used to run all chunks
*/
अटल पूर्णांक msghandler (lua_State *L) {
  स्थिर अक्षर *संदेश = lua_tostring(L, 1);
  अगर (संदेश == शून्य) {  /* is error object not a string? */
    अगर (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      वापस 1;  /* that is the message */
    अन्यथा
      संदेश = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, संदेश, 1);  /* append a standard traceback */
  वापस 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
अटल पूर्णांक docall (lua_State *L, पूर्णांक narg, पूर्णांक nres) {
  पूर्णांक status;
  पूर्णांक base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_जोड़ो(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(संक_विघ्न, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  setsignal(संक_विघ्न, संक_पूर्व); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  वापस status;
}


अटल व्योम print_version (व्योम) {
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
अटल व्योम createargtable (lua_State *L, अक्षर **argv, पूर्णांक argc, पूर्णांक script) {
  पूर्णांक i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  क्रम (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


अटल पूर्णांक dochunk (lua_State *L, पूर्णांक status) {
  अगर (status == LUA_OK) status = docall(L, 0, 0);
  वापस report(L, status);
}


अटल पूर्णांक dofile (lua_State *L, स्थिर अक्षर *name) {
  वापस dochunk(L, luaL_loadfilex(L, name, "bt"));
}


अटल पूर्णांक dostring (lua_State *L, स्थिर अक्षर *s, स्थिर अक्षर *name) {
  वापस dochunk(L, luaL_loadbufferx(L, s, म_माप(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
अटल पूर्णांक dolibrary (lua_State *L, अक्षर *globname) {
  पूर्णांक status;
  अक्षर *suffix = शून्य;
  अक्षर *modname = म_अक्षर(globname, '=');
  अगर (modname == शून्य) {  /* no explicit name? */
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
  अगर (status == LUA_OK) {
    अगर (suffix != शून्य)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  वापस report(L, status);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
अटल पूर्णांक pushargs (lua_State *L) {
  पूर्णांक i, n;
  अगर (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (पूर्णांक)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  क्रम (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  वापस n;
}


अटल पूर्णांक handle_script (lua_State *L, अक्षर **argv) {
  पूर्णांक status;
  स्थिर अक्षर *fname = argv[0];
  अगर (म_भेद(fname, "-") == 0 && म_भेद(argv[-1], "--") != 0)
    fname = शून्य;  /* stdin */
  status = luaL_loadfilex(L, fname, "bt");
  अगर (status == LUA_OK) {
    पूर्णांक n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  वापस report(L, status);
}


/* bits of various argument indicators in 'args' */
#घोषणा has_error	1	/* bad option */
#घोषणा has_i		2	/* -i */
#घोषणा has_v		4	/* -v */
#घोषणा has_e		8	/* -e */
#घोषणा has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
अटल पूर्णांक collectargs (अक्षर **argv, पूर्णांक *first) {
  पूर्णांक args = 0;
  पूर्णांक i;
  अगर (argv[0] != शून्य) {  /* is there a program name? */
    अगर (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  अन्यथा {  /* no program name */
    *first = -1;
    वापस 0;
  }
  क्रम (i = 1; argv[i] != शून्य; i++) {  /* handle arguments */
    *first = i;
    अगर (argv[i][0] != '-')  /* not an option? */
        वापस args;  /* stop handling options */
    चयन (argv[i][1]) {  /* else check option */
      हाल '-':  /* '--' */
        अगर (argv[i][2] != '\0')  /* extra characters after '--'? */
          वापस has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *first = (argv[i + 1] != शून्य) ? i + 1 : 0;
        वापस args;
      हाल '\0':  /* '-' */
        वापस args;  /* script "name" is '-' */
      हाल 'E':
        अगर (argv[i][2] != '\0')  /* extra characters? */
          वापस has_error;  /* invalid option */
        args |= has_E;
        अवरोध;
      हाल 'W':
        अगर (argv[i][2] != '\0')  /* extra characters? */
          वापस has_error;  /* invalid option */
        अवरोध;
      हाल 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      हाल 'v':
        अगर (argv[i][2] != '\0')  /* extra characters? */
          वापस has_error;  /* invalid option */
        args |= has_v;
        अवरोध;
      हाल 'e':
        args |= has_e;  /* FALLTHROUGH */
      हाल 'l':  /* both options need an argument */
        अगर (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          अगर (argv[i] == शून्य || argv[i][0] == '-')
            वापस has_error;  /* no next argument or it is another option */
        }
        अवरोध;
      शेष:  /* invalid option */
        वापस has_error;
    }
  }
  *first = 0;  /* no script name */
  वापस args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
अटल पूर्णांक runargs (lua_State *L, अक्षर **argv, पूर्णांक n) {
  पूर्णांक i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  क्रम (i = 1; i < n; i++) {
    पूर्णांक option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    चयन (option) {
      हाल 'e':  हाल 'l': {
        पूर्णांक status;
        अक्षर *extra = argv[i] + 2;  /* both options need an argument */
        अगर (*extra == '\0') extra = argv[++i];
        lua_assert(extra != शून्य);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        अगर (status != LUA_OK) वापस 0;
        अवरोध;
      }
      हाल 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        अवरोध;
    }
  }
  वापस 1;
}


अटल अक्षर *(*l_getenv)(स्थिर अक्षर *name);

/* Function to ignore environment variables, used by option -E */
अटल अक्षर *no_getenv (स्थिर अक्षर *name) {
  UNUSED(name);
  वापस शून्य;
}


अटल पूर्णांक handle_luainit (lua_State *L) {
  स्थिर अक्षर *name = "=" LUA_INITVARVERSION;
  स्थिर अक्षर *init = l_getenv(name + 1);
  अगर (init == शून्य) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  अगर (init == शून्य) वापस LUA_OK;
  अन्यथा अगर (init[0] == '@')
    वापस dofile(L, init+1);
  अन्यथा
    वापस dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#अगर !defined(LUA_PROMPT)
#घोषणा LUA_PROMPT		"> "
#घोषणा LUA_PROMPT2		">> "
#पूर्ण_अगर

#अगर !defined(LUA_MAXINPUT)
#घोषणा LUA_MAXINPUT		512
#पूर्ण_अगर


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#अगर !defined(lua_stdin_is_tty)	/* { */

#अगर defined(LUA_USE_POSIX)	/* { */

#समावेश <unistd.h>
#घोषणा lua_stdin_is_tty()	isatty(0)

#या_अगर defined(LUA_USE_WINDOWS)	/* }{ */

#समावेश <io.h>
#समावेश <windows.h>

#घोषणा lua_stdin_is_tty()	_isatty(_fileno(मानक_निवेश))

#अन्यथा				/* }{ */

/* ISO C definition */
#घोषणा lua_stdin_is_tty()	1  /* assume stdin is a tty */

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#अगर !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#अगर defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#समावेश <readline/readline.h>
#समावेश <readline/history.h>

#घोषणा lua_initreadline(L)  \
	((व्योम)L, *cast(स्थिर अक्षर **, &rl_readline_name)="lua")
#घोषणा lua_readline(buff,prompt)	((व्योम)buff, readline(prompt))
#घोषणा lua_saveline(पंक्ति)	add_history(पंक्ति)
#घोषणा lua_freeline(पंक्ति)	मुक्त(पंक्ति)

#अन्यथा		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
प्रकार अक्षर *(*l_readlineT) (स्थिर अक्षर *prompt);
अटल l_readlineT l_readline = शून्य;

/* pointer to 'add_history' function (if any) */
प्रकार व्योम (*l_addhistT) (स्थिर अक्षर *string);
अटल l_addhistT l_addhist = शून्य;


अटल अक्षर *lua_readline (अक्षर *buff, स्थिर अक्षर *prompt) {
  अगर (l_readline != शून्य)  /* is there a 'readline'? */
    वापस (*l_readline)(prompt);  /* use it */
  अन्यथा {  /* emulate 'readline' over 'buff' */
    ख_माला_दो(prompt, मानक_निकास);
    ख_साफ(मानक_निकास);  /* show prompt */
    वापस ख_माला_लो(buff, LUA_MAXINPUT, मानक_निवेश);  /* read line */
  }
}


अटल व्योम lua_saveline (स्थिर अक्षर *पंक्ति) {
  अगर (l_addhist != शून्य)  /* is there an 'add_history'? */
    (*l_addhist)(पंक्ति);  /* use it */
  /* else nothing to be done */
}


अटल व्योम lua_freeline (अक्षर *पंक्ति) {
  अगर (l_readline != शून्य)  /* is there a 'readline'? */
    मुक्त(पंक्ति);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#अगर defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#समावेश <dlfcn.h>

अटल व्योम lua_initreadline (lua_State *L) {
  स्थिर अक्षर *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  व्योम *lib;  /* library handle */
  अगर (rllib == शून्य)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  अगर (lib != शून्य) {
    स्थिर अक्षर **name = cast(स्थिर अक्षर**, dlsym(lib, "rl_readline_name"));
    अगर (name != शून्य)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    अगर (l_readline != शून्य)  /* could load readline function? */
      वापस;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#अन्यथा		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#घोषणा lua_initreadline(L)	((व्योम)L)

#पूर्ण_अगर		/* } */

#पूर्ण_अगर				/* } */

#पूर्ण_अगर				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
अटल स्थिर अक्षर *get_prompt (lua_State *L, पूर्णांक firstline) {
  अगर (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    वापस (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  अन्यथा {  /* apply 'tostring' over the value */
    स्थिर अक्षर *p = luaL_tolstring(L, -1, शून्य);
    lua_remove(L, -2);  /* remove original value */
    वापस p;
  }
}

/* mark in error messages for incomplete statements */
#घोषणा EOFMARK		"<eof>"
#घोषणा marklen		(माप(EOFMARK)/माप(अक्षर) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
अटल पूर्णांक incomplete (lua_State *L, पूर्णांक status) {
  अगर (status == LUA_ERRSYNTAX) {
    माप_प्रकार lmsg;
    स्थिर अक्षर *संदेश = lua_tolstring(L, -1, &lmsg);
    अगर (lmsg >= marklen && म_भेद(संदेश + lmsg - marklen, EOFMARK) == 0)
      वापस 1;
  }
  वापस 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
अटल पूर्णांक pushline (lua_State *L, पूर्णांक firstline) {
  अक्षर बफर[LUA_MAXINPUT];
  माप_प्रकार l;
  स्थिर अक्षर *prmt = get_prompt(L, firstline);
  अक्षर *b = lua_readline(बफर, prmt);
  lua_निकालो(L, 1);  /* remove prompt */
  अगर (b == शून्य)
    वापस 0;  /* no input */
  l = म_माप(b);
  अगर (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  वापस 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
अटल पूर्णांक addreturn (lua_State *L) {
  स्थिर अक्षर *पंक्ति = lua_tostring(L, -1);  /* original line */
  स्थिर अक्षर *retline = lua_pushfstring(L, "return %s;", पंक्ति);
  पूर्णांक status = luaL_loadbufferx(L, retline, म_माप(retline), "=stdin", "t");
  अगर (status == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  अन्यथा
    lua_निकालो(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  वापस status;
}


अटल व्योम checklocal (स्थिर अक्षर *पंक्ति) {
  अटल स्थिर माप_प्रकार szloc = माप("local") - 1;
  अटल स्थिर अक्षर space[] = " \t";
  पंक्ति += म_अखोज(पंक्ति, space);  /* skip spaces */
  अगर (म_भेदन(पंक्ति, "local", szloc) == 0 &&  /* "local"? */
      म_अक्षर(space, *(पंक्ति + szloc)) != शून्य) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
अटल पूर्णांक multiline (lua_State *L) {
  माप_प्रकार len;
  स्थिर अक्षर *पंक्ति = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(पंक्ति);
  क्रम (;;) {  /* repeat until gets a complete statement */
    पूर्णांक status = luaL_loadbufferx(L, पंक्ति, len, "=stdin", "t");  /* try it */
    अगर (!incomplete(L, status) || !pushline(L, 0))
      वापस status;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_जोड़ो(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    पंक्ति = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
अटल पूर्णांक loadline (lua_State *L) {
  स्थिर अक्षर *पंक्ति;
  पूर्णांक status;
  lua_settop(L, 0);
  अगर (!pushline(L, 1))
    वापस -1;  /* no input */
  अगर ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  पंक्ति = lua_tostring(L, 1);
  अगर (पंक्ति[0] != '\0')  /* non empty? */
    lua_saveline(पंक्ति);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  वापस status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
अटल व्योम l_print (lua_State *L) {
  पूर्णांक n = lua_gettop(L);
  अगर (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_जोड़ो(L, 1);
    अगर (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
अटल व्योम doREPL (lua_State *L) {
  पूर्णांक status;
  स्थिर अक्षर *oldprogname = progname;
  progname = शून्य;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  जबतक ((status = loadline(L)) != -1) {
    अगर (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    अगर (status == LUA_OK) l_print(L);
    अन्यथा report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#अगर !defined(luai_openlibs)
#अगर defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#घोषणा luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#अन्यथा
/* The default is to open all standard libraries */
#घोषणा luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#पूर्ण_अगर
#पूर्ण_अगर


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
अटल पूर्णांक pmain (lua_State *L) {
  पूर्णांक argc = (पूर्णांक)lua_tointeger(L, 1);
  अक्षर **argv = (अक्षर **)lua_touserdata(L, 2);
  पूर्णांक script;
  पूर्णांक args = collectargs(argv, &script);
  पूर्णांक optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  अगर (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    वापस 0;
  }
  अगर (args & has_v)  /* option '-v'? */
    print_version();
  अगर (args & has_E) {  /* option '-E'? */
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
  अगर (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    वापस 0;  /* error running LUA_INIT */
  अगर (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    वापस 0;  /* something failed */
  अगर (script > 0) {  /* execute main script (if there is one) */
    अगर (handle_script(L, argv + script) != LUA_OK)
      वापस 0;  /* interrupt in case of error */
  }
  अगर (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  अन्यथा अगर (script < 1 && !(args & (has_e | has_v))) { /* no active option? */
    अगर (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    अन्यथा dofile(L, शून्य);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  वापस 1;
}


पूर्णांक मुख्य (पूर्णांक argc, अक्षर **argv) {
  पूर्णांक status, परिणाम;
  lua_State *L = luaL_newstate();  /* create state */
  अगर (L == शून्य) {
    l_message(argv[0], "cannot create state: not enough memory");
    वापस निकास_त्रुटि;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 2, 1, 0);  /* do the call */
  परिणाम = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  lua_close(L);
  वापस (परिणाम && status == LUA_OK) ? निकास_सफल : निकास_त्रुटि;
}

