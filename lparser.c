/*
** $Id: lparser.c $
** Lua Parser
** See Copyright Notice in lua.h
*/

#define lparser_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lcode.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "llex.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lopcodes.h"
#ଅନ୍ତର୍ଭାଵ "lparser.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"



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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା BlockCnt {
  ସଂରଚନା BlockCnt *previous;  /* chain */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstlabel;  /* index of first label in this block */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstgoto;  /* index of first pending goto in this block */
  short nactvar;  /* number of active declarations at block entry */
  lu_byte upval;  /* true if some variable in the block is an upvalue */
  lu_byte isloop;  /* 1 if 'block' is a loop; 2 if it has pending breaks */
  lu_byte insidetbc;  /* true if inside the scope of a to-be-closed var. */
} BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
ସ୍ଥିର ଶୂନ୍ୟ statement (LexState *ls);
ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି (LexState *ls, expdesc *v);


ସ୍ଥିର l_noret error_expected (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


ସ୍ଥିର l_noret errorlimit (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit, ଅଚଳ ଅକ୍ଷର *what) {
  lua_State *L = fs->ls->L;
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = fs->f->linedefined;
  ଅଚଳ ଅକ୍ଷର *where = (ଧାଡ଼ି == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", ଧାଡ଼ି);
  ସନ୍ଦେଶ = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, ସନ୍ଦେଶ);
}


ଶୂନ୍ୟ luaY_checklimit (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା v, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l, ଅଚଳ ଅକ୍ଷର *what) {
  ଯଦି (l_unlikely(v > l)) errorlimit(fs, l, what);
}


/*
** Test whether next token is 'c'; if so, skip it.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testnext (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  ଯଦି (ls->t.token == c) {
    luaX_next(ls);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;
}


/*
** Check that next token is 'c'.
*/
ସ୍ଥିର ଶୂନ୍ୟ check (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  ଯଦି (ls->t.token != c)
    error_expected(ls, c);
}


/*
** Check that next token is 'c' and skip it.
*/
ସ୍ଥିର ଶୂନ୍ୟ checknext (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  check(ls, c);
  luaX_next(ls);
}


#define check_condition(ls,c,ସନ୍ଦେଶ)	{ ଯଦି (!(c)) luaX_syntaxerror(ls, ସନ୍ଦେଶ); }


/*
** Check that next token is 'what' and skip it. In case of error,
** raise an error that the expected 'what' should match a 'who'
** in line 'where' (if that is not the current line).
*/
ସ୍ଥିର ଶୂନ୍ୟ check_match (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା what, ପୂର୍ଣ୍ଣସଂଖ୍ୟା who, ପୂର୍ଣ୍ଣସଂଖ୍ୟା where) {
  ଯଦି (l_unlikely(!testnext(ls, what))) {
    ଯଦି (where == ls->linenumber)  /* all in the same line? */
      error_expected(ls, what);  /* do not need a complex message */
    ଅନ୍ୟଥା {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


ସ୍ଥିର TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  ଫେରନ୍ତୁ ts;
}


ସ୍ଥିର ଶୂନ୍ୟ init_exp (expdesc *e, expkind k, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i) {
  e->f = e->t = NO_JUMP;
  e->k = k;
  e->u.info = i;
}


ସ୍ଥିର ଶୂନ୍ୟ codestring (expdesc *e, TString *s) {
  e->f = e->t = NO_JUMP;
  e->k = VKSTR;
  e->u.strval = s;
}


ସ୍ଥିର ଶୂନ୍ୟ codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}


/*
** Register a new local variable in the active 'Proto' (for debug
** information).
*/
ସ୍ଥିର short registerlocalvar (LexState *ls, FuncState *fs,
                               TString *varname) {
  Proto *f = fs->f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->ndebugvars, f->sizelocvars,
                  LocVar, SHRT_MAX, "local variables");
  ଯେତେବେଳେ (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = ଖାଲି;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  luaC_objbarrier(ls->L, f, varname);
  ଫେରନ୍ତୁ fs->ndebugvars++;
}


/*
** Create a new variable with the given 'name' and given 'kind'.
** Return its index in the function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_varkind (LexState *ls, TString *name, lu_byte kind) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  luaM_growvector(L, dyd->actvar.arr, dyd->actvar.n + 1,
             dyd->actvar.size, Vardesc, SHRT_MAX, "variable declarations");
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = kind;  /* default */
  var->vd.name = name;
  ଫେରନ୍ତୁ dyd->actvar.n - 1 - fs->firstlocal;
}


/*
** Create a new local variable with the given 'name' and regular kind.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_localvar (LexState *ls, TString *name) {
  ଫେରନ୍ତୁ new_varkind(ls, name, VDKREG);
}

#define new_localvarliteral(ls,v) \
    new_localvar(ls,  \
      luaX_newstring(ls, "" v, (ଆକାର(v)/ଆକାର(ଅକ୍ଷର)) - 1));



/*
** Return the "variable description" (Vardesc) of a given variable.
** (Unless noted otherwise, all variables are referred to by their
** compiler indices.)
*/
ସ୍ଥିର Vardesc *getlocalvardesc (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା vidx) {
  ଫେରନ୍ତୁ &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}


/*
** Convert 'nvar', a compiler index level, to its corresponding
** register. For that, search for the highest variable below that level
** that is in a register and uses its register index ('ridx') plus one.
*/
ସ୍ଥିର lu_byte reglevel (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvar) {
  ଯେତେବେଳେ (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);  /* get previous variable */
    ଯଦି (varinreg(vd))  /* is in a register? */
      ଫେରନ୍ତୁ cast_byte(vd->vd.ridx + 1);
  }
  ଫେରନ୍ତୁ 0;  /* no variables in registers */
}


/*
** Return the number of variables in the register stack for the given
** function.
*/
lu_byte luaY_nvarstack (FuncState *fs) {
  ଫେରନ୍ତୁ reglevel(fs, fs->nactvar);
}


/*
** Get the debug-information entry for current variable 'vidx'.
*/
ସ୍ଥିର LocVar *localdebuginfo (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା vidx) {
  Vardesc *vd = getlocalvardesc(fs,  vidx);
  ଯଦି (!varinreg(vd))
    ଫେରନ୍ତୁ ଖାଲି;  /* no debug info. for constants */
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = vd->vd.pidx;
    lua_assert(idx < fs->ndebugvars);
    ଫେରନ୍ତୁ &fs->f->locvars[idx];
  }
}


/*
** Create an expression representing variable 'vidx'
*/
ସ୍ଥିର ଶୂନ୍ୟ init_var (FuncState *fs, expdesc *e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା vidx) {
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
ସ୍ଥିର ଶୂନ୍ୟ check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = ଖାଲି;  /* to be set if variable is const */
  ବାଛ (e->k) {
    ସ୍ଥିତି VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି VLOCAL: ସ୍ଥିତି VVARGVAR: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      ଯଦି (vardesc->vd.kind != VDKREG)  /* not a regular variable? */
        varname = vardesc->vd.name;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      ଯଦି (up->kind != VDKREG)
        varname = up->name;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      e->k = VINDEXED;
    }  /* FALLTHROUGH */
    ସ୍ଥିତି VINDEXUP: ସ୍ଥିତି VINDEXSTR: ସ୍ଥିତି VINDEXED: {  /* global variable */
      ଯଦି (e->u.ind.ro)  /* read-only? */
        varname = tsvalue(&fs->f->k[e->u.ind.keystr]);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      lua_assert(e->k == VINDEXI);  /* this one doesn't need any check */
      ଫେରନ୍ତୁ;  /* integer index cannot be read-only */
  }
  ଯଦି (varname)
    luaK_semerror(ls, "attempt to assign to const variable '%s'",
                      getstr(varname));
}


