/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#परिभाषय lfunc_c
#परिभाषय LUA_CORE

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



CClosure *luaF_newCclosure (lua_State *L, पूर्णाङ्क nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, आकार_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  प्रत्यागम c;
}


LClosure *luaF_newLclosure (lua_State *L, पूर्णाङ्क nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, आकार_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = रिक्त;
  c->nupvalues = cast_byte(nupvals);
  यावत् (nupvals--) c->upvals[nupvals] = रिक्त;
  प्रत्यागम c;
}


/*
** fill a closure with new closed upvalues
*/
शून्य luaF_initupvals (lua_State *L, LClosure *cl) {
  पूर्णाङ्क i;
  प्रति (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, परिमाण(ऊर्ध्वम्_Val));
    ऊर्ध्वम्_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.मानम्;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
स्थिर ऊर्ध्वम्_Val *newupval (lua_State *L, StkId level, ऊर्ध्वम्_Val **पूर्व) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, परिमाण(ऊर्ध्वम्_Val));
  ऊर्ध्वम्_Val *uv = gco2upv(o);
  ऊर्ध्वम्_Val *अग्र = *पूर्व;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.उद्घाटय.अग्र = अग्र;  /* link it to list of open upvalues */
  uv->u.उद्घाटय.previous = पूर्व;
  यदि (अग्र)
    अग्र->u.उद्घाटय.previous = &uv->u.उद्घाटय.अग्र;
  *पूर्व = uv;
  यदि (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  प्रत्यागम uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
ऊर्ध्वम्_Val *luaF_findupval (lua_State *L, StkId level) {
  ऊर्ध्वम्_Val **pp = &L->openupval;
  ऊर्ध्वम्_Val *p;
  lua_assert(isintwups(L) || L->openupval == रिक्त);
  यावत् ((p = *pp) != रिक्त && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    यदि (uplevel(p) == level)  /* corresponding upvalue? */
      प्रत्यागम p;  /* return it */
    pp = &p->u.उद्घाटय.अग्र;
  }
  /* not found: create a new upvalue after 'pp' */
  प्रत्यागम newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
स्थिर शून्य callclosemethod (lua_State *L, TValue *obj, TValue *err, पूर्णाङ्क yy) {
  StkId शिखरम् = L->शिखरम्.p;
  StkId func = शिखरम्;
  ध्रुव TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, शिखरम्++, tm);  /* will call metamethod... */
  setobj2s(L, शिखरम्++, obj);  /* with 'self' as the 1st argument */
  यदि (err != रिक्त)  /* if there was an error... */
    setobj2s(L, शिखरम्++, err);  /* then error object will be 2nd argument */
  L->शिखरम्.p = शिखरम्;  /* add function and arguments */
  यदि (yy)
    luaD_call(L, func, 0);
  अन्यथा
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
स्थिर शून्य checkclosemth (lua_State *L, StkId level) {
  ध्रुव TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  यदि (ttisnil(tm)) {  /* no metamethod? */
    पूर्णाङ्क idx = cast_int(level - L->ci->func.p);  /* variable index */
    ध्रुव वर्ण *vname = luaG_findlocal(L, L->ci, idx, रिक्त);
    यदि (vname == रिक्त) vname = "?";
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
स्थिर शून्य prepcallclosemth (lua_State *L, StkId level, TStatus स्थितिः,
                                            पूर्णाङ्क yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  विकल्प (स्थितिः) {
    पक्ष LUA_OK:
      L->शिखरम्.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    पक्ष CLOSEKTOP:  /* don't need to change top */
      errobj = रिक्त;  /* no error object */
      भङ्ग;
    स्वभाव:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, स्थितिः, level + 1);  /* set error object */
      भङ्ग;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#परिभाषय MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
शून्य luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  यदि (l_isfalse(s2v(level)))
    प्रत्यागम;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  यावत् (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(अचिह्नित ह्रस्व, level - L->tbclist.p);
  L->tbclist.p = level;
}


शून्य luaF_unlinkupval (ऊर्ध्वम्_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.उद्घाटय.previous = uv->u.उद्घाटय.अग्र;
  यदि (uv->u.उद्घाटय.अग्र)
    uv->u.उद्घाटय.अग्र->u.उद्घाटय.previous = uv->u.उद्घाटय.previous;
}


/*
** Close all upvalues up to the given stack level.
*/
शून्य luaF_closeupval (lua_State *L, StkId level) {
  ऊर्ध्वम्_Val *uv;
  यावत् ((uv = L->openupval) != रिक्त && uplevel(uv) >= level) {
    TValue *slot = &uv->u.मानम्;  /* new position for value */
    lua_assert(uplevel(uv) < L->शिखरम्.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, slot, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = slot;  /* now current value lives here */
    यदि (!iswhite(uv)) {  /* neither white nor dead? */
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
  यावत् (tbc > L->राशिः.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus स्थितिः, पूर्णाङ्क yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  यावत् (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, स्थितिः, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  प्रत्यागम level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, परिमाण(Proto));
  Proto *f = gco2p(o);
  f->k = रिक्त;
  f->sizek = 0;
  f->p = रिक्त;
  f->sizep = 0;
  f->संहिता = रिक्त;
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
  प्रत्यागम f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, परिमाण(Proto))
            + cast_uint(p->sizep) * परिमाण(Proto*)
            + cast_uint(p->sizek) * परिमाण(TValue)
            + cast_uint(p->sizelocvars) * परिमाण(LocVar)
            + cast_uint(p->sizeupvalues) * परिमाण(Upvaldesc);
  यदि (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * परिमाण(Instruction);
    sz += cast_uint(p->sizelineinfo) * परिमाण(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * परिमाण(Abs_पङ्क्तिः_Info);
  }
  प्रत्यागम sz;
}


शून्य luaF_freeproto (lua_State *L, Proto *f) {
  यदि (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->संहिता, cast_sizet(f->sizecode));
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
ध्रुव वर्ण *luaF_getlocalname (ध्रुव Proto *f, अचिह्नित local_सङ्ख्या, पूर्णाङ्क pc) {
  पूर्णाङ्क i;
  प्रति (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    यदि (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_सङ्ख्या--;
      यदि (local_सङ्ख्या == 0)
        प्रत्यागम getstr(f->locvars[i].varname);
    }
  }
  प्रत्यागम रिक्त;  /* not found */
}

