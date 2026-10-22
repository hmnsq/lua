/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <stddef.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lfunc.h"
#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, পূর্ণাঙ্ক nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ঘূৰাই c;
}


LClosure *luaF_newLclosure (lua_State *L, পূর্ণাঙ্ক nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = রিক্ত;
  c->nupvalues = cast_byte(nupvals);
  যেতিয়ালৈ (nupvals--) c->upvals[nupvals] = রিক্ত;
  ঘূৰাই c;
}


/*
** fill a closure with new closed upvalues
*/
শূন্য luaF_initupvals (lua_State *L, LClosure *cl) {
  পূর্ণাঙ্ক i;
  বাবে (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, পরিমাণ(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.মান;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
স্থির UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, পরিমাণ(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  যদি (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  যদি (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ঘূৰাই uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == রিক্ত);
  যেতিয়ালৈ ((p = *pp) != রিক্ত && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    যদি (uplevel(p) == level)  /* corresponding upvalue? */
      ঘূৰাই p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  ঘূৰাই newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
স্থির শূন্য callclosemethod (lua_State *L, TValue *obj, TValue *err, পূর্ণাঙ্ক yy) {
  StkId শীৰ্ষ = L->শীৰ্ষ.p;
  StkId func = শীৰ্ষ;
  ধ্রুৱ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, শীৰ্ষ++, tm);  /* will call metamethod... */
  setobj2s(L, শীৰ্ষ++, obj);  /* with 'self' as the 1st argument */
  যদি (err != রিক্ত)  /* if there was an error... */
    setobj2s(L, শীৰ্ষ++, err);  /* then error object will be 2nd argument */
  L->শীৰ্ষ.p = শীৰ্ষ;  /* add function and arguments */
  যদি (yy)
    luaD_call(L, func, 0);
  অন্যথা
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
স্থির শূন্য checkclosemth (lua_State *L, StkId level) {
  ধ্রুৱ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  যদি (ttisnil(tm)) {  /* no metamethod? */
    পূর্ণাঙ্ক idx = cast_int(level - L->ci->func.p);  /* variable index */
    ধ্রুৱ ৱর্ণ *vname = luaG_findlocal(L, L->ci, idx, রিক্ত);
    যদি (vname == রিক্ত) vname = "?";
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
স্থির শূন্য prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            পূর্ণাঙ্ক yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ৱিকল্প (status) {
    পক্ষ LUA_OK:
      L->শীৰ্ষ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    পক্ষ CLOSEKTOP:  /* don't need to change top */
      errobj = রিক্ত;  /* no error object */
      ভঙ্গ;
    স্ৱভাৱ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      ভঙ্গ;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
শূন্য luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  যদি (l_isfalse(s2v(level)))
    ঘূৰাই;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  যেতিয়ালৈ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


শূন্য luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  যদি (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
শূন্য luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  যেতিয়ালৈ ((uv = L->openupval) != রিক্ত && uplevel(uv) >= level) {
    TValue *slot = &uv->u.মান;  /* new position for value */
    lua_assert(uplevel(uv) < L->শীৰ্ষ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    যদি (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
স্থির শূন্য poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  যেতিয়ালৈ (tbc > L->স্তূপ.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, পূর্ণাঙ্ক yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  যেতিয়ালৈ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ঘূৰাই level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, পরিমাণ(Proto));
  Proto *f = gco2p(o);
  f->k = রিক্ত;
  f->sizek = 0;
  f->p = রিক্ত;
  f->sizep = 0;
  f->code = রিক্ত;
  f->sizecode = 0;
  f->lineinfo = রিক্ত;
  f->sizelineinfo = 0;
  f->abslineinfo = রিক্ত;
  f->sizeabslineinfo = 0;
  f->upvalues = রিক্ত;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = রিক্ত;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = রিক্ত;
  ঘূৰাই f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, পরিমাণ(Proto))
            + cast_uint(p->sizep) * পরিমাণ(Proto*)
            + cast_uint(p->sizek) * পরিমাণ(TValue)
            + cast_uint(p->sizelocvars) * পরিমাণ(LocVar)
            + cast_uint(p->sizeupvalues) * পরিমাণ(Upvaldesc);
  যদি (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * পরিমাণ(Instruction);
    sz += cast_uint(p->sizelineinfo) * পরিমাণ(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * পরিমাণ(Abs_সাৰি_Info);
  }
  ঘূৰাই sz;
}


শূন্য luaF_freeproto (lua_State *L, Proto *f) {
  যদি (!(f->flag & PF_FIXED)) {
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
ধ্রুৱ ৱর্ণ *luaF_getlocalname (ধ্রুৱ Proto *f, unsigned local_সংখ্যা, পূর্ণাঙ্ক pc) {
  পূর্ণাঙ্ক i;
  বাবে (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    যদি (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_সংখ্যা--;
      যদি (local_সংখ্যা == 0)
        ঘূৰাই getstr(f->locvars[i].varname);
    }
  }
  ঘূৰাই রিক্ত;  /* not found */
}

