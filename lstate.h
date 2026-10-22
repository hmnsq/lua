/*
** $Id: lstate.h $
** Global State
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lstate_h
#घोषणा lstate_h

#समावेश "lua.h"


/* Some header files included here need this definition */
प्रकार काष्ठा CallInfo CallInfo;


#समावेश "lobject.h"
#समावेश "ltm.h"
#समावेश "lzio.h"


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
#घोषणा yieldable(L)		(((L)->nCcalls & 0xffff0000) == 0)

/* real number of C calls */
#घोषणा getCcalls(L)	((L)->nCcalls & 0xffff)


/* Increment the number of non-yieldable calls */
#घोषणा incnny(L)	((L)->nCcalls += 0x10000)

/* Decrement the number of non-yieldable calls */
#घोषणा decnny(L)	((L)->nCcalls -= 0x10000)

/* Non-yieldable call increment */
#घोषणा nyci	(0x10000 | 1)




काष्ठा lua_longjmp;  /* defined in ldo.c */


/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook'
** is thread safe
*/
#अगर !defined(l_signalT)
#समावेश <signal.h>
#घोषणा l_signalT	संक_पूर्ण_प्रकार
#पूर्ण_अगर


/*
** Extra stack space to handle TM calls and some other extras. This
** space is not included in 'stack_last'. It is used only to avoid stack
** checks, either because the element will be promptly popped or because
** there will be a stack check soon after the push. Function frames
** never use this extra space, so it does not need to be kept clean.
*/
#घोषणा EXTRA_ढेर   5


/*
** Size of cache for strings in the API. 'N' is the number of
** sets (better be a prime) and "M" is the size of each set.
** (M == 1 makes a direct cache.)
*/
#अगर !defined(STRCACHE_N)
#घोषणा STRCACHE_N              53
#घोषणा STRCACHE_M              2
#पूर्ण_अगर


#घोषणा BASIC_ढेर_आकार        (2*LUA_MINSTACK)

#घोषणा stacksize(th)	cast_int((th)->ढेर_last.p - (th)->ढेर.p)


/* kinds of Garbage Collection */
#घोषणा KGC_INC		0	/* incremental gc */
#घोषणा KGC_GENMINOR	1	/* generational gc in minor (regular) mode */
#घोषणा KGC_GENMAJOR	2	/* generational in major mode */


