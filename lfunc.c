/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#تعریف_کرو lfunc_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <stddef.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"



CClosure *luaF_newCclosure (lua_State *L, صحیح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, سائز_Cclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  واپس c;
}


LClosure *luaF_newLclosure (lua_State *L, صحیح nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, سائز_Lclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = تہی;
  c->nupvalues = cast_byte(nupvals);
  جب (nupvals--) c->upvals[nupvals] = تہی;
  واپس c;
}


/*
** fill a closure with new closed upvalues
*/
خالی luaF_initupvals (lua_State *L, LClosure *cl) {
  صحیح i;
  برائے (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, حجم(اوپر_Val));
    اوپر_Val *uv = gco2upv(o);
    uv->v.p = &uv->u.قدر;  /* make it closed */
    setnilvalue(uv->v.p);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
**/
ساکن اوپر_Val *newupval (lua_State *L, StkId level, اوپر_Val **پچھلا_2) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, حجم(اوپر_Val));
  اوپر_Val *uv = gco2upv(o);
  اوپر_Val *اگلا = *پچھلا_2;
  uv->v.p = s2v(level);  /* current value lives in the stack */
  uv->u.open.اگلا = اگلا;  /* link it to list of open upvalues */
  uv->u.open.پچھلا = پچھلا_2;
  اگر (اگلا)
    اگلا->u.open.پچھلا = &uv->u.open.اگلا;
  *پچھلا_2 = uv;
  اگر (!isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = G(L)->twups;  /* link it to the list */
    G(L)->twups = L;
  }
  واپس uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
*/
اوپر_Val *luaF_findupval (lua_State *L, StkId level) {
  اوپر_Val **pp = &L->openupval;
  اوپر_Val *p;
  lua_assert(isintwups(L) || L->openupval == تہی);
  جب ((p = *pp) != تہی && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    اگر (uplevel(p) == level)  /* corresponding upvalue? */
      واپس p;  /* return it */
    pp = &p->u.open.اگلا;
  }
  /* not found: create a new upvalue after 'pp' */
  واپس newupval(L, level, pp);
}


