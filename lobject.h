/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#अगर_अघोषित lobject_h
#घोषणा lobject_h


#समावेश <stdarg.h>


#समावेश "llimits.h"
#समावेश "lua.h"


/*
** Extra types for collectable non-values
*/
#घोषणा LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#घोषणा LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#घोषणा LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#घोषणा LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#घोषणा makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
प्रकार जोड़ मूल्य_2 {
  काष्ठा GCObject *gc;    /* collectable objects */
  व्योम *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_संख्या n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} मूल्य_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#घोषणा TValuefields	मूल्य_2 मूल्य_3; lu_byte tt_

प्रकार काष्ठा TValue {
  TValuefields;
} TValue;


#घोषणा val_(o)		((o)->मूल्य_3)
#घोषणा valraw(o)	(val_(o))


/* raw type tag of a TValue */
#घोषणा rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#घोषणा novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#घोषणा withvariant(t)	((t) & 0x3F)
#घोषणा ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#घोषणा ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#घोषणा checktag(o,t)		(rawtt(o) == (t))
#घोषणा checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#घोषणा righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#घोषणा checkliveness(L,obj) \
	((व्योम)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == शून्य || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#घोषणा settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#घोषणा setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); स्थिर TValue *io2=(obj2); \
          io1->मूल्य_3 = io2->मूल्य_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#घोषणा setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#घोषणा setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#घोषणा setobjt2t	setobj
/* to new object */
#घोषणा setobj2n	setobj
/* to table */
#घोषणा setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
प्रकार जोड़ ढेर_मूल्य {
  TValue val;
  काष्ठा {
    TValuefields;
    अचिन्हित लघु delta;
  } tbclist;
} ढेर_मूल्य;


/* index to stack elements */
प्रकार ढेर_मूल्य *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
प्रकार जोड़ {
  StkId p;  /* actual pointer */
  सूचक_भेद_प्रकार offset;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#घोषणा s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#घोषणा LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#घोषणा LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#घोषणा LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#घोषणा LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#घोषणा ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#घोषणा tagisempty(tag)		(novariant(tag) == LUA_TNIL)


/* macro to test for a standard nil */
#घोषणा ttisstrictnil(o)	checktag((o), LUA_VNIL)


#घोषणा setnilvalue(obj)	settt_(obj, LUA_VNIL)
#घोषणा setnilvalue2s(stk)	setnilvalue(s2v(stk))


#घोषणा isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#घोषणा isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#घोषणा isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#घोषणा ABSTKEYCONSTANT		{शून्य}, LUA_VABSTKEY


/* mark an entry as empty */
#घोषणा setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#घोषणा LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#घोषणा LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#घोषणा ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#घोषणा ttisfalse(o)		checktag((o), LUA_VFALSE)
#घोषणा ttistrue(o)		checktag((o), LUA_VTRUE)


#घोषणा l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#घोषणा tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#घोषणा setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#घोषणा setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#घोषणा LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#घोषणा ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#घोषणा thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#घोषणा setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#घोषणा setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#घोषणा CommonHeader	काष्ठा GCObject *अग्र; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
प्रकार काष्ठा GCObject {
  CommonHeader;
} GCObject;


/* Bit mark for collectable types */
#घोषणा BIT_ISCOLLECTABLE	(1 << 6)

#घोषणा iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)

/* mark a tag as collectable */
#घोषणा ctb(t)			((t) | BIT_ISCOLLECTABLE)

#घोषणा gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#घोषणा gcvalueraw(v)	((v).gc)

#घोषणा setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#घोषणा LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#घोषणा LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#घोषणा ttisnumber(o)		checktype((o), LUA_TNUMBER)
#घोषणा ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#घोषणा ttisinteger(o)		checktag((o), LUA_VNUMINT)

#घोषणा nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_अंक(ivalue(o)) : fltvalue(o)))
#घोषणा fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#घोषणा ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#घोषणा fltvalueraw(v)	((v).n)
#घोषणा ivalueraw(v)	((v).i)

#घोषणा setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#घोषणा chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#घोषणा setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#घोषणा chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#घोषणा LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#घोषणा LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#घोषणा ttisstring(o)		checktype((o), LUA_TSTRING)
#घोषणा ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#घोषणा ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#घोषणा tsvalueraw(v)	(gco2ts((v).gc))

#घोषणा tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#घोषणा setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#घोषणा setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#घोषणा setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#घोषणा LSTRREG		-1  /* regular long string */
#घोषणा LSTRFIX		-2  /* fixed external long string */
#घोषणा LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
प्रकार काष्ठा TString {
  CommonHeader;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  अचिन्हित पूर्णांक hash;
  जोड़ {
    माप_प्रकार lnglen;  /* length for long strings */
    काष्ठा TString *hnext;  /* linked list for hash table */
  } u;
  अक्षर *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  व्योम *ud;  /* user data for external strings */
} TString;


#घोषणा strisshr(ts)	((ts)->shrlen >= 0)
#घोषणा isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#घोषणा rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#घोषणा getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#घोषणा getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#घोषणा getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#घोषणा tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#घोषणा getlstr(ts, len)  \
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
#घोषणा LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#घोषणा LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#घोषणा ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#घोषणा ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#घोषणा pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#घोषणा uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#घोषणा pvalueraw(v)	((v).p)

#घोषणा setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#घोषणा setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
प्रकार जोड़ UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
प्रकार काष्ठा Udata {
  CommonHeader;
  अचिन्हित लघु nuvalue;  /* number of user values */
  माप_प्रकार len;  /* number of bytes */
  काष्ठा Table *metatable;
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
प्रकार काष्ठा Udata0 {
  CommonHeader;
  अचिन्हित लघु nuvalue;  /* number of user values */
  माप_प्रकार len;  /* number of bytes */
  काष्ठा Table *metatable;
  जोड़ {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#घोषणा udatamemoffset(nuv) \
       ((nuv) == 0 ? दुरत्व(Udata0, bindata)  \
		   : दुरत्व(Udata, uv) + (माप(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#घोषणा getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#घोषणा sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#घोषणा LUA_VPROTO	makevariant(LUA_TPROTO, 0)


प्रकार l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
प्रकार काष्ठा Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte kind;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
प्रकार काष्ठा LocVar {
  TString *varname;
  पूर्णांक startpc;  /* first point where variable is active */
  पूर्णांक endpc;    /* first point where variable is dead */
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
प्रकार काष्ठा Abs_पंक्ति_Info {
  पूर्णांक pc;
  पूर्णांक पंक्ति;
} Abs_पंक्ति_Info;


/*
** Flags in Prototypes
*/
#घोषणा PF_VAHID	1  /* function has hidden vararg arguments */
#घोषणा PF_VATAB	2  /* function has vararg table */
#घोषणा PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#घोषणा isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#घोषणा needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
प्रकार काष्ठा Proto {
  CommonHeader;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  पूर्णांक sizeupvalues;  /* size of 'upvalues' */
  पूर्णांक sizek;  /* size of 'k' */
  पूर्णांक sizecode;
  पूर्णांक sizelineinfo;
  पूर्णांक sizep;  /* size of 'p' */
  पूर्णांक sizelocvars;
  पूर्णांक sizeabslineinfo;  /* size of 'abslineinfo' */
  पूर्णांक linedefined;  /* debug information  */
  पूर्णांक lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *code;  /* opcodes */
  काष्ठा Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_पंक्ति_Info *abslineinfo;  /* idem */
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

#घोषणा LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#घोषणा LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#घोषणा LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#घोषणा LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#घोषणा ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#घोषणा ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#घोषणा ttislcf(o)		checktag((o), LUA_VLCF)
#घोषणा ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#घोषणा ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#घोषणा isLfunction(o)	ttisLclosure(o)

#घोषणा clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#घोषणा clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#घोषणा fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#घोषणा clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#घोषणा fvalueraw(v)	((v).f)

#घोषणा setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#घोषणा setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#घोषणा setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#घोषणा setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
प्रकार काष्ठा UpVal {
  CommonHeader;
  जोड़ {
    TValue *p;  /* points to stack or to its own value */
    सूचक_भेद_प्रकार offset;  /* used while the stack is being reallocated */
  } v;
  जोड़ {
    काष्ठा {  /* (when open) */
      काष्ठा UpVal *अग्र;  /* linked list */
      काष्ठा UpVal **previous;
    } खोलो;
    TValue मूल्य;  /* the value (when closed) */
  } u;
} UpVal;



#घोषणा ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist

प्रकार काष्ठा CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


प्रकार काष्ठा LClosure {
  ClosureHeader;
  काष्ठा Proto *p;
  UpVal *upvals[1];  /* list of upvalues */
} LClosure;


प्रकार जोड़ Closure {
  CClosure c;
  LClosure l;
} Closure;


#घोषणा getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#घोषणा LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#घोषणा ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#घोषणा hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#घोषणा sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#घोषणा sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
प्रकार जोड़ गाँठ_2 {
  काष्ठा गाँठ_कुंजी {
    TValuefields;  /* fields for value */
    lu_byte कुंजी_tt;  /* key type */
    पूर्णांक अग्र;  /* for chaining */
    मूल्य_2 कुंजी_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} गाँठ_2;


/* copy a value into a key */
#घोषणा setnodekey(गाँठ,obj) \
	{ गाँठ_2 *n_=(गाँठ); स्थिर TValue *io_=(obj); \
	  n_->u.कुंजी_val = io_->मूल्य_3; n_->u.कुंजी_tt = io_->tt_; }


/* copy a value from a key */
#घोषणा getnodekey(L,obj,गाँठ) \
	{ TValue *io_=(obj); स्थिर गाँठ_2 *n_=(गाँठ); \
	  io_->मूल्य_3 = n_->u.कुंजी_val; io_->tt_ = n_->u.कुंजी_tt; \
	  checkliveness(L,io_); }



प्रकार काष्ठा Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  अचिन्हित पूर्णांक asize;  /* number of slots in 'array' array */
  मूल्य_2 *array;  /* array part */
  गाँठ_2 *गाँठ;
  काष्ठा Table *metatable;
  GCObject *gclist;
} Table;


/*
** Macros to manipulate keys inserted in nodes
*/
#घोषणा keytt(गाँठ)		((गाँठ)->u.कुंजी_tt)
#घोषणा keyval(गाँठ)		((गाँठ)->u.कुंजी_val)

#घोषणा keyisnil(गाँठ)		(keytt(गाँठ) == LUA_TNIL)
#घोषणा keyisinteger(गाँठ)	(keytt(गाँठ) == LUA_VNUMINT)
#घोषणा keyival(गाँठ)		(keyval(गाँठ).i)
#घोषणा keyisshrstr(गाँठ)	(keytt(गाँठ) == ctb(LUA_VSHRSTR))
#घोषणा keystrval(गाँठ)		(gco2ts(keyval(गाँठ).gc))

#घोषणा setnilkey(गाँठ)		(keytt(गाँठ) = LUA_TNIL)

#घोषणा keyiscollectable(n)	(keytt(n) & BIT_ISCOLLECTABLE)

#घोषणा gckey(n)	(keyval(n).gc)
#घोषणा gckeyN(n)	(keyiscollectable(n) ? gckey(n) : शून्य)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#घोषणा setdeadkey(गाँठ)	(keytt(गाँठ) = LUA_TDEADKEY)
#घोषणा keyisdead(गाँठ)		(keytt(गाँठ) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#घोषणा lmod(s,आकार) \
	(check_exp((आकार&(आकार-1))==0, (cast_uint(s) & cast_uint((आकार)-1))))


#घोषणा twoto(x)	(1u<<(x))
#घोषणा sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#घोषणा UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#घोषणा pushvfstring(L, argp, fmt, संदेश)	\
  { बहु_शुरू(argp, fmt); \
  संदेश = luaO_pushvfstring(L, fmt, argp); \
  बहु_पूर्ण(argp); \
  अगर (संदेश == शून्य) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC पूर्णांक luaO_utf8esc (अक्षर *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (अचिन्हित पूर्णांक x);
LUAI_FUNC lu_byte luaO_codeparam (अचिन्हित पूर्णांक p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC पूर्णांक luaO_rawarith (lua_State *L, पूर्णांक op, स्थिर TValue *p1,
                             स्थिर TValue *p2, TValue *res);
LUAI_FUNC व्योम luaO_arith (lua_State *L, पूर्णांक op, स्थिर TValue *p1,
                           स्थिर TValue *p2, StkId res);
LUAI_FUNC माप_प्रकार luaO_str2num (स्थिर अक्षर *s, TValue *o);
LUAI_FUNC अचिन्हित luaO_tostringbuff (स्थिर TValue *obj, अक्षर *buff);
LUAI_FUNC lu_byte luaO_hexavalue (पूर्णांक c);
LUAI_FUNC व्योम luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC स्थिर अक्षर *luaO_pushvfstring (lua_State *L, स्थिर अक्षर *fmt,
                                                       बहु_सूची argp);
LUAI_FUNC स्थिर अक्षर *luaO_pushfstring (lua_State *L, स्थिर अक्षर *fmt, ...);
LUAI_FUNC व्योम luaO_chunkid (अक्षर *out, स्थिर अक्षर *source, माप_प्रकार srclen);


#पूर्ण_अगर

