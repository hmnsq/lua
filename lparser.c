/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lcode.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "llex.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lopcodes.h"
#ضمن "lparser.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"



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
تعريفنوع بنية BlockCnt {
  بنية BlockCnt *previous;  /* chain */
  صحيح firstlabel;  /* index of first label in this block */
  صحيح firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
ساكن فراغ statement (LexState *ls);
ساكن فراغ expr (LexState *ls, expdesc *v);


ساكن l_noret error_expected (LexState *ls, صحيح token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ساكن l_noret errorlimit (FuncState *fs, صحيح limit, ثابت حرف *what) {
  lua_State *L = fs->ls->L;
  ثابت حرف *msg;
  صحيح line = fs->f->linedefined;
  ثابت حرف *where = (line == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", line);
  msg = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, msg);
}


فراغ luaY_checklimit (FuncState *fs, صحيح v, صحيح l, ثابت حرف *what) {
  אם (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ساكن صحيح testnext (LexState *ls, صحيح c) {
  אם (ls->t.token == c) {
    luaX_next(ls);
    החזר 1;
  }
  אחרת החזר 0;
}


/*
** Check that next token is 'c'.
*/
ساكن فراغ check (LexState *ls, صحيح c) {
  אם (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ساكن فراغ checknext (LexState *ls, صحيح c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,msg)	{ אם (!(c)) luaX_syntaxerror(ls, msg); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ساكن فراغ check_match (LexState *ls, صحيح what, صحيح who, صحيح where) {
  אם (l_unlikely(!testnext(ls, what))) {
    אם (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    אחרת {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


ساكن TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  החזר ts;
}


ساكن فراغ init_exp (expdesc *e, expkind k, صحيح i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


ساكن فراغ codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


ساكن فراغ codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
ساكن short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  صحيح oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  כלעוד (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = عدم;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  החזר fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ساكن صحيح new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  החזר dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ساكن صحيح new_localvar (LexState *ls, TString *name) {
  החזר new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (حجم(v)/حجم(حرف)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ساكن Vardesc *getlocalvardesc (FuncState *fs, صحيح vidx) {
  החזר &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ساكن lu_byte reglevel (FuncState *fs, صحيح nvar) {
  כלעוד (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    אם (varinreg(vd))  /* is in a register? */
      החזר cast_byte(vd->vd.ridx + 1);
  }
  החזר 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  החזר reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ساكن LocVar *localdebuginfo (FuncState *fs, صحيح vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  אם (!varinreg(vd))
    החזר عدم;  /* no debug info. for constants */
  אחרת {
    صحيح idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    החזר &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ساكن فراغ init_var (FuncState *fs, expdesc *e, صحيح vidx) {
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
ساكن فراغ check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = عدم;  /* to be set if variable is const */
  بدل (e->k) {
    حالة VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      اكسر;
    }
    حالة VLOCAL: حالة VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      אם (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      اكسر;
    }
    حالة VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      אם (up->kind != VDKREG)
        varname = up->name;
      اكسر;
    }
    حالة VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    حالة VINDEXUP: حالة VINDEXSTR: حالة VINDEXED: {  /* global variable */
      אם (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      اكسر;
    }
    افتراضي:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      החזר;  /* integer index cannot be read-only */
  }
  אם (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ساكن فراغ adjustlocalvars (LexState *ls, صحيح nvars) {
  FuncState *fs = ls->fs;
  صحيح reglevel = luaY_nvarstack(fs);
  صحيح i;
  עבור (i = 0; i < nvars; i++) {
    صحيح vidx = fs->nactvar++;
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
ساكن فراغ removevars (FuncState *fs, صحيح tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  כלעוד (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    אם (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ساكن صحيح searchupvalue (FuncState *fs, TString *name) {
  صحيح i;
  Upvaldesc *up = fs->f->upvalues;
  עבור (i = 0; i < fs->nups; i++) {
    אם (eqstr(up[i].name, name)) החזר i;
  }
  החזר -1;  /* not found */
}


ساكن Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  صحيح oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  כלעוד (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = عدم;
  החזר &f->upvalues[fs->nups++];
}


ساكن صحيح newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  אם (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  אחרת {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  החזר fs->nups - 1;
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
ساكن صحيح searchvar (FuncState *fs, TString *n, expdesc *var) {
  صحيح i;
  עבור (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    אם (varglobal(vd)) {  /* global declaration? */
      אם (vd->vd.name == عدم) {  /* collective declaration? */
        אם (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      אחרת {  /* global name */
        אם (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          החזר VGLOBAL;
        }
        אחרת אם (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    אחרת אם (eqstr(n, vd->vd.name)) {  /* found? */
      אם (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      אחרת {  /* local variable */
        init_var(fs, var, i);
        אם (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      החזר cast_int(var->k);
    }
  }
  החזר -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ساكن فراغ markupval (FuncState *fs, صحيح level) {
  BlockCnt *bl = fs->bl;
  כלעוד (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ساكن فراغ marktobeclosed (FuncState *fs) {
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
ساكن فراغ singlevaraux (FuncState *fs, TString *n, expdesc *var, صحيح base) {
  صحيح v = searchvar(fs, n, var);  /* look up variables at current level */
  אם (v >= 0) {  /* found? */
    אם (!base) {
      אם (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      אם (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  אחרת {  /* not found at current level; try upvalues */
    صحيح idx = searchupvalue(fs, n);  /* try existing upvalues */
    אם (idx < 0) {  /* not found? */
      אם (fs->prev != عدم)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      אם (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      אחרת  /* it is a global or a constant */
        החזר;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ساكن فراغ buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc key;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  אם (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&key, varname);  /* key is variable name */
  luaK_indexed(fs, var, &key);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ساكن فراغ buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  אם (var->k == VGLOBAL) {  /* global name? */
    صحيح info = var->u.info;
    /* global by default in the scope of a global declaration? */
    אם (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    אם (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    אחרת  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


ساكن فراغ singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ساكن فراغ adjust_assign (LexState *ls, صحيح nvars, صحيح nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  صحيح needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  אם (hasmultret(e->k)) {  /* last expression has multiple returns? */
    صحيح extra = needed + 1;  /* discount last expression itself */
    אם (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  אחרת {
    אם (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    אם (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  אם (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  אחרת  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ساكن l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ثابت حرف *varname = (tsname != عدم) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->line, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
ساكن فراغ closegoto (LexState *ls, صحيح g, Labeldesc *label, صحيح bup) {
  صحيح i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  אם (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  אם (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  עבור (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ساكن Labeldesc *findlabel (LexState *ls, TString *name, صحيح ilb) {
  Dyndata *dyd = ls->dyd;
  עבור (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    אם (eqstr(lb->name, name))  /* correct label? */
      החזר lb;
  }
  החזר عدم;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ساكن صحيح newlabelentry (LexState *ls, Labellist *l, TString *name,
                          صحيح line, صحيح pc) {
  صحيح n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  החזר n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ساكن صحيح newgotoentry (LexState *ls, TString *name, صحيح line) {
  FuncState *fs = ls->fs;
  صحيح pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  החזר newlabelentry(ls, &ls->dyd->gt, name, line, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ساكن فراغ createlabel (LexState *ls, TString *name, صحيح line, صحيح last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  صحيح l = newlabelentry(ls, ll, name, line, luaK_getlabel(fs));
  אם (last) {  /* label is last no-op statement in the block? */
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
ساكن فراغ solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  صحيح outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  صحيح igt = bl->firstgoto;  /* first goto in the finishing block */
  כלעוד (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    אם (lb != عدم)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    אחרת {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      אם (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ساكن فراغ enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != عدم && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
ساكن l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->line);
}


ساكن فراغ leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  אם (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  אם (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  אם (bl->previous == عدم) {  /* was it the last block? */
    אם (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ساكن Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  אם (fs->np >= f->sizep) {
    صحيح oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    כלעוד (oldsize < f->sizep)
      f->p[oldsize++] = عدم;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  החזר clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ساكن فراغ codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ساكن فراغ open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = عدم;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ساكن فراغ close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == عدم);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, AbsLineInfo);
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
ساكن صحيح block_follow (LexState *ls, صحيح withuntil) {
  بدل (ls->t.token) {
    حالة TK_ELSE: حالة TK_ELSEIF:
    حالة TK_END: حالة TK_EOS:
      החזר 1;
    حالة TK_UNTIL: החזר withuntil;
    افتراضي: החזר 0;
  }
}


ساكن فراغ statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  כלעוד (!block_follow(ls, 1)) {
    אם (ls->t.token == TK_RETURN) {
      statement(ls);
      החזר;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ساكن فراغ fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc key;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &key);
  luaK_indexed(fs, v, &key);
}


ساكن فراغ yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  expr(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

تعريفنوع بنية ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  صحيح nh;  /* total number of 'record' elements */
  صحيح na;  /* number of array elements already stored */
  صحيح tostore;  /* number of array elements pending to be stored */
  صحيح maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#אם MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


ساكن فراغ recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, key, val;
  אם (ls->t.token == TK_NAME)
    codename(ls, &key);
  אחרת  /* ls->t.token == '[' */
    yindex(ls, &key);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &key);
  expr(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ساكن فراغ closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  אם (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ساكن فراغ lastlistfield (FuncState *fs, ConsControl *cc) {
  אם (cc->tostore == 0) החזר;
  אם (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  אחרת {
    אם (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


ساكن فراغ listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  expr(ls, &cc->v);
  cc->tostore++;
}


ساكن فراغ field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  بدل(ls->t.token) {
    حالة TK_NAME: {  /* may be 'listfield' or 'recfield' */
      אם (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      אחרת
        recfield(ls, cc);
      اكسر;
    }
    حالة '[': {
      recfield(ls, cc);
      اكسر;
    }
    افتراضي: {
      listfield(ls, cc);
      اكسر;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ساكن صحيح maxtostore (FuncState *fs) {
  صحيح numfreeregs = MAX_FSTACK - fs->freereg;
  אם (numfreeregs >= 160)  /* "lots" of registers? */
    החזר numfreeregs / 5;  /* use up to 1/5 of them */
  אחרת אם (numfreeregs >= 80)  /* still "enough" registers? */
    החזר 10;  /* one 'SETLIST' instruction for each 10 values */
  אחרת  /* save registers for potential more nesting */
    החזר 1;
}


ساكن فراغ constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  صحيح line = ls->linenumber;
  صحيح pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  עשה {
    אם (ls->t.token == /*{*/ '}') اكسر;
    אם (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } כלעוד (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, line);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ساكن فراغ setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ساكن فراغ parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  صحيح nparams = 0;
  صحيح varargk = 0;
  אם (ls->t.token != ')') {  /* is 'parlist' not empty? */
    עשה {
      بدل (ls->t.token) {
        حالة TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          اكسر;
        }
        حالة TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          אם (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          אחרת
            new_localvarliteral(ls, "(vararg table)");
          اكسر;
        }
        افتراضي: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } כלעוד (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  אם (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ساكن فراغ body (LexState *ls, expdesc *e, صحيح ismethod, صحيح line) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = line;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  אם (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, line);
  codeclosure(ls, e);
  close_func(ls);
}


ساكن صحيح explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  صحيح n = 1;  /* at least one expression */
  expr(ls, v);
  כלעוד (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    expr(ls, v);
    n++;
  }
  החזר n;
}


ساكن فراغ funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  صحيح base, nparams;
  صحيح line = ls->linenumber;
  بدل (ls->t.token) {
    حالة '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      אם (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      אחרת {
        explist(ls, &args);
        אם (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', line);
      اكسر;
    }
    حالة '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      اكسر;
    }
    حالة TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      اكسر;
    }
    افتراضي: {
      luaX_syntaxerror(ls, "function arguments expected");
      החזר;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  אם (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  אחרת {
    אם (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, line);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ساكن فراغ primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  بدل (ls->t.token) {
    حالة '(': {
      صحيح line = ls->linenumber;
      luaX_next(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      החזר;
    }
    حالة TK_NAME: {
      singlevar(ls, v);
      החזר;
    }
    افتراضي: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ساكن فراغ suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  עבור (;;) {
    بدل (ls->t.token) {
      حالة '.': {  /* fieldsel */
        fieldsel(ls, v);
        اكسر;
      }
      حالة '[': {  /* '[' exp ']' */
        expdesc key;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &key);
        luaK_indexed(fs, v, &key);
        اكسر;
      }
      حالة ':': {  /* ':' NAME funcargs */
        expdesc key;
        luaX_next(ls);
        codename(ls, &key);
        luaK_self(fs, v, &key);
        funcargs(ls, v);
        اكسر;
      }
      حالة '(': حالة TK_STRING: حالة '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        اكسر;
      }
      افتراضي: החזר;
    }
  }
}


ساكن فراغ simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  بدل (ls->t.token) {
    حالة TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      اكسر;
    }
    حالة TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      اكسر;
    }
    حالة TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      اكسر;
    }
    حالة TK_NIL: {
      init_exp(v, VNIL, 0);
      اكسر;
    }
    حالة TK_TRUE: {
      init_exp(v, VTRUE, 0);
      اكسر;
    }
    حالة TK_FALSE: {
      init_exp(v, VFALSE, 0);
      اكسر;
    }
    حالة TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      اكسر;
    }
    حالة '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      החזר;
    }
    حالة TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      החזר;
    }
    افتراضي: {
      suffixedexp(ls, v);
      החזר;
    }
  }
  luaX_next(ls);
}


ساكن UnOpr getunopr (صحيح op) {
  بدل (op) {
    حالة TK_NOT: החזר OPR_NOT;
    حالة '-': החזר OPR_MINUS;
    حالة '~': החזר OPR_BNOT;
    حالة '#': החזר OPR_LEN;
    افتراضي: החזר OPR_NOUNOPR;
  }
}


ساكن BinOpr getbinopr (صحيح op) {
  بدل (op) {
    حالة '+': החזר OPR_ADD;
    حالة '-': החזר OPR_SUB;
    حالة '*': החזר OPR_MUL;
    حالة '%': החזר OPR_MOD;
    حالة '^': החזר OPR_POW;
    حالة '/': החזר OPR_DIV;
    حالة TK_IDIV: החזר OPR_IDIV;
    حالة '&': החזר OPR_BAND;
    حالة '|': החזר OPR_BOR;
    حالة '~': החזר OPR_BXOR;
    حالة TK_SHL: החזר OPR_SHL;
    حالة TK_SHR: החזר OPR_SHR;
    حالة TK_CONCAT: החזר OPR_CONCAT;
    حالة TK_NE: החזר OPR_NE;
    حالة TK_EQ: החזר OPR_EQ;
    حالة '<': החזר OPR_LT;
    حالة TK_LE: החזר OPR_LE;
    حالة '>': החזר OPR_GT;
    حالة TK_GE: החזר OPR_GE;
    حالة TK_AND: החזר OPR_AND;
    حالة TK_OR: החזר OPR_OR;
    افتراضي: החזר OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ساكن ثابت بنية {
  lu_byte left;  /* left priority for each binary operator */
  lu_byte right; /* right priority */
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
ساكن BinOpr subexpr (LexState *ls, expdesc *v, صحيح limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  אם (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    صحيح line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, line);
  }
  אחרת simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  כלעוד (op != OPR_NOBINOPR && priority[op].left > limit) {
    expdesc v2;
    BinOpr nextop;
    صحيح line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].right);
    luaK_posfix(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  leavelevel(ls);
  החזר op;  /* return first untreated operator */
}


ساكن فراغ expr (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


ساكن فراغ block (LexState *ls) {
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
بنية LHS_assign {
  بنية LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ساكن فراغ check_conflict (LexState *ls, بنية LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  صحيح conflict = 0;
  עבור (; lh; lh = lh->prev) {  /* check all previous assignments */
    אם (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      אם (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        אם (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      אחרת {  /* table is a register */
        אם (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        אם (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  אם (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    אם (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    אחרת
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
ساكن فراغ storevartop (FuncState *fs, expdesc *var) {
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
ساكن فراغ restassign (LexState *ls, بنية LHS_assign *lh, صحيح nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  אם (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    بنية LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    אם (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  אחרת {  /* restassign -> '=' explist */
    صحيح nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    אם (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    אחרת {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      החזר;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ساكن صحيح cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  expr(ls, &v);  /* read condition */
  אם (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  החזר v.f;
}


ساكن فراغ gotostat (LexState *ls, صحيح line) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, line);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ساكن فراغ breakstat (LexState *ls, صحيح line) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  עבור (bl = ls->fs->bl; bl != عدم; bl = bl->previous) {
    אם (bl->isloop)  /* found one? */
      اذهب ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, line);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ساكن فراغ checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  אם (l_unlikely(lb != عدم))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->line);  /* error */
}


ساكن فراغ labelstat (LexState *ls, TString *name, صحيح line) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  כלעוד (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, line, block_follow(ls, 0));
}


ساكن فراغ whilestat (LexState *ls, صحيح line) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  صحيح whileinit;
  صحيح condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, line);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ساكن فراغ repeatstat (LexState *ls, صحيح line) {
  /* repeatstat -> REPEAT block UNTIL cond */
  صحيح condexit;
  FuncState *fs = ls->fs;
  صحيح repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, line);
  condexit = cond(ls);  /* read condition (inside scope block) */
  אם (bl2.upval) {  /* upvalues? */
    صحيح exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
ساكن فراغ exp1 (LexState *ls) {
  expdesc e;
  expr(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
ساكن فراغ fixforjump (FuncState *fs, صحيح pc, صحيح dest, صحيح back) {
  Instruction *jmp = &fs->f->code[pc];
  صحيح offset = dest - (pc + 1);
  אם (back)
    offset = -offset;
  אם (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
ساكن فراغ forbody (LexState *ls, صحيح base, صحيح line, صحيح nvars, صحيح isgen) {
  /* forbody -> DO block */
  ساكن ثابت OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ساكن ثابت OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  صحيح prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  אם (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, line);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, line);
}


ساكن فراغ fornum (LexState *ls, TString *varname, صحيح line) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  صحيح base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  אם (testnext(ls, ','))
    exp1(ls);  /* optional step */
  אחרת {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, line, 1, 0);
}


ساكن فراغ forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  صحيح nvars = 4;  /* function, state, closing, control */
  صحيح line;
  صحيح base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  כלעוד (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  line = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, line, nvars - 3, 1);
}


ساكن فراغ forstat (LexState *ls, صحيح line) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  بدل (ls->t.token) {
    حالة '=': fornum(ls, varname, line); اكسر;
    حالة ',': حالة TK_IN: forlist(ls, varname); اكسر;
    افتراضي: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, line);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ساكن فراغ test_then_block (LexState *ls, صحيح *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  صحيح condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  אם (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ساكن فراغ ifstat (LexState *ls, صحيح line) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  صحيح escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  כלעוד (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  אם (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, line);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ساكن فراغ localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  صحيح fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ساكن lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  אם (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ثابت حرف *attr = getstr(ts);
    checknext(ls, '>');
    אם (strcmp(attr, "const") == 0)
      החזר RDKCONST;  /* read-only variable */
    אחרת אם (strcmp(attr, "close") == 0)
      החזר RDKTOCLOSE;  /* to-be-closed variable */
    אחרת
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  החזר df;  /* return default value */
}


ساكن فراغ checktoclose (FuncState *fs, صحيح level) {
  אם (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ساكن فراغ localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  صحيح toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  صحيح vidx;  /* index of last variable */
  صحيح nvars = 0;
  صحيح nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  עשה {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    אם (kind == RDKTOCLOSE) {  /* to-be-closed? */
      אם (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } כלעוד (testnext(ls, ','));
  אם (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  אחרת {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  אם (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  אחרת {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ساكن lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  بدل (kind) {
    حالة RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      החזר kind;  /* to avoid warnings */
    حالة RDKCONST:
      החזר GDKCONST;  /* adjust kind for global variable */
    افتراضي:
      החזר kind;
  }
}


ساكن فراغ checkglobal (LexState *ls, TString *varname, صحيح line) {
  FuncState *fs = ls->fs;
  expdesc var;
  صحيح k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, line);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
ساكن فراغ initglobal (LexState *ls, صحيح nvars, صحيح firstidx, صحيح n,
                        صحيح line) {
  אם (n == nvars) {  /* traversed all variables? */
    expdesc e;
    صحيح nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  אחרת {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, line);
    leavelevel(ls);
    checkglobal(ls, varname, line);
    storevartop(fs, &var);
  }
}


ساكن فراغ globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  صحيح nvars = 0;
  صحيح lastidx;  /* index of last registered variable */
  עשה {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } כלעוד (testnext(ls, ','));
  אם (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ساكن فراغ globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  אם (!testnext(ls, '*'))
    globalnames(ls, defkind);
  אחרת {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, عدم, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ساكن فراغ globalfunc (LexState *ls, صحيح line) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, line);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, line);  /* definition "happens" in the first line */
}


ساكن فراغ globalstatfunc (LexState *ls, صحيح line) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  אם (testnext(ls, TK_FUNCTION))
    globalfunc(ls, line);
  אחרת
    globalstat(ls);
}


ساكن صحيح funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  صحيح ismethod = 0;
  singlevar(ls, v);
  כלעוד (ls->t.token == '.')
    fieldsel(ls, v);
  אם (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  החזר ismethod;
}


ساكن فراغ funcstat (LexState *ls, صحيح line) {
  /* funcstat -> FUNCTION funcname body */
  صحيح ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, line);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, line);  /* definition "happens" in the first line */
}


ساكن فراغ exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  بنية LHS_assign v;
  suffixedexp(ls, &v.v);
  אם (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = عدم;
    restassign(ls, &v, 1);
  }
  אחרת {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


ساكن فراغ retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  صحيح nret;  /* number of values being returned */
  صحيح first = luaY_nvarstack(fs);  /* first slot to be returned */
  אם (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  אחרת {
    nret = explist(ls, &e);  /* optional return values */
    אם (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      אם (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    אחרת {
      אם (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      אחרת {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ساكن فراغ statement (LexState *ls) {
  صحيح line = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  بدل (ls->t.token) {
    حالة ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      اكسر;
    }
    حالة TK_IF: {  /* stat -> ifstat */
      ifstat(ls, line);
      اكسر;
    }
    حالة TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, line);
      اكسر;
    }
    حالة TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, line);
      اكسر;
    }
    حالة TK_FOR: {  /* stat -> forstat */
      forstat(ls, line);
      اكسر;
    }
    حالة TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, line);
      اكسر;
    }
    حالة TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, line);
      اكسر;
    }
    حالة TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      אם (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      אחרת
        localstat(ls);
      اكسر;
    }
    حالة TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, line);
      اكسر;
    }
    حالة TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), line);
      اكسر;
    }
    حالة TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      اكسر;
    }
    حالة TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, line);
      اكسر;
    }
    حالة TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, line);
      اكسر;
    }
#אם LUA_COMPAT_GLOBAL
    حالة TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      אם (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        صحيح lk = luaX_lookahead(ls);
        אם (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, line);
          اكسر;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    افتراضي: {  /* stat -> func | assignment */
      exprstat(ls);
      اكسر;
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
ساكن فراغ mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, ثابت حرف *name, صحيح firstchar) {
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
  החזר cl;
}

