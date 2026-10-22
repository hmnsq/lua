/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lfunc_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, ಪೂರ್ಣಾಂಕ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, ಗಾತ್ರ_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ಹಿಂತಿರುಗಿ c;
}


LClosure *luaF_newLclosure (lua_State *L, ಪೂರ್ಣಾಂಕ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, ಗಾತ್ರ_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ಖಾಲಿ;
  c->nupvalues = cast_byte(nupvals);
  ಆಗ (nupvals--) c->upvals[nupvals] = ಖಾಲಿ;
  ಹಿಂತಿರುಗಿ c;
}


/*
** fill a closure with new closed upvalues
*/
ಶೂನ್ಯ luaF_initupvals (lua_State *L, LClosure *cl) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, ಗಾತ್ರ(ಮೇಲೆ_Val));
    ಮೇಲೆ_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.ಮೌಲ್ಯ;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ಸ್ಥಿರ ಮೇಲೆ_Val *newupval (lua_State *L, StkId level, ಮೇಲೆ_Val **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, ಗಾತ್ರ(ಮೇಲೆ_Val));
  ಮೇಲೆ_Val *uv = gco2upv(o);
  ಮೇಲೆ_Val *ಮುಂದಿನ = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.ಮುಂದಿನ = ಮುಂದಿನ;  /* link it to list of open upvalues */
  uv->u.open.ಹಿಂದಿನ = prev;
  ಆದರೆ (ಮುಂದಿನ)
    ಮುಂದಿನ->u.open.ಹಿಂದಿನ = &uv->u.open.ಮುಂದಿನ;
  *prev = uv;
  ಆದರೆ (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ಹಿಂತಿರುಗಿ uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
ಮೇಲೆ_Val *luaF_findupval (lua_State *L, StkId level) {
  ಮೇಲೆ_Val **pp = &L->openupval;
  ಮೇಲೆ_Val *p;
  lua_assert(isintwups(L) || L->openupval == ಖಾಲಿ);
  ಆಗ ((p = *pp) != ಖಾಲಿ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    ಆದರೆ (uplevel(p) == level)  /* corresponding upvalue? */
      ಹಿಂತಿರುಗಿ p;  /* return it */
    pp = &p->u.open.ಮುಂದಿನ;
  }
  /* not found: create a new upvalue after 'pp' */
  ಹಿಂತಿರುಗಿ newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ callclosemethod (lua_State *L, TValue *obj, TValue *err, ಪೂರ್ಣಾಂಕ yy) {
  StkId ತುದಿ = L->ತುದಿ.p;
  StkId func = ತುದಿ;
  ಸ್ಥಿರಾಂಕ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, ತುದಿ++, tm);  /* will call metamethod... */
  setobj2s(L, ತುದಿ++, obj);  /* with 'self' as the 1st argument */
  ಆದರೆ (err != ಖಾಲಿ)  /* if there was an error... */
    setobj2s(L, ತುದಿ++, err);  /* then error object will be 2nd argument */
  L->ತುದಿ.p = ತುದಿ;  /* add function and arguments */
  ಆದರೆ (yy)
    luaD_call(L, func, 0);
  ಇಲ್ಲದಿದ್ದರೆ
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkclosemth (lua_State *L, StkId level) {
  ಸ್ಥಿರಾಂಕ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  ಆದರೆ (ttisnil(tm)) {  /* no metamethod? */
    ಪೂರ್ಣಾಂಕ idx = cast_int(level - L->ci->func.p);  /* variable index */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vname = luaG_findlocal(L, L->ci, idx, ಖಾಲಿ);
    ಆದರೆ (vname == ಖಾಲಿ) vname = "?";
    luaG_runerror(L, "variable '%s' got a non-closable value", vname);
  }
}


/*
** Prepare and call a closing method.
** If status is CLOSEKTOP, the call to the closing method will be pushed
** at the top of the stack. Otherwise, values can be pushed right after
** the 'level' of the upvalue being closed, as everything after that
** won't be used again.
*/
ಸ್ಥಿರ ಶೂನ್ಯ prepcallclosemth (lua_State *L, StkId level, TStatus ಸ್ಥಿತಿ_2,
                                            ಪೂರ್ಣಾಂಕ yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ಆಯ್ಕೆ (ಸ್ಥಿತಿ_2) {
    ಸ್ಥಿತಿ LUA_OK:
      L->ತುದಿ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    ಸ್ಥಿತಿ CLOSEKTOP:  /* don't need to change top */
      errobj = ಖಾಲಿ;  /* no error object */
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, ಸ್ಥಿತಿ_2, level + 1);  /* set error object */
      ನಿಲ್ಲಿಸು;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
ಶೂನ್ಯ luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  ಆದರೆ (l_isfalse(s2v(level)))
    ಹಿಂತಿರುಗಿ;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  ಆಗ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ, level - L->tbclist.p);
  L->tbclist.p = level;
}


ಶೂನ್ಯ luaF_unlinkupval (ಮೇಲೆ_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.ಹಿಂದಿನ = uv->u.open.ಮುಂದಿನ;
  ಆದರೆ (uv->u.open.ಮುಂದಿನ)
    uv->u.open.ಮುಂದಿನ->u.open.ಹಿಂದಿನ = uv->u.open.ಹಿಂದಿನ;
}


/*
** Close all upvalues up to the given stack level.
*/
ಶೂನ್ಯ luaF_closeupval (lua_State *L, StkId level) {
  ಮೇಲೆ_Val *uv;
  ಆಗ ((uv = L->openupval) != ಖಾಲಿ && uplevel(uv) >= level) {
    TValue *ಸ್ಥಾನ = &uv->u.ಮೌಲ್ಯ;  /* new position for value */
    lua_assert(uplevel(uv) < L->ತುದಿ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, ಸ್ಥಾನ, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = ಸ್ಥಾನ;  /* now current value lives here */
    ಆದರೆ (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, ಸ್ಥಾನ);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  ಆಗ (tbc > L->ರಾಶಿ.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus ಸ್ಥಿತಿ_2, ಪೂರ್ಣಾಂಕ yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  ಆಗ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, ಸ್ಥಿತಿ_2, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ಹಿಂತಿರುಗಿ level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, ಗಾತ್ರ(Proto));
  Proto *f = gco2p(o);
  f->k = ಖಾಲಿ;
  f->sizek = 0;
  f->p = ಖಾಲಿ;
  f->sizep = 0;
  f->ಸಂಕೇತ_2 = ಖಾಲಿ;
  f->sizecode = 0;
  f->lineinfo = ಖಾಲಿ;
  f->sizelineinfo = 0;
  f->abslineinfo = ಖಾಲಿ;
  f->sizeabslineinfo = 0;
  f->upvalues = ಖಾಲಿ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ಖಾಲಿ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->ಮೂಲ = ಖಾಲಿ;
  ಹಿಂತಿರುಗಿ f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, ಗಾತ್ರ(Proto))
            + cast_uint(p->sizep) * ಗಾತ್ರ(Proto*)
            + cast_uint(p->sizek) * ಗಾತ್ರ(TValue)
            + cast_uint(p->sizelocvars) * ಗಾತ್ರ(LocVar)
            + cast_uint(p->sizeupvalues) * ಗಾತ್ರ(Upvaldesc);
  ಆದರೆ (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * ಗಾತ್ರ(Instruction);
    sz += cast_uint(p->sizelineinfo) * ಗಾತ್ರ(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * ಗಾತ್ರ(Abs_ಸಾಲು_Info);
  }
  ಹಿಂತಿರುಗಿ sz;
}


ಶೂನ್ಯ luaF_freeproto (lua_State *L, Proto *f) {
  ಆದರೆ (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->ಸಂಕೇತ_2, cast_sizet(f->sizecode));
    luaM_freearray(L, f->lineinfo, cast_sizet(f->sizelineinfo));
    luaM_freearray(L, f->abslineinfo, cast_sizet(f->sizeabslineinfo));
  }
  luaM_freearray(L, f->p, cast_sizet(f->sizep));
  luaM_freearray(L, f->k, cast_sizet(f->sizek));
  luaM_freearray(L, f->locvars, cast_sizet(f->sizelocvars));
  luaM_freearray(L, f->upvalues, cast_sizet(f->sizeupvalues));
  luaM_free(L, f);
}


/*
** Look for n-th local variable at line 'line' in function 'func'.
** Returns NULL if not found.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaF_getlocalname (ಸ್ಥಿರಾಂಕ Proto *f, ಚಿಹ್ನೆರಹಿತ local_ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಂಕ pc) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    ಆದರೆ (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_ಸಂಖ್ಯೆ--;
      ಆದರೆ (local_ಸಂಖ್ಯೆ == 0)
        ಹಿಂತಿರುಗಿ getstr(f->locvars[i].varname);
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* not found */
}

