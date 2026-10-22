/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#define ldo_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <setjmp.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lopcodes.h"
#অন্তর্ভাব "lparser.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "ltm.h"
#অন্তর্ভাব "lundump.h"
#অন্তর্ভাব "lvm.h"
#অন্তর্ভাব "lzio.h"



#define errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#যদি !defined(luai_userstateresume)
#define luai_userstateresume(L,n)	((শূন্য)L)
#endif

#যদি !defined(luai_userstateyield)
#define luai_userstateyield(L,n)	((শূন্য)L)
#endif


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
প্রকারলেখ কাঠামো lua_longjmp {
  কাঠামো lua_longjmp *previous;
  jmp_buf b;
  অস্থির TStatus status;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#যদি !defined(LUAI_THROW)				/* { */

#যদি defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#define LUAI_THROW(L,c)		throw(c)

স্থির শূন্য LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, শূন্য *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    যদি (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->status = -1;  /* create some error code */
  }
}


#নতুবা_যদি defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#define LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#define LUAI_TRY(L,c,f,ud)	যদি (_setjmp((c)->b) == 0) ((f)(L, ud))

#অন্যথা							/* }{ */

/* ISO C handling with long jumps */
#define LUAI_THROW(L,c)		longjmp((c)->b, 1)
#define LUAI_TRY(L,c,f,ud)	যদি (setjmp((c)->b) == 0) ((f)(L, ud))

#endif							/* } */

#endif							/* } */


