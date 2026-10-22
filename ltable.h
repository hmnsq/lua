/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#अगर_अघोषित ltable_h
#घोषणा ltable_h

#समावेश "lobject.h"


#घोषणा gnode(t,i)	(&(t)->गाँठ[i])
#घोषणा gval(n)		(&(n)->i_val)
#घोषणा gnext(n)	((n)->u.अग्र)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#घोषणा invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#घोषणा BITDUMMY		(1 << 6)
#घोषणा NOTBITDUMMY		cast_byte(~BITDUMMY)
#घोषणा isdummy(t)		((t)->flags & BITDUMMY)

#घोषणा setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#घोषणा setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#घोषणा allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#घोषणा nodefromval(v)	cast(गाँठ_2 *, (v))



#घोषणा luaH_fastgeti(t,k,res,tag) \
  { Table *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    अगर ((u < h->asize)) { \
      tag = *getArrTag(h, u); \
      अगर (!tagisempty(tag)) { farr2val(h, u, tag, res); }} \
    अन्यथा { tag = luaH_getint(h, (k), res); }}


#घोषणा luaH_fastseti(t,k,val,hres) \
  { Table *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    अगर ((u < h->asize)) { \
      lu_byte *tag = getArrTag(h, u); \
      अगर (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*tag)) \
        { fval2arr(h, u, tag, val); hres = HOK; } \
      अन्यथा hres = ~cast_int(u); } \
    अन्यथा { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#घोषणा HOK		0
#घोषणा HNOTFOUND	1
#घोषणा HNOTATABLE	2
#घोषणा HFIRSTNODE	3

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
#घोषणा getArrTag(t,k)	(cast(lu_byte*, (t)->array) + माप(अचिन्हित) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#घोषणा getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#घोषणा lenhint(t)	cast(अचिन्हित*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#घोषणा arr2obj(h,k,val)  \
  ((val)->tt_ = *getArrTag(h,(k)), (val)->मूल्य_3 = *getArrVal(h,(k)))

#घोषणा obj2arr(h,k,val)  \
  (*getArrTag(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->मूल्य_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#घोषणा farr2val(h,k,tag,res)  \
  ((res)->tt_ = tag, (res)->मूल्य_3 = *getArrVal(h,(k)))

#घोषणा fval2arr(h,k,tag,val)  \
  (*tag = (val)->tt_, *getArrVal(h,(k)) = (val)->मूल्य_3)


LUAI_FUNC lu_byte luaH_get (Table *t, स्थिर TValue *कुंजी, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (Table *t, TString *कुंजी, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (Table *t, TString *कुंजी, TValue *res);
LUAI_FUNC lu_byte luaH_getint (Table *t, lua_Integer कुंजी, TValue *res);

/* Special get for metamethods */
LUAI_FUNC स्थिर TValue *luaH_Hgetshortstr (Table *t, TString *कुंजी);

LUAI_FUNC पूर्णांक luaH_psetint (Table *t, lua_Integer कुंजी, TValue *val);
LUAI_FUNC पूर्णांक luaH_psetshortstr (Table *t, TString *कुंजी, TValue *val);
LUAI_FUNC पूर्णांक luaH_psetstr (Table *t, TString *कुंजी, TValue *val);
LUAI_FUNC पूर्णांक luaH_pset (Table *t, स्थिर TValue *कुंजी, TValue *val);

LUAI_FUNC व्योम luaH_setint (lua_State *L, Table *t, lua_Integer कुंजी,
                                                    TValue *मूल्य);
LUAI_FUNC व्योम luaH_set (lua_State *L, Table *t, स्थिर TValue *कुंजी,
                                                 TValue *मूल्य);

LUAI_FUNC व्योम luaH_finishset (lua_State *L, Table *t, स्थिर TValue *कुंजी,
                                              TValue *मूल्य, पूर्णांक hres);
LUAI_FUNC Table *luaH_new (lua_State *L);
LUAI_FUNC व्योम luaH_resize (lua_State *L, Table *t, अचिन्हित nasize,
                                                    अचिन्हित nhsize);
LUAI_FUNC व्योम luaH_resizearray (lua_State *L, Table *t, अचिन्हित nasize);
LUAI_FUNC lu_mem lua_H_आकार (Table *t);
LUAI_FUNC व्योम luaH_free (lua_State *L, Table *t);
LUAI_FUNC पूर्णांक lua_H_अग्र (lua_State *L, Table *t, StkId कुंजी);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, Table *t);


#अगर defined(LUA_DEBUG)
LUAI_FUNC गाँठ_2 *luaH_mainposition (स्थिर Table *t, स्थिर TValue *कुंजी);
#पूर्ण_अगर


#पूर्ण_अगर
