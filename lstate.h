/*
** $Id: lstate.h $
** Global State
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lstate_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lstate_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


/* Some header files included here need this definition */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CallInfo CallInfo;


#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


/*
** Some notes about garbage-collected objects: All objects in Lua must
** be kept somehow accessible until being freed, so all objects always
** belong to one (and only one) of these lists, using field 'next' of
** the 'CommonHeader' for the link:
**
** 'allgc': all objects not marked for finalization;
** 'finobj': all objects marked for finalization;
** 'tobefnz': all objects ready to be finalized;
** 'fixedgc': all objects that are not to be collected (currently
** only small strings, such as reserved words).
**
** For the generational collector, some of these lists have marks for
** generations. Each mark points to the first element in the list for
** that particular generation; that generation goes until the next mark.
**
** 'allgc' -> 'survival': new objects;
** 'survival' -> 'old': objects that survived one collection;
** 'old1' -> 'reallyold': objects that became old in last collection;
** 'reallyold' -> NULL: objects old for more than one cycle.
**
** 'finobj' -> 'finobjsur': new objects marked for finalization;
** 'finobjsur' -> 'finobjold1': survived   """";
** 'finobjold1' -> 'finobjrold': just old  """";
** 'finobjrold' -> NULL: really old       """".
**
** All lists can contain elements older than their main ages, due
** to 'luaC_checkfinalizer' and 'udata2finalize', which move
** objects between the normal lists and the "marked for finalization"
** lists. Moreover, barriers can age young objects in young lists as
** OLD0, which then become OLD1. However, a list never contains
** elements younger than their main ages.
**
** The generational collector also uses a pointer 'firstold1', which
** points to the first OLD1 object in the list. It is used to optimize
** 'markold'. (Potentially OLD1 objects can be anywhere between 'allgc'
** and 'reallyold', but often the list has no OLD1 objects or they are
** after 'old1'.) Note the difference between it and 'old1':
** 'firstold1': no OLD1 objects before this point; there can be all
**   ages after it.
** 'old1': no objects younger than OLD1 after this point.
*/

/*
** Moreover, there is another set of lists that control gray objects.
** These lists are linked by fields 'gclist'. (All objects that
** can become gray have such a field. The field is not the same
** in all objects, but it always has this name.)  Any gray object
** must belong to one of these lists, and all objects in these lists
** must be gray (with two exceptions explained below):
**
** 'gray': regular gray objects, still waiting to be visited.
** 'grayagain': objects that must be revisited at the atomic phase.
**   That includes
**   - black objects got in a write barrier;
**   - all kinds of weak tables during propagation phase;
**   - all threads.
** 'weak': tables with weak values to be cleared;
** 'ephemeron': ephemeron tables with white->white entries;
** 'allweak': tables with weak keys and/or weak values to be cleared.
**
** The exceptions to that "gray rule" are:
** - TOUCHED2 objects in generational mode stay in a gray list (because
** they must be visited again at the end of the cycle), but they are
** marked black because assignments to them must activate barriers (to
** move them back to TOUCHED1).
** - Open upvalues are kept gray to avoid barriers, but they stay out
** of gray lists. (They don't even have a 'gclist' field.)
*/



/*
** About 'nCcalls':  This count has two parts: the lower 16 bits counts
** the number of recursive invocations in the C stack; the higher
** 16 bits counts the number of non-yieldable calls in the stack.
** (They are together so that we can change and save both with one
** instruction.)
*/


/* true if this thread does not have non-yieldable calls in the stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ yieldable(L)		(((L)->nCcalls & 0xffff0000) == 0)

/* real number of C calls */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getCcalls(L)	((L)->nCcalls & 0xffff)


/* Increment the number of non-yieldable calls */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ incnny(L)	((L)->nCcalls += 0x10000)

/* Decrement the number of non-yieldable calls */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ decnny(L)	((L)->nCcalls -= 0x10000)

/* Non-yieldable call increment */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nyci	(0x10000 | 1)




ਬਣਤਰ lua_longjmp;  /* defined in ldo.c */


/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook'
** is thread safe
*/
#ਜੇ !defined(l_signalT)
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_signalT	sig_atomic_t
#ਜੇ_ਅੰਤ


