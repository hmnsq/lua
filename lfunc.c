/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stddef.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, முழு nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  திரும்பு c;
}


LClosure *luaF_newLclosure (lua_State *L, முழு nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = வெற்று;
  c->nupvalues = cast_byte(nupvals);
  போது (nupvals--) c->upvals[nupvals] = வெற்று;
  திரும்பு c;
}


/*
** fill a closure with new closed upvalues
*/
வெறுமை luaF_initupvals (lua_State *L, LClosure *cl) {
  முழு i;
  க்கு (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, அளவு(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.மதிப்பு;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
நிலையான UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, அளவு(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  எனில் (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  எனில் (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  திரும்பு uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == வெற்று);
  போது ((p = *pp) != வெற்று && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    எனில் (uplevel(p) == level)  /* corresponding upvalue? */
      திரும்பு p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  திரும்பு newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
நிலையான வெறுமை callclosemethod (lua_State *L, TValue *obj, TValue *err, முழு yy) {
  StkId உச்சி = L->உச்சி.p;
  StkId func = உச்சி;
  மாறிலி TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, உச்சி++, tm);  /* will call metamethod... */
  setobj2s(L, உச்சி++, obj);  /* with 'self' as the 1st argument */
  எனில் (err != வெற்று)  /* if there was an error... */
    setobj2s(L, உச்சி++, err);  /* then error object will be 2nd argument */
  L->உச்சி.p = உச்சி;  /* add function and arguments */
  எனில் (yy)
    luaD_call(L, func, 0);
  இல்லையெனில்
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
நிலையான வெறுமை checkclosemth (lua_State *L, StkId level) {
  மாறிலி TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  எனில் (ttisnil(tm)) {  /* no metamethod? */
    முழு idx = cast_int(level - L->ci->func.p);  /* variable index */
    மாறிலி எழுத்து *vname = luaG_findlocal(L, L->ci, idx, வெற்று);
    எனில் (vname == வெற்று) vname = "?";
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
நிலையான வெறுமை prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            முழு yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  தேர்வு (status) {
    நிலை LUA_OK:
      L->உச்சி.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    நிலை CLOSEKTOP:  /* don't need to change top */
      errobj = வெற்று;  /* no error object */
      நிறுத்து;
    இயல்பு:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      நிறுத்து;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
வெறுமை luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  எனில் (l_isfalse(s2v(level)))
    திரும்பு;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  போது (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


வெறுமை luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  எனில் (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
வெறுமை luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  போது ((uv = L->openupval) != வெற்று && uplevel(uv) >= level) {
    TValue *slot = &uv->u.மதிப்பு;  /* new position for value */
    lua_assert(uplevel(uv) < L->உச்சி.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    எனில் (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
நிலையான வெறுமை poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  போது (tbc > L->அடுக்கு.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, முழு yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  போது (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  திரும்பு level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, அளவு(Proto));
  Proto *f = gco2p(o);
  f->k = வெற்று;
  f->sizek = 0;
  f->p = வெற்று;
  f->sizep = 0;
  f->code = வெற்று;
  f->sizecode = 0;
  f->lineinfo = வெற்று;
  f->sizelineinfo = 0;
  f->abslineinfo = வெற்று;
  f->sizeabslineinfo = 0;
  f->upvalues = வெற்று;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = வெற்று;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = வெற்று;
  திரும்பு f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, அளவு(Proto))
            + cast_uint(p->sizep) * அளவு(Proto*)
            + cast_uint(p->sizek) * அளவு(TValue)
            + cast_uint(p->sizelocvars) * அளவு(LocVar)
            + cast_uint(p->sizeupvalues) * அளவு(Upvaldesc);
  எனில் (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * அளவு(Instruction);
    sz += cast_uint(p->sizelineinfo) * அளவு(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * அளவு(Abs_வரி_Info);
  }
  திரும்பு sz;
}


வெறுமை luaF_freeproto (lua_State *L, Proto *f) {
  எனில் (!(f->flag & PF_FIXED)) {
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
மாறிலி எழுத்து *luaF_getlocalname (மாறிலி Proto *f, unsigned local_எண், முழு pc) {
  முழு i;
  க்கு (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    எனில் (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_எண்--;
      எனில் (local_எண் == 0)
        திரும்பு getstr(f->locvars[i].varname);
    }
  }
  திரும்பு வெற்று;  /* not found */
}

