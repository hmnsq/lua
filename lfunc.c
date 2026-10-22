/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stddef.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  ଫେରନ୍ତୁ c;
}


LClosure *luaF_newLclosure (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ଖାଲି;
  c->nupvalues = cast_byte(nupvals);
  ଯେତେବେଳେ (nupvals--) c->upvals[nupvals] = ଖାଲି;
  ଫେରନ୍ତୁ c;
}


/*
** fill a closure with new closed upvalues
*/
ଶୂନ୍ୟ luaF_initupvals (lua_State *L, LClosure *cl) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, ଆକାର(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.ମୂଲ୍ୟ;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ସ୍ଥିର UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, ଆକାର(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  ଯଦି (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  ଯଦି (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  ଫେରନ୍ତୁ uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == ଖାଲି);
  ଯେତେବେଳେ ((p = *pp) != ଖାଲି && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    ଯଦି (uplevel(p) == level)  /* corresponding upvalue? */
      ଫେରନ୍ତୁ p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  ଫେରନ୍ତୁ newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ସ୍ଥିର ଶୂନ୍ୟ callclosemethod (lua_State *L, TValue *obj, TValue *err, ପୂର୍ଣ୍ଣସଂଖ୍ୟା yy) {
  StkId ଶୀର୍ଷ = L->ଶୀର୍ଷ.p;
  StkId func = ଶୀର୍ଷ;
  ଅଚଳ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, ଶୀର୍ଷ++, tm);  /* will call metamethod... */
  setobj2s(L, ଶୀର୍ଷ++, obj);  /* with 'self' as the 1st argument */
  ଯଦି (err != ଖାଲି)  /* if there was an error... */
    setobj2s(L, ଶୀର୍ଷ++, err);  /* then error object will be 2nd argument */
  L->ଶୀର୍ଷ.p = ଶୀର୍ଷ;  /* add function and arguments */
  ଯଦି (yy)
    luaD_call(L, func, 0);
  ଅନ୍ୟଥା
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ସ୍ଥିର ଶୂନ୍ୟ checkclosemth (lua_State *L, StkId level) {
  ଅଚଳ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  ଯଦି (ttisnil(tm)) {  /* no metamethod? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = cast_int(level - L->ci->func.p);  /* variable index */
    ଅଚଳ ଅକ୍ଷର *vname = luaG_findlocal(L, L->ci, idx, ଖାଲି);
    ଯଦି (vname == ଖାଲି) vname = "?";
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
ସ୍ଥିର ଶୂନ୍ୟ prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            ପୂର୍ଣ୍ଣସଂଖ୍ୟା yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ବାଛ (status) {
    ସ୍ଥିତି LUA_OK:
      L->ଶୀର୍ଷ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    ସ୍ଥିତି CLOSEKTOP:  /* don't need to change top */
      errobj = ଖାଲି;  /* no error object */
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      ଭାଙ୍ଗ;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
ଶୂନ୍ୟ luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  ଯଦି (l_isfalse(s2v(level)))
    ଫେରନ୍ତୁ;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  ଯେତେବେଳେ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


ଶୂନ୍ୟ luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  ଯଦି (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
ଶୂନ୍ୟ luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  ଯେତେବେଳେ ((uv = L->openupval) != ଖାଲି && uplevel(uv) >= level) {
    TValue *slot = &uv->u.ମୂଲ୍ୟ;  /* new position for value */
    lua_assert(uplevel(uv) < L->ଶୀର୍ଷ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    ଯଦି (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ସ୍ଥିର ଶୂନ୍ୟ poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  ଯେତେବେଳେ (tbc > L->ଗଦା.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, ପୂର୍ଣ୍ଣସଂଖ୍ୟା yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  ଯେତେବେଳେ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  ଫେରନ୍ତୁ level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, ଆକାର(Proto));
  Proto *f = gco2p(o);
  f->k = ଖାଲି;
  f->sizek = 0;
  f->p = ଖାଲି;
  f->sizep = 0;
  f->code = ଖାଲି;
  f->sizecode = 0;
  f->lineinfo = ଖାଲି;
  f->sizelineinfo = 0;
  f->abslineinfo = ଖାଲି;
  f->sizeabslineinfo = 0;
  f->upvalues = ଖାଲି;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ଖାଲି;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = ଖାଲି;
  ଫେରନ୍ତୁ f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, ଆକାର(Proto))
            + cast_uint(p->sizep) * ଆକାର(Proto*)
            + cast_uint(p->sizek) * ଆକାର(TValue)
            + cast_uint(p->sizelocvars) * ଆକାର(LocVar)
            + cast_uint(p->sizeupvalues) * ଆକାର(Upvaldesc);
  ଯଦି (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * ଆକାର(Instruction);
    sz += cast_uint(p->sizelineinfo) * ଆକାର(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * ଆକାର(Abs_ଧାଡ଼ି_Info);
  }
  ଫେରନ୍ତୁ sz;
}


ଶୂନ୍ୟ luaF_freeproto (lua_State *L, Proto *f) {
  ଯଦି (!(f->flag & PF_FIXED)) {
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
ଅଚଳ ଅକ୍ଷର *luaF_getlocalname (ଅଚଳ Proto *f, unsigned local_ସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    ଯଦି (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_ସଂଖ୍ୟା--;
      ଯଦି (local_ସଂଖ୍ୟା == 0)
        ଫେରନ୍ତୁ getstr(f->locvars[i].varname);
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* not found */
}