प्रकार काष्ठा stringtable {
  TString **hash;  /* array of buckets (linked lists of strings) */
  पूर्णांक nuse;  /* number of elements */
  पूर्णांक आकार;  /* number of buckets */
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
काष्ठा CallInfo {
  StkIdRel func;  /* function index in the stack */
  StkIdRel शीर्ष;  /* top for this function */
  काष्ठा CallInfo *previous, *अग्र;  /* dynamic call link */
  जोड़ {
    काष्ठा {  /* only for Lua functions */
      स्थिर Instruction *savedpc;
      अस्थिर l_signalT trap;  /* function is tracing lines/counts */
      पूर्णांक nextraargs;  /* # of extra arguments in vararg functions */
    } l;
    काष्ठा {  /* only for C functions */
      lua_KFunction k;  /* continuation in case of yields */
      सूचक_भेद_प्रकार old_errfunc;
      lua_KContext ctx;  /* context info. in case of yields */
    } c;
  } u;
  जोड़ {
    पूर्णांक funcidx;  /* called-function index */
    पूर्णांक nyield;  /* number of values yielded */
    पूर्णांक nres;  /* number of values returned */
  } u2;
  l_uint32 callstatus;
};


/*
** Maximum expected number of results from a function
** (must fit in CIST_NRESULTS).
*/
#घोषणा MAXRESULTS	250


/*
** Bits in CallInfo status
*/
/* bits 0-7 are the expected number of results from this function + 1 */
#घोषणा CIST_NRESULTS	0xffu

/* bits 8-11 count call metamethods (and their extra arguments) */
#घोषणा CIST_CCMT	8  /* the offset, not the mask */
#घोषणा MAX_CCMT	(0xfu << CIST_CCMT)

/* Bits 12-14 are used for CIST_RECST (see below) */
#घोषणा CIST_RECST	12  /* the offset, not the mask */

/* call is running a C function (still in first 16 bits) */
#घोषणा CIST_C		(1u << (CIST_RECST + 3))
/* call is on a fresh "luaV_execute" frame */
#घोषणा CIST_FRESH	(cast(l_uint32, CIST_C) << 1)
/* function is closing tbc variables */
#घोषणा CIST_CLSRET	(CIST_FRESH << 1)
/* function has tbc variables to close */
#घोषणा CIST_TBC	(CIST_CLSRET << 1)
/* original value of 'allowhook' */
#घोषणा CIST_OAH	(CIST_TBC << 1)
/* call is running a debug hook */
#घोषणा CIST_HOOKED	(CIST_OAH << 1)
/* doing a yieldable protected call */
#घोषणा CIST_YPCALL	(CIST_HOOKED << 1)
/* call was tail called */
#घोषणा CIST_पुच्छ	(CIST_YPCALL << 1)
/* last hook called yielded */
#घोषणा CIST_HOOKYIELD	(CIST_पुच्छ << 1)
/* function "called" a finalizer */
#घोषणा CIST_FIN	(CIST_HOOKYIELD << 1)


#घोषणा get_nresults(cs)  (cast_int((cs) & CIST_NRESULTS) - 1)

/*
** Field CIST_RECST stores the "recover status", used to keep the error
** status while closing to-be-closed variables in coroutines, so that
** Lua can correctly resume after an yield from a __close method called
** because of an error.  (Three bits are enough for error status.)
*/
#घोषणा getcistrecst(ci)     (((ci)->callstatus >> CIST_RECST) & 7)
#घोषणा setcistrecst(ci,st)  \
  check_exp(((st) & 7) == (st),   /* status must fit in three bits */  \
            ((ci)->callstatus = ((ci)->callstatus & ~(7u << CIST_RECST))  \
                                | (cast(l_uint32, st) << CIST_RECST)))


/* active function is a Lua function */
#घोषणा isLua(ci)	(!((ci)->callstatus & CIST_C))

/* call is running Lua code (not a hook) */
#घोषणा isLuacode(ci)	(!((ci)->callstatus & (CIST_C | CIST_HOOKED)))


#घोषणा setoah(ci,v)  \
  ((ci)->callstatus = ((v) ? (ci)->callstatus | CIST_OAH  \
                           : (ci)->callstatus & ~CIST_OAH))
#घोषणा getoah(ci)  (((ci)->callstatus & CIST_OAH) ? 1 : 0)


/*
** 'per thread' state
*/
काष्ठा lua_State {
  CommonHeader;
  lu_byte allowhook;
  TStatus status;
  StkIdRel शीर्ष;  /* first free slot in the stack */
  काष्ठा global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  StkIdRel ढेर_last;  /* end of stack (last element + 1) */
  StkIdRel ढेर;  /* stack base */
  UpVal *openupval;  /* list of open upvalues in this stack */
  StkIdRel tbclist;  /* list of to-be-closed variables */
  GCObject *gclist;
  काष्ठा lua_State *twups;  /* list of threads with open upvalues */
  काष्ठा lua_longjmp *errorJmp;  /* current error recover point */
  CallInfo base_ci;  /* CallInfo for first level (C host) */
  अस्थिर lua_Hook hook;
  सूचक_भेद_प्रकार errfunc;  /* current error handling function (stack index) */
  l_uint32 nCcalls;  /* number of nested non-yieldable or C calls */
  पूर्णांक oldpc;  /* last pc traced */
  पूर्णांक nci;  /* number of items in 'ci' list */
  पूर्णांक basehookcount;
  पूर्णांक hookcount;
  अस्थिर l_signalT hookmask;
  काष्ठा {  /* info about transferred values (for call/return hooks) */
    पूर्णांक ftransfer;  /* offset of first value transferred */
    पूर्णांक ntransfer;  /* number of values transferred */
  } transferinfo;
};


/*
** thread state + extra space
*/
प्रकार काष्ठा LX {
  lu_byte extra_[LUA_EXTRASPACE];
  lua_State l;
} LX;


/*
** 'global state', shared by all threads of this state
*/
प्रकार काष्ठा global_State {
  lua_Alloc frealloc;  /* function to reallocate memory */
  व्योम *ud;         /* auxiliary data to 'frealloc' */
  l_mem GCtotalbytes;  /* number of bytes currently allocated + debt */
  l_mem GCdebt;  /* bytes counted but not yet allocated */
  l_mem GCmarked;  /* number of objects marked in a GC cycle */
  l_mem GCmajorminor;  /* auxiliary counter to control major-minor shifts */
  stringtable strt;  /* hash table for strings */
  TValue l_registry;
  TValue nilvalue;  /* a nil value */
  अचिन्हित पूर्णांक seed;  /* randomized seed for hashes */
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
  GCObject *gray;  /* list of gray objects */
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
  काष्ठा lua_State *twups;  /* list of threads with open upvalues */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TString *memerrmsg;  /* message for memory-allocation errors */
  TString *tmname[TM_N];  /* array with tag-method names */
  काष्ठा Table *mt[LUA_NUMTYPES];  /* metatables for basic types */
  TString *strcache[STRCACHE_N][STRCACHE_M];  /* cache for strings in API */
  lua_WarnFunction warnf;  /* warning function */
  व्योम *ud_warn;         /* auxiliary data to 'warnf' */
  LX mainth;  /* main thread of this state */
} global_State;


#घोषणा G(L)	(L->l_G)
#घोषणा mainthread(G)	(&(G)->mainth.l)

/*
** 'g->nilvalue' being a nil value flags that the state was completely
** build.
*/
#घोषणा completestate(g)	ttisnil(&g->nilvalue)


/*
** Union of all collectable objects
** ISO C99, 6.5.2.3 p.5:
** "if a union contains several structures that share a common initial
** sequence [...], and if the union object currently contains one
** of these structures, it is permitted to inspect the common initial
** part of any of them anywhere that a declaration of the complete type
** of the union is visible."
*/
जोड़ GCUnion {
  GCObject gc;  /* common header */
  काष्ठा TString ts;
  काष्ठा Udata u;
  जोड़ Closure cl;
  काष्ठा Table h;
  काष्ठा Proto p;
  काष्ठा lua_State th;  /* thread */
  काष्ठा UpVal upv;
};


/* macros to convert a GCObject into a specific value
** ISO C99, 6.3.2.2 p.7:
** "A pointer to an object or incomplete type may be converted to a
** pointer to a different object or incomplete type. If the resulting
** pointer is not correctly aligned for the pointed-to type, the
** behavior is undefined. Otherwise, when converted back again, the
** result shall compare equal to the original pointer."
*/
#घोषणा gco2(v,T,o)	check_exp((o)->tt == v, cast(T*, o))
#घोषणा gco2nv(t,T,o)	check_exp(novariant((o)->tt) == t, cast(T*, o))
#घोषणा gco2ts(o)	gco2nv(LUA_TSTRING, TString, o)
#घोषणा gco2u(o)	gco2(LUA_VUSERDATA, Udata, o)
#घोषणा gco2lcl(o)	(&gco2(LUA_VLCL, Closure, o)->l)
#घोषणा gco2ccl(o)	(&gco2(LUA_VCCL, Closure, o)->c)
#घोषणा gco2cl(o)	gco2nv(LUA_TFUNCTION, Closure, o)
#घोषणा gco2t(o)	gco2(LUA_VTABLE, Table, o)
#घोषणा gco2p(o)	gco2(LUA_VPROTO, Proto, o)
#घोषणा gco2th(o)	gco2(LUA_VTHREAD, lua_State, o)
#घोषणा gco2upv(o)	gco2(LUA_VUPVAL, UpVal, o)


/*
** macro to convert a Lua object into a GCObject
*/
#घोषणा obj2gco(v)  \
	check_exp(novariant((v)->tt) >= LUA_TSTRING, cast(GCObject*, v))


/* actual number of total memory allocated */
#घोषणा gettotalbytes(g)	((g)->GCtotalbytes - (g)->GCdebt)


LUAI_FUNC व्योम luaE_setdebt (global_State *g, l_mem debt);
LUAI_FUNC व्योम luaE_freethread (lua_State *L, lua_State *L1);
LUAI_FUNC lu_mem luaE_threadsize (lua_State *L);
LUAI_FUNC CallInfo *luaE_extendCI (lua_State *L, पूर्णांक err);
LUAI_FUNC व्योम luaE_shrinkCI (lua_State *L);
LUAI_FUNC व्योम luaE_checkcstack (lua_State *L);
LUAI_FUNC व्योम luaE_incCstack (lua_State *L);
LUAI_FUNC व्योम luaE_warning (lua_State *L, स्थिर अक्षर *संदेश, पूर्णांक tocont);
LUAI_FUNC व्योम luaE_warnerror (lua_State *L, स्थिर अक्षर *where);
LUAI_FUNC TStatus luaE_resetthread (lua_State *L, TStatus status);


#पूर्ण_अगर

