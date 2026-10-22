/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#परिभाषय ldo_c
#परिभाषय LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <setjmp.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lparser.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"
#अन्तर्भाव "lundump.h"
#अन्तर्भाव "lvm.h"
#अन्तर्भाव "lzio.h"



#परिभाषय errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#यदि !defined(luai_userstateresume)
#परिभाषय luai_userstateresume(L,n)	((शून्य)L)
#यदि_अन्तः

#यदि !defined(luai_userstateyield)
#परिभाषय luai_userstateyield(L,n)	((शून्य)L)
#यदि_अन्तः


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
प्रकारनिर्देश संरचना lua_longjmp {
  संरचना lua_longjmp *previous;
  jmp_buf b;
  चञ्चल TStatus स्थितिः;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#यदि !defined(LUAI_THROW)				/* { */

#यदि defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#परिभाषय LUAI_THROW(L,c)		throw(c)

स्थिर शून्य LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, शून्य *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    यदि (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->स्थितिः = -1;  /* create some error code */
  }
}


#नोचेद्यदि defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#परिभाषय LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#परिभाषय LUAI_TRY(L,c,f,ud)	यदि (_setjmp((c)->b) == 0) ((f)(L, ud))

#अन्यथा							/* }{ */

/* ISO C handling with long jumps */
#परिभाषय LUAI_THROW(L,c)		longjmp((c)->b, 1)
#परिभाषय LUAI_TRY(L,c,f,ud)	यदि (setjmp((c)->b) == 0) ((f)(L, ud))

#यदि_अन्तः							/* } */

#यदि_अन्तः							/* } */


