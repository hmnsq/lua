/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ltable_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltable_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gnode(t,i)	(&(t)->ਗੰਢ[i])
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gval(n)		(&(n)->i_val)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gnext(n)	((n)->u.ਅਗਲਾ)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BITDUMMY		(1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOTBITDUMMY		cast_byte(~BITDUMMY)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isdummy(t)		((t)->flags & BITDUMMY)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nodefromval(v)	cast(ਗੰਢ_2 *, (v))



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaH_fastgeti(t,k,res,ਟੈਗ) \
  { ਸਾਰਣੀ *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ਜੇ ((u < h->asize)) { \
      ਟੈਗ = *get_Arr_ਟੈਗ(h, u); \
      ਜੇ (!tagisempty(ਟੈਗ)) { farr2val(h, u, ਟੈਗ, res); }} \
    ਨਹੀਂਤਾਂ { ਟੈਗ = luaH_getint(h, (k), res); }}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaH_fastseti(t,k,val,hres) \
  { ਸਾਰਣੀ *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ਜੇ ((u < h->asize)) { \
      lu_byte *ਟੈਗ = get_Arr_ਟੈਗ(h, u); \
      ਜੇ (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*ਟੈਗ)) \
        { fval2arr(h, u, ਟੈਗ, val); hres = HOK; } \
      ਨਹੀਂਤਾਂ hres = ~cast_int(u); } \
    ਨਹੀਂਤਾਂ { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOK		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HNOTFOUND	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HNOTATABLE	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HFIRSTNODE	3

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_Arr_ਟੈਗ(t,k)	(cast(lu_byte*, (t)->array) + ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lenhint(t)	cast(ਬਿਨਾਨਿਸ਼ਾਨੀ*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_ਟੈਗ(h,(k)), (val)->ਮੁੱਲ_3 = *getArrVal(h,(k)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ obj2arr(h,k,val)  \
  (*get_Arr_ਟੈਗ(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->ਮੁੱਲ_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ farr2val(h,k,ਟੈਗ,res)  \
  ((res)->tt_ = ਟੈਗ, (res)->ਮੁੱਲ_3 = *getArrVal(h,(k)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fval2arr(h,k,ਟੈਗ,val)  \
  (*ਟੈਗ = (val)->tt_, *getArrVal(h,(k)) = (val)->ਮੁੱਲ_3)


LUAI_FUNC lu_byte luaH_get (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *res);
LUAI_FUNC lu_byte luaH_getint (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ਅਚਲ TValue *luaH_Hgetshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ);

LUAI_FUNC ਪੂਰਨ luaH_psetint (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *val);
LUAI_FUNC ਪੂਰਨ luaH_psetshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *val);
LUAI_FUNC ਪੂਰਨ luaH_psetstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *val);
LUAI_FUNC ਪੂਰਨ luaH_pset (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *val);

LUAI_FUNC ਖਾਲੀ luaH_setint (lua_State *L, ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ,
                                                    TValue *ਮੁੱਲ);
LUAI_FUNC ਖਾਲੀ lua_H_ਸੈੱਟ (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ,
                                                 TValue *ਮੁੱਲ);

LUAI_FUNC ਖਾਲੀ luaH_finishset (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ,
                                              TValue *ਮੁੱਲ, ਪੂਰਨ hres);
LUAI_FUNC ਸਾਰਣੀ *luaH_new (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaH_resize (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ nasize,
                                                    ਬਿਨਾਨਿਸ਼ਾਨੀ nhsize);
LUAI_FUNC ਖਾਲੀ luaH_resizearray (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ nasize);
LUAI_FUNC lu_mem lua_H_ਮਾਪ (ਸਾਰਣੀ *t);
LUAI_FUNC ਖਾਲੀ luaH_free (lua_State *L, ਸਾਰਣੀ *t);
LUAI_FUNC ਪੂਰਨ lua_H_ਅਗਲਾ (lua_State *L, ਸਾਰਣੀ *t, StkId ਕੁੰਜੀ);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, ਸਾਰਣੀ *t);


#ਜੇ defined(LUA_DEBUG)
LUAI_FUNC ਗੰਢ_2 *luaH_mainposition (ਅਚਲ ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ);
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ
