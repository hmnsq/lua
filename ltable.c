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
प्रकारनिर्देश संरचना { गाँठो_2 *dummy; गाँठो_2 follows_p_गाँठो; } Limbox_aux;

प्रकारनिर्देश सङ्घ {
  गाँठो_2 *lastfree;
  वर्ण padding[दुरत्व(Limbox_aux, follows_p_गाँठो)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->गाँठो) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(पूर्णाङ्क) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(परिमाण(मान_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, गाँठो_2)


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
स्थिर अचर गाँठो_2 dummynode_ = {
  {{रिक्त}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {रिक्त}}  /* key type, next, and key value */
};


स्थिर अचर TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
स्थिर गाँठो_2 *hashint (अचर Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  यदि (ui <= cast_uint(पूर्णांक_उच्च))
    फर्काऊ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  अन्यथा
    फर्काऊ hashmod(t, ui);
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
#यदि !defined(l_hashfloat)
स्थिर अचिन्हित l_hashfloat (lua_सङ्ख्या n) {
  पूर्णाङ्क i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_अङ्क(पूर्णांक_न्यून);
  यदि (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(भ_असल)(n) == cast_अङ्क(विशाल_मान));
    फर्काऊ 0;
  }
  अन्यथा {  /* normal case */
    अचिन्हित पूर्णाङ्क u = cast_uint(i) + cast_uint(ni);
    फर्काऊ (u <= cast_uint(पूर्णांक_उच्च) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
स्थिर गाँठो_2 *mainpositionTV (अचर Table *t, अचर TValue *साँचो) {
  विकल्प (ttypetag(साँचो)) {
    पक्ष LUA_VNUMINT: {
      lua_Integer i = ivalue(साँचो);
      फर्काऊ hashint(t, i);
    }
    पक्ष LUA_VNUMFLT: {
      lua_सङ्ख्या n = fltvalue(साँचो);
      फर्काऊ hashmod(t, l_hashfloat(n));
    }
    पक्ष LUA_VSHRSTR: {
      TString *ts = tsvalue(साँचो);
      फर्काऊ hashstr(t, ts);
    }
    पक्ष LUA_VLNGSTR: {
      TString *ts = tsvalue(साँचो);
      फर्काऊ hashpow2(t, luaS_hashlongstr(ts));
    }
    पक्ष LUA_VFALSE:
      फर्काऊ hashboolean(t, 0);
    पक्ष LUA_VTRUE:
      फर्काऊ hashboolean(t, 1);
    पक्ष LUA_VLIGHTUSERDATA: {
      शून्य *p = pvalue(साँचो);
      फर्काऊ hashpointer(t, p);
    }
    पक्ष LUA_VLCF: {
      lua_CFunction f = fvalue(साँचो);
      फर्काऊ hashpointer(t, f);
    }
    स्वभाव: {
      GCObject *o = gcvalue(साँचो);
      फर्काऊ hashpointer(t, o);
    }
  }
}


l_sinline गाँठो_2 *mainpositionfromnode (अचर Table *t, गाँठो_2 *nd) {
  TValue साँचो;
  getnodekey(cast(lua_State *, रिक्त), &साँचो, nd);
  फर्काऊ mainpositionTV(t, &साँचो);
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
स्थिर पूर्णाङ्क equalkey (अचर TValue *k1, अचर गाँठो_2 *n2, पूर्णाङ्क deadok) {
  यदि (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    यदि (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      फर्काऊ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    अन्यथा यदि (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      फर्काऊ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   अन्यथा
     फर्काऊ 0;  /* otherwise, different variants cannot be equal */
  }
  अन्यथा {  /* equal variants */
    विकल्प (keytt(n2)) {
      पक्ष LUA_VNIL: पक्ष LUA_VFALSE: पक्ष LUA_VTRUE:
        फर्काऊ 1;
      पक्ष LUA_VNUMINT:
        फर्काऊ (ivalue(k1) == keyival(n2));
      पक्ष LUA_VNUMFLT:
        फर्काऊ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      पक्ष LUA_VLIGHTUSERDATA:
        फर्काऊ pvalue(k1) == pvalueraw(keyval(n2));
      पक्ष LUA_VLCF:
        फर्काऊ fvalue(k1) == fvalueraw(keyval(n2));
      पक्ष ctb(LUA_VLNGSTR):
        फर्काऊ luaS_eqstr(tsvalue(k1), keystrval(n2));
      स्वभाव:
        फर्काऊ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
स्थिर अचर TValue *getgeneric (Table *t, अचर TValue *साँचो, पूर्णाङ्क deadok) {
  गाँठो_2 *n = mainpositionTV(t, साँचो);
  लागि (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (equalkey(साँचो, n, deadok))
      फर्काऊ gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0)
        फर्काऊ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
स्थिर अचिन्हित checkrange (lua_Integer k, अचिन्हित limit) {
  फर्काऊ (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
स्थिर अचिन्हित keyinarray (Table *t, अचर TValue *साँचो) {
  फर्काऊ (ttisinteger(साँचो)) ? ikeyinarray(t, ivalue(साँचो)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
स्थिर अचिन्हित findindex (lua_State *L, Table *t, TValue *साँचो,
                               अचिन्हित asize) {
  अचिन्हित पूर्णाङ्क i;
  यदि (ttisnil(साँचो)) फर्काऊ 0;  /* first iteration */
  i = keyinarray(t, साँचो);
  यदि (i != 0)  /* is 'key' inside array part? */
    फर्काऊ i;  /* yes; that's the index */
  अन्यथा {
    अचर TValue *n = getgeneric(t, साँचो, 1);
    यदि (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    फर्काऊ (i + 1) + asize;
  }
}


पूर्णाङ्क luaH_next (lua_State *L, Table *t, StkId साँचो) {
  अचिन्हित पूर्णाङ्क asize = t->asize;
  अचिन्हित पूर्णाङ्क i = findindex(L, t, s2v(साँचो), asize);  /* find original key */
  लागि (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    यदि (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(साँचो), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(साँचो + 1));
      फर्काऊ 1;
    }
  }
  लागि (i -= asize; i < sizenode(t); i++) {  /* hash part */
    यदि (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      गाँठो_2 *n = gnode(t, i);
      getnodekey(L, s2v(साँचो), n);
      setobj2s(L, साँचो + 1, gval(n));
      फर्काऊ 1;
    }
  }
  फर्काऊ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? परिमाण(Limbox) : 0)

/* 'node' size in bytes */
स्थिर माप_प्रकार sizehash (Table *t) {
  फर्काऊ cast_sizet(sizenode(t)) * परिमाण(गाँठो_2) + extraLastfree(t);
}


स्थिर शून्य freehash (lua_State *L, Table *t) {
  यदि (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    वर्ण *arr = cast_charp(t->गाँठो) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

स्थिर पूर्णाङ्क insertkey (Table *t, अचर TValue *साँचो, TValue *मान);
स्थिर शून्य newcheckedkey (Table *t, अचर TValue *साँचो, TValue *मान);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
प्रकारनिर्देश संरचना {
  अचिन्हित total;
  अचिन्हित na;
  पूर्णाङ्क deleted;
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
  पूर्णाङ्क i;
  अचिन्हित पूर्णाङ्क twotoi;  /* 2^i (candidate for optimal size) */
  अचिन्हित पूर्णाङ्क a = 0;  /* number of elements smaller than 2^i */
  अचिन्हित पूर्णाङ्क na = 0;  /* number of elements to go to array part */
  अचिन्हित पूर्णाङ्क optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  लागि (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    अचिन्हित nums = ct->nums[i];
    a += nums;
    यदि (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  फर्काऊ optimal;
}


स्थिर शून्य countint (lua_Integer साँचो, Counters *ct) {
  अचिन्हित पूर्णाङ्क k = arrayindex(साँचो);
  यदि (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline पूर्णाङ्क arraykeyisempty (अचर Table *t, अचिन्हित साँचो) {
  पूर्णाङ्क tag = *getArrTag(t, साँचो - 1);
  फर्काऊ tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
स्थिर शून्य numusearray (अचर Table *t, Counters *ct) {
  पूर्णाङ्क lg;
  अचिन्हित पूर्णाङ्क ttlg;  /* 2^lg */
  अचिन्हित पूर्णाङ्क ause = 0;  /* summation of 'nums' */
  अचिन्हित पूर्णाङ्क i = 1;  /* index to traverse all array keys */
  अचिन्हित पूर्णाङ्क asize = t->asize;
  /* traverse each slice */
  लागि (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    अचिन्हित पूर्णाङ्क lc = 0;  /* counter */
    अचिन्हित पूर्णाङ्क lim = ttlg;
    यदि (lim > asize) {
      lim = asize;  /* adjust upper limit */
      यदि (i > lim)
        रोक;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    लागि (; i <= lim; i++) {
      यदि (!arraykeyisempty(t, i))
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
स्थिर शून्य numusehash (अचर Table *t, Counters *ct) {
  अचिन्हित i = sizenode(t);
  अचिन्हित total = 0;
  जबसम्म (i--) {
    गाँठो_2 *n = &t->गाँठो[i];
    यदि (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    अन्यथा {
      total++;
      यदि (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
स्थिर माप_प्रकार concretesize (अचिन्हित पूर्णाङ्क size) {
  यदि (size == 0)
    फर्काऊ 0;
  अन्यथा  /* space for the two arrays plus an unsigned in between */
    फर्काऊ size * (परिमाण(मान_2) + 1) + परिमाण(अचिन्हित);
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
स्थिर मान_2 *resizearray (lua_State *L , Table *t,
                               अचिन्हित oldasize,
                               अचिन्हित newasize) {
  यदि (oldasize == newasize)
    फर्काऊ t->array;  /* nothing to be done */
  अन्यथा यदि (newasize == 0) {  /* erasing array? */
    मान_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    फर्काऊ रिक्त;
  }
  अन्यथा {
    माप_प्रकार newasizeb = concretesize(newasize);
    मान_2 *np = cast(मान_2 *,
                  luaM_reallocvector(L, रिक्त, 0, newasizeb, lu_byte));
    यदि (np == रिक्त)  /* allocation error? */
      फर्काऊ रिक्त;
    np += newasize;  /* shift pointer to the end of value segment */
    यदि (oldasize > 0) {
      /* move common elements to new position */
      माप_प्रकार oldasizeb = concretesize(oldasize);
      मान_2 *op = t->array;  /* original array */
      अचिन्हित tomove = (oldasize < newasize) ? oldasize : newasize;
      माप_प्रकार tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      स_नकल(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    फर्काऊ np;
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
  यदि (size == 0) {  /* no elements to hash part? */
    t->गाँठो = cast(गाँठो_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  अन्यथा {
    पूर्णाङ्क i;
    पूर्णाङ्क lsize = luaO_ceillog2(size);
    यदि (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    यदि (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->गाँठो = luaM_newvector(L, size, गाँठो_2);
    अन्यथा {
      माप_प्रकार bsize = size * परिमाण(गाँठो_2) + परिमाण(Limbox);
      वर्ण *गाँठो = luaM_newblock(L, bsize);
      t->गाँठो = cast(गाँठो_2 *, गाँठो + परिमाण(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    लागि (i = 0; i < cast_int(size); i++) {
      गाँठो_2 *n = gnode(t, i);
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
  लागि (j = 0; j < size; j++) {
    गाँठो_2 *old = gnode(ot, j);
    यदि (!isempty(gval(old))) {
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
  गाँठो_2 *गाँठो = t1->गाँठो;
  पूर्णाङ्क bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->गाँठो = t2->गाँठो;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->गाँठो = गाँठो;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
स्थिर शून्य reinsertOldSlice (Table *t, अचिन्हित oldasize,
                                        अचिन्हित newasize) {
  अचिन्हित i;
  लागि (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    यदि (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue साँचो, aux;
      setivalue(&साँचो, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &साँचो, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
स्थिर शून्य clearNewSlice (Table *t, अचिन्हित oldasize, अचिन्हित newasize) {
  लागि (; oldasize < newasize; oldasize++)
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
  मान_2 *newarray;
  यदि (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  यदि (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  यदि (l_unlikely(newarray == रिक्त && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  यदि (newarray != रिक्त)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


शून्य luaH_resizearray (lua_State *L, Table *t, अचिन्हित पूर्णाङ्क nasize) {
  अचिन्हित nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
स्थिर शून्य rehash (lua_State *L, Table *t, अचर TValue *ek) {
  अचिन्हित asize;  /* optimal size for array part */
  Counters ct;
  अचिन्हित i;
  अचिन्हित nsize;  /* size for the hash part */
  /* reset counts */
  लागि (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  यदि (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  यदि (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  अन्यथा {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  यदि (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, परिमाण(Table));
  Table *t = gco2t(o);
  t->metatable = रिक्त;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = रिक्त;
  t->asize = 0;
  setnodevector(L, t, 0);
  फर्काऊ t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, परिमाण(Table)) + concretesize(t->asize);
  यदि (!isdummy(t))
    sz += sizehash(t);
  फर्काऊ sz;
}


/*
** Frees a table.
*/
शून्य luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


स्थिर गाँठो_2 *getfreepos (Table *t) {
  यदि (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    जबसम्म (getlastfree(t) > t->गाँठो) {
      गाँठो_2 *मुक्त = --getlastfree(t);
      यदि (keyisnil(मुक्त))
        फर्काऊ मुक्त;
    }
  }
  अन्यथा {  /* no 'lastfree' information */
    अचिन्हित i = sizenode(t);
    जबसम्म (i--) {  /* do a linear search */
      गाँठो_2 *मुक्त = gnode(t, i);
      यदि (keyisnil(मुक्त))
        फर्काऊ मुक्त;
    }
  }
  फर्काऊ रिक्त;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
स्थिर पूर्णाङ्क insertkey (Table *t, अचर TValue *साँचो, TValue *मान) {
  गाँठो_2 *mp = mainpositionTV(t, साँचो);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, साँचो, 0)));
  यदि (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    गाँठो_2 *othern;
    गाँठो_2 *f = getfreepos(t);  /* get a free place */
    यदि (f == रिक्त)  /* cannot find a free place? */
      फर्काऊ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    यदि (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      जबसम्म (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      यदि (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    अन्यथा {  /* colliding node is in its own main position */
      /* new node will go into free position */
      यदि (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      अन्यथा lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, साँचो);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), मान);
  फर्काऊ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
स्थिर शून्य newcheckedkey (Table *t, अचर TValue *साँचो, TValue *मान) {
  अचिन्हित i = keyinarray(t, साँचो);
  यदि (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, मान);  /* set value in the array */
  अन्यथा {
    पूर्णाङ्क done = insertkey(t, साँचो, मान);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(शून्य, done);  /* to avoid warnings */
  }
}


स्थिर शून्य luaH_newkey (lua_State *L, Table *t, अचर TValue *साँचो,
                                                 TValue *मान) {
  यदि (!ttisnil(मान)) {  /* do not insert nil values */
    पूर्णाङ्क done = insertkey(t, साँचो, मान);
    यदि (!done) {  /* could not find a free place? */
      rehash(L, t, साँचो);  /* grow table */
      newcheckedkey(t, साँचो, मान);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), साँचो);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (शून्य)0, (शून्य)0, 1);
  }
}


स्थिर अचर TValue *getintfromhash (Table *t, lua_Integer साँचो) {
  गाँठो_2 *n = hashint(t, साँचो);
  lua_assert(!ikeyinarray(t, साँचो));
  लागि (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (keyisinteger(n) && keyival(n) == साँचो)
      फर्काऊ gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0) रोक;
      n += nx;
    }
  }
  फर्काऊ &absentkey;
}


स्थिर पूर्णाङ्क hashkeyisempty (Table *t, lua_Unsigned साँचो) {
  अचर TValue *val = getintfromhash(t, l_castU2S(साँचो));
  फर्काऊ isempty(val);
}


स्थिर lu_byte finishnodeget (अचर TValue *val, TValue *res) {
  यदि (!ttisnil(val)) {
    setobj(((lua_State*)रिक्त), res, val);
  }
  फर्काऊ ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer साँचो, TValue *res) {
  अचिन्हित k = ikeyinarray(t, साँचो);
  यदि (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    यदि (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    फर्काऊ tag;
  }
  अन्यथा
    फर्काऊ finishnodeget(getintfromhash(t, साँचो), res);
}


/*
** search function for short strings
*/
अचर TValue *luaH_Hgetshortstr (Table *t, TString *साँचो) {
  गाँठो_2 *n = hashstr(t, साँचो);
  lua_assert(strisshr(साँचो));
  लागि (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (keyisshrstr(n) && eqshrstr(keystrval(n), साँचो))
      फर्काऊ gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0)
        फर्काऊ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *साँचो, TValue *res) {
  फर्काऊ finishnodeget(luaH_Hgetshortstr(t, साँचो), res);
}


स्थिर अचर TValue *Hgetlongstr (Table *t, TString *साँचो) {
  TValue ko;
  lua_assert(!strisshr(साँचो));
  setsvalue(cast(lua_State *, रिक्त), &ko, साँचो);
  फर्काऊ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


स्थिर अचर TValue *Hgetstr (Table *t, TString *साँचो) {
  यदि (strisshr(साँचो))
    फर्काऊ luaH_Hgetshortstr(t, साँचो);
  अन्यथा
    फर्काऊ Hgetlongstr(t, साँचो);
}


lu_byte luaH_getstr (Table *t, TString *साँचो, TValue *res) {
  फर्काऊ finishnodeget(Hgetstr(t, साँचो), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, अचर TValue *साँचो, TValue *res) {
  अचर TValue *slot;
  विकल्प (ttypetag(साँचो)) {
    पक्ष LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(साँचो));
      रोक;
    पक्ष LUA_VNUMINT:
      फर्काऊ luaH_getint(t, ivalue(साँचो), res);
    पक्ष LUA_VNIL:
      slot = &absentkey;
      रोक;
    पक्ष LUA_VNUMFLT: {
      lua_Integer k;
      यदि (luaV_flttointeger(fltvalue(साँचो), &k, F2Ieq)) /* integral index? */
        फर्काऊ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    स्वभाव:
      slot = getgeneric(t, साँचो, 0);
      रोक;
  }
  फर्काऊ finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
स्थिर पूर्णाङ्क retpsetcode (Table *t, अचर TValue *slot) {
  यदि (isabstkey(slot))
    फर्काऊ HNOTFOUND;  /* no slot with that key */
  अन्यथा  /* return node encoded */
    फर्काऊ cast_int((cast(गाँठो_2*, slot) - t->गाँठो)) + HFIRSTNODE;
}


स्थिर पूर्णाङ्क finishnodeset (Table *t, अचर TValue *slot, TValue *val) {
  यदि (!ttisnil(slot)) {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    फर्काऊ HOK;  /* success */
  }
  अन्यथा
    फर्काऊ retpsetcode(t, slot);
}


स्थिर पूर्णाङ्क rawfinishnodeset (अचर TValue *slot, TValue *val) {
  यदि (isabstkey(slot))
    फर्काऊ 0;  /* no slot with that key */
  अन्यथा {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    फर्काऊ 1;  /* success */
  }
}


पूर्णाङ्क luaH_psetint (Table *t, lua_Integer साँचो, TValue *val) {
  lua_assert(!ikeyinarray(t, साँचो));
  फर्काऊ finishnodeset(t, getintfromhash(t, साँचो), val);
}


स्थिर पूर्णाङ्क psetint (Table *t, lua_Integer साँचो, TValue *val) {
  पूर्णाङ्क hres;
  luaH_fastseti(t, साँचो, val, hres);
  फर्काऊ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

पूर्णाङ्क luaH_psetshortstr (Table *t, TString *साँचो, TValue *val) {
  अचर TValue *slot = luaH_Hgetshortstr(t, साँचो);
  यदि (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);  /* update it */
    फर्काऊ HOK;  /* done */
  }
  अन्यथा यदि (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    यदि (ttisnil(val))  /* new value is nil? */
      फर्काऊ HOK;  /* done (value is already nil/absent) */
    यदि (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(साँचो))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, रिक्त), &tk, साँचो);
      यदि (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        फर्काऊ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  फर्काऊ retpsetcode(t, slot);
}


पूर्णाङ्क luaH_psetstr (Table *t, TString *साँचो, TValue *val) {
  यदि (strisshr(साँचो))
    फर्काऊ luaH_psetshortstr(t, साँचो, val);
  अन्यथा
    फर्काऊ finishnodeset(t, Hgetlongstr(t, साँचो), val);
}


पूर्णाङ्क luaH_pset (Table *t, अचर TValue *साँचो, TValue *val) {
  विकल्प (ttypetag(साँचो)) {
    पक्ष LUA_VSHRSTR: फर्काऊ luaH_psetshortstr(t, tsvalue(साँचो), val);
    पक्ष LUA_VNUMINT: फर्काऊ psetint(t, ivalue(साँचो), val);
    पक्ष LUA_VNIL: फर्काऊ HNOTFOUND;
    पक्ष LUA_VNUMFLT: {
      lua_Integer k;
      यदि (luaV_flttointeger(fltvalue(साँचो), &k, F2Ieq)) /* integral index? */
        फर्काऊ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    स्वभाव:
      फर्काऊ finishnodeset(t, getgeneric(t, साँचो, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
शून्य luaH_finishset (lua_State *L, Table *t, अचर TValue *साँचो,
                                    TValue *मान, पूर्णाङ्क hres) {
  lua_assert(hres != HOK);
  यदि (hres == HNOTFOUND) {
    TValue aux;
    अचर TValue *actk = साँचो;  /* actual key to insert */
    यदि (l_unlikely(ttisnil(साँचो)))
      luaG_runerror(L, "table index is nil");
    अन्यथा यदि (ttisfloat(साँचो)) {
      lua_सङ्ख्या f = fltvalue(साँचो);
      lua_Integer k;
      यदि (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      अन्यथा यदि (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    अन्यथा यदि (isextstr(साँचो)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(साँचो));
      setsvalue2s(L, L->शीर्ष.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->शीर्ष.p - 1), मान);
      L->शीर्ष.p--;
      फर्काऊ;
    }
    luaH_newkey(L, t, actk, मान);
  }
  अन्यथा यदि (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), मान);
  }
  अन्यथा {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), मान);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
शून्य luaH_set (lua_State *L, Table *t, अचर TValue *साँचो, TValue *मान) {
  पूर्णाङ्क hres = luaH_pset(t, साँचो, मान);
  यदि (hres != HOK)
    luaH_finishset(L, t, साँचो, मान, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
शून्य luaH_setint (lua_State *L, Table *t, lua_Integer साँचो, TValue *मान) {
  अचिन्हित ik = ikeyinarray(t, साँचो);
  यदि (ik > 0)
    obj2arr(t, ik - 1, मान);
  अन्यथा {
    पूर्णाङ्क ok = rawfinishnodeset(getintfromhash(t, साँचो), मान);
    यदि (!ok) {
      TValue k;
      setivalue(&k, साँचो);
      luaH_newkey(L, t, &k, मान);
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
स्थिर lua_Unsigned hash_खोज (lua_State *L, Table *t, अचिन्हित asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  अचिन्हित rnd = G(L)->seed;
  पूर्णाङ्क n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  अचिन्हित mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  अचिन्हित incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  जबसम्म (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    यदि (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    अन्यथा {
      j = LUA_MAXINTEGER;
      यदि (hashkeyisempty(t, j))  /* t[j] not present? */
        रोक;  /* 'j' now is an absent index */
      अन्यथा  /* weird case */
        फर्काऊ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  जबसम्म (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    यदि (hashkeyisempty(t, m)) j = m;
    अन्यथा i = m;
  }
  फर्काऊ i;
}


स्थिर अचिन्हित पूर्णाङ्क binsearch (Table *array, अचिन्हित पूर्णाङ्क i, अचिन्हित पूर्णाङ्क j) {
  lua_assert(i <= j);
  जबसम्म (j - i > 1u) {  /*द्विआधारी खोज*/
    अचिन्हित पूर्णाङ्क m = (i + j) / 2;
    यदि (arraykeyisempty(array, m)) j = m;
    अन्यथा i = m;
  }
  फर्काऊ i;
}


/* return a border, saving it as a hint for next call */
स्थिर lua_Unsigned newhint (Table *t, अचिन्हित hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  फर्काऊ hint;
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
  यदि (asize > 0) {  /* is there an array part? */
    अचर अचिन्हित maxvicinity = 4;
    अचिन्हित limit = *lenhint(t);  /* start with the hint */
    यदि (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    यदि (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      लागि (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        यदि (!arraykeyisempty(t, limit))
          फर्काऊ newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      फर्काऊ newhint(t, binsearch(t, 0, limit));
    }
    अन्यथा {  /* 'limit' is present in table; look for a border after it */
      अचिन्हित i;
      /* look for a border in the vicinity of the hint */
      लागि (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        यदि (arraykeyisempty(t, limit))
          फर्काऊ newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      यदि (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        फर्काऊ newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  यदि (isdummy(t) || hashkeyisempty(t, asize + 1))
    फर्काऊ asize;  /* 'asize + 1' is empty */
  अन्यथा  /* 'asize + 1' is also non empty */
    फर्काऊ hash_खोज(L, t, asize);
}



#यदि defined(LUA_DEBUG)

/* export this function for the test library */

गाँठो_2 *luaH_mainposition (अचर Table *t, अचर TValue *साँचो) {
  फर्काऊ mainpositionTV(t, साँचो);
}

#endif
