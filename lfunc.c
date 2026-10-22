/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stddef.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, পূর্ণসংখ্যা nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, আকার_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ফেরত c;
}


LClosure *luaF_newLclosure (lua_State *L, পূর্ণসংখ্যা nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, আকার_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ফাঁকা;
  c->nupvalues = cast_byte(nupvals);
  যখন (nupvals--) c->upvals[nupvals] = ফাঁকা;
  ফেরত c;
}


/*
** fill a closure with new closed upvalues
*/
শূন্য luaF_initupvals (lua_State *L, LClosure *cl) {
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, আকার(UpVal));
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
স্থির UpVal *newupval (lua_State *L, StkId level, UpVal **পূর্ব) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, আকার(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *পরবর্তী = *পূর্ব;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.পরবর্তী = পরবর্তী;  /* link it to list of open upvalues */
  uv->u.open.previous = পূর্ব;
  যদি (পরবর্তী)
    পরবর্তী->u.open.previous = &uv->u.open.পরবর্তী;
  *পূর্ব = uv;
  যদি (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ফেরত uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == ফাঁকা);
  যখন ((p = *pp) != ফাঁকা && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    যদি (uplevel(p) == level)  /* corresponding upvalue? */
      ফেরত p;  /* return it */
    pp = &p->u.open.পরবর্তী;
  }
  /* not found: create a new upvalue after 'pp' */
  ফেরত newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
স্থির শূন্য callclosemethod (lua_State *L, TValue *obj, TValue *err, পূর্ণসংখ্যা yy) {
  StkId শীর্ষ = L->শীর্ষ.p;
  StkId func = শীর্ষ;
  ধ্রুবক TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, শীর্ষ++, tm);  /* will call metamethod... */
  setobj2s(L, শীর্ষ++, obj);  /* with 'self' as the 1st argument */
  যদি (err != ফাঁকা)  /* if there was an error... */
    setobj2s(L, শীর্ষ++, err);  /* then error object will be 2nd argument */
  L->শীর্ষ.p = শীর্ষ;  /* add function and arguments */
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
  ধ্রুবক TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  যদি (ttisnil(tm)) {  /* no metamethod? */
    পূর্ণসংখ্যা idx = cast_int(level - L->ci->func.p);  /* variable index */
    ধ্রুবক অক্ষর *vname = luaG_findlocal(L, L->ci, idx, ফাঁকা);
    যদি (vname == ফাঁকা) vname = "?";
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
                                            পূর্ণসংখ্যা yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  নির্বাচন (status) {
    ক্ষেত্র LUA_OK:
      L->শীর্ষ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    ক্ষেত্র CLOSEKTOP:  /* don't need to change top */
      errobj = ফাঁকা;  /* no error object */
      বিরতি;
    পূর্বনির্ধারিত:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      বিরতি;
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
    ফেরত;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  যখন (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


শূন্য luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.পরবর্তী;
  যদি (uv->u.open.পরবর্তী)
    uv->u.open.পরবর্তী->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
শূন্য luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  যখন ((uv = L->openupval) != ফাঁকা && uplevel(uv) >= level) {
    TValue *slot = &uv->u.মান;  /* new position for value */
    lua_assert(uplevel(uv) < L->শীর্ষ.p);
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
  যখন (tbc > L->স্তূপ.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, পূর্ণসংখ্যা yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  যখন (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ফেরত level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, আকার(Proto));
  Proto *f = gco2p(o);
  f->k = ফাঁকা;
  f->sizek = 0;
  f->p = ফাঁকা;
  f->sizep = 0;
  f->code = ফাঁকা;
  f->sizecode = 0;
  f->lineinfo = ফাঁকা;
  f->sizelineinfo = 0;
  f->abslineinfo = ফাঁকা;
  f->sizeabslineinfo = 0;
  f->upvalues = ফাঁকা;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ফাঁকা;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = ফাঁকা;
  ফেরত f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, আকার(Proto))
            + cast_uint(p->sizep) * আকার(Proto*)
            + cast_uint(p->sizek) * আকার(TValue)
            + cast_uint(p->sizelocvars) * আকার(LocVar)
            + cast_uint(p->sizeupvalues) * আকার(Upvaldesc);
  যদি (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * আকার(Instruction);
    sz += cast_uint(p->sizelineinfo) * আকার(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * আকার(Abs_সারি_Info);
  }
  ফেরত sz;
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
ধ্রুবক অক্ষর *luaF_getlocalname (ধ্রুবক Proto *f, unsigned local_সংখ্যা, পূর্ণসংখ্যা pc) {
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    যদি (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_সংখ্যা--;
      যদি (local_সংখ্যা == 0)
        ফেরত getstr(f->locvars[i].varname);
    }
  }
  ফেরত ফাঁকা;  /* not found */
}

