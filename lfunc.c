/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <stddef.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, صحیح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  بازگشت c;
}


LClosure *luaF_newLclosure (lua_State *L, صحیح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = پوچ;
  c->nupvalues = cast_byte(nupvals);
  وقتی (nupvals--) c->upvals[nupvals] = پوچ;
  بازگشت c;
}


/*
** fill a closure with new closed upvalues
*/
تهی luaF_initupvals (lua_State *L, LClosure *cl) {
  صحیح i;
  برای (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, اندازه(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.مقدار;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ایستا UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, اندازه(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  اگر (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  اگر (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  بازگشت uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == پوچ);
  وقتی ((p = *pp) != پوچ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    اگر (uplevel(p) == level)  /* corresponding upvalue? */
      بازگشت p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  بازگشت newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ایستا تهی callclosemethod (lua_State *L, TValue *obj, TValue *err, صحیح yy) {
  StkId بالاترین = L->بالاترین.p;
  StkId func = بالاترین;
  ثابت TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, بالاترین++, tm);  /* will call metamethod... */
  setobj2s(L, بالاترین++, obj);  /* with 'self' as the 1st argument */
  اگر (err != پوچ)  /* if there was an error... */
    setobj2s(L, بالاترین++, err);  /* then error object will be 2nd argument */
  L->بالاترین.p = بالاترین;  /* add function and arguments */
  اگر (yy)
    luaD_call(L, func, 0);
  وگرنه
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ایستا تهی checkclosemth (lua_State *L, StkId level) {
  ثابت TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  اگر (ttisnil(tm)) {  /* no metamethod? */
    صحیح idx = cast_int(level - L->ci->func.p);  /* variable index */
    ثابت نویسه *vname = luaG_findlocal(L, L->ci, idx, پوچ);
    اگر (vname == پوچ) vname = "?";
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
ایستا تهی prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            صحیح yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  انتخاب (status) {
    حالت LUA_OK:
      L->بالاترین.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    حالت CLOSEKTOP:  /* don't need to change top */
      errobj = پوچ;  /* no error object */
      بشکن;
    پیشفرض:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      بشکن;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
تهی luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  اگر (l_isfalse(s2v(level)))
    بازگشت;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  وقتی (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


تهی luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  اگر (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
تهی luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  وقتی ((uv = L->openupval) != پوچ && uplevel(uv) >= level) {
    TValue *slot = &uv->u.مقدار;  /* new position for value */
    lua_assert(uplevel(uv) < L->بالاترین.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    اگر (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ایستا تهی poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  وقتی (tbc > L->پشته.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, صحیح yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  وقتی (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  بازگشت level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, اندازه(Proto));
  Proto *f = gco2p(o);
  f->k = پوچ;
  f->sizek = 0;
  f->p = پوچ;
  f->sizep = 0;
  f->code = پوچ;
  f->sizecode = 0;
  f->lineinfo = پوچ;
  f->sizelineinfo = 0;
  f->abslineinfo = پوچ;
  f->sizeabslineinfo = 0;
  f->upvalues = پوچ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = پوچ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = پوچ;
  بازگشت f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, اندازه(Proto))
            + cast_uint(p->sizep) * اندازه(Proto*)
            + cast_uint(p->sizek) * اندازه(TValue)
            + cast_uint(p->sizelocvars) * اندازه(LocVar)
            + cast_uint(p->sizeupvalues) * اندازه(Upvaldesc);
  اگر (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * اندازه(Instruction);
    sz += cast_uint(p->sizelineinfo) * اندازه(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * اندازه(Abs_سطر_Info);
  }
  بازگشت sz;
}


تهی luaF_freeproto (lua_State *L, Proto *f) {
  اگر (!(f->flag & PF_FIXED)) {
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
ثابت نویسه *luaF_getlocalname (ثابت Proto *f, unsigned local_عدد, صحیح pc) {
  صحیح i;
  برای (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    اگر (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_عدد--;
      اگر (local_عدد == 0)
        بازگشت getstr(f->locvars[i].varname);
    }
  }
  بازگشت پوچ;  /* not found */
}

