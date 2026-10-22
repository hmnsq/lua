/*
** $Id: lua.c $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lua_c

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ <signal.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


#ಆದರೆ !defined(LUA_PROGNAME)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_PROGNAME		"lua"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_INIT_VAR)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INIT_VAR		"LUA_INIT"
#ಆದರೆ_ಅಂತ್ಯ

/* Name of the environment variable with the name of the readline library */
#ಆದರೆ !defined(LUA_RLLIB_VAR)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RLLIB_VAR		"LUA_READLINELIB"
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


ಸ್ಥಿರ lua_State *globalL = ಖಾಲಿ;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *progname = LUA_PROGNAME;


#ಆದರೆ defined(LUA_USE_POSIX)   /* { */

/*
** Use 'sigaction' when available.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setsignal (ಪೂರ್ಣಾಂಕ sig, ಶೂನ್ಯ (*handler)(ಪೂರ್ಣಾಂಕ)) {
  ರಚನೆ sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, ಖಾಲಿ);
}

#ಇಲ್ಲದಿದ್ದರೆ           /* }{ */

#ವ್ಯಾಖ್ಯಾನಿಸು setsignal            ಸಂಕೇತ

#ಆದರೆ_ಅಂತ್ಯ                               /* } */


/*
** Hook set by signal function to stop the interpreter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ lstop (lua_State *L, lua_Debug *ar) {
  (ಶೂನ್ಯ)ar;  /* unused arg. */
  lua_sethook(L, ಖಾಲಿ, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ laction (ಪೂರ್ಣಾಂಕ i) {
  ಪೂರ್ಣಾಂಕ flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  setsignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, flag, 1);
}


ಸ್ಥಿರ ಶೂನ್ಯ print_usage (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *badoption) {
  lua_writestringerror("%s: ", progname);
  ಆದರೆ (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  ಇಲ್ಲದಿದ್ದರೆ
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
ಸ್ಥಿರ ಶೂನ್ಯ l_ಸಂದೇಶ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  ಆದರೆ (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", ಸಂದೇಶ);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ report (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2) {
  ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = lua_tostring(L, -1);
    ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ)
      ಸಂದೇಶ = "(error message not a string)";
    l_ಸಂದೇಶ(progname, ಸಂದೇಶ);
    lua_ತೆಗೆ(L, 1);  /* remove message */
  }
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/*
** Message handler used to run all chunks
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ msghandler (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = lua_tostring(L, 1);
  ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ) {  /* is error object not a string? */
    ಆದರೆ (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      ಹಿಂತಿರುಗಿ 1;  /* that is the message */
    ಇಲ್ಲದಿದ್ದರೆ
      ಸಂದೇಶ = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, ಸಂದೇಶ, 1);  /* append a standard traceback */
  ಹಿಂತಿರುಗಿ 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ docall (lua_State *L, ಪೂರ್ಣಾಂಕ narg, ಪೂರ್ಣಾಂಕ nres) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಪೂರ್ಣಾಂಕ ತಳ = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_ಸೇರಿಸು(L, ತಳ);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  setsignal(SIGINT, laction);  /* set C-signal handler */
  ಸ್ಥಿತಿ_2 = lua_pcall(L, narg, nres, ತಳ);
  setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, ತಳ);  /* remove message handler from the stack */
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


