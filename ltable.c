/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#घोषणा ltable_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


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

#समावेश <math.h>
#समावेश <limits.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lgc.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#घोषणा LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
प्रकार काष्ठा { गाँठ_2 *dummy; गाँठ_2 follows_p_गाँठ; } Limbox_aux;

प्रकार जोड़ {
  गाँठ_2 *lastfree;
  अक्षर padding[दुरत्व(Limbox_aux, follows_p_गाँठ)];
} Limbox;

#घोषणा haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#घोषणा getlastfree(t)     ((cast(Limbox *, (t)->गाँठ) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#घोषणा MAXABITS	(l_numbits(पूर्णांक) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#घोषणा MAXASIZEB	(MAX_SIZET/(माप(मूल्य_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#घोषणा MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#घोषणा MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#घोषणा MAXHSIZE	luaM_limitN(1 << MAXHBITS, गाँठ_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#घोषणा hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#घोषणा hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#घोषणा hashstr(t,str)		hashpow2(t, (str)->hash)
#घोषणा hashboolean(t,p)	hashpow2(t, p)


#घोषणा hashpointer(t,p)	hashmod(t, point2uint(p))


#घोषणा dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
अटल स्थिर गाँठ_2 dummynode_ = {
  {{शून्य}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {शून्य}}  /* key type, next, and key value */
};


अटल स्थिर TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
अटल गाँठ_2 *hashint (स्थिर Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  अगर (ui <= cast_uint(पूर्णांक_उच्च))
    वापस gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  अन्यथा
    वापस hashmod(t, ui);
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
#अगर !defined(l_hashfloat)
अटल अचिन्हित l_hashfloat (lua_संख्या n) {
  पूर्णांक i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_अंक(पूर्णांक_न्यून);
  अगर (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(भ_असल)(n) == cast_अंक(विशाल_मान));
    वापस 0;
  }
  अन्यथा {  /* normal case */
    अचिन्हित पूर्णांक u = cast_uint(i) + cast_uint(ni);
    वापस (u <= cast_uint(पूर्णांक_उच्च) ? u : ~u);
  }
}
#पूर्ण_अगर


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
अटल गाँठ_2 *mainpositionTV (स्थिर Table *t, स्थिर TValue *कुंजी) {
  चयन (ttypetag(कुंजी)) {
    हाल LUA_VNUMINT: {
      lua_Integer i = ivalue(कुंजी);
      वापस hashint(t, i);
    }
    हाल LUA_VNUMFLT: {
      lua_संख्या n = fltvalue(कुंजी);
      वापस hashmod(t, l_hashfloat(n));
    }
    हाल LUA_VSHRSTR: {
      TString *ts = tsvalue(कुंजी);
      वापस hashstr(t, ts);
    }
    हाल LUA_VLNGSTR: {
      TString *ts = tsvalue(कुंजी);
      वापस hashpow2(t, luaS_hashlongstr(ts));
    }
    हाल LUA_VFALSE:
      वापस hashboolean(t, 0);
    हाल LUA_VTRUE:
      वापस hashboolean(t, 1);
    हाल LUA_VLIGHTUSERDATA: {
      व्योम *p = pvalue(कुंजी);
      वापस hashpointer(t, p);
    }
    हाल LUA_VLCF: {
      lua_CFunction f = fvalue(कुंजी);
      वापस hashpointer(t, f);
    }
    शेष: {
      GCObject *o = gcvalue(कुंजी);
      वापस hashpointer(t, o);
    }
  }
}


