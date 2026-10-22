/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#నిర్వచించు lfunc_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stddef.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, పూర్ణాంకం nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, కొలత_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  తిరిగి c;
}


LClosure *luaF_newLclosure (lua_State *L, పూర్ణాంకం nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, కొలత_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = ఖాళీ;
  c->nupvalues = cast_byte(nupvals);
  అయితే (nupvals--) c->upvals[nupvals] = ఖాళీ;
  తిరిగి c;
}


/*
** fill a closure with new closed upvalues
*/
శూన్య luaF_initupvals (lua_State *L, LClosure *cl) {
  పూర్ణాంకం i;
  కొరకు (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, పరిమాణం(పైకి_Val));
    పైకి_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.విలువ;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
స్థిర పైకి_Val *newupval (lua_State *L, StkId level, పైకి_Val **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, పరిమాణం(పైకి_Val));
  పైకి_Val *uv = gco2upv(o);
  పైకి_Val *తదుపరి = *prev;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.తదుపరి = తదుపరి;  /* link it to list of open upvalues */
  uv->u.open.మునుపటి = prev;
  ఒకవేళ (తదుపరి)
    తదుపరి->u.open.మునుపటి = &uv->u.open.తదుపరి;
  *prev = uv;
  ఒకవేళ (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  తిరిగి uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
పైకి_Val *luaF_findupval (lua_State *L, StkId level) {
  పైకి_Val **pp = &L->openupval;
  పైకి_Val *p;
  lua_assert(isintwups(L) || L->openupval == ఖాళీ);
  అయితే ((p = *pp) != ఖాళీ && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    ఒకవేళ (uplevel(p) == level)  /* corresponding upvalue? */
      తిరిగి p;  /* return it */
    pp = &p->u.open.తదుపరి;
  }
  /* not found: create a new upvalue after 'pp' */
  తిరిగి newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
స్థిర శూన్య callclosemethod (lua_State *L, TValue *obj, TValue *err, పూర్ణాంకం yy) {
  StkId శిఖరం = L->శిఖరం.p;
  StkId func = శిఖరం;
  స్థిరరాశి TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, శిఖరం++, tm);  /* will call metamethod... */
  setobj2s(L, శిఖరం++, obj);  /* with 'self' as the 1st argument */
  ఒకవేళ (err != ఖాళీ)  /* if there was an error... */
    setobj2s(L, శిఖరం++, err);  /* then error object will be 2nd argument */
  L->శిఖరం.p = శిఖరం;  /* add function and arguments */
  ఒకవేళ (yy)
    luaD_call(L, func, 0);
  లేకపోతే
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
స్థిర శూన్య checkclosemth (lua_State *L, StkId level) {
  స్థిరరాశి TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  ఒకవేళ (ttisnil(tm)) {  /* no metamethod? */
    పూర్ణాంకం idx = cast_int(level - L->ci->func.p);  /* variable index */
    స్థిరరాశి అక్షరం *vname = luaG_findlocal(L, L->ci, idx, ఖాళీ);
    ఒకవేళ (vname == ఖాళీ) vname = "?";
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
స్థిర శూన్య prepcallclosemth (lua_State *L, StkId level, TStatus స్థితి_2,
                                            పూర్ణాంకం yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  ఎంపిక (స్థితి_2) {
    స్థితి LUA_OK:
      L->శిఖరం.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    స్థితి CLOSEKTOP:  /* don't need to change top */
      errobj = ఖాళీ;  /* no error object */
      ఆపు;
    అప్రమేయ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, స్థితి_2, level + 1);  /* set error object */
      ఆపు;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#నిర్వచించు MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
శూన్య luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  ఒకవేళ (l_isfalse(s2v(level)))
    తిరిగి;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  అయితే (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(గుర్తులేని పొట్టి, level - L->tbclist.p);
  L->tbclist.p = level;
}


శూన్య luaF_unlinkupval (పైకి_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.మునుపటి = uv->u.open.తదుపరి;
  ఒకవేళ (uv->u.open.తదుపరి)
    uv->u.open.తదుపరి->u.open.మునుపటి = uv->u.open.మునుపటి;
}


/*
** Close all upvalues up to the given stack level.
*/
శూన్య luaF_closeupval (lua_State *L, StkId level) {
  పైకి_Val *uv;
  అయితే ((uv = L->openupval) != ఖాళీ && uplevel(uv) >= level) {
    TValue *స్థానం = &uv->u.విలువ;  /* new position for value */
    lua_assert(uplevel(uv) < L->శిఖరం.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, స్థానం, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = స్థానం;  /* now current value lives here */
    ఒకవేళ (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, స్థానం);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
స్థిర శూన్య poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  అయితే (tbc > L->దొంతర.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus స్థితి_2, పూర్ణాంకం yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  అయితే (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, స్థితి_2, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  తిరిగి level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, పరిమాణం(Proto));
  Proto *f = gco2p(o);
  f->k = ఖాళీ;
  f->sizek = 0;
  f->p = ఖాళీ;
  f->sizep = 0;
  f->సంకేతం_2 = ఖాళీ;
  f->sizecode = 0;
  f->lineinfo = ఖాళీ;
  f->sizelineinfo = 0;
  f->abslineinfo = ఖాళీ;
  f->sizeabslineinfo = 0;
  f->upvalues = ఖాళీ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = ఖాళీ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->ఆధారం = ఖాళీ;
  తిరిగి f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, పరిమాణం(Proto))
            + cast_uint(p->sizep) * పరిమాణం(Proto*)
            + cast_uint(p->sizek) * పరిమాణం(TValue)
            + cast_uint(p->sizelocvars) * పరిమాణం(LocVar)
            + cast_uint(p->sizeupvalues) * పరిమాణం(Upvaldesc);
  ఒకవేళ (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * పరిమాణం(Instruction);
    sz += cast_uint(p->sizelineinfo) * పరిమాణం(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * పరిమాణం(Abs_పంక్తి_Info);
  }
  తిరిగి sz;
}


శూన్య luaF_freeproto (lua_State *L, Proto *f) {
  ఒకవేళ (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->సంకేతం_2, cast_sizet(f->sizecode));
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
స్థిరరాశి అక్షరం *luaF_getlocalname (స్థిరరాశి Proto *f, గుర్తులేని local_సంఖ్య, పూర్ణాంకం pc) {
  పూర్ణాంకం i;
  కొరకు (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    ఒకవేళ (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_సంఖ్య--;
      ఒకవేళ (local_సంఖ్య == 0)
        తిరిగి getstr(f->locvars[i].varname);
    }
  }
  తిరిగి ఖాళీ;  /* not found */
}