/*
** Start the scope for the last 'nvars' created variables.
*/
ସ୍ଥିର ଶୂନ୍ୟ adjustlocalvars (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars) {
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା reglevel = luaY_nvarstack(fs);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < nvars; i++) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା vidx = fs->nactvar++;
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
ସ୍ଥିର ଶୂନ୍ୟ removevars (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  ଯେତେବେଳେ (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    ଯଦି (var)  /* does it have debug information? */
      var->endpc = fs->pc;
  }
}


/*
** Search the upvalues of the function 'fs' for one
** with the given 'name'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searchupvalue (FuncState *fs, TString *name) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Upvaldesc *up = fs->f->upvalues;
  ପାଇଁ (i = 0; i < fs->nups; i++) {
    ଯଦି (eqstr(up[i].name, name)) ଫେରନ୍ତୁ i;
  }
  ଫେରନ୍ତୁ -1;  /* not found */
}


ସ୍ଥିର Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldsize = f->sizeupvalues;
  luaY_checklimit(fs, fs->nups + 1, MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  Upvaldesc, MAXUPVAL, "upvalues");
  ଯେତେବେଳେ (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = ଖାଲି;
  ଫେରନ୍ତୁ &f->upvalues[fs->nups++];
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  ଯଦି (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    lua_assert(eqstr(name, getlocalvardesc(prev, v->u.var.vidx)->vd.name));
  }
  ଅନ୍ୟଥା {
    up->instack = 0;
    up->idx = cast_byte(v->u.info);
    up->kind = prev->f->upvalues[v->u.info].kind;
    lua_assert(eqstr(name, prev->f->upvalues[v->u.info].name));
  }
  up->name = name;
  luaC_objbarrier(fs->ls->L, fs->f, name);
  ଫେରନ୍ତୁ fs->nups - 1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searchvar (FuncState *fs, TString *n, expdesc *var) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = cast_int(fs->nactvar) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    ଯଦି (varglobal(vd)) {  /* global declaration? */
      ଯଦି (vd->vd.name == ଖାଲି) {  /* collective declaration? */
        ଯଦି (var->u.info < 0)  /* no previous collective declaration? */
          var->u.info = fs->firstlocal + i;  /* this is the first one */
      }
      ଅନ୍ୟଥା {  /* global name */
        ଯଦି (eqstr(n, vd->vd.name)) {  /* found? */
          init_exp(var, VGLOBAL, fs->firstlocal + i);
          ଫେରନ୍ତୁ VGLOBAL;
        }
        ଅନ୍ୟଥା ଯଦି (var->u.info == -1)  /* active preambular declaration? */
          var->u.info = -2;  /* invalidate preambular declaration */
      }
    }
    ଅନ୍ୟଥା ଯଦି (eqstr(n, vd->vd.name)) {  /* found? */
      ଯଦି (vd->vd.kind == RDKCTC)  /* compile-time constant? */
        init_exp(var, VCONST, fs->firstlocal + i);
      ଅନ୍ୟଥା {  /* local variable */
        init_var(fs, var, i);
        ଯଦି (vd->vd.kind == RDKVAVAR)  /* vararg parameter? */
          var->k = VVARGVAR;
      }
      ଫେରନ୍ତୁ cast_int(var->k);
    }
  }
  ଫେରନ୍ତୁ -1;  /* not found */
}


