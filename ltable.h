/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی ltable_h
#تعریف_کړه ltable_h

#ضمن "lobject.h"


#تعریف_کړه gnode(t,i)	(&(t)->غوټه[i])
#تعریف_کړه gval(n)		(&(n)->i_val)
#تعریف_کړه gnext(n)	((n)->u.بل)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#تعریف_کړه invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#تعریف_کړه BITDUMMY		(1 << 6)
#تعریف_کړه NOTBITDUMMY		cast_byte(~BITDUMMY)
#تعریف_کړه isdummy(t)		((t)->flags & BITDUMMY)

#تعریف_کړه setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#تعریف_کړه setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#تعریف_کړه allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#تعریف_کړه nodefromval(v)	cast(غوټه_2 *, (v))



#تعریف_کړه luaH_fastgeti(t,k,res,نښان) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    إذا ((u < h->asize)) { \
      نښان = *get_Arr_نښان(h, u); \
      إذا (!tagisempty(نښان)) { farr2val(h, u, نښان, res); }} \
    وإلا { نښان = luaH_getint(h, (k), res); }}


#تعریف_کړه luaH_fastseti(t,k,val,hres) \
  { جدول *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    إذا ((u < h->asize)) { \
      lu_byte *نښان = get_Arr_نښان(h, u); \
      إذا (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*نښان)) \
        { fval2arr(h, u, نښان, val); hres = HOK; } \
      وإلا hres = ~cast_int(u); } \
    وإلا { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#تعریف_کړه HOK		0
#تعریف_کړه HNOTFOUND	1
#تعریف_کړه HNOTATABLE	2
#تعریف_کړه HFIRSTNODE	3

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
#تعریف_کړه get_Arr_نښان(t,k)	(cast(lu_byte*, (t)->array) + حجم(بینښه) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#تعریف_کړه getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#تعریف_کړه lenhint(t)	cast(بینښه*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#تعریف_کړه arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_نښان(h,(k)), (val)->ارزښت_3 = *getArrVal(h,(k)))

#تعریف_کړه obj2arr(h,k,val)  \
  (*get_Arr_نښان(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->ارزښت_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#تعریف_کړه farr2val(h,k,نښان,res)  \
  ((res)->tt_ = نښان, (res)->ارزښت_3 = *getArrVal(h,(k)))

#تعریف_کړه fval2arr(h,k,نښان,val)  \
  (*نښان = (val)->tt_, *getArrVal(h,(k)) = (val)->ارزښت_3)


LUAI_FUNC lu_byte luaH_get (جدول *t, ثابت TValue *کلی, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (جدول *t, TString *کلی, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (جدول *t, TString *کلی, TValue *res);
LUAI_FUNC lu_byte luaH_getint (جدول *t, lua_Integer کلی, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ثابت TValue *luaH_Hgetshortstr (جدول *t, TString *کلی);

LUAI_FUNC صحيح luaH_psetint (جدول *t, lua_Integer کلی, TValue *val);
LUAI_FUNC صحيح luaH_psetshortstr (جدول *t, TString *کلی, TValue *val);
LUAI_FUNC صحيح luaH_psetstr (جدول *t, TString *کلی, TValue *val);
LUAI_FUNC صحيح luaH_pset (جدول *t, ثابت TValue *کلی, TValue *val);

LUAI_FUNC فراغ luaH_setint (lua_State *L, جدول *t, lua_Integer کلی,
                                                    TValue *ارزښت);
LUAI_FUNC فراغ lua_H_سېټ (lua_State *L, جدول *t, ثابت TValue *کلی,
                                                 TValue *ارزښت);

LUAI_FUNC فراغ luaH_finishset (lua_State *L, جدول *t, ثابت TValue *کلی,
                                              TValue *ارزښت, صحيح hres);
LUAI_FUNC جدول *luaH_new (lua_State *L);
LUAI_FUNC فراغ luaH_resize (lua_State *L, جدول *t, بینښه nasize,
                                                    بینښه nhsize);
LUAI_FUNC فراغ luaH_resizearray (lua_State *L, جدول *t, بینښه nasize);
LUAI_FUNC lu_mem lua_H_اندازه (جدول *t);
LUAI_FUNC فراغ luaH_free (lua_State *L, جدول *t);
LUAI_FUNC صحيح lua_H_بل (lua_State *L, جدول *t, StkId کلی);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, جدول *t);


#إذا defined(LUA_DEBUG)
LUAI_FUNC غوټه_2 *luaH_mainposition (ثابت جدول *t, ثابت TValue *کلی);
#د_که_پای


#د_که_پای
