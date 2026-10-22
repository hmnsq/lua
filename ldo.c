/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#تعریف_کرو ldo_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <setjmp.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lparser.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"
#شامل "lundump.h"
#شامل "lvm.h"
#شامل "lzio.h"



#تعریف_کرو errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#اگر !defined(luai_userstateresume)
#تعریف_کرو luai_userstateresume(L,n)	((خالی)L)
#اگر_ختم

#اگر !defined(luai_userstateyield)
#تعریف_کرو luai_userstateyield(L,n)	((خالی)L)
#اگر_ختم


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
قسمتعریف ساخت lua_longjmp {
  ساخت lua_longjmp *پچھلا;
  jmp_buf b;
  متلون TStatus حالت_2;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#اگر !defined(LUAI_THROW)				/* { */

#اگر defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#تعریف_کرو LUAI_THROW(L,c)		throw(c)

ساکن خالی LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, خالی *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    اگر (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->حالت_2 = -1;  /* create some error code */
  }
}


#وگرنہ_اگر defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#تعریف_کرو LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#تعریف_کرو LUAI_TRY(L,c,f,ud)	اگر (_setjmp((c)->b) == 0) ((f)(L, ud))

#ورنہ							/* }{ */

/* ISO C handling with long jumps */
#تعریف_کرو LUAI_THROW(L,c)		longjmp((c)->b, 1)
#تعریف_کرو LUAI_TRY(L,c,f,ud)	اگر (setjmp((c)->b) == 0) ((f)(L, ud))

#اگر_ختم							/* } */

#اگر_ختم							/* } */


