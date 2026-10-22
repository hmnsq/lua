/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده ltable_h
#تعریف_کن ltable_h

#ضمن "lobject.h"


#تعریف_کن gnode(t,i)	(&(t)->گره[i])
#تعریف_کن gval(n)		(&(n)->i_val)
#تعریف_کن gnext(n)	((n)->u.بعدی)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#تعریف_کن invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#تعریف_کن BITDUMMY		(1 << 6)
#تعریف_کن NOTBITDUMMY		cast_byte(~BITDUMMY)
#تعریف_کن isdummy(t)		((t)->flags & BITDUMMY)

#تعریف_کن setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#تعریف_کن setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#تعریف_کن allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#تعریف_کن nodefromval(v)	cast(گره_2 *, (v))



#تعریف_کن luaH_fastgeti(t,k,res,برچسب) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    إذا ((u < h->asize)) { \
      برچسب = *get_Arr_برچسب(h, u); \
      إذا (!tagisempty(برچسب)) { farr2val(h, u, برچسب, res); }} \
    وإلا { برچسب = luaH_getint(h, (k), res); }}


#تعریف_کن luaH_fastseti(t,k,val,hres) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    إذا ((u < h->asize)) { \
      lu_byte *برچسب = get_Arr_برچسب(h, u); \
      إذا (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*برچسب)) \
        { fval2arr(h, u, برچسب, val); hres = HOK; } \
      وإلا hres = ~cast_int(u); } \
    وإلا { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#تعریف_کن HOK		0
#تعریف_کن HNOTFOUND	1
#تعریف_کن HNOTATABLE	2
#تعریف_کن HFIRSTNODE	3

/*
** 'luaH_get*' operations set 'res', unless the value is absent, and
** return the tag of the result.
** The 'luaH_pset*' (pre-set) operations set the given value and return
** HOK, unless the original value is absent. In that case, if the key
** is really absent, they return HNOTFOUND. Otherwise, if there is a
** slot with that key but with no value, 'luaH_pset*' return an encoding
** of where the key is (usually called 'hres'). (pset cannot set that
** value because there might be a metamethod.) If the slot is in the
** hash part, the encoding is (HFIRSTNODE + hash index); if the slot is
** in the array part, the encoding is (~array index), a negative value.
** The value HNOTATABLE is used by the fast macros to signal that the
** value being indexed is not a table.
** (The size for the array part is limited by the maximum power of two
** that fits in an unsigned integer; that is INT_MAX+1. So, the C-index
** ranges from 0, which encodes to -1, to INT_MAX, which encodes to
** INT_MIN. The size of the hash part is limited by the maximum power of
** two that fits in a signed integer; that is (INT_MAX+1)/2. So, it is
** safe to add HFIRSTNODE to any index there.)
*/


/*
** The array part of a table is represented by an inverted array of
** values followed by an array of tags, to avoid wasting space with
** padding. In between them there is an unsigned int, explained later.
** The 'array' pointer points between the two arrays, so that values are
** indexed with negative indices and tags with non-negative indices.

             Values                              Tags
  --------------------------------------------------------
  ...  |   Value 1     |   Value 0     |unsigned|0|1|...
  --------------------------------------------------------
                                       ^ t->array

** All accesses to 't->array' should be through the macros 'getArrTag'
** and 'getArrVal'.
*/

/* Computes the address of the tag for the abstract C-index 'k' */
#تعریف_کن get_Arr_برچسب(t,k)	(cast(lu_byte*, (t)->array) + حجم(بیعلامت) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#تعریف_کن getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#تعریف_کن lenhint(t)	cast(بیعلامت*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#تعریف_کن arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_برچسب(h,(k)), (val)->مقدار_3 = *getArrVal(h,(k)))

#تعریف_کن obj2arr(h,k,val)  \
  (*get_Arr_برچسب(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->مقدار_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#تعریف_کن farr2val(h,k,برچسب,res)  \
  ((res)->tt_ = برچسب, (res)->مقدار_3 = *getArrVal(h,(k)))

#تعریف_کن fval2arr(h,k,برچسب,val)  \
  (*برچسب = (val)->tt_, *getArrVal(h,(k)) = (val)->مقدار_3)


LUAI_FUNC lu_byte luaH_get (جدول *t, ثابت TValue *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (جدول *t, TString *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (جدول *t, TString *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getint (جدول *t, lua_Integer کلید, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ثابت TValue *luaH_Hgetshortstr (جدول *t, TString *کلید);

LUAI_FUNC صحيح luaH_psetint (جدول *t, lua_Integer کلید, TValue *val);
LUAI_FUNC صحيح luaH_psetshortstr (جدول *t, TString *کلید, TValue *val);
LUAI_FUNC صحيح luaH_psetstr (جدول *t, TString *کلید, TValue *val);
LUAI_FUNC صحيح luaH_pset (جدول *t, ثابت TValue *کلید, TValue *val);

LUAI_FUNC فراغ luaH_setint (lua_State *L, جدول *t, lua_Integer کلید,
                                                    TValue *مقدار);
LUAI_FUNC فراغ lua_H_مجموعه (lua_State *L, جدول *t, ثابت TValue *کلید,
                                                 TValue *مقدار);

LUAI_FUNC فراغ luaH_finishset (lua_State *L, جدول *t, ثابت TValue *کلید,
                                              TValue *مقدار, صحيح hres);
LUAI_FUNC جدول *luaH_new (lua_State *L);
LUAI_FUNC فراغ luaH_resize (lua_State *L, جدول *t, بیعلامت nasize,
                                                    بیعلامت nhsize);
LUAI_FUNC فراغ luaH_resizearray (lua_State *L, جدول *t, بیعلامت nasize);
LUAI_FUNC lu_mem lua_H_اندازه (جدول *t);
LUAI_FUNC فراغ luaH_free (lua_State *L, جدول *t);
LUAI_FUNC صحيح lua_H_بعدی (lua_State *L, جدول *t, StkId کلید);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, جدول *t);


#إذا defined(LUA_DEBUG)
LUAI_FUNC گره_2 *luaH_mainposition (ثابت جدول *t, ثابت TValue *کلید);
#پایان_اگر


#پایان_اگر
