/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lobject_h
#ವ್ಯಾಖ್ಯಾನಿಸು lobject_h


#ಅನ್ತರ್ಭಾವ <stdarg.h>


#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lua.h"


/*
** Extra types for collectable non-values
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#ವ್ಯಾಖ್ಯಾನಿಸು makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ ಮೌಲ್ಯ_2 {
  ರಚನೆ GCObject *gc;    /* collectable objects */
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

#ವ್ಯಾಖ್ಯಾನಿಸು TValuefields	ಮೌಲ್ಯ_2 ಮೌಲ್ಯ_3; lu_byte tt_

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TValue {
  TValuefields;
} TValue;


#ವ್ಯಾಖ್ಯಾನಿಸು val_(o)		((o)->ಮೌಲ್ಯ_3)
#ವ್ಯಾಖ್ಯಾನಿಸು valraw(o)	(val_(o))


/* raw type tag of a TValue */
#ವ್ಯಾಖ್ಯಾನಿಸು rawtt(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#ವ್ಯಾಖ್ಯಾನಿಸು novariant(t)	((t) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
#ವ್ಯಾಖ್ಯಾನಿಸು withvariant(t)	((t) & 0x3F)
#ವ್ಯಾಖ್ಯಾನಿಸು ttypetag(o)	withvariant(rawtt(o))

/* type of a TValue */
#ವ್ಯಾಖ್ಯಾನಿಸು ttype(o)	(novariant(rawtt(o)))


/* Macros to test type */
#ವ್ಯಾಖ್ಯಾನಿಸು checktag(o,t)		(rawtt(o) == (t))
#ವ್ಯಾಖ್ಯಾನಿಸು checktype(o,t)		(ttype(o) == (t))


/* Macros for internal tests */

/* collectable object has the same tag as the original value */
#ವ್ಯಾಖ್ಯಾನಿಸು righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು checkliveness(L,obj) \
	((ಶೂನ್ಯ)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == ಖಾಲಿ || !isdead(G(L),gcvalue(obj))))))


/* Macros to set values */

/* set a value's tag */
#ವ್ಯಾಖ್ಯಾನಿಸು settt_(o,t)	((o)->tt_=(t))


/* main macro to copy values (from 'obj2' to 'obj1') */
#ವ್ಯಾಖ್ಯಾನಿಸು setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); ಸ್ಥಿರಾಂಕ TValue *io2=(obj2); \
          io1->ಮೌಲ್ಯ_3 = io2->ಮೌಲ್ಯ_3; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/

/* from stack to stack */
#ವ್ಯಾಖ್ಯಾನಿಸು setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) */
#ವ್ಯಾಖ್ಯಾನಿಸು setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table */
#ವ್ಯಾಖ್ಯಾನಿಸು setobjt2t	setobj
/* to new object */
#ವ್ಯಾಖ್ಯಾನಿಸು setobj2n	setobj
/* to table */
#ವ್ಯಾಖ್ಯಾನಿಸು setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ ರಾಶಿ_ಮೌಲ್ಯ {
  TValue val;
  ರಚನೆ {
    TValuefields;
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ delta;
  } tbclist;
} ರಾಶಿ_ಮೌಲ್ಯ;


/* index to stack elements */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಾಶಿ_ಮೌಲ್ಯ *StkId;


/*
** When reallocating the stack, change all pointers to the stack into
** proper offsets.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ {
  StkId p;  /* actual pointer */
  ptrdiff_t ಸ್ಥಾನಾಂತರ;  /* used while the stack is being reallocated */
} StkIdRel;


/* convert a 'StackValue' to a 'TValue' */
#ವ್ಯಾಖ್ಯಾನಿಸು s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VABSTKEY	makevariant(LUA_TNIL, 2)

/* Special variant to signal that a fast get is accessing a non-table */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VNOTABLE    makevariant(LUA_TNIL, 3)


/* macro to test for (any kind of) nil */
#ವ್ಯಾಖ್ಯಾನಿಸು ttisnil(v)		checktype((v), LUA_TNIL)

