/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#घोषणा lfunc_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <stddef.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lgc.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, पूर्णांक nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, आकार_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  वापस c;
}


LClosure *luaF_newLclosure (lua_State *L, पूर्णांक nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, आकार_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = शून्य;
  c->nupvalues = cast_byte(nupvals);
  जबतक (nupvals--) c->upvals[nupvals] = शून्य;
  वापस c;
}


/*
** fill a closure with new closed upvalues
*/
व्योम luaF_initupvals (lua_State *L, LClosure *cl) {
  पूर्णांक i;
  क्रम (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, माप(UpVal));
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
अटल UpVal *newupval (lua_State *L, StkId level, UpVal **पूर्व) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, माप(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *अग्र = *पूर्व;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.खोलो.अग्र = अग्र;  /* link it to list of open upvalues */
  uv->u.खोलो.previous = पूर्व;
  अगर (अग्र)
    अग्र->u.खोलो.previous = &uv->u.खोलो.अग्र;
  *पूर्व = uv;
  अगर (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  वापस uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == शून्य);
  जबतक ((p = *pp) != शून्य && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    अगर (uplevel(p) == level)  /* corresponding upvalue? */
      वापस p;  /* return it */
    pp = &p->u.खोलो.अग्र;
  }
  /* not found: create a new upvalue after 'pp' */
  वापस newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
अटल व्योम callclosemethod (lua_State *L, TValue *obj, TValue *err, पूर्णांक yy) {
  StkId शीर्ष = L->शीर्ष.p;
  StkId func = शीर्ष;
  स्थिर TValue *पंचांग = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, शीर्ष++, पंचांग);  /* will call metamethod... */
  setobj2s(L, शीर्ष++, obj);  /* with 'self' as the 1st argument */
  अगर (err != शून्य)  /* if there was an error... */
    setobj2s(L, शीर्ष++, err);  /* then error object will be 2nd argument */
  L->शीर्ष.p = शीर्ष;  /* add function and arguments */
  अगर (yy)
    luaD_call(L, func, 0);
  अन्यथा
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
अटल व्योम checkclosemth (lua_State *L, StkId level) {
  स्थिर TValue *पंचांग = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  अगर (ttisnil(पंचांग)) {  /* no metamethod? */
    पूर्णांक idx = cast_int(level - L->ci->func.p);  /* variable index */
    स्थिर अक्षर *vname = luaG_findlocal(L, L->ci, idx, शून्य);
    अगर (vname == शून्य) vname = "?";
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
अटल व्योम prepcallclosemth (lua_State *L, StkId level, TStatus status,
                                            पूर्णांक yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  चयन (status) {
    हाल LUA_OK:
      L->शीर्ष.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    हाल CLOSEKTOP:  /* don't need to change top */
      errobj = शून्य;  /* no error object */
      अवरोध;
    शेष:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, status, level + 1);  /* set error object */
      अवरोध;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#घोषणा MAXDELTA       अच_लघु_उच्च


/*
** Insert a variable in the list of to-be-closed variables.
*/
व्योम luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  अगर (l_isfalse(s2v(level)))
    वापस;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  जबतक (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(अचिन्हित लघु, level - L->tbclist.p);
  L->tbclist.p = level;
}


व्योम luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.खोलो.previous = uv->u.खोलो.अग्र;
  अगर (uv->u.खोलो.अग्र)
    uv->u.खोलो.अग्र->u.खोलो.previous = uv->u.खोलो.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
व्योम luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  जबतक ((uv = L->openupval) != शून्य && uplevel(uv) >= level) {
    TValue *slot = &uv->u.मूल्य;  /* new position for value */
    lua_assert(uplevel(uv) < L->शीर्ष.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    अगर (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
अटल व्योम poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  जबतक (tbc > L->ढेर.p && tbc->tbclist.delta == 0)
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
  जबतक (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, status, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  वापस level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, माप(Proto));
  Proto *f = gco2p(o);
  f->k = शून्य;
  f->sizek = 0;
  f->p = शून्य;
  f->sizep = 0;
  f->code = शून्य;
  f->sizecode = 0;
  f->lineinfo = शून्य;
  f->sizelineinfo = 0;
  f->abslineinfo = शून्य;
  f->sizeabslineinfo = 0;
  f->upvalues = शून्य;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = शून्य;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = शून्य;
  वापस f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, माप(Proto))
            + cast_uint(p->sizep) * माप(Proto*)
            + cast_uint(p->sizek) * माप(TValue)
            + cast_uint(p->sizelocvars) * माप(LocVar)
            + cast_uint(p->sizeupvalues) * माप(Upvaldesc);
  अगर (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * माप(Instruction);
    sz += cast_uint(p->sizelineinfo) * माप(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * माप(Abs_पंक्ति_Info);
  }
  वापस sz;
}


व्योम luaF_freeproto (lua_State *L, Proto *f) {
  अगर (!(f->flag & PF_FIXED)) {
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
स्थिर अक्षर *luaF_getlocalname (स्थिर Proto *f, अचिन्हित local_संख्या, पूर्णांक pc) {
  पूर्णांक i;
  क्रम (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    अगर (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_संख्या--;
      अगर (local_संख्या == 0)
        वापस getstr(f->locvars[i].varname);
    }
  }
  वापस शून्य;  /* not found */
}

