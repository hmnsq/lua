/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stddef.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, पूर्णांक nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  परत c;
}


LClosure *luaF_newLclosure (lua_State *L, पूर्णांक nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = रिक्त;
  c->nupvalues = cast_byte(nupvals);
  पर्यंत (nupvals--) c->upvals[nupvals] = रिक्त;
  परत c;
}


/*
** fill a closure with new closed upvalues
*/
शून्य luaF_initupvals (lua_State *L, LClosure *cl) {
  पूर्णांक i;
  साठी (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, आकार(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v.p = &uv->u.मूल्य;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
स्थिर UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, आकार(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.खोलो.next = next;  /* link it to list of open upvalues */
  uv->u.खोलो.previous = prev;
  जर (next)
    next->u.खोलो.previous = &uv->u.खोलो.next;
  *prev = uv;
  जर (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  परत uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == रिक्त);
  पर्यंत ((p = *pp) != रिक्त && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    जर (uplevel(p) == level)  /* corresponding upvalue? */
      परत p;  /* return it */
    pp = &p->u.खोलो.next;
  }
  /* not found: create a new upvalue after 'pp' */
  परत newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
स्थिर शून्य callclosemethod (lua_State *L, TValue *obj, TValue *err, पूर्णांक yy) {
  StkId शीर्ष = L->शीर्ष.p;
  StkId func = शीर्ष;
  स्थिरांक TValue *पंचांग = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, शीर्ष++, पंचांग);  /* will call metamethod... */
  setobj2s(L, शीर्ष++, obj);  /* with 'self' as the 1st argument */
  जर (err != रिक्त)  /* if there was an error... */
    setobj2s(L, शीर्ष++, err);  /* then error object will be 2nd argument */
  L->शीर्ष.p = शीर्ष;  /* add function and arguments */
  जर (yy)
    luaD_call(L, func, 0);
  अन्यथा
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
स्थिर शून्य checkclosemth (lua_State *L, StkId level) {
  स्थिरांक TValue *पंचांग = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  जर (ttisnil(पंचांग)) {  /* no metamethod? */
    पूर्णांक idx = cast_int(level - L->ci->func.p);  /* variable index */
    स्थिरांक अक्षर *vname = luaG_findlocal(L, L->ci, idx, रिक्त);
    जर (vname == रिक्त) vname = "?";
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
स्थिर शून्य prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            पूर्णांक yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  निवड (status) {
    स्थिती LUA_OK:
      L->शीर्ष.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    स्थिती CLOSEKTOP:  /* don't need to change top */
      errobj = रिक्त;  /* no error object */
      थांबा;
    पूर्वनिर्धारित:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      थांबा;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#define MAXDELTA       अच_लघु_उच्च


/*
** Insert a variable in the list of to-be-closed variables.
*/
शून्य luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  जर (l_isfalse(s2v(level)))
    परत;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  पर्यंत (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(अचिन्हित लघु, level - L->tbclist.p);
  L->tbclist.p = level;
}


शून्य luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.खोलो.previous = uv->u.खोलो.next;
  जर (uv->u.खोलो.next)
    uv->u.खोलो.next->u.खोलो.previous = uv->u.खोलो.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
शून्य luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  पर्यंत ((uv = L->openupval) != रिक्त && uplevel(uv) >= level) {
    TValue *slot = &uv->u.मूल्य;  /* new position for value */
    lua_assert(uplevel(uv) < L->शीर्ष.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    जर (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
स्थिर शून्य poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  पर्यंत (tbc > L->चळत.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus status, पूर्णांक yy) {
  सूचक_भेद_प्रकार levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  पर्यंत (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  परत level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, आकार(Proto));
  Proto *f = gco2p(o);
  f->k = रिक्त;
  f->sizek = 0;
  f->p = रिक्त;
  f->sizep = 0;
  f->code = रिक्त;
  f->sizecode = 0;
  f->lineinfo = रिक्त;
  f->sizelineinfo = 0;
  f->abslineinfo = रिक्त;
  f->sizeabslineinfo = 0;
  f->upvalues = रिक्त;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = रिक्त;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = रिक्त;
  परत f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, आकार(Proto))
            + cast_uint(p->sizep) * आकार(Proto*)
            + cast_uint(p->sizek) * आकार(TValue)
            + cast_uint(p->sizelocvars) * आकार(LocVar)
            + cast_uint(p->sizeupvalues) * आकार(Upvaldesc);
  जर (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * आकार(Instruction);
    sz += cast_uint(p->sizelineinfo) * आकार(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * आकार(Abs_ओळ_Info);
  }
  परत sz;
}


शून्य luaF_freeproto (lua_State *L, Proto *f) {
  जर (!(f->flag & PF_FIXED)) {
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
स्थिरांक अक्षर *luaF_getlocalname (स्थिरांक Proto *f, अचिन्हित local_संख्या, पूर्णांक pc) {
  पूर्णांक i;
  साठी (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    जर (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_संख्या--;
      जर (local_संख्या == 0)
        परत getstr(f->locvars[i].varname);
    }
  }
  परत रिक्त;  /* not found */
}

