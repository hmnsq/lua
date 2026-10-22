/*
** $Id: lgc.h $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lgc_h
#وصف_ڪريو lgc_h


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
#وصف_ڪريو GCSpropagate	0
#وصف_ڪريو GCSenteratomic	1
#وصف_ڪريو GCSatomic	2
#وصف_ڪريو GCSswpallgc	3
#وصف_ڪريو GCSswpfinobj	4
#وصف_ڪريو GCSswptobefnz	5
#وصف_ڪريو GCSswpend	6
#وصف_ڪريو GCScallfin	7
#وصف_ڪريو GCSpause	8


#وصف_ڪريو issweepphase(g)  \
	(GCSswpallgc <= (g)->gcstate && (g)->gcstate <= GCSswpend)


/*
** macro to tell when main invariant (white objects cannot point to black
** ones) must be kept. During a collection, the sweep phase may break
** the invariant, as objects turned white may point to still-black
** objects. The invariant is restored when sweep ends and all objects
** are white again.
*/

#وصف_ڪريو keepinvariant(g)	((g)->gcstate <= GCSatomic)


/*
** some useful bit tricks
*/
#وصف_ڪريو resetbits(x,m)		((x) &= cast_byte(~(m)))
#وصف_ڪريو setbits(x,m)		((x) |= (m))
#وصف_ڪريو testbits(x,m)		((x) & (m))
#وصف_ڪريو bitmask(b)		(1<<(b))
#وصف_ڪريو bit2mask(b1,b2)		(bitmask(b1) | bitmask(b2))
#وصف_ڪريو l_setbit(x,b)		setbits(x, bitmask(b))
#وصف_ڪريو resetbit(x,b)		resetbits(x, bitmask(b))
#وصف_ڪريو testbit(x,b)		testbits(x, bitmask(b))


/*
** Layout for bit use in 'marked' field. First three bits are
** used for object "age" in generational mode. Last bit is used
** by tests.
*/
#وصف_ڪريو WHITE0_ٻٽو_انگ	3  /* object is white (type 0) */
#وصف_ڪريو WHITE1_ٻٽو_انگ	4  /* object is white (type 1) */
#وصف_ڪريو BLACKBIT	5  /* object is black */
#وصف_ڪريو FINALIZEDBIT	6  /* object has been marked for finalization */

#وصف_ڪريو TESTBIT		7



#وصف_ڪريو WHITEBITS	bit2mask(WHITE0_ٻٽو_انگ, WHITE1_ٻٽو_انگ)


#وصف_ڪريو iswhite(x)      testbits((x)->marked, WHITEBITS)
#وصف_ڪريو isblack(x)      testbit((x)->marked, BLACKBIT)
#وصف_ڪريو isgray(x)  /* neither white nor black */  \
	(!testbits((x)->marked, WHITEBITS | bitmask(BLACKBIT)))

#وصف_ڪريو tofinalize(x)	testbit((x)->marked, FINALIZEDBIT)

#وصف_ڪريو otherwhite(g)	((g)->currentwhite ^ WHITEBITS)
#وصف_ڪريو isdeadm(ow,m)	((m) & (ow))
#وصف_ڪريو isdead(g,v)	isdeadm(otherwhite(g), (v)->marked)

#وصف_ڪريو changewhite(x)	((x)->marked ^= WHITEBITS)
#وصف_ڪريو nw2black(x)  \
	check_exp(!iswhite(x), l_setbit((x)->marked, BLACKBIT))

#وصف_ڪريو luaC_white(g)	cast_byte((g)->currentwhite & WHITEBITS)


/* object age in generational mode */
#وصف_ڪريو G_NEW		0	/* created in current cycle */
#وصف_ڪريو G_SURVIVAL	1	/* created in previous cycle */
#وصف_ڪريو G_OLD0		2	/* marked old by frw. barrier in this cycle */
#وصف_ڪريو G_OLD1		3	/* first full cycle as old */
#وصف_ڪريو G_OLD		4	/* really old object (not to be visited) */
#وصف_ڪريو G_TOUCHED1	5	/* old object touched this cycle */
#وصف_ڪريو G_TOUCHED2	6	/* old object touched in previous cycle */

#وصف_ڪريو AGEBITS		7  /* all age bits (111) */

#وصف_ڪريو getage(o)	((o)->marked & AGEBITS)
#وصف_ڪريو setage(o,a)  ((o)->marked = cast_byte(((o)->marked & (~AGEBITS)) | a))
#وصف_ڪريو isold(o)	(getage(o) > G_SURVIVAL)


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
#وصف_ڪريو LUAI_MINORMAJOR         70

