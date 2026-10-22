/*
** $Id: lstate.h $
** Global State
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف lstate_h
#تعریف_کرو lstate_h

#ضمن "lua.h"


/* Some header files included here need this definition */
تعريفنوع بنية CallInfo CallInfo;


#ضمن "lobject.h"
#ضمن "ltm.h"
#ضمن "lzio.h"


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
#تعریف_کرو yieldable(L)		(((L)->nCcalls & 0xffff0000) == 0)

/* real number of C calls */
#تعریف_کرو getCcalls(L)	((L)->nCcalls & 0xffff)


/* Increment the number of non-yieldable calls */
#تعریف_کرو incnny(L)	((L)->nCcalls += 0x10000)

/* Decrement the number of non-yieldable calls */
#تعریف_کرو decnny(L)	((L)->nCcalls -= 0x10000)

/* Non-yieldable call increment */
#تعریف_کرو nyci	(0x10000 | 1)




بنية lua_longjmp;  /* defined in ldo.c */


/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook'
** is thread safe
*/
#إذا !defined(l_signalT)
#ضمن <signal.h>
#تعریف_کرو l_signalT	sig_atomic_t
#جے_انت


/*
** Extra stack space to handle TM calls and some other extras. This
** space is not included in 'stack_last'. It is used only to avoid stack
** checks, either because the element will be promptly popped or because
** there will be a stack check soon after the push. Function frames
** never use this extra space, so it does not need to be kept clean.
*/
#تعریف_کرو EXTRA_ڈھیر   5


/*
** Size of cache for strings in the API. 'N' is the number of
** sets (better be a prime) and "M" is the size of each set.
** (M == 1 makes a direct cache.)
*/
#إذا !defined(STRCACHE_N)
#تعریف_کرو STRCACHE_N              53
#تعریف_کرو STRCACHE_M              2
#جے_انت


#تعریف_کرو BASIC_ڈھیر_ناپ        (2*LUA_MINSTACK)

#تعریف_کرو stacksize(th)	cast_int((th)->ڈھیر_آخری.p - (th)->ڈھیر.p)


/* kinds of Garbage Collection */
#تعریف_کرو KGC_INC		0	/* incremental gc */
#تعریف_کرو KGC_GENMINOR	1	/* generational gc in minor (regular) mode */
#تعریف_کرو KGC_GENMAJOR	2	/* generational in major mode */


