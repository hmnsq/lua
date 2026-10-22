/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lcode.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "llex.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lparser.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"



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
تعریفنوع ساختار BlockCnt {
  ساختار BlockCnt *previous;  /* chain */
  صحیح firstlabel;  /* index of first label in this block */
  صحیح firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
ایستا تهی statement (LexState *ls);
ایستا تهی عبارت (LexState *ls, expdesc *v);


ایستا l_noret error_expected (LexState *ls, صحیح token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ایستا l_noret errorlimit (FuncState *fs, صحیح limit, ثابت نویسه *what) {
  lua_State *L = fs->ls->L;
  ثابت نویسه *پیام;
  صحیح سطر = fs->f->linedefined;
  ثابت نویسه *where = (سطر == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", سطر);
  پیام = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, پیام);
}


تهی luaY_checklimit (FuncState *fs, صحیح v, صحیح l, ثابت نویسه *what) {
  اگر (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ایستا صحیح testnext (LexState *ls, صحیح c) {
  اگر (ls->t.token == c) {
    luaX_next(ls);
    بازگشت 1;
  }
  وگرنه بازگشت 0;
}


/*
** Check that next token is 'c'.
*/
ایستا تهی check (LexState *ls, صحیح c) {
  اگر (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ایستا تهی checknext (LexState *ls, صحیح c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,پیام)	{ اگر (!(c)) luaX_syntaxerror(ls, پیام); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ایستا تهی check_match (LexState *ls, صحیح what, صحیح who, صحیح where) {
  اگر (l_unlikely(!testnext(ls, what))) {
    اگر (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    وگرنه {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


ایستا TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  بازگشت ts;
}


ایستا تهی init_exp (expdesc *e, expkind k, صحیح i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


ایستا تهی codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


ایستا تهی codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
ایستا short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  صحیح oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  وقتی (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = پوچ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  بازگشت fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ایستا صحیح new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  بازگشت dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ایستا صحیح new_localvar (LexState *ls, TString *name) {
  بازگشت new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (اندازه(v)/اندازه(نویسه)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ایستا Vardesc *getlocalvardesc (FuncState *fs, صحیح vidx) {
  بازگشت &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ایستا lu_byte reglevel (FuncState *fs, صحیح nvar) {
  وقتی (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    اگر (varinreg(vd))  /* is in a register? */
      بازگشت cast_byte(vd->vd.ridx + 1);
  }
  بازگشت 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  بازگشت reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ایستا LocVar *localdebuginfo (FuncState *fs, صحیح vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  اگر (!varinreg(vd))
    بازگشت پوچ;  /* no debug info. for constants */
  وگرنه {
    صحیح idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    بازگشت &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ایستا تهی init_var (FuncState *fs, expdesc *e, صحیح vidx) {
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
ایستا تهی check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = پوچ;  /* to be set if variable is const */
  انتخاب (e->k) {
    حالت VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      بشکن;
    }
    حالت VLOCAL: حالت VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      اگر (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      بشکن;
    }
    حالت VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      اگر (up->kind != VDKREG)
        varname = up->name;
      بشکن;
    }
    حالت VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    حالت VINDEXUP: حالت VINDEXSTR: حالت VINDEXED: {  /* global variable */
      اگر (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      بشکن;
    }
    پیشفرض:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      بازگشت;  /* integer index cannot be read-only */
  }
  اگر (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ایستا تهی adjustlocalvars (LexState *ls, صحیح nvars) {
  FuncState *fs = ls->fs;
  صحیح reglevel = luaY_nvarstack(fs);
  صحیح i;
  برای (i = 0; i < nvars; i++) {
    صحیح vidx = fs->nactvar++;
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
ایستا تهی removevars (FuncState *fs, صحیح tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  وقتی (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    اگر (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ایستا صحیح searchupvalue (FuncState *fs, TString *name) {
  صحیح i;
  Upvaldesc *up = fs->f->upvalues;
  برای (i = 0; i < fs->nups; i++) {
    اگر (eqstr(up[i].name, name)) بازگشت i;
  }
  بازگشت -1;  /* not found */
}


ایستا Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  صحیح oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  وقتی (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = پوچ;
  بازگشت &f->upvalues[fs->nups++];
}


ایستا صحیح newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  اگر (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  وگرنه {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  بازگشت fs->nups - 1;
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
ایستا صحیح searchvar (FuncState *fs, TString *n, expdesc *var) {
  صحیح i;
  برای (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    اگر (varglobal(vd)) {  /* global declaration? */
      اگر (vd->vd.name == پوچ) {  /* collective declaration? */
        اگر (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      وگرنه {  /* global name */
        اگر (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          بازگشت VGLOBAL;
        }
        وگرنه اگر (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    وگرنه اگر (eqstr(n, vd->vd.name)) {  /* found? */
      اگر (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      وگرنه {  /* local variable */
        init_var(fs, var, i);
        اگر (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      بازگشت cast_int(var->k);
    }
  }
  بازگشت -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ایستا تهی markupval (FuncState *fs, صحیح level) {
  BlockCnt *bl = fs->bl;
  وقتی (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ایستا تهی marktobeclosed (FuncState *fs) {
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
ایستا تهی singlevaraux (FuncState *fs, TString *n, expdesc *var, صحیح base) {
  صحیح v = searchvar(fs, n, var);  /* look up variables at current level */
  اگر (v >= 0) {  /* found? */
    اگر (!base) {
      اگر (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      اگر (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  وگرنه {  /* not found at current level; try upvalues */
    صحیح idx = searchupvalue(fs, n);  /* try existing upvalues */
    اگر (idx < 0) {  /* not found? */
      اگر (fs->prev != پوچ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      اگر (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      وگرنه  /* it is a global or a constant */
        بازگشت;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ایستا تهی buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc کلید;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  اگر (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&کلید, varname);  /* key is variable name */
  luaK_indexed(fs, var, &کلید);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ایستا تهی buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  اگر (var->k == VGLOBAL) {  /* global name? */
    صحیح info = var->u.info;
    /* global by default in the scope of a global declaration? */
    اگر (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    اگر (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    وگرنه  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


ایستا تهی singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ایستا تهی adjust_assign (LexState *ls, صحیح nvars, صحیح nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  صحیح needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  اگر (hasmultret(e->k)) {  /* last expression has multiple returns? */
    صحیح extra = needed + 1;  /* discount last expression itself */
    اگر (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  وگرنه {
    اگر (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    اگر (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  اگر (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  وگرنه  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ایستا l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ثابت نویسه *varname = (tsname != پوچ) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->سطر, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
ایستا تهی closegoto (LexState *ls, صحیح g, Labeldesc *label, صحیح bup) {
  صحیح i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  اگر (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  اگر (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  برای (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ایستا Labeldesc *findlabel (LexState *ls, TString *name, صحیح ilb) {
  Dyndata *dyd = ls->dyd;
  برای (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    اگر (eqstr(lb->name, name))  /* correct label? */
      بازگشت lb;
  }
  بازگشت پوچ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ایستا صحیح newlabelentry (LexState *ls, Labellist *l, TString *name,
                          صحیح سطر, صحیح pc) {
  صحیح n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].سطر = سطر;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  بازگشت n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ایستا صحیح newgotoentry (LexState *ls, TString *name, صحیح سطر) {
  FuncState *fs = ls->fs;
  صحیح pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  بازگشت newlabelentry(ls, &ls->dyd->gt, name, سطر, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ایستا تهی createlabel (LexState *ls, TString *name, صحیح سطر, صحیح last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  صحیح l = newlabelentry(ls, ll, name, سطر, luaK_getlabel(fs));
  اگر (last) {  /* label is last no-op statement in the block? */
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
ایستا تهی solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  صحیح outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  صحیح igt = bl->firstgoto;  /* first goto in the finishing block */
  وقتی (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    اگر (lb != پوچ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    وگرنه {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      اگر (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ایستا تهی enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != پوچ && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
ایستا l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->سطر);
}


ایستا تهی leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  اگر (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  اگر (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  اگر (bl->previous == پوچ) {  /* was it the last block? */
    اگر (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ایستا Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  اگر (fs->np >= f->sizep) {
    صحیح oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    وقتی (oldsize < f->sizep)
      f->p[oldsize++] = پوچ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  بازگشت clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ایستا تهی codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ایستا تهی open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = پوچ;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ایستا تهی close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == پوچ);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_سطر_Info);
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
ایستا صحیح block_follow (LexState *ls, صحیح withuntil) {
  انتخاب (ls->t.token) {
    حالت TK_ELSE: حالت TK_ELSEIF:
    حالت TK_END: حالت TK_EOS:
      بازگشت 1;
    حالت TK_UNTIL: بازگشت withuntil;
    پیشفرض: بازگشت 0;
  }
}


ایستا تهی statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  وقتی (!block_follow(ls, 1)) {
    اگر (ls->t.token == TK_RETURN) {
      statement(ls);
      بازگشت;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ایستا تهی fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc کلید;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &کلید);
  luaK_indexed(fs, v, &کلید);
}


ایستا تهی yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  عبارت(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

تعریفنوع ساختار ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  صحیح nh;  /* total number of 'record' elements */
  صحیح na;  /* number of array elements already stored */
  صحیح tostore;  /* number of array elements pending to be stored */
  صحیح maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#اگر MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


ایستا تهی recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, کلید, val;
  اگر (ls->t.token == TK_NAME)
    codename(ls, &کلید);
  وگرنه  /* ls->t.token == '[' */
    yindex(ls, &کلید);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &کلید);
  عبارت(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ایستا تهی closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  اگر (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ایستا تهی lastlistfield (FuncState *fs, ConsControl *cc) {
  اگر (cc->tostore == 0) بازگشت;
  اگر (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  وگرنه {
    اگر (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


ایستا تهی listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  عبارت(ls, &cc->v);
  cc->tostore++;
}


ایستا تهی field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  انتخاب(ls->t.token) {
    حالت TK_NAME: {  /* may be 'listfield' or 'recfield' */
      اگر (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      وگرنه
        recfield(ls, cc);
      بشکن;
    }
    حالت '[': {
      recfield(ls, cc);
      بشکن;
    }
    پیشفرض: {
      listfield(ls, cc);
      بشکن;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ایستا صحیح maxtostore (FuncState *fs) {
  صحیح numfreeregs = MAX_FSTACK - fs->freereg;
  اگر (numfreeregs >= 160)  /* "lots" of registers? */
    بازگشت numfreeregs / 5;  /* use up to 1/5 of them */
  وگرنه اگر (numfreeregs >= 80)  /* still "enough" registers? */
    بازگشت 10;  /* one 'SETLIST' instruction for each 10 values */
  وگرنه  /* save registers for potential more nesting */
    بازگشت 1;
}


ایستا تهی constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  صحیح سطر = ls->linenumber;
  صحیح pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  انجامبده {
    اگر (ls->t.token == /*{*/ '}') بشکن;
    اگر (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } وقتی (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, سطر);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ایستا تهی setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ایستا تهی parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  صحیح nparams = 0;
  صحیح varargk = 0;
  اگر (ls->t.token != ')') {  /* is 'parlist' not empty? */
    انجامبده {
      انتخاب (ls->t.token) {
        حالت TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          بشکن;
        }
        حالت TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          اگر (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          وگرنه
            new_localvarliteral(ls, "(vararg table)");
          بشکن;
        }
        پیشفرض: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } وقتی (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  اگر (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ایستا تهی body (LexState *ls, expdesc *e, صحیح ismethod, صحیح سطر) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = سطر;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  اگر (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, سطر);
  codeclosure(ls, e);
  close_func(ls);
}


ایستا صحیح explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  صحیح n = 1;  /* at least one expression */
  عبارت(ls, v);
  وقتی (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    عبارت(ls, v);
    n++;
  }
  بازگشت n;
}


ایستا تهی funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  صحیح base, nparams;
  صحیح سطر = ls->linenumber;
  انتخاب (ls->t.token) {
    حالت '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      اگر (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      وگرنه {
        explist(ls, &args);
        اگر (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', سطر);
      بشکن;
    }
    حالت '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      بشکن;
    }
    حالت TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      بشکن;
    }
    پیشفرض: {
      luaX_syntaxerror(ls, "function arguments expected");
      بازگشت;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  اگر (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  وگرنه {
    اگر (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, سطر);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ایستا تهی primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  انتخاب (ls->t.token) {
    حالت '(': {
      صحیح سطر = ls->linenumber;
      luaX_next(ls);
      عبارت(ls, v);
      check_match(ls, ')', '(', سطر);
      luaK_dischargevars(ls->fs, v);
      بازگشت;
    }
    حالت TK_NAME: {
      singlevar(ls, v);
      بازگشت;
    }
    پیشفرض: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ایستا تهی suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  برای (;;) {
    انتخاب (ls->t.token) {
      حالت '.': {  /* fieldsel */
        fieldsel(ls, v);
        بشکن;
      }
      حالت '[': {  /* '[' exp ']' */
        expdesc کلید;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &کلید);
        luaK_indexed(fs, v, &کلید);
        بشکن;
      }
      حالت ':': {  /* ':' NAME funcargs */
        expdesc کلید;
        luaX_next(ls);
        codename(ls, &کلید);
        luaK_self(fs, v, &کلید);
        funcargs(ls, v);
        بشکن;
      }
      حالت '(': حالت TK_STRING: حالت '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        بشکن;
      }
      پیشفرض: بازگشت;
    }
  }
}


ایستا تهی simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  انتخاب (ls->t.token) {
    حالت TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      بشکن;
    }
    حالت TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      بشکن;
    }
    حالت TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      بشکن;
    }
    حالت TK_NIL: {
      init_exp(v, VNIL, 0);
      بشکن;
    }
    حالت TK_TRUE: {
      init_exp(v, VTRUE, 0);
      بشکن;
    }
    حالت TK_FALSE: {
      init_exp(v, VFALSE, 0);
      بشکن;
    }
    حالت TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      بشکن;
    }
    حالت '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      بازگشت;
    }
    حالت TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      بازگشت;
    }
    پیشفرض: {
      suffixedexp(ls, v);
      بازگشت;
    }
  }
  luaX_next(ls);
}


ایستا UnOpr getunopr (صحیح op) {
  انتخاب (op) {
    حالت TK_NOT: بازگشت OPR_NOT;
    حالت '-': بازگشت OPR_MINUS;
    حالت '~': بازگشت OPR_BNOT;
    حالت '#': بازگشت OPR_LEN;
    پیشفرض: بازگشت OPR_NOUNOPR;
  }
}


ایستا BinOpr getbinopr (صحیح op) {
  انتخاب (op) {
    حالت '+': بازگشت OPR_ADD;
    حالت '-': بازگشت OPR_SUB;
    حالت '*': بازگشت OPR_MUL;
    حالت '%': بازگشت OPR_MOD;
    حالت '^': بازگشت OPR_POW;
    حالت '/': بازگشت OPR_DIV;
    حالت TK_IDIV: بازگشت OPR_IDIV;
    حالت '&': بازگشت OPR_BAND;
    حالت '|': بازگشت OPR_BOR;
    حالت '~': بازگشت OPR_BXOR;
    حالت TK_SHL: بازگشت OPR_SHL;
    حالت TK_SHR: بازگشت OPR_SHR;
    حالت TK_CONCAT: بازگشت OPR_CONCAT;
    حالت TK_NE: بازگشت OPR_NE;
    حالت TK_EQ: بازگشت OPR_EQ;
    حالت '<': بازگشت OPR_LT;
    حالت TK_LE: بازگشت OPR_LE;
    حالت '>': بازگشت OPR_GT;
    حالت TK_GE: بازگشت OPR_GE;
    حالت TK_AND: بازگشت OPR_AND;
    حالت TK_OR: بازگشت OPR_OR;
    پیشفرض: بازگشت OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ایستا ثابت ساختار {
  lu_byte چپ;  /* left priority for each binary operator */
  lu_byte راست; /* right priority */
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
ایستا BinOpr subexpr (LexState *ls, expdesc *v, صحیح limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  اگر (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    صحیح سطر = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, سطر);
  }
  وگرنه simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  وقتی (op != OPR_NOBINOPR && priority[op].چپ > limit) {
    expdesc v2;
    BinOpr nextop;
    صحیح سطر = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].راست);
    luaK_posfix(ls->fs, op, v, &v2, سطر);
    op = nextop;
  }
  leavelevel(ls);
  بازگشت op;  /* return first untreated operator */
}


ایستا تهی عبارت (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


ایستا تهی block (LexState *ls) {
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
ساختار LHS_assign {
  ساختار LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ایستا تهی check_conflict (LexState *ls, ساختار LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  صحیح conflict = 0;
  برای (; lh; lh = lh->prev) {  /* check all previous assignments */
    اگر (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      اگر (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        اگر (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      وگرنه {  /* table is a register */
        اگر (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        اگر (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  اگر (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    اگر (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    وگرنه
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
ایستا تهی storevartop (FuncState *fs, expdesc *var) {
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
ایستا تهی restassign (LexState *ls, ساختار LHS_assign *lh, صحیح nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  اگر (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ساختار LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    اگر (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  وگرنه {  /* restassign -> '=' explist */
    صحیح nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    اگر (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    وگرنه {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      بازگشت;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ایستا صحیح cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  عبارت(ls, &v);  /* read condition */
  اگر (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  بازگشت v.f;
}


ایستا تهی gotostat (LexState *ls, صحیح سطر) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, سطر);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ایستا تهی breakstat (LexState *ls, صحیح سطر) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  برای (bl = ls->fs->bl; bl != پوچ; bl = bl->previous) {
    اگر (bl->isloop)  /* found one? */
      برو ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, سطر);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ایستا تهی checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  اگر (l_unlikely(lb != پوچ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->سطر);  /* error */
}


ایستا تهی labelstat (LexState *ls, TString *name, صحیح سطر) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  وقتی (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, سطر, block_follow(ls, 0));
}


ایستا تهی whilestat (LexState *ls, صحیح سطر) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  صحیح whileinit;
  صحیح condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, سطر);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ایستا تهی repeatstat (LexState *ls, صحیح سطر) {
  /* repeatstat -> REPEAT block UNTIL cond */
  صحیح condexit;
  FuncState *fs = ls->fs;
  صحیح repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, سطر);
  condexit = cond(ls);  /* read condition (inside scope block) */
  اگر (bl2.upval) {  /* upvalues? */
    صحیح exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
ایستا تهی exp1 (LexState *ls) {
  expdesc e;
  عبارت(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
ایستا تهی fixforjump (FuncState *fs, صحیح pc, صحیح dest, صحیح back) {
  Instruction *jmp = &fs->f->code[pc];
  صحیح offset = dest - (pc + 1);
  اگر (back)
    offset = -offset;
  اگر (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
ایستا تهی forbody (LexState *ls, صحیح base, صحیح سطر, صحیح nvars, صحیح isgen) {
  /* forbody -> DO block */
  ایستا ثابت OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ایستا ثابت OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  صحیح prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  اگر (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, سطر);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, سطر);
}


ایستا تهی fornum (LexState *ls, TString *varname, صحیح سطر) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  صحیح base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  اگر (testnext(ls, ','))
    exp1(ls);  /* optional step */
  وگرنه {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, سطر, 1, 0);
}


ایستا تهی forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  صحیح nvars = 4;  /* function, state, closing, control */
  صحیح سطر;
  صحیح base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  وقتی (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  سطر = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, سطر, nvars - 3, 1);
}


ایستا تهی forstat (LexState *ls, صحیح سطر) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  انتخاب (ls->t.token) {
    حالت '=': fornum(ls, varname, سطر); بشکن;
    حالت ',': حالت TK_IN: forlist(ls, varname); بشکن;
    پیشفرض: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, سطر);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ایستا تهی test_then_block (LexState *ls, صحیح *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  صحیح condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  اگر (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ایستا تهی ifstat (LexState *ls, صحیح سطر) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  صحیح escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  وقتی (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  اگر (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, سطر);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ایستا تهی localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  صحیح fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ایستا lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  اگر (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ثابت نویسه *attr = getstr(ts);
    checknext(ls, '>');
    اگر (strcmp(attr, "const") == 0)
      بازگشت RDKCONST;  /* read-only variable */
    وگرنه اگر (strcmp(attr, "close") == 0)
      بازگشت RDKTOCLOSE;  /* to-be-closed variable */
    وگرنه
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  بازگشت df;  /* return default value */
}


ایستا تهی checktoclose (FuncState *fs, صحیح level) {
  اگر (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ایستا تهی localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  صحیح toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  صحیح vidx;  /* index of last variable */
  صحیح nvars = 0;
  صحیح nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  انجامبده {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    اگر (kind == RDKTOCLOSE) {  /* to-be-closed? */
      اگر (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } وقتی (testnext(ls, ','));
  اگر (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  وگرنه {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  اگر (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  وگرنه {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ایستا lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  انتخاب (kind) {
    حالت RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      بازگشت kind;  /* to avoid warnings */
    حالت RDKCONST:
      بازگشت GDKCONST;  /* adjust kind for global variable */
    پیشفرض:
      بازگشت kind;
  }
}


ایستا تهی checkglobal (LexState *ls, TString *varname, صحیح سطر) {
  FuncState *fs = ls->fs;
  expdesc var;
  صحیح k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, سطر);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
ایستا تهی initglobal (LexState *ls, صحیح nvars, صحیح firstidx, صحیح n,
                        صحیح سطر) {
  اگر (n == nvars) {  /* traversed all variables? */
    expdesc e;
    صحیح nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  وگرنه {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, سطر);
    leavelevel(ls);
    checkglobal(ls, varname, سطر);
    storevartop(fs, &var);
  }
}


ایستا تهی globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  صحیح nvars = 0;
  صحیح lastidx;  /* index of last registered variable */
  انجامبده {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } وقتی (testnext(ls, ','));
  اگر (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ایستا تهی globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  اگر (!testnext(ls, '*'))
    globalnames(ls, defkind);
  وگرنه {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, پوچ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ایستا تهی globalfunc (LexState *ls, صحیح سطر) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, سطر);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, سطر);  /* definition "happens" in the first line */
}


ایستا تهی globalstatfunc (LexState *ls, صحیح سطر) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  اگر (testnext(ls, TK_FUNCTION))
    globalfunc(ls, سطر);
  وگرنه
    globalstat(ls);
}


ایستا صحیح funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  صحیح ismethod = 0;
  singlevar(ls, v);
  وقتی (ls->t.token == '.')
    fieldsel(ls, v);
  اگر (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  بازگشت ismethod;
}


ایستا تهی funcstat (LexState *ls, صحیح سطر) {
  /* funcstat -> FUNCTION funcname body */
  صحیح ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, سطر);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, سطر);  /* definition "happens" in the first line */
}


ایستا تهی exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ساختار LHS_assign v;
  suffixedexp(ls, &v.v);
  اگر (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = پوچ;
    restassign(ls, &v, 1);
  }
  وگرنه {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


ایستا تهی retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  صحیح nret;  /* number of values being returned */
  صحیح first = luaY_nvarstack(fs);  /* first slot to be returned */
  اگر (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  وگرنه {
    nret = explist(ls, &e);  /* optional return values */
    اگر (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      اگر (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    وگرنه {
      اگر (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      وگرنه {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ایستا تهی statement (LexState *ls) {
  صحیح سطر = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  انتخاب (ls->t.token) {
    حالت ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      بشکن;
    }
    حالت TK_IF: {  /* stat -> ifstat */
      ifstat(ls, سطر);
      بشکن;
    }
    حالت TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, سطر);
      بشکن;
    }
    حالت TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, سطر);
      بشکن;
    }
    حالت TK_FOR: {  /* stat -> forstat */
      forstat(ls, سطر);
      بشکن;
    }
    حالت TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, سطر);
      بشکن;
    }
    حالت TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, سطر);
      بشکن;
    }
    حالت TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      اگر (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      وگرنه
        localstat(ls);
      بشکن;
    }
    حالت TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, سطر);
      بشکن;
    }
    حالت TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), سطر);
      بشکن;
    }
    حالت TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      بشکن;
    }
    حالت TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, سطر);
      بشکن;
    }
    حالت TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, سطر);
      بشکن;
    }
#اگر LUA_COMPAT_GLOBAL
    حالت TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      اگر (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        صحیح lk = luaX_lookahead(ls);
        اگر (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, سطر);
          بشکن;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    پیشفرض: {  /* stat -> func | assignment */
      exprstat(ls);
      بشکن;
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
ایستا تهی mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, ثابت نویسه *name, صحیح firstchar) {
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
  بازگشت cl;
}