শূন্য luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  যদি (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  অন্যথা {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->শীর্ষ.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->শীর্ষ.p - 1);  /* move it to 'oldtop' */
  }
  L->শীর্ষ.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  যদি (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->status = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  অন্যথা {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->status = errcode;
    যদি (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->শীর্ষ.p++, L->শীর্ষ.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    অন্যথা {  /* no handler at all; abort */
      যদি (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  যদি (L->errorJmp) {
    /* unroll error entries up to the first level */
    যখন (L->errorJmp->previous != ফাঁকা)
      L->errorJmp = L->errorJmp->previous;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, শূন্য *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.status = LUA_OK;
  lj.previous = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.previous;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  ফেরত lj.status;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#define STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#যদি !defined(LUAI_MAXSTACK)
#যদি 1000000 < (INT_MAX / 2)
#define LUAI_MAXSTACK           1000000
#অন্যথা
#define LUAI_MAXSTACK           (INT_MAX / 2u)
#endif
#endif


/* maximum stack size that respects size_t */
#define MAXSTACK_BYSIZET  ((MAX_SIZET / আকার(স্তূপ_মান)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#define MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#define ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *বার্তা = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->শীর্ষ.p, বার্তা);
  L->শীর্ষ.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
পূর্ণসংখ্যা luaD_checkminstack (lua_State *L) {
  যদি (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    ফেরত 0;  /* not enough C-stack slots */
  যদি (L->ci->পরবর্তী == ফাঁকা && luaE_extendCI(L, 0) == ফাঁকা)
    ফেরত 0;  /* unable to allocate first ci */
  যদি (L->ci->পরবর্তী->পরবর্তী == ফাঁকা && luaE_extendCI(L, 0) == ফাঁকা)
    ফেরত 0;  /* unable to allocate second ci */
  যদি (L->স্তূপ_last.p - L->শীর্ষ.p >= BASIC_স্তূপ_আকার)
    ফেরত 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  অন্যথা  /* try to grow stack to a size with enough free slots */
    ফেরত luaD_growstack(L, BASIC_স্তূপ_আকার, 0);
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
#যদি !defined(LUAI_STRICT_ADDRESS)
#define LUAI_STRICT_ADDRESS	1
#endif

#যদি LUAI_STRICT_ADDRESS
/*
** Change all pointers to the stack into offsets.
*/
স্থির শূন্য relstack (lua_State *L) {
  CallInfo *ci;
  UpVal *up;
  L->শীর্ষ.offset = savestack(L, L->শীর্ষ.p);
  L->tbclist.offset = savestack(L, L->tbclist.p);
  জন্য (up = L->openupval; up != ফাঁকা; up = up->u.open.পরবর্তী)
    up->v.offset = savestack(L, uplevel(up));
  জন্য (ci = L->ci; ci != ফাঁকা; ci = ci->previous) {
    ci->শীর্ষ.offset = savestack(L, ci->শীর্ষ.p);
    ci->func.offset = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
স্থির শূন্য correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  UpVal *up;
  UNUSED(oldstack);
  L->শীর্ষ.p = restorestack(L, L->শীর্ষ.offset);
  L->tbclist.p = restorestack(L, L->tbclist.offset);
  জন্য (up = L->openupval; up != ফাঁকা; up = up->u.open.পরবর্তী)
    up->v.p = s2v(restorestack(L, up->v.offset));
  জন্য (ci = L->ci; ci != ফাঁকা; ci = ci->previous) {
    ci->শীর্ষ.p = restorestack(L, ci->শীর্ষ.offset);
    ci->func.p = restorestack(L, ci->func.offset);
    যদি (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#অন্যথা
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

স্থির শূন্য relstack (lua_State *L) { UNUSED(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
স্থির শূন্য correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  UpVal *up;
  StkId newstack = L->স্তূপ.p;
  যদি (oldstack == newstack)
    ফেরত;
  L->শীর্ষ.p = L->শীর্ষ.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  জন্য (up = L->openupval; up != ফাঁকা; up = up->u.open.পরবর্তী)
    up->v.p = s2v(uplevel(up) - oldstack + newstack);
  জন্য (ci = L->ci; ci != ফাঁকা; ci = ci->previous) {
    ci->শীর্ষ.p = ci->শীর্ষ.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    যদি (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#endif


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
পূর্ণসংখ্যা luaD_reallocstack (lua_State *L, পূর্ণসংখ্যা newsize, পূর্ণসংখ্যা raiseerror) {
  পূর্ণসংখ্যা oldsize = stacksize(L);
  পূর্ণসংখ্যা i;
  StkId newstack;
  StkId oldstack = L->স্তূপ.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_স্তূপ,
                                   newsize + EXTRA_স্তূপ, স্তূপ_মান);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  যদি (l_unlikely(newstack == ফাঁকা)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    যদি (raiseerror)
      luaM_error(L);
    অন্যথা ফেরত 0;  /* do not raise an error */
  }
  L->স্তূপ.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->স্তূপ_last.p = L->স্তূপ.p + newsize;
  জন্য (i = oldsize + EXTRA_স্তূপ; i < newsize + EXTRA_স্তূপ; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  ফেরত 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
পূর্ণসংখ্যা luaD_growstack (lua_State *L, পূর্ণসংখ্যা n, পূর্ণসংখ্যা raiseerror) {
  পূর্ণসংখ্যা আকার_2 = stacksize(L);
  যদি (l_unlikely(আকার_2 > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    যদি (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    ফেরত 0;  /* if not 'raiseerror', just signal it */
  }
  অন্যথা যদি (n < MAXSTACK) {  /* avoids arithmetic overflows */
    পূর্ণসংখ্যা newsize = আকার_2 + (আকার_2 >> 1);  /* tentative new size (size * 1.5) */
    পূর্ণসংখ্যা needed = cast_int(L->শীর্ষ.p - L->স্তূপ.p) + n;
    যদি (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    যদি (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    যদি (l_likely(newsize <= MAXSTACK))
      ফেরত luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  যদি (raiseerror)
    luaG_runerror(L, "stack overflow");
  ফেরত 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
স্থির পূর্ণসংখ্যা stackinuse (lua_State *L) {
  CallInfo *ci;
  পূর্ণসংখ্যা res;
  StkId lim = L->শীর্ষ.p;
  জন্য (ci = L->ci; ci != ফাঁকা; ci = ci->previous) {
    যদি (lim < ci->শীর্ষ.p) lim = ci->শীর্ষ.p;
  }
  lua_assert(lim <= L->স্তূপ_last.p + EXTRA_স্তূপ);
  res = cast_int(lim - L->স্তূপ.p) + 1;  /* part of stack in use */
  যদি (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  ফেরত res;
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
শূন্য luaD_shrinkstack (lua_State *L) {
  পূর্ণসংখ্যা inuse = stackinuse(L);
  পূর্ণসংখ্যা max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  যদি (inuse <= MAXSTACK && stacksize(L) > max) {
    পূর্ণসংখ্যা nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  অন্যথা  /* don't change stack */
    condmovestack(L,(শূন্য)0,(শূন্য)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
শূন্য luaD_hook (lua_State *L, পূর্ণসংখ্যা event, পূর্ণসংখ্যা সারি,
                              পূর্ণসংখ্যা ftransfer, পূর্ণসংখ্যা ntransfer) {
  lua_Hook hook = L->hook;
  যদি (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t শীর্ষ = savestack(L, L->শীর্ষ.p);  /* preserve original 'top' */
    ptrdiff_t ci_শীর্ষ = savestack(L, ci->শীর্ষ.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = সারি;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    যদি (isLua(ci) && L->শীর্ষ.p < ci->শীর্ষ.p)
      L->শীর্ষ.p = ci->শীর্ষ.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    যদি (ci->শীর্ষ.p < L->শীর্ষ.p + LUA_MINSTACK)
      ci->শীর্ষ.p = L->শীর্ষ.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_lock(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->শীর্ষ.p = restorestack(L, ci_শীর্ষ);
    L->শীর্ষ.p = restorestack(L, শীর্ষ);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
শূন্য luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  যদি (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    পূর্ণসংখ্যা event = (ci->callstatus & CIST_লেজ) ? LUA_HOOKTAILCALL
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
স্থির শূন্য rethook (lua_State *L, CallInfo *ci, পূর্ণসংখ্যা nres) {
  যদি (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->শীর্ষ.p - nres;  /* index of first result */
    পূর্ণসংখ্যা delta = 0;  /* correction for vararg functions */
    পূর্ণসংখ্যা ftransfer;
    যদি (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      যদি (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  যদি (isLua(ci = ci->previous))
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
স্থির unsigned tryfuncTM (lua_State *L, StkId func, unsigned status) {
  ধ্রুবক TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  যদি (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  জন্য (p = L->শীর্ষ.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->শীর্ষ.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  যদি ((status & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  ফেরত status + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline শূন্য genmoveresults (lua_State *L, StkId res, পূর্ণসংখ্যা nres,
                                             পূর্ণসংখ্যা wanted) {
  StkId firstresult = L->শীর্ষ.p - nres;  /* index of first result */
  পূর্ণসংখ্যা i;
  যদি (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  জন্য (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  জন্য (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->শীর্ষ.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline শূন্য moveresults (lua_State *L, StkId res, পূর্ণসংখ্যা nres,
                                          l_uint32 fwanted) {
  নির্বাচন (fwanted) {  /* handle typical cases separately */
    ক্ষেত্র 0 + 1:  /* no values needed */
      L->শীর্ষ.p = res;
      ফেরত;
    ক্ষেত্র 1 + 1:  /* one value needed */
      যদি (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      অন্যথা  /* at least one result */
        setobjs2s(L, res, L->শীর্ষ.p - nres);  /* move it to proper place */
      L->শীর্ষ.p = res + 1;
      ফেরত;
    ক্ষেত্র LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      বিরতি;
    পূর্বনির্ধারিত: {  /* two/more results and/or to-be-closed variables */
      পূর্ণসংখ্যা wanted = get_nresults(fwanted);
      যদি (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        যদি (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        যদি (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      বিরতি;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
শূন্য luaD_poscall (lua_State *L, CallInfo *ci, পূর্ণসংখ্যা nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  যদি (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->previous;  /* back to caller (after closing variables) */
}



#define পরবর্তী_ci(L)  (L->ci->পরবর্তী ? L->ci->পরবর্তী : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, unsigned status,
                                                StkId শীর্ষ) {
  CallInfo *ci = L->ci = পরবর্তী_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((status & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = status;
  ci->শীর্ষ.p = শীর্ষ;
  ফেরত ci;
}


/*
** precall for C functions
*/
l_sinline পূর্ণসংখ্যা precallC (lua_State *L, StkId func, unsigned status,
                                            lua_CFunction f) {
  পূর্ণসংখ্যা n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, status | CIST_C,
                               L->শীর্ষ.p + LUA_MINSTACK);
  lua_assert(ci->শীর্ষ.p <= L->স্তূপ_last.p);
  যদি (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    পূর্ণসংখ্যা narg = cast_int(L->শীর্ষ.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_lock(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  ফেরত n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
পূর্ণসংখ্যা luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    পূর্ণসংখ্যা narg1, পূর্ণসংখ্যা delta) {
  unsigned status = LUA_MULTRET + 1;
 retry:
  নির্বাচন (ttypetag(s2v(func))) {
    ক্ষেত্র LUA_VCCL:  /* C closure */
      ফেরত precallC(L, func, status, clCvalue(s2v(func))->f);
    ক্ষেত্র LUA_VLCF:  /* light C function */
      ফেরত precallC(L, func, status, fvalue(s2v(func)));
    ক্ষেত্র LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      পূর্ণসংখ্যা fsize = p->maxstacksize;  /* frame size */
      পূর্ণসংখ্যা nfixparams = p->numparams;
      পূর্ণসংখ্যা i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      জন্য (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      জন্য (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->শীর্ষ.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->শীর্ষ.p <= L->স্তূপ_last.p);
      ci->u.l.savedpc = p->code;  /* starting point */
      ci->callstatus |= CIST_লেজ;
      L->শীর্ষ.p = func + narg1;  /* set top */
      ফেরত -1;
    }
    পূর্বনির্ধারিত: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      status = tryfuncTM(L, func, status);  /* try '__call' metamethod */
      narg1++;
      যাও retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, পূর্ণসংখ্যা nresults) {
  unsigned status = cast_uint(nresults + 1);
  lua_assert(status <= MAXRESULTS + 1);
 retry:
  নির্বাচন (ttypetag(s2v(func))) {
    ক্ষেত্র LUA_VCCL:  /* C closure */
      precallC(L, func, status, clCvalue(s2v(func))->f);
      ফেরত ফাঁকা;
    ক্ষেত্র LUA_VLCF:  /* light C function */
      precallC(L, func, status, fvalue(s2v(func)));
      ফেরত ফাঁকা;
    ক্ষেত্র LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      পূর্ণসংখ্যা narg = cast_int(L->শীর্ষ.p - func) - 1;  /* number of real arguments */
      পূর্ণসংখ্যা nfixparams = p->numparams;
      পূর্ণসংখ্যা fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, status, func + 1 + fsize);
      ci->u.l.savedpc = p->code;  /* starting point */
      জন্য (; narg < nfixparams; narg++)
        setnilvalue2s(L->শীর্ষ.p++);  /* complete missing arguments */
      lua_assert(ci->শীর্ষ.p <= L->স্তূপ_last.p);
      ফেরত ci;
    }
    পূর্বনির্ধারিত: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      status = tryfuncTM(L, func, status);  /* try '__call' metamethod */
      যাও retry;  /* try again with metamethod */
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
l_sinline শূন্য ccall (lua_State *L, StkId func, পূর্ণসংখ্যা nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  যদি (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  যদি ((ci = luaD_precall(L, func, nResults)) != ফাঁকা) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
শূন্য luaD_call (lua_State *L, StkId func, পূর্ণসংখ্যা nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
শূন্য luaD_callnoyield (lua_State *L, StkId func, পূর্ণসংখ্যা nResults) {
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
স্থির TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus status = getcistrecst(ci);  /* get original status */
  যদি (l_likely(status == LUA_OK))  /* no error? */
    status = LUA_YIELD;  /* was interrupted by an yield */
  অন্যথা {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, status, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, status, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  ফেরত status;
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
স্থির শূন্য finishCcall (lua_State *L, CallInfo *ci) {
  পূর্ণসংখ্যা n;  /* actual number of results from C function */
  যদি (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  অন্যথা {
    TStatus status = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != ফাঁকা && yieldable(L));
    যদি (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      status = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(status), ci->u.c.ctx);  /* call continuation */
    lua_lock(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
স্থির শূন্য unroll (lua_State *L, শূন্য *ud) {
  CallInfo *ci;
  UNUSED(ud);
  যখন ((ci = L->ci) != &L->base_ci) {  /* something in the stack */
    যদি (!isLua(ci))  /* C function? */
      finishCcall(L, ci);  /* complete its execution */
    অন্যথা {  /* Lua function */
      luaV_finishOp(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
স্থির CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  জন্য (ci = L->ci; ci != ফাঁকা; ci = ci->previous) {  /* search for a pcall */
    যদি (ci->callstatus & CIST_YPCALL)
      ফেরত ci;
  }
  ফেরত ফাঁকা;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
স্থির পূর্ণসংখ্যা resume_error (lua_State *L, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা narg) {
  api_checkpop(L, narg);
  L->শীর্ষ.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->শীর্ষ.p, luaS_new(L, বার্তা));  /* push error message */
  api_incr_শীর্ষ(L);
  lua_unlock(L);
  ফেরত LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
স্থির শূন্য resume (lua_State *L, শূন্য *ud) {
  পূর্ণসংখ্যা n = *(cast(পূর্ণসংখ্যা*, ud));  /* number of arguments */
  StkId firstArg = L->শীর্ষ.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  যদি (L->status == LUA_OK)  /* starting a coroutine? */
    ccall(L, firstArg - 1, LUA_MULTRET, 0);  /* just call its body */
  অন্যথা {  /* resuming from previous yield */
    lua_assert(L->status == LUA_YIELD);
    L->status = LUA_OK;  /* mark that it is running (again) */
    যদি (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->শীর্ষ.p = firstArg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    অন্যথা {  /* 'common' yield */
      যদি (ci->u.c.k != ফাঁকা) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_lock(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, ফাঁকা);  /* run continuation */
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
স্থির TStatus precover (lua_State *L, TStatus status) {
  CallInfo *ci;
  যখন (errorstatus(status) && (ci = findpcall(L)) != ফাঁকা) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, status);  /* status to finish 'pcall' */
    status = luaD_rawrunprotected(L, unroll, ফাঁকা);
  }
  ফেরত status;
}


LUA_API পূর্ণসংখ্যা lua_resume (lua_State *L, lua_State *from, পূর্ণসংখ্যা nargs,
                                      পূর্ণসংখ্যা *nresults) {
  TStatus status;
  lua_lock(L);
  যদি (L->status == LUA_OK) {  /* may be starting a coroutine */
    যদি (L->ci != &L->base_ci)  /* not in base level? */
      ফেরত resume_error(L, "cannot resume non-suspended coroutine", nargs);
    অন্যথা যদি (L->শীর্ষ.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      ফেরত resume_error(L, "cannot resume dead coroutine", nargs);
  }
  অন্যথা যদি (L->status != LUA_YIELD)  /* ended with errors? */
    ফেরত resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  যদি (getCcalls(L) >= LUAI_MAXCCALLS)
    ফেরত resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->status == LUA_OK) ? nargs + 1 : nargs);
  status = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  status = precover(L, status);
  যদি (l_likely(!errorstatus(status)))
    lua_assert(status == L->status);  /* normal end or yield */
  অন্যথা {  /* unrecoverable error */
    L->status = status;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, status, L->শীর্ষ.p);  /* push error message */
    L->ci->শীর্ষ.p = L->শীর্ষ.p;
  }
  *nresults = (status == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->শীর্ষ.p - (L->ci->func.p + 1));
  lua_unlock(L);
  ফেরত APIstatus(status);
}


LUA_API পূর্ণসংখ্যা lua_isyieldable (lua_State *L) {
  ফেরত yieldable(L);
}


LUA_API পূর্ণসংখ্যা lua_yieldk (lua_State *L, পূর্ণসংখ্যা nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_lock(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  যদি (l_unlikely(!yieldable(L))) {
    যদি (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    অন্যথা
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->status = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  যদি (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == ফাঁকা, "hooks cannot continue after yielding");
  }
  অন্যথা {
    যদি ((ci->u.c.k = k) != ফাঁকা)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  ফেরত 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
কাঠামো CloseP {
  StkId level;
  TStatus status;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
স্থির শূন্য closepaux (lua_State *L, শূন্য *ud) {
  কাঠামো CloseP *pcl = cast(কাঠামো CloseP *, ud);
  luaF_close(L, pcl->level, pcl->status, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus status) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  জন্য (;;) {  /* keep closing upvalues until no more errors */
    কাঠামো CloseP pcl;
    pcl.level = restorestack(L, level); pcl.status = status;
    status = luaD_rawrunprotected(L, &closepaux, &pcl);
    যদি (l_likely(status == LUA_OK))  /* no more errors? */
      ফেরত pcl.status;
    অন্যথা {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, শূন্য *u, ptrdiff_t old_শীর্ষ,
                                  ptrdiff_t ef) {
  TStatus status;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  status = luaD_rawrunprotected(L, func, u);
  যদি (l_unlikely(status != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    status = luaD_closeprotected(L, old_শীর্ষ, status);
    luaD_seterrorobj(L, status, restorestack(L, old_শীর্ষ));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  ফেরত status;
}



/*
** Execute a protected parser.
*/
কাঠামো SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ধ্রুবক অক্ষর *mode;
  ধ্রুবক অক্ষর *name;
};


স্থির শূন্য checkmode (lua_State *L, ধ্রুবক অক্ষর *mode, ধ্রুবক অক্ষর *x) {
  যদি (strchr(mode, x[0]) == ফাঁকা) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, mode);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
স্থির শূন্য f_parser (lua_State *L, শূন্য *ud) {
  LClosure *cl;
  কাঠামো SParser *p = cast(কাঠামো SParser *, ud);
  ধ্রুবক অক্ষর *mode = p->mode ? p->mode : "bt";
  পূর্ণসংখ্যা c;
  Table *anchor;
  ptrdiff_t otop = savestack(L, L->শীর্ষ.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->শীর্ষ.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  যদি (c == LUA_SIGNATURE[0]) {
    পূর্ণসংখ্যা fixed = 0;
    যদি (strchr(mode, 'B') != ফাঁকা)
      fixed = 1;
    অন্যথা
      checkmode(L, mode, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  অন্যথা {
    checkmode(L, mode, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->শীর্ষ.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->শীর্ষ.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
শূন্য luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->শীর্ষ.p++), obj);  /* temporary anchor in the stack */
  luaH_set(L, anchor, s2v(L->শীর্ষ.p - 1), s2v(L->শীর্ষ.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->শীর্ষ.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ধ্রুবক অক্ষর *name,
                                            ধ্রুবক অক্ষর *mode) {
  কাঠামো SParser p;
  TStatus status;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.mode = mode;
  p.dyd.actvar.arr = ফাঁকা; p.dyd.actvar.আকার_2 = 0;
  p.dyd.gt.arr = ফাঁকা; p.dyd.gt.আকার_2 = 0;
  p.dyd.label.arr = ফাঁকা; p.dyd.label.আকার_2 = 0;
  luaZ_initbuffer(L, &p.buff);
  status = luaD_pcall(L, f_parser, &p, savestack(L, L->শীর্ষ.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.আকার_2));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.আকার_2));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.আকার_2));
  decnny(L);
  ফেরত status;
}


