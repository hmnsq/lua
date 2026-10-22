/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#నిర్వచించు ltable_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


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

#అన్తర్భావ <math.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#నిర్వచించు LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
రకనిర్వచనం నిర్మాణం { కణుపు_2 *dummy; కణుపు_2 follows_p_కణుపు; } Limbox_aux;

రకనిర్వచనం సంఘం {
  కణుపు_2 *lastfree;
  అక్షరం padding[offsetof(Limbox_aux, follows_p_కణుపు)];
} Limbox;

#నిర్వచించు haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#నిర్వచించు getlastfree(t)     ((cast(Limbox *, (t)->కణుపు) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#నిర్వచించు MAXABITS	(l_numbits(పూర్ణాంకం) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#నిర్వచించు MAXASIZEB	(MAX_SIZET/(పరిమాణం(విలువ_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#నిర్వచించు MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#నిర్వచించు MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#నిర్వచించు MAXHSIZE	lua_M_హద్దు_N(1 << MAXHBITS, కణుపు_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#నిర్వచించు hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#నిర్వచించు hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#నిర్వచించు hashstr(t,str)		hashpow2(t, (str)->hash)
#నిర్వచించు hashboolean(t,p)	hashpow2(t, p)


#నిర్వచించు hashpointer(t,p)	hashmod(t, point2uint(p))


#నిర్వచించు dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
స్థిర స్థిరరాశి కణుపు_2 dummynode_ = {
  {{ఖాళీ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ఖాళీ}}  /* key type, next, and key value */
};


స్థిర స్థిరరాశి TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
స్థిర కణుపు_2 *hashint (స్థిరరాశి పట్టిక *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ఒకవేళ (ui <= cast_uint(గరిష్ఠ_పూర్ణాంకం))
    తిరిగి gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  లేకపోతే
    తిరిగి hashmod(t, ui);
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
#ఒకవేళ !defined(l_hashfloat)
స్థిర గుర్తులేని l_hashfloat (lua_సంఖ్య n) {
  పూర్ణాంకం i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_అంకె(INT_MIN);
  ఒకవేళ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_అంకె(HUGE_VAL));
    తిరిగి 0;
  }
  లేకపోతే {  /* normal case */
    గుర్తులేని పూర్ణాంకం u = cast_uint(i) + cast_uint(ni);
    తిరిగి (u <= cast_uint(గరిష్ఠ_పూర్ణాంకం) ? u : ~u);
  }
}
#అయితే_ముగింపు


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
స్థిర కణుపు_2 *mainpositionTV (స్థిరరాశి పట్టిక *t, స్థిరరాశి TValue *కీలకం) {
  ఎంపిక (ttypetag(కీలకం)) {
    స్థితి LUA_VNUMINT: {
      lua_Integer i = ivalue(కీలకం);
      తిరిగి hashint(t, i);
    }
    స్థితి LUA_VNUMFLT: {
      lua_సంఖ్య n = fltvalue(కీలకం);
      తిరిగి hashmod(t, l_hashfloat(n));
    }
    స్థితి LUA_VSHRSTR: {
      TString *ts = tsvalue(కీలకం);
      తిరిగి hashstr(t, ts);
    }
    స్థితి LUA_VLNGSTR: {
      TString *ts = tsvalue(కీలకం);
      తిరిగి hashpow2(t, luaS_hashlongstr(ts));
    }
    స్థితి LUA_VFALSE:
      తిరిగి hashboolean(t, 0);
    స్థితి LUA_VTRUE:
      తిరిగి hashboolean(t, 1);
    స్థితి LUA_VLIGHTUSERDATA: {
      శూన్య *p = pvalue(కీలకం);
      తిరిగి hashpointer(t, p);
    }
    స్థితి LUA_VLCF: {
      lua_CFunction f = fvalue(కీలకం);
      తిరిగి hashpointer(t, f);
    }
    అప్రమేయ: {
      GCObject *o = gcvalue(కీలకం);
      తిరిగి hashpointer(t, o);
    }
  }
}


