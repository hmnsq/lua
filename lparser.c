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
प्रकारनिर्देश संरचना BlockCnt {
  संरचना BlockCnt *previous;  /* chain */
  पूर्णाङ्क firstlabel;  /* index of first label in this block */
  पूर्णाङ्क firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
स्थिर शून्य statement (LexState *ls);
स्थिर शून्य व्यञ्जकम् (LexState *ls, expdesc *v);


स्थिर l_noret error_expected (LexState *ls, पूर्णाङ्क token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


स्थिर l_noret errorlimit (FuncState *fs, पूर्णाङ्क limit, ध्रुव वर्ण *what) {
  lua_State *L = fs->ls->L;
  ध्रुव वर्ण *सन्देशः;
  पूर्णाङ्क पङ्क्तिः = fs->f->linedefined;
  ध्रुव वर्ण *where = (पङ्क्तिः == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", पङ्क्तिः);
  सन्देशः = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, सन्देशः);
}


शून्य luaY_checklimit (FuncState *fs, पूर्णाङ्क v, पूर्णाङ्क l, ध्रुव वर्ण *what) {
  सचे (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
स्थिर पूर्णाङ्क testnext (LexState *ls, पूर्णाङ्क c) {
  सचे (ls->t.token == c) {
    luaX_next(ls);
    पटिनिवत्त 1;
  }
  अञ्ञथा पटिनिवत्त 0;
}


/*
** Check that next token is 'c'.
*/
स्थिर शून्य check (LexState *ls, पूर्णाङ्क c) {
  सचे (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
स्थिर शून्य checknext (LexState *ls, पूर्णाङ्क c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,सन्देशः)	{ सचे (!(c)) luaX_syntaxerror(ls, सन्देशः); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
स्थिर शून्य check_match (LexState *ls, पूर्णाङ्क what, पूर्णाङ्क who, पूर्णाङ्क where) {
  सचे (l_unlikely(!testnext(ls, what))) {
    सचे (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    अञ्ञथा {
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
  पटिनिवत्त ts;
}


स्थिर शून्य init_exp (expdesc *e, expkind k, पूर्णाङ्क i) {
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
स्थिर short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  पूर्णाङ्क oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  याव (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = रिक्त;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  पटिनिवत्त fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
स्थिर पूर्णाङ्क new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  पटिनिवत्त dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
स्थिर पूर्णाङ्क new_localvar (LexState *ls, TString *name) {
  पटिनिवत्त new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (परिमाण(v)/परिमाण(वर्ण)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
स्थिर Vardesc *getlocalvardesc (FuncState *fs, पूर्णाङ्क vidx) {
  पटिनिवत्त &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
स्थिर lu_byte reglevel (FuncState *fs, पूर्णाङ्क nvar) {
  याव (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    सचे (varinreg(vd))  /* is in a register? */
      पटिनिवत्त cast_byte(vd->vd.ridx + 1);
  }
  पटिनिवत्त 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  पटिनिवत्त reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
स्थिर LocVar *localdebuginfo (FuncState *fs, पूर्णाङ्क vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  सचे (!varinreg(vd))
    पटिनिवत्त रिक्त;  /* no debug info. for constants */
  अञ्ञथा {
    पूर्णाङ्क idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    पटिनिवत्त &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
स्थिर शून्य init_var (FuncState *fs, expdesc *e, पूर्णाङ्क vidx) {
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
  विकल्प (e->k) {
    पक्ष VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      भङ्ग;
    }
    पक्ष VLOCAL: पक्ष VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      सचे (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      भङ्ग;
    }
    पक्ष VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      सचे (up->kind != VDKREG)
        varname = up->name;
      भङ्ग;
    }
    पक्ष VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    पक्ष VINDEXUP: पक्ष VINDEXSTR: पक्ष VINDEXED: {  /* global variable */
      सचे (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      भङ्ग;
    }
    स्वभाव:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      पटिनिवत्त;  /* integer index cannot be read-only */
  }
  सचे (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
स्थिर शून्य adjustlocalvars (LexState *ls, पूर्णाङ्क nvars) {
  FuncState *fs = ls->fs;
  पूर्णाङ्क reglevel = luaY_nvarstack(fs);
  पूर्णाङ्क i;
  अत्थाय (i = 0; i < nvars; i++) {
    पूर्णाङ्क vidx = fs->nactvar++;
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
स्थिर शून्य removevars (FuncState *fs, पूर्णाङ्क tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  याव (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    सचे (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
स्थिर पूर्णाङ्क searchupvalue (FuncState *fs, TString *name) {
  पूर्णाङ्क i;
  Upvaldesc *up = fs->f->upvalues;
  अत्थाय (i = 0; i < fs->nups; i++) {
    सचे (eqstr(up[i].name, name)) पटिनिवत्त i;
  }
  पटिनिवत्त -1;  /* not found */
}


स्थिर Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  पूर्णाङ्क oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  याव (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = रिक्त;
  पटिनिवत्त &f->upvalues[fs->nups++];
}


स्थिर पूर्णाङ्क newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  सचे (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  अञ्ञथा {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  पटिनिवत्त fs->nups - 1;
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
स्थिर पूर्णाङ्क searchvar (FuncState *fs, TString *n, expdesc *var) {
  पूर्णाङ्क i;
  अत्थाय (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    सचे (varglobal(vd)) {  /* global declaration? */
      सचे (vd->vd.name == रिक्त) {  /* collective declaration? */
        सचे (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      अञ्ञथा {  /* global name */
        सचे (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          पटिनिवत्त VGLOBAL;
        }
        अञ्ञथा सचे (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    अञ्ञथा सचे (eqstr(n, vd->vd.name)) {  /* found? */
      सचे (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      अञ्ञथा {  /* local variable */
        init_var(fs, var, i);
        सचे (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      पटिनिवत्त cast_int(var->k);
    }
  }
  पटिनिवत्त -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
स्थिर शून्य markupval (FuncState *fs, पूर्णाङ्क level) {
  BlockCnt *bl = fs->bl;
  याव (bl->nactvar > level)
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
स्थिर शून्य singlevaraux (FuncState *fs, TString *n, expdesc *var, पूर्णाङ्क base) {
  पूर्णाङ्क v = searchvar(fs, n, var);  /* look up variables at current level */
  सचे (v >= 0) {  /* found? */
    सचे (!base) {
      सचे (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      सचे (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  अञ्ञथा {  /* not found at current level; try upvalues */
    पूर्णाङ्क idx = searchupvalue(fs, n);  /* try existing upvalues */
    सचे (idx < 0) {  /* not found? */
      सचे (fs->prev != रिक्त)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      सचे (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      अञ्ञथा  /* it is a global or a constant */
        पटिनिवत्त;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


स्थिर शून्य buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc कुञ्चिका;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  सचे (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&कुञ्चिका, varname);  /* key is variable name */
  luaK_indexed(fs, var, &कुञ्चिका);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
स्थिर शून्य buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  सचे (var->k == VGLOBAL) {  /* global name? */
    पूर्णाङ्क info = var->u.info;
    /* global by default in the scope of a global declaration? */
    सचे (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    सचे (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    अञ्ञथा  /* anyway must be a global */
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
स्थिर शून्य adjust_assign (LexState *ls, पूर्णाङ्क nvars, पूर्णाङ्क nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  पूर्णाङ्क needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  सचे (hasmultret(e->k)) {  /* last expression has multiple returns? */
    पूर्णाङ्क extra = needed + 1;  /* discount last expression itself */
    सचे (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  अञ्ञथा {
    सचे (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    सचे (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  सचे (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  अञ्ञथा  /* adding 'needed' is actually a subtraction */
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
  ध्रुव वर्ण *varname = (tsname != रिक्त) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->पङ्क्तिः, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
स्थिर शून्य closegoto (LexState *ls, पूर्णाङ्क g, Labeldesc *label, पूर्णाङ्क bup) {
  पूर्णाङ्क i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  सचे (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  सचे (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  अत्थाय (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
स्थिर Labeldesc *findlabel (LexState *ls, TString *name, पूर्णाङ्क ilb) {
  Dyndata *dyd = ls->dyd;
  अत्थाय (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    सचे (eqstr(lb->name, name))  /* correct label? */
      पटिनिवत्त lb;
  }
  पटिनिवत्त रिक्त;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
स्थिर पूर्णाङ्क newlabelentry (LexState *ls, Labellist *l, TString *name,
                          पूर्णाङ्क पङ्क्तिः, पूर्णाङ्क pc) {
  पूर्णाङ्क n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].पङ्क्तिः = पङ्क्तिः;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  पटिनिवत्त n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
स्थिर पूर्णाङ्क newgotoentry (LexState *ls, TString *name, पूर्णाङ्क पङ्क्तिः) {
  FuncState *fs = ls->fs;
  पूर्णाङ्क pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  पटिनिवत्त newlabelentry(ls, &ls->dyd->gt, name, पङ्क्तिः, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
स्थिर शून्य createlabel (LexState *ls, TString *name, पूर्णाङ्क पङ्क्तिः, पूर्णाङ्क last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  पूर्णाङ्क l = newlabelentry(ls, ll, name, पङ्क्तिः, luaK_getlabel(fs));
  सचे (last) {  /* label is last no-op statement in the block? */
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
  पूर्णाङ्क outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  पूर्णाङ्क igt = bl->firstgoto;  /* first goto in the finishing block */
  याव (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    सचे (lb != रिक्त)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    अञ्ञथा {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      सचे (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
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
                    getstr(gt->name), gt->पङ्क्तिः);
}


स्थिर शून्य leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  सचे (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  सचे (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  सचे (bl->previous == रिक्त) {  /* was it the last block? */
    सचे (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
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
  सचे (fs->np >= f->sizep) {
    पूर्णाङ्क oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    याव (oldsize < f->sizep)
      f->p[oldsize++] = रिक्त;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  पटिनिवत्त clp;
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
                       fs->nabslineinfo, Abs_पङ्क्तिः_Info);
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
स्थिर पूर्णाङ्क block_follow (LexState *ls, पूर्णाङ्क withuntil) {
  विकल्प (ls->t.token) {
    पक्ष TK_ELSE: पक्ष TK_ELSEIF:
    पक्ष TK_END: पक्ष TK_EOS:
      पटिनिवत्त 1;
    पक्ष TK_UNTIL: पटिनिवत्त withuntil;
    स्वभाव: पटिनिवत्त 0;
  }
}


स्थिर शून्य statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  याव (!block_follow(ls, 1)) {
    सचे (ls->t.token == TK_RETURN) {
      statement(ls);
      पटिनिवत्त;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


स्थिर शून्य fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc कुञ्चिका;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &कुञ्चिका);
  luaK_indexed(fs, v, &कुञ्चिका);
}


स्थिर शून्य yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  व्यञ्जकम्(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

प्रकारनिर्देश संरचना ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  पूर्णाङ्क nh;  /* total number of 'record' elements */
  पूर्णाङ्क na;  /* number of array elements already stored */
  पूर्णाङ्क tostore;  /* number of array elements pending to be stored */
  पूर्णाङ्क maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#सचे MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


स्थिर शून्य recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, कुञ्चिका, val;
  सचे (ls->t.token == TK_NAME)
    codename(ls, &कुञ्चिका);
  अञ्ञथा  /* ls->t.token == '[' */
    yindex(ls, &कुञ्चिका);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &कुञ्चिका);
  व्यञ्जकम्(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


स्थिर शून्य closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  सचे (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


स्थिर शून्य lastlistfield (FuncState *fs, ConsControl *cc) {
  सचे (cc->tostore == 0) पटिनिवत्त;
  सचे (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  अञ्ञथा {
    सचे (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


स्थिर शून्य listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  व्यञ्जकम्(ls, &cc->v);
  cc->tostore++;
}


स्थिर शून्य field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  विकल्प(ls->t.token) {
    पक्ष TK_NAME: {  /* may be 'listfield' or 'recfield' */
      सचे (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      अञ्ञथा
        recfield(ls, cc);
      भङ्ग;
    }
    पक्ष '[': {
      recfield(ls, cc);
      भङ्ग;
    }
    स्वभाव: {
      listfield(ls, cc);
      भङ्ग;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
स्थिर पूर्णाङ्क maxtostore (FuncState *fs) {
  पूर्णाङ्क numfreeregs = MAX_FSTACK - fs->freereg;
  सचे (numfreeregs >= 160)  /* "lots" of registers? */
    पटिनिवत्त numfreeregs / 5;  /* use up to 1/5 of them */
  अञ्ञथा सचे (numfreeregs >= 80)  /* still "enough" registers? */
    पटिनिवत्त 10;  /* one 'SETLIST' instruction for each 10 values */
  अञ्ञथा  /* save registers for potential more nesting */
    पटिनिवत्त 1;
}


स्थिर शून्य constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  पूर्णाङ्क पङ्क्तिः = ls->linenumber;
  पूर्णाङ्क pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  करोहि {
    सचे (ls->t.token == /*{*/ '}') भङ्ग;
    सचे (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } याव (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, पङ्क्तिः);
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
  पूर्णाङ्क nparams = 0;
  पूर्णाङ्क varargk = 0;
  सचे (ls->t.token != ')') {  /* is 'parlist' not empty? */
    करोहि {
      विकल्प (ls->t.token) {
        पक्ष TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          भङ्ग;
        }
        पक्ष TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          सचे (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          अञ्ञथा
            new_localvarliteral(ls, "(vararg table)");
          भङ्ग;
        }
        स्वभाव: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } याव (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  सचे (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


स्थिर शून्य body (LexState *ls, expdesc *e, पूर्णाङ्क ismethod, पूर्णाङ्क पङ्क्तिः) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = पङ्क्तिः;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  सचे (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, पङ्क्तिः);
  codeclosure(ls, e);
  close_func(ls);
}


स्थिर पूर्णाङ्क explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  पूर्णाङ्क n = 1;  /* at least one expression */
  व्यञ्जकम्(ls, v);
  याव (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    व्यञ्जकम्(ls, v);
    n++;
  }
  पटिनिवत्त n;
}


स्थिर शून्य funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  पूर्णाङ्क base, nparams;
  पूर्णाङ्क पङ्क्तिः = ls->linenumber;
  विकल्प (ls->t.token) {
    पक्ष '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      सचे (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      अञ्ञथा {
        explist(ls, &args);
        सचे (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', पङ्क्तिः);
      भङ्ग;
    }
    पक्ष '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      भङ्ग;
    }
    पक्ष TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      भङ्ग;
    }
    स्वभाव: {
      luaX_syntaxerror(ls, "function arguments expected");
      पटिनिवत्त;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  सचे (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  अञ्ञथा {
    सचे (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, पङ्क्तिः);
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
  विकल्प (ls->t.token) {
    पक्ष '(': {
      पूर्णाङ्क पङ्क्तिः = ls->linenumber;
      luaX_next(ls);
      व्यञ्जकम्(ls, v);
      check_match(ls, ')', '(', पङ्क्तिः);
      luaK_dischargevars(ls->fs, v);
      पटिनिवत्त;
    }
    पक्ष TK_NAME: {
      singlevar(ls, v);
      पटिनिवत्त;
    }
    स्वभाव: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


स्थिर शून्य suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  अत्थाय (;;) {
    विकल्प (ls->t.token) {
      पक्ष '.': {  /* fieldsel */
        fieldsel(ls, v);
        भङ्ग;
      }
      पक्ष '[': {  /* '[' exp ']' */
        expdesc कुञ्चिका;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &कुञ्चिका);
        luaK_indexed(fs, v, &कुञ्चिका);
        भङ्ग;
      }
      पक्ष ':': {  /* ':' NAME funcargs */
        expdesc कुञ्चिका;
        luaX_next(ls);
        codename(ls, &कुञ्चिका);
        luaK_self(fs, v, &कुञ्चिका);
        funcargs(ls, v);
        भङ्ग;
      }
      पक्ष '(': पक्ष TK_STRING: पक्ष '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        भङ्ग;
      }
      स्वभाव: पटिनिवत्त;
    }
  }
}


स्थिर शून्य simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  विकल्प (ls->t.token) {
    पक्ष TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      भङ्ग;
    }
    पक्ष TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      भङ्ग;
    }
    पक्ष TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      भङ्ग;
    }
    पक्ष TK_NIL: {
      init_exp(v, VNIL, 0);
      भङ्ग;
    }
    पक्ष TK_TRUE: {
      init_exp(v, VTRUE, 0);
      भङ्ग;
    }
    पक्ष TK_FALSE: {
      init_exp(v, VFALSE, 0);
      भङ्ग;
    }
    पक्ष TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      भङ्ग;
    }
    पक्ष '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      पटिनिवत्त;
    }
    पक्ष TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      पटिनिवत्त;
    }
    स्वभाव: {
      suffixedexp(ls, v);
      पटिनिवत्त;
    }
  }
  luaX_next(ls);
}


स्थिर UnOpr getunopr (पूर्णाङ्क op) {
  विकल्प (op) {
    पक्ष TK_NOT: पटिनिवत्त OPR_NOT;
    पक्ष '-': पटिनिवत्त OPR_MINUS;
    पक्ष '~': पटिनिवत्त OPR_BNOT;
    पक्ष '#': पटिनिवत्त OPR_LEN;
    स्वभाव: पटिनिवत्त OPR_NOUNOPR;
  }
}


स्थिर BinOpr getbinopr (पूर्णाङ्क op) {
  विकल्प (op) {
    पक्ष '+': पटिनिवत्त OPR_ADD;
    पक्ष '-': पटिनिवत्त OPR_SUB;
    पक्ष '*': पटिनिवत्त OPR_MUL;
    पक्ष '%': पटिनिवत्त OPR_MOD;
    पक्ष '^': पटिनिवत्त OPR_POW;
    पक्ष '/': पटिनिवत्त OPR_DIV;
    पक्ष TK_IDIV: पटिनिवत्त OPR_IDIV;
    पक्ष '&': पटिनिवत्त OPR_BAND;
    पक्ष '|': पटिनिवत्त OPR_BOR;
    पक्ष '~': पटिनिवत्त OPR_BXOR;
    पक्ष TK_SHL: पटिनिवत्त OPR_SHL;
    पक्ष TK_SHR: पटिनिवत्त OPR_SHR;
    पक्ष TK_CONCAT: पटिनिवत्त OPR_CONCAT;
    पक्ष TK_NE: पटिनिवत्त OPR_NE;
    पक्ष TK_EQ: पटिनिवत्त OPR_EQ;
    पक्ष '<': पटिनिवत्त OPR_LT;
    पक्ष TK_LE: पटिनिवत्त OPR_LE;
    पक्ष '>': पटिनिवत्त OPR_GT;
    पक्ष TK_GE: पटिनिवत्त OPR_GE;
    पक्ष TK_AND: पटिनिवत्त OPR_AND;
    पक्ष TK_OR: पटिनिवत्त OPR_OR;
    स्वभाव: पटिनिवत्त OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
स्थिर ध्रुव संरचना {
  lu_byte वामः;  /* left priority for each binary operator */
  lu_byte दक्षिणः; /* right priority */
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
स्थिर BinOpr subexpr (LexState *ls, expdesc *v, पूर्णाङ्क limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  सचे (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    पूर्णाङ्क पङ्क्तिः = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, पङ्क्तिः);
  }
  अञ्ञथा simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  याव (op != OPR_NOBINOPR && priority[op].वामः > limit) {
    expdesc v2;
    BinOpr nextop;
    पूर्णाङ्क पङ्क्तिः = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].दक्षिणः);
    luaK_posfix(ls->fs, op, v, &v2, पङ्क्तिः);
    op = nextop;
  }
  leavelevel(ls);
  पटिनिवत्त op;  /* return first untreated operator */
}


स्थिर शून्य व्यञ्जकम् (LexState *ls, expdesc *v) {
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
  पूर्णाङ्क conflict = 0;
  अत्थाय (; lh; lh = lh->prev) {  /* check all previous assignments */
    सचे (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      सचे (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        सचे (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      अञ्ञथा {  /* table is a register */
        सचे (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        सचे (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  सचे (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    सचे (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    अञ्ञथा
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
स्थिर शून्य restassign (LexState *ls, संरचना LHS_assign *lh, पूर्णाङ्क nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  सचे (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    संरचना LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    सचे (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  अञ्ञथा {  /* restassign -> '=' explist */
    पूर्णाङ्क nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    सचे (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    अञ्ञथा {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      पटिनिवत्त;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


स्थिर पूर्णाङ्क cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  व्यञ्जकम्(ls, &v);  /* read condition */
  सचे (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  पटिनिवत्त v.f;
}


स्थिर शून्य gotostat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, पङ्क्तिः);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
स्थिर शून्य breakstat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  अत्थाय (bl = ls->fs->bl; bl != रिक्त; bl = bl->previous) {
    सचे (bl->isloop)  /* found one? */
      गच्छ ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, पङ्क्तिः);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
स्थिर शून्य checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  सचे (l_unlikely(lb != रिक्त))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->पङ्क्तिः);  /* error */
}


स्थिर शून्य labelstat (LexState *ls, TString *name, पूर्णाङ्क पङ्क्तिः) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  याव (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, पङ्क्तिः, block_follow(ls, 0));
}


स्थिर शून्य whilestat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  पूर्णाङ्क whileinit;
  पूर्णाङ्क condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, पङ्क्तिः);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


स्थिर शून्य repeatstat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* repeatstat -> REPEAT block UNTIL cond */
  पूर्णाङ्क condexit;
  FuncState *fs = ls->fs;
  पूर्णाङ्क repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, पङ्क्तिः);
  condexit = cond(ls);  /* read condition (inside scope block) */
  सचे (bl2.upval) {  /* upvalues? */
    पूर्णाङ्क exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
स्थिर शून्य exp1 (LexState *ls) {
  expdesc e;
  व्यञ्जकम्(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
स्थिर शून्य fixforjump (FuncState *fs, पूर्णाङ्क pc, पूर्णाङ्क dest, पूर्णाङ्क back) {
  Instruction *jmp = &fs->f->code[pc];
  पूर्णाङ्क offset = dest - (pc + 1);
  सचे (back)
    offset = -offset;
  सचे (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
स्थिर शून्य forbody (LexState *ls, पूर्णाङ्क base, पूर्णाङ्क पङ्क्तिः, पूर्णाङ्क nvars, पूर्णाङ्क isgen) {
  /* forbody -> DO block */
  स्थिर ध्रुव OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  स्थिर ध्रुव OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  पूर्णाङ्क prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  सचे (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, पङ्क्तिः);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, पङ्क्तिः);
}


स्थिर शून्य fornum (LexState *ls, TString *varname, पूर्णाङ्क पङ्क्तिः) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  पूर्णाङ्क base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  सचे (testnext(ls, ','))
    exp1(ls);  /* optional step */
  अञ्ञथा {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, पङ्क्तिः, 1, 0);
}


स्थिर शून्य forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  पूर्णाङ्क nvars = 4;  /* function, state, closing, control */
  पूर्णाङ्क पङ्क्तिः;
  पूर्णाङ्क base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  याव (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  पङ्क्तिः = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, पङ्क्तिः, nvars - 3, 1);
}


स्थिर शून्य forstat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  विकल्प (ls->t.token) {
    पक्ष '=': fornum(ls, varname, पङ्क्तिः); भङ्ग;
    पक्ष ',': पक्ष TK_IN: forlist(ls, varname); भङ्ग;
    स्वभाव: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, पङ्क्तिः);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


स्थिर शून्य test_then_block (LexState *ls, पूर्णाङ्क *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  पूर्णाङ्क condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  सचे (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


स्थिर शून्य ifstat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  पूर्णाङ्क escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  याव (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  सचे (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, पङ्क्तिः);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


स्थिर शून्य localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  पूर्णाङ्क fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


स्थिर lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  सचे (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ध्रुव वर्ण *attr = getstr(ts);
    checknext(ls, '>');
    सचे (strcmp(attr, "const") == 0)
      पटिनिवत्त RDKCONST;  /* read-only variable */
    अञ्ञथा सचे (strcmp(attr, "close") == 0)
      पटिनिवत्त RDKTOCLOSE;  /* to-be-closed variable */
    अञ्ञथा
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  पटिनिवत्त df;  /* return default value */
}


स्थिर शून्य checktoclose (FuncState *fs, पूर्णाङ्क level) {
  सचे (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


स्थिर शून्य localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  पूर्णाङ्क toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  पूर्णाङ्क vidx;  /* index of last variable */
  पूर्णाङ्क nvars = 0;
  पूर्णाङ्क nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  करोहि {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    सचे (kind == RDKTOCLOSE) {  /* to-be-closed? */
      सचे (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } याव (testnext(ls, ','));
  सचे (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  अञ्ञथा {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  सचे (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  अञ्ञथा {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


स्थिर lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  विकल्प (kind) {
    पक्ष RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      पटिनिवत्त kind;  /* to avoid warnings */
    पक्ष RDKCONST:
      पटिनिवत्त GDKCONST;  /* adjust kind for global variable */
    स्वभाव:
      पटिनिवत्त kind;
  }
}


स्थिर शून्य checkglobal (LexState *ls, TString *varname, पूर्णाङ्क पङ्क्तिः) {
  FuncState *fs = ls->fs;
  expdesc var;
  पूर्णाङ्क k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, पङ्क्तिः);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
स्थिर शून्य initglobal (LexState *ls, पूर्णाङ्क nvars, पूर्णाङ्क firstidx, पूर्णाङ्क n,
                        पूर्णाङ्क पङ्क्तिः) {
  सचे (n == nvars) {  /* traversed all variables? */
    expdesc e;
    पूर्णाङ्क nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  अञ्ञथा {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, पङ्क्तिः);
    leavelevel(ls);
    checkglobal(ls, varname, पङ्क्तिः);
    storevartop(fs, &var);
  }
}


स्थिर शून्य globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  पूर्णाङ्क nvars = 0;
  पूर्णाङ्क lastidx;  /* index of last registered variable */
  करोहि {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } याव (testnext(ls, ','));
  सचे (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


स्थिर शून्य globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  सचे (!testnext(ls, '*'))
    globalnames(ls, defkind);
  अञ्ञथा {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, रिक्त, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


स्थिर शून्य globalfunc (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, पङ्क्तिः);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, पङ्क्तिः);  /* definition "happens" in the first line */
}


स्थिर शून्य globalstatfunc (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  सचे (testnext(ls, TK_FUNCTION))
    globalfunc(ls, पङ्क्तिः);
  अञ्ञथा
    globalstat(ls);
}


स्थिर पूर्णाङ्क funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  पूर्णाङ्क ismethod = 0;
  singlevar(ls, v);
  याव (ls->t.token == '.')
    fieldsel(ls, v);
  सचे (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  पटिनिवत्त ismethod;
}


स्थिर शून्य funcstat (LexState *ls, पूर्णाङ्क पङ्क्तिः) {
  /* funcstat -> FUNCTION funcname body */
  पूर्णाङ्क ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, पङ्क्तिः);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, पङ्क्तिः);  /* definition "happens" in the first line */
}


स्थिर शून्य exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  संरचना LHS_assign v;
  suffixedexp(ls, &v.v);
  सचे (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = रिक्त;
    restassign(ls, &v, 1);
  }
  अञ्ञथा {  /* stat -> func */
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
  पूर्णाङ्क nret;  /* number of values being returned */
  पूर्णाङ्क first = luaY_nvarstack(fs);  /* first slot to be returned */
  सचे (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  अञ्ञथा {
    nret = explist(ls, &e);  /* optional return values */
    सचे (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      सचे (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    अञ्ञथा {
      सचे (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      अञ्ञथा {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


स्थिर शून्य statement (LexState *ls) {
  पूर्णाङ्क पङ्क्तिः = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  विकल्प (ls->t.token) {
    पक्ष ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      भङ्ग;
    }
    पक्ष TK_IF: {  /* stat -> ifstat */
      ifstat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_FOR: {  /* stat -> forstat */
      forstat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      सचे (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      अञ्ञथा
        localstat(ls);
      भङ्ग;
    }
    पक्ष TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      भङ्ग;
    }
    पक्ष TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, पङ्क्तिः);
      भङ्ग;
    }
#सचे LUA_COMPAT_GLOBAL
    पक्ष TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      सचे (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        पूर्णाङ्क lk = luaX_lookahead(ls);
        सचे (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, पङ्क्तिः);
          भङ्ग;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    स्वभाव: {  /* stat -> func | assignment */
      exprstat(ls);
      भङ्ग;
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
                       Dyndata *dyd, ध्रुव वर्ण *name, पूर्णाङ्क firstchar) {
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
  पटिनिवत्त cl;
}