शून्य luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  यदि (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  अन्यथा {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->शिखरम्.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->शिखरम्.p - 1);  /* move it to 'oldtop' */
  }
  L->शिखरम्.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  यदि (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->स्थितिः = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  अन्यथा {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->स्थितिः = errcode;
    यदि (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->शिखरम्.p++, L->शिखरम्.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    अन्यथा {  /* no handler at all; abort */
      यदि (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  यदि (L->errorJmp) {
    /* unroll error entries up to the first level */
    यावत् (L->errorJmp->previous != रिक्त)
      L->errorJmp = L->errorJmp->previous;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, शून्य *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.स्थितिः = LUA_OK;
  lj.previous = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.previous;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  प्रत्यागम lj.स्थितिः;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#परिभाषय STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#यदि !defined(LUAI_MAXSTACK)
#यदि 1000000 < (अधिकतम_पूर्णाङ्कः / 2)
#परिभाषय LUAI_MAXSTACK           1000000
#अन्यथा
#परिभाषय LUAI_MAXSTACK           (अधिकतम_पूर्णाङ्कः / 2u)
#यदि_अन्तः
#यदि_अन्तः


/* maximum stack size that respects size_t */
#परिभाषय MAXSTACK_BYSIZET  ((MAX_SIZET / परिमाण(राशिः_मानम्)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#परिभाषय MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#परिभाषय ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *सन्देशः = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->शिखरम्.p, सन्देशः);
  L->शिखरम्.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
पूर्णाङ्क luaD_checkminstack (lua_State *L) {
  यदि (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    प्रत्यागम 0;  /* not enough C-stack slots */
  यदि (L->ci->अग्र == रिक्त && luaE_extendCI(L, 0) == रिक्त)
    प्रत्यागम 0;  /* unable to allocate first ci */
  यदि (L->ci->अग्र->अग्र == रिक्त && luaE_extendCI(L, 0) == रिक्त)
    प्रत्यागम 0;  /* unable to allocate second ci */
  यदि (L->राशिः_चरमः.p - L->शिखरम्.p >= BASIC_राशिः_आकार)
    प्रत्यागम 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  अन्यथा  /* try to grow stack to a size with enough free slots */
    प्रत्यागम luaD_growstack(L, BASIC_राशिः_आकार, 0);
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
#यदि !defined(LUAI_STRICT_स्थानाङ्कः)
#परिभाषय LUAI_STRICT_स्थानाङ्कः	1
#यदि_अन्तः

#यदि LUAI_STRICT_स्थानाङ्कः
/*
** Change all pointers to the stack into offsets.
*/
स्थिर शून्य relstack (lua_State *L) {
  CallInfo *ci;
  ऊर्ध्वम्_Val *ऊर्ध्वम्;
  L->शिखरम्.विस्थापनम् = savestack(L, L->शिखरम्.p);
  L->tbclist.विस्थापनम् = savestack(L, L->tbclist.p);
  प्रति (ऊर्ध्वम् = L->openupval; ऊर्ध्वम् != रिक्त; ऊर्ध्वम् = ऊर्ध्वम्->u.उद्घाटय.अग्र)
    ऊर्ध्वम्->v.विस्थापनम् = savestack(L, uplevel(ऊर्ध्वम्));
  प्रति (ci = L->ci; ci != रिक्त; ci = ci->previous) {
    ci->शिखरम्.विस्थापनम् = savestack(L, ci->शिखरम्.p);
    ci->func.विस्थापनम् = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
स्थिर शून्य correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ऊर्ध्वम्_Val *ऊर्ध्वम्;
  अप्रयुक्तम्(oldstack);
  L->शिखरम्.p = restorestack(L, L->शिखरम्.विस्थापनम्);
  L->tbclist.p = restorestack(L, L->tbclist.विस्थापनम्);
  प्रति (ऊर्ध्वम् = L->openupval; ऊर्ध्वम् != रिक्त; ऊर्ध्वम् = ऊर्ध्वम्->u.उद्घाटय.अग्र)
    ऊर्ध्वम्->v.p = s2v(restorestack(L, ऊर्ध्वम्->v.विस्थापनम्));
  प्रति (ci = L->ci; ci != रिक्त; ci = ci->previous) {
    ci->शिखरम्.p = restorestack(L, ci->शिखरम्.विस्थापनम्);
    ci->func.p = restorestack(L, ci->func.विस्थापनम्);
    यदि (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#अन्यथा
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

स्थिर शून्य relstack (lua_State *L) { अप्रयुक्तम्(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
स्थिर शून्य correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ऊर्ध्वम्_Val *ऊर्ध्वम्;
  StkId newstack = L->राशिः.p;
  यदि (oldstack == newstack)
    प्रत्यागम;
  L->शिखरम्.p = L->शिखरम्.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  प्रति (ऊर्ध्वम् = L->openupval; ऊर्ध्वम् != रिक्त; ऊर्ध्वम् = ऊर्ध्वम्->u.उद्घाटय.अग्र)
    ऊर्ध्वम्->v.p = s2v(uplevel(ऊर्ध्वम्) - oldstack + newstack);
  प्रति (ci = L->ci; ci != रिक्त; ci = ci->previous) {
    ci->शिखरम्.p = ci->शिखरम्.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    यदि (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#यदि_अन्तः


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
पूर्णाङ्क luaD_reallocstack (lua_State *L, पूर्णाङ्क newsize, पूर्णाङ्क raiseerror) {
  पूर्णाङ्क oldsize = stacksize(L);
  पूर्णाङ्क i;
  StkId newstack;
  StkId oldstack = L->राशिः.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_राशिः,
                                   newsize + EXTRA_राशिः, राशिः_मानम्);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  यदि (l_unlikely(newstack == रिक्त)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    यदि (raiseerror)
      luaM_error(L);
    अन्यथा प्रत्यागम 0;  /* do not raise an error */
  }
  L->राशिः.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->राशिः_चरमः.p = L->राशिः.p + newsize;
  प्रति (i = oldsize + EXTRA_राशिः; i < newsize + EXTRA_राशिः; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  प्रत्यागम 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
पूर्णाङ्क luaD_growstack (lua_State *L, पूर्णाङ्क n, पूर्णाङ्क raiseerror) {
  पूर्णाङ्क आकार = stacksize(L);
  यदि (l_unlikely(आकार > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    यदि (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    प्रत्यागम 0;  /* if not 'raiseerror', just signal it */
  }
  अन्यथा यदि (n < MAXSTACK) {  /* avoids arithmetic overflows */
    पूर्णाङ्क newsize = आकार + (आकार >> 1);  /* tentative new size (size * 1.5) */
    पूर्णाङ्क needed = cast_int(L->शिखरम्.p - L->राशिः.p) + n;
    यदि (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    यदि (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    यदि (l_likely(newsize <= MAXSTACK))
      प्रत्यागम luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  यदि (raiseerror)
    luaG_runerror(L, "stack overflow");
  प्रत्यागम 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
स्थिर पूर्णाङ्क stackinuse (lua_State *L) {
  CallInfo *ci;
  पूर्णाङ्क res;
  StkId lim = L->शिखरम्.p;
  प्रति (ci = L->ci; ci != रिक्त; ci = ci->previous) {
    यदि (lim < ci->शिखरम्.p) lim = ci->शिखरम्.p;
  }
  lua_assert(lim <= L->राशिः_चरमः.p + EXTRA_राशिः);
  res = cast_int(lim - L->राशिः.p) + 1;  /* part of stack in use */
  यदि (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  प्रत्यागम res;
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
शून्य luaD_shrinkstack (lua_State *L) {
  पूर्णाङ्क inuse = stackinuse(L);
  पूर्णाङ्क max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  यदि (inuse <= MAXSTACK && stacksize(L) > max) {
    पूर्णाङ्क nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  अन्यथा  /* don't change stack */
    condmovestack(L,(शून्य)0,(शून्य)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
शून्य luaD_hook (lua_State *L, पूर्णाङ्क event, पूर्णाङ्क पङ्क्तिः,
                              पूर्णाङ्क ftransfer, पूर्णाङ्क ntransfer) {
  lua_Hook hook = L->hook;
  यदि (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t शिखरम् = savestack(L, L->शिखरम्.p);  /* preserve original 'top' */
    ptrdiff_t ci_शिखरम् = savestack(L, ci->शिखरम्.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = पङ्क्तिः;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    यदि (isLua(ci) && L->शिखरम्.p < ci->शिखरम्.p)
      L->शिखरम्.p = ci->शिखरम्.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    यदि (ci->शिखरम्.p < L->शिखरम्.p + LUA_MINSTACK)
      ci->शिखरम्.p = L->शिखरम्.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_तालकम्(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->शिखरम्.p = restorestack(L, ci_शिखरम्);
    L->शिखरम्.p = restorestack(L, शिखरम्);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
शून्य luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  यदि (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    पूर्णाङ्क event = (ci->callstatus & CIST_पुच्छ) ? LUA_HOOKTAILCALL
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
स्थिर शून्य rethook (lua_State *L, CallInfo *ci, पूर्णाङ्क nres) {
  यदि (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->शिखरम्.p - nres;  /* index of first result */
    पूर्णाङ्क delta = 0;  /* correction for vararg functions */
    पूर्णाङ्क ftransfer;
    यदि (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      यदि (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  यदि (isLua(ci = ci->previous))
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
स्थिर अचिह्नित tryfuncTM (lua_State *L, StkId func, अचिह्नित स्थितिः) {
  ध्रुव TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  यदि (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  प्रति (p = L->शिखरम्.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->शिखरम्.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  यदि ((स्थितिः & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  प्रत्यागम स्थितिः + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline शून्य genmoveresults (lua_State *L, StkId res, पूर्णाङ्क nres,
                                             पूर्णाङ्क wanted) {
  StkId firstresult = L->शिखरम्.p - nres;  /* index of first result */
  पूर्णाङ्क i;
  यदि (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  प्रति (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  प्रति (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->शिखरम्.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline शून्य moveresults (lua_State *L, StkId res, पूर्णाङ्क nres,
                                          l_uint32 fwanted) {
  विकल्प (fwanted) {  /* handle typical cases separately */
    पक्ष 0 + 1:  /* no values needed */
      L->शिखरम्.p = res;
      प्रत्यागम;
    पक्ष 1 + 1:  /* one value needed */
      यदि (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      अन्यथा  /* at least one result */
        setobjs2s(L, res, L->शिखरम्.p - nres);  /* move it to proper place */
      L->शिखरम्.p = res + 1;
      प्रत्यागम;
    पक्ष LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      भङ्ग;
    स्वभाव: {  /* two/more results and/or to-be-closed variables */
      पूर्णाङ्क wanted = get_nresults(fwanted);
      यदि (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        यदि (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        यदि (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      भङ्ग;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
शून्य luaD_poscall (lua_State *L, CallInfo *ci, पूर्णाङ्क nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  यदि (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->previous;  /* back to caller (after closing variables) */
}



#परिभाषय अग्र_ci(L)  (L->ci->अग्र ? L->ci->अग्र : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, अचिह्नित स्थितिः,
                                                StkId शिखरम्) {
  CallInfo *ci = L->ci = अग्र_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((स्थितिः & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = स्थितिः;
  ci->शिखरम्.p = शिखरम्;
  प्रत्यागम ci;
}


/*
** precall for C functions
*/
l_sinline पूर्णाङ्क precallC (lua_State *L, StkId func, अचिह्नित स्थितिः,
                                            lua_CFunction f) {
  पूर्णाङ्क n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, स्थितिः | CIST_C,
                               L->शिखरम्.p + LUA_MINSTACK);
  lua_assert(ci->शिखरम्.p <= L->राशिः_चरमः.p);
  यदि (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    पूर्णाङ्क narg = cast_int(L->शिखरम्.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_तालकम्(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  प्रत्यागम n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
पूर्णाङ्क luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    पूर्णाङ्क narg1, पूर्णाङ्क delta) {
  अचिह्नित स्थितिः = LUA_MULTRET + 1;
 retry:
  विकल्प (ttypetag(s2v(func))) {
    पक्ष LUA_VCCL:  /* C closure */
      प्रत्यागम precallC(L, func, स्थितिः, clCvalue(s2v(func))->f);
    पक्ष LUA_VLCF:  /* light C function */
      प्रत्यागम precallC(L, func, स्थितिः, fvalue(s2v(func)));
    पक्ष LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      पूर्णाङ्क fsize = p->maxstacksize;  /* frame size */
      पूर्णाङ्क nfixparams = p->numparams;
      पूर्णाङ्क i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      प्रति (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      प्रति (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->शिखरम्.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->शिखरम्.p <= L->राशिः_चरमः.p);
      ci->u.l.savedpc = p->संहिता;  /* starting point */
      ci->callstatus |= CIST_पुच्छ;
      L->शिखरम्.p = func + narg1;  /* set top */
      प्रत्यागम -1;
    }
    स्वभाव: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      स्थितिः = tryfuncTM(L, func, स्थितिः);  /* try '__call' metamethod */
      narg1++;
      गच्छ retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, पूर्णाङ्क nresults) {
  अचिह्नित स्थितिः = cast_uint(nresults + 1);
  lua_assert(स्थितिः <= MAXRESULTS + 1);
 retry:
  विकल्प (ttypetag(s2v(func))) {
    पक्ष LUA_VCCL:  /* C closure */
      precallC(L, func, स्थितिः, clCvalue(s2v(func))->f);
      प्रत्यागम रिक्त;
    पक्ष LUA_VLCF:  /* light C function */
      precallC(L, func, स्थितिः, fvalue(s2v(func)));
      प्रत्यागम रिक्त;
    पक्ष LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      पूर्णाङ्क narg = cast_int(L->शिखरम्.p - func) - 1;  /* number of real arguments */
      पूर्णाङ्क nfixparams = p->numparams;
      पूर्णाङ्क fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, स्थितिः, func + 1 + fsize);
      ci->u.l.savedpc = p->संहिता;  /* starting point */
      प्रति (; narg < nfixparams; narg++)
        setnilvalue2s(L->शिखरम्.p++);  /* complete missing arguments */
      lua_assert(ci->शिखरम्.p <= L->राशिः_चरमः.p);
      प्रत्यागम ci;
    }
    स्वभाव: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      स्थितिः = tryfuncTM(L, func, स्थितिः);  /* try '__call' metamethod */
      गच्छ retry;  /* try again with metamethod */
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
l_sinline शून्य ccall (lua_State *L, StkId func, पूर्णाङ्क nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  यदि (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  यदि ((ci = luaD_precall(L, func, nResults)) != रिक्त) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
शून्य luaD_call (lua_State *L, StkId func, पूर्णाङ्क nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
शून्य luaD_callnoyield (lua_State *L, StkId func, पूर्णाङ्क nResults) {
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
स्थिर TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus स्थितिः = getcistrecst(ci);  /* get original status */
  यदि (l_likely(स्थितिः == LUA_OK))  /* no error? */
    स्थितिः = LUA_YIELD;  /* was interrupted by an yield */
  अन्यथा {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, स्थितिः, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, स्थितिः, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  प्रत्यागम स्थितिः;
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
स्थिर शून्य finishCcall (lua_State *L, CallInfo *ci) {
  पूर्णाङ्क n;  /* actual number of results from C function */
  यदि (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  अन्यथा {
    TStatus स्थितिः = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != रिक्त && yieldable(L));
    यदि (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      स्थितिः = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(स्थितिः), ci->u.c.ctx);  /* call continuation */
    lua_तालकम्(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
स्थिर शून्य unroll (lua_State *L, शून्य *ud) {
  CallInfo *ci;
  अप्रयुक्तम्(ud);
  यावत् ((ci = L->ci) != &L->आधारः_ci) {  /* something in the stack */
    यदि (!isLua(ci))  /* C function? */
      finishCcall(L, ci);  /* complete its execution */
    अन्यथा {  /* Lua function */
      luaV_finishOp(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
स्थिर CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  प्रति (ci = L->ci; ci != रिक्त; ci = ci->previous) {  /* search for a pcall */
    यदि (ci->callstatus & CIST_YPCALL)
      प्रत्यागम ci;
  }
  प्रत्यागम रिक्त;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
स्थिर पूर्णाङ्क resume_error (lua_State *L, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क narg) {
  api_checkpop(L, narg);
  L->शिखरम्.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->शिखरम्.p, luaS_new(L, सन्देशः));  /* push error message */
  api_incr_शिखरम्(L);
  lua_unlock(L);
  प्रत्यागम LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
स्थिर शून्य resume (lua_State *L, शून्य *ud) {
  पूर्णाङ्क n = *(cast(पूर्णाङ्क*, ud));  /* number of arguments */
  StkId firstArg = L->शिखरम्.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  यदि (L->स्थितिः == LUA_OK)  /* starting a coroutine? */
    ccall(L, firstArg - 1, LUA_MULTRET, 0);  /* just call its body */
  अन्यथा {  /* resuming from previous yield */
    lua_assert(L->स्थितिः == LUA_YIELD);
    L->स्थितिः = LUA_OK;  /* mark that it is running (again) */
    यदि (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->शिखरम्.p = firstArg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    अन्यथा {  /* 'common' yield */
      यदि (ci->u.c.k != रिक्त) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_तालकम्(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, रिक्त);  /* run continuation */
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
स्थिर TStatus precover (lua_State *L, TStatus स्थितिः) {
  CallInfo *ci;
  यावत् (errorstatus(स्थितिः) && (ci = findpcall(L)) != रिक्त) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, स्थितिः);  /* status to finish 'pcall' */
    स्थितिः = luaD_rawrunprotected(L, unroll, रिक्त);
  }
  प्रत्यागम स्थितिः;
}


LUA_API पूर्णाङ्क lua_resume (lua_State *L, lua_State *from, पूर्णाङ्क nargs,
                                      पूर्णाङ्क *nresults) {
  TStatus स्थितिः;
  lua_तालकम्(L);
  यदि (L->स्थितिः == LUA_OK) {  /* may be starting a coroutine */
    यदि (L->ci != &L->आधारः_ci)  /* not in base level? */
      प्रत्यागम resume_error(L, "cannot resume non-suspended coroutine", nargs);
    अन्यथा यदि (L->शिखरम्.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      प्रत्यागम resume_error(L, "cannot resume dead coroutine", nargs);
  }
  अन्यथा यदि (L->स्थितिः != LUA_YIELD)  /* ended with errors? */
    प्रत्यागम resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  यदि (getCcalls(L) >= LUAI_MAXCCALLS)
    प्रत्यागम resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->स्थितिः == LUA_OK) ? nargs + 1 : nargs);
  स्थितिः = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  स्थितिः = precover(L, स्थितिः);
  यदि (l_likely(!errorstatus(स्थितिः)))
    lua_assert(स्थितिः == L->स्थितिः);  /* normal end or yield */
  अन्यथा {  /* unrecoverable error */
    L->स्थितिः = स्थितिः;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, स्थितिः, L->शिखरम्.p);  /* push error message */
    L->ci->शिखरम्.p = L->शिखरम्.p;
  }
  *nresults = (स्थितिः == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->शिखरम्.p - (L->ci->func.p + 1));
  lua_unlock(L);
  प्रत्यागम APIstatus(स्थितिः);
}


LUA_API पूर्णाङ्क lua_isyieldable (lua_State *L) {
  प्रत्यागम yieldable(L);
}


LUA_API पूर्णाङ्क lua_yieldk (lua_State *L, पूर्णाङ्क nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_तालकम्(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  यदि (l_unlikely(!yieldable(L))) {
    यदि (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    अन्यथा
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->स्थितिः = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  यदि (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == रिक्त, "hooks cannot continue after yielding");
  }
  अन्यथा {
    यदि ((ci->u.c.k = k) != रिक्त)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  प्रत्यागम 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
संरचना CloseP {
  StkId level;
  TStatus स्थितिः;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
स्थिर शून्य closepaux (lua_State *L, शून्य *ud) {
  संरचना CloseP *pcl = cast(संरचना CloseP *, ud);
  luaF_close(L, pcl->level, pcl->स्थितिः, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus स्थितिः) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  प्रति (;;) {  /* keep closing upvalues until no more errors */
    संरचना CloseP pcl;
    pcl.level = restorestack(L, level); pcl.स्थितिः = स्थितिः;
    स्थितिः = luaD_rawrunprotected(L, &closepaux, &pcl);
    यदि (l_likely(स्थितिः == LUA_OK))  /* no more errors? */
      प्रत्यागम pcl.स्थितिः;
    अन्यथा {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, शून्य *u, ptrdiff_t old_शिखरम्,
                                  ptrdiff_t ef) {
  TStatus स्थितिः;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  स्थितिः = luaD_rawrunprotected(L, func, u);
  यदि (l_unlikely(स्थितिः != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    स्थितिः = luaD_closeprotected(L, old_शिखरम्, स्थितिः);
    luaD_seterrorobj(L, स्थितिः, restorestack(L, old_शिखरम्));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  प्रत्यागम स्थितिः;
}



/*
** Execute a protected parser.
*/
संरचना SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ध्रुव वर्ण *बहुलकम्;
  ध्रुव वर्ण *name;
};


स्थिर शून्य checkmode (lua_State *L, ध्रुव वर्ण *बहुलकम्, ध्रुव वर्ण *x) {
  यदि (strchr(बहुलकम्, x[0]) == रिक्त) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, बहुलकम्);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
स्थिर शून्य f_parser (lua_State *L, शून्य *ud) {
  LClosure *cl;
  संरचना SParser *p = cast(संरचना SParser *, ud);
  ध्रुव वर्ण *बहुलकम् = p->बहुलकम् ? p->बहुलकम् : "bt";
  पूर्णाङ्क c;
  सारणी *anchor;
  ptrdiff_t otop = savestack(L, L->शिखरम्.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->शिखरम्.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  यदि (c == LUA_SIGNATURE[0]) {
    पूर्णाङ्क fixed = 0;
    यदि (strchr(बहुलकम्, 'B') != रिक्त)
      fixed = 1;
    अन्यथा
      checkmode(L, बहुलकम्, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  अन्यथा {
    checkmode(L, बहुलकम्, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->शिखरम्.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->शिखरम्.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
शून्य luaD_anchorobj (lua_State *L, सारणी *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->शिखरम्.p++), obj);  /* temporary anchor in the stack */
  lua_H_समुच्चयः(L, anchor, s2v(L->शिखरम्.p - 1), s2v(L->शिखरम्.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->शिखरम्.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ध्रुव वर्ण *name,
                                            ध्रुव वर्ण *बहुलकम्) {
  संरचना SParser p;
  TStatus स्थितिः;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.बहुलकम् = बहुलकम्;
  p.dyd.actvar.arr = रिक्त; p.dyd.actvar.आकार = 0;
  p.dyd.gt.arr = रिक्त; p.dyd.gt.आकार = 0;
  p.dyd.label.arr = रिक्त; p.dyd.label.आकार = 0;
  luaZ_initbuffer(L, &p.buff);
  स्थितिः = luaD_pcall(L, f_parser, &p, savestack(L, L->शिखरम्.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.आकार));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.आकार));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.आकार));
  decnny(L);
  प्रत्यागम स्थितिः;
}


