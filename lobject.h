/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#यदि_अपरिभाषितम् lobject_h
#परिभाषय lobject_h


#अन्तर्भाव <stdarg.h>


#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lua.h"


/*
** Extra types for collectable non-values
*/
#परिभाषय LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#परिभाषय LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#परिभाषय LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#परिभाषय LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#परिभाषय makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
प्रकारनिर्देश सङ्घ मानम्_2 {
  संरचना GCObject *gc;    /* collectable objects */
  शून्य *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_सङ्ख्या n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} मानम्_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#परिभाषय TValuefields	मानम्_2 मानम्_3; lu_byte tt_

प्रकारनिर्देश संरचना TValue {
  TValuefields;
} TValue;


#परिभाषय val_(o)		((o)->मानम्_3)
#परिभाषय valraw(o)	(val_(o))


/* raw type tag of a TValue */
#परिभाषय rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#परिभाषय novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#परिभाषय withvariant(t)	((t) & 0x3F)
#परिभाषय ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#परिभाषय ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#परिभाषय checktag(o,t)		(rawtt(o) == (t))
#परिभाषय checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#परिभाषय righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#परिभाषय checkliveness(L,obj) \
	((शून्य)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == रिक्त || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#परिभाषय settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#परिभाषय setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ध्रुव TValue *io2=(obj2); \
          io1->मानम्_3 = io2->मानम्_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#परिभाषय setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#परिभाषय setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#परिभाषय setobjt2t	setobj
/* to new object */
#परिभाषय setobj2n	setobj
/* to table */
#परिभाषय setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
प्रकारनिर्देश सङ्घ राशिः_मानम् {
  TValue val;
  संरचना {
    TValuefields;
    अचिह्नित ह्रस्व delta;
  } tbclist;
} राशिः_मानम्;


/* index to stack elements */
प्रकारनिर्देश राशिः_मानम् *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
प्रकारनिर्देश सङ्घ {
  StkId p;  /* actual pointer */
  ptrdiff_t विस्थापनम्;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#परिभाषय s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#परिभाषय LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#परिभाषय LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#परिभाषय LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#परिभाषय LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#परिभाषय ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#परिभाषय tagisempty(चिह्नकम्)		(novariant(चिह्नकम्) == LUA_TNIL)


/* macro to test for a standard nil */
#परिभाषय ttisstrictnil(o)	checktag((o), LUA_VNIL)


#परिभाषय setnilvalue(obj)	settt_(obj, LUA_VNIL)
#परिभाषय setnilvalue2s(stk)	setnilvalue(s2v(stk))


#परिभाषय isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#परिभाषय isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#परिभाषय isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#परिभाषय ABSTKEYCONSTANT		{रिक्त}, LUA_VABSTKEY


/* mark an entry as empty */
#परिभाषय setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#परिभाषय LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#परिभाषय LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#परिभाषय ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#परिभाषय ttisfalse(o)		checktag((o), LUA_VFALSE)
#परिभाषय ttistrue(o)		checktag((o), LUA_VTRUE)


#परिभाषय l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#परिभाषय tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#परिभाषय setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#परिभाषय setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#परिभाषय LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#परिभाषय ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#परिभाषय thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#परिभाषय setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#परिभाषय setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#परिभाषय Common_शीर्षकम्	संरचना GCObject *अग्र; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
प्रकारनिर्देश संरचना GCObject {
  Common_शीर्षकम्;
} GCObject;


/* Bit mark for collectable types */
#परिभाषय BIT_ISCOLLECTABLE	(1 << 6)

#परिभाषय iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)

/* mark a tag as collectable */
#परिभाषय ctb(t)			((t) | BIT_ISCOLLECTABLE)

#परिभाषय gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#परिभाषय gcvalueraw(v)	((v).gc)

#परिभाषय setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#परिभाषय LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#परिभाषय LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#परिभाषय ttisnumber(o)		checktype((o), LUA_TNUMBER)
#परिभाषय ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#परिभाषय ttisinteger(o)		checktag((o), LUA_VNUMINT)

#परिभाषय nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_अङ्कः(ivalue(o)) : fltvalue(o)))
#परिभाषय fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#परिभाषय ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#परिभाषय fltvalueraw(v)	((v).n)
#परिभाषय ivalueraw(v)	((v).i)

#परिभाषय setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#परिभाषय chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#परिभाषय setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#परिभाषय chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#परिभाषय LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#परिभाषय LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#परिभाषय ttisstring(o)		checktype((o), LUA_TSTRING)
#परिभाषय ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#परिभाषय ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#परिभाषय tsvalueraw(v)	(gco2ts((v).gc))

#परिभाषय tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#परिभाषय setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#परिभाषय setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#परिभाषय setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#परिभाषय LSTRREG		-1  /* regular long string */
#परिभाषय LSTRFIX		-2  /* fixed external long string */
#परिभाषय LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
प्रकारनिर्देश संरचना TString {
  Common_शीर्षकम्;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  अचिह्नित पूर्णाङ्क hash;
  सङ्घ {
    size_t lnglen;  /* length for long strings */
    संरचना TString *hnext;  /* linked list for hash table */
  } u;
  वर्ण *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  शून्य *ud;  /* user data for external strings */
} TString;


#परिभाषय strisshr(ts)	((ts)->shrlen >= 0)
#परिभाषय isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#परिभाषय rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#परिभाषय getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#परिभाषय getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#परिभाषय getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#परिभाषय tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#परिभाषय getlstr(ts, len)  \
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
#परिभाषय LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#परिभाषय LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#परिभाषय ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#परिभाषय ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#परिभाषय pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#परिभाषय uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#परिभाषय pvalueraw(v)	((v).p)

#परिभाषय setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#परिभाषय setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
प्रकारनिर्देश सङ्घ UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
प्रकारनिर्देश संरचना Udata {
  Common_शीर्षकम्;
  अचिह्नित ह्रस्व nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  संरचना सारणी *metatable;
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
प्रकारनिर्देश संरचना Udata0 {
  Common_शीर्षकम्;
  अचिह्नित ह्रस्व nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  संरचना सारणी *metatable;
  सङ्घ {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#परिभाषय udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (परिमाण(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#परिभाषय getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#परिभाषय sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#परिभाषय LUA_VPROTO	makevariant(LUA_TPROTO, 0)


प्रकारनिर्देश l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
प्रकारनिर्देश संरचना Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte विधा;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
प्रकारनिर्देश संरचना LocVar {
  TString *varname;
  पूर्णाङ्क startpc;  /* first point where variable is active */
  पूर्णाङ्क endpc;    /* first point where variable is dead */
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
प्रकारनिर्देश संरचना Abs_पङ्क्तिः_Info {
  पूर्णाङ्क pc;
  पूर्णाङ्क पङ्क्तिः;
} Abs_पङ्क्तिः_Info;


/*
** Flags in Prototypes
*/
#परिभाषय PF_VAHID	1  /* function has hidden vararg arguments */
#परिभाषय PF_VATAB	2  /* function has vararg table */
#परिभाषय PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#परिभाषय isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#परिभाषय needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
प्रकारनिर्देश संरचना Proto {
  Common_शीर्षकम्;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  पूर्णाङ्क sizeupvalues;  /* size of 'upvalues' */
  पूर्णाङ्क sizek;  /* size of 'k' */
  पूर्णाङ्क sizecode;
  पूर्णाङ्क sizelineinfo;
  पूर्णाङ्क sizep;  /* size of 'p' */
  पूर्णाङ्क sizelocvars;
  पूर्णाङ्क sizeabslineinfo;  /* size of 'abslineinfo' */
  पूर्णाङ्क linedefined;  /* debug information  */
  पूर्णाङ्क lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *संहिता;  /* opcodes */
  संरचना Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_पङ्क्तिः_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *source;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#परिभाषय LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#परिभाषय LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#परिभाषय LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#परिभाषय LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#परिभाषय ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#परिभाषय ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#परिभाषय ttislcf(o)		checktag((o), LUA_VLCF)
#परिभाषय ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#परिभाषय ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#परिभाषय isLfunction(o)	ttisLclosure(o)

#परिभाषय clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#परिभाषय clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#परिभाषय fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#परिभाषय clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#परिभाषय fvalueraw(v)	((v).f)

#परिभाषय setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#परिभाषय setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#परिभाषय setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#परिभाषय setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
प्रकारनिर्देश संरचना ऊर्ध्वम्_Val {
  Common_शीर्षकम्;
  सङ्घ {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t विस्थापनम्;  /* used while the stack is being reallocated */
  } v;
  सङ्घ {
    संरचना {  /* (when open) */
      संरचना ऊर्ध्वम्_Val *अग्र;  /* linked list */
      संरचना ऊर्ध्वम्_Val **previous;
    } उद्घाटय;
    TValue मानम्;  /* the value (when closed) */
  } u;
} ऊर्ध्वम्_Val;



#परिभाषय संवृतिः_शीर्षकम् \
	Common_शीर्षकम्; lu_byte nupvalues; GCObject *gclist

प्रकारनिर्देश संरचना CClosure {
  संवृतिः_शीर्षकम्;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


प्रकारनिर्देश संरचना LClosure {
  संवृतिः_शीर्षकम्;
  संरचना Proto *p;
  ऊर्ध्वम्_Val *upvals[1];  /* list of upvalues */
} LClosure;


प्रकारनिर्देश सङ्घ संवृतिः {
  CClosure c;
  LClosure l;
} संवृतिः;


#परिभाषय getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#परिभाषय LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#परिभाषय ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#परिभाषय hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#परिभाषय sethvalue(L,obj,x) \
  { TValue *io = (obj); सारणी *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#परिभाषय sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
प्रकारनिर्देश सङ्घ ग्रन्थिः_2 {
  संरचना ग्रन्थिः_कुञ्चिका {
    TValuefields;  /* fields for value */
    lu_byte कुञ्चिका_tt;  /* key type */
    पूर्णाङ्क अग्र;  /* for chaining */
    मानम्_2 कुञ्चिका_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} ग्रन्थिः_2;


/* copy a value into a key */
#परिभाषय setnodekey(ग्रन्थिः,obj) \
	{ ग्रन्थिः_2 *n_=(ग्रन्थिः); ध्रुव TValue *io_=(obj); \
	  n_->u.कुञ्चिका_val = io_->मानम्_3; n_->u.कुञ्चिका_tt = io_->tt_; }


/* copy a value from a key */
#परिभाषय getnodekey(L,obj,ग्रन्थिः) \
	{ TValue *io_=(obj); ध्रुव ग्रन्थिः_2 *n_=(ग्रन्थिः); \
	  io_->मानम्_3 = n_->u.कुञ्चिका_val; io_->tt_ = n_->u.कुञ्चिका_tt; \
	  checkliveness(L,io_); }



प्रकारनिर्देश संरचना सारणी {
  Common_शीर्षकम्;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  अचिह्नित पूर्णाङ्क asize;  /* number of slots in 'array' array */
  मानम्_2 *array;  /* array part */
  ग्रन्थिः_2 *ग्रन्थिः;
  संरचना सारणी *metatable;
  GCObject *gclist;
} सारणी;


/*
** Macros to manipulate keys inserted in nodes
*/
#परिभाषय keytt(ग्रन्थिः)		((ग्रन्थिः)->u.कुञ्चिका_tt)
#परिभाषय keyval(ग्रन्थिः)		((ग्रन्थिः)->u.कुञ्चिका_val)

#परिभाषय keyisnil(ग्रन्थिः)		(keytt(ग्रन्थिः) == LUA_TNIL)
#परिभाषय keyisinteger(ग्रन्थिः)	(keytt(ग्रन्थिः) == LUA_VNUMINT)
#परिभाषय keyival(ग्रन्थिः)		(keyval(ग्रन्थिः).i)
#परिभाषय keyisshrstr(ग्रन्थिः)	(keytt(ग्रन्थिः) == ctb(LUA_VSHRSTR))
#परिभाषय keystrval(ग्रन्थिः)		(gco2ts(keyval(ग्रन्थिः).gc))

#परिभाषय setnilkey(ग्रन्थिः)		(keytt(ग्रन्थिः) = LUA_TNIL)

#परिभाषय keyiscollectable(n)	(keytt(n) & BIT_ISCOLLECTABLE)

#परिभाषय gckey(n)	(keyval(n).gc)
#परिभाषय gckeyN(n)	(keyiscollectable(n) ? gckey(n) : रिक्त)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#परिभाषय setdeadkey(ग्रन्थिः)	(keytt(ग्रन्थिः) = LUA_TDEADKEY)
#परिभाषय keyisdead(ग्रन्थिः)		(keytt(ग्रन्थिः) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#परिभाषय lmod(s,आकार) \
	(check_exp((आकार&(आकार-1))==0, (cast_uint(s) & cast_uint((आकार)-1))))


#परिभाषय twoto(x)	(1u<<(x))
#परिभाषय sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#परिभाषय UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#परिभाषय pushvfstring(L, argp, fmt, सन्देशः)	\
  { va_start(argp, fmt); \
  सन्देशः = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  यदि (सन्देशः == रिक्त) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC पूर्णाङ्क luaO_utf8esc (वर्ण *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (अचिह्नित पूर्णाङ्क x);
LUAI_FUNC lu_byte luaO_codeparam (अचिह्नित पूर्णाङ्क p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC पूर्णाङ्क luaO_rawarith (lua_State *L, पूर्णाङ्क op, ध्रुव TValue *p1,
                             ध्रुव TValue *p2, TValue *res);
LUAI_FUNC शून्य luaO_arith (lua_State *L, पूर्णाङ्क op, ध्रुव TValue *p1,
                           ध्रुव TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ध्रुव वर्ण *s, TValue *o);
LUAI_FUNC अचिह्नित luaO_tostringbuff (ध्रुव TValue *obj, वर्ण *buff);
LUAI_FUNC lu_byte luaO_hexavalue (पूर्णाङ्क c);
LUAI_FUNC शून्य luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ध्रुव वर्ण *luaO_pushvfstring (lua_State *L, ध्रुव वर्ण *fmt,
                                                       va_list argp);
LUAI_FUNC ध्रुव वर्ण *luaO_pushfstring (lua_State *L, ध्रुव वर्ण *fmt, ...);
LUAI_FUNC शून्य luaO_chunkid (वर्ण *बहिर्गमनम्, ध्रुव वर्ण *source, size_t srclen);


#यदि_अन्तः

