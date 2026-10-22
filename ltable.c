/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#परिभाषय ltable_c
#परिभाषय LUA_CORE

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
#परिभाषय LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
प्रकारनिर्देश संरचना { ग्रन्थिः_2 *dummy; ग्रन्थिः_2 follows_p_ग्रन्थिः; } Limbox_aux;

प्रकारनिर्देश सङ्घ {
  ग्रन्थिः_2 *lastfree;
  वर्ण padding[offsetof(Limbox_aux, follows_p_ग्रन्थिः)];
} Limbox;

#परिभाषय haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#परिभाषय getlastfree(t)     ((cast(Limbox *, (t)->ग्रन्थिः) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#परिभाषय MAXABITS	(l_numbits(पूर्णाङ्क) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#परिभाषय MAXASIZEB	(MAX_SIZET/(परिमाण(मानम्_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#परिभाषय MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#परिभाषय MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#परिभाषय MAXHSIZE	lua_M_सीमा_N(1 << MAXHBITS, ग्रन्थिः_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#परिभाषय hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#परिभाषय hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#परिभाषय hashstr(t,str)		hashpow2(t, (str)->hash)
#परिभाषय hashboolean(t,p)	hashpow2(t, p)


#परिभाषय hashpointer(t,p)	hashmod(t, point2uint(p))


#परिभाषय dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
स्थिर ध्रुव ग्रन्थिः_2 dummynode_ = {
  {{रिक्त}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {रिक्त}}  /* key type, next, and key value */
};


स्थिर ध्रुव TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
स्थिर ग्रन्थिः_2 *hashint (ध्रुव सारणी *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  यदि (ui <= cast_uint(अधिकतम_पूर्णाङ्कः))
    प्रत्यागम gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  अन्यथा
    प्रत्यागम hashmod(t, ui);
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
स्थिर अचिह्नित l_hashfloat (lua_सङ्ख्या n) {
  पूर्णाङ्क i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_अङ्कः(INT_MIN);
  यदि (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_अङ्कः(HUGE_VAL));
    प्रत्यागम 0;
  }
  अन्यथा {  /* normal case */
    अचिह्नित पूर्णाङ्क u = cast_uint(i) + cast_uint(ni);
    प्रत्यागम (u <= cast_uint(अधिकतम_पूर्णाङ्कः) ? u : ~u);
  }
}
#यदि_अन्तः


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
स्थिर ग्रन्थिः_2 *mainpositionTV (ध्रुव सारणी *t, ध्रुव TValue *कुञ्चिका) {
  विकल्प (ttypetag(कुञ्चिका)) {
    पक्ष LUA_VNUMINT: {
      lua_Integer i = ivalue(कुञ्चिका);
      प्रत्यागम hashint(t, i);
    }
    पक्ष LUA_VNUMFLT: {
      lua_सङ्ख्या n = fltvalue(कुञ्चिका);
      प्रत्यागम hashmod(t, l_hashfloat(n));
    }
    पक्ष LUA_VSHRSTR: {
      TString *ts = tsvalue(कुञ्चिका);
      प्रत्यागम hashstr(t, ts);
    }
    पक्ष LUA_VLNGSTR: {
      TString *ts = tsvalue(कुञ्चिका);
      प्रत्यागम hashpow2(t, luaS_hashlongstr(ts));
    }
    पक्ष LUA_VFALSE:
      प्रत्यागम hashboolean(t, 0);
    पक्ष LUA_VTRUE:
      प्रत्यागम hashboolean(t, 1);
    पक्ष LUA_VLIGHTUSERDATA: {
      शून्य *p = pvalue(कुञ्चिका);
      प्रत्यागम hashpointer(t, p);
    }
    पक्ष LUA_VLCF: {
      lua_CFunction f = fvalue(कुञ्चिका);
      प्रत्यागम hashpointer(t, f);
    }
    स्वभाव: {
      GCObject *o = gcvalue(कुञ्चिका);
      प्रत्यागम hashpointer(t, o);
    }
  }
}


