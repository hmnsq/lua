/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#جيڪڏهن_وصف_نه_ٿيل lobject_h
#وصف_ڪريو lobject_h


#ضمن <stdarg.h>


#ضمن "llimits.h"
#ضمن "lua.h"


/*
** Extra types for collectable non-values
*/
#وصف_ڪريو LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#وصف_ڪريو LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#وصف_ڪريو LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#وصف_ڪريو LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#وصف_ڪريو makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
تعريفنوع اتحاد قدر_2 {
  بنية GCObject *gc;    /* collectable objects */
  فراغ *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_عدد n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} قدر_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#وصف_ڪريو TValuefields	قدر_2 قدر_3; lu_byte tt_

تعريفنوع بنية TValue {
  TValuefields;
} TValue;


#وصف_ڪريو val_(o)		((o)->قدر_3)
#وصف_ڪريو valraw(o)	(val_(o))


/* raw type tag of a TValue */
#وصف_ڪريو rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#وصف_ڪريو novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#وصف_ڪريو withvariant(t)	((t) & 0x3F)
#وصف_ڪريو ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#وصف_ڪريو ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#وصف_ڪريو checktag(o,t)		(rawtt(o) == (t))
#وصف_ڪريو checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#وصف_ڪريو righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#وصف_ڪريو checkliveness(L,obj) \
	((فراغ)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == عدم || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#وصف_ڪريو settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#وصف_ڪريو setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ثابت TValue *io2=(obj2); \
          io1->قدر_3 = io2->قدر_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#وصف_ڪريو setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#وصف_ڪريو setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#وصف_ڪريو setobjt2t	setobj
/* to new object */
#وصف_ڪريو setobj2n	setobj
/* to table */
#وصف_ڪريو setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
تعريفنوع اتحاد ڍير_قدر {
  TValue val;
  بنية {
    TValuefields;
    بنانشان ننڍو delta;
  } tbclist;
} ڍير_قدر;


/* index to stack elements */
تعريفنوع ڍير_قدر *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
تعريفنوع اتحاد {
  StkId p;  /* actual pointer */
  ptrdiff_t وٿي;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#وصف_ڪريو s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#وصف_ڪريو LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#وصف_ڪريو LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#وصف_ڪريو LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#وصف_ڪريو LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#وصف_ڪريو ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#وصف_ڪريو tagisempty(ٽيگ)		(novariant(ٽيگ) == LUA_TNIL)


/* macro to test for a standard nil */
#وصف_ڪريو ttisstrictnil(o)	checktag((o), LUA_VNIL)


#وصف_ڪريو setnilvalue(obj)	settt_(obj, LUA_VNIL)
#وصف_ڪريو setnilvalue2s(stk)	setnilvalue(s2v(stk))


#وصف_ڪريو isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#وصف_ڪريو isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#وصف_ڪريو isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#وصف_ڪريو ABSTKEYCONSTANT		{عدم}, LUA_VABSTKEY


/* mark an entry as empty */
#وصف_ڪريو setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#وصف_ڪريو LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#وصف_ڪريو LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#وصف_ڪريو ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#وصف_ڪريو ttisfalse(o)		checktag((o), LUA_VFALSE)
#وصف_ڪريو ttistrue(o)		checktag((o), LUA_VTRUE)


#وصف_ڪريو l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#وصف_ڪريو tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#وصف_ڪريو setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#وصف_ڪريو setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#وصف_ڪريو LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#وصف_ڪريو ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#وصف_ڪريو thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#وصف_ڪريو setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#وصف_ڪريو setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

/* }================================================================== */


/*
** {==================================================================
** Collectable Objects
** ===================================================================
*/

/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#وصف_ڪريو Common_سرو	بنية GCObject *ايندڙ; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
تعريفنوع بنية GCObject {
  Common_سرو;
} GCObject;


/* Bit mark for collectable types */
#وصف_ڪريو ٻٽو_انگ_ISCOLLECTABLE	(1 << 6)

#وصف_ڪريو iscollectable(o)	(rawtt(o) & ٻٽو_انگ_ISCOLLECTABLE)

/* mark a tag as collectable */
#وصف_ڪريو ctb(t)			((t) | ٻٽو_انگ_ISCOLLECTABLE)

#وصف_ڪريو gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#وصف_ڪريو gcvalueraw(v)	((v).gc)

#وصف_ڪريو setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#وصف_ڪريو LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#وصف_ڪريو LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#وصف_ڪريو ttisnumber(o)		checktype((o), LUA_TNUMBER)
#وصف_ڪريو ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#وصف_ڪريو ttisinteger(o)		checktag((o), LUA_VNUMINT)

#وصف_ڪريو nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_انگ(ivalue(o)) : fltvalue(o)))
#وصف_ڪريو fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#وصف_ڪريو ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#وصف_ڪريو fltvalueraw(v)	((v).n)
#وصف_ڪريو ivalueraw(v)	((v).i)

#وصف_ڪريو setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#وصف_ڪريو chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#وصف_ڪريو setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#وصف_ڪريو chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#وصف_ڪريو LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#وصف_ڪريو LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#وصف_ڪريو ttisstring(o)		checktype((o), LUA_TSTRING)
#وصف_ڪريو ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#وصف_ڪريو ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#وصف_ڪريو tsvalueraw(v)	(gco2ts((v).gc))

#وصف_ڪريو tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#وصف_ڪريو setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#وصف_ڪريو setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#وصف_ڪريو setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#وصف_ڪريو LSTRREG		-1  /* regular long string */
#وصف_ڪريو LSTRFIX		-2  /* fixed external long string */
#وصف_ڪريو LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
تعريفنوع بنية TString {
  Common_سرو;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  بنانشان صحيح hash;
  اتحاد {
    size_t lnglen;  /* length for long strings */
    بنية TString *hnext;  /* linked list for hash table */
  } u;
  حرف *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  فراغ *ud;  /* user data for external strings */
} TString;


#وصف_ڪريو strisshr(ts)	((ts)->shrlen >= 0)
#وصف_ڪريو isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#وصف_ڪريو rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#وصف_ڪريو getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#وصف_ڪريو getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#وصف_ڪريو getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#وصف_ڪريو tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#وصف_ڪريو getlstr(ts, len)  \
	(strisshr(ts) \
	? (cast_void((len) = cast_sizet((ts)->shrlen)), rawgetshrstr(ts)) \
	: (cast_void((len) = (ts)->u.lnglen), (ts)->contents))

/* }================================================================== */


/*
** {==================================================================
** Userdata
** ===================================================================
*/


/*
** Light userdata should be a variant of userdata, but for compatibility
** reasons they are also different types.
*/
#وصف_ڪريو LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#وصف_ڪريو LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#وصف_ڪريو ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#وصف_ڪريو ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#وصف_ڪريو pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#وصف_ڪريو uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#وصف_ڪريو pvalueraw(v)	((v).p)

#وصف_ڪريو setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#وصف_ڪريو setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
تعريفنوع اتحاد UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
تعريفنوع بنية Udata {
  Common_سرو;
  بنانشان ننڍو nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  بنية جدول *metatable;
  GCObject *gclist;
  UValue uv[1];  /* user values */
} Udata;


/*
** Header for userdata with no user values. These userdata do not need
** to be gray during GC, and therefore do not need a 'gclist' field.
** To simplify, the code always use 'Udata' for both kinds of userdata,
** making sure it never accesses 'gclist' on userdata with no user values.
** This structure here is used only to compute the correct size for
** this representation. (The 'bindata' field in its end ensures correct
** alignment for binary data following this header.)
*/
تعريفنوع بنية Udata0 {
  Common_سرو;
  بنانشان ننڍو nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  بنية جدول *metatable;
  اتحاد {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#وصف_ڪريو udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (حجم(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#وصف_ڪريو getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#وصف_ڪريو sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#وصف_ڪريو LUA_VPROTO	makevariant(LUA_TPROTO, 0)


تعريفنوع l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
تعريفنوع بنية Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte قسم;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
تعريفنوع بنية LocVar {
  TString *varname;
  صحيح startpc;  /* first point where variable is active */
  صحيح endpc;    /* first point where variable is dead */
} LocVar;


/*
** Associates the absolute line source for a given instruction ('pc').
** The array 'lineinfo' gives, for each instruction, the difference in
** lines from the previous instruction. When that difference does not
** fit into a byte, Lua saves the absolute line for that instruction.
** (Lua also saves the absolute line periodically, to speed up the
** computation of a line number: we can use binary search in the
** absolute-line array, but we must traverse the 'lineinfo' array
** linearly to compute a line.)
*/
تعريفنوع بنية Abs_سٽ_Info {
  صحيح pc;
  صحيح سٽ;
} Abs_سٽ_Info;


/*
** Flags in Prototypes
*/
#وصف_ڪريو PF_VAHID	1  /* function has hidden vararg arguments */
#وصف_ڪريو PF_VATAB	2  /* function has vararg table */
#وصف_ڪريو PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#وصف_ڪريو isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#وصف_ڪريو needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
تعريفنوع بنية Proto {
  Common_سرو;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  صحيح sizeupvalues;  /* size of 'upvalues' */
  صحيح sizek;  /* size of 'k' */
  صحيح sizecode;
  صحيح sizelineinfo;
  صحيح sizep;  /* size of 'p' */
  صحيح sizelocvars;
  صحيح sizeabslineinfo;  /* size of 'abslineinfo' */
  صحيح linedefined;  /* debug information  */
  صحيح lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *ڪوڊ;  /* opcodes */
  بنية Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_سٽ_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *ذريعو;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#وصف_ڪريو LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#وصف_ڪريو LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#وصف_ڪريو LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#وصف_ڪريو LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#وصف_ڪريو ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#وصف_ڪريو ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#وصف_ڪريو ttislcf(o)		checktag((o), LUA_VLCF)
#وصف_ڪريو ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#وصف_ڪريو ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#وصف_ڪريو isLfunction(o)	ttisLclosure(o)

#وصف_ڪريو clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#وصف_ڪريو clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#وصف_ڪريو fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#وصف_ڪريو clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#وصف_ڪريو fvalueraw(v)	((v).f)

#وصف_ڪريو setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#وصف_ڪريو setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#وصف_ڪريو setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#وصف_ڪريو setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
تعريفنوع بنية مٿي_Val {
  Common_سرو;
  اتحاد {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t وٿي;  /* used while the stack is being reallocated */
  } v;
  اتحاد {
    بنية {  /* (when open) */
      بنية مٿي_Val *ايندڙ;  /* linked list */
      بنية مٿي_Val **اڳيون;
    } open;
    TValue قدر;  /* the value (when closed) */
  } u;
} مٿي_Val;



#وصف_ڪريو گهيرو_سرو \
	Common_سرو; lu_byte nupvalues; GCObject *gclist

تعريفنوع بنية CClosure {
  گهيرو_سرو;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


تعريفنوع بنية LClosure {
  گهيرو_سرو;
  بنية Proto *p;
  مٿي_Val *upvals[1];  /* list of upvalues */
} LClosure;


تعريفنوع اتحاد گهيرو {
  CClosure c;
  LClosure l;
} گهيرو;


#وصف_ڪريو getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#وصف_ڪريو LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#وصف_ڪريو ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#وصف_ڪريو hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#وصف_ڪريو sethvalue(L,obj,x) \
  { TValue *io = (obj); جدول *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#وصف_ڪريو sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
تعريفنوع اتحاد ڳنڍ_2 {
  بنية ڳنڍ_ڪنجي {
    TValuefields;  /* fields for value */
    lu_byte ڪنجي_tt;  /* key type */
    صحيح ايندڙ;  /* for chaining */
    قدر_2 ڪنجي_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} ڳنڍ_2;


/* copy a value into a key */
#وصف_ڪريو setnodekey(ڳنڍ,obj) \
	{ ڳنڍ_2 *n_=(ڳنڍ); ثابت TValue *io_=(obj); \
	  n_->u.ڪنجي_val = io_->قدر_3; n_->u.ڪنجي_tt = io_->tt_; }


/* copy a value from a key */
#وصف_ڪريو getnodekey(L,obj,ڳنڍ) \
	{ TValue *io_=(obj); ثابت ڳنڍ_2 *n_=(ڳنڍ); \
	  io_->قدر_3 = n_->u.ڪنجي_val; io_->tt_ = n_->u.ڪنجي_tt; \
	  checkliveness(L,io_); }



تعريفنوع بنية جدول {
  Common_سرو;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  بنانشان صحيح asize;  /* number of slots in 'array' array */
  قدر_2 *array;  /* array part */
  ڳنڍ_2 *ڳنڍ;
  بنية جدول *metatable;
  GCObject *gclist;
} جدول;


/*
** Macros to manipulate keys inserted in nodes
*/
#وصف_ڪريو keytt(ڳنڍ)		((ڳنڍ)->u.ڪنجي_tt)
#وصف_ڪريو keyval(ڳنڍ)		((ڳنڍ)->u.ڪنجي_val)

#وصف_ڪريو keyisnil(ڳنڍ)		(keytt(ڳنڍ) == LUA_TNIL)
#وصف_ڪريو keyisinteger(ڳنڍ)	(keytt(ڳنڍ) == LUA_VNUMINT)
#وصف_ڪريو keyival(ڳنڍ)		(keyval(ڳنڍ).i)
#وصف_ڪريو keyisshrstr(ڳنڍ)	(keytt(ڳنڍ) == ctb(LUA_VSHRSTR))
#وصف_ڪريو keystrval(ڳنڍ)		(gco2ts(keyval(ڳنڍ).gc))

#وصف_ڪريو setnilkey(ڳنڍ)		(keytt(ڳنڍ) = LUA_TNIL)

#وصف_ڪريو keyiscollectable(n)	(keytt(n) & ٻٽو_انگ_ISCOLLECTABLE)

#وصف_ڪريو gckey(n)	(keyval(n).gc)
#وصف_ڪريو gckeyN(n)	(keyiscollectable(n) ? gckey(n) : عدم)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#وصف_ڪريو setdeadkey(ڳنڍ)	(keytt(ڳنڍ) = LUA_TDEADKEY)
#وصف_ڪريو keyisdead(ڳنڍ)		(keytt(ڳنڍ) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#وصف_ڪريو lmod(s,سائيز) \
	(check_exp((سائيز&(سائيز-1))==0, (cast_uint(s) & cast_uint((سائيز)-1))))


#وصف_ڪريو twoto(x)	(1u<<(x))
#وصف_ڪريو sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#وصف_ڪريو UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#وصف_ڪريو pushvfstring(L, argp, fmt, نياپو)	\
  { va_start(argp, fmt); \
  نياپو = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  إذا (نياپو == عدم) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC صحيح luaO_utf8esc (حرف *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (بنانشان صحيح x);
LUAI_FUNC lu_byte luaO_codeparam (بنانشان صحيح p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC صحيح luaO_rawarith (lua_State *L, صحيح op, ثابت TValue *p1,
                             ثابت TValue *p2, TValue *res);
LUAI_FUNC فراغ luaO_arith (lua_State *L, صحيح op, ثابت TValue *p1,
                           ثابت TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ثابت حرف *s, TValue *o);
LUAI_FUNC بنانشان luaO_tostringbuff (ثابت TValue *obj, حرف *buff);
LUAI_FUNC lu_byte luaO_hexavalue (صحيح c);
LUAI_FUNC فراغ luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ثابت حرف *luaO_pushvfstring (lua_State *L, ثابت حرف *fmt,
                                                       va_list argp);
LUAI_FUNC ثابت حرف *luaO_pushfstring (lua_State *L, ثابت حرف *fmt, ...);
LUAI_FUNC فراغ luaO_chunkid (حرف *ٻاهريون, ثابت حرف *ذريعو, size_t srclen);


#جيڪڏهن_پورو

