/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#నిర్వచించు ldo_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <setjmp.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lparser.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"
#అన్తర్భావ "lundump.h"
#అన్తర్భావ "lvm.h"
#అన్తర్భావ "lzio.h"



#నిర్వచించు errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#ఒకవేళ !defined(luai_userstateresume)
#నిర్వచించు luai_userstateresume(L,n)	((శూన్య)L)
#అయితే_ముగింపు

#ఒకవేళ !defined(luai_userstateyield)
#నిర్వచించు luai_userstateyield(L,n)	((శూన్య)L)
#అయితే_ముగింపు


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
రకనిర్వచనం నిర్మాణం lua_longjmp {
  నిర్మాణం lua_longjmp *మునుపటి;
  jmp_buf b;
  అస్థిర TStatus స్థితి_2;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#ఒకవేళ !defined(LUAI_THROW)				/* { */

#ఒకవేళ defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#నిర్వచించు LUAI_THROW(L,c)		throw(c)

స్థిర శూన్య LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, శూన్య *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    ఒకవేళ (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->స్థితి_2 = -1;  /* create some error code */
  }
}


#లేదావేళ defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#నిర్వచించు LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#నిర్వచించు LUAI_TRY(L,c,f,ud)	ఒకవేళ (_setjmp((c)->b) == 0) ((f)(L, ud))

#లేకపోతే							/* }{ */

/* ISO C handling with long jumps */
#నిర్వచించు LUAI_THROW(L,c)		longjmp((c)->b, 1)
#నిర్వచించు LUAI_TRY(L,c,f,ud)	ఒకవేళ (setjmp((c)->b) == 0) ((f)(L, ud))

#అయితే_ముగింపు							/* } */

#అయితే_ముగింపు							/* } */


