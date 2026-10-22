/*
** $Id: lgc.h $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lgc_h
#تعریف_کن lgc_h


#ضمن <stddef.h>


#ضمن "lobject.h"
#ضمن "lstate.h"

/*
** Collectable objects may have one of three colors: white, which means
** the object is not marked; gray, which means the object is marked, but
** its references may be not marked; and black, which means that the
** object and all its references are marked.  The main invariant of the
** garbage collector, while marking objects, is that a black object can
** never point to a white one. Moreover, any gray object must be in a
** "gray list" (gray, grayagain, weak, allweak, ephemeron) so that it
** can be visited again before finishing the collection cycle. (Open
** upvalues are an exception to this rule, as they are attached to
** a corresponding thread.)  These lists have no meaning when the
** invariant is not being enforced (e.g., sweep phase).
*/


/*
** Possible states of the Garbage Collector
*/
#تعریف_کن GCSpropagate	0
#تعریف_کن GCSenteratomic	1
#تعریف_کن GCSatomic	2
#تعریف_کن GCSswpallgc	3
#تعریف_کن GCSswpfinobj	4
#تعریف_کن GCSswptobefnz	5
#تعریف_کن GCSswpend	6
#تعریف_کن GCScallfin	7
#تعریف_کن GCSpause	8


#تعریف_کن issweepphase(g)  \
	(GCSswpallgc <= (g)->gcstate && (g)->gcstate <= GCSswpend)


/*
** macro to tell when main invariant (white objects cannot point to black
** ones) must be kept. During a collection, the sweep phase may break
** the invariant, as objects turned white may point to still-black
** objects. The invariant is restored when sweep ends and all objects
** are white again.
*/

#تعریف_کن keepinvariant(g)	((g)->gcstate <= GCSatomic)


/*
** some useful bit tricks
*/
#تعریف_کن resetbits(x,m)		((x) &= cast_byte(~(m)))
#تعریف_کن setbits(x,m)		((x) |= (m))
#تعریف_کن testbits(x,m)		((x) & (m))
#تعریف_کن bitmask(b)		(1<<(b))
#تعریف_کن bit2mask(b1,b2)		(bitmask(b1) | bitmask(b2))
#تعریف_کن l_setbit(x,b)		setbits(x, bitmask(b))
#تعریف_کن resetbit(x,b)		resetbits(x, bitmask(b))
#تعریف_کن testbit(x,b)		testbits(x, bitmask(b))


/*
** Layout for bit use in 'marked' field. First three bits are
** used for object "age" in generational mode. Last bit is used
** by tests.
*/
#تعریف_کن WHITE0_بیت	3  /* object is white (type 0) */
#تعریف_کن WHITE1_بیت	4  /* object is white (type 1) */
#تعریف_کن BLACKBIT	5  /* object is black */
#تعریف_کن FINALIZEDBIT	6  /* object has been marked for finalization */

#تعریف_کن TESTBIT		7



#تعریف_کن WHITEBITS	bit2mask(WHITE0_بیت, WHITE1_بیت)


#تعریف_کن iswhite(x)      testbits((x)->marked, WHITEBITS)
#تعریف_کن isblack(x)      testbit((x)->marked, BLACKBIT)
#تعریف_کن isgray(x)  /* neither white nor black */  \
	(!testbits((x)->marked, WHITEBITS | bitmask(BLACKBIT)))

#تعریف_کن tofinalize(x)	testbit((x)->marked, FINALIZEDBIT)

#تعریف_کن otherwhite(g)	((g)->currentwhite ^ WHITEBITS)
#تعریف_کن isdeadm(ow,m)	((m) & (ow))
#تعریف_کن isdead(g,v)	isdeadm(otherwhite(g), (v)->marked)

#تعریف_کن changewhite(x)	((x)->marked ^= WHITEBITS)
#تعریف_کن nw2black(x)  \
	check_exp(!iswhite(x), l_setbit((x)->marked, BLACKBIT))

#تعریف_کن luaC_white(g)	cast_byte((g)->currentwhite & WHITEBITS)


/* object age in generational mode */
#تعریف_کن G_NEW		0	/* created in current cycle */
#تعریف_کن G_SURVIVAL	1	/* created in previous cycle */
#تعریف_کن G_OLD0		2	/* marked old by frw. barrier in this cycle */
#تعریف_کن G_OLD1		3	/* first full cycle as old */
#تعریف_کن G_OLD		4	/* really old object (not to be visited) */
#تعریف_کن G_TOUCHED1	5	/* old object touched this cycle */
#تعریف_کن G_TOUCHED2	6	/* old object touched in previous cycle */

#تعریف_کن AGEBITS		7  /* all age bits (111) */

#تعریف_کن getage(o)	((o)->marked & AGEBITS)
#تعریف_کن setage(o,a)  ((o)->marked = cast_byte(((o)->marked & (~AGEBITS)) | a))
#تعریف_کن isold(o)	(getage(o) > G_SURVIVAL)


