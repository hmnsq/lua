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
  จำนวนเต็ม firstlabel;  /* index of first label in this block */
  จำนวนเต็ม firstgoto;  /* index of first pending goto in this block */
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


static l_noret error_expected (LexState *ls, จำนวนเต็ม token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


static l_noret errorlimit (FuncState *fs, จำนวนเต็ม limit, const var_na *what) {
  lua_State *L = fs->ls->L;
  const var_na *msg;
  จำนวนเต็ม line = fs->f->linedefined;
  const var_na *where = (line == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", line);
  msg = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, msg);
}


void luaY_checklimit (FuncState *fs, จำนวนเต็ม v, จำนวนเต็ม l, const var_na *what) {
  ถ้า (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
static จำนวนเต็ม testnext (LexState *ls, จำนวนเต็ม c) {
  ถ้า (ls->t.token == c) {
    luaX_next(ls);
    คืน 1;
  }
  มิฉะนั้น คืน 0;
}


/*
** Check that next token is 'c'.
*/
static void check (LexState *ls, จำนวนเต็ม c) {
  ถ้า (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
static void checknext (LexState *ls, จำนวนเต็ม c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,msg)	{ ถ้า (!(c)) luaX_syntaxerror(ls, msg); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
static void check_match (LexState *ls, จำนวนเต็ม what, จำนวนเต็ม who, จำนวนเต็ม where) {
  ถ้า (l_unlikely(!testnext(ls, what))) {
    ถ้า (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    มิฉะนั้น {
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
  คืน ts;
}


static void init_exp (expdesc *e, expkind k, จำนวนเต็ม i) {
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
  จำนวนเต็ม oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ขณะ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = NULL;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  คืน fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
static จำนวนเต็ม new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  คืน dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
static จำนวนเต็ม new_localvar (LexState *ls, TString *name) {
  คืน new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (parimaa_na(v)/parimaa_na(var_na)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
static Vardesc *getlocalvardesc (FuncState *fs, จำนวนเต็ม vidx) {
  คืน &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
static lu_byte reglevel (FuncState *fs, จำนวนเต็ม nvar) {
  ขณะ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ถ้า (varinreg(vd))  /* is in a register? */
      คืน cast_byte(vd->vd.ridx + 1);
  }
  คืน 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  คืน reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
static LocVar *localdebuginfo (FuncState *fs, จำนวนเต็ม vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ถ้า (!varinreg(vd))
    คืน NULL;  /* no debug info. for constants */
  มิฉะนั้น {
    จำนวนเต็ม idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    คืน &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
static void init_var (FuncState *fs, expdesc *e, จำนวนเต็ม vidx) {
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
      ถ้า (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      bha_nga;
    }
    pak_sa VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      ถ้า (up->kind != VDKREG)
        varname = up->name;
      bha_nga;
    }
    pak_sa VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    pak_sa VINDEXUP: pak_sa VINDEXSTR: pak_sa VINDEXED: {  /* global variable */
      ถ้า (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      bha_nga;
    }
    default:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      คืน;  /* integer index cannot be read-only */
  }
  ถ้า (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
static void adjustlocalvars (LexState *ls, จำนวนเต็ม nvars) {
  FuncState *fs = ls->fs;
  จำนวนเต็ม reglevel = luaY_nvarstack(fs);
  จำนวนเต็ม i;
  สำหรับ (i = 0; i < nvars; i++) {
    จำนวนเต็ม vidx = fs->nactvar++;
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
static void removevars (FuncState *fs, จำนวนเต็ม tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ขณะ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ถ้า (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
static จำนวนเต็ม searchupvalue (FuncState *fs, TString *name) {
  จำนวนเต็ม i;
  Upvaldesc *up = fs->f->upvalues;
  สำหรับ (i = 0; i < fs->nups; i++) {
    ถ้า (eqstr(up[i].name, name)) คืน i;
  }
  คืน -1;  /* not found */
}


static Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  จำนวนเต็ม oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ขณะ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = NULL;
  คืน &f->upvalues[fs->nups++];
}


static จำนวนเต็ม newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ถ้า (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  มิฉะนั้น {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  คืน fs->nups - 1;
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
static จำนวนเต็ม searchvar (FuncState *fs, TString *n, expdesc *var) {
  จำนวนเต็ม i;
  สำหรับ (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ถ้า (varglobal(vd)) {  /* global declaration? */
      ถ้า (vd->vd.name == NULL) {  /* collective declaration? */
        ถ้า (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      มิฉะนั้น {  /* global name */
        ถ้า (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          คืน VGLOBAL;
        }
        มิฉะนั้น ถ้า (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    มิฉะนั้น ถ้า (eqstr(n, vd->vd.name)) {  /* found? */
      ถ้า (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      มิฉะนั้น {  /* local variable */
        init_var(fs, var, i);
        ถ้า (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      คืน cast_int(var->k);
    }
  }
  คืน -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
static void markupval (FuncState *fs, จำนวนเต็ม level) {
  BlockCnt *bl = fs->bl;
  ขณะ (bl->nactvar > level)
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
static void singlevaraux (FuncState *fs, TString *n, expdesc *var, จำนวนเต็ม base) {
  จำนวนเต็ม v = searchvar(fs, n, var);  /* look up variables at current level */
  ถ้า (v >= 0) {  /* found? */
    ถ้า (!base) {
      ถ้า (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ถ้า (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  มิฉะนั้น {  /* not found at current level; try upvalues */
    จำนวนเต็ม idx = searchupvalue(fs, n);  /* try existing upvalues */
    ถ้า (idx < 0) {  /* not found? */
      ถ้า (fs->prev != NULL)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ถ้า (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      มิฉะนั้น  /* it is a global or a constant */
        คืน;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


static void buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc key;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ถ้า (var->k == VGLOBAL)
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
  ถ้า (var->k == VGLOBAL) {  /* global name? */
    จำนวนเต็ม info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ถ้า (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ถ้า (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    มิฉะนั้น  /* anyway must be a global */
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
static void adjust_assign (LexState *ls, จำนวนเต็ม nvars, จำนวนเต็ม nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  จำนวนเต็ม needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ถ้า (hasmultret(e->k)) {  /* last expression has multiple returns? */
    จำนวนเต็ม extra = needed + 1;  /* discount last expression itself */
    ถ้า (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  มิฉะนั้น {
    ถ้า (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ถ้า (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ถ้า (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  มิฉะนั้น  /* adding 'needed' is actually a subtraction */
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
static void closegoto (LexState *ls, จำนวนเต็ม g, Labeldesc *label, จำนวนเต็ม bup) {
  จำนวนเต็ม i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ถ้า (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ถ้า (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  สำหรับ (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
static Labeldesc *findlabel (LexState *ls, TString *name, จำนวนเต็ม ilb) {
  Dyndata *dyd = ls->dyd;
  สำหรับ (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ถ้า (eqstr(lb->name, name))  /* correct label? */
      คืน lb;
  }
  คืน NULL;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
static จำนวนเต็ม newlabelentry (LexState *ls, Labellist *l, TString *name,
                          จำนวนเต็ม line, จำนวนเต็ม pc) {
  จำนวนเต็ม n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  คืน n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
static จำนวนเต็ม newgotoentry (LexState *ls, TString *name, จำนวนเต็ม line) {
  FuncState *fs = ls->fs;
  จำนวนเต็ม pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  คืน newlabelentry(ls, &ls->dyd->gt, name, line, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
static void createlabel (LexState *ls, TString *name, จำนวนเต็ม line, จำนวนเต็ม last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  จำนวนเต็ม l = newlabelentry(ls, ll, name, line, luaK_getlabel(fs));
  ถ้า (last) {  /* label is last no-op statement in the block? */
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
  จำนวนเต็ม outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  จำนวนเต็ม igt = bl->firstgoto;  /* first goto in the finishing block */
  ขณะ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ถ้า (lb != NULL)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    มิฉะนั้น {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ถ้า (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
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
  ถ้า (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ถ้า (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ถ้า (bl->previous == NULL) {  /* was it the last block? */
    ถ้า (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
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
  ถ้า (fs->np >= f->sizep) {
    จำนวนเต็ม oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ขณะ (oldsize < f->sizep)
      f->p[oldsize++] = NULL;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  คืน clp;
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
static จำนวนเต็ม block_follow (LexState *ls, จำนวนเต็ม withuntil) {
  switch (ls->t.token) {
    pak_sa TK_ELSE: pak_sa TK_ELSEIF:
    pak_sa TK_END: pak_sa TK_EOS:
      คืน 1;
    pak_sa TK_UNTIL: คืน withuntil;
    default: คืน 0;
  }
}


static void statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ขณะ (!block_follow(ls, 1)) {
    ถ้า (ls->t.token == TK_RETURN) {
      statement(ls);
      คืน;  /* 'return' must be last statement */
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
  จำนวนเต็ม nh;  /* total number of 'record' elements */
  จำนวนเต็ม na;  /* number of array elements already stored */
  จำนวนเต็ม tostore;  /* number of array elements pending to be stored */
  จำนวนเต็ม maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#ถ้า MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


static void recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, key, val;
  ถ้า (ls->t.token == TK_NAME)
    codename(ls, &key);
  มิฉะนั้น  /* ls->t.token == '[' */
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
  ถ้า (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


static void lastlistfield (FuncState *fs, ConsControl *cc) {
  ถ้า (cc->tostore == 0) คืน;
  ถ้า (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  มิฉะนั้น {
    ถ้า (cc->v.k != VVOID)
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
      ถ้า (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      มิฉะนั้น
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
static จำนวนเต็ม maxtostore (FuncState *fs) {
  จำนวนเต็ม numfreeregs = MAX_FSTACK - fs->freereg;
  ถ้า (numfreeregs >= 160)  /* "lots" of registers? */
    คืน numfreeregs / 5;  /* use up to 1/5 of them */
  มิฉะนั้น ถ้า (numfreeregs >= 80)  /* still "enough" registers? */
    คืน 10;  /* one 'SETLIST' instruction for each 10 values */
  มิฉะนั้น  /* save registers for potential more nesting */
    คืน 1;
}


static void constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  จำนวนเต็ม line = ls->linenumber;
  จำนวนเต็ม pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
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
    ถ้า (ls->t.token == /*{*/ '}') bha_nga;
    ถ้า (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ขณะ (testnext(ls, ',') || testnext(ls, ';'));
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
  จำนวนเต็ม nparams = 0;
  จำนวนเต็ม varargk = 0;
  ถ้า (ls->t.token != ')') {  /* is 'parlist' not empty? */
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
          ถ้า (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          มิฉะนั้น
            new_localvarliteral(ls, "(vararg table)");
          bha_nga;
        }
        default: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ขณะ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ถ้า (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


static void body (LexState *ls, expdesc *e, จำนวนเต็ม ismethod, จำนวนเต็ม line) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = line;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ถ้า (ismethod) {
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


static จำนวนเต็ม explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  จำนวนเต็ม n = 1;  /* at least one expression */
  expr(ls, v);
  ขณะ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    expr(ls, v);
    n++;
  }
  คืน n;
}


static void funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  จำนวนเต็ม base, nparams;
  จำนวนเต็ม line = ls->linenumber;
  switch (ls->t.token) {
    pak_sa '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      ถ้า (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      มิฉะนั้น {
        explist(ls, &args);
        ถ้า (hasmultret(args.k))
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
      คืน;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  ถ้า (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  มิฉะนั้น {
    ถ้า (args.k != VVOID)
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
      จำนวนเต็ม line = ls->linenumber;
      luaX_next(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      คืน;
    }
    pak_sa TK_NAME: {
      singlevar(ls, v);
      คืน;
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
  สำหรับ (;;) {
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
      default: คืน;
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
      คืน;
    }
    pak_sa TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      คืน;
    }
    default: {
      suffixedexp(ls, v);
      คืน;
    }
  }
  luaX_next(ls);
}


static UnOpr getunopr (จำนวนเต็ม op) {
  switch (op) {
    pak_sa TK_NOT: คืน OPR_NOT;
    pak_sa '-': คืน OPR_MINUS;
    pak_sa '~': คืน OPR_BNOT;
    pak_sa '#': คืน OPR_LEN;
    default: คืน OPR_NOUNOPR;
  }
}


static BinOpr getbinopr (จำนวนเต็ม op) {
  switch (op) {
    pak_sa '+': คืน OPR_ADD;
    pak_sa '-': คืน OPR_SUB;
    pak_sa '*': คืน OPR_MUL;
    pak_sa '%': คืน OPR_MOD;
    pak_sa '^': คืน OPR_POW;
    pak_sa '/': คืน OPR_DIV;
    pak_sa TK_IDIV: คืน OPR_IDIV;
    pak_sa '&': คืน OPR_BAND;
    pak_sa '|': คืน OPR_BOR;
    pak_sa '~': คืน OPR_BXOR;
    pak_sa TK_SHL: คืน OPR_SHL;
    pak_sa TK_SHR: คืน OPR_SHR;
    pak_sa TK_CONCAT: คืน OPR_CONCAT;
    pak_sa TK_NE: คืน OPR_NE;
    pak_sa TK_EQ: คืน OPR_EQ;
    pak_sa '<': คืน OPR_LT;
    pak_sa TK_LE: คืน OPR_LE;
    pak_sa '>': คืน OPR_GT;
    pak_sa TK_GE: คืน OPR_GE;
    pak_sa TK_AND: คืน OPR_AND;
    pak_sa TK_OR: คืน OPR_OR;
    default: คืน OPR_NOBINOPR;
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
static BinOpr subexpr (LexState *ls, expdesc *v, จำนวนเต็ม limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ถ้า (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    จำนวนเต็ม line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, line);
  }
  มิฉะนั้น simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ขณะ (op != OPR_NOBINOPR && priority[op].left > limit) {
    expdesc v2;
    BinOpr nextop;
    จำนวนเต็ม line = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].right);
    luaK_posfix(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  leavelevel(ls);
  คืน op;  /* return first untreated operator */
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
  จำนวนเต็ม conflict = 0;
  สำหรับ (; lh; lh = lh->prev) {  /* check all previous assignments */
    ถ้า (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ถ้า (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ถ้า (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      มิฉะนั้น {  /* table is a register */
        ถ้า (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ถ้า (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ถ้า (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ถ้า (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    มิฉะนั้น
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
static void restassign (LexState *ls, sa_nracanaa LHS_assign *lh, จำนวนเต็ม nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ถ้า (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    sa_nracanaa LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ถ้า (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  มิฉะนั้น {  /* restassign -> '=' explist */
    จำนวนเต็ม nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ถ้า (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    มิฉะนั้น {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      คืน;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


static จำนวนเต็ม cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  expr(ls, &v);  /* read condition */
  ถ้า (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  คืน v.f;
}


static void gotostat (LexState *ls, จำนวนเต็ม line) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, line);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
static void breakstat (LexState *ls, จำนวนเต็ม line) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  สำหรับ (bl = ls->fs->bl; bl != NULL; bl = bl->previous) {
    ถ้า (bl->isloop)  /* found one? */
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
  ถ้า (l_unlikely(lb != NULL))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->line);  /* error */
}


static void labelstat (LexState *ls, TString *name, จำนวนเต็ม line) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ขณะ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, line, block_follow(ls, 0));
}


static void whilestat (LexState *ls, จำนวนเต็ม line) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  จำนวนเต็ม whileinit;
  จำนวนเต็ม condexit;
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


static void repeatstat (LexState *ls, จำนวนเต็ม line) {
  /* repeatstat -> REPEAT block UNTIL cond */
  จำนวนเต็ม condexit;
  FuncState *fs = ls->fs;
  จำนวนเต็ม repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, line);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ถ้า (bl2.upval) {  /* upvalues? */
    จำนวนเต็ม exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
static void fixforjump (FuncState *fs, จำนวนเต็ม pc, จำนวนเต็ม dest, จำนวนเต็ม back) {
  Instruction *jmp = &fs->f->code[pc];
  จำนวนเต็ม offset = dest - (pc + 1);
  ถ้า (back)
    offset = -offset;
  ถ้า (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
static void forbody (LexState *ls, จำนวนเต็ม base, จำนวนเต็ม line, จำนวนเต็ม nvars, จำนวนเต็ม isgen) {
  /* forbody -> DO block */
  static const OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  static const OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  จำนวนเต็ม prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ถ้า (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, line);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, line);
}


static void fornum (LexState *ls, TString *varname, จำนวนเต็ม line) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  จำนวนเต็ม base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  ถ้า (testnext(ls, ','))
    exp1(ls);  /* optional step */
  มิฉะนั้น {  /* default step = 1 */
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
  จำนวนเต็ม nvars = 4;  /* function, state, closing, control */
  จำนวนเต็ม line;
  จำนวนเต็ม base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ขณะ (testnext(ls, ',')) {
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


static void forstat (LexState *ls, จำนวนเต็ม line) {
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


static void test_then_block (LexState *ls, จำนวนเต็ม *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  จำนวนเต็ม condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  ถ้า (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


static void ifstat (LexState *ls, จำนวนเต็ม line) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  จำนวนเต็ม escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  ขณะ (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  ถ้า (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, line);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


static void localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  จำนวนเต็ม fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


static lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ถ้า (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    const var_na *attr = getstr(ts);
    checknext(ls, '>');
    ถ้า (strcmp(attr, "const") == 0)
      คืน RDKCONST;  /* read-only variable */
    มิฉะนั้น ถ้า (strcmp(attr, "close") == 0)
      คืน RDKTOCLOSE;  /* to-be-closed variable */
    มิฉะนั้น
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  คืน df;  /* return default value */
}


static void checktoclose (FuncState *fs, จำนวนเต็ม level) {
  ถ้า (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


static void localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  จำนวนเต็ม toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  จำนวนเต็ม vidx;  /* index of last variable */
  จำนวนเต็ม nvars = 0;
  จำนวนเต็ม nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  do {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    ถ้า (kind == RDKTOCLOSE) {  /* to-be-closed? */
      ถ้า (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ขณะ (testnext(ls, ','));
  ถ้า (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  มิฉะนั้น {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ถ้า (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  มิฉะนั้น {
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
      คืน kind;  /* to avoid warnings */
    pak_sa RDKCONST:
      คืน GDKCONST;  /* adjust kind for global variable */
    default:
      คืน kind;
  }
}


static void checkglobal (LexState *ls, TString *varname, จำนวนเต็ม line) {
  FuncState *fs = ls->fs;
  expdesc var;
  จำนวนเต็ม k;
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
static void initglobal (LexState *ls, จำนวนเต็ม nvars, จำนวนเต็ม firstidx, จำนวนเต็ม n,
                        จำนวนเต็ม line) {
  ถ้า (n == nvars) {  /* traversed all variables? */
    expdesc e;
    จำนวนเต็ม nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  มิฉะนั้น {  /* handle variable 'n' */
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
  จำนวนเต็ม nvars = 0;
  จำนวนเต็ม lastidx;  /* index of last registered variable */
  do {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } ขณะ (testnext(ls, ','));
  ถ้า (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


static void globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ถ้า (!testnext(ls, '*'))
    globalnames(ls, defkind);
  มิฉะนั้น {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, NULL, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


static void globalfunc (LexState *ls, จำนวนเต็ม line) {
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


static void globalstatfunc (LexState *ls, จำนวนเต็ม line) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  ถ้า (testnext(ls, TK_FUNCTION))
    globalfunc(ls, line);
  มิฉะนั้น
    globalstat(ls);
}


static จำนวนเต็ม funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  จำนวนเต็ม ismethod = 0;
  singlevar(ls, v);
  ขณะ (ls->t.token == '.')
    fieldsel(ls, v);
  ถ้า (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  คืน ismethod;
}


static void funcstat (LexState *ls, จำนวนเต็ม line) {
  /* funcstat -> FUNCTION funcname body */
  จำนวนเต็ม ismethod;
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
  ถ้า (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = NULL;
    restassign(ls, &v, 1);
  }
  มิฉะนั้น {  /* stat -> func */
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
  จำนวนเต็ม nret;  /* number of values being returned */
  จำนวนเต็ม first = luaY_nvarstack(fs);  /* first slot to be returned */
  ถ้า (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  มิฉะนั้น {
    nret = explist(ls, &e);  /* optional return values */
    ถ้า (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ถ้า (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    มิฉะนั้น {
      ถ้า (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      มิฉะนั้น {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


static void statement (LexState *ls) {
  จำนวนเต็ม line = ls->linenumber;  /* may be needed for error messages */
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
      ถ้า (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      มิฉะนั้น
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
#ถ้า LUA_COMPAT_GLOBAL
    pak_sa TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ถ้า (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        จำนวนเต็ม lk = luaX_lookahead(ls);
        ถ้า (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
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
                       Dyndata *dyd, const var_na *name, จำนวนเต็ม firstchar) {
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
  คืน cl;
}

