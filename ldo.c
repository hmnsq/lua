/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#وصف_ڪريو ldo_c
#وصف_ڪريو LUA_CORE

#ضمن "lprefix.h"


#ضمن <setjmp.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lgc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lopcodes.h"
#ضمن "lparser.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "ltm.h"
#ضمن "lundump.h"
#ضمن "lvm.h"
#ضمن "lzio.h"



#وصف_ڪريو errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#إذا !defined(luai_userstateresume)
#وصف_ڪريو luai_userstateresume(L,n)	((فراغ)L)
#جيڪڏهن_پورو

#إذا !defined(luai_userstateyield)
#وصف_ڪريو luai_userstateyield(L,n)	((فراغ)L)
#جيڪڏهن_پورو


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
تعريفنوع بنية lua_longjmp {
  بنية lua_longjmp *اڳيون;
  jmp_buf b;
  متطاير TStatus حالت;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#إذا !defined(LUAI_THROW)				/* { */

#إذا defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#وصف_ڪريو LUAI_THROW(L,c)		throw(c)

ساكن فراغ LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, فراغ *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    إذا (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->حالت = -1;  /* create some error code */
  }
}


#وإلاإذا defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#وصف_ڪريو LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#وصف_ڪريو LUAI_TRY(L,c,f,ud)	إذا (_setjmp((c)->b) == 0) ((f)(L, ud))

#وإلا							/* }{ */

/* ISO C handling with long jumps */
#وصف_ڪريو LUAI_THROW(L,c)		longjmp((c)->b, 1)
#وصف_ڪريو LUAI_TRY(L,c,f,ud)	إذا (setjmp((c)->b) == 0) ((f)(L, ud))

#جيڪڏهن_پورو							/* } */

#جيڪڏهن_پورو							/* } */


