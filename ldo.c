/*
** $Id: ldo.c $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ldo_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <setjmp.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"
#ಅನ್ತರ್ಭಾವ "lundump.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"



#ವ್ಯಾಖ್ಯಾನಿಸು errorstatus(s)	((s) > LUA_YIELD)


/*
** these macros allow user-specific actions when a thread is
** resumed/yielded.
*/
#ಆದರೆ !defined(luai_userstateresume)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateresume(L,n)	((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(luai_userstateyield)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateyield(L,n)	((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/* chained list of long jump buffers */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ lua_longjmp {
  ರಚನೆ lua_longjmp *ಹಿಂದಿನ;
  jmp_buf b;
  ಅಸ್ಥಿರ TStatus ಸ್ಥಿತಿ_2;  /* error code */
} lua_longjmp;


/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when available (POSIX), and with
** longjmp/setjmp otherwise.
*/
#ಆದರೆ !defined(LUAI_THROW)				/* { */

#ಆದರೆ defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_THROW(L,c)		throw(c)

ಸ್ಥಿರ ಶೂನ್ಯ LUAI_TRY (lua_State *L, lua_longjmp *c, Pfunc f, ಶೂನ್ಯ *ud) {
  try {
    f(L, ud);  /* call function protected */
  }
  catch (lua_longjmp *c1) { /* Lua error */
    ಆದರೆ (c1 != c)  /* not the correct level? */
      throw;  /* rethrow to upper level */
  }
  catch (...) {  /* non-Lua exception */
    c->ಸ್ಥಿತಿ_2 = -1;  /* create some error code */
  }
}


#ಅಥವಾಆದರೆ defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, use _longjmp/_setjmp (more efficient) */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_TRY(L,c,f,ud)	ಆದರೆ (_setjmp((c)->b) == 0) ((f)(L, ud))

#ಇಲ್ಲದಿದ್ದರೆ							/* }{ */

/* ISO C handling with long jumps */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_THROW(L,c)		longjmp((c)->b, 1)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_TRY(L,c,f,ud)	ಆದರೆ (setjmp((c)->b) == 0) ((f)(L, ud))

#ಆದರೆ_ಅಂತ್ಯ							/* } */

#ಆದರೆ_ಅಂತ್ಯ							/* } */


ಶೂನ್ಯ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop) {
  ಆದರೆ (errcode == LUA_ERRMEM) {  /* memory error? */
    setsvalue2s(L, oldtop, G(L)->memerrmsg); /* reuse preregistered msg. */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_assert(errorstatus(errcode));  /* must be a real error */
    lua_assert(!ttisnil(s2v(L->ತುದಿ.p - 1)));  /* with a non-nil object */
    setobjs2s(L, oldtop, L->ತುದಿ.p - 1);  /* move it to 'oldtop' */
  }
  L->ತುದಿ.p = oldtop + 1;  /* top goes back to old top plus error object */
}