l_sinline गाँठ_2 *mainpositionfromnode (स्थिर Table *t, गाँठ_2 *nd) {
  TValue कुंजी;
  getnodekey(cast(lua_State *, शून्य), &कुंजी, nd);
  वापस mainpositionTV(t, &कुंजी);
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
अटल पूर्णांक equalkey (स्थिर TValue *k1, स्थिर गाँठ_2 *n2, पूर्णांक deadok) {
  अगर (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    अगर (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      वापस luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    अन्यथा अगर (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      वापस gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   अन्यथा
     वापस 0;  /* otherwise, different variants cannot be equal */
  }
  अन्यथा {  /* equal variants */
    चयन (keytt(n2)) {
      हाल LUA_VNIL: हाल LUA_VFALSE: हाल LUA_VTRUE:
        वापस 1;
      हाल LUA_VNUMINT:
        वापस (ivalue(k1) == keyival(n2));
      हाल LUA_VNUMFLT:
        वापस luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      हाल LUA_VLIGHTUSERDATA:
        वापस pvalue(k1) == pvalueraw(keyval(n2));
      हाल LUA_VLCF:
        वापस fvalue(k1) == fvalueraw(keyval(n2));
      हाल ctb(LUA_VLNGSTR):
        वापस luaS_eqstr(tsvalue(k1), keystrval(n2));
      शेष:
        वापस gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
अटल स्थिर TValue *getgeneric (Table *t, स्थिर TValue *कुंजी, पूर्णांक deadok) {
  गाँठ_2 *n = mainpositionTV(t, कुंजी);
  क्रम (;;) {  /* check whether 'key' is somewhere in the chain */
    अगर (equalkey(कुंजी, n, deadok))
      वापस gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      अगर (nx == 0)
        वापस &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
अटल अचिन्हित checkrange (lua_Integer k, अचिन्हित limit) {
  वापस (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#घोषणा arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#घोषणा ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
अटल अचिन्हित keyinarray (Table *t, स्थिर TValue *कुंजी) {
  वापस (ttisinteger(कुंजी)) ? ikeyinarray(t, ivalue(कुंजी)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
अटल अचिन्हित findindex (lua_State *L, Table *t, TValue *कुंजी,
                               अचिन्हित asize) {
  अचिन्हित पूर्णांक i;
  अगर (ttisnil(कुंजी)) वापस 0;  /* first iteration */
  i = keyinarray(t, कुंजी);
  अगर (i != 0)  /* is 'key' inside array part? */
    वापस i;  /* yes; that's the index */
  अन्यथा {
    स्थिर TValue *n = getgeneric(t, कुंजी, 1);
    अगर (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    वापस (i + 1) + asize;
  }
}


पूर्णांक lua_H_अग्र (lua_State *L, Table *t, StkId कुंजी) {
  अचिन्हित पूर्णांक asize = t->asize;
  अचिन्हित पूर्णांक i = findindex(L, t, s2v(कुंजी), asize);  /* find original key */
  क्रम (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    अगर (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(कुंजी), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(कुंजी + 1));
      वापस 1;
    }
  }
  क्रम (i -= asize; i < sizenode(t); i++) {  /* hash part */
    अगर (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      गाँठ_2 *n = gnode(t, i);
      getnodekey(L, s2v(कुंजी), n);
      setobj2s(L, कुंजी + 1, gval(n));
      वापस 1;
    }
  }
  वापस 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#घोषणा extraLastfree(t)	(haslastfree(t) ? माप(Limbox) : 0)

/* 'node' size in bytes */
अटल माप_प्रकार sizehash (Table *t) {
  वापस cast_sizet(sizenode(t)) * माप(गाँठ_2) + extraLastfree(t);
}


अटल व्योम freehash (lua_State *L, Table *t) {
  अगर (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    अक्षर *arr = cast_charp(t->गाँठ) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

अटल पूर्णांक insertkey (Table *t, स्थिर TValue *कुंजी, TValue *मूल्य);
अटल व्योम newcheckedkey (Table *t, स्थिर TValue *कुंजी, TValue *मूल्य);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
प्रकार काष्ठा {
  अचिन्हित योग;
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
#घोषणा arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
अटल अचिन्हित computesizes (Counters *ct) {
  पूर्णांक i;
  अचिन्हित पूर्णांक twotoi;  /* 2^i (candidate for optimal size) */
  अचिन्हित पूर्णांक a = 0;  /* number of elements smaller than 2^i */
  अचिन्हित पूर्णांक na = 0;  /* number of elements to go to array part */
  अचिन्हित पूर्णांक optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  क्रम (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    अचिन्हित nums = ct->nums[i];
    a += nums;
    अगर (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  वापस optimal;
}


अटल व्योम countint (lua_Integer कुंजी, Counters *ct) {
  अचिन्हित पूर्णांक k = arrayindex(कुंजी);
  अगर (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline पूर्णांक arraykeyisempty (स्थिर Table *t, अचिन्हित कुंजी) {
  पूर्णांक tag = *getArrTag(t, कुंजी - 1);
  वापस tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
अटल व्योम numusearray (स्थिर Table *t, Counters *ct) {
  पूर्णांक lg;
  अचिन्हित पूर्णांक ttlg;  /* 2^lg */
  अचिन्हित पूर्णांक ause = 0;  /* summation of 'nums' */
  अचिन्हित पूर्णांक i = 1;  /* index to traverse all array keys */
  अचिन्हित पूर्णांक asize = t->asize;
  /* traverse each slice */
  क्रम (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    अचिन्हित पूर्णांक lc = 0;  /*गणक*/
    अचिन्हित पूर्णांक lim = ttlg;
    अगर (lim > asize) {
      lim = asize;  /* adjust upper limit */
      अगर (i > lim)
        अवरोध;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    क्रम (; i <= lim; i++) {
      अगर (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->योग += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
अटल व्योम numusehash (स्थिर Table *t, Counters *ct) {
  अचिन्हित i = sizenode(t);
  अचिन्हित योग = 0;
  जबतक (i--) {
    गाँठ_2 *n = &t->गाँठ[i];
    अगर (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    अन्यथा {
      योग++;
      अगर (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->योग += योग;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
अटल माप_प्रकार concretesize (अचिन्हित पूर्णांक आकार) {
  अगर (आकार == 0)
    वापस 0;
  अन्यथा  /* space for the two arrays plus an unsigned in between */
    वापस आकार * (माप(मूल्य_2) + 1) + माप(अचिन्हित);
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
अटल मूल्य_2 *resizearray (lua_State *L , Table *t,
                               अचिन्हित oldasize,
                               अचिन्हित newasize) {
  अगर (oldasize == newasize)
    वापस t->array;  /* nothing to be done */
  अन्यथा अगर (newasize == 0) {  /* erasing array? */
    मूल्य_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    वापस शून्य;
  }
  अन्यथा {
    माप_प्रकार newasizeb = concretesize(newasize);
    मूल्य_2 *np = cast(मूल्य_2 *,
                  luaM_reallocvector(L, शून्य, 0, newasizeb, lu_byte));
    अगर (np == शून्य)  /* allocation error? */
      वापस शून्य;
    np += newasize;  /* shift pointer to the end of value segment */
    अगर (oldasize > 0) {
      /* move common elements to new position */
      माप_प्रकार oldasizeb = concretesize(oldasize);
      मूल्य_2 *op = t->array;  /* original array */
      अचिन्हित tomove = (oldasize < newasize) ? oldasize : newasize;
      माप_प्रकार tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      स_नकल(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    वापस np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
अटल व्योम setnodevector (lua_State *L, Table *t, अचिन्हित आकार) {
  अगर (आकार == 0) {  /* no elements to hash part? */
    t->गाँठ = cast(गाँठ_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  अन्यथा {
    पूर्णांक i;
    पूर्णांक lsize = luaO_ceillog2(आकार);
    अगर (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    आकार = twoto(lsize);
    अगर (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->गाँठ = luaM_newvector(L, आकार, गाँठ_2);
    अन्यथा {
      माप_प्रकार bsize = आकार * माप(गाँठ_2) + माप(Limbox);
      अक्षर *गाँठ = luaM_newblock(L, bsize);
      t->गाँठ = cast(गाँठ_2 *, गाँठ + माप(Limbox));
      getlastfree(t) = gnode(t, आकार);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    क्रम (i = 0; i < cast_int(आकार); i++) {
      गाँठ_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
अटल व्योम reinserthash (lua_State *L, Table *ot, Table *t) {
  अचिन्हित j;
  अचिन्हित आकार = sizenode(ot);
  क्रम (j = 0; j < आकार; j++) {
    गाँठ_2 *old = gnode(ot, j);
    अगर (!isempty(gval(old))) {
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
अटल व्योम exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  गाँठ_2 *गाँठ = t1->गाँठ;
  पूर्णांक bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->गाँठ = t2->गाँठ;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->गाँठ = गाँठ;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
अटल व्योम reinsertOldSlice (Table *t, अचिन्हित oldasize,
                                        अचिन्हित newasize) {
  अचिन्हित i;
  क्रम (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    अगर (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue कुंजी, aux;
      setivalue(&कुंजी, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &कुंजी, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
अटल व्योम clearNewSlice (Table *t, अचिन्हित oldasize, अचिन्हित newasize) {
  क्रम (; oldasize < newasize; oldasize++)
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
व्योम luaH_resize (lua_State *L, Table *t, अचिन्हित newasize,
                                          अचिन्हित nhsize) {
  Table newt;  /* to keep the new hash part */
  अचिन्हित oldasize = t->asize;
  मूल्य_2 *newarray;
  अगर (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  अगर (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  अगर (l_unlikely(newarray == शून्य && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  अगर (newarray != शून्य)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


व्योम luaH_resizearray (lua_State *L, Table *t, अचिन्हित पूर्णांक nasize) {
  अचिन्हित nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
अटल व्योम rehash (lua_State *L, Table *t, स्थिर TValue *ek) {
  अचिन्हित asize;  /* optimal size for array part */
  Counters ct;
  अचिन्हित i;
  अचिन्हित nsize;  /* size for the hash part */
  /* reset counts */
  क्रम (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.योग = 1;  /* count extra key */
  अगर (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  अगर (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  अन्यथा {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.योग - ct.na;
  अगर (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, माप(Table));
  Table *t = gco2t(o);
  t->metatable = शून्य;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = शून्य;
  t->asize = 0;
  setnodevector(L, t, 0);
  वापस t;
}


lu_mem lua_H_आकार (Table *t) {
  lu_mem sz = cast(lu_mem, माप(Table)) + concretesize(t->asize);
  अगर (!isdummy(t))
    sz += sizehash(t);
  वापस sz;
}


/*
** Frees a table.
*/
व्योम luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


अटल गाँठ_2 *getfreepos (Table *t) {
  अगर (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    जबतक (getlastfree(t) > t->गाँठ) {
      गाँठ_2 *मुक्त = --getlastfree(t);
      अगर (keyisnil(मुक्त))
        वापस मुक्त;
    }
  }
  अन्यथा {  /* no 'lastfree' information */
    अचिन्हित i = sizenode(t);
    जबतक (i--) {  /* do a linear search */
      गाँठ_2 *मुक्त = gnode(t, i);
      अगर (keyisnil(मुक्त))
        वापस मुक्त;
    }
  }
  वापस शून्य;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
अटल पूर्णांक insertkey (Table *t, स्थिर TValue *कुंजी, TValue *मूल्य) {
  गाँठ_2 *mp = mainpositionTV(t, कुंजी);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, कुंजी, 0)));
  अगर (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    गाँठ_2 *othern;
    गाँठ_2 *f = getfreepos(t);  /* get a free place */
    अगर (f == शून्य)  /* cannot find a free place? */
      वापस 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    अगर (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      जबतक (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      अगर (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    अन्यथा {  /* colliding node is in its own main position */
      /* new node will go into free position */
      अगर (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      अन्यथा lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, कुंजी);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), मूल्य);
  वापस 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
अटल व्योम newcheckedkey (Table *t, स्थिर TValue *कुंजी, TValue *मूल्य) {
  अचिन्हित i = keyinarray(t, कुंजी);
  अगर (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, मूल्य);  /* set value in the array */
  अन्यथा {
    पूर्णांक done = insertkey(t, कुंजी, मूल्य);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(व्योम, done);  /* to avoid warnings */
  }
}


अटल व्योम luaH_newkey (lua_State *L, Table *t, स्थिर TValue *कुंजी,
                                                 TValue *मूल्य) {
  अगर (!ttisnil(मूल्य)) {  /* do not insert nil values */
    पूर्णांक done = insertkey(t, कुंजी, मूल्य);
    अगर (!done) {  /* could not find a free place? */
      rehash(L, t, कुंजी);  /* grow table */
      newcheckedkey(t, कुंजी, मूल्य);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), कुंजी);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (व्योम)0, (व्योम)0, 1);
  }
}


अटल स्थिर TValue *getintfromhash (Table *t, lua_Integer कुंजी) {
  गाँठ_2 *n = hashint(t, कुंजी);
  lua_assert(!ikeyinarray(t, कुंजी));
  क्रम (;;) {  /* check whether 'key' is somewhere in the chain */
    अगर (keyisinteger(n) && keyival(n) == कुंजी)
      वापस gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      अगर (nx == 0) अवरोध;
      n += nx;
    }
  }
  वापस &absentkey;
}


अटल पूर्णांक hashkeyisempty (Table *t, lua_Unsigned कुंजी) {
  स्थिर TValue *val = getintfromhash(t, l_castU2S(कुंजी));
  वापस isempty(val);
}


अटल lu_byte finishnodeget (स्थिर TValue *val, TValue *res) {
  अगर (!ttisnil(val)) {
    setobj(((lua_State*)शून्य), res, val);
  }
  वापस ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer कुंजी, TValue *res) {
  अचिन्हित k = ikeyinarray(t, कुंजी);
  अगर (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    अगर (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    वापस tag;
  }
  अन्यथा
    वापस finishnodeget(getintfromhash(t, कुंजी), res);
}


/*
** search function for short strings
*/
स्थिर TValue *luaH_Hgetshortstr (Table *t, TString *कुंजी) {
  गाँठ_2 *n = hashstr(t, कुंजी);
  lua_assert(strisshr(कुंजी));
  क्रम (;;) {  /* check whether 'key' is somewhere in the chain */
    अगर (keyisshrstr(n) && eqshrstr(keystrval(n), कुंजी))
      वापस gval(n);  /* that's it */
    अन्यथा {
      पूर्णांक nx = gnext(n);
      अगर (nx == 0)
        वापस &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *कुंजी, TValue *res) {
  वापस finishnodeget(luaH_Hgetshortstr(t, कुंजी), res);
}


अटल स्थिर TValue *Hgetlongstr (Table *t, TString *कुंजी) {
  TValue ko;
  lua_assert(!strisshr(कुंजी));
  setsvalue(cast(lua_State *, शून्य), &ko, कुंजी);
  वापस getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


अटल स्थिर TValue *Hgetstr (Table *t, TString *कुंजी) {
  अगर (strisshr(कुंजी))
    वापस luaH_Hgetshortstr(t, कुंजी);
  अन्यथा
    वापस Hgetlongstr(t, कुंजी);
}


lu_byte luaH_getstr (Table *t, TString *कुंजी, TValue *res) {
  वापस finishnodeget(Hgetstr(t, कुंजी), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, स्थिर TValue *कुंजी, TValue *res) {
  स्थिर TValue *slot;
  चयन (ttypetag(कुंजी)) {
    हाल LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(कुंजी));
      अवरोध;
    हाल LUA_VNUMINT:
      वापस luaH_getint(t, ivalue(कुंजी), res);
    हाल LUA_VNIL:
      slot = &absentkey;
      अवरोध;
    हाल LUA_VNUMFLT: {
      lua_Integer k;
      अगर (luaV_flttointeger(fltvalue(कुंजी), &k, F2Ieq)) /* integral index? */
        वापस luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    शेष:
      slot = getgeneric(t, कुंजी, 0);
      अवरोध;
  }
  वापस finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
अटल पूर्णांक retpsetcode (Table *t, स्थिर TValue *slot) {
  अगर (isabstkey(slot))
    वापस HNOTFOUND;  /* no slot with that key */
  अन्यथा  /* return node encoded */
    वापस cast_int((cast(गाँठ_2*, slot) - t->गाँठ)) + HFIRSTNODE;
}


अटल पूर्णांक finishnodeset (Table *t, स्थिर TValue *slot, TValue *val) {
  अगर (!ttisnil(slot)) {
    setobj(((lua_State*)शून्य), cast(TValue*, slot), val);
    वापस HOK;  /* success */
  }
  अन्यथा
    वापस retpsetcode(t, slot);
}


अटल पूर्णांक rawfinishnodeset (स्थिर TValue *slot, TValue *val) {
  अगर (isabstkey(slot))
    वापस 0;  /* no slot with that key */
  अन्यथा {
    setobj(((lua_State*)शून्य), cast(TValue*, slot), val);
    वापस 1;  /* success */
  }
}


पूर्णांक luaH_psetint (Table *t, lua_Integer कुंजी, TValue *val) {
  lua_assert(!ikeyinarray(t, कुंजी));
  वापस finishnodeset(t, getintfromhash(t, कुंजी), val);
}


अटल पूर्णांक psetint (Table *t, lua_Integer कुंजी, TValue *val) {
  पूर्णांक hres;
  luaH_fastseti(t, कुंजी, val, hres);
  वापस hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

पूर्णांक luaH_psetshortstr (Table *t, TString *कुंजी, TValue *val) {
  स्थिर TValue *slot = luaH_Hgetshortstr(t, कुंजी);
  अगर (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)शून्य), cast(TValue*, slot), val);  /* update it */
    वापस HOK;  /* done */
  }
  अन्यथा अगर (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    अगर (ttisnil(val))  /* new value is nil? */
      वापस HOK;  /* done (value is already nil/absent) */
    अगर (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(कुंजी))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, शून्य), &tk, कुंजी);
      अगर (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        वापस HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  वापस retpsetcode(t, slot);
}


पूर्णांक luaH_psetstr (Table *t, TString *कुंजी, TValue *val) {
  अगर (strisshr(कुंजी))
    वापस luaH_psetshortstr(t, कुंजी, val);
  अन्यथा
    वापस finishnodeset(t, Hgetlongstr(t, कुंजी), val);
}


पूर्णांक luaH_pset (Table *t, स्थिर TValue *कुंजी, TValue *val) {
  चयन (ttypetag(कुंजी)) {
    हाल LUA_VSHRSTR: वापस luaH_psetshortstr(t, tsvalue(कुंजी), val);
    हाल LUA_VNUMINT: वापस psetint(t, ivalue(कुंजी), val);
    हाल LUA_VNIL: वापस HNOTFOUND;
    हाल LUA_VNUMFLT: {
      lua_Integer k;
      अगर (luaV_flttointeger(fltvalue(कुंजी), &k, F2Ieq)) /* integral index? */
        वापस psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    शेष:
      वापस finishnodeset(t, getgeneric(t, कुंजी, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
व्योम luaH_finishset (lua_State *L, Table *t, स्थिर TValue *कुंजी,
                                    TValue *मूल्य, पूर्णांक hres) {
  lua_assert(hres != HOK);
  अगर (hres == HNOTFOUND) {
    TValue aux;
    स्थिर TValue *actk = कुंजी;  /* actual key to insert */
    अगर (l_unlikely(ttisnil(कुंजी)))
      luaG_runerror(L, "table index is nil");
    अन्यथा अगर (ttisfloat(कुंजी)) {
      lua_संख्या f = fltvalue(कुंजी);
      lua_Integer k;
      अगर (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      अन्यथा अगर (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    अन्यथा अगर (isextstr(कुंजी)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(कुंजी));
      setsvalue2s(L, L->शीर्ष.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->शीर्ष.p - 1), मूल्य);
      L->शीर्ष.p--;
      वापस;
    }
    luaH_newkey(L, t, actk, मूल्य);
  }
  अन्यथा अगर (hres > 0) {  /* regular Node? */
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
व्योम luaH_set (lua_State *L, Table *t, स्थिर TValue *कुंजी, TValue *मूल्य) {
  पूर्णांक hres = luaH_pset(t, कुंजी, मूल्य);
  अगर (hres != HOK)
    luaH_finishset(L, t, कुंजी, मूल्य, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
व्योम luaH_setint (lua_State *L, Table *t, lua_Integer कुंजी, TValue *मूल्य) {
  अचिन्हित ik = ikeyinarray(t, कुंजी);
  अगर (ik > 0)
    obj2arr(t, ik - 1, मूल्य);
  अन्यथा {
    पूर्णांक ok = rawfinishnodeset(getintfromhash(t, कुंजी), मूल्य);
    अगर (!ok) {
      TValue k;
      setivalue(&k, कुंजी);
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
अटल lua_Unsigned hash_खोज (lua_State *L, Table *t, अचिन्हित asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  अचिन्हित rnd = G(L)->seed;
  पूर्णांक n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  अचिन्हित mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  अचिन्हित incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  जबतक (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    अगर (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    अन्यथा {
      j = LUA_MAXINTEGER;
      अगर (hashkeyisempty(t, j))  /* t[j] not present? */
        अवरोध;  /* 'j' now is an absent index */
      अन्यथा  /* weird case */
        वापस j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  जबतक (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    अगर (hashkeyisempty(t, m)) j = m;
    अन्यथा i = m;
  }
  वापस i;
}


अटल अचिन्हित पूर्णांक binsearch (Table *array, अचिन्हित पूर्णांक i, अचिन्हित पूर्णांक j) {
  lua_assert(i <= j);
  जबतक (j - i > 1u) {  /*द्विआधारी खोज*/
    अचिन्हित पूर्णांक m = (i + j) / 2;
    अगर (arraykeyisempty(array, m)) j = m;
    अन्यथा i = m;
  }
  वापस i;
}


/* return a border, saving it as a hint for next call */
अटल lua_Unsigned newhint (Table *t, अचिन्हित hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  वापस hint;
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
  अगर (asize > 0) {  /* is there an array part? */
    स्थिर अचिन्हित maxvicinity = 4;
    अचिन्हित limit = *lenhint(t);  /* start with the hint */
    अगर (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    अगर (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      क्रम (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        अगर (!arraykeyisempty(t, limit))
          वापस newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      वापस newhint(t, binsearch(t, 0, limit));
    }
    अन्यथा {  /* 'limit' is present in table; look for a border after it */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      क्रम (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        अगर (arraykeyisempty(t, limit))
          वापस newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      अगर (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        वापस newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  अगर (isdummy(t) || hashkeyisempty(t, asize + 1))
    वापस asize;  /* 'asize + 1' is empty */
  अन्यथा  /* 'asize + 1' is also non empty */
    वापस hash_खोज(L, t, asize);
}



#अगर defined(LUA_DEBUG)

/* export this function for the test library */

गाँठ_2 *luaH_mainposition (स्थिर Table *t, स्थिर TValue *कुंजी) {
  वापस mainpositionTV(t, कुंजी);
}

#पूर्ण_अगर
