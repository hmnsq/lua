/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#ಅನ್ತರ್ಭಾವ <stdarg.h>


#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lua.h"


/*
** Extra types for collectable non-values
*/
#define LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#define LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#define LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#define makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ ಮೌಲ್ಯ_2 {
  ಸಂರಚನಾ GCObject *gc;    /* collectable objects */
  ಶೂನ್ಯ *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_ಸಂಖ್ಯೆ n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} ಮೌಲ್ಯ_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#define TValuefields	ಮೌಲ್ಯ_2 ಮೌಲ್ಯ_3; lu_byte tt_

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TValue {
  TValuefields;
} TValue;


#define val_(o)		((o)->ಮೌಲ್ಯ_3)
#define valraw(o)	(val_(o))


/* raw type tag of a TValue */
#define rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#define novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#define withvariant(t)	((t) & 0x3F)
#define ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#define ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#define checktag(o,t)		(rawtt(o) == (t))
#define checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#define righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#define checkliveness(L,obj) \
	((ಶೂನ್ಯ)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == ರಿಕ್ತ || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#define settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ಧ್ರುವ TValue *io2=(obj2); \
          io1->ಮೌಲ್ಯ_3 = io2->ಮೌಲ್ಯ_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#define setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#define setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#define setobjt2t	setobj
/* to new object */
#define setobj2n	setobj
/* to table */
#define setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ ರಾಶಿ_ಮೌಲ್ಯ {
  TValue val;
  ಸಂರಚನಾ {
    TValuefields;
    unsigned short delta;
  } tbclist;
} ರಾಶಿ_ಮೌಲ್ಯ;


/* index to stack elements */
ಪ್ರಕಾರನಿರ್ದೇಶ ರಾಶಿ_ಮೌಲ್ಯ *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ {
  StkId p;  /* actual pointer */
  ptrdiff_t offset;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#define s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#define LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#define LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#define LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#define LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#define ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#define tagisempty(tag)		(novariant(tag) == LUA_TNIL)


/* macro to test for a standard nil */
#define ttisstrictnil(o)	checktag((o), LUA_VNIL)


#define setnilvalue(obj)	settt_(obj, LUA_VNIL)
#define setnilvalue2s(stk)	setnilvalue(s2v(stk))


#define isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#define isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#define isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#define ABSTKEYCONSTANT		{ರಿಕ್ತ}, LUA_VABSTKEY


/* mark an entry as empty */
#define setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#define LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#define LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#define ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#define ttisfalse(o)		checktag((o), LUA_VFALSE)
#define ttistrue(o)		checktag((o), LUA_VTRUE)


#define l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#define tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#define setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#define setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#define LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#define ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#define setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#define setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#define CommonHeader	ಸಂರಚನಾ GCObject *next; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ GCObject {
  CommonHeader;
} GCObject;


/* Bit mark for collectable types */
#define BIT_ISCOLLECTABLE	(1 << 6)

#define iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)

/* mark a tag as collectable */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)

#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#define gcvalueraw(v)	((v).gc)

#define setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#define LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#define LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_VNUMINT)

#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_ಅಂಕೆ(ivalue(o)) : fltvalue(o)))
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#define fltvalueraw(v)	((v).n)
#define ivalueraw(v)	((v).i)

#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#define chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#define chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#define LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#define LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#define tsvalueraw(v)	(gco2ts((v).gc))

#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#define setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#define setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#define setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#define LSTRREG		-1  /* regular long string */
#define LSTRFIX		-2  /* fixed external long string */
#define LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TString {
  CommonHeader;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  unsigned ಪೂರ್ಣಾಙ್ಕ hash;
  ಸಙ್ಘ {
    size_t lnglen;  /* length for long strings */
    ಸಂರಚನಾ TString *hnext;  /* linked list for hash table */
  } u;
  ವರ್ಣ *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  ಶೂನ್ಯ *ud;  /* user data for external strings */
} TString;


