/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् ltable_h
#परिभाषय ltable_h

#अन्तर्भाव "lobject.h"


#परिभाषय gnode(t,i)	(&(t)->ग्रन्थिः[i])
#परिभाषय gval(n)		(&(n)->i_val)
#परिभाषय gnext(n)	((n)->u.अग्र)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#परिभाषय invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#परिभाषय BITDUMMY		(1 << 6)
#परिभाषय NOTBITDUMMY		cast_byte(~BITDUMMY)
#परिभाषय isdummy(t)		((t)->flags & BITDUMMY)

#परिभाषय setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#परिभाषय setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#परिभाषय allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#परिभाषय nodefromval(v)	cast(ग्रन्थिः_2 *, (v))



#परिभाषय luaH_fastgeti(t,k,res,चिह्नकम्) \
  { सारणी *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    यदि ((u < h->asize)) { \
      चिह्नकम् = *get_Arr_चिह्नकम्(h, u); \
      यदि (!tagisempty(चिह्नकम्)) { farr2val(h, u, चिह्नकम्, res); }} \
    अन्यथा { चिह्नकम् = luaH_getint(h, (k), res); }}


#परिभाषय luaH_fastseti(t,k,val,hres) \
  { सारणी *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    यदि ((u < h->asize)) { \
      lu_byte *चिह्नकम् = get_Arr_चिह्नकम्(h, u); \
      यदि (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*चिह्नकम्)) \
        { fval2arr(h, u, चिह्नकम्, val); hres = HOK; } \
      अन्यथा hres = ~cast_int(u); } \
    अन्यथा { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#परिभाषय HOK		0
#परिभाषय HNOTFOUND	1
#परिभाषय HNOTATABLE	2
#परिभाषय HFIRSTNODE	3

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
#परिभाषय get_Arr_चिह्नकम्(t,k)	(cast(lu_byte*, (t)->array) + परिमाण(अचिह्नित) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#परिभाषय getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#परिभाषय lenhint(t)	cast(अचिह्नित*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#परिभाषय arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_चिह्नकम्(h,(k)), (val)->मानम्_3 = *getArrVal(h,(k)))

#परिभाषय obj2arr(h,k,val)  \
  (*get_Arr_चिह्नकम्(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->मानम्_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#परिभाषय farr2val(h,k,चिह्नकम्,res)  \
  ((res)->tt_ = चिह्नकम्, (res)->मानम्_3 = *getArrVal(h,(k)))

#परिभाषय fval2arr(h,k,चिह्नकम्,val)  \
  (*चिह्नकम् = (val)->tt_, *getArrVal(h,(k)) = (val)->मानम्_3)


LUAI_FUNC lu_byte luaH_get (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (सारणी *t, TString *कुञ्चिका, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (सारणी *t, TString *कुञ्चिका, TValue *res);
LUAI_FUNC lu_byte luaH_getint (सारणी *t, lua_Integer कुञ्चिका, TValue *res);

/* Special get for metamethods */
LUAI_FUNC ध्रुव TValue *luaH_Hgetshortstr (सारणी *t, TString *कुञ्चिका);

LUAI_FUNC पूर्णाङ्क luaH_psetint (सारणी *t, lua_Integer कुञ्चिका, TValue *val);
LUAI_FUNC पूर्णाङ्क luaH_psetshortstr (सारणी *t, TString *कुञ्चिका, TValue *val);
LUAI_FUNC पूर्णाङ्क luaH_psetstr (सारणी *t, TString *कुञ्चिका, TValue *val);
LUAI_FUNC पूर्णाङ्क luaH_pset (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *val);

LUAI_FUNC शून्य luaH_setint (lua_State *L, सारणी *t, lua_Integer कुञ्चिका,
                                                    TValue *मानम्);
LUAI_FUNC शून्य lua_H_समुच्चयः (lua_State *L, सारणी *t, ध्रुव TValue *कुञ्चिका,
                                                 TValue *मानम्);

LUAI_FUNC शून्य luaH_finishset (lua_State *L, सारणी *t, ध्रुव TValue *कुञ्चिका,
                                              TValue *मानम्, पूर्णाङ्क hres);
LUAI_FUNC सारणी *luaH_new (lua_State *L);
LUAI_FUNC शून्य luaH_resize (lua_State *L, सारणी *t, अचिह्नित nasize,
                                                    अचिह्नित nhsize);
LUAI_FUNC शून्य luaH_resizearray (lua_State *L, सारणी *t, अचिह्नित nasize);
LUAI_FUNC lu_mem lua_H_आकार (सारणी *t);
LUAI_FUNC शून्य luaH_free (lua_State *L, सारणी *t);
LUAI_FUNC पूर्णाङ्क lua_H_अग्र (lua_State *L, सारणी *t, StkId कुञ्चिका);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, सारणी *t);


#यदि defined(LUA_DEBUG)
LUAI_FUNC ग्रन्थिः_2 *luaH_mainposition (ध्रुव सारणी *t, ध्रुव TValue *कुञ्चिका);
#यदि_अन्तः


#यदि_अन्तः
