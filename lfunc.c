/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

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



CClosure *luaF_newCclosure (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ಪ್ರತ್ಯಾಗಮ c;
}


LClosure *luaF_newLclosure (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ರಿಕ್ತ;
  c->nupvalues = cast_byte(nupvals);
  ಯಾವತ್ (nupvals--) c->upvals[nupvals] = ರಿಕ್ತ;
  ಪ್ರತ್ಯಾಗಮ c;
}


/*
** fill a closure with new closed upvalues
*/
ಶೂನ್ಯ luaF_initupvals (lua_State *L, LClosure *cl) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, ಪರಿಮಾಣ(UpVal));
    UpVal *uv = gco2upv(o);
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
ಸ್ಥಿರ UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, ಪರಿಮಾಣ(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  ಯದಿ (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  ಯದಿ (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ಪ್ರತ್ಯಾಗಮ uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == ರಿಕ್ತ);
  ಯಾವತ್ ((p = *pp) != ರಿಕ್ತ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    ಯದಿ (uplevel(p) == level)  /* corresponding upvalue? */
      ಪ್ರತ್ಯಾಗಮ p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  ಪ್ರತ್ಯಾಗಮ newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ callclosemethod (lua_State *L, TValue *obj, TValue *err, ಪೂರ್ಣಾಙ್ಕ yy) {
  StkId ತುದಿ = L->ತುದಿ.p;
  StkId func = ತುದಿ;
  ಧ್ರುವ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, ತುದಿ++, tm);  /* will call metamethod... */
  setobj2s(L, ತುದಿ++, obj);  /* with 'self' as the 1st argument */
  ಯದಿ (err != ರಿಕ್ತ)  /* if there was an error... */
    setobj2s(L, ತುದಿ++, err);  /* then error object will be 2nd argument */
  L->ತುದಿ.p = ತುದಿ;  /* add function and arguments */
  ಯದಿ (yy)
    luaD_call(L, func, 0);
  ಅನ್ಯಥಾ
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkclosemth (lua_State *L, StkId level) {
  ಧ್ರುವ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  ಯದಿ (ttisnil(tm)) {  /* no metamethod? */
    ಪೂರ್ಣಾಙ್ಕ idx = cast_int(level - L->ci->func.p);  /* variable index */
    ಧ್ರುವ ವರ್ಣ *vname = luaG_findlocal(L, L->ci, idx, ರಿಕ್ತ);
    ಯದಿ (vname == ರಿಕ್ತ) vname = "?";
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
ಸ್ಥಿರ ಶೂನ್ಯ prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            ಪೂರ್ಣಾಙ್ಕ yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ವಿಕಲ್ಪ (status) {
    ಪಕ್ಷ LUA_OK:
      L->ತುದಿ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    ಪಕ್ಷ CLOSEKTOP:  /* don't need to change top */
      errobj = ರಿಕ್ತ;  /* no error object */
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      ಭಙ್ಗ;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
ಶೂನ್ಯ luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  ಯದಿ (l_isfalse(s2v(level)))
    ಪ್ರತ್ಯಾಗಮ;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  ಯಾವತ್ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


ಶೂನ್ಯ luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  ಯದಿ (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
ಶೂನ್ಯ luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  ಯಾವತ್ ((uv = L->openupval) != ರಿಕ್ತ && uplevel(uv) >= level) {
    TValue *slot = &uv->u.ಮೌಲ್ಯ;  /* new position for value */
    lua_assert(uplevel(uv) < L->ತುದಿ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    ಯದಿ (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
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
  ಯಾವತ್ (tbc > L->ರಾಶಿ.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, ಪೂರ್ಣಾಙ್ಕ yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  ಯಾವತ್ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ಪ್ರತ್ಯಾಗಮ level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, ಪರಿಮಾಣ(Proto));
  Proto *f = gco2p(o);
  f->k = ರಿಕ್ತ;
  f->sizek = 0;
  f->p = ರಿಕ್ತ;
  f->sizep = 0;
  f->code = ರಿಕ್ತ;
  f->sizecode = 0;
  f->lineinfo = ರಿಕ್ತ;
  f->sizelineinfo = 0;
  f->abslineinfo = ರಿಕ್ತ;
  f->sizeabslineinfo = 0;
  f->upvalues = ರಿಕ್ತ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ರಿಕ್ತ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = ರಿಕ್ತ;
  ಪ್ರತ್ಯಾಗಮ f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, ಪರಿಮಾಣ(Proto))
            + cast_uint(p->sizep) * ಪರಿಮಾಣ(Proto*)
            + cast_uint(p->sizek) * ಪರಿಮಾಣ(TValue)
            + cast_uint(p->sizelocvars) * ಪರಿಮಾಣ(LocVar)
            + cast_uint(p->sizeupvalues) * ಪರಿಮಾಣ(Upvaldesc);
  ಯದಿ (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * ಪರಿಮಾಣ(Instruction);
    sz += cast_uint(p->sizelineinfo) * ಪರಿಮಾಣ(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * ಪರಿಮಾಣ(Abs_ಸಾಲು_Info);
  }
  ಪ್ರತ್ಯಾಗಮ sz;
}


ಶೂನ್ಯ luaF_freeproto (lua_State *L, Proto *f) {
  ಯದಿ (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->code, cast_sizet(f->sizecode));
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
ಧ್ರುವ ವರ್ಣ *luaF_getlocalname (ಧ್ರುವ Proto *f, unsigned local_ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಙ್ಕ pc) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    ಯದಿ (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_ಸಂಖ್ಯೆ--;
      ಯದಿ (local_ಸಂಖ್ಯೆ == 0)
        ಪ್ರತ್ಯಾಗಮ getstr(f->locvars[i].varname);
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* not found */
}

