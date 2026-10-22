/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldo_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lundump.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#ਜੇ !defined(luai_userstateresume)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateresume(L,n)	((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(luai_userstateyield)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateyield(L,n)	((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ lua_longjmp {
  ਬਣਤਰ lua_longjmp *ਪਿਛਲਾ;
  jmp_buf b;
  ਅਸਥਿਰ TStatus ਸਥਿਤੀ;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#ਜੇ !defined(LUAI_THROW)				/* { */

#ਜੇ defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_THROW(L,c)		throw(c)

ਸਥਿਰ ਖਾਲੀ LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, ਖਾਲੀ *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    ਜੇ (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->ਸਥਿਤੀ = -1;  /* create some error code */
  }
}


#ਜਾਂਜੇ defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_TRY(L,c,f,ud)	ਜੇ (_setjmp((c)->b) == 0) ((f)(L, ud))

#ਨਹੀਂਤਾਂ							/* }{ */

/* ISO C handling with long jumps */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_THROW(L,c)		longjmp((c)->b, 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_TRY(L,c,f,ud)	ਜੇ (setjmp((c)->b) == 0) ((f)(L, ud))

#ਜੇ_ਅੰਤ							/* } */

#ਜੇ_ਅੰਤ							/* } */


ਖਾਲੀ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  ਜੇ (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  ਨਹੀਂਤਾਂ {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->ਸਿਖਰ.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->ਸਿਖਰ.p - 1);  /* move it to 'oldtop' */
  }
  L->ਸਿਖਰ.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  ਜੇ (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->ਸਥਿਤੀ = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  ਨਹੀਂਤਾਂ {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->ਸਥਿਤੀ = errcode;
    ਜੇ (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->ਸਿਖਰ.p++, L->ਸਿਖਰ.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    ਨਹੀਂਤਾਂ {  /* no handler at all; abort */
      ਜੇ (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  ਜੇ (L->errorJmp) {
    /* unroll error entries up to the first level */
    ਜਦੋਂ (L->errorJmp->ਪਿਛਲਾ != ਨਿਰਾ)
      L->errorJmp = L->errorJmp->ਪਿਛਲਾ;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ਖਾਲੀ *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.ਸਥਿਤੀ = LUA_OK;
  lj.ਪਿਛਲਾ = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.ਪਿਛਲਾ;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  ਵਾਪਸ lj.ਸਥਿਤੀ;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#ਜੇ !defined(LUAI_MAXSTACK)
#ਜੇ 1000000 < (ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXSTACK           1000000
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXSTACK           (ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 2u)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ


/* maximum stack size that respects size_t */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXSTACK_BYSIZET  ((MAX_SIZET / ਆਕਾਰ(ਢੇਰ_ਮੁੱਲ)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *ਸੁਨੇਹਾ = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->ਸਿਖਰ.p, ਸੁਨੇਹਾ);
  L->ਸਿਖਰ.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
ਪੂਰਨ luaD_checkminstack (lua_State *L) {
  ਜੇ (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    ਵਾਪਸ 0;  /* not enough C-stack slots */
  ਜੇ (L->ci->ਅਗਲਾ == ਨਿਰਾ && luaE_extendCI(L, 0) == ਨਿਰਾ)
    ਵਾਪਸ 0;  /* unable to allocate first ci */
  ਜੇ (L->ci->ਅਗਲਾ->ਅਗਲਾ == ਨਿਰਾ && luaE_extendCI(L, 0) == ਨਿਰਾ)
    ਵਾਪਸ 0;  /* unable to allocate second ci */
  ਜੇ (L->ਢੇਰ_ਆਖਰੀ.p - L->ਸਿਖਰ.p >= BASIC_ਢੇਰ_ਮਾਪ)
    ਵਾਪਸ 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  ਨਹੀਂਤਾਂ  /* try to grow stack to a size with enough free slots */
    ਵਾਪਸ luaD_growstack(L, BASIC_ਢੇਰ_ਮਾਪ, 0);
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
#ਜੇ !defined(LUAI_STRICT_ਪਤਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_STRICT_ਪਤਾ	1
#ਜੇ_ਅੰਤ

#ਜੇ LUAI_STRICT_ਪਤਾ
/*
** Change all pointers to the stack into offsets.
*/
ਸਥਿਰ ਖਾਲੀ relstack (lua_State *L) {
  CallInfo *ci;
  ਉੱਪਰ_Val *ਉੱਪਰ;
  L->ਸਿਖਰ.ਵਿੱਥ = savestack(L, L->ਸਿਖਰ.p);
  L->tbclist.ਵਿੱਥ = savestack(L, L->tbclist.p);
  ਲਈ (ਉੱਪਰ = L->openupval; ਉੱਪਰ != ਨਿਰਾ; ਉੱਪਰ = ਉੱਪਰ->u.open.ਅਗਲਾ)
    ਉੱਪਰ->v.ਵਿੱਥ = savestack(L, uplevel(ਉੱਪਰ));
  ਲਈ (ci = L->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {
    ci->ਸਿਖਰ.ਵਿੱਥ = savestack(L, ci->ਸਿਖਰ.p);
    ci->func.ਵਿੱਥ = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
ਸਥਿਰ ਖਾਲੀ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ਉੱਪਰ_Val *ਉੱਪਰ;
  ਅਣਵਰਤਿਆ(oldstack);
  L->ਸਿਖਰ.p = restorestack(L, L->ਸਿਖਰ.ਵਿੱਥ);
  L->tbclist.p = restorestack(L, L->tbclist.ਵਿੱਥ);
  ਲਈ (ਉੱਪਰ = L->openupval; ਉੱਪਰ != ਨਿਰਾ; ਉੱਪਰ = ਉੱਪਰ->u.open.ਅਗਲਾ)
    ਉੱਪਰ->v.p = s2v(restorestack(L, ਉੱਪਰ->v.ਵਿੱਥ));
  ਲਈ (ci = L->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {
    ci->ਸਿਖਰ.p = restorestack(L, ci->ਸਿਖਰ.ਵਿੱਥ);
    ci->func.p = restorestack(L, ci->func.ਵਿੱਥ);
    ਜੇ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#ਨਹੀਂਤਾਂ
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

ਸਥਿਰ ਖਾਲੀ relstack (lua_State *L) { ਅਣਵਰਤਿਆ(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
ਸਥਿਰ ਖਾਲੀ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ਉੱਪਰ_Val *ਉੱਪਰ;
  StkId newstack = L->ਢੇਰ.p;
  ਜੇ (oldstack == newstack)
    ਵਾਪਸ;
  L->ਸਿਖਰ.p = L->ਸਿਖਰ.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  ਲਈ (ਉੱਪਰ = L->openupval; ਉੱਪਰ != ਨਿਰਾ; ਉੱਪਰ = ਉੱਪਰ->u.open.ਅਗਲਾ)
    ਉੱਪਰ->v.p = s2v(uplevel(ਉੱਪਰ) - oldstack + newstack);
  ਲਈ (ci = L->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {
    ci->ਸਿਖਰ.p = ci->ਸਿਖਰ.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    ਜੇ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#ਜੇ_ਅੰਤ


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
ਪੂਰਨ luaD_reallocstack (lua_State *L, ਪੂਰਨ newsize, ਪੂਰਨ raiseerror) {
  ਪੂਰਨ oldsize = stacksize(L);
  ਪੂਰਨ i;
  StkId newstack;
  StkId oldstack = L->ਢੇਰ.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_ਢੇਰ,
                                   newsize + EXTRA_ਢੇਰ, ਢੇਰ_ਮੁੱਲ);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  ਜੇ (l_unlikely(newstack == ਨਿਰਾ)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    ਜੇ (raiseerror)
      luaM_error(L);
    ਨਹੀਂਤਾਂ ਵਾਪਸ 0;  /* do not raise an error */
  }
  L->ਢੇਰ.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->ਢੇਰ_ਆਖਰੀ.p = L->ਢੇਰ.p + newsize;
  ਲਈ (i = oldsize + EXTRA_ਢੇਰ; i < newsize + EXTRA_ਢੇਰ; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  ਵਾਪਸ 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
ਪੂਰਨ luaD_growstack (lua_State *L, ਪੂਰਨ n, ਪੂਰਨ raiseerror) {
  ਪੂਰਨ ਮਾਪ = stacksize(L);
  ਜੇ (l_unlikely(ਮਾਪ > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    ਜੇ (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    ਵਾਪਸ 0;  /* if not 'raiseerror', just signal it */
  }
  ਨਹੀਂਤਾਂ ਜੇ (n < MAXSTACK) {  /* avoids arithmetic overflows */
    ਪੂਰਨ newsize = ਮਾਪ + (ਮਾਪ >> 1);  /* tentative new size (size * 1.5) */
    ਪੂਰਨ needed = cast_int(L->ਸਿਖਰ.p - L->ਢੇਰ.p) + n;
    ਜੇ (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    ਜੇ (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    ਜੇ (l_likely(newsize <= MAXSTACK))
      ਵਾਪਸ luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  ਜੇ (raiseerror)
    luaG_runerror(L, "stack overflow");
  ਵਾਪਸ 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
ਸਥਿਰ ਪੂਰਨ stackinuse (lua_State *L) {
  CallInfo *ci;
  ਪੂਰਨ res;
  StkId lim = L->ਸਿਖਰ.p;
  ਲਈ (ci = L->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {
    ਜੇ (lim < ci->ਸਿਖਰ.p) lim = ci->ਸਿਖਰ.p;
  }
  lua_assert(lim <= L->ਢੇਰ_ਆਖਰੀ.p + EXTRA_ਢੇਰ);
  res = cast_int(lim - L->ਢੇਰ.p) + 1;  /* part of stack in use */
  ਜੇ (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  ਵਾਪਸ res;
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
ਖਾਲੀ luaD_shrinkstack (lua_State *L) {
  ਪੂਰਨ inuse = stackinuse(L);
  ਪੂਰਨ max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  ਜੇ (inuse <= MAXSTACK && stacksize(L) > max) {
    ਪੂਰਨ nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  ਨਹੀਂਤਾਂ  /* don't change stack */
    condmovestack(L,(ਖਾਲੀ)0,(ਖਾਲੀ)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
ਖਾਲੀ luaD_hook (lua_State *L, ਪੂਰਨ event, ਪੂਰਨ ਸਤਰ,
                              ਪੂਰਨ ftransfer, ਪੂਰਨ ntransfer) {
  lua_Hook hook = L->hook;
  ਜੇ (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t ਸਿਖਰ = savestack(L, L->ਸਿਖਰ.p);  /* preserve original 'top' */
    ptrdiff_t ci_ਸਿਖਰ = savestack(L, ci->ਸਿਖਰ.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = ਸਤਰ;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    ਜੇ (isLua(ci) && L->ਸਿਖਰ.p < ci->ਸਿਖਰ.p)
      L->ਸਿਖਰ.p = ci->ਸਿਖਰ.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    ਜੇ (ci->ਸਿਖਰ.p < L->ਸਿਖਰ.p + LUA_MINSTACK)
      ci->ਸਿਖਰ.p = L->ਸਿਖਰ.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_ਤਾਲਾ(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->ਸਿਖਰ.p = restorestack(L, ci_ਸਿਖਰ);
    L->ਸਿਖਰ.p = restorestack(L, ਸਿਖਰ);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
ਖਾਲੀ luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  ਜੇ (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    ਪੂਰਨ event = (ci->callstatus & CIST_ਪੂਛ) ? LUA_HOOKTAILCALL
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
ਸਥਿਰ ਖਾਲੀ rethook (lua_State *L, CallInfo *ci, ਪੂਰਨ nres) {
  ਜੇ (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->ਸਿਖਰ.p - nres;  /* index of first result */
    ਪੂਰਨ delta = 0;  /* correction for vararg functions */
    ਪੂਰਨ ftransfer;
    ਜੇ (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      ਜੇ (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  ਜੇ (isLua(ci = ci->ਪਿਛਲਾ))
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
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ tryfuncTM (lua_State *L, StkId func, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਥਿਤੀ) {
  ਅਚਲ TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  ਜੇ (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  ਲਈ (p = L->ਸਿਖਰ.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->ਸਿਖਰ.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  ਜੇ ((ਸਥਿਤੀ & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  ਵਾਪਸ ਸਥਿਤੀ + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline ਖਾਲੀ genmoveresults (lua_State *L, StkId res, ਪੂਰਨ nres,
                                             ਪੂਰਨ wanted) {
  StkId firstresult = L->ਸਿਖਰ.p - nres;  /* index of first result */
  ਪੂਰਨ i;
  ਜੇ (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  ਲਈ (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  ਲਈ (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->ਸਿਖਰ.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline ਖਾਲੀ moveresults (lua_State *L, StkId res, ਪੂਰਨ nres,
                                          l_uint32 fwanted) {
  ਚੋਣ (fwanted) {  /* handle typical cases separately */
    ਹਾਲਤ 0 + 1:  /* no values needed */
      L->ਸਿਖਰ.p = res;
      ਵਾਪਸ;
    ਹਾਲਤ 1 + 1:  /* one value needed */
      ਜੇ (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      ਨਹੀਂਤਾਂ  /* at least one result */
        setobjs2s(L, res, L->ਸਿਖਰ.p - nres);  /* move it to proper place */
      L->ਸਿਖਰ.p = res + 1;
      ਵਾਪਸ;
    ਹਾਲਤ LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      ਰੋਕੋ;
    ਮੂਲ: {  /* two/more results and/or to-be-closed variables */
      ਪੂਰਨ wanted = get_nresults(fwanted);
      ਜੇ (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        ਜੇ (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        ਜੇ (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      ਰੋਕੋ;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
ਖਾਲੀ luaD_poscall (lua_State *L, CallInfo *ci, ਪੂਰਨ nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  ਜੇ (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->ਪਿਛਲਾ;  /* back to caller (after closing variables) */
}



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_ci(L)  (L->ci->ਅਗਲਾ ? L->ci->ਅਗਲਾ : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਥਿਤੀ,
                                                StkId ਸਿਖਰ) {
  CallInfo *ci = L->ci = ਅਗਲਾ_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((ਸਥਿਤੀ & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = ਸਥਿਤੀ;
  ci->ਸਿਖਰ.p = ਸਿਖਰ;
  ਵਾਪਸ ci;
}


/*
** precall for C functions
*/
l_sinline ਪੂਰਨ precallC (lua_State *L, StkId func, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਥਿਤੀ,
                                            lua_CFunction f) {
  ਪੂਰਨ n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, ਸਥਿਤੀ | CIST_C,
                               L->ਸਿਖਰ.p + LUA_MINSTACK);
  lua_assert(ci->ਸਿਖਰ.p <= L->ਢੇਰ_ਆਖਰੀ.p);
  ਜੇ (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    ਪੂਰਨ narg = cast_int(L->ਸਿਖਰ.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_ਤਾਲਾ(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  ਵਾਪਸ n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
ਪੂਰਨ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    ਪੂਰਨ narg1, ਪੂਰਨ delta) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਥਿਤੀ = LUA_MULTRET + 1;
 retry:
  ਚੋਣ (ttypetag(s2v(func))) {
    ਹਾਲਤ LUA_VCCL:  /* C closure */
      ਵਾਪਸ precallC(L, func, ਸਥਿਤੀ, clCvalue(s2v(func))->f);
    ਹਾਲਤ LUA_VLCF:  /* light C function */
      ਵਾਪਸ precallC(L, func, ਸਥਿਤੀ, fvalue(s2v(func)));
    ਹਾਲਤ LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      ਪੂਰਨ fsize = p->maxstacksize;  /* frame size */
      ਪੂਰਨ nfixparams = p->numparams;
      ਪੂਰਨ i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      ਲਈ (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      ਲਈ (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->ਸਿਖਰ.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->ਸਿਖਰ.p <= L->ਢੇਰ_ਆਖਰੀ.p);
      ci->u.l.savedpc = p->ਕੋਡ;  /* starting point */
      ci->callstatus |= CIST_ਪੂਛ;
      L->ਸਿਖਰ.p = func + narg1;  /* set top */
      ਵਾਪਸ -1;
    }
    ਮੂਲ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      ਸਥਿਤੀ = tryfuncTM(L, func, ਸਥਿਤੀ);  /* try '__call' metamethod */
      narg1++;
      ਜਾਓ retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, ਪੂਰਨ nresults) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਥਿਤੀ = cast_uint(nresults + 1);
  lua_assert(ਸਥਿਤੀ <= MAXRESULTS + 1);
 retry:
  ਚੋਣ (ttypetag(s2v(func))) {
    ਹਾਲਤ LUA_VCCL:  /* C closure */
      precallC(L, func, ਸਥਿਤੀ, clCvalue(s2v(func))->f);
      ਵਾਪਸ ਨਿਰਾ;
    ਹਾਲਤ LUA_VLCF:  /* light C function */
      precallC(L, func, ਸਥਿਤੀ, fvalue(s2v(func)));
      ਵਾਪਸ ਨਿਰਾ;
    ਹਾਲਤ LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      ਪੂਰਨ narg = cast_int(L->ਸਿਖਰ.p - func) - 1;  /* number of real arguments */
      ਪੂਰਨ nfixparams = p->numparams;
      ਪੂਰਨ fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, ਸਥਿਤੀ, func + 1 + fsize);
      ci->u.l.savedpc = p->ਕੋਡ;  /* starting point */
      ਲਈ (; narg < nfixparams; narg++)
        setnilvalue2s(L->ਸਿਖਰ.p++);  /* complete missing arguments */
      lua_assert(ci->ਸਿਖਰ.p <= L->ਢੇਰ_ਆਖਰੀ.p);
      ਵਾਪਸ ci;
    }
    ਮੂਲ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      ਸਥਿਤੀ = tryfuncTM(L, func, ਸਥਿਤੀ);  /* try '__call' metamethod */
      ਜਾਓ retry;  /* try again with metamethod */
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
l_sinline ਖਾਲੀ ccall (lua_State *L, StkId func, ਪੂਰਨ nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  ਜੇ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  ਜੇ ((ci = luaD_precall(L, func, nResults)) != ਨਿਰਾ) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
ਖਾਲੀ luaD_call (lua_State *L, StkId func, ਪੂਰਨ nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
ਖਾਲੀ luaD_callnoyield (lua_State *L, StkId func, ਪੂਰਨ nResults) {
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
ਸਥਿਰ TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus ਸਥਿਤੀ = getcistrecst(ci);  /* get original status */
  ਜੇ (l_likely(ਸਥਿਤੀ == LUA_OK))  /* no error? */
    ਸਥਿਤੀ = LUA_YIELD;  /* was interrupted by an yield */
  ਨਹੀਂਤਾਂ {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, ਸਥਿਤੀ, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, ਸਥਿਤੀ, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  ਵਾਪਸ ਸਥਿਤੀ;
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
ਸਥਿਰ ਖਾਲੀ ਅੰਤ_Ccall (lua_State *L, CallInfo *ci) {
  ਪੂਰਨ n;  /* actual number of results from C function */
  ਜੇ (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  ਨਹੀਂਤਾਂ {
    TStatus ਸਥਿਤੀ = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != ਨਿਰਾ && yieldable(L));
    ਜੇ (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      ਸਥਿਤੀ = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(ਸਥਿਤੀ), ci->u.c.ctx);  /* call continuation */
    lua_ਤਾਲਾ(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
ਸਥਿਰ ਖਾਲੀ unroll (lua_State *L, ਖਾਲੀ *ud) {
  CallInfo *ci;
  ਅਣਵਰਤਿਆ(ud);
  ਜਦੋਂ ((ci = L->ci) != &L->ਅਧਾਰ_ci) {  /* something in the stack */
    ਜੇ (!isLua(ci))  /* C function? */
      ਅੰਤ_Ccall(L, ci);  /* complete its execution */
    ਨਹੀਂਤਾਂ {  /* Lua function */
      lua_V_ਅੰਤ_Op(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
ਸਥਿਰ CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  ਲਈ (ci = L->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {  /* search for a pcall */
    ਜੇ (ci->callstatus & CIST_YPCALL)
      ਵਾਪਸ ci;
  }
  ਵਾਪਸ ਨਿਰਾ;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
ਸਥਿਰ ਪੂਰਨ resume_error (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ narg) {
  api_checkpop(L, narg);
  L->ਸਿਖਰ.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->ਸਿਖਰ.p, luaS_new(L, ਸੁਨੇਹਾ));  /* push error message */
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
  ਵਾਪਸ LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
ਸਥਿਰ ਖਾਲੀ resume (lua_State *L, ਖਾਲੀ *ud) {
  ਪੂਰਨ n = *(cast(ਪੂਰਨ*, ud));  /* number of arguments */
  StkId ਪਹਿਲਾ_Arg = L->ਸਿਖਰ.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  ਜੇ (L->ਸਥਿਤੀ == LUA_OK)  /* starting a coroutine? */
    ccall(L, ਪਹਿਲਾ_Arg - 1, LUA_MULTRET, 0);  /* just call its body */
  ਨਹੀਂਤਾਂ {  /* resuming from previous yield */
    lua_assert(L->ਸਥਿਤੀ == LUA_YIELD);
    L->ਸਥਿਤੀ = LUA_OK;  /* mark that it is running (again) */
    ਜੇ (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->ਸਿਖਰ.p = ਪਹਿਲਾ_Arg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    ਨਹੀਂਤਾਂ {  /* 'common' yield */
      ਜੇ (ci->u.c.k != ਨਿਰਾ) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_ਤਾਲਾ(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, ਨਿਰਾ);  /* run continuation */
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
ਸਥਿਰ TStatus precover (lua_State *L, TStatus ਸਥਿਤੀ) {
  CallInfo *ci;
  ਜਦੋਂ (errorstatus(ਸਥਿਤੀ) && (ci = findpcall(L)) != ਨਿਰਾ) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, ਸਥਿਤੀ);  /* status to finish 'pcall' */
    ਸਥਿਤੀ = luaD_rawrunprotected(L, unroll, ਨਿਰਾ);
  }
  ਵਾਪਸ ਸਥਿਤੀ;
}


LUA_API ਪੂਰਨ lua_resume (lua_State *L, lua_State *from, ਪੂਰਨ nargs,
                                      ਪੂਰਨ *nresults) {
  TStatus ਸਥਿਤੀ;
  lua_ਤਾਲਾ(L);
  ਜੇ (L->ਸਥਿਤੀ == LUA_OK) {  /* may be starting a coroutine */
    ਜੇ (L->ci != &L->ਅਧਾਰ_ci)  /* not in base level? */
      ਵਾਪਸ resume_error(L, "cannot resume non-suspended coroutine", nargs);
    ਨਹੀਂਤਾਂ ਜੇ (L->ਸਿਖਰ.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      ਵਾਪਸ resume_error(L, "cannot resume dead coroutine", nargs);
  }
  ਨਹੀਂਤਾਂ ਜੇ (L->ਸਥਿਤੀ != LUA_YIELD)  /* ended with errors? */
    ਵਾਪਸ resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  ਜੇ (getCcalls(L) >= LUAI_MAXCCALLS)
    ਵਾਪਸ resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->ਸਥਿਤੀ == LUA_OK) ? nargs + 1 : nargs);
  ਸਥਿਤੀ = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  ਸਥਿਤੀ = precover(L, ਸਥਿਤੀ);
  ਜੇ (l_likely(!errorstatus(ਸਥਿਤੀ)))
    lua_assert(ਸਥਿਤੀ == L->ਸਥਿਤੀ);  /* normal end or yield */
  ਨਹੀਂਤਾਂ {  /* unrecoverable error */
    L->ਸਥਿਤੀ = ਸਥਿਤੀ;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, ਸਥਿਤੀ, L->ਸਿਖਰ.p);  /* push error message */
    L->ci->ਸਿਖਰ.p = L->ਸਿਖਰ.p;
  }
  *nresults = (ਸਥਿਤੀ == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->ਸਿਖਰ.p - (L->ci->func.p + 1));
  lua_unlock(L);
  ਵਾਪਸ APIstatus(ਸਥਿਤੀ);
}


LUA_API ਪੂਰਨ lua_isyieldable (lua_State *L) {
  ਵਾਪਸ yieldable(L);
}


LUA_API ਪੂਰਨ lua_yieldk (lua_State *L, ਪੂਰਨ nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_ਤਾਲਾ(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  ਜੇ (l_unlikely(!yieldable(L))) {
    ਜੇ (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    ਨਹੀਂਤਾਂ
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->ਸਥਿਤੀ = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  ਜੇ (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == ਨਿਰਾ, "hooks cannot continue after yielding");
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ ((ci->u.c.k = k) != ਨਿਰਾ)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  ਵਾਪਸ 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
ਬਣਤਰ CloseP {
  StkId level;
  TStatus ਸਥਿਤੀ;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
ਸਥਿਰ ਖਾਲੀ closepaux (lua_State *L, ਖਾਲੀ *ud) {
  ਬਣਤਰ CloseP *pcl = cast(ਬਣਤਰ CloseP *, ud);
  luaF_close(L, pcl->level, pcl->ਸਥਿਤੀ, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus ਸਥਿਤੀ) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ਲਈ (;;) {  /* keep closing upvalues until no more errors */
    ਬਣਤਰ CloseP pcl;
    pcl.level = restorestack(L, level); pcl.ਸਥਿਤੀ = ਸਥਿਤੀ;
    ਸਥਿਤੀ = luaD_rawrunprotected(L, &closepaux, &pcl);
    ਜੇ (l_likely(ਸਥਿਤੀ == LUA_OK))  /* no more errors? */
      ਵਾਪਸ pcl.ਸਥਿਤੀ;
    ਨਹੀਂਤਾਂ {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, ਖਾਲੀ *u, ptrdiff_t old_ਸਿਖਰ,
                                  ptrdiff_t ef) {
  TStatus ਸਥਿਤੀ;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  ਸਥਿਤੀ = luaD_rawrunprotected(L, func, u);
  ਜੇ (l_unlikely(ਸਥਿਤੀ != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    ਸਥਿਤੀ = luaD_closeprotected(L, old_ਸਿਖਰ, ਸਥਿਤੀ);
    luaD_seterrorobj(L, ਸਥਿਤੀ, restorestack(L, old_ਸਿਖਰ));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  ਵਾਪਸ ਸਥਿਤੀ;
}



/*
** Execute a protected parser.
*/
ਬਣਤਰ SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ;
  ਅਚਲ ਅੱਖਰ *name;
};


ਸਥਿਰ ਖਾਲੀ checkmode (lua_State *L, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *x) {
  ਜੇ (strchr(ਬਹੁਲਕ, x[0]) == ਨਿਰਾ) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, ਬਹੁਲਕ);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
ਸਥਿਰ ਖਾਲੀ f_parser (lua_State *L, ਖਾਲੀ *ud) {
  LClosure *cl;
  ਬਣਤਰ SParser *p = cast(ਬਣਤਰ SParser *, ud);
  ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ = p->ਬਹੁਲਕ ? p->ਬਹੁਲਕ : "bt";
  ਪੂਰਨ c;
  ਸਾਰਣੀ *anchor;
  ptrdiff_t otop = savestack(L, L->ਸਿਖਰ.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->ਸਿਖਰ.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  ਜੇ (c == LUA_SIGNATURE[0]) {
    ਪੂਰਨ fixed = 0;
    ਜੇ (strchr(ਬਹੁਲਕ, 'B') != ਨਿਰਾ)
      fixed = 1;
    ਨਹੀਂਤਾਂ
      checkmode(L, ਬਹੁਲਕ, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  ਨਹੀਂਤਾਂ {
    checkmode(L, ਬਹੁਲਕ, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->ਸਿਖਰ.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->ਸਿਖਰ.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
ਖਾਲੀ luaD_anchorobj (lua_State *L, ਸਾਰਣੀ *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->ਸਿਖਰ.p++), obj);  /* temporary anchor in the stack */
  lua_H_ਸੈੱਟ(L, anchor, s2v(L->ਸਿਖਰ.p - 1), s2v(L->ਸਿਖਰ.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->ਸਿਖਰ.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ਅਚਲ ਅੱਖਰ *name,
                                            ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ਬਣਤਰ SParser p;
  TStatus ਸਥਿਤੀ;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.ਬਹੁਲਕ = ਬਹੁਲਕ;
  p.dyd.actvar.arr = ਨਿਰਾ; p.dyd.actvar.ਮਾਪ = 0;
  p.dyd.gt.arr = ਨਿਰਾ; p.dyd.gt.ਮਾਪ = 0;
  p.dyd.label.arr = ਨਿਰਾ; p.dyd.label.ਮਾਪ = 0;
  luaZ_initbuffer(L, &p.buff);
  ਸਥਿਤੀ = luaD_pcall(L, f_parser, &p, savestack(L, L->ਸਿਖਰ.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.ਮਾਪ));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.ਮਾਪ));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.ਮਾਪ));
  decnny(L);
  ਵਾਪਸ ਸਥਿਤੀ;
}