ಸ್ಥಿರ ಶೂನ್ಯ print_ಆವೃತ್ತಿ (ಶೂನ್ಯ) {
  lua_writestring(LUA_COPYRIGHT, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(LUA_COPYRIGHT));
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
ಸ್ಥಿರ ಶೂನ್ಯ createargtable (lua_State *L, ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ argc, ಪೂರ್ಣಾಂಕ script) {
  ಪೂರ್ಣಾಂಕ i, narg;
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  ಗಾಗಿ (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dochunk (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2) {
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) ಸ್ಥಿತಿ_2 = docall(L, 0, 0);
  ಹಿಂತಿರುಗಿ report(L, ಸ್ಥಿತಿ_2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dofile (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  ಹಿಂತಿರುಗಿ dochunk(L, luaL_loadfilex(L, name, "bt"));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dostring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  ಹಿಂತಿರುಗಿ dochunk(L, luaL_loadbufferx(L, s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s), name, "t"));
}


/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
** If there is no explicit modname and globname contains a '-', cut
** the suffix after '-' (the "version") to make the global name.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dolibrary (lua_State *L, ಅಕ್ಷರ *globname) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಅಕ್ಷರ *suffix = ಖಾಲಿ;
  ಅಕ್ಷರ *modname = strchr(globname, '=');
  ಆದರೆ (modname == ಖಾಲಿ) {  /* no explicit name? */
    modname = globname;  /* module name is equal to global name */
    suffix = strchr(modname, *LUA_IGMARK);  /* look for a suffix mark */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    *modname = '\0';  /* global name ends here */
    modname++;  /* module name starts after the '=' */
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  ಸ್ಥಿತಿ_2 = docall(L, 1, 1);  /* call 'require(modname)' */
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) {
    ಆದರೆ (suffix != ಖಾಲಿ)  /* is there a suffix mark? */
      *suffix = '\0';  /* remove suffix from global name */
    lua_setglobal(L, globname);  /* globname = require(modname) */
  }
  ಹಿಂತಿರುಗಿ report(L, ಸ್ಥಿತಿ_2);
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pushargs (lua_State *L) {
  ಪೂರ್ಣಾಂಕ i, n;
  ಆದರೆ (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (ಪೂರ್ಣಾಂಕ)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  ಗಾಗಿ (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  ಹಿಂತಿರುಗಿ n;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_script (lua_State *L, ಅಕ್ಷರ **argv) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname = argv[0];
  ಆದರೆ (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = ಖಾಲಿ;  /* stdin */
  ಸ್ಥಿತಿ_2 = luaL_loadfilex(L, fname, "bt");
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) {
    ಪೂರ್ಣಾಂಕ n = pushargs(L);  /* push arguments to script */
    ಸ್ಥಿತಿ_2 = docall(L, n, LUA_MULTRET);
  }
  ಹಿಂತಿರುಗಿ report(L, ಸ್ಥಿತಿ_2);
}


/* bits of various argument indicators in 'args' */
#ವ್ಯಾಖ್ಯಾನಿಸು has_error	1	/* bad option */
#ವ್ಯಾಖ್ಯಾನಿಸು has_i		2	/* -i */
#ವ್ಯಾಖ್ಯಾನಿಸು has_v		4	/* -v */
#ವ್ಯಾಖ್ಯಾನಿಸು has_e		8	/* -e */
#ವ್ಯಾಖ್ಯಾನಿಸು has_E		16	/* -E */


/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code or an error code if it finds any
** invalid argument. In case of error, 'first' is the index of the bad
** argument.  Otherwise, 'first' is -1 if there is no program name,
** 0 if there is no script name, or the index of the script name.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ collectargs (ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ *ಮೊದಲ) {
  ಪೂರ್ಣಾಂಕ ವಾದಗಳು = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ (argv[0] != ಖಾಲಿ) {  /* is there a program name? */
    ಆದರೆ (argv[0][0])  /* not empty? */
      progname = argv[0];  /* save it */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* no program name */
    *ಮೊದಲ = -1;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ (i = 1; argv[i] != ಖಾಲಿ; i++) {  /* handle arguments */
    *ಮೊದಲ = i;
    ಆದರೆ (argv[i][0] != '-')  /* not an option? */
        ಹಿಂತಿರುಗಿ ವಾದಗಳು;  /* stop handling options */
    ಆಯ್ಕೆ (argv[i][1]) {  /* else check option */
      ಸ್ಥಿತಿ '-':  /* '--' */
        ಆದರೆ (argv[i][2] != '\0')  /* extra characters after '--'? */
          ಹಿಂತಿರುಗಿ has_error;  /* invalid option */
        /* if there is a script name, it comes after '--' */
        *ಮೊದಲ = (argv[i + 1] != ಖಾಲಿ) ? i + 1 : 0;
        ಹಿಂತಿರುಗಿ ವಾದಗಳು;
      ಸ್ಥಿತಿ '\0':  /* '-' */
        ಹಿಂತಿರುಗಿ ವಾದಗಳು;  /* script "name" is '-' */
      ಸ್ಥಿತಿ 'E':
        ಆದರೆ (argv[i][2] != '\0')  /* extra characters? */
          ಹಿಂತಿರುಗಿ has_error;  /* invalid option */
        ವಾದಗಳು |= has_E;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 'W':
        ಆದರೆ (argv[i][2] != '\0')  /* extra characters? */
          ಹಿಂತಿರುಗಿ has_error;  /* invalid option */
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 'i':
        ವಾದಗಳು |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      ಸ್ಥಿತಿ 'v':
        ಆದರೆ (argv[i][2] != '\0')  /* extra characters? */
          ಹಿಂತಿರುಗಿ has_error;  /* invalid option */
        ವಾದಗಳು |= has_v;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 'e':
        ವಾದಗಳು |= has_e;  /* FALLTHROUGH */
      ಸ್ಥಿತಿ 'l':  /* both options need an argument */
        ಆದರೆ (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          ಆದರೆ (argv[i] == ಖಾಲಿ || argv[i][0] == '-')
            ಹಿಂತಿರುಗಿ has_error;  /* no next argument or it is another option */
        }
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:  /* invalid option */
        ಹಿಂತಿರುಗಿ has_error;
    }
  }
  *ಮೊದಲ = 0;  /* no script name */
  ಹಿಂತಿರುಗಿ ವಾದಗಳು;
}


/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ runargs (lua_State *L, ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ i;
  lua_warning(L, "@off", 0);  /* by default, Lua stand-alone has warnings off */
  ಗಾಗಿ (i = 1; i < n; i++) {
    ಪೂರ್ಣಾಂಕ option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    ಆಯ್ಕೆ (option) {
      ಸ್ಥಿತಿ 'e':  ಸ್ಥಿತಿ 'l': {
        ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
        ಅಕ್ಷರ *extra = argv[i] + 2;  /* both options need an argument */
        ಆದರೆ (*extra == '\0') extra = argv[++i];
        lua_assert(extra != ಖಾಲಿ);
        ಸ್ಥಿತಿ_2 = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK) ಹಿಂತಿರುಗಿ 0;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'W':
        lua_warning(L, "@on", 0);  /* warnings on */
        ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಅಕ್ಷರ *(*l_getenv)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);

/* Function to ignore environment variables, used by option -E */
ಸ್ಥಿರ ಅಕ್ಷರ *no_getenv (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  ಬಳಸದ(name);
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_luainit (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = "=" LUA_INITVARVERSION;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *init = l_getenv(name + 1);
  ಆದರೆ (init == ಖಾಲಿ) {
    name = "=" LUA_INIT_VAR;
    init = l_getenv(name + 1);  /* try alternative name */
  }
  ಆದರೆ (init == ಖಾಲಿ) ಹಿಂತಿರುಗಿ LUA_OK;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (init[0] == '@')
    ಹಿಂತಿರುಗಿ dofile(L, init+1);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ dostring(L, init, name);
}


/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/

#ಆದರೆ !defined(LUA_PROMPT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_PROMPT		"> "
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_PROMPT2		">> "
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_MAXINPUT)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXINPUT		512
#ಆದರೆ_ಅಂತ್ಯ


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#ಆದರೆ !defined(lua_stdin_is_tty)	/* { */

#ಆದರೆ defined(LUA_USE_POSIX)	/* { */

#ಅನ್ತರ್ಭಾವ <unistd.h>
#ವ್ಯಾಖ್ಯಾನಿಸು lua_stdin_is_tty()	isatty(0)

#ಅಥವಾಆದರೆ defined(LUA_USE_WINDOWS)	/* }{ */

#ಅನ್ತರ್ಭಾವ <io.h>
#ಅನ್ತರ್ಭಾವ <windows.h>

#ವ್ಯಾಖ್ಯಾನಿಸು lua_stdin_is_tty()	_isatty(_fileno(stdin))

#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */

/* ISO C definition */
#ವ್ಯಾಖ್ಯಾನಿಸು lua_stdin_is_tty()	1  /* assume stdin is a tty */

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** * lua_initreadline initializes the readline system.
** * lua_readline defines how to show a prompt and then read a line from
**   the standard input.
** * lua_saveline defines how to "save" a read line in a "history".
** * lua_freeline defines how to free a line read by lua_readline.
*/

#ಆದರೆ !defined(lua_readline)	/* { */
/* Otherwise, all previously listed functions should be defined. */

#ಆದರೆ defined(LUA_USE_READLINE)	/* { */
/* Lua will be linked with '-lreadline' */

#ಅನ್ತರ್ಭಾವ <readline/readline.h>
#ಅನ್ತರ್ಭಾವ <readline/history.h>

#ವ್ಯಾಖ್ಯಾನಿಸು lua_initreadline(L)  \
	((ಶೂನ್ಯ)L, *cast(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **, &rl_readline_name)="lua")
#ವ್ಯಾಖ್ಯಾನಿಸು lua_readline(buff,prompt)	((ಶೂನ್ಯ)buff, readline(prompt))
#ವ್ಯಾಖ್ಯಾನಿಸು lua_saveline(ಸಾಲು)	ಕೂಡಿಸು_history(ಸಾಲು)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_freeline(ಸಾಲು)	ಬಿಡುಗಡೆ(ಸಾಲು)

#ಇಲ್ಲದಿದ್ದರೆ		/* }{ */
/* use dynamically loaded readline (or nothing) */

/* pointer to 'readline' function (if any) */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ *(*l_readlineT) (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt);
ಸ್ಥಿರ l_readlineT l_readline = ಖಾಲಿ;

/* pointer to 'add_history' function (if any) */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*l_addhistT) (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string);
ಸ್ಥಿರ l_addhistT l_addhist = ಖಾಲಿ;


ಸ್ಥಿರ ಅಕ್ಷರ *lua_readline (ಅಕ್ಷರ *buff, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt) {
  ಆದರೆ (l_readline != ಖಾಲಿ)  /* is there a 'readline'? */
    ಹಿಂತಿರುಗಿ (*l_readline)(prompt);  /* use it */
  ಇಲ್ಲದಿದ್ದರೆ {  /* emulate 'readline' over 'buff' */
    fputs(prompt, stdout);
    fflush(stdout);  /* show prompt */
    ಹಿಂತಿರುಗಿ fgets(buff, LUA_MAXINPUT, stdin);  /* read line */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ lua_saveline (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು) {
  ಆದರೆ (l_addhist != ಖಾಲಿ)  /* is there an 'add_history'? */
    (*l_addhist)(ಸಾಲು);  /* use it */
  /* else nothing to be done */
}


ಸ್ಥಿರ ಶೂನ್ಯ lua_freeline (ಅಕ್ಷರ *ಸಾಲು) {
  ಆದರೆ (l_readline != ಖಾಲಿ)  /* is there a 'readline'? */
    ಬಿಡುಗಡೆ(ಸಾಲು);  /* free line created by it */
  /* else 'lua_readline' used an automatic buffer; nothing to free */
}


#ಆದರೆ defined(LUA_USE_DLOPEN) && defined(LUA_READLINELIB)		/* { */
/* try to load 'readline' dynamically */

#ಅನ್ತರ್ಭಾವ <dlfcn.h>

ಸ್ಥಿರ ಶೂನ್ಯ lua_initreadline (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rllib = l_getenv(LUA_RLLIB_VAR);  /* name of readline library */
  ಶೂನ್ಯ *lib;  /* library handle */
  ಆದರೆ (rllib == ಖಾಲಿ)  /* no environment variable? */
    rllib = LUA_READLINELIB;  /* use default name */
  lib = dlopen(rllib, RTLD_NOW | RTLD_LOCAL);
  ಆದರೆ (lib != ಖಾಲಿ) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name = cast(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**, dlsym(lib, "rl_readline_name"));
    ಆದರೆ (name != ಖಾಲಿ)
      *name = "lua";
    l_readline = cast(l_readlineT, cast_func(dlsym(lib, "readline")));
    l_addhist = cast(l_addhistT, cast_func(dlsym(lib, "add_history")));
    ಆದರೆ (l_readline != ಖಾಲಿ)  /* could load readline function? */
      ಹಿಂತಿರುಗಿ;  /* everything ok */
    /* else emit a warning */
  }
  lua_warning(L, "unable to load readline library '", 1);
  lua_warning(L, rllib, 1);
  lua_warning(L, "'", 0);
}

#ಇಲ್ಲದಿದ್ದರೆ		/* }{ */
/* no dlopen or LUA_READLINELIB undefined */

/* Leave pointers with NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು lua_initreadline(L)	((ಶೂನ್ಯ)L)

#ಆದರೆ_ಅಂತ್ಯ		/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** Return the string to be used as a prompt by the interpreter. Leave
** the string (or nil, if using the default value) on the stack, to keep
** it anchored.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_prompt (lua_State *L, ಪೂರ್ಣಾಂಕ firstline) {
  ಆದರೆ (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
    ಹಿಂತಿರುಗಿ (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
  ಇಲ್ಲದಿದ್ದರೆ {  /* apply 'tostring' over the value */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = luaL_tolstring(L, -1, ಖಾಲಿ);
    lua_remove(L, -2);  /* remove original value */
    ಹಿಂತಿರುಗಿ p;
  }
}

/* mark in error messages for incomplete statements */
#ವ್ಯಾಖ್ಯಾನಿಸು EOFMARK		"<eof>"
#ವ್ಯಾಖ್ಯಾನಿಸು marklen		(ಗಾತ್ರ(EOFMARK)/ಗಾತ್ರ(ಅಕ್ಷರ) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ incomplete (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2) {
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_ERRSYNTAX) {
    size_t lmsg;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = lua_tolstring(L, -1, &lmsg);
    ಆದರೆ (lmsg >= marklen && strcmp(ಸಂದೇಶ + lmsg - marklen, EOFMARK) == 0)
      ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pushline (lua_State *L, ಪೂರ್ಣಾಂಕ firstline) {
  ಅಕ್ಷರ ಬಫರ್[LUA_MAXINPUT];
  size_t l;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prmt = get_prompt(L, firstline);
  ಅಕ್ಷರ *b = lua_readline(ಬಫರ್, prmt);
  lua_ತೆಗೆ(L, 1);  /* remove prompt */
  ಆದರೆ (b == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;  /* no input */
  l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b);
  ಆದರೆ (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  lua_pushlstring(L, b, l);
  lua_freeline(b);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ addreturn (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು = lua_tostring(L, -1);  /* original line */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *retline = lua_pushfstring(L, "return %s;", ಸಾಲು);
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = luaL_loadbufferx(L, retline, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(retline), "=stdin", "t");
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK)
    lua_remove(L, -2);  /* remove modified line */
  ಇಲ್ಲದಿದ್ದರೆ
    lua_ತೆಗೆ(L, 2);  /* pop result from 'luaL_loadbufferx' and modified line */
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


ಸ್ಥಿರ ಶೂನ್ಯ checklocal (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ size_t szloc = ಗಾತ್ರ("local") - 1;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ space[] = " \t";
  ಸಾಲು += strspn(ಸಾಲು, space);  /* skip spaces */
  ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ಸಾಲು, "local", szloc) == 0 &&  /* "local"? */
      strchr(space, *(ಸಾಲು + szloc)) != ಖಾಲಿ) {  /* followed by a space? */
    lua_writestringerror("%s\n",
      "warning: locals do not survive across lines in interactive mode");
  }
}


/*
** Read multiple lines until a complete Lua statement or an error not
** for an incomplete statement. Start with first line already read in
** the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiline (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು = lua_tolstring(L, 1, &len);  /* get first line */
  checklocal(ಸಾಲು);
  ಗಾಗಿ (;;) {  /* repeat until gets a complete statement */
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = luaL_loadbufferx(L, ಸಾಲು, len, "=stdin", "t");  /* try it */
    ಆದರೆ (!incomplete(L, ಸ್ಥಿತಿ_2) || !pushline(L, 0))
      ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;  /* should not or cannot try to add continuation line */
    lua_remove(L, -2);  /* remove error message (from incomplete line) */
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_ಸೇರಿಸು(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
    ಸಾಲು = lua_tolstring(L, 1, &len);  /* get what is has */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loadline (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು;
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  lua_settop(L, 0);
  ಆದರೆ (!pushline(L, 1))
    ಹಿಂತಿರುಗಿ -1;  /* no input */
  ಆದರೆ ((ಸ್ಥಿತಿ_2 = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    ಸ್ಥಿತಿ_2 = multiline(L);  /* try as command, maybe with continuation lines */
  ಸಾಲು = lua_tostring(L, 1);
  ಆದರೆ (ಸಾಲು[0] != '\0')  /* non empty? */
    lua_saveline(ಸಾಲು);  /* keep history */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
ಸ್ಥಿರ ಶೂನ್ಯ l_print (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);
  ಆದರೆ (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_ಸೇರಿಸು(L, 1);
    ಆದರೆ (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_ಸಂದೇಶ(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
ಸ್ಥಿರ ಶೂನ್ಯ doREPL (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oldprogname = progname;
  progname = ಖಾಲಿ;  /* no 'progname' on errors in interactive mode */
  lua_initreadline(L);
  ಆಗ ((ಸ್ಥಿತಿ_2 = loadline(L)) != -1) {
    ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK)
      ಸ್ಥಿತಿ_2 = docall(L, 0, LUA_MULTRET);
    ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) l_print(L);
    ಇಲ್ಲದಿದ್ದರೆ report(L, ಸ್ಥಿತಿ_2);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}

/* }================================================================== */

#ಆದರೆ !defined(luai_openlibs)
#ಆದರೆ defined(LUA_NODEBUGLIB)
/* With this option, code must require the debug library before using it */
#ವ್ಯಾಖ್ಯಾನಿಸು luai_openlibs(L)  luaL_openselectedlibs(L, ~LUA_DBLIBK, LUA_DBLIBK)
#ಇಲ್ಲದಿದ್ದರೆ
/* The default is to open all standard libraries */
#ವ್ಯಾಖ್ಯಾನಿಸು luai_openlibs(L)  luaL_openselectedlibs(L, ~0, 0)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pmain (lua_State *L) {
  ಪೂರ್ಣಾಂಕ argc = (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, 1);
  ಅಕ್ಷರ **argv = (ಅಕ್ಷರ **)lua_touserdata(L, 2);
  ಪೂರ್ಣಾಂಕ script;
  ಪೂರ್ಣಾಂಕ ವಾದಗಳು = collectargs(argv, &script);
  ಪೂರ್ಣಾಂಕ optlim = (script > 0) ? script : argc; /* first argv not an option */
  luaL_checkversion(L);  /* check that interpreter has correct version */
  ಆದರೆ (ವಾದಗಳು == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ (ವಾದಗಳು & has_v)  /* option '-v'? */
    print_ಆವೃತ್ತಿ();
  ಆದರೆ (ವಾದಗಳು & has_E) {  /* option '-E'? */
    l_getenv = &no_getenv;  /* program will ignore environment variables */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  ಇಲ್ಲದಿದ್ದರೆ
    l_getenv = &getenv;
  luai_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  lua_gc(L, LUA_GCRESTART);  /* start GC... */
  lua_gc(L, LUA_GCGEN);  /* ...in generational mode */
  ಆದರೆ (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
    ಹಿಂತಿರುಗಿ 0;  /* error running LUA_INIT */
  ಆದರೆ (!runargs(L, argv, optlim))  /* execute arguments -e, -l, and -W */
    ಹಿಂತಿರುಗಿ 0;  /* something failed */
  ಆದರೆ (script > 0) {  /* execute main script (if there is one) */
    ಆದರೆ (handle_script(L, argv + script) != LUA_OK)
      ಹಿಂತಿರುಗಿ 0;  /* interrupt in case of error */
  }
  ಆದರೆ (ವಾದಗಳು & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (script < 1 && !(ವಾದಗಳು & (has_e | has_v))) { /* no active option? */
    ಆದರೆ (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_ಆವೃತ್ತಿ();
      doREPL(L);  /* do read-eval-print loop */
    }
    ಇಲ್ಲದಿದ್ದರೆ dofile(L, ಖಾಲಿ);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  ಹಿಂತಿರುಗಿ 1;
}


ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ (ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, ಫಲಿತಾಂಶ;
  lua_State *L = luaL_newstate();  /* create state */
  ಆದರೆ (L == ಖಾಲಿ) {
    l_ಸಂದೇಶ(argv[0], "cannot create state: not enough memory");
    ಹಿಂತಿರುಗಿ EXIT_FAILURE;
  }
  lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  lua_pushinteger(L, argc);  /* 1st argument */
  lua_pushlightuserdata(L, argv); /* 2nd argument */
  ಸ್ಥಿತಿ_2 = lua_pcall(L, 2, 1, 0);  /* do the call */
  ಫಲಿತಾಂಶ = lua_toboolean(L, -1);  /* get result */
  report(L, ಸ್ಥಿತಿ_2);
  lua_close(L);
  ಹಿಂತಿರುಗಿ (ಫಲಿತಾಂಶ && ಸ್ಥಿತಿ_2 == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