l_sinline కణుపు_2 *mainpositionfromnode (స్థిరరాశి పట్టిక *t, కణుపు_2 *nd) {
  TValue కీలకం;
  getnodekey(cast(lua_State *, ఖాళీ), &కీలకం, nd);
  తిరిగి mainpositionTV(t, &కీలకం);
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
స్థిర పూర్ణాంకం equalkey (స్థిరరాశి TValue *k1, స్థిరరాశి కణుపు_2 *n2, పూర్ణాంకం deadok) {
  ఒకవేళ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ఒకవేళ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      తిరిగి luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    లేకపోతే ఒకవేళ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      తిరిగి gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   లేకపోతే
     తిరిగి 0;  /* otherwise, different variants cannot be equal */
  }
  లేకపోతే {  /* equal variants */
    ఎంపిక (keytt(n2)) {
      స్థితి LUA_VNIL: స్థితి LUA_VFALSE: స్థితి LUA_VTRUE:
        తిరిగి 1;
      స్థితి LUA_VNUMINT:
        తిరిగి (ivalue(k1) == keyival(n2));
      స్థితి LUA_VNUMFLT:
        తిరిగి luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      స్థితి LUA_VLIGHTUSERDATA:
        తిరిగి pvalue(k1) == pvalueraw(keyval(n2));
      స్థితి LUA_VLCF:
        తిరిగి fvalue(k1) == fvalueraw(keyval(n2));
      స్థితి ctb(LUA_VLNGSTR):
        తిరిగి luaS_eqstr(tsvalue(k1), keystrval(n2));
      అప్రమేయ:
        తిరిగి gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
స్థిర స్థిరరాశి TValue *getgeneric (పట్టిక *t, స్థిరరాశి TValue *కీలకం, పూర్ణాంకం deadok) {
  కణుపు_2 *n = mainpositionTV(t, కీలకం);
  కొరకు (;;) {  /* check whether 'key' is somewhere in the chain */
    ఒకవేళ (equalkey(కీలకం, n, deadok))
      తిరిగి gval(n);  /* that's it */
    లేకపోతే {
      పూర్ణాంకం nx = gnext(n);
      ఒకవేళ (nx == 0)
        తిరిగి &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
స్థిర గుర్తులేని checkrange (lua_Integer k, గుర్తులేని హద్దు) {
  తిరిగి (l_castS2U(k) - 1u < హద్దు) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#నిర్వచించు arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#నిర్వచించు ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
స్థిర గుర్తులేని keyinarray (పట్టిక *t, స్థిరరాశి TValue *కీలకం) {
  తిరిగి (ttisinteger(కీలకం)) ? ikeyinarray(t, ivalue(కీలకం)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
స్థిర గుర్తులేని findindex (lua_State *L, పట్టిక *t, TValue *కీలకం,
                               గుర్తులేని asize) {
  గుర్తులేని పూర్ణాంకం i;
  ఒకవేళ (ttisnil(కీలకం)) తిరిగి 0;  /* first iteration */
  i = keyinarray(t, కీలకం);
  ఒకవేళ (i != 0)  /* is 'key' inside array part? */
    తిరిగి i;  /* yes; that's the index */
  లేకపోతే {
    స్థిరరాశి TValue *n = getgeneric(t, కీలకం, 1);
    ఒకవేళ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    తిరిగి (i + 1) + asize;
  }
}


పూర్ణాంకం lua_H_తదుపరి (lua_State *L, పట్టిక *t, StkId కీలకం) {
  గుర్తులేని పూర్ణాంకం asize = t->asize;
  గుర్తులేని పూర్ణాంకం i = findindex(L, t, s2v(కీలకం), asize);  /* find original key */
  కొరకు (; i < asize; i++) {  /* try first array part */
    lu_byte ట్యాగ్ = *get_Arr_ట్యాగ్(t, i);
    ఒకవేళ (!tagisempty(ట్యాగ్)) {  /* a non-empty entry? */
      setivalue(s2v(కీలకం), cast_int(i) + 1);
      farr2val(t, i, ట్యాగ్, s2v(కీలకం + 1));
      తిరిగి 1;
    }
  }
  కొరకు (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ఒకవేళ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      కణుపు_2 *n = gnode(t, i);
      getnodekey(L, s2v(కీలకం), n);
      setobj2s(L, కీలకం + 1, gval(n));
      తిరిగి 1;
    }
  }
  తిరిగి 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#నిర్వచించు extraLastfree(t)	(haslastfree(t) ? పరిమాణం(Limbox) : 0)

/* 'node' size in bytes */
స్థిర size_t sizehash (పట్టిక *t) {
  తిరిగి cast_sizet(sizenode(t)) * పరిమాణం(కణుపు_2) + extraLastfree(t);
}


స్థిర శూన్య freehash (lua_State *L, పట్టిక *t) {
  ఒకవేళ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    అక్షరం *arr = cast_charp(t->కణుపు) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

స్థిర పూర్ణాంకం insertkey (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *విలువ);
స్థిర శూన్య newcheckedkey (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *విలువ);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
రకనిర్వచనం నిర్మాణం {
  గుర్తులేని మొత్తం;
  గుర్తులేని na;
  పూర్ణాంకం deleted;
  గుర్తులేని nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#నిర్వచించు arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
స్థిర గుర్తులేని computesizes (Counters *ct) {
  పూర్ణాంకం i;
  గుర్తులేని పూర్ణాంకం twotoi;  /* 2^i (candidate for optimal size) */
  గుర్తులేని పూర్ణాంకం a = 0;  /* number of elements smaller than 2^i */
  గుర్తులేని పూర్ణాంకం na = 0;  /* number of elements to go to array part */
  గుర్తులేని పూర్ణాంకం optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  కొరకు (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    గుర్తులేని nums = ct->nums[i];
    a += nums;
    ఒకవేళ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  తిరిగి optimal;
}


స్థిర శూన్య countint (lua_Integer కీలకం, Counters *ct) {
  గుర్తులేని పూర్ణాంకం k = arrayindex(కీలకం);
  ఒకవేళ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline పూర్ణాంకం arraykeyisempty (స్థిరరాశి పట్టిక *t, గుర్తులేని కీలకం) {
  పూర్ణాంకం ట్యాగ్ = *get_Arr_ట్యాగ్(t, కీలకం - 1);
  తిరిగి tagisempty(ట్యాగ్);
}


/*
** Count keys in array part of table 't'.
*/
స్థిర శూన్య numusearray (స్థిరరాశి పట్టిక *t, Counters *ct) {
  పూర్ణాంకం lg;
  గుర్తులేని పూర్ణాంకం ttlg;  /* 2^lg */
  గుర్తులేని పూర్ణాంకం ause = 0;  /* summation of 'nums' */
  గుర్తులేని పూర్ణాంకం i = 1;  /* index to traverse all array keys */
  గుర్తులేని పూర్ణాంకం asize = t->asize;
  /* traverse each slice */
  కొరకు (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    గుర్తులేని పూర్ణాంకం lc = 0;  /* counter */
    గుర్తులేని పూర్ణాంకం lim = ttlg;
    ఒకవేళ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ఒకవేళ (i > lim)
        ఆపు;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    కొరకు (; i <= lim; i++) {
      ఒకవేళ (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->మొత్తం += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
స్థిర శూన్య numusehash (స్థిరరాశి పట్టిక *t, Counters *ct) {
  గుర్తులేని i = sizenode(t);
  గుర్తులేని మొత్తం = 0;
  అయితే (i--) {
    కణుపు_2 *n = &t->కణుపు[i];
    ఒకవేళ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    లేకపోతే {
      మొత్తం++;
      ఒకవేళ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->మొత్తం += మొత్తం;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
స్థిర size_t concretesize (గుర్తులేని పూర్ణాంకం కొలత) {
  ఒకవేళ (కొలత == 0)
    తిరిగి 0;
  లేకపోతే  /* space for the two arrays plus an unsigned in between */
    తిరిగి కొలత * (పరిమాణం(విలువ_2) + 1) + పరిమాణం(గుర్తులేని);
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
స్థిర విలువ_2 *resizearray (lua_State *L , పట్టిక *t,
                               గుర్తులేని oldasize,
                               గుర్తులేని newasize) {
  ఒకవేళ (oldasize == newasize)
    తిరిగి t->array;  /* nothing to be done */
  లేకపోతే ఒకవేళ (newasize == 0) {  /* erasing array? */
    విలువ_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    తిరిగి ఖాళీ;
  }
  లేకపోతే {
    size_t newasizeb = concretesize(newasize);
    విలువ_2 *np = cast(విలువ_2 *,
                  luaM_reallocvector(L, ఖాళీ, 0, newasizeb, lu_byte));
    ఒకవేళ (np == ఖాళీ)  /* allocation error? */
      తిరిగి ఖాళీ;
    np += newasize;  /* shift pointer to the end of value segment */
    ఒకవేళ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      విలువ_2 *op = t->array;  /* original array */
      గుర్తులేని tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      జ్ఞాపకం_నకలు(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    తిరిగి np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
స్థిర శూన్య setnodevector (lua_State *L, పట్టిక *t, గుర్తులేని కొలత) {
  ఒకవేళ (కొలత == 0) {  /* no elements to hash part? */
    t->కణుపు = cast(కణుపు_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  లేకపోతే {
    పూర్ణాంకం i;
    పూర్ణాంకం lsize = luaO_ceillog2(కొలత);
    ఒకవేళ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    కొలత = twoto(lsize);
    ఒకవేళ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->కణుపు = luaM_newvector(L, కొలత, కణుపు_2);
    లేకపోతే {
      size_t bsize = కొలత * పరిమాణం(కణుపు_2) + పరిమాణం(Limbox);
      అక్షరం *కణుపు = luaM_newblock(L, bsize);
      t->కణుపు = cast(కణుపు_2 *, కణుపు + పరిమాణం(Limbox));
      getlastfree(t) = gnode(t, కొలత);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    కొరకు (i = 0; i < cast_int(కొలత); i++) {
      కణుపు_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
స్థిర శూన్య reinserthash (lua_State *L, పట్టిక *ot, పట్టిక *t) {
  గుర్తులేని j;
  గుర్తులేని కొలత = sizenode(ot);
  కొరకు (j = 0; j < కొలత; j++) {
    కణుపు_2 *old = gnode(ot, j);
    ఒకవేళ (!isempty(gval(old))) {
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
స్థిర శూన్య exchangehashpart (పట్టిక *t1, పట్టిక *t2) {
  lu_byte lsizenode = t1->lsizenode;
  కణుపు_2 *కణుపు = t1->కణుపు;
  పూర్ణాంకం bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->కణుపు = t2->కణుపు;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->కణుపు = కణుపు;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
స్థిర శూన్య reinsert_Old_ముక్క (పట్టిక *t, గుర్తులేని oldasize,
                                        గుర్తులేని newasize) {
  గుర్తులేని i;
  కొరకు (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte ట్యాగ్ = *get_Arr_ట్యాగ్(t, i);
    ఒకవేళ (!tagisempty(ట్యాగ్)) {  /* a non-empty entry? */
      TValue కీలకం, aux;
      setivalue(&కీలకం, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, ట్యాగ్, &aux);  /* copy value into 'aux' */
      insertkey(t, &కీలకం, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
స్థిర శూన్య clear_New_ముక్క (పట్టిక *t, గుర్తులేని oldasize, గుర్తులేని newasize) {
  కొరకు (; oldasize < newasize; oldasize++)
    *get_Arr_ట్యాగ్(t, oldasize) = LUA_VEMPTY;
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
శూన్య luaH_resize (lua_State *L, పట్టిక *t, గుర్తులేని newasize,
                                          గుర్తులేని nhsize) {
  పట్టిక newt;  /* to keep the new hash part */
  గుర్తులేని oldasize = t->asize;
  విలువ_2 *newarray;
  ఒకవేళ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ఒకవేళ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsert_Old_ముక్క(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ఒకవేళ (l_unlikely(newarray == ఖాళీ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ఒకవేళ (newarray != ఖాళీ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clear_New_ముక్క(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


శూన్య luaH_resizearray (lua_State *L, పట్టిక *t, గుర్తులేని పూర్ణాంకం nasize) {
  గుర్తులేని nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
స్థిర శూన్య rehash (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *ek) {
  గుర్తులేని asize;  /* optimal size for array part */
  Counters ct;
  గుర్తులేని i;
  గుర్తులేని nsize;  /* size for the hash part */
  /* reset counts */
  కొరకు (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.మొత్తం = 1;  /* count extra key */
  ఒకవేళ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ఒకవేళ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  లేకపోతే {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.మొత్తం - ct.na;
  ఒకవేళ (ct.deleted) {  /* table has deleted entries? */
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


పట్టిక *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, పరిమాణం(పట్టిక));
  పట్టిక *t = gco2t(o);
  t->metatable = ఖాళీ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ఖాళీ;
  t->asize = 0;
  setnodevector(L, t, 0);
  తిరిగి t;
}


lu_mem lua_H_కొలత (పట్టిక *t) {
  lu_mem sz = cast(lu_mem, పరిమాణం(పట్టిక)) + concretesize(t->asize);
  ఒకవేళ (!isdummy(t))
    sz += sizehash(t);
  తిరిగి sz;
}


/*
** Frees a table.
*/
శూన్య luaH_free (lua_State *L, పట్టిక *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


స్థిర కణుపు_2 *getfreepos (పట్టిక *t) {
  ఒకవేళ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    అయితే (getlastfree(t) > t->కణుపు) {
      కణుపు_2 *విడుదల = --getlastfree(t);
      ఒకవేళ (keyisnil(విడుదల))
        తిరిగి విడుదల;
    }
  }
  లేకపోతే {  /* no 'lastfree' information */
    గుర్తులేని i = sizenode(t);
    అయితే (i--) {  /* do a linear search */
      కణుపు_2 *విడుదల = gnode(t, i);
      ఒకవేళ (keyisnil(విడుదల))
        తిరిగి విడుదల;
    }
  }
  తిరిగి ఖాళీ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
స్థిర పూర్ణాంకం insertkey (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *విలువ) {
  కణుపు_2 *mp = mainpositionTV(t, కీలకం);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, కీలకం, 0)));
  ఒకవేళ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    కణుపు_2 *othern;
    కణుపు_2 *f = getfreepos(t);  /* get a free place */
    ఒకవేళ (f == ఖాళీ)  /* cannot find a free place? */
      తిరిగి 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ఒకవేళ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      అయితే (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ఒకవేళ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    లేకపోతే {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ఒకవేళ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      లేకపోతే lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, కీలకం);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), విలువ);
  తిరిగి 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
స్థిర శూన్య newcheckedkey (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *విలువ) {
  గుర్తులేని i = keyinarray(t, కీలకం);
  ఒకవేళ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, విలువ);  /* set value in the array */
  లేకపోతే {
    పూర్ణాంకం పూర్తి = insertkey(t, కీలకం, విలువ);  /* insert key in the hash part */
    lua_assert(పూర్తి);  /* it cannot fail */
    cast(శూన్య, పూర్తి);  /* to avoid warnings */
  }
}


స్థిర శూన్య luaH_newkey (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *కీలకం,
                                                 TValue *విలువ) {
  ఒకవేళ (!ttisnil(విలువ)) {  /* do not insert nil values */
    పూర్ణాంకం పూర్తి = insertkey(t, కీలకం, విలువ);
    ఒకవేళ (!పూర్తి) {  /* could not find a free place? */
      rehash(L, t, కీలకం);  /* grow table */
      newcheckedkey(t, కీలకం, విలువ);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), కీలకం);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (శూన్య)0, (శూన్య)0, 1);
  }
}


స్థిర స్థిరరాశి TValue *getintfromhash (పట్టిక *t, lua_Integer కీలకం) {
  కణుపు_2 *n = hashint(t, కీలకం);
  lua_assert(!ikeyinarray(t, కీలకం));
  కొరకు (;;) {  /* check whether 'key' is somewhere in the chain */
    ఒకవేళ (keyisinteger(n) && keyival(n) == కీలకం)
      తిరిగి gval(n);  /* that's it */
    లేకపోతే {
      పూర్ణాంకం nx = gnext(n);
      ఒకవేళ (nx == 0) ఆపు;
      n += nx;
    }
  }
  తిరిగి &absentkey;
}


స్థిర పూర్ణాంకం hashkeyisempty (పట్టిక *t, lua_Unsigned కీలకం) {
  స్థిరరాశి TValue *val = getintfromhash(t, l_castU2S(కీలకం));
  తిరిగి isempty(val);
}


స్థిర lu_byte finishnodeget (స్థిరరాశి TValue *val, TValue *res) {
  ఒకవేళ (!ttisnil(val)) {
    setobj(((lua_State*)ఖాళీ), res, val);
  }
  తిరిగి ttypetag(val);
}


lu_byte luaH_getint (పట్టిక *t, lua_Integer కీలకం, TValue *res) {
  గుర్తులేని k = ikeyinarray(t, కీలకం);
  ఒకవేళ (k > 0) {
    lu_byte ట్యాగ్ = *get_Arr_ట్యాగ్(t, k - 1);
    ఒకవేళ (!tagisempty(ట్యాగ్))
      farr2val(t, k - 1, ట్యాగ్, res);
    తిరిగి ట్యాగ్;
  }
  లేకపోతే
    తిరిగి finishnodeget(getintfromhash(t, కీలకం), res);
}


/*
** search function for short strings
*/
స్థిరరాశి TValue *luaH_Hgetshortstr (పట్టిక *t, TString *కీలకం) {
  కణుపు_2 *n = hashstr(t, కీలకం);
  lua_assert(strisshr(కీలకం));
  కొరకు (;;) {  /* check whether 'key' is somewhere in the chain */
    ఒకవేళ (keyisshrstr(n) && eqshrstr(keystrval(n), కీలకం))
      తిరిగి gval(n);  /* that's it */
    లేకపోతే {
      పూర్ణాంకం nx = gnext(n);
      ఒకవేళ (nx == 0)
        తిరిగి &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (పట్టిక *t, TString *కీలకం, TValue *res) {
  తిరిగి finishnodeget(luaH_Hgetshortstr(t, కీలకం), res);
}


స్థిర స్థిరరాశి TValue *Hgetlongstr (పట్టిక *t, TString *కీలకం) {
  TValue ko;
  lua_assert(!strisshr(కీలకం));
  setsvalue(cast(lua_State *, ఖాళీ), &ko, కీలకం);
  తిరిగి getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


స్థిర స్థిరరాశి TValue *Hgetstr (పట్టిక *t, TString *కీలకం) {
  ఒకవేళ (strisshr(కీలకం))
    తిరిగి luaH_Hgetshortstr(t, కీలకం);
  లేకపోతే
    తిరిగి Hgetlongstr(t, కీలకం);
}


lu_byte luaH_getstr (పట్టిక *t, TString *కీలకం, TValue *res) {
  తిరిగి finishnodeget(Hgetstr(t, కీలకం), res);
}


/*
** main search function
*/
lu_byte luaH_get (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *res) {
  స్థిరరాశి TValue *స్థానం;
  ఎంపిక (ttypetag(కీలకం)) {
    స్థితి LUA_VSHRSTR:
      స్థానం = luaH_Hgetshortstr(t, tsvalue(కీలకం));
      ఆపు;
    స్థితి LUA_VNUMINT:
      తిరిగి luaH_getint(t, ivalue(కీలకం), res);
    స్థితి LUA_VNIL:
      స్థానం = &absentkey;
      ఆపు;
    స్థితి LUA_VNUMFLT: {
      lua_Integer k;
      ఒకవేళ (luaV_flttointeger(fltvalue(కీలకం), &k, F2Ieq)) /* integral index? */
        తిరిగి luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    అప్రమేయ:
      స్థానం = getgeneric(t, కీలకం, 0);
      ఆపు;
  }
  తిరిగి finishnodeget(స్థానం, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
స్థిర పూర్ణాంకం retpsetcode (పట్టిక *t, స్థిరరాశి TValue *స్థానం) {
  ఒకవేళ (isabstkey(స్థానం))
    తిరిగి HNOTFOUND;  /* no slot with that key */
  లేకపోతే  /* return node encoded */
    తిరిగి cast_int((cast(కణుపు_2*, స్థానం) - t->కణుపు)) + HFIRSTNODE;
}


స్థిర పూర్ణాంకం finishnodeset (పట్టిక *t, స్థిరరాశి TValue *స్థానం, TValue *val) {
  ఒకవేళ (!ttisnil(స్థానం)) {
    setobj(((lua_State*)ఖాళీ), cast(TValue*, స్థానం), val);
    తిరిగి HOK;  /* success */
  }
  లేకపోతే
    తిరిగి retpsetcode(t, స్థానం);
}


స్థిర పూర్ణాంకం rawfinishnodeset (స్థిరరాశి TValue *స్థానం, TValue *val) {
  ఒకవేళ (isabstkey(స్థానం))
    తిరిగి 0;  /* no slot with that key */
  లేకపోతే {
    setobj(((lua_State*)ఖాళీ), cast(TValue*, స్థానం), val);
    తిరిగి 1;  /* success */
  }
}


పూర్ణాంకం luaH_psetint (పట్టిక *t, lua_Integer కీలకం, TValue *val) {
  lua_assert(!ikeyinarray(t, కీలకం));
  తిరిగి finishnodeset(t, getintfromhash(t, కీలకం), val);
}


స్థిర పూర్ణాంకం psetint (పట్టిక *t, lua_Integer కీలకం, TValue *val) {
  పూర్ణాంకం hres;
  luaH_fastseti(t, కీలకం, val, hres);
  తిరిగి hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

పూర్ణాంకం luaH_psetshortstr (పట్టిక *t, TString *కీలకం, TValue *val) {
  స్థిరరాశి TValue *స్థానం = luaH_Hgetshortstr(t, కీలకం);
  ఒకవేళ (!ttisnil(స్థానం)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ఖాళీ), cast(TValue*, స్థానం), val);  /* update it */
    తిరిగి HOK;  /*పూర్తి*/
  }
  లేకపోతే ఒకవేళ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ఒకవేళ (ttisnil(val))  /* new value is nil? */
      తిరిగి HOK;  /* done (value is already nil/absent) */
    ఒకవేళ (isabstkey(స్థానం) &&  /* key is absent? */
       !(isblack(t) && iswhite(కీలకం))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ఖాళీ), &tk, కీలకం);
      ఒకవేళ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        తిరిగి HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  తిరిగి retpsetcode(t, స్థానం);
}


పూర్ణాంకం luaH_psetstr (పట్టిక *t, TString *కీలకం, TValue *val) {
  ఒకవేళ (strisshr(కీలకం))
    తిరిగి luaH_psetshortstr(t, కీలకం, val);
  లేకపోతే
    తిరిగి finishnodeset(t, Hgetlongstr(t, కీలకం), val);
}


పూర్ణాంకం luaH_pset (పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *val) {
  ఎంపిక (ttypetag(కీలకం)) {
    స్థితి LUA_VSHRSTR: తిరిగి luaH_psetshortstr(t, tsvalue(కీలకం), val);
    స్థితి LUA_VNUMINT: తిరిగి psetint(t, ivalue(కీలకం), val);
    స్థితి LUA_VNIL: తిరిగి HNOTFOUND;
    స్థితి LUA_VNUMFLT: {
      lua_Integer k;
      ఒకవేళ (luaV_flttointeger(fltvalue(కీలకం), &k, F2Ieq)) /* integral index? */
        తిరిగి psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    అప్రమేయ:
      తిరిగి finishnodeset(t, getgeneric(t, కీలకం, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
శూన్య luaH_finishset (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *కీలకం,
                                    TValue *విలువ, పూర్ణాంకం hres) {
  lua_assert(hres != HOK);
  ఒకవేళ (hres == HNOTFOUND) {
    TValue aux;
    స్థిరరాశి TValue *actk = కీలకం;  /* actual key to insert */
    ఒకవేళ (l_unlikely(ttisnil(కీలకం)))
      luaG_runerror(L, "table index is nil");
    లేకపోతే ఒకవేళ (ttisfloat(కీలకం)) {
      lua_సంఖ్య f = fltvalue(కీలకం);
      lua_Integer k;
      ఒకవేళ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      లేకపోతే ఒకవేళ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    లేకపోతే ఒకవేళ (isextstr(కీలకం)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(కీలకం));
      setsvalue2s(L, L->శిఖరం.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->శిఖరం.p - 1), విలువ);
      L->శిఖరం.p--;
      తిరిగి;
    }
    luaH_newkey(L, t, actk, విలువ);
  }
  లేకపోతే ఒకవేళ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), విలువ);
  }
  లేకపోతే {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), విలువ);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
శూన్య lua_H_సమితి (lua_State *L, పట్టిక *t, స్థిరరాశి TValue *కీలకం, TValue *విలువ) {
  పూర్ణాంకం hres = luaH_pset(t, కీలకం, విలువ);
  ఒకవేళ (hres != HOK)
    luaH_finishset(L, t, కీలకం, విలువ, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
శూన్య luaH_setint (lua_State *L, పట్టిక *t, lua_Integer కీలకం, TValue *విలువ) {
  గుర్తులేని ik = ikeyinarray(t, కీలకం);
  ఒకవేళ (ik > 0)
    obj2arr(t, ik - 1, విలువ);
  లేకపోతే {
    పూర్ణాంకం ok = rawfinishnodeset(getintfromhash(t, కీలకం), విలువ);
    ఒకవేళ (!ok) {
      TValue k;
      setivalue(&k, కీలకం);
      luaH_newkey(L, t, &k, విలువ);
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
స్థిర lua_Unsigned hash_శోధన (lua_State *L, పట్టిక *t, గుర్తులేని asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  గుర్తులేని rnd = G(L)->విత్తనం;
  పూర్ణాంకం n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  గుర్తులేని ముసుగు_2 = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  గుర్తులేని incr = (rnd & ముసుగు_2) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  అయితే (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ఒకవేళ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    లేకపోతే {
      j = LUA_MAXINTEGER;
      ఒకవేళ (hashkeyisempty(t, j))  /* t[j] not present? */
        ఆపు;  /* 'j' now is an absent index */
      లేకపోతే  /* weird case */
        తిరిగి j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  అయితే (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ఒకవేళ (hashkeyisempty(t, m)) j = m;
    లేకపోతే i = m;
  }
  తిరిగి i;
}


స్థిర గుర్తులేని పూర్ణాంకం binsearch (పట్టిక *array, గుర్తులేని పూర్ణాంకం i, గుర్తులేని పూర్ణాంకం j) {
  lua_assert(i <= j);
  అయితే (j - i > 1u) {  /*ద్విమాన శోధన*/
    గుర్తులేని పూర్ణాంకం m = (i + j) / 2;
    ఒకవేళ (arraykeyisempty(array, m)) j = m;
    లేకపోతే i = m;
  }
  తిరిగి i;
}


/* return a border, saving it as a hint for next call */
స్థిర lua_Unsigned newhint (పట్టిక *t, గుర్తులేని hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  తిరిగి hint;
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
lua_Unsigned luaH_getn (lua_State *L, పట్టిక *t) {
  గుర్తులేని asize = t->asize;
  ఒకవేళ (asize > 0) {  /* is there an array part? */
    స్థిరరాశి గుర్తులేని maxvicinity = 4;
    గుర్తులేని హద్దు = *lenhint(t);  /* start with the hint */
    ఒకవేళ (హద్దు == 0)
      హద్దు = 1;  /* make limit a valid index in the array */
    ఒకవేళ (arraykeyisempty(t, హద్దు)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      గుర్తులేని i;
      /* look for a border in the vicinity of the hint */
      కొరకు (i = 0; i < maxvicinity && హద్దు > 1; i++) {
        హద్దు--;
        ఒకవేళ (!arraykeyisempty(t, హద్దు))
          తిరిగి newhint(t, హద్దు);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      తిరిగి newhint(t, binsearch(t, 0, హద్దు));
    }
    లేకపోతే {  /* 'limit' is present in table; look for a border after it */
      గుర్తులేని i;
      /* look for a border in the vicinity of the hint */
      కొరకు (i = 0; i < maxvicinity && హద్దు < asize; i++) {
        హద్దు++;
        ఒకవేళ (arraykeyisempty(t, హద్దు))
          తిరిగి newhint(t, హద్దు - 1);  /* 'limit - 1' is a border */
      }
      ఒకవేళ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        తిరిగి newhint(t, binsearch(t, హద్దు, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ఒకవేళ (isdummy(t) || hashkeyisempty(t, asize + 1))
    తిరిగి asize;  /* 'asize + 1' is empty */
  లేకపోతే  /* 'asize + 1' is also non empty */
    తిరిగి hash_శోధన(L, t, asize);
}



#ఒకవేళ defined(LUA_DEBUG)

/* export this function for the test library */

కణుపు_2 *luaH_mainposition (స్థిరరాశి పట్టిక *t, స్థిరరాశి TValue *కీలకం) {
  తిరిగి mainpositionTV(t, కీలకం);
}

#అయితే_ముగింపు
