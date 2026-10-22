/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stddef.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, પૂર્ણાંક nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  પાછા c;
}


LClosure *luaF_newLclosure (lua_State *L, પૂર્ણાંક nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ખાલી;
  c->nupvalues = cast_byte(nupvals);
  જ્યારે (nupvals--) c->upvals[nupvals] = ખાલી;
  પાછા c;
}


/*
** fill a closure with new closed upvalues
*/
શૂન્ય luaF_initupvals (lua_State *L, LClosure *cl) {
  પૂર્ણાંક i;
  માટે (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, કદ(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.કિંમત;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
સ્થિર UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, કદ(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  જો (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  જો (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  પાછા uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == ખાલી);
  જ્યારે ((p = *pp) != ખાલી && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    જો (uplevel(p) == level)  /* corresponding upvalue? */
      પાછા p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  પાછા newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
સ્થિર શૂન્ય callclosemethod (lua_State *L, TValue *obj, TValue *err, પૂર્ણાંક yy) {
  StkId ટોચ = L->ટોચ.p;
  StkId func = ટોચ;
  અચલ TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, ટોચ++, tm);  /* will call metamethod... */
  setobj2s(L, ટોચ++, obj);  /* with 'self' as the 1st argument */
  જો (err != ખાલી)  /* if there was an error... */
    setobj2s(L, ટોચ++, err);  /* then error object will be 2nd argument */
  L->ટોચ.p = ટોચ;  /* add function and arguments */
  જો (yy)
    luaD_call(L, func, 0);
  નહીંતર
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
સ્થિર શૂન્ય checkclosemth (lua_State *L, StkId level) {
  અચલ TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  જો (ttisnil(tm)) {  /* no metamethod? */
    પૂર્ણાંક idx = cast_int(level - L->ci->func.p);  /* variable index */
    અચલ અક્ષર *vname = luaG_findlocal(L, L->ci, idx, ખાલી);
    જો (vname == ખાલી) vname = "?";
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
સ્થિર શૂન્ય prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            પૂર્ણાંક yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  પસંદગી (status) {
    સ્થિતિ LUA_OK:
      L->ટોચ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    સ્થિતિ CLOSEKTOP:  /* don't need to change top */
      errobj = ખાલી;  /* no error object */
      તોડો;
    મૂળભૂત:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      તોડો;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
શૂન્ય luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  જો (l_isfalse(s2v(level)))
    પાછા;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  જ્યારે (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


શૂન્ય luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  જો (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
શૂન્ય luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  જ્યારે ((uv = L->openupval) != ખાલી && uplevel(uv) >= level) {
    TValue *slot = &uv->u.કિંમત;  /* new position for value */
    lua_assert(uplevel(uv) < L->ટોચ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    જો (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
સ્થિર શૂન્ય poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  જ્યારે (tbc > L->ઢગલો.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, પૂર્ણાંક yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  જ્યારે (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  પાછા level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, કદ(Proto));
  Proto *f = gco2p(o);
  f->k = ખાલી;
  f->sizek = 0;
  f->p = ખાલી;
  f->sizep = 0;
  f->code = ખાલી;
  f->sizecode = 0;
  f->lineinfo = ખાલી;
  f->sizelineinfo = 0;
  f->abslineinfo = ખાલી;
  f->sizeabslineinfo = 0;
  f->upvalues = ખાલી;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ખાલી;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = ખાલી;
  પાછા f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, કદ(Proto))
            + cast_uint(p->sizep) * કદ(Proto*)
            + cast_uint(p->sizek) * કદ(TValue)
            + cast_uint(p->sizelocvars) * કદ(LocVar)
            + cast_uint(p->sizeupvalues) * કદ(Upvaldesc);
  જો (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * કદ(Instruction);
    sz += cast_uint(p->sizelineinfo) * કદ(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * કદ(Abs_લીટી_Info);
  }
  પાછા sz;
}


શૂન્ય luaF_freeproto (lua_State *L, Proto *f) {
  જો (!(f->flag & PF_FIXED)) {
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
અચલ અક્ષર *luaF_getlocalname (અચલ Proto *f, unsigned local_સંખ્યા, પૂર્ણાંક pc) {
  પૂર્ણાંક i;
  માટે (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    જો (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_સંખ્યા--;
      જો (local_સંખ્યા == 0)
        પાછા getstr(f->locvars[i].varname);
    }
  }
  પાછા ખાલી;  /* not found */
}