/*
** Macro to test the result of a table access. Formally, it should
** distinguish between LUA_VEMPTY/LUA_VABSTKEY/LUA_VNOTABLE and
** other tags. As currently nil is equivalent to LUA_VEMPTY, it is
** simpler to just test whether the value is nil.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು tagisempty(ಟ್ಯಾಗ್)		(novariant(ಟ್ಯಾಗ್) == LUA_TNIL)


/* macro to test for a standard nil */
#ವ್ಯಾಖ್ಯಾನಿಸು ttisstrictnil(o)	checktag((o), LUA_VNIL)


#ವ್ಯಾಖ್ಯಾನಿಸು setnilvalue(obj)	settt_(obj, LUA_VNIL)
#ವ್ಯಾಖ್ಯಾನಿಸು setnilvalue2s(stk)	setnilvalue(s2v(stk))


#ವ್ಯಾಖ್ಯಾನಿಸು isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು isempty(v)		ttisnil(v)


/* macro defining a value corresponding to an absent key */
#ವ್ಯಾಖ್ಯಾನಿಸು ABSTKEYCONSTANT		{ಖಾಲಿ}, LUA_VABSTKEY


/* mark an entry as empty */
#ವ್ಯಾಖ್ಯಾನಿಸು setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#ವ್ಯಾಖ್ಯಾನಿಸು ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisfalse(o)		checktag((o), LUA_VFALSE)
#ವ್ಯಾಖ್ಯಾನಿಸು ttistrue(o)		checktag((o), LUA_VTRUE)


#ವ್ಯಾಖ್ಯಾನಿಸು l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
#ವ್ಯಾಖ್ಯಾನಿಸು tagisfalse(t)	((t) == LUA_VFALSE || novariant(t) == LUA_TNIL)



#ವ್ಯಾಖ್ಯಾನಿಸು setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#ವ್ಯಾಖ್ಯಾನಿಸು setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads
** ===================================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#ವ್ಯಾಖ್ಯಾನಿಸು ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#ವ್ಯಾಖ್ಯಾನಿಸು thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#ವ್ಯಾಖ್ಯಾನಿಸು setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

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
#ವ್ಯಾಖ್ಯಾನಿಸು Common_ಶೀರ್ಷಿಕೆ	ರಚನೆ GCObject *ಮುಂದಿನ; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ GCObject {
  Common_ಶೀರ್ಷಿಕೆ;
} GCObject;


/* Bit mark for collectable types */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಿಟ್_ISCOLLECTABLE	(1 << 6)

#ವ್ಯಾಖ್ಯಾನಿಸು iscollectable(o)	(rawtt(o) & ಬಿಟ್_ISCOLLECTABLE)

/* mark a tag as collectable */
#ವ್ಯಾಖ್ಯಾನಿಸು ctb(t)			((t) | ಬಿಟ್_ISCOLLECTABLE)

#ವ್ಯಾಖ್ಯಾನಿಸು gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#ವ್ಯಾಖ್ಯಾನಿಸು gcvalueraw(v)	((v).gc)

#ವ್ಯಾಖ್ಯಾನಿಸು setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

#ವ್ಯಾಖ್ಯಾನಿಸು ttisnumber(o)		checktype((o), LUA_TNUMBER)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisinteger(o)		checktag((o), LUA_VNUMINT)

#ವ್ಯಾಖ್ಯಾನಿಸು nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_ಅಂಕೆ(ivalue(o)) : fltvalue(o)))
#ವ್ಯಾಖ್ಯಾನಿಸು fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#ವ್ಯಾಖ್ಯಾನಿಸು ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#ವ್ಯಾಖ್ಯಾನಿಸು fltvalueraw(v)	((v).n)
#ವ್ಯಾಖ್ಯಾನಿಸು ivalueraw(v)	((v).i)

#ವ್ಯಾಖ್ಯಾನಿಸು setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#ವ್ಯಾಖ್ಯಾನಿಸು chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#ವ್ಯಾಖ್ಯಾನಿಸು setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#ವ್ಯಾಖ್ಯಾನಿಸು chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