/*
** Major collections will shift to minor ones after a collection
** collects at least LUAI_MAJORMINOR% of the new bytes.
*/
#وصف_ڪريو LUAI_MAJORMINOR         50

/*
** A young (minor) collection will run after creating LUAI_GENMINORMUL%
** new bytes.
*/
#وصف_ڪريو LUAI_GENMINORMUL         20


/* incremental */

/* Number of bytes must be LUAI_GCPAUSE% before starting new cycle */
#وصف_ڪريو LUAI_GCPAUSE    250

/*
** Step multiplier: The collector handles LUAI_GCMUL% work units for
** each new allocated word. (Each "work unit" corresponds roughly to
** sweeping one object or traversing one slot.)
*/
#وصف_ڪريو LUAI_GCMUL      200

/* How many bytes to allocate before next GC step */
#وصف_ڪريو LUAI_GCSTEPSIZE	(200 * حجم(جدول))


#وصف_ڪريو setgcparam(g,p,v)  (g->gcparams[LUA_GCP##p] = luaO_codeparam(v))
#وصف_ڪريو applygcparam(g,p,x)  luaO_applyparam(g->gcparams[LUA_GCP##p], x)

/* }====================================================== */


/*
** Control when GC is running:
*/
#وصف_ڪريو GCSTPUSR	1  /* bit true when GC stopped by user */
#وصف_ڪريو GCSTPGC		2  /* bit true when GC stopped by itself */
#وصف_ڪريو GCSTPCLS	4  /* bit true when closing Lua state */
#وصف_ڪريو gcrunning(g)	((g)->gcstp == 0)


/*
** Does one step of collection when debt becomes zero. 'pre'/'pos'
** allows some adjustments to be done only when needed. macro
** 'condchangemem' is used only for heavy tests (forcing a full
** GC cycle on every opportunity)
*/

#إذا !defined(HARDMEMTESTS)
#وصف_ڪريو condchangemem(L,pre,pos,emg)	((فراغ)0)
#وإلا
#وصف_ڪريو condchangemem(L,pre,pos,emg)  \
	{ إذا (gcrunning(G(L))) { pre; luaC_fullgc(L, emg); pos; } }
#جيڪڏهن_پورو

#وصف_ڪريو luaC_condGC(L,pre,pos) \
	{ إذا (G(L)->GCdebt <= 0)  \
	  { pre; lua_C_قدم(L); condchangemem(L,{},{},0); pos;} \
	  وإلا condchangemem(L,pre,pos,0); }

/* more often than not, 'pre'/'pos' are empty */
#وصف_ڪريو luaC_checkGC(L)		luaC_condGC(L,(فراغ)0,(فراغ)0)


#وصف_ڪريو luaC_objbarrier(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? \
	luaC_barrier_(L,obj2gco(p),obj2gco(o)) : cast_void(0))

#وصف_ڪريو luaC_barrier(L,p,v) (  \
	iscollectable(v) ? luaC_objbarrier(L,p,gcvalue(v)) : cast_void(0))

#وصف_ڪريو luaC_objbarrierback(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? luaC_barrierback_(L,p) : cast_void(0))

#وصف_ڪريو luaC_barrierback(L,p,v) (  \
	iscollectable(v) ? luaC_objbarrierback(L, p, gcvalue(v)) : cast_void(0))

LUAI_FUNC فراغ luaC_fix (lua_State *L, GCObject *o);
LUAI_FUNC فراغ luaC_freeallobjects (lua_State *L);
LUAI_FUNC فراغ lua_C_قدم (lua_State *L);
LUAI_FUNC فراغ luaC_runtilstate (lua_State *L, صحيح state, صحيح fast);
LUAI_FUNC فراغ luaC_fullgc (lua_State *L, صحيح isemergency);
LUAI_FUNC GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz);
LUAI_FUNC GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz,
                                                 size_t وٿي);
LUAI_FUNC فراغ luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v);
LUAI_FUNC فراغ luaC_barrierback_ (lua_State *L, GCObject *o);
LUAI_FUNC فراغ luaC_checkfinalizer (lua_State *L, GCObject *o, جدول *mt);
LUAI_FUNC فراغ luaC_changemode (lua_State *L, صحيح newmode);


#جيڪڏهن_پورو
