/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <string.h>

#include "lua.h"

#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"



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
typedef sa_nracanaa BlockCnt {
  sa_nracanaa BlockCnt *previous;  /* chain */
  puur_naa_nka firstlabel;  /* index of first label in this block */
  puur_naa_nka firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
static void statement (LexState *ls);
static void expr (LexState *ls, expdesc *v);


static l_noret error_expected (LexState *ls, puur_naa_nka token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


static l_noret errorlimit (FuncState *fs, puur_naa_nka limit, const var_na *what) {
  lua_State *L = fs->ls->L;
  const var_na *msg;
  puur_naa_nka line = fs->f->linedefined;
  const var_na *where = (line == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", line);
  msg = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, msg);
}


void luaY_checklimit (FuncState *fs, puur_naa_nka v, puur_naa_nka l, const var_na *what) {
  ᤀᤡ᤺ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
static puur_naa_nka testnext (LexState *ls, puur_naa_nka c) {
  ᤀᤡ᤺ (ls->t.token == c) {
    luaX_next(ls);
    ᤀᤡᤴ 1;
  }
  ᤀᤡ᤺ᤆᤡ ᤀᤡᤴ 0;
}


/*
** Check that next token is 'c'.
*/
static void check (LexState *ls, puur_naa_nka c) {
  ᤀᤡ᤺ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
static void checknext (LexState *ls, puur_naa_nka c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,msg)	{ ᤀᤡ᤺ (!(c)) luaX_syntaxerror(ls, msg); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
static void check_match (LexState *ls, puur_naa_nka what, puur_naa_nka who, puur_naa_nka where) {
  ᤀᤡ᤺ (l_unlikely(!testnext(ls, what))) {
    ᤀᤡ᤺ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    ᤀᤡ᤺ᤆᤡ {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


static TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  ᤀᤡᤴ ts;
}


static void init_exp (expdesc *e, expkind k, puur_naa_nka i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


static void codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


static void codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
static short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  puur_naa_nka oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ᤀᤠ᤺ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = NULL;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ᤀᤡᤴ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
static puur_naa_nka new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  ᤀᤡᤴ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
static puur_naa_nka new_localvar (LexState *ls, TString *name) {
  ᤀᤡᤴ new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (parimaa_na(v)/parimaa_na(var_na)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
static Vardesc *getlocalvardesc (FuncState *fs, puur_naa_nka vidx) {
  ᤀᤡᤴ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
static lu_byte reglevel (FuncState *fs, puur_naa_nka nvar) {
  ᤀᤠ᤺ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ᤀᤡ᤺ (varinreg(vd))  /* is in a register? */
      ᤀᤡᤴ cast_byte(vd->vd.ridx + 1);
  }
  ᤀᤡᤴ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ᤀᤡᤴ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
static LocVar *localdebuginfo (FuncState *fs, puur_naa_nka vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ᤀᤡ᤺ (!varinreg(vd))
    ᤀᤡᤴ NULL;  /* no debug info. for constants */
  ᤀᤡ᤺ᤆᤡ {
    puur_naa_nka idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ᤀᤡᤴ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
static void init_var (FuncState *fs, expdesc *e, puur_naa_nka vidx) {
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
static void check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = NULL;  /* to be set if variable is const */
  switch (e->k) {
    pak_sa VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      bha_nga;
    }
    pak_sa VLOCAL: pak_sa VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ᤀᤡ᤺ (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      bha_nga;
    }
    pak_sa VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      ᤀᤡ᤺ (up->kind != VDKREG)
        varname = up->name;
      bha_nga;
    }
    pak_sa VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    pak_sa VINDEXUP: pak_sa VINDEXSTR: pak_sa VINDEXED: {  /* global variable */
      ᤀᤡ᤺ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      bha_nga;
    }
    default:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ᤀᤡᤴ;  /* integer index cannot be read-only */
  }
  ᤀᤡ᤺ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
static void adjustlocalvars (LexState *ls, puur_naa_nka nvars) {
  FuncState *fs = ls->fs;
  puur_naa_nka reglevel = luaY_nvarstack(fs);
  puur_naa_nka i;
  for (i = 0; i < nvars; i++) {
    puur_naa_nka vidx = fs->nactvar++;
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
static void removevars (FuncState *fs, puur_naa_nka tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ᤀᤠ᤺ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ᤀᤡ᤺ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
static puur_naa_nka searchupvalue (FuncState *fs, TString *name) {
  puur_naa_nka i;
  Upvaldesc *up = fs->f->upvalues;
  for (i = 0; i < fs->nups; i++) {
    ᤀᤡ᤺ (eqstr(up[i].name, name)) ᤀᤡᤴ i;
  }
  ᤀᤡᤴ -1;  /* not found */
}


static Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  puur_naa_nka oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ᤀᤠ᤺ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = NULL;
  ᤀᤡᤴ &f->upvalues[fs->nups++];
}


static puur_naa_nka newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ᤀᤡ᤺ (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  ᤀᤡ᤺ᤆᤡ {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ᤀᤡᤴ fs->nups - 1;
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
static puur_naa_nka searchvar (FuncState *fs, TString *n, expdesc *var) {
  puur_naa_nka i;
  for (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ᤀᤡ᤺ (varglobal(vd)) {  /* global declaration? */
      ᤀᤡ᤺ (vd->vd.name == NULL) {  /* collective declaration? */
        ᤀᤡ᤺ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      ᤀᤡ᤺ᤆᤡ {  /* global name */
        ᤀᤡ᤺ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ᤀᤡᤴ VGLOBAL;
        }
        ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (eqstr(n, vd->vd.name)) {  /* found? */
      ᤀᤡ᤺ (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      ᤀᤡ᤺ᤆᤡ {  /* local variable */
        init_var(fs, var, i);
        ᤀᤡ᤺ (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ᤀᤡᤴ cast_int(var->k);
    }
  }
  ᤀᤡᤴ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
static void markupval (FuncState *fs, puur_naa_nka level) {
  BlockCnt *bl = fs->bl;
  ᤀᤠ᤺ (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
static void marktobeclosed (FuncState *fs) {
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
static void singlevaraux (FuncState *fs, TString *n, expdesc *var, puur_naa_nka base) {
  puur_naa_nka v = searchvar(fs, n, var);  /* look up variables at current level */
  ᤀᤡ᤺ (v >= 0) {  /* found? */
    ᤀᤡ᤺ (!base) {
      ᤀᤡ᤺ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ᤀᤡ᤺ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  ᤀᤡ᤺ᤆᤡ {  /* not found at current level; try upvalues */
    puur_naa_nka idx = searchupvalue(fs, n);  /* try existing upvalues */
    ᤀᤡ᤺ (idx < 0) {  /* not found? */
      ᤀᤡ᤺ (fs->prev != NULL)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ᤀᤡ᤺ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      ᤀᤡ᤺ᤆᤡ  /* it is a global or a constant */
        ᤀᤡᤴ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


static void buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc key;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ᤀᤡ᤺ (var->k == VGLOBAL)
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
static void buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ᤀᤡ᤺ (var->k == VGLOBAL) {  /* global name? */
    puur_naa_nka info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ᤀᤡ᤺ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ᤀᤡ᤺ (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    ᤀᤡ᤺ᤆᤡ  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


static void singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
static void adjust_assign (LexState *ls, puur_naa_nka nvars, puur_naa_nka nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  puur_naa_nka needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ᤀᤡ᤺ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    puur_naa_nka extra = needed + 1;  /* discount last expression itself */
    ᤀᤡ᤺ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  ᤀᤡ᤺ᤆᤡ {
    ᤀᤡ᤺ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ᤀᤡ᤺ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ᤀᤡ᤺ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  ᤀᤡ᤺ᤆᤡ  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
static l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  const var_na *varname = (tsname != NULL) ? getstr(tsname) : "*";
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
static void closegoto (LexState *ls, puur_naa_nka g, Labeldesc *label, puur_naa_nka bup) {
  puur_naa_nka i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ᤀᤡ᤺ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ᤀᤡ᤺ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  for (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
static Labeldesc *findlabel (LexState *ls, TString *name, puur_naa_nka ilb) {
  Dyndata *dyd = ls->dyd;
  for (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ᤀᤡ᤺ (eqstr(lb->name, name))  /* correct label? */
      ᤀᤡᤴ lb;
  }
  ᤀᤡᤴ NULL;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
static puur_naa_nka newlabelentry (LexState *ls, Labellist *l, TString *name,
                          puur_naa_nka line, puur_naa_nka pc) {
  puur_naa_nka n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ᤀᤡᤴ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
static puur_naa_nka newgotoentry (LexState *ls, TString *name, puur_naa_nka line) {
  FuncState *fs = ls->fs;
  puur_naa_nka pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ᤀᤡᤴ newlabelentry(ls, &ls->dyd->gt, name, line, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
static void createlabel (LexState *ls, TString *name, puur_naa_nka line, puur_naa_nka last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  puur_naa_nka l = newlabelentry(ls, ll, name, line, luaK_getlabel(fs));
  ᤀᤡ᤺ (last) {  /* label is last no-op statement in the block? */
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
static void solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  puur_naa_nka outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  puur_naa_nka igt = bl->firstgoto;  /* first goto in the finishing block */
  ᤀᤠ᤺ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ᤀᤡ᤺ (lb != NULL)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    ᤀᤡ᤺ᤆᤡ {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ᤀᤡ᤺ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


static void enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != NULL && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
static l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->line);
}


static void leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ᤀᤡ᤺ (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ᤀᤡ᤺ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ᤀᤡ᤺ (bl->previous == NULL) {  /* was it the last block? */
    ᤀᤡ᤺ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
static Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ᤀᤡ᤺ (fs->np >= f->sizep) {
    puur_naa_nka oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ᤀᤠ᤺ (oldsize < f->sizep)
      f->p[oldsize++] = NULL;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ᤀᤡᤴ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
static void codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


static void open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = NULL;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


static void close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == NULL);
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
static puur_naa_nka block_follow (LexState *ls, puur_naa_nka withuntil) {
  switch (ls->t.token) {
    pak_sa TK_ELSE: pak_sa TK_ELSEIF:
    pak_sa TK_END: pak_sa TK_EOS:
      ᤀᤡᤴ 1;
    pak_sa TK_UNTIL: ᤀᤡᤴ withuntil;
    default: ᤀᤡᤴ 0;
  }
}


static void statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ᤀᤠ᤺ (!block_follow(ls, 1)) {
    ᤀᤡ᤺ (ls->t.token == TK_RETURN) {
      statement(ls);
      ᤀᤡᤴ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


static void fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc key;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &key);
  luaK_indexed(fs, v, &key);
}


static void yindex (LexState *ls, expdesc *v) {
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

typedef sa_nracanaa ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  puur_naa_nka nh;  /* total number of 'record' elements */
  puur_naa_nka na;  /* number of array elements already stored */
  puur_naa_nka tostore;  /* number of array elements pending to be stored */
  puur_naa_nka maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#ᤀᤡ᤺ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


static void recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, key, val;
  ᤀᤡ᤺ (ls->t.token == TK_NAME)
    codename(ls, &key);
  ᤀᤡ᤺ᤆᤡ  /* ls->t.token == '[' */
    yindex(ls, &key);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &key);
  expr(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


static void closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ᤀᤡ᤺ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


static void lastlistfield (FuncState *fs, ConsControl *cc) {
  ᤀᤡ᤺ (cc->tostore == 0) ᤀᤡᤴ;
  ᤀᤡ᤺ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  ᤀᤡ᤺ᤆᤡ {
    ᤀᤡ᤺ (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


static void listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  expr(ls, &cc->v);
  cc->tostore++;
}


static void field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  switch(ls->t.token) {
    pak_sa TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ᤀᤡ᤺ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      ᤀᤡ᤺ᤆᤡ
        recfield(ls, cc);
      bha_nga;
    }
    pak_sa '[': {
      recfield(ls, cc);
      bha_nga;
    }
    default: {
      listfield(ls, cc);
      bha_nga;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
static puur_naa_nka maxtostore (FuncState *fs) {
  puur_naa_nka numfreeregs = MAX_FSTACK - fs->freereg;
  ᤀᤡ᤺ (numfreeregs >= 160)  /* "lots" of registers? */
    ᤀᤡᤴ numfreeregs / 5;  /* use up to 1/5 of them */
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (numfreeregs >= 80)  /* still "enough" registers? */
    ᤀᤡᤴ 10;  /* one 'SETLIST' instruction for each 10 values */
  ᤀᤡ᤺ᤆᤡ  /* save registers for potential more nesting */
    ᤀᤡᤴ 1;
}


static void constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  puur_naa_nka line = ls->linenumber;
  puur_naa_nka pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  do {
    ᤀᤡ᤺ (ls->t.token == /*{*/ '}') bha_nga;
    ᤀᤡ᤺ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ᤀᤠ᤺ (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, line);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


static void setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


static void parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  puur_naa_nka nparams = 0;
  puur_naa_nka varargk = 0;
  ᤀᤡ᤺ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    do {
      switch (ls->t.token) {
        pak_sa TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          bha_nga;
        }
        pak_sa TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          ᤀᤡ᤺ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          ᤀᤡ᤺ᤆᤡ
            new_localvarliteral(ls, "(vararg table)");
          bha_nga;
        }
        default: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ᤀᤠ᤺ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ᤀᤡ᤺ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


static void body (LexState *ls, expdesc *e, puur_naa_nka ismethod, puur_naa_nka line) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = line;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ᤀᤡ᤺ (ismethod) {
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


static puur_naa_nka explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  puur_naa_nka n = 1;  /* at least one expression */
  expr(ls, v);
  ᤀᤠ᤺ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    expr(ls, v);
    n++;
  }
  ᤀᤡᤴ n;
}


static void funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  puur_naa_nka base, nparams;
  puur_naa_nka line = ls->linenumber;
  switch (ls->t.token) {
    pak_sa '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      ᤀᤡ᤺ (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      ᤀᤡ᤺ᤆᤡ {
        explist(ls, &args);
        ᤀᤡ᤺ (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', line);
      bha_nga;
    }
    pak_sa '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      bha_nga;
    }
    pak_sa TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      bha_nga;
    }
    default: {
      luaX_syntaxerror(ls, "function arguments expected");
      ᤀᤡᤴ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  ᤀᤡ᤺ (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  ᤀᤡ᤺ᤆᤡ {
    ᤀᤡ᤺ (args.k != VVOID)
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


static void primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  switch (ls->t.token) {
    pak_sa '(': {
      puur_naa_nka line = ls->linenumber;
      luaX_next(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      ᤀᤡᤴ;
    }
    pak_sa TK_NAME: {
      singlevar(ls, v);
      ᤀᤡᤴ;
    }
    default: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


static void suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  for (;;) {
    switch (ls->t.token) {
      pak_sa '.': {  /* fieldsel */
        fieldsel(ls, v);
        bha_nga;
      }
      pak_sa '[': {  /* '[' exp ']' */
        expdesc key;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &key);
        luaK_indexed(fs, v, &key);
        bha_nga;
      }
      pak_sa ':': {  /* ':' NAME funcargs */
        expdesc key;
        luaX_next(ls);
        codename(ls, &key);
        luaK_self(fs, v, &key);
        funcargs(ls, v);
        bha_nga;
      }
      pak_sa '(': pak_sa TK_STRING: pak_sa '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        bha_nga;
      }
      default: ᤀᤡᤴ;
    }
  }
}


static void simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  switch (ls->t.token) {
    pak_sa TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      bha_nga;
    }
    pak_sa TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      bha_nga;
    }
    pak_sa TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      bha_nga;
    }
    pak_sa TK_NIL: {
      init_exp(v, VNIL, 0);
      bha_nga;
    }
    pak_sa TK_TRUE: {
      init_exp(v, VTRUE, 0);
      bha_nga;
    }
    pak_sa TK_FALSE: {
      init_exp(v, VFALSE, 0);
      bha_nga;
    }
    pak_sa TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      bha_nga;
    }
    pak_sa '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ᤀᤡᤴ;
    }
    pak_sa TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      ᤀᤡᤴ;
    }
    default: {
      suffixedexp(ls, v);
      ᤀᤡᤴ;
    }
  }
  luaX_next(ls);
}


static UnOpr getunopr (puur_naa_nka op) {
  switch (op) {
    pak_sa TK_NOT: ᤀᤡᤴ OPR_NOT;
    pak_sa '-': ᤀᤡᤴ OPR_MINUS;
    pak_sa '~': ᤀᤡᤴ OPR_BNOT;
    pak_sa '#': ᤀᤡᤴ OPR_LEN;
    default: ᤀᤡᤴ OPR_NOUNOPR;
  }
}


static BinOpr getbinopr (puur_naa_nka op) {
  switch (op) {
    pak_sa '+': ᤀᤡᤴ OPR_ADD;
    pak_sa '-': ᤀᤡᤴ OPR_SUB;
    pak_sa '*': ᤀᤡᤴ OPR_MUL;
    pak_sa '%': ᤀᤡᤴ OPR_MOD;
    pak_sa '^': ᤀᤡᤴ OPR_POW;
    pak_sa '/': ᤀᤡᤴ OPR_DIV;
    pak_sa TK_IDIV: ᤀᤡᤴ OPR_IDIV;
    pak_sa '&': ᤀᤡᤴ OPR_BAND;
    pak_sa '|': ᤀᤡᤴ OPR_BOR;
    pak_sa '~': ᤀᤡᤴ OPR_BXOR;
    pak_sa TK_SHL: ᤀᤡᤴ OPR_SHL;
    pak_sa TK_SHR: ᤀᤡᤴ OPR_SHR;
    pak_sa TK_CONCAT: ᤀᤡᤴ OPR_CONCAT;
    pak_sa TK_NE: ᤀᤡᤴ OPR_NE;
    pak_sa TK_EQ: ᤀᤡᤴ OPR_EQ;
    pak_sa '<': ᤀᤡᤴ OPR_LT;
    pak_sa TK_LE: ᤀᤡᤴ OPR_LE;
    pak_sa '>': ᤀᤡᤴ OPR_GT;
    pak_sa TK_GE: ᤀᤡᤴ OPR_GE;
    pak_sa TK_AND: ᤀᤡᤴ OPR_AND;
    pak_sa TK_OR: ᤀᤡᤴ OPR_OR;
    default: ᤀᤡᤴ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
static const sa_nracanaa {
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
static BinOpr subexpr (LexState *ls, expdesc *v, puur_naa_nka limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ᤀᤡ᤺ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    puur_naa_nka line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, line);
  }
  ᤀᤡ᤺ᤆᤡ simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ᤀᤠ᤺ (op != OPR_NOBINOPR && priority[op].left > limit) {
    expdesc v2;
    BinOpr nextop;
    puur_naa_nka line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].right);
    luaK_posfix(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  leavelevel(ls);
  ᤀᤡᤴ op;  /* return first untreated operator */
}


static void expr (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


static void block (LexState *ls) {
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
sa_nracanaa LHS_assign {
  sa_nracanaa LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
static void check_conflict (LexState *ls, sa_nracanaa LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  puur_naa_nka conflict = 0;
  for (; lh; lh = lh->prev) {  /* check all previous assignments */
    ᤀᤡ᤺ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ᤀᤡ᤺ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ᤀᤡ᤺ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      ᤀᤡ᤺ᤆᤡ {  /* table is a register */
        ᤀᤡ᤺ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ᤀᤡ᤺ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ᤀᤡ᤺ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ᤀᤡ᤺ (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    ᤀᤡ᤺ᤆᤡ
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
static void storevartop (FuncState *fs, expdesc *var) {
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
static void restassign (LexState *ls, sa_nracanaa LHS_assign *lh, puur_naa_nka nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ᤀᤡ᤺ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    sa_nracanaa LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ᤀᤡ᤺ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  ᤀᤡ᤺ᤆᤡ {  /* restassign -> '=' explist */
    puur_naa_nka nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ᤀᤡ᤺ (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    ᤀᤡ᤺ᤆᤡ {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ᤀᤡᤴ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


static puur_naa_nka cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  expr(ls, &v);  /* read condition */
  ᤀᤡ᤺ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ᤀᤡᤴ v.f;
}


static void gotostat (LexState *ls, puur_naa_nka line) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, line);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
static void breakstat (LexState *ls, puur_naa_nka line) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  for (bl = ls->fs->bl; bl != NULL; bl = bl->previous) {
    ᤀᤡ᤺ (bl->isloop)  /* found one? */
      goto ok;
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
static void checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ᤀᤡ᤺ (l_unlikely(lb != NULL))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->line);  /* error */
}


static void labelstat (LexState *ls, TString *name, puur_naa_nka line) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ᤀᤠ᤺ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, line, block_follow(ls, 0));
}


static void whilestat (LexState *ls, puur_naa_nka line) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  puur_naa_nka whileinit;
  puur_naa_nka condexit;
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


static void repeatstat (LexState *ls, puur_naa_nka line) {
  /* repeatstat -> REPEAT block UNTIL cond */
  puur_naa_nka condexit;
  FuncState *fs = ls->fs;
  puur_naa_nka repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, line);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ᤀᤡ᤺ (bl2.upval) {  /* upvalues? */
    puur_naa_nka exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
static void exp1 (LexState *ls) {
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
static void fixforjump (FuncState *fs, puur_naa_nka pc, puur_naa_nka dest, puur_naa_nka back) {
  Instruction *jmp = &fs->f->code[pc];
  puur_naa_nka offset = dest - (pc + 1);
  ᤀᤡ᤺ (back)
    offset = -offset;
  ᤀᤡ᤺ (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
static void forbody (LexState *ls, puur_naa_nka base, puur_naa_nka line, puur_naa_nka nvars, puur_naa_nka isgen) {
  /* forbody -> DO block */
  static const OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  static const OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  puur_naa_nka prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ᤀᤡ᤺ (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, line);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, line);
}


static void fornum (LexState *ls, TString *varname, puur_naa_nka line) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  puur_naa_nka base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  ᤀᤡ᤺ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  ᤀᤡ᤺ᤆᤡ {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, line, 1, 0);
}


static void forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  puur_naa_nka nvars = 4;  /* function, state, closing, control */
  puur_naa_nka line;
  puur_naa_nka base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ᤀᤠ᤺ (testnext(ls, ',')) {
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


static void forstat (LexState *ls, puur_naa_nka line) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  switch (ls->t.token) {
    pak_sa '=': fornum(ls, varname, line); bha_nga;
    pak_sa ',': pak_sa TK_IN: forlist(ls, varname); bha_nga;
    default: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, line);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


static void test_then_block (LexState *ls, puur_naa_nka *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  puur_naa_nka condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  ᤀᤡ᤺ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


static void ifstat (LexState *ls, puur_naa_nka line) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  puur_naa_nka escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  ᤀᤠ᤺ (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  ᤀᤡ᤺ (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, line);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


static void localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  puur_naa_nka fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


static lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ᤀᤡ᤺ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    const var_na *attr = getstr(ts);
    checknext(ls, '>');
    ᤀᤡ᤺ (strcmp(attr, "const") == 0)
      ᤀᤡᤴ RDKCONST;  /* read-only variable */
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(attr, "close") == 0)
      ᤀᤡᤴ RDKTOCLOSE;  /* to-be-closed variable */
    ᤀᤡ᤺ᤆᤡ
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ᤀᤡᤴ df;  /* return default value */
}


static void checktoclose (FuncState *fs, puur_naa_nka level) {
  ᤀᤡ᤺ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


static void localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  puur_naa_nka toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  puur_naa_nka vidx;  /* index of last variable */
  puur_naa_nka nvars = 0;
  puur_naa_nka nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  do {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    ᤀᤡ᤺ (kind == RDKTOCLOSE) {  /* to-be-closed? */
      ᤀᤡ᤺ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ᤀᤠ᤺ (testnext(ls, ','));
  ᤀᤡ᤺ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  ᤀᤡ᤺ᤆᤡ {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ᤀᤡ᤺ (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  ᤀᤡ᤺ᤆᤡ {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


static lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  switch (kind) {
    pak_sa RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ᤀᤡᤴ kind;  /* to avoid warnings */
    pak_sa RDKCONST:
      ᤀᤡᤴ GDKCONST;  /* adjust kind for global variable */
    default:
      ᤀᤡᤴ kind;
  }
}


static void checkglobal (LexState *ls, TString *varname, puur_naa_nka line) {
  FuncState *fs = ls->fs;
  expdesc var;
  puur_naa_nka k;
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
static void initglobal (LexState *ls, puur_naa_nka nvars, puur_naa_nka firstidx, puur_naa_nka n,
                        puur_naa_nka line) {
  ᤀᤡ᤺ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    puur_naa_nka nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  ᤀᤡ᤺ᤆᤡ {  /* handle variable 'n' */
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


static void globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  puur_naa_nka nvars = 0;
  puur_naa_nka lastidx;  /* index of last registered variable */
  do {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } ᤀᤠ᤺ (testnext(ls, ','));
  ᤀᤡ᤺ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


static void globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ᤀᤡ᤺ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  ᤀᤡ᤺ᤆᤡ {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, NULL, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


static void globalfunc (LexState *ls, puur_naa_nka line) {
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


static void globalstatfunc (LexState *ls, puur_naa_nka line) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  ᤀᤡ᤺ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, line);
  ᤀᤡ᤺ᤆᤡ
    globalstat(ls);
}


static puur_naa_nka funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  puur_naa_nka ismethod = 0;
  singlevar(ls, v);
  ᤀᤠ᤺ (ls->t.token == '.')
    fieldsel(ls, v);
  ᤀᤡ᤺ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ᤀᤡᤴ ismethod;
}


static void funcstat (LexState *ls, puur_naa_nka line) {
  /* funcstat -> FUNCTION funcname body */
  puur_naa_nka ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, line);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, line);  /* definition "happens" in the first line */
}


static void exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  sa_nracanaa LHS_assign v;
  suffixedexp(ls, &v.v);
  ᤀᤡ᤺ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = NULL;
    restassign(ls, &v, 1);
  }
  ᤀᤡ᤺ᤆᤡ {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


static void retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  puur_naa_nka nret;  /* number of values being returned */
  puur_naa_nka first = luaY_nvarstack(fs);  /* first slot to be returned */
  ᤀᤡ᤺ (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  ᤀᤡ᤺ᤆᤡ {
    nret = explist(ls, &e);  /* optional return values */
    ᤀᤡ᤺ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ᤀᤡ᤺ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    ᤀᤡ᤺ᤆᤡ {
      ᤀᤡ᤺ (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      ᤀᤡ᤺ᤆᤡ {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


static void statement (LexState *ls) {
  puur_naa_nka line = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  switch (ls->t.token) {
    pak_sa ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      bha_nga;
    }
    pak_sa TK_IF: {  /* stat -> ifstat */
      ifstat(ls, line);
      bha_nga;
    }
    pak_sa TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, line);
      bha_nga;
    }
    pak_sa TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, line);
      bha_nga;
    }
    pak_sa TK_FOR: {  /* stat -> forstat */
      forstat(ls, line);
      bha_nga;
    }
    pak_sa TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, line);
      bha_nga;
    }
    pak_sa TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, line);
      bha_nga;
    }
    pak_sa TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      ᤀᤡ᤺ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      ᤀᤡ᤺ᤆᤡ
        localstat(ls);
      bha_nga;
    }
    pak_sa TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, line);
      bha_nga;
    }
    pak_sa TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), line);
      bha_nga;
    }
    pak_sa TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      bha_nga;
    }
    pak_sa TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, line);
      bha_nga;
    }
    pak_sa TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, line);
      bha_nga;
    }
#ᤀᤡ᤺ LUA_COMPAT_GLOBAL
    pak_sa TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ᤀᤡ᤺ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        puur_naa_nka lk = luaX_lookahead(ls);
        ᤀᤡ᤺ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, line);
          bha_nga;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    default: {  /* stat -> func | assignment */
      exprstat(ls);
      bha_nga;
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
static void mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, const var_na *name, puur_naa_nka firstchar) {
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
  ᤀᤡᤴ cl;
}

