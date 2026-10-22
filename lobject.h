/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#నిర్వచితం_కాకపోతే lobject_h
#నిర్వచించు lobject_h


#అన్తర్భావ <stdarg.h>


#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lua.h"


/*
** Extra types for collectable non-values
*/
#నిర్వచించు LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#నిర్వచించు LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#నిర్వచించు LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#నిర్వచించు LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#నిర్వచించు makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
రకనిర్వచనం సంఘం విలువ_2 {
  నిర్మాణం GCObject *gc;    /* collectable objects */
  శూన్య *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_సంఖ్య n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} విలువ_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#నిర్వచించు TValuefields	విలువ_2 విలువ_3; lu_byte tt_

రకనిర్వచనం నిర్మాణం TValue {
  TValuefields;
} TValue;


#నిర్వచించు val_(o)		((o)->విలువ_3)
#నిర్వచించు valraw(o)	(val_(o))


/* raw type tag of a TValue */
#నిర్వచించు rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#నిర్వచించు novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#నిర్వచించు withvariant(t)	((t) & 0x3F)
#నిర్వచించు ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#నిర్వచించు ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#నిర్వచించు checktag(o,t)		(rawtt(o) == (t))
#నిర్వచించు checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#నిర్వచించు righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#నిర్వచించు checkliveness(L,obj) \
	((శూన్య)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == ఖాళీ || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#నిర్వచించు settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#నిర్వచించు setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); స్థిరరాశి TValue *io2=(obj2); \
          io1->విలువ_3 = io2->విలువ_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#నిర్వచించు setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#నిర్వచించు setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#నిర్వచించు setobjt2t	setobj
/* to new object */
#నిర్వచించు setobj2n	setobj
/* to table */
#నిర్వచించు setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
రకనిర్వచనం సంఘం దొంతర_విలువ {
  TValue val;
  నిర్మాణం {
    TValuefields;
    గుర్తులేని పొట్టి delta;
  } tbclist;
} దొంతర_విలువ;


/* index to stack elements */
రకనిర్వచనం దొంతర_విలువ *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
రకనిర్వచనం సంఘం {
  StkId p;  /* actual pointer */
  ptrdiff_t స్థానభ్రంశం;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#నిర్వచించు s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#నిర్వచించు LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#నిర్వచించు LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#నిర్వచించు LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#నిర్వచించు LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#నిర్వచించు ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#నిర్వచించు tagisempty(ట్యాగ్)		(novariant(ట్యాగ్) == LUA_TNIL)


/* macro to test for a standard nil */
#నిర్వచించు ttisstrictnil(o)	checktag((o), LUA_VNIL)


#నిర్వచించు setnilvalue(obj)	settt_(obj, LUA_VNIL)
#నిర్వచించు setnilvalue2s(stk)	setnilvalue(s2v(stk))


#నిర్వచించు isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#నిర్వచించు isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#నిర్వచించు isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#నిర్వచించు ABSTKEYCONSTANT		{ఖాళీ}, LUA_VABSTKEY


/* mark an entry as empty */
#నిర్వచించు setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#నిర్వచించు LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#నిర్వచించు LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#నిర్వచించు ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#నిర్వచించు ttisfalse(o)		checktag((o), LUA_VFALSE)
#నిర్వచించు ttistrue(o)		checktag((o), LUA_VTRUE)


#నిర్వచించు l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#నిర్వచించు tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#నిర్వచించు setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#నిర్వచించు setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#నిర్వచించు LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#నిర్వచించు ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#నిర్వచించు thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#నిర్వచించు setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#నిర్వచించు setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#నిర్వచించు Common_శీర్షిక	నిర్మాణం GCObject *తదుపరి; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
రకనిర్వచనం నిర్మాణం GCObject {
  Common_శీర్షిక;
} GCObject;


/* Bit mark for collectable types */
#నిర్వచించు బిట్టు_ISCOLLECTABLE	(1 << 6)

#నిర్వచించు iscollectable(o)	(rawtt(o) & బిట్టు_ISCOLLECTABLE)

/* mark a tag as collectable */
#నిర్వచించు ctb(t)			((t) | బిట్టు_ISCOLLECTABLE)

#నిర్వచించు gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#నిర్వచించు gcvalueraw(v)	((v).gc)

#నిర్వచించు setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#నిర్వచించు LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#నిర్వచించు LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#నిర్వచించు ttisnumber(o)		checktype((o), LUA_TNUMBER)
#నిర్వచించు ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#నిర్వచించు ttisinteger(o)		checktag((o), LUA_VNUMINT)

#నిర్వచించు nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_అంకె(ivalue(o)) : fltvalue(o)))
#నిర్వచించు fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#నిర్వచించు ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#నిర్వచించు fltvalueraw(v)	((v).n)
#నిర్వచించు ivalueraw(v)	((v).i)

#నిర్వచించు setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#నిర్వచించు chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#నిర్వచించు setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#నిర్వచించు chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#నిర్వచించు LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#నిర్వచించు LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#నిర్వచించు ttisstring(o)		checktype((o), LUA_TSTRING)
#నిర్వచించు ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#నిర్వచించు ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#నిర్వచించు tsvalueraw(v)	(gco2ts((v).gc))

#నిర్వచించు tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#నిర్వచించు setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#నిర్వచించు setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#నిర్వచించు setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#నిర్వచించు LSTRREG		-1  /* regular long string */
#నిర్వచించు LSTRFIX		-2  /* fixed external long string */
#నిర్వచించు LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
రకనిర్వచనం నిర్మాణం TString {
  Common_శీర్షిక;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  గుర్తులేని పూర్ణాంకం hash;
  సంఘం {
    size_t lnglen;  /* length for long strings */
    నిర్మాణం TString *hnext;  /* linked list for hash table */
  } u;
  అక్షరం *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  శూన్య *ud;  /* user data for external strings */
} TString;


#నిర్వచించు strisshr(ts)	((ts)->shrlen >= 0)
#నిర్వచించు isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#నిర్వచించు rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#నిర్వచించు getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#నిర్వచించు getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#నిర్వచించు getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#నిర్వచించు tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#నిర్వచించు getlstr(ts, len)  \
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
#నిర్వచించు LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#నిర్వచించు LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#నిర్వచించు ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#నిర్వచించు ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#నిర్వచించు pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#నిర్వచించు uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#నిర్వచించు pvalueraw(v)	((v).p)

#నిర్వచించు setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#నిర్వచించు setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
రకనిర్వచనం సంఘం UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
రకనిర్వచనం నిర్మాణం Udata {
  Common_శీర్షిక;
  గుర్తులేని పొట్టి nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  నిర్మాణం పట్టిక *metatable;
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
రకనిర్వచనం నిర్మాణం Udata0 {
  Common_శీర్షిక;
  గుర్తులేని పొట్టి nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  నిర్మాణం పట్టిక *metatable;
  సంఘం {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#నిర్వచించు udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (పరిమాణం(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#నిర్వచించు getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#నిర్వచించు sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#నిర్వచించు LUA_VPROTO	makevariant(LUA_TPROTO, 0)


రకనిర్వచనం l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
రకనిర్వచనం నిర్మాణం Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte రకం;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
రకనిర్వచనం నిర్మాణం LocVar {
  TString *varname;
  పూర్ణాంకం startpc;  /* first point where variable is active */
  పూర్ణాంకం endpc;    /* first point where variable is dead */
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
రకనిర్వచనం నిర్మాణం Abs_పంక్తి_Info {
  పూర్ణాంకం pc;
  పూర్ణాంకం పంక్తి;
} Abs_పంక్తి_Info;


/*
** Flags in Prototypes
*/
#నిర్వచించు PF_VAHID	1  /* function has hidden vararg arguments */
#నిర్వచించు PF_VATAB	2  /* function has vararg table */
#నిర్వచించు PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#నిర్వచించు isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#నిర్వచించు needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
రకనిర్వచనం నిర్మాణం Proto {
  Common_శీర్షిక;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  పూర్ణాంకం sizeupvalues;  /* size of 'upvalues' */
  పూర్ణాంకం sizek;  /* size of 'k' */
  పూర్ణాంకం sizecode;
  పూర్ణాంకం sizelineinfo;
  పూర్ణాంకం sizep;  /* size of 'p' */
  పూర్ణాంకం sizelocvars;
  పూర్ణాంకం sizeabslineinfo;  /* size of 'abslineinfo' */
  పూర్ణాంకం linedefined;  /* debug information  */
  పూర్ణాంకం lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *సంకేతం_2;  /* opcodes */
  నిర్మాణం Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_పంక్తి_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *ఆధారం;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#నిర్వచించు LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#నిర్వచించు LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#నిర్వచించు LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#నిర్వచించు LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#నిర్వచించు ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#నిర్వచించు ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#నిర్వచించు ttislcf(o)		checktag((o), LUA_VLCF)
#నిర్వచించు ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#నిర్వచించు ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#నిర్వచించు isLfunction(o)	ttisLclosure(o)

#నిర్వచించు clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#నిర్వచించు clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#నిర్వచించు fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#నిర్వచించు clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#నిర్వచించు fvalueraw(v)	((v).f)

#నిర్వచించు setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#నిర్వచించు setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#నిర్వచించు setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#నిర్వచించు setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
రకనిర్వచనం నిర్మాణం పైకి_Val {
  Common_శీర్షిక;
  సంఘం {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t స్థానభ్రంశం;  /* used while the stack is being reallocated */
  } v;
  సంఘం {
    నిర్మాణం {  /* (when open) */
      నిర్మాణం పైకి_Val *తదుపరి;  /* linked list */
      నిర్మాణం పైకి_Val **మునుపటి;
    } open;
    TValue విలువ;  /* the value (when closed) */
  } u;
} పైకి_Val;



#నిర్వచించు సంవృతం_శీర్షిక \
	Common_శీర్షిక; lu_byte nupvalues; GCObject *gclist

రకనిర్వచనం నిర్మాణం CClosure {
  సంవృతం_శీర్షిక;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


రకనిర్వచనం నిర్మాణం LClosure {
  సంవృతం_శీర్షిక;
  నిర్మాణం Proto *p;
  పైకి_Val *upvals[1];  /* list of upvalues */
} LClosure;


రకనిర్వచనం సంఘం సంవృతం {
  CClosure c;
  LClosure l;
} సంవృతం;


#నిర్వచించు getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#నిర్వచించు LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#నిర్వచించు ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#నిర్వచించు hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#నిర్వచించు sethvalue(L,obj,x) \
  { TValue *io = (obj); పట్టిక *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#నిర్వచించు sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
రకనిర్వచనం సంఘం కణుపు_2 {
  నిర్మాణం కణుపు_కీలకం {
    TValuefields;  /* fields for value */
    lu_byte కీలకం_tt;  /* key type */
    పూర్ణాంకం తదుపరి;  /* for chaining */
    విలువ_2 కీలకం_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} కణుపు_2;


/* copy a value into a key */
#నిర్వచించు setnodekey(కణుపు,obj) \
	{ కణుపు_2 *n_=(కణుపు); స్థిరరాశి TValue *io_=(obj); \
	  n_->u.కీలకం_val = io_->విలువ_3; n_->u.కీలకం_tt = io_->tt_; }


/* copy a value from a key */
#నిర్వచించు getnodekey(L,obj,కణుపు) \
	{ TValue *io_=(obj); స్థిరరాశి కణుపు_2 *n_=(కణుపు); \
	  io_->విలువ_3 = n_->u.కీలకం_val; io_->tt_ = n_->u.కీలకం_tt; \
	  checkliveness(L,io_); }



రకనిర్వచనం నిర్మాణం పట్టిక {
  Common_శీర్షిక;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  గుర్తులేని పూర్ణాంకం asize;  /* number of slots in 'array' array */
  విలువ_2 *array;  /* array part */
  కణుపు_2 *కణుపు;
  నిర్మాణం పట్టిక *metatable;
  GCObject *gclist;
} పట్టిక;


/*
** Macros to manipulate keys inserted in nodes
*/
#నిర్వచించు keytt(కణుపు)		((కణుపు)->u.కీలకం_tt)
#నిర్వచించు keyval(కణుపు)		((కణుపు)->u.కీలకం_val)

#నిర్వచించు keyisnil(కణుపు)		(keytt(కణుపు) == LUA_TNIL)
#నిర్వచించు keyisinteger(కణుపు)	(keytt(కణుపు) == LUA_VNUMINT)
#నిర్వచించు keyival(కణుపు)		(keyval(కణుపు).i)
#నిర్వచించు keyisshrstr(కణుపు)	(keytt(కణుపు) == ctb(LUA_VSHRSTR))
#నిర్వచించు keystrval(కణుపు)		(gco2ts(keyval(కణుపు).gc))

#నిర్వచించు setnilkey(కణుపు)		(keytt(కణుపు) = LUA_TNIL)

#నిర్వచించు keyiscollectable(n)	(keytt(n) & బిట్టు_ISCOLLECTABLE)

#నిర్వచించు gckey(n)	(keyval(n).gc)
#నిర్వచించు gckeyN(n)	(keyiscollectable(n) ? gckey(n) : ఖాళీ)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#నిర్వచించు setdeadkey(కణుపు)	(keytt(కణుపు) = LUA_TDEADKEY)
#నిర్వచించు keyisdead(కణుపు)		(keytt(కణుపు) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#నిర్వచించు lmod(s,కొలత) \
	(check_exp((కొలత&(కొలత-1))==0, (cast_uint(s) & cast_uint((కొలత)-1))))


#నిర్వచించు twoto(x)	(1u<<(x))
#నిర్వచించు sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#నిర్వచించు UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#నిర్వచించు pushvfstring(L, argp, fmt, సందేశం)	\
  { va_start(argp, fmt); \
  సందేశం = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  ఒకవేళ (సందేశం == ఖాళీ) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC పూర్ణాంకం luaO_utf8esc (అక్షరం *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (గుర్తులేని పూర్ణాంకం x);
LUAI_FUNC lu_byte luaO_codeparam (గుర్తులేని పూర్ణాంకం p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC పూర్ణాంకం luaO_rawarith (lua_State *L, పూర్ణాంకం op, స్థిరరాశి TValue *p1,
                             స్థిరరాశి TValue *p2, TValue *res);
LUAI_FUNC శూన్య luaO_arith (lua_State *L, పూర్ణాంకం op, స్థిరరాశి TValue *p1,
                           స్థిరరాశి TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (స్థిరరాశి అక్షరం *s, TValue *o);
LUAI_FUNC గుర్తులేని luaO_tostringbuff (స్థిరరాశి TValue *obj, అక్షరం *buff);
LUAI_FUNC lu_byte luaO_hexavalue (పూర్ణాంకం c);
LUAI_FUNC శూన్య luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC స్థిరరాశి అక్షరం *luaO_pushvfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt,
                                                       va_list argp);
LUAI_FUNC స్థిరరాశి అక్షరం *luaO_pushfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...);
LUAI_FUNC శూన్య luaO_chunkid (అక్షరం *బహిర్గతం, స్థిరరాశి అక్షరం *ఆధారం, size_t srclen);


#అయితే_ముగింపు