/*
** Extra stack space to handle TM calls and some other extras. This
** space is not included in 'stack_last'. It is used only to avoid stack
** checks, either because the element will be promptly popped or because
** there will be a stack check soon after the push. Function frames
** never use this extra space, so it does not need to be kept clean.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTRA_ਢੇਰ   5


/*
** Size of cache for strings in the API. 'N' is the number of
** sets (better be a prime) and "M" is the size of each set.
** (M == 1 makes a direct cache.)
*/
#ਜੇ !defined(STRCACHE_N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRCACHE_N              53
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRCACHE_M              2
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASIC_ਢੇਰ_ਮਾਪ        (2*LUA_MINSTACK)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ stacksize(th)	cast_int((th)->ਢੇਰ_ਆਖਰੀ.p - (th)->ਢੇਰ.p)


/* kinds of Garbage Collection */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KGC_INC		0	/* incremental gc */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KGC_GENMINOR	1	/* generational gc in minor (regular) mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KGC_GENMAJOR	2	/* generational in major mode */


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ stringtable {
  TString **hash;  /* array of buckets (linked lists of strings) */
  ਪੂਰਨ nuse;  /* number of elements */
  ਪੂਰਨ ਮਾਪ;  /* number of buckets */
} stringtable;


/*
** Information about a call.
** About union 'u':
** - field 'l' is used only for Lua functions;
** - field 'c' is used only for C functions.
** About union 'u2':
** - field 'funcidx' is used only by C functions while doing a
** protected call;
** - field 'nyield' is used only while a function is "doing" an
** yield (from the yield until the next resume);
** - field 'nres' is used only while closing tbc variables when
** returning from a function;
*/
ਬਣਤਰ CallInfo {
  StkIdRel func;  /* function index in the stack */
  StkIdRel ਸਿਖਰ;  /* top for this function */
  ਬਣਤਰ CallInfo *ਪਿਛਲਾ, *ਅਗਲਾ;  /* dynamic call link */
  ਸੰਘ {
    ਬਣਤਰ {  /* only for Lua functions */
      ਅਚਲ Instruction *savedpc;
      ਅਸਥਿਰ l_signalT trap;  /* function is tracing lines/counts */
      ਪੂਰਨ nextraargs;  /* # of extra arguments in vararg functions */
    } l;
    ਬਣਤਰ {  /* only for C functions */
      lua_KFunction k;  /* continuation in case of yields */
      ptrdiff_t old_errfunc;
      lua_KContext ctx;  /* context info. in case of yields */
    } c;
  } u;
  ਸੰਘ {
    ਪੂਰਨ funcidx;  /* called-function index */
    ਪੂਰਨ nyield;  /* number of values yielded */
    ਪੂਰਨ nres;  /* number of values returned */
  } u2;
  l_uint32 callstatus;
};


/*
** Maximum expected number of results from a function
** (must fit in CIST_NRESULTS).
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXRESULTS	250


/*
** Bits in CallInfo status
*/
/* bits 0-7 are the expected number of results from this function + 1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_NRESULTS	0xffu

/* bits 8-11 count call metamethods (and their extra arguments) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_CCMT	8  /* the offset, not the mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_CCMT	(0xfu << CIST_CCMT)

/* Bits 12-14 are used for CIST_RECST (see below) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_RECST	12  /* the offset, not the mask */

/* call is running a C function (still in first 16 bits) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_C		(1u << (CIST_RECST + 3))
/* call is on a fresh "luaV_execute" frame */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_FRESH	(cast(l_uint32, CIST_C) << 1)
/* function is closing tbc variables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_CLSRET	(CIST_FRESH << 1)
/* function has tbc variables to close */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_TBC	(CIST_CLSRET << 1)
/* original value of 'allowhook' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_OAH	(CIST_TBC << 1)
/* call is running a debug hook */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_HOOKED	(CIST_OAH << 1)
/* doing a yieldable protected call */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_YPCALL	(CIST_HOOKED << 1)
/* call was tail called */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_ਪੂਛ	(CIST_YPCALL << 1)
/* last hook called yielded */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_HOOKYIELD	(CIST_ਪੂਛ << 1)
/* function "called" a finalizer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIST_FIN	(CIST_HOOKYIELD << 1)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_nresults(cs)  (cast_int((cs) & CIST_NRESULTS) - 1)

/*
** Field CIST_RECST stores the "recover status", used to keep the error
** status while closing to-be-closed variables in coroutines, so that
** Lua can correctly resume after an yield from a __close method called
** because of an error.  (Three bits are enough for error status.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getcistrecst(ci)     (((ci)->callstatus >> CIST_RECST) & 7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setcistrecst(ci,st)  \
  check_exp(((st) & 7) == (st),   /* status must fit in three bits */  \
            ((ci)->callstatus = ((ci)->callstatus & ~(7u << CIST_RECST))  \
                                | (cast(l_uint32, st) << CIST_RECST)))