l_noret luaD_throw (lua_State *L, TStatus errcode) {
  ಆದರೆ (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->ಸ್ಥಿತಿ_2 = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* thread has no error handler */
    global_State *g = G(L);
    lua_State *mainth = mainthread(g);
    errcode = luaE_resetthread(L, errcode);  /* close all upvalues */
    L->ಸ್ಥಿತಿ_2 = errcode;
    ಆದರೆ (mainth->errorJmp) {  /* main thread has a handler? */
      setobjs2s(L, mainth->ತುದಿ.p++, L->ತುದಿ.p - 1);  /* copy error obj. */
      luaD_throw(mainth, errcode);  /* re-throw in main thread */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* no handler at all; abort */
      ಆದರೆ (g->panic) {  /* panic function? */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode) {
  ಆದರೆ (L->errorJmp) {
    /* unroll error entries up to the first level */
    ಆಗ (L->errorJmp->ಹಿಂದಿನ != ಖಾಲಿ)
      L->errorJmp = L->errorJmp->ಹಿಂದಿನ;
  }
  luaD_throw(L, errcode);
}


TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ಶೂನ್ಯ *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  lua_longjmp lj;
  lj.ಸ್ಥಿತಿ_2 = LUA_OK;
  lj.ಹಿಂದಿನ = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj, f, ud);  /* call 'f' catching errors */
  L->errorJmp = lj.ಹಿಂದಿನ;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  ಹಿಂತಿರುಗಿ lj.ಸ್ಥಿತಿ_2;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/

/* some stack space for error handling */
#ವ್ಯಾಖ್ಯಾನಿಸು STACKERRSPACE	200


/*
** LUAI_MAXSTACK limits the size of the Lua stack.
** It must fit into INT_MAX/2.
*/

#ಆದರೆ !defined(LUAI_MAXSTACK)
#ಆದರೆ 1000000 < (ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 2)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXSTACK           1000000
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXSTACK           (ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 2u)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ


/* maximum stack size that respects size_t */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXSTACK_BYSIZET  ((MAX_SIZET / ಗಾತ್ರ(ರಾಶಿ_ಮೌಲ್ಯ)) - STACKERRSPACE)

/*
** Minimum between LUAI_MAXSTACK and MAXSTACK_BYSIZET
** (Maximum size for the stack must respect size_t.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXSTACK	cast_int(LUAI_MAXSTACK < MAXSTACK_BYSIZET  \
			        ? LUAI_MAXSTACK : MAXSTACK_BYSIZET)


/* stack size with extra space for error handling */
#ವ್ಯಾಖ್ಯಾನಿಸು ERRORSTACKSIZE	(MAXSTACK + STACKERRSPACE)


/* raise a stack error while running the message handler */
l_noret luaD_errerr (lua_State *L) {
  TString *ಸಂದೇಶ = luaS_newliteral(L, "error in error handling");
  setsvalue2s(L, L->ತುದಿ.p, ಸಂದೇಶ);
  L->ತುದಿ.p++;  /* assume EXTRA_STACK */
  luaD_throw(L, LUA_ERRERR);
}


/*
** Check whether stacks have enough space to run a simple function (such
** as a finalizer): At least BASIC_STACK_SIZE in the Lua stack, two
** available CallInfos, and two "slots" in the C stack.
*/
ಪೂರ್ಣಾಂಕ luaD_checkminstack (lua_State *L) {
  ಆದರೆ (getCcalls(L) >= LUAI_MAXCCALLS - 2)
    ಹಿಂತಿರುಗಿ 0;  /* not enough C-stack slots */
  ಆದರೆ (L->ci->ಮುಂದಿನ == ಖಾಲಿ && luaE_extendCI(L, 0) == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;  /* unable to allocate first ci */
  ಆದರೆ (L->ci->ಮುಂದಿನ->ಮುಂದಿನ == ಖಾಲಿ && luaE_extendCI(L, 0) == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;  /* unable to allocate second ci */
  ಆದರೆ (L->ರಾಶಿ_ಕೊನೆಯ.p - L->ತುದಿ.p >= BASIC_ರಾಶಿ_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ 1;  /* enough (BASIC_STACK_SIZE) free slots in the Lua stack */
  ಇಲ್ಲದಿದ್ದರೆ  /* try to grow stack to a size with enough free slots */
    ಹಿಂತಿರುಗಿ luaD_growstack(L, BASIC_ರಾಶಿ_ಗಾತ್ರ, 0);
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
#ಆದರೆ !defined(LUAI_STRICT_ವಿಳಾಸ)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_STRICT_ವಿಳಾಸ	1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ LUAI_STRICT_ವಿಳಾಸ
/*
** Change all pointers to the stack into offsets.
*/
ಸ್ಥಿರ ಶೂನ್ಯ relstack (lua_State *L) {
  CallInfo *ci;
  ಮೇಲೆ_Val *ಮೇಲೆ;
  L->ತುದಿ.ಸ್ಥಾನಾಂತರ = savestack(L, L->ತುದಿ.p);
  L->tbclist.ಸ್ಥಾನಾಂತರ = savestack(L, L->tbclist.p);
  ಗಾಗಿ (ಮೇಲೆ = L->openupval; ಮೇಲೆ != ಖಾಲಿ; ಮೇಲೆ = ಮೇಲೆ->u.open.ಮುಂದಿನ)
    ಮೇಲೆ->v.ಸ್ಥಾನಾಂತರ = savestack(L, uplevel(ಮೇಲೆ));
  ಗಾಗಿ (ci = L->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {
    ci->ತುದಿ.ಸ್ಥಾನಾಂತರ = savestack(L, ci->ತುದಿ.p);
    ci->func.ಸ್ಥಾನಾಂತರ = savestack(L, ci->func.p);
  }
}


/*
** Change back all offsets into pointers.
*/
ಸ್ಥಿರ ಶೂನ್ಯ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ಮೇಲೆ_Val *ಮೇಲೆ;
  ಬಳಸದ(oldstack);
  L->ತುದಿ.p = restorestack(L, L->ತುದಿ.ಸ್ಥಾನಾಂತರ);
  L->tbclist.p = restorestack(L, L->tbclist.ಸ್ಥಾನಾಂತರ);
  ಗಾಗಿ (ಮೇಲೆ = L->openupval; ಮೇಲೆ != ಖಾಲಿ; ಮೇಲೆ = ಮೇಲೆ->u.open.ಮುಂದಿನ)
    ಮೇಲೆ->v.p = s2v(restorestack(L, ಮೇಲೆ->v.ಸ್ಥಾನಾಂತರ));
  ಗಾಗಿ (ci = L->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {
    ci->ತುದಿ.p = restorestack(L, ci->ತುದಿ.ಸ್ಥಾನಾಂತರ);
    ci->func.p = restorestack(L, ci->func.ಸ್ಥಾನಾಂತರ);
    ಆದರೆ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}

#ಇಲ್ಲದಿದ್ದರೆ
/*
** Assume that it is fine to use an address after its deallocation,
** as long as we do not dereference it.
*/

ಸ್ಥಿರ ಶೂನ್ಯ relstack (lua_State *L) { ಬಳಸದ(L); }  /* do nothing */


/*
** Correct pointers into 'oldstack' to point into 'L->stack'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ correctstack (lua_State *L, StkId oldstack) {
  CallInfo *ci;
  ಮೇಲೆ_Val *ಮೇಲೆ;
  StkId newstack = L->ರಾಶಿ.p;
  ಆದರೆ (oldstack == newstack)
    ಹಿಂತಿರುಗಿ;
  L->ತುದಿ.p = L->ತುದಿ.p - oldstack + newstack;
  L->tbclist.p = L->tbclist.p - oldstack + newstack;
  ಗಾಗಿ (ಮೇಲೆ = L->openupval; ಮೇಲೆ != ಖಾಲಿ; ಮೇಲೆ = ಮೇಲೆ->u.open.ಮುಂದಿನ)
    ಮೇಲೆ->v.p = s2v(uplevel(ಮೇಲೆ) - oldstack + newstack);
  ಗಾಗಿ (ci = L->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {
    ci->ತುದಿ.p = ci->ತುದಿ.p - oldstack + newstack;
    ci->func.p = ci->func.p - oldstack + newstack;
    ಆದರೆ (isLua(ci))
      ci->u.l.trap = 1;  /* signal to update 'trap' in 'luaV_execute' */
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Reallocate the stack to a new size, correcting all pointers into it.
** In case of allocation error, raise an error or return false according
** to 'raiseerror'.
*/
ಪೂರ್ಣಾಂಕ luaD_reallocstack (lua_State *L, ಪೂರ್ಣಾಂಕ newsize, ಪೂರ್ಣಾಂಕ raiseerror) {
  ಪೂರ್ಣಾಂಕ oldsize = stacksize(L);
  ಪೂರ್ಣಾಂಕ i;
  StkId newstack;
  StkId oldstack = L->ರಾಶಿ.p;
  lu_byte oldgcstop = G(L)->gcstopem;
  lua_assert(newsize <= MAXSTACK || newsize == ERRORSTACKSIZE);
  relstack(L);  /* change pointers to offsets */
  G(L)->gcstopem = 1;  /* stop emergency collection */
  newstack = luaM_reallocvector(L, oldstack, oldsize + EXTRA_ರಾಶಿ,
                                   newsize + EXTRA_ರಾಶಿ, ರಾಶಿ_ಮೌಲ್ಯ);
  G(L)->gcstopem = oldgcstop;  /* restore emergency collection */
  ಆದರೆ (l_unlikely(newstack == ಖಾಲಿ)) {  /* reallocation failed? */
    correctstack(L, oldstack);  /* change offsets back to pointers */
    ಆದರೆ (raiseerror)
      luaM_error(L);
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;  /* do not raise an error */
  }
  L->ರಾಶಿ.p = newstack;
  correctstack(L, oldstack);  /* change offsets back to pointers */
  L->ರಾಶಿ_ಕೊನೆಯ.p = L->ರಾಶಿ.p + newsize;
  ಗಾಗಿ (i = oldsize + EXTRA_ರಾಶಿ; i < newsize + EXTRA_ರಾಶಿ; i++)
    setnilvalue2s(newstack + i); /* erase new segment */
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Try to grow the stack by at least 'n' elements. When 'raiseerror'
** is true, raises any error; otherwise, return 0 in case of errors.
*/
ಪೂರ್ಣಾಂಕ luaD_growstack (lua_State *L, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ raiseerror) {
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = stacksize(L);
  ಆದರೆ (l_unlikely(ಗಾತ್ರ_2 > MAXSTACK)) {
    /* if stack is larger than maximum, thread is already using the
       extra space reserved for errors, that is, thread is handling
       a stack error; cannot grow further than that. */
    lua_assert(stacksize(L) == ERRORSTACKSIZE);
    ಆದರೆ (raiseerror)
      luaD_errerr(L);  /* stack error inside message handler */
    ಹಿಂತಿರುಗಿ 0;  /* if not 'raiseerror', just signal it */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n < MAXSTACK) {  /* avoids arithmetic overflows */
    ಪೂರ್ಣಾಂಕ newsize = ಗಾತ್ರ_2 + (ಗಾತ್ರ_2 >> 1);  /* tentative new size (size * 1.5) */
    ಪೂರ್ಣಾಂಕ needed = cast_int(L->ತುದಿ.p - L->ರಾಶಿ.p) + n;
    ಆದರೆ (newsize > MAXSTACK)  /* cannot cross the limit */
      newsize = MAXSTACK;
    ಆದರೆ (newsize < needed)  /* but must respect what was asked for */
      newsize = needed;
    ಆದರೆ (l_likely(newsize <= MAXSTACK))
      ಹಿಂತಿರುಗಿ luaD_reallocstack(L, newsize, raiseerror);
  }
  /* else stack overflow */
  /* add extra size to be able to handle the error message */
  luaD_reallocstack(L, ERRORSTACKSIZE, raiseerror);
  ಆದರೆ (raiseerror)
    luaG_runerror(L, "stack overflow");
  ಹಿಂತಿರುಗಿ 0;
}


/*
** Compute how much of the stack is being used, by computing the
** maximum top of all call frames in the stack and the current top.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stackinuse (lua_State *L) {
  CallInfo *ci;
  ಪೂರ್ಣಾಂಕ res;
  StkId lim = L->ತುದಿ.p;
  ಗಾಗಿ (ci = L->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {
    ಆದರೆ (lim < ci->ತುದಿ.p) lim = ci->ತುದಿ.p;
  }
  lua_assert(lim <= L->ರಾಶಿ_ಕೊನೆಯ.p + EXTRA_ರಾಶಿ);
  res = cast_int(lim - L->ರಾಶಿ.p) + 1;  /* part of stack in use */
  ಆದರೆ (res < LUA_MINSTACK)
    res = LUA_MINSTACK;  /* ensure a minimum size */
  ಹಿಂತಿರುಗಿ res;
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
ಶೂನ್ಯ luaD_shrinkstack (lua_State *L) {
  ಪೂರ್ಣಾಂಕ inuse = stackinuse(L);
  ಪೂರ್ಣಾಂಕ max = (inuse > MAXSTACK / 3) ? MAXSTACK : inuse * 3;
  /* if thread is currently not handling a stack overflow and its
     size is larger than maximum "reasonable" size, shrink it */
  ಆದರೆ (inuse <= MAXSTACK && stacksize(L) > max) {
    ಪೂರ್ಣಾಂಕ nsize = (inuse > MAXSTACK / 2) ? MAXSTACK : inuse * 2;
    luaD_reallocstack(L, nsize, 0);  /* ok if that fails */
  }
  ಇಲ್ಲದಿದ್ದರೆ  /* don't change stack */
    condmovestack(L,(ಶೂನ್ಯ)0,(ಶೂನ್ಯ)0);  /* (change only for debugging) */
  luaE_shrinkCI(L);  /* shrink CI list */
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which trigger this
** function, can be changed asynchronously by signals.)
*/
ಶೂನ್ಯ luaD_hook (lua_State *L, ಪೂರ್ಣಾಂಕ event, ಪೂರ್ಣಾಂಕ ಸಾಲು,
                              ಪೂರ್ಣಾಂಕ ftransfer, ಪೂರ್ಣಾಂಕ ntransfer) {
  lua_Hook hook = L->hook;
  ಆದರೆ (hook && L->allowhook) {  /* make sure there is a hook */
    CallInfo *ci = L->ci;
    ptrdiff_t ತುದಿ = savestack(L, L->ತುದಿ.p);  /* preserve original 'top' */
    ptrdiff_t ci_ತುದಿ = savestack(L, ci->ತುದಿ.p);  /* idem for 'ci->top' */
    lua_Debug ar;
    ar.event = event;
    ar.currentline = ಸಾಲು;
    ar.i_ci = ci;
    L->transferinfo.ftransfer = ftransfer;
    L->transferinfo.ntransfer = ntransfer;
    ಆದರೆ (isLua(ci) && L->ತುದಿ.p < ci->ತುದಿ.p)
      L->ತುದಿ.p = ci->ತುದಿ.p;  /* protect entire activation register */
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    ಆದರೆ (ci->ತುದಿ.p < L->ತುದಿ.p + LUA_MINSTACK)
      ci->ತುದಿ.p = L->ತುದಿ.p + LUA_MINSTACK;
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_ಬೀಗ(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->ತುದಿ.p = restorestack(L, ci_ತುದಿ);
    L->ತುದಿ.p = restorestack(L, ತುದಿ);
    ci->callstatus &= ~CIST_HOOKED;
  }
}


/*
** Executes a call hook for Lua functions. This function is called
** whenever 'hookmask' is not zero, so it checks whether call hooks are
** active.
*/
ಶೂನ್ಯ luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;  /* set 'oldpc' for new function */
  ಆದರೆ (L->hookmask & LUA_MASKCALL) {  /* is call hook on? */
    ಪೂರ್ಣಾಂಕ event = (ci->callstatus & CIST_ಬಾಲ) ? LUA_HOOKTAILCALL
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
ಸ್ಥಿರ ಶೂನ್ಯ rethook (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಂಕ nres) {
  ಆದರೆ (L->hookmask & LUA_MASKRET) {  /* is return hook on? */
    StkId firstres = L->ತುದಿ.p - nres;  /* index of first result */
    ಪೂರ್ಣಾಂಕ delta = 0;  /* correction for vararg functions */
    ಪೂರ್ಣಾಂಕ ftransfer;
    ಆದರೆ (isLua(ci)) {
      Proto *p = ci_func(ci)->p;
      ಆದರೆ (p->flag & PF_VAHID)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;  /* if vararg, back to virtual 'func' */
    ftransfer = cast_int(firstres - ci->func.p);
    luaD_hook(L, LUA_HOOKRET, -1, ftransfer, nres);  /* call it */
    ci->func.p -= delta;
  }
  ಆದರೆ (isLua(ci = ci->ಹಿಂದಿನ))
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
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ tryfuncTM (lua_State *L, StkId func, ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿತಿ_2) {
  ಸ್ಥಿರಾಂಕ TValue *tm;
  StkId p;
  tm = luaT_gettmbyobj(L, s2v(func), TM_CALL);
  ಆದರೆ (l_unlikely(ttisnil(tm)))  /* no metamethod? */
    luaG_callerror(L, s2v(func));
  ಗಾಗಿ (p = L->ತುದಿ.p; p > func; p--)  /* open space for metamethod */
    setobjs2s(L, p, p-1);
  L->ತುದಿ.p++;  /* stack space pre-allocated by the caller */
  setobj2s(L, func, tm);  /* metamethod is the new function to be called */
  ಆದರೆ ((ಸ್ಥಿತಿ_2 & MAX_CCMT) == MAX_CCMT)  /* is counter full? */
    luaG_runerror(L, "'__call' chain too long");
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 + (1u << CIST_CCMT);  /* increment counter */
}


/* Generic case for 'moveresult' */
l_sinline ಶೂನ್ಯ genmoveresults (lua_State *L, StkId res, ಪೂರ್ಣಾಂಕ nres,
                                             ಪೂರ್ಣಾಂಕ wanted) {
  StkId firstresult = L->ತುದಿ.p - nres;  /* index of first result */
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ (nres > wanted)  /* extra results? */
    nres = wanted;  /* don't need them */
  ಗಾಗಿ (i = 0; i < nres; i++)  /* move all results to correct place */
    setobjs2s(L, res + i, firstresult + i);
  ಗಾಗಿ (; i < wanted; i++)  /* complete wanted number of results */
    setnilvalue2s(res + i);
  L->ತುದಿ.p = res + wanted;  /* top points after the last result */
}


/*
** Given 'nres' results at 'firstResult', move 'fwanted-1' of them
** to 'res'.  Handle most typical cases (zero results for commands,
** one result for expressions, multiple results for tail calls/single
** parameters) separated. The flag CIST_TBC in 'fwanted', if set,
** forces the switch to go to the default case.
*/
l_sinline ಶೂನ್ಯ moveresults (lua_State *L, StkId res, ಪೂರ್ಣಾಂಕ nres,
                                          l_uint32 fwanted) {
  ಆಯ್ಕೆ (fwanted) {  /* handle typical cases separately */
    ಸ್ಥಿತಿ 0 + 1:  /* no values needed */
      L->ತುದಿ.p = res;
      ಹಿಂತಿರುಗಿ;
    ಸ್ಥಿತಿ 1 + 1:  /* one value needed */
      ಆದರೆ (nres == 0)   /* no results? */
        setnilvalue2s(res);  /* adjust with nil */
      ಇಲ್ಲದಿದ್ದರೆ  /* at least one result */
        setobjs2s(L, res, L->ತುದಿ.p - nres);  /* move it to proper place */
      L->ತುದಿ.p = res + 1;
      ಹಿಂತಿರುಗಿ;
    ಸ್ಥಿತಿ LUA_MULTRET + 1:
      genmoveresults(L, res, nres, nres);  /* we want all results */
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* two/more results and/or to-be-closed variables */
      ಪೂರ್ಣಾಂಕ wanted = get_nresults(fwanted);
      ಆದರೆ (fwanted & CIST_TBC) {  /* to-be-closed variables? */
        L->ci->u2.nres = nres;
        L->ci->callstatus |= CIST_CLSRET;  /* in case of yields */
        res = luaF_close(L, res, CLOSEKTOP, 1);
        L->ci->callstatus &= ~CIST_CLSRET;
        ಆದರೆ (L->hookmask) {  /* if needed, call hook after '__close's */
          ptrdiff_t savedres = savestack(L, res);
          rethook(L, L->ci, nres);
          res = restorestack(L, savedres);  /* hook can move stack */
        }
        ಆದರೆ (wanted == LUA_MULTRET)
          wanted = nres;  /* we want all results */
      }
      genmoveresults(L, res, nres, wanted);
      ನಿಲ್ಲಿಸು;
    }
  }
}


/*
** Finishes a function call: calls hook if necessary, moves current
** number of results to proper place, and returns to previous call
** info. If function has to close variables, hook must be called after
** that.
*/
ಶೂನ್ಯ luaD_poscall (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಂಕ nres) {
  l_uint32 fwanted = ci->callstatus & (CIST_TBC | CIST_NRESULTS);
  ಆದರೆ (l_unlikely(L->hookmask) && !(fwanted & CIST_TBC))
    rethook(L, ci, nres);
  /* move results to proper place */
  moveresults(L, ci->func.p, nres, fwanted);
  /* function cannot be in any of these cases when returning */
  lua_assert(!(ci->callstatus &
        (CIST_HOOKED | CIST_YPCALL | CIST_FIN | CIST_CLSRET)));
  L->ci = ci->ಹಿಂದಿನ;  /* back to caller (after closing variables) */
}



#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_ci(L)  (L->ci->ಮುಂದಿನ ? L->ci->ಮುಂದಿನ : luaE_extendCI(L, 1))


/*
** Allocate and initialize CallInfo structure. At this point, the
** only valid fields in the call status are number of results,
** CIST_C (if it's a C function), and number of extra arguments.
** (All these bit-fields fit in 16-bit values.)
*/
l_sinline CallInfo *prepCallInfo (lua_State *L, StkId func, ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿತಿ_2,
                                                StkId ತುದಿ) {
  CallInfo *ci = L->ci = ಮುಂದಿನ_ci(L);  /* new frame */
  ci->func.p = func;
  lua_assert((ಸ್ಥಿತಿ_2 & ~(CIST_NRESULTS | CIST_C | MAX_CCMT)) == 0);
  ci->callstatus = ಸ್ಥಿತಿ_2;
  ci->ತುದಿ.p = ತುದಿ;
  ಹಿಂತಿರುಗಿ ci;
}


/*
** precall for C functions
*/
l_sinline ಪೂರ್ಣಾಂಕ precallC (lua_State *L, StkId func, ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿತಿ_2,
                                            lua_CFunction f) {
  ಪೂರ್ಣಾಂಕ n;  /* number of returns */
  CallInfo *ci;
  checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
  L->ci = ci = prepCallInfo(L, func, ಸ್ಥಿತಿ_2 | CIST_C,
                               L->ತುದಿ.p + LUA_MINSTACK);
  lua_assert(ci->ತುದಿ.p <= L->ರಾಶಿ_ಕೊನೆಯ.p);
  ಆದರೆ (l_unlikely(L->hookmask & LUA_MASKCALL)) {
    ಪೂರ್ಣಾಂಕ narg = cast_int(L->ತುದಿ.p - func) - 1;
    luaD_hook(L, LUA_HOOKCALL, -1, 1, narg);
  }
  lua_unlock(L);
  n = (*f)(L);  /* do the actual call */
  lua_ಬೀಗ(L);
  api_checknelems(L, n);
  luaD_poscall(L, ci, n);
  ಹಿಂತಿರುಗಿ n;
}


/*
** Prepare a function for a tail call, building its call info on top
** of the current call info. 'narg1' is the number of arguments plus 1
** (so that it includes the function itself). Return the number of
** results, if it was a C function, or -1 for a Lua function.
*/
ಪೂರ್ಣಾಂಕ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    ಪೂರ್ಣಾಂಕ narg1, ಪೂರ್ಣಾಂಕ delta) {
  ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿತಿ_2 = LUA_MULTRET + 1;
 retry:
  ಆಯ್ಕೆ (ttypetag(s2v(func))) {
    ಸ್ಥಿತಿ LUA_VCCL:  /* C closure */
      ಹಿಂತಿರುಗಿ precallC(L, func, ಸ್ಥಿತಿ_2, clCvalue(s2v(func))->f);
    ಸ್ಥಿತಿ LUA_VLCF:  /* light C function */
      ಹಿಂತಿರುಗಿ precallC(L, func, ಸ್ಥಿತಿ_2, fvalue(s2v(func)));
    ಸ್ಥಿತಿ LUA_VLCL: {  /* Lua function */
      Proto *p = clLvalue(s2v(func))->p;
      ಪೂರ್ಣಾಂಕ fsize = p->maxstacksize;  /* frame size */
      ಪೂರ್ಣಾಂಕ nfixparams = p->numparams;
      ಪೂರ್ಣಾಂಕ i;
      checkstackp(L, fsize - delta, func);
      ci->func.p -= delta;  /* restore 'func' (if vararg) */
      ಗಾಗಿ (i = 0; i < narg1; i++)  /* move down function and arguments */
        setobjs2s(L, ci->func.p + i, func + i);
      func = ci->func.p;  /* moved-down function */
      ಗಾಗಿ (; narg1 <= nfixparams; narg1++)
        setnilvalue2s(func + narg1);  /* complete missing arguments */
      ci->ತುದಿ.p = func + 1 + fsize;  /* top for new function */
      lua_assert(ci->ತುದಿ.p <= L->ರಾಶಿ_ಕೊನೆಯ.p);
      ci->u.l.savedpc = p->ಸಂಕೇತ_2;  /* starting point */
      ci->callstatus |= CIST_ಬಾಲ;
      L->ತುದಿ.p = func + narg1;  /* set top */
      ಹಿಂತಿರುಗಿ -1;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      ಸ್ಥಿತಿ_2 = tryfuncTM(L, func, ಸ್ಥಿತಿ_2);  /* try '__call' metamethod */
      narg1++;
      ಹೋಗು retry;  /* try again */
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
CallInfo *luaD_precall (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nresults) {
  ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿತಿ_2 = cast_uint(nresults + 1);
  lua_assert(ಸ್ಥಿತಿ_2 <= MAXRESULTS + 1);
 retry:
  ಆಯ್ಕೆ (ttypetag(s2v(func))) {
    ಸ್ಥಿತಿ LUA_VCCL:  /* C closure */
      precallC(L, func, ಸ್ಥಿತಿ_2, clCvalue(s2v(func))->f);
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಸ್ಥಿತಿ LUA_VLCF:  /* light C function */
      precallC(L, func, ಸ್ಥಿತಿ_2, fvalue(s2v(func)));
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಸ್ಥಿತಿ LUA_VLCL: {  /* Lua function */
      CallInfo *ci;
      Proto *p = clLvalue(s2v(func))->p;
      ಪೂರ್ಣಾಂಕ narg = cast_int(L->ತುದಿ.p - func) - 1;  /* number of real arguments */
      ಪೂರ್ಣಾಂಕ nfixparams = p->numparams;
      ಪೂರ್ಣಾಂಕ fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      L->ci = ci = prepCallInfo(L, func, ಸ್ಥಿತಿ_2, func + 1 + fsize);
      ci->u.l.savedpc = p->ಸಂಕೇತ_2;  /* starting point */
      ಗಾಗಿ (; narg < nfixparams; narg++)
        setnilvalue2s(L->ತುದಿ.p++);  /* complete missing arguments */
      lua_assert(ci->ತುದಿ.p <= L->ರಾಶಿ_ಕೊನೆಯ.p);
      ಹಿಂತಿರುಗಿ ci;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* not a function */
      checkstackp(L, 1, func);  /* space for metamethod */
      ಸ್ಥಿತಿ_2 = tryfuncTM(L, func, ಸ್ಥಿತಿ_2);  /* try '__call' metamethod */
      ಹೋಗು retry;  /* try again with metamethod */
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
l_sinline ಶೂನ್ಯ ccall (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  ಆದರೆ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS)) {
    checkstackp(L, 0, func);  /* free any use of EXTRA_STACK */
    luaE_checkcstack(L);
  }
  ಆದರೆ ((ci = luaD_precall(L, func, nResults)) != ಖಾಲಿ) {  /* Lua function? */
    ci->callstatus |= CIST_FRESH;  /* mark that it is a "fresh" execute */
    luaV_execute(L, ci);  /* call it */
  }
  L->nCcalls -= inc;
}


/*
** External interface for 'ccall'
*/
ಶೂನ್ಯ luaD_call (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults) {
  ccall(L, func, nResults, 1);
}


/*
** Similar to 'luaD_call', but does not allow yields during the call.
*/
ಶೂನ್ಯ luaD_callnoyield (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults) {
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
ಸ್ಥಿರ TStatus finishpcallk (lua_State *L,  CallInfo *ci) {
  TStatus ಸ್ಥಿತಿ_2 = getcistrecst(ci);  /* get original status */
  ಆದರೆ (l_likely(ಸ್ಥಿತಿ_2 == LUA_OK))  /* no error? */
    ಸ್ಥಿತಿ_2 = LUA_YIELD;  /* was interrupted by an yield */
  ಇಲ್ಲದಿದ್ದರೆ {  /* error */
    StkId func = restorestack(L, ci->u2.funcidx);
    L->allowhook = getoah(ci);  /* restore 'allowhook' */
    func = luaF_close(L, func, ಸ್ಥಿತಿ_2, 1);  /* can yield or raise an error */
    luaD_seterrorobj(L, ಸ್ಥಿತಿ_2, func);
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
    setcistrecst(ci, LUA_OK);  /* clear original status */
  }
  ci->callstatus &= ~CIST_YPCALL;
  L->errfunc = ci->u.c.old_errfunc;
  /* if it is here, there were errors or yields; unlike 'lua_pcallk',
     do not change status */
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಮುಕ್ತಾಯ_Ccall (lua_State *L, CallInfo *ci) {
  ಪೂರ್ಣಾಂಕ n;  /* actual number of results from C function */
  ಆದರೆ (ci->callstatus & CIST_CLSRET) {  /* was closing TBC variable? */
    lua_assert(ci->callstatus & CIST_TBC);
    n = ci->u2.nres;  /* just redo 'luaD_poscall' */
    /* don't need to reset CIST_CLSRET, as it will be set again anyway */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    TStatus ಸ್ಥಿತಿ_2 = LUA_YIELD;  /* default if there were no errors */
    lua_KFunction kf = ci->u.c.k;  /* continuation function */
    /* must have a continuation and must be able to call it */
    lua_assert(kf != ಖಾಲಿ && yieldable(L));
    ಆದರೆ (ci->callstatus & CIST_YPCALL)   /* was inside a 'lua_pcallk'? */
      ಸ್ಥಿತಿ_2 = finishpcallk(L, ci);  /* finish it */
    adjustresults(L, LUA_MULTRET);  /* finish 'lua_callk' */
    lua_unlock(L);
    n = (*kf)(L, APIstatus(ಸ್ಥಿತಿ_2), ci->u.c.ctx);  /* call continuation */
    lua_ಬೀಗ(L);
    api_checknelems(L, n);
  }
  luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop).
*/
ಸ್ಥಿರ ಶೂನ್ಯ unroll (lua_State *L, ಶೂನ್ಯ *ud) {
  CallInfo *ci;
  ಬಳಸದ(ud);
  ಆಗ ((ci = L->ci) != &L->ತಳ_ci) {  /* something in the stack */
    ಆದರೆ (!isLua(ci))  /* C function? */
      ಮುಕ್ತಾಯ_Ccall(L, ci);  /* complete its execution */
    ಇಲ್ಲದಿದ್ದರೆ {  /* Lua function */
      lua_V_ಮುಕ್ತಾಯ_Op(L);  /* finish interrupted instruction */
      luaV_execute(L, ci);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
ಸ್ಥಿರ CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  ಗಾಗಿ (ci = L->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {  /* search for a pcall */
    ಆದರೆ (ci->callstatus & CIST_YPCALL)
      ಹಿಂತಿರುಗಿ ci;
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no pending pcall */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resume_error (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ narg) {
  api_checkpop(L, narg);
  L->ತುದಿ.p -= narg;  /* remove args from the stack */
  setsvalue2s(L, L->ತುದಿ.p, luaS_new(L, ಸಂದೇಶ));  /* push error message */
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resume (lua_State *L, ಶೂನ್ಯ *ud) {
  ಪೂರ್ಣಾಂಕ n = *(cast(ಪೂರ್ಣಾಂಕ*, ud));  /* number of arguments */
  StkId ಮೊದಲ_Arg = L->ತುದಿ.p - n;  /* first argument */
  CallInfo *ci = L->ci;
  ಆದರೆ (L->ಸ್ಥಿತಿ_2 == LUA_OK)  /* starting a coroutine? */
    ccall(L, ಮೊದಲ_Arg - 1, LUA_MULTRET, 0);  /* just call its body */
  ಇಲ್ಲದಿದ್ದರೆ {  /* resuming from previous yield */
    lua_assert(L->ಸ್ಥಿತಿ_2 == LUA_YIELD);
    L->ಸ್ಥಿತಿ_2 = LUA_OK;  /* mark that it is running (again) */
    ಆದರೆ (isLua(ci)) {  /* yielded inside a hook? */
      /* undo increment made by 'luaG_traceexec': instruction was not
         executed yet */
      lua_assert(ci->callstatus & CIST_HOOKYIELD);
      ci->u.l.savedpc--;
      L->ತುದಿ.p = ಮೊದಲ_Arg;  /* discard arguments */
      luaV_execute(L, ci);  /* just continue running Lua code */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* 'common' yield */
      ಆದರೆ (ci->u.c.k != ಖಾಲಿ) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_ಬೀಗ(L);
        api_checknelems(L, n);
      }
      luaD_poscall(L, ci, n);  /* finish 'luaD_call' */
    }
    unroll(L, ಖಾಲಿ);  /* run continuation */
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
ಸ್ಥಿರ TStatus precover (lua_State *L, TStatus ಸ್ಥಿತಿ_2) {
  CallInfo *ci;
  ಆಗ (errorstatus(ಸ್ಥಿತಿ_2) && (ci = findpcall(L)) != ಖಾಲಿ) {
    L->ci = ci;  /* go down to recovery functions */
    setcistrecst(ci, ಸ್ಥಿತಿ_2);  /* status to finish 'pcall' */
    ಸ್ಥಿತಿ_2 = luaD_rawrunprotected(L, unroll, ಖಾಲಿ);
  }
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_resume (lua_State *L, lua_State *from, ಪೂರ್ಣಾಂಕ nargs,
                                      ಪೂರ್ಣಾಂಕ *nresults) {
  TStatus ಸ್ಥಿತಿ_2;
  lua_ಬೀಗ(L);
  ಆದರೆ (L->ಸ್ಥಿತಿ_2 == LUA_OK) {  /* may be starting a coroutine */
    ಆದರೆ (L->ci != &L->ತಳ_ci)  /* not in base level? */
      ಹಿಂತಿರುಗಿ resume_error(L, "cannot resume non-suspended coroutine", nargs);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (L->ತುದಿ.p - (L->ci->func.p + 1) == nargs)  /* no function? */
      ಹಿಂತಿರುಗಿ resume_error(L, "cannot resume dead coroutine", nargs);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (L->ಸ್ಥಿತಿ_2 != LUA_YIELD)  /* ended with errors? */
    ಹಿಂತಿರುಗಿ resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  ಆದರೆ (getCcalls(L) >= LUAI_MAXCCALLS)
    ಹಿಂತಿರುಗಿ resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  luai_userstateresume(L, nargs);
  api_checkpop(L, (L->ಸ್ಥಿತಿ_2 == LUA_OK) ? nargs + 1 : nargs);
  ಸ್ಥಿತಿ_2 = luaD_rawrunprotected(L, resume, &nargs);
   /* continue running after recoverable errors */
  ಸ್ಥಿತಿ_2 = precover(L, ಸ್ಥಿತಿ_2);
  ಆದರೆ (l_likely(!errorstatus(ಸ್ಥಿತಿ_2)))
    lua_assert(ಸ್ಥಿತಿ_2 == L->ಸ್ಥಿತಿ_2);  /* normal end or yield */
  ಇಲ್ಲದಿದ್ದರೆ {  /* unrecoverable error */
    L->ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_2;  /* mark thread as 'dead' */
    luaD_seterrorobj(L, ಸ್ಥಿತಿ_2, L->ತುದಿ.p);  /* push error message */
    L->ci->ತುದಿ.p = L->ತುದಿ.p;
  }
  *nresults = (ಸ್ಥಿತಿ_2 == LUA_YIELD) ? L->ci->u2.nyield
                                    : cast_int(L->ತುದಿ.p - (L->ci->func.p + 1));
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ APIstatus(ಸ್ಥಿತಿ_2);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_isyieldable (lua_State *L) {
  ಹಿಂತಿರುಗಿ yieldable(L);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_yieldk (lua_State *L, ಪೂರ್ಣಾಂಕ nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  luai_userstateyield(L, nresults);
  lua_ಬೀಗ(L);
  ci = L->ci;
  api_checkpop(L, nresults);
  ಆದರೆ (l_unlikely(!yieldable(L))) {
    ಆದರೆ (L != mainthread(G(L)))
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    ಇಲ್ಲದಿದ್ದರೆ
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->ಸ್ಥಿತಿ_2 = LUA_YIELD;
  ci->u2.nyield = nresults;  /* save number of results */
  ಆದರೆ (isLua(ci)) {  /* inside a hook? */
    lua_assert(!isLuacode(ci));
    api_check(L, nresults == 0, "hooks cannot yield values");
    api_check(L, k == ಖಾಲಿ, "hooks cannot continue after yielding");
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ ((ci->u.c.k = k) != ಖಾಲಿ)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* save context */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ 0;  /* return to 'luaD_hook' */
}


/*
** Auxiliary structure to call 'luaF_close' in protected mode.
*/
ರಚನೆ CloseP {
  StkId level;
  TStatus ಸ್ಥಿತಿ_2;
};


/*
** Auxiliary function to call 'luaF_close' in protected mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ closepaux (lua_State *L, ಶೂನ್ಯ *ud) {
  ರಚನೆ CloseP *pcl = cast(ರಚನೆ CloseP *, ud);
  luaF_close(L, pcl->level, pcl->ಸ್ಥಿತಿ_2, 0);
}


/*
** Calls 'luaF_close' in protected mode. Return the original status
** or, in case of errors, the new status.
*/
TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level, TStatus ಸ್ಥಿತಿ_2) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ಗಾಗಿ (;;) {  /* keep closing upvalues until no more errors */
    ರಚನೆ CloseP pcl;
    pcl.level = restorestack(L, level); pcl.ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_2;
    ಸ್ಥಿತಿ_2 = luaD_rawrunprotected(L, &closepaux, &pcl);
    ಆದರೆ (l_likely(ಸ್ಥಿತಿ_2 == LUA_OK))  /* no more errors? */
      ಹಿಂತಿರುಗಿ pcl.ಸ್ಥಿತಿ_2;
    ಇಲ್ಲದಿದ್ದರೆ {  /* an error occurred; restore saved state and repeat */
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
TStatus luaD_pcall (lua_State *L, Pfunc func, ಶೂನ್ಯ *u, ptrdiff_t old_ತುದಿ,
                                  ptrdiff_t ef) {
  TStatus ಸ್ಥಿತಿ_2;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  ಸ್ಥಿತಿ_2 = luaD_rawrunprotected(L, func, u);
  ಆದರೆ (l_unlikely(ಸ್ಥಿತಿ_2 != LUA_OK)) {  /* an error occurred? */
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    ಸ್ಥಿತಿ_2 = luaD_closeprotected(L, old_ತುದಿ, ಸ್ಥಿತಿ_2);
    luaD_seterrorobj(L, ಸ್ಥಿತಿ_2, restorestack(L, old_ತುದಿ));
    luaD_shrinkstack(L);   /* restore stack size in case of overflow */
  }
  L->errfunc = old_errfunc;
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}



/*
** Execute a protected parser.
*/
ರಚನೆ SParser {  /* data to 'f_parser' */
  ZIO *z;
  Mbuffer buff;  /* dynamic structure used by the scanner */
  Dyndata dyd;  /* dynamic structures used by the parser */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
};


ಸ್ಥಿರ ಶೂನ್ಯ checkmode (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *x) {
  ಆದರೆ (strchr(ಬಹುಲಕ, x[0]) == ಖಾಲಿ) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, ಬಹುಲಕ);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


/*
** Before the first call to the reader function, Lua reserves a slot
** with a table for anchoring stuff.
*/
ಸ್ಥಿರ ಶೂನ್ಯ f_parser (lua_State *L, ಶೂನ್ಯ *ud) {
  LClosure *cl;
  ರಚನೆ SParser *p = cast(ರಚನೆ SParser *, ud);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = p->ಬಹುಲಕ ? p->ಬಹುಲಕ : "bt";
  ಪೂರ್ಣಾಂಕ c;
  ಕೋಷ್ಟಕ *anchor;
  ptrdiff_t otop = savestack(L, L->ತುದಿ.p);  /* original top */
  luaD_checkstack(L, 2);
  anchor = luaH_new(L);  /* create the anchor table */
  sethvalue2s(L, L->ತುದಿ.p++, anchor);  /* anchor the anchor table */
  c = zgetc(p->z);  /* read first character */
  ಆದರೆ (c == LUA_SIGNATURE[0]) {
    ಪೂರ್ಣಾಂಕ fixed = 0;
    ಆದರೆ (strchr(ಬಹುಲಕ, 'B') != ಖಾಲಿ)
      fixed = 1;
    ಇಲ್ಲದಿದ್ದರೆ
      checkmode(L, ಬಹುಲಕ, "binary");
    cl = luaU_undump(L, p->z, anchor, p->name, fixed);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    checkmode(L, ಬಹುಲಕ, "text");
    cl = luaY_parser(L, p->z, anchor, &p->buff, &p->dyd, p->name, c);
  }
  L->ತುದಿ.p = restorestack(L, otop);  /* restore stack */
  setclLvalue2s(L, L->ತುದಿ.p++, cl);  /* push closure */
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


/*
** Anchor an object in a table in the stack.  First, anchor the object
** temporarily in the stack, as luaH_set may call an emergency GC.
** Then, add it in the table with itself as its key.
*/
ಶೂನ್ಯ luaD_anchorobj (lua_State *L, ಕೋಷ್ಟಕ *anchor, GCObject *obj) {
  setgcovalue(L, s2v(L->ತುದಿ.p++), obj);  /* temporary anchor in the stack */
  lua_H_ಗಣ(L, anchor, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 1));
  /* Because this is a new key, luaH_set will call the GC barrier, so
     we don't need to call the barrier again here */
  L->ತುದಿ.p--;
}


TStatus luaD_protectedparser (lua_State *L, ZIO *z, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                                            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ರಚನೆ SParser p;
  TStatus ಸ್ಥಿತಿ_2;
  incnny(L);  /* cannot yield during parsing */
  p.z = z; p.name = name; p.ಬಹುಲಕ = ಬಹುಲಕ;
  p.dyd.actvar.arr = ಖಾಲಿ; p.dyd.actvar.ಗಾತ್ರ_2 = 0;
  p.dyd.gt.arr = ಖಾಲಿ; p.dyd.gt.ಗಾತ್ರ_2 = 0;
  p.dyd.label.arr = ಖಾಲಿ; p.dyd.label.ಗಾತ್ರ_2 = 0;
  luaZ_initbuffer(L, &p.buff);
  ಸ್ಥಿತಿ_2 = luaD_pcall(L, f_parser, &p, savestack(L, L->ತುದಿ.p), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, cast_sizet(p.dyd.actvar.ಗಾತ್ರ_2));
  luaM_freearray(L, p.dyd.gt.arr, cast_sizet(p.dyd.gt.ಗಾತ್ರ_2));
  luaM_freearray(L, p.dyd.label.arr, cast_sizet(p.dyd.label.ಗಾತ್ರ_2));
  decnny(L);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


