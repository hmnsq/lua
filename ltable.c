/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


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

#অন্তর্ভাৱ <math.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lstring.h"
#অন্তর্ভাৱ "ltable.h"
#অন্তর্ভাৱ "lvm.h"


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
প্রকারনির্দেশ সংরচনা { গ্ৰন্থি_2 *dummy; গ্ৰন্থি_2 follows_p_গ্ৰন্থি; } Limbox_aux;

প্রকারনির্দেশ সঙ্ঘ {
  গ্ৰন্থি_2 *lastfree;
  ৱর্ণ padding[offsetof(Limbox_aux, follows_p_গ্ৰন্থি)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->গ্ৰন্থি) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(পূর্ণাঙ্ক) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(পরিমাণ(মান_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, গ্ৰন্থি_2)


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
স্থির ধ্রুৱ গ্ৰন্থি_2 dummynode_ = {
  {{রিক্ত}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {রিক্ত}}  /* key type, next, and key value */
};


স্থির ধ্রুৱ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
স্থির গ্ৰন্থি_2 *hashint (ধ্রুৱ Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  যদি (ui <= cast_uint(INT_MAX))
    ঘূৰাই gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  অন্যথা
    ঘূৰাই hashmod(t, ui);
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
#যদি !defined(l_hashfloat)
স্থির unsigned l_hashfloat (lua_সংখ্যা n) {
  পূর্ণাঙ্ক i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_অঙ্ক(INT_MIN);
  যদি (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_অঙ্ক(HUGE_VAL));
    ঘূৰাই 0;
  }
  অন্যথা {  /* normal case */
    unsigned পূর্ণাঙ্ক u = cast_uint(i) + cast_uint(ni);
    ঘূৰাই (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
স্থির গ্ৰন্থি_2 *mainpositionTV (ধ্রুৱ Table *t, ধ্রুৱ TValue *চাবি) {
  ৱিকল্প (ttypetag(চাবি)) {
    পক্ষ LUA_VNUMINT: {
      lua_Integer i = ivalue(চাবি);
      ঘূৰাই hashint(t, i);
    }
    পক্ষ LUA_VNUMFLT: {
      lua_সংখ্যা n = fltvalue(চাবি);
      ঘূৰাই hashmod(t, l_hashfloat(n));
    }
    পক্ষ LUA_VSHRSTR: {
      TString *ts = tsvalue(চাবি);
      ঘূৰাই hashstr(t, ts);
    }
    পক্ষ LUA_VLNGSTR: {
      TString *ts = tsvalue(চাবি);
      ঘূৰাই hashpow2(t, luaS_hashlongstr(ts));
    }
    পক্ষ LUA_VFALSE:
      ঘূৰাই hashboolean(t, 0);
    পক্ষ LUA_VTRUE:
      ঘূৰাই hashboolean(t, 1);
    পক্ষ LUA_VLIGHTUSERDATA: {
      শূন্য *p = pvalue(চাবি);
      ঘূৰাই hashpointer(t, p);
    }
    পক্ষ LUA_VLCF: {
      lua_CFunction f = fvalue(চাবি);
      ঘূৰাই hashpointer(t, f);
    }
    স্ৱভাৱ: {
      GCObject *o = gcvalue(চাবি);
      ঘূৰাই hashpointer(t, o);
    }
  }
}


l_sinline গ্ৰন্থি_2 *mainpositionfromnode (ধ্রুৱ Table *t, গ্ৰন্থি_2 *nd) {
  TValue চাবি;
  getnodekey(cast(lua_State *, রিক্ত), &চাবি, nd);
  ঘূৰাই mainpositionTV(t, &চাবি);
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
স্থির পূর্ণাঙ্ক equalkey (ধ্রুৱ TValue *k1, ধ্রুৱ গ্ৰন্থি_2 *n2, পূর্ণাঙ্ক deadok) {
  যদি (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    যদি (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ঘূৰাই luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    অন্যথা যদি (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ঘূৰাই gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   অন্যথা
     ঘূৰাই 0;  /* otherwise, different variants cannot be equal */
  }
  অন্যথা {  /* equal variants */
    ৱিকল্প (keytt(n2)) {
      পক্ষ LUA_VNIL: পক্ষ LUA_VFALSE: পক্ষ LUA_VTRUE:
        ঘূৰাই 1;
      পক্ষ LUA_VNUMINT:
        ঘূৰাই (ivalue(k1) == keyival(n2));
      পক্ষ LUA_VNUMFLT:
        ঘূৰাই luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      পক্ষ LUA_VLIGHTUSERDATA:
        ঘূৰাই pvalue(k1) == pvalueraw(keyval(n2));
      পক্ষ LUA_VLCF:
        ঘূৰাই fvalue(k1) == fvalueraw(keyval(n2));
      পক্ষ ctb(LUA_VLNGSTR):
        ঘূৰাই luaS_eqstr(tsvalue(k1), keystrval(n2));
      স্ৱভাৱ:
        ঘূৰাই gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
স্থির ধ্রুৱ TValue *getgeneric (Table *t, ধ্রুৱ TValue *চাবি, পূর্ণাঙ্ক deadok) {
  গ্ৰন্থি_2 *n = mainpositionTV(t, চাবি);
  বাবে (;;) {  /* check whether 'key' is somewhere in the chain */
    যদি (equalkey(চাবি, n, deadok))
      ঘূৰাই gval(n);  /* that's it */
    অন্যথা {
      পূর্ণাঙ্ক nx = gnext(n);
      যদি (nx == 0)
        ঘূৰাই &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
স্থির unsigned checkrange (lua_Integer k, unsigned limit) {
  ঘূৰাই (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
স্থির unsigned keyinarray (Table *t, ধ্রুৱ TValue *চাবি) {
  ঘূৰাই (ttisinteger(চাবি)) ? ikeyinarray(t, ivalue(চাবি)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
স্থির unsigned findindex (lua_State *L, Table *t, TValue *চাবি,
                               unsigned asize) {
  unsigned পূর্ণাঙ্ক i;
  যদি (ttisnil(চাবি)) ঘূৰাই 0;  /* first iteration */
  i = keyinarray(t, চাবি);
  যদি (i != 0)  /* is 'key' inside array part? */
    ঘূৰাই i;  /* yes; that's the index */
  অন্যথা {
    ধ্রুৱ TValue *n = getgeneric(t, চাবি, 1);
    যদি (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ঘূৰাই (i + 1) + asize;
  }
}


পূর্ণাঙ্ক luaH_next (lua_State *L, Table *t, StkId চাবি) {
  unsigned পূর্ণাঙ্ক asize = t->asize;
  unsigned পূর্ণাঙ্ক i = findindex(L, t, s2v(চাবি), asize);  /* find original key */
  বাবে (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    যদি (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(চাবি), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(চাবি + 1));
      ঘূৰাই 1;
    }
  }
  বাবে (i -= asize; i < sizenode(t); i++) {  /* hash part */
    যদি (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      গ্ৰন্থি_2 *n = gnode(t, i);
      getnodekey(L, s2v(চাবি), n);
      setobj2s(L, চাবি + 1, gval(n));
      ঘূৰাই 1;
    }
  }
  ঘূৰাই 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? পরিমাণ(Limbox) : 0)

/* 'node' size in bytes */
স্থির size_t sizehash (Table *t) {
  ঘূৰাই cast_sizet(sizenode(t)) * পরিমাণ(গ্ৰন্থি_2) + extraLastfree(t);
}


স্থির শূন্য freehash (lua_State *L, Table *t) {
  যদি (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    ৱর্ণ *arr = cast_charp(t->গ্ৰন্থি) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

স্থির পূর্ণাঙ্ক insertkey (Table *t, ধ্রুৱ TValue *চাবি, TValue *মান);
স্থির শূন্য newcheckedkey (Table *t, ধ্রুৱ TValue *চাবি, TValue *মান);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
প্রকারনির্দেশ সংরচনা {
  unsigned total;
  unsigned na;
  পূর্ণাঙ্ক deleted;
  unsigned nums[MAXABITS + 1];
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
স্থির unsigned computesizes (Counters *ct) {
  পূর্ণাঙ্ক i;
  unsigned পূর্ণাঙ্ক twotoi;  /* 2^i (candidate for optimal size) */
  unsigned পূর্ণাঙ্ক a = 0;  /* number of elements smaller than 2^i */
  unsigned পূর্ণাঙ্ক na = 0;  /* number of elements to go to array part */
  unsigned পূর্ণাঙ্ক optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  বাবে (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    যদি (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ঘূৰাই optimal;
}


স্থির শূন্য countint (lua_Integer চাবি, Counters *ct) {
  unsigned পূর্ণাঙ্ক k = arrayindex(চাবি);
  যদি (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline পূর্ণাঙ্ক arraykeyisempty (ধ্রুৱ Table *t, unsigned চাবি) {
  পূর্ণাঙ্ক tag = *getArrTag(t, চাবি - 1);
  ঘূৰাই tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
স্থির শূন্য numusearray (ধ্রুৱ Table *t, Counters *ct) {
  পূর্ণাঙ্ক lg;
  unsigned পূর্ণাঙ্ক ttlg;  /* 2^lg */
  unsigned পূর্ণাঙ্ক ause = 0;  /* summation of 'nums' */
  unsigned পূর্ণাঙ্ক i = 1;  /* index to traverse all array keys */
  unsigned পূর্ণাঙ্ক asize = t->asize;
  /* traverse each slice */
  বাবে (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned পূর্ণাঙ্ক lc = 0;  /* counter */
    unsigned পূর্ণাঙ্ক lim = ttlg;
    যদি (lim > asize) {
      lim = asize;  /* adjust upper limit */
      যদি (i > lim)
        ভঙ্গ;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    বাবে (; i <= lim; i++) {
      যদি (!arraykeyisempty(t, i))
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
স্থির শূন্য numusehash (ধ্রুৱ Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  যেতিয়ালৈ (i--) {
    গ্ৰন্থি_2 *n = &t->গ্ৰন্থি[i];
    যদি (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    অন্যথা {
      total++;
      যদি (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
স্থির size_t concretesize (unsigned পূর্ণাঙ্ক size) {
  যদি (size == 0)
    ঘূৰাই 0;
  অন্যথা  /* space for the two arrays plus an unsigned in between */
    ঘূৰাই size * (পরিমাণ(মান_2) + 1) + পরিমাণ(unsigned);
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
স্থির মান_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  যদি (oldasize == newasize)
    ঘূৰাই t->array;  /* nothing to be done */
  অন্যথা যদি (newasize == 0) {  /* erasing array? */
    মান_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ঘূৰাই রিক্ত;
  }
  অন্যথা {
    size_t newasizeb = concretesize(newasize);
    মান_2 *np = cast(মান_2 *,
                  luaM_reallocvector(L, রিক্ত, 0, newasizeb, lu_byte));
    যদি (np == রিক্ত)  /* allocation error? */
      ঘূৰাই রিক্ত;
    np += newasize;  /* shift pointer to the end of value segment */
    যদি (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      মান_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ঘূৰাই np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
স্থির শূন্য setnodevector (lua_State *L, Table *t, unsigned size) {
  যদি (size == 0) {  /* no elements to hash part? */
    t->গ্ৰন্থি = cast(গ্ৰন্থি_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  অন্যথা {
    পূর্ণাঙ্ক i;
    পূর্ণাঙ্ক lsize = luaO_ceillog2(size);
    যদি (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    যদি (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->গ্ৰন্থি = luaM_newvector(L, size, গ্ৰন্থি_2);
    অন্যথা {
      size_t bsize = size * পরিমাণ(গ্ৰন্থি_2) + পরিমাণ(Limbox);
      ৱর্ণ *গ্ৰন্থি = luaM_newblock(L, bsize);
      t->গ্ৰন্থি = cast(গ্ৰন্থি_2 *, গ্ৰন্থি + পরিমাণ(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    বাবে (i = 0; i < cast_int(size); i++) {
      গ্ৰন্থি_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
স্থির শূন্য reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  বাবে (j = 0; j < size; j++) {
    গ্ৰন্থি_2 *old = gnode(ot, j);
    যদি (!isempty(gval(old))) {
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
স্থির শূন্য exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  গ্ৰন্থি_2 *গ্ৰন্থি = t1->গ্ৰন্থি;
  পূর্ণাঙ্ক bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->গ্ৰন্থি = t2->গ্ৰন্থি;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->গ্ৰন্থি = গ্ৰন্থি;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
স্থির শূন্য reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  বাবে (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    যদি (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue চাবি, aux;
      setivalue(&চাবি, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &চাবি, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
স্থির শূন্য clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  বাবে (; oldasize < newasize; oldasize++)
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
শূন্য luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  মান_2 *newarray;
  যদি (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  যদি (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  যদি (l_unlikely(newarray == রিক্ত && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  যদি (newarray != রিক্ত)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


শূন্য luaH_resizearray (lua_State *L, Table *t, unsigned পূর্ণাঙ্ক nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
স্থির শূন্য rehash (lua_State *L, Table *t, ধ্রুৱ TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  বাবে (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  যদি (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  যদি (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  অন্যথা {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  যদি (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, পরিমাণ(Table));
  Table *t = gco2t(o);
  t->metatable = রিক্ত;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = রিক্ত;
  t->asize = 0;
  setnodevector(L, t, 0);
  ঘূৰাই t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, পরিমাণ(Table)) + concretesize(t->asize);
  যদি (!isdummy(t))
    sz += sizehash(t);
  ঘূৰাই sz;
}


/*
** Frees a table.
*/
শূন্য luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


স্থির গ্ৰন্থি_2 *getfreepos (Table *t) {
  যদি (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    যেতিয়ালৈ (getlastfree(t) > t->গ্ৰন্থি) {
      গ্ৰন্থি_2 *মুক্ত = --getlastfree(t);
      যদি (keyisnil(মুক্ত))
        ঘূৰাই মুক্ত;
    }
  }
  অন্যথা {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    যেতিয়ালৈ (i--) {  /* do a linear search */
      গ্ৰন্থি_2 *মুক্ত = gnode(t, i);
      যদি (keyisnil(মুক্ত))
        ঘূৰাই মুক্ত;
    }
  }
  ঘূৰাই রিক্ত;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
স্থির পূর্ণাঙ্ক insertkey (Table *t, ধ্রুৱ TValue *চাবি, TValue *মান) {
  গ্ৰন্থি_2 *mp = mainpositionTV(t, চাবি);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, চাবি, 0)));
  যদি (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    গ্ৰন্থি_2 *othern;
    গ্ৰন্থি_2 *f = getfreepos(t);  /* get a free place */
    যদি (f == রিক্ত)  /* cannot find a free place? */
      ঘূৰাই 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    যদি (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      যেতিয়ালৈ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      যদি (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    অন্যথা {  /* colliding node is in its own main position */
      /* new node will go into free position */
      যদি (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      অন্যথা lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, চাবি);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), মান);
  ঘূৰাই 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
স্থির শূন্য newcheckedkey (Table *t, ধ্রুৱ TValue *চাবি, TValue *মান) {
  unsigned i = keyinarray(t, চাবি);
  যদি (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, মান);  /* set value in the array */
  অন্যথা {
    পূর্ণাঙ্ক done = insertkey(t, চাবি, মান);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(শূন্য, done);  /* to avoid warnings */
  }
}


স্থির শূন্য luaH_newkey (lua_State *L, Table *t, ধ্রুৱ TValue *চাবি,
                                                 TValue *মান) {
  যদি (!ttisnil(মান)) {  /* do not insert nil values */
    পূর্ণাঙ্ক done = insertkey(t, চাবি, মান);
    যদি (!done) {  /* could not find a free place? */
      rehash(L, t, চাবি);  /* grow table */
      newcheckedkey(t, চাবি, মান);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), চাবি);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (শূন্য)0, (শূন্য)0, 1);
  }
}


স্থির ধ্রুৱ TValue *getintfromhash (Table *t, lua_Integer চাবি) {
  গ্ৰন্থি_2 *n = hashint(t, চাবি);
  lua_assert(!ikeyinarray(t, চাবি));
  বাবে (;;) {  /* check whether 'key' is somewhere in the chain */
    যদি (keyisinteger(n) && keyival(n) == চাবি)
      ঘূৰাই gval(n);  /* that's it */
    অন্যথা {
      পূর্ণাঙ্ক nx = gnext(n);
      যদি (nx == 0) ভঙ্গ;
      n += nx;
    }
  }
  ঘূৰাই &absentkey;
}


স্থির পূর্ণাঙ্ক hashkeyisempty (Table *t, lua_Unsigned চাবি) {
  ধ্রুৱ TValue *val = getintfromhash(t, l_castU2S(চাবি));
  ঘূৰাই isempty(val);
}


স্থির lu_byte finishnodeget (ধ্রুৱ TValue *val, TValue *res) {
  যদি (!ttisnil(val)) {
    setobj(((lua_State*)রিক্ত), res, val);
  }
  ঘূৰাই ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer চাবি, TValue *res) {
  unsigned k = ikeyinarray(t, চাবি);
  যদি (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    যদি (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    ঘূৰাই tag;
  }
  অন্যথা
    ঘূৰাই finishnodeget(getintfromhash(t, চাবি), res);
}


/*
** search function for short strings
*/
ধ্রুৱ TValue *luaH_Hgetshortstr (Table *t, TString *চাবি) {
  গ্ৰন্থি_2 *n = hashstr(t, চাবি);
  lua_assert(strisshr(চাবি));
  বাবে (;;) {  /* check whether 'key' is somewhere in the chain */
    যদি (keyisshrstr(n) && eqshrstr(keystrval(n), চাবি))
      ঘূৰাই gval(n);  /* that's it */
    অন্যথা {
      পূর্ণাঙ্ক nx = gnext(n);
      যদি (nx == 0)
        ঘূৰাই &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *চাবি, TValue *res) {
  ঘূৰাই finishnodeget(luaH_Hgetshortstr(t, চাবি), res);
}


স্থির ধ্রুৱ TValue *Hgetlongstr (Table *t, TString *চাবি) {
  TValue ko;
  lua_assert(!strisshr(চাবি));
  setsvalue(cast(lua_State *, রিক্ত), &ko, চাবি);
  ঘূৰাই getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


স্থির ধ্রুৱ TValue *Hgetstr (Table *t, TString *চাবি) {
  যদি (strisshr(চাবি))
    ঘূৰাই luaH_Hgetshortstr(t, চাবি);
  অন্যথা
    ঘূৰাই Hgetlongstr(t, চাবি);
}


lu_byte luaH_getstr (Table *t, TString *চাবি, TValue *res) {
  ঘূৰাই finishnodeget(Hgetstr(t, চাবি), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, ধ্রুৱ TValue *চাবি, TValue *res) {
  ধ্রুৱ TValue *slot;
  ৱিকল্প (ttypetag(চাবি)) {
    পক্ষ LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(চাবি));
      ভঙ্গ;
    পক্ষ LUA_VNUMINT:
      ঘূৰাই luaH_getint(t, ivalue(চাবি), res);
    পক্ষ LUA_VNIL:
      slot = &absentkey;
      ভঙ্গ;
    পক্ষ LUA_VNUMFLT: {
      lua_Integer k;
      যদি (luaV_flttointeger(fltvalue(চাবি), &k, F2Ieq)) /* integral index? */
        ঘূৰাই luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    স্ৱভাৱ:
      slot = getgeneric(t, চাবি, 0);
      ভঙ্গ;
  }
  ঘূৰাই finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
স্থির পূর্ণাঙ্ক retpsetcode (Table *t, ধ্রুৱ TValue *slot) {
  যদি (isabstkey(slot))
    ঘূৰাই HNOTFOUND;  /* no slot with that key */
  অন্যথা  /* return node encoded */
    ঘূৰাই cast_int((cast(গ্ৰন্থি_2*, slot) - t->গ্ৰন্থি)) + HFIRSTNODE;
}


স্থির পূর্ণাঙ্ক finishnodeset (Table *t, ধ্রুৱ TValue *slot, TValue *val) {
  যদি (!ttisnil(slot)) {
    setobj(((lua_State*)রিক্ত), cast(TValue*, slot), val);
    ঘূৰাই HOK;  /* success */
  }
  অন্যথা
    ঘূৰাই retpsetcode(t, slot);
}


স্থির পূর্ণাঙ্ক rawfinishnodeset (ধ্রুৱ TValue *slot, TValue *val) {
  যদি (isabstkey(slot))
    ঘূৰাই 0;  /* no slot with that key */
  অন্যথা {
    setobj(((lua_State*)রিক্ত), cast(TValue*, slot), val);
    ঘূৰাই 1;  /* success */
  }
}


পূর্ণাঙ্ক luaH_psetint (Table *t, lua_Integer চাবি, TValue *val) {
  lua_assert(!ikeyinarray(t, চাবি));
  ঘূৰাই finishnodeset(t, getintfromhash(t, চাবি), val);
}


স্থির পূর্ণাঙ্ক psetint (Table *t, lua_Integer চাবি, TValue *val) {
  পূর্ণাঙ্ক hres;
  luaH_fastseti(t, চাবি, val, hres);
  ঘূৰাই hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

পূর্ণাঙ্ক luaH_psetshortstr (Table *t, TString *চাবি, TValue *val) {
  ধ্রুৱ TValue *slot = luaH_Hgetshortstr(t, চাবি);
  যদি (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)রিক্ত), cast(TValue*, slot), val);  /* update it */
    ঘূৰাই HOK;  /* done */
  }
  অন্যথা যদি (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    যদি (ttisnil(val))  /* new value is nil? */
      ঘূৰাই HOK;  /* done (value is already nil/absent) */
    যদি (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(চাবি))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, রিক্ত), &tk, চাবি);
      যদি (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ঘূৰাই HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ঘূৰাই retpsetcode(t, slot);
}


পূর্ণাঙ্ক luaH_psetstr (Table *t, TString *চাবি, TValue *val) {
  যদি (strisshr(চাবি))
    ঘূৰাই luaH_psetshortstr(t, চাবি, val);
  অন্যথা
    ঘূৰাই finishnodeset(t, Hgetlongstr(t, চাবি), val);
}


পূর্ণাঙ্ক luaH_pset (Table *t, ধ্রুৱ TValue *চাবি, TValue *val) {
  ৱিকল্প (ttypetag(চাবি)) {
    পক্ষ LUA_VSHRSTR: ঘূৰাই luaH_psetshortstr(t, tsvalue(চাবি), val);
    পক্ষ LUA_VNUMINT: ঘূৰাই psetint(t, ivalue(চাবি), val);
    পক্ষ LUA_VNIL: ঘূৰাই HNOTFOUND;
    পক্ষ LUA_VNUMFLT: {
      lua_Integer k;
      যদি (luaV_flttointeger(fltvalue(চাবি), &k, F2Ieq)) /* integral index? */
        ঘূৰাই psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    স্ৱভাৱ:
      ঘূৰাই finishnodeset(t, getgeneric(t, চাবি, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
শূন্য luaH_finishset (lua_State *L, Table *t, ধ্রুৱ TValue *চাবি,
                                    TValue *মান, পূর্ণাঙ্ক hres) {
  lua_assert(hres != HOK);
  যদি (hres == HNOTFOUND) {
    TValue aux;
    ধ্রুৱ TValue *actk = চাবি;  /* actual key to insert */
    যদি (l_unlikely(ttisnil(চাবি)))
      luaG_runerror(L, "table index is nil");
    অন্যথা যদি (ttisfloat(চাবি)) {
      lua_সংখ্যা f = fltvalue(চাবি);
      lua_Integer k;
      যদি (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      অন্যথা যদি (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    অন্যথা যদি (isextstr(চাবি)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(চাবি));
      setsvalue2s(L, L->শীৰ্ষ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->শীৰ্ষ.p - 1), মান);
      L->শীৰ্ষ.p--;
      ঘূৰাই;
    }
    luaH_newkey(L, t, actk, মান);
  }
  অন্যথা যদি (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), মান);
  }
  অন্যথা {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), মান);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
শূন্য luaH_set (lua_State *L, Table *t, ধ্রুৱ TValue *চাবি, TValue *মান) {
  পূর্ণাঙ্ক hres = luaH_pset(t, চাবি, মান);
  যদি (hres != HOK)
    luaH_finishset(L, t, চাবি, মান, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
শূন্য luaH_setint (lua_State *L, Table *t, lua_Integer চাবি, TValue *মান) {
  unsigned ik = ikeyinarray(t, চাবি);
  যদি (ik > 0)
    obj2arr(t, ik - 1, মান);
  অন্যথা {
    পূর্ণাঙ্ক ok = rawfinishnodeset(getintfromhash(t, চাবি), মান);
    যদি (!ok) {
      TValue k;
      setivalue(&k, চাবি);
      luaH_newkey(L, t, &k, মান);
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
স্থির lua_Unsigned hash_অনুসন্ধান (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  পূর্ণাঙ্ক n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  যেতিয়ালৈ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    যদি (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    অন্যথা {
      j = LUA_MAXINTEGER;
      যদি (hashkeyisempty(t, j))  /* t[j] not present? */
        ভঙ্গ;  /* 'j' now is an absent index */
      অন্যথা  /* weird case */
        ঘূৰাই j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  যেতিয়ালৈ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    যদি (hashkeyisempty(t, m)) j = m;
    অন্যথা i = m;
  }
  ঘূৰাই i;
}


স্থির unsigned পূর্ণাঙ্ক binsearch (Table *array, unsigned পূর্ণাঙ্ক i, unsigned পূর্ণাঙ্ক j) {
  lua_assert(i <= j);
  যেতিয়ালৈ (j - i > 1u) {  /*দ্বিমিক অনুসন্ধান*/
    unsigned পূর্ণাঙ্ক m = (i + j) / 2;
    যদি (arraykeyisempty(array, m)) j = m;
    অন্যথা i = m;
  }
  ঘূৰাই i;
}


/* return a border, saving it as a hint for next call */
স্থির lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ঘূৰাই hint;
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
  unsigned asize = t->asize;
  যদি (asize > 0) {  /* is there an array part? */
    ধ্রুৱ unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    যদি (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    যদি (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      বাবে (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        যদি (!arraykeyisempty(t, limit))
          ঘূৰাই newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ঘূৰাই newhint(t, binsearch(t, 0, limit));
    }
    অন্যথা {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      বাবে (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        যদি (arraykeyisempty(t, limit))
          ঘূৰাই newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      যদি (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ঘূৰাই newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  যদি (isdummy(t) || hashkeyisempty(t, asize + 1))
    ঘূৰাই asize;  /* 'asize + 1' is empty */
  অন্যথা  /* 'asize + 1' is also non empty */
    ঘূৰাই hash_অনুসন্ধান(L, t, asize);
}



#যদি defined(LUA_DEBUG)

/* export this function for the test library */

গ্ৰন্থি_2 *luaH_mainposition (ধ্রুৱ Table *t, ধ্রুৱ TValue *চাবি) {
  ঘূৰাই mainpositionTV(t, চাবি);
}

#endif
