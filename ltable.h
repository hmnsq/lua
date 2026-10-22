/*
** $Id: ltable.h $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే ltable_h
#నిర్వచించు ltable_h

#అన్తర్భావ "lobject.h"


#నిర్వచించు gnode(t,i)	(&(t)->కణుపు[i])
#నిర్వచించు gval(n)		(&(n)->i_val)
#నిర్వచించు gnext(n)	((n)->u.తదుపరి)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#నిర్వచించు invalidateTMcache(t)	((t)->flags &= cast_byte(~maskflags))


/*
** Bit BITDUMMY set in 'flags' means the table is using the dummy node
** for its hash part.
*/

#నిర్వచించు BITDUMMY		(1 << 6)
#నిర్వచించు NOTBITDUMMY		cast_byte(~BITDUMMY)
#నిర్వచించు isdummy(t)		((t)->flags & BITDUMMY)

#నిర్వచించు setnodummy(t)		((t)->flags &= NOTBITDUMMY)
#నిర్వచించు setdummy(t)		((t)->flags |= BITDUMMY)



/* allocated size for hash nodes */
#నిర్వచించు allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#నిర్వచించు nodefromval(v)	cast(కణుపు_2 *, (v))



#నిర్వచించు luaH_fastgeti(t,k,res,ట్యాగ్) \
  { పట్టిక *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ఒకవేళ ((u < h->asize)) { \
      ట్యాగ్ = *get_Arr_ట్యాగ్(h, u); \
      ఒకవేళ (!tagisempty(ట్యాగ్)) { farr2val(h, u, ట్యాగ్, res); }} \
    లేకపోతే { ట్యాగ్ = luaH_getint(h, (k), res); }}


#నిర్వచించు luaH_fastseti(t,k,val,hres) \
  { పట్టిక *h = t; lua_Unsigned u = l_castS2U(k) - 1u; \
    ఒకవేళ ((u < h->asize)) { \
      lu_byte *ట్యాగ్ = get_Arr_ట్యాగ్(h, u); \
      ఒకవేళ (checknoTM(h->metatable, TM_NEWINDEX) || !tagisempty(*ట్యాగ్)) \
        { fval2arr(h, u, ట్యాగ్, val); hres = HOK; } \
      లేకపోతే hres = ~cast_int(u); } \
    లేకపోతే { hres = luaH_psetint(h, k, val); }}


/* results from pset */
#నిర్వచించు HOK		0
#నిర్వచించు HNOTFOUND	1
#నిర్వచించు HNOTATABLE	2
#నిర్వచించు HFIRSTNODE	3

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
#నిర్వచించు get_Arr_ట్యాగ్(t,k)	(cast(lu_byte*, (t)->array) + పరిమాణం(గుర్తులేని) + (k))

/* Computes the address of the value for the abstract C-index 'k' */
#నిర్వచించు getArrVal(t,k)	((t)->array - 1 - (k))


/*
** The unsigned between the two arrays is used as a hint for #t;
** see luaH_getn. It is stored there to avoid wasting space in
** the structure Table for tables with no array part.
*/
#నిర్వచించు lenhint(t)	cast(గుర్తులేని*, (t)->array)


/*
** Move TValues to/from arrays, using C indices
*/
#నిర్వచించు arr2obj(h,k,val)  \
  ((val)->tt_ = *get_Arr_ట్యాగ్(h,(k)), (val)->విలువ_3 = *getArrVal(h,(k)))

#నిర్వచించు obj2arr(h,k,val)  \
  (*get_Arr_ట్యాగ్(h,(k)) = (val)->tt_, *getArrVal(h,(k)) = (val)->విలువ_3)


/*
** Often, we need to check the tag of a value before moving it. The
** following macros also move TValues to/from arrays, but receive the
** precomputed tag value or address as an extra argument.
*/
#నిర్వచించు farr2val(h,k,ట్యాగ్,res)  \
  ((res)->tt_ = ట్యాగ్, (res)->విలువ_3 = *getArrVal(h,(k)))

#నిర్వచించు fval2arr(h,k,ట్యాగ్,val)  \
  (*ట్యాగ్ = (val)->tt_, *getArrVal(h,(k)) = (val)->విలువ_3)


LUAI_FUNC lu_byte luaH_get (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *res);
LUAI_FUNC lu_byte luaH_getshortstr (పట్టిక *t, TString *కీలకం, TValue *res);
LUAI_FUNC lu_byte luaH_getstr (పట్టిక *t, TString *కీలకం, TValue *res);
LUAI_FUNC lu_byte luaH_getint (పట్టిక *t, lua_Integer కీలకం, TValue *res);

/* Special get for metamethods */
LUAI_FUNC స్థిరరాశి TValue *luaH_Hgetshortstr (పట్టిక *t, TString *కీలకం);

LUAI_FUNC పూర్ణాంకం luaH_psetint (పట్టిక *t, lua_Integer కీలకం, TValue *val);
LUAI_FUNC పూర్ణాంకం luaH_psetshortstr (పట్టిక *t, TString *కీలకం, TValue *val);
LUAI_FUNC పూర్ణాంకం luaH_psetstr (పట్టిక *t, TString *కీలకం, TValue *val);
LUAI_FUNC పూర్ణాంకం luaH_pset (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *val);

LUAI_FUNC శూన్య luaH_setint (lua_State *L, పట్టిక *t, lua_Integer కీలకం,
                                                    TValue *విలువ);
LUAI_FUNC శూన్య lua_H_సమితి (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *కీలకం,
                                                 TValue *విలువ);

LUAI_FUNC శూన్య luaH_finishset (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *కీలకం,
                                              TValue *విలువ, పూర్ణాంకం hres);
LUAI_FUNC పట్టిక *luaH_new (lua_State *L);
LUAI_FUNC శూన్య luaH_resize (lua_State *L, పట్టిక *t, గుర్తులేని nasize,
                                                    గుర్తులేని nhsize);
LUAI_FUNC శూన్య luaH_resizearray (lua_State *L, పట్టిక *t, గుర్తులేని nasize);
LUAI_FUNC lu_mem lua_H_కొలత (పట్టిక *t);
LUAI_FUNC శూన్య luaH_free (lua_State *L, పట్టిక *t);
LUAI_FUNC పూర్ణాంకం lua_H_తదుపరి (lua_State *L, పట్టిక *t, StkId కీలకం);
LUAI_FUNC lua_Unsigned luaH_getn (lua_State *L, పట్టిక *t);


#ఒకవేళ defined(LUA_DEBUG)
LUAI_FUNC కణుపు_2 *luaH_mainposition (స్థిరరాశి పట్టిక *t, స్థిరరాశి TValue *కీలకం);
#అయితే_ముగింపు


#అయితే_ముగింపు