فراغ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  إذا (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  وإلا {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->چوٽي.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->چوٽي.p - 1);  /* move it to 'oldtop' */
  }
  L->چوٽي.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  إذا (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->حالت = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  وإلا {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->حالت = errcode;
    إذا (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->چوٽي.p++, L->چوٽي.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    وإلا {  /* no handler at all; abort */
      إذا (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  إذا (L->errorJmp) {
    /* unroll error entries up to the first level */
    طالما (L->errorJmp->اڳيون != عدم)
      L->errorJmp = L->errorJmp->اڳيون;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, فراغ *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.حالت = LUA_OK;
  lj.اڳيون = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.اڳيون;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  أرجع lj.حالت;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#وصف_ڪريو STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#إذا !defined(LUAI_MAXSTACK)
#إذا 1000000 < (وڌ_کان_وڌ_عدد / 2)
#وصف_ڪريو LUAI_MAXSTACK           1000000
#وإلا
#وصف_ڪريو LUAI_MAXSTACK           (وڌ_کان_وڌ_عدد / 2u)
#جيڪڏهن_پورو
#جيڪڏهن_پورو


/* maximum stack size that respects size_t */
#وصف_ڪريو MAXSTACK_BYSIZET  ((MAX_SIZET / حجم(ڍير_قدر)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#وصف_ڪريو MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#وصف_ڪريو ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *نياپو = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->چوٽي.p, نياپو);
  L->چوٽي.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
صحيح luaD_checkminstack (lua_State *L) {
  إذا (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    أرجع 0;  /* not enough C-stack slots */
  إذا (L->ci->ايندڙ == عدم && luaE_extendCI(L, 0) == عدم)
    أرجع 0;  /* unable to allocate first ci */
  إذا (L->ci->ايندڙ->ايندڙ == عدم && luaE_extendCI(L, 0) == عدم)
    أرجع 0;  /* unable to allocate second ci */
  إذا (L->ڍير_آخري.p - L->چوٽي.p >= BASIC_ڍير_سائيز)
    أرجع 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  وإلا  /* try to grow stack to a size with enough free slots */
    أرجع luaD_growstack(L, BASIC_ڍير_سائيز, 0);
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
#إذا !defined(LUAI_STRICT_پتو)
#وصف_ڪريو LUAI_STRICT_پتو	1
#جيڪڏهن_پورو

#إذا LUAI_STRICT_پتو
/*
** Change all pointers to the stack into offsets.
*/
ساكن فراغ relstack (lua_State *L) {
  CallInfo *ci;
  مٿي_Val *مٿي;
  L->چوٽي.وٿي = savestack(L, L->چوٽي.p);
  L->tbclist.وٿي = savestack(L, L->tbclist.p);
  لكل (مٿي = L->openupval; مٿي != عدم; مٿي = مٿي->u.open.ايندڙ)
    مٿي->v.وٿي = savestack(L, uplevel(مٿي));
  لكل (ci = L->ci; ci != عدم; ci = ci->اڳيون) {
    ci->چوٽي.وٿي = savestack(L, ci->چوٽي.p);
    ci->func.وٿي = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
ساكن فراغ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  مٿي_Val *مٿي;
  اڻ_استعمال_ٿيل(oldstack);
  L->چوٽي.p = restorestack(L, L->چوٽي.وٿي);
  L->tbclist.p = restorestack(L, L->tbclist.وٿي);
  لكل (مٿي = L->openupval; مٿي != عدم; مٿي = مٿي->u.open.ايندڙ)
    مٿي->v.p = s2v(restorestack(L, مٿي->v.وٿي));
  لكل (ci = L->ci; ci != عدم; ci = ci->اڳيون) {
    ci->چوٽي.p = restorestack(L, ci->چوٽي.وٿي);
    ci->func.p = restorestack(L, ci->func.وٿي);
    إذا (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#وإلا
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

ساكن فراغ relstack (lua_State *L) { اڻ_استعمال_ٿيل(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
ساكن فراغ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  مٿي_Val *مٿي;
  StkId newstack = L->ڍير.p;
  إذا (oldstack == newstack)
    أرجع;
  L->چوٽي.p = L->چوٽي.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  لكل (مٿي = L->openupval; مٿي != عدم; مٿي = مٿي->u.open.ايندڙ)
    مٿي->v.p = s2v(uplevel(مٿي) - oldstack + newstack);
  لكل (ci = L->ci; ci != عدم; ci = ci->اڳيون) {
    ci->چوٽي.p = ci->چوٽي.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    إذا (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#جيڪڏهن_پورو


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
صحيح luaD_reallocstack (lua_State *L, صحيح newsize, صحيح raiseerror) {
  صحيح oldsize = stacksize(L);
  صحيح i;
  StkId newstack;
  StkId oldstack = L->ڍير.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_ڍير,
                                   newsize + EXTRA_ڍير, ڍير_قدر);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  إذا (l_unlikely(newstack == عدم)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    إذا (raiseerror)
      luaM_error(L);
    وإلا أرجع 0;  /* do not raise an error */
  }
  L->ڍير.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->ڍير_آخري.p = L->ڍير.p + newsize;
  لكل (i = oldsize + EXTRA_ڍير; i < newsize + EXTRA_ڍير; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  أرجع 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
صحيح luaD_growstack (lua_State *L, صحيح n, صحيح raiseerror) {
  صحيح سائيز = stacksize(L);
  إذا (l_unlikely(سائيز > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    إذا (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    أرجع 0;  /* if not 'raiseerror', just signal it */
  }
  وإلا إذا (n < MAXSTACK) {  /* avoids arithmetic overflows */
    صحيح newsize = سائيز + (سائيز >> 1);  /* tentative new size (size * 1.5) */
    صحيح needed = cast_int(L->چوٽي.p - L->ڍير.p) + n;
    إذا (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    إذا (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    إذا (l_likely(newsize <= MAXSTACK))
      أرجع luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  إذا (raiseerror)
    luaG_runerror(L, "stack overflow");
  أرجع 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
ساكن صحيح stackinuse (lua_State *L) {
  CallInfo *ci;
  صحيح res;
  StkId lim = L->چوٽي.p;
  لكل (ci = L->ci; ci != عدم; ci = ci->اڳيون) {
    إذا (lim < ci->چوٽي.p) lim = ci->چوٽي.p;
  }
  lua_assert(lim <= L->ڍير_آخري.p + EXTRA_ڍير);
  res = cast_int(lim - L->ڍير.p) + 1;  /* part of stack in use */
  إذا (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  أرجع res;
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
فراغ luaD_shrinkstack (lua_State *L) {
  صحيح inuse = stackinuse(L);
  صحيح max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  إذا (inuse <= MAXSTACK && stacksize(L) > max) {
    صحيح nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  وإلا  /* don't change stack */
    condmovestack(L,(فراغ)0,(فراغ)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
فراغ luaD_hook (lua_State *L, صحيح event, صحيح سٽ,
                              صحيح ftransfer, صحيح ntransfer) {
  lua_Hook hook = L->hook;
  إذا (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t چوٽي = savestack(L, L->چوٽي.p);  /* preserve original 'top' */
    ptrdiff_t ci_چوٽي = savestack(L, ci->چوٽي.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = سٽ;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    إذا (isLua(ci) && L->چوٽي.p < ci->چوٽي.p)
      L->چوٽي.p = ci->چوٽي.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    إذا (ci->چوٽي.p < L->چوٽي.p + LUA_MINSTACK)
      ci->چوٽي.p = L->چوٽي.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_تالو(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->چوٽي.p = restorestack(L, ci_چوٽي);
    L->چوٽي.p = restorestack(L, چوٽي);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
فراغ luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  إذا (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    صحيح event = (ci->callstatus & CIST_پڇ) ? LUA_HOOKTAILCALL
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
ساكن فراغ rethook (lua_State *L, CallInfo *ci, صحيح nres) {
  إذا (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->چوٽي.p - nres;  /* index of first result */
    صحيح delta = 0;  /* correction for vararg functions */
    صحيح ftransfer;
    إذا (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      إذا (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  إذا (isLua(ci = ci->اڳيون))
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
ساكن بنانشان tryfuncTM (lua_State *L, StkId func, بنانشان حالت) {
  ثابت TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  إذا (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  لكل (p = L->چوٽي.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->چوٽي.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  إذا ((حالت & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  أرجع حالت + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline فراغ genmoveresults (lua_State *L, StkId res, صحيح nres,
                                             صحيح wanted) {
  StkId firstresult = L->چوٽي.p - nres;  /* index of first result */
  صحيح i;
  إذا (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  لكل (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  لكل (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->چوٽي.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline فراغ moveresults (lua_State *L, StkId res, صحيح nres,
                                          l_uint32 fwanted) {
  بدل (fwanted) {  /* handle typical cases separately */
    حالة 0 + 1:  /* no values needed */
      L->چوٽي.p = res;
      أرجع;
    حالة 1 + 1:  /* one value needed */
      إذا (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      وإلا  /* at least one result */
        setobjs2s(L, res, L->چوٽي.p - nres);  /* move it to proper place */
      L->چوٽي.p = res + 1;
      أرجع;
    حالة LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      اكسر;
    افتراضي: {  /* two/more results and/or to-be-closed variables */
      صحيح wanted = get_nresults(fwanted);
      إذا (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        إذا (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        إذا (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      اكسر;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
فراغ luaD_poscall (lua_State *L, CallInfo *ci, صحيح nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  إذا (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->اڳيون;  /* back to caller (after closing variables) */
}



#وصف_ڪريو ايندڙ_ci(L)  (L->ci->ايندڙ ? L->ci->ايندڙ : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, بنانشان حالت,
                                                StkId چوٽي) {
  CallInfo *ci = L->ci = ايندڙ_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((حالت & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = حالت;
  ci->چوٽي.p = چوٽي;
  أرجع ci;
}


/*
** precall for C functions
*/
l_sinline صحيح precallC (lua_State *L, StkId func, بنانشان حالت,
                                            lua_CFunction f) {
  صحيح n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, حالت | CIST_C,
                               L->چوٽي.p + LUA_MINSTACK);
  lua_assert(ci->چوٽي.p <= L->ڍير_آخري.p);
  إذا (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    صحيح narg = cast_int(L->چوٽي.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_تالو(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  أرجع n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
صحيح luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    صحيح narg1, صحيح delta) {
  بنانشان حالت = LUA_MULTRET + 1;
 retry:
  بدل (ttypetag(s2v(func))) {
    حالة LUA_VCCL:  /* C closure */
      أرجع precallC(L, func, حالت, clCvalue(s2v(func))->f);
    حالة LUA_VLCF:  /* light C function */
      أرجع precallC(L, func, حالت, fvalue(s2v(func)));
    حالة LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      صحيح fsize = p->maxstacksize;  /* frame size */
      صحيح nfixparams = p->numparams;
      صحيح i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      لكل (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      لكل (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->چوٽي.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->چوٽي.p <= L->ڍير_آخري.p);
      ci->u.l.savedpc = p->ڪوڊ;  /* starting point */
      ci->callstatus |= CIST_پڇ;
      L->چوٽي.p = func + narg1;  /* set top */
      أرجع -1;
    }
    افتراضي: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      حالت = tryfuncTM(L, func, حالت);  /* try '__call' metamethod */
      narg1++;
      اذهب retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, صحيح nresults) {
  بنانشان حالت = cast_uint(nresults + 1);
  lua_assert(حالت <= MAXRESULTS + 1);
 retry:
  بدل (ttypetag(s2v(func))) {
    حالة LUA_VCCL:  /* C closure */
      precallC(L, func, حالت, clCvalue(s2v(func))->f);
      أرجع عدم;
    حالة LUA_VLCF:  /* light C function */
      precallC(L, func, حالت, fvalue(s2v(func)));
      أرجع عدم;
    حالة LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      صحيح narg = cast_int(L->چوٽي.p - func) - 1;  /* number of real arguments */
      صحيح nfixparams = p->numparams;
      صحيح fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, حالت, func + 1 + fsize);
      ci->u.l.savedpc = p->ڪوڊ;  /* starting point */
      لكل (; narg < nfixparams; narg++)
        setnilvalue2s(L->چوٽي.p++);  /* complete missing arguments */
      lua_assert(ci->چوٽي.p <= L->ڍير_آخري.p);
      أرجع ci;
    }
    افتراضي: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      حالت = tryfuncTM(L, func, حالت);  /* try '__call' metamethod */
      اذهب retry;  /* try again with metamethod */
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
l_sinline فراغ ccall (lua_State *L, StkId func, صحيح nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  إذا (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  إذا ((ci = luaD_precall(L, func, nResults)) != عدم) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
فراغ luaD_call (lua_State *L, StkId func, صحيح nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
فراغ luaD_callnoyield (lua_State *L, StkId func, صحيح nResults) {
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
ساكن TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus حالت = getcistrecst(ci);  /* get original status */
  إذا (l_likely(حالت == LUA_OK))  /* no error? */
    حالت = LUA_YIELD;  /* was interrupted by an yield */
  وإلا {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, حالت, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, حالت, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  أرجع حالت;
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
ساكن فراغ پڄاڻي_Ccall (lua_State *L, CallInfo *ci) {
  صحيح n;  /* actual number of results from C function */
  إذا (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  وإلا {
    TStatus حالت = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != عدم && yieldable(L));
    إذا (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      حالت = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(حالت), ci->u.c.ctx);  /* call continuation */
    lua_تالو(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
ساكن فراغ unroll (lua_State *L, فراغ *ud) {
  CallInfo *ci;
  اڻ_استعمال_ٿيل(ud);
  طالما ((ci = L->ci) != &L->بنياد_ci) {  /* something in the stack */
    إذا (!isLua(ci))  /* C function? */
      پڄاڻي_Ccall(L, ci);  /* complete its execution */
    وإلا {  /* Lua function */
      lua_V_پڄاڻي_Op(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
ساكن CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  لكل (ci = L->ci; ci != عدم; ci = ci->اڳيون) {  /* search for a pcall */
    إذا (ci->callstatus & CIST_YPCALL)
      أرجع ci;
  }
  أرجع عدم;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
ساكن صحيح resume_error (lua_State *L, ثابت حرف *نياپو, صحيح narg) {
  api_checkpop(L, narg);
  L->چوٽي.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->چوٽي.p, luaS_new(L, نياپو));  /* push error message */
  api_incr_چوٽي(L);
  lua_unlock(L);
  أرجع LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
ساكن فراغ resume (lua_State *L, فراغ *ud) {
  صحيح n = *(cast(صحيح*, ud));  /* number of arguments */
  StkId پهريون_Arg = L->چوٽي.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  إذا (L->حالت == LUA_OK)  /* starting a coroutine? */
    ccall(L, پهريون_Arg - 1, LUA_MULTRET, 0);  /* just call its body */
  وإلا {  /* resuming from previous yield */
    lua_assert(L->حالت == LUA_YIELD);
    L->حالت = LUA_OK;  /* mark that it is running (again) */
    إذا (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->چوٽي.p = پهريون_Arg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    وإلا {  /* 'common' yield */
      إذا (ci->u.c.k != عدم) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_تالو(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, عدم);  /* run continuation */
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
ساكن TStatus precover (lua_State *L, TStatus حالت) {
  CallInfo *ci;
  طالما (errorstatus(حالت) && (ci = findpcall(L)) != عدم) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, حالت);  /* status to finish 'pcall' */
    حالت = luaD_rawrunprotected(L, unroll, عدم);
  }
  أرجع حالت;
}


LUA_API صحيح lua_resume (lua_State *L, lua_State *from, صحيح nargs,
                                      صحيح *nresults) {
  TStatus حالت;
  lua_تالو(L);
  إذا (L->حالت == LUA_OK) {  /* may be starting a coroutine */
    إذا (L->ci != &L->بنياد_ci)  /* not in base level? */
      أرجع resume_error(L, "cannot resume non-suspended coroutine", nargs);
    وإلا إذا (L->چوٽي.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      أرجع resume_error(L, "cannot resume dead coroutine", nargs);
  }
  وإلا إذا (L->حالت != LUA_YIELD)  /* ended with errors? */
    أرجع resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  إذا (getCcalls(L) >= LUAI_MAXCCALLS)
    أرجع resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->حالت == LUA_OK) ? nargs + 1 : nargs);
  حالت = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  حالت = precover(L, حالت);
  إذا (l_likely(!errorstatus(حالت)))
    lua_assert(حالت == L->حالت);  /* normal end or yield */
  وإلا {  /* unrecoverable error */
    L->حالت = حالت;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, حالت, L->چوٽي.p);  /* push error message */
    L->ci->چوٽي.p = L->چوٽي.p;
  }
  *nresults = (حالت == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->چوٽي.p - (L->ci->func.p + 1));
  lua_unlock(L);
  أرجع APIstatus(حالت);
}


LUA_API صحيح lua_isyieldable (lua_State *L) {
  أرجع yieldable(L);
}


LUA_API صحيح lua_yieldk (lua_State *L, صحيح nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_تالو(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  إذا (l_unlikely(!yieldable(L))) {
    إذا (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    وإلا
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->حالت = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  إذا (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == عدم, "hooks cannot continue after yielding");
  }
  وإلا {
    إذا ((ci->u.c.k = k) != عدم)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  أرجع 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
بنية CloseP {
  StkId level;
  TStatus حالت;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
ساكن فراغ closepaux (lua_State *L, فراغ *ud) {
  بنية CloseP *pcl = cast(بنية CloseP *, ud);
  luaF_close(L, pcl->level, pcl->حالت, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus حالت) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  لكل (;;) {  /* keep closing upvalues until no more errors */
    بنية CloseP pcl;
    pcl.level = restorestack(L, level); pcl.حالت = حالت;
    حالت = luaD_rawrunprotected(L, &closepaux, &pcl);
    إذا (l_likely(حالت == LUA_OK))  /* no more errors? */
      أرجع pcl.حالت;
    وإلا {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, فراغ *u, ptrdiff_t old_چوٽي,
                                  ptrdiff_t ef) {
  TStatus حالت;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  حالت = luaD_rawrunprotected(L, func, u);
  إذا (l_unlikely(حالت != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    حالت = luaD_closeprotected(L, old_چوٽي, حالت);
    luaD_seterrorobj(L, حالت, restorestack(L, old_چوٽي));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  أرجع حالت;
}



/*
** Execute a protected parser.
*/
بنية SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ثابت حرف *گهڻائي;
  ثابت حرف *name;
};


ساكن فراغ checkmode (lua_State *L, ثابت حرف *گهڻائي, ثابت حرف *x) {
  إذا (strchr(گهڻائي, x[0]) == عدم) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, گهڻائي);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
ساكن فراغ f_parser (lua_State *L, فراغ *ud) {
  LClosure *cl;
  بنية SParser *p = cast(بنية SParser *, ud);
  ثابت حرف *گهڻائي = p->گهڻائي ? p->گهڻائي : "bt";
  صحيح c;
  جدول *anchor;
  ptrdiff_t otop = savestack(L, L->چوٽي.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->چوٽي.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  إذا (c == LUA_SIGNATURE[0]) {
    صحيح fixed = 0;
    إذا (strchr(گهڻائي, 'B') != عدم)
      fixed = 1;
    وإلا
      checkmode(L, گهڻائي, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  وإلا {
    checkmode(L, گهڻائي, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->چوٽي.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->چوٽي.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
فراغ luaD_anchorobj (lua_State *L, جدول *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->چوٽي.p++), obj);  /* temporary anchor in the stack */
  lua_H_سيٽ(L, anchor, s2v(L->چوٽي.p - 1), s2v(L->چوٽي.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->چوٽي.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ثابت حرف *name,
                                            ثابت حرف *گهڻائي) {
  بنية SParser p;
  TStatus حالت;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.گهڻائي = گهڻائي;
  p.dyd.actvar.arr = عدم; p.dyd.actvar.سائيز = 0;
  p.dyd.gt.arr = عدم; p.dyd.gt.سائيز = 0;
  p.dyd.label.arr = عدم; p.dyd.label.سائيز = 0;
  luaZ_initbuffer(L, &p.buff);
  حالت = luaD_pcall(L, f_parser, &p, savestack(L, L->چوٽي.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.سائيز));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.سائيز));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.سائيز));
  decnny(L);
  أرجع حالت;
}


