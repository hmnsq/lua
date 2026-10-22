/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lparser_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lcode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"



/* maximum number of variable declarations per function (must be
   smaller than 250, due to the bytecode format) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXVARS		200


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hasmultret(k)		((k) == VCALL || (k) == VVARARG)


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ eqstr(a,b)	((a) == (b))


/*
** nodes for block list (list of active blocks)
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਖੰਡ_Cnt {
  ਬਣਤਰ ਖੰਡ_Cnt *ਪਿਛਲਾ;  /* chain */
  ਪੂਰਨ firstlabel;  /* index of first label in this block */
  ਪੂਰਨ firstgoto;  /* index of first pending goto in this block */
  ਛੋਟਾ nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} ਖੰਡ_Cnt;



/*
** prototypes for recursive non-terminal functions
*/
ਸਥਿਰ ਖਾਲੀ statement (LexState *ls);
ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ (LexState *ls, expdesc *v);


ਸਥਿਰ l_noret error_expected (LexState *ls, ਪੂਰਨ token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ਸਥਿਰ l_noret errorlimit (FuncState *fs, ਪੂਰਨ ਹੱਦਬੰਦੀ, ਅਚਲ ਅੱਖਰ *what) {
  lua_State *L = fs->ls->L;
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
  ਪੂਰਨ ਸਤਰ = fs->f->linedefined;
  ਅਚਲ ਅੱਖਰ *where = (ਸਤਰ == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", ਸਤਰ);
  ਸੁਨੇਹਾ = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, ਹੱਦਬੰਦੀ, where);
  luaX_syntaxerror(fs->ls, ਸੁਨੇਹਾ);
}


ਖਾਲੀ luaY_checklimit (FuncState *fs, ਪੂਰਨ v, ਪੂਰਨ l, ਅਚਲ ਅੱਖਰ *what) {
  ਜੇ (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ਸਥਿਰ ਪੂਰਨ testnext (LexState *ls, ਪੂਰਨ c) {
  ਜੇ (ls->t.token == c) {
    lua_X_ਅਗਲਾ(ls);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;
}


/*
** Check that next token is 'c'.
*/
ਸਥਿਰ ਖਾਲੀ check (LexState *ls, ਪੂਰਨ c) {
  ਜੇ (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ਸਥਿਰ ਖਾਲੀ checknext (LexState *ls, ਪੂਰਨ c) {
  check(ls, c);
  lua_X_ਅਗਲਾ(ls);
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_condition(ls,c,ਸੁਨੇਹਾ)	{ ਜੇ (!(c)) luaX_syntaxerror(ls, ਸੁਨੇਹਾ); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ਸਥਿਰ ਖਾਲੀ check_ਮੇਲ (LexState *ls, ਪੂਰਨ what, ਪੂਰਨ who, ਪੂਰਨ where) {
  ਜੇ (l_unlikely(!testnext(ls, what))) {
    ਜੇ (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    ਨਹੀਂਤਾਂ {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


ਸਥਿਰ TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  lua_X_ਅਗਲਾ(ls);
  ਵਾਪਸ ts;
}


ਸਥਿਰ ਖਾਲੀ init_exp (expdesc *e, expkind k, ਪੂਰਨ i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


ਸਥਿਰ ਖਾਲੀ codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


ਸਥਿਰ ਖਾਲੀ codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
ਸਥਿਰ ਛੋਟਾ registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  ਪੂਰਨ oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ਜਦੋਂ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ਨਿਰਾ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ਵਾਪਸ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ਸਥਿਰ ਪੂਰਨ new_varkind (LexState *ls, TString *name, lu_byte ਕਿਸਮ) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.ਮਾਪ, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.ਕਿਸਮ = ਕਿਸਮ;  /* default */
  var->vd.name = name;
  ਵਾਪਸ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ਸਥਿਰ ਪੂਰਨ new_localvar (LexState *ls, TString *name) {
  ਵਾਪਸ new_varkind(ls, name, VDKREG);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (ਆਕਾਰ(v)/ਆਕਾਰ(ਅੱਖਰ)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ਸਥਿਰ Vardesc *getlocalvardesc (FuncState *fs, ਪੂਰਨ vidx) {
  ਵਾਪਸ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ਸਥਿਰ lu_byte reglevel (FuncState *fs, ਪੂਰਨ nvar) {
  ਜਦੋਂ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ਜੇ (varinreg(vd))  /* is in a register? */
      ਵਾਪਸ cast_byte(vd->vd.ridx + 1);
  }
  ਵਾਪਸ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ਵਾਪਸ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ਸਥਿਰ LocVar *localdebuginfo (FuncState *fs, ਪੂਰਨ vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ਜੇ (!varinreg(vd))
    ਵਾਪਸ ਨਿਰਾ;  /* no debug info. for constants */
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ਵਾਪਸ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ਸਥਿਰ ਖਾਲੀ init_var (FuncState *fs, expdesc *e, ਪੂਰਨ vidx) {
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
ਸਥਿਰ ਖਾਲੀ check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ਨਿਰਾ;  /* to be set if variable is const */
  ਚੋਣ (e->k) {
    ਹਾਲਤ VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VLOCAL: ਹਾਲਤ VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ਜੇ (vardesc->vd.ਕਿਸਮ != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VUPVAL: {
      Upvaldesc *ਉੱਪਰ = &fs->f->upvalues[e->u.info];
      ਜੇ (ਉੱਪਰ->ਕਿਸਮ != VDKREG)
        varname = ਉੱਪਰ->name;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    ਹਾਲਤ VINDEXUP: ਹਾਲਤ VINDEXSTR: ਹਾਲਤ VINDEXED: {  /* global variable */
      ਜੇ (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      ਰੋਕੋ;
    }
    ਮੂਲ:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ਵਾਪਸ;  /* integer index cannot be read-only */
  }
  ਜੇ (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ਸਥਿਰ ਖਾਲੀ adjustlocalvars (LexState *ls, ਪੂਰਨ nvars) {
  FuncState *fs = ls->fs;
  ਪੂਰਨ reglevel = luaY_nvarstack(fs);
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < nvars; i++) {
    ਪੂਰਨ vidx = fs->nactvar++;
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
ਸਥਿਰ ਖਾਲੀ removevars (FuncState *fs, ਪੂਰਨ tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ਜਦੋਂ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ਜੇ (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ਸਥਿਰ ਪੂਰਨ searchupvalue (FuncState *fs, TString *name) {
  ਪੂਰਨ i;
  Upvaldesc *ਉੱਪਰ = fs->f->upvalues;
  ਲਈ (i = 0; i < fs->nups; i++) {
    ਜੇ (eqstr(ਉੱਪਰ[i].name, name)) ਵਾਪਸ i;
  }
  ਵਾਪਸ -1;  /* not found */
}


ਸਥਿਰ Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  ਪੂਰਨ oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ਜਦੋਂ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ਨਿਰਾ;
  ਵਾਪਸ &f->upvalues[fs->nups++];
}


ਸਥਿਰ ਪੂਰਨ newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *ਉੱਪਰ = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ਜੇ (v->k == VLOCAL) {
    ਉੱਪਰ->instack = 1;
    ਉੱਪਰ->idx = v->u.var.ridx;
    ਉੱਪਰ->ਕਿਸਮ = getlocalvardesc(prev, v->u.var.vidx)->vd.ਕਿਸਮ;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  ਨਹੀਂਤਾਂ {
    ਉੱਪਰ->instack = 0;
    ਉੱਪਰ->idx = cast_byte(v->u.info);
    ਉੱਪਰ->ਕਿਸਮ = prev->f->upvalues[v->u.info].ਕਿਸਮ;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  ਉੱਪਰ->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ਵਾਪਸ fs->nups - 1;
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
ਸਥਿਰ ਪੂਰਨ searchvar (FuncState *fs, TString *n, expdesc *var) {
  ਪੂਰਨ i;
  ਲਈ (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ਜੇ (varglobal(vd)) {  /* global declaration? */
      ਜੇ (vd->vd.name == ਨਿਰਾ) {  /* collective declaration? */
        ਜੇ (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      ਨਹੀਂਤਾਂ {  /* global name */
        ਜੇ (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ਵਾਪਸ VGLOBAL;
        }
        ਨਹੀਂਤਾਂ ਜੇ (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (eqstr(n, vd->vd.name)) {  /* found? */
      ਜੇ (vd->vd.ਕਿਸਮ == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      ਨਹੀਂਤਾਂ {  /* local variable */
        init_var(fs, var, i);
        ਜੇ (vd->vd.ਕਿਸਮ == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ਵਾਪਸ cast_int(var->k);
    }
  }
  ਵਾਪਸ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ਸਥਿਰ ਖਾਲੀ markupval (FuncState *fs, ਪੂਰਨ level) {
  ਖੰਡ_Cnt *bl = fs->bl;
  ਜਦੋਂ (bl->nactvar > level)
    bl = bl->ਪਿਛਲਾ;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ਸਥਿਰ ਖਾਲੀ marktobeclosed (FuncState *fs) {
  ਖੰਡ_Cnt *bl = fs->bl;
  bl->upval = 1;
  bl->insidetbc = 1;
  fs->needclose = 1;
}


/*
** Find a variable with the given name 'n'. If it is an upvalue, add
** this upvalue into all intermediate functions. If it is a global, set
** 'var' as 'void' as a flag.
*/
ਸਥਿਰ ਖਾਲੀ singlevaraux (FuncState *fs, TString *n, expdesc *var, ਪੂਰਨ ਅਧਾਰ) {
  ਪੂਰਨ v = searchvar(fs, n, var);  /* look up variables at current level */
  ਜੇ (v >= 0) {  /* found? */
    ਜੇ (!ਅਧਾਰ) {
      ਜੇ (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ਜੇ (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  ਨਹੀਂਤਾਂ {  /* not found at current level; try upvalues */
    ਪੂਰਨ idx = searchupvalue(fs, n);  /* try existing upvalues */
    ਜੇ (idx < 0) {  /* not found? */
      ਜੇ (fs->prev != ਨਿਰਾ)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ਜੇ (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      ਨਹੀਂਤਾਂ  /* it is a global or a constant */
        ਵਾਪਸ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ਸਥਿਰ ਖਾਲੀ buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc ਕੁੰਜੀ;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ਜੇ (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&ਕੁੰਜੀ, varname);  /* key is variable name */
  lua_K_ਸੂਚਕਾਂਕਿਤ(fs, var, &ਕੁੰਜੀ);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ਸਥਿਰ ਖਾਲੀ buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ਜੇ (var->k == VGLOBAL) {  /* global name? */
    ਪੂਰਨ info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ਜੇ (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ਜੇ (info != -1 && ls->dyd->actvar.arr[info].vd.ਕਿਸਮ == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    ਨਹੀਂਤਾਂ  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.ਕਿਸਮ == GDKREG);
  }
}


ਸਥਿਰ ਖਾਲੀ singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ਸਥਿਰ ਖਾਲੀ adjust_ਸੌਂਪੋ (LexState *ls, ਪੂਰਨ nvars, ਪੂਰਨ nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  ਪੂਰਨ needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ਜੇ (hasmultret(e->k)) {  /* last expression has multiple returns? */
    ਪੂਰਨ extra = needed + 1;  /* discount last expression itself */
    ਜੇ (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ਜੇ (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ਜੇ (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  ਨਹੀਂਤਾਂ  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ enterlevel(ls)	luaE_incCstack(ls->L)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ਸਥਿਰ l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ਅਚਲ ਅੱਖਰ *varname = (tsname != ਨਿਰਾ) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->ਸਤਰ, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
ਸਥਿਰ ਖਾਲੀ closegoto (LexState *ls, ਪੂਰਨ g, Labeldesc *label, ਪੂਰਨ bup) {
  ਪੂਰਨ i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ਜੇ (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ਜੇ (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->ਕੋਡ[gt->pc + 1] = fs->f->ਕੋਡ[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->ਕੋਡ[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  ਲਈ (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ਸਥਿਰ Labeldesc *findlabel (LexState *ls, TString *name, ਪੂਰਨ ilb) {
  Dyndata *dyd = ls->dyd;
  ਲਈ (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ਜੇ (eqstr(lb->name, name))  /* correct label? */
      ਵਾਪਸ lb;
  }
  ਵਾਪਸ ਨਿਰਾ;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ਸਥਿਰ ਪੂਰਨ newlabelentry (LexState *ls, Labellist *l, TString *name,
                          ਪੂਰਨ ਸਤਰ, ਪੂਰਨ pc) {
  ਪੂਰਨ n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->ਮਾਪ,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].ਸਤਰ = ਸਤਰ;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ਵਾਪਸ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ਸਥਿਰ ਪੂਰਨ newgotoentry (LexState *ls, TString *name, ਪੂਰਨ ਸਤਰ) {
  FuncState *fs = ls->fs;
  ਪੂਰਨ pc = luaK_jump(fs);  /* create jump */
  lua_K_ਕੋਡ_ABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ਵਾਪਸ newlabelentry(ls, &ls->dyd->gt, name, ਸਤਰ, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ਸਥਿਰ ਖਾਲੀ createlabel (LexState *ls, TString *name, ਪੂਰਨ ਸਤਰ, ਪੂਰਨ ਆਖਰੀ) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  ਪੂਰਨ l = newlabelentry(ls, ll, name, ਸਤਰ, luaK_getlabel(fs));
  ਜੇ (ਆਖਰੀ) {  /* label is last no-op statement in the block? */
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
ਸਥਿਰ ਖਾਲੀ solvegotos (FuncState *fs, ਖੰਡ_Cnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  ਪੂਰਨ outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  ਪੂਰਨ igt = bl->firstgoto;  /* first goto in the finishing block */
  ਜਦੋਂ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ਜੇ (lb != ਨਿਰਾ)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    ਨਹੀਂਤਾਂ {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ਜੇ (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ਸਥਿਰ ਖਾਲੀ enterblock (FuncState *fs, ਖੰਡ_Cnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ਨਿਰਾ && fs->bl->insidetbc);
  bl->ਪਿਛਲਾ = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
ਸਥਿਰ l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->ਸਤਰ);
}


ਸਥਿਰ ਖਾਲੀ leaveblock (FuncState *fs) {
  ਖੰਡ_Cnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ਜੇ (bl->ਪਿਛਲਾ && bl->upval)  /* need a 'close'? */
    lua_K_ਕੋਡ_ABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ਜੇ (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ਜੇ (bl->ਪਿਛਲਾ == ਨਿਰਾ) {  /* was it the last block? */
    ਜੇ (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->ਪਿਛਲਾ;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ਸਥਿਰ Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ਜੇ (fs->np >= f->sizep) {
    ਪੂਰਨ oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ਜਦੋਂ (oldsize < f->sizep)
      f->p[oldsize++] = ਨਿਰਾ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ਵਾਪਸ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ਸਥਿਰ ਖਾਲੀ codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, lua_K_ਕੋਡ_ABx(fs, OP_ਘੇਰਾ, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ਸਥਿਰ ਖਾਲੀ open_func (LexState *ls, FuncState *fs, ਖੰਡ_Cnt *bl) {
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
  fs->bl = ਨਿਰਾ;
  f->ਸਰੋਤ = ls->ਸਰੋਤ;
  luaC_objbarrier(L, f, f->ਸਰੋਤ);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ਸਥਿਰ ਖਾਲੀ close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue ਆਰਜ਼ੀ;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ਨਿਰਾ);
  lua_K_ਅੰਤ(fs);
  luaM_shrinkvector(L, f->ਕੋਡ, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_ਸਤਰ_Info);
  luaM_shrinkvector(L, f->k, f->sizek, fs->nk, TValue);
  luaM_shrinkvector(L, f->p, f->sizep, fs->np, Proto *);
  luaM_shrinkvector(L, f->locvars, f->sizelocvars, fs->ndebugvars, LocVar);
  luaM_shrinkvector(L, f->upvalues, f->sizeupvalues, fs->nups, Upvaldesc);
  /* remove kcache table from scanner table ("weigh" its anchor) */
  sethvalue(L, &ਆਰਜ਼ੀ, fs->kcache);  /* key to be set to nil */
  lua_H_ਸੈੱਟ(L, ls->h, &ਆਰਜ਼ੀ, &G(L)->nilvalue);
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
ਸਥਿਰ ਪੂਰਨ ਖੰਡ_follow (LexState *ls, ਪੂਰਨ withuntil) {
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ TK_ELSE: ਹਾਲਤ TK_ELSEIF:
    ਹਾਲਤ TK_ਸਿਰਾ: ਹਾਲਤ TK_EOS:
      ਵਾਪਸ 1;
    ਹਾਲਤ TK_UNTIL: ਵਾਪਸ withuntil;
    ਮੂਲ: ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਖਾਲੀ statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ਜਦੋਂ (!ਖੰਡ_follow(ls, 1)) {
    ਜੇ (ls->t.token == TK_RETURN) {
      statement(ls);
      ਵਾਪਸ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ਸਥਿਰ ਖਾਲੀ fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc ਕੁੰਜੀ;
  luaK_exp2anyregup(fs, v);
  lua_X_ਅਗਲਾ(ls);  /* skip the dot or colon */
  codename(ls, &ਕੁੰਜੀ);
  lua_K_ਸੂਚਕਾਂਕਿਤ(fs, v, &ਕੁੰਜੀ);
}


ਸਥਿਰ ਖਾਲੀ yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  lua_X_ਅਗਲਾ(ls);  /* skip the '[' */
  ਵਿਅੰਜਕ(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  ਪੂਰਨ nh;  /* total number of 'record' elements */
  ਪੂਰਨ na;  /* number of array elements already stored */
  ਪੂਰਨ tostore;  /* number of array elements pending to be stored */
  ਪੂਰਨ maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_CNST	(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ/2)
#ਜੇ MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MAX_CNST
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਖਾਲੀ recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, ਕੁੰਜੀ, val;
  ਜੇ (ls->t.token == TK_NAME)
    codename(ls, &ਕੁੰਜੀ);
  ਨਹੀਂਤਾਂ  /* ls->t.token == '[' */
    yindex(ls, &ਕੁੰਜੀ);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  lua_K_ਸੂਚਕਾਂਕਿਤ(fs, &tab, &ਕੁੰਜੀ);
  ਵਿਅੰਜਕ(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ਸਥਿਰ ਖਾਲੀ closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ਜੇ (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ਸਥਿਰ ਖਾਲੀ lastlistfield (FuncState *fs, ConsControl *cc) {
  ਜੇ (cc->tostore == 0) ਵਾਪਸ;
  ਜੇ (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


ਸਥਿਰ ਖਾਲੀ listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  ਵਿਅੰਜਕ(ls, &cc->v);
  cc->tostore++;
}


ਸਥਿਰ ਖਾਲੀ ਖੇਤਰ (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  ਚੋਣ(ls->t.token) {
    ਹਾਲਤ TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ਜੇ (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      ਨਹੀਂਤਾਂ
        recfield(ls, cc);
      ਰੋਕੋ;
    }
    ਹਾਲਤ '[': {
      recfield(ls, cc);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      listfield(ls, cc);
      ਰੋਕੋ;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ਸਥਿਰ ਪੂਰਨ maxtostore (FuncState *fs) {
  ਪੂਰਨ numfreeregs = MAX_FSTACK - fs->freereg;
  ਜੇ (numfreeregs >= 160)  /* "lots" of registers? */
    ਵਾਪਸ numfreeregs / 5;  /* use up to 1/5 of them */
  ਨਹੀਂਤਾਂ ਜੇ (numfreeregs >= 80)  /* still "enough" registers? */
    ਵਾਪਸ 10;  /* one 'SETLIST' instruction for each 10 values */
  ਨਹੀਂਤਾਂ  /* save registers for potential more nesting */
    ਵਾਪਸ 1;
}


ਸਥਿਰ ਖਾਲੀ constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  ਪੂਰਨ ਸਤਰ = ls->linenumber;
  ਪੂਰਨ pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  lua_K_ਕੋਡ(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  ਕਰੋ {
    ਜੇ (ls->t.token == /*{*/ '}') ਰੋਕੋ;
    ਜੇ (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    ਖੇਤਰ(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ਜਦੋਂ (testnext(ls, ',') || testnext(ls, ';'));
  check_ਮੇਲ(ls, /*{*/ '}', '{' /*}*/, ਸਤਰ);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ਸਥਿਰ ਖਾਲੀ setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  lua_K_ਕੋਡ_ABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ਸਥਿਰ ਖਾਲੀ parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  ਪੂਰਨ nparams = 0;
  ਪੂਰਨ varargk = 0;
  ਜੇ (ls->t.token != ')') {  /* is 'parlist' not empty? */
    ਕਰੋ {
      ਚੋਣ (ls->t.token) {
        ਹਾਲਤ TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          ਰੋਕੋ;
        }
        ਹਾਲਤ TK_DOTS: {
          varargk = 1;
          lua_X_ਅਗਲਾ(ls);  /* skip '...' */
          ਜੇ (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          ਨਹੀਂਤਾਂ
            new_localvarliteral(ls, "(vararg table)");
          ਰੋਕੋ;
        }
        ਮੂਲ: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ਜਦੋਂ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ਜੇ (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ਸਥਿਰ ਖਾਲੀ body (LexState *ls, expdesc *e, ਪੂਰਨ ismethod, ਪੂਰਨ ਸਤਰ) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  ਖੰਡ_Cnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = ਸਤਰ;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ਜੇ (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_ਮੇਲ(ls, TK_ਸਿਰਾ, TK_FUNCTION, ਸਤਰ);
  codeclosure(ls, e);
  close_func(ls);
}


ਸਥਿਰ ਪੂਰਨ explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  ਪੂਰਨ n = 1;  /* at least one expression */
  ਵਿਅੰਜਕ(ls, v);
  ਜਦੋਂ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    ਵਿਅੰਜਕ(ls, v);
    n++;
  }
  ਵਾਪਸ n;
}


ਸਥਿਰ ਖਾਲੀ funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc ਦਲੀਲਾਂ;
  ਪੂਰਨ ਅਧਾਰ, nparams;
  ਪੂਰਨ ਸਤਰ = ls->linenumber;
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ '(': {  /* funcargs -> '(' [ explist ] ')' */
      lua_X_ਅਗਲਾ(ls);
      ਜੇ (ls->t.token == ')')  /* arg list is empty? */
        ਦਲੀਲਾਂ.k = VVOID;
      ਨਹੀਂਤਾਂ {
        explist(ls, &ਦਲੀਲਾਂ);
        ਜੇ (hasmultret(ਦਲੀਲਾਂ.k))
          luaK_setmultret(fs, &ਦਲੀਲਾਂ);
      }
      check_ਮੇਲ(ls, ')', '(', ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &ਦਲੀਲਾਂ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_STRING: {  /* funcargs -> STRING */
      codestring(&ਦਲੀਲਾਂ, ls->t.seminfo.ts);
      lua_X_ਅਗਲਾ(ls);  /* must use 'seminfo' before 'next' */
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      luaX_syntaxerror(ls, "function arguments expected");
      ਵਾਪਸ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  ਅਧਾਰ = f->u.info;  /* base register for call */
  ਜੇ (hasmultret(ਦਲੀਲਾਂ.k))
    nparams = LUA_MULTRET;  /* open call */
  ਨਹੀਂਤਾਂ {
    ਜੇ (ਦਲੀਲਾਂ.k != VVOID)
      luaK_exp2nextreg(fs, &ਦਲੀਲਾਂ);  /* close last argument */
    nparams = fs->freereg - (ਅਧਾਰ+1);
  }
  init_exp(f, VCALL, lua_K_ਕੋਡ_ABC(fs, OP_CALL, ਅਧਾਰ, nparams+1, 2));
  luaK_fixline(fs, ਸਤਰ);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(ਅਧਾਰ + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ਸਥਿਰ ਖਾਲੀ primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ '(': {
      ਪੂਰਨ ਸਤਰ = ls->linenumber;
      lua_X_ਅਗਲਾ(ls);
      ਵਿਅੰਜਕ(ls, v);
      check_ਮੇਲ(ls, ')', '(', ਸਤਰ);
      luaK_dischargevars(ls->fs, v);
      ਵਾਪਸ;
    }
    ਹਾਲਤ TK_NAME: {
      singlevar(ls, v);
      ਵਾਪਸ;
    }
    ਮੂਲ: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ਸਥਿਰ ਖਾਲੀ suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  ਲਈ (;;) {
    ਚੋਣ (ls->t.token) {
      ਹਾਲਤ '.': {  /* fieldsel */
        fieldsel(ls, v);
        ਰੋਕੋ;
      }
      ਹਾਲਤ '[': {  /* '[' exp ']' */
        expdesc ਕੁੰਜੀ;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &ਕੁੰਜੀ);
        lua_K_ਸੂਚਕਾਂਕਿਤ(fs, v, &ਕੁੰਜੀ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ ':': {  /* ':' NAME funcargs */
        expdesc ਕੁੰਜੀ;
        lua_X_ਅਗਲਾ(ls);
        codename(ls, &ਕੁੰਜੀ);
        luaK_self(fs, v, &ਕੁੰਜੀ);
        funcargs(ls, v);
        ਰੋਕੋ;
      }
      ਹਾਲਤ '(': ਹਾਲਤ TK_STRING: ਹਾਲਤ '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        ਰੋਕੋ;
      }
      ਮੂਲ: ਵਾਪਸ;
    }
  }
}


ਸਥਿਰ ਖਾਲੀ simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_NIL: {
      init_exp(v, VNIL, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRUE: {
      init_exp(v, VTRUE, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_FALSE: {
      init_exp(v, VFALSE, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, lua_K_ਕੋਡ_ABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      ਰੋਕੋ;
    }
    ਹਾਲਤ '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ਵਾਪਸ;
    }
    ਹਾਲਤ TK_FUNCTION: {
      lua_X_ਅਗਲਾ(ls);
      body(ls, v, 0, ls->linenumber);
      ਵਾਪਸ;
    }
    ਮੂਲ: {
      suffixedexp(ls, v);
      ਵਾਪਸ;
    }
  }
  lua_X_ਅਗਲਾ(ls);
}


ਸਥਿਰ UnOpr getunopr (ਪੂਰਨ op) {
  ਚੋਣ (op) {
    ਹਾਲਤ TK_NOT: ਵਾਪਸ OPR_NOT;
    ਹਾਲਤ '-': ਵਾਪਸ OPR_MINUS;
    ਹਾਲਤ '~': ਵਾਪਸ OPR_BNOT;
    ਹਾਲਤ '#': ਵਾਪਸ OPR_LEN;
    ਮੂਲ: ਵਾਪਸ OPR_NOUNOPR;
  }
}


ਸਥਿਰ BinOpr getbinopr (ਪੂਰਨ op) {
  ਚੋਣ (op) {
    ਹਾਲਤ '+': ਵਾਪਸ OPR_ਜੋੜੋ;
    ਹਾਲਤ '-': ਵਾਪਸ OPR_SUB;
    ਹਾਲਤ '*': ਵਾਪਸ OPR_MUL;
    ਹਾਲਤ '%': ਵਾਪਸ OPR_MOD;
    ਹਾਲਤ '^': ਵਾਪਸ OPR_POW;
    ਹਾਲਤ '/': ਵਾਪਸ OPR_DIV;
    ਹਾਲਤ TK_IDIV: ਵਾਪਸ OPR_IDIV;
    ਹਾਲਤ '&': ਵਾਪਸ OPR_BAND;
    ਹਾਲਤ '|': ਵਾਪਸ OPR_BOR;
    ਹਾਲਤ '~': ਵਾਪਸ OPR_BXOR;
    ਹਾਲਤ TK_SHL: ਵਾਪਸ OPR_SHL;
    ਹਾਲਤ TK_SHR: ਵਾਪਸ OPR_SHR;
    ਹਾਲਤ TK_CONCAT: ਵਾਪਸ OPR_CONCAT;
    ਹਾਲਤ TK_NE: ਵਾਪਸ OPR_NE;
    ਹਾਲਤ TK_EQ: ਵਾਪਸ OPR_EQ;
    ਹਾਲਤ '<': ਵਾਪਸ OPR_LT;
    ਹਾਲਤ TK_LE: ਵਾਪਸ OPR_LE;
    ਹਾਲਤ '>': ਵਾਪਸ OPR_GT;
    ਹਾਲਤ TK_GE: ਵਾਪਸ OPR_GE;
    ਹਾਲਤ TK_AND: ਵਾਪਸ OPR_AND;
    ਹਾਲਤ TK_OR: ਵਾਪਸ OPR_OR;
    ਮੂਲ: ਵਾਪਸ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
  lu_byte ਖੱਬਾ;  /* left priority for each binary operator */
  lu_byte ਸੱਜਾ; /* right priority */
} ਤਰਜੀਹ[] = {  /* ORDER OPR */
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNARY_ਤਰਜੀਹ	12  /* priority for unary operators */


/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
ਸਥਿਰ BinOpr subexpr (LexState *ls, expdesc *v, ਪੂਰਨ ਹੱਦਬੰਦੀ) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ਜੇ (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    ਪੂਰਨ ਸਤਰ = ls->linenumber;
    lua_X_ਅਗਲਾ(ls);  /* skip operator */
    subexpr(ls, v, UNARY_ਤਰਜੀਹ);
    lua_K_ਅਗੇਤਰ(ls->fs, uop, v, ਸਤਰ);
  }
  ਨਹੀਂਤਾਂ simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ਜਦੋਂ (op != OPR_NOBINOPR && ਤਰਜੀਹ[op].ਖੱਬਾ > ਹੱਦਬੰਦੀ) {
    expdesc v2;
    BinOpr nextop;
    ਪੂਰਨ ਸਤਰ = ls->linenumber;
    lua_X_ਅਗਲਾ(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, ਤਰਜੀਹ[op].ਸੱਜਾ);
    luaK_posfix(ls->fs, op, v, &v2, ਸਤਰ);
    op = nextop;
  }
  leavelevel(ls);
  ਵਾਪਸ op;  /* return first untreated operator */
}


ਸਥਿਰ ਖਾਲੀ ਵਿਅੰਜਕ (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


ਸਥਿਰ ਖਾਲੀ ਖੰਡ (LexState *ls) {
  /* block -> statlist */
  FuncState *fs = ls->fs;
  ਖੰਡ_Cnt bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** assignment
*/
ਬਣਤਰ LHS_ਸੌਂਪੋ {
  ਬਣਤਰ LHS_ਸੌਂਪੋ *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ਸਥਿਰ ਖਾਲੀ check_conflict (LexState *ls, ਬਣਤਰ LHS_ਸੌਂਪੋ *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  ਪੂਰਨ conflict = 0;
  ਲਈ (; lh; lh = lh->prev) {  /* check all previous assignments */
    ਜੇ (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ਜੇ (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ਜੇ (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      ਨਹੀਂਤਾਂ {  /* table is a register */
        ਜੇ (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ਜੇ (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ਜੇ (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ਜੇ (v->k == VLOCAL)
      lua_K_ਕੋਡ_ABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    ਨਹੀਂਤਾਂ
      lua_K_ਕੋਡ_ABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
ਸਥਿਰ ਖਾਲੀ storevartop (FuncState *fs, expdesc *var) {
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
ਸਥਿਰ ਖਾਲੀ restassign (LexState *ls, ਬਣਤਰ LHS_ਸੌਂਪੋ *lh, ਪੂਰਨ nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ਜੇ (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ਬਣਤਰ LHS_ਸੌਂਪੋ nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ਜੇ (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  ਨਹੀਂਤਾਂ {  /* restassign -> '=' explist */
    ਪੂਰਨ nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ਜੇ (nexps != nvars)
      adjust_ਸੌਂਪੋ(ls, nvars, nexps, &e);
    ਨਹੀਂਤਾਂ {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ਵਾਪਸ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ਸਥਿਰ ਪੂਰਨ cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  ਵਿਅੰਜਕ(ls, &v);  /* read condition */
  ਜੇ (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ਵਾਪਸ v.f;
}


ਸਥਿਰ ਖਾਲੀ gotostat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, ਸਤਰ);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ਸਥਿਰ ਖਾਲੀ breakstat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  ਖੰਡ_Cnt *bl;  /* to look for an enclosing loop */
  ਲਈ (bl = ls->fs->bl; bl != ਨਿਰਾ; bl = bl->ਪਿਛਲਾ) {
    ਜੇ (bl->isloop)  /* found one? */
      ਜਾਓ ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  lua_X_ਅਗਲਾ(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, ਸਤਰ);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ਸਥਿਰ ਖਾਲੀ checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ਜੇ (l_unlikely(lb != ਨਿਰਾ))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->ਸਤਰ);  /* error */
}


ਸਥਿਰ ਖਾਲੀ labelstat (LexState *ls, TString *name, ਪੂਰਨ ਸਤਰ) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ਜਦੋਂ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, ਸਤਰ, ਖੰਡ_follow(ls, 0));
}


ਸਥਿਰ ਖਾਲੀ whilestat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  ਪੂਰਨ whileinit;
  ਪੂਰਨ condexit;
  ਖੰਡ_Cnt bl;
  lua_X_ਅਗਲਾ(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  ਖੰਡ(ls);
  luaK_jumpto(fs, whileinit);
  check_ਮੇਲ(ls, TK_ਸਿਰਾ, TK_WHILE, ਸਤਰ);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ਸਥਿਰ ਖਾਲੀ repeatstat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* repeatstat -> REPEAT block UNTIL cond */
  ਪੂਰਨ condexit;
  FuncState *fs = ls->fs;
  ਪੂਰਨ repeat_init = luaK_getlabel(fs);
  ਖੰਡ_Cnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  lua_X_ਅਗਲਾ(ls);  /* skip REPEAT */
  statlist(ls);
  check_ਮੇਲ(ls, TK_UNTIL, TK_REPEAT, ਸਤਰ);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ਜੇ (bl2.upval) {  /* upvalues? */
    ਪੂਰਨ ਬਾਹਰ_ਨਿਕਲੋ = luaK_jump(fs);  /* normal exit must jump over fix */
    luaK_patchtohere(fs, condexit);  /* repetition must close upvalues */
    lua_K_ਕੋਡ_ABC(fs, OP_CLOSE, reglevel(fs, bl2.nactvar), 0, 0);
    condexit = luaK_jump(fs);  /* repeat after closing upvalues */
    luaK_patchtohere(fs, ਬਾਹਰ_ਨਿਕਲੋ);  /* normal exit comes to here */
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
ਸਥਿਰ ਖਾਲੀ exp1 (LexState *ls) {
  expdesc e;
  ਵਿਅੰਜਕ(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
ਸਥਿਰ ਖਾਲੀ fixforjump (FuncState *fs, ਪੂਰਨ pc, ਪੂਰਨ dest, ਪੂਰਨ ਪਿੱਠ) {
  Instruction *jmp = &fs->f->ਕੋਡ[pc];
  ਪੂਰਨ ਵਿੱਥ = dest - (pc + 1);
  ਜੇ (ਪਿੱਠ)
    ਵਿੱਥ = -ਵਿੱਥ;
  ਜੇ (l_unlikely(ਵਿੱਥ > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, ਵਿੱਥ);
}


/*
** Generate code for a 'for' loop.
*/
ਸਥਿਰ ਖਾਲੀ forbody (LexState *ls, ਪੂਰਨ ਅਧਾਰ, ਪੂਰਨ ਸਤਰ, ਪੂਰਨ nvars, ਪੂਰਨ isgen) {
  /* forbody -> DO block */
  ਸਥਿਰ ਅਚਲ Op_ਕੋਡ forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ਸਥਿਰ ਅਚਲ Op_ਕੋਡ forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  ਖੰਡ_Cnt bl;
  FuncState *fs = ls->fs;
  ਪੂਰਨ prep, endfor;
  checknext(ls, TK_DO);
  prep = lua_K_ਕੋਡ_ABx(fs, forprep[isgen], ਅਧਾਰ, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  ਖੰਡ(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ਜੇ (isgen) {  /* generic for? */
    lua_K_ਕੋਡ_ABC(fs, OP_TFORCALL, ਅਧਾਰ, 0, nvars);
    luaK_fixline(fs, ਸਤਰ);
  }
  endfor = lua_K_ਕੋਡ_ABx(fs, forloop[isgen], ਅਧਾਰ, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, ਸਤਰ);
}


ਸਥਿਰ ਖਾਲੀ fornum (LexState *ls, TString *varname, ਪੂਰਨ ਸਤਰ) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  ਪੂਰਨ ਅਧਾਰ = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /*ਹੱਦਬੰਦੀ*/
  ਜੇ (testnext(ls, ','))
    exp1(ls);  /* optional step */
  ਨਹੀਂਤਾਂ {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, ਅਧਾਰ, ਸਤਰ, 1, 0);
}


ਸਥਿਰ ਖਾਲੀ forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  ਪੂਰਨ nvars = 4;  /* function, state, closing, control */
  ਪੂਰਨ ਸਤਰ;
  ਪੂਰਨ ਅਧਾਰ = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ਜਦੋਂ (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_ਵਿੱਚ);
  ਸਤਰ = ls->linenumber;
  adjust_ਸੌਂਪੋ(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, ਅਧਾਰ, ਸਤਰ, nvars - 3, 1);
}


ਸਥਿਰ ਖਾਲੀ forstat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  ਖੰਡ_Cnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  lua_X_ਅਗਲਾ(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ '=': fornum(ls, varname, ਸਤਰ); ਰੋਕੋ;
    ਹਾਲਤ ',': ਹਾਲਤ TK_ਵਿੱਚ: forlist(ls, varname); ਰੋਕੋ;
    ਮੂਲ: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_ਮੇਲ(ls, TK_ਸਿਰਾ, TK_FOR, ਸਤਰ);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ਸਥਿਰ ਖਾਲੀ test_then_ਖੰਡ (LexState *ls, ਪੂਰਨ *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  ਪੂਰਨ condtrue;
  lua_X_ਅਗਲਾ(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  ਖੰਡ(ls);  /* 'then' part */
  ਜੇ (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ਸਥਿਰ ਖਾਲੀ ifstat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  ਪੂਰਨ escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_ਖੰਡ(ls, &escapelist);  /* IF cond THEN block */
  ਜਦੋਂ (ls->t.token == TK_ELSEIF)
    test_then_ਖੰਡ(ls, &escapelist);  /* ELSEIF cond THEN block */
  ਜੇ (testnext(ls, TK_ELSE))
    ਖੰਡ(ls);  /* 'else' part */
  check_ਮੇਲ(ls, TK_ਸਿਰਾ, TK_IF, ਸਤਰ);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ਸਥਿਰ ਖਾਲੀ localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  ਪੂਰਨ fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ਸਥਿਰ lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ਜੇ (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ਅਚਲ ਅੱਖਰ *attr = getstr(ts);
    checknext(ls, '>');
    ਜੇ (strcmp(attr, "const") == 0)
      ਵਾਪਸ RDKCONST;  /* read-only variable */
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(attr, "close") == 0)
      ਵਾਪਸ RDKTOCLOSE;  /* to-be-closed variable */
    ਨਹੀਂਤਾਂ
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ਵਾਪਸ df;  /* return default value */
}


ਸਥਿਰ ਖਾਲੀ checktoclose (FuncState *fs, ਪੂਰਨ level) {
  ਜੇ (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    lua_K_ਕੋਡ_ABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ਸਥਿਰ ਖਾਲੀ localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  ਪੂਰਨ toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  ਪੂਰਨ vidx;  /* index of last variable */
  ਪੂਰਨ nvars = 0;
  ਪੂਰਨ nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  ਕਰੋ {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte ਕਿਸਮ = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, ਕਿਸਮ);  /* predeclare it */
    ਜੇ (ਕਿਸਮ == RDKTOCLOSE) {  /* to-be-closed? */
      ਜੇ (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ਜਦੋਂ (testnext(ls, ','));
  ਜੇ (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  ਨਹੀਂਤਾਂ {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ਜੇ (nvars == nexps &&  /* no adjustments? */
      var->vd.ਕਿਸਮ == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.ਕਿਸਮ = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  ਨਹੀਂਤਾਂ {
    adjust_ਸੌਂਪੋ(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ਸਥਿਰ lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte ਕਿਸਮ = getvarattribute(ls, df);
  ਚੋਣ (ਕਿਸਮ) {
    ਹਾਲਤ RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ਵਾਪਸ ਕਿਸਮ;  /* to avoid warnings */
    ਹਾਲਤ RDKCONST:
      ਵਾਪਸ GDKCONST;  /* adjust kind for global variable */
    ਮੂਲ:
      ਵਾਪਸ ਕਿਸਮ;
  }
}


ਸਥਿਰ ਖਾਲੀ checkglobal (LexState *ls, TString *varname, ਪੂਰਨ ਸਤਰ) {
  FuncState *fs = ls->fs;
  expdesc var;
  ਪੂਰਨ k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, ਸਤਰ);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
ਸਥਿਰ ਖਾਲੀ initglobal (LexState *ls, ਪੂਰਨ nvars, ਪੂਰਨ firstidx, ਪੂਰਨ n,
                        ਪੂਰਨ ਸਤਰ) {
  ਜੇ (n == nvars) {  /* traversed all variables? */
    expdesc e;
    ਪੂਰਨ nexps = explist(ls, &e);  /* read list of expressions */
    adjust_ਸੌਂਪੋ(ls, nvars, nexps, &e);
  }
  ਨਹੀਂਤਾਂ {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, ਸਤਰ);
    leavelevel(ls);
    checkglobal(ls, varname, ਸਤਰ);
    storevartop(fs, &var);
  }
}


ਸਥਿਰ ਖਾਲੀ globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  ਪੂਰਨ nvars = 0;
  ਪੂਰਨ lastidx;  /* index of last registered variable */
  ਕਰੋ {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte ਕਿਸਮ = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, ਕਿਸਮ);
    nvars++;
  } ਜਦੋਂ (testnext(ls, ','));
  ਜੇ (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ਸਥਿਰ ਖਾਲੀ globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ਜੇ (!testnext(ls, '*'))
    globalnames(ls, defkind);
  ਨਹੀਂਤਾਂ {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ਨਿਰਾ, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ਸਥਿਰ ਖਾਲੀ globalfunc (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, ਸਤਰ);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, ਸਤਰ);  /* definition "happens" in the first line */
}


ਸਥਿਰ ਖਾਲੀ globalstatfunc (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  lua_X_ਅਗਲਾ(ls);  /* skip 'global' */
  ਜੇ (testnext(ls, TK_FUNCTION))
    globalfunc(ls, ਸਤਰ);
  ਨਹੀਂਤਾਂ
    globalstat(ls);
}


ਸਥਿਰ ਪੂਰਨ funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  ਪੂਰਨ ismethod = 0;
  singlevar(ls, v);
  ਜਦੋਂ (ls->t.token == '.')
    fieldsel(ls, v);
  ਜੇ (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ਵਾਪਸ ismethod;
}


ਸਥਿਰ ਖਾਲੀ funcstat (LexState *ls, ਪੂਰਨ ਸਤਰ) {
  /* funcstat -> FUNCTION funcname body */
  ਪੂਰਨ ismethod;
  expdesc v, b;
  lua_X_ਅਗਲਾ(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, ਸਤਰ);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, ਸਤਰ);  /* definition "happens" in the first line */
}


ਸਥਿਰ ਖਾਲੀ exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ਬਣਤਰ LHS_ਸੌਂਪੋ v;
  suffixedexp(ls, &v.v);
  ਜੇ (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ਨਿਰਾ;
    restassign(ls, &v, 1);
  }
  ਨਹੀਂਤਾਂ {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


ਸਥਿਰ ਖਾਲੀ retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  ਪੂਰਨ nret;  /* number of values being returned */
  ਪੂਰਨ ਪਹਿਲਾ = luaY_nvarstack(fs);  /* first slot to be returned */
  ਜੇ (ਖੰਡ_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  ਨਹੀਂਤਾਂ {
    nret = explist(ls, &e);  /* optional return values */
    ਜੇ (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ਜੇ (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        ਸੈੱਟ_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    ਨਹੀਂਤਾਂ {
      ਜੇ (nret == 1)  /* only one single value? */
        ਪਹਿਲਾ = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      ਨਹੀਂਤਾਂ {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - ਪਹਿਲਾ);
      }
    }
  }
  luaK_ret(fs, ਪਹਿਲਾ, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ਸਥਿਰ ਖਾਲੀ statement (LexState *ls) {
  ਪੂਰਨ ਸਤਰ = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  ਚੋਣ (ls->t.token) {
    ਹਾਲਤ ';': {  /* stat -> ';' (empty statement) */
      lua_X_ਅਗਲਾ(ls);  /* skip ';' */
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_IF: {  /* stat -> ifstat */
      ifstat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_DO: {  /* stat -> DO block END */
      lua_X_ਅਗਲਾ(ls);  /* skip DO */
      ਖੰਡ(ls);
      check_ਮੇਲ(ls, TK_ਸਿਰਾ, TK_DO, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_FOR: {  /* stat -> forstat */
      forstat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_LOCAL: {  /* stat -> localstat */
      lua_X_ਅਗਲਾ(ls);  /* skip LOCAL */
      ਜੇ (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      ਨਹੀਂਤਾਂ
        localstat(ls);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_DBCOLON: {  /* stat -> label */
      lua_X_ਅਗਲਾ(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_RETURN: {  /* stat -> retstat */
      lua_X_ਅਗਲਾ(ls);  /* skip RETURN */
      retstat(ls);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_GOTO: {  /* stat -> 'goto' NAME */
      lua_X_ਅਗਲਾ(ls);  /* skip 'goto' */
      gotostat(ls, ਸਤਰ);
      ਰੋਕੋ;
    }
#ਜੇ LUA_COMPAT_GLOBAL
    ਹਾਲਤ TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ਜੇ (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        ਪੂਰਨ lk = luaX_lookahead(ls);
        ਜੇ (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, ਸਤਰ);
          ਰੋਕੋ;
        }
      }  /* else... */
    }
#ਜੇ_ਅੰਤ
    /* FALLTHROUGH */
    ਮੂਲ: {  /* stat -> func | assignment */
      exprstat(ls);
      ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ mainfunc (LexState *ls, FuncState *fs) {
  ਖੰਡ_Cnt bl;
  Upvaldesc *env;
  open_func(ls, fs, &bl);
  setvararg(fs);  /* main function is always vararg */
  env = allocupvalue(fs);  /* ...set environment upvalue */
  env->instack = 1;
  env->idx = 0;
  env->ਕਿਸਮ = VDKREG;
  env->name = ls->envn;
  luaC_objbarrier(ls->L, fs->f, env->name);
  lua_X_ਅਗਲਾ(ls);  /* read first token */
  statlist(ls);  /* parse main body */
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, ਸਾਰਣੀ *anchor, Mbuffer *buff,
                       Dyndata *dyd, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ firstchar) {
  LexState lexstate;
  FuncState funcstate;
  LClosure *cl;
  lexstate.h = anchor;  /* table for scanner */
  cl = luaF_newLclosure(L, 1);  /* create main closure */
  luaD_anchorobj(L, anchor, obj2gco(cl));  /* anchor it in scanner table */
  funcstate.f = cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  funcstate.f->ਸਰੋਤ = luaS_new(L, name);  /* create and anchor TString */
  luaC_objbarrier(L, funcstate.f, funcstate.f->ਸਰੋਤ);
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, funcstate.f->ਸਰੋਤ, firstchar);
  mainfunc(&lexstate, &funcstate);
  lua_assert(!funcstate.prev && funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  ਵਾਪਸ cl;
}