/* active function is a Lua function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isLua(ci)	(!((ci)->callstatus & CIST_C))

/* call is running Lua code (not a hook) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isLuacode(ci)	(!((ci)->callstatus & (CIST_C | CIST_HOOKED)))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setoah(ci,v)  \
  ((ci)->callstatus = ((v) ? (ci)->callstatus | CIST_OAH  \
                           : (ci)->callstatus & ~CIST_OAH))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getoah(ci)  (((ci)->callstatus & CIST_OAH) ? 1 : 0)


/*
** 'per thread' state
*/
ਬਣਤਰ lua_State {
  Common_ਸਿਰਲੇਖ;
  lu_byte allowhook;
  TStatus ਸਥਿਤੀ;
  StkIdRel ਸਿਖਰ;  /* first free slot in the stack */
  ਬਣਤਰ global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  StkIdRel ਢੇਰ_ਆਖਰੀ;  /* end of stack (last element + 1) */
  StkIdRel ਢੇਰ;  /* stack base */
  ਉੱਪਰ_Val *openupval;  /* list of open upvalues in this stack */
  StkIdRel tbclist;  /* list of to-be-closed variables */
  GCObject *gclist;
  ਬਣਤਰ lua_State *twups;  /* list of threads with open upvalues */
  ਬਣਤਰ lua_longjmp *errorJmp;  /* current error recover point */
  CallInfo ਅਧਾਰ_ci;  /* CallInfo for first level (C host) */
  ਅਸਥਿਰ lua_Hook hook;
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  l_uint32 nCcalls;  /* number of nested non-yieldable or C calls */
  ਪੂਰਨ oldpc;  /* last pc traced */
  ਪੂਰਨ nci;  /* number of items in 'ci' list */
  ਪੂਰਨ basehookcount;
  ਪੂਰਨ hookcount;
  ਅਸਥਿਰ l_signalT hookmask;
  ਬਣਤਰ {  /* info about transferred values (for call/return hooks) */
    ਪੂਰਨ ftransfer;  /* offset of first value transferred */
    ਪੂਰਨ ntransfer;  /* number of values transferred */
  } transferinfo;
};


/*
** thread state + extra space
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ LX {
  lu_byte extra_[LUA_EXTRASPACE];
  lua_State l;
} LX;


/*
** 'global state', shared by all threads of this state
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ global_State {
  lua_Alloc frealloc;  /* function to reallocate memory */
  ਖਾਲੀ *ud;         /* auxiliary data to 'frealloc' */
  l_mem GCtotalbytes;  /* number of bytes currently allocated + debt */
  l_mem GCdebt;  /* bytes counted but not yet allocated */
  l_mem GCmarked;  /* number of objects marked in a GC cycle */
  l_mem GCmajorminor;  /* auxiliary counter to control major-minor shifts */
  stringtable strt;  /* hash table for strings */
  TValue l_registry;
  TValue nilvalue;  /* a nil value */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬੀਜ;  /* randomized seed for hashes */
  lu_byte gcparams[LUA_GCPN];
  lu_byte currentwhite;
  lu_byte gcstate;  /* state of garbage collector */
  lu_byte gckind;  /* kind of GC running */
  lu_byte gcstopem;  /* stops emergency collections */
  lu_byte gcstp;  /* control whether GC is running */
  lu_byte gcemergency;  /* true if this is an emergency collection */
  GCObject *allgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* current position of sweep in list */
  GCObject *finobj;  /* list of collectable objects with finalizers */
  GCObject *ਸਲੇਟੀ;  /* list of gray objects */
  GCObject *grayagain;  /* list of objects to be traversed atomically */
  GCObject *weak;  /* list of tables with weak values */
  GCObject *ephemeron;  /* list of ephemeron tables (weak keys) */
  GCObject *allweak;  /* list of all-weak tables */
  GCObject *tobefnz;  /* list of userdata to be GC */
  GCObject *fixedgc;  /* list of objects not to be collected */
  /* fields for generational collector */
  GCObject *survival;  /* start of objects that survived one GC cycle */
  GCObject *old1;  /* start of old1 objects */
  GCObject *reallyold;  /* objects more than one cycle old ("really old") */
  GCObject *firstold1;  /* first OLD1 object in the list (if any) */
  GCObject *finobjsur;  /* list of survival objects with finalizers */
  GCObject *finobjold1;  /* list of old1 objects with finalizers */
  GCObject *finobjrold;  /* list of really old objects with finalizers */
  ਬਣਤਰ lua_State *twups;  /* list of threads with open upvalues */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TString *memerrmsg;  /* message for memory-allocation errors */
  TString *tmname[TM_N];  /* array with tag-method names */
  ਬਣਤਰ ਸਾਰਣੀ *mt[LUA_NUMTYPES];  /* metatables for basic types */
  TString *strcache[STRCACHE_N][STRCACHE_M];  /* cache for strings in API */
  lua_WarnFunction warnf;  /* warning function */
  ਖਾਲੀ *ud_warn;         /* auxiliary data to 'warnf' */
  LX mainth;  /* main thread of this state */
} global_State;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G(L)	(L->l_G)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mainthread(G)	(&(G)->mainth.l)

