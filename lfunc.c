/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lfunc_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, ਪੂਰਨ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, ਮਾਪ_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ਵਾਪਸ c;
}


LClosure *luaF_newLclosure (lua_State *L, ਪੂਰਨ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, ਮਾਪ_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ਨਿਰਾ;
  c->nupvalues = cast_byte(nupvals);
  ਜਦੋਂ (nupvals--) c->upvals[nupvals] = ਨਿਰਾ;
  ਵਾਪਸ c;
}


/*
** fill a closure with new closed upvalues
*/
ਖਾਲੀ luaF_initupvals (lua_State *L, LClosure *cl) {
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, ਆਕਾਰ(ਉੱਪਰ_Val));
    ਉੱਪਰ_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.ਮੁੱਲ;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ਸਥਿਰ ਉੱਪਰ_Val *newupval (lua_State *L, StkId level, ਉੱਪਰ_Val **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, ਆਕਾਰ(ਉੱਪਰ_Val));
  ਉੱਪਰ_Val *uv = gco2upv(o);
  ਉੱਪਰ_Val *ਅਗਲਾ = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.ਅਗਲਾ = ਅਗਲਾ;  /* link it to list of open upvalues */
  uv->u.open.ਪਿਛਲਾ = prev;
  ਜੇ (ਅਗਲਾ)
    ਅਗਲਾ->u.open.ਪਿਛਲਾ = &uv->u.open.ਅਗਲਾ;
  *prev = uv;
  ਜੇ (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ਵਾਪਸ uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
ਉੱਪਰ_Val *luaF_findupval (lua_State *L, StkId level) {
  ਉੱਪਰ_Val **pp = &L->openupval;
  ਉੱਪਰ_Val *p;
  lua_assert(isintwups(L) || L->openupval == ਨਿਰਾ);
  ਜਦੋਂ ((p = *pp) != ਨਿਰਾ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    ਜੇ (uplevel(p) == level)  /* corresponding upvalue? */
      ਵਾਪਸ p;  /* return it */
    pp = &p->u.open.ਅਗਲਾ;
  }
  /* not found: create a new upvalue after 'pp' */
  ਵਾਪਸ newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ਸਥਿਰ ਖਾਲੀ callclosemethod (lua_State *L, TValue *obj, TValue *err, ਪੂਰਨ yy) {
  StkId ਸਿਖਰ = L->ਸਿਖਰ.p;
  StkId func = ਸਿਖਰ;
  ਅਚਲ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, ਸਿਖਰ++, tm);  /* will call metamethod... */
  setobj2s(L, ਸਿਖਰ++, obj);  /* with 'self' as the 1st argument */
  ਜੇ (err != ਨਿਰਾ)  /* if there was an error... */
    setobj2s(L, ਸਿਖਰ++, err);  /* then error object will be 2nd argument */
  L->ਸਿਖਰ.p = ਸਿਖਰ;  /* add function and arguments */
  ਜੇ (yy)
    luaD_call(L, func, 0);
  ਨਹੀਂਤਾਂ
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ਸਥਿਰ ਖਾਲੀ checkclosemth (lua_State *L, StkId level) {
  ਅਚਲ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  ਜੇ (ttisnil(tm)) {  /* no metamethod? */
    ਪੂਰਨ idx = cast_int(level - L->ci->func.p);  /* variable index */
    ਅਚਲ ਅੱਖਰ *vname = luaG_findlocal(L, L->ci, idx, ਨਿਰਾ);
    ਜੇ (vname == ਨਿਰਾ) vname = "?";
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
ਸਥਿਰ ਖਾਲੀ prepcallclosemth (lua_State *L, StkId level, TStatus ਸਥਿਤੀ,
                                            ਪੂਰਨ yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ਚੋਣ (ਸਥਿਤੀ) {
    ਹਾਲਤ LUA_OK:
      L->ਸਿਖਰ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    ਹਾਲਤ CLOSEKTOP:  /* don't need to change top */
      errobj = ਨਿਰਾ;  /* no error object */
      ਰੋਕੋ;
    ਮੂਲ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, ਸਥਿਤੀ, level + 1);  /* set error object */
      ਰੋਕੋ;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
ਖਾਲੀ luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  ਜੇ (l_isfalse(s2v(level)))
    ਵਾਪਸ;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  ਜਦੋਂ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ, level - L->tbclist.p);
  L->tbclist.p = level;
}


ਖਾਲੀ luaF_unlinkupval (ਉੱਪਰ_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.ਪਿਛਲਾ = uv->u.open.ਅਗਲਾ;
  ਜੇ (uv->u.open.ਅਗਲਾ)
    uv->u.open.ਅਗਲਾ->u.open.ਪਿਛਲਾ = uv->u.open.ਪਿਛਲਾ;
}


/*
** Close all upvalues up to the given stack level.
*/
ਖਾਲੀ luaF_closeupval (lua_State *L, StkId level) {
  ਉੱਪਰ_Val *uv;
  ਜਦੋਂ ((uv = L->openupval) != ਨਿਰਾ && uplevel(uv) >= level) {
    TValue *ਥਾਂ = &uv->u.ਮੁੱਲ;  /* new position for value */
    lua_assert(uplevel(uv) < L->ਸਿਖਰ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, ਥਾਂ, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = ਥਾਂ;  /* now current value lives here */
    ਜੇ (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, ਥਾਂ);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ਸਥਿਰ ਖਾਲੀ poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  ਜਦੋਂ (tbc > L->ਢੇਰ.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus ਸਥਿਤੀ, ਪੂਰਨ yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  ਜਦੋਂ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, ਸਥਿਤੀ, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ਵਾਪਸ level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, ਆਕਾਰ(Proto));
  Proto *f = gco2p(o);
  f->k = ਨਿਰਾ;
  f->sizek = 0;
  f->p = ਨਿਰਾ;
  f->sizep = 0;
  f->ਕੋਡ = ਨਿਰਾ;
  f->sizecode = 0;
  f->lineinfo = ਨਿਰਾ;
  f->sizelineinfo = 0;
  f->abslineinfo = ਨਿਰਾ;
  f->sizeabslineinfo = 0;
  f->upvalues = ਨਿਰਾ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ਨਿਰਾ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->ਸਰੋਤ = ਨਿਰਾ;
  ਵਾਪਸ f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, ਆਕਾਰ(Proto))
            + cast_uint(p->sizep) * ਆਕਾਰ(Proto*)
            + cast_uint(p->sizek) * ਆਕਾਰ(TValue)
            + cast_uint(p->sizelocvars) * ਆਕਾਰ(LocVar)
            + cast_uint(p->sizeupvalues) * ਆਕਾਰ(Upvaldesc);
  ਜੇ (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * ਆਕਾਰ(Instruction);
    sz += cast_uint(p->sizelineinfo) * ਆਕਾਰ(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * ਆਕਾਰ(Abs_ਸਤਰ_Info);
  }
  ਵਾਪਸ sz;
}


ਖਾਲੀ luaF_freeproto (lua_State *L, Proto *f) {
  ਜੇ (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->ਕੋਡ, cast_sizet(f->sizecode));
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
ਅਚਲ ਅੱਖਰ *luaF_getlocalname (ਅਚਲ Proto *f, ਬਿਨਾਨਿਸ਼ਾਨੀ local_ਸੰਖਿਆ, ਪੂਰਨ pc) {
  ਪੂਰਨ i;
  ਲਈ (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    ਜੇ (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_ਸੰਖਿਆ--;
      ਜੇ (local_ਸੰਖਿਆ == 0)
        ਵਾਪਸ getstr(f->locvars[i].varname);
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* not found */
}

