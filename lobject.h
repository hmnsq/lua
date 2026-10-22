/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lobject_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lobject_h


#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>


#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


/*
** Extra types for collectable non-values
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ ਮੁੱਲ_2 {
  ਬਣਤਰ GCObject *gc;    /* collectable objects */
  ਖਾਲੀ *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_ਸੰਖਿਆ n;    /* float numbers */
  /* not used, but may avoid warnings for uninitialized value */
  lu_byte ub;
} ਮੁੱਲ_2;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TValuefields	ਮੁੱਲ_2 ਮੁੱਲ_3; lu_byte tt_

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TValue {
  TValuefields;
} TValue;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ val_(o)		((o)->ਮੁੱਲ_3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ valraw(o)	(val_(o))


/* raw type tag of a TValue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ withvariant(t)	((t) & 0x3F)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checktag(o,t)		(rawtt(o) == (t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkliveness(L,obj) \
	((ਖਾਲੀ)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == ਨਿਰਾ || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ਅਚਲ TValue *io2=(obj2); \
          io1->ਮੁੱਲ_3 = io2->ਮੁੱਲ_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobjt2t	setobj
/* to new object */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobj2n	setobj
/* to table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ ਢੇਰ_ਮੁੱਲ {
  TValue val;
  ਬਣਤਰ {
    TValuefields;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ delta;
  } tbclist;
} ਢੇਰ_ਮੁੱਲ;


/* index to stack elements */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਢੇਰ_ਮੁੱਲ *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ {
  StkId p;  /* actual pointer */
  ptrdiff_t ਵਿੱਥ;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tagisempty(ਟੈਗ)		(novariant(ਟੈਗ) == LUA_TNIL)


/* macro to test for a standard nil */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisstrictnil(o)	checktag((o), LUA_VNIL)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setnilvalue(obj)	settt_(obj, LUA_VNIL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setnilvalue2s(stk)	setnilvalue(s2v(stk))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABSTKEYCONSTANT		{ਨਿਰਾ}, LUA_VABSTKEY


/* mark an entry as empty */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisfalse(o)		checktag((o), LUA_VFALSE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttistrue(o)		checktag((o), LUA_VTRUE)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Common_ਸਿਰਲੇਖ	ਬਣਤਰ GCObject *ਅਗਲਾ; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ GCObject {
  Common_ਸਿਰਲੇਖ;
} GCObject;


/* Bit mark for collectable types */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦੋਆਧਾਰੀ_ਅੰਕ_ISCOLLECTABLE	(1 << 6)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ iscollectable(o)	(rawtt(o) & ਦੋਆਧਾਰੀ_ਅੰਕ_ISCOLLECTABLE)

/* mark a tag as collectable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ctb(t)			((t) | ਦੋਆਧਾਰੀ_ਅੰਕ_ISCOLLECTABLE)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gcvalueraw(v)	((v).gc)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisnumber(o)		checktype((o), LUA_TNUMBER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisinteger(o)		checktag((o), LUA_VNUMINT)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_ਅੰਕ(ivalue(o)) : fltvalue(o)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fltvalueraw(v)	((v).n)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ivalueraw(v)	((v).i)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisstring(o)		checktype((o), LUA_TSTRING)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tsvalueraw(v)	(gco2ts((v).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSTRREG		-1  /* regular long string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSTRFIX		-2  /* fixed external long string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TString {
  Common_ਸਿਰਲੇਖ;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ hash;
  ਸੰਘ {
    size_t lnglen;  /* length for long strings */
    ਬਣਤਰ TString *hnext;  /* linked list for hash table */
  } u;
  ਅੱਖਰ *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  ਖਾਲੀ *ud;  /* user data for external strings */
} TString;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ strisshr(ts)	((ts)->shrlen >= 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getlstr(ts, len)  \
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pvalueraw(v)	((v).p)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Udata {
  Common_ਸਿਰਲੇਖ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ਬਣਤਰ ਸਾਰਣੀ *metatable;
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Udata0 {
  Common_ਸਿਰਲੇਖ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ਬਣਤਰ ਸਾਰਣੀ *metatable;
  ਸੰਘ {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (ਆਕਾਰ(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VPROTO	makevariant(LUA_TPROTO, 0)


ਕਿਸਮਪਰਿਭਾਸ਼ਾ l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte ਕਿਸਮ;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ LocVar {
  TString *varname;
  ਪੂਰਨ startpc;  /* first point where variable is active */
  ਪੂਰਨ endpc;    /* first point where variable is dead */
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Abs_ਸਤਰ_Info {
  ਪੂਰਨ pc;
  ਪੂਰਨ ਸਤਰ;
} Abs_ਸਤਰ_Info;


/*
** Flags in Prototypes
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_VAHID	1  /* function has hidden vararg arguments */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_VATAB	2  /* function has vararg table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Proto {
  Common_ਸਿਰਲੇਖ;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  ਪੂਰਨ sizeupvalues;  /* size of 'upvalues' */
  ਪੂਰਨ sizek;  /* size of 'k' */
  ਪੂਰਨ sizecode;
  ਪੂਰਨ sizelineinfo;
  ਪੂਰਨ sizep;  /* size of 'p' */
  ਪੂਰਨ sizelocvars;
  ਪੂਰਨ sizeabslineinfo;  /* size of 'abslineinfo' */
  ਪੂਰਨ linedefined;  /* debug information  */
  ਪੂਰਨ lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *ਕੋਡ;  /* opcodes */
  ਬਣਤਰ Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_ਸਤਰ_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *ਸਰੋਤ;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttislcf(o)		checktag((o), LUA_VLCF)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isLfunction(o)	ttisLclosure(o)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fvalueraw(v)	((v).f)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਉੱਪਰ_Val {
  Common_ਸਿਰਲੇਖ;
  ਸੰਘ {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t ਵਿੱਥ;  /* used while the stack is being reallocated */
  } v;
  ਸੰਘ {
    ਬਣਤਰ {  /* (when open) */
      ਬਣਤਰ ਉੱਪਰ_Val *ਅਗਲਾ;  /* linked list */
      ਬਣਤਰ ਉੱਪਰ_Val **ਪਿਛਲਾ;
    } open;
    TValue ਮੁੱਲ;  /* the value (when closed) */
  } u;
} ਉੱਪਰ_Val;



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਘੇਰਾ_ਸਿਰਲੇਖ \
	Common_ਸਿਰਲੇਖ; lu_byte nupvalues; GCObject *gclist

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CClosure {
  ਘੇਰਾ_ਸਿਰਲੇਖ;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ LClosure {
  ਘੇਰਾ_ਸਿਰਲੇਖ;
  ਬਣਤਰ Proto *p;
  ਉੱਪਰ_Val *upvals[1];  /* list of upvalues */
} LClosure;


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ ਘੇਰਾ {
  CClosure c;
  LClosure l;
} ਘੇਰਾ;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sethvalue(L,obj,x) \
  { TValue *io = (obj); ਸਾਰਣੀ *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ ਗੰਢ_2 {
  ਬਣਤਰ ਗੰਢ_ਕੁੰਜੀ {
    TValuefields;  /* fields for value */
    lu_byte ਕੁੰਜੀ_tt;  /* key type */
    ਪੂਰਨ ਅਗਲਾ;  /* for chaining */
    ਮੁੱਲ_2 ਕੁੰਜੀ_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} ਗੰਢ_2;


/* copy a value into a key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setnodekey(ਗੰਢ,obj) \
	{ ਗੰਢ_2 *n_=(ਗੰਢ); ਅਚਲ TValue *io_=(obj); \
	  n_->u.ਕੁੰਜੀ_val = io_->ਮੁੱਲ_3; n_->u.ਕੁੰਜੀ_tt = io_->tt_; }


/* copy a value from a key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getnodekey(L,obj,ਗੰਢ) \
	{ TValue *io_=(obj); ਅਚਲ ਗੰਢ_2 *n_=(ਗੰਢ); \
	  io_->ਮੁੱਲ_3 = n_->u.ਕੁੰਜੀ_val; io_->tt_ = n_->u.ਕੁੰਜੀ_tt; \
	  checkliveness(L,io_); }



ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸਾਰਣੀ {
  Common_ਸਿਰਲੇਖ;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize;  /* number of slots in 'array' array */
  ਮੁੱਲ_2 *array;  /* array part */
  ਗੰਢ_2 *ਗੰਢ;
  ਬਣਤਰ ਸਾਰਣੀ *metatable;
  GCObject *gclist;
} ਸਾਰਣੀ;


/*
** Macros to manipulate keys inserted in nodes
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keytt(ਗੰਢ)		((ਗੰਢ)->u.ਕੁੰਜੀ_tt)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyval(ਗੰਢ)		((ਗੰਢ)->u.ਕੁੰਜੀ_val)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyisnil(ਗੰਢ)		(keytt(ਗੰਢ) == LUA_TNIL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyisinteger(ਗੰਢ)	(keytt(ਗੰਢ) == LUA_VNUMINT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyival(ਗੰਢ)		(keyval(ਗੰਢ).i)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyisshrstr(ਗੰਢ)	(keytt(ਗੰਢ) == ctb(LUA_VSHRSTR))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keystrval(ਗੰਢ)		(gco2ts(keyval(ਗੰਢ).gc))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setnilkey(ਗੰਢ)		(keytt(ਗੰਢ) = LUA_TNIL)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyiscollectable(n)	(keytt(n) & ਦੋਆਧਾਰੀ_ਅੰਕ_ISCOLLECTABLE)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gckey(n)	(keyval(n).gc)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gckeyN(n)	(keyiscollectable(n) ? gckey(n) : ਨਿਰਾ)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setdeadkey(ਗੰਢ)	(keytt(ਗੰਢ) = LUA_TDEADKEY)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyisdead(ਗੰਢ)		(keytt(ਗੰਢ) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lmod(s,ਮਾਪ) \
	(check_exp((ਮਾਪ&(ਮਾਪ-1))==0, (cast_uint(s) & cast_uint((ਮਾਪ)-1))))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ twoto(x)	(1u<<(x))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pushvfstring(L, argp, fmt, ਸੁਨੇਹਾ)	\
  { va_start(argp, fmt); \
  ਸੁਨੇਹਾ = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  ਜੇ (ਸੁਨੇਹਾ == ਨਿਰਾ) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC ਪੂਰਨ luaO_utf8esc (ਅੱਖਰ *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x);
LUAI_FUNC lu_byte luaO_codeparam (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC ਪੂਰਨ luaO_rawarith (lua_State *L, ਪੂਰਨ op, ਅਚਲ TValue *p1,
                             ਅਚਲ TValue *p2, TValue *res);
LUAI_FUNC ਖਾਲੀ luaO_arith (lua_State *L, ਪੂਰਨ op, ਅਚਲ TValue *p1,
                           ਅਚਲ TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ਅਚਲ ਅੱਖਰ *s, TValue *o);
LUAI_FUNC ਬਿਨਾਨਿਸ਼ਾਨੀ luaO_tostringbuff (ਅਚਲ TValue *obj, ਅੱਖਰ *buff);
LUAI_FUNC lu_byte luaO_hexavalue (ਪੂਰਨ c);
LUAI_FUNC ਖਾਲੀ luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaO_pushvfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt,
                                                       va_list argp);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaO_pushfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...);
LUAI_FUNC ਖਾਲੀ luaO_chunkid (ਅੱਖਰ *ਬਾਹਰਲਾ, ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, size_t srclen);


#ਜੇ_ਅੰਤ

