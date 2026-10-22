/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#నిర్వచించు lparser_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lcode.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "llex.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lparser.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"



/* maximum number of variable declarations per function (must be
   smaller than 250, due to the bytecode format) */
#నిర్వచించు MAXVARS		200


#నిర్వచించు hasmultret(k)		((k) == VCALL || (k) == VVARARG)


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#నిర్వచించు eqstr(a,b)	((a) == (b))


/*
** nodes for block list (list of active blocks)
*/
రకనిర్వచనం నిర్మాణం ఖండం_Cnt {
  నిర్మాణం ఖండం_Cnt *మునుపటి;  /* chain */
  పూర్ణాంకం firstlabel;  /* index of first label in this block */
  పూర్ణాంకం firstgoto;  /* index of first pending goto in this block */
  పొట్టి nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} ఖండం_Cnt;



/*
** prototypes for recursive non-terminal functions
*/
స్థిర శూన్య statement (LexState *ls);
స్థిర శూన్య సమాసం (LexState *ls, expdesc *v);


స్థిర l_noret error_expected (LexState *ls, పూర్ణాంకం token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


స్థిర l_noret errorlimit (FuncState *fs, పూర్ణాంకం హద్దు, స్థిరరాశి అక్షరం *what) {
  lua_State *L = fs->ls->L;
  స్థిరరాశి అక్షరం *సందేశం;
  పూర్ణాంకం పంక్తి = fs->f->linedefined;
  స్థిరరాశి అక్షరం *where = (పంక్తి == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", పంక్తి);
  సందేశం = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, హద్దు, where);
  luaX_syntaxerror(fs->ls, సందేశం);
}


శూన్య luaY_checklimit (FuncState *fs, పూర్ణాంకం v, పూర్ణాంకం l, స్థిరరాశి అక్షరం *what) {
  ఒకవేళ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
స్థిర పూర్ణాంకం testnext (LexState *ls, పూర్ణాంకం c) {
  ఒకవేళ (ls->t.token == c) {
    lua_X_తదుపరి(ls);
    తిరిగి 1;
  }
  లేకపోతే తిరిగి 0;
}


/*
** Check that next token is 'c'.
*/
స్థిర శూన్య check (LexState *ls, పూర్ణాంకం c) {
  ఒకవేళ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
స్థిర శూన్య checknext (LexState *ls, పూర్ణాంకం c) {
  check(ls, c);
  lua_X_తదుపరి(ls);
}


#నిర్వచించు check_condition(ls,c,సందేశం)	{ ఒకవేళ (!(c)) luaX_syntaxerror(ls, సందేశం); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
స్థిర శూన్య check_సరిపోలిక (LexState *ls, పూర్ణాంకం what, పూర్ణాంకం who, పూర్ణాంకం where) {
  ఒకవేళ (l_unlikely(!testnext(ls, what))) {
    ఒకవేళ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    లేకపోతే {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


స్థిర TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  lua_X_తదుపరి(ls);
  తిరిగి ts;
}


స్థిర శూన్య init_exp (expdesc *e, expkind k, పూర్ణాంకం i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


స్థిర శూన్య codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


స్థిర శూన్య codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
స్థిర పొట్టి registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  పూర్ణాంకం oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  అయితే (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ఖాళీ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  తిరిగి fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
స్థిర పూర్ణాంకం new_varkind (LexState *ls, TString *name, lu_byte రకం) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.కొలత, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.రకం = రకం;  /* default */
  var->vd.name = name;
  తిరిగి dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
స్థిర పూర్ణాంకం new_localvar (LexState *ls, TString *name) {
  తిరిగి new_varkind(ls, name, VDKREG);
}

#నిర్వచించు new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (పరిమాణం(v)/పరిమాణం(అక్షరం)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
స్థిర Vardesc *getlocalvardesc (FuncState *fs, పూర్ణాంకం vidx) {
  తిరిగి &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
స్థిర lu_byte reglevel (FuncState *fs, పూర్ణాంకం nvar) {
  అయితే (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ఒకవేళ (varinreg(vd))  /* is in a register? */
      తిరిగి cast_byte(vd->vd.ridx + 1);
  }
  తిరిగి 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  తిరిగి reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
స్థిర LocVar *localdebuginfo (FuncState *fs, పూర్ణాంకం vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ఒకవేళ (!varinreg(vd))
    తిరిగి ఖాళీ;  /* no debug info. for constants */
  లేకపోతే {
    పూర్ణాంకం idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    తిరిగి &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
స్థిర శూన్య init_var (FuncState *fs, expdesc *e, పూర్ణాంకం vidx) {
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
స్థిర శూన్య check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ఖాళీ;  /* to be set if variable is const */
  ఎంపిక (e->k) {
    స్థితి VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      ఆపు;
    }
    స్థితి VLOCAL: స్థితి VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ఒకవేళ (vardesc->vd.రకం != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      ఆపు;
    }
    స్థితి VUPVAL: {
      Upvaldesc *పైకి = &fs->f->upvalues[e->u.info];
      ఒకవేళ (పైకి->రకం != VDKREG)
        varname = పైకి->name;
      ఆపు;
    }
    స్థితి VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    స్థితి VINDEXUP: స్థితి VINDEXSTR: స్థితి VINDEXED: {  /* global variable */
      ఒకవేళ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      ఆపు;
    }
    అప్రమేయ:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      తిరిగి;  /* integer index cannot be read-only */
  }
  ఒకవేళ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
స్థిర శూన్య adjustlocalvars (LexState *ls, పూర్ణాంకం nvars) {
  FuncState *fs = ls->fs;
  పూర్ణాంకం reglevel = luaY_nvarstack(fs);
  పూర్ణాంకం i;
  కొరకు (i = 0; i < nvars; i++) {
    పూర్ణాంకం vidx = fs->nactvar++;
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
స్థిర శూన్య removevars (FuncState *fs, పూర్ణాంకం tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  అయితే (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ఒకవేళ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
స్థిర పూర్ణాంకం searchupvalue (FuncState *fs, TString *name) {
  పూర్ణాంకం i;
  Upvaldesc *పైకి = fs->f->upvalues;
  కొరకు (i = 0; i < fs->nups; i++) {
    ఒకవేళ (eqstr(పైకి[i].name, name)) తిరిగి i;
  }
  తిరిగి -1;  /* not found */
}


స్థిర Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  పూర్ణాంకం oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  అయితే (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ఖాళీ;
  తిరిగి &f->upvalues[fs->nups++];
}


స్థిర పూర్ణాంకం newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *పైకి = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ఒకవేళ (v->k == VLOCAL) {
    పైకి->instack = 1;
    పైకి->idx = v->u.var.ridx;
    పైకి->రకం = getlocalvardesc(prev, v->u.var.vidx)->vd.రకం;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  లేకపోతే {
    పైకి->instack = 0;
    పైకి->idx = cast_byte(v->u.info);
    పైకి->రకం = prev->f->upvalues[v->u.info].రకం;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  పైకి->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  తిరిగి fs->nups - 1;
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
స్థిర పూర్ణాంకం searchvar (FuncState *fs, TString *n, expdesc *var) {
  పూర్ణాంకం i;
  కొరకు (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ఒకవేళ (varglobal(vd)) {  /* global declaration? */
      ఒకవేళ (vd->vd.name == ఖాళీ) {  /* collective declaration? */
        ఒకవేళ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      లేకపోతే {  /* global name */
        ఒకవేళ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          తిరిగి VGLOBAL;
        }
        లేకపోతే ఒకవేళ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    లేకపోతే ఒకవేళ (eqstr(n, vd->vd.name)) {  /* found? */
      ఒకవేళ (vd->vd.రకం == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      లేకపోతే {  /* local variable */
        init_var(fs, var, i);
        ఒకవేళ (vd->vd.రకం == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      తిరిగి cast_int(var->k);
    }
  }
  తిరిగి -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
స్థిర శూన్య markupval (FuncState *fs, పూర్ణాంకం level) {
  ఖండం_Cnt *bl = fs->bl;
  అయితే (bl->nactvar > level)
    bl = bl->మునుపటి;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
స్థిర శూన్య marktobeclosed (FuncState *fs) {
  ఖండం_Cnt *bl = fs->bl;
  bl->upval = 1;
  bl->insidetbc = 1;
  fs->needclose = 1;
}


/*
** Find a variable with the given name 'n'. If it is an upvalue, add
** this upvalue into all intermediate functions. If it is a global, set
** 'var' as 'void' as a flag.
*/
స్థిర శూన్య singlevaraux (FuncState *fs, TString *n, expdesc *var, పూర్ణాంకం పునాది) {
  పూర్ణాంకం v = searchvar(fs, n, var);  /* look up variables at current level */
  ఒకవేళ (v >= 0) {  /* found? */
    ఒకవేళ (!పునాది) {
      ఒకవేళ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ఒకవేళ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  లేకపోతే {  /* not found at current level; try upvalues */
    పూర్ణాంకం idx = searchupvalue(fs, n);  /* try existing upvalues */
    ఒకవేళ (idx < 0) {  /* not found? */
      ఒకవేళ (fs->prev != ఖాళీ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ఒకవేళ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      లేకపోతే  /* it is a global or a constant */
        తిరిగి;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


స్థిర శూన్య buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc కీలకం;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ఒకవేళ (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&కీలకం, varname);  /* key is variable name */
  lua_K_సూచీకృత(fs, var, &కీలకం);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
స్థిర శూన్య buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ఒకవేళ (var->k == VGLOBAL) {  /* global name? */
    పూర్ణాంకం info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ఒకవేళ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ఒకవేళ (info != -1 && ls->dyd->actvar.arr[info].vd.రకం == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    లేకపోతే  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.రకం == GDKREG);
  }
}


స్థిర శూన్య singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
స్థిర శూన్య adjust_కేటాయించు (LexState *ls, పూర్ణాంకం nvars, పూర్ణాంకం nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  పూర్ణాంకం needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ఒకవేళ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    పూర్ణాంకం extra = needed + 1;  /* discount last expression itself */
    ఒకవేళ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  లేకపోతే {
    ఒకవేళ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ఒకవేళ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ఒకవేళ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  లేకపోతే  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#నిర్వచించు enterlevel(ls)	luaE_incCstack(ls->L)


#నిర్వచించు leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
స్థిర l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  స్థిరరాశి అక్షరం *varname = (tsname != ఖాళీ) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->పంక్తి, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
స్థిర శూన్య closegoto (LexState *ls, పూర్ణాంకం g, Labeldesc *label, పూర్ణాంకం bup) {
  పూర్ణాంకం i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ఒకవేళ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ఒకవేళ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->సంకేతం_2[gt->pc + 1] = fs->f->సంకేతం_2[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->సంకేతం_2[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  కొరకు (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
స్థిర Labeldesc *findlabel (LexState *ls, TString *name, పూర్ణాంకం ilb) {
  Dyndata *dyd = ls->dyd;
  కొరకు (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ఒకవేళ (eqstr(lb->name, name))  /* correct label? */
      తిరిగి lb;
  }
  తిరిగి ఖాళీ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
స్థిర పూర్ణాంకం newlabelentry (LexState *ls, Labellist *l, TString *name,
                          పూర్ణాంకం పంక్తి, పూర్ణాంకం pc) {
  పూర్ణాంకం n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->కొలత,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].పంక్తి = పంక్తి;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  తిరిగి n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
స్థిర పూర్ణాంకం newgotoentry (LexState *ls, TString *name, పూర్ణాంకం పంక్తి) {
  FuncState *fs = ls->fs;
  పూర్ణాంకం pc = luaK_jump(fs);  /* create jump */
  lua_K_సంకేతం_ABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  తిరిగి newlabelentry(ls, &ls->dyd->gt, name, పంక్తి, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
స్థిర శూన్య createlabel (LexState *ls, TString *name, పూర్ణాంకం పంక్తి, పూర్ణాంకం చివరి) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  పూర్ణాంకం l = newlabelentry(ls, ll, name, పంక్తి, luaK_getlabel(fs));
  ఒకవేళ (చివరి) {  /* label is last no-op statement in the block? */
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
స్థిర శూన్య solvegotos (FuncState *fs, ఖండం_Cnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  పూర్ణాంకం outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  పూర్ణాంకం igt = bl->firstgoto;  /* first goto in the finishing block */
  అయితే (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ఒకవేళ (lb != ఖాళీ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    లేకపోతే {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ఒకవేళ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


స్థిర శూన్య enterblock (FuncState *fs, ఖండం_Cnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ఖాళీ && fs->bl->insidetbc);
  bl->మునుపటి = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
స్థిర l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->పంక్తి);
}


స్థిర శూన్య leaveblock (FuncState *fs) {
  ఖండం_Cnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ఒకవేళ (bl->మునుపటి && bl->upval)  /* need a 'close'? */
    lua_K_సంకేతం_ABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ఒకవేళ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ఒకవేళ (bl->మునుపటి == ఖాళీ) {  /* was it the last block? */
    ఒకవేళ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->మునుపటి;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
స్థిర Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ఒకవేళ (fs->np >= f->sizep) {
    పూర్ణాంకం oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    అయితే (oldsize < f->sizep)
      f->p[oldsize++] = ఖాళీ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  తిరిగి clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
స్థిర శూన్య codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, lua_K_సంకేతం_ABx(fs, OP_సంవృతం, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


స్థిర శూన్య open_func (LexState *ls, FuncState *fs, ఖండం_Cnt *bl) {
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
  fs->bl = ఖాళీ;
  f->ఆధారం = ls->ఆధారం;
  luaC_objbarrier(L, f, f->ఆధారం);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


స్థిర శూన్య close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue తాత్కాలిక;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ఖాళీ);
  lua_K_ముగింపు(fs);
  luaM_shrinkvector(L, f->సంకేతం_2, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_పంక్తి_Info);
  luaM_shrinkvector(L, f->k, f->sizek, fs->nk, TValue);
  luaM_shrinkvector(L, f->p, f->sizep, fs->np, Proto *);
  luaM_shrinkvector(L, f->locvars, f->sizelocvars, fs->ndebugvars, LocVar);
  luaM_shrinkvector(L, f->upvalues, f->sizeupvalues, fs->nups, Upvaldesc);
  /* remove kcache table from scanner table ("weigh" its anchor) */
  sethvalue(L, &తాత్కాలిక, fs->kcache);  /* key to be set to nil */
  lua_H_సమితి(L, ls->h, &తాత్కాలిక, &G(L)->nilvalue);
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
స్థిర పూర్ణాంకం ఖండం_follow (LexState *ls, పూర్ణాంకం withuntil) {
  ఎంపిక (ls->t.token) {
    స్థితి TK_ELSE: స్థితి TK_ELSEIF:
    స్థితి TK_చివర: స్థితి TK_EOS:
      తిరిగి 1;
    స్థితి TK_UNTIL: తిరిగి withuntil;
    అప్రమేయ: తిరిగి 0;
  }
}


స్థిర శూన్య statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  అయితే (!ఖండం_follow(ls, 1)) {
    ఒకవేళ (ls->t.token == TK_RETURN) {
      statement(ls);
      తిరిగి;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


స్థిర శూన్య fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc కీలకం;
  luaK_exp2anyregup(fs, v);
  lua_X_తదుపరి(ls);  /* skip the dot or colon */
  codename(ls, &కీలకం);
  lua_K_సూచీకృత(fs, v, &కీలకం);
}


స్థిర శూన్య yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  lua_X_తదుపరి(ls);  /* skip the '[' */
  సమాసం(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

రకనిర్వచనం నిర్మాణం ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  పూర్ణాంకం nh;  /* total number of 'record' elements */
  పూర్ణాంకం na;  /* number of array elements already stored */
  పూర్ణాంకం tostore;  /* number of array elements pending to be stored */
  పూర్ణాంకం maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#నిర్వచించు MAX_CNST	(గరిష్ఠ_పూర్ణాంకం/2)
#ఒకవేళ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#నిర్వచనం_రద్దు MAX_CNST
#నిర్వచించు MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#అయితే_ముగింపు


స్థిర శూన్య recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, కీలకం, val;
  ఒకవేళ (ls->t.token == TK_NAME)
    codename(ls, &కీలకం);
  లేకపోతే  /* ls->t.token == '[' */
    yindex(ls, &కీలకం);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  lua_K_సూచీకృత(fs, &tab, &కీలకం);
  సమాసం(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


స్థిర శూన్య closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ఒకవేళ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


స్థిర శూన్య lastlistfield (FuncState *fs, ConsControl *cc) {
  ఒకవేళ (cc->tostore == 0) తిరిగి;
  ఒకవేళ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  లేకపోతే {
    ఒకవేళ (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


స్థిర శూన్య listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  సమాసం(ls, &cc->v);
  cc->tostore++;
}


స్థిర శూన్య క్షేత్రం (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  ఎంపిక(ls->t.token) {
    స్థితి TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ఒకవేళ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      లేకపోతే
        recfield(ls, cc);
      ఆపు;
    }
    స్థితి '[': {
      recfield(ls, cc);
      ఆపు;
    }
    అప్రమేయ: {
      listfield(ls, cc);
      ఆపు;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
స్థిర పూర్ణాంకం maxtostore (FuncState *fs) {
  పూర్ణాంకం numfreeregs = MAX_FSTACK - fs->freereg;
  ఒకవేళ (numfreeregs >= 160)  /* "lots" of registers? */
    తిరిగి numfreeregs / 5;  /* use up to 1/5 of them */
  లేకపోతే ఒకవేళ (numfreeregs >= 80)  /* still "enough" registers? */
    తిరిగి 10;  /* one 'SETLIST' instruction for each 10 values */
  లేకపోతే  /* save registers for potential more nesting */
    తిరిగి 1;
}


స్థిర శూన్య constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  పూర్ణాంకం పంక్తి = ls->linenumber;
  పూర్ణాంకం pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  lua_K_సంకేతం(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  చేయి {
    ఒకవేళ (ls->t.token == /*{*/ '}') ఆపు;
    ఒకవేళ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    క్షేత్రం(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } అయితే (testnext(ls, ',') || testnext(ls, ';'));
  check_సరిపోలిక(ls, /*{*/ '}', '{' /*}*/, పంక్తి);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


స్థిర శూన్య setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  lua_K_సంకేతం_ABC(fs, OP_VARARGPREP, 0, 0, 0);
}


స్థిర శూన్య parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  పూర్ణాంకం nparams = 0;
  పూర్ణాంకం varargk = 0;
  ఒకవేళ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    చేయి {
      ఎంపిక (ls->t.token) {
        స్థితి TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          ఆపు;
        }
        స్థితి TK_DOTS: {
          varargk = 1;
          lua_X_తదుపరి(ls);  /* skip '...' */
          ఒకవేళ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          లేకపోతే
            new_localvarliteral(ls, "(vararg table)");
          ఆపు;
        }
        అప్రమేయ: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } అయితే (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ఒకవేళ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


స్థిర శూన్య body (LexState *ls, expdesc *e, పూర్ణాంకం ismethod, పూర్ణాంకం పంక్తి) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  ఖండం_Cnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = పంక్తి;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ఒకవేళ (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_సరిపోలిక(ls, TK_చివర, TK_FUNCTION, పంక్తి);
  codeclosure(ls, e);
  close_func(ls);
}


స్థిర పూర్ణాంకం explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  పూర్ణాంకం n = 1;  /* at least one expression */
  సమాసం(ls, v);
  అయితే (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    సమాసం(ls, v);
    n++;
  }
  తిరిగి n;
}


స్థిర శూన్య funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc వాదనలు;
  పూర్ణాంకం పునాది, nparams;
  పూర్ణాంకం పంక్తి = ls->linenumber;
  ఎంపిక (ls->t.token) {
    స్థితి '(': {  /* funcargs -> '(' [ explist ] ')' */
      lua_X_తదుపరి(ls);
      ఒకవేళ (ls->t.token == ')')  /* arg list is empty? */
        వాదనలు.k = VVOID;
      లేకపోతే {
        explist(ls, &వాదనలు);
        ఒకవేళ (hasmultret(వాదనలు.k))
          luaK_setmultret(fs, &వాదనలు);
      }
      check_సరిపోలిక(ls, ')', '(', పంక్తి);
      ఆపు;
    }
    స్థితి '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &వాదనలు);
      ఆపు;
    }
    స్థితి TK_STRING: {  /* funcargs -> STRING */
      codestring(&వాదనలు, ls->t.seminfo.ts);
      lua_X_తదుపరి(ls);  /* must use 'seminfo' before 'next' */
      ఆపు;
    }
    అప్రమేయ: {
      luaX_syntaxerror(ls, "function arguments expected");
      తిరిగి;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  పునాది = f->u.info;  /* base register for call */
  ఒకవేళ (hasmultret(వాదనలు.k))
    nparams = LUA_MULTRET;  /* open call */
  లేకపోతే {
    ఒకవేళ (వాదనలు.k != VVOID)
      luaK_exp2nextreg(fs, &వాదనలు);  /* close last argument */
    nparams = fs->freereg - (పునాది+1);
  }
  init_exp(f, VCALL, lua_K_సంకేతం_ABC(fs, OP_CALL, పునాది, nparams+1, 2));
  luaK_fixline(fs, పంక్తి);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(పునాది + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


స్థిర శూన్య primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  ఎంపిక (ls->t.token) {
    స్థితి '(': {
      పూర్ణాంకం పంక్తి = ls->linenumber;
      lua_X_తదుపరి(ls);
      సమాసం(ls, v);
      check_సరిపోలిక(ls, ')', '(', పంక్తి);
      luaK_dischargevars(ls->fs, v);
      తిరిగి;
    }
    స్థితి TK_NAME: {
      singlevar(ls, v);
      తిరిగి;
    }
    అప్రమేయ: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


స్థిర శూన్య suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  కొరకు (;;) {
    ఎంపిక (ls->t.token) {
      స్థితి '.': {  /* fieldsel */
        fieldsel(ls, v);
        ఆపు;
      }
      స్థితి '[': {  /* '[' exp ']' */
        expdesc కీలకం;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &కీలకం);
        lua_K_సూచీకృత(fs, v, &కీలకం);
        ఆపు;
      }
      స్థితి ':': {  /* ':' NAME funcargs */
        expdesc కీలకం;
        lua_X_తదుపరి(ls);
        codename(ls, &కీలకం);
        luaK_self(fs, v, &కీలకం);
        funcargs(ls, v);
        ఆపు;
      }
      స్థితి '(': స్థితి TK_STRING: స్థితి '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        ఆపు;
      }
      అప్రమేయ: తిరిగి;
    }
  }
}


స్థిర శూన్య simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  ఎంపిక (ls->t.token) {
    స్థితి TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      ఆపు;
    }
    స్థితి TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      ఆపు;
    }
    స్థితి TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      ఆపు;
    }
    స్థితి TK_NIL: {
      init_exp(v, VNIL, 0);
      ఆపు;
    }
    స్థితి TK_TRUE: {
      init_exp(v, VTRUE, 0);
      ఆపు;
    }
    స్థితి TK_FALSE: {
      init_exp(v, VFALSE, 0);
      ఆపు;
    }
    స్థితి TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, lua_K_సంకేతం_ABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      ఆపు;
    }
    స్థితి '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      తిరిగి;
    }
    స్థితి TK_FUNCTION: {
      lua_X_తదుపరి(ls);
      body(ls, v, 0, ls->linenumber);
      తిరిగి;
    }
    అప్రమేయ: {
      suffixedexp(ls, v);
      తిరిగి;
    }
  }
  lua_X_తదుపరి(ls);
}


స్థిర UnOpr getunopr (పూర్ణాంకం op) {
  ఎంపిక (op) {
    స్థితి TK_NOT: తిరిగి OPR_NOT;
    స్థితి '-': తిరిగి OPR_MINUS;
    స్థితి '~': తిరిగి OPR_BNOT;
    స్థితి '#': తిరిగి OPR_LEN;
    అప్రమేయ: తిరిగి OPR_NOUNOPR;
  }
}


స్థిర BinOpr getbinopr (పూర్ణాంకం op) {
  ఎంపిక (op) {
    స్థితి '+': తిరిగి OPR_కూడు;
    స్థితి '-': తిరిగి OPR_SUB;
    స్థితి '*': తిరిగి OPR_MUL;
    స్థితి '%': తిరిగి OPR_MOD;
    స్థితి '^': తిరిగి OPR_POW;
    స్థితి '/': తిరిగి OPR_DIV;
    స్థితి TK_IDIV: తిరిగి OPR_IDIV;
    స్థితి '&': తిరిగి OPR_BAND;
    స్థితి '|': తిరిగి OPR_BOR;
    స్థితి '~': తిరిగి OPR_BXOR;
    స్థితి TK_SHL: తిరిగి OPR_SHL;
    స్థితి TK_SHR: తిరిగి OPR_SHR;
    స్థితి TK_CONCAT: తిరిగి OPR_CONCAT;
    స్థితి TK_NE: తిరిగి OPR_NE;
    స్థితి TK_EQ: తిరిగి OPR_EQ;
    స్థితి '<': తిరిగి OPR_LT;
    స్థితి TK_LE: తిరిగి OPR_LE;
    స్థితి '>': తిరిగి OPR_GT;
    స్థితి TK_GE: తిరిగి OPR_GE;
    స్థితి TK_AND: తిరిగి OPR_AND;
    స్థితి TK_OR: తిరిగి OPR_OR;
    అప్రమేయ: తిరిగి OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
స్థిర స్థిరరాశి నిర్మాణం {
  lu_byte ఎడమ;  /* left priority for each binary operator */
  lu_byte కుడి; /* right priority */
} ప్రాధాన్యత[] = {  /* ORDER OPR */
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

#నిర్వచించు UNARY_ప్రాధాన్యత	12  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
స్థిర BinOpr subexpr (LexState *ls, expdesc *v, పూర్ణాంకం హద్దు) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ఒకవేళ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    పూర్ణాంకం పంక్తి = ls->linenumber;
    lua_X_తదుపరి(ls);  /* skip operator */
    subexpr(ls, v, UNARY_ప్రాధాన్యత);
    lua_K_ఉపసర్గ(ls->fs, uop, v, పంక్తి);
  }
  లేకపోతే simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  అయితే (op != OPR_NOBINOPR && ప్రాధాన్యత[op].ఎడమ > హద్దు) {
    expdesc v2;
    BinOpr nextop;
    పూర్ణాంకం పంక్తి = ls->linenumber;
    lua_X_తదుపరి(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, ప్రాధాన్యత[op].కుడి);
    luaK_posfix(ls->fs, op, v, &v2, పంక్తి);
    op = nextop;
  }
  leavelevel(ls);
  తిరిగి op;  /* return first untreated operator */
}


స్థిర శూన్య సమాసం (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


స్థిర శూన్య ఖండం (LexState *ls) {
  /* block -> statlist */
  FuncState *fs = ls->fs;
  ఖండం_Cnt bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
నిర్మాణం LHS_కేటాయించు {
  నిర్మాణం LHS_కేటాయించు *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
స్థిర శూన్య check_conflict (LexState *ls, నిర్మాణం LHS_కేటాయించు *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  పూర్ణాంకం conflict = 0;
  కొరకు (; lh; lh = lh->prev) {  /* check all previous assignments */
    ఒకవేళ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ఒకవేళ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ఒకవేళ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      లేకపోతే {  /* table is a register */
        ఒకవేళ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ఒకవేళ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ఒకవేళ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ఒకవేళ (v->k == VLOCAL)
      lua_K_సంకేతం_ABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    లేకపోతే
      lua_K_సంకేతం_ABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
స్థిర శూన్య storevartop (FuncState *fs, expdesc *var) {
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
స్థిర శూన్య restassign (LexState *ls, నిర్మాణం LHS_కేటాయించు *lh, పూర్ణాంకం nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ఒకవేళ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    నిర్మాణం LHS_కేటాయించు nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ఒకవేళ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  లేకపోతే {  /* restassign -> '=' explist */
    పూర్ణాంకం nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ఒకవేళ (nexps != nvars)
      adjust_కేటాయించు(ls, nvars, nexps, &e);
    లేకపోతే {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      తిరిగి;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


స్థిర పూర్ణాంకం cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  సమాసం(ls, &v);  /* read condition */
  ఒకవేళ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  తిరిగి v.f;
}


స్థిర శూన్య gotostat (LexState *ls, పూర్ణాంకం పంక్తి) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, పంక్తి);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
స్థిర శూన్య breakstat (LexState *ls, పూర్ణాంకం పంక్తి) {
  ఖండం_Cnt *bl;  /* to look for an enclosing loop */
  కొరకు (bl = ls->fs->bl; bl != ఖాళీ; bl = bl->మునుపటి) {
    ఒకవేళ (bl->isloop)  /* found one? */
      వెళ్ళు ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  lua_X_తదుపరి(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, పంక్తి);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
స్థిర శూన్య checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ఒకవేళ (l_unlikely(lb != ఖాళీ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->పంక్తి);  /* error */
}


స్థిర శూన్య labelstat (LexState *ls, TString *name, పూర్ణాంకం పంక్తి) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  అయితే (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, పంక్తి, ఖండం_follow(ls, 0));
}


స్థిర శూన్య whilestat (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  పూర్ణాంకం whileinit;
  పూర్ణాంకం condexit;
  ఖండం_Cnt bl;
  lua_X_తదుపరి(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  ఖండం(ls);
  luaK_jumpto(fs, whileinit);
  check_సరిపోలిక(ls, TK_చివర, TK_WHILE, పంక్తి);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


స్థిర శూన్య repeatstat (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* repeatstat -> REPEAT block UNTIL cond */
  పూర్ణాంకం condexit;
  FuncState *fs = ls->fs;
  పూర్ణాంకం repeat_init = luaK_getlabel(fs);
  ఖండం_Cnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  lua_X_తదుపరి(ls);  /* skip REPEAT */
  statlist(ls);
  check_సరిపోలిక(ls, TK_UNTIL, TK_REPEAT, పంక్తి);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ఒకవేళ (bl2.upval) {  /* upvalues? */
    పూర్ణాంకం నిష్క్రమించు = luaK_jump(fs);  /* normal exit must jump over fix */
    luaK_patchtohere(fs, condexit);  /* repetition must close upvalues */
    lua_K_సంకేతం_ABC(fs, OP_CLOSE, reglevel(fs, bl2.nactvar), 0, 0);
    condexit = luaK_jump(fs);  /* repeat after closing upvalues */
    luaK_patchtohere(fs, నిష్క్రమించు);  /* normal exit comes to here */
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
స్థిర శూన్య exp1 (LexState *ls) {
  expdesc e;
  సమాసం(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
స్థిర శూన్య fixforjump (FuncState *fs, పూర్ణాంకం pc, పూర్ణాంకం dest, పూర్ణాంకం వెనుక) {
  Instruction *jmp = &fs->f->సంకేతం_2[pc];
  పూర్ణాంకం స్థానభ్రంశం = dest - (pc + 1);
  ఒకవేళ (వెనుక)
    స్థానభ్రంశం = -స్థానభ్రంశం;
  ఒకవేళ (l_unlikely(స్థానభ్రంశం > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, స్థానభ్రంశం);
}


/*
** Generate code for a 'for' loop.
*/
స్థిర శూన్య forbody (LexState *ls, పూర్ణాంకం పునాది, పూర్ణాంకం పంక్తి, పూర్ణాంకం nvars, పూర్ణాంకం isgen) {
  /* forbody -> DO block */
  స్థిర స్థిరరాశి Op_సంకేతం forprep[2] = {OP_FORPREP, OP_TFORPREP};
  స్థిర స్థిరరాశి Op_సంకేతం forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  ఖండం_Cnt bl;
  FuncState *fs = ls->fs;
  పూర్ణాంకం prep, endfor;
  checknext(ls, TK_DO);
  prep = lua_K_సంకేతం_ABx(fs, forprep[isgen], పునాది, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  ఖండం(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ఒకవేళ (isgen) {  /* generic for? */
    lua_K_సంకేతం_ABC(fs, OP_TFORCALL, పునాది, 0, nvars);
    luaK_fixline(fs, పంక్తి);
  }
  endfor = lua_K_సంకేతం_ABx(fs, forloop[isgen], పునాది, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, పంక్తి);
}


స్థిర శూన్య fornum (LexState *ls, TString *varname, పూర్ణాంకం పంక్తి) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  పూర్ణాంకం పునాది = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /*హద్దు*/
  ఒకవేళ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  లేకపోతే {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, పునాది, పంక్తి, 1, 0);
}


స్థిర శూన్య forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  పూర్ణాంకం nvars = 4;  /* function, state, closing, control */
  పూర్ణాంకం పంక్తి;
  పూర్ణాంకం పునాది = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  అయితే (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_లోపల);
  పంక్తి = ls->linenumber;
  adjust_కేటాయించు(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, పునాది, పంక్తి, nvars - 3, 1);
}


స్థిర శూన్య forstat (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  ఖండం_Cnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  lua_X_తదుపరి(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  ఎంపిక (ls->t.token) {
    స్థితి '=': fornum(ls, varname, పంక్తి); ఆపు;
    స్థితి ',': స్థితి TK_లోపల: forlist(ls, varname); ఆపు;
    అప్రమేయ: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_సరిపోలిక(ls, TK_చివర, TK_FOR, పంక్తి);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


స్థిర శూన్య test_then_ఖండం (LexState *ls, పూర్ణాంకం *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  పూర్ణాంకం condtrue;
  lua_X_తదుపరి(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  ఖండం(ls);  /* 'then' part */
  ఒకవేళ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


స్థిర శూన్య ifstat (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  పూర్ణాంకం escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_ఖండం(ls, &escapelist);  /* IF cond THEN block */
  అయితే (ls->t.token == TK_ELSEIF)
    test_then_ఖండం(ls, &escapelist);  /* ELSEIF cond THEN block */
  ఒకవేళ (testnext(ls, TK_ELSE))
    ఖండం(ls);  /* 'else' part */
  check_సరిపోలిక(ls, TK_చివర, TK_IF, పంక్తి);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


స్థిర శూన్య localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  పూర్ణాంకం fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


స్థిర lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ఒకవేళ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    స్థిరరాశి అక్షరం *attr = getstr(ts);
    checknext(ls, '>');
    ఒకవేళ (strcmp(attr, "const") == 0)
      తిరిగి RDKCONST;  /* read-only variable */
    లేకపోతే ఒకవేళ (strcmp(attr, "close") == 0)
      తిరిగి RDKTOCLOSE;  /* to-be-closed variable */
    లేకపోతే
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  తిరిగి df;  /* return default value */
}


స్థిర శూన్య checktoclose (FuncState *fs, పూర్ణాంకం level) {
  ఒకవేళ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    lua_K_సంకేతం_ABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


స్థిర శూన్య localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  పూర్ణాంకం toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  పూర్ణాంకం vidx;  /* index of last variable */
  పూర్ణాంకం nvars = 0;
  పూర్ణాంకం nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  చేయి {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte రకం = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, రకం);  /* predeclare it */
    ఒకవేళ (రకం == RDKTOCLOSE) {  /* to-be-closed? */
      ఒకవేళ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } అయితే (testnext(ls, ','));
  ఒకవేళ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  లేకపోతే {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ఒకవేళ (nvars == nexps &&  /* no adjustments? */
      var->vd.రకం == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.రకం = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  లేకపోతే {
    adjust_కేటాయించు(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


స్థిర lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte రకం = getvarattribute(ls, df);
  ఎంపిక (రకం) {
    స్థితి RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      తిరిగి రకం;  /* to avoid warnings */
    స్థితి RDKCONST:
      తిరిగి GDKCONST;  /* adjust kind for global variable */
    అప్రమేయ:
      తిరిగి రకం;
  }
}


స్థిర శూన్య checkglobal (LexState *ls, TString *varname, పూర్ణాంకం పంక్తి) {
  FuncState *fs = ls->fs;
  expdesc var;
  పూర్ణాంకం k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, పంక్తి);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
స్థిర శూన్య initglobal (LexState *ls, పూర్ణాంకం nvars, పూర్ణాంకం firstidx, పూర్ణాంకం n,
                        పూర్ణాంకం పంక్తి) {
  ఒకవేళ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    పూర్ణాంకం nexps = explist(ls, &e);  /* read list of expressions */
    adjust_కేటాయించు(ls, nvars, nexps, &e);
  }
  లేకపోతే {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, పంక్తి);
    leavelevel(ls);
    checkglobal(ls, varname, పంక్తి);
    storevartop(fs, &var);
  }
}


స్థిర శూన్య globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  పూర్ణాంకం nvars = 0;
  పూర్ణాంకం lastidx;  /* index of last registered variable */
  చేయి {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte రకం = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, రకం);
    nvars++;
  } అయితే (testnext(ls, ','));
  ఒకవేళ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


స్థిర శూన్య globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ఒకవేళ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  లేకపోతే {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ఖాళీ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


స్థిర శూన్య globalfunc (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, పంక్తి);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, పంక్తి);  /* definition "happens" in the first line */
}


స్థిర శూన్య globalstatfunc (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  lua_X_తదుపరి(ls);  /* skip 'global' */
  ఒకవేళ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, పంక్తి);
  లేకపోతే
    globalstat(ls);
}


స్థిర పూర్ణాంకం funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  పూర్ణాంకం ismethod = 0;
  singlevar(ls, v);
  అయితే (ls->t.token == '.')
    fieldsel(ls, v);
  ఒకవేళ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  తిరిగి ismethod;
}


స్థిర శూన్య funcstat (LexState *ls, పూర్ణాంకం పంక్తి) {
  /* funcstat -> FUNCTION funcname body */
  పూర్ణాంకం ismethod;
  expdesc v, b;
  lua_X_తదుపరి(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, పంక్తి);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, పంక్తి);  /* definition "happens" in the first line */
}


స్థిర శూన్య exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  నిర్మాణం LHS_కేటాయించు v;
  suffixedexp(ls, &v.v);
  ఒకవేళ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ఖాళీ;
    restassign(ls, &v, 1);
  }
  లేకపోతే {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


స్థిర శూన్య retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  పూర్ణాంకం nret;  /* number of values being returned */
  పూర్ణాంకం మొదటి = luaY_nvarstack(fs);  /* first slot to be returned */
  ఒకవేళ (ఖండం_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  లేకపోతే {
    nret = explist(ls, &e);  /* optional return values */
    ఒకవేళ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ఒకవేళ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        సమితి_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    లేకపోతే {
      ఒకవేళ (nret == 1)  /* only one single value? */
        మొదటి = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      లేకపోతే {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - మొదటి);
      }
    }
  }
  luaK_ret(fs, మొదటి, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


స్థిర శూన్య statement (LexState *ls) {
  పూర్ణాంకం పంక్తి = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  ఎంపిక (ls->t.token) {
    స్థితి ';': {  /* stat -> ';' (empty statement) */
      lua_X_తదుపరి(ls);  /* skip ';' */
      ఆపు;
    }
    స్థితి TK_IF: {  /* stat -> ifstat */
      ifstat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_DO: {  /* stat -> DO block END */
      lua_X_తదుపరి(ls);  /* skip DO */
      ఖండం(ls);
      check_సరిపోలిక(ls, TK_చివర, TK_DO, పంక్తి);
      ఆపు;
    }
    స్థితి TK_FOR: {  /* stat -> forstat */
      forstat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_LOCAL: {  /* stat -> localstat */
      lua_X_తదుపరి(ls);  /* skip LOCAL */
      ఒకవేళ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      లేకపోతే
        localstat(ls);
      ఆపు;
    }
    స్థితి TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_DBCOLON: {  /* stat -> label */
      lua_X_తదుపరి(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), పంక్తి);
      ఆపు;
    }
    స్థితి TK_RETURN: {  /* stat -> retstat */
      lua_X_తదుపరి(ls);  /* skip RETURN */
      retstat(ls);
      ఆపు;
    }
    స్థితి TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, పంక్తి);
      ఆపు;
    }
    స్థితి TK_GOTO: {  /* stat -> 'goto' NAME */
      lua_X_తదుపరి(ls);  /* skip 'goto' */
      gotostat(ls, పంక్తి);
      ఆపు;
    }
#ఒకవేళ LUA_COMPAT_GLOBAL
    స్థితి TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ఒకవేళ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        పూర్ణాంకం lk = luaX_lookahead(ls);
        ఒకవేళ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, పంక్తి);
          ఆపు;
        }
      }  /* else... */
    }
#అయితే_ముగింపు
    /* FALLTHROUGH */
    అప్రమేయ: {  /* stat -> func | assignment */
      exprstat(ls);
      ఆపు;
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
స్థిర శూన్య mainfunc (LexState *ls, FuncState *fs) {
  ఖండం_Cnt bl;
  Upvaldesc *env;
  open_func(ls, fs, &bl);
  setvararg(fs);  /* main function is always vararg */
  env = allocupvalue(fs);  /* ...set environment upvalue */
  env->instack = 1;
  env->idx = 0;
  env->రకం = VDKREG;
  env->name = ls->envn;
  luaC_objbarrier(ls->L, fs->f, env->name);
  lua_X_తదుపరి(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, పట్టిక *anchor, Mbuffer *buff,
                       Dyndata *dyd, స్థిరరాశి అక్షరం *name, పూర్ణాంకం firstchar) {
  LexState lexstate;
  FuncState funcstate;
  LClosure *cl;
  lexstate.h = anchor;  /* table for scanner */
  cl = luaF_newLclosure(L, 1);  /* create main closure */
  luaD_anchorobj(L, anchor, obj2gco(cl));  /* anchor it in scanner table */
  funcstate.f = cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  funcstate.f->ఆధారం = luaS_new(L, name);  /* create and anchor TString */
  luaC_objbarrier(L, funcstate.f, funcstate.f->ఆధారం);
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, funcstate.f->ఆధారం, firstchar);
  mainfunc(&lexstate, &funcstate);
  lua_assert(!funcstate.prev && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  తిరిగి cl;
}