/*
** Call closing method for object 'obj' with error object 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
*/
ساکن خالی callclosemethod (lua_State *L, TValue *obj, TValue *err, صحیح yy) {
  StkId چوٹی = L->چوٹی.p;
  StkId func = چوٹی;
  ثابت TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, چوٹی++, tm);  /* will call metamethod... */
  setobj2s(L, چوٹی++, obj);  /* with 'self' as the 1st argument */
  اگر (err != تہی)  /* if there was an error... */
    setobj2s(L, چوٹی++, err);  /* then error object will be 2nd argument */
  L->چوٹی.p = چوٹی;  /* add function and arguments */
  اگر (yy)
    luaD_call(L, func, 0);
  ورنہ
    luaD_callnoyield(L, func, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
*/
ساکن خالی checkclosemth (lua_State *L, StkId level) {
  ثابت TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  اگر (ttisnil(tm)) {  /* no metamethod? */
    صحیح idx = cast_int(level - L->ci->func.p);  /* variable index */
    ثابت حرف *vname = luaG_findlocal(L, L->ci, idx, تہی);
    اگر (vname == تہی) vname = "?";
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
ساکن خالی prepcallclosemth (lua_State *L, StkId level, TStatus حالت_2,
                                            صحیح yy) {
  TValue *uv = s2v(level);  /* value being closed */
  TValue *errobj;
  انتخاب (حالت_2) {
    حالت LUA_OK:
      L->چوٹی.p = level + 1;  /* call will be at this level */
      /* FALLTHROUGH */
    حالت CLOSEKTOP:  /* don't need to change top */
      errobj = تہی;  /* no error object */
      توڑو;
    طےشدہ:  /* 'luaD_seterrorobj' will set top to level + 2 */
      errobj = s2v(level + 1);  /* error object goes after 'uv' */
      luaD_seterrorobj(L, حالت_2, level + 1);  /* set error object */
      توڑو;
  }
  callclosemethod(L, uv, errobj, yy);
}


/* Maximum value for deltas in 'tbclist' */
#تعریف_کرو MAXDELTA       USHRT_MAX


/*
** Insert a variable in the list of to-be-closed variables.
*/
خالی luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist.p);
  اگر (l_isfalse(s2v(level)))
    واپس;  /* false doesn't need to be closed */
  checkclosemth(L, level);  /* value must have a close method */
  جب (cast_uint(level - L->tbclist.p) > MAXDELTA) {
    L->tbclist.p += MAXDELTA;  /* create a dummy node at maximum delta */
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(غیرعلامتی مختصر, level - L->tbclist.p);
  L->tbclist.p = level;
}


خالی luaF_unlinkupval (اوپر_Val *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.پچھلا = uv->u.open.اگلا;
  اگر (uv->u.open.اگلا)
    uv->u.open.اگلا->u.open.پچھلا = uv->u.open.پچھلا;
}


/*
** Close all upvalues up to the given stack level.
*/
خالی luaF_closeupval (lua_State *L, StkId level) {
  اوپر_Val *uv;
  جب ((uv = L->openupval) != تہی && uplevel(uv) >= level) {
    TValue *خانہ = &uv->u.قدر;  /* new position for value */
    lua_assert(uplevel(uv) < L->چوٹی.p);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list */
    setobj(L, خانہ, uv->v.p);  /* move value to upvalue slot */
    uv->v.p = خانہ;  /* now current value lives here */
    اگر (!iswhite(uv)) {  /* neither white nor dead? */
      nw2black(uv);  /* closed upvalues cannot be gray */
      luaC_barrier(L, uv, خانہ);
    }
  }
}


/*
** Remove first element from the tbclist plus its dummy nodes.
*/
ساکن خالی poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy */
  tbc -= tbc->tbclist.delta;
  جب (tbc > L->ڈھیر.p && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes */
  L->tbclist.p = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level. Return restored 'level'.
*/
StkId luaF_close (lua_State *L, StkId level, TStatus حالت_2, صحیح yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues */
  جب (L->tbclist.p >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist.p;  /* get variable index */
    poptbclist(L);  /* remove it from list */
    prepcallclosemth(L, tbc, حالت_2, yy);  /* close variable */
    level = restorestack(L, levelrel);
  }
  واپس level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, حجم(Proto));
  Proto *f = gco2p(o);
  f->k = تہی;
  f->sizek = 0;
  f->p = تہی;
  f->sizep = 0;
  f->رمز = تہی;
  f->sizecode = 0;
  f->lineinfo = تہی;
  f->sizelineinfo = 0;
  f->abslineinfo = تہی;
  f->sizeabslineinfo = 0;
  f->upvalues = تہی;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->flag = 0;
  f->maxstacksize = 0;
  f->locvars = تہی;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->ماخذ = تہی;
  واپس f;
}


lu_mem luaF_protosize (Proto *p) {
  lu_mem sz = cast(lu_mem, حجم(Proto))
            + cast_uint(p->sizep) * حجم(Proto*)
            + cast_uint(p->sizek) * حجم(TValue)
            + cast_uint(p->sizelocvars) * حجم(LocVar)
            + cast_uint(p->sizeupvalues) * حجم(Upvaldesc);
  اگر (!(p->flag & PF_FIXED)) {
    sz += cast_uint(p->sizecode) * حجم(Instruction);
    sz += cast_uint(p->sizelineinfo) * حجم(lu_byte);
    sz += cast_uint(p->sizeabslineinfo) * حجم(Abs_سطر_Info);
  }
  واپس sz;
}


خالی luaF_freeproto (lua_State *L, Proto *f) {
  اگر (!(f->flag & PF_FIXED)) {
    luaM_freearray(L, f->رمز, cast_sizet(f->sizecode));
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
ثابت حرف *luaF_getlocalname (ثابت Proto *f, غیرعلامتی local_نمبر, صحیح pc) {
  صحیح i;
  برائے (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    اگر (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_نمبر--;
      اگر (local_نمبر == 0)
        واپس getstr(f->locvars[i].varname);
    }
  }
  واپس تہی;  /* not found */
}

