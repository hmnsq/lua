/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lcode.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"



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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ BlockCnt {
  ಸಂರಚನಾ BlockCnt *previous;  /* chain */
  ಪೂರ್ಣಾಙ್ಕ firstlabel;  /* index of first label in this block */
  ಪೂರ್ಣಾಙ್ಕ firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
ಸ್ಥಿರ ಶೂನ್ಯ statement (LexState *ls);
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ (LexState *ls, expdesc *v);


ಸ್ಥಿರ l_noret error_expected (LexState *ls, ಪೂರ್ಣಾಙ್ಕ token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ಸ್ಥಿರ l_noret errorlimit (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ limit, ಧ್ರುವ ವರ್ಣ *what) {
  lua_State *L = fs->ls->L;
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = fs->f->linedefined;
  ಧ್ರುವ ವರ್ಣ *where = (ಸಾಲು == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", ಸಾಲು);
  ಸಂದೇಶ = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, ಸಂದೇಶ);
}


ಶೂನ್ಯ luaY_checklimit (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ v, ಪೂರ್ಣಾಙ್ಕ l, ಧ್ರುವ ವರ್ಣ *what) {
  ಯದಿ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testnext (LexState *ls, ಪೂರ್ಣಾಙ್ಕ c) {
  ಯದಿ (ls->t.token == c) {
    luaX_next(ls);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** Check that next token is 'c'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check (LexState *ls, ಪೂರ್ಣಾಙ್ಕ c) {
  ಯದಿ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checknext (LexState *ls, ಪೂರ್ಣಾಙ್ಕ c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,ಸಂದೇಶ)	{ ಯದಿ (!(c)) luaX_syntaxerror(ls, ಸಂದೇಶ); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_match (LexState *ls, ಪೂರ್ಣಾಙ್ಕ what, ಪೂರ್ಣಾಙ್ಕ who, ಪೂರ್ಣಾಙ್ಕ where) {
  ಯದಿ (l_unlikely(!testnext(ls, what))) {
    ಯದಿ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    ಅನ್ಯಥಾ {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


ಸ್ಥಿರ TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  ಪ್ರತ್ಯಾಗಮ ts;
}


ಸ್ಥಿರ ಶೂನ್ಯ init_exp (expdesc *e, expkind k, ಪೂರ್ಣಾಙ್ಕ i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


ಸ್ಥಿರ ಶೂನ್ಯ codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


ಸ್ಥಿರ ಶೂನ್ಯ codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
ಸ್ಥಿರ short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  ಪೂರ್ಣಾಙ್ಕ oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ಯಾವತ್ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ರಿಕ್ತ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ಪ್ರತ್ಯಾಗಮ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  ಪ್ರತ್ಯಾಗಮ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ new_localvar (LexState *ls, TString *name) {
  ಪ್ರತ್ಯಾಗಮ new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (ಪರಿಮಾಣ(v)/ಪರಿಮಾಣ(ವರ್ಣ)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ಸ್ಥಿರ Vardesc *getlocalvardesc (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ vidx) {
  ಪ್ರತ್ಯಾಗಮ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ಸ್ಥಿರ lu_byte reglevel (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ nvar) {
  ಯಾವತ್ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ಯದಿ (varinreg(vd))  /* is in a register? */
      ಪ್ರತ್ಯಾಗಮ cast_byte(vd->vd.ridx + 1);
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ಪ್ರತ್ಯಾಗಮ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ಸ್ಥಿರ LocVar *localdebuginfo (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ಯದಿ (!varinreg(vd))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no debug info. for constants */
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ಪ್ರತ್ಯಾಗಮ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ಸ್ಥಿರ ಶೂನ್ಯ init_var (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಙ್ಕ vidx) {
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
ಸ್ಥಿರ ಶೂನ್ಯ check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ರಿಕ್ತ;  /* to be set if variable is const */
  ವಿಕಲ್ಪ (e->k) {
    ಪಕ್ಷ VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ VLOCAL: ಪಕ್ಷ VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ಯದಿ (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      ಯದಿ (up->kind != VDKREG)
        varname = up->name;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    ಪಕ್ಷ VINDEXUP: ಪಕ್ಷ VINDEXSTR: ಪಕ್ಷ VINDEXED: {  /* global variable */
      ಯದಿ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ಪ್ರತ್ಯಾಗಮ;  /* integer index cannot be read-only */
  }
  ಯದಿ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjustlocalvars (LexState *ls, ಪೂರ್ಣಾಙ್ಕ nvars) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ reglevel = luaY_nvarstack(fs);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < nvars; i++) {
    ಪೂರ್ಣಾಙ್ಕ vidx = fs->nactvar++;
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
ಸ್ಥಿರ ಶೂನ್ಯ removevars (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ಯಾವತ್ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ಯದಿ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searchupvalue (FuncState *fs, TString *name) {
  ಪೂರ್ಣಾಙ್ಕ i;
  Upvaldesc *up = fs->f->upvalues;
  ಪ್ರತಿ (i = 0; i < fs->nups; i++) {
    ಯದಿ (eqstr(up[i].name, name)) ಪ್ರತ್ಯಾಗಮ i;
  }
  ಪ್ರತ್ಯಾಗಮ -1;  /* not found */
}


ಸ್ಥಿರ Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  ಪೂರ್ಣಾಙ್ಕ oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ಯಾವತ್ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ರಿಕ್ತ;
  ಪ್ರತ್ಯಾಗಮ &f->upvalues[fs->nups++];
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ಯದಿ (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  ಅನ್ಯಥಾ {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ಪ್ರತ್ಯಾಗಮ fs->nups - 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searchvar (FuncState *fs, TString *n, expdesc *var) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ಯದಿ (varglobal(vd)) {  /* global declaration? */
      ಯದಿ (vd->vd.name == ರಿಕ್ತ) {  /* collective declaration? */
        ಯದಿ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      ಅನ್ಯಥಾ {  /* global name */
        ಯದಿ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ಪ್ರತ್ಯಾಗಮ VGLOBAL;
        }
        ಅನ್ಯಥಾ ಯದಿ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    ಅನ್ಯಥಾ ಯದಿ (eqstr(n, vd->vd.name)) {  /* found? */
      ಯದಿ (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      ಅನ್ಯಥಾ {  /* local variable */
        init_var(fs, var, i);
        ಯದಿ (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ಪ್ರತ್ಯಾಗಮ cast_int(var->k);
    }
  }
  ಪ್ರತ್ಯಾಗಮ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ಸ್ಥಿರ ಶೂನ್ಯ markupval (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ level) {
  BlockCnt *bl = fs->bl;
  ಯಾವತ್ (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ಸ್ಥಿರ ಶೂನ್ಯ marktobeclosed (FuncState *fs) {
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
ಸ್ಥಿರ ಶೂನ್ಯ singlevaraux (FuncState *fs, TString *n, expdesc *var, ಪೂರ್ಣಾಙ್ಕ base) {
  ಪೂರ್ಣಾಙ್ಕ v = searchvar(fs, n, var);  /* look up variables at current level */
  ಯದಿ (v >= 0) {  /* found? */
    ಯದಿ (!base) {
      ಯದಿ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ಯದಿ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  ಅನ್ಯಥಾ {  /* not found at current level; try upvalues */
    ಪೂರ್ಣಾಙ್ಕ idx = searchupvalue(fs, n);  /* try existing upvalues */
    ಯದಿ (idx < 0) {  /* not found? */
      ಯದಿ (fs->prev != ರಿಕ್ತ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ಯದಿ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      ಅನ್ಯಥಾ  /* it is a global or a constant */
        ಪ್ರತ್ಯಾಗಮ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc ಕೀಲಿ;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ಯದಿ (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&ಕೀಲಿ, varname);  /* key is variable name */
  luaK_indexed(fs, var, &ಕೀಲಿ);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ಸ್ಥಿರ ಶೂನ್ಯ buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ಯದಿ (var->k == VGLOBAL) {  /* global name? */
    ಪೂರ್ಣಾಙ್ಕ info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ಯದಿ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ಯದಿ (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    ಅನ್ಯಥಾ  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjust_assign (LexState *ls, ಪೂರ್ಣಾಙ್ಕ nvars, ಪೂರ್ಣಾಙ್ಕ nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ಯದಿ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    ಪೂರ್ಣಾಙ್ಕ extra = needed + 1;  /* discount last expression itself */
    ಯದಿ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  ಅನ್ಯಥಾ {
    ಯದಿ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ಯದಿ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ಯದಿ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  ಅನ್ಯಥಾ  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ಸ್ಥಿರ l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ಧ್ರುವ ವರ್ಣ *varname = (tsname != ರಿಕ್ತ) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->ಸಾಲು, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
ಸ್ಥಿರ ಶೂನ್ಯ closegoto (LexState *ls, ಪೂರ್ಣಾಙ್ಕ g, Labeldesc *label, ಪೂರ್ಣಾಙ್ಕ bup) {
  ಪೂರ್ಣಾಙ್ಕ i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ಯದಿ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ಯದಿ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  ಪ್ರತಿ (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ಸ್ಥಿರ Labeldesc *findlabel (LexState *ls, TString *name, ಪೂರ್ಣಾಙ್ಕ ilb) {
  Dyndata *dyd = ls->dyd;
  ಪ್ರತಿ (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ಯದಿ (eqstr(lb->name, name))  /* correct label? */
      ಪ್ರತ್ಯಾಗಮ lb;
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ newlabelentry (LexState *ls, Labellist *l, TString *name,
                          ಪೂರ್ಣಾಙ್ಕ ಸಾಲು, ಪೂರ್ಣಾಙ್ಕ pc) {
  ಪೂರ್ಣಾಙ್ಕ n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].ಸಾಲು = ಸಾಲು;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ಪ್ರತ್ಯಾಗಮ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ newgotoentry (LexState *ls, TString *name, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ಪ್ರತ್ಯಾಗಮ newlabelentry(ls, &ls->dyd->gt, name, ಸಾಲು, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ಸ್ಥಿರ ಶೂನ್ಯ createlabel (LexState *ls, TString *name, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು, ಪೂರ್ಣಾಙ್ಕ last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  ಪೂರ್ಣಾಙ್ಕ l = newlabelentry(ls, ll, name, ಸಾಲು, luaK_getlabel(fs));
  ಯದಿ (last) {  /* label is last no-op statement in the block? */
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
ಸ್ಥಿರ ಶೂನ್ಯ solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  ಪೂರ್ಣಾಙ್ಕ outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  ಪೂರ್ಣಾಙ್ಕ igt = bl->firstgoto;  /* first goto in the finishing block */
  ಯಾವತ್ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ಯದಿ (lb != ರಿಕ್ತ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    ಅನ್ಯಥಾ {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ಯದಿ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ಸ್ಥಿರ ಶೂನ್ಯ enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ರಿಕ್ತ && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
ಸ್ಥಿರ l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->ಸಾಲು);
}


ಸ್ಥಿರ ಶೂನ್ಯ leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ಯದಿ (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ಯದಿ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ಯದಿ (bl->previous == ರಿಕ್ತ) {  /* was it the last block? */
    ಯದಿ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ಸ್ಥಿರ Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ಯದಿ (fs->np >= f->sizep) {
    ಪೂರ್ಣಾಙ್ಕ oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ಯಾವತ್ (oldsize < f->sizep)
      f->p[oldsize++] = ರಿಕ್ತ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ಪ್ರತ್ಯಾಗಮ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ಸ್ಥಿರ ಶೂನ್ಯ codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ಸ್ಥಿರ ಶೂನ್ಯ open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = ರಿಕ್ತ;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ರಿಕ್ತ);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_ಸಾಲು_Info);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ block_follow (LexState *ls, ಪೂರ್ಣಾಙ್ಕ withuntil) {
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ TK_ELSE: ಪಕ್ಷ TK_ELSEIF:
    ಪಕ್ಷ TK_END: ಪಕ್ಷ TK_EOS:
      ಪ್ರತ್ಯಾಗಮ 1;
    ಪಕ್ಷ TK_UNTIL: ಪ್ರತ್ಯಾಗಮ withuntil;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ಯಾವತ್ (!block_follow(ls, 1)) {
    ಯದಿ (ls->t.token == TK_RETURN) {
      statement(ls);
      ಪ್ರತ್ಯಾಗಮ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc ಕೀಲಿ;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &ಕೀಲಿ);
  luaK_indexed(fs, v, &ಕೀಲಿ);
}


ಸ್ಥಿರ ಶೂನ್ಯ yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  ಅಭಿವ್ಯಕ್ತಿ(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  ಪೂರ್ಣಾಙ್ಕ nh;  /* total number of 'record' elements */
  ಪೂರ್ಣಾಙ್ಕ na;  /* number of array elements already stored */
  ಪೂರ್ಣಾಙ್ಕ tostore;  /* number of array elements pending to be stored */
  ಪೂರ್ಣಾಙ್ಕ maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#ಯದಿ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


ಸ್ಥಿರ ಶೂನ್ಯ recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, ಕೀಲಿ, val;
  ಯದಿ (ls->t.token == TK_NAME)
    codename(ls, &ಕೀಲಿ);
  ಅನ್ಯಥಾ  /* ls->t.token == '[' */
    yindex(ls, &ಕೀಲಿ);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &ಕೀಲಿ);
  ಅಭಿವ್ಯಕ್ತಿ(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ಸ್ಥಿರ ಶೂನ್ಯ closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ಯದಿ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ lastlistfield (FuncState *fs, ConsControl *cc) {
  ಯದಿ (cc->tostore == 0) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  ಅನ್ಯಥಾ {
    ಯದಿ (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


ಸ್ಥಿರ ಶೂನ್ಯ listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  ಅಭಿವ್ಯಕ್ತಿ(ls, &cc->v);
  cc->tostore++;
}


ಸ್ಥಿರ ಶೂನ್ಯ field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  ವಿಕಲ್ಪ(ls->t.token) {
    ಪಕ್ಷ TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ಯದಿ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      ಅನ್ಯಥಾ
        recfield(ls, cc);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ '[': {
      recfield(ls, cc);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      listfield(ls, cc);
      ಭಙ್ಗ;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ maxtostore (FuncState *fs) {
  ಪೂರ್ಣಾಙ್ಕ numfreeregs = MAX_FSTACK - fs->freereg;
  ಯದಿ (numfreeregs >= 160)  /* "lots" of registers? */
    ಪ್ರತ್ಯಾಗಮ numfreeregs / 5;  /* use up to 1/5 of them */
  ಅನ್ಯಥಾ ಯದಿ (numfreeregs >= 80)  /* still "enough" registers? */
    ಪ್ರತ್ಯಾಗಮ 10;  /* one 'SETLIST' instruction for each 10 values */
  ಅನ್ಯಥಾ  /* save registers for potential more nesting */
    ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;
  ಪೂರ್ಣಾಙ್ಕ pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  ಕುರು {
    ಯದಿ (ls->t.token == /*{*/ '}') ಭಙ್ಗ;
    ಯದಿ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ಯಾವತ್ (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, ಸಾಲು);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ಸ್ಥಿರ ಶೂನ್ಯ setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  ಪೂರ್ಣಾಙ್ಕ nparams = 0;
  ಪೂರ್ಣಾಙ್ಕ varargk = 0;
  ಯದಿ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    ಕುರು {
      ವಿಕಲ್ಪ (ls->t.token) {
        ಪಕ್ಷ TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          ಯದಿ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          ಅನ್ಯಥಾ
            new_localvarliteral(ls, "(vararg table)");
          ಭಙ್ಗ;
        }
        ಸ್ವಭಾವ: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ಯಾವತ್ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ಯದಿ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ಸ್ಥಿರ ಶೂನ್ಯ body (LexState *ls, expdesc *e, ಪೂರ್ಣಾಙ್ಕ ismethod, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = ಸಾಲು;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ಯದಿ (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, ಸಾಲು);
  codeclosure(ls, e);
  close_func(ls);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  ಪೂರ್ಣಾಙ್ಕ n = 1;  /* at least one expression */
  ಅಭಿವ್ಯಕ್ತಿ(ls, v);
  ಯಾವತ್ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    ಅಭಿವ್ಯಕ್ತಿ(ls, v);
    n++;
  }
  ಪ್ರತ್ಯಾಗಮ n;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  ಪೂರ್ಣಾಙ್ಕ base, nparams;
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      ಯದಿ (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      ಅನ್ಯಥಾ {
        explist(ls, &args);
        ಯದಿ (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      luaX_syntaxerror(ls, "function arguments expected");
      ಪ್ರತ್ಯಾಗಮ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  ಯದಿ (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  ಅನ್ಯಥಾ {
    ಯದಿ (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, ಸಾಲು);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ಸ್ಥಿರ ಶೂನ್ಯ primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ '(': {
      ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;
      luaX_next(ls);
      ಅಭಿವ್ಯಕ್ತಿ(ls, v);
      check_match(ls, ')', '(', ಸಾಲು);
      luaK_dischargevars(ls->fs, v);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಪಕ್ಷ TK_NAME: {
      singlevar(ls, v);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಸ್ವಭಾವ: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  ಪ್ರತಿ (;;) {
    ವಿಕಲ್ಪ (ls->t.token) {
      ಪಕ್ಷ '.': {  /* fieldsel */
        fieldsel(ls, v);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ '[': {  /* '[' exp ']' */
        expdesc ಕೀಲಿ;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &ಕೀಲಿ);
        luaK_indexed(fs, v, &ಕೀಲಿ);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ ':': {  /* ':' NAME funcargs */
        expdesc ಕೀಲಿ;
        luaX_next(ls);
        codename(ls, &ಕೀಲಿ);
        luaK_self(fs, v, &ಕೀಲಿ);
        funcargs(ls, v);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ '(': ಪಕ್ಷ TK_STRING: ಪಕ್ಷ '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ;
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_NIL: {
      init_exp(v, VNIL, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRUE: {
      init_exp(v, VTRUE, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_FALSE: {
      init_exp(v, VFALSE, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಪಕ್ಷ TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಸ್ವಭಾವ: {
      suffixedexp(ls, v);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  luaX_next(ls);
}


ಸ್ಥಿರ UnOpr getunopr (ಪೂರ್ಣಾಙ್ಕ op) {
  ವಿಕಲ್ಪ (op) {
    ಪಕ್ಷ TK_NOT: ಪ್ರತ್ಯಾಗಮ OPR_NOT;
    ಪಕ್ಷ '-': ಪ್ರತ್ಯಾಗಮ OPR_MINUS;
    ಪಕ್ಷ '~': ಪ್ರತ್ಯಾಗಮ OPR_BNOT;
    ಪಕ್ಷ '#': ಪ್ರತ್ಯಾಗಮ OPR_LEN;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ OPR_NOUNOPR;
  }
}


ಸ್ಥಿರ BinOpr getbinopr (ಪೂರ್ಣಾಙ್ಕ op) {
  ವಿಕಲ್ಪ (op) {
    ಪಕ್ಷ '+': ಪ್ರತ್ಯಾಗಮ OPR_ADD;
    ಪಕ್ಷ '-': ಪ್ರತ್ಯಾಗಮ OPR_SUB;
    ಪಕ್ಷ '*': ಪ್ರತ್ಯಾಗಮ OPR_MUL;
    ಪಕ್ಷ '%': ಪ್ರತ್ಯಾಗಮ OPR_MOD;
    ಪಕ್ಷ '^': ಪ್ರತ್ಯಾಗಮ OPR_POW;
    ಪಕ್ಷ '/': ಪ್ರತ್ಯಾಗಮ OPR_DIV;
    ಪಕ್ಷ TK_IDIV: ಪ್ರತ್ಯಾಗಮ OPR_IDIV;
    ಪಕ್ಷ '&': ಪ್ರತ್ಯಾಗಮ OPR_BAND;
    ಪಕ್ಷ '|': ಪ್ರತ್ಯಾಗಮ OPR_BOR;
    ಪಕ್ಷ '~': ಪ್ರತ್ಯಾಗಮ OPR_BXOR;
    ಪಕ್ಷ TK_SHL: ಪ್ರತ್ಯಾಗಮ OPR_SHL;
    ಪಕ್ಷ TK_SHR: ಪ್ರತ್ಯಾಗಮ OPR_SHR;
    ಪಕ್ಷ TK_CONCAT: ಪ್ರತ್ಯಾಗಮ OPR_CONCAT;
    ಪಕ್ಷ TK_NE: ಪ್ರತ್ಯಾಗಮ OPR_NE;
    ಪಕ್ಷ TK_EQ: ಪ್ರತ್ಯಾಗಮ OPR_EQ;
    ಪಕ್ಷ '<': ಪ್ರತ್ಯಾಗಮ OPR_LT;
    ಪಕ್ಷ TK_LE: ಪ್ರತ್ಯಾಗಮ OPR_LE;
    ಪಕ್ಷ '>': ಪ್ರತ್ಯಾಗಮ OPR_GT;
    ಪಕ್ಷ TK_GE: ಪ್ರತ್ಯಾಗಮ OPR_GE;
    ಪಕ್ಷ TK_AND: ಪ್ರತ್ಯಾಗಮ OPR_AND;
    ಪಕ್ಷ TK_OR: ಪ್ರತ್ಯಾಗಮ OPR_OR;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
  lu_byte ಎಡ;  /* left priority for each binary operator */
  lu_byte ಬಲ; /* right priority */
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
ಸ್ಥಿರ BinOpr subexpr (LexState *ls, expdesc *v, ಪೂರ್ಣಾಙ್ಕ limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ಯದಿ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, ಸಾಲು);
  }
  ಅನ್ಯಥಾ simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ಯಾವತ್ (op != OPR_NOBINOPR && priority[op].ಎಡ > limit) {
    expdesc v2;
    BinOpr nextop;
    ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].ಬಲ);
    luaK_posfix(ls->fs, op, v, &v2, ಸಾಲು);
    op = nextop;
  }
  leavelevel(ls);
  ಪ್ರತ್ಯಾಗಮ op;  /* return first untreated operator */
}


ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


ಸ್ಥಿರ ಶೂನ್ಯ block (LexState *ls) {
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
ಸಂರಚನಾ LHS_assign {
  ಸಂರಚನಾ LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_conflict (LexState *ls, ಸಂರಚನಾ LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  ಪೂರ್ಣಾಙ್ಕ conflict = 0;
  ಪ್ರತಿ (; lh; lh = lh->prev) {  /* check all previous assignments */
    ಯದಿ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ಯದಿ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ಯದಿ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      ಅನ್ಯಥಾ {  /* table is a register */
        ಯದಿ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ಯದಿ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ಯದಿ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ಯದಿ (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    ಅನ್ಯಥಾ
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
ಸ್ಥಿರ ಶೂನ್ಯ storevartop (FuncState *fs, expdesc *var) {
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
ಸ್ಥಿರ ಶೂನ್ಯ restassign (LexState *ls, ಸಂರಚನಾ LHS_assign *lh, ಪೂರ್ಣಾಙ್ಕ nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ಯದಿ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ಸಂರಚನಾ LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ಯದಿ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  ಅನ್ಯಥಾ {  /* restassign -> '=' explist */
    ಪೂರ್ಣಾಙ್ಕ nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ಯದಿ (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    ಅನ್ಯಥಾ {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ಪ್ರತ್ಯಾಗಮ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  ಅಭಿವ್ಯಕ್ತಿ(ls, &v);  /* read condition */
  ಯದಿ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ಪ್ರತ್ಯಾಗಮ v.f;
}


ಸ್ಥಿರ ಶೂನ್ಯ gotostat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, ಸಾಲು);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ಸ್ಥಿರ ಶೂನ್ಯ breakstat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  ಪ್ರತಿ (bl = ls->fs->bl; bl != ರಿಕ್ತ; bl = bl->previous) {
    ಯದಿ (bl->isloop)  /* found one? */
      ಗಚ್ಛ ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, ಸಾಲು);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ಯದಿ (l_unlikely(lb != ರಿಕ್ತ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->ಸಾಲು);  /* error */
}


ಸ್ಥಿರ ಶೂನ್ಯ labelstat (LexState *ls, TString *name, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ಯಾವತ್ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, ಸಾಲು, block_follow(ls, 0));
}


ಸ್ಥಿರ ಶೂನ್ಯ whilestat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ whileinit;
  ಪೂರ್ಣಾಙ್ಕ condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, ಸಾಲು);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ಸ್ಥಿರ ಶೂನ್ಯ repeatstat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* repeatstat -> REPEAT block UNTIL cond */
  ಪೂರ್ಣಾಙ್ಕ condexit;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, ಸಾಲು);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ಯದಿ (bl2.upval) {  /* upvalues? */
    ಪೂರ್ಣಾಙ್ಕ exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
ಸ್ಥಿರ ಶೂನ್ಯ exp1 (LexState *ls) {
  expdesc e;
  ಅಭಿವ್ಯಕ್ತಿ(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fixforjump (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ pc, ಪೂರ್ಣಾಙ್ಕ dest, ಪೂರ್ಣಾಙ್ಕ back) {
  Instruction *jmp = &fs->f->code[pc];
  ಪೂರ್ಣಾಙ್ಕ offset = dest - (pc + 1);
  ಯದಿ (back)
    offset = -offset;
  ಯದಿ (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
ಸ್ಥಿರ ಶೂನ್ಯ forbody (LexState *ls, ಪೂರ್ಣಾಙ್ಕ base, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು, ಪೂರ್ಣಾಙ್ಕ nvars, ಪೂರ್ಣಾಙ್ಕ isgen) {
  /* forbody -> DO block */
  ಸ್ಥಿರ ಧ್ರುವ OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ಸ್ಥಿರ ಧ್ರುವ OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ಯದಿ (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, ಸಾಲು);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, ಸಾಲು);
}


ಸ್ಥಿರ ಶೂನ್ಯ fornum (LexState *ls, TString *varname, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  ಯದಿ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  ಅನ್ಯಥಾ {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, ಸಾಲು, 1, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  ಪೂರ್ಣಾಙ್ಕ nvars = 4;  /* function, state, closing, control */
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು;
  ಪೂರ್ಣಾಙ್ಕ base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ಯಾವತ್ (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  ಸಾಲು = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, ಸಾಲು, nvars - 3, 1);
}


ಸ್ಥಿರ ಶೂನ್ಯ forstat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ '=': fornum(ls, varname, ಸಾಲು); ಭಙ್ಗ;
    ಪಕ್ಷ ',': ಪಕ್ಷ TK_IN: forlist(ls, varname); ಭಙ್ಗ;
    ಸ್ವಭಾವ: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, ಸಾಲು);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ಸ್ಥಿರ ಶೂನ್ಯ test_then_block (LexState *ls, ಪೂರ್ಣಾಙ್ಕ *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  ಯದಿ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ಸ್ಥಿರ ಶೂನ್ಯ ifstat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  ಯಾವತ್ (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  ಯದಿ (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, ಸಾಲು);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ಸ್ಥಿರ ಶೂನ್ಯ localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ಸ್ಥಿರ lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ಯದಿ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ಧ್ರುವ ವರ್ಣ *attr = getstr(ts);
    checknext(ls, '>');
    ಯದಿ (strcmp(attr, "const") == 0)
      ಪ್ರತ್ಯಾಗಮ RDKCONST;  /* read-only variable */
    ಅನ್ಯಥಾ ಯದಿ (strcmp(attr, "close") == 0)
      ಪ್ರತ್ಯಾಗಮ RDKTOCLOSE;  /* to-be-closed variable */
    ಅನ್ಯಥಾ
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ಪ್ರತ್ಯಾಗಮ df;  /* return default value */
}


ಸ್ಥಿರ ಶೂನ್ಯ checktoclose (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ level) {
  ಯದಿ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  ಪೂರ್ಣಾಙ್ಕ vidx;  /* index of last variable */
  ಪೂರ್ಣಾಙ್ಕ nvars = 0;
  ಪೂರ್ಣಾಙ್ಕ nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  ಕುರು {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    ಯದಿ (kind == RDKTOCLOSE) {  /* to-be-closed? */
      ಯದಿ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ಯಾವತ್ (testnext(ls, ','));
  ಯದಿ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  ಅನ್ಯಥಾ {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ಯದಿ (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  ಅನ್ಯಥಾ {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ಸ್ಥಿರ lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  ವಿಕಲ್ಪ (kind) {
    ಪಕ್ಷ RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ಪ್ರತ್ಯಾಗಮ kind;  /* to avoid warnings */
    ಪಕ್ಷ RDKCONST:
      ಪ್ರತ್ಯಾಗಮ GDKCONST;  /* adjust kind for global variable */
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ kind;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ checkglobal (LexState *ls, TString *varname, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  FuncState *fs = ls->fs;
  expdesc var;
  ಪೂರ್ಣಾಙ್ಕ k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, ಸಾಲು);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
ಸ್ಥಿರ ಶೂನ್ಯ initglobal (LexState *ls, ಪೂರ್ಣಾಙ್ಕ nvars, ಪೂರ್ಣಾಙ್ಕ firstidx, ಪೂರ್ಣಾಙ್ಕ n,
                        ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  ಯದಿ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    ಪೂರ್ಣಾಙ್ಕ nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  ಅನ್ಯಥಾ {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, ಸಾಲು);
    leavelevel(ls);
    checkglobal(ls, varname, ಸಾಲು);
    storevartop(fs, &var);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಙ್ಕ nvars = 0;
  ಪೂರ್ಣಾಙ್ಕ lastidx;  /* index of last registered variable */
  ಕುರು {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } ಯಾವತ್ (testnext(ls, ','));
  ಯದಿ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ಸ್ಥಿರ ಶೂನ್ಯ globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ಯದಿ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  ಅನ್ಯಥಾ {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ರಿಕ್ತ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ globalfunc (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, ಸಾಲು);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, ಸಾಲು);  /* definition "happens" in the first line */
}


ಸ್ಥಿರ ಶೂನ್ಯ globalstatfunc (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  ಯದಿ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, ಸಾಲು);
  ಅನ್ಯಥಾ
    globalstat(ls);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  ಪೂರ್ಣಾಙ್ಕ ismethod = 0;
  singlevar(ls, v);
  ಯಾವತ್ (ls->t.token == '.')
    fieldsel(ls, v);
  ಯದಿ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ಪ್ರತ್ಯಾಗಮ ismethod;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcstat (LexState *ls, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  /* funcstat -> FUNCTION funcname body */
  ಪೂರ್ಣಾಙ್ಕ ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, ಸಾಲು);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, ಸಾಲು);  /* definition "happens" in the first line */
}


ಸ್ಥಿರ ಶೂನ್ಯ exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ಸಂರಚನಾ LHS_assign v;
  suffixedexp(ls, &v.v);
  ಯದಿ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ರಿಕ್ತ;
    restassign(ls, &v, 1);
  }
  ಅನ್ಯಥಾ {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  ಪೂರ್ಣಾಙ್ಕ nret;  /* number of values being returned */
  ಪೂರ್ಣಾಙ್ಕ first = luaY_nvarstack(fs);  /* first slot to be returned */
  ಯದಿ (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  ಅನ್ಯಥಾ {
    nret = explist(ls, &e);  /* optional return values */
    ಯದಿ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ಯದಿ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    ಅನ್ಯಥಾ {
      ಯದಿ (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      ಅನ್ಯಥಾ {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ಸ್ಥಿರ ಶೂನ್ಯ statement (LexState *ls) {
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  ವಿಕಲ್ಪ (ls->t.token) {
    ಪಕ್ಷ ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IF: {  /* stat -> ifstat */
      ifstat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_FOR: {  /* stat -> forstat */
      forstat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      ಯದಿ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      ಅನ್ಯಥಾ
        localstat(ls);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, ಸಾಲು);
      ಭಙ್ಗ;
    }
#ಯದಿ LUA_COMPAT_GLOBAL
    ಪಕ್ಷ TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ಯದಿ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        ಪೂರ್ಣಾಙ್ಕ lk = luaX_lookahead(ls);
        ಯದಿ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, ಸಾಲು);
          ಭಙ್ಗ;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    ಸ್ವಭಾವ: {  /* stat -> func | assignment */
      exprstat(ls);
      ಭಙ್ಗ;
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
ಸ್ಥಿರ ಶೂನ್ಯ mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ firstchar) {
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
  ಪ್ರತ್ಯಾಗಮ cl;
}

