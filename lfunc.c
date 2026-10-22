/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#تعریف_کړه lfunc_c
#تعریف_کړه LUA_CORE

#ضمن "lprefix.h"


#ضمن <stddef.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lgc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, صحيح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, اندازه_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  أرجع c;
}


LClosure *luaF_newLclosure (lua_State *L, صحيح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, اندازه_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = عدم;
  c->nupvalues = cast_byte(nupvals);
  طالما (nupvals--) c->upvals[nupvals] = عدم;
  أرجع c;
}


/*
** fill a closure with new closed upvalues
*/
فراغ luaF_initupvals (lua_State *L, LClosure *cl) {
  صحيح i;
  لكل (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, حجم(پورته_Val));
    پورته_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.ارزښت;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ساكن پورته_Val *newupval (lua_State *L, StkId level, پورته_Val **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, حجم(پورته_Val));
  پورته_Val *uv = gco2upv(o);
  پورته_Val *بل = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.بل = بل;  /* link it to list of open upvalues */
  uv->u.open.پخوانی = prev;
  إذا (بل)
    بل->u.open.پخوانی = &uv->u.open.بل;
  *prev = uv;
  إذا (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  أرجع uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
پورته_Val *luaF_findupval (lua_State *L, StkId level) {
  پورته_Val **pp = &L->openupval;
  پورته_Val *p;
  lua_assert(isintwups(L) || L->openupval == عدم);
  طالما ((p = *pp) != عدم && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    إذا (uplevel(p) == level)  /* corresponding upvalue? */
      أرجع p;  /* return it */
    pp = &p->u.open.بل;
  }
  /* not found: create a new upvalue after 'pp' */
  أرجع newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ساكن فراغ callclosemethod (lua_State *L, TValue *obj, TValue *err, صحيح yy) {
  StkId سر = L->سر.p;
  StkId func = سر;
  ثابت TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, سر++, tm);  /* will call metamethod... */
  setobj2s(L, سر++, obj);  /* with 'self' as the 1st argument */
  إذا (err != عدم)  /* if there was an error... */
    setobj2s(L, سر++, err);  /* then error object will be 2nd argument */
  L->سر.p = سر;  /* add function and arguments */
  إذا (yy)
    luaD_call(L, func, 0);
  وإلا
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ساكن فراغ checkclosemth (lua_State *L, StkId level) {
  ثابت TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  إذا (ttisnil(tm)) {  /* no metamethod? */
    صحيح idx = cast_int(level - L->ci->func.p);  /* variable index */
    ثابت حرف *vname = luaG_findlocal(L, L->ci, idx, عدم);
    إذا (vname == عدم) vname = "?";
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
ساكن فراغ prepcallclosemth (lua_State *L, StkId level, TStatus حالت,
                                            صحيح yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  بدل (حالت) {
    حالة LUA_OK:
      L->سر.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    حالة CLOSEKTOP:  /* don't need to change top */
      errobj = عدم;  /* no error object */
      اكسر;
    افتراضي:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, حالت, level + 1);  /* set error object */
      اكسر;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#تعریف_کړه MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
فراغ luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  إذا (l_isfalse(s2v(level)))
    أرجع;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  طالما (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(بینښه لنډ, level - L->tbclist.p);
  L->tbclist.p = level;
}


فراغ luaF_unlinkupval (پورته_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.پخوانی = uv->u.open.بل;
  إذا (uv->u.open.بل)
    uv->u.open.بل->u.open.پخوانی = uv->u.open.پخوانی;
}


/*
** Close all upvalues up to the given stack level.
*/
فراغ luaF_closeupval (lua_State *L, StkId level) {
  پورته_Val *uv;
  طالما ((uv = L->openupval) != عدم && uplevel(uv) >= level) {
    TValue *خونه = &uv->u.ارزښت;  /* new position for value */
    lua_assert(uplevel(uv) < L->سر.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, خونه, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = خونه;  /* now current value lives here */
    إذا (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, خونه);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ساكن فراغ poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  طالما (tbc > L->ډېری.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus حالت, صحيح yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  طالما (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, حالت, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  أرجع level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, حجم(Proto));
  Proto *f = gco2p(o);
  f->k = عدم;
  f->sizek = 0;
  f->p = عدم;
  f->sizep = 0;
  f->کوډ = عدم;
  f->sizecode = 0;
  f->lineinfo = عدم;
  f->sizelineinfo = 0;
  f->abslineinfo = عدم;
  f->sizeabslineinfo = 0;
  f->upvalues = عدم;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = عدم;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->سرچینه = عدم;
  أرجع f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, حجم(Proto))
            + cast_uint(p->sizep) * حجم(Proto*)
            + cast_uint(p->sizek) * حجم(TValue)
            + cast_uint(p->sizelocvars) * حجم(LocVar)
            + cast_uint(p->sizeupvalues) * حجم(Upvaldesc);
  إذا (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * حجم(Instruction);
    sz += cast_uint(p->sizelineinfo) * حجم(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * حجم(Abs_کرښه_Info);
  }
  أرجع sz;
}


فراغ luaF_freeproto (lua_State *L, Proto *f) {
  إذا (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->کوډ, cast_sizet(f->sizecode));
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
ثابت حرف *luaF_getlocalname (ثابت Proto *f, بینښه local_شمېره, صحيح pc) {
  صحيح i;
  لكل (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    إذا (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_شمېره--;
      إذا (local_شمېره == 0)
        أرجع getstr(f->locvars[i].varname);
    }
  }
  أرجع عدم;  /* not found */
}