శూన్య luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  ఒకవేళ (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  లేకపోతే {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->శిఖరం.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->శిఖరం.p - 1);  /* move it to 'oldtop' */
  }
  L->శిఖరం.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  ఒకవేళ (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->స్థితి_2 = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  లేకపోతే {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->స్థితి_2 = errcode;
    ఒకవేళ (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->శిఖరం.p++, L->శిఖరం.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    లేకపోతే {  /* no handler at all; abort */
      ఒకవేళ (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  ఒకవేళ (L->errorJmp) {
    /* unroll error entries up to the first level */
    అయితే (L->errorJmp->మునుపటి != ఖాళీ)
      L->errorJmp = L->errorJmp->మునుపటి;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, శూన్య *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.స్థితి_2 = LUA_OK;
  lj.మునుపటి = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.మునుపటి;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  తిరిగి lj.స్థితి_2;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#నిర్వచించు STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#ఒకవేళ !defined(LUAI_MAXSTACK)
#ఒకవేళ 1000000 < (గరిష్ఠ_పూర్ణాంకం / 2)
#నిర్వచించు LUAI_MAXSTACK           1000000
#లేకపోతే
#నిర్వచించు LUAI_MAXSTACK           (గరిష్ఠ_పూర్ణాంకం / 2u)
#అయితే_ముగింపు
#అయితే_ముగింపు


/* maximum stack size that respects size_t */
#నిర్వచించు MAXSTACK_BYSIZET  ((MAX_SIZET / పరిమాణం(దొంతర_విలువ)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#నిర్వచించు MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#నిర్వచించు ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *సందేశం = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->శిఖరం.p, సందేశం);
  L->శిఖరం.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
పూర్ణాంకం luaD_checkminstack (lua_State *L) {
  ఒకవేళ (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    తిరిగి 0;  /* not enough C-stack slots */
  ఒకవేళ (L->ci->తదుపరి == ఖాళీ && luaE_extendCI(L, 0) == ఖాళీ)
    తిరిగి 0;  /* unable to allocate first ci */
  ఒకవేళ (L->ci->తదుపరి->తదుపరి == ఖాళీ && luaE_extendCI(L, 0) == ఖాళీ)
    తిరిగి 0;  /* unable to allocate second ci */
  ఒకవేళ (L->దొంతర_చివరి.p - L->శిఖరం.p >= BASIC_దొంతర_కొలత)
    తిరిగి 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  లేకపోతే  /* try to grow stack to a size with enough free slots */
    తిరిగి luaD_growstack(L, BASIC_దొంతర_కొలత, 0);
}


/*
** In ISO C, any pointer use after the pointer has been deallocated is
** undefined behavior. So, before a stack reallocation, all pointers
** should be changed to offsets, and after the reallocation they should
** be changed back to pointers. As during the reallocation the pointers
** are invalid, the reallocation cannot run emergency collections.
** Alternatively, we can use the old address after the deallocation.
** That is not strict ISO C, but seems to work fine everywhere.
** The following macro chooses how strict is the code.
*/
#ఒకవేళ !defined(LUAI_STRICT_చిరునామా)
#నిర్వచించు LUAI_STRICT_చిరునామా	1
#అయితే_ముగింపు

#ఒకవేళ LUAI_STRICT_చిరునామా
/*
** Change all pointers to the stack into offsets.
*/
స్థిర శూన్య relstack (lua_State *L) {
  CallInfo *ci;
  పైకి_Val *పైకి;
  L->శిఖరం.స్థానభ్రంశం = savestack(L, L->శిఖరం.p);
  L->tbclist.స్థానభ్రంశం = savestack(L, L->tbclist.p);
  కొరకు (పైకి = L->openupval; పైకి != ఖాళీ; పైకి = పైకి->u.open.తదుపరి)
    పైకి->v.స్థానభ్రంశం = savestack(L, uplevel(పైకి));
  కొరకు (ci = L->ci; ci != ఖాళీ; ci = ci->మునుపటి) {
    ci->శిఖరం.స్థానభ్రంశం = savestack(L, ci->శిఖరం.p);
    ci->func.స్థానభ్రంశం = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
స్థిర శూన్య correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  పైకి_Val *పైకి;
  వాడని(oldstack);
  L->శిఖరం.p = restorestack(L, L->శిఖరం.స్థానభ్రంశం);
  L->tbclist.p = restorestack(L, L->tbclist.స్థానభ్రంశం);
  కొరకు (పైకి = L->openupval; పైకి != ఖాళీ; పైకి = పైకి->u.open.తదుపరి)
    పైకి->v.p = s2v(restorestack(L, పైకి->v.స్థానభ్రంశం));
  కొరకు (ci = L->ci; ci != ఖాళీ; ci = ci->మునుపటి) {
    ci->శిఖరం.p = restorestack(L, ci->శిఖరం.స్థానభ్రంశం);
    ci->func.p = restorestack(L, ci->func.స్థానభ్రంశం);
    ఒకవేళ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#లేకపోతే
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

స్థిర శూన్య relstack (lua_State *L) { వాడని(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
స్థిర శూన్య correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  పైకి_Val *పైకి;
  StkId newstack = L->దొంతర.p;
  ఒకవేళ (oldstack == newstack)
    తిరిగి;
  L->శిఖరం.p = L->శిఖరం.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  కొరకు (పైకి = L->openupval; పైకి != ఖాళీ; పైకి = పైకి->u.open.తదుపరి)
    పైకి->v.p = s2v(uplevel(పైకి) - oldstack + newstack);
  కొరకు (ci = L->ci; ci != ఖాళీ; ci = ci->మునుపటి) {
    ci->శిఖరం.p = ci->శిఖరం.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    ఒకవేళ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#అయితే_ముగింపు


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
పూర్ణాంకం luaD_reallocstack (lua_State *L, పూర్ణాంకం newsize, పూర్ణాంకం raiseerror) {
  పూర్ణాంకం oldsize = stacksize(L);
  పూర్ణాంకం i;
  StkId newstack;
  StkId oldstack = L->దొంతర.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_దొంతర,
                                   newsize + EXTRA_దొంతర, దొంతర_విలువ);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  ఒకవేళ (l_unlikely(newstack == ఖాళీ)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    ఒకవేళ (raiseerror)
      luaM_error(L);
    లేకపోతే తిరిగి 0;  /* do not raise an error */
  }
  L->దొంతర.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->దొంతర_చివరి.p = L->దొంతర.p + newsize;
  కొరకు (i = oldsize + EXTRA_దొంతర; i < newsize + EXTRA_దొంతర; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  తిరిగి 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
పూర్ణాంకం luaD_growstack (lua_State *L, పూర్ణాంకం n, పూర్ణాంకం raiseerror) {
  పూర్ణాంకం కొలత = stacksize(L);
  ఒకవేళ (l_unlikely(కొలత > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    ఒకవేళ (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    తిరిగి 0;  /* if not 'raiseerror', just signal it */
  }
  లేకపోతే ఒకవేళ (n < MAXSTACK) {  /* avoids arithmetic overflows */
    పూర్ణాంకం newsize = కొలత + (కొలత >> 1);  /* tentative new size (size * 1.5) */
    పూర్ణాంకం needed = cast_int(L->శిఖరం.p - L->దొంతర.p) + n;
    ఒకవేళ (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    ఒకవేళ (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    ఒకవేళ (l_likely(newsize <= MAXSTACK))
      తిరిగి luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  ఒకవేళ (raiseerror)
    luaG_runerror(L, "stack overflow");
  తిరిగి 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
స్థిర పూర్ణాంకం stackinuse (lua_State *L) {
  CallInfo *ci;
  పూర్ణాంకం res;
  StkId lim = L->శిఖరం.p;
  కొరకు (ci = L->ci; ci != ఖాళీ; ci = ci->మునుపటి) {
    ఒకవేళ (lim < ci->శిఖరం.p) lim = ci->శిఖరం.p;
  }
  lua_assert(lim <= L->దొంతర_చివరి.p + EXTRA_దొంతర);
  res = cast_int(lim - L->దొంతర.p) + 1;  /* part of stack in use */
  ఒకవేళ (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  తిరిగి res;
}


/*
** If stack size is more than 3 times the current use, reduce that size
** to twice the current use. (So, the final stack size is at most 2/3 the
** previous size, and half of its entries are empty.)
** As a particular case, if stack was handling a stack overflow and now
** it is not, 'max' (limited by MAXSTACK) will be smaller than
** stacksize (equal to ERRORSTACKSIZE in this case), and so the stack
** will be reduced to a "regular" size.
*/
శూన్య luaD_shrinkstack (lua_State *L) {
  పూర్ణాంకం inuse = stackinuse(L);
  పూర్ణాంకం max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  ఒకవేళ (inuse <= MAXSTACK && stacksize(L) > max) {
    పూర్ణాంకం nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  లేకపోతే  /* don't change stack */
    condmovestack(L,(శూన్య)0,(శూన్య)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
శూన్య luaD_hook (lua_State *L, పూర్ణాంకం event, పూర్ణాంకం పంక్తి,
                              పూర్ణాంకం ftransfer, పూర్ణాంకం ntransfer) {
  lua_Hook hook = L->hook;
  ఒకవేళ (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t శిఖరం = savestack(L, L->శిఖరం.p);  /* preserve original 'top' */
    ptrdiff_t ci_శిఖరం = savestack(L, ci->శిఖరం.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = పంక్తి;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    ఒకవేళ (isLua(ci) && L->శిఖరం.p < ci->శిఖరం.p)
      L->శిఖరం.p = ci->శిఖరం.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    ఒకవేళ (ci->శిఖరం.p < L->శిఖరం.p + LUA_MINSTACK)
      ci->శిఖరం.p = L->శిఖరం.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_తాళం(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->శిఖరం.p = restorestack(L, ci_శిఖరం);
    L->శిఖరం.p = restorestack(L, శిఖరం);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
శూన్య luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  ఒకవేళ (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    పూర్ణాంకం event = (ci->callstatus & CIST_తోక) ? LUA_HOOKTAILCALL
                                             : LUA_HOOKCALL;
    Proto *p = ci_func(ci)->p;
    ci->u.l.savedpc++;  /* hooks assume 'pc' is already incremented */
    luaD_hook(L, event, -1, 1, p->numparams);
    ci->u.l.savedpc--;  /* correct 'pc' */
  }
}


/*
** Executes a return hook for Lua and C functions and sets/corrects
** 'oldpc'. (Note that this correction is needed by the line hook, so it
** is done even when return hooks are off.)
*/
స్థిర శూన్య rethook (lua_State *L, CallInfo *ci, పూర్ణాంకం nres) {
  ఒకవేళ (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->శిఖరం.p - nres;  /* index of first result */
    పూర్ణాంకం delta = 0;  /* correction for vararg functions */
    పూర్ణాంకం ftransfer;
    ఒకవేళ (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      ఒకవేళ (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  ఒకవేళ (isLua(ci = ci->మునుపటి))
    L->oldpc = pcRel(ci->u.l.savedpc, ci_func(ci)->p);  /* set 'oldpc' */
}


/*
** Check whether 'func' has a '__call' metafield. If so, put it in the
** stack, below original 'func', so that 'luaD_precall' can call it.
** Raise an error if there is no '__call' metafield.
** Bits CIST_CCMT in status count how many _call metamethods were
** invoked and how many corresponding extra arguments were pushed.
** (This count will be saved in the 'callstatus' of the call).
**  Raise an error if this counter overflows.
*/
స్థిర గుర్తులేని tryfuncTM (lua_State *L, StkId func, గుర్తులేని స్థితి_2) {
  స్థిరరాశి TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  ఒకవేళ (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  కొరకు (p = L->శిఖరం.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->శిఖరం.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  ఒకవేళ ((స్థితి_2 & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  తిరిగి స్థితి_2 + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline శూన్య genmoveresults (lua_State *L, StkId res, పూర్ణాంకం nres,
                                             పూర్ణాంకం wanted) {
  StkId firstresult = L->శిఖరం.p - nres;  /* index of first result */
  పూర్ణాంకం i;
  ఒకవేళ (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  కొరకు (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  కొరకు (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->శిఖరం.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline శూన్య moveresults (lua_State *L, StkId res, పూర్ణాంకం nres,
                                          l_uint32 fwanted) {
  ఎంపిక (fwanted) {  /* handle typical cases separately */
    స్థితి 0 + 1:  /* no values needed */
      L->శిఖరం.p = res;
      తిరిగి;
    స్థితి 1 + 1:  /* one value needed */
      ఒకవేళ (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      లేకపోతే  /* at least one result */
        setobjs2s(L, res, L->శిఖరం.p - nres);  /* move it to proper place */
      L->శిఖరం.p = res + 1;
      తిరిగి;
    స్థితి LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      ఆపు;
    అప్రమేయ: {  /* two/more results and/or to-be-closed variables */
      పూర్ణాంకం wanted = get_nresults(fwanted);
      ఒకవేళ (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        ఒకవేళ (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        ఒకవేళ (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      ఆపు;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
శూన్య luaD_poscall (lua_State *L, CallInfo *ci, పూర్ణాంకం nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  ఒకవేళ (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->మునుపటి;  /* back to caller (after closing variables) */
}



#నిర్వచించు తదుపరి_ci(L)  (L->ci->తదుపరి ? L->ci->తదుపరి : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, గుర్తులేని స్థితి_2,
                                                StkId శిఖరం) {
  CallInfo *ci = L->ci = తదుపరి_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((స్థితి_2 & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = స్థితి_2;
  ci->శిఖరం.p = శిఖరం;
  తిరిగి ci;
}


/*
** precall for C functions
*/
l_sinline పూర్ణాంకం precallC (lua_State *L, StkId func, గుర్తులేని స్థితి_2,
                                            lua_CFunction f) {
  పూర్ణాంకం n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, స్థితి_2 | CIST_C,
                               L->శిఖరం.p + LUA_MINSTACK);
  lua_assert(ci->శిఖరం.p <= L->దొంతర_చివరి.p);
  ఒకవేళ (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    పూర్ణాంకం narg = cast_int(L->శిఖరం.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_తాళం(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  తిరిగి n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
పూర్ణాంకం luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    పూర్ణాంకం narg1, పూర్ణాంకం delta) {
  గుర్తులేని స్థితి_2 = LUA_MULTRET + 1;
 retry:
  ఎంపిక (ttypetag(s2v(func))) {
    స్థితి LUA_VCCL:  /* C closure */
      తిరిగి precallC(L, func, స్థితి_2, clCvalue(s2v(func))->f);
    స్థితి LUA_VLCF:  /* light C function */
      తిరిగి precallC(L, func, స్థితి_2, fvalue(s2v(func)));
    స్థితి LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      పూర్ణాంకం fsize = p->maxstacksize;  /* frame size */
      పూర్ణాంకం nfixparams = p->numparams;
      పూర్ణాంకం i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      కొరకు (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      కొరకు (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->శిఖరం.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->శిఖరం.p <= L->దొంతర_చివరి.p);
      ci->u.l.savedpc = p->సంకేతం_2;  /* starting point */
      ci->callstatus |= CIST_తోక;
      L->శిఖరం.p = func + narg1;  /* set top */
      తిరిగి -1;
    }
    అప్రమేయ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      స్థితి_2 = tryfuncTM(L, func, స్థితి_2);  /* try '__call' metamethod */
      narg1++;
      వెళ్ళు retry;  /* try again */
    }
  }
}


/*
** Prepares the call to a function (C or Lua). For C functions, also do
** the call. The function to be called is at '*func'.  The arguments
** are on the stack, right after the function.  Returns the CallInfo
** to be executed, if it was a Lua function. Otherwise (a C function)
** returns NULL, with all the results on the stack, starting at the
** original function position.
*/
CallInfo *luaD_precall (lua_State *L, StkId func, పూర్ణాంకం nresults) {
  గుర్తులేని స్థితి_2 = cast_uint(nresults + 1);
  lua_assert(స్థితి_2 <= MAXRESULTS + 1);
 retry:
  ఎంపిక (ttypetag(s2v(func))) {
    స్థితి LUA_VCCL:  /* C closure */
      precallC(L, func, స్థితి_2, clCvalue(s2v(func))->f);
      తిరిగి ఖాళీ;
    స్థితి LUA_VLCF:  /* light C function */
      precallC(L, func, స్థితి_2, fvalue(s2v(func)));
      తిరిగి ఖాళీ;
    స్థితి LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      పూర్ణాంకం narg = cast_int(L->శిఖరం.p - func) - 1;  /* number of real arguments */
      పూర్ణాంకం nfixparams = p->numparams;
      పూర్ణాంకం fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, స్థితి_2, func + 1 + fsize);
      ci->u.l.savedpc = p->సంకేతం_2;  /* starting point */
      కొరకు (; narg < nfixparams; narg++)
        setnilvalue2s(L->శిఖరం.p++);  /* complete missing arguments */
      lua_assert(ci->శిఖరం.p <= L->దొంతర_చివరి.p);
      తిరిగి ci;
    }
    అప్రమేయ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      స్థితి_2 = tryfuncTM(L, func, స్థితి_2);  /* try '__call' metamethod */
      వెళ్ళు retry;  /* try again with metamethod */
    }
  }
}


/*
** Call a function (C or Lua) through C. 'inc' can be 1 (increment
** number of recursive invocations in the C stack) or nyci (the same
** plus increment number of non-yieldable calls).
** This function can be called with some use of EXTRA_STACK, so it should
** check the stack before doing anything else. 'luaD_precall' already
** does that.
*/
l_sinline శూన్య ccall (lua_State *L, StkId func, పూర్ణాంకం nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  ఒకవేళ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  ఒకవేళ ((ci = luaD_precall(L, func, nResults)) != ఖాళీ) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
శూన్య luaD_call (lua_State *L, StkId func, పూర్ణాంకం nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
శూన్య luaD_callnoyield (lua_State *L, StkId func, పూర్ణాంకం nResults) {
  ccall(L, func, nResults, nyci);
}


/*
** Finish the job of 'lua_pcallk' after it was interrupted by an yield.
** (The caller, 'finishCcall', does the final call to 'adjustresults'.)
** The main job is to complete the 'luaD_pcall' called by 'lua_pcallk'.
** If a '__close' method yields here, eventually control will be back
** to 'finishCcall' (when that '__close' method finally returns) and
** 'finishpcallk' will run again and close any still pending '__close'
** methods. Similarly, if a '__close' method errs, 'precover' calls
** 'unroll' which calls ''finishCcall' and we are back here again, to
** close any pending '__close' methods.
** Note that, up to the call to 'luaF_close', the corresponding
** 'CallInfo' is not modified, so that this repeated run works like the
** first one (except that it has at least one less '__close' to do). In
** particular, field CIST_RECST preserves the error status across these
** multiple runs, changing only if there is a new error.
*/
స్థిర TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus స్థితి_2 = getcistrecst(ci);  /* get original status */
  ఒకవేళ (l_likely(స్థితి_2 == LUA_OK))  /* no error? */
    స్థితి_2 = LUA_YIELD;  /* was interrupted by an yield */
  లేకపోతే {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, స్థితి_2, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, స్థితి_2, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  తిరిగి స్థితి_2;
}


/*
** Completes the execution of a C function interrupted by an yield.
** The interruption must have happened while the function was either
** closing its tbc variables in 'moveresults' or executing
** 'lua_callk'/'lua_pcallk'. In the first case, it just redoes
** 'luaD_poscall'. In the second case, the call to 'finishpcallk'
** finishes the interrupted execution of 'lua_pcallk'.  After that, it
** calls the continuation of the interrupted function and finally it
** completes the job of the 'luaD_call' that called the function.  In
** the call to 'adjustresults', we do not know the number of results
** of the function called by 'lua_callk'/'lua_pcallk', so we are
** conservative and use LUA_MULTRET (always adjust).
*/
స్థిర శూన్య ముగింపు_Ccall (lua_State *L, CallInfo *ci) {
  పూర్ణాంకం n;  /* actual number of results from C function */
  ఒకవేళ (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  లేకపోతే {
    TStatus స్థితి_2 = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != ఖాళీ && yieldable(L));
    ఒకవేళ (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      స్థితి_2 = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(స్థితి_2), ci->u.c.ctx);  /* call continuation */
    lua_తాళం(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
స్థిర శూన్య unroll (lua_State *L, శూన్య *ud) {
  CallInfo *ci;
  వాడని(ud);
  అయితే ((ci = L->ci) != &L->పునాది_ci) {  /* something in the stack */
    ఒకవేళ (!isLua(ci))  /* C function? */
      ముగింపు_Ccall(L, ci);  /* complete its execution */
    లేకపోతే {  /* Lua function */
      lua_V_ముగింపు_Op(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
స్థిర CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  కొరకు (ci = L->ci; ci != ఖాళీ; ci = ci->మునుపటి) {  /* search for a pcall */
    ఒకవేళ (ci->callstatus & CIST_YPCALL)
      తిరిగి ci;
  }
  తిరిగి ఖాళీ;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
స్థిర పూర్ణాంకం resume_error (lua_State *L, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం narg) {
  api_checkpop(L, narg);
  L->శిఖరం.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->శిఖరం.p, luaS_new(L, సందేశం));  /* push error message */
  api_incr_శిఖరం(L);
  lua_unlock(L);
  తిరిగి LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
స్థిర శూన్య resume (lua_State *L, శూన్య *ud) {
  పూర్ణాంకం n = *(cast(పూర్ణాంకం*, ud));  /* number of arguments */
  StkId మొదటి_Arg = L->శిఖరం.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  ఒకవేళ (L->స్థితి_2 == LUA_OK)  /* starting a coroutine? */
    ccall(L, మొదటి_Arg - 1, LUA_MULTRET, 0);  /* just call its body */
  లేకపోతే {  /* resuming from previous yield */
    lua_assert(L->స్థితి_2 == LUA_YIELD);
    L->స్థితి_2 = LUA_OK;  /* mark that it is running (again) */
    ఒకవేళ (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->శిఖరం.p = మొదటి_Arg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    లేకపోతే {  /* 'common' yield */
      ఒకవేళ (ci->u.c.k != ఖాళీ) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_తాళం(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, ఖాళీ);  /* run continuation */
  }
}


/*
** Unrolls a coroutine in protected mode while there are recoverable
** errors, that is, errors inside a protected call. (Any error
** interrupts 'unroll', and this loop protects it again so it can
** continue.) Stops with a normal end (status == LUA_OK), an yield
** (status == LUA_YIELD), or an unprotected error ('findpcall' doesn't
** find a recover point).
*/
స్థిర TStatus precover (lua_State *L, TStatus స్థితి_2) {
  CallInfo *ci;
  అయితే (errorstatus(స్థితి_2) && (ci = findpcall(L)) != ఖాళీ) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, స్థితి_2);  /* status to finish 'pcall' */
    స్థితి_2 = luaD_rawrunprotected(L, unroll, ఖాళీ);
  }
  తిరిగి స్థితి_2;
}


LUA_API పూర్ణాంకం lua_resume (lua_State *L, lua_State *from, పూర్ణాంకం nargs,
                                      పూర్ణాంకం *nresults) {
  TStatus స్థితి_2;
  lua_తాళం(L);
  ఒకవేళ (L->స్థితి_2 == LUA_OK) {  /* may be starting a coroutine */
    ఒకవేళ (L->ci != &L->పునాది_ci)  /* not in base level? */
      తిరిగి resume_error(L, "cannot resume non-suspended coroutine", nargs);
    లేకపోతే ఒకవేళ (L->శిఖరం.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      తిరిగి resume_error(L, "cannot resume dead coroutine", nargs);
  }
  లేకపోతే ఒకవేళ (L->స్థితి_2 != LUA_YIELD)  /* ended with errors? */
    తిరిగి resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  ఒకవేళ (getCcalls(L) >= LUAI_MAXCCALLS)
    తిరిగి resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->స్థితి_2 == LUA_OK) ? nargs + 1 : nargs);
  స్థితి_2 = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  స్థితి_2 = precover(L, స్థితి_2);
  ఒకవేళ (l_likely(!errorstatus(స్థితి_2)))
    lua_assert(స్థితి_2 == L->స్థితి_2);  /* normal end or yield */
  లేకపోతే {  /* unrecoverable error */
    L->స్థితి_2 = స్థితి_2;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, స్థితి_2, L->శిఖరం.p);  /* push error message */
    L->ci->శిఖరం.p = L->శిఖరం.p;
  }
  *nresults = (స్థితి_2 == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->శిఖరం.p - (L->ci->func.p + 1));
  lua_unlock(L);
  తిరిగి APIstatus(స్థితి_2);
}


LUA_API పూర్ణాంకం lua_isyieldable (lua_State *L) {
  తిరిగి yieldable(L);
}


LUA_API పూర్ణాంకం lua_yieldk (lua_State *L, పూర్ణాంకం nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_తాళం(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  ఒకవేళ (l_unlikely(!yieldable(L))) {
    ఒకవేళ (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    లేకపోతే
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->స్థితి_2 = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  ఒకవేళ (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == ఖాళీ, "hooks cannot continue after yielding");
  }
  లేకపోతే {
    ఒకవేళ ((ci->u.c.k = k) != ఖాళీ)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  తిరిగి 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
నిర్మాణం CloseP {
  StkId level;
  TStatus స్థితి_2;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
స్థిర శూన్య closepaux (lua_State *L, శూన్య *ud) {
  నిర్మాణం CloseP *pcl = cast(నిర్మాణం CloseP *, ud);
  luaF_close(L, pcl->level, pcl->స్థితి_2, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus స్థితి_2) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  కొరకు (;;) {  /* keep closing upvalues until no more errors */
    నిర్మాణం CloseP pcl;
    pcl.level = restorestack(L, level); pcl.స్థితి_2 = స్థితి_2;
    స్థితి_2 = luaD_rawrunprotected(L, &closepaux, &pcl);
    ఒకవేళ (l_likely(స్థితి_2 == LUA_OK))  /* no more errors? */
      తిరిగి pcl.స్థితి_2;
    లేకపోతే {  /* an error occurred; restore saved state and repeat */
      L->ci = old_ci;
      L->allowhook = old_allowhooks;
    }
  }
}


/*
** Call the C function 'func' in protected mode, restoring basic
** thread information ('allowhook', etc.) and in particular
** its stack level in case of errors.
*/
TStatus luaD_pcall (lua_State *L, Pfunc func, శూన్య *u, ptrdiff_t old_శిఖరం,
                                  ptrdiff_t ef) {
  TStatus స్థితి_2;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  స్థితి_2 = luaD_rawrunprotected(L, func, u);
  ఒకవేళ (l_unlikely(స్థితి_2 != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    స్థితి_2 = luaD_closeprotected(L, old_శిఖరం, స్థితి_2);
    luaD_seterrorobj(L, స్థితి_2, restorestack(L, old_శిఖరం));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  తిరిగి స్థితి_2;
}



/*
** Execute a protected parser.
*/
నిర్మాణం SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  స్థిరరాశి అక్షరం *బహుళకం;
  స్థిరరాశి అక్షరం *name;
};


స్థిర శూన్య checkmode (lua_State *L, స్థిరరాశి అక్షరం *బహుళకం, స్థిరరాశి అక్షరం *x) {
  ఒకవేళ (strchr(బహుళకం, x[0]) == ఖాళీ) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, బహుళకం);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
స్థిర శూన్య f_parser (lua_State *L, శూన్య *ud) {
  LClosure *cl;
  నిర్మాణం SParser *p = cast(నిర్మాణం SParser *, ud);
  స్థిరరాశి అక్షరం *బహుళకం = p->బహుళకం ? p->బహుళకం : "bt";
  పూర్ణాంకం c;
  పట్టిక *anchor;
  ptrdiff_t otop = savestack(L, L->శిఖరం.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->శిఖరం.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  ఒకవేళ (c == LUA_SIGNATURE[0]) {
    పూర్ణాంకం fixed = 0;
    ఒకవేళ (strchr(బహుళకం, 'B') != ఖాళీ)
      fixed = 1;
    లేకపోతే
      checkmode(L, బహుళకం, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  లేకపోతే {
    checkmode(L, బహుళకం, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->శిఖరం.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->శిఖరం.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
శూన్య luaD_anchorobj (lua_State *L, పట్టిక *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->శిఖరం.p++), obj);  /* temporary anchor in the stack */
  lua_H_సమితి(L, anchor, s2v(L->శిఖరం.p - 1), s2v(L->శిఖరం.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->శిఖరం.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, స్థిరరాశి అక్షరం *name,
                                            స్థిరరాశి అక్షరం *బహుళకం) {
  నిర్మాణం SParser p;
  TStatus స్థితి_2;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.బహుళకం = బహుళకం;
  p.dyd.actvar.arr = ఖాళీ; p.dyd.actvar.కొలత = 0;
  p.dyd.gt.arr = ఖాళీ; p.dyd.gt.కొలత = 0;
  p.dyd.label.arr = ఖాళీ; p.dyd.label.కొలత = 0;
  luaZ_initbuffer(L, &p.buff);
  స్థితి_2 = luaD_pcall(L, f_parser, &p, savestack(L, L->శిఖరం.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.కొలత));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.కొలత));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.కొలత));
  decnny(L);
  తిరిగి స్థితి_2;
}