تعريفنوع بنية stringtable {
  TString **hash;  /* array of buckets (linked lists of strings) */
  صحيح nuse;  /* number of elements */
  صحيح ناپ;  /* number of buckets */
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
بنية CallInfo {
  StkIdRel func;  /* function index in the stack */
  StkIdRel چوٹی;  /* top for this function */
  بنية CallInfo *پچھلا, *اگلا;  /* dynamic call link */
  اتحاد {
    بنية {  /* only for Lua functions */
      ثابت Instruction *savedpc;
      متطاير l_signalT trap;  /* function is tracing lines/counts */
      صحيح nextraargs;  /* # of extra arguments in vararg functions */
    } l;
    بنية {  /* only for C functions */
      lua_KFunction k;  /* continuation in case of yields */
      ptrdiff_t old_errfunc;
      lua_KContext ctx;  /* context info. in case of yields */
    } c;
  } u;
  اتحاد {
    صحيح funcidx;  /* called-function index */
    صحيح nyield;  /* number of values yielded */
    صحيح nres;  /* number of values returned */
  } u2;
  l_uint32 callstatus;
};


/*
** Maximum expected number of results from a function
** (must fit in CIST_NRESULTS).
*/
#تعریف_کرو MAXRESULTS	250


/*
** Bits in CallInfo status
*/
/* bits 0-7 are the expected number of results from this function + 1 */
#تعریف_کرو CIST_NRESULTS	0xffu

/* bits 8-11 count call metamethods (and their extra arguments) */
#تعریف_کرو CIST_CCMT	8  /* the offset, not the mask */
#تعریف_کرو MAX_CCMT	(0xfu << CIST_CCMT)

/* Bits 12-14 are used for CIST_RECST (see below) */
#تعریف_کرو CIST_RECST	12  /* the offset, not the mask */

/* call is running a C function (still in first 16 bits) */
#تعریف_کرو CIST_C		(1u << (CIST_RECST + 3))
/* call is on a fresh "luaV_execute" frame */
#تعریف_کرو CIST_FRESH	(cast(l_uint32, CIST_C) << 1)
/* function is closing tbc variables */
#تعریف_کرو CIST_CLSRET	(CIST_FRESH << 1)
/* function has tbc variables to close */
#تعریف_کرو CIST_TBC	(CIST_CLSRET << 1)
/* original value of 'allowhook' */
#تعریف_کرو CIST_OAH	(CIST_TBC << 1)
/* call is running a debug hook */
#تعریف_کرو CIST_HOOKED	(CIST_OAH << 1)
/* doing a yieldable protected call */
#تعریف_کرو CIST_YPCALL	(CIST_HOOKED << 1)
/* call was tail called */
#تعریف_کرو CIST_پوچھ	(CIST_YPCALL << 1)
/* last hook called yielded */
#تعریف_کرو CIST_HOOKYIELD	(CIST_پوچھ << 1)
/* function "called" a finalizer */
#تعریف_کرو CIST_FIN	(CIST_HOOKYIELD << 1)


#تعریف_کرو get_nresults(cs)  (cast_int((cs) & CIST_NRESULTS) - 1)

/*
** Field CIST_RECST stores the "recover status", used to keep the error
** status while closing to-be-closed variables in coroutines, so that
** Lua can correctly resume after an yield from a __close method called
** because of an error.  (Three bits are enough for error status.)
*/
#تعریف_کرو getcistrecst(ci)     (((ci)->callstatus >> CIST_RECST) & 7)
#تعریف_کرو setcistrecst(ci,st)  \
  check_exp(((st) & 7) == (st),   /* status must fit in three bits */  \
            ((ci)->callstatus = ((ci)->callstatus & ~(7u << CIST_RECST))  \
                                | (cast(l_uint32, st) << CIST_RECST)))


/* active function is a Lua function */
#تعریف_کرو isLua(ci)	(!((ci)->callstatus & CIST_C))

/* call is running Lua code (not a hook) */
#تعریف_کرو isLuacode(ci)	(!((ci)->callstatus & (CIST_C | CIST_HOOKED)))


#تعریف_کرو setoah(ci,v)  \
  ((ci)->callstatus = ((v) ? (ci)->callstatus | CIST_OAH  \
                           : (ci)->callstatus & ~CIST_OAH))
#تعریف_کرو getoah(ci)  (((ci)->callstatus & CIST_OAH) ? 1 : 0)


/*
** 'per thread' state
*/
بنية lua_State {
  Common_سرخی;
  lu_byte allowhook;
  TStatus حالت;
  StkIdRel چوٹی;  /* first free slot in the stack */
  بنية global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  StkIdRel ڈھیر_آخری;  /* end of stack (last element + 1) */
  StkIdRel ڈھیر;  /* stack base */
  اتے_Val *openupval;  /* list of open upvalues in this stack */
  StkIdRel tbclist;  /* list of to-be-closed variables */
  GCObject *gclist;
  بنية lua_State *twups;  /* list of threads with open upvalues */
  بنية lua_longjmp *errorJmp;  /* current error recover point */
  CallInfo ادھار_ci;  /* CallInfo for first level (C host) */
  متطاير lua_Hook hook;
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  l_uint32 nCcalls;  /* number of nested non-yieldable or C calls */
  صحيح oldpc;  /* last pc traced */
  صحيح nci;  /* number of items in 'ci' list */
  صحيح basehookcount;
  صحيح hookcount;
  متطاير l_signalT hookmask;
  بنية {  /* info about transferred values (for call/return hooks) */
    صحيح ftransfer;  /* offset of first value transferred */
    صحيح ntransfer;  /* number of values transferred */
  } transferinfo;
};


/*
** thread state + extra space
*/
تعريفنوع بنية LX {
  lu_byte extra_[LUA_EXTRASPACE];
  lua_State l;
} LX;


/*
** 'global state', shared by all threads of this state
*/
تعريفنوع بنية global_State {
  lua_Alloc frealloc;  /* function to reallocate memory */
  فراغ *ud;         /* auxiliary data to 'frealloc' */
  l_mem GCtotalbytes;  /* number of bytes currently allocated + debt */
  l_mem GCdebt;  /* bytes counted but not yet allocated */
  l_mem GCmarked;  /* number of objects marked in a GC cycle */
  l_mem GCmajorminor;  /* auxiliary counter to control major-minor shifts */
  stringtable strt;  /* hash table for strings */
  TValue l_registry;
  TValue nilvalue;  /* a nil value */
  بنانشانی صحيح بیج;  /* randomized seed for hashes */
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
  GCObject *سلیٹی;  /* list of gray objects */
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
  بنية lua_State *twups;  /* list of threads with open upvalues */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TString *memerrmsg;  /* message for memory-allocation errors */
  TString *tmname[TM_N];  /* array with tag-method names */
  بنية جدول *mt[LUA_NUMTYPES];  /* metatables for basic types */
  TString *strcache[STRCACHE_N][STRCACHE_M];  /* cache for strings in API */
  lua_WarnFunction warnf;  /* warning function */
  فراغ *ud_warn;         /* auxiliary data to 'warnf' */
  LX mainth;  /* main thread of this state */
} global_State;


#تعریف_کرو G(L)	(L->l_G)
#تعریف_کرو mainthread(G)	(&(G)->mainth.l)

/*
** 'g->nilvalue' being a nil value flags that the state was completely
** build.
*/
#تعریف_کرو completestate(g)	ttisnil(&g->nilvalue)


/*
** Union of all collectable objects
** ISO C99, 6.5.2.3 p.5:
** "if a union contains several structures that share a common initial
** sequence [...], and if the union object currently contains one
** of these structures, it is permitted to inspect the common initial
** part of any of them anywhere that a declaration of the complete type
** of the union is visible."
*/
اتحاد GCUnion {
  GCObject gc;  /* common header */
  بنية TString ts;
  بنية Udata u;
  اتحاد گھیرا cl;
  بنية جدول h;
  بنية Proto p;
  بنية lua_State th;  /* thread */
  بنية اتے_Val upv;
};


/* macros to convert a GCObject into a specific value
** ISO C99, 6.3.2.2 p.7:
** "A pointer to an object or incomplete type may be converted to a
** pointer to a different object or incomplete type. If the resulting
** pointer is not correctly aligned for the pointed-to type, the
** behavior is undefined. Otherwise, when converted back again, the
** result shall compare equal to the original pointer."
*/
#تعریف_کرو gco2(v,T,o)	check_exp((o)->tt == v, cast(T*, o))
#تعریف_کرو gco2nv(t,T,o)	check_exp(novariant((o)->tt) == t, cast(T*, o))
#تعریف_کرو gco2ts(o)	gco2nv(LUA_TSTRING, TString, o)
#تعریف_کرو gco2u(o)	gco2(LUA_VUSERDATA, Udata, o)
#تعریف_کرو gco2lcl(o)	(&gco2(LUA_VLCL, گھیرا, o)->l)
#تعریف_کرو gco2ccl(o)	(&gco2(LUA_VCCL, گھیرا, o)->c)
#تعریف_کرو gco2cl(o)	gco2nv(LUA_TFUNCTION, گھیرا, o)
#تعریف_کرو gco2t(o)	gco2(LUA_VTABLE, جدول, o)
#تعریف_کرو gco2p(o)	gco2(LUA_VPROTO, Proto, o)
#تعریف_کرو gco2th(o)	gco2(LUA_VTHREAD, lua_State, o)
#تعریف_کرو gco2upv(o)	gco2(LUA_VUPVAL, اتے_Val, o)


/*
** macro to convert a Lua object into a GCObject
*/
#تعریف_کرو obj2gco(v)  \
	check_exp(novariant((v)->tt) >= LUA_TSTRING, cast(GCObject*, v))


/* actual number of total memory allocated */
#تعریف_کرو gettotalbytes(g)	((g)->GCtotalbytes - (g)->GCdebt)


LUAI_FUNC فراغ luaE_setdebt (global_State *g, l_mem debt);
LUAI_FUNC فراغ luaE_freethread (lua_State *L, lua_State *L1);
LUAI_FUNC lu_mem luaE_threadsize (lua_State *L);
LUAI_FUNC CallInfo *luaE_extendCI (lua_State *L, صحيح err);
LUAI_FUNC فراغ luaE_shrinkCI (lua_State *L);
LUAI_FUNC فراغ luaE_checkcstack (lua_State *L);
LUAI_FUNC فراغ luaE_incCstack (lua_State *L);
LUAI_FUNC فراغ luaE_warning (lua_State *L, ثابت حرف *پیغام, صحيح tocont);
LUAI_FUNC فراغ luaE_warnerror (lua_State *L, ثابت حرف *where);
LUAI_FUNC TStatus luaE_resetthread (lua_State *L, TStatus حالت);


#جے_انت

