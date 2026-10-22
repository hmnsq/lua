/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stddef.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, പൂർണ്ണസംഖ്യ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  തിരികെ c;
}


LClosure *luaF_newLclosure (lua_State *L, പൂർണ്ണസംഖ്യ nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ശൂന്യ;
  c->nupvalues = cast_byte(nupvals);
  ആകുമ്പോൾ (nupvals--) c->upvals[nupvals] = ശൂന്യ;
  തിരികെ c;
}


/*
** fill a closure with new closed upvalues
*/
ശൂന്യം luaF_initupvals (lua_State *L, LClosure *cl) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, വലുപ്പം(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.മൂല്യം;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
സ്ഥിതം UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, വലുപ്പം(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.next = next;  /* link it to list of open upvalues */
  uv->u.open.previous = prev;
  എങ്കിൽ (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  എങ്കിൽ (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  തിരികെ uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == ശൂന്യ);
  ആകുമ്പോൾ ((p = *pp) != ശൂന്യ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    എങ്കിൽ (uplevel(p) == level)  /* corresponding upvalue? */
      തിരികെ p;  /* return it */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' */
  തിരികെ newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
സ്ഥിതം ശൂന്യം callclosemethod (lua_State *L, TValue *obj, TValue *err, പൂർണ്ണസംഖ്യ yy) {
  StkId മുകൾ = L->മുകൾ.p;
  StkId func = മുകൾ;
  സ്ഥിരം TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, മുകൾ++, tm);  /* will call metamethod... */
  setobj2s(L, മുകൾ++, obj);  /* with 'self' as the 1st argument */
  എങ്കിൽ (err != ശൂന്യ)  /* if there was an error... */
    setobj2s(L, മുകൾ++, err);  /* then error object will be 2nd argument */
  L->മുകൾ.p = മുകൾ;  /* add function and arguments */
  എങ്കിൽ (yy)
    luaD_call(L, func, 0);
  അല്ലെങ്കിൽ
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
സ്ഥിതം ശൂന്യം checkclosemth (lua_State *L, StkId level) {
  സ്ഥിരം TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  എങ്കിൽ (ttisnil(tm)) {  /* no metamethod? */
    പൂർണ്ണസംഖ്യ idx = cast_int(level - L->ci->func.p);  /* variable index */
    സ്ഥിരം അക്ഷരം *vname = luaG_findlocal(L, L->ci, idx, ശൂന്യ);
    എങ്കിൽ (vname == ശൂന്യ) vname = "?";
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
സ്ഥിതം ശൂന്യം prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            പൂർണ്ണസംഖ്യ yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  തിരഞ്ഞെടുപ്പ് (status) {
    അവസ്ഥ LUA_OK:
      L->മുകൾ.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    അവസ്ഥ CLOSEKTOP:  /* don't need to change top */
      errobj = ശൂന്യ;  /* no error object */
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      നിർത്തുക;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
ശൂന്യം luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  എങ്കിൽ (l_isfalse(s2v(level)))
    തിരികെ;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  ആകുമ്പോൾ (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist.p);
  L->tbclist.p = level;
}


ശൂന്യം luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  എങ്കിൽ (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
ശൂന്യം luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  ആകുമ്പോൾ ((uv = L->openupval) != ശൂന്യ && uplevel(uv) >= level) {
    TValue *slot = &uv->u.മൂല്യം;  /* new position for value */
    lua_assert(uplevel(uv) < L->മുകൾ.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    എങ്കിൽ (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
സ്ഥിതം ശൂന്യം poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  ആകുമ്പോൾ (tbc > L->അട്ടി.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, പൂർണ്ണസംഖ്യ yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  ആകുമ്പോൾ (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  തിരികെ level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, വലുപ്പം(Proto));
  Proto *f = gco2p(o);
  f->k = ശൂന്യ;
  f->sizek = 0;
  f->p = ശൂന്യ;
  f->sizep = 0;
  f->code = ശൂന്യ;
  f->sizecode = 0;
  f->lineinfo = ശൂന്യ;
  f->sizelineinfo = 0;
  f->abslineinfo = ശൂന്യ;
  f->sizeabslineinfo = 0;
  f->upvalues = ശൂന്യ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ശൂന്യ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = ശൂന്യ;
  തിരികെ f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, വലുപ്പം(Proto))
            + cast_uint(p->sizep) * വലുപ്പം(Proto*)
            + cast_uint(p->sizek) * വലുപ്പം(TValue)
            + cast_uint(p->sizelocvars) * വലുപ്പം(LocVar)
            + cast_uint(p->sizeupvalues) * വലുപ്പം(Upvaldesc);
  എങ്കിൽ (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * വലുപ്പം(Instruction);
    sz += cast_uint(p->sizelineinfo) * വലുപ്പം(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * വലുപ്പം(Abs_വരി_Info);
  }
  തിരികെ sz;
}


ശൂന്യം luaF_freeproto (lua_State *L, Proto *f) {
  എങ്കിൽ (!(f->flag & PF_FIXED)) {
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
സ്ഥിരം അക്ഷരം *luaF_getlocalname (സ്ഥിരം Proto *f, unsigned local_സംഖ്യ, പൂർണ്ണസംഖ്യ pc) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    എങ്കിൽ (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_സംഖ്യ--;
      എങ്കിൽ (local_സംഖ്യ == 0)
        തിരികെ getstr(f->locvars[i].varname);
    }
  }
  തിരികെ ശൂന്യ;  /* not found */
}