/*
** Mark block where variable at given level was defined
** (to emit close instructions later).
*/
ସ୍ଥିର ଶୂନ୍ୟ markupval (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
  BlockCnt *bl = fs->bl;
  ଯେତେବେଳେ (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}


/*
** Mark that current block has a to-be-closed variable.
*/
ସ୍ଥିର ଶୂନ୍ୟ marktobeclosed (FuncState *fs) {
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
ସ୍ଥିର ଶୂନ୍ୟ singlevaraux (FuncState *fs, TString *n, expdesc *var, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = searchvar(fs, n, var);  /* look up variables at current level */
  ଯଦି (v >= 0) {  /* found? */
    ଯଦି (!base) {
      ଯଦି (var->k == VVARGVAR)  /* vararg parameter? */
        luaK_vapar2local(fs, var);  /* change it to a regular local */
      ଯଦି (var->k == VLOCAL)
        markupval(fs, var->u.var.vidx);  /* will be used as an upvalue */
    }
    /* else nothing else to be done */
  }
  ଅନ୍ୟଥା {  /* not found at current level; try upvalues */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = searchupvalue(fs, n);  /* try existing upvalues */
    ଯଦି (idx < 0) {  /* not found? */
      ଯଦି (fs->prev != ଖାଲି)  /* more levels? */
        singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
      ଯଦି (var->k == VLOCAL || var->k == VUPVAL)  /* local or upvalue? */
        idx  = newupvalue(fs, n, var);  /* will be a new upvalue */
      ଅନ୍ୟଥା  /* it is a global or a constant */
        ଫେରନ୍ତୁ;  /* don't need to do anything at this level */
    }
    init_exp(var, VUPVAL, idx);  /* new or old upvalue */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ buildglobal (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  expdesc ଚାବି;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
  ଯଦି (var->k == VGLOBAL)
    luaK_semerror(ls, "%s is global when accessing variable '%s'",
                      LUA_ENV, getstr(varname));
  luaK_exp2anyregup(fs, var);  /* _ENV could be a constant */
  codestring(&ଚାବି, varname);  /* key is variable name */
  luaK_indexed(fs, var, &ଚାବି);  /* 'var' represents _ENV[varname] */
}


/*
** Find a variable with the given name 'n', handling global variables
** too.
*/
ସ୍ଥିର ଶୂନ୍ୟ buildvar (LexState *ls, TString *varname, expdesc *var) {
  FuncState *fs = ls->fs;
  init_exp(var, VGLOBAL, -1);  /* global by default */
  singlevaraux(fs, varname, var, 1);
  ଯଦି (var->k == VGLOBAL) {  /* global name? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା info = var->u.info;
    /* global by default in the scope of a global declaration? */
    ଯଦି (info == -2)
      luaK_semerror(ls, "variable '%s' not declared", getstr(varname));
    buildglobal(ls, varname, var);
    ଯଦି (info != -1 && ls->dyd->actvar.arr[info].vd.kind == GDKCONST)
      var->u.ind.ro = 1;  /* mark variable as read-only */
    ଅନ୍ୟଥା  /* anyway must be a global */
      lua_assert(info == -1 || ls->dyd->actvar.arr[info].vd.kind == GDKREG);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ singlevar (LexState *ls, expdesc *var) {
  buildvar(ls, str_checkname(ls), var);
}


/*
** Adjust the number of results from an expression list 'e' with 'nexps'
** expressions to 'nvars' values.
*/
ସ୍ଥିର ଶୂନ୍ୟ adjust_assign (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା needed = nvars - nexps;  /* extra values needed */
  luaK_checkstack(fs, needed);
  ଯଦି (hasmultret(e->k)) {  /* last expression has multiple returns? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = needed + 1;  /* discount last expression itself */
    ଯଦି (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
  }
  ଅନ୍ୟଥା {
    ଯଦି (e->k != VVOID)  /* at least one expression? */
      luaK_exp2nextreg(fs, e);  /* close last expression */
    ଯଦି (needed > 0)  /* missing values? */
      luaK_nil(fs, fs->freereg, needed);  /* complete with nils */
  }
  ଯଦି (needed > 0)
    luaK_reserveregs(fs, needed);  /* registers for extra values */
  ଅନ୍ୟଥା  /* adding 'needed' is actually a subtraction */
    fs->freereg = cast_byte(fs->freereg + needed);  /* remove extra values */
}


#define enterlevel(ls)	luaE_incCstack(ls->L)


#define leavelevel(ls) ((ls)->L->nCcalls--)


/*
** Generates an error that a goto jumps into the scope of some
** variable declaration.
*/
ସ୍ଥିର l_noret jumpscopeerror (LexState *ls, Labeldesc *gt) {
  TString *tsname = getlocalvardesc(ls->fs, gt->nactvar)->vd.name;
  ଅଚଳ ଅକ୍ଷର *varname = (tsname != ଖାଲି) ? getstr(tsname) : "*";
  luaK_semerror(ls,
     "<goto %s> at line %d jumps into the scope of '%s'",
      getstr(gt->name), gt->ଧାଡ଼ି, varname);  /* raise the error */
}


/*
** Closes the goto at index 'g' to given 'label' and removes it
** from the list of pending gotos.
** If it jumps into the scope of some variable, raises an error.
** The goto needs a CLOSE if it jumps out of a block with upvalues,
** or out of the scope of some variable and the block has upvalues
** (signaled by parameter 'bup').
*/
ସ୍ଥିର ଶୂନ୍ୟ closegoto (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା g, Labeldesc *label, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bup) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  FuncState *fs = ls->fs;
  Labellist *gl = &ls->dyd->gt;  /* list of gotos */
  Labeldesc *gt = &gl->arr[g];  /* goto to be resolved */
  lua_assert(eqstr(gt->name, label->name));
  ଯଦି (l_unlikely(gt->nactvar < label->nactvar))  /* enter some scope? */
    jumpscopeerror(ls, gt);
  ଯଦି (gt->close ||
      (label->nactvar < gt->nactvar && bup)) {  /* needs close? */
    lu_byte stklevel = reglevel(fs, label->nactvar);
    /* move jump to CLOSE position */
    fs->f->code[gt->pc + 1] = fs->f->code[gt->pc];
    /* put CLOSE instruction at original position */
    fs->f->code[gt->pc] = CREATE_ABCk(OP_CLOSE, stklevel, 0, 0, 0);
    gt->pc++;  /* must point to jump instruction */
  }
  luaK_patchlist(ls->fs, gt->pc, label->pc);  /* goto jumps to label */
  ପାଇଁ (i = g; i < gl->n - 1; i++)  /* remove goto from pending list */
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** Search for an active label with the given name, starting at
** index 'ilb' (so that it can search for all labels in current block
** or all labels in current function).
*/
ସ୍ଥିର Labeldesc *findlabel (LexState *ls, TString *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ilb) {
  Dyndata *dyd = ls->dyd;
  ପାଇଁ (; ilb < dyd->label.n; ilb++) {
    Labeldesc *lb = &dyd->label.arr[ilb];
    ଯଦି (eqstr(lb->name, name))  /* correct label? */
      ଫେରନ୍ତୁ lb;
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* label not found */
}


/*
** Adds a new label/goto in the corresponding list.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newlabelentry (LexState *ls, Labellist *l, TString *name,
                          ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].ଧାଡ଼ି = ଧାଡ଼ି;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  ଫେରନ୍ତୁ n;
}


/*
** Create an entry for the goto and the code for it. As it is not known
** at this point whether the goto may need a CLOSE, the code has a jump
** followed by an CLOSE. (As the CLOSE comes after the jump, it is a
** dead instruction; it works as a placeholder.) When the goto is closed
** against a label, if it needs a CLOSE, the two instructions swap
** positions, so that the CLOSE comes before the jump.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newgotoentry (LexState *ls, TString *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc = luaK_jump(fs);  /* create jump */
  luaK_codeABC(fs, OP_CLOSE, 0, 1, 0);  /* spaceholder, marked as dead */
  ଫେରନ୍ତୁ newlabelentry(ls, &ls->dyd->gt, name, ଧାଡ଼ି, pc);
}


/*
** Create a new label with the given 'name' at the given 'line'.
** 'last' tells whether label is the last non-op statement in its
** block. Solves all pending gotos to this new label and adds
** a close instruction if necessary.
** Returns true iff it added a close instruction.
*/
ସ୍ଥିର ଶୂନ୍ୟ createlabel (LexState *ls, TString *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା l = newlabelentry(ls, ll, name, ଧାଡ଼ି, luaK_getlabel(fs));
  ଯଦି (last) {  /* label is last no-op statement in the block? */
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
ସ୍ଥିର ଶୂନ୍ୟ solvegotos (FuncState *fs, BlockCnt *bl) {
  LexState *ls = fs->ls;
  Labellist *gl = &ls->dyd->gt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା outlevel = reglevel(fs, bl->nactvar);  /* level outside the block */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା igt = bl->firstgoto;  /* first goto in the finishing block */
  ଯେତେବେଳେ (igt < gl->n) {   /* for each pending goto */
    Labeldesc *gt = &gl->arr[igt];
    /* search for a matching label in the current block */
    Labeldesc *lb = findlabel(ls, gt->name, bl->firstlabel);
    ଯଦି (lb != ଖାଲି)  /* found a match? */
      closegoto(ls, igt, lb, bl->upval);  /* close and remove goto */
    ଅନ୍ୟଥା {  /* adjust 'goto' for outer block */
      /* block has variables to be closed and goto escapes the scope of
         some variable? */
      ଯଦି (bl->upval && reglevel(fs, gt->nactvar) > outlevel)
        gt->close = 1;  /* jump may need a close */
      gt->nactvar = bl->nactvar;  /* correct level for outer block */
      igt++;  /* go to next goto */
    }
  }
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
}


ସ୍ଥିର ଶୂନ୍ୟ enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  /* inherit 'insidetbc' from enclosing block */
  bl->insidetbc = (fs->bl != ଖାଲି && fs->bl->insidetbc);
  bl->previous = fs->bl;  /* link block in function's block list */
  fs->bl = bl;
  lua_assert(fs->freereg == luaY_nvarstack(fs));
}


/*
** generates an error for an undefined 'goto'.
*/
ସ୍ଥିର l_noret undefgoto (LexState *ls, Labeldesc *gt) {
  /* breaks are checked when created, cannot be undefined */
  lua_assert(!eqstr(gt->name, ls->brkn));
  luaK_semerror(ls, "no visible label '%s' for <goto> at line %d",
                    getstr(gt->name), gt->ଧାଡ଼ି);
}


ସ୍ଥିର ଶୂନ୍ୟ leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  lu_byte stklevel = reglevel(fs, bl->nactvar);  /* level outside block */
  ଯଦି (bl->previous && bl->upval)  /* need a 'close'? */
    luaK_codeABC(fs, OP_CLOSE, stklevel, 0, 0);
  fs->freereg = stklevel;  /* free registers */
  removevars(fs, bl->nactvar);  /* remove block locals */
  lua_assert(bl->nactvar == fs->nactvar);  /* back to level on entry */
  ଯଦି (bl->isloop == 2)  /* has to fix pending breaks? */
    createlabel(ls, ls->brkn, 0, 0);
  solvegotos(fs, bl);
  ଯଦି (bl->previous == ଖାଲି) {  /* was it the last block? */
    ଯଦି (bl->firstgoto < ls->dyd->gt.n)  /* still pending gotos? */
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
  }
  fs->bl = bl->previous;  /* current block now is previous one */
}


/*
** adds a new prototype into list of prototypes
*/
ସ୍ଥିର Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;  /* prototype of current function */
  ଯଦି (fs->np >= f->sizep) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, Proto *, MAXARG_Bx, "functions");
    ଯେତେବେଳେ (oldsize < f->sizep)
      f->p[oldsize++] = ଖାଲି;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  ଫେରନ୍ତୁ clp;
}


/*
** codes instruction to create new closure in parent function.
** The OP_CLOSURE instruction uses the last available register,
** so that, if it invokes the GC, the GC knows which registers
** are in use at that time.

*/
ସ୍ଥିର ଶୂନ୍ୟ codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


ସ୍ଥିର ଶୂନ୍ୟ open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
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
  fs->bl = ଖାଲି;
  f->source = ls->source;
  luaC_objbarrier(L, f, f->source);
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  fs->kcache = luaH_new(L);  /* create table for function */
  luaD_anchorobj(L, ls->h, obj2gco(fs->kcache));  /* anchor it */
  enterblock(fs, bl, 0);
}


ସ୍ଥିର ଶୂନ୍ୟ close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  TValue temp;
  luaK_ret(fs, luaY_nvarstack(fs), 0);  /* final return */
  leaveblock(fs);
  lua_assert(fs->bl == ଖାଲି);
  luaK_finish(fs);
  luaM_shrinkvector(L, f->code, f->sizecode, fs->pc, Instruction);
  luaM_shrinkvector(L, f->lineinfo, f->sizelineinfo, fs->pc, ls_byte);
  luaM_shrinkvector(L, f->abslineinfo, f->sizeabslineinfo,
                       fs->nabslineinfo, Abs_ଧାଡ଼ି_Info);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା block_follow (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା withuntil) {
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି TK_ELSE: ସ୍ଥିତି TK_ELSEIF:
    ସ୍ଥିତି TK_END: ସ୍ଥିତି TK_EOS:
      ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି TK_UNTIL: ଫେରନ୍ତୁ withuntil;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ 0;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ statlist (LexState *ls) {
  /* statlist -> { stat [';'] } */
  ଯେତେବେଳେ (!block_follow(ls, 1)) {
    ଯଦି (ls->t.token == TK_RETURN) {
      statement(ls);
      ଫେରନ୍ତୁ;  /* 'return' must be last statement */
    }
    statement(ls);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ fieldsel (LexState *ls, expdesc *v) {
  /* fieldsel -> ['.' | ':'] NAME */
  FuncState *fs = ls->fs;
  expdesc ଚାବି;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  codename(ls, &ଚାବି);
  luaK_indexed(fs, v, &ଚାବି);
}


ସ୍ଥିର ଶୂନ୍ୟ yindex (LexState *ls, expdesc *v) {
  /* index -> '[' expr ']' */
  luaX_next(ls);  /* skip the '[' */
  ପଦସମଷ୍ଟି(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ConsControl {
  expdesc v;  /* last list item read */
  expdesc *t;  /* table descriptor */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nh;  /* total number of 'record' elements */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା na;  /* number of array elements already stored */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tostore;  /* number of array elements pending to be stored */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxtostore;  /* maximum number of pending elements */
} ConsControl;


/*
** Maximum number of elements in a constructor, to control the following:
** * counter overflows;
** * overflows in 'extra' for OP_NEWTABLE and OP_SETLIST;
** * overflows when adding multiple returns in OP_SETLIST.
*/
#define MAX_CNST	(INT_MAX/2)
#ଯଦି MAX_CNST/(MAXARG_vC + 1) > MAXARG_Ax
#undef MAX_CNST
#define MAX_CNST	(MAXARG_Ax * (MAXARG_vC + 1))
#endif


ସ୍ଥିର ଶୂନ୍ୟ recfield (LexState *ls, ConsControl *cc) {
  /* recfield -> (NAME | '['exp']') = exp */
  FuncState *fs = ls->fs;
  lu_byte reg = ls->fs->freereg;
  expdesc tab, ଚାବି, val;
  ଯଦି (ls->t.token == TK_NAME)
    codename(ls, &ଚାବି);
  ଅନ୍ୟଥା  /* ls->t.token == '[' */
    yindex(ls, &ଚାବି);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &ଚାବି);
  ପଦସମଷ୍ଟି(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;  /* free registers */
}


ସ୍ଥିର ଶୂନ୍ୟ closelistfield (FuncState *fs, ConsControl *cc) {
  lua_assert(cc->tostore > 0);
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  ଯଦି (cc->tostore >= cc->maxtostore) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->na += cc->tostore;
    cc->tostore = 0;  /* no more items pending */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ lastlistfield (FuncState *fs, ConsControl *cc) {
  ଯଦି (cc->tostore == 0) ଫେରନ୍ତୁ;
  ଯଦି (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  ଅନ୍ୟଥା {
    ଯଦି (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


ସ୍ଥିର ଶୂନ୍ୟ listfield (LexState *ls, ConsControl *cc) {
  /* listfield -> exp */
  ପଦସମଷ୍ଟି(ls, &cc->v);
  cc->tostore++;
}


ସ୍ଥିର ଶୂନ୍ୟ field (LexState *ls, ConsControl *cc) {
  /* field -> listfield | recfield */
  ବାଛ(ls->t.token) {
    ସ୍ଥିତି TK_NAME: {  /* may be 'listfield' or 'recfield' */
      ଯଦି (luaX_lookahead(ls) != '=')  /* expression? */
        listfield(ls, cc);
      ଅନ୍ୟଥା
        recfield(ls, cc);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି '[': {
      recfield(ls, cc);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      listfield(ls, cc);
      ଭାଙ୍ଗ;
    }
  }
}


/*
** Compute a limit for how many registers a constructor can use before
** emitting a 'SETLIST' instruction, based on how many registers are
** available.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxtostore (FuncState *fs) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା numfreeregs = MAX_FSTACK - fs->freereg;
  ଯଦି (numfreeregs >= 160)  /* "lots" of registers? */
    ଫେରନ୍ତୁ numfreeregs / 5;  /* use up to 1/5 of them */
  ଅନ୍ୟଥା ଯଦି (numfreeregs >= 80)  /* still "enough" registers? */
    ଫେରନ୍ତୁ 10;  /* one 'SETLIST' instruction for each 10 values */
  ଅନ୍ୟଥା  /* save registers for potential more nesting */
    ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଶୂନ୍ୟ constructor (LexState *ls, expdesc *t) {
  /* constructor -> '{' [ field { sep field } [sep] ] '}'
     sep -> ',' | ';' */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc = luaK_codevABCk(fs, OP_NEWTABLE, 0, 0, 0, 0);
  ConsControl cc;
  luaK_code(fs, 0);  /* space for extra arg. */
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);  /* table will be at stack top */
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);  /* no value (yet) */
  checknext(ls, '{' /*}*/);
  cc.maxtostore = maxtostore(fs);
  କର {
    ଯଦି (ls->t.token == /*{*/ '}') ଭାଙ୍ଗ;
    ଯଦି (cc.v.k != VVOID)  /* is there a previous list item? */
      closelistfield(fs, &cc);  /* close it */
    field(ls, &cc);
    luaY_checklimit(fs, cc.tostore + cc.na + cc.nh, MAX_CNST,
                    "items in a constructor");
  } ଯେତେବେଳେ (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, /*{*/ '}', '{' /*}*/, ଧାଡ଼ି);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}

/* }====================================================================== */


ସ୍ଥିର ଶୂନ୍ୟ setvararg (FuncState *fs) {
  fs->f->flag |= PF_VAHID;  /* by default, use hidden vararg arguments */
  luaK_codeABC(fs, OP_VARARGPREP, 0, 0, 0);
}


ସ୍ଥିର ଶୂନ୍ୟ parlist (LexState *ls) {
  /* parlist -> [ {NAME ','} (NAME | '...') ] */
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nparams = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା varargk = 0;
  ଯଦି (ls->t.token != ')') {  /* is 'parlist' not empty? */
    କର {
      ବାଛ (ls->t.token) {
        ସ୍ଥିତି TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି TK_DOTS: {
          varargk = 1;
          luaX_next(ls);  /* skip '...' */
          ଯଦି (ls->t.token == TK_NAME)
            new_varkind(ls, str_checkname(ls), RDKVAVAR);
          ଅନ୍ୟଥା
            new_localvarliteral(ls, "(vararg table)");
          ଭାଙ୍ଗ;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } ଯେତେବେଳେ (!varargk && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = cast_byte(fs->nactvar);
  ଯଦି (varargk) {
    setvararg(fs);  /* declared vararg */
    adjustlocalvars(ls, 1);  /* vararg parameter */
  }
  /* reserve registers for parameters (plus vararg parameter, if present) */
  luaK_reserveregs(fs, fs->nactvar);
}


ସ୍ଥିର ଶୂନ୍ୟ body (LexState *ls, expdesc *e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ismethod, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* body ->  '(' parlist ')' block END */
  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = ଧାଡ଼ି;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  ଯଦି (ismethod) {
    new_localvarliteral(ls, "self");  /* create 'self' parameter */
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, ଧାଡ଼ି);
  codeclosure(ls, e);
  close_func(ls);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା explist (LexState *ls, expdesc *v) {
  /* explist -> expr { ',' expr } */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 1;  /* at least one expression */
  ପଦସମଷ୍ଟି(ls, v);
  ଯେତେବେଳେ (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    ପଦସମଷ୍ଟି(ls, v);
    n++;
  }
  ଫେରନ୍ତୁ n;
}


ସ୍ଥିର ଶୂନ୍ୟ funcargs (LexState *ls, expdesc *f) {
  FuncState *fs = ls->fs;
  expdesc args;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, nparams;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି '(': {  /* funcargs -> '(' [ explist ] ')' */
      luaX_next(ls);
      ଯଦି (ls->t.token == ')')  /* arg list is empty? */
        args.k = VVOID;
      ଅନ୍ୟଥା {
        explist(ls, &args);
        ଯଦି (hasmultret(args.k))
          luaK_setmultret(fs, &args);
      }
      check_match(ls, ')', '(', ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି '{' /*}*/: {  /* funcargs -> constructor */
      constructor(ls, &args);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_STRING: {  /* funcargs -> STRING */
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'next' */
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      luaX_syntaxerror(ls, "function arguments expected");
      ଫେରନ୍ତୁ;  /* to avoid warnings */
    }
  }
  lua_assert(f->k == VNONRELOC);
  base = f->u.info;  /* base register for call */
  ଯଦି (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  ଅନ୍ୟଥା {
    ଯଦି (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABC(fs, OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, ଧାଡ଼ି);
  /* call removes function and arguments and leaves one result (unless
     changed later) */
  fs->freereg = cast_byte(base + 1);
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


ସ୍ଥିର ଶୂନ୍ୟ primaryexp (LexState *ls, expdesc *v) {
  /* primaryexp -> NAME | '(' expr ')' */
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି '(': {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;
      luaX_next(ls);
      ପଦସମଷ୍ଟି(ls, v);
      check_match(ls, ')', '(', ଧାଡ଼ି);
      luaK_dischargevars(ls->fs, v);
      ଫେରନ୍ତୁ;
    }
    ସ୍ଥିତି TK_NAME: {
      singlevar(ls, v);
      ଫେରନ୍ତୁ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ suffixedexp (LexState *ls, expdesc *v) {
  /* suffixedexp ->
       primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
  FuncState *fs = ls->fs;
  primaryexp(ls, v);
  ପାଇଁ (;;) {
    ବାଛ (ls->t.token) {
      ସ୍ଥିତି '.': {  /* fieldsel */
        fieldsel(ls, v);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '[': {  /* '[' exp ']' */
        expdesc ଚାବି;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &ଚାବି);
        luaK_indexed(fs, v, &ଚାବି);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି ':': {  /* ':' NAME funcargs */
        expdesc ଚାବି;
        luaX_next(ls);
        codename(ls, &ଚାବି);
        luaK_self(fs, v, &ଚାବି);
        funcargs(ls, v);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '(': ସ୍ଥିତି TK_STRING: ସ୍ଥିତି '{' /*}*/: {  /* funcargs */
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v);
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ;
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ simpleexp (LexState *ls, expdesc *v) {
  /* simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  constructor | FUNCTION body | suffixedexp */
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_NIL: {
      init_exp(v, VNIL, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRUE: {
      init_exp(v, VTRUE, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_FALSE: {
      init_exp(v, VFALSE, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_DOTS: {  /* vararg */
      FuncState *fs = ls->fs;
      check_condition(ls, isvararg(fs->f),
                      "cannot use '...' outside a vararg function");
      init_exp(v, VVARARG, luaK_codeABC(fs, OP_VARARG, 0, fs->f->numparams, 1));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି '{' /*}*/: {  /* constructor */
      constructor(ls, v);
      ଫେରନ୍ତୁ;
    }
    ସ୍ଥିତି TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      ଫେରନ୍ତୁ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      suffixedexp(ls, v);
      ଫେରନ୍ତୁ;
    }
  }
  luaX_next(ls);
}


ସ୍ଥିର UnOpr getunopr (ପୂର୍ଣ୍ଣସଂଖ୍ୟା op) {
  ବାଛ (op) {
    ସ୍ଥିତି TK_NOT: ଫେରନ୍ତୁ OPR_NOT;
    ସ୍ଥିତି '-': ଫେରନ୍ତୁ OPR_MINUS;
    ସ୍ଥିତି '~': ଫେରନ୍ତୁ OPR_BNOT;
    ସ୍ଥିତି '#': ଫେରନ୍ତୁ OPR_LEN;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ OPR_NOUNOPR;
  }
}


ସ୍ଥିର BinOpr getbinopr (ପୂର୍ଣ୍ଣସଂଖ୍ୟା op) {
  ବାଛ (op) {
    ସ୍ଥିତି '+': ଫେରନ୍ତୁ OPR_ADD;
    ସ୍ଥିତି '-': ଫେରନ୍ତୁ OPR_SUB;
    ସ୍ଥିତି '*': ଫେରନ୍ତୁ OPR_MUL;
    ସ୍ଥିତି '%': ଫେରନ୍ତୁ OPR_MOD;
    ସ୍ଥିତି '^': ଫେରନ୍ତୁ OPR_POW;
    ସ୍ଥିତି '/': ଫେରନ୍ତୁ OPR_DIV;
    ସ୍ଥିତି TK_IDIV: ଫେରନ୍ତୁ OPR_IDIV;
    ସ୍ଥିତି '&': ଫେରନ୍ତୁ OPR_BAND;
    ସ୍ଥିତି '|': ଫେରନ୍ତୁ OPR_BOR;
    ସ୍ଥିତି '~': ଫେରନ୍ତୁ OPR_BXOR;
    ସ୍ଥିତି TK_SHL: ଫେରନ୍ତୁ OPR_SHL;
    ସ୍ଥିତି TK_SHR: ଫେରନ୍ତୁ OPR_SHR;
    ସ୍ଥିତି TK_CONCAT: ଫେରନ୍ତୁ OPR_CONCAT;
    ସ୍ଥିତି TK_NE: ଫେରନ୍ତୁ OPR_NE;
    ସ୍ଥିତି TK_EQ: ଫେରନ୍ତୁ OPR_EQ;
    ସ୍ଥିତି '<': ଫେରନ୍ତୁ OPR_LT;
    ସ୍ଥିତି TK_LE: ଫେରନ୍ତୁ OPR_LE;
    ସ୍ଥିତି '>': ଫେରନ୍ତୁ OPR_GT;
    ସ୍ଥିତି TK_GE: ଫେରନ୍ତୁ OPR_GE;
    ସ୍ଥିତି TK_AND: ଫେରନ୍ତୁ OPR_AND;
    ସ୍ଥିତି TK_OR: ଫେରନ୍ତୁ OPR_OR;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ OPR_NOBINOPR;
  }
}


/*
** Priority table for binary operators.
*/
ସ୍ଥିର ଅଚଳ ସଂରଚନା {
  lu_byte ବାମ;  /* left priority for each binary operator */
  lu_byte ଡାହାଣ; /* right priority */
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
ସ୍ଥିର BinOpr subexpr (LexState *ls, expdesc *v, ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit) {
  BinOpr op;
  UnOpr uop;
  enterlevel(ls);
  uop = getunopr(ls->t.token);
  ଯଦି (uop != OPR_NOUNOPR) {  /* prefix (unary) operator? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, ଧାଡ଼ି);
  }
  ଅନ୍ୟଥା simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  ଯେତେବେଳେ (op != OPR_NOBINOPR && priority[op].ବାମ > limit) {
    expdesc v2;
    BinOpr nextop;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;
    luaX_next(ls);  /* skip operator */
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].ଡାହାଣ);
    luaK_posfix(ls->fs, op, v, &v2, ଧାଡ଼ି);
    op = nextop;
  }
  leavelevel(ls);
  ଫେରନ୍ତୁ op;  /* return first untreated operator */
}


ସ୍ଥିର ଶୂନ୍ୟ ପଦସମଷ୍ଟି (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


ସ୍ଥିର ଶୂନ୍ୟ block (LexState *ls) {
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
ସଂରଚନା LHS_assign {
  ସଂରଚନା LHS_assign *prev;
  expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** check whether, in an assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous assignment to a
** table. If so, save original upvalue/local value in a safe place and
** use this safe copy in the previous assignment.
*/
ସ୍ଥିର ଶୂନ୍ୟ check_conflict (LexState *ls, ସଂରଚନା LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  lu_byte extra = fs->freereg;  /* eventual position to save local variable */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା conflict = 0;
  ପାଇଁ (; lh; lh = lh->prev) {  /* check all previous assignments */
    ଯଦି (vkisindexed(lh->v.k)) {  /* assignment to table field? */
      ଯଦି (lh->v.k == VINDEXUP) {  /* is table an upvalue? */
        ଯଦି (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;  /* table is the upvalue being assigned now */
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
      }
      ଅନ୍ୟଥା {  /* table is a register */
        ଯଦି (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;  /* table is the local being assigned now */
          lh->v.u.ind.t = extra;  /* assignment will use safe copy */
        }
        /* is index the local being assigned? */
        ଯଦି (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;  /* previous assignment will use safe copy */
        }
      }
    }
  }
  ଯଦି (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    ଯଦି (v->k == VLOCAL)
      luaK_codeABC(fs, OP_MOVE, extra, v->u.var.ridx, 0);
    ଅନ୍ୟଥା
      luaK_codeABC(fs, OP_GETUPVAL, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


/* Create code to store the "top" register in 'var' */
ସ୍ଥିର ଶୂନ୍ୟ storevartop (FuncState *fs, expdesc *var) {
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
ସ୍ଥିର ଶୂନ୍ୟ restassign (LexState *ls, ସଂରଚନା LHS_assign *lh, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars) {
  expdesc e;
  check_condition(ls, vkisvar(lh->v.k), "syntax error");
  check_readonly(ls, &lh->v);
  ଯଦି (testnext(ls, ',')) {  /* restassign -> ',' suffixedexp restassign */
    ସଂରଚନା LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    ଯଦି (!vkisindexed(nv.v.k))
      check_conflict(ls, lh, &nv.v);
    enterlevel(ls);  /* control recursion depth */
    restassign(ls, &nv, nvars+1);
    leavelevel(ls);
  }
  ଅନ୍ୟଥା {  /* restassign -> '=' explist */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    ଯଦି (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    ଅନ୍ୟଥା {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      ଫେରନ୍ତୁ;  /* avoid default */
    }
  }
  storevartop(ls->fs, &lh->v);  /* default assignment */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cond (LexState *ls) {
  /* cond -> exp */
  expdesc v;
  ପଦସମଷ୍ଟି(ls, &v);  /* read condition */
  ଯଦି (v.k == VNIL) v.k = VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  ଫେରନ୍ତୁ v.f;
}


ସ୍ଥିର ଶୂନ୍ୟ gotostat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  TString *name = str_checkname(ls);  /* label's name */
  newgotoentry(ls, name, ଧାଡ଼ି);
}


/*
** Break statement. Semantically equivalent to "goto break".
*/
ସ୍ଥିର ଶୂନ୍ୟ breakstat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  BlockCnt *bl;  /* to look for an enclosing loop */
  ପାଇଁ (bl = ls->fs->bl; bl != ଖାଲି; bl = bl->previous) {
    ଯଦି (bl->isloop)  /* found one? */
      ଯାଅ ok;
  }
  luaX_syntaxerror(ls, "break outside loop");
 ok:
  bl->isloop = 2;  /* signal that block has pending breaks */
  luaX_next(ls);  /* skip break */
  newgotoentry(ls, ls->brkn, ଧାଡ଼ି);
}


/*
** Check whether there is already a label with the given 'name' at
** current function.
*/
ସ୍ଥିର ଶୂନ୍ୟ checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name, ls->fs->firstlabel);
  ଯଦି (l_unlikely(lb != ଖାଲି))  /* already defined? */
    luaK_semerror(ls, "label '%s' already defined on line %d",
                      getstr(name), lb->ଧାଡ଼ି);  /* error */
}


ସ୍ଥିର ଶୂନ୍ୟ labelstat (LexState *ls, TString *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* label -> '::' NAME '::' */
  checknext(ls, TK_DBCOLON);  /* skip double colon */
  ଯେତେବେଳେ (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);  /* skip other no-op statements */
  checkrepeated(ls, name);  /* check for repeated labels */
  createlabel(ls, name, ଧାଡ଼ି, block_follow(ls, 0));
}


ସ୍ଥିର ଶୂନ୍ୟ whilestat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* whilestat -> WHILE cond DO block END */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା whileinit;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା condexit;
  BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_jumpto(fs, whileinit);
  check_match(ls, TK_END, TK_WHILE, ଧାଡ଼ି);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


ସ୍ଥିର ଶୂନ୍ୟ repeatstat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* repeatstat -> REPEAT block UNTIL cond */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା condexit;
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);  /* loop block */
  enterblock(fs, &bl2, 0);  /* scope block */
  luaX_next(ls);  /* skip REPEAT */
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, ଧାଡ଼ି);
  condexit = cond(ls);  /* read condition (inside scope block) */
  ଯଦି (bl2.upval) {  /* upvalues? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା exit = luaK_jump(fs);  /* normal exit must jump over fix */
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
ସ୍ଥିର ଶୂନ୍ୟ exp1 (LexState *ls) {
  expdesc e;
  ପଦସମଷ୍ଟି(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == VNONRELOC);
}


/*
** Fix for instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua). 'back' true means
** a back jump.
*/
ସ୍ଥିର ଶୂନ୍ୟ fixforjump (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା back) {
  Instruction *jmp = &fs->f->code[pc];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = dest - (pc + 1);
  ଯଦି (back)
    offset = -offset;
  ଯଦି (l_unlikely(offset > MAXARG_Bx))
    luaX_syntaxerror(fs->ls, "control structure too long");
  SETARG_Bx(*jmp, offset);
}


/*
** Generate code for a 'for' loop.
*/
ସ୍ଥିର ଶୂନ୍ୟ forbody (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isgen) {
  /* forbody -> DO block */
  ସ୍ଥିର ଅଚଳ OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  ସ୍ଥିର ଅଚଳ OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  fs->freereg--;  /* both 'forprep' remove one register from the stack */
  enterblock(fs, &bl, 0);  /* scope for declared variables */
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);  /* end of scope for declared variables */
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  ଯଦି (isgen) {  /* generic for? */
    luaK_codeABC(fs, OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, ଧାଡ଼ି);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, ଧାଡ଼ି);
}


ସ୍ଥିର ଶୂନ୍ୟ fornum (LexState *ls, TString *varname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* fornum -> NAME = exp,exp[,exp] forbody */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା base = fs->freereg;
  new_localvarliteral(ls, "(for state)");
  new_localvarliteral(ls, "(for state)");
  new_varkind(ls, varname, RDKCONST);  /* control variable */
  checknext(ls, '=');
  exp1(ls);  /* initial value */
  checknext(ls, ',');
  exp1(ls);  /* limit */
  ଯଦି (testnext(ls, ','))
    exp1(ls);  /* optional step */
  ଅନ୍ୟଥା {  /* default step = 1 */
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 2);  /* start scope for internal variables */
  forbody(ls, base, ଧାଡ଼ି, 1, 0);
}


ସ୍ଥିର ଶୂନ୍ୟ forlist (LexState *ls, TString *indexname) {
  /* forlist -> NAME {,NAME} IN explist forbody */
  FuncState *fs = ls->fs;
  expdesc e;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars = 4;  /* function, state, closing, control */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା base = fs->freereg;
  /* create internal variables */
  new_localvarliteral(ls, "(for state)");  /* iterator function */
  new_localvarliteral(ls, "(for state)");  /* state */
  new_localvarliteral(ls, "(for state)");  /* closing var. (after swap) */
  new_varkind(ls, indexname, RDKCONST);  /* control variable */
  /* other declared variables */
  ଯେତେବେଳେ (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  ଧାଡ଼ି = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 3);  /* start scope for internal variables */
  marktobeclosed(fs);  /* last internal var. must be closed */
  luaK_checkstack(fs, 2);  /* extra space to call iterator */
  forbody(ls, base, ଧାଡ଼ି, nvars - 3, 1);
}


ସ୍ଥିର ଶୂନ୍ୟ forstat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* forstat -> FOR (fornum | forlist) END */
  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = str_checkname(ls);  /* first variable name */
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି '=': fornum(ls, varname, ଧାଡ଼ି); ଭାଙ୍ଗ;
    ସ୍ଥିତି ',': ସ୍ଥିତି TK_IN: forlist(ls, varname); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, ଧାଡ଼ି);
  leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


ସ୍ଥିର ଶୂନ୍ୟ test_then_block (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *escapelist) {
  /* test_then_block -> [IF | ELSEIF] cond THEN block */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା condtrue;
  luaX_next(ls);  /* skip IF or ELSEIF */
  condtrue = cond(ls);  /* read condition */
  checknext(ls, TK_THEN);
  block(ls);  /* 'then' part */
  ଯଦି (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, condtrue);
}


ସ୍ଥିର ଶୂନ୍ୟ ifstat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* ifstat -> IF cond THEN block {ELSEIF cond THEN block} [ELSE block] END */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା escapelist = NO_JUMP;  /* exit list for finished parts */
  test_then_block(ls, &escapelist);  /* IF cond THEN block */
  ଯେତେବେଳେ (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);  /* ELSEIF cond THEN block */
  ଯଦି (testnext(ls, TK_ELSE))
    block(ls);  /* 'else' part */
  check_match(ls, TK_END, TK_IF, ଧାଡ଼ି);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


ସ୍ଥିର ଶୂନ୍ୟ localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fvar = fs->nactvar;  /* function's variable index */
  new_localvar(ls, str_checkname(ls));  /* new local variable */
  adjustlocalvars(ls, 1);  /* enter its scope */
  body(ls, &b, 0, ls->linenumber);  /* function created in next register */
  /* debug information will only see the variable after this point! */
  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


ସ୍ଥିର lu_byte getvarattribute (LexState *ls, lu_byte df) {
  /* attrib -> ['<' NAME '>'] */
  ଯଦି (testnext(ls, '<')) {
    TString *ts = str_checkname(ls);
    ଅଚଳ ଅକ୍ଷର *attr = getstr(ts);
    checknext(ls, '>');
    ଯଦି (strcmp(attr, "const") == 0)
      ଫେରନ୍ତୁ RDKCONST;  /* read-only variable */
    ଅନ୍ୟଥା ଯଦି (strcmp(attr, "close") == 0)
      ଫେରନ୍ତୁ RDKTOCLOSE;  /* to-be-closed variable */
    ଅନ୍ୟଥା
      luaK_semerror(ls, "unknown attribute '%s'", attr);
  }
  ଫେରନ୍ତୁ df;  /* return default value */
}


ସ୍ଥିର ଶୂନ୍ୟ checktoclose (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
  ଯଦି (level != -1) {  /* is there a to-be-closed variable? */
    marktobeclosed(fs);
    luaK_codeABC(fs, OP_TBC, reglevel(fs, level), 0, 0);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ localstat (LexState *ls) {
  /* stat -> LOCAL NAME attrib { ',' NAME attrib } ['=' explist] */
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା toclose = -1;  /* index of to-be-closed variable (if any) */
  Vardesc *var;  /* last variable */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା vidx;  /* index of last variable */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nexps;
  expdesc e;
  /* get prefixed attribute (if any); default is regular local variable */
  lu_byte defkind = getvarattribute(ls, VDKREG);
  କର {  /* for each variable */
    TString *vname = str_checkname(ls);  /* get its name */
    lu_byte kind = getvarattribute(ls, defkind);  /* postfixed attribute */
    vidx = new_varkind(ls, vname, kind);  /* predeclare it */
    ଯଦି (kind == RDKTOCLOSE) {  /* to-be-closed? */
      ଯଦି (toclose != -1)  /* one already present? */
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } ଯେତେବେଳେ (testnext(ls, ','));
  ଯଦି (testnext(ls, '='))  /* initialization? */
    nexps = explist(ls, &e);
  ଅନ୍ୟଥା {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);  /* retrieve last variable */
  ଯଦି (nvars == nexps &&  /* no adjustments? */
      var->vd.kind == RDKCONST &&  /* last variable is const? */
      luaK_exp2const(fs, &e, &var->k)) {  /* compile-time constant? */
    var->vd.kind = RDKCTC;  /* variable is a compile-time constant */
    adjustlocalvars(ls, nvars - 1);  /* exclude last variable */
    fs->nactvar++;  /* but count it */
  }
  ଅନ୍ୟଥା {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


ସ୍ଥିର lu_byte getglobalattribute (LexState *ls, lu_byte df) {
  lu_byte kind = getvarattribute(ls, df);
  ବାଛ (kind) {
    ସ୍ଥିତି RDKTOCLOSE:
      luaK_semerror(ls, "global variables cannot be to-be-closed");
      ଫେରନ୍ତୁ kind;  /* to avoid warnings */
    ସ୍ଥିତି RDKCONST:
      ଫେରନ୍ତୁ GDKCONST;  /* adjust kind for global variable */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ kind;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ checkglobal (LexState *ls, TString *varname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  FuncState *fs = ls->fs;
  expdesc var;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
  buildglobal(ls, varname, &var);  /* create global variable in 'var' */
  k = var.u.ind.keystr;  /* index of global name in 'k' */
  luaK_codecheckglobal(fs, &var, k, ଧାଡ଼ି);
}


/*
** Recursively traverse list of globals to be initalized. When
** going, generate table description for the global. In the end,
** after all indices have been generated, read list of initializing
** expressions. When returning, generate the assignment of the value on
** the stack to the corresponding table description. 'n' is the variable
** being handled, range [0, nvars - 1].
*/
ସ୍ଥିର ଶୂନ୍ୟ initglobal (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars, ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstidx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  ଯଦି (n == nvars) {  /* traversed all variables? */
    expdesc e;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nexps = explist(ls, &e);  /* read list of expressions */
    adjust_assign(ls, nvars, nexps, &e);
  }
  ଅନ୍ୟଥା {  /* handle variable 'n' */
    FuncState *fs = ls->fs;
    expdesc var;
    TString *varname = getlocalvardesc(fs, firstidx + n)->vd.name;
    buildglobal(ls, varname, &var);  /* create global variable in 'var' */
    enterlevel(ls);  /* control recursion depth */
    initglobal(ls, nvars, firstidx, n + 1, ଧାଡ଼ି);
    leavelevel(ls);
    checkglobal(ls, varname, ଧାଡ଼ି);
    storevartop(fs, &var);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ globalnames (LexState *ls, lu_byte defkind) {
  FuncState *fs = ls->fs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvars = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastidx;  /* index of last registered variable */
  କର {  /* for each name */
    TString *vname = str_checkname(ls);
    lu_byte kind = getglobalattribute(ls, defkind);
    lastidx = new_varkind(ls, vname, kind);
    nvars++;
  } ଯେତେବେଳେ (testnext(ls, ','));
  ଯଦି (testnext(ls, '='))  /* initialization? */
    initglobal(ls, nvars, lastidx - nvars + 1, 0, ls->linenumber);
  fs->nactvar = cast_short(fs->nactvar + nvars);  /* activate declaration */
}


ସ୍ଥିର ଶୂନ୍ୟ globalstat (LexState *ls) {
  /* globalstat -> (GLOBAL) attrib '*'
     globalstat -> (GLOBAL) attrib NAME attrib {',' NAME attrib} */
  FuncState *fs = ls->fs;
  /* get prefixed attribute (if any); default is regular global variable */
  lu_byte defkind = getglobalattribute(ls, GDKREG);
  ଯଦି (!testnext(ls, '*'))
    globalnames(ls, defkind);
  ଅନ୍ୟଥା {
    /* use NULL as name to represent '*' entries */
    new_varkind(ls, ଖାଲି, defkind);
    fs->nactvar++;  /* activate declaration */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ globalfunc (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* globalfunc -> (GLOBAL FUNCTION) NAME body */
  expdesc var, b;
  FuncState *fs = ls->fs;
  TString *fname = str_checkname(ls);
  new_varkind(ls, fname, GDKREG);  /* declare global variable */
  fs->nactvar++;  /* enter its scope */
  buildglobal(ls, fname, &var);
  body(ls, &b, 0, ls->linenumber);  /* compile and return closure in 'b' */
  checkglobal(ls, fname, ଧାଡ଼ି);
  luaK_storevar(fs, &var, &b);
  luaK_fixline(fs, ଧାଡ଼ି);  /* definition "happens" in the first line */
}


ସ୍ଥିର ଶୂନ୍ୟ globalstatfunc (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* stat -> GLOBAL globalfunc | GLOBAL globalstat */
  luaX_next(ls);  /* skip 'global' */
  ଯଦି (testnext(ls, TK_FUNCTION))
    globalfunc(ls, ଧାଡ଼ି);
  ଅନ୍ୟଥା
    globalstat(ls);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା funcname (LexState *ls, expdesc *v) {
  /* funcname -> NAME {fieldsel} [':' NAME] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ismethod = 0;
  singlevar(ls, v);
  ଯେତେବେଳେ (ls->t.token == '.')
    fieldsel(ls, v);
  ଯଦି (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  ଫେରନ୍ତୁ ismethod;
}


ସ୍ଥିର ଶୂନ୍ୟ funcstat (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  /* funcstat -> FUNCTION funcname body */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ismethod;
  expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = funcname(ls, &v);
  check_readonly(ls, &v);
  body(ls, &b, ismethod, ଧାଡ଼ି);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, ଧାଡ଼ି);  /* definition "happens" in the first line */
}


ସ୍ଥିର ଶୂନ୍ୟ exprstat (LexState *ls) {
  /* stat -> func | assignment */
  FuncState *fs = ls->fs;
  ସଂରଚନା LHS_assign v;
  suffixedexp(ls, &v.v);
  ଯଦି (ls->t.token == '=' || ls->t.token == ',') { /* stat -> assignment ? */
    v.prev = ଖାଲି;
    restassign(ls, &v, 1);
  }
  ଅନ୍ୟଥା {  /* stat -> func */
    Instruction *inst;
    check_condition(ls, v.v.k == VCALL, "syntax error");
    inst = &getinstruction(fs, &v.v);
    SETARG_C(*inst, 1);  /* call statement uses no results */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ retstat (LexState *ls) {
  /* stat -> RETURN [explist] [';'] */
  FuncState *fs = ls->fs;
  expdesc e;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nret;  /* number of values being returned */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା first = luaY_nvarstack(fs);  /* first slot to be returned */
  ଯଦି (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;  /* return no values */
  ଅନ୍ୟଥା {
    nret = explist(ls, &e);  /* optional return values */
    ଯଦି (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      ଯଦି (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {  /* tail call? */
        SET_OPCODE(getinstruction(fs,&e), OP_TAILCALL);
        lua_assert(GETARG_A(getinstruction(fs,&e)) == luaY_nvarstack(fs));
      }
      nret = LUA_MULTRET;  /* return all values */
    }
    ଅନ୍ୟଥା {
      ଯଦି (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);  /* can use original slot */
      ଅନ୍ୟଥା {  /* values must go to the top of the stack */
        luaK_exp2nextreg(fs, &e);
        lua_assert(nret == fs->freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');  /* skip optional semicolon */
}


ସ୍ଥିର ଶୂନ୍ୟ statement (LexState *ls) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;  /* may be needed for error messages */
  enterlevel(ls);
  ବାଛ (ls->t.token) {
    ସ୍ଥିତି ';': {  /* stat -> ';' (empty statement) */
      luaX_next(ls);  /* skip ';' */
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IF: {  /* stat -> ifstat */
      ifstat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_WHILE: {  /* stat -> whilestat */
      whilestat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_DO: {  /* stat -> DO block END */
      luaX_next(ls);  /* skip DO */
      block(ls);
      check_match(ls, TK_END, TK_DO, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_FOR: {  /* stat -> forstat */
      forstat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_REPEAT: {  /* stat -> repeatstat */
      repeatstat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_FUNCTION: {  /* stat -> funcstat */
      funcstat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_LOCAL: {  /* stat -> localstat */
      luaX_next(ls);  /* skip LOCAL */
      ଯଦି (testnext(ls, TK_FUNCTION))  /* local function? */
        localfunc(ls);
      ଅନ୍ୟଥା
        localstat(ls);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_GLOBAL: {  /* stat -> globalstatfunc */
      globalstatfunc(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      labelstat(ls, str_checkname(ls), ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_RETURN: {  /* stat -> retstat */
      luaX_next(ls);  /* skip RETURN */
      retstat(ls);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_BREAK: {  /* stat -> breakstat */
      breakstat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_GOTO: {  /* stat -> 'goto' NAME */
      luaX_next(ls);  /* skip 'goto' */
      gotostat(ls, ଧାଡ଼ି);
      ଭାଙ୍ଗ;
    }
#ଯଦି LUA_COMPAT_GLOBAL
    ସ୍ଥିତି TK_NAME: {
      /* compatibility code to parse global keyword when "global"
         is not reserved */
      ଯଦି (ls->t.seminfo.ts == ls->glbn) {  /* current = "global"? */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା lk = luaX_lookahead(ls);
        ଯଦି (lk == '<' || lk == TK_NAME || lk == '*' || lk == TK_FUNCTION) {
          /* 'global <attrib>' or 'global name' or 'global *' or
             'global function' */
          globalstatfunc(ls, ଧାଡ଼ି);
          ଭାଙ୍ଗ;
        }
      }  /* else... */
    }
#endif
    /* FALLTHROUGH */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  /* stat -> func | assignment */
      exprstat(ls);
      ଭାଙ୍ଗ;
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
ସ୍ଥିର ଶୂନ୍ୟ mainfunc (LexState *ls, FuncState *fs) {
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
                       Dyndata *dyd, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstchar) {
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
  ଫେରନ୍ତୁ cl;
}