/*
** 'g->nilvalue' being a nil value flags that the state was completely
** build.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ completestate(g)	ttisnil(&g->nilvalue)


/*
** Union of all collectable objects
** ISO C99, 6.5.2.3 p.5:
** "if a union contains several structures that share a common initial
** sequence [...], and if the union object currently contains one
** of these structures, it is permitted to inspect the common initial
** part of any of them anywhere that a declaration of the complete type
** of the union is visible."
*/
ਸੰਘ GCUnion {
  GCObject gc;  /* common header */
  ਬਣਤਰ TString ts;
  ਬਣਤਰ Udata u;
  ਸੰਘ ਘੇਰਾ cl;
  ਬਣਤਰ ਸਾਰਣੀ h;
  ਬਣਤਰ Proto p;
  ਬਣਤਰ lua_State th;  /* thread */
  ਬਣਤਰ ਉੱਪਰ_Val upv;
};


/* macros to convert a GCObject into a specific value
** ISO C99, 6.3.2.2 p.7:
** "A pointer to an object or incomplete type may be converted to a
** pointer to a different object or incomplete type. If the resulting
** pointer is not correctly aligned for the pointed-to type, the
** behavior is undefined. Otherwise, when converted back again, the
** result shall compare equal to the original pointer."
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2(v,T,o)	check_exp((o)->tt == v, cast(T*, o))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2nv(t,T,o)	check_exp(novariant((o)->tt) == t, cast(T*, o))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2ts(o)	gco2nv(LUA_TSTRING, TString, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2u(o)	gco2(LUA_VUSERDATA, Udata, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2lcl(o)	(&gco2(LUA_VLCL, ਘੇਰਾ, o)->l)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2ccl(o)	(&gco2(LUA_VCCL, ਘੇਰਾ, o)->c)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2cl(o)	gco2nv(LUA_TFUNCTION, ਘੇਰਾ, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2t(o)	gco2(LUA_VTABLE, ਸਾਰਣੀ, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2p(o)	gco2(LUA_VPROTO, Proto, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2th(o)	gco2(LUA_VTHREAD, lua_State, o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gco2upv(o)	gco2(LUA_VUPVAL, ਉੱਪਰ_Val, o)


/*
** macro to convert a Lua object into a GCObject
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ obj2gco(v)  \
	check_exp(novariant((v)->tt) >= LUA_TSTRING, cast(GCObject*, v))


/* actual number of total memory allocated */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gettotalbytes(g)	((g)->GCtotalbytes - (g)->GCdebt)


LUAI_FUNC ਖਾਲੀ luaE_setdebt (global_State *g, l_mem debt);
LUAI_FUNC ਖਾਲੀ luaE_freethread (lua_State *L, lua_State *L1);
LUAI_FUNC lu_mem luaE_threadsize (lua_State *L);
LUAI_FUNC CallInfo *luaE_extendCI (lua_State *L, ਪੂਰਨ err);
LUAI_FUNC ਖਾਲੀ luaE_shrinkCI (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaE_checkcstack (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaE_incCstack (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaE_warning (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont);
LUAI_FUNC ਖਾਲੀ luaE_warnerror (lua_State *L, ਅਚਲ ਅੱਖਰ *where);
LUAI_FUNC TStatus luaE_resetthread (lua_State *L, TStatus ਸਥਿਤੀ);


#ਜੇ_ਅੰਤ

