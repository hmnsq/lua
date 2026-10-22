/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ltable_h
#ವ್ಯಾಖ್ಯಾನಿಸು ltable_h

#ಅನ್ತರ್ಭಾವ "lobject.h"


#ವ್ಯಾಖ್ಯಾನಿಸು gnode(t,i)	(&(t)->ಗಂಟು[i])
#ವ್ಯಾಖ್ಯಾನಿಸು gval(n)		(&(n)->i_val)
#ವ್ಯಾಖ್ಯಾನಿಸು gnext(n)	((n)->u.ಮುಂದಿನ)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು BITDUMMY		(1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು NOTBITDUMMY		cast_byte(~BITDUMMY)
#ವ್ಯಾಖ್ಯಾನಿಸು isdummy(t)		((t)->flags & BITDUMMY)

#ವ್ಯಾಖ್ಯಾನಿಸು setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#ವ್ಯಾಖ್ಯಾನಿಸು setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#ವ್ಯಾಖ್ಯಾನಿಸು allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#ವ್ಯಾಖ್ಯಾನಿಸು nodefromval(v)	cast(ಗಂಟು_2 *, (v))



#ವ್ಯಾಖ್ಯಾನಿಸು luaH_fastgeti(t,k,res,ಟ್ಯಾಗ್) \
  { ಕೋಷ್ಟಕ *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ಆದರೆ ((u < h->asize)) { \
      ಟ್ಯಾಗ್ = *get_Arr_ಟ್ಯಾಗ್(h, u); \
      ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್)) { farr2val(h, u, ಟ್ಯಾಗ್, res); }} \
    ಇಲ್ಲದಿದ್ದರೆ { ಟ್ಯಾಗ್ = luaH_getint(h, (k), res); }}


#ವ್ಯಾಖ್ಯಾನಿಸು luaH_fastseti(t,k,val,hres) \
  { ಕೋಷ್ಟಕ *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ಆದರೆ ((u < h->asize)) { \
      lu_byte *ಟ್ಯಾಗ್ = get_Arr_ಟ್ಯಾಗ್(h, u); \
      ಆದರೆ (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*ಟ್ಯಾಗ್)) \
        { fval2arr(h, u, ಟ್ಯಾಗ್, val); hres = HOK; } \
      ಇಲ್ಲದಿದ್ದರೆ hres = ~cast_int(u); } \
    ಇಲ್ಲದಿದ್ದರೆ { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#ವ್ಯಾಖ್ಯಾನಿಸು HOK		0
#ವ್ಯಾಖ್ಯಾನಿಸು HNOTFOUND	1
#ವ್ಯಾಖ್ಯಾನಿಸು HNOTATABLE	2
#ವ್ಯಾಖ್ಯಾನಿಸು HFIRSTNODE	3

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
#ವ್ಯಾಖ್ಯಾನಿಸು get_Arr_ಟ್ಯಾಗ್(t,k)	(cast(lu_byte*, (t)->array) + ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#ವ್ಯಾಖ್ಯಾನಿಸು getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lenhint(t)	cast(ಚಿಹ್ನೆರಹಿತ*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#ವ್ಯಾಖ್ಯಾನಿಸು arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_ಟ್ಯಾಗ್(h,(k)), (val)->ಮೌಲ್ಯ_3 = *getArrVal(h,(k)))

#ವ್ಯಾಖ್ಯಾನಿಸು obj2arr(h,k,val)  \
  (*get_Arr_ಟ್ಯಾಗ್(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->ಮೌಲ್ಯ_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು farr2val(h,k,ಟ್ಯಾಗ್,res)  \
  ((res)->tt_ = ಟ್ಯಾಗ್, (res)->ಮೌಲ್ಯ_3 = *getArrVal(h,(k)))

#ವ್ಯಾಖ್ಯಾನಿಸು fval2arr(h,k,ಟ್ಯಾಗ್,val)  \
  (*ಟ್ಯಾಗ್ = (val)->tt_, *getArrVal(h,(k)) = (val)->ಮೌಲ್ಯ_3)


LUAI_FUNC lu_byte luaH_get (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *res);
LUAI_FUNC lu_byte luaH_getint (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ಸ್ಥಿರಾಂಕ TValue *luaH_Hgetshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ);

LUAI_FUNC ಪೂರ್ಣಾಂಕ luaH_psetint (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *val);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaH_psetshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *val);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaH_psetstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *val);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaH_pset (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *val);

LUAI_FUNC ಶೂನ್ಯ luaH_setint (lua_State *L, ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ,
                                                    TValue *ಮೌಲ್ಯ);
LUAI_FUNC ಶೂನ್ಯ lua_H_ಗಣ (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ,
                                                 TValue *ಮೌಲ್ಯ);

LUAI_FUNC ಶೂನ್ಯ luaH_finishset (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ,
                                              TValue *ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ hres);
LUAI_FUNC ಕೋಷ್ಟಕ *luaH_new (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaH_resize (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ nasize,
                                                    ಚಿಹ್ನೆರಹಿತ nhsize);
LUAI_FUNC ಶೂನ್ಯ luaH_resizearray (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ nasize);
LUAI_FUNC lu_mem lua_H_ಗಾತ್ರ (ಕೋಷ್ಟಕ *t);
LUAI_FUNC ಶೂನ್ಯ luaH_free (lua_State *L, ಕೋಷ್ಟಕ *t);
LUAI_FUNC ಪೂರ್ಣಾಂಕ lua_H_ಮುಂದಿನ (lua_State *L, ಕೋಷ್ಟಕ *t, StkId ಕೀಲಿ);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, ಕೋಷ್ಟಕ *t);


#ಆದರೆ defined(LUA_DEBUG)
LUAI_FUNC ಗಂಟು_2 *luaH_mainposition (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ);
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ
