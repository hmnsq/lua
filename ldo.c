/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#define ldo_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <setjmp.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lparser.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"
#അന്തര്ഭാവ "lundump.h"
#അന്തര്ഭാവ "lvm.h"
#അന്തര്ഭാവ "lzio.h"



#define errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#എങ്കിൽ !defined(luai_userstateresume)
#define luai_userstateresume(L,n)	((ശൂന്യം)L)
#endif

#എങ്കിൽ !defined(luai_userstateyield)
#define luai_userstateyield(L,n)	((ശൂന്യം)L)
#endif


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
തരനിർവചനം ഘടന lua_longjmp {
  ഘടന lua_longjmp *previous;
  jmp_buf b;
  അസ്ഥിര TStatus status;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#എങ്കിൽ !defined(LUAI_THROW)				/* { */

#എങ്കിൽ defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#define LUAI_THROW(L,c)		throw(c)

സ്ഥിതം ശൂന്യം LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, ശൂന്യം *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    എങ്കിൽ (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->status = -1;  /* create some error code */
  }
}


#അല്ലെങ്കിലെങ്കിൽ defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#define LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#define LUAI_TRY(L,c,f,ud)	എങ്കിൽ (_setjmp((c)->b) == 0) ((f)(L, ud))

#അല്ലെങ്കിൽ							/* }{ */

/* ISO C handling with long jumps */
#define LUAI_THROW(L,c)		longjmp((c)->b, 1)
#define LUAI_TRY(L,c,f,ud)	എങ്കിൽ (setjmp((c)->b) == 0) ((f)(L, ud))

#endif							/* } */

#endif							/* } */


