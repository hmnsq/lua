/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


/*
** Implementation of tables (aka arrays, objects, or hash tables).
** Tables keep its elements in two parts: an array part and a hash part.
** Non-negative integer keys are all candidates to be kept in the array
** part. The actual size of the array is the largest 'n' such that
** more than half the slots between 1 and n are in use.
** Hash uses a mix of chained scatter table with Brent's variation.
** A main invariant of these tables is that, if an element is not
** in its main position (i.e. the 'original' position that its hash gives
** to it), then the colliding element is in its own main position.
** Hence even when the load factor reaches 100%, performance remains good.
*/

#अन्तर्भाव <math.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#define LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
प्रकारव्याख्या संरचना { गाठ_2 *dummy; गाठ_2 follows_p_गाठ; } Limbox_aux;

प्रकारव्याख्या संघ {
  गाठ_2 *lastfree;
  अक्षर padding[दुरत्व(Limbox_aux, follows_p_गाठ)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->गाठ) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(पूर्णांक) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(आकार(मूल्य_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#define MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#define MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, गाठ_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#define hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#define hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#define hashstr(t,str)		hashpow2(t, (str)->hash)
#define hashboolean(t,p)	hashpow2(t, p)


#define hashpointer(t,p)	hashmod(t, point2uint(p))


#define dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
स्थिर स्थिरांक गाठ_2 dummynode_ = {
  {{रिक्त}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {रिक्त}}  /* key type, next, and key value */
};


स्थिर स्थिरांक TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
स्थिर गाठ_2 *hashint (स्थिरांक Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  जर (ui <= cast_uint(पूर्णांक_उच्च))
    परत gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  अन्यथा
    परत hashmod(t, ui);
}


/*
** Hash for floating-point numbers.
** The main computation should be just
**     n = frexp(n, &i); return (n * INT_MAX) + i
** but there are some numerical subtleties.
** In a two-complement representation, INT_MAX may not have an exact
** representation as a float, but INT_MIN does; because the absolute
** value of 'frexp' is smaller than 1 (unless 'n' is inf/NaN), the
** absolute value of the product 'frexp * -INT_MIN' is smaller or equal
** to INT_MAX. Next, the use of 'unsigned int' avoids overflows when
** adding 'i'; the use of '~u' (instead of '-u') avoids problems with
** INT_MIN.
*/
#जर !defined(l_hashfloat)
स्थिर अचिन्हित l_hashfloat (lua_संख्या n) {
  पूर्णांक i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_अंक(पूर्णांक_न्यून);
  जर (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(भ_असल)(n) == cast_अंक(विशाल_मान));
    परत 0;
  }
  अन्यथा {  /* normal case */
    अचिन्हित पूर्णांक u = cast_uint(i) + cast_uint(ni);
    परत (u <= cast_uint(पूर्णांक_उच्च) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
स्थिर गाठ_2 *mainpositionTV (स्थिरांक Table *t, स्थिरांक TValue *किल्ली) {
  निवड (ttypetag(किल्ली)) {
    स्थिती LUA_VNUMINT: {
      lua_Integer i = ivalue(किल्ली);
      परत hashint(t, i);
    }
    स्थिती LUA_VNUMFLT: {
      lua_संख्या n = fltvalue(किल्ली);
      परत hashmod(t, l_hashfloat(n));
    }
    स्थिती LUA_VSHRSTR: {
      TString *ts = tsvalue(किल्ली);
      परत hashstr(t, ts);
    }
    स्थिती LUA_VLNGSTR: {
      TString *ts = tsvalue(किल्ली);
      परत hashpow2(t, luaS_hashlongstr(ts));
    }
    स्थिती LUA_VFALSE:
      परत hashboolean(t, 0);
    स्थिती LUA_VTRUE:
      परत hashboolean(t, 1);
    स्थिती LUA_VLIGHTUSERDATA: {
      शून्य *p = pvalue(किल्ली);
      परत hashpointer(t, p);
    }
    स्थिती LUA_VLCF: {
      lua_CFunction f = fvalue(किल्ली);
      परत hashpointer(t, f);
    }
    पूर्वनिर्धारित: {
      GCObject *o = gcvalue(किल्ली);
      परत hashpointer(t, o);
    }
  }
}


l_sinline गाठ_2 *mainpositionfromnode (स्थिरांक Table *t, गाठ_2 *nd) {
  TValue किल्ली;
  getnodekey(cast(lua_State *, रिक्त), &किल्ली, nd);
  परत mainpositionTV(t, &किल्ली);
}


/*
** Check whether key 'k1' is equal to the key in node 'n2'. This
** equality is raw, so there are no metamethods. Floats with integer
** values have been normalized, so integers cannot be equal to
** floats. It is assumed that 'eqshrstr' is simply pointer equality,
** so that short strings are handled in the default case.  The flag
** 'deadok' means to accept dead keys as equal to their original values.
** (Only collectable objects can produce dead keys.) Note that dead
** long strings are also compared by identity.  Once a key is dead,
** its corresponding value may be collected, and then another value
** can be created with the same address. If this other value is given
** to 'next', 'equalkey' will signal a false positive. In a regular
** traversal, this situation should never happen, as all keys given to
** 'next' came from the table itself, and therefore could not have been
** collected. Outside a regular traversal, we have garbage in, garbage
** out. What is relevant is that this false positive does not break
** anything.  (In particular, 'next' will return some other valid item
** on the table or nil.)
*/
स्थिर पूर्णांक equalkey (स्थिरांक TValue *k1, स्थिरांक गाठ_2 *n2, पूर्णांक deadok) {
  जर (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    जर (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      परत luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    अन्यथा जर (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      परत gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   अन्यथा
     परत 0;  /* otherwise, different variants cannot be equal */
  }
  अन्यथा {  /* equal variants */
    निवड (keytt(n2)) {
      स्थिती LUA_VNIL: स्थिती LUA_VFALSE: स्थिती LUA_VTRUE:
        परत 1;
      स्थिती LUA_VNUMINT:
        परत (ivalue(k1) == keyival(n2));
      स्थिती LUA_VNUMFLT:
        परत luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      स्थिती LUA_VLIGHTUSERDATA:
        परत pvalue(k1) == pvalueraw(keyval(n2));
      स्थिती LUA_VLCF:
        परत fvalue(k1) == fvalueraw(keyval(n2));
      स्थिती ctb(LUA_VLNGSTR):
        परत luaS_eqstr(tsvalue(k1), keystrval(n2));
      पूर्वनिर्धारित:
        परत gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
स्थिर स्थिरांक TValue *getgeneric (Table *t, स्थिरांक TValue *किल्ली, पूर्णांक deadok) {
  गाठ_2 *n = mainpositionTV(t, किल्ली);
  साठी (;;) {  /* check whether 'key' is somewhere in the chain */
    जर (equalkey(किल्ली, n, deadok))
      परत gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      जर (nx == 0)
        परत &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
स्थिर अचिन्हित checkrange (lua_Integer k, अचिन्हित limit) {
  परत (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#define arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#define ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
स्थिर अचिन्हित keyinarray (Table *t, स्थिरांक TValue *किल्ली) {
  परत (ttisinteger(किल्ली)) ? ikeyinarray(t, ivalue(किल्ली)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
स्थिर अचिन्हित findindex (lua_State *L, Table *t, TValue *किल्ली,
                               अचिन्हित asize) {
  अचिन्हित पूर्णांक i;
  जर (ttisnil(किल्ली)) परत 0;  /* first iteration */
  i = keyinarray(t, किल्ली);
  जर (i != 0)  /* is 'key' inside array part? */
    परत i;  /* yes; that's the index */
  अन्यथा {
    स्थिरांक TValue *n = getgeneric(t, किल्ली, 1);
    जर (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    परत (i + 1) + asize;
  }
}


पूर्णांक luaH_next (lua_State *L, Table *t, StkId किल्ली) {
  अचिन्हित पूर्णांक asize = t->asize;
  अचिन्हित पूर्णांक i = findindex(L, t, s2v(किल्ली), asize);  /* find original key */
  साठी (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    जर (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(किल्ली), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(किल्ली + 1));
      परत 1;
    }
  }
  साठी (i -= asize; i < sizenode(t); i++) {  /* hash part */
    जर (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      गाठ_2 *n = gnode(t, i);
      getnodekey(L, s2v(किल्ली), n);
      setobj2s(L, किल्ली + 1, gval(n));
      परत 1;
    }
  }
  परत 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? आकार(Limbox) : 0)

/* 'node' size in bytes */
स्थिर माप_प्रकार sizehash (Table *t) {
  परत cast_sizet(sizenode(t)) * आकार(गाठ_2) + extraLastfree(t);
}


स्थिर शून्य freehash (lua_State *L, Table *t) {
  जर (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    अक्षर *arr = cast_charp(t->गाठ) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

स्थिर पूर्णांक insertkey (Table *t, स्थिरांक TValue *किल्ली, TValue *मूल्य);
स्थिर शून्य newcheckedkey (Table *t, स्थिरांक TValue *किल्ली, TValue *मूल्य);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
प्रकारव्याख्या संरचना {
  अचिन्हित total;
  अचिन्हित na;
  पूर्णांक deleted;
  अचिन्हित nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#define arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
स्थिर अचिन्हित computesizes (Counters *ct) {
  पूर्णांक i;
  अचिन्हित पूर्णांक twotoi;  /* 2^i (candidate for optimal size) */
  अचिन्हित पूर्णांक a = 0;  /* number of elements smaller than 2^i */
  अचिन्हित पूर्णांक na = 0;  /* number of elements to go to array part */
  अचिन्हित पूर्णांक optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  साठी (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    अचिन्हित nums = ct->nums[i];
    a += nums;
    जर (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  परत optimal;
}


स्थिर शून्य countint (lua_Integer किल्ली, Counters *ct) {
  अचिन्हित पूर्णांक k = arrayindex(किल्ली);
  जर (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline पूर्णांक arraykeyisempty (स्थिरांक Table *t, अचिन्हित किल्ली) {
  पूर्णांक tag = *getArrTag(t, किल्ली - 1);
  परत tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
स्थिर शून्य numusearray (स्थिरांक Table *t, Counters *ct) {
  पूर्णांक lg;
  अचिन्हित पूर्णांक ttlg;  /* 2^lg */
  अचिन्हित पूर्णांक ause = 0;  /* summation of 'nums' */
  अचिन्हित पूर्णांक i = 1;  /* index to traverse all array keys */
  अचिन्हित पूर्णांक asize = t->asize;
  /* traverse each slice */
  साठी (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    अचिन्हित पूर्णांक lc = 0;  /* counter */
    अचिन्हित पूर्णांक lim = ttlg;
    जर (lim > asize) {
      lim = asize;  /* adjust upper limit */
      जर (i > lim)
        थांबा;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    साठी (; i <= lim; i++) {
      जर (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->total += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
स्थिर शून्य numusehash (स्थिरांक Table *t, Counters *ct) {
  अचिन्हित i = sizenode(t);
  अचिन्हित total = 0;
  पर्यंत (i--) {
    गाठ_2 *n = &t->गाठ[i];
    जर (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    अन्यथा {
      total++;
      जर (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
स्थिर माप_प्रकार concretesize (अचिन्हित पूर्णांक size) {
  जर (size == 0)
    परत 0;
  अन्यथा  /* space for the two arrays plus an unsigned in between */
    परत size * (आकार(मूल्य_2) + 1) + आकार(अचिन्हित);
}


/*
** Resize the array part of a table. If new size is equal to the old,
** do nothing. Else, if new size is zero, free the old array. (It must
** be present, as the sizes are different.) Otherwise, allocate a new
** array, move the common elements to new proper position, and then
** frees the old array.
** We could reallocate the array, but we still would need to move the
** elements to their new position, so the copy implicit in realloc is a
** waste. Moreover, most allocators will move the array anyway when the
** new size is double the old one (the most common case).
*/
स्थिर मूल्य_2 *resizearray (lua_State *L , Table *t,
                               अचिन्हित oldasize,
                               अचिन्हित newasize) {
  जर (oldasize == newasize)
    परत t->array;  /* nothing to be done */
  अन्यथा जर (newasize == 0) {  /* erasing array? */
    मूल्य_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    परत रिक्त;
  }
  अन्यथा {
    माप_प्रकार newasizeb = concretesize(newasize);
    मूल्य_2 *np = cast(मूल्य_2 *,
                  luaM_reallocvector(L, रिक्त, 0, newasizeb, lu_byte));
    जर (np == रिक्त)  /* allocation error? */
      परत रिक्त;
    np += newasize;  /* shift pointer to the end of value segment */
    जर (oldasize > 0) {
      /* move common elements to new position */
      माप_प्रकार oldasizeb = concretesize(oldasize);
      मूल्य_2 *op = t->array;  /* original array */
      अचिन्हित tomove = (oldasize < newasize) ? oldasize : newasize;
      माप_प्रकार tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      स_नकल(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    परत np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
स्थिर शून्य setnodevector (lua_State *L, Table *t, अचिन्हित size) {
  जर (size == 0) {  /* no elements to hash part? */
    t->गाठ = cast(गाठ_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  अन्यथा {
    पूर्णांक i;
    पूर्णांक lsize = luaO_ceillog2(size);
    जर (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    जर (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->गाठ = luaM_newvector(L, size, गाठ_2);
    अन्यथा {
      माप_प्रकार bsize = size * आकार(गाठ_2) + आकार(Limbox);
      अक्षर *गाठ = luaM_newblock(L, bsize);
      t->गाठ = cast(गाठ_2 *, गाठ + आकार(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    साठी (i = 0; i < cast_int(size); i++) {
      गाठ_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
स्थिर शून्य reinserthash (lua_State *L, Table *ot, Table *t) {
  अचिन्हित j;
  अचिन्हित size = sizenode(ot);
  साठी (j = 0; j < size; j++) {
    गाठ_2 *old = gnode(ot, j);
    जर (!isempty(gval(old))) {
      /* doesn't need barrier/invalidate cache, as entry was
         already present in the table */
      TValue k;
      getnodekey(L, &k, old);
      newcheckedkey(t, &k, gval(old));
    }
  }
}


/*
** Exchange the hash part of 't1' and 't2'. (In 'flags', only the dummy
** bit must be exchanged:  The metamethod bits do not change during a
** resize, so the "real" table can keep their values.)
*/
स्थिर शून्य exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  गाठ_2 *गाठ = t1->गाठ;
  पूर्णांक bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->गाठ = t2->गाठ;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->गाठ = गाठ;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
स्थिर शून्य reinsertOldSlice (Table *t, अचिन्हित oldasize,
                                        अचिन्हित newasize) {
  अचिन्हित i;
  साठी (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    जर (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue किल्ली, aux;
      setivalue(&किल्ली, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &किल्ली, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
स्थिर शून्य clearNewSlice (Table *t, अचिन्हित oldasize, अचिन्हित newasize) {
  साठी (; oldasize < newasize; oldasize++)
    *getArrTag(t, oldasize) = LUA_VEMPTY;
}


/*
** Resize table 't' for the new given sizes. Both allocations (for
** the hash part and for the array part) can fail, which creates some
** subtleties. If the first allocation, for the hash part, fails, an
** error is raised and that is it. Otherwise, it copies the elements from
** the shrinking part of the array (if it is shrinking) into the new
** hash. Then it reallocates the array part.  If that fails, the table
** is in its original state; the function frees the new hash part and then
** raises the allocation error. Otherwise, it sets the new hash part
** into the table, initializes the new part of the array (if any) with
** nils and reinserts the elements of the old hash back into the new
** parts of the table.
** Note that if the new size for the array part ('newasize') is equal to
** the old one ('oldasize'), this function will do nothing with that
** part.
*/
शून्य luaH_resize (lua_State *L, Table *t, अचिन्हित newasize,
                                          अचिन्हित nhsize) {
  Table newt;  /* to keep the new hash part */
  अचिन्हित oldasize = t->asize;
  मूल्य_2 *newarray;
  जर (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  जर (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  जर (l_unlikely(newarray == रिक्त && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  जर (newarray != रिक्त)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


शून्य luaH_resizearray (lua_State *L, Table *t, अचिन्हित पूर्णांक nasize) {
  अचिन्हित nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
स्थिर शून्य rehash (lua_State *L, Table *t, स्थिरांक TValue *ek) {
  अचिन्हित asize;  /* optimal size for array part */
  Counters ct;
  अचिन्हित i;
  अचिन्हित nsize;  /* size for the hash part */
  /* reset counts */
  साठी (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  जर (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  जर (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  अन्यथा {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  जर (ct.deleted) {  /* table has deleted entries? */
    /* insertion-deletion-insertion: give hash some extra size to
       avoid repeated resizings */
    nsize += nsize >> 2;
  }
  /* resize the table to new computed sizes */
  luaH_resize(L, t, asize, nsize);
}

/*
** }=============================================================
*/


Table *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, आकार(Table));
  Table *t = gco2t(o);
  t->metatable = रिक्त;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = रिक्त;
  t->asize = 0;
  setnodevector(L, t, 0);
  परत t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, आकार(Table)) + concretesize(t->asize);
  जर (!isdummy(t))
    sz += sizehash(t);
  परत sz;
}


/*
** Frees a table.
*/
शून्य luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


स्थिर गाठ_2 *getfreepos (Table *t) {
  जर (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    पर्यंत (getlastfree(t) > t->गाठ) {
      गाठ_2 *मुक्त = --getlastfree(t);
      जर (keyisnil(मुक्त))
        परत मुक्त;
    }
  }
  अन्यथा {  /* no 'lastfree' information */
    अचिन्हित i = sizenode(t);
    पर्यंत (i--) {  /* do a linear search */
      गाठ_2 *मुक्त = gnode(t, i);
      जर (keyisnil(मुक्त))
        परत मुक्त;
    }
  }
  परत रिक्त;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
स्थिर पूर्णांक insertkey (Table *t, स्थिरांक TValue *किल्ली, TValue *मूल्य) {
  गाठ_2 *mp = mainpositionTV(t, किल्ली);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, किल्ली, 0)));
  जर (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    गाठ_2 *othern;
    गाठ_2 *f = getfreepos(t);  /* get a free place */
    जर (f == रिक्त)  /* cannot find a free place? */
      परत 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    जर (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      पर्यंत (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      जर (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    अन्यथा {  /* colliding node is in its own main position */
      /* new node will go into free position */
      जर (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      अन्यथा lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, किल्ली);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), मूल्य);
  परत 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
स्थिर शून्य newcheckedkey (Table *t, स्थिरांक TValue *किल्ली, TValue *मूल्य) {
  अचिन्हित i = keyinarray(t, किल्ली);
  जर (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, मूल्य);  /* set value in the array */
  अन्यथा {
    पूर्णांक done = insertkey(t, किल्ली, मूल्य);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(शून्य, done);  /* to avoid warnings */
  }
}


स्थिर शून्य luaH_newkey (lua_State *L, Table *t, स्थिरांक TValue *किल्ली,
                                                 TValue *मूल्य) {
  जर (!ttisnil(मूल्य)) {  /* do not insert nil values */
    पूर्णांक done = insertkey(t, किल्ली, मूल्य);
    जर (!done) {  /* could not find a free place? */
      rehash(L, t, किल्ली);  /* grow table */
      newcheckedkey(t, किल्ली, मूल्य);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), किल्ली);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (शून्य)0, (शून्य)0, 1);
  }
}


स्थिर स्थिरांक TValue *getintfromhash (Table *t, lua_Integer किल्ली) {
  गाठ_2 *n = hashint(t, किल्ली);
  lua_assert(!ikeyinarray(t, किल्ली));
  साठी (;;) {  /* check whether 'key' is somewhere in the chain */
    जर (keyisinteger(n) && keyival(n) == किल्ली)
      परत gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      जर (nx == 0) थांबा;
      n += nx;
    }
  }
  परत &absentkey;
}


स्थिर पूर्णांक hashkeyisempty (Table *t, lua_Unsigned किल्ली) {
  स्थिरांक TValue *val = getintfromhash(t, l_castU2S(किल्ली));
  परत isempty(val);
}


स्थिर lu_byte finishnodeget (स्थिरांक TValue *val, TValue *res) {
  जर (!ttisnil(val)) {
    setobj(((lua_State*)रिक्त), res, val);
  }
  परत ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer किल्ली, TValue *res) {
  अचिन्हित k = ikeyinarray(t, किल्ली);
  जर (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    जर (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    परत tag;
  }
  अन्यथा
    परत finishnodeget(getintfromhash(t, किल्ली), res);
}


/*
** search function for short strings
*/
स्थिरांक TValue *luaH_Hgetshortstr (Table *t, TString *किल्ली) {
  गाठ_2 *n = hashstr(t, किल्ली);
  lua_assert(strisshr(किल्ली));
  साठी (;;) {  /* check whether 'key' is somewhere in the chain */
    जर (keyisshrstr(n) && eqshrstr(keystrval(n), किल्ली))
      परत gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      जर (nx == 0)
        परत &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *किल्ली, TValue *res) {
  परत finishnodeget(luaH_Hgetshortstr(t, किल्ली), res);
}


स्थिर स्थिरांक TValue *Hgetlongstr (Table *t, TString *किल्ली) {
  TValue ko;
  lua_assert(!strisshr(किल्ली));
  setsvalue(cast(lua_State *, रिक्त), &ko, किल्ली);
  परत getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


स्थिर स्थिरांक TValue *Hgetstr (Table *t, TString *किल्ली) {
  जर (strisshr(किल्ली))
    परत luaH_Hgetshortstr(t, किल्ली);
  अन्यथा
    परत Hgetlongstr(t, किल्ली);
}


lu_byte luaH_getstr (Table *t, TString *किल्ली, TValue *res) {
  परत finishnodeget(Hgetstr(t, किल्ली), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, स्थिरांक TValue *किल्ली, TValue *res) {
  स्थिरांक TValue *slot;
  निवड (ttypetag(किल्ली)) {
    स्थिती LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(किल्ली));
      थांबा;
    स्थिती LUA_VNUMINT:
      परत luaH_getint(t, ivalue(किल्ली), res);
    स्थिती LUA_VNIL:
      slot = &absentkey;
      थांबा;
    स्थिती LUA_VNUMFLT: {
      lua_Integer k;
      जर (luaV_flttointeger(fltvalue(किल्ली), &k, F2Ieq)) /* integral index? */
        परत luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    पूर्वनिर्धारित:
      slot = getgeneric(t, किल्ली, 0);
      थांबा;
  }
  परत finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
स्थिर पूर्णांक retpsetcode (Table *t, स्थिरांक TValue *slot) {
  जर (isabstkey(slot))
    परत HNOTFOUND;  /* no slot with that key */
  अन्यथा  /* return node encoded */
    परत cast_int((cast(गाठ_2*, slot) - t->गाठ)) + HFIRSTNODE;
}


स्थिर पूर्णांक finishnodeset (Table *t, स्थिरांक TValue *slot, TValue *val) {
  जर (!ttisnil(slot)) {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    परत HOK;  /* success */
  }
  अन्यथा
    परत retpsetcode(t, slot);
}


स्थिर पूर्णांक rawfinishnodeset (स्थिरांक TValue *slot, TValue *val) {
  जर (isabstkey(slot))
    परत 0;  /* no slot with that key */
  अन्यथा {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    परत 1;  /* success */
  }
}


पूर्णांक luaH_psetint (Table *t, lua_Integer किल्ली, TValue *val) {
  lua_assert(!ikeyinarray(t, किल्ली));
  परत finishnodeset(t, getintfromhash(t, किल्ली), val);
}


स्थिर पूर्णांक psetint (Table *t, lua_Integer किल्ली, TValue *val) {
  पूर्णांक hres;
  luaH_fastseti(t, किल्ली, val, hres);
  परत hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

पूर्णांक luaH_psetshortstr (Table *t, TString *किल्ली, TValue *val) {
  स्थिरांक TValue *slot = luaH_Hgetshortstr(t, किल्ली);
  जर (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);  /* update it */
    परत HOK;  /* done */
  }
  अन्यथा जर (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    जर (ttisnil(val))  /* new value is nil? */
      परत HOK;  /* done (value is already nil/absent) */
    जर (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(किल्ली))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, रिक्त), &tk, किल्ली);
      जर (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        परत HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  परत retpsetcode(t, slot);
}


पूर्णांक luaH_psetstr (Table *t, TString *किल्ली, TValue *val) {
  जर (strisshr(किल्ली))
    परत luaH_psetshortstr(t, किल्ली, val);
  अन्यथा
    परत finishnodeset(t, Hgetlongstr(t, किल्ली), val);
}


पूर्णांक luaH_pset (Table *t, स्थिरांक TValue *किल्ली, TValue *val) {
  निवड (ttypetag(किल्ली)) {
    स्थिती LUA_VSHRSTR: परत luaH_psetshortstr(t, tsvalue(किल्ली), val);
    स्थिती LUA_VNUMINT: परत psetint(t, ivalue(किल्ली), val);
    स्थिती LUA_VNIL: परत HNOTFOUND;
    स्थिती LUA_VNUMFLT: {
      lua_Integer k;
      जर (luaV_flttointeger(fltvalue(किल्ली), &k, F2Ieq)) /* integral index? */
        परत psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    पूर्वनिर्धारित:
      परत finishnodeset(t, getgeneric(t, किल्ली, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
शून्य luaH_finishset (lua_State *L, Table *t, स्थिरांक TValue *किल्ली,
                                    TValue *मूल्य, पूर्णांक hres) {
  lua_assert(hres != HOK);
  जर (hres == HNOTFOUND) {
    TValue aux;
    स्थिरांक TValue *actk = किल्ली;  /* actual key to insert */
    जर (l_unlikely(ttisnil(किल्ली)))
      luaG_runerror(L, "table index is nil");
    अन्यथा जर (ttisfloat(किल्ली)) {
      lua_संख्या f = fltvalue(किल्ली);
      lua_Integer k;
      जर (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      अन्यथा जर (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    अन्यथा जर (isextstr(किल्ली)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(किल्ली));
      setsvalue2s(L, L->शीर्ष.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->शीर्ष.p - 1), मूल्य);
      L->शीर्ष.p--;
      परत;
    }
    luaH_newkey(L, t, actk, मूल्य);
  }
  अन्यथा जर (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), मूल्य);
  }
  अन्यथा {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), मूल्य);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
शून्य luaH_set (lua_State *L, Table *t, स्थिरांक TValue *किल्ली, TValue *मूल्य) {
  पूर्णांक hres = luaH_pset(t, किल्ली, मूल्य);
  जर (hres != HOK)
    luaH_finishset(L, t, किल्ली, मूल्य, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
शून्य luaH_setint (lua_State *L, Table *t, lua_Integer किल्ली, TValue *मूल्य) {
  अचिन्हित ik = ikeyinarray(t, किल्ली);
  जर (ik > 0)
    obj2arr(t, ik - 1, मूल्य);
  अन्यथा {
    पूर्णांक ok = rawfinishnodeset(getintfromhash(t, किल्ली), मूल्य);
    जर (!ok) {
      TValue k;
      setivalue(&k, किल्ली);
      luaH_newkey(L, t, &k, मूल्य);
    }
  }
}


/*
** Try to find a boundary in the hash part of table 't'. From the
** caller, we know that 'asize + 1' is present. We want to find a larger
** key that is absent from the table, so that we can do a binary search
** between the two keys to find a boundary. We keep doubling 'j' until
** we get an absent index.  If the doubling would overflow, we try
** LUA_MAXINTEGER. If it is absent, we are ready for the binary search.
** ('j', being max integer, is larger or equal to 'i', but it cannot be
** equal because it is absent while 'i' is present.) Otherwise, 'j' is a
** boundary. ('j + 1' cannot be a present integer key because it is not
** a valid integer in Lua.)
** About 'rnd': If we used a fixed algorithm, a bad actor could fill
** a table with only the keys that would be probed, in such a way that
** a small table could result in a huge length. To avoid that, we use
** the state's seed as a source of randomness. For the first probe,
** we "randomly double" 'i' by adding to it a random number roughly its
** width.
*/
स्थिर lua_Unsigned hash_शोध (lua_State *L, Table *t, अचिन्हित asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  अचिन्हित rnd = G(L)->seed;
  पूर्णांक n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  अचिन्हित mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  अचिन्हित incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  पर्यंत (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    जर (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    अन्यथा {
      j = LUA_MAXINTEGER;
      जर (hashkeyisempty(t, j))  /* t[j] not present? */
        थांबा;  /* 'j' now is an absent index */
      अन्यथा  /* weird case */
        परत j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  पर्यंत (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    जर (hashkeyisempty(t, m)) j = m;
    अन्यथा i = m;
  }
  परत i;
}


स्थिर अचिन्हित पूर्णांक binsearch (Table *array, अचिन्हित पूर्णांक i, अचिन्हित पूर्णांक j) {
  lua_assert(i <= j);
  पर्यंत (j - i > 1u) {  /*द्विमान शोध*/
    अचिन्हित पूर्णांक m = (i + j) / 2;
    जर (arraykeyisempty(array, m)) j = m;
    अन्यथा i = m;
  }
  परत i;
}


/* return a border, saving it as a hint for next call */
स्थिर lua_Unsigned newhint (Table *t, अचिन्हित hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  परत hint;
}


/*
** Try to find a border in table 't'. (A 'border' is an integer index
** such that t[i] is present and t[i+1] is absent, or 0 if t[1] is absent,
** or 'maxinteger' if t[maxinteger] is present.)
** If there is an array part, try to find a border there. First try
** to find it in the vicinity of the previous result (hint), to handle
** cases like 't[#t + 1] = val' or 't[#t] = nil', that move the border
** by one entry. Otherwise, do a binary search to find the border.
** If there is no array part, or its last element is non empty, the
** border may be in the hash part.
*/
lua_Unsigned luaH_getn (lua_State *L, Table *t) {
  अचिन्हित asize = t->asize;
  जर (asize > 0) {  /* is there an array part? */
    स्थिरांक अचिन्हित maxvicinity = 4;
    अचिन्हित limit = *lenhint(t);  /* start with the hint */
    जर (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    जर (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      साठी (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        जर (!arraykeyisempty(t, limit))
          परत newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      परत newhint(t, binsearch(t, 0, limit));
    }
    अन्यथा {  /* 'limit' is present in table; look for a border after it */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      साठी (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        जर (arraykeyisempty(t, limit))
          परत newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      जर (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        परत newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  जर (isdummy(t) || hashkeyisempty(t, asize + 1))
    परत asize;  /* 'asize + 1' is empty */
  अन्यथा  /* 'asize + 1' is also non empty */
    परत hash_शोध(L, t, asize);
}



#जर defined(LUA_DEBUG)

/* export this function for the test library */

गाठ_2 *luaH_mainposition (स्थिरांक Table *t, स्थिरांक TValue *किल्ली) {
  परत mainpositionTV(t, किल्ली);
}

#endif
