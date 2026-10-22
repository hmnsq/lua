/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lparser_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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
#ವ್ಯಾಖ್ಯಾನಿಸು MAXVARS		200


#ವ್ಯಾಖ್ಯಾನಿಸು hasmultret(k)		((k) == VCALL || (k) == VVARARG)


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#ವ್ಯಾಖ್ಯಾನಿಸು eqstr(a,b)	((a) == (b))


/*
** nodes for block list (list of active blocks)
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಖಂಡ_Cnt {
  ರಚನೆ ಖಂಡ_Cnt *ಹಿಂದಿನ;  /* chain */
  ಪೂರ್ಣಾಂಕ firstlabel;  /* index of first label in this block */
  ಪೂರ್ಣಾಂಕ firstgoto;  /* index of first pending goto in this block */
  ಗಿಡ್ಡ nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} ಖಂಡ_Cnt;



/*
** prototypes for recursive non-terminal functions
*/
ಸ್ಥಿರ ಶೂನ್ಯ statement (LexState *ls);
ಸ್ಥಿರ ಶೂನ್ಯ ಅಭಿವ್ಯಕ್ತಿ (LexState *ls, expdesc *v);


ಸ್ಥಿರ l_noret error_expected (LexState *ls, ಪೂರ್ಣಾಂಕ token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ಸ್ಥಿರ l_noret errorlimit (FuncState *fs, ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what) {
  lua_State *L = fs->ls->L;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
  ಪೂರ್ಣಾಂಕ ಸಾಲು = fs->f->linedefined;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *where = (ಸಾಲು == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", ಸಾಲು);
  ಸಂದೇಶ = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, ಎಲ್ಲೆ, where);
  luaX_syntaxerror(fs->ls, ಸಂದೇಶ);
}


ಶೂನ್ಯ luaY_checklimit (FuncState *fs, ಪೂರ್ಣಾಂಕ v, ಪೂರ್ಣಾಂಕ l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what) {
  ಆದರೆ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testnext (LexState *ls, ಪೂರ್ಣಾಂಕ c) {
  ಆದರೆ (ls->t.token == c) {
    lua_X_ಮುಂದಿನ(ls);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;
}


/*
** Check that next token is 'c'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check (LexState *ls, ಪೂರ್ಣಾಂಕ c) {
  ಆದರೆ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checknext (LexState *ls, ಪೂರ್ಣಾಂಕ c) {
  check(ls, c);
  lua_X_ಮುಂದಿನ(ls);
}


#ವ್ಯಾಖ್ಯಾನಿಸು check_condition(ls,c,ಸಂದೇಶ)	{ ಆದರೆ (!(c)) luaX_syntaxerror(ls, ಸಂದೇಶ); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_ಹೊಂದಿಕೆ (LexState *ls, ಪೂರ್ಣಾಂಕ what, ಪೂರ್ಣಾಂಕ who, ಪೂರ್ಣಾಂಕ where) {
  ಆದರೆ (l_unlikely(!testnext(ls, what))) {
    ಆದರೆ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    ಇಲ್ಲದಿದ್ದರೆ {
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
  lua_X_ಮುಂದಿನ(ls);
  ಹಿಂತಿರುಗಿ ts;
}


ಸ್ಥಿರ ಶೂನ್ಯ init_exp (expdesc *e, expkind k, ಪೂರ್ಣಾಂಕ i) {
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
ಸ್ಥಿರ ಗಿಡ್ಡ registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  ಪೂರ್ಣಾಂಕ oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ಆಗ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ಖಾಲಿ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ಹಿಂತಿರುಗಿ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ new_varkind (LexState *ls, TString *name, lu_byte ಬಗೆ) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.ಗಾತ್ರ_2, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.ಬಗೆ = ಬಗೆ;  /* default */
  var->vd.name = name;
  ಹಿಂತಿರುಗಿ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ new_localvar (LexState *ls, TString *name) {
  ಹಿಂತಿರುಗಿ new_varkind(ls, name, VDKREG);
}

#ವ್ಯಾಖ್ಯಾನಿಸು new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (ಗಾತ್ರ(v)/ಗಾತ್ರ(ಅಕ್ಷರ)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ಸ್ಥಿರ Vardesc *getlocalvardesc (FuncState *fs, ಪೂರ್ಣಾಂಕ vidx) {
  ಹಿಂತಿರುಗಿ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ಸ್ಥಿರ lu_byte reglevel (FuncState *fs, ಪೂರ್ಣಾಂಕ nvar) {
  ಆಗ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ಆದರೆ (varinreg(vd))  /* is in a register? */
      ಹಿಂತಿರುಗಿ cast_byte(vd->vd.ridx + 1);
  }
  ಹಿಂತಿರುಗಿ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ಹಿಂತಿರುಗಿ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ಸ್ಥಿರ LocVar *localdebuginfo (FuncState *fs, ಪೂರ್ಣಾಂಕ vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ಆದರೆ (!varinreg(vd))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no debug info. for constants */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ಹಿಂತಿರುಗಿ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ಸ್ಥಿರ ಶೂನ್ಯ init_var (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ vidx) {
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
  TString *varname = ಖಾಲಿ;  /* to be set if variable is const */
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VLOCAL: ಸ್ಥಿತಿ VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ಆದರೆ (vardesc->vd.ಬಗೆ != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VUPVAL: {
      Upvaldesc *ಮೇಲೆ = &fs->f->upvalues[e->u.info];
      ಆದರೆ (ಮೇಲೆ->ಬಗೆ != VDKREG)
        varname = ಮೇಲೆ->name;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ VINDEXUP: ಸ್ಥಿತಿ VINDEXSTR: ಸ್ಥಿತಿ VINDEXED: {  /* global variable */
      ಆದರೆ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ಹಿಂತಿರುಗಿ;  /* integer index cannot be read-only */
  }
  ಆದರೆ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjustlocalvars (LexState *ls, ಪೂರ್ಣಾಂಕ nvars) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ reglevel = luaY_nvarstack(fs);
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < nvars; i++) {
    ಪೂರ್ಣಾಂಕ vidx = fs->nactvar++;
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
ಸ್ಥಿರ ಶೂನ್ಯ removevars (FuncState *fs, ಪೂರ್ಣಾಂಕ tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ಆಗ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ಆದರೆ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searchupvalue (FuncState *fs, TString *name) {
  ಪೂರ್ಣಾಂಕ i;
  Upvaldesc *ಮೇಲೆ = fs->f->upvalues;
  ಗಾಗಿ (i = 0; i < fs->nups; i++) {
    ಆದರೆ (eqstr(ಮೇಲೆ[i].name, name)) ಹಿಂತಿರುಗಿ i;
  }
  ಹಿಂತಿರುಗಿ -1;  /* not found */
}


ಸ್ಥಿರ Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  ಪೂರ್ಣಾಂಕ oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ಆಗ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ಖಾಲಿ;
  ಹಿಂತಿರುಗಿ &f->upvalues[fs->nups++];
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *ಮೇಲೆ = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ಆದರೆ (v->k == VLOCAL) {
    ಮೇಲೆ->instack = 1;
    ಮೇಲೆ->idx = v->u.var.ridx;
    ಮೇಲೆ->ಬಗೆ = getlocalvardesc(prev, v->u.var.vidx)->vd.ಬಗೆ;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಮೇಲೆ->instack = 0;
    ಮೇಲೆ->idx = cast_byte(v->u.info);
    ಮೇಲೆ->ಬಗೆ = prev->f->upvalues[v->u.info].ಬಗೆ;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  ಮೇಲೆ->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ಹಿಂತಿರುಗಿ fs->nups - 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searchvar (FuncState *fs, TString *n, expdesc *var) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ಆದರೆ (varglobal(vd)) {  /* global declaration? */
      ಆದರೆ (vd->vd.name == ಖಾಲಿ) {  /* collective declaration? */
        ಆದರೆ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* global name */
        ಆದರೆ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ಹಿಂತಿರುಗಿ VGLOBAL;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (eqstr(n, vd->vd.name)) {  /* found? */
      ಆದರೆ (vd->vd.ಬಗೆ == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      ಇಲ್ಲದಿದ್ದರೆ {  /* local variable */
        init_var(fs, var, i);
        ಆದರೆ (vd->vd.ಬಗೆ == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ಹಿಂತಿರುಗಿ cast_int(var->k);
    }
  }
  ಹಿಂತಿರುಗಿ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ಸ್ಥಿರ ಶೂನ್ಯ markupval (FuncState *fs, ಪೂರ್ಣಾಂಕ level) {
  ಖಂಡ_Cnt *bl = fs->bl;
  ಆಗ (bl->nactvar > level)
    bl = bl->ಹಿಂದಿನ;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ಸ್ಥಿರ ಶೂನ್ಯ marktobeclosed (FuncState *fs) {
  ಖಂಡ_Cnt *bl = fs->bl;
  bl->upval = 1;
  bl->insidetbc = 1;
  fs->needclose = 1;
}


/*
** Find a variable with the given name 'n'. If it is an upvalue, add
** this upvalue into all intermediate functions. If it is a global, set
** 'var' as 'void' as a flag.
*/
ಸ್ಥಿರ ಶೂನ್ಯ singlevaraux (FuncState *fs, TString *n, expdesc *var, ಪೂರ್ಣಾಂಕ ತಳ) {
  ಪೂರ್ಣಾಂಕ v = searchvar(fs, n, var);  /* look up variables at current level */
  ಆದರೆ (v >= 0) {  /* found? */
    ಆದರೆ (!ತಳ) {
      ಆದರೆ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ಆದರೆ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* not found at current level; try upvalues */
    ಪೂರ್ಣಾಂಕ idx = searchupvalue(fs, n);  /* try existing upvalues */
    ಆದರೆ (idx < 0) {  /* not found? */
      ಆದರೆ (fs->prev != ಖಾಲಿ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ಆದರೆ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      ಇಲ್ಲದಿದ್ದರೆ  /* it is a global or a constant */
        ಹಿಂತಿರುಗಿ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc ಕೀಲಿ;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ಆದರೆ (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&ಕೀಲಿ, varname);  /* key is variable name */
  lua_K_ಸೂಚ್ಯಂಕಿತ(fs, var, &ಕೀಲಿ);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ಸ್ಥಿರ ಶೂನ್ಯ buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ಆದರೆ (var->k == VGLOBAL) {  /* global name? */
    ಪೂರ್ಣಾಂಕ info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ಆದರೆ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ಆದರೆ (info != -1 && ls->dyd->actvar.arr[info].vd.ಬಗೆ == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    ಇಲ್ಲದಿದ್ದರೆ  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.ಬಗೆ == GDKREG);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjust_ನಿಯೋಜಿಸು (LexState *ls, ಪೂರ್ಣಾಂಕ nvars, ಪೂರ್ಣಾಂಕ nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ಆದರೆ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    ಪೂರ್ಣಾಂಕ extra = needed + 1;  /* discount last expression itself */
    ಆದರೆ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ಆದರೆ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ಆದರೆ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  ಇಲ್ಲದಿದ್ದರೆ  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#ವ್ಯಾಖ್ಯಾನಿಸು enterlevel(ls)	luaE_incCstack(ls->L)


#ವ್ಯಾಖ್ಯಾನಿಸು leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ಸ್ಥಿರ l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *varname = (tsname != ಖಾಲಿ) ? getstr(tsname) : "*";
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
ಸ್ಥಿರ ಶೂನ್ಯ closegoto (LexState *ls, ಪೂರ್ಣಾಂಕ g, Labeldesc *label, ಪೂರ್ಣಾಂಕ bup) {
  ಪೂರ್ಣಾಂಕ i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ಆದರೆ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ಆದರೆ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->ಸಂಕೇತ_2[gt->pc + 1] = fs->f->ಸಂಕೇತ_2[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->ಸಂಕೇತ_2[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  ಗಾಗಿ (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ಸ್ಥಿರ Labeldesc *findlabel (LexState *ls, TString *name, ಪೂರ್ಣಾಂಕ ilb) {
  Dyndata *dyd = ls->dyd;
  ಗಾಗಿ (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ಆದರೆ (eqstr(lb->name, name))  /* correct label? */
      ಹಿಂತಿರುಗಿ lb;
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newlabelentry (LexState *ls, Labellist *l, TString *name,
                          ಪೂರ್ಣಾಂಕ ಸಾಲು, ಪೂರ್ಣಾಂಕ pc) {
  ಪೂರ್ಣಾಂಕ n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->ಗಾತ್ರ_2,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].ಸಾಲು = ಸಾಲು;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ಹಿಂತಿರುಗಿ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newgotoentry (LexState *ls, TString *name, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ pc = luaK_jump(fs);  /* create jump */
  lua_K_ಸಂಕೇತ_ABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ಹಿಂತಿರುಗಿ newlabelentry(ls, &ls->dyd->gt, name, ಸಾಲು, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ಸ್ಥಿರ ಶೂನ್ಯ createlabel (LexState *ls, TString *name, ಪೂರ್ಣಾಂಕ ಸಾಲು, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  ಪೂರ್ಣಾಂಕ l = newlabelentry(ls, ll, name, ಸಾಲು, luaK_getlabel(fs));
  ಆದರೆ (ಕೊನೆಯ) {  /* label is last no-op statement in the block? */
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
ಸ್ಥಿರ ಶೂನ್ಯ solvegotos (FuncState *fs, ಖಂಡ_Cnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  ಪೂರ್ಣಾಂಕ outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  ಪೂರ್ಣಾಂಕ igt = bl->firstgoto;  /* first goto in the finishing block */
  ಆಗ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ಆದರೆ (lb != ಖಾಲಿ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    ಇಲ್ಲದಿದ್ದರೆ {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ಆದರೆ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ಸ್ಥಿರ ಶೂನ್ಯ enterblock (FuncState *fs, ಖಂಡ_Cnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ಖಾಲಿ && fs->bl->insidetbc);
  bl->ಹಿಂದಿನ = fs->bl;  /* link block in function's block list */
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
  ಖಂಡ_Cnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ಆದರೆ (bl->ಹಿಂದಿನ && bl->upval)  /* need a 'close'? */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ಆದರೆ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ಆದರೆ (bl->ಹಿಂದಿನ == ಖಾಲಿ) {  /* was it the last block? */
    ಆದರೆ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->ಹಿಂದಿನ;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ಸ್ಥಿರ Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ಆದರೆ (fs->np >= f->sizep) {
    ಪೂರ್ಣಾಂಕ oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ಆಗ (oldsize < f->sizep)
      f->p[oldsize++] = ಖಾಲಿ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ಹಿಂತಿರುಗಿ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ಸ್ಥಿರ ಶೂನ್ಯ codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, lua_K_ಸಂಕೇತ_ABx(fs, OP_ಆವರಣ, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ಸ್ಥಿರ ಶೂನ್ಯ open_func (LexState *ls, FuncState *fs, ಖಂಡ_Cnt *bl) {
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
  fs->bl = ಖಾಲಿ;
  f->ಮೂಲ = ls->ಮೂಲ;
  luaC_objbarrier(L, f, f->ಮೂಲ);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue ತಾತ್ಕಾಲಿಕ;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ಖಾಲಿ);
  lua_K_ಮುಕ್ತಾಯ(fs);
  luaM_shrinkvector(L, f->ಸಂಕೇತ_2, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_ಸಾಲು_Info);
  luaM_shrinkvector(L, f->k, f->sizek, fs->nk, TValue);
  luaM_shrinkvector(L, f->p, f->sizep, fs->np, Proto *);
  luaM_shrinkvector(L, f->locvars, f->sizelocvars, fs->ndebugvars, LocVar);
  luaM_shrinkvector(L, f->upvalues, f->sizeupvalues, fs->nups, Upvaldesc);
  /* remove kcache table from scanner table ("weigh" its anchor) */
  sethvalue(L, &ತಾತ್ಕಾಲಿಕ, fs->kcache);  /* key to be set to nil */
  lua_H_ಗಣ(L, ls->h, &ತಾತ್ಕಾಲಿಕ, &G(L)->nilvalue);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಖಂಡ_follow (LexState *ls, ಪೂರ್ಣಾಂಕ withuntil) {
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ TK_ELSE: ಸ್ಥಿತಿ TK_ELSEIF:
    ಸ್ಥಿತಿ TK_ಕೊನೆ: ಸ್ಥಿತಿ TK_EOS:
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ TK_UNTIL: ಹಿಂತಿರುಗಿ withuntil;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ಆಗ (!ಖಂಡ_follow(ls, 1)) {
    ಆದರೆ (ls->t.token == TK_RETURN) {
      statement(ls);
      ಹಿಂತಿರುಗಿ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc ಕೀಲಿ;
  luaK_exp2anyregup(fs, v);
  lua_X_ಮುಂದಿನ(ls);  /* skip the dot or colon */
  codename(ls, &ಕೀಲಿ);
  lua_K_ಸೂಚ್ಯಂಕಿತ(fs, v, &ಕೀಲಿ);
}


ಸ್ಥಿರ ಶೂನ್ಯ yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  lua_X_ಮುಂದಿನ(ls);  /* skip the '[' */
  ಅಭಿವ್ಯಕ್ತಿ(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  ಪೂರ್ಣಾಂಕ nh;  /* total number of 'record' elements */
  ಪೂರ್ಣಾಂಕ na;  /* number of array elements already stored */
  ಪೂರ್ಣಾಂಕ tostore;  /* number of array elements pending to be stored */
  ಪೂರ್ಣಾಂಕ maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_CNST	(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ/2)
#ಆದರೆ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MAX_CNST
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಶೂನ್ಯ recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, ಕೀಲಿ, val;
  ಆದರೆ (ls->t.token == TK_NAME)
    codename(ls, &ಕೀಲಿ);
  ಇಲ್ಲದಿದ್ದರೆ  /* ls->t.token == '[' */
    yindex(ls, &ಕೀಲಿ);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  lua_K_ಸೂಚ್ಯಂಕಿತ(fs, &tab, &ಕೀಲಿ);
  ಅಭಿವ್ಯಕ್ತಿ(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ಸ್ಥಿರ ಶೂನ್ಯ closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ಆದರೆ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ lastlistfield (FuncState *fs, ConsControl *cc) {
  ಆದರೆ (cc->tostore == 0) ಹಿಂತಿರುಗಿ;
  ಆದರೆ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (cc->v.k != VVOID)
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


ಸ್ಥಿರ ಶೂನ್ಯ ಕ್ಷೇತ್ರ (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  ಆಯ್ಕೆ(ls->t.token) {
    ಸ್ಥಿತಿ TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ಆದರೆ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      ಇಲ್ಲದಿದ್ದರೆ
        recfield(ls, cc);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ '[': {
      recfield(ls, cc);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      listfield(ls, cc);
      ನಿಲ್ಲಿಸು;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ maxtostore (FuncState *fs) {
  ಪೂರ್ಣಾಂಕ numfreeregs = MAX_FSTACK - fs->freereg;
  ಆದರೆ (numfreeregs >= 160)  /* "lots" of registers? */
    ಹಿಂತಿರುಗಿ numfreeregs / 5;  /* use up to 1/5 of them */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (numfreeregs >= 80)  /* still "enough" registers? */
    ಹಿಂತಿರುಗಿ 10;  /* one 'SETLIST' instruction for each 10 values */
  ಇಲ್ಲದಿದ್ದರೆ  /* save registers for potential more nesting */
    ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;
  ಪೂರ್ಣಾಂಕ pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  lua_K_ಸಂಕೇತ(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  ಮಾಡು {
    ಆದರೆ (ls->t.token == /*{*/ '}') ನಿಲ್ಲಿಸು;
    ಆದರೆ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    ಕ್ಷೇತ್ರ(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ಆಗ (testnext(ls, ',') || testnext(ls, ';'));
  check_ಹೊಂದಿಕೆ(ls, /*{*/ '}', '{' /*}*/, ಸಾಲು);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ಸ್ಥಿರ ಶೂನ್ಯ setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  lua_K_ಸಂಕೇತ_ABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  ಪೂರ್ಣಾಂಕ nparams = 0;
  ಪೂರ್ಣಾಂಕ varargk = 0;
  ಆದರೆ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    ಮಾಡು {
      ಆಯ್ಕೆ (ls->t.token) {
        ಸ್ಥಿತಿ TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ TK_DOTS: {
          varargk = 1;
          lua_X_ಮುಂದಿನ(ls);  /* skip '...' */
          ಆದರೆ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          ಇಲ್ಲದಿದ್ದರೆ
            new_localvarliteral(ls, "(vararg table)");
          ನಿಲ್ಲಿಸು;
        }
        ಪೂರ್ವನಿಯೋಜಿತ: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ಆಗ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ಆದರೆ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ಸ್ಥಿರ ಶೂನ್ಯ body (LexState *ls, expdesc *e, ಪೂರ್ಣಾಂಕ ismethod, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  ಖಂಡ_Cnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = ಸಾಲು;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ಆದರೆ (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_ಹೊಂದಿಕೆ(ls, TK_ಕೊನೆ, TK_FUNCTION, ಸಾಲು);
  codeclosure(ls, e);
  close_func(ls);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  ಪೂರ್ಣಾಂಕ n = 1;  /* at least one expression */
  ಅಭಿವ್ಯಕ್ತಿ(ls, v);
  ಆಗ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    ಅಭಿವ್ಯಕ್ತಿ(ls, v);
    n++;
  }
  ಹಿಂತಿರುಗಿ n;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc ವಾದಗಳು;
  ಪೂರ್ಣಾಂಕ ತಳ, nparams;
  ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ '(': {  /* funcargs -> '(' [ explist ] ')' */
      lua_X_ಮುಂದಿನ(ls);
      ಆದರೆ (ls->t.token == ')')  /* arg list is empty? */
        ವಾದಗಳು.k = VVOID;
      ಇಲ್ಲದಿದ್ದರೆ {
        explist(ls, &ವಾದಗಳು);
        ಆದರೆ (hasmultret(ವಾದಗಳು.k))
          luaK_setmultret(fs, &ವಾದಗಳು);
      }
      check_ಹೊಂದಿಕೆ(ls, ')', '(', ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &ವಾದಗಳು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_STRING: {  /* funcargs -> STRING */
      codestring(&ವಾದಗಳು, ls->t.seminfo.ts);
      lua_X_ಮುಂದಿನ(ls);  /* must use 'seminfo' before 'next' */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      luaX_syntaxerror(ls, "function arguments expected");
      ಹಿಂತಿರುಗಿ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  ತಳ = f->u.info;  /* base register for call */
  ಆದರೆ (hasmultret(ವಾದಗಳು.k))
    nparams = LUA_MULTRET;  /* open call */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (ವಾದಗಳು.k != VVOID)
      luaK_exp2nextreg(fs, &ವಾದಗಳು);  /* close last argument */
    nparams = fs->freereg - (ತಳ+1);
  }
  init_exp(f, VCALL, lua_K_ಸಂಕೇತ_ABC(fs, OP_CALL, ತಳ, nparams+1, 2));
  luaK_fixline(fs, ಸಾಲು);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(ತಳ + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ಸ್ಥಿರ ಶೂನ್ಯ primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ '(': {
      ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;
      lua_X_ಮುಂದಿನ(ls);
      ಅಭಿವ್ಯಕ್ತಿ(ls, v);
      check_ಹೊಂದಿಕೆ(ls, ')', '(', ಸಾಲು);
      luaK_dischargevars(ls->fs, v);
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ TK_NAME: {
      singlevar(ls, v);
      ಹಿಂತಿರುಗಿ;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  ಗಾಗಿ (;;) {
    ಆಯ್ಕೆ (ls->t.token) {
      ಸ್ಥಿತಿ '.': {  /* fieldsel */
        fieldsel(ls, v);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '[': {  /* '[' exp ']' */
        expdesc ಕೀಲಿ;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &ಕೀಲಿ);
        lua_K_ಸೂಚ್ಯಂಕಿತ(fs, v, &ಕೀಲಿ);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ ':': {  /* ':' NAME funcargs */
        expdesc ಕೀಲಿ;
        lua_X_ಮುಂದಿನ(ls);
        codename(ls, &ಕೀಲಿ);
        luaK_self(fs, v, &ಕೀಲಿ);
        funcargs(ls, v);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '(': ಸ್ಥಿತಿ TK_STRING: ಸ್ಥಿತಿ '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ;
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_NIL: {
      init_exp(v, VNIL, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRUE: {
      init_exp(v, VTRUE, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_FALSE: {
      init_exp(v, VFALSE, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, lua_K_ಸಂಕೇತ_ABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ TK_FUNCTION: {
      lua_X_ಮುಂದಿನ(ls);
      body(ls, v, 0, ls->linenumber);
      ಹಿಂತಿರುಗಿ;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      suffixedexp(ls, v);
      ಹಿಂತಿರುಗಿ;
    }
  }
  lua_X_ಮುಂದಿನ(ls);
}


ಸ್ಥಿರ UnOpr getunopr (ಪೂರ್ಣಾಂಕ op) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ TK_NOT: ಹಿಂತಿರುಗಿ OPR_NOT;
    ಸ್ಥಿತಿ '-': ಹಿಂತಿರುಗಿ OPR_MINUS;
    ಸ್ಥಿತಿ '~': ಹಿಂತಿರುಗಿ OPR_BNOT;
    ಸ್ಥಿತಿ '#': ಹಿಂತಿರುಗಿ OPR_LEN;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ OPR_NOUNOPR;
  }
}


ಸ್ಥಿರ BinOpr getbinopr (ಪೂರ್ಣಾಂಕ op) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ '+': ಹಿಂತಿರುಗಿ OPR_ಕೂಡಿಸು;
    ಸ್ಥಿತಿ '-': ಹಿಂತಿರುಗಿ OPR_SUB;
    ಸ್ಥಿತಿ '*': ಹಿಂತಿರುಗಿ OPR_MUL;
    ಸ್ಥಿತಿ '%': ಹಿಂತಿರುಗಿ OPR_MOD;
    ಸ್ಥಿತಿ '^': ಹಿಂತಿರುಗಿ OPR_POW;
    ಸ್ಥಿತಿ '/': ಹಿಂತಿರುಗಿ OPR_DIV;
    ಸ್ಥಿತಿ TK_IDIV: ಹಿಂತಿರುಗಿ OPR_IDIV;
    ಸ್ಥಿತಿ '&': ಹಿಂತಿರುಗಿ OPR_BAND;
    ಸ್ಥಿತಿ '|': ಹಿಂತಿರುಗಿ OPR_BOR;
    ಸ್ಥಿತಿ '~': ಹಿಂತಿರುಗಿ OPR_BXOR;
    ಸ್ಥಿತಿ TK_SHL: ಹಿಂತಿರುಗಿ OPR_SHL;
    ಸ್ಥಿತಿ TK_SHR: ಹಿಂತಿರುಗಿ OPR_SHR;
    ಸ್ಥಿತಿ TK_CONCAT: ಹಿಂತಿರುಗಿ OPR_CONCAT;
    ಸ್ಥಿತಿ TK_NE: ಹಿಂತಿರುಗಿ OPR_NE;
    ಸ್ಥಿತಿ TK_EQ: ಹಿಂತಿರುಗಿ OPR_EQ;
    ಸ್ಥಿತಿ '<': ಹಿಂತಿರುಗಿ OPR_LT;
    ಸ್ಥಿತಿ TK_LE: ಹಿಂತಿರುಗಿ OPR_LE;
    ಸ್ಥಿತಿ '>': ಹಿಂತಿರುಗಿ OPR_GT;
    ಸ್ಥಿತಿ TK_GE: ಹಿಂತಿರುಗಿ OPR_GE;
    ಸ್ಥಿತಿ TK_AND: ಹಿಂತಿರುಗಿ OPR_AND;
    ಸ್ಥಿತಿ TK_OR: ಹಿಂತಿರುಗಿ OPR_OR;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
  lu_byte ಎಡ;  /* left priority for each binary operator */
  lu_byte ಬಲ; /* right priority */
} ಆದ್ಯತೆ[] = {  /* ORDER OPR */
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

#ವ್ಯಾಖ್ಯಾನಿಸು UNARY_ಆದ್ಯತೆ	12  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
ಸ್ಥಿರ BinOpr subexpr (LexState *ls, expdesc *v, ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ಆದರೆ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;
    lua_X_ಮುಂದಿನ(ls);  /* skip operator */
    subexpr(ls, v, UNARY_ಆದ್ಯತೆ);
    lua_K_ಪೂರ್ವಪ್ರತ್ಯಯ(ls->fs, uop, v, ಸಾಲು);
  }
  ಇಲ್ಲದಿದ್ದರೆ simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ಆಗ (op != OPR_NOBINOPR && ಆದ್ಯತೆ[op].ಎಡ > ಎಲ್ಲೆ) {
    expdesc v2;
    BinOpr nextop;
    ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;
    lua_X_ಮುಂದಿನ(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, ಆದ್ಯತೆ[op].ಬಲ);
    luaK_posfix(ls->fs, op, v, &v2, ಸಾಲು);
    op = nextop;
  }
  leavelevel(ls);
  ಹಿಂತಿರುಗಿ op;  /* return first untreated operator */
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


ಸ್ಥಿರ ಶೂನ್ಯ ಖಂಡ (LexState *ls) {
  /* block -> statlist */
  FuncState *fs = ls->fs;
  ಖಂಡ_Cnt bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
ರಚನೆ LHS_ನಿಯೋಜಿಸು {
  ರಚನೆ LHS_ನಿಯೋಜಿಸು *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_conflict (LexState *ls, ರಚನೆ LHS_ನಿಯೋಜಿಸು *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  ಪೂರ್ಣಾಂಕ conflict = 0;
  ಗಾಗಿ (; lh; lh = lh->prev) {  /* check all previous assignments */
    ಆದರೆ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ಆದರೆ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ಆದರೆ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* table is a register */
        ಆದರೆ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ಆದರೆ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ಆದರೆ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ಆದರೆ (v->k == VLOCAL)
      lua_K_ಸಂಕೇತ_ABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    ಇಲ್ಲದಿದ್ದರೆ
      lua_K_ಸಂಕೇತ_ABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
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
ಸ್ಥಿರ ಶೂನ್ಯ restassign (LexState *ls, ರಚನೆ LHS_ನಿಯೋಜಿಸು *lh, ಪೂರ್ಣಾಂಕ nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ಆದರೆ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ರಚನೆ LHS_ನಿಯೋಜಿಸು nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ಆದರೆ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* restassign -> '=' explist */
    ಪೂರ್ಣಾಂಕ nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ಆದರೆ (nexps != nvars)
      adjust_ನಿಯೋಜಿಸು(ls, nvars, nexps, &e);
    ಇಲ್ಲದಿದ್ದರೆ {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ಹಿಂತಿರುಗಿ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  ಅಭಿವ್ಯಕ್ತಿ(ls, &v);  /* read condition */
  ಆದರೆ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ಹಿಂತಿರುಗಿ v.f;
}


ಸ್ಥಿರ ಶೂನ್ಯ gotostat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, ಸಾಲು);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ಸ್ಥಿರ ಶೂನ್ಯ breakstat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಖಂಡ_Cnt *bl;  /* to look for an enclosing loop */
  ಗಾಗಿ (bl = ls->fs->bl; bl != ಖಾಲಿ; bl = bl->ಹಿಂದಿನ) {
    ಆದರೆ (bl->isloop)  /* found one? */
      ಹೋಗು ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  lua_X_ಮುಂದಿನ(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, ಸಾಲು);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ಆದರೆ (l_unlikely(lb != ಖಾಲಿ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->ಸಾಲು);  /* error */
}


ಸ್ಥಿರ ಶೂನ್ಯ labelstat (LexState *ls, TString *name, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ಆಗ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, ಸಾಲು, ಖಂಡ_follow(ls, 0));
}


ಸ್ಥಿರ ಶೂನ್ಯ whilestat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ whileinit;
  ಪೂರ್ಣಾಂಕ condexit;
  ಖಂಡ_Cnt bl;
  lua_X_ಮುಂದಿನ(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  ಖಂಡ(ls);
  luaK_jumpto(fs, whileinit);
  check_ಹೊಂದಿಕೆ(ls, TK_ಕೊನೆ, TK_WHILE, ಸಾಲು);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ಸ್ಥಿರ ಶೂನ್ಯ repeatstat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* repeatstat -> REPEAT block UNTIL cond */
  ಪೂರ್ಣಾಂಕ condexit;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ repeat_init = luaK_getlabel(fs);
  ಖಂಡ_Cnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  lua_X_ಮುಂದಿನ(ls);  /* skip REPEAT */
  statlist(ls);
  check_ಹೊಂದಿಕೆ(ls, TK_UNTIL, TK_REPEAT, ಸಾಲು);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ಆದರೆ (bl2.upval) {  /* upvalues? */
    ಪೂರ್ಣಾಂಕ ನಿರ್ಗಮಿಸು = luaK_jump(fs);  /* normal exit must jump over fix */
    luaK_patchtohere(fs, condexit);  /* repetition must close upvalues */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_CLOSE, reglevel(fs, bl2.nactvar), 0, 0);
    condexit = luaK_jump(fs);  /* repeat after closing upvalues */
    luaK_patchtohere(fs, ನಿರ್ಗಮಿಸು);  /* normal exit comes to here */
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
ಸ್ಥಿರ ಶೂನ್ಯ fixforjump (FuncState *fs, ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ dest, ಪೂರ್ಣಾಂಕ ಹಿಂಭಾಗ) {
  Instruction *jmp = &fs->f->ಸಂಕೇತ_2[pc];
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = dest - (pc + 1);
  ಆದರೆ (ಹಿಂಭಾಗ)
    ಸ್ಥಾನಾಂತರ = -ಸ್ಥಾನಾಂತರ;
  ಆದರೆ (l_unlikely(ಸ್ಥಾನಾಂತರ > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, ಸ್ಥಾನಾಂತರ);
}


/*
** Generate code for a 'for' loop.
*/
ಸ್ಥಿರ ಶೂನ್ಯ forbody (LexState *ls, ಪೂರ್ಣಾಂಕ ತಳ, ಪೂರ್ಣಾಂಕ ಸಾಲು, ಪೂರ್ಣಾಂಕ nvars, ಪೂರ್ಣಾಂಕ isgen) {
  /* forbody -> DO block */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Op_ಸಂಕೇತ forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Op_ಸಂಕೇತ forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  ಖಂಡ_Cnt bl;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ prep, endfor;
  checknext(ls, TK_DO);
  prep = lua_K_ಸಂಕೇತ_ABx(fs, forprep[isgen], ತಳ, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  ಖಂಡ(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ಆದರೆ (isgen) {  /* generic for? */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_TFORCALL, ತಳ, 0, nvars);
    luaK_fixline(fs, ಸಾಲು);
  }
  endfor = lua_K_ಸಂಕೇತ_ABx(fs, forloop[isgen], ತಳ, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, ಸಾಲು);
}


ಸ್ಥಿರ ಶೂನ್ಯ fornum (LexState *ls, TString *varname, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ ತಳ = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /*ಎಲ್ಲೆ*/
  ಆದರೆ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  ಇಲ್ಲದಿದ್ದರೆ {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, ತಳ, ಸಾಲು, 1, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  ಪೂರ್ಣಾಂಕ nvars = 4;  /* function, state, closing, control */
  ಪೂರ್ಣಾಂಕ ಸಾಲು;
  ಪೂರ್ಣಾಂಕ ತಳ = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ಆಗ (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_ಒಳ);
  ಸಾಲು = ls->linenumber;
  adjust_ನಿಯೋಜಿಸು(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, ತಳ, ಸಾಲು, nvars - 3, 1);
}


ಸ್ಥಿರ ಶೂನ್ಯ forstat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  ಖಂಡ_Cnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  lua_X_ಮುಂದಿನ(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ '=': fornum(ls, varname, ಸಾಲು); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ',': ಸ್ಥಿತಿ TK_ಒಳ: forlist(ls, varname); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_ಹೊಂದಿಕೆ(ls, TK_ಕೊನೆ, TK_FOR, ಸಾಲು);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ಸ್ಥಿರ ಶೂನ್ಯ test_then_ಖಂಡ (LexState *ls, ಪೂರ್ಣಾಂಕ *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ condtrue;
  lua_X_ಮುಂದಿನ(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  ಖಂಡ(ls);  /* 'then' part */
  ಆದರೆ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ಸ್ಥಿರ ಶೂನ್ಯ ifstat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_ಖಂಡ(ls, &escapelist);  /* IF cond THEN block */
  ಆಗ (ls->t.token == TK_ELSEIF)
    test_then_ಖಂಡ(ls, &escapelist);  /* ELSEIF cond THEN block */
  ಆದರೆ (testnext(ls, TK_ELSE))
    ಖಂಡ(ls);  /* 'else' part */
  check_ಹೊಂದಿಕೆ(ls, TK_ಕೊನೆ, TK_IF, ಸಾಲು);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ಸ್ಥಿರ ಶೂನ್ಯ localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ಸ್ಥಿರ lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ಆದರೆ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr = getstr(ts);
    checknext(ls, '>');
    ಆದರೆ (strcmp(attr, "const") == 0)
      ಹಿಂತಿರುಗಿ RDKCONST;  /* read-only variable */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(attr, "close") == 0)
      ಹಿಂತಿರುಗಿ RDKTOCLOSE;  /* to-be-closed variable */
    ಇಲ್ಲದಿದ್ದರೆ
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ಹಿಂತಿರುಗಿ df;  /* return default value */
}


ಸ್ಥಿರ ಶೂನ್ಯ checktoclose (FuncState *fs, ಪೂರ್ಣಾಂಕ level) {
  ಆದರೆ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    lua_K_ಸಂಕೇತ_ABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  ಪೂರ್ಣಾಂಕ toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  ಪೂರ್ಣಾಂಕ vidx;  /* index of last variable */
  ಪೂರ್ಣಾಂಕ nvars = 0;
  ಪೂರ್ಣಾಂಕ nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  ಮಾಡು {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte ಬಗೆ = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, ಬಗೆ);  /* predeclare it */
    ಆದರೆ (ಬಗೆ == RDKTOCLOSE) {  /* to-be-closed? */
      ಆದರೆ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ಆಗ (testnext(ls, ','));
  ಆದರೆ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  ಇಲ್ಲದಿದ್ದರೆ {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ಆದರೆ (nvars == nexps &&  /* no adjustments? */
      var->vd.ಬಗೆ == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.ಬಗೆ = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    adjust_ನಿಯೋಜಿಸು(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ಸ್ಥಿರ lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte ಬಗೆ = getvarattribute(ls, df);
  ಆಯ್ಕೆ (ಬಗೆ) {
    ಸ್ಥಿತಿ RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ಹಿಂತಿರುಗಿ ಬಗೆ;  /* to avoid warnings */
    ಸ್ಥಿತಿ RDKCONST:
      ಹಿಂತಿರುಗಿ GDKCONST;  /* adjust kind for global variable */
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ ಬಗೆ;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ checkglobal (LexState *ls, TString *varname, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  FuncState *fs = ls->fs;
  expdesc var;
  ಪೂರ್ಣಾಂಕ k;
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
ಸ್ಥಿರ ಶೂನ್ಯ initglobal (LexState *ls, ಪೂರ್ಣಾಂಕ nvars, ಪೂರ್ಣಾಂಕ firstidx, ಪೂರ್ಣಾಂಕ n,
                        ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಆದರೆ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    ಪೂರ್ಣಾಂಕ nexps = explist(ls, &e);  /* read list of expressions */
    adjust_ನಿಯೋಜಿಸು(ls, nvars, nexps, &e);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* handle variable 'n' */
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
  ಪೂರ್ಣಾಂಕ nvars = 0;
  ಪೂರ್ಣಾಂಕ lastidx;  /* index of last registered variable */
  ಮಾಡು {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte ಬಗೆ = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, ಬಗೆ);
    nvars++;
  } ಆಗ (testnext(ls, ','));
  ಆದರೆ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ಸ್ಥಿರ ಶೂನ್ಯ globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ಆದರೆ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  ಇಲ್ಲದಿದ್ದರೆ {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ಖಾಲಿ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ globalfunc (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
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


ಸ್ಥಿರ ಶೂನ್ಯ globalstatfunc (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  lua_X_ಮುಂದಿನ(ls);  /* skip 'global' */
  ಆದರೆ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, ಸಾಲು);
  ಇಲ್ಲದಿದ್ದರೆ
    globalstat(ls);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  ಪೂರ್ಣಾಂಕ ismethod = 0;
  singlevar(ls, v);
  ಆಗ (ls->t.token == '.')
    fieldsel(ls, v);
  ಆದರೆ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ಹಿಂತಿರುಗಿ ismethod;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcstat (LexState *ls, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  /* funcstat -> FUNCTION funcname body */
  ಪೂರ್ಣಾಂಕ ismethod;
  expdesc v, b;
  lua_X_ಮುಂದಿನ(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, ಸಾಲು);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, ಸಾಲು);  /* definition "happens" in the first line */
}


ಸ್ಥಿರ ಶೂನ್ಯ exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ರಚನೆ LHS_ನಿಯೋಜಿಸು v;
  suffixedexp(ls, &v.v);
  ಆದರೆ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ಖಾಲಿ;
    restassign(ls, &v, 1);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* stat -> func */
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
  ಪೂರ್ಣಾಂಕ nret;  /* number of values being returned */
  ಪೂರ್ಣಾಂಕ ಮೊದಲ = luaY_nvarstack(fs);  /* first slot to be returned */
  ಆದರೆ (ಖಂಡ_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  ಇಲ್ಲದಿದ್ದರೆ {
    nret = explist(ls, &e);  /* optional return values */
    ಆದರೆ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ಆದರೆ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        ಗಣ_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (nret == 1)  /* only one single value? */
        ಮೊದಲ = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      ಇಲ್ಲದಿದ್ದರೆ {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - ಮೊದಲ);
      }
    }
  }
  luaK_ret(fs, ಮೊದಲ, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ಸ್ಥಿರ ಶೂನ್ಯ statement (LexState *ls) {
  ಪೂರ್ಣಾಂಕ ಸಾಲು = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  ಆಯ್ಕೆ (ls->t.token) {
    ಸ್ಥಿತಿ ';': {  /* stat -> ';' (empty statement) */
      lua_X_ಮುಂದಿನ(ls);  /* skip ';' */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_IF: {  /* stat -> ifstat */
      ifstat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_DO: {  /* stat -> DO block END */
      lua_X_ಮುಂದಿನ(ls);  /* skip DO */
      ಖಂಡ(ls);
      check_ಹೊಂದಿಕೆ(ls, TK_ಕೊನೆ, TK_DO, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_FOR: {  /* stat -> forstat */
      forstat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_LOCAL: {  /* stat -> localstat */
      lua_X_ಮುಂದಿನ(ls);  /* skip LOCAL */
      ಆದರೆ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      ಇಲ್ಲದಿದ್ದರೆ
        localstat(ls);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_DBCOLON: {  /* stat -> label */
      lua_X_ಮುಂದಿನ(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_RETURN: {  /* stat -> retstat */
      lua_X_ಮುಂದಿನ(ls);  /* skip RETURN */
      retstat(ls);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_GOTO: {  /* stat -> 'goto' NAME */
      lua_X_ಮುಂದಿನ(ls);  /* skip 'goto' */
      gotostat(ls, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ LUA_COMPAT_GLOBAL
    ಸ್ಥಿತಿ TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ಆದರೆ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        ಪೂರ್ಣಾಂಕ lk = luaX_lookahead(ls);
        ಆದರೆ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, ಸಾಲು);
          ನಿಲ್ಲಿಸು;
        }
      }  /* else... */
    }
#ಆದರೆ_ಅಂತ್ಯ
    /* FALLTHROUGH */
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* stat -> func | assignment */
      exprstat(ls);
      ನಿಲ್ಲಿಸು;
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
  ಖಂಡ_Cnt bl;
  Upvaldesc *env;
  open_func(ls, fs, &bl);
  setvararg(fs);  /* main function is always vararg */
  env = allocupvalue(fs);  /* ...set environment upvalue */
  env->instack = 1;
  env->idx = 0;
  env->ಬಗೆ = VDKREG;
  env->name = ls->envn;
  luaC_objbarrier(ls->L, fs->f, env->name);
  lua_X_ಮುಂದಿನ(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, ಕೋಷ್ಟಕ *anchor, Mbuffer *buff,
                       Dyndata *dyd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ firstchar) {
  LexState lexstate;
  FuncState funcstate;
  LClosure *cl;
  lexstate.h = anchor;  /* table for scanner */
  cl = luaF_newLclosure(L, 1);  /* create main closure */
  luaD_anchorobj(L, anchor, obj2gco(cl));  /* anchor it in scanner table */
  funcstate.f = cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  funcstate.f->ಮೂಲ = luaS_new(L, name);  /* create and anchor TString */
  luaC_objbarrier(L, funcstate.f, funcstate.f->ಮೂಲ);
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, funcstate.f->ಮೂಲ, firstchar);
  mainfunc(&lexstate, &funcstate);
  lua_assert(!funcstate.prev && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  ಹಿಂತಿರುಗಿ cl;
}

