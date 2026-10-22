/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lcode.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "llex.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lopcodes.h"
#અન્તર્ભાવ "lparser.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"



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
પ્રકારવ્યાખ્યા સંરચના BlockCnt {
  સંરચના BlockCnt *previous;  /* chain */
  પૂર્ણાંક firstlabel;  /* index of first label in this block */
  પૂર્ણાંક firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
સ્થિર શૂન્ય statement (LexState *ls);
સ્થિર શૂન્ય પદાવલિ (LexState *ls, expdesc *v);


સ્થિર l_noret error_expected (LexState *ls, પૂર્ણાંક token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


સ્થિર l_noret errorlimit (FuncState *fs, પૂર્ણાંક limit, અચલ અક્ષર *what) {
  lua_State *L = fs->ls->L;
  અચલ અક્ષર *સંદેશ;
  પૂર્ણાંક લીટી = fs->f->linedefined;
  અચલ અક્ષર *where = (લીટી == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", લીટી);
  સંદેશ = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, સંદેશ);
}


શૂન્ય luaY_checklimit (FuncState *fs, પૂર્ણાંક v, પૂર્ણાંક l, અચલ અક્ષર *what) {
  જો (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
સ્થિર પૂર્ણાંક testnext (LexState *ls, પૂર્ણાંક c) {
  જો (ls->t.token == c) {
    luaX_next(ls);
    પાછા 1;
  }
  નહીંતર પાછા 0;
}


/*
** Check that next token is 'c'.
*/
સ્થિર શૂન્ય check (LexState *ls, પૂર્ણાંક c) {
  જો (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
સ્થિર શૂન્ય checknext (LexState *ls, પૂર્ણાંક c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,સંદેશ)	{ જો (!(c)) luaX_syntaxerror(ls, સંદેશ); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
સ્થિર શૂન્ય check_match (LexState *ls, પૂર્ણાંક what, પૂર્ણાંક who, પૂર્ણાંક where) {
  જો (l_unlikely(!testnext(ls, what))) {
    જો (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    નહીંતર {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


સ્થિર TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  પાછા ts;
}


સ્થિર શૂન્ય init_exp (expdesc *e, expkind k, પૂર્ણાંક i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


સ્થિર શૂન્ય codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


સ્થિર શૂન્ય codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
સ્થિર short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  પૂર્ણાંક oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  જ્યારે (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ખાલી;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  પાછા fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
સ્થિર પૂર્ણાંક new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  પાછા dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
સ્થિર પૂર્ણાંક new_localvar (LexState *ls, TString *name) {
  પાછા new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (કદ(v)/કદ(અક્ષર)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
સ્થિર Vardesc *getlocalvardesc (FuncState *fs, પૂર્ણાંક vidx) {
  પાછા &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
સ્થિર lu_byte reglevel (FuncState *fs, પૂર્ણાંક nvar) {
  જ્યારે (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    જો (varinreg(vd))  /* is in a register? */
      પાછા cast_byte(vd->vd.ridx + 1);
  }
  પાછા 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  પાછા reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
સ્થિર LocVar *localdebuginfo (FuncState *fs, પૂર્ણાંક vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  જો (!varinreg(vd))
    પાછા ખાલી;  /* no debug info. for constants */
  નહીંતર {
    પૂર્ણાંક idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    પાછા &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
સ્થિર શૂન્ય init_var (FuncState *fs, expdesc *e, પૂર્ણાંક vidx) {
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
સ્થિર શૂન્ય check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ખાલી;  /* to be set if variable is const */
  પસંદગી (e->k) {
    સ્થિતિ VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      તોડો;
    }
    સ્થિતિ VLOCAL: સ્થિતિ VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      જો (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      તોડો;
    }
    સ્થિતિ VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      જો (up->kind != VDKREG)
        varname = up->name;
      તોડો;
    }
    સ્થિતિ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    સ્થિતિ VINDEXUP: સ્થિતિ VINDEXSTR: સ્થિતિ VINDEXED: {  /* global variable */
      જો (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      તોડો;
    }
    મૂળભૂત:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      પાછા;  /* integer index cannot be read-only */
  }
  જો (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
સ્થિર શૂન્ય adjustlocalvars (LexState *ls, પૂર્ણાંક nvars) {
  FuncState *fs = ls->fs;
  પૂર્ણાંક reglevel = luaY_nvarstack(fs);
  પૂર્ણાંક i;
  માટે (i = 0; i < nvars; i++) {
    પૂર્ણાંક vidx = fs->nactvar++;
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
સ્થિર શૂન્ય removevars (FuncState *fs, પૂર્ણાંક tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  જ્યારે (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    જો (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
સ્થિર પૂર્ણાંક searchupvalue (FuncState *fs, TString *name) {
  પૂર્ણાંક i;
  Upvaldesc *up = fs->f->upvalues;
  માટે (i = 0; i < fs->nups; i++) {
    જો (eqstr(up[i].name, name)) પાછા i;
  }
  પાછા -1;  /* not found */
}


સ્થિર Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  પૂર્ણાંક oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  જ્યારે (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ખાલી;
  પાછા &f->upvalues[fs->nups++];
}


સ્થિર પૂર્ણાંક newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  જો (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  નહીંતર {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  પાછા fs->nups - 1;
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
સ્થિર પૂર્ણાંક searchvar (FuncState *fs, TString *n, expdesc *var) {
  પૂર્ણાંક i;
  માટે (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    જો (varglobal(vd)) {  /* global declaration? */
      જો (vd->vd.name == ખાલી) {  /* collective declaration? */
        જો (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      નહીંતર {  /* global name */
        જો (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          પાછા VGLOBAL;
        }
        નહીંતર જો (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    નહીંતર જો (eqstr(n, vd->vd.name)) {  /* found? */
      જો (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      નહીંતર {  /* local variable */
        init_var(fs, var, i);
        જો (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      પાછા cast_int(var->k);
    }
  }
  પાછા -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
સ્થિર શૂન્ય markupval (FuncState *fs, પૂર્ણાંક level) {
  BlockCnt *bl = fs->bl;
  જ્યારે (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
સ્થિર શૂન્ય marktobeclosed (FuncState *fs) {
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
સ્થિર શૂન્ય singlevaraux (FuncState *fs, TString *n, expdesc *var, પૂર્ણાંક base) {
  પૂર્ણાંક v = searchvar(fs, n, var);  /* look up variables at current level */
  જો (v >= 0) {  /* found? */
    જો (!base) {
      જો (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      જો (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  નહીંતર {  /* not found at current level; try upvalues */
    પૂર્ણાંક idx = searchupvalue(fs, n);  /* try existing upvalues */
    જો (idx < 0) {  /* not found? */
      જો (fs->prev != ખાલી)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      જો (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      નહીંતર  /* it is a global or a constant */
        પાછા;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


સ્થિર શૂન્ય buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc ચાવી;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  જો (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&ચાવી, varname);  /* key is variable name */
  luaK_indexed(fs, var, &ચાવી);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
સ્થિર શૂન્ય buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  જો (var->k == VGLOBAL) {  /* global name? */
    પૂર્ણાંક info = var->u.info;
    /* global by default in the scope of a global declaration? */
    જો (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    જો (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    નહીંતર  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


સ્થિર શૂન્ય singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
સ્થિર શૂન્ય adjust_assign (LexState *ls, પૂર્ણાંક nvars, પૂર્ણાંક nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  પૂર્ણાંક needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  જો (hasmultret(e->k)) {  /* last expression has multiple returns? */
    પૂર્ણાંક extra = needed + 1;  /* discount last expression itself */
    જો (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  નહીંતર {
    જો (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    જો (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  જો (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  નહીંતર  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
સ્થિર l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  અચલ અક્ષર *varname = (tsname != ખાલી) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->લીટી, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
સ્થિર શૂન્ય closegoto (LexState *ls, પૂર્ણાંક g, Labeldesc *label, પૂર્ણાંક bup) {
  પૂર્ણાંક i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  જો (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  જો (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  માટે (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
સ્થિર Labeldesc *findlabel (LexState *ls, TString *name, પૂર્ણાંક ilb) {
  Dyndata *dyd = ls->dyd;
  માટે (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    જો (eqstr(lb->name, name))  /* correct label? */
      પાછા lb;
  }
  પાછા ખાલી;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
સ્થિર પૂર્ણાંક newlabelentry (LexState *ls, Labellist *l, TString *name,
                          પૂર્ણાંક લીટી, પૂર્ણાંક pc) {
  પૂર્ણાંક n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].લીટી = લીટી;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  પાછા n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
સ્થિર પૂર્ણાંક newgotoentry (LexState *ls, TString *name, પૂર્ણાંક લીટી) {
  FuncState *fs = ls->fs;
  પૂર્ણાંક pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  પાછા newlabelentry(ls, &ls->dyd->gt, name, લીટી, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
સ્થિર શૂન્ય createlabel (LexState *ls, TString *name, પૂર્ણાંક લીટી, પૂર્ણાંક last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  પૂર્ણાંક l = newlabelentry(ls, ll, name, લીટી, luaK_getlabel(fs));
  જો (last) {  /* label is last no-op statement in the block? */
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
સ્થિર શૂન્ય solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  પૂર્ણાંક outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  પૂર્ણાંક igt = bl->firstgoto;  /* first goto in the finishing block */
  જ્યારે (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    જો (lb != ખાલી)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    નહીંતર {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      જો (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


સ્થિર શૂન્ય enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ખાલી && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
સ્થિર l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->લીટી);
}


સ્થિર શૂન્ય leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  જો (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  જો (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  જો (bl->previous == ખાલી) {  /* was it the last block? */
    જો (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
સ્થિર Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  જો (fs->np >= f->sizep) {
    પૂર્ણાંક oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    જ્યારે (oldsize < f->sizep)
      f->p[oldsize++] = ખાલી;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  પાછા clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
સ્થિર શૂન્ય codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


સ્થિર શૂન્ય open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = ખાલી;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


સ્થિર શૂન્ય close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ખાલી);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_લીટી_Info);
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
સ્થિર પૂર્ણાંક block_follow (LexState *ls, પૂર્ણાંક withuntil) {
  પસંદગી (ls->t.token) {
    સ્થિતિ TK_ELSE: સ્થિતિ TK_ELSEIF:
    સ્થિતિ TK_END: સ્થિતિ TK_EOS:
      પાછા 1;
    સ્થિતિ TK_UNTIL: પાછા withuntil;
    મૂળભૂત: પાછા 0;
  }
}


સ્થિર શૂન્ય statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  જ્યારે (!block_follow(ls, 1)) {
    જો (ls->t.token == TK_RETURN) {
      statement(ls);
      પાછા;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


સ્થિર શૂન્ય fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc ચાવી;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &ચાવી);
  luaK_indexed(fs, v, &ચાવી);
}


સ્થિર શૂન્ય yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  પદાવલિ(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

પ્રકારવ્યાખ્યા સંરચના ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  પૂર્ણાંક nh;  /* total number of 'record' elements */
  પૂર્ણાંક na;  /* number of array elements already stored */
  પૂર્ણાંક tostore;  /* number of array elements pending to be stored */
  પૂર્ણાંક maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#જો MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


સ્થિર શૂન્ય recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, ચાવી, val;
  જો (ls->t.token == TK_NAME)
    codename(ls, &ચાવી);
  નહીંતર  /* ls->t.token == '[' */
    yindex(ls, &ચાવી);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &ચાવી);
  પદાવલિ(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


સ્થિર શૂન્ય closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  જો (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


સ્થિર શૂન્ય lastlistfield (FuncState *fs, ConsControl *cc) {
  જો (cc->tostore == 0) પાછા;
  જો (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  નહીંતર {
    જો (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


સ્થિર શૂન્ય listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  પદાવલિ(ls, &cc->v);
  cc->tostore++;
}


સ્થિર શૂન્ય field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  પસંદગી(ls->t.token) {
    સ્થિતિ TK_NAME: {  /* may be 'listfield' or 'recfield' */
      જો (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      નહીંતર
        recfield(ls, cc);
      તોડો;
    }
    સ્થિતિ '[': {
      recfield(ls, cc);
      તોડો;
    }
    મૂળભૂત: {
      listfield(ls, cc);
      તોડો;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
સ્થિર પૂર્ણાંક maxtostore (FuncState *fs) {
  પૂર્ણાંક numfreeregs = MAX_FSTACK - fs->freereg;
  જો (numfreeregs >= 160)  /* "lots" of registers? */
    પાછા numfreeregs / 5;  /* use up to 1/5 of them */
  નહીંતર જો (numfreeregs >= 80)  /* still "enough" registers? */
    પાછા 10;  /* one 'SETLIST' instruction for each 10 values */
  નહીંતર  /* save registers for potential more nesting */
    પાછા 1;
}


સ્થિર શૂન્ય constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  પૂર્ણાંક લીટી = ls->linenumber;
  પૂર્ણાંક pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  કરો {
    જો (ls->t.token == /*{*/ '}') તોડો;
    જો (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } જ્યારે (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, લીટી);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


સ્થિર શૂન્ય setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


સ્થિર શૂન્ય parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  પૂર્ણાંક nparams = 0;
  પૂર્ણાંક varargk = 0;
  જો (ls->t.token != ')') {  /* is 'parlist' not empty? */
    કરો {
      પસંદગી (ls->t.token) {
        સ્થિતિ TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          તોડો;
        }
        સ્થિતિ TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          જો (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          નહીંતર
            new_localvarliteral(ls, "(vararg table)");
          તોડો;
        }
        મૂળભૂત: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } જ્યારે (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  જો (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


સ્થિર શૂન્ય body (LexState *ls, expdesc *e, પૂર્ણાંક ismethod, પૂર્ણાંક લીટી) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = લીટી;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  જો (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, લીટી);
  codeclosure(ls, e);
  close_func(ls);
}


સ્થિર પૂર્ણાંક explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  પૂર્ણાંક n = 1;  /* at least one expression */
  પદાવલિ(ls, v);
  જ્યારે (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    પદાવલિ(ls, v);
    n++;
  }
  પાછા n;
}


સ્થિર શૂન્ય funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  પૂર્ણાંક base, nparams;
  પૂર્ણાંક લીટી = ls->linenumber;
  પસંદગી (ls->t.token) {
    સ્થિતિ '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      જો (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      નહીંતર {
        explist(ls, &args);
        જો (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', લીટી);
      તોડો;
    }
    સ્થિતિ '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      તોડો;
    }
    સ્થિતિ TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      તોડો;
    }
    મૂળભૂત: {
      luaX_syntaxerror(ls, "function arguments expected");
      પાછા;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  જો (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  નહીંતર {
    જો (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, લીટી);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


સ્થિર શૂન્ય primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  પસંદગી (ls->t.token) {
    સ્થિતિ '(': {
      પૂર્ણાંક લીટી = ls->linenumber;
      luaX_next(ls);
      પદાવલિ(ls, v);
      check_match(ls, ')', '(', લીટી);
      luaK_dischargevars(ls->fs, v);
      પાછા;
    }
    સ્થિતિ TK_NAME: {
      singlevar(ls, v);
      પાછા;
    }
    મૂળભૂત: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


સ્થિર શૂન્ય suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  માટે (;;) {
    પસંદગી (ls->t.token) {
      સ્થિતિ '.': {  /* fieldsel */
        fieldsel(ls, v);
        તોડો;
      }
      સ્થિતિ '[': {  /* '[' exp ']' */
        expdesc ચાવી;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &ચાવી);
        luaK_indexed(fs, v, &ચાવી);
        તોડો;
      }
      સ્થિતિ ':': {  /* ':' NAME funcargs */
        expdesc ચાવી;
        luaX_next(ls);
        codename(ls, &ચાવી);
        luaK_self(fs, v, &ચાવી);
        funcargs(ls, v);
        તોડો;
      }
      સ્થિતિ '(': સ્થિતિ TK_STRING: સ્થિતિ '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        તોડો;
      }
      મૂળભૂત: પાછા;
    }
  }
}


સ્થિર શૂન્ય simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  પસંદગી (ls->t.token) {
    સ્થિતિ TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      તોડો;
    }
    સ્થિતિ TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      તોડો;
    }
    સ્થિતિ TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      તોડો;
    }
    સ્થિતિ TK_NIL: {
      init_exp(v, VNIL, 0);
      તોડો;
    }
    સ્થિતિ TK_TRUE: {
      init_exp(v, VTRUE, 0);
      તોડો;
    }
    સ્થિતિ TK_FALSE: {
      init_exp(v, VFALSE, 0);
      તોડો;
    }
    સ્થિતિ TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      તોડો;
    }
    સ્થિતિ '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      પાછા;
    }
    સ્થિતિ TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      પાછા;
    }
    મૂળભૂત: {
      suffixedexp(ls, v);
      પાછા;
    }
  }
  luaX_next(ls);
}


સ્થિર UnOpr getunopr (પૂર્ણાંક op) {
  પસંદગી (op) {
    સ્થિતિ TK_NOT: પાછા OPR_NOT;
    સ્થિતિ '-': પાછા OPR_MINUS;
    સ્થિતિ '~': પાછા OPR_BNOT;
    સ્થિતિ '#': પાછા OPR_LEN;
    મૂળભૂત: પાછા OPR_NOUNOPR;
  }
}


સ્થિર BinOpr getbinopr (પૂર્ણાંક op) {
  પસંદગી (op) {
    સ્થિતિ '+': પાછા OPR_ADD;
    સ્થિતિ '-': પાછા OPR_SUB;
    સ્થિતિ '*': પાછા OPR_MUL;
    સ્થિતિ '%': પાછા OPR_MOD;
    સ્થિતિ '^': પાછા OPR_POW;
    સ્થિતિ '/': પાછા OPR_DIV;
    સ્થિતિ TK_IDIV: પાછા OPR_IDIV;
    સ્થિતિ '&': પાછા OPR_BAND;
    સ્થિતિ '|': પાછા OPR_BOR;
    સ્થિતિ '~': પાછા OPR_BXOR;
    સ્થિતિ TK_SHL: પાછા OPR_SHL;
    સ્થિતિ TK_SHR: પાછા OPR_SHR;
    સ્થિતિ TK_CONCAT: પાછા OPR_CONCAT;
    સ્થિતિ TK_NE: પાછા OPR_NE;
    સ્થિતિ TK_EQ: પાછા OPR_EQ;
    સ્થિતિ '<': પાછા OPR_LT;
    સ્થિતિ TK_LE: પાછા OPR_LE;
    સ્થિતિ '>': પાછા OPR_GT;
    સ્થિતિ TK_GE: પાછા OPR_GE;
    સ્થિતિ TK_AND: પાછા OPR_AND;
    સ્થિતિ TK_OR: પાછા OPR_OR;
    મૂળભૂત: પાછા OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
સ્થિર અચલ સંરચના {
  lu_byte ડાબું;  /* left priority for each binary operator */
  lu_byte જમણું; /* right priority */
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
સ્થિર BinOpr subexpr (LexState *ls, expdesc *v, પૂર્ણાંક limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  જો (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    પૂર્ણાંક લીટી = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, લીટી);
  }
  નહીંતર simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  જ્યારે (op != OPR_NOBINOPR && priority[op].ડાબું > limit) {
    expdesc v2;
    BinOpr nextop;
    પૂર્ણાંક લીટી = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].જમણું);
    luaK_posfix(ls->fs, op, v, &v2, લીટી);
    op = nextop;
  }
  leavelevel(ls);
  પાછા op;  /* return first untreated operator */
}


સ્થિર શૂન્ય પદાવલિ (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


સ્થિર શૂન્ય block (LexState *ls) {
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
સંરચના LHS_assign {
  સંરચના LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
સ્થિર શૂન્ય check_conflict (LexState *ls, સંરચના LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  પૂર્ણાંક conflict = 0;
  માટે (; lh; lh = lh->prev) {  /* check all previous assignments */
    જો (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      જો (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        જો (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      નહીંતર {  /* table is a register */
        જો (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        જો (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  જો (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    જો (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    નહીંતર
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
સ્થિર શૂન્ય storevartop (FuncState *fs, expdesc *var) {
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
સ્થિર શૂન્ય restassign (LexState *ls, સંરચના LHS_assign *lh, પૂર્ણાંક nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  જો (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    સંરચના LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    જો (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  નહીંતર {  /* restassign -> '=' explist */
    પૂર્ણાંક nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    જો (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    નહીંતર {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      પાછા;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


સ્થિર પૂર્ણાંક cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  પદાવલિ(ls, &v);  /* read condition */
  જો (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  પાછા v.f;
}


સ્થિર શૂન્ય gotostat (LexState *ls, પૂર્ણાંક લીટી) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, લીટી);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
સ્થિર શૂન્ય breakstat (LexState *ls, પૂર્ણાંક લીટી) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  માટે (bl = ls->fs->bl; bl != ખાલી; bl = bl->previous) {
    જો (bl->isloop)  /* found one? */
      જાઓ ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, લીટી);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
સ્થિર શૂન્ય checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  જો (l_unlikely(lb != ખાલી))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->લીટી);  /* error */
}


સ્થિર શૂન્ય labelstat (LexState *ls, TString *name, પૂર્ણાંક લીટી) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  જ્યારે (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, લીટી, block_follow(ls, 0));
}


સ્થિર શૂન્ય whilestat (LexState *ls, પૂર્ણાંક લીટી) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  પૂર્ણાંક whileinit;
  પૂર્ણાંક condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, લીટી);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


સ્થિર શૂન્ય repeatstat (LexState *ls, પૂર્ણાંક લીટી) {
  /* repeatstat -> REPEAT block UNTIL cond */
  પૂર્ણાંક condexit;
  FuncState *fs = ls->fs;
  પૂર્ણાંક repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, લીટી);
  condexit = cond(ls);  /* read condition (inside scope block) */
  જો (bl2.upval) {  /* upvalues? */
    પૂર્ણાંક exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
સ્થિર શૂન્ય exp1 (LexState *ls) {
  expdesc e;
  પદાવલિ(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
સ્થિર શૂન્ય fixforjump (FuncState *fs, પૂર્ણાંક pc, પૂર્ણાંક dest, પૂર્ણાંક back) {
  Instruction *jmp = &fs->f->code[pc];
  પૂર્ણાંક offset = dest - (pc + 1);
  જો (back)
    offset = -offset;
  જો (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
સ્થિર શૂન્ય forbody (LexState *ls, પૂર્ણાંક base, પૂર્ણાંક લીટી, પૂર્ણાંક nvars, પૂર્ણાંક isgen) {
  /* forbody -> DO block */
  સ્થિર અચલ OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  સ્થિર અચલ OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  પૂર્ણાંક prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  જો (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, લીટી);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, લીટી);
}


સ્થિર શૂન્ય fornum (LexState *ls, TString *varname, પૂર્ણાંક લીટી) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  પૂર્ણાંક base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  જો (testnext(ls, ','))
    exp1(ls);  /* optional step */
  નહીંતર {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, લીટી, 1, 0);
}


સ્થિર શૂન્ય forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  પૂર્ણાંક nvars = 4;  /* function, state, closing, control */
  પૂર્ણાંક લીટી;
  પૂર્ણાંક base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  જ્યારે (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  લીટી = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, લીટી, nvars - 3, 1);
}


સ્થિર શૂન્ય forstat (LexState *ls, પૂર્ણાંક લીટી) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  પસંદગી (ls->t.token) {
    સ્થિતિ '=': fornum(ls, varname, લીટી); તોડો;
    સ્થિતિ ',': સ્થિતિ TK_IN: forlist(ls, varname); તોડો;
    મૂળભૂત: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, લીટી);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


સ્થિર શૂન્ય test_then_block (LexState *ls, પૂર્ણાંક *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  પૂર્ણાંક condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  જો (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


સ્થિર શૂન્ય ifstat (LexState *ls, પૂર્ણાંક લીટી) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  પૂર્ણાંક escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  જ્યારે (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  જો (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, લીટી);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


સ્થિર શૂન્ય localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  પૂર્ણાંક fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


સ્થિર lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  જો (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    અચલ અક્ષર *attr = getstr(ts);
    checknext(ls, '>');
    જો (strcmp(attr, "const") == 0)
      પાછા RDKCONST;  /* read-only variable */
    નહીંતર જો (strcmp(attr, "close") == 0)
      પાછા RDKTOCLOSE;  /* to-be-closed variable */
    નહીંતર
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  પાછા df;  /* return default value */
}


સ્થિર શૂન્ય checktoclose (FuncState *fs, પૂર્ણાંક level) {
  જો (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


સ્થિર શૂન્ય localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  પૂર્ણાંક toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  પૂર્ણાંક vidx;  /* index of last variable */
  પૂર્ણાંક nvars = 0;
  પૂર્ણાંક nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  કરો {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    જો (kind == RDKTOCLOSE) {  /* to-be-closed? */
      જો (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } જ્યારે (testnext(ls, ','));
  જો (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  નહીંતર {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  જો (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  નહીંતર {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


સ્થિર lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  પસંદગી (kind) {
    સ્થિતિ RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      પાછા kind;  /* to avoid warnings */
    સ્થિતિ RDKCONST:
      પાછા GDKCONST;  /* adjust kind for global variable */
    મૂળભૂત:
      પાછા kind;
  }
}


સ્થિર શૂન્ય checkglobal (LexState *ls, TString *varname, પૂર્ણાંક લીટી) {
  FuncState *fs = ls->fs;
  expdesc var;
  પૂર્ણાંક k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, લીટી);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
સ્થિર શૂન્ય initglobal (LexState *ls, પૂર્ણાંક nvars, પૂર્ણાંક firstidx, પૂર્ણાંક n,
                        પૂર્ણાંક લીટી) {
  જો (n == nvars) {  /* traversed all variables? */
    expdesc e;
    પૂર્ણાંક nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  નહીંતર {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, લીટી);
    leavelevel(ls);
    checkglobal(ls, varname, લીટી);
    storevartop(fs, &var);
  }
}


સ્થિર શૂન્ય globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  પૂર્ણાંક nvars = 0;
  પૂર્ણાંક lastidx;  /* index of last registered variable */
  કરો {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } જ્યારે (testnext(ls, ','));
  જો (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


સ્થિર શૂન્ય globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  જો (!testnext(ls, '*'))
    globalnames(ls, defkind);
  નહીંતર {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ખાલી, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


સ્થિર શૂન્ય globalfunc (LexState *ls, પૂર્ણાંક લીટી) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, લીટી);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, લીટી);  /* definition "happens" in the first line */
}


સ્થિર શૂન્ય globalstatfunc (LexState *ls, પૂર્ણાંક લીટી) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  જો (testnext(ls, TK_FUNCTION))
    globalfunc(ls, લીટી);
  નહીંતર
    globalstat(ls);
}


સ્થિર પૂર્ણાંક funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  પૂર્ણાંક ismethod = 0;
  singlevar(ls, v);
  જ્યારે (ls->t.token == '.')
    fieldsel(ls, v);
  જો (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  પાછા ismethod;
}


સ્થિર શૂન્ય funcstat (LexState *ls, પૂર્ણાંક લીટી) {
  /* funcstat -> FUNCTION funcname body */
  પૂર્ણાંક ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, લીટી);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, લીટી);  /* definition "happens" in the first line */
}


સ્થિર શૂન્ય exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  સંરચના LHS_assign v;
  suffixedexp(ls, &v.v);
  જો (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ખાલી;
    restassign(ls, &v, 1);
  }
  નહીંતર {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


સ્થિર શૂન્ય retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  પૂર્ણાંક nret;  /* number of values being returned */
  પૂર્ણાંક first = luaY_nvarstack(fs);  /* first slot to be returned */
  જો (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  નહીંતર {
    nret = explist(ls, &e);  /* optional return values */
    જો (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      જો (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    નહીંતર {
      જો (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      નહીંતર {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


સ્થિર શૂન્ય statement (LexState *ls) {
  પૂર્ણાંક લીટી = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  પસંદગી (ls->t.token) {
    સ્થિતિ ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      તોડો;
    }
    સ્થિતિ TK_IF: {  /* stat -> ifstat */
      ifstat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, લીટી);
      તોડો;
    }
    સ્થિતિ TK_FOR: {  /* stat -> forstat */
      forstat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      જો (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      નહીંતર
        localstat(ls);
      તોડો;
    }
    સ્થિતિ TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), લીટી);
      તોડો;
    }
    સ્થિતિ TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      તોડો;
    }
    સ્થિતિ TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, લીટી);
      તોડો;
    }
    સ્થિતિ TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, લીટી);
      તોડો;
    }
#જો LUA_COMPAT_GLOBAL
    સ્થિતિ TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      જો (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        પૂર્ણાંક lk = luaX_lookahead(ls);
        જો (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, લીટી);
          તોડો;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    મૂળભૂત: {  /* stat -> func | assignment */
      exprstat(ls);
      તોડો;
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
સ્થિર શૂન્ય mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, અચલ અક્ષર *name, પૂર્ણાંક firstchar) {
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
  પાછા cl;
}

