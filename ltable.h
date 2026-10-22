/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ ltable_h
#تعریف_کرو ltable_h

#شامل "lobject.h"


#تعریف_کرو gnode(t,i)	(&(t)->گرہ[i])
#تعریف_کرو gval(n)		(&(n)->i_val)
#تعریف_کرو gnext(n)	((n)->u.اگلا)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#تعریف_کرو invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#تعریف_کرو BITDUMMY		(1 << 6)
#تعریف_کرو NOTBITDUMMY		cast_byte(~BITDUMMY)
#تعریف_کرو isdummy(t)		((t)->flags & BITDUMMY)

#تعریف_کرو setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#تعریف_کرو setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#تعریف_کرو allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#تعریف_کرو nodefromval(v)	cast(گرہ_2 *, (v))



#تعریف_کرو luaH_fastgeti(t,k,res,لیبل) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    اگر ((u < h->asize)) { \
      لیبل = *get_Arr_لیبل(h, u); \
      اگر (!tagisempty(لیبل)) { farr2val(h, u, لیبل, res); }} \
    ورنہ { لیبل = luaH_getint(h, (k), res); }}


#تعریف_کرو luaH_fastseti(t,k,val,hres) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    اگر ((u < h->asize)) { \
      lu_byte *لیبل = get_Arr_لیبل(h, u); \
      اگر (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*لیبل)) \
        { fval2arr(h, u, لیبل, val); hres = HOK; } \
      ورنہ hres = ~cast_int(u); } \
    ورنہ { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#تعریف_کرو HOK		0
#تعریف_کرو HNOTFOUND	1
#تعریف_کرو HNOTATABLE	2
#تعریف_کرو HFIRSTNODE	3

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
#تعریف_کرو get_Arr_لیبل(t,k)	(cast(lu_byte*, (t)->array) + حجم(غیرعلامتی) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#تعریف_کرو getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#تعریف_کرو lenhint(t)	cast(غیرعلامتی*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#تعریف_کرو arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_لیبل(h,(k)), (val)->قدر_3 = *getArrVal(h,(k)))

#تعریف_کرو obj2arr(h,k,val)  \
  (*get_Arr_لیبل(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->قدر_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#تعریف_کرو farr2val(h,k,لیبل,res)  \
  ((res)->tt_ = لیبل, (res)->قدر_3 = *getArrVal(h,(k)))

#تعریف_کرو fval2arr(h,k,لیبل,val)  \
  (*لیبل = (val)->tt_, *getArrVal(h,(k)) = (val)->قدر_3)


LUAI_FUNC lu_byte luaH_get (جدول *t, ثابت TValue *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (جدول *t, TString *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (جدول *t, TString *کلید, TValue *res);
LUAI_FUNC lu_byte luaH_getint (جدول *t, lua_Integer کلید, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ثابت TValue *luaH_Hgetshortstr (جدول *t, TString *کلید);

LUAI_FUNC صحیح luaH_psetint (جدول *t, lua_Integer کلید, TValue *val);
LUAI_FUNC صحیح luaH_psetshortstr (جدول *t, TString *کلید, TValue *val);
LUAI_FUNC صحیح luaH_psetstr (جدول *t, TString *کلید, TValue *val);
LUAI_FUNC صحیح luaH_pset (جدول *t, ثابت TValue *کلید, TValue *val);

LUAI_FUNC خالی luaH_setint (lua_State *L, جدول *t, lua_Integer کلید,
                                                    TValue *قدر);
LUAI_FUNC خالی lua_H_سیٹ (lua_State *L, جدول *t, ثابت TValue *کلید,
                                                 TValue *قدر);

LUAI_FUNC خالی luaH_finishset (lua_State *L, جدول *t, ثابت TValue *کلید,
                                              TValue *قدر, صحیح hres);
LUAI_FUNC جدول *luaH_new (lua_State *L);
LUAI_FUNC خالی luaH_resize (lua_State *L, جدول *t, غیرعلامتی nasize,
                                                    غیرعلامتی nhsize);
LUAI_FUNC خالی luaH_resizearray (lua_State *L, جدول *t, غیرعلامتی nasize);
LUAI_FUNC lu_mem lua_H_سائز (جدول *t);
LUAI_FUNC خالی luaH_free (lua_State *L, جدول *t);
LUAI_FUNC صحیح lua_H_اگلا (lua_State *L, جدول *t, StkId کلید);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, جدول *t);


#اگر defined(LUA_DEBUG)
LUAI_FUNC گرہ_2 *luaH_mainposition (ثابت جدول *t, ثابت TValue *کلید);
#اگر_ختم


#اگر_ختم
