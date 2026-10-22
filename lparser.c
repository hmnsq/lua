/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lcode.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "llex.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lopcodes.h"
#অন্তর্ভাব "lparser.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"



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
প্রকারলেখ কাঠামো BlockCnt {
  কাঠামো BlockCnt *previous;  /* chain */
  পূর্ণসংখ্যা firstlabel;  /* index of first label in this block */
  পূর্ণসংখ্যা firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
স্থির শূন্য statement (LexState *ls);
স্থির শূন্য রাশিমালা (LexState *ls, expdesc *v);


স্থির l_noret error_expected (LexState *ls, পূর্ণসংখ্যা token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


স্থির l_noret errorlimit (FuncState *fs, পূর্ণসংখ্যা limit, ধ্রুবক অক্ষর *what) {
  lua_State *L = fs->ls->L;
  ধ্রুবক অক্ষর *বার্তা;
  পূর্ণসংখ্যা সারি = fs->f->linedefined;
  ধ্রুবক অক্ষর *where = (সারি == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", সারি);
  বার্তা = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, বার্তা);
}


শূন্য luaY_checklimit (FuncState *fs, পূর্ণসংখ্যা v, পূর্ণসংখ্যা l, ধ্রুবক অক্ষর *what) {
  যদি (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
স্থির পূর্ণসংখ্যা testnext (LexState *ls, পূর্ণসংখ্যা c) {
  যদি (ls->t.token == c) {
    lua_X_পরবর্তী(ls);
    ফেরত 1;
  }
  অন্যথা ফেরত 0;
}


/*
** Check that next token is 'c'.
*/
স্থির শূন্য check (LexState *ls, পূর্ণসংখ্যা c) {
  যদি (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
স্থির শূন্য checknext (LexState *ls, পূর্ণসংখ্যা c) {
  check(ls, c);
  lua_X_পরবর্তী(ls);
}


#define check_condition(ls,c,বার্তা)	{ যদি (!(c)) luaX_syntaxerror(ls, বার্তা); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
স্থির শূন্য check_match (LexState *ls, পূর্ণসংখ্যা what, পূর্ণসংখ্যা who, পূর্ণসংখ্যা where) {
  যদি (l_unlikely(!testnext(ls, what))) {
    যদি (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    অন্যথা {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


স্থির TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  lua_X_পরবর্তী(ls);
  ফেরত ts;
}


স্থির শূন্য init_exp (expdesc *e, expkind k, পূর্ণসংখ্যা i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


স্থির শূন্য codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


স্থির শূন্য codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
স্থির short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  পূর্ণসংখ্যা oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  যখন (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ফাঁকা;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ফেরত fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
স্থির পূর্ণসংখ্যা new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.আকার_2, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  ফেরত dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
স্থির পূর্ণসংখ্যা new_localvar (LexState *ls, TString *name) {
  ফেরত new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (আকার(v)/আকার(অক্ষর)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
স্থির Vardesc *getlocalvardesc (FuncState *fs, পূর্ণসংখ্যা vidx) {
  ফেরত &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
স্থির lu_byte reglevel (FuncState *fs, পূর্ণসংখ্যা nvar) {
  যখন (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    যদি (varinreg(vd))  /* is in a register? */
      ফেরত cast_byte(vd->vd.ridx + 1);
  }
  ফেরত 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ফেরত reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
স্থির LocVar *localdebuginfo (FuncState *fs, পূর্ণসংখ্যা vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  যদি (!varinreg(vd))
    ফেরত ফাঁকা;  /* no debug info. for constants */
  অন্যথা {
    পূর্ণসংখ্যা idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ফেরত &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
স্থির শূন্য init_var (FuncState *fs, expdesc *e, পূর্ণসংখ্যা vidx) {
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
স্থির শূন্য check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ফাঁকা;  /* to be set if variable is const */
  নির্বাচন (e->k) {
    ক্ষেত্র VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      বিরতি;
    }
    ক্ষেত্র VLOCAL: ক্ষেত্র VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      যদি (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      বিরতি;
    }
    ক্ষেত্র VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      যদি (up->kind != VDKREG)
        varname = up->name;
      বিরতি;
    }
    ক্ষেত্র VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    ক্ষেত্র VINDEXUP: ক্ষেত্র VINDEXSTR: ক্ষেত্র VINDEXED: {  /* global variable */
      যদি (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      বিরতি;
    }
    পূর্বনির্ধারিত:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ফেরত;  /* integer index cannot be read-only */
  }
  যদি (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
স্থির শূন্য adjustlocalvars (LexState *ls, পূর্ণসংখ্যা nvars) {
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা reglevel = luaY_nvarstack(fs);
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < nvars; i++) {
    পূর্ণসংখ্যা vidx = fs->nactvar++;
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
স্থির শূন্য removevars (FuncState *fs, পূর্ণসংখ্যা tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  যখন (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    যদি (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
স্থির পূর্ণসংখ্যা searchupvalue (FuncState *fs, TString *name) {
  পূর্ণসংখ্যা i;
  Upvaldesc *up = fs->f->upvalues;
  জন্য (i = 0; i < fs->nups; i++) {
    যদি (eqstr(up[i].name, name)) ফেরত i;
  }
  ফেরত -1;  /* not found */
}


স্থির Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  পূর্ণসংখ্যা oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  যখন (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ফাঁকা;
  ফেরত &f->upvalues[fs->nups++];
}


স্থির পূর্ণসংখ্যা newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *পূর্ব = fs->পূর্ব;
  যদি (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(পূর্ব, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(পূর্ব, v->u.var.vidx)->vd.name));
  }
  অন্যথা {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = পূর্ব->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, পূর্ব->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ফেরত fs->nups - 1;
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
স্থির পূর্ণসংখ্যা searchvar (FuncState *fs, TString *n, expdesc *var) {
  পূর্ণসংখ্যা i;
  জন্য (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    যদি (varglobal(vd)) {  /* global declaration? */
      যদি (vd->vd.name == ফাঁকা) {  /* collective declaration? */
        যদি (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      অন্যথা {  /* global name */
        যদি (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ফেরত VGLOBAL;
        }
        অন্যথা যদি (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    অন্যথা যদি (eqstr(n, vd->vd.name)) {  /* found? */
      যদি (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      অন্যথা {  /* local variable */
        init_var(fs, var, i);
        যদি (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ফেরত cast_int(var->k);
    }
  }
  ফেরত -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
স্থির শূন্য markupval (FuncState *fs, পূর্ণসংখ্যা level) {
  BlockCnt *bl = fs->bl;
  যখন (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
স্থির শূন্য marktobeclosed (FuncState *fs) {
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
স্থির শূন্য singlevaraux (FuncState *fs, TString *n, expdesc *var, পূর্ণসংখ্যা base) {
  পূর্ণসংখ্যা v = searchvar(fs, n, var);  /* look up variables at current level */
  যদি (v >= 0) {  /* found? */
    যদি (!base) {
      যদি (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      যদি (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  অন্যথা {  /* not found at current level; try upvalues */
    পূর্ণসংখ্যা idx = searchupvalue(fs, n);  /* try existing upvalues */
    যদি (idx < 0) {  /* not found? */
      যদি (fs->পূর্ব != ফাঁকা)  /* more levels? */
        singlevaraux(fs->পূর্ব, n, var, 0);  /* try upper levels */
      যদি (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      অন্যথা  /* it is a global or a constant */
        ফেরত;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


স্থির শূন্য buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc চাবি;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  যদি (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&চাবি, varname);  /* key is variable name */
  luaK_indexed(fs, var, &চাবি);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
স্থির শূন্য buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  যদি (var->k == VGLOBAL) {  /* global name? */
    পূর্ণসংখ্যা info = var->u.info;
    /* global by default in the scope of a global declaration? */
    যদি (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    যদি (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    অন্যথা  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


স্থির শূন্য singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
স্থির শূন্য adjust_assign (LexState *ls, পূর্ণসংখ্যা nvars, পূর্ণসংখ্যা nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  যদি (hasmultret(e->k)) {  /* last expression has multiple returns? */
    পূর্ণসংখ্যা extra = needed + 1;  /* discount last expression itself */
    যদি (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  অন্যথা {
    যদি (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    যদি (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  যদি (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  অন্যথা  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
স্থির l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ধ্রুবক অক্ষর *varname = (tsname != ফাঁকা) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->সারি, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
স্থির শূন্য closegoto (LexState *ls, পূর্ণসংখ্যা g, Labeldesc *label, পূর্ণসংখ্যা bup) {
  পূর্ণসংখ্যা i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  যদি (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  যদি (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  জন্য (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
স্থির Labeldesc *findlabel (LexState *ls, TString *name, পূর্ণসংখ্যা ilb) {
  Dyndata *dyd = ls->dyd;
  জন্য (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    যদি (eqstr(lb->name, name))  /* correct label? */
      ফেরত lb;
  }
  ফেরত ফাঁকা;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
স্থির পূর্ণসংখ্যা newlabelentry (LexState *ls, Labellist *l, TString *name,
                          পূর্ণসংখ্যা সারি, পূর্ণসংখ্যা pc) {
  পূর্ণসংখ্যা n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->আকার_2,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].সারি = সারি;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ফেরত n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
স্থির পূর্ণসংখ্যা newgotoentry (LexState *ls, TString *name, পূর্ণসংখ্যা সারি) {
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ফেরত newlabelentry(ls, &ls->dyd->gt, name, সারি, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
স্থির শূন্য createlabel (LexState *ls, TString *name, পূর্ণসংখ্যা সারি, পূর্ণসংখ্যা last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  পূর্ণসংখ্যা l = newlabelentry(ls, ll, name, সারি, luaK_getlabel(fs));
  যদি (last) {  /* label is last no-op statement in the block? */
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
স্থির শূন্য solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  পূর্ণসংখ্যা outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  পূর্ণসংখ্যা igt = bl->firstgoto;  /* first goto in the finishing block */
  যখন (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    যদি (lb != ফাঁকা)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    অন্যথা {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      যদি (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


স্থির শূন্য enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ফাঁকা && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
স্থির l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->সারি);
}


স্থির শূন্য leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  যদি (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  যদি (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  যদি (bl->previous == ফাঁকা) {  /* was it the last block? */
    যদি (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
স্থির Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  যদি (fs->np >= f->sizep) {
    পূর্ণসংখ্যা oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    যখন (oldsize < f->sizep)
      f->p[oldsize++] = ফাঁকা;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ফেরত clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
স্থির শূন্য codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->পূর্ব;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


স্থির শূন্য open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
  lua_State *L = ls->L;
  Proto *f = fs->f;
  fs->পূর্ব = ls->fs;  /* linked list of funcstates */
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
  fs->bl = ফাঁকা;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


স্থির শূন্য close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue অস্থায়ী;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ফাঁকা);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_সারি_Info);
  luaM_shrinkvector(L, f->k, f->sizek, fs->nk, TValue);
  luaM_shrinkvector(L, f->p, f->sizep, fs->np, Proto *);
  luaM_shrinkvector(L, f->locvars, f->sizelocvars, fs->ndebugvars, LocVar);
  luaM_shrinkvector(L, f->upvalues, f->sizeupvalues, fs->nups, Upvaldesc);
  /* remove kcache table from scanner table ("weigh" its anchor) */
  sethvalue(L, &অস্থায়ী, fs->kcache);  /* key to be set to nil */
  luaH_set(L, ls->h, &অস্থায়ী, &G(L)->nilvalue);
  ls->fs = fs->পূর্ব;
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
স্থির পূর্ণসংখ্যা block_follow (LexState *ls, পূর্ণসংখ্যা withuntil) {
  নির্বাচন (ls->t.token) {
    ক্ষেত্র TK_ELSE: ক্ষেত্র TK_ELSEIF:
    ক্ষেত্র TK_END: ক্ষেত্র TK_EOS:
      ফেরত 1;
    ক্ষেত্র TK_UNTIL: ফেরত withuntil;
    পূর্বনির্ধারিত: ফেরত 0;
  }
}


স্থির শূন্য statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  যখন (!block_follow(ls, 1)) {
    যদি (ls->t.token == TK_RETURN) {
      statement(ls);
      ফেরত;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


স্থির শূন্য fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc চাবি;
  luaK_exp2anyregup(fs, v);
  lua_X_পরবর্তী(ls);  /* skip the dot or colon */
  codename(ls, &চাবি);
  luaK_indexed(fs, v, &চাবি);
}


স্থির শূন্য yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  lua_X_পরবর্তী(ls);  /* skip the '[' */
  রাশিমালা(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

প্রকারলেখ কাঠামো ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  পূর্ণসংখ্যা nh;  /* total number of 'record' elements */
  পূর্ণসংখ্যা na;  /* number of array elements already stored */
  পূর্ণসংখ্যা tostore;  /* number of array elements pending to be stored */
  পূর্ণসংখ্যা maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#যদি MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


স্থির শূন্য recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, চাবি, val;
  যদি (ls->t.token == TK_NAME)
    codename(ls, &চাবি);
  অন্যথা  /* ls->t.token == '[' */
    yindex(ls, &চাবি);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &চাবি);
  রাশিমালা(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


স্থির শূন্য closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  যদি (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


স্থির শূন্য lastlistfield (FuncState *fs, ConsControl *cc) {
  যদি (cc->tostore == 0) ফেরত;
  যদি (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  অন্যথা {
    যদি (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


স্থির শূন্য listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  রাশিমালা(ls, &cc->v);
  cc->tostore++;
}


স্থির শূন্য field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  নির্বাচন(ls->t.token) {
    ক্ষেত্র TK_NAME: {  /* may be 'listfield' or 'recfield' */
      যদি (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      অন্যথা
        recfield(ls, cc);
      বিরতি;
    }
    ক্ষেত্র '[': {
      recfield(ls, cc);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      listfield(ls, cc);
      বিরতি;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
স্থির পূর্ণসংখ্যা maxtostore (FuncState *fs) {
  পূর্ণসংখ্যা numfreeregs = MAX_FSTACK - fs->freereg;
  যদি (numfreeregs >= 160)  /* "lots" of registers? */
    ফেরত numfreeregs / 5;  /* use up to 1/5 of them */
  অন্যথা যদি (numfreeregs >= 80)  /* still "enough" registers? */
    ফেরত 10;  /* one 'SETLIST' instruction for each 10 values */
  অন্যথা  /* save registers for potential more nesting */
    ফেরত 1;
}


স্থির শূন্য constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা সারি = ls->linenumber;
  পূর্ণসংখ্যা pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  করো {
    যদি (ls->t.token == /*{*/ '}') বিরতি;
    যদি (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } যখন (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, সারি);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


স্থির শূন্য setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


স্থির শূন্য parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  পূর্ণসংখ্যা nparams = 0;
  পূর্ণসংখ্যা varargk = 0;
  যদি (ls->t.token != ')') {  /* is 'parlist' not empty? */
    করো {
      নির্বাচন (ls->t.token) {
        ক্ষেত্র TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          বিরতি;
        }
        ক্ষেত্র TK_DOTS: {
          varargk = 1;
          lua_X_পরবর্তী(ls);  /* skip '...' */
          যদি (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          অন্যথা
            new_localvarliteral(ls, "(vararg table)");
          বিরতি;
        }
        পূর্বনির্ধারিত: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } যখন (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  যদি (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


স্থির শূন্য body (LexState *ls, expdesc *e, পূর্ণসংখ্যা ismethod, পূর্ণসংখ্যা সারি) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = সারি;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  যদি (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, সারি);
  codeclosure(ls, e);
  close_func(ls);
}


স্থির পূর্ণসংখ্যা explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  পূর্ণসংখ্যা n = 1;  /* at least one expression */
  রাশিমালা(ls, v);
  যখন (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    রাশিমালা(ls, v);
    n++;
  }
  ফেরত n;
}


স্থির শূন্য funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  পূর্ণসংখ্যা base, nparams;
  পূর্ণসংখ্যা সারি = ls->linenumber;
  নির্বাচন (ls->t.token) {
    ক্ষেত্র '(': {  /* funcargs -> '(' [ explist ] ')' */
      lua_X_পরবর্তী(ls);
      যদি (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      অন্যথা {
        explist(ls, &args);
        যদি (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', সারি);
      বিরতি;
    }
    ক্ষেত্র '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      বিরতি;
    }
    ক্ষেত্র TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      lua_X_পরবর্তী(ls);  /* must use 'seminfo' before 'next' */
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      luaX_syntaxerror(ls, "function arguments expected");
      ফেরত;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  যদি (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  অন্যথা {
    যদি (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, সারি);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


স্থির শূন্য primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  নির্বাচন (ls->t.token) {
    ক্ষেত্র '(': {
      পূর্ণসংখ্যা সারি = ls->linenumber;
      lua_X_পরবর্তী(ls);
      রাশিমালা(ls, v);
      check_match(ls, ')', '(', সারি);
      luaK_dischargevars(ls->fs, v);
      ফেরত;
    }
    ক্ষেত্র TK_NAME: {
      singlevar(ls, v);
      ফেরত;
    }
    পূর্বনির্ধারিত: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


স্থির শূন্য suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  জন্য (;;) {
    নির্বাচন (ls->t.token) {
      ক্ষেত্র '.': {  /* fieldsel */
        fieldsel(ls, v);
        বিরতি;
      }
      ক্ষেত্র '[': {  /* '[' exp ']' */
        expdesc চাবি;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &চাবি);
        luaK_indexed(fs, v, &চাবি);
        বিরতি;
      }
      ক্ষেত্র ':': {  /* ':' NAME funcargs */
        expdesc চাবি;
        lua_X_পরবর্তী(ls);
        codename(ls, &চাবি);
        luaK_self(fs, v, &চাবি);
        funcargs(ls, v);
        বিরতি;
      }
      ক্ষেত্র '(': ক্ষেত্র TK_STRING: ক্ষেত্র '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        বিরতি;
      }
      পূর্বনির্ধারিত: ফেরত;
    }
  }
}


স্থির শূন্য simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  নির্বাচন (ls->t.token) {
    ক্ষেত্র TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      বিরতি;
    }
    ক্ষেত্র TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      বিরতি;
    }
    ক্ষেত্র TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      বিরতি;
    }
    ক্ষেত্র TK_NIL: {
      init_exp(v, VNIL, 0);
      বিরতি;
    }
    ক্ষেত্র TK_TRUE: {
      init_exp(v, VTRUE, 0);
      বিরতি;
    }
    ক্ষেত্র TK_FALSE: {
      init_exp(v, VFALSE, 0);
      বিরতি;
    }
    ক্ষেত্র TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      বিরতি;
    }
    ক্ষেত্র '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ফেরত;
    }
    ক্ষেত্র TK_FUNCTION: {
      lua_X_পরবর্তী(ls);
      body(ls, v, 0, ls->linenumber);
      ফেরত;
    }
    পূর্বনির্ধারিত: {
      suffixedexp(ls, v);
      ফেরত;
    }
  }
  lua_X_পরবর্তী(ls);
}


স্থির UnOpr getunopr (পূর্ণসংখ্যা op) {
  নির্বাচন (op) {
    ক্ষেত্র TK_NOT: ফেরত OPR_NOT;
    ক্ষেত্র '-': ফেরত OPR_MINUS;
    ক্ষেত্র '~': ফেরত OPR_BNOT;
    ক্ষেত্র '#': ফেরত OPR_LEN;
    পূর্বনির্ধারিত: ফেরত OPR_NOUNOPR;
  }
}


স্থির BinOpr getbinopr (পূর্ণসংখ্যা op) {
  নির্বাচন (op) {
    ক্ষেত্র '+': ফেরত OPR_ADD;
    ক্ষেত্র '-': ফেরত OPR_SUB;
    ক্ষেত্র '*': ফেরত OPR_MUL;
    ক্ষেত্র '%': ফেরত OPR_MOD;
    ক্ষেত্র '^': ফেরত OPR_POW;
    ক্ষেত্র '/': ফেরত OPR_DIV;
    ক্ষেত্র TK_IDIV: ফেরত OPR_IDIV;
    ক্ষেত্র '&': ফেরত OPR_BAND;
    ক্ষেত্র '|': ফেরত OPR_BOR;
    ক্ষেত্র '~': ফেরত OPR_BXOR;
    ক্ষেত্র TK_SHL: ফেরত OPR_SHL;
    ক্ষেত্র TK_SHR: ফেরত OPR_SHR;
    ক্ষেত্র TK_CONCAT: ফেরত OPR_CONCAT;
    ক্ষেত্র TK_NE: ফেরত OPR_NE;
    ক্ষেত্র TK_EQ: ফেরত OPR_EQ;
    ক্ষেত্র '<': ফেরত OPR_LT;
    ক্ষেত্র TK_LE: ফেরত OPR_LE;
    ক্ষেত্র '>': ফেরত OPR_GT;
    ক্ষেত্র TK_GE: ফেরত OPR_GE;
    ক্ষেত্র TK_AND: ফেরত OPR_AND;
    ক্ষেত্র TK_OR: ফেরত OPR_OR;
    পূর্বনির্ধারিত: ফেরত OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
স্থির ধ্রুবক কাঠামো {
  lu_byte বাম;  /* left priority for each binary operator */
  lu_byte ডান; /* right priority */
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
স্থির BinOpr subexpr (LexState *ls, expdesc *v, পূর্ণসংখ্যা limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  যদি (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    পূর্ণসংখ্যা সারি = ls->linenumber;
    lua_X_পরবর্তী(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, সারি);
  }
  অন্যথা simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  যখন (op != OPR_NOBINOPR && priority[op].বাম > limit) {
    expdesc v2;
    BinOpr nextop;
    পূর্ণসংখ্যা সারি = ls->linenumber;
    lua_X_পরবর্তী(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].ডান);
    luaK_posfix(ls->fs, op, v, &v2, সারি);
    op = nextop;
  }
  leavelevel(ls);
  ফেরত op;  /* return first untreated operator */
}


স্থির শূন্য রাশিমালা (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


স্থির শূন্য block (LexState *ls) {
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
কাঠামো LHS_assign {
  কাঠামো LHS_assign *পূর্ব;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
স্থির শূন্য check_conflict (LexState *ls, কাঠামো LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  পূর্ণসংখ্যা conflict = 0;
  জন্য (; lh; lh = lh->পূর্ব) {  /* check all previous assignments */
    যদি (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      যদি (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        যদি (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      অন্যথা {  /* table is a register */
        যদি (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        যদি (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  যদি (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    যদি (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    অন্যথা
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
স্থির শূন্য storevartop (FuncState *fs, expdesc *var) {
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
স্থির শূন্য restassign (LexState *ls, কাঠামো LHS_assign *lh, পূর্ণসংখ্যা nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  যদি (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    কাঠামো LHS_assign nv;
    nv.পূর্ব = lh;
    suffixedexp(ls, &nv.v);
    যদি (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  অন্যথা {  /* restassign -> '=' explist */
    পূর্ণসংখ্যা nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    যদি (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    অন্যথা {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ফেরত;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


স্থির পূর্ণসংখ্যা cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  রাশিমালা(ls, &v);  /* read condition */
  যদি (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ফেরত v.f;
}


স্থির শূন্য gotostat (LexState *ls, পূর্ণসংখ্যা সারি) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, সারি);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
স্থির শূন্য breakstat (LexState *ls, পূর্ণসংখ্যা সারি) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  জন্য (bl = ls->fs->bl; bl != ফাঁকা; bl = bl->previous) {
    যদি (bl->isloop)  /* found one? */
      যাও ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  lua_X_পরবর্তী(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, সারি);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
স্থির শূন্য checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  যদি (l_unlikely(lb != ফাঁকা))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->সারি);  /* error */
}


স্থির শূন্য labelstat (LexState *ls, TString *name, পূর্ণসংখ্যা সারি) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  যখন (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, সারি, block_follow(ls, 0));
}


স্থির শূন্য whilestat (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা whileinit;
  পূর্ণসংখ্যা condexit;
  BlockCnt bl;
  lua_X_পরবর্তী(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, সারি);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


স্থির শূন্য repeatstat (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* repeatstat -> REPEAT block UNTIL cond */
  পূর্ণসংখ্যা condexit;
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  lua_X_পরবর্তী(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, সারি);
  condexit = cond(ls);  /* read condition (inside scope block) */
  যদি (bl2.upval) {  /* upvalues? */
    পূর্ণসংখ্যা exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
স্থির শূন্য exp1 (LexState *ls) {
  expdesc e;
  রাশিমালা(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
স্থির শূন্য fixforjump (FuncState *fs, পূর্ণসংখ্যা pc, পূর্ণসংখ্যা dest, পূর্ণসংখ্যা back) {
  Instruction *jmp = &fs->f->code[pc];
  পূর্ণসংখ্যা offset = dest - (pc + 1);
  যদি (back)
    offset = -offset;
  যদি (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
স্থির শূন্য forbody (LexState *ls, পূর্ণসংখ্যা base, পূর্ণসংখ্যা সারি, পূর্ণসংখ্যা nvars, পূর্ণসংখ্যা isgen) {
  /* forbody -> DO block */
  স্থির ধ্রুবক OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  স্থির ধ্রুবক OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  যদি (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, সারি);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, সারি);
}


স্থির শূন্য fornum (LexState *ls, TString *varname, পূর্ণসংখ্যা সারি) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  যদি (testnext(ls, ','))
    exp1(ls);  /* optional step */
  অন্যথা {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, সারি, 1, 0);
}


স্থির শূন্য forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  পূর্ণসংখ্যা nvars = 4;  /* function, state, closing, control */
  পূর্ণসংখ্যা সারি;
  পূর্ণসংখ্যা base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  যখন (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  সারি = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, সারি, nvars - 3, 1);
}


স্থির শূন্য forstat (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  lua_X_পরবর্তী(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  নির্বাচন (ls->t.token) {
    ক্ষেত্র '=': fornum(ls, varname, সারি); বিরতি;
    ক্ষেত্র ',': ক্ষেত্র TK_IN: forlist(ls, varname); বিরতি;
    পূর্বনির্ধারিত: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, সারি);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


স্থির শূন্য test_then_block (LexState *ls, পূর্ণসংখ্যা *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা condtrue;
  lua_X_পরবর্তী(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  যদি (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


স্থির শূন্য ifstat (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  যখন (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  যদি (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, সারি);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


স্থির শূন্য localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


স্থির lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  যদি (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ধ্রুবক অক্ষর *attr = getstr(ts);
    checknext(ls, '>');
    যদি (strcmp(attr, "const") == 0)
      ফেরত RDKCONST;  /* read-only variable */
    অন্যথা যদি (strcmp(attr, "close") == 0)
      ফেরত RDKTOCLOSE;  /* to-be-closed variable */
    অন্যথা
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ফেরত df;  /* return default value */
}


স্থির শূন্য checktoclose (FuncState *fs, পূর্ণসংখ্যা level) {
  যদি (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


স্থির শূন্য localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  পূর্ণসংখ্যা vidx;  /* index of last variable */
  পূর্ণসংখ্যা nvars = 0;
  পূর্ণসংখ্যা nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  করো {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    যদি (kind == RDKTOCLOSE) {  /* to-be-closed? */
      যদি (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } যখন (testnext(ls, ','));
  যদি (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  অন্যথা {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  যদি (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  অন্যথা {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


স্থির lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  নির্বাচন (kind) {
    ক্ষেত্র RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ফেরত kind;  /* to avoid warnings */
    ক্ষেত্র RDKCONST:
      ফেরত GDKCONST;  /* adjust kind for global variable */
    পূর্বনির্ধারিত:
      ফেরত kind;
  }
}


স্থির শূন্য checkglobal (LexState *ls, TString *varname, পূর্ণসংখ্যা সারি) {
  FuncState *fs = ls->fs;
  expdesc var;
  পূর্ণসংখ্যা k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, সারি);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
স্থির শূন্য initglobal (LexState *ls, পূর্ণসংখ্যা nvars, পূর্ণসংখ্যা firstidx, পূর্ণসংখ্যা n,
                        পূর্ণসংখ্যা সারি) {
  যদি (n == nvars) {  /* traversed all variables? */
    expdesc e;
    পূর্ণসংখ্যা nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  অন্যথা {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, সারি);
    leavelevel(ls);
    checkglobal(ls, varname, সারি);
    storevartop(fs, &var);
  }
}


স্থির শূন্য globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  পূর্ণসংখ্যা nvars = 0;
  পূর্ণসংখ্যা lastidx;  /* index of last registered variable */
  করো {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } যখন (testnext(ls, ','));
  যদি (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


স্থির শূন্য globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  যদি (!testnext(ls, '*'))
    globalnames(ls, defkind);
  অন্যথা {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ফাঁকা, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


স্থির শূন্য globalfunc (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, সারি);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, সারি);  /* definition "happens" in the first line */
}


স্থির শূন্য globalstatfunc (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  lua_X_পরবর্তী(ls);  /* skip 'global' */
  যদি (testnext(ls, TK_FUNCTION))
    globalfunc(ls, সারি);
  অন্যথা
    globalstat(ls);
}


স্থির পূর্ণসংখ্যা funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  পূর্ণসংখ্যা ismethod = 0;
  singlevar(ls, v);
  যখন (ls->t.token == '.')
    fieldsel(ls, v);
  যদি (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ফেরত ismethod;
}


স্থির শূন্য funcstat (LexState *ls, পূর্ণসংখ্যা সারি) {
  /* funcstat -> FUNCTION funcname body */
  পূর্ণসংখ্যা ismethod;
  expdesc v, b;
  lua_X_পরবর্তী(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, সারি);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, সারি);  /* definition "happens" in the first line */
}


স্থির শূন্য exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  কাঠামো LHS_assign v;
  suffixedexp(ls, &v.v);
  যদি (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.পূর্ব = ফাঁকা;
    restassign(ls, &v, 1);
  }
  অন্যথা {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


স্থির শূন্য retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  পূর্ণসংখ্যা nret;  /* number of values being returned */
  পূর্ণসংখ্যা first = luaY_nvarstack(fs);  /* first slot to be returned */
  যদি (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  অন্যথা {
    nret = explist(ls, &e);  /* optional return values */
    যদি (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      যদি (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    অন্যথা {
      যদি (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      অন্যথা {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


স্থির শূন্য statement (LexState *ls) {
  পূর্ণসংখ্যা সারি = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  নির্বাচন (ls->t.token) {
    ক্ষেত্র ';': {  /* stat -> ';' (empty statement) */
      lua_X_পরবর্তী(ls);  /* skip ';' */
      বিরতি;
    }
    ক্ষেত্র TK_IF: {  /* stat -> ifstat */
      ifstat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_DO: {  /* stat -> DO block END */
      lua_X_পরবর্তী(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_FOR: {  /* stat -> forstat */
      forstat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_LOCAL: {  /* stat -> localstat */
      lua_X_পরবর্তী(ls);  /* skip LOCAL */
      যদি (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      অন্যথা
        localstat(ls);
      বিরতি;
    }
    ক্ষেত্র TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_DBCOLON: {  /* stat -> label */
      lua_X_পরবর্তী(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), সারি);
      বিরতি;
    }
    ক্ষেত্র TK_RETURN: {  /* stat -> retstat */
      lua_X_পরবর্তী(ls);  /* skip RETURN */
      retstat(ls);
      বিরতি;
    }
    ক্ষেত্র TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, সারি);
      বিরতি;
    }
    ক্ষেত্র TK_GOTO: {  /* stat -> 'goto' NAME */
      lua_X_পরবর্তী(ls);  /* skip 'goto' */
      gotostat(ls, সারি);
      বিরতি;
    }
#যদি LUA_COMPAT_GLOBAL
    ক্ষেত্র TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      যদি (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        পূর্ণসংখ্যা lk = luaX_lookahead(ls);
        যদি (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, সারি);
          বিরতি;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    পূর্বনির্ধারিত: {  /* stat -> func | assignment */
      exprstat(ls);
      বিরতি;
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
স্থির শূন্য mainfunc (LexState *ls, FuncState *fs) {
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
  lua_X_পরবর্তী(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, Table *anchor, Mbuffer *buff,
                       Dyndata *dyd, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা firstchar) {
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
  lua_assert(!funcstate.পূর্ব && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  ফেরত cl;
}