ശൂന്യം luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  എങ്കിൽ (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  അല്ലെങ്കിൽ {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->മുകൾ.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->മുകൾ.p - 1);  /* move it to 'oldtop' */
  }
  L->മുകൾ.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  എങ്കിൽ (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->status = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  അല്ലെങ്കിൽ {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->status = errcode;
    എങ്കിൽ (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->മുകൾ.p++, L->മുകൾ.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    അല്ലെങ്കിൽ {  /* no handler at all; abort */
      എങ്കിൽ (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  എങ്കിൽ (L->errorJmp) {
    /* unroll error entries up to the first level */
    ആകുമ്പോൾ (L->errorJmp->previous != ശൂന്യ)
      L->errorJmp = L->errorJmp->previous;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ശൂന്യം *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.status = LUA_OK;
  lj.previous = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.previous;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  തിരികെ lj.status;
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

#എങ്കിൽ !defined(LUAI_MAXSTACK)
#എങ്കിൽ 1000000 < (INT_MAX / 2)
#define LUAI_MAXSTACK           1000000
#അല്ലെങ്കിൽ
#define LUAI_MAXSTACK           (INT_MAX / 2u)
#endif
#endif


/* maximum stack size that respects size_t */
#define MAXSTACK_BYSIZET  ((MAX_SIZET / വലുപ്പം(അട്ടി_മൂല്യം)) - STACKERRSPACE)

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
  TString *സന്ദേശം = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->മുകൾ.p, സന്ദേശം);
  L->മുകൾ.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
പൂർണ്ണസംഖ്യ luaD_checkminstack (lua_State *L) {
  എങ്കിൽ (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    തിരികെ 0;  /* not enough C-stack slots */
  എങ്കിൽ (L->ci->next == ശൂന്യ && luaE_extendCI(L, 0) == ശൂന്യ)
    തിരികെ 0;  /* unable to allocate first ci */
  എങ്കിൽ (L->ci->next->next == ശൂന്യ && luaE_extendCI(L, 0) == ശൂന്യ)
    തിരികെ 0;  /* unable to allocate second ci */
  എങ്കിൽ (L->അട്ടി_last.p - L->മുകൾ.p >= BASIC_അട്ടി_SIZE)
    തിരികെ 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  അല്ലെങ്കിൽ  /* try to grow stack to a size with enough free slots */
    തിരികെ luaD_growstack(L, BASIC_അട്ടി_SIZE, 0);
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
#എങ്കിൽ !defined(LUAI_STRICT_ADDRESS)
#define LUAI_STRICT_ADDRESS	1
#endif

#എങ്കിൽ LUAI_STRICT_ADDRESS
/*
** Change all pointers to the stack into offsets.
*/
സ്ഥിതം ശൂന്യം relstack (lua_State *L) {
  CallInfo *ci;
  UpVal *up;
  L->മുകൾ.offset = savestack(L, L->മുകൾ.p);
  L->tbclist.offset = savestack(L, L->tbclist.p);
  വേണ്ടി (up = L->openupval; up != ശൂന്യ; up = up->u.open.next)
    up->v.offset = savestack(L, uplevel(up));
  വേണ്ടി (ci = L->ci; ci != ശൂന്യ; ci = ci->previous) {
    ci->മുകൾ.offset = savestack(L, ci->മുകൾ.p);
    ci->func.offset = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
സ്ഥിതം ശൂന്യം correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  UpVal *up;
  UNUSED(oldstack);
  L->മുകൾ.p = restorestack(L, L->മുകൾ.offset);
  L->tbclist.p = restorestack(L, L->tbclist.offset);
  വേണ്ടി (up = L->openupval; up != ശൂന്യ; up = up->u.open.next)
    up->v.p = s2v(restorestack(L, up->v.offset));
  വേണ്ടി (ci = L->ci; ci != ശൂന്യ; ci = ci->previous) {
    ci->മുകൾ.p = restorestack(L, ci->മുകൾ.offset);
    ci->func.p = restorestack(L, ci->func.offset);
    എങ്കിൽ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#അല്ലെങ്കിൽ
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

സ്ഥിതം ശൂന്യം relstack (lua_State *L) { UNUSED(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
സ്ഥിതം ശൂന്യം correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  UpVal *up;
  StkId newstack = L->അട്ടി.p;
  എങ്കിൽ (oldstack == newstack)
    തിരികെ;
  L->മുകൾ.p = L->മുകൾ.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  വേണ്ടി (up = L->openupval; up != ശൂന്യ; up = up->u.open.next)
    up->v.p = s2v(uplevel(up) - oldstack + newstack);
  വേണ്ടി (ci = L->ci; ci != ശൂന്യ; ci = ci->previous) {
    ci->മുകൾ.p = ci->മുകൾ.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    എങ്കിൽ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#endif


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
പൂർണ്ണസംഖ്യ luaD_reallocstack (lua_State *L, പൂർണ്ണസംഖ്യ newsize, പൂർണ്ണസംഖ്യ raiseerror) {
  പൂർണ്ണസംഖ്യ oldsize = stacksize(L);
  പൂർണ്ണസംഖ്യ i;
  StkId newstack;
  StkId oldstack = L->അട്ടി.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_അട്ടി,
                                   newsize + EXTRA_അട്ടി, അട്ടി_മൂല്യം);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  എങ്കിൽ (l_unlikely(newstack == ശൂന്യ)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    എങ്കിൽ (raiseerror)
      luaM_error(L);
    അല്ലെങ്കിൽ തിരികെ 0;  /* do not raise an error */
  }
  L->അട്ടി.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->അട്ടി_last.p = L->അട്ടി.p + newsize;
  വേണ്ടി (i = oldsize + EXTRA_അട്ടി; i < newsize + EXTRA_അട്ടി; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  തിരികെ 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
പൂർണ്ണസംഖ്യ luaD_growstack (lua_State *L, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ raiseerror) {
  പൂർണ്ണസംഖ്യ size = stacksize(L);
  എങ്കിൽ (l_unlikely(size > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    എങ്കിൽ (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    തിരികെ 0;  /* if not 'raiseerror', just signal it */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (n < MAXSTACK) {  /* avoids arithmetic overflows */
    പൂർണ്ണസംഖ്യ newsize = size + (size >> 1);  /* tentative new size (size * 1.5) */
    പൂർണ്ണസംഖ്യ needed = cast_int(L->മുകൾ.p - L->അട്ടി.p) + n;
    എങ്കിൽ (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    എങ്കിൽ (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    എങ്കിൽ (l_likely(newsize <= MAXSTACK))
      തിരികെ luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  എങ്കിൽ (raiseerror)
    luaG_runerror(L, "stack overflow");
  തിരികെ 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ stackinuse (lua_State *L) {
  CallInfo *ci;
  പൂർണ്ണസംഖ്യ res;
  StkId lim = L->മുകൾ.p;
  വേണ്ടി (ci = L->ci; ci != ശൂന്യ; ci = ci->previous) {
    എങ്കിൽ (lim < ci->മുകൾ.p) lim = ci->മുകൾ.p;
  }
  lua_assert(lim <= L->അട്ടി_last.p + EXTRA_അട്ടി);
  res = cast_int(lim - L->അട്ടി.p) + 1;  /* part of stack in use */
  എങ്കിൽ (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  തിരികെ res;
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
ശൂന്യം luaD_shrinkstack (lua_State *L) {
  പൂർണ്ണസംഖ്യ inuse = stackinuse(L);
  പൂർണ്ണസംഖ്യ max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  എങ്കിൽ (inuse <= MAXSTACK && stacksize(L) > max) {
    പൂർണ്ണസംഖ്യ nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  അല്ലെങ്കിൽ  /* don't change stack */
    condmovestack(L,(ശൂന്യം)0,(ശൂന്യം)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
ശൂന്യം luaD_hook (lua_State *L, പൂർണ്ണസംഖ്യ event, പൂർണ്ണസംഖ്യ വരി,
                              പൂർണ്ണസംഖ്യ ftransfer, പൂർണ്ണസംഖ്യ ntransfer) {
  lua_Hook hook = L->hook;
  എങ്കിൽ (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t മുകൾ = savestack(L, L->മുകൾ.p);  /* preserve original 'top' */
    ptrdiff_t ci_മുകൾ = savestack(L, ci->മുകൾ.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = വരി;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    എങ്കിൽ (isLua(ci) && L->മുകൾ.p < ci->മുകൾ.p)
      L->മുകൾ.p = ci->മുകൾ.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    എങ്കിൽ (ci->മുകൾ.p < L->മുകൾ.p + LUA_MINSTACK)
      ci->മുകൾ.p = L->മുകൾ.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_lock(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->മുകൾ.p = restorestack(L, ci_മുകൾ);
    L->മുകൾ.p = restorestack(L, മുകൾ);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
ശൂന്യം luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  എങ്കിൽ (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    പൂർണ്ണസംഖ്യ event = (ci->callstatus & CIST_TAIL) ? LUA_HOOKTAILCALL
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
സ്ഥിതം ശൂന്യം rethook (lua_State *L, CallInfo *ci, പൂർണ്ണസംഖ്യ nres) {
  എങ്കിൽ (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->മുകൾ.p - nres;  /* index of first result */
    പൂർണ്ണസംഖ്യ delta = 0;  /* correction for vararg functions */
    പൂർണ്ണസംഖ്യ ftransfer;
    എങ്കിൽ (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      എങ്കിൽ (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  എങ്കിൽ (isLua(ci = ci->previous))
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
സ്ഥിതം unsigned tryfuncTM (lua_State *L, StkId func, unsigned status) {
  സ്ഥിരം TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  എങ്കിൽ (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  വേണ്ടി (p = L->മുകൾ.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->മുകൾ.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  എങ്കിൽ ((status & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  തിരികെ status + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline ശൂന്യം genmoveresults (lua_State *L, StkId res, പൂർണ്ണസംഖ്യ nres,
                                             പൂർണ്ണസംഖ്യ wanted) {
  StkId firstresult = L->മുകൾ.p - nres;  /* index of first result */
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  വേണ്ടി (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  വേണ്ടി (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->മുകൾ.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline ശൂന്യം moveresults (lua_State *L, StkId res, പൂർണ്ണസംഖ്യ nres,
                                          l_uint32 fwanted) {
  തിരഞ്ഞെടുപ്പ് (fwanted) {  /* handle typical cases separately */
    അവസ്ഥ 0 + 1:  /* no values needed */
      L->മുകൾ.p = res;
      തിരികെ;
    അവസ്ഥ 1 + 1:  /* one value needed */
      എങ്കിൽ (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      അല്ലെങ്കിൽ  /* at least one result */
        setobjs2s(L, res, L->മുകൾ.p - nres);  /* move it to proper place */
      L->മുകൾ.p = res + 1;
      തിരികെ;
    അവസ്ഥ LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി: {  /* two/more results and/or to-be-closed variables */
      പൂർണ്ണസംഖ്യ wanted = get_nresults(fwanted);
      എങ്കിൽ (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        എങ്കിൽ (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        എങ്കിൽ (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      നിർത്തുക;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
ശൂന്യം luaD_poscall (lua_State *L, CallInfo *ci, പൂർണ്ണസംഖ്യ nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  എങ്കിൽ (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->previous;  /* back to caller (after closing variables) */
}



#define next_ci(L)  (L->ci->next ? L->ci->next : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, unsigned status,
                                                StkId മുകൾ) {
  CallInfo *ci = L->ci = next_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((status & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = status;
  ci->മുകൾ.p = മുകൾ;
  തിരികെ ci;
}


/*
** precall for C functions
*/
l_sinline പൂർണ്ണസംഖ്യ precallC (lua_State *L, StkId func, unsigned status,
                                            lua_CFunction f) {
  പൂർണ്ണസംഖ്യ n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, status | CIST_C,
                               L->മുകൾ.p + LUA_MINSTACK);
  lua_assert(ci->മുകൾ.p <= L->അട്ടി_last.p);
  എങ്കിൽ (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    പൂർണ്ണസംഖ്യ narg = cast_int(L->മുകൾ.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_lock(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  തിരികെ n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
പൂർണ്ണസംഖ്യ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    പൂർണ്ണസംഖ്യ narg1, പൂർണ്ണസംഖ്യ delta) {
  unsigned status = LUA_MULTRET + 1;
 retry:
  തിരഞ്ഞെടുപ്പ് (ttypetag(s2v(func))) {
    അവസ്ഥ LUA_VCCL:  /* C closure */
      തിരികെ precallC(L, func, status, clCvalue(s2v(func))->f);
    അവസ്ഥ LUA_VLCF:  /* light C function */
      തിരികെ precallC(L, func, status, fvalue(s2v(func)));
    അവസ്ഥ LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      പൂർണ്ണസംഖ്യ fsize = p->maxstacksize;  /* frame size */
      പൂർണ്ണസംഖ്യ nfixparams = p->numparams;
      പൂർണ്ണസംഖ്യ i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      വേണ്ടി (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      വേണ്ടി (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->മുകൾ.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->മുകൾ.p <= L->അട്ടി_last.p);
      ci->u.l.savedpc = p->code;  /* starting point */
      ci->callstatus |= CIST_TAIL;
      L->മുകൾ.p = func + narg1;  /* set top */
      തിരികെ -1;
    }
    സ്ഥിരസ്ഥിതി: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      status = tryfuncTM(L, func, status);  /* try '__call' metamethod */
      narg1++;
      പോകുക retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nresults) {
  unsigned status = cast_uint(nresults + 1);
  lua_assert(status <= MAXRESULTS + 1);
 retry:
  തിരഞ്ഞെടുപ്പ് (ttypetag(s2v(func))) {
    അവസ്ഥ LUA_VCCL:  /* C closure */
      precallC(L, func, status, clCvalue(s2v(func))->f);
      തിരികെ ശൂന്യ;
    അവസ്ഥ LUA_VLCF:  /* light C function */
      precallC(L, func, status, fvalue(s2v(func)));
      തിരികെ ശൂന്യ;
    അവസ്ഥ LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      പൂർണ്ണസംഖ്യ narg = cast_int(L->മുകൾ.p - func) - 1;  /* number of real arguments */
      പൂർണ്ണസംഖ്യ nfixparams = p->numparams;
      പൂർണ്ണസംഖ്യ fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, status, func + 1 + fsize);
      ci->u.l.savedpc = p->code;  /* starting point */
      വേണ്ടി (; narg < nfixparams; narg++)
        setnilvalue2s(L->മുകൾ.p++);  /* complete missing arguments */
      lua_assert(ci->മുകൾ.p <= L->അട്ടി_last.p);
      തിരികെ ci;
    }
    സ്ഥിരസ്ഥിതി: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      status = tryfuncTM(L, func, status);  /* try '__call' metamethod */
      പോകുക retry;  /* try again with metamethod */
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
l_sinline ശൂന്യം ccall (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  എങ്കിൽ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  എങ്കിൽ ((ci = luaD_precall(L, func, nResults)) != ശൂന്യ) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
ശൂന്യം luaD_call (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
ശൂന്യം luaD_callnoyield (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults) {
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
സ്ഥിതം TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus status = getcistrecst(ci);  /* get original status */
  എങ്കിൽ (l_likely(status == LUA_OK))  /* no error? */
    status = LUA_YIELD;  /* was interrupted by an yield */
  അല്ലെങ്കിൽ {  /* error */
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
  തിരികെ status;
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
സ്ഥിതം ശൂന്യം finishCcall (lua_State *L, CallInfo *ci) {
  പൂർണ്ണസംഖ്യ n;  /* actual number of results from C function */
  എങ്കിൽ (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  അല്ലെങ്കിൽ {
    TStatus status = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != ശൂന്യ && yieldable(L));
    എങ്കിൽ (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
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
സ്ഥിതം ശൂന്യം unroll (lua_State *L, ശൂന്യം *ud) {
  CallInfo *ci;
  UNUSED(ud);
  ആകുമ്പോൾ ((ci = L->ci) != &L->base_ci) {  /* something in the stack */
    എങ്കിൽ (!isLua(ci))  /* C function? */
      finishCcall(L, ci);  /* complete its execution */
    അല്ലെങ്കിൽ {  /* Lua function */
      luaV_finishOp(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
സ്ഥിതം CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  വേണ്ടി (ci = L->ci; ci != ശൂന്യ; ci = ci->previous) {  /* search for a pcall */
    എങ്കിൽ (ci->callstatus & CIST_YPCALL)
      തിരികെ ci;
  }
  തിരികെ ശൂന്യ;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ resume_error (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ narg) {
  api_checkpop(L, narg);
  L->മുകൾ.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->മുകൾ.p, luaS_new(L, സന്ദേശം));  /* push error message */
  api_incr_മുകൾ(L);
  lua_unlock(L);
  തിരികെ LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
സ്ഥിതം ശൂന്യം resume (lua_State *L, ശൂന്യം *ud) {
  പൂർണ്ണസംഖ്യ n = *(cast(പൂർണ്ണസംഖ്യ*, ud));  /* number of arguments */
  StkId firstArg = L->മുകൾ.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  എങ്കിൽ (L->status == LUA_OK)  /* starting a coroutine? */
    ccall(L, firstArg - 1, LUA_MULTRET, 0);  /* just call its body */
  അല്ലെങ്കിൽ {  /* resuming from previous yield */
    lua_assert(L->status == LUA_YIELD);
    L->status = LUA_OK;  /* mark that it is running (again) */
    എങ്കിൽ (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->മുകൾ.p = firstArg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    അല്ലെങ്കിൽ {  /* 'common' yield */
      എങ്കിൽ (ci->u.c.k != ശൂന്യ) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_lock(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, ശൂന്യ);  /* run continuation */
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
സ്ഥിതം TStatus precover (lua_State *L, TStatus status) {
  CallInfo *ci;
  ആകുമ്പോൾ (errorstatus(status) && (ci = findpcall(L)) != ശൂന്യ) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, status);  /* status to finish 'pcall' */
    status = luaD_rawrunprotected(L, unroll, ശൂന്യ);
  }
  തിരികെ status;
}


LUA_API പൂർണ്ണസംഖ്യ lua_resume (lua_State *L, lua_State *from, പൂർണ്ണസംഖ്യ nargs,
                                      പൂർണ്ണസംഖ്യ *nresults) {
  TStatus status;
  lua_lock(L);
  എങ്കിൽ (L->status == LUA_OK) {  /* may be starting a coroutine */
    എങ്കിൽ (L->ci != &L->base_ci)  /* not in base level? */
      തിരികെ resume_error(L, "cannot resume non-suspended coroutine", nargs);
    അല്ലെങ്കിൽ എങ്കിൽ (L->മുകൾ.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      തിരികെ resume_error(L, "cannot resume dead coroutine", nargs);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (L->status != LUA_YIELD)  /* ended with errors? */
    തിരികെ resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  എങ്കിൽ (getCcalls(L) >= LUAI_MAXCCALLS)
    തിരികെ resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->status == LUA_OK) ? nargs + 1 : nargs);
  status = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  status = precover(L, status);
  എങ്കിൽ (l_likely(!errorstatus(status)))
    lua_assert(status == L->status);  /* normal end or yield */
  അല്ലെങ്കിൽ {  /* unrecoverable error */
    L->status = status;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, status, L->മുകൾ.p);  /* push error message */
    L->ci->മുകൾ.p = L->മുകൾ.p;
  }
  *nresults = (status == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->മുകൾ.p - (L->ci->func.p + 1));
  lua_unlock(L);
  തിരികെ APIstatus(status);
}


LUA_API പൂർണ്ണസംഖ്യ lua_isyieldable (lua_State *L) {
  തിരികെ yieldable(L);
}


LUA_API പൂർണ്ണസംഖ്യ lua_yieldk (lua_State *L, പൂർണ്ണസംഖ്യ nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_lock(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  എങ്കിൽ (l_unlikely(!yieldable(L))) {
    എങ്കിൽ (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    അല്ലെങ്കിൽ
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->status = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  എങ്കിൽ (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == ശൂന്യ, "hooks cannot continue after yielding");
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ ((ci->u.c.k = k) != ശൂന്യ)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  തിരികെ 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
ഘടന CloseP {
  StkId level;
  TStatus status;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
സ്ഥിതം ശൂന്യം closepaux (lua_State *L, ശൂന്യം *ud) {
  ഘടന CloseP *pcl = cast(ഘടന CloseP *, ud);
  luaF_close(L, pcl->level, pcl->status, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus status) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  വേണ്ടി (;;) {  /* keep closing upvalues until no more errors */
    ഘടന CloseP pcl;
    pcl.level = restorestack(L, level); pcl.status = status;
    status = luaD_rawrunprotected(L, &closepaux, &pcl);
    എങ്കിൽ (l_likely(status == LUA_OK))  /* no more errors? */
      തിരികെ pcl.status;
    അല്ലെങ്കിൽ {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, ശൂന്യം *u, ptrdiff_t old_മുകൾ,
                                  ptrdiff_t ef) {
  TStatus status;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  status = luaD_rawrunprotected(L, func, u);
  എങ്കിൽ (l_unlikely(status != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    status = luaD_closeprotected(L, old_മുകൾ, status);
    luaD_seterrorobj(L, status, restorestack(L, old_മുകൾ));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  തിരികെ status;
}



/*
** Execute a protected parser.
*/
ഘടന SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  സ്ഥിരം അക്ഷരം *mode;
  സ്ഥിരം അക്ഷരം *name;
};


സ്ഥിതം ശൂന്യം checkmode (lua_State *L, സ്ഥിരം അക്ഷരം *mode, സ്ഥിരം അക്ഷരം *x) {
  എങ്കിൽ (strchr(mode, x[0]) == ശൂന്യ) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, mode);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
സ്ഥിതം ശൂന്യം f_parser (lua_State *L, ശൂന്യം *ud) {
  LClosure *cl;
  ഘടന SParser *p = cast(ഘടന SParser *, ud);
  സ്ഥിരം അക്ഷരം *mode = p->mode ? p->mode : "bt";
  പൂർണ്ണസംഖ്യ c;
  Table *anchor;
  ptrdiff_t otop = savestack(L, L->മുകൾ.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->മുകൾ.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  എങ്കിൽ (c == LUA_SIGNATURE[0]) {
    പൂർണ്ണസംഖ്യ fixed = 0;
    എങ്കിൽ (strchr(mode, 'B') != ശൂന്യ)
      fixed = 1;
    അല്ലെങ്കിൽ
      checkmode(L, mode, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  അല്ലെങ്കിൽ {
    checkmode(L, mode, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->മുകൾ.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->മുകൾ.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
ശൂന്യം luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->മുകൾ.p++), obj);  /* temporary anchor in the stack */
  luaH_set(L, anchor, s2v(L->മുകൾ.p - 1), s2v(L->മുകൾ.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->മുകൾ.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, സ്ഥിരം അക്ഷരം *name,
                                            സ്ഥിരം അക്ഷരം *mode) {
  ഘടന SParser p;
  TStatus status;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.mode = mode;
  p.dyd.actvar.arr = ശൂന്യ; p.dyd.actvar.size = 0;
  p.dyd.gt.arr = ശൂന്യ; p.dyd.gt.size = 0;
  p.dyd.label.arr = ശൂന്യ; p.dyd.label.size = 0;
  luaZ_initbuffer(L, &p.buff);
  status = luaD_pcall(L, f_parser, &p, savestack(L, L->മുകൾ.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.size));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.size));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.size));
  decnny(L);
  തിരികെ status;
}