#define strisshr(ts)	((ts)->shrlen >= 0)
#define isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#define rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#define getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#define getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#define getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#define tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#define getlstr(ts, len)  \
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
#define LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#define LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#define ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#define pvalueraw(v)	((v).p)

#define setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#define setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Udata {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ಸಂರಚನಾ Table *metatable;
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Udata0 {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ಸಂರಚನಾ Table *metatable;
  ಸಙ್ಘ {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#define udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (ಪರಿಮಾಣ(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#define getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#define sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#define LUA_VPROTO	makevariant(LUA_TPROTO, 0)


ಪ್ರಕಾರನಿರ್ದೇಶ l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte kind;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ LocVar {
  TString *varname;
  ಪೂರ್ಣಾಙ್ಕ startpc;  /* first point where variable is active */
  ಪೂರ್ಣಾಙ್ಕ endpc;    /* first point where variable is dead */
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Abs_ಸಾಲು_Info {
  ಪೂರ್ಣಾಙ್ಕ pc;
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು;
} Abs_ಸಾಲು_Info;


/*
** Flags in Prototypes
*/
#define PF_VAHID	1  /* function has hidden vararg arguments */
#define PF_VATAB	2  /* function has vararg table */
#define PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#define isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#define needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Proto {
  CommonHeader;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  ಪೂರ್ಣಾಙ್ಕ sizeupvalues;  /* size of 'upvalues' */
  ಪೂರ್ಣಾಙ್ಕ sizek;  /* size of 'k' */
  ಪೂರ್ಣಾಙ್ಕ sizecode;
  ಪೂರ್ಣಾಙ್ಕ sizelineinfo;
  ಪೂರ್ಣಾಙ್ಕ sizep;  /* size of 'p' */
  ಪೂರ್ಣಾಙ್ಕ sizelocvars;
  ಪೂರ್ಣಾಙ್ಕ sizeabslineinfo;  /* size of 'abslineinfo' */
  ಪೂರ್ಣಾಙ್ಕ linedefined;  /* debug information  */
  ಪೂರ್ಣಾಙ್ಕ lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *code;  /* opcodes */
  ಸಂರಚನಾ Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_ಸಾಲು_Info *abslineinfo;  /* idem */
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

#define LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#define LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#define LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#define LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#define ttislcf(o)		checktag((o), LUA_VLCF)
#define ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#define ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#define isLfunction(o)	ttisLclosure(o)

#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#define fvalueraw(v)	((v).f)

#define setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#define setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#define setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#define setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ UpVal {
  CommonHeader;
  ಸಙ್ಘ {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t offset;  /* used while the stack is being reallocated */
  } v;
  ಸಙ್ಘ {
    ಸಂರಚನಾ {  /* (when open) */
      ಸಂರಚನಾ UpVal *next;  /* linked list */
      ಸಂರಚನಾ UpVal **previous;
    } open;
    TValue ಮೌಲ್ಯ;  /* the value (when closed) */
  } u;
} UpVal;



#define ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ LClosure {
  ClosureHeader;
  ಸಂರಚನಾ Proto *p;
  UpVal *upvals[1];  /* list of upvalues */
} LClosure;


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ Closure {
  CClosure c;
  LClosure l;
} Closure;


#define getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#define LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#define ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#define sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#define sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ ಗಂಟು_2 {
  ಸಂರಚನಾ ಗಂಟು_ಕೀಲಿ {
    TValuefields;  /* fields for value */
    lu_byte ಕೀಲಿ_tt;  /* key type */
    ಪೂರ್ಣಾಙ್ಕ next;  /* for chaining */
    ಮೌಲ್ಯ_2 ಕೀಲಿ_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} ಗಂಟು_2;


/* copy a value into a key */
#define setnodekey(ಗಂಟು,obj) \
	{ ಗಂಟು_2 *n_=(ಗಂಟು); ಧ್ರುವ TValue *io_=(obj); \
	  n_->u.ಕೀಲಿ_val = io_->ಮೌಲ್ಯ_3; n_->u.ಕೀಲಿ_tt = io_->tt_; }


/* copy a value from a key */
#define getnodekey(L,obj,ಗಂಟು) \
	{ TValue *io_=(obj); ಧ್ರುವ ಗಂಟು_2 *n_=(ಗಂಟು); \
	  io_->ಮೌಲ್ಯ_3 = n_->u.ಕೀಲಿ_val; io_->tt_ = n_->u.ಕೀಲಿ_tt; \
	  checkliveness(L,io_); }



ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  unsigned ಪೂರ್ಣಾಙ್ಕ asize;  /* number of slots in 'array' array */
  ಮೌಲ್ಯ_2 *array;  /* array part */
  ಗಂಟು_2 *ಗಂಟು;
  ಸಂರಚನಾ Table *metatable;
  GCObject *gclist;
} Table;


