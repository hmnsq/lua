/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lcode.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "llex.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lparser.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"



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
प्रकारव्याख्या संरचना BlockCnt {
  संरचना BlockCnt *previous;  /* chain */
  पूर्णांक firstlabel;  /* index of first label in this block */
  पूर्णांक firstgoto;  /* index of first pending goto in this block */
  लघु nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
स्थिर शून्य statement (LexState *ls);
स्थिर शून्य अभिव्यक्ती (LexState *ls, expdesc *v);


स्थिर l_noret error_expected (LexState *ls, पूर्णांक token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


स्थिर l_noret errorlimit (FuncState *fs, पूर्णांक limit, स्थिरांक अक्षर *what) {
  lua_State *L = fs->ls->L;
  स्थिरांक अक्षर *संदेश;
  पूर्णांक ओळ = fs->f->linedefined;
  स्थिरांक अक्षर *where = (ओळ == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", ओळ);
  संदेश = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, संदेश);
}


शून्य luaY_checklimit (FuncState *fs, पूर्णांक v, पूर्णांक l, स्थिरांक अक्षर *what) {
  जर (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
स्थिर पूर्णांक testnext (LexState *ls, पूर्णांक c) {
  जर (ls->t.token == c) {
    luaX_next(ls);
    परत 1;
  }
  अन्यथा परत 0;
}


/*
** Check that next token is 'c'.
*/
स्थिर शून्य check (LexState *ls, पूर्णांक c) {
  जर (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
स्थिर शून्य checknext (LexState *ls, पूर्णांक c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,संदेश)	{ जर (!(c)) luaX_syntaxerror(ls, संदेश); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
स्थिर शून्य check_match (LexState *ls, पूर्णांक what, पूर्णांक who, पूर्णांक where) {
  जर (l_unlikely(!testnext(ls, what))) {
    जर (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    अन्यथा {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


स्थिर TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  परत ts;
}


स्थिर शून्य init_exp (expdesc *e, expkind k, पूर्णांक i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


स्थिर शून्य codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


स्थिर शून्य codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
स्थिर लघु registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  पूर्णांक oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, लघु_उच्च, "local variables");
  पर्यंत (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = रिक्त;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  परत fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
स्थिर पूर्णांक new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, लघु_उच्च, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  परत dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
स्थिर पूर्णांक new_localvar (LexState *ls, TString *name) {
  परत new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (आकार(v)/आकार(अक्षर)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
स्थिर Vardesc *getlocalvardesc (FuncState *fs, पूर्णांक vidx) {
  परत &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
स्थिर lu_byte reglevel (FuncState *fs, पूर्णांक nvar) {
  पर्यंत (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    जर (varinreg(vd))  /* is in a register? */
      परत cast_byte(vd->vd.ridx + 1);
  }
  परत 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  परत reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
स्थिर LocVar *localdebuginfo (FuncState *fs, पूर्णांक vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  जर (!varinreg(vd))
    परत रिक्त;  /* no debug info. for constants */
  अन्यथा {
    पूर्णांक idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    परत &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
स्थिर शून्य init_var (FuncState *fs, expdesc *e, पूर्णांक vidx) {
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
स्थिर शून्य check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = रिक्त;  /* to be set if variable is const */
  निवड (e->k) {
    स्थिती VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      थांबा;
    }
    स्थिती VLOCAL: स्थिती VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      जर (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      थांबा;
    }
    स्थिती VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      जर (up->kind != VDKREG)
        varname = up->name;
      थांबा;
    }
    स्थिती VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    स्थिती VINDEXUP: स्थिती VINDEXSTR: स्थिती VINDEXED: {  /* global variable */
      जर (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      थांबा;
    }
    पूर्वनिर्धारित:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      परत;  /* integer index cannot be read-only */
  }
  जर (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
स्थिर शून्य adjustlocalvars (LexState *ls, पूर्णांक nvars) {
  FuncState *fs = ls->fs;
  पूर्णांक reglevel = luaY_nvarstack(fs);
  पूर्णांक i;
  साठी (i = 0; i < nvars; i++) {
    पूर्णांक vidx = fs->nactvar++;
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
स्थिर शून्य removevars (FuncState *fs, पूर्णांक tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  पर्यंत (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    जर (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
स्थिर पूर्णांक searchupvalue (FuncState *fs, TString *name) {
  पूर्णांक i;
  Upvaldesc *up = fs->f->upvalues;
  साठी (i = 0; i < fs->nups; i++) {
    जर (eqstr(up[i].name, name)) परत i;
  }
  परत -1;  /* not found */
}


स्थिर Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  पूर्णांक oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  पर्यंत (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = रिक्त;
  परत &f->upvalues[fs->nups++];
}


स्थिर पूर्णांक newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  जर (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  अन्यथा {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  परत fs->nups - 1;
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
स्थिर पूर्णांक searchvar (FuncState *fs, TString *n, expdesc *var) {
  पूर्णांक i;
  साठी (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    जर (varglobal(vd)) {  /* global declaration? */
      जर (vd->vd.name == रिक्त) {  /* collective declaration? */
        जर (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      अन्यथा {  /* global name */
        जर (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          परत VGLOBAL;
        }
        अन्यथा जर (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    अन्यथा जर (eqstr(n, vd->vd.name)) {  /* found? */
      जर (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      अन्यथा {  /* local variable */
        init_var(fs, var, i);
        जर (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      परत cast_int(var->k);
    }
  }
  परत -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
स्थिर शून्य markupval (FuncState *fs, पूर्णांक level) {
  BlockCnt *bl = fs->bl;
  पर्यंत (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
स्थिर शून्य marktobeclosed (FuncState *fs) {
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
स्थिर शून्य singlevaraux (FuncState *fs, TString *n, expdesc *var, पूर्णांक base) {
  पूर्णांक v = searchvar(fs, n, var);  /* look up variables at current level */
  जर (v >= 0) {  /* found? */
    जर (!base) {
      जर (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      जर (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  अन्यथा {  /* not found at current level; try upvalues */
    पूर्णांक idx = searchupvalue(fs, n);  /* try existing upvalues */
    जर (idx < 0) {  /* not found? */
      जर (fs->prev != रिक्त)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      जर (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      अन्यथा  /* it is a global or a constant */
        परत;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


स्थिर शून्य buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc किल्ली;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  जर (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&किल्ली, varname);  /* key is variable name */
  luaK_indexed(fs, var, &किल्ली);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
स्थिर शून्य buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  जर (var->k == VGLOBAL) {  /* global name? */
    पूर्णांक info = var->u.info;
    /* global by default in the scope of a global declaration? */
    जर (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    जर (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    अन्यथा  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


स्थिर शून्य singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
स्थिर शून्य adjust_assign (LexState *ls, पूर्णांक nvars, पूर्णांक nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  पूर्णांक needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  जर (hasmultret(e->k)) {  /* last expression has multiple returns? */
    पूर्णांक extra = needed + 1;  /* discount last expression itself */
    जर (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  अन्यथा {
    जर (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    जर (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  जर (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  अन्यथा  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
स्थिर l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  स्थिरांक अक्षर *varname = (tsname != रिक्त) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->ओळ, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
स्थिर शून्य closegoto (LexState *ls, पूर्णांक g, Labeldesc *label, पूर्णांक bup) {
  पूर्णांक i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  जर (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  जर (gt->बंद ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  साठी (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
स्थिर Labeldesc *findlabel (LexState *ls, TString *name, पूर्णांक ilb) {
  Dyndata *dyd = ls->dyd;
  साठी (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    जर (eqstr(lb->name, name))  /* correct label? */
      परत lb;
  }
  परत रिक्त;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
स्थिर पूर्णांक newlabelentry (LexState *ls, Labellist *l, TString *name,
                          पूर्णांक ओळ, पूर्णांक pc) {
  पूर्णांक n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, लघु_उच्च, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].ओळ = ओळ;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].बंद = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  परत n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
स्थिर पूर्णांक newgotoentry (LexState *ls, TString *name, पूर्णांक ओळ) {
  FuncState *fs = ls->fs;
  पूर्णांक pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  परत newlabelentry(ls, &ls->dyd->gt, name, ओळ, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
स्थिर शून्य createlabel (LexState *ls, TString *name, पूर्णांक ओळ, पूर्णांक last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  पूर्णांक l = newlabelentry(ls, ll, name, ओळ, luaK_getlabel(fs));
  जर (last) {  /* label is last no-op statement in the block? */
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
स्थिर शून्य solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  पूर्णांक outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  पूर्णांक igt = bl->firstgoto;  /* first goto in the finishing block */
  पर्यंत (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    जर (lb != रिक्त)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    अन्यथा {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      जर (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->बंद = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


स्थिर शून्य enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != रिक्त && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
स्थिर l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->ओळ);
}


स्थिर शून्य leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  जर (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  जर (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  जर (bl->previous == रिक्त) {  /* was it the last block? */
    जर (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
स्थिर Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  जर (fs->np >= f->sizep) {
    पूर्णांक oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    पर्यंत (oldsize < f->sizep)
      f->p[oldsize++] = रिक्त;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  परत clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
स्थिर शून्य codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


स्थिर शून्य open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = रिक्त;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


स्थिर शून्य close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == रिक्त);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_ओळ_Info);
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
स्थिर पूर्णांक block_follow (LexState *ls, पूर्णांक withuntil) {
  निवड (ls->t.token) {
    स्थिती TK_ELSE: स्थिती TK_ELSEIF:
    स्थिती TK_END: स्थिती TK_EOS:
      परत 1;
    स्थिती TK_UNTIL: परत withuntil;
    पूर्वनिर्धारित: परत 0;
  }
}


स्थिर शून्य statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  पर्यंत (!block_follow(ls, 1)) {
    जर (ls->t.token == TK_RETURN) {
      statement(ls);
      परत;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


स्थिर शून्य fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc किल्ली;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &किल्ली);
  luaK_indexed(fs, v, &किल्ली);
}


स्थिर शून्य yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  अभिव्यक्ती(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

प्रकारव्याख्या संरचना ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  पूर्णांक nh;  /* total number of 'record' elements */
  पूर्णांक na;  /* number of array elements already stored */
  पूर्णांक tostore;  /* number of array elements pending to be stored */
  पूर्णांक maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(पूर्णांक_उच्च/2)
#जर MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


स्थिर शून्य recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, किल्ली, val;
  जर (ls->t.token == TK_NAME)
    codename(ls, &किल्ली);
  अन्यथा  /* ls->t.token == '[' */
    yindex(ls, &किल्ली);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &किल्ली);
  अभिव्यक्ती(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


स्थिर शून्य closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  जर (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


स्थिर शून्य lastlistfield (FuncState *fs, ConsControl *cc) {
  जर (cc->tostore == 0) परत;
  जर (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  अन्यथा {
    जर (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


स्थिर शून्य listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  अभिव्यक्ती(ls, &cc->v);
  cc->tostore++;
}


स्थिर शून्य field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  निवड(ls->t.token) {
    स्थिती TK_NAME: {  /* may be 'listfield' or 'recfield' */
      जर (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      अन्यथा
        recfield(ls, cc);
      थांबा;
    }
    स्थिती '[': {
      recfield(ls, cc);
      थांबा;
    }
    पूर्वनिर्धारित: {
      listfield(ls, cc);
      थांबा;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
स्थिर पूर्णांक maxtostore (FuncState *fs) {
  पूर्णांक numfreeregs = MAX_FSTACK - fs->freereg;
  जर (numfreeregs >= 160)  /* "lots" of registers? */
    परत numfreeregs / 5;  /* use up to 1/5 of them */
  अन्यथा जर (numfreeregs >= 80)  /* still "enough" registers? */
    परत 10;  /* one 'SETLIST' instruction for each 10 values */
  अन्यथा  /* save registers for potential more nesting */
    परत 1;
}


स्थिर शून्य constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  पूर्णांक ओळ = ls->linenumber;
  पूर्णांक pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  करा {
    जर (ls->t.token == /*{*/ '}') थांबा;
    जर (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } पर्यंत (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, ओळ);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


स्थिर शून्य setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


स्थिर शून्य parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  पूर्णांक nparams = 0;
  पूर्णांक varargk = 0;
  जर (ls->t.token != ')') {  /* is 'parlist' not empty? */
    करा {
      निवड (ls->t.token) {
        स्थिती TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          थांबा;
        }
        स्थिती TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          जर (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          अन्यथा
            new_localvarliteral(ls, "(vararg table)");
          थांबा;
        }
        पूर्वनिर्धारित: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } पर्यंत (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  जर (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


स्थिर शून्य body (LexState *ls, expdesc *e, पूर्णांक ismethod, पूर्णांक ओळ) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = ओळ;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  जर (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, ओळ);
  codeclosure(ls, e);
  close_func(ls);
}


स्थिर पूर्णांक explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  पूर्णांक n = 1;  /* at least one expression */
  अभिव्यक्ती(ls, v);
  पर्यंत (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    अभिव्यक्ती(ls, v);
    n++;
  }
  परत n;
}


स्थिर शून्य funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  पूर्णांक base, nparams;
  पूर्णांक ओळ = ls->linenumber;
  निवड (ls->t.token) {
    स्थिती '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      जर (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      अन्यथा {
        explist(ls, &args);
        जर (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', ओळ);
      थांबा;
    }
    स्थिती '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      थांबा;
    }
    स्थिती TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      थांबा;
    }
    पूर्वनिर्धारित: {
      luaX_syntaxerror(ls, "function arguments expected");
      परत;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  जर (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  अन्यथा {
    जर (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, ओळ);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


स्थिर शून्य primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  निवड (ls->t.token) {
    स्थिती '(': {
      पूर्णांक ओळ = ls->linenumber;
      luaX_next(ls);
      अभिव्यक्ती(ls, v);
      check_match(ls, ')', '(', ओळ);
      luaK_dischargevars(ls->fs, v);
      परत;
    }
    स्थिती TK_NAME: {
      singlevar(ls, v);
      परत;
    }
    पूर्वनिर्धारित: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


स्थिर शून्य suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  साठी (;;) {
    निवड (ls->t.token) {
      स्थिती '.': {  /* fieldsel */
        fieldsel(ls, v);
        थांबा;
      }
      स्थिती '[': {  /* '[' exp ']' */
        expdesc किल्ली;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &किल्ली);
        luaK_indexed(fs, v, &किल्ली);
        थांबा;
      }
      स्थिती ':': {  /* ':' NAME funcargs */
        expdesc किल्ली;
        luaX_next(ls);
        codename(ls, &किल्ली);
        luaK_self(fs, v, &किल्ली);
        funcargs(ls, v);
        थांबा;
      }
      स्थिती '(': स्थिती TK_STRING: स्थिती '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        थांबा;
      }
      पूर्वनिर्धारित: परत;
    }
  }
}


स्थिर शून्य simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  निवड (ls->t.token) {
    स्थिती TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      थांबा;
    }
    स्थिती TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      थांबा;
    }
    स्थिती TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      थांबा;
    }
    स्थिती TK_NIL: {
      init_exp(v, VNIL, 0);
      थांबा;
    }
    स्थिती TK_TRUE: {
      init_exp(v, VTRUE, 0);
      थांबा;
    }
    स्थिती TK_FALSE: {
      init_exp(v, VFALSE, 0);
      थांबा;
    }
    स्थिती TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      थांबा;
    }
    स्थिती '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      परत;
    }
    स्थिती TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      परत;
    }
    पूर्वनिर्धारित: {
      suffixedexp(ls, v);
      परत;
    }
  }
  luaX_next(ls);
}


स्थिर UnOpr getunopr (पूर्णांक op) {
  निवड (op) {
    स्थिती TK_NOT: परत OPR_NOT;
    स्थिती '-': परत OPR_MINUS;
    स्थिती '~': परत OPR_BNOT;
    स्थिती '#': परत OPR_LEN;
    पूर्वनिर्धारित: परत OPR_NOUNOPR;
  }
}


स्थिर BinOpr getbinopr (पूर्णांक op) {
  निवड (op) {
    स्थिती '+': परत OPR_ADD;
    स्थिती '-': परत OPR_SUB;
    स्थिती '*': परत OPR_MUL;
    स्थिती '%': परत OPR_MOD;
    स्थिती '^': परत OPR_POW;
    स्थिती '/': परत OPR_DIV;
    स्थिती TK_IDIV: परत OPR_IDIV;
    स्थिती '&': परत OPR_BAND;
    स्थिती '|': परत OPR_BOR;
    स्थिती '~': परत OPR_BXOR;
    स्थिती TK_SHL: परत OPR_SHL;
    स्थिती TK_SHR: परत OPR_SHR;
    स्थिती TK_CONCAT: परत OPR_CONCAT;
    स्थिती TK_NE: परत OPR_NE;
    स्थिती TK_EQ: परत OPR_EQ;
    स्थिती '<': परत OPR_LT;
    स्थिती TK_LE: परत OPR_LE;
    स्थिती '>': परत OPR_GT;
    स्थिती TK_GE: परत OPR_GE;
    स्थिती TK_AND: परत OPR_AND;
    स्थिती TK_OR: परत OPR_OR;
    पूर्वनिर्धारित: परत OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
स्थिर स्थिरांक संरचना {
  lu_byte डावा;  /* left priority for each binary operator */
  lu_byte उजवा; /* right priority */
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
स्थिर BinOpr subexpr (LexState *ls, expdesc *v, पूर्णांक limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  जर (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    पूर्णांक ओळ = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, ओळ);
  }
  अन्यथा simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  पर्यंत (op != OPR_NOBINOPR && priority[op].डावा > limit) {
    expdesc v2;
    BinOpr nextop;
    पूर्णांक ओळ = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].उजवा);
    luaK_posfix(ls->fs, op, v, &v2, ओळ);
    op = nextop;
  }
  leavelevel(ls);
  परत op;  /* return first untreated operator */
}


स्थिर शून्य अभिव्यक्ती (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


स्थिर शून्य block (LexState *ls) {
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
संरचना LHS_assign {
  संरचना LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
स्थिर शून्य check_conflict (LexState *ls, संरचना LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  पूर्णांक conflict = 0;
  साठी (; lh; lh = lh->prev) {  /* check all previous assignments */
    जर (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      जर (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        जर (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      अन्यथा {  /* table is a register */
        जर (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        जर (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  जर (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    जर (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    अन्यथा
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
स्थिर शून्य storevartop (FuncState *fs, expdesc *var) {
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
स्थिर शून्य restassign (LexState *ls, संरचना LHS_assign *lh, पूर्णांक nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  जर (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    संरचना LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    जर (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  अन्यथा {  /* restassign -> '=' explist */
    पूर्णांक nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    जर (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    अन्यथा {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      परत;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


स्थिर पूर्णांक cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  अभिव्यक्ती(ls, &v);  /* read condition */
  जर (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  परत v.f;
}


स्थिर शून्य gotostat (LexState *ls, पूर्णांक ओळ) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, ओळ);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
स्थिर शून्य breakstat (LexState *ls, पूर्णांक ओळ) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  साठी (bl = ls->fs->bl; bl != रिक्त; bl = bl->previous) {
    जर (bl->isloop)  /* found one? */
      जा ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, ओळ);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
स्थिर शून्य checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  जर (l_unlikely(lb != रिक्त))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->ओळ);  /* error */
}


स्थिर शून्य labelstat (LexState *ls, TString *name, पूर्णांक ओळ) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  पर्यंत (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, ओळ, block_follow(ls, 0));
}


स्थिर शून्य whilestat (LexState *ls, पूर्णांक ओळ) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  पूर्णांक whileinit;
  पूर्णांक condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, ओळ);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


स्थिर शून्य repeatstat (LexState *ls, पूर्णांक ओळ) {
  /* repeatstat -> REPEAT block UNTIL cond */
  पूर्णांक condexit;
  FuncState *fs = ls->fs;
  पूर्णांक repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, ओळ);
  condexit = cond(ls);  /* read condition (inside scope block) */
  जर (bl2.upval) {  /* upvalues? */
    पूर्णांक निकास = luaK_jump(fs);  /* normal exit must jump over fix */
    luaK_patchtohere(fs, condexit);  /* repetition must close upvalues */
    luaK_codeABC(fs, OP_CLOSE, reglevel(fs, bl2.nactvar), 0, 0);
    condexit = luaK_jump(fs);  /* repeat after closing upvalues */
    luaK_patchtohere(fs, निकास);  /* normal exit comes to here */
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
स्थिर शून्य exp1 (LexState *ls) {
  expdesc e;
  अभिव्यक्ती(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
स्थिर शून्य fixforjump (FuncState *fs, पूर्णांक pc, पूर्णांक dest, पूर्णांक back) {
  Instruction *jmp = &fs->f->code[pc];
  पूर्णांक offset = dest - (pc + 1);
  जर (back)
    offset = -offset;
  जर (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
स्थिर शून्य forbody (LexState *ls, पूर्णांक base, पूर्णांक ओळ, पूर्णांक nvars, पूर्णांक isgen) {
  /* forbody -> DO block */
  स्थिर स्थिरांक OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  स्थिर स्थिरांक OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  पूर्णांक prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  जर (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, ओळ);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, ओळ);
}


स्थिर शून्य fornum (LexState *ls, TString *varname, पूर्णांक ओळ) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  पूर्णांक base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  जर (testnext(ls, ','))
    exp1(ls);  /* optional step */
  अन्यथा {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, ओळ, 1, 0);
}


स्थिर शून्य forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  पूर्णांक nvars = 4;  /* function, state, closing, control */
  पूर्णांक ओळ;
  पूर्णांक base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  पर्यंत (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  ओळ = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, ओळ, nvars - 3, 1);
}


स्थिर शून्य forstat (LexState *ls, पूर्णांक ओळ) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  निवड (ls->t.token) {
    स्थिती '=': fornum(ls, varname, ओळ); थांबा;
    स्थिती ',': स्थिती TK_IN: forlist(ls, varname); थांबा;
    पूर्वनिर्धारित: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, ओळ);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


स्थिर शून्य test_then_block (LexState *ls, पूर्णांक *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  पूर्णांक condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  जर (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


स्थिर शून्य ifstat (LexState *ls, पूर्णांक ओळ) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  पूर्णांक escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  पर्यंत (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  जर (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, ओळ);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


स्थिर शून्य localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  पूर्णांक fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


स्थिर lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  जर (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    स्थिरांक अक्षर *attr = getstr(ts);
    checknext(ls, '>');
    जर (म_भेद(attr, "const") == 0)
      परत RDKCONST;  /* read-only variable */
    अन्यथा जर (म_भेद(attr, "close") == 0)
      परत RDKTOCLOSE;  /* to-be-closed variable */
    अन्यथा
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  परत df;  /* return default value */
}


स्थिर शून्य checktoclose (FuncState *fs, पूर्णांक level) {
  जर (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


स्थिर शून्य localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  पूर्णांक toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  पूर्णांक vidx;  /* index of last variable */
  पूर्णांक nvars = 0;
  पूर्णांक nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  करा {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    जर (kind == RDKTOCLOSE) {  /* to-be-closed? */
      जर (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } पर्यंत (testnext(ls, ','));
  जर (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  अन्यथा {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  जर (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  अन्यथा {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


स्थिर lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  निवड (kind) {
    स्थिती RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      परत kind;  /* to avoid warnings */
    स्थिती RDKCONST:
      परत GDKCONST;  /* adjust kind for global variable */
    पूर्वनिर्धारित:
      परत kind;
  }
}


स्थिर शून्य checkglobal (LexState *ls, TString *varname, पूर्णांक ओळ) {
  FuncState *fs = ls->fs;
  expdesc var;
  पूर्णांक k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, ओळ);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
स्थिर शून्य initglobal (LexState *ls, पूर्णांक nvars, पूर्णांक firstidx, पूर्णांक n,
                        पूर्णांक ओळ) {
  जर (n == nvars) {  /* traversed all variables? */
    expdesc e;
    पूर्णांक nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  अन्यथा {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, ओळ);
    leavelevel(ls);
    checkglobal(ls, varname, ओळ);
    storevartop(fs, &var);
  }
}


स्थिर शून्य globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  पूर्णांक nvars = 0;
  पूर्णांक lastidx;  /* index of last registered variable */
  करा {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } पर्यंत (testnext(ls, ','));
  जर (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


स्थिर शून्य globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  जर (!testnext(ls, '*'))
    globalnames(ls, defkind);
  अन्यथा {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, रिक्त, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


स्थिर शून्य globalfunc (LexState *ls, पूर्णांक ओळ) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, ओळ);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, ओळ);  /* definition "happens" in the first line */
}


स्थिर शून्य globalstatfunc (LexState *ls, पूर्णांक ओळ) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  जर (testnext(ls, TK_FUNCTION))
    globalfunc(ls, ओळ);
  अन्यथा
    globalstat(ls);
}


स्थिर पूर्णांक funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  पूर्णांक ismethod = 0;
  singlevar(ls, v);
  पर्यंत (ls->t.token == '.')
    fieldsel(ls, v);
  जर (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  परत ismethod;
}


स्थिर शून्य funcstat (LexState *ls, पूर्णांक ओळ) {
  /* funcstat -> FUNCTION funcname body */
  पूर्णांक ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, ओळ);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, ओळ);  /* definition "happens" in the first line */
}


स्थिर शून्य exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  संरचना LHS_assign v;
  suffixedexp(ls, &v.v);
  जर (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = रिक्त;
    restassign(ls, &v, 1);
  }
  अन्यथा {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


स्थिर शून्य retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  पूर्णांक nret;  /* number of values being returned */
  पूर्णांक first = luaY_nvarstack(fs);  /* first slot to be returned */
  जर (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  अन्यथा {
    nret = explist(ls, &e);  /* optional return values */
    जर (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      जर (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    अन्यथा {
      जर (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      अन्यथा {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


स्थिर शून्य statement (LexState *ls) {
  पूर्णांक ओळ = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  निवड (ls->t.token) {
    स्थिती ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      थांबा;
    }
    स्थिती TK_IF: {  /* stat -> ifstat */
      ifstat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, ओळ);
      थांबा;
    }
    स्थिती TK_FOR: {  /* stat -> forstat */
      forstat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      जर (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      अन्यथा
        localstat(ls);
      थांबा;
    }
    स्थिती TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, ओळ);
      थांबा;
    }
    स्थिती TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), ओळ);
      थांबा;
    }
    स्थिती TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      थांबा;
    }
    स्थिती TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, ओळ);
      थांबा;
    }
    स्थिती TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, ओळ);
      थांबा;
    }
#जर LUA_COMPAT_GLOBAL
    स्थिती TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      जर (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        पूर्णांक lk = luaX_lookahead(ls);
        जर (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, ओळ);
          थांबा;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    पूर्वनिर्धारित: {  /* stat -> func | assignment */
      exprstat(ls);
      थांबा;
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
स्थिर शून्य mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, स्थिरांक अक्षर *name, पूर्णांक firstchar) {
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
  परत cl;
}