#ವ್ಯಾಖ್ಯಾನಿಸು ttisstring(o)		checktype((o), LUA_TSTRING)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#ವ್ಯಾಖ್ಯಾನಿಸು ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#ವ್ಯಾಖ್ಯಾನಿಸು tsvalueraw(v)	(gco2ts((v).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack */
#ವ್ಯಾಖ್ಯಾನಿಸು setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object */
#ವ್ಯಾಖ್ಯಾನಿಸು setsvalue2n	setsvalue


/* Kinds of long strings (stored in 'shrlen') */
#ವ್ಯಾಖ್ಯಾನಿಸು LSTRREG		-1  /* regular long string */
#ವ್ಯಾಖ್ಯಾನಿಸು LSTRFIX		-2  /* fixed external long string */
#ವ್ಯಾಖ್ಯಾನಿಸು LSTRMEM		-3  /* external long string with deallocation */


/*
** Header for a string value.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TString {
  Common_ಶೀರ್ಷಿಕೆ;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  ls_byte shrlen;  /* length for short strings, negative for long strings */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ hash;
  ಒಕ್ಕೂಟ {
    size_t lnglen;  /* length for long strings */
    ರಚನೆ TString *hnext;  /* linked list for hash table */
  } u;
  ಅಕ್ಷರ *contents;  /* pointer to content in long strings */
  lua_Alloc falloc;  /* deallocation function for external strings */
  ಶೂನ್ಯ *ud;  /* user data for external strings */
} TString;


#ವ್ಯಾಖ್ಯಾನಿಸು strisshr(ts)	((ts)->shrlen >= 0)
#ವ್ಯಾಖ್ಯಾನಿಸು isextstr(ts)	(ttislngstring(ts) && tsvalue(ts)->shrlen != LSTRREG)


/*
** Get the actual string (array of bytes) from a 'TString'. (Generic
** version and specialized versions for long and short strings.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು rawgetshrstr(ts)  (cast_charp(&(ts)->contents))
#ವ್ಯಾಖ್ಯಾನಿಸು getshrstr(ts)	check_exp(strisshr(ts), rawgetshrstr(ts))
#ವ್ಯಾಖ್ಯಾನಿಸು getlngstr(ts)	check_exp(!strisshr(ts), (ts)->contents)
#ವ್ಯಾಖ್ಯಾನಿಸು getstr(ts) 	(strisshr(ts) ? rawgetshrstr(ts) : (ts)->contents)


/* get string length from 'TString *ts' */
#ವ್ಯಾಖ್ಯಾನಿಸು tsslen(ts)  \
	(strisshr(ts) ? cast_sizet((ts)->shrlen) : (ts)->u.lnglen)

/*
** Get string and length */
#ವ್ಯಾಖ್ಯಾನಿಸು getlstr(ts, len)  \
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
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#ವ್ಯಾಖ್ಯಾನಿಸು ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#ವ್ಯಾಖ್ಯಾನಿಸು pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#ವ್ಯಾಖ್ಯಾನಿಸು uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು pvalueraw(v)	((v).p)