/*
** Macros to manipulate keys inserted in nodes
*/
#define keytt(ಗಂಟು)		((ಗಂಟು)->u.ಕೀಲಿ_tt)
#define keyval(ಗಂಟು)		((ಗಂಟು)->u.ಕೀಲಿ_val)

#define keyisnil(ಗಂಟು)		(keytt(ಗಂಟು) == LUA_TNIL)
#define keyisinteger(ಗಂಟು)	(keytt(ಗಂಟು) == LUA_VNUMINT)
#define keyival(ಗಂಟು)		(keyval(ಗಂಟು).i)
#define keyisshrstr(ಗಂಟು)	(keytt(ಗಂಟು) == ctb(LUA_VSHRSTR))
#define keystrval(ಗಂಟು)		(gco2ts(keyval(ಗಂಟು).gc))

#define setnilkey(ಗಂಟು)		(keytt(ಗಂಟು) = LUA_TNIL)

#define keyiscollectable(n)	(keytt(n) & BIT_ISCOLLECTABLE)

#define gckey(n)	(keyval(n).gc)
#define gckeyN(n)	(keyiscollectable(n) ? gckey(n) : ರಿಕ್ತ)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#define setdeadkey(ಗಂಟು)	(keytt(ಗಂಟು) = LUA_TDEADKEY)
#define keyisdead(ಗಂಟು)		(keytt(ಗಂಟು) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast_uint(s) & cast_uint((size)-1))))


#define twoto(x)	(1u<<(x))
#define sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#define pushvfstring(L, argp, fmt, ಸಂದೇಶ)	\
  { va_start(argp, fmt); \
  ಸಂದೇಶ = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaO_utf8esc (ವರ್ಣ *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (unsigned ಪೂರ್ಣಾಙ್ಕ x);
LUAI_FUNC lu_byte luaO_codeparam (unsigned ಪೂರ್ಣಾಙ್ಕ p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaO_rawarith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, ಧ್ರುವ TValue *p1,
                             ಧ್ರುವ TValue *p2, TValue *res);
LUAI_FUNC ಶೂನ್ಯ luaO_arith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, ಧ್ರುವ TValue *p1,
                           ಧ್ರುವ TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ಧ್ರುವ ವರ್ಣ *s, TValue *o);
LUAI_FUNC unsigned luaO_tostringbuff (ಧ್ರುವ TValue *obj, ವರ್ಣ *buff);
LUAI_FUNC lu_byte luaO_hexavalue (ಪೂರ್ಣಾಙ್ಕ c);
LUAI_FUNC ಶೂನ್ಯ luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ಧ್ರುವ ವರ್ಣ *luaO_pushvfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt,
                                                       va_list argp);
LUAI_FUNC ಧ್ರುವ ವರ್ಣ *luaO_pushfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...);
LUAI_FUNC ಶೂನ್ಯ luaO_chunkid (ವರ್ಣ *out, ಧ್ರುವ ವರ್ಣ *source, size_t srclen);


#endif