خالی luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  اگر (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  ورنہ {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->چوٹی.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->چوٹی.p - 1);  /* move it to 'oldtop' */
  }
  L->چوٹی.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  اگر (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->حالت_2 = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  ورنہ {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->حالت_2 = errcode;
    اگر (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->چوٹی.p++, L->چوٹی.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    ورنہ {  /* no handler at all; abort */
      اگر (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  اگر (L->errorJmp) {
    /* unroll error entries up to the first level */
    جب (L->errorJmp->پچھلا != تہی)
      L->errorJmp = L->errorJmp->پچھلا;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, خالی *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.حالت_2 = LUA_OK;
  lj.پچھلا = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.پچھلا;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  واپس lj.حالت_2;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#تعریف_کرو STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#اگر !defined(LUAI_MAXSTACK)
#اگر 1000000 < (زیادہ_سے_زیادہ_عدد / 2)
#تعریف_کرو LUAI_MAXSTACK           1000000
#ورنہ
#تعریف_کرو LUAI_MAXSTACK           (زیادہ_سے_زیادہ_عدد / 2u)
#اگر_ختم
#اگر_ختم


/* maximum stack size that respects size_t */
#تعریف_کرو MAXSTACK_BYSIZET  ((MAX_SIZET / حجم(ڈھیر_قدر)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#تعریف_کرو MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#تعریف_کرو ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *پیغام = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->چوٹی.p, پیغام);
  L->چوٹی.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
صحیح luaD_checkminstack (lua_State *L) {
  اگر (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    واپس 0;  /* not enough C-stack slots */
  اگر (L->ci->اگلا == تہی && luaE_extendCI(L, 0) == تہی)
    واپس 0;  /* unable to allocate first ci */
  اگر (L->ci->اگلا->اگلا == تہی && luaE_extendCI(L, 0) == تہی)
    واپس 0;  /* unable to allocate second ci */
  اگر (L->ڈھیر_آخری.p - L->چوٹی.p >= BASIC_ڈھیر_سائز)
    واپس 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  ورنہ  /* try to grow stack to a size with enough free slots */
    واپس luaD_growstack(L, BASIC_ڈھیر_سائز, 0);
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
#اگر !defined(LUAI_STRICT_پتہ)
#تعریف_کرو LUAI_STRICT_پتہ	1
#اگر_ختم

#اگر LUAI_STRICT_پتہ
/*
** Change all pointers to the stack into offsets.
*/
ساکن خالی relstack (lua_State *L) {
  CallInfo *ci;
  اوپر_Val *اوپر;
  L->چوٹی.انحراف = savestack(L, L->چوٹی.p);
  L->tbclist.انحراف = savestack(L, L->tbclist.p);
  برائے (اوپر = L->openupval; اوپر != تہی; اوپر = اوپر->u.open.اگلا)
    اوپر->v.انحراف = savestack(L, uplevel(اوپر));
  برائے (ci = L->ci; ci != تہی; ci = ci->پچھلا) {
    ci->چوٹی.انحراف = savestack(L, ci->چوٹی.p);
    ci->func.انحراف = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
ساکن خالی correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  اوپر_Val *اوپر;
  غیر_مستعمل(oldstack);
  L->چوٹی.p = restorestack(L, L->چوٹی.انحراف);
  L->tbclist.p = restorestack(L, L->tbclist.انحراف);
  برائے (اوپر = L->openupval; اوپر != تہی; اوپر = اوپر->u.open.اگلا)
    اوپر->v.p = s2v(restorestack(L, اوپر->v.انحراف));
  برائے (ci = L->ci; ci != تہی; ci = ci->پچھلا) {
    ci->چوٹی.p = restorestack(L, ci->چوٹی.انحراف);
    ci->func.p = restorestack(L, ci->func.انحراف);
    اگر (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#ورنہ
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

ساکن خالی relstack (lua_State *L) { غیر_مستعمل(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
ساکن خالی correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  اوپر_Val *اوپر;
  StkId newstack = L->ڈھیر.p;
  اگر (oldstack == newstack)
    واپس;
  L->چوٹی.p = L->چوٹی.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  برائے (اوپر = L->openupval; اوپر != تہی; اوپر = اوپر->u.open.اگلا)
    اوپر->v.p = s2v(uplevel(اوپر) - oldstack + newstack);
  برائے (ci = L->ci; ci != تہی; ci = ci->پچھلا) {
    ci->چوٹی.p = ci->چوٹی.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    اگر (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#اگر_ختم


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
صحیح luaD_reallocstack (lua_State *L, صحیح newsize, صحیح raiseerror) {
  صحیح oldsize = stacksize(L);
  صحیح i;
  StkId newstack;
  StkId oldstack = L->ڈھیر.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_ڈھیر,
                                   newsize + EXTRA_ڈھیر, ڈھیر_قدر);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  اگر (l_unlikely(newstack == تہی)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    اگر (raiseerror)
      luaM_error(L);
    ورنہ واپس 0;  /* do not raise an error */
  }
  L->ڈھیر.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->ڈھیر_آخری.p = L->ڈھیر.p + newsize;
  برائے (i = oldsize + EXTRA_ڈھیر; i < newsize + EXTRA_ڈھیر; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  واپس 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
صحیح luaD_growstack (lua_State *L, صحیح n, صحیح raiseerror) {
  صحیح سائز = stacksize(L);
  اگر (l_unlikely(سائز > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    اگر (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    واپس 0;  /* if not 'raiseerror', just signal it */
  }
  ورنہ اگر (n < MAXSTACK) {  /* avoids arithmetic overflows */
    صحیح newsize = سائز + (سائز >> 1);  /* tentative new size (size * 1.5) */
    صحیح needed = cast_int(L->چوٹی.p - L->ڈھیر.p) + n;
    اگر (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    اگر (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    اگر (l_likely(newsize <= MAXSTACK))
      واپس luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  اگر (raiseerror)
    luaG_runerror(L, "stack overflow");
  واپس 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
ساکن صحیح stackinuse (lua_State *L) {
  CallInfo *ci;
  صحیح res;
  StkId lim = L->چوٹی.p;
  برائے (ci = L->ci; ci != تہی; ci = ci->پچھلا) {
    اگر (lim < ci->چوٹی.p) lim = ci->چوٹی.p;
  }
  lua_assert(lim <= L->ڈھیر_آخری.p + EXTRA_ڈھیر);
  res = cast_int(lim - L->ڈھیر.p) + 1;  /* part of stack in use */
  اگر (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  واپس res;
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
خالی luaD_shrinkstack (lua_State *L) {
  صحیح inuse = stackinuse(L);
  صحیح max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  اگر (inuse <= MAXSTACK && stacksize(L) > max) {
    صحیح nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  ورنہ  /* don't change stack */
    condmovestack(L,(خالی)0,(خالی)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
خالی luaD_hook (lua_State *L, صحیح event, صحیح سطر,
                              صحیح ftransfer, صحیح ntransfer) {
  lua_Hook hook = L->hook;
  اگر (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t چوٹی = savestack(L, L->چوٹی.p);  /* preserve original 'top' */
    ptrdiff_t ci_چوٹی = savestack(L, ci->چوٹی.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = سطر;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    اگر (isLua(ci) && L->چوٹی.p < ci->چوٹی.p)
      L->چوٹی.p = ci->چوٹی.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    اگر (ci->چوٹی.p < L->چوٹی.p + LUA_MINSTACK)
      ci->چوٹی.p = L->چوٹی.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_تالا(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->چوٹی.p = restorestack(L, ci_چوٹی);
    L->چوٹی.p = restorestack(L, چوٹی);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
خالی luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  اگر (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    صحیح event = (ci->callstatus & CIST_دم) ? LUA_HOOKTAILCALL
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
ساکن خالی rethook (lua_State *L, CallInfo *ci, صحیح nres) {
  اگر (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->چوٹی.p - nres;  /* index of first result */
    صحیح delta = 0;  /* correction for vararg functions */
    صحیح ftransfer;
    اگر (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      اگر (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  اگر (isLua(ci = ci->پچھلا))
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
ساکن غیرعلامتی tryfuncTM (lua_State *L, StkId func, غیرعلامتی حالت_2) {
  ثابت TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  اگر (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  برائے (p = L->چوٹی.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->چوٹی.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  اگر ((حالت_2 & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  واپس حالت_2 + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline خالی genmoveresults (lua_State *L, StkId res, صحیح nres,
                                             صحیح wanted) {
  StkId firstresult = L->چوٹی.p - nres;  /* index of first result */
  صحیح i;
  اگر (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  برائے (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  برائے (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->چوٹی.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline خالی moveresults (lua_State *L, StkId res, صحیح nres,
                                          l_uint32 fwanted) {
  انتخاب (fwanted) {  /* handle typical cases separately */
    حالت 0 + 1:  /* no values needed */
      L->چوٹی.p = res;
      واپس;
    حالت 1 + 1:  /* one value needed */
      اگر (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      ورنہ  /* at least one result */
        setobjs2s(L, res, L->چوٹی.p - nres);  /* move it to proper place */
      L->چوٹی.p = res + 1;
      واپس;
    حالت LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      توڑو;
    طےشدہ: {  /* two/more results and/or to-be-closed variables */
      صحیح wanted = get_nresults(fwanted);
      اگر (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        اگر (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        اگر (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      توڑو;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
خالی luaD_poscall (lua_State *L, CallInfo *ci, صحیح nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  اگر (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->پچھلا;  /* back to caller (after closing variables) */
}



#تعریف_کرو اگلا_ci(L)  (L->ci->اگلا ? L->ci->اگلا : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, غیرعلامتی حالت_2,
                                                StkId چوٹی) {
  CallInfo *ci = L->ci = اگلا_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((حالت_2 & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = حالت_2;
  ci->چوٹی.p = چوٹی;
  واپس ci;
}


/*
** precall for C functions
*/
l_sinline صحیح precallC (lua_State *L, StkId func, غیرعلامتی حالت_2,
                                            lua_CFunction f) {
  صحیح n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, حالت_2 | CIST_C,
                               L->چوٹی.p + LUA_MINSTACK);
  lua_assert(ci->چوٹی.p <= L->ڈھیر_آخری.p);
  اگر (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    صحیح narg = cast_int(L->چوٹی.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_تالا(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  واپس n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
صحیح luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    صحیح narg1, صحیح delta) {
  غیرعلامتی حالت_2 = LUA_MULTRET + 1;
 retry:
  انتخاب (ttypetag(s2v(func))) {
    حالت LUA_VCCL:  /* C closure */
      واپس precallC(L, func, حالت_2, clCvalue(s2v(func))->f);
    حالت LUA_VLCF:  /* light C function */
      واپس precallC(L, func, حالت_2, fvalue(s2v(func)));
    حالت LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      صحیح fsize = p->maxstacksize;  /* frame size */
      صحیح nfixparams = p->numparams;
      صحیح i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      برائے (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      برائے (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->چوٹی.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->چوٹی.p <= L->ڈھیر_آخری.p);
      ci->u.l.savedpc = p->رمز;  /* starting point */
      ci->callstatus |= CIST_دم;
      L->چوٹی.p = func + narg1;  /* set top */
      واپس -1;
    }
    طےشدہ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      حالت_2 = tryfuncTM(L, func, حالت_2);  /* try '__call' metamethod */
      narg1++;
      جاؤ retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, صحیح nresults) {
  غیرعلامتی حالت_2 = cast_uint(nresults + 1);
  lua_assert(حالت_2 <= MAXRESULTS + 1);
 retry:
  انتخاب (ttypetag(s2v(func))) {
    حالت LUA_VCCL:  /* C closure */
      precallC(L, func, حالت_2, clCvalue(s2v(func))->f);
      واپس تہی;
    حالت LUA_VLCF:  /* light C function */
      precallC(L, func, حالت_2, fvalue(s2v(func)));
      واپس تہی;
    حالت LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      صحیح narg = cast_int(L->چوٹی.p - func) - 1;  /* number of real arguments */
      صحیح nfixparams = p->numparams;
      صحیح fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, حالت_2, func + 1 + fsize);
      ci->u.l.savedpc = p->رمز;  /* starting point */
      برائے (; narg < nfixparams; narg++)
        setnilvalue2s(L->چوٹی.p++);  /* complete missing arguments */
      lua_assert(ci->چوٹی.p <= L->ڈھیر_آخری.p);
      واپس ci;
    }
    طےشدہ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      حالت_2 = tryfuncTM(L, func, حالت_2);  /* try '__call' metamethod */
      جاؤ retry;  /* try again with metamethod */
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
l_sinline خالی ccall (lua_State *L, StkId func, صحیح nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  اگر (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  اگر ((ci = luaD_precall(L, func, nResults)) != تہی) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
خالی luaD_call (lua_State *L, StkId func, صحیح nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
خالی luaD_callnoyield (lua_State *L, StkId func, صحیح nResults) {
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
ساکن TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus حالت_2 = getcistrecst(ci);  /* get original status */
  اگر (l_likely(حالت_2 == LUA_OK))  /* no error? */
    حالت_2 = LUA_YIELD;  /* was interrupted by an yield */
  ورنہ {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, حالت_2, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, حالت_2, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  واپس حالت_2;
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
ساکن خالی اختتام_Ccall (lua_State *L, CallInfo *ci) {
  صحیح n;  /* actual number of results from C function */
  اگر (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  ورنہ {
    TStatus حالت_2 = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != تہی && yieldable(L));
    اگر (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      حالت_2 = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(حالت_2), ci->u.c.ctx);  /* call continuation */
    lua_تالا(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
ساکن خالی unroll (lua_State *L, خالی *ud) {
  CallInfo *ci;
  غیر_مستعمل(ud);
  جب ((ci = L->ci) != &L->بنیاد_ci) {  /* something in the stack */
    اگر (!isLua(ci))  /* C function? */
      اختتام_Ccall(L, ci);  /* complete its execution */
    ورنہ {  /* Lua function */
      lua_V_اختتام_Op(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
ساکن CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  برائے (ci = L->ci; ci != تہی; ci = ci->پچھلا) {  /* search for a pcall */
    اگر (ci->callstatus & CIST_YPCALL)
      واپس ci;
  }
  واپس تہی;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
ساکن صحیح resume_error (lua_State *L, ثابت حرف *پیغام, صحیح narg) {
  api_checkpop(L, narg);
  L->چوٹی.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->چوٹی.p, luaS_new(L, پیغام));  /* push error message */
  api_incr_چوٹی(L);
  lua_unlock(L);
  واپس LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
ساکن خالی resume (lua_State *L, خالی *ud) {
  صحیح n = *(cast(صحیح*, ud));  /* number of arguments */
  StkId پہلا_Arg = L->چوٹی.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  اگر (L->حالت_2 == LUA_OK)  /* starting a coroutine? */
    ccall(L, پہلا_Arg - 1, LUA_MULTRET, 0);  /* just call its body */
  ورنہ {  /* resuming from previous yield */
    lua_assert(L->حالت_2 == LUA_YIELD);
    L->حالت_2 = LUA_OK;  /* mark that it is running (again) */
    اگر (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->چوٹی.p = پہلا_Arg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    ورنہ {  /* 'common' yield */
      اگر (ci->u.c.k != تہی) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_تالا(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, تہی);  /* run continuation */
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
ساکن TStatus precover (lua_State *L, TStatus حالت_2) {
  CallInfo *ci;
  جب (errorstatus(حالت_2) && (ci = findpcall(L)) != تہی) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, حالت_2);  /* status to finish 'pcall' */
    حالت_2 = luaD_rawrunprotected(L, unroll, تہی);
  }
  واپس حالت_2;
}


LUA_API صحیح lua_resume (lua_State *L, lua_State *from, صحیح nargs,
                                      صحیح *nresults) {
  TStatus حالت_2;
  lua_تالا(L);
  اگر (L->حالت_2 == LUA_OK) {  /* may be starting a coroutine */
    اگر (L->ci != &L->بنیاد_ci)  /* not in base level? */
      واپس resume_error(L, "cannot resume non-suspended coroutine", nargs);
    ورنہ اگر (L->چوٹی.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      واپس resume_error(L, "cannot resume dead coroutine", nargs);
  }
  ورنہ اگر (L->حالت_2 != LUA_YIELD)  /* ended with errors? */
    واپس resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  اگر (getCcalls(L) >= LUAI_MAXCCALLS)
    واپس resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->حالت_2 == LUA_OK) ? nargs + 1 : nargs);
  حالت_2 = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  حالت_2 = precover(L, حالت_2);
  اگر (l_likely(!errorstatus(حالت_2)))
    lua_assert(حالت_2 == L->حالت_2);  /* normal end or yield */
  ورنہ {  /* unrecoverable error */
    L->حالت_2 = حالت_2;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, حالت_2, L->چوٹی.p);  /* push error message */
    L->ci->چوٹی.p = L->چوٹی.p;
  }
  *nresults = (حالت_2 == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->چوٹی.p - (L->ci->func.p + 1));
  lua_unlock(L);
  واپس APIstatus(حالت_2);
}


LUA_API صحیح lua_isyieldable (lua_State *L) {
  واپس yieldable(L);
}


LUA_API صحیح lua_yieldk (lua_State *L, صحیح nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_تالا(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  اگر (l_unlikely(!yieldable(L))) {
    اگر (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    ورنہ
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->حالت_2 = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  اگر (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == تہی, "hooks cannot continue after yielding");
  }
  ورنہ {
    اگر ((ci->u.c.k = k) != تہی)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  واپس 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
ساخت CloseP {
  StkId level;
  TStatus حالت_2;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
ساکن خالی closepaux (lua_State *L, خالی *ud) {
  ساخت CloseP *pcl = cast(ساخت CloseP *, ud);
  luaF_close(L, pcl->level, pcl->حالت_2, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus حالت_2) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  برائے (;;) {  /* keep closing upvalues until no more errors */
    ساخت CloseP pcl;
    pcl.level = restorestack(L, level); pcl.حالت_2 = حالت_2;
    حالت_2 = luaD_rawrunprotected(L, &closepaux, &pcl);
    اگر (l_likely(حالت_2 == LUA_OK))  /* no more errors? */
      واپس pcl.حالت_2;
    ورنہ {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, خالی *u, ptrdiff_t old_چوٹی,
                                  ptrdiff_t ef) {
  TStatus حالت_2;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  حالت_2 = luaD_rawrunprotected(L, func, u);
  اگر (l_unlikely(حالت_2 != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    حالت_2 = luaD_closeprotected(L, old_چوٹی, حالت_2);
    luaD_seterrorobj(L, حالت_2, restorestack(L, old_چوٹی));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  واپس حالت_2;
}



/*
** Execute a protected parser.
*/
ساخت SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ثابت حرف *منوال;
  ثابت حرف *name;
};


ساکن خالی checkmode (lua_State *L, ثابت حرف *منوال, ثابت حرف *x) {
  اگر (strchr(منوال, x[0]) == تہی) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, منوال);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
ساکن خالی f_parser (lua_State *L, خالی *ud) {
  LClosure *cl;
  ساخت SParser *p = cast(ساخت SParser *, ud);
  ثابت حرف *منوال = p->منوال ? p->منوال : "bt";
  صحیح c;
  جدول *anchor;
  ptrdiff_t otop = savestack(L, L->چوٹی.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->چوٹی.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  اگر (c == LUA_SIGNATURE[0]) {
    صحیح fixed = 0;
    اگر (strchr(منوال, 'B') != تہی)
      fixed = 1;
    ورنہ
      checkmode(L, منوال, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  ورنہ {
    checkmode(L, منوال, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->چوٹی.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->چوٹی.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
خالی luaD_anchorobj (lua_State *L, جدول *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->چوٹی.p++), obj);  /* temporary anchor in the stack */
  lua_H_سیٹ(L, anchor, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->چوٹی.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ثابت حرف *name,
                                            ثابت حرف *منوال) {
  ساخت SParser p;
  TStatus حالت_2;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.منوال = منوال;
  p.dyd.actvar.arr = تہی; p.dyd.actvar.سائز = 0;
  p.dyd.gt.arr = تہی; p.dyd.gt.سائز = 0;
  p.dyd.label.arr = تہی; p.dyd.label.سائز = 0;
  luaZ_initbuffer(L, &p.buff);
  حالت_2 = luaD_pcall(L, f_parser, &p, savestack(L, L->چوٹی.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.سائز));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.سائز));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.سائز));
  decnny(L);
  واپس حالت_2;
}