l_sinline ग्रन्थिः_2 *mainpositionfromnode (ध्रुव सारणी *t, ग्रन्थिः_2 *nd) {
  TValue कुञ्चिका;
  getnodekey(cast(lua_State *, रिक्त), &कुञ्चिका, nd);
  प्रत्यागम mainpositionTV(t, &कुञ्चिका);
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
स्थिर पूर्णाङ्क equalkey (ध्रुव TValue *k1, ध्रुव ग्रन्थिः_2 *n2, पूर्णाङ्क deadok) {
  यदि (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    यदि (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      प्रत्यागम luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    अन्यथा यदि (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      प्रत्यागम gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   अन्यथा
     प्रत्यागम 0;  /* otherwise, different variants cannot be equal */
  }
  अन्यथा {  /* equal variants */
    विकल्प (keytt(n2)) {
      पक्ष LUA_VNIL: पक्ष LUA_VFALSE: पक्ष LUA_VTRUE:
        प्रत्यागम 1;
      पक्ष LUA_VNUMINT:
        प्रत्यागम (ivalue(k1) == keyival(n2));
      पक्ष LUA_VNUMFLT:
        प्रत्यागम luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      पक्ष LUA_VLIGHTUSERDATA:
        प्रत्यागम pvalue(k1) == pvalueraw(keyval(n2));
      पक्ष LUA_VLCF:
        प्रत्यागम fvalue(k1) == fvalueraw(keyval(n2));
      पक्ष ctb(LUA_VLNGSTR):
        प्रत्यागम luaS_eqstr(tsvalue(k1), keystrval(n2));
      स्वभाव:
        प्रत्यागम gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
स्थिर ध्रुव TValue *getgeneric (सारणी *t, ध्रुव TValue *कुञ्चिका, पूर्णाङ्क deadok) {
  ग्रन्थिः_2 *n = mainpositionTV(t, कुञ्चिका);
  प्रति (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (equalkey(कुञ्चिका, n, deadok))
      प्रत्यागम gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0)
        प्रत्यागम &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
स्थिर अचिह्नित checkrange (lua_Integer k, अचिह्नित सीमा) {
  प्रत्यागम (l_castS2U(k) - 1u < सीमा) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#परिभाषय arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#परिभाषय ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
स्थिर अचिह्नित keyinarray (सारणी *t, ध्रुव TValue *कुञ्चिका) {
  प्रत्यागम (ttisinteger(कुञ्चिका)) ? ikeyinarray(t, ivalue(कुञ्चिका)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
स्थिर अचिह्नित findindex (lua_State *L, सारणी *t, TValue *कुञ्चिका,
                               अचिह्नित asize) {
  अचिह्नित पूर्णाङ्क i;
  यदि (ttisnil(कुञ्चिका)) प्रत्यागम 0;  /* first iteration */
  i = keyinarray(t, कुञ्चिका);
  यदि (i != 0)  /* is 'key' inside array part? */
    प्रत्यागम i;  /* yes; that's the index */
  अन्यथा {
    ध्रुव TValue *n = getgeneric(t, कुञ्चिका, 1);
    यदि (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    प्रत्यागम (i + 1) + asize;
  }
}


पूर्णाङ्क lua_H_अग्र (lua_State *L, सारणी *t, StkId कुञ्चिका) {
  अचिह्नित पूर्णाङ्क asize = t->asize;
  अचिह्नित पूर्णाङ्क i = findindex(L, t, s2v(कुञ्चिका), asize);  /* find original key */
  प्रति (; i < asize; i++) {  /* try first array part */
    lu_byte चिह्नकम् = *get_Arr_चिह्नकम्(t, i);
    यदि (!tagisempty(चिह्नकम्)) {  /* a non-empty entry? */
      setivalue(s2v(कुञ्चिका), cast_int(i) + 1);
      farr2val(t, i, चिह्नकम्, s2v(कुञ्चिका + 1));
      प्रत्यागम 1;
    }
  }
  प्रति (i -= asize; i < sizenode(t); i++) {  /* hash part */
    यदि (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ग्रन्थिः_2 *n = gnode(t, i);
      getnodekey(L, s2v(कुञ्चिका), n);
      setobj2s(L, कुञ्चिका + 1, gval(n));
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#परिभाषय extraLastfree(t)	(haslastfree(t) ? परिमाण(Limbox) : 0)

/* 'node' size in bytes */
स्थिर size_t sizehash (सारणी *t) {
  प्रत्यागम cast_sizet(sizenode(t)) * परिमाण(ग्रन्थिः_2) + extraLastfree(t);
}


स्थिर शून्य freehash (lua_State *L, सारणी *t) {
  यदि (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    वर्ण *arr = cast_charp(t->ग्रन्थिः) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

स्थिर पूर्णाङ्क insertkey (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *मानम्);
स्थिर शून्य newcheckedkey (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *मानम्);


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
  अचिह्नित योग;
  अचिह्नित na;
  पूर्णाङ्क deleted;
  अचिह्नित nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#परिभाषय arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
स्थिर अचिह्नित computesizes (Counters *ct) {
  पूर्णाङ्क i;
  अचिह्नित पूर्णाङ्क twotoi;  /* 2^i (candidate for optimal size) */
  अचिह्नित पूर्णाङ्क a = 0;  /* number of elements smaller than 2^i */
  अचिह्नित पूर्णाङ्क na = 0;  /* number of elements to go to array part */
  अचिह्नित पूर्णाङ्क optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  प्रति (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    अचिह्नित nums = ct->nums[i];
    a += nums;
    यदि (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  प्रत्यागम optimal;
}


स्थिर शून्य countint (lua_Integer कुञ्चिका, Counters *ct) {
  अचिह्नित पूर्णाङ्क k = arrayindex(कुञ्चिका);
  यदि (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline पूर्णाङ्क arraykeyisempty (ध्रुव सारणी *t, अचिह्नित कुञ्चिका) {
  पूर्णाङ्क चिह्नकम् = *get_Arr_चिह्नकम्(t, कुञ्चिका - 1);
  प्रत्यागम tagisempty(चिह्नकम्);
}


/*
** Count keys in array part of table 't'.
*/
स्थिर शून्य numusearray (ध्रुव सारणी *t, Counters *ct) {
  पूर्णाङ्क lg;
  अचिह्नित पूर्णाङ्क ttlg;  /* 2^lg */
  अचिह्नित पूर्णाङ्क ause = 0;  /* summation of 'nums' */
  अचिह्नित पूर्णाङ्क i = 1;  /* index to traverse all array keys */
  अचिह्नित पूर्णाङ्क asize = t->asize;
  /* traverse each slice */
  प्रति (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    अचिह्नित पूर्णाङ्क lc = 0;  /*गणक*/
    अचिह्नित पूर्णाङ्क lim = ttlg;
    यदि (lim > asize) {
      lim = asize;  /* adjust upper limit */
      यदि (i > lim)
        भङ्ग;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    प्रति (; i <= lim; i++) {
      यदि (!arraykeyisempty(t, i))
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
स्थिर शून्य numusehash (ध्रुव सारणी *t, Counters *ct) {
  अचिह्नित i = sizenode(t);
  अचिह्नित योग = 0;
  यावत् (i--) {
    ग्रन्थिः_2 *n = &t->ग्रन्थिः[i];
    यदि (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    अन्यथा {
      योग++;
      यदि (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->योग += योग;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
स्थिर size_t concretesize (अचिह्नित पूर्णाङ्क आकार) {
  यदि (आकार == 0)
    प्रत्यागम 0;
  अन्यथा  /* space for the two arrays plus an unsigned in between */
    प्रत्यागम आकार * (परिमाण(मानम्_2) + 1) + परिमाण(अचिह्नित);
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
स्थिर मानम्_2 *resizearray (lua_State *L , सारणी *t,
                               अचिह्नित oldasize,
                               अचिह्नित newasize) {
  यदि (oldasize == newasize)
    प्रत्यागम t->array;  /* nothing to be done */
  अन्यथा यदि (newasize == 0) {  /* erasing array? */
    मानम्_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    प्रत्यागम रिक्त;
  }
  अन्यथा {
    size_t newasizeb = concretesize(newasize);
    मानम्_2 *np = cast(मानम्_2 *,
                  luaM_reallocvector(L, रिक्त, 0, newasizeb, lu_byte));
    यदि (np == रिक्त)  /* allocation error? */
      प्रत्यागम रिक्त;
    np += newasize;  /* shift pointer to the end of value segment */
    यदि (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      मानम्_2 *op = t->array;  /* original array */
      अचिह्नित tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      स्मृतिं_प्रतिलिख(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    प्रत्यागम np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
स्थिर शून्य setnodevector (lua_State *L, सारणी *t, अचिह्नित आकार) {
  यदि (आकार == 0) {  /* no elements to hash part? */
    t->ग्रन्थिः = cast(ग्रन्थिः_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  अन्यथा {
    पूर्णाङ्क i;
    पूर्णाङ्क lsize = luaO_ceillog2(आकार);
    यदि (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    आकार = twoto(lsize);
    यदि (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ग्रन्थिः = luaM_newvector(L, आकार, ग्रन्थिः_2);
    अन्यथा {
      size_t bsize = आकार * परिमाण(ग्रन्थिः_2) + परिमाण(Limbox);
      वर्ण *ग्रन्थिः = luaM_newblock(L, bsize);
      t->ग्रन्थिः = cast(ग्रन्थिः_2 *, ग्रन्थिः + परिमाण(Limbox));
      getlastfree(t) = gnode(t, आकार);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    प्रति (i = 0; i < cast_int(आकार); i++) {
      ग्रन्थिः_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
स्थिर शून्य reinserthash (lua_State *L, सारणी *ot, सारणी *t) {
  अचिह्नित j;
  अचिह्नित आकार = sizenode(ot);
  प्रति (j = 0; j < आकार; j++) {
    ग्रन्थिः_2 *old = gnode(ot, j);
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
स्थिर शून्य exchangehashpart (सारणी *t1, सारणी *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ग्रन्थिः_2 *ग्रन्थिः = t1->ग्रन्थिः;
  पूर्णाङ्क bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->ग्रन्थिः = t2->ग्रन्थिः;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->ग्रन्थिः = ग्रन्थिः;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
स्थिर शून्य reinsert_Old_खण्डिका (सारणी *t, अचिह्नित oldasize,
                                        अचिह्नित newasize) {
  अचिह्नित i;
  प्रति (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte चिह्नकम् = *get_Arr_चिह्नकम्(t, i);
    यदि (!tagisempty(चिह्नकम्)) {  /* a non-empty entry? */
      TValue कुञ्चिका, aux;
      setivalue(&कुञ्चिका, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, चिह्नकम्, &aux);  /* copy value into 'aux' */
      insertkey(t, &कुञ्चिका, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
स्थिर शून्य clear_New_खण्डिका (सारणी *t, अचिह्नित oldasize, अचिह्नित newasize) {
  प्रति (; oldasize < newasize; oldasize++)
    *get_Arr_चिह्नकम्(t, oldasize) = LUA_VEMPTY;
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
शून्य luaH_resize (lua_State *L, सारणी *t, अचिह्नित newasize,
                                          अचिह्नित nhsize) {
  सारणी newt;  /* to keep the new hash part */
  अचिह्नित oldasize = t->asize;
  मानम्_2 *newarray;
  यदि (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  यदि (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsert_Old_खण्डिका(t, oldasize, newasize);
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
  clear_New_खण्डिका(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


शून्य luaH_resizearray (lua_State *L, सारणी *t, अचिह्नित पूर्णाङ्क nasize) {
  अचिह्नित nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
स्थिर शून्य rehash (lua_State *L, सारणी *t, ध्रुव TValue *ek) {
  अचिह्नित asize;  /* optimal size for array part */
  Counters ct;
  अचिह्नित i;
  अचिह्नित nsize;  /* size for the hash part */
  /* reset counts */
  प्रति (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.योग = 1;  /* count extra key */
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
  nsize = ct.योग - ct.na;
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


सारणी *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, परिमाण(सारणी));
  सारणी *t = gco2t(o);
  t->metatable = रिक्त;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = रिक्त;
  t->asize = 0;
  setnodevector(L, t, 0);
  प्रत्यागम t;
}


lu_mem lua_H_आकार (सारणी *t) {
  lu_mem sz = cast(lu_mem, परिमाण(सारणी)) + concretesize(t->asize);
  यदि (!isdummy(t))
    sz += sizehash(t);
  प्रत्यागम sz;
}


/*
** Frees a table.
*/
शून्य luaH_free (lua_State *L, सारणी *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


स्थिर ग्रन्थिः_2 *getfreepos (सारणी *t) {
  यदि (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    यावत् (getlastfree(t) > t->ग्रन्थिः) {
      ग्रन्थिः_2 *मुक्त = --getlastfree(t);
      यदि (keyisnil(मुक्त))
        प्रत्यागम मुक्त;
    }
  }
  अन्यथा {  /* no 'lastfree' information */
    अचिह्नित i = sizenode(t);
    यावत् (i--) {  /* do a linear search */
      ग्रन्थिः_2 *मुक्त = gnode(t, i);
      यदि (keyisnil(मुक्त))
        प्रत्यागम मुक्त;
    }
  }
  प्रत्यागम रिक्त;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
स्थिर पूर्णाङ्क insertkey (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *मानम्) {
  ग्रन्थिः_2 *mp = mainpositionTV(t, कुञ्चिका);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, कुञ्चिका, 0)));
  यदि (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ग्रन्थिः_2 *othern;
    ग्रन्थिः_2 *f = getfreepos(t);  /* get a free place */
    यदि (f == रिक्त)  /* cannot find a free place? */
      प्रत्यागम 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    यदि (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      यावत् (othern + gnext(othern) != mp)  /* find previous */
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
  setnodekey(mp, कुञ्चिका);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), मानम्);
  प्रत्यागम 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
स्थिर शून्य newcheckedkey (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *मानम्) {
  अचिह्नित i = keyinarray(t, कुञ्चिका);
  यदि (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, मानम्);  /* set value in the array */
  अन्यथा {
    पूर्णाङ्क समाप्तम् = insertkey(t, कुञ्चिका, मानम्);  /* insert key in the hash part */
    lua_assert(समाप्तम्);  /* it cannot fail */
    cast(शून्य, समाप्तम्);  /* to avoid warnings */
  }
}


स्थिर शून्य luaH_newkey (lua_State *L, सारणी *t, ध्रुव TValue *कुञ्चिका,
                                                 TValue *मानम्) {
  यदि (!ttisnil(मानम्)) {  /* do not insert nil values */
    पूर्णाङ्क समाप्तम् = insertkey(t, कुञ्चिका, मानम्);
    यदि (!समाप्तम्) {  /* could not find a free place? */
      rehash(L, t, कुञ्चिका);  /* grow table */
      newcheckedkey(t, कुञ्चिका, मानम्);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), कुञ्चिका);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (शून्य)0, (शून्य)0, 1);
  }
}


स्थिर ध्रुव TValue *getintfromhash (सारणी *t, lua_Integer कुञ्चिका) {
  ग्रन्थिः_2 *n = hashint(t, कुञ्चिका);
  lua_assert(!ikeyinarray(t, कुञ्चिका));
  प्रति (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (keyisinteger(n) && keyival(n) == कुञ्चिका)
      प्रत्यागम gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0) भङ्ग;
      n += nx;
    }
  }
  प्रत्यागम &absentkey;
}


स्थिर पूर्णाङ्क hashkeyisempty (सारणी *t, lua_Unsigned कुञ्चिका) {
  ध्रुव TValue *val = getintfromhash(t, l_castU2S(कुञ्चिका));
  प्रत्यागम isempty(val);
}


स्थिर lu_byte finishnodeget (ध्रुव TValue *val, TValue *res) {
  यदि (!ttisnil(val)) {
    setobj(((lua_State*)रिक्त), res, val);
  }
  प्रत्यागम ttypetag(val);
}


lu_byte luaH_getint (सारणी *t, lua_Integer कुञ्चिका, TValue *res) {
  अचिह्नित k = ikeyinarray(t, कुञ्चिका);
  यदि (k > 0) {
    lu_byte चिह्नकम् = *get_Arr_चिह्नकम्(t, k - 1);
    यदि (!tagisempty(चिह्नकम्))
      farr2val(t, k - 1, चिह्नकम्, res);
    प्रत्यागम चिह्नकम्;
  }
  अन्यथा
    प्रत्यागम finishnodeget(getintfromhash(t, कुञ्चिका), res);
}


/*
** search function for short strings
*/
ध्रुव TValue *luaH_Hgetshortstr (सारणी *t, TString *कुञ्चिका) {
  ग्रन्थिः_2 *n = hashstr(t, कुञ्चिका);
  lua_assert(strisshr(कुञ्चिका));
  प्रति (;;) {  /* check whether 'key' is somewhere in the chain */
    यदि (keyisshrstr(n) && eqshrstr(keystrval(n), कुञ्चिका))
      प्रत्यागम gval(n);  /* that's it */
    अन्यथा {
      पूर्णाङ्क nx = gnext(n);
      यदि (nx == 0)
        प्रत्यागम &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (सारणी *t, TString *कुञ्चिका, TValue *res) {
  प्रत्यागम finishnodeget(luaH_Hgetshortstr(t, कुञ्चिका), res);
}


स्थिर ध्रुव TValue *Hgetlongstr (सारणी *t, TString *कुञ्चिका) {
  TValue ko;
  lua_assert(!strisshr(कुञ्चिका));
  setsvalue(cast(lua_State *, रिक्त), &ko, कुञ्चिका);
  प्रत्यागम getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


स्थिर ध्रुव TValue *Hgetstr (सारणी *t, TString *कुञ्चिका) {
  यदि (strisshr(कुञ्चिका))
    प्रत्यागम luaH_Hgetshortstr(t, कुञ्चिका);
  अन्यथा
    प्रत्यागम Hgetlongstr(t, कुञ्चिका);
}


lu_byte luaH_getstr (सारणी *t, TString *कुञ्चिका, TValue *res) {
  प्रत्यागम finishnodeget(Hgetstr(t, कुञ्चिका), res);
}


/*
** main search function
*/
lu_byte luaH_get (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *res) {
  ध्रुव TValue *slot;
  विकल्प (ttypetag(कुञ्चिका)) {
    पक्ष LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(कुञ्चिका));
      भङ्ग;
    पक्ष LUA_VNUMINT:
      प्रत्यागम luaH_getint(t, ivalue(कुञ्चिका), res);
    पक्ष LUA_VNIL:
      slot = &absentkey;
      भङ्ग;
    पक्ष LUA_VNUMFLT: {
      lua_Integer k;
      यदि (luaV_flttointeger(fltvalue(कुञ्चिका), &k, F2Ieq)) /* integral index? */
        प्रत्यागम luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    स्वभाव:
      slot = getgeneric(t, कुञ्चिका, 0);
      भङ्ग;
  }
  प्रत्यागम finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
स्थिर पूर्णाङ्क retpsetcode (सारणी *t, ध्रुव TValue *slot) {
  यदि (isabstkey(slot))
    प्रत्यागम HNOTFOUND;  /* no slot with that key */
  अन्यथा  /* return node encoded */
    प्रत्यागम cast_int((cast(ग्रन्थिः_2*, slot) - t->ग्रन्थिः)) + HFIRSTNODE;
}


स्थिर पूर्णाङ्क finishnodeset (सारणी *t, ध्रुव TValue *slot, TValue *val) {
  यदि (!ttisnil(slot)) {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    प्रत्यागम HOK;  /* success */
  }
  अन्यथा
    प्रत्यागम retpsetcode(t, slot);
}


स्थिर पूर्णाङ्क rawfinishnodeset (ध्रुव TValue *slot, TValue *val) {
  यदि (isabstkey(slot))
    प्रत्यागम 0;  /* no slot with that key */
  अन्यथा {
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);
    प्रत्यागम 1;  /* success */
  }
}


पूर्णाङ्क luaH_psetint (सारणी *t, lua_Integer कुञ्चिका, TValue *val) {
  lua_assert(!ikeyinarray(t, कुञ्चिका));
  प्रत्यागम finishnodeset(t, getintfromhash(t, कुञ्चिका), val);
}


स्थिर पूर्णाङ्क psetint (सारणी *t, lua_Integer कुञ्चिका, TValue *val) {
  पूर्णाङ्क hres;
  luaH_fastseti(t, कुञ्चिका, val, hres);
  प्रत्यागम hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

पूर्णाङ्क luaH_psetshortstr (सारणी *t, TString *कुञ्चिका, TValue *val) {
  ध्रुव TValue *slot = luaH_Hgetshortstr(t, कुञ्चिका);
  यदि (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)रिक्त), cast(TValue*, slot), val);  /* update it */
    प्रत्यागम HOK;  /*समाप्तम्*/
  }
  अन्यथा यदि (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    यदि (ttisnil(val))  /* new value is nil? */
      प्रत्यागम HOK;  /* done (value is already nil/absent) */
    यदि (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(कुञ्चिका))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, रिक्त), &tk, कुञ्चिका);
      यदि (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        प्रत्यागम HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  प्रत्यागम retpsetcode(t, slot);
}


पूर्णाङ्क luaH_psetstr (सारणी *t, TString *कुञ्चिका, TValue *val) {
  यदि (strisshr(कुञ्चिका))
    प्रत्यागम luaH_psetshortstr(t, कुञ्चिका, val);
  अन्यथा
    प्रत्यागम finishnodeset(t, Hgetlongstr(t, कुञ्चिका), val);
}


पूर्णाङ्क luaH_pset (सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *val) {
  विकल्प (ttypetag(कुञ्चिका)) {
    पक्ष LUA_VSHRSTR: प्रत्यागम luaH_psetshortstr(t, tsvalue(कुञ्चिका), val);
    पक्ष LUA_VNUMINT: प्रत्यागम psetint(t, ivalue(कुञ्चिका), val);
    पक्ष LUA_VNIL: प्रत्यागम HNOTFOUND;
    पक्ष LUA_VNUMFLT: {
      lua_Integer k;
      यदि (luaV_flttointeger(fltvalue(कुञ्चिका), &k, F2Ieq)) /* integral index? */
        प्रत्यागम psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    स्वभाव:
      प्रत्यागम finishnodeset(t, getgeneric(t, कुञ्चिका, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
शून्य luaH_finishset (lua_State *L, सारणी *t, ध्रुव TValue *कुञ्चिका,
                                    TValue *मानम्, पूर्णाङ्क hres) {
  lua_assert(hres != HOK);
  यदि (hres == HNOTFOUND) {
    TValue aux;
    ध्रुव TValue *actk = कुञ्चिका;  /* actual key to insert */
    यदि (l_unlikely(ttisnil(कुञ्चिका)))
      luaG_runerror(L, "table index is nil");
    अन्यथा यदि (ttisfloat(कुञ्चिका)) {
      lua_सङ्ख्या f = fltvalue(कुञ्चिका);
      lua_Integer k;
      यदि (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      अन्यथा यदि (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    अन्यथा यदि (isextstr(कुञ्चिका)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(कुञ्चिका));
      setsvalue2s(L, L->शिखरम्.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->शिखरम्.p - 1), मानम्);
      L->शिखरम्.p--;
      प्रत्यागम;
    }
    luaH_newkey(L, t, actk, मानम्);
  }
  अन्यथा यदि (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), मानम्);
  }
  अन्यथा {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), मानम्);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
शून्य lua_H_समुच्चयः (lua_State *L, सारणी *t, ध्रुव TValue *कुञ्चिका, TValue *मानम्) {
  पूर्णाङ्क hres = luaH_pset(t, कुञ्चिका, मानम्);
  यदि (hres != HOK)
    luaH_finishset(L, t, कुञ्चिका, मानम्, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
शून्य luaH_setint (lua_State *L, सारणी *t, lua_Integer कुञ्चिका, TValue *मानम्) {
  अचिह्नित ik = ikeyinarray(t, कुञ्चिका);
  यदि (ik > 0)
    obj2arr(t, ik - 1, मानम्);
  अन्यथा {
    पूर्णाङ्क ok = rawfinishnodeset(getintfromhash(t, कुञ्चिका), मानम्);
    यदि (!ok) {
      TValue k;
      setivalue(&k, कुञ्चिका);
      luaH_newkey(L, t, &k, मानम्);
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
स्थिर lua_Unsigned hash_अन्वेषणम् (lua_State *L, सारणी *t, अचिह्नित asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  अचिह्नित rnd = G(L)->बीजम्;
  पूर्णाङ्क n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  अचिह्नित आवरणम्_2 = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  अचिह्नित incr = (rnd & आवरणम्_2) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  यावत् (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    यदि (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    अन्यथा {
      j = LUA_MAXINTEGER;
      यदि (hashkeyisempty(t, j))  /* t[j] not present? */
        भङ्ग;  /* 'j' now is an absent index */
      अन्यथा  /* weird case */
        प्रत्यागम j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  यावत् (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    यदि (hashkeyisempty(t, m)) j = m;
    अन्यथा i = m;
  }
  प्रत्यागम i;
}


स्थिर अचिह्नित पूर्णाङ्क binsearch (सारणी *array, अचिह्नित पूर्णाङ्क i, अचिह्नित पूर्णाङ्क j) {
  lua_assert(i <= j);
  यावत् (j - i > 1u) {  /*द्विमान अन्वेषणम्*/
    अचिह्नित पूर्णाङ्क m = (i + j) / 2;
    यदि (arraykeyisempty(array, m)) j = m;
    अन्यथा i = m;
  }
  प्रत्यागम i;
}


/* return a border, saving it as a hint for next call */
स्थिर lua_Unsigned newhint (सारणी *t, अचिह्नित hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  प्रत्यागम hint;
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
lua_Unsigned luaH_getn (lua_State *L, सारणी *t) {
  अचिह्नित asize = t->asize;
  यदि (asize > 0) {  /* is there an array part? */
    ध्रुव अचिह्नित maxvicinity = 4;
    अचिह्नित सीमा = *lenhint(t);  /* start with the hint */
    यदि (सीमा == 0)
      सीमा = 1;  /* make limit a valid index in the array */
    यदि (arraykeyisempty(t, सीमा)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      अचिह्नित i;
      /* look for a border in the vicinity of the hint */
      प्रति (i = 0; i < maxvicinity && सीमा > 1; i++) {
        सीमा--;
        यदि (!arraykeyisempty(t, सीमा))
          प्रत्यागम newhint(t, सीमा);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      प्रत्यागम newhint(t, binsearch(t, 0, सीमा));
    }
    अन्यथा {  /* 'limit' is present in table; look for a border after it */
      अचिह्नित i;
      /* look for a border in the vicinity of the hint */
      प्रति (i = 0; i < maxvicinity && सीमा < asize; i++) {
        सीमा++;
        यदि (arraykeyisempty(t, सीमा))
          प्रत्यागम newhint(t, सीमा - 1);  /* 'limit - 1' is a border */
      }
      यदि (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        प्रत्यागम newhint(t, binsearch(t, सीमा, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  यदि (isdummy(t) || hashkeyisempty(t, asize + 1))
    प्रत्यागम asize;  /* 'asize + 1' is empty */
  अन्यथा  /* 'asize + 1' is also non empty */
    प्रत्यागम hash_अन्वेषणम्(L, t, asize);
}



#यदि defined(LUA_DEBUG)

/* export this function for the test library */

ग्रन्थिः_2 *luaH_mainposition (ध्रुव सारणी *t, ध्रुव TValue *कुञ्चिका) {
  प्रत्यागम mainpositionTV(t, कुञ्चिका);
}

#यदि_अन्तः