/*
** In generational mode, objects are created 'new'. After surviving one
** cycle, they become 'survival'. Both 'new' and 'survival' can point
** to any other object, as they are traversed at the end of the cycle.
** We call them both 'young' objects.
** If a survival object survives another cycle, it becomes 'old1'.
** 'old1' objects can still point to survival objects (but not to
** new objects), so they still must be traversed. After another cycle
** (that, being old, 'old1' objects will "survive" no matter what)
** finally the 'old1' object becomes really 'old', and then they
** are no more traversed.
**
** To keep its invariants, the generational mode uses the same barriers
** also used by the incremental mode. If a young object is caught in a
** forward barrier, it cannot become old immediately, because it can
** still point to other young objects. Instead, it becomes 'old0',
** which in the next cycle becomes 'old1'. So, 'old0' objects is
** old but can point to new and survival objects; 'old1' is old
** but cannot point to new objects; and 'old' cannot point to any
** young object.
**
** If any old object ('old0', 'old1', 'old') is caught in a back
** barrier, it becomes 'touched1' and goes into a gray list, to be
** visited at the end of the cycle.  There it evolves to 'touched2',
** which can point to survivals but not to new objects. In yet another
** cycle then it becomes 'old' again.
**
** The generational mode must also control the colors of objects,
** because of the barriers.  While the mutator is running, young objects
** are kept white. 'old', 'old1', and 'touched2' objects are kept black,
** as they cannot point to new objects; exceptions are threads and open
** upvalues, which age to 'old1' and 'old' but are kept gray. 'old0'
** objects may be gray or black, as in the incremental mode. 'touched1'
** objects are kept gray, as they must be visited again at the end of
** the cycle.
*/


/*
** {======================================================
** Default Values for GC parameters
** =======================================================
*/

/*
** Minor collections will shift to major ones after LUAI_MINORMAJOR%
** bytes become old.
*/
#تعریف_کن LUAI_MINORMAJOR         70

/*
** Major collections will shift to minor ones after a collection
** collects at least LUAI_MAJORMINOR% of the new bytes.
*/
#تعریف_کن LUAI_MAJORMINOR         50

/*
** A young (minor) collection will run after creating LUAI_GENMINORMUL%
** new bytes.
*/
#تعریف_کن LUAI_GENMINORMUL         20


/* incremental */

/* Number of bytes must be LUAI_GCPAUSE% before starting new cycle */
#تعریف_کن LUAI_GCPAUSE    250

/*
** Step multiplier: The collector handles LUAI_GCMUL% work units for
** each new allocated word. (Each "work unit" corresponds roughly to
** sweeping one object or traversing one slot.)
*/
#تعریف_کن LUAI_GCMUL      200

/* How many bytes to allocate before next GC step */
#تعریف_کن LUAI_GCSTEPSIZE	(200 * حجم(جدول))


#تعریف_کن setgcparam(g,p,v)  (g->gcparams[LUA_GCP##p] = luaO_codeparam(v))
#تعریف_کن applygcparam(g,p,x)  luaO_applyparam(g->gcparams[LUA_GCP##p], x)

/* }====================================================== */


/*
** Control when GC is running:
*/
#تعریف_کن GCSTPUSR	1  /* bit true when GC stopped by user */
#تعریف_کن GCSTPGC		2  /* bit true when GC stopped by itself */
#تعریف_کن GCSTPCLS	4  /* bit true when closing Lua state */
#تعریف_کن gcrunning(g)	((g)->gcstp == 0)


/*
** Does one step of collection when debt becomes zero. 'pre'/'pos'
** allows some adjustments to be done only when needed. macro
** 'condchangemem' is used only for heavy tests (forcing a full
** GC cycle on every opportunity)
*/

#إذا !defined(HARDMEMTESTS)
#تعریف_کن condchangemem(L,pre,pos,emg)	((فراغ)0)
#وإلا
#تعریف_کن condchangemem(L,pre,pos,emg)  \
	{ إذا (gcrunning(G(L))) { pre; luaC_fullgc(L, emg); pos; } }
#پایان_اگر

#تعریف_کن luaC_condGC(L,pre,pos) \
	{ إذا (G(L)->GCdebt <= 0)  \
	  { pre; lua_C_گام(L); condchangemem(L,{},{},0); pos;} \
	  وإلا condchangemem(L,pre,pos,0); }

/* more often than not, 'pre'/'pos' are empty */
#تعریف_کن luaC_checkGC(L)		luaC_condGC(L,(فراغ)0,(فراغ)0)


#تعریف_کن luaC_objbarrier(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? \
	luaC_barrier_(L,obj2gco(p),obj2gco(o)) : cast_void(0))

#تعریف_کن luaC_barrier(L,p,v) (  \
	iscollectable(v) ? luaC_objbarrier(L,p,gcvalue(v)) : cast_void(0))

#تعریف_کن luaC_objbarrierback(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? luaC_barrierback_(L,p) : cast_void(0))

#تعریف_کن luaC_barrierback(L,p,v) (  \
	iscollectable(v) ? luaC_objbarrierback(L, p, gcvalue(v)) : cast_void(0))

LUAI_FUNC فراغ luaC_fix (lua_State *L, GCObject *o);
LUAI_FUNC فراغ luaC_freeallobjects (lua_State *L);
LUAI_FUNC فراغ lua_C_گام (lua_State *L);
LUAI_FUNC فراغ luaC_runtilstate (lua_State *L, صحيح state, صحيح fast);
LUAI_FUNC فراغ luaC_fullgc (lua_State *L, صحيح isemergency);
LUAI_FUNC GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz);
LUAI_FUNC GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz,
                                                 size_t جابه_جایی);
LUAI_FUNC فراغ luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v);
LUAI_FUNC فراغ luaC_barrierback_ (lua_State *L, GCObject *o);
LUAI_FUNC فراغ luaC_checkfinalizer (lua_State *L, GCObject *o, جدول *mt);
LUAI_FUNC فراغ luaC_changemode (lua_State *L, صحيح newmode);


#پایان_اگر
