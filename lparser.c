/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lcode.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "llex.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lparser.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"



/* maximum number of variable declarations per function (must be
   smaller than 250, due to the bytecode format) */
#define MAXVARS		200


#define hasmultret(k)		((k) == VCALL || (k) == VVARARG)


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#define eqstr(a,b)	((a) == (b))


/*
** nodes for block list (list of active blocks)
*/
തരനിർവചനം ഘടന BlockCnt {
  ഘടന BlockCnt *previous;  /* chain */
  പൂർണ്ണസംഖ്യ firstlabel;  /* index of first label in this block */
  പൂർണ്ണസംഖ്യ firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
സ്ഥിതം ശൂന്യം statement (LexState *ls);
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം (LexState *ls, expdesc *v);


സ്ഥിതം l_noret error_expected (LexState *ls, പൂർണ്ണസംഖ്യ token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


സ്ഥിതം l_noret errorlimit (FuncState *fs, പൂർണ്ണസംഖ്യ limit, സ്ഥിരം അക്ഷരം *what) {
  lua_State *L = fs->ls->L;
  സ്ഥിരം അക്ഷരം *സന്ദേശം;
  പൂർണ്ണസംഖ്യ വരി = fs->f->linedefined;
  സ്ഥിരം അക്ഷരം *where = (വരി == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", വരി);
  സന്ദേശം = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, സന്ദേശം);
}


ശൂന്യം luaY_checklimit (FuncState *fs, പൂർണ്ണസംഖ്യ v, പൂർണ്ണസംഖ്യ l, സ്ഥിരം അക്ഷരം *what) {
  എങ്കിൽ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ testnext (LexState *ls, പൂർണ്ണസംഖ്യ c) {
  എങ്കിൽ (ls->t.token == c) {
    luaX_next(ls);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ തിരികെ 0;
}


/*
** Check that next token is 'c'.
*/
സ്ഥിതം ശൂന്യം check (LexState *ls, പൂർണ്ണസംഖ്യ c) {
  എങ്കിൽ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
സ്ഥിതം ശൂന്യം checknext (LexState *ls, പൂർണ്ണസംഖ്യ c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,സന്ദേശം)	{ എങ്കിൽ (!(c)) luaX_syntaxerror(ls, സന്ദേശം); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
സ്ഥിതം ശൂന്യം check_match (LexState *ls, പൂർണ്ണസംഖ്യ what, പൂർണ്ണസംഖ്യ who, പൂർണ്ണസംഖ്യ where) {
  എങ്കിൽ (l_unlikely(!testnext(ls, what))) {
    എങ്കിൽ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    അല്ലെങ്കിൽ {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


സ്ഥിതം TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  തിരികെ ts;
}


സ്ഥിതം ശൂന്യം init_exp (expdesc *e, expkind k, പൂർണ്ണസംഖ്യ i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


സ്ഥിതം ശൂന്യം codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


സ്ഥിതം ശൂന്യം codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
സ്ഥിതം short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  പൂർണ്ണസംഖ്യ oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ആകുമ്പോൾ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ശൂന്യ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  തിരികെ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  തിരികെ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ new_localvar (LexState *ls, TString *name) {
  തിരികെ new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (വലുപ്പം(v)/വലുപ്പം(അക്ഷരം)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
സ്ഥിതം Vardesc *getlocalvardesc (FuncState *fs, പൂർണ്ണസംഖ്യ vidx) {
  തിരികെ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
സ്ഥിതം lu_byte reglevel (FuncState *fs, പൂർണ്ണസംഖ്യ nvar) {
  ആകുമ്പോൾ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    എങ്കിൽ (varinreg(vd))  /* is in a register? */
      തിരികെ cast_byte(vd->vd.ridx + 1);
  }
  തിരികെ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  തിരികെ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
സ്ഥിതം LocVar *localdebuginfo (FuncState *fs, പൂർണ്ണസംഖ്യ vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  എങ്കിൽ (!varinreg(vd))
    തിരികെ ശൂന്യ;  /* no debug info. for constants */
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    തിരികെ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
സ്ഥിതം ശൂന്യം init_var (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ vidx) {
  e->f = e->t = NO_JUMP;
  e->k = VLOCAL;
  e->u.var.vidx = cast_short(vidx);
  e->u.var.ridx = getlocalvardesc(fs, vidx)->vd.ridx;
}


/*
** Raises an error if variable described by 'e' is read only; moreover,
** if 'e' is t[exp] where t is the vararg parameter, change it to index
** a real table. (Virtual vararg tables cannot be changed.)
*/
സ്ഥിതം ശൂന്യം check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ശൂന്യ;  /* to be set if variable is const */
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      നിർത്തുക;
    }
    അവസ്ഥ VLOCAL: അവസ്ഥ VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      എങ്കിൽ (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      നിർത്തുക;
    }
    അവസ്ഥ VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      എങ്കിൽ (up->kind != VDKREG)
        varname = up->name;
      നിർത്തുക;
    }
    അവസ്ഥ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    അവസ്ഥ VINDEXUP: അവസ്ഥ VINDEXSTR: അവസ്ഥ VINDEXED: {  /* global variable */
      എങ്കിൽ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      തിരികെ;  /* integer index cannot be read-only */
  }
  എങ്കിൽ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
സ്ഥിതം ശൂന്യം adjustlocalvars (LexState *ls, പൂർണ്ണസംഖ്യ nvars) {
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ reglevel = luaY_nvarstack(fs);
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < nvars; i++) {
    പൂർണ്ണസംഖ്യ vidx = fs->nactvar++;
    Vardesc *var = getlocalvardesc(fs, vidx);
    var->vd.ridx = cast_byte(reglevel++);
    var->vd.pidx = registerlocalvar(ls, fs, var->vd.name);
    luaY_checklimit(fs, reglevel, MAXVARS, "local variables");
  }
}


/*
** Close the scope for all variables up to level 'tolevel'.
** (debug info.)
*/
സ്ഥിതം ശൂന്യം removevars (FuncState *fs, പൂർണ്ണസംഖ്യ tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ആകുമ്പോൾ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    എങ്കിൽ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ searchupvalue (FuncState *fs, TString *name) {
  പൂർണ്ണസംഖ്യ i;
  Upvaldesc *up = fs->f->upvalues;
  വേണ്ടി (i = 0; i < fs->nups; i++) {
    എങ്കിൽ (eqstr(up[i].name, name)) തിരികെ i;
  }
  തിരികെ -1;  /* not found */
}


സ്ഥിതം Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  പൂർണ്ണസംഖ്യ oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ആകുമ്പോൾ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ശൂന്യ;
  തിരികെ &f->upvalues[fs->nups++];
}


സ്ഥിതം പൂർണ്ണസംഖ്യ newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  എങ്കിൽ (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  അല്ലെങ്കിൽ {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  തിരികെ fs->nups - 1;
}


/*
** Look for an active variable with the name 'n' in the
** function 'fs'. If found, initialize 'var' with it and return
** its expression kind; otherwise return -1. While searching,
** var->u.info==-1 means that the preambular global declaration is
** active (the default while there is no other global declaration);
** var->u.info==-2 means there is no active collective declaration
** (some previous global declaration but no collective declaration);
** and var->u.info>=0 points to the inner-most (the first one found)
** collective declaration, if there is one.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ searchvar (FuncState *fs, TString *n, expdesc *var) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    എങ്കിൽ (varglobal(vd)) {  /* global declaration? */
      എങ്കിൽ (vd->vd.name == ശൂന്യ) {  /* collective declaration? */
        എങ്കിൽ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      അല്ലെങ്കിൽ {  /* global name */
        എങ്കിൽ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          തിരികെ VGLOBAL;
        }
        അല്ലെങ്കിൽ എങ്കിൽ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    അല്ലെങ്കിൽ എങ്കിൽ (eqstr(n, vd->vd.name)) {  /* found? */
      എങ്കിൽ (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      അല്ലെങ്കിൽ {  /* local variable */
        init_var(fs, var, i);
        എങ്കിൽ (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      തിരികെ cast_int(var->k);
    }
  }
  തിരികെ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
സ്ഥിതം ശൂന്യം markupval (FuncState *fs, പൂർണ്ണസംഖ്യ level) {
  BlockCnt *bl = fs->bl;
  ആകുമ്പോൾ (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
സ്ഥിതം ശൂന്യം marktobeclosed (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  bl->upval = 1;
  bl->insidetbc = 1;
  fs->needclose = 1;
}


/*
** Find a variable with the given name 'n'. If it is an upvalue, add
** this upvalue into all intermediate functions. If it is a global, set
** 'var' as 'void' as a flag.
*/
സ്ഥിതം ശൂന്യം singlevaraux (FuncState *fs, TString *n, expdesc *var, പൂർണ്ണസംഖ്യ base) {
  പൂർണ്ണസംഖ്യ v = searchvar(fs, n, var);  /* look up variables at current level */
  എങ്കിൽ (v >= 0) {  /* found? */
    എങ്കിൽ (!base) {
      എങ്കിൽ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      എങ്കിൽ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  അല്ലെങ്കിൽ {  /* not found at current level; try upvalues */
    പൂർണ്ണസംഖ്യ idx = searchupvalue(fs, n);  /* try existing upvalues */
    എങ്കിൽ (idx < 0) {  /* not found? */
      എങ്കിൽ (fs->prev != ശൂന്യ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      എങ്കിൽ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      അല്ലെങ്കിൽ  /* it is a global or a constant */
        തിരികെ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


സ്ഥിതം ശൂന്യം buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc താക്കോൽ;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  എങ്കിൽ (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&താക്കോൽ, varname);  /* key is variable name */
  luaK_indexed(fs, var, &താക്കോൽ);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
സ്ഥിതം ശൂന്യം buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  എങ്കിൽ (var->k == VGLOBAL) {  /* global name? */
    പൂർണ്ണസംഖ്യ info = var->u.info;
    /* global by default in the scope of a global declaration? */
    എങ്കിൽ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    എങ്കിൽ (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    അല്ലെങ്കിൽ  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


സ്ഥിതം ശൂന്യം singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
സ്ഥിതം ശൂന്യം adjust_assign (LexState *ls, പൂർണ്ണസംഖ്യ nvars, പൂർണ്ണസംഖ്യ nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  എങ്കിൽ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    പൂർണ്ണസംഖ്യ extra = needed + 1;  /* discount last expression itself */
    എങ്കിൽ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    എങ്കിൽ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  എങ്കിൽ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  അല്ലെങ്കിൽ  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
സ്ഥിതം l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  സ്ഥിരം അക്ഷരം *varname = (tsname != ശൂന്യ) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->വരി, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
സ്ഥിതം ശൂന്യം closegoto (LexState *ls, പൂർണ്ണസംഖ്യ g, Labeldesc *label, പൂർണ്ണസംഖ്യ bup) {
  പൂർണ്ണസംഖ്യ i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  എങ്കിൽ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  എങ്കിൽ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  വേണ്ടി (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
സ്ഥിതം Labeldesc *findlabel (LexState *ls, TString *name, പൂർണ്ണസംഖ്യ ilb) {
  Dyndata *dyd = ls->dyd;
  വേണ്ടി (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    എങ്കിൽ (eqstr(lb->name, name))  /* correct label? */
      തിരികെ lb;
  }
  തിരികെ ശൂന്യ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ newlabelentry (LexState *ls, Labellist *l, TString *name,
                          പൂർണ്ണസംഖ്യ വരി, പൂർണ്ണസംഖ്യ pc) {
  പൂർണ്ണസംഖ്യ n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].വരി = വരി;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  തിരികെ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ newgotoentry (LexState *ls, TString *name, പൂർണ്ണസംഖ്യ വരി) {
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  തിരികെ newlabelentry(ls, &ls->dyd->gt, name, വരി, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
സ്ഥിതം ശൂന്യം createlabel (LexState *ls, TString *name, പൂർണ്ണസംഖ്യ വരി, പൂർണ്ണസംഖ്യ last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  പൂർണ്ണസംഖ്യ l = newlabelentry(ls, ll, name, വരി, luaK_getlabel(fs));
  എങ്കിൽ (last) {  /* label is last no-op statement in the block? */
    /* assume that locals are already out of scope */
    ll->arr[l].nactvar = fs->bl->nactvar;
  }
}


/*
** Traverse the pending gotos of the finishing block checking whether
** each match some label of that block. Those that do not match are
** "exported" to the outer block, to be solved there. In particular,
** its 'nactvar' is updated with the level of the inner block,
** as the variables of the inner block are now out of scope.
*/
സ്ഥിതം ശൂന്യം solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  പൂർണ്ണസംഖ്യ outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  പൂർണ്ണസംഖ്യ igt = bl->firstgoto;  /* first goto in the finishing block */
  ആകുമ്പോൾ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    എങ്കിൽ (lb != ശൂന്യ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    അല്ലെങ്കിൽ {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      എങ്കിൽ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


സ്ഥിതം ശൂന്യം enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ശൂന്യ && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
സ്ഥിതം l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->വരി);
}


സ്ഥിതം ശൂന്യം leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  എങ്കിൽ (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  എങ്കിൽ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  എങ്കിൽ (bl->previous == ശൂന്യ) {  /* was it the last block? */
    എങ്കിൽ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
സ്ഥിതം Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  എങ്കിൽ (fs->np >= f->sizep) {
    പൂർണ്ണസംഖ്യ oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ആകുമ്പോൾ (oldsize < f->sizep)
      f->p[oldsize++] = ശൂന്യ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  തിരികെ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
സ്ഥിതം ശൂന്യം codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


സ്ഥിതം ശൂന്യം open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
  lua_State *L = ls->L;
  Proto *f = fs->f;
  fs->prev = ls->fs;  /* linked list of funcstates */
  fs->ls = ls;
  ls->fs = fs;
  fs->pc = 0;
  fs->previousline = f->linedefined;
  fs->iwthabs = 0;
  fs->lasttarget = 0;
  fs->freereg = 0;
  fs->nk = 0;
  fs->nabslineinfo = 0;
  fs->np = 0;
  fs->nups = 0;
  fs->ndebugvars = 0;
  fs->nactvar = 0;
  fs->needclose = 0;
  fs->firstlocal = ls->dyd->actvar.n;
  fs->firstlabel = ls->dyd->label.n;
  fs->bl = ശൂന്യ;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


സ്ഥിതം ശൂന്യം close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ശൂന്യ);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_വരി_Info);
  luaM_shrinkvector(L, f->k, f->sizek, fs->nk, TValue);
  luaM_shrinkvector(L, f->p, f->sizep, fs->np, Proto *);
  luaM_shrinkvector(L, f->locvars, f->sizelocvars, fs->ndebugvars, LocVar);
  luaM_shrinkvector(L, f->upvalues, f->sizeupvalues, fs->nups, Upvaldesc);
  /* remove kcache table from scanner table ("weigh" its anchor) */
  sethvalue(L, &temp, fs->kcache);  /* key to be set to nil */
  luaH_set(L, ls->h, &temp, &G(L)->nilvalue);
  ls->fs = fs->prev;
  luaC_checkGC(L);
}


/*
** {======================================================================
** GRAMMAR RULES
** =======================================================================
*/


/*
** check whether current token is in the follow set of a block.
** 'until' closes syntactical blocks, but do not close scope,
** so it is handled in separate.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ block_follow (LexState *ls, പൂർണ്ണസംഖ്യ withuntil) {
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ TK_ELSE: അവസ്ഥ TK_ELSEIF:
    അവസ്ഥ TK_END: അവസ്ഥ TK_EOS:
      തിരികെ 1;
    അവസ്ഥ TK_UNTIL: തിരികെ withuntil;
    സ്ഥിരസ്ഥിതി: തിരികെ 0;
  }
}


സ്ഥിതം ശൂന്യം statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ആകുമ്പോൾ (!block_follow(ls, 1)) {
    എങ്കിൽ (ls->t.token == TK_RETURN) {
      statement(ls);
      തിരികെ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


സ്ഥിതം ശൂന്യം fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc താക്കോൽ;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &താക്കോൽ);
  luaK_indexed(fs, v, &താക്കോൽ);
}


സ്ഥിതം ശൂന്യം yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  വ്യഞ്ജകം(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

തരനിർവചനം ഘടന ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  പൂർണ്ണസംഖ്യ nh;  /* total number of 'record' elements */
  പൂർണ്ണസംഖ്യ na;  /* number of array elements already stored */
  പൂർണ്ണസംഖ്യ tostore;  /* number of array elements pending to be stored */
  പൂർണ്ണസംഖ്യ maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#എങ്കിൽ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


സ്ഥിതം ശൂന്യം recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, താക്കോൽ, val;
  എങ്കിൽ (ls->t.token == TK_NAME)
    codename(ls, &താക്കോൽ);
  അല്ലെങ്കിൽ  /* ls->t.token == '[' */
    yindex(ls, &താക്കോൽ);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &താക്കോൽ);
  വ്യഞ്ജകം(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


സ്ഥിതം ശൂന്യം closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  എങ്കിൽ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


സ്ഥിതം ശൂന്യം lastlistfield (FuncState *fs, ConsControl *cc) {
  എങ്കിൽ (cc->tostore == 0) തിരികെ;
  എങ്കിൽ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


സ്ഥിതം ശൂന്യം listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  വ്യഞ്ജകം(ls, &cc->v);
  cc->tostore++;
}


സ്ഥിതം ശൂന്യം field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  തിരഞ്ഞെടുപ്പ്(ls->t.token) {
    അവസ്ഥ TK_NAME: {  /* may be 'listfield' or 'recfield' */
      എങ്കിൽ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      അല്ലെങ്കിൽ
        recfield(ls, cc);
      നിർത്തുക;
    }
    അവസ്ഥ '[': {
      recfield(ls, cc);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      listfield(ls, cc);
      നിർത്തുക;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ maxtostore (FuncState *fs) {
  പൂർണ്ണസംഖ്യ numfreeregs = MAX_FSTACK - fs->freereg;
  എങ്കിൽ (numfreeregs >= 160)  /* "lots" of registers? */
    തിരികെ numfreeregs / 5;  /* use up to 1/5 of them */
  അല്ലെങ്കിൽ എങ്കിൽ (numfreeregs >= 80)  /* still "enough" registers? */
    തിരികെ 10;  /* one 'SETLIST' instruction for each 10 values */
  അല്ലെങ്കിൽ  /* save registers for potential more nesting */
    തിരികെ 1;
}


സ്ഥിതം ശൂന്യം constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ വരി = ls->linenumber;
  പൂർണ്ണസംഖ്യ pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  ചെയ്യുക {
    എങ്കിൽ (ls->t.token == /*{*/ '}') നിർത്തുക;
    എങ്കിൽ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ആകുമ്പോൾ (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, വരി);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


സ്ഥിതം ശൂന്യം setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


സ്ഥിതം ശൂന്യം parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  പൂർണ്ണസംഖ്യ nparams = 0;
  പൂർണ്ണസംഖ്യ varargk = 0;
  എങ്കിൽ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    ചെയ്യുക {
      തിരഞ്ഞെടുപ്പ് (ls->t.token) {
        അവസ്ഥ TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          നിർത്തുക;
        }
        അവസ്ഥ TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          എങ്കിൽ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          അല്ലെങ്കിൽ
            new_localvarliteral(ls, "(vararg table)");
          നിർത്തുക;
        }
        സ്ഥിരസ്ഥിതി: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ആകുമ്പോൾ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  എങ്കിൽ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


സ്ഥിതം ശൂന്യം body (LexState *ls, expdesc *e, പൂർണ്ണസംഖ്യ ismethod, പൂർണ്ണസംഖ്യ വരി) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = വരി;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  എങ്കിൽ (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, വരി);
  codeclosure(ls, e);
  close_func(ls);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  പൂർണ്ണസംഖ്യ n = 1;  /* at least one expression */
  വ്യഞ്ജകം(ls, v);
  ആകുമ്പോൾ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    വ്യഞ്ജകം(ls, v);
    n++;
  }
  തിരികെ n;
}


സ്ഥിതം ശൂന്യം funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  പൂർണ്ണസംഖ്യ base, nparams;
  പൂർണ്ണസംഖ്യ വരി = ls->linenumber;
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      എങ്കിൽ (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      അല്ലെങ്കിൽ {
        explist(ls, &args);
        എങ്കിൽ (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', വരി);
      നിർത്തുക;
    }
    അവസ്ഥ '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      നിർത്തുക;
    }
    അവസ്ഥ TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      luaX_syntaxerror(ls, "function arguments expected");
      തിരികെ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  എങ്കിൽ (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  അല്ലെങ്കിൽ {
    എങ്കിൽ (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, വരി);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


സ്ഥിതം ശൂന്യം primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ '(': {
      പൂർണ്ണസംഖ്യ വരി = ls->linenumber;
      luaX_next(ls);
      വ്യഞ്ജകം(ls, v);
      check_match(ls, ')', '(', വരി);
      luaK_dischargevars(ls->fs, v);
      തിരികെ;
    }
    അവസ്ഥ TK_NAME: {
      singlevar(ls, v);
      തിരികെ;
    }
    സ്ഥിരസ്ഥിതി: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


സ്ഥിതം ശൂന്യം suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  വേണ്ടി (;;) {
    തിരഞ്ഞെടുപ്പ് (ls->t.token) {
      അവസ്ഥ '.': {  /* fieldsel */
        fieldsel(ls, v);
        നിർത്തുക;
      }
      അവസ്ഥ '[': {  /* '[' exp ']' */
        expdesc താക്കോൽ;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &താക്കോൽ);
        luaK_indexed(fs, v, &താക്കോൽ);
        നിർത്തുക;
      }
      അവസ്ഥ ':': {  /* ':' NAME funcargs */
        expdesc താക്കോൽ;
        luaX_next(ls);
        codename(ls, &താക്കോൽ);
        luaK_self(fs, v, &താക്കോൽ);
        funcargs(ls, v);
        നിർത്തുക;
      }
      അവസ്ഥ '(': അവസ്ഥ TK_STRING: അവസ്ഥ '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: തിരികെ;
    }
  }
}


സ്ഥിതം ശൂന്യം simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      നിർത്തുക;
    }
    അവസ്ഥ TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      നിർത്തുക;
    }
    അവസ്ഥ TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      നിർത്തുക;
    }
    അവസ്ഥ TK_NIL: {
      init_exp(v, VNIL, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRUE: {
      init_exp(v, VTRUE, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_FALSE: {
      init_exp(v, VFALSE, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      നിർത്തുക;
    }
    അവസ്ഥ '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      തിരികെ;
    }
    അവസ്ഥ TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      തിരികെ;
    }
    സ്ഥിരസ്ഥിതി: {
      suffixedexp(ls, v);
      തിരികെ;
    }
  }
  luaX_next(ls);
}


സ്ഥിതം UnOpr getunopr (പൂർണ്ണസംഖ്യ op) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ TK_NOT: തിരികെ OPR_NOT;
    അവസ്ഥ '-': തിരികെ OPR_MINUS;
    അവസ്ഥ '~': തിരികെ OPR_BNOT;
    അവസ്ഥ '#': തിരികെ OPR_LEN;
    സ്ഥിരസ്ഥിതി: തിരികെ OPR_NOUNOPR;
  }
}


സ്ഥിതം BinOpr getbinopr (പൂർണ്ണസംഖ്യ op) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ '+': തിരികെ OPR_ADD;
    അവസ്ഥ '-': തിരികെ OPR_SUB;
    അവസ്ഥ '*': തിരികെ OPR_MUL;
    അവസ്ഥ '%': തിരികെ OPR_MOD;
    അവസ്ഥ '^': തിരികെ OPR_POW;
    അവസ്ഥ '/': തിരികെ OPR_DIV;
    അവസ്ഥ TK_IDIV: തിരികെ OPR_IDIV;
    അവസ്ഥ '&': തിരികെ OPR_BAND;
    അവസ്ഥ '|': തിരികെ OPR_BOR;
    അവസ്ഥ '~': തിരികെ OPR_BXOR;
    അവസ്ഥ TK_SHL: തിരികെ OPR_SHL;
    അവസ്ഥ TK_SHR: തിരികെ OPR_SHR;
    അവസ്ഥ TK_CONCAT: തിരികെ OPR_CONCAT;
    അവസ്ഥ TK_NE: തിരികെ OPR_NE;
    അവസ്ഥ TK_EQ: തിരികെ OPR_EQ;
    അവസ്ഥ '<': തിരികെ OPR_LT;
    അവസ്ഥ TK_LE: തിരികെ OPR_LE;
    അവസ്ഥ '>': തിരികെ OPR_GT;
    അവസ്ഥ TK_GE: തിരികെ OPR_GE;
    അവസ്ഥ TK_AND: തിരികെ OPR_AND;
    അവസ്ഥ TK_OR: തിരികെ OPR_OR;
    സ്ഥിരസ്ഥിതി: തിരികെ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
സ്ഥിതം സ്ഥിരം ഘടന {
  lu_byte ഇടത്;  /* left priority for each binary operator */
  lu_byte വലത്; /* right priority */
} priority[] = {  /* ORDER OPR */
   {10, 10}, {10, 10},           /* '+' '-' */
   {11, 11}, {11, 11},           /* '*' '%' */
   {14, 13},                  /* '^' (right associative) */
   {11, 11}, {11, 11},           /* '/' '//' */
   {6, 6}, {4, 4}, {5, 5},   /* '&' '|' '~' */
   {7, 7}, {7, 7},           /* '<<' '>>' */
   {9, 8},                   /* '..' (right associative) */
   {3, 3}, {3, 3}, {3, 3},   /* ==, <, <= */
   {3, 3}, {3, 3}, {3, 3},   /* ~=, >, >= */
   {2, 2}, {1, 1}            /* and, or */
};

#define UNARY_PRIORITY	12  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
സ്ഥിതം BinOpr subexpr (LexState *ls, expdesc *v, പൂർണ്ണസംഖ്യ limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  എങ്കിൽ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    പൂർണ്ണസംഖ്യ വരി = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, വരി);
  }
  അല്ലെങ്കിൽ simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ആകുമ്പോൾ (op != OPR_NOBINOPR && priority[op].ഇടത് > limit) {
    expdesc v2;
    BinOpr nextop;
    പൂർണ്ണസംഖ്യ വരി = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].വലത്);
    luaK_posfix(ls->fs, op, v, &v2, വരി);
    op = nextop;
  }
  leavelevel(ls);
  തിരികെ op;  /* return first untreated operator */
}


സ്ഥിതം ശൂന്യം വ്യഞ്ജകം (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


സ്ഥിതം ശൂന്യം block (LexState *ls) {
  /* block -> statlist */
  FuncState *fs = ls->fs;
  BlockCnt bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
ഘടന LHS_assign {
  ഘടന LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
സ്ഥിതം ശൂന്യം check_conflict (LexState *ls, ഘടന LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  പൂർണ്ണസംഖ്യ conflict = 0;
  വേണ്ടി (; lh; lh = lh->prev) {  /* check all previous assignments */
    എങ്കിൽ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      എങ്കിൽ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        എങ്കിൽ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      അല്ലെങ്കിൽ {  /* table is a register */
        എങ്കിൽ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        എങ്കിൽ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  എങ്കിൽ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    എങ്കിൽ (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    അല്ലെങ്കിൽ
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
സ്ഥിതം ശൂന്യം storevartop (FuncState *fs, expdesc *var) {
  expdesc e;
  init_exp(&e, VNONRELOC, fs->freereg - 1);
  luaK_storevar(fs, var, &e);  /* will also free the top register */
}


/*
** Parse and compile a multiple assignment. The first "variable"
** (a 'suffixedexp') was already read by the caller.
**
** assignment -> suffixedexp restassign
** restassign -> ',' suffixedexp restassign | '=' explist
*/
സ്ഥിതം ശൂന്യം restassign (LexState *ls, ഘടന LHS_assign *lh, പൂർണ്ണസംഖ്യ nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  എങ്കിൽ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ഘടന LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    എങ്കിൽ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  അല്ലെങ്കിൽ {  /* restassign -> '=' explist */
    പൂർണ്ണസംഖ്യ nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    എങ്കിൽ (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    അല്ലെങ്കിൽ {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      തിരികെ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  വ്യഞ്ജകം(ls, &v);  /* read condition */
  എങ്കിൽ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  തിരികെ v.f;
}


സ്ഥിതം ശൂന്യം gotostat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, വരി);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
സ്ഥിതം ശൂന്യം breakstat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  വേണ്ടി (bl = ls->fs->bl; bl != ശൂന്യ; bl = bl->previous) {
    എങ്കിൽ (bl->isloop)  /* found one? */
      പോകുക ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, വരി);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
സ്ഥിതം ശൂന്യം checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  എങ്കിൽ (l_unlikely(lb != ശൂന്യ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->വരി);  /* error */
}


സ്ഥിതം ശൂന്യം labelstat (LexState *ls, TString *name, പൂർണ്ണസംഖ്യ വരി) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ആകുമ്പോൾ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, വരി, block_follow(ls, 0));
}


സ്ഥിതം ശൂന്യം whilestat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ whileinit;
  പൂർണ്ണസംഖ്യ condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, വരി);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


സ്ഥിതം ശൂന്യം repeatstat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* repeatstat -> REPEAT block UNTIL cond */
  പൂർണ്ണസംഖ്യ condexit;
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, വരി);
  condexit = cond(ls);  /* read condition (inside scope block) */
  എങ്കിൽ (bl2.upval) {  /* upvalues? */
    പൂർണ്ണസംഖ്യ exit = luaK_jump(fs);  /* normal exit must jump over fix */
    luaK_patchtohere(fs, condexit);  /* repetition must close upvalues */
    luaK_codeABC(fs, OP_CLOSE, reglevel(fs, bl2.nactvar), 0, 0);
    condexit = luaK_jump(fs);  /* repeat after closing upvalues */
    luaK_patchtohere(fs, exit);  /* normal exit comes to here */
  }
  luaK_patchlist(fs, condexit, repeat_init);  /* close the loop */
  leaveblock(fs);  /* finish scope */
  leaveblock(fs);  /* finish loop */
}


/*
** Read an expression and generate code to put its results in next
** stack slot.
**
*/
സ്ഥിതം ശൂന്യം exp1 (LexState *ls) {
  expdesc e;
  വ്യഞ്ജകം(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
സ്ഥിതം ശൂന്യം fixforjump (FuncState *fs, പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ dest, പൂർണ്ണസംഖ്യ back) {
  Instruction *jmp = &fs->f->code[pc];
  പൂർണ്ണസംഖ്യ offset = dest - (pc + 1);
  എങ്കിൽ (back)
    offset = -offset;
  എങ്കിൽ (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
സ്ഥിതം ശൂന്യം forbody (LexState *ls, പൂർണ്ണസംഖ്യ base, പൂർണ്ണസംഖ്യ വരി, പൂർണ്ണസംഖ്യ nvars, പൂർണ്ണസംഖ്യ isgen) {
  /* forbody -> DO block */
  സ്ഥിതം സ്ഥിരം OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  സ്ഥിതം സ്ഥിരം OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  എങ്കിൽ (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, വരി);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, വരി);
}


സ്ഥിതം ശൂന്യം fornum (LexState *ls, TString *varname, പൂർണ്ണസംഖ്യ വരി) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  എങ്കിൽ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  അല്ലെങ്കിൽ {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, വരി, 1, 0);
}


സ്ഥിതം ശൂന്യം forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  പൂർണ്ണസംഖ്യ nvars = 4;  /* function, state, closing, control */
  പൂർണ്ണസംഖ്യ വരി;
  പൂർണ്ണസംഖ്യ base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ആകുമ്പോൾ (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  വരി = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, വരി, nvars - 3, 1);
}


സ്ഥിതം ശൂന്യം forstat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ '=': fornum(ls, varname, വരി); നിർത്തുക;
    അവസ്ഥ ',': അവസ്ഥ TK_IN: forlist(ls, varname); നിർത്തുക;
    സ്ഥിരസ്ഥിതി: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, വരി);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


സ്ഥിതം ശൂന്യം test_then_block (LexState *ls, പൂർണ്ണസംഖ്യ *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  എങ്കിൽ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


സ്ഥിതം ശൂന്യം ifstat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  ആകുമ്പോൾ (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  എങ്കിൽ (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, വരി);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


സ്ഥിതം ശൂന്യം localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


സ്ഥിതം lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  എങ്കിൽ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    സ്ഥിരം അക്ഷരം *attr = getstr(ts);
    checknext(ls, '>');
    എങ്കിൽ (strcmp(attr, "const") == 0)
      തിരികെ RDKCONST;  /* read-only variable */
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(attr, "close") == 0)
      തിരികെ RDKTOCLOSE;  /* to-be-closed variable */
    അല്ലെങ്കിൽ
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  തിരികെ df;  /* return default value */
}


സ്ഥിതം ശൂന്യം checktoclose (FuncState *fs, പൂർണ്ണസംഖ്യ level) {
  എങ്കിൽ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


സ്ഥിതം ശൂന്യം localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  പൂർണ്ണസംഖ്യ vidx;  /* index of last variable */
  പൂർണ്ണസംഖ്യ nvars = 0;
  പൂർണ്ണസംഖ്യ nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  ചെയ്യുക {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    എങ്കിൽ (kind == RDKTOCLOSE) {  /* to-be-closed? */
      എങ്കിൽ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ആകുമ്പോൾ (testnext(ls, ','));
  എങ്കിൽ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  അല്ലെങ്കിൽ {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  എങ്കിൽ (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  അല്ലെങ്കിൽ {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


സ്ഥിതം lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  തിരഞ്ഞെടുപ്പ് (kind) {
    അവസ്ഥ RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      തിരികെ kind;  /* to avoid warnings */
    അവസ്ഥ RDKCONST:
      തിരികെ GDKCONST;  /* adjust kind for global variable */
    സ്ഥിരസ്ഥിതി:
      തിരികെ kind;
  }
}


സ്ഥിതം ശൂന്യം checkglobal (LexState *ls, TString *varname, പൂർണ്ണസംഖ്യ വരി) {
  FuncState *fs = ls->fs;
  expdesc var;
  പൂർണ്ണസംഖ്യ k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, വരി);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
സ്ഥിതം ശൂന്യം initglobal (LexState *ls, പൂർണ്ണസംഖ്യ nvars, പൂർണ്ണസംഖ്യ firstidx, പൂർണ്ണസംഖ്യ n,
                        പൂർണ്ണസംഖ്യ വരി) {
  എങ്കിൽ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    പൂർണ്ണസംഖ്യ nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  അല്ലെങ്കിൽ {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, വരി);
    leavelevel(ls);
    checkglobal(ls, varname, വരി);
    storevartop(fs, &var);
  }
}


സ്ഥിതം ശൂന്യം globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  പൂർണ്ണസംഖ്യ nvars = 0;
  പൂർണ്ണസംഖ്യ lastidx;  /* index of last registered variable */
  ചെയ്യുക {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } ആകുമ്പോൾ (testnext(ls, ','));
  എങ്കിൽ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


സ്ഥിതം ശൂന്യം globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  എങ്കിൽ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  അല്ലെങ്കിൽ {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ശൂന്യ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


സ്ഥിതം ശൂന്യം globalfunc (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, വരി);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, വരി);  /* definition "happens" in the first line */
}


സ്ഥിതം ശൂന്യം globalstatfunc (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  എങ്കിൽ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, വരി);
  അല്ലെങ്കിൽ
    globalstat(ls);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  പൂർണ്ണസംഖ്യ ismethod = 0;
  singlevar(ls, v);
  ആകുമ്പോൾ (ls->t.token == '.')
    fieldsel(ls, v);
  എങ്കിൽ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  തിരികെ ismethod;
}


സ്ഥിതം ശൂന്യം funcstat (LexState *ls, പൂർണ്ണസംഖ്യ വരി) {
  /* funcstat -> FUNCTION funcname body */
  പൂർണ്ണസംഖ്യ ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, വരി);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, വരി);  /* definition "happens" in the first line */
}


സ്ഥിതം ശൂന്യം exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ഘടന LHS_assign v;
  suffixedexp(ls, &v.v);
  എങ്കിൽ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ശൂന്യ;
    restassign(ls, &v, 1);
  }
  അല്ലെങ്കിൽ {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


സ്ഥിതം ശൂന്യം retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  പൂർണ്ണസംഖ്യ nret;  /* number of values being returned */
  പൂർണ്ണസംഖ്യ first = luaY_nvarstack(fs);  /* first slot to be returned */
  എങ്കിൽ (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  അല്ലെങ്കിൽ {
    nret = explist(ls, &e);  /* optional return values */
    എങ്കിൽ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      എങ്കിൽ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    അല്ലെങ്കിൽ {
      എങ്കിൽ (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      അല്ലെങ്കിൽ {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


സ്ഥിതം ശൂന്യം statement (LexState *ls) {
  പൂർണ്ണസംഖ്യ വരി = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  തിരഞ്ഞെടുപ്പ് (ls->t.token) {
    അവസ്ഥ ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      നിർത്തുക;
    }
    അവസ്ഥ TK_IF: {  /* stat -> ifstat */
      ifstat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_FOR: {  /* stat -> forstat */
      forstat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      എങ്കിൽ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      അല്ലെങ്കിൽ
        localstat(ls);
      നിർത്തുക;
    }
    അവസ്ഥ TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      നിർത്തുക;
    }
    അവസ്ഥ TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, വരി);
      നിർത്തുക;
    }
#എങ്കിൽ LUA_COMPAT_GLOBAL
    അവസ്ഥ TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      എങ്കിൽ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        പൂർണ്ണസംഖ്യ lk = luaX_lookahead(ls);
        എങ്കിൽ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, വരി);
          നിർത്തുക;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    സ്ഥിരസ്ഥിതി: {  /* stat -> func | assignment */
      exprstat(ls);
      നിർത്തുക;
    }
  }
  lua_assert(ls->fs->f->maxstacksize >= ls->fs->freereg &&
             ls->fs->freereg >= luaY_nvarstack(ls->fs));
  ls->fs->freereg = luaY_nvarstack(ls->fs);  /* free registers */
  leavelevel(ls);
}

/* }====================================================================== */

/* }====================================================================== */


/*
** compiles the main function, which is a regular vararg function with an
** upvalue named LUA_ENV
*/
സ്ഥിതം ശൂന്യം mainfunc (LexState *ls, FuncState *fs) {
  BlockCnt bl;
  Upvaldesc *env;
  open_func(ls, fs, &bl);
  setvararg(fs);  /* main function is always vararg */
  env = allocupvalue(fs);  /* ...set environment upvalue */
  env->instack = 1;
  env->idx = 0;
  env->kind = VDKREG;
  env->name = ls->envn;
  luaC_objbarrier(ls->L, fs->f, env->name);
  luaX_next(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, Table *anchor, Mbuffer *buff,
                       Dyndata *dyd, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ firstchar) {
  LexState lexstate;
  FuncState funcstate;
  LClosure *cl;
  lexstate.h = anchor;  /* table for scanner */
  cl = luaF_newLclosure(L, 1);  /* create main closure */
  luaD_anchorobj(L, anchor, obj2gco(cl));  /* anchor it in scanner table */
  funcstate.f = cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  funcstate.f->source = luaS_new(L, name);  /* create and anchor TString */
  luaC_objbarrier(L, funcstate.f, funcstate.f->source);
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, funcstate.f->source, firstchar);
  mainfunc(&lexstate, &funcstate);
  lua_assert(!funcstate.prev && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  തിരികെ cl;
}

