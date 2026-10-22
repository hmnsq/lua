/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lcode.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "llex.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lparser.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"



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
வகைவரை கட்டமைப்பு BlockCnt {
  கட்டமைப்பு BlockCnt *previous;  /* chain */
  முழு firstlabel;  /* index of first label in this block */
  முழு firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
நிலையான வெறுமை statement (LexState *ls);
நிலையான வெறுமை கோவை (LexState *ls, expdesc *v);


நிலையான l_noret error_expected (LexState *ls, முழு token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


நிலையான l_noret errorlimit (FuncState *fs, முழு limit, மாறிலி எழுத்து *what) {
  lua_State *L = fs->ls->L;
  மாறிலி எழுத்து *செய்தி;
  முழு வரி = fs->f->linedefined;
  மாறிலி எழுத்து *where = (வரி == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", வரி);
  செய்தி = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, செய்தி);
}


வெறுமை luaY_checklimit (FuncState *fs, முழு v, முழு l, மாறிலி எழுத்து *what) {
  எனில் (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
நிலையான முழு testnext (LexState *ls, முழு c) {
  எனில் (ls->t.token == c) {
    luaX_next(ls);
    திரும்பு 1;
  }
  இல்லையெனில் திரும்பு 0;
}


/*
** Check that next token is 'c'.
*/
நிலையான வெறுமை check (LexState *ls, முழு c) {
  எனில் (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
நிலையான வெறுமை checknext (LexState *ls, முழு c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,செய்தி)	{ எனில் (!(c)) luaX_syntaxerror(ls, செய்தி); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
நிலையான வெறுமை check_match (LexState *ls, முழு what, முழு who, முழு where) {
  எனில் (l_unlikely(!testnext(ls, what))) {
    எனில் (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    இல்லையெனில் {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


நிலையான TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  திரும்பு ts;
}


நிலையான வெறுமை init_exp (expdesc *e, expkind k, முழு i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


நிலையான வெறுமை codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


நிலையான வெறுமை codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
நிலையான short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  முழு oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  போது (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = வெற்று;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  திரும்பு fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
நிலையான முழு new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  திரும்பு dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
நிலையான முழு new_localvar (LexState *ls, TString *name) {
  திரும்பு new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (அளவு(v)/அளவு(எழுத்து)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
நிலையான Vardesc *getlocalvardesc (FuncState *fs, முழு vidx) {
  திரும்பு &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
நிலையான lu_byte reglevel (FuncState *fs, முழு nvar) {
  போது (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    எனில் (varinreg(vd))  /* is in a register? */
      திரும்பு cast_byte(vd->vd.ridx + 1);
  }
  திரும்பு 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  திரும்பு reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
நிலையான LocVar *localdebuginfo (FuncState *fs, முழு vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  எனில் (!varinreg(vd))
    திரும்பு வெற்று;  /* no debug info. for constants */
  இல்லையெனில் {
    முழு idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    திரும்பு &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
நிலையான வெறுமை init_var (FuncState *fs, expdesc *e, முழு vidx) {
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
நிலையான வெறுமை check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = வெற்று;  /* to be set if variable is const */
  தேர்வு (e->k) {
    நிலை VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      நிறுத்து;
    }
    நிலை VLOCAL: நிலை VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      எனில் (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      நிறுத்து;
    }
    நிலை VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      எனில் (up->kind != VDKREG)
        varname = up->name;
      நிறுத்து;
    }
    நிலை VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    நிலை VINDEXUP: நிலை VINDEXSTR: நிலை VINDEXED: {  /* global variable */
      எனில் (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      நிறுத்து;
    }
    இயல்பு:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      திரும்பு;  /* integer index cannot be read-only */
  }
  எனில் (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
நிலையான வெறுமை adjustlocalvars (LexState *ls, முழு nvars) {
  FuncState *fs = ls->fs;
  முழு reglevel = luaY_nvarstack(fs);
  முழு i;
  க்கு (i = 0; i < nvars; i++) {
    முழு vidx = fs->nactvar++;
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
நிலையான வெறுமை removevars (FuncState *fs, முழு tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  போது (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    எனில் (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
நிலையான முழு searchupvalue (FuncState *fs, TString *name) {
  முழு i;
  Upvaldesc *up = fs->f->upvalues;
  க்கு (i = 0; i < fs->nups; i++) {
    எனில் (eqstr(up[i].name, name)) திரும்பு i;
  }
  திரும்பு -1;  /* not found */
}


நிலையான Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  முழு oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  போது (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = வெற்று;
  திரும்பு &f->upvalues[fs->nups++];
}


நிலையான முழு newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  எனில் (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  இல்லையெனில் {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  திரும்பு fs->nups - 1;
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
நிலையான முழு searchvar (FuncState *fs, TString *n, expdesc *var) {
  முழு i;
  க்கு (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    எனில் (varglobal(vd)) {  /* global declaration? */
      எனில் (vd->vd.name == வெற்று) {  /* collective declaration? */
        எனில் (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      இல்லையெனில் {  /* global name */
        எனில் (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          திரும்பு VGLOBAL;
        }
        இல்லையெனில் எனில் (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    இல்லையெனில் எனில் (eqstr(n, vd->vd.name)) {  /* found? */
      எனில் (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      இல்லையெனில் {  /* local variable */
        init_var(fs, var, i);
        எனில் (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      திரும்பு cast_int(var->k);
    }
  }
  திரும்பு -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
நிலையான வெறுமை markupval (FuncState *fs, முழு level) {
  BlockCnt *bl = fs->bl;
  போது (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
நிலையான வெறுமை marktobeclosed (FuncState *fs) {
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
நிலையான வெறுமை singlevaraux (FuncState *fs, TString *n, expdesc *var, முழு base) {
  முழு v = searchvar(fs, n, var);  /* look up variables at current level */
  எனில் (v >= 0) {  /* found? */
    எனில் (!base) {
      எனில் (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      எனில் (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  இல்லையெனில் {  /* not found at current level; try upvalues */
    முழு idx = searchupvalue(fs, n);  /* try existing upvalues */
    எனில் (idx < 0) {  /* not found? */
      எனில் (fs->prev != வெற்று)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      எனில் (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      இல்லையெனில்  /* it is a global or a constant */
        திரும்பு;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


நிலையான வெறுமை buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc சாவி;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  எனில் (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&சாவி, varname);  /* key is variable name */
  luaK_indexed(fs, var, &சாவி);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
நிலையான வெறுமை buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  எனில் (var->k == VGLOBAL) {  /* global name? */
    முழு info = var->u.info;
    /* global by default in the scope of a global declaration? */
    எனில் (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    எனில் (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    இல்லையெனில்  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


நிலையான வெறுமை singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
நிலையான வெறுமை adjust_assign (LexState *ls, முழு nvars, முழு nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  முழு needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  எனில் (hasmultret(e->k)) {  /* last expression has multiple returns? */
    முழு extra = needed + 1;  /* discount last expression itself */
    எனில் (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  இல்லையெனில் {
    எனில் (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    எனில் (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  எனில் (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  இல்லையெனில்  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
நிலையான l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  மாறிலி எழுத்து *varname = (tsname != வெற்று) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->வரி, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
நிலையான வெறுமை closegoto (LexState *ls, முழு g, Labeldesc *label, முழு bup) {
  முழு i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  எனில் (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  எனில் (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  க்கு (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
நிலையான Labeldesc *findlabel (LexState *ls, TString *name, முழு ilb) {
  Dyndata *dyd = ls->dyd;
  க்கு (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    எனில் (eqstr(lb->name, name))  /* correct label? */
      திரும்பு lb;
  }
  திரும்பு வெற்று;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
நிலையான முழு newlabelentry (LexState *ls, Labellist *l, TString *name,
                          முழு வரி, முழு pc) {
  முழு n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].வரி = வரி;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  திரும்பு n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
நிலையான முழு newgotoentry (LexState *ls, TString *name, முழு வரி) {
  FuncState *fs = ls->fs;
  முழு pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  திரும்பு newlabelentry(ls, &ls->dyd->gt, name, வரி, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
நிலையான வெறுமை createlabel (LexState *ls, TString *name, முழு வரி, முழு last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  முழு l = newlabelentry(ls, ll, name, வரி, luaK_getlabel(fs));
  எனில் (last) {  /* label is last no-op statement in the block? */
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
நிலையான வெறுமை solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  முழு outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  முழு igt = bl->firstgoto;  /* first goto in the finishing block */
  போது (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    எனில் (lb != வெற்று)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    இல்லையெனில் {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      எனில் (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


நிலையான வெறுமை enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != வெற்று && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
நிலையான l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->வரி);
}


நிலையான வெறுமை leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  எனில் (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  எனில் (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  எனில் (bl->previous == வெற்று) {  /* was it the last block? */
    எனில் (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
நிலையான Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  எனில் (fs->np >= f->sizep) {
    முழு oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    போது (oldsize < f->sizep)
      f->p[oldsize++] = வெற்று;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  திரும்பு clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
நிலையான வெறுமை codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


நிலையான வெறுமை open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = வெற்று;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


நிலையான வெறுமை close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == வெற்று);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_வரி_Info);
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
நிலையான முழு block_follow (LexState *ls, முழு withuntil) {
  தேர்வு (ls->t.token) {
    நிலை TK_ELSE: நிலை TK_ELSEIF:
    நிலை TK_END: நிலை TK_EOS:
      திரும்பு 1;
    நிலை TK_UNTIL: திரும்பு withuntil;
    இயல்பு: திரும்பு 0;
  }
}


நிலையான வெறுமை statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  போது (!block_follow(ls, 1)) {
    எனில் (ls->t.token == TK_RETURN) {
      statement(ls);
      திரும்பு;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


நிலையான வெறுமை fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc சாவி;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &சாவி);
  luaK_indexed(fs, v, &சாவி);
}


நிலையான வெறுமை yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  கோவை(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

வகைவரை கட்டமைப்பு ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  முழு nh;  /* total number of 'record' elements */
  முழு na;  /* number of array elements already stored */
  முழு tostore;  /* number of array elements pending to be stored */
  முழு maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#எனில் MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


நிலையான வெறுமை recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, சாவி, val;
  எனில் (ls->t.token == TK_NAME)
    codename(ls, &சாவி);
  இல்லையெனில்  /* ls->t.token == '[' */
    yindex(ls, &சாவி);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &சாவி);
  கோவை(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


நிலையான வெறுமை closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  எனில் (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


நிலையான வெறுமை lastlistfield (FuncState *fs, ConsControl *cc) {
  எனில் (cc->tostore == 0) திரும்பு;
  எனில் (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  இல்லையெனில் {
    எனில் (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


நிலையான வெறுமை listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  கோவை(ls, &cc->v);
  cc->tostore++;
}


நிலையான வெறுமை field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  தேர்வு(ls->t.token) {
    நிலை TK_NAME: {  /* may be 'listfield' or 'recfield' */
      எனில் (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      இல்லையெனில்
        recfield(ls, cc);
      நிறுத்து;
    }
    நிலை '[': {
      recfield(ls, cc);
      நிறுத்து;
    }
    இயல்பு: {
      listfield(ls, cc);
      நிறுத்து;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
நிலையான முழு maxtostore (FuncState *fs) {
  முழு numfreeregs = MAX_FSTACK - fs->freereg;
  எனில் (numfreeregs >= 160)  /* "lots" of registers? */
    திரும்பு numfreeregs / 5;  /* use up to 1/5 of them */
  இல்லையெனில் எனில் (numfreeregs >= 80)  /* still "enough" registers? */
    திரும்பு 10;  /* one 'SETLIST' instruction for each 10 values */
  இல்லையெனில்  /* save registers for potential more nesting */
    திரும்பு 1;
}


நிலையான வெறுமை constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  முழு வரி = ls->linenumber;
  முழு pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  செய் {
    எனில் (ls->t.token == /*{*/ '}') நிறுத்து;
    எனில் (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } போது (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, வரி);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


நிலையான வெறுமை setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


நிலையான வெறுமை parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  முழு nparams = 0;
  முழு varargk = 0;
  எனில் (ls->t.token != ')') {  /* is 'parlist' not empty? */
    செய் {
      தேர்வு (ls->t.token) {
        நிலை TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          நிறுத்து;
        }
        நிலை TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          எனில் (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          இல்லையெனில்
            new_localvarliteral(ls, "(vararg table)");
          நிறுத்து;
        }
        இயல்பு: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } போது (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  எனில் (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


நிலையான வெறுமை body (LexState *ls, expdesc *e, முழு ismethod, முழு வரி) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = வரி;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  எனில் (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, வரி);
  codeclosure(ls, e);
  close_func(ls);
}


நிலையான முழு explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  முழு n = 1;  /* at least one expression */
  கோவை(ls, v);
  போது (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    கோவை(ls, v);
    n++;
  }
  திரும்பு n;
}


நிலையான வெறுமை funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  முழு base, nparams;
  முழு வரி = ls->linenumber;
  தேர்வு (ls->t.token) {
    நிலை '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      எனில் (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      இல்லையெனில் {
        explist(ls, &args);
        எனில் (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', வரி);
      நிறுத்து;
    }
    நிலை '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      நிறுத்து;
    }
    நிலை TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      நிறுத்து;
    }
    இயல்பு: {
      luaX_syntaxerror(ls, "function arguments expected");
      திரும்பு;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  எனில் (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  இல்லையெனில் {
    எனில் (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, வரி);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


நிலையான வெறுமை primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  தேர்வு (ls->t.token) {
    நிலை '(': {
      முழு வரி = ls->linenumber;
      luaX_next(ls);
      கோவை(ls, v);
      check_match(ls, ')', '(', வரி);
      luaK_dischargevars(ls->fs, v);
      திரும்பு;
    }
    நிலை TK_NAME: {
      singlevar(ls, v);
      திரும்பு;
    }
    இயல்பு: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


நிலையான வெறுமை suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  க்கு (;;) {
    தேர்வு (ls->t.token) {
      நிலை '.': {  /* fieldsel */
        fieldsel(ls, v);
        நிறுத்து;
      }
      நிலை '[': {  /* '[' exp ']' */
        expdesc சாவி;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &சாவி);
        luaK_indexed(fs, v, &சாவி);
        நிறுத்து;
      }
      நிலை ':': {  /* ':' NAME funcargs */
        expdesc சாவி;
        luaX_next(ls);
        codename(ls, &சாவி);
        luaK_self(fs, v, &சாவி);
        funcargs(ls, v);
        நிறுத்து;
      }
      நிலை '(': நிலை TK_STRING: நிலை '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        நிறுத்து;
      }
      இயல்பு: திரும்பு;
    }
  }
}


நிலையான வெறுமை simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  தேர்வு (ls->t.token) {
    நிலை TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      நிறுத்து;
    }
    நிலை TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      நிறுத்து;
    }
    நிலை TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      நிறுத்து;
    }
    நிலை TK_NIL: {
      init_exp(v, VNIL, 0);
      நிறுத்து;
    }
    நிலை TK_TRUE: {
      init_exp(v, VTRUE, 0);
      நிறுத்து;
    }
    நிலை TK_FALSE: {
      init_exp(v, VFALSE, 0);
      நிறுத்து;
    }
    நிலை TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      நிறுத்து;
    }
    நிலை '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      திரும்பு;
    }
    நிலை TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      திரும்பு;
    }
    இயல்பு: {
      suffixedexp(ls, v);
      திரும்பு;
    }
  }
  luaX_next(ls);
}


நிலையான UnOpr getunopr (முழு op) {
  தேர்வு (op) {
    நிலை TK_NOT: திரும்பு OPR_NOT;
    நிலை '-': திரும்பு OPR_MINUS;
    நிலை '~': திரும்பு OPR_BNOT;
    நிலை '#': திரும்பு OPR_LEN;
    இயல்பு: திரும்பு OPR_NOUNOPR;
  }
}


நிலையான BinOpr getbinopr (முழு op) {
  தேர்வு (op) {
    நிலை '+': திரும்பு OPR_ADD;
    நிலை '-': திரும்பு OPR_SUB;
    நிலை '*': திரும்பு OPR_MUL;
    நிலை '%': திரும்பு OPR_MOD;
    நிலை '^': திரும்பு OPR_POW;
    நிலை '/': திரும்பு OPR_DIV;
    நிலை TK_IDIV: திரும்பு OPR_IDIV;
    நிலை '&': திரும்பு OPR_BAND;
    நிலை '|': திரும்பு OPR_BOR;
    நிலை '~': திரும்பு OPR_BXOR;
    நிலை TK_SHL: திரும்பு OPR_SHL;
    நிலை TK_SHR: திரும்பு OPR_SHR;
    நிலை TK_CONCAT: திரும்பு OPR_CONCAT;
    நிலை TK_NE: திரும்பு OPR_NE;
    நிலை TK_EQ: திரும்பு OPR_EQ;
    நிலை '<': திரும்பு OPR_LT;
    நிலை TK_LE: திரும்பு OPR_LE;
    நிலை '>': திரும்பு OPR_GT;
    நிலை TK_GE: திரும்பு OPR_GE;
    நிலை TK_AND: திரும்பு OPR_AND;
    நிலை TK_OR: திரும்பு OPR_OR;
    இயல்பு: திரும்பு OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
நிலையான மாறிலி கட்டமைப்பு {
  lu_byte இடது;  /* left priority for each binary operator */
  lu_byte வலது; /* right priority */
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
நிலையான BinOpr subexpr (LexState *ls, expdesc *v, முழு limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  எனில் (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    முழு வரி = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, வரி);
  }
  இல்லையெனில் simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  போது (op != OPR_NOBINOPR && priority[op].இடது > limit) {
    expdesc v2;
    BinOpr nextop;
    முழு வரி = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].வலது);
    luaK_posfix(ls->fs, op, v, &v2, வரி);
    op = nextop;
  }
  leavelevel(ls);
  திரும்பு op;  /* return first untreated operator */
}


நிலையான வெறுமை கோவை (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


நிலையான வெறுமை block (LexState *ls) {
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
கட்டமைப்பு LHS_assign {
  கட்டமைப்பு LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
நிலையான வெறுமை check_conflict (LexState *ls, கட்டமைப்பு LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  முழு conflict = 0;
  க்கு (; lh; lh = lh->prev) {  /* check all previous assignments */
    எனில் (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      எனில் (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        எனில் (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      இல்லையெனில் {  /* table is a register */
        எனில் (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        எனில் (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  எனில் (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    எனில் (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    இல்லையெனில்
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
நிலையான வெறுமை storevartop (FuncState *fs, expdesc *var) {
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
நிலையான வெறுமை restassign (LexState *ls, கட்டமைப்பு LHS_assign *lh, முழு nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  எனில் (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    கட்டமைப்பு LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    எனில் (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  இல்லையெனில் {  /* restassign -> '=' explist */
    முழு nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    எனில் (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    இல்லையெனில் {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      திரும்பு;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


நிலையான முழு cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  கோவை(ls, &v);  /* read condition */
  எனில் (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  திரும்பு v.f;
}


நிலையான வெறுமை gotostat (LexState *ls, முழு வரி) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, வரி);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
நிலையான வெறுமை breakstat (LexState *ls, முழு வரி) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  க்கு (bl = ls->fs->bl; bl != வெற்று; bl = bl->previous) {
    எனில் (bl->isloop)  /* found one? */
      செல் ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, வரி);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
நிலையான வெறுமை checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  எனில் (l_unlikely(lb != வெற்று))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->வரி);  /* error */
}


நிலையான வெறுமை labelstat (LexState *ls, TString *name, முழு வரி) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  போது (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, வரி, block_follow(ls, 0));
}


நிலையான வெறுமை whilestat (LexState *ls, முழு வரி) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  முழு whileinit;
  முழு condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, வரி);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


நிலையான வெறுமை repeatstat (LexState *ls, முழு வரி) {
  /* repeatstat -> REPEAT block UNTIL cond */
  முழு condexit;
  FuncState *fs = ls->fs;
  முழு repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, வரி);
  condexit = cond(ls);  /* read condition (inside scope block) */
  எனில் (bl2.upval) {  /* upvalues? */
    முழு exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
நிலையான வெறுமை exp1 (LexState *ls) {
  expdesc e;
  கோவை(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
நிலையான வெறுமை fixforjump (FuncState *fs, முழு pc, முழு dest, முழு back) {
  Instruction *jmp = &fs->f->code[pc];
  முழு offset = dest - (pc + 1);
  எனில் (back)
    offset = -offset;
  எனில் (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
நிலையான வெறுமை forbody (LexState *ls, முழு base, முழு வரி, முழு nvars, முழு isgen) {
  /* forbody -> DO block */
  நிலையான மாறிலி OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  நிலையான மாறிலி OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  முழு prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  எனில் (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, வரி);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, வரி);
}


நிலையான வெறுமை fornum (LexState *ls, TString *varname, முழு வரி) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  முழு base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  எனில் (testnext(ls, ','))
    exp1(ls);  /* optional step */
  இல்லையெனில் {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, வரி, 1, 0);
}


நிலையான வெறுமை forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  முழு nvars = 4;  /* function, state, closing, control */
  முழு வரி;
  முழு base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  போது (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  வரி = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, வரி, nvars - 3, 1);
}


நிலையான வெறுமை forstat (LexState *ls, முழு வரி) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  தேர்வு (ls->t.token) {
    நிலை '=': fornum(ls, varname, வரி); நிறுத்து;
    நிலை ',': நிலை TK_IN: forlist(ls, varname); நிறுத்து;
    இயல்பு: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, வரி);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


நிலையான வெறுமை test_then_block (LexState *ls, முழு *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  முழு condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  எனில் (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


நிலையான வெறுமை ifstat (LexState *ls, முழு வரி) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  முழு escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  போது (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  எனில் (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, வரி);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


நிலையான வெறுமை localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  முழு fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


நிலையான lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  எனில் (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    மாறிலி எழுத்து *attr = getstr(ts);
    checknext(ls, '>');
    எனில் (strcmp(attr, "const") == 0)
      திரும்பு RDKCONST;  /* read-only variable */
    இல்லையெனில் எனில் (strcmp(attr, "close") == 0)
      திரும்பு RDKTOCLOSE;  /* to-be-closed variable */
    இல்லையெனில்
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  திரும்பு df;  /* return default value */
}


நிலையான வெறுமை checktoclose (FuncState *fs, முழு level) {
  எனில் (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


நிலையான வெறுமை localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  முழு toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  முழு vidx;  /* index of last variable */
  முழு nvars = 0;
  முழு nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  செய் {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    எனில் (kind == RDKTOCLOSE) {  /* to-be-closed? */
      எனில் (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } போது (testnext(ls, ','));
  எனில் (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  இல்லையெனில் {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  எனில் (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  இல்லையெனில் {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


நிலையான lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  தேர்வு (kind) {
    நிலை RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      திரும்பு kind;  /* to avoid warnings */
    நிலை RDKCONST:
      திரும்பு GDKCONST;  /* adjust kind for global variable */
    இயல்பு:
      திரும்பு kind;
  }
}


நிலையான வெறுமை checkglobal (LexState *ls, TString *varname, முழு வரி) {
  FuncState *fs = ls->fs;
  expdesc var;
  முழு k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, வரி);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
நிலையான வெறுமை initglobal (LexState *ls, முழு nvars, முழு firstidx, முழு n,
                        முழு வரி) {
  எனில் (n == nvars) {  /* traversed all variables? */
    expdesc e;
    முழு nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  இல்லையெனில் {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, வரி);
    leavelevel(ls);
    checkglobal(ls, varname, வரி);
    storevartop(fs, &var);
  }
}


நிலையான வெறுமை globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  முழு nvars = 0;
  முழு lastidx;  /* index of last registered variable */
  செய் {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } போது (testnext(ls, ','));
  எனில் (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


நிலையான வெறுமை globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  எனில் (!testnext(ls, '*'))
    globalnames(ls, defkind);
  இல்லையெனில் {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, வெற்று, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


நிலையான வெறுமை globalfunc (LexState *ls, முழு வரி) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, வரி);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, வரி);  /* definition "happens" in the first line */
}


நிலையான வெறுமை globalstatfunc (LexState *ls, முழு வரி) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  எனில் (testnext(ls, TK_FUNCTION))
    globalfunc(ls, வரி);
  இல்லையெனில்
    globalstat(ls);
}


நிலையான முழு funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  முழு ismethod = 0;
  singlevar(ls, v);
  போது (ls->t.token == '.')
    fieldsel(ls, v);
  எனில் (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  திரும்பு ismethod;
}


நிலையான வெறுமை funcstat (LexState *ls, முழு வரி) {
  /* funcstat -> FUNCTION funcname body */
  முழு ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, வரி);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, வரி);  /* definition "happens" in the first line */
}


நிலையான வெறுமை exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  கட்டமைப்பு LHS_assign v;
  suffixedexp(ls, &v.v);
  எனில் (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = வெற்று;
    restassign(ls, &v, 1);
  }
  இல்லையெனில் {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


நிலையான வெறுமை retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  முழு nret;  /* number of values being returned */
  முழு first = luaY_nvarstack(fs);  /* first slot to be returned */
  எனில் (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  இல்லையெனில் {
    nret = explist(ls, &e);  /* optional return values */
    எனில் (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      எனில் (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    இல்லையெனில் {
      எனில் (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      இல்லையெனில் {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


நிலையான வெறுமை statement (LexState *ls) {
  முழு வரி = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  தேர்வு (ls->t.token) {
    நிலை ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      நிறுத்து;
    }
    நிலை TK_IF: {  /* stat -> ifstat */
      ifstat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, வரி);
      நிறுத்து;
    }
    நிலை TK_FOR: {  /* stat -> forstat */
      forstat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      எனில் (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      இல்லையெனில்
        localstat(ls);
      நிறுத்து;
    }
    நிலை TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), வரி);
      நிறுத்து;
    }
    நிலை TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      நிறுத்து;
    }
    நிலை TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, வரி);
      நிறுத்து;
    }
    நிலை TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, வரி);
      நிறுத்து;
    }
#எனில் LUA_COMPAT_GLOBAL
    நிலை TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      எனில் (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        முழு lk = luaX_lookahead(ls);
        எனில் (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, வரி);
          நிறுத்து;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    இயல்பு: {  /* stat -> func | assignment */
      exprstat(ls);
      நிறுத்து;
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
நிலையான வெறுமை mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, மாறிலி எழுத்து *name, முழு firstchar) {
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
  திரும்பு cl;
}

