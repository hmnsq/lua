/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#جے_بنا_تعریف lobject_h
#تعریف_کرو lobject_h


#ضمن <stdarg.h>


#ضمن "llimits.h"
#ضمن "lua.h"


/*
** Extra types for collectable non-values
*/
#تعریف_کرو LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#تعریف_کرو LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#تعریف_کرو LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#تعریف_کرو LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#تعریف_کرو makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
تعريفنوع اتحاد قدر_2 {
  بنية GCObject *gc;    /* collectable objects */
  فراغ *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_نمبر n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} قدر_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#تعریف_کرو TValuefields	قدر_2 قدر_3; lu_byte tt_

تعريفنوع بنية TValue {
  TValuefields;
} TValue;


#تعریف_کرو val_(o)		((o)->قدر_3)
#تعریف_کرو valraw(o)	(val_(o))


/* raw type tag of a TValue */
#تعریف_کرو rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#تعریف_کرو novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#تعریف_کرو withvariant(t)	((t) & 0x3F)
#تعریف_کرو ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#تعریف_کرو ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#تعریف_کرو checktag(o,t)		(rawtt(o) == (t))
#تعریف_کرو checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#تعریف_کرو righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#تعریف_کرو checkliveness(L,obj) \
	((فراغ)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == عدم || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#تعریف_کرو settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#تعریف_کرو setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ثابت TValue *io2=(obj2); \
          io1->قدر_3 = io2->قدر_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#تعریف_کرو setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#تعریف_کرو setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#تعریف_کرو setobjt2t	setobj
/* to new object */
#تعریف_کرو setobj2n	setobj
/* to table */
#تعریف_کرو setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
تعريفنوع اتحاد ڈھیر_قدر {
  TValue val;
  بنية {
    TValuefields;
    بنانشانی چھوٹا delta;
  } tbclist;
} ڈھیر_قدر;


/* index to stack elements */
تعريفنوع ڈھیر_قدر *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
تعريفنوع اتحاد {
  StkId p;  /* actual pointer */
  ptrdiff_t وتھ;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#تعریف_کرو s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#تعریف_کرو LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#تعریف_کرو LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#تعریف_کرو LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#تعریف_کرو LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#تعریف_کرو ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#تعریف_کرو tagisempty(ٹیگ)		(novariant(ٹیگ) == LUA_TNIL)


/* macro to test for a standard nil */
#تعریف_کرو ttisstrictnil(o)	checktag((o), LUA_VNIL)


#تعریف_کرو setnilvalue(obj)	settt_(obj, LUA_VNIL)
#تعریف_کرو setnilvalue2s(stk)	setnilvalue(s2v(stk))


#تعریف_کرو isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#تعریف_کرو isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#تعریف_کرو isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#تعریف_کرو ABSTKEYCONSTANT		{عدم}, LUA_VABSTKEY


/* mark an entry as empty */
#تعریف_کرو setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#تعریف_کرو LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#تعریف_کرو LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#تعریف_کرو ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#تعریف_کرو ttisfalse(o)		checktag((o), LUA_VFALSE)
#تعریف_کرو ttistrue(o)		checktag((o), LUA_VTRUE)


#تعریف_کرو l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#تعریف_کرو tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#تعریف_کرو setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#تعریف_کرو setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#تعریف_کرو LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#تعریف_کرو ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#تعریف_کرو thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#تعریف_کرو setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#تعریف_کرو setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#تعریف_کرو Common_سرخی	بنية GCObject *اگلا; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
تعريفنوع بنية GCObject {
  Common_سرخی;
} GCObject;


/* Bit mark for collectable types */
#تعریف_کرو ثنائی_ہندسہ_ISCOLLECTABLE	(1 << 6)

#تعریف_کرو iscollectable(o)	(rawtt(o) & ثنائی_ہندسہ_ISCOLLECTABLE)

/* mark a tag as collectable */
#تعریف_کرو ctb(t)			((t) | ثنائی_ہندسہ_ISCOLLECTABLE)

#تعریف_کرو gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#تعریف_کرو gcvalueraw(v)	((v).gc)

#تعریف_کرو setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#تعریف_کرو LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#تعریف_کرو LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#تعریف_کرو ttisnumber(o)		checktype((o), LUA_TNUMBER)
#تعریف_کرو ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#تعریف_کرو ttisinteger(o)		checktag((o), LUA_VNUMINT)

#تعریف_کرو nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_عدد(ivalue(o)) : fltvalue(o)))
#تعریف_کرو fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#تعریف_کرو ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#تعریف_کرو fltvalueraw(v)	((v).n)
#تعریف_کرو ivalueraw(v)	((v).i)

#تعریف_کرو setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#تعریف_کرو chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#تعریف_کرو setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#تعریف_کرو chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#تعریف_کرو LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#تعریف_کرو LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#تعریف_کرو ttisstring(o)		checktype((o), LUA_TSTRING)
#تعریف_کرو ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#تعریف_کرو ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#تعریف_کرو tsvalueraw(v)	(gco2ts((v).gc))

#تعریف_کرو tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#تعریف_کرو setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#تعریف_کرو setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#تعریف_کرو setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#تعریف_کرو LSTRREG		-1  /* regular long string */
#تعریف_کرو LSTRFIX		-2  /* fixed external long string */
#تعریف_کرو LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
تعريفنوع بنية TString {
  Common_سرخی;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  بنانشانی صحيح hash;
  اتحاد {
    size_t lnglen;  /* length for long strings */
    بنية TString *hnext;  /* linked list for hash table */
  } u;
  حرف *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  فراغ *ud;  /* user data for external strings */
} TString;


#تعریف_کرو strisshr(ts)	((ts)->shrlen >= 0)
#تعریف_کرو isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#تعریف_کرو rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#تعریف_کرو getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#تعریف_کرو getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#تعریف_کرو getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#تعریف_کرو tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#تعریف_کرو getlstr(ts, len)  \
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
#تعریف_کرو LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#تعریف_کرو LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#تعریف_کرو ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#تعریف_کرو ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#تعریف_کرو pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#تعریف_کرو uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#تعریف_کرو pvalueraw(v)	((v).p)

#تعریف_کرو setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#تعریف_کرو setuvalue(L,obj,x) \
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
  Common_سرخی;
  بنانشانی چھوٹا nuvalue;  /* number of user values */
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
  Common_سرخی;
  بنانشانی چھوٹا nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  بنية جدول *metatable;
  اتحاد {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#تعریف_کرو udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (حجم(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#تعریف_کرو getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#تعریف_کرو sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#تعریف_کرو LUA_VPROTO	makevariant(LUA_TPROTO, 0)


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
تعريفنوع بنية Abs_سطر_Info {
  صحيح pc;
  صحيح سطر;
} Abs_سطر_Info;


/*
** Flags in Prototypes
*/
#تعریف_کرو PF_VAHID	1  /* function has hidden vararg arguments */
#تعریف_کرو PF_VATAB	2  /* function has vararg table */
#تعریف_کرو PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#تعریف_کرو isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#تعریف_کرو needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
تعريفنوع بنية Proto {
  Common_سرخی;
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
  Instruction *کوڈ;  /* opcodes */
  بنية Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_سطر_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *سروت;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#تعریف_کرو LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#تعریف_کرو LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#تعریف_کرو LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#تعریف_کرو LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#تعریف_کرو ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#تعریف_کرو ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#تعریف_کرو ttislcf(o)		checktag((o), LUA_VLCF)
#تعریف_کرو ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#تعریف_کرو ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#تعریف_کرو isLfunction(o)	ttisLclosure(o)

#تعریف_کرو clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#تعریف_کرو clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#تعریف_کرو fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#تعریف_کرو clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#تعریف_کرو fvalueraw(v)	((v).f)

#تعریف_کرو setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#تعریف_کرو setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#تعریف_کرو setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#تعریف_کرو setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
تعريفنوع بنية اتے_Val {
  Common_سرخی;
  اتحاد {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t وتھ;  /* used while the stack is being reallocated */
  } v;
  اتحاد {
    بنية {  /* (when open) */
      بنية اتے_Val *اگلا;  /* linked list */
      بنية اتے_Val **پچھلا;
    } open;
    TValue قدر;  /* the value (when closed) */
  } u;
} اتے_Val;



#تعریف_کرو گھیرا_سرخی \
	Common_سرخی; lu_byte nupvalues; GCObject *gclist

تعريفنوع بنية CClosure {
  گھیرا_سرخی;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


تعريفنوع بنية LClosure {
  گھیرا_سرخی;
  بنية Proto *p;
  اتے_Val *upvals[1];  /* list of upvalues */
} LClosure;


تعريفنوع اتحاد گھیرا {
  CClosure c;
  LClosure l;
} گھیرا;


#تعریف_کرو getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#تعریف_کرو LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#تعریف_کرو ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#تعریف_کرو hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#تعریف_کرو sethvalue(L,obj,x) \
  { TValue *io = (obj); جدول *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#تعریف_کرو sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
تعريفنوع اتحاد گرہ_2 {
  بنية گرہ_کلید {
    TValuefields;  /* fields for value */
    lu_byte کلید_tt;  /* key type */
    صحيح اگلا;  /* for chaining */
    قدر_2 کلید_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} گرہ_2;


/* copy a value into a key */
#تعریف_کرو setnodekey(گرہ,obj) \
	{ گرہ_2 *n_=(گرہ); ثابت TValue *io_=(obj); \
	  n_->u.کلید_val = io_->قدر_3; n_->u.کلید_tt = io_->tt_; }


/* copy a value from a key */
#تعریف_کرو getnodekey(L,obj,گرہ) \
	{ TValue *io_=(obj); ثابت گرہ_2 *n_=(گرہ); \
	  io_->قدر_3 = n_->u.کلید_val; io_->tt_ = n_->u.کلید_tt; \
	  checkliveness(L,io_); }



تعريفنوع بنية جدول {
  Common_سرخی;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  بنانشانی صحيح asize;  /* number of slots in 'array' array */
  قدر_2 *array;  /* array part */
  گرہ_2 *گرہ;
  بنية جدول *metatable;
  GCObject *gclist;
} جدول;


/*
** Macros to manipulate keys inserted in nodes
*/
#تعریف_کرو keytt(گرہ)		((گرہ)->u.کلید_tt)
#تعریف_کرو keyval(گرہ)		((گرہ)->u.کلید_val)

#تعریف_کرو keyisnil(گرہ)		(keytt(گرہ) == LUA_TNIL)
#تعریف_کرو keyisinteger(گرہ)	(keytt(گرہ) == LUA_VNUMINT)
#تعریف_کرو keyival(گرہ)		(keyval(گرہ).i)
#تعریف_کرو keyisshrstr(گرہ)	(keytt(گرہ) == ctb(LUA_VSHRSTR))
#تعریف_کرو keystrval(گرہ)		(gco2ts(keyval(گرہ).gc))

#تعریف_کرو setnilkey(گرہ)		(keytt(گرہ) = LUA_TNIL)

#تعریف_کرو keyiscollectable(n)	(keytt(n) & ثنائی_ہندسہ_ISCOLLECTABLE)

#تعریف_کرو gckey(n)	(keyval(n).gc)
#تعریف_کرو gckeyN(n)	(keyiscollectable(n) ? gckey(n) : عدم)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#تعریف_کرو setdeadkey(گرہ)	(keytt(گرہ) = LUA_TDEADKEY)
#تعریف_کرو keyisdead(گرہ)		(keytt(گرہ) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#تعریف_کرو lmod(s,ناپ) \
	(check_exp((ناپ&(ناپ-1))==0, (cast_uint(s) & cast_uint((ناپ)-1))))


#تعریف_کرو twoto(x)	(1u<<(x))
#تعریف_کرو sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#تعریف_کرو UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#تعریف_کرو pushvfstring(L, argp, fmt, پیغام)	\
  { va_start(argp, fmt); \
  پیغام = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  إذا (پیغام == عدم) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC صحيح luaO_utf8esc (حرف *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (بنانشانی صحيح x);
LUAI_FUNC lu_byte luaO_codeparam (بنانشانی صحيح p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC صحيح luaO_rawarith (lua_State *L, صحيح op, ثابت TValue *p1,
                             ثابت TValue *p2, TValue *res);
LUAI_FUNC فراغ luaO_arith (lua_State *L, صحيح op, ثابت TValue *p1,
                           ثابت TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ثابت حرف *s, TValue *o);
LUAI_FUNC بنانشانی luaO_tostringbuff (ثابت TValue *obj, حرف *buff);
LUAI_FUNC lu_byte luaO_hexavalue (صحيح c);
LUAI_FUNC فراغ luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ثابت حرف *luaO_pushvfstring (lua_State *L, ثابت حرف *fmt,
                                                       va_list argp);
LUAI_FUNC ثابت حرف *luaO_pushfstring (lua_State *L, ثابت حرف *fmt, ...);
LUAI_FUNC فراغ luaO_chunkid (حرف *باہرلا, ثابت حرف *سروت, size_t srclen);


#جے_انت