#ವ್ಯಾಖ್ಯಾನಿಸು setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#ವ್ಯಾಖ್ಯಾನಿಸು setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* Ensures that addresses after this type are always fully aligned. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Udata {
  Common_ಶೀರ್ಷಿಕೆ;
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ರಚನೆ ಕೋಷ್ಟಕ *metatable;
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Udata0 {
  Common_ಶೀರ್ಷಿಕೆ;
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  ರಚನೆ ಕೋಷ್ಟಕ *metatable;
  ಒಕ್ಕೂಟ {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
#ವ್ಯಾಖ್ಯಾನಿಸು udatamemoffset(nuv) \
       ((nuv) == 0 ? offsetof(Udata0, bindata)  \
		   : offsetof(Udata, uv) + (ಗಾತ್ರ(UValue) * (nuv)))

/* get the address of the memory block inside 'Udata' */
#ವ್ಯಾಖ್ಯಾನಿಸು getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata */
#ವ್ಯಾಖ್ಯಾನಿಸು sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VPROTO	makevariant(LUA_TPROTO, 0)


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ l_uint32 Instruction;


/*
** Description of an upvalue for function prototypes
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte ಬಗೆ;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ LocVar {
  TString *varname;
  ಪೂರ್ಣಾಂಕ startpc;  /* first point where variable is active */
  ಪೂರ್ಣಾಂಕ endpc;    /* first point where variable is dead */
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Abs_ಸಾಲು_Info {
  ಪೂರ್ಣಾಂಕ pc;
  ಪೂರ್ಣಾಂಕ ಸಾಲು;
} Abs_ಸಾಲು_Info;


/*
** Flags in Prototypes
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PF_VAHID	1  /* function has hidden vararg arguments */
#ವ್ಯಾಖ್ಯಾನಿಸು PF_VATAB	2  /* function has vararg table */
#ವ್ಯಾಖ್ಯಾನಿಸು PF_FIXED	4  /* prototype has parts in fixed memory */

/* a vararg function either has hidden args. or a vararg table */
#ವ್ಯಾಖ್ಯಾನಿಸು isvararg(p)	((p)->flag & (PF_VAHID | PF_VATAB))

/*
** mark that a function needs a vararg table. (The flag PF_VAHID will
** be cleared later.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು needvatab(p)	((p)->flag |= PF_VATAB)

/*
** Function Prototypes
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Proto {
  Common_ಶೀರ್ಷಿಕೆ;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte flag;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  ಪೂರ್ಣಾಂಕ sizeupvalues;  /* size of 'upvalues' */
  ಪೂರ್ಣಾಂಕ sizek;  /* size of 'k' */
  ಪೂರ್ಣಾಂಕ sizecode;
  ಪೂರ್ಣಾಂಕ sizelineinfo;
  ಪೂರ್ಣಾಂಕ sizep;  /* size of 'p' */
  ಪೂರ್ಣಾಂಕ sizelocvars;
  ಪೂರ್ಣಾಂಕ sizeabslineinfo;  /* size of 'abslineinfo' */
  ಪೂರ್ಣಾಂಕ linedefined;  /* debug information  */
  ಪೂರ್ಣಾಂಕ lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *ಸಂಕೇತ_2;  /* opcodes */
  ರಚನೆ Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  Abs_ಸಾಲು_Info *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *ಮೂಲ;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions
** ===================================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

#ವ್ಯಾಖ್ಯಾನಿಸು ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#ವ್ಯಾಖ್ಯಾನಿಸು ttislcf(o)		checktag((o), LUA_VLCF)
#ವ್ಯಾಖ್ಯಾನಿಸು ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#ವ್ಯಾಖ್ಯಾನಿಸು ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#ವ್ಯಾಖ್ಯಾನಿಸು isLfunction(o)	ttisLclosure(o)

#ವ್ಯಾಖ್ಯಾನಿಸು clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#ವ್ಯಾಖ್ಯಾನಿಸು clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#ವ್ಯಾಖ್ಯಾನಿಸು fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#ವ್ಯಾಖ್ಯಾನಿಸು clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು fvalueraw(v)	((v).f)

#ವ್ಯಾಖ್ಯಾನಿಸು setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#ವ್ಯಾಖ್ಯಾನಿಸು setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#ವ್ಯಾಖ್ಯಾನಿಸು setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#ವ್ಯಾಖ್ಯಾನಿಸು setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಮೇಲೆ_Val {
  Common_ಶೀರ್ಷಿಕೆ;
  ಒಕ್ಕೂಟ {
    TValue *p;  /* points to stack or to its own value */
    ptrdiff_t ಸ್ಥಾನಾಂತರ;  /* used while the stack is being reallocated */
  } v;
  ಒಕ್ಕೂಟ {
    ರಚನೆ {  /* (when open) */
      ರಚನೆ ಮೇಲೆ_Val *ಮುಂದಿನ;  /* linked list */
      ರಚನೆ ಮೇಲೆ_Val **ಹಿಂದಿನ;
    } open;
    TValue ಮೌಲ್ಯ;  /* the value (when closed) */
  } u;
} ಮೇಲೆ_Val;



#ವ್ಯಾಖ್ಯಾನಿಸು ಆವರಣ_ಶೀರ್ಷಿಕೆ \
	Common_ಶೀರ್ಷಿಕೆ; lu_byte nupvalues; GCObject *gclist

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CClosure {
  ಆವರಣ_ಶೀರ್ಷಿಕೆ;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ LClosure {
  ಆವರಣ_ಶೀರ್ಷಿಕೆ;
  ರಚನೆ Proto *p;
  ಮೇಲೆ_Val *upvals[1];  /* list of upvalues */
} LClosure;


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ ಆವರಣ {
  CClosure c;
  LClosure l;
} ಆವರಣ;


#ವ್ಯಾಖ್ಯಾನಿಸು getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#ವ್ಯಾಖ್ಯಾನಿಸು ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#ವ್ಯಾಖ್ಯಾನಿಸು hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು sethvalue(L,obj,x) \
  { TValue *io = (obj); ಕೋಷ್ಟಕ *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#ವ್ಯಾಖ್ಯಾನಿಸು sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ ಗಂಟು_2 {
  ರಚನೆ ಗಂಟು_ಕೀಲಿ {
    TValuefields;  /* fields for value */
    lu_byte ಕೀಲಿ_tt;  /* key type */
    ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;  /* for chaining */
    ಮೌಲ್ಯ_2 ಕೀಲಿ_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} ಗಂಟು_2;


/* copy a value into a key */
#ವ್ಯಾಖ್ಯಾನಿಸು setnodekey(ಗಂಟು,obj) \
	{ ಗಂಟು_2 *n_=(ಗಂಟು); ಸ್ಥಿರಾಂಕ TValue *io_=(obj); \
	  n_->u.ಕೀಲಿ_val = io_->ಮೌಲ್ಯ_3; n_->u.ಕೀಲಿ_tt = io_->tt_; }


/* copy a value from a key */
#ವ್ಯಾಖ್ಯಾನಿಸು getnodekey(L,obj,ಗಂಟು) \
	{ TValue *io_=(obj); ಸ್ಥಿರಾಂಕ ಗಂಟು_2 *n_=(ಗಂಟು); \
	  io_->ಮೌಲ್ಯ_3 = n_->u.ಕೀಲಿ_val; io_->tt_ = n_->u.ಕೀಲಿ_tt; \
	  checkliveness(L,io_); }



ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕೋಷ್ಟಕ {
  Common_ಶೀರ್ಷಿಕೆ;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of number of slots of 'node' array */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize;  /* number of slots in 'array' array */
  ಮೌಲ್ಯ_2 *array;  /* array part */
  ಗಂಟು_2 *ಗಂಟು;
  ರಚನೆ ಕೋಷ್ಟಕ *metatable;
  GCObject *gclist;
} ಕೋಷ್ಟಕ;


/*
** Macros to manipulate keys inserted in nodes
*/
#ವ್ಯಾಖ್ಯಾನಿಸು keytt(ಗಂಟು)		((ಗಂಟು)->u.ಕೀಲಿ_tt)
#ವ್ಯಾಖ್ಯಾನಿಸು keyval(ಗಂಟು)		((ಗಂಟು)->u.ಕೀಲಿ_val)

#ವ್ಯಾಖ್ಯಾನಿಸು keyisnil(ಗಂಟು)		(keytt(ಗಂಟು) == LUA_TNIL)
#ವ್ಯಾಖ್ಯಾನಿಸು keyisinteger(ಗಂಟು)	(keytt(ಗಂಟು) == LUA_VNUMINT)
#ವ್ಯಾಖ್ಯಾನಿಸು keyival(ಗಂಟು)		(keyval(ಗಂಟು).i)
#ವ್ಯಾಖ್ಯಾನಿಸು keyisshrstr(ಗಂಟು)	(keytt(ಗಂಟು) == ctb(LUA_VSHRSTR))
#ವ್ಯಾಖ್ಯಾನಿಸು keystrval(ಗಂಟು)		(gco2ts(keyval(ಗಂಟು).gc))

#ವ್ಯಾಖ್ಯಾನಿಸು setnilkey(ಗಂಟು)		(keytt(ಗಂಟು) = LUA_TNIL)

#ವ್ಯಾಖ್ಯಾನಿಸು keyiscollectable(n)	(keytt(n) & ಬಿಟ್_ISCOLLECTABLE)

#ವ್ಯಾಖ್ಯಾನಿಸು gckey(n)	(keyval(n).gc)
#ವ್ಯಾಖ್ಯಾನಿಸು gckeyN(n)	(keyiscollectable(n) ? gckey(n) : ಖಾಲಿ)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು setdeadkey(ಗಂಟು)	(keytt(ಗಂಟು) = LUA_TDEADKEY)
#ವ್ಯಾಖ್ಯಾನಿಸು keyisdead(ಗಂಟು)		(keytt(ಗಂಟು) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lmod(s,ಗಾತ್ರ_2) \
	(check_exp((ಗಾತ್ರ_2&(ಗಾತ್ರ_2-1))==0, (cast_uint(s) & cast_uint((ಗಾತ್ರ_2)-1))))


#ವ್ಯಾಖ್ಯಾನಿಸು twoto(x)	(1u<<(x))
#ವ್ಯಾಖ್ಯಾನಿಸು sizenode(t)	cast_uint(twoto((t)->lsizenode))


/* size of buffer for 'luaO_utf8esc' function */
#ವ್ಯಾಖ್ಯಾನಿಸು UTF8BUFFSZ	8


/* macro to call 'luaO_pushvfstring' correctly */
#ವ್ಯಾಖ್ಯಾನಿಸು pushvfstring(L, argp, fmt, ಸಂದೇಶ)	\
  { va_start(argp, fmt); \
  ಸಂದೇಶ = luaO_pushvfstring(L, fmt, argp); \
  va_end(argp); \
  ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ) luaD_throw(L, LUA_ERRMEM);  /* only after 'va_end' */ }


LUAI_FUNC ಪೂರ್ಣಾಂಕ luaO_utf8esc (ಅಕ್ಷರ *buff, l_uint32 x);
LUAI_FUNC lu_byte luaO_ceillog2 (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x);
LUAI_FUNC lu_byte luaO_codeparam (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ p);
LUAI_FUNC l_mem luaO_applyparam (lu_byte p, l_mem x);

LUAI_FUNC ಪೂರ್ಣಾಂಕ luaO_rawarith (lua_State *L, ಪೂರ್ಣಾಂಕ op, ಸ್ಥಿರಾಂಕ TValue *p1,
                             ಸ್ಥಿರಾಂಕ TValue *p2, TValue *res);
LUAI_FUNC ಶೂನ್ಯ luaO_arith (lua_State *L, ಪೂರ್ಣಾಂಕ op, ಸ್ಥಿರಾಂಕ TValue *p1,
                           ಸ್ಥಿರಾಂಕ TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, TValue *o);
LUAI_FUNC ಚಿಹ್ನೆರಹಿತ luaO_tostringbuff (ಸ್ಥಿರಾಂಕ TValue *obj, ಅಕ್ಷರ *buff);
LUAI_FUNC lu_byte luaO_hexavalue (ಪೂರ್ಣಾಂಕ c);
LUAI_FUNC ಶೂನ್ಯ luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaO_pushvfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt,
                                                       va_list argp);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaO_pushfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);
LUAI_FUNC ಶೂನ್ಯ luaO_chunkid (ಅಕ್ಷರ *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, size_t srclen);


#ಆದರೆ_ಅಂತ್ಯ

