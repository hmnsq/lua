/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


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

#அந்தர்பாவ <math.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "lvm.h"


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
வகைவரை கட்டமைப்பு { முனை_2 *dummy; முனை_2 follows_p_முனை; } Limbox_aux;

வகைவரை ஒன்றிப்பு {
  முனை_2 *lastfree;
  எழுத்து padding[offsetof(Limbox_aux, follows_p_முனை)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->முனை) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(முழு) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(அளவு(மதிப்பு_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, முனை_2)


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
நிலையான மாறிலி முனை_2 dummynode_ = {
  {{வெற்று}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {வெற்று}}  /* key type, next, and key value */
};


நிலையான மாறிலி TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
நிலையான முனை_2 *hashint (மாறிலி Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  எனில் (ui <= cast_uint(INT_MAX))
    திரும்பு gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  இல்லையெனில்
    திரும்பு hashmod(t, ui);
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
#எனில் !defined(l_hashfloat)
நிலையான unsigned l_hashfloat (lua_எண் n) {
  முழு i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_இலக்கம்(INT_MIN);
  எனில் (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_இலக்கம்(HUGE_VAL));
    திரும்பு 0;
  }
  இல்லையெனில் {  /* normal case */
    unsigned முழு u = cast_uint(i) + cast_uint(ni);
    திரும்பு (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
நிலையான முனை_2 *mainpositionTV (மாறிலி Table *t, மாறிலி TValue *சாவி) {
  தேர்வு (ttypetag(சாவி)) {
    நிலை LUA_VNUMINT: {
      lua_Integer i = ivalue(சாவி);
      திரும்பு hashint(t, i);
    }
    நிலை LUA_VNUMFLT: {
      lua_எண் n = fltvalue(சாவி);
      திரும்பு hashmod(t, l_hashfloat(n));
    }
    நிலை LUA_VSHRSTR: {
      TString *ts = tsvalue(சாவி);
      திரும்பு hashstr(t, ts);
    }
    நிலை LUA_VLNGSTR: {
      TString *ts = tsvalue(சாவி);
      திரும்பு hashpow2(t, luaS_hashlongstr(ts));
    }
    நிலை LUA_VFALSE:
      திரும்பு hashboolean(t, 0);
    நிலை LUA_VTRUE:
      திரும்பு hashboolean(t, 1);
    நிலை LUA_VLIGHTUSERDATA: {
      வெறுமை *p = pvalue(சாவி);
      திரும்பு hashpointer(t, p);
    }
    நிலை LUA_VLCF: {
      lua_CFunction f = fvalue(சாவி);
      திரும்பு hashpointer(t, f);
    }
    இயல்பு: {
      GCObject *o = gcvalue(சாவி);
      திரும்பு hashpointer(t, o);
    }
  }
}


l_sinline முனை_2 *mainpositionfromnode (மாறிலி Table *t, முனை_2 *nd) {
  TValue சாவி;
  getnodekey(cast(lua_State *, வெற்று), &சாவி, nd);
  திரும்பு mainpositionTV(t, &சாவி);
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
நிலையான முழு equalkey (மாறிலி TValue *k1, மாறிலி முனை_2 *n2, முழு deadok) {
  எனில் (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    எனில் (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      திரும்பு luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    இல்லையெனில் எனில் (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      திரும்பு gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   இல்லையெனில்
     திரும்பு 0;  /* otherwise, different variants cannot be equal */
  }
  இல்லையெனில் {  /* equal variants */
    தேர்வு (keytt(n2)) {
      நிலை LUA_VNIL: நிலை LUA_VFALSE: நிலை LUA_VTRUE:
        திரும்பு 1;
      நிலை LUA_VNUMINT:
        திரும்பு (ivalue(k1) == keyival(n2));
      நிலை LUA_VNUMFLT:
        திரும்பு luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      நிலை LUA_VLIGHTUSERDATA:
        திரும்பு pvalue(k1) == pvalueraw(keyval(n2));
      நிலை LUA_VLCF:
        திரும்பு fvalue(k1) == fvalueraw(keyval(n2));
      நிலை ctb(LUA_VLNGSTR):
        திரும்பு luaS_eqstr(tsvalue(k1), keystrval(n2));
      இயல்பு:
        திரும்பு gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
நிலையான மாறிலி TValue *getgeneric (Table *t, மாறிலி TValue *சாவி, முழு deadok) {
  முனை_2 *n = mainpositionTV(t, சாவி);
  க்கு (;;) {  /* check whether 'key' is somewhere in the chain */
    எனில் (equalkey(சாவி, n, deadok))
      திரும்பு gval(n);  /* that's it */
    இல்லையெனில் {
      முழு nx = gnext(n);
      எனில் (nx == 0)
        திரும்பு &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
நிலையான unsigned checkrange (lua_Integer k, unsigned limit) {
  திரும்பு (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
நிலையான unsigned keyinarray (Table *t, மாறிலி TValue *சாவி) {
  திரும்பு (ttisinteger(சாவி)) ? ikeyinarray(t, ivalue(சாவி)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
நிலையான unsigned findindex (lua_State *L, Table *t, TValue *சாவி,
                               unsigned asize) {
  unsigned முழு i;
  எனில் (ttisnil(சாவி)) திரும்பு 0;  /* first iteration */
  i = keyinarray(t, சாவி);
  எனில் (i != 0)  /* is 'key' inside array part? */
    திரும்பு i;  /* yes; that's the index */
  இல்லையெனில் {
    மாறிலி TValue *n = getgeneric(t, சாவி, 1);
    எனில் (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    திரும்பு (i + 1) + asize;
  }
}


முழு luaH_next (lua_State *L, Table *t, StkId சாவி) {
  unsigned முழு asize = t->asize;
  unsigned முழு i = findindex(L, t, s2v(சாவி), asize);  /* find original key */
  க்கு (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    எனில் (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(சாவி), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(சாவி + 1));
      திரும்பு 1;
    }
  }
  க்கு (i -= asize; i < sizenode(t); i++) {  /* hash part */
    எனில் (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      முனை_2 *n = gnode(t, i);
      getnodekey(L, s2v(சாவி), n);
      setobj2s(L, சாவி + 1, gval(n));
      திரும்பு 1;
    }
  }
  திரும்பு 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? அளவு(Limbox) : 0)

/* 'node' size in bytes */
நிலையான size_t sizehash (Table *t) {
  திரும்பு cast_sizet(sizenode(t)) * அளவு(முனை_2) + extraLastfree(t);
}


நிலையான வெறுமை freehash (lua_State *L, Table *t) {
  எனில் (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    எழுத்து *arr = cast_charp(t->முனை) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

நிலையான முழு insertkey (Table *t, மாறிலி TValue *சாவி, TValue *மதிப்பு);
நிலையான வெறுமை newcheckedkey (Table *t, மாறிலி TValue *சாவி, TValue *மதிப்பு);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
வகைவரை கட்டமைப்பு {
  unsigned total;
  unsigned na;
  முழு deleted;
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
நிலையான unsigned computesizes (Counters *ct) {
  முழு i;
  unsigned முழு twotoi;  /* 2^i (candidate for optimal size) */
  unsigned முழு a = 0;  /* number of elements smaller than 2^i */
  unsigned முழு na = 0;  /* number of elements to go to array part */
  unsigned முழு optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  க்கு (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    எனில் (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  திரும்பு optimal;
}


நிலையான வெறுமை countint (lua_Integer சாவி, Counters *ct) {
  unsigned முழு k = arrayindex(சாவி);
  எனில் (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline முழு arraykeyisempty (மாறிலி Table *t, unsigned சாவி) {
  முழு tag = *getArrTag(t, சாவி - 1);
  திரும்பு tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
நிலையான வெறுமை numusearray (மாறிலி Table *t, Counters *ct) {
  முழு lg;
  unsigned முழு ttlg;  /* 2^lg */
  unsigned முழு ause = 0;  /* summation of 'nums' */
  unsigned முழு i = 1;  /* index to traverse all array keys */
  unsigned முழு asize = t->asize;
  /* traverse each slice */
  க்கு (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned முழு lc = 0;  /* counter */
    unsigned முழு lim = ttlg;
    எனில் (lim > asize) {
      lim = asize;  /* adjust upper limit */
      எனில் (i > lim)
        நிறுத்து;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    க்கு (; i <= lim; i++) {
      எனில் (!arraykeyisempty(t, i))
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
நிலையான வெறுமை numusehash (மாறிலி Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  போது (i--) {
    முனை_2 *n = &t->முனை[i];
    எனில் (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    இல்லையெனில் {
      total++;
      எனில் (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
நிலையான size_t concretesize (unsigned முழு size) {
  எனில் (size == 0)
    திரும்பு 0;
  இல்லையெனில்  /* space for the two arrays plus an unsigned in between */
    திரும்பு size * (அளவு(மதிப்பு_2) + 1) + அளவு(unsigned);
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
நிலையான மதிப்பு_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  எனில் (oldasize == newasize)
    திரும்பு t->array;  /* nothing to be done */
  இல்லையெனில் எனில் (newasize == 0) {  /* erasing array? */
    மதிப்பு_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    திரும்பு வெற்று;
  }
  இல்லையெனில் {
    size_t newasizeb = concretesize(newasize);
    மதிப்பு_2 *np = cast(மதிப்பு_2 *,
                  luaM_reallocvector(L, வெற்று, 0, newasizeb, lu_byte));
    எனில் (np == வெற்று)  /* allocation error? */
      திரும்பு வெற்று;
    np += newasize;  /* shift pointer to the end of value segment */
    எனில் (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      மதிப்பு_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    திரும்பு np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
நிலையான வெறுமை setnodevector (lua_State *L, Table *t, unsigned size) {
  எனில் (size == 0) {  /* no elements to hash part? */
    t->முனை = cast(முனை_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  இல்லையெனில் {
    முழு i;
    முழு lsize = luaO_ceillog2(size);
    எனில் (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    எனில் (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->முனை = luaM_newvector(L, size, முனை_2);
    இல்லையெனில் {
      size_t bsize = size * அளவு(முனை_2) + அளவு(Limbox);
      எழுத்து *முனை = luaM_newblock(L, bsize);
      t->முனை = cast(முனை_2 *, முனை + அளவு(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    க்கு (i = 0; i < cast_int(size); i++) {
      முனை_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
நிலையான வெறுமை reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  க்கு (j = 0; j < size; j++) {
    முனை_2 *old = gnode(ot, j);
    எனில் (!isempty(gval(old))) {
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
நிலையான வெறுமை exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  முனை_2 *முனை = t1->முனை;
  முழு bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->முனை = t2->முனை;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->முனை = முனை;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
நிலையான வெறுமை reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  க்கு (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    எனில் (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue சாவி, aux;
      setivalue(&சாவி, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &சாவி, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
நிலையான வெறுமை clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  க்கு (; oldasize < newasize; oldasize++)
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
வெறுமை luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  மதிப்பு_2 *newarray;
  எனில் (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  எனில் (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  எனில் (l_unlikely(newarray == வெற்று && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  எனில் (newarray != வெற்று)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


வெறுமை luaH_resizearray (lua_State *L, Table *t, unsigned முழு nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
நிலையான வெறுமை rehash (lua_State *L, Table *t, மாறிலி TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  க்கு (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  எனில் (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  எனில் (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  இல்லையெனில் {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  எனில் (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, அளவு(Table));
  Table *t = gco2t(o);
  t->metatable = வெற்று;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = வெற்று;
  t->asize = 0;
  setnodevector(L, t, 0);
  திரும்பு t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, அளவு(Table)) + concretesize(t->asize);
  எனில் (!isdummy(t))
    sz += sizehash(t);
  திரும்பு sz;
}


/*
** Frees a table.
*/
வெறுமை luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


நிலையான முனை_2 *getfreepos (Table *t) {
  எனில் (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    போது (getlastfree(t) > t->முனை) {
      முனை_2 *விடுவி = --getlastfree(t);
      எனில் (keyisnil(விடுவி))
        திரும்பு விடுவி;
    }
  }
  இல்லையெனில் {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    போது (i--) {  /* do a linear search */
      முனை_2 *விடுவி = gnode(t, i);
      எனில் (keyisnil(விடுவி))
        திரும்பு விடுவி;
    }
  }
  திரும்பு வெற்று;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
நிலையான முழு insertkey (Table *t, மாறிலி TValue *சாவி, TValue *மதிப்பு) {
  முனை_2 *mp = mainpositionTV(t, சாவி);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, சாவி, 0)));
  எனில் (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    முனை_2 *othern;
    முனை_2 *f = getfreepos(t);  /* get a free place */
    எனில் (f == வெற்று)  /* cannot find a free place? */
      திரும்பு 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    எனில் (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      போது (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      எனில் (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    இல்லையெனில் {  /* colliding node is in its own main position */
      /* new node will go into free position */
      எனில் (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      இல்லையெனில் lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, சாவி);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), மதிப்பு);
  திரும்பு 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
நிலையான வெறுமை newcheckedkey (Table *t, மாறிலி TValue *சாவி, TValue *மதிப்பு) {
  unsigned i = keyinarray(t, சாவி);
  எனில் (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, மதிப்பு);  /* set value in the array */
  இல்லையெனில் {
    முழு done = insertkey(t, சாவி, மதிப்பு);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(வெறுமை, done);  /* to avoid warnings */
  }
}


நிலையான வெறுமை luaH_newkey (lua_State *L, Table *t, மாறிலி TValue *சாவி,
                                                 TValue *மதிப்பு) {
  எனில் (!ttisnil(மதிப்பு)) {  /* do not insert nil values */
    முழு done = insertkey(t, சாவி, மதிப்பு);
    எனில் (!done) {  /* could not find a free place? */
      rehash(L, t, சாவி);  /* grow table */
      newcheckedkey(t, சாவி, மதிப்பு);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), சாவி);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (வெறுமை)0, (வெறுமை)0, 1);
  }
}


நிலையான மாறிலி TValue *getintfromhash (Table *t, lua_Integer சாவி) {
  முனை_2 *n = hashint(t, சாவி);
  lua_assert(!ikeyinarray(t, சாவி));
  க்கு (;;) {  /* check whether 'key' is somewhere in the chain */
    எனில் (keyisinteger(n) && keyival(n) == சாவி)
      திரும்பு gval(n);  /* that's it */
    இல்லையெனில் {
      முழு nx = gnext(n);
      எனில் (nx == 0) நிறுத்து;
      n += nx;
    }
  }
  திரும்பு &absentkey;
}


நிலையான முழு hashkeyisempty (Table *t, lua_Unsigned சாவி) {
  மாறிலி TValue *val = getintfromhash(t, l_castU2S(சாவி));
  திரும்பு isempty(val);
}


நிலையான lu_byte finishnodeget (மாறிலி TValue *val, TValue *res) {
  எனில் (!ttisnil(val)) {
    setobj(((lua_State*)வெற்று), res, val);
  }
  திரும்பு ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer சாவி, TValue *res) {
  unsigned k = ikeyinarray(t, சாவி);
  எனில் (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    எனில் (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    திரும்பு tag;
  }
  இல்லையெனில்
    திரும்பு finishnodeget(getintfromhash(t, சாவி), res);
}


/*
** search function for short strings
*/
மாறிலி TValue *luaH_Hgetshortstr (Table *t, TString *சாவி) {
  முனை_2 *n = hashstr(t, சாவி);
  lua_assert(strisshr(சாவி));
  க்கு (;;) {  /* check whether 'key' is somewhere in the chain */
    எனில் (keyisshrstr(n) && eqshrstr(keystrval(n), சாவி))
      திரும்பு gval(n);  /* that's it */
    இல்லையெனில் {
      முழு nx = gnext(n);
      எனில் (nx == 0)
        திரும்பு &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *சாவி, TValue *res) {
  திரும்பு finishnodeget(luaH_Hgetshortstr(t, சாவி), res);
}


நிலையான மாறிலி TValue *Hgetlongstr (Table *t, TString *சாவி) {
  TValue ko;
  lua_assert(!strisshr(சாவி));
  setsvalue(cast(lua_State *, வெற்று), &ko, சாவி);
  திரும்பு getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


நிலையான மாறிலி TValue *Hgetstr (Table *t, TString *சாவி) {
  எனில் (strisshr(சாவி))
    திரும்பு luaH_Hgetshortstr(t, சாவி);
  இல்லையெனில்
    திரும்பு Hgetlongstr(t, சாவி);
}


lu_byte luaH_getstr (Table *t, TString *சாவி, TValue *res) {
  திரும்பு finishnodeget(Hgetstr(t, சாவி), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, மாறிலி TValue *சாவி, TValue *res) {
  மாறிலி TValue *slot;
  தேர்வு (ttypetag(சாவி)) {
    நிலை LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(சாவி));
      நிறுத்து;
    நிலை LUA_VNUMINT:
      திரும்பு luaH_getint(t, ivalue(சாவி), res);
    நிலை LUA_VNIL:
      slot = &absentkey;
      நிறுத்து;
    நிலை LUA_VNUMFLT: {
      lua_Integer k;
      எனில் (luaV_flttointeger(fltvalue(சாவி), &k, F2Ieq)) /* integral index? */
        திரும்பு luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    இயல்பு:
      slot = getgeneric(t, சாவி, 0);
      நிறுத்து;
  }
  திரும்பு finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
நிலையான முழு retpsetcode (Table *t, மாறிலி TValue *slot) {
  எனில் (isabstkey(slot))
    திரும்பு HNOTFOUND;  /* no slot with that key */
  இல்லையெனில்  /* return node encoded */
    திரும்பு cast_int((cast(முனை_2*, slot) - t->முனை)) + HFIRSTNODE;
}


நிலையான முழு finishnodeset (Table *t, மாறிலி TValue *slot, TValue *val) {
  எனில் (!ttisnil(slot)) {
    setobj(((lua_State*)வெற்று), cast(TValue*, slot), val);
    திரும்பு HOK;  /* success */
  }
  இல்லையெனில்
    திரும்பு retpsetcode(t, slot);
}


நிலையான முழு rawfinishnodeset (மாறிலி TValue *slot, TValue *val) {
  எனில் (isabstkey(slot))
    திரும்பு 0;  /* no slot with that key */
  இல்லையெனில் {
    setobj(((lua_State*)வெற்று), cast(TValue*, slot), val);
    திரும்பு 1;  /* success */
  }
}


முழு luaH_psetint (Table *t, lua_Integer சாவி, TValue *val) {
  lua_assert(!ikeyinarray(t, சாவி));
  திரும்பு finishnodeset(t, getintfromhash(t, சாவி), val);
}


நிலையான முழு psetint (Table *t, lua_Integer சாவி, TValue *val) {
  முழு hres;
  luaH_fastseti(t, சாவி, val, hres);
  திரும்பு hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

முழு luaH_psetshortstr (Table *t, TString *சாவி, TValue *val) {
  மாறிலி TValue *slot = luaH_Hgetshortstr(t, சாவி);
  எனில் (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)வெற்று), cast(TValue*, slot), val);  /* update it */
    திரும்பு HOK;  /* done */
  }
  இல்லையெனில் எனில் (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    எனில் (ttisnil(val))  /* new value is nil? */
      திரும்பு HOK;  /* done (value is already nil/absent) */
    எனில் (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(சாவி))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, வெற்று), &tk, சாவி);
      எனில் (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        திரும்பு HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  திரும்பு retpsetcode(t, slot);
}


முழு luaH_psetstr (Table *t, TString *சாவி, TValue *val) {
  எனில் (strisshr(சாவி))
    திரும்பு luaH_psetshortstr(t, சாவி, val);
  இல்லையெனில்
    திரும்பு finishnodeset(t, Hgetlongstr(t, சாவி), val);
}


முழு luaH_pset (Table *t, மாறிலி TValue *சாவி, TValue *val) {
  தேர்வு (ttypetag(சாவி)) {
    நிலை LUA_VSHRSTR: திரும்பு luaH_psetshortstr(t, tsvalue(சாவி), val);
    நிலை LUA_VNUMINT: திரும்பு psetint(t, ivalue(சாவி), val);
    நிலை LUA_VNIL: திரும்பு HNOTFOUND;
    நிலை LUA_VNUMFLT: {
      lua_Integer k;
      எனில் (luaV_flttointeger(fltvalue(சாவி), &k, F2Ieq)) /* integral index? */
        திரும்பு psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    இயல்பு:
      திரும்பு finishnodeset(t, getgeneric(t, சாவி, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
வெறுமை luaH_finishset (lua_State *L, Table *t, மாறிலி TValue *சாவி,
                                    TValue *மதிப்பு, முழு hres) {
  lua_assert(hres != HOK);
  எனில் (hres == HNOTFOUND) {
    TValue aux;
    மாறிலி TValue *actk = சாவி;  /* actual key to insert */
    எனில் (l_unlikely(ttisnil(சாவி)))
      luaG_runerror(L, "table index is nil");
    இல்லையெனில் எனில் (ttisfloat(சாவி)) {
      lua_எண் f = fltvalue(சாவி);
      lua_Integer k;
      எனில் (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      இல்லையெனில் எனில் (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    இல்லையெனில் எனில் (isextstr(சாவி)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(சாவி));
      setsvalue2s(L, L->உச்சி.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->உச்சி.p - 1), மதிப்பு);
      L->உச்சி.p--;
      திரும்பு;
    }
    luaH_newkey(L, t, actk, மதிப்பு);
  }
  இல்லையெனில் எனில் (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), மதிப்பு);
  }
  இல்லையெனில் {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), மதிப்பு);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
வெறுமை luaH_set (lua_State *L, Table *t, மாறிலி TValue *சாவி, TValue *மதிப்பு) {
  முழு hres = luaH_pset(t, சாவி, மதிப்பு);
  எனில் (hres != HOK)
    luaH_finishset(L, t, சாவி, மதிப்பு, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
வெறுமை luaH_setint (lua_State *L, Table *t, lua_Integer சாவி, TValue *மதிப்பு) {
  unsigned ik = ikeyinarray(t, சாவி);
  எனில் (ik > 0)
    obj2arr(t, ik - 1, மதிப்பு);
  இல்லையெனில் {
    முழு ok = rawfinishnodeset(getintfromhash(t, சாவி), மதிப்பு);
    எனில் (!ok) {
      TValue k;
      setivalue(&k, சாவி);
      luaH_newkey(L, t, &k, மதிப்பு);
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
நிலையான lua_Unsigned hash_தேடல் (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  முழு n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  போது (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    எனில் (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    இல்லையெனில் {
      j = LUA_MAXINTEGER;
      எனில் (hashkeyisempty(t, j))  /* t[j] not present? */
        நிறுத்து;  /* 'j' now is an absent index */
      இல்லையெனில்  /* weird case */
        திரும்பு j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  போது (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    எனில் (hashkeyisempty(t, m)) j = m;
    இல்லையெனில் i = m;
  }
  திரும்பு i;
}


நிலையான unsigned முழு binsearch (Table *array, unsigned முழு i, unsigned முழு j) {
  lua_assert(i <= j);
  போது (j - i > 1u) {  /*இருமத் தேடல்*/
    unsigned முழு m = (i + j) / 2;
    எனில் (arraykeyisempty(array, m)) j = m;
    இல்லையெனில் i = m;
  }
  திரும்பு i;
}


/* return a border, saving it as a hint for next call */
நிலையான lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  திரும்பு hint;
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
  எனில் (asize > 0) {  /* is there an array part? */
    மாறிலி unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    எனில் (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    எனில் (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      க்கு (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        எனில் (!arraykeyisempty(t, limit))
          திரும்பு newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      திரும்பு newhint(t, binsearch(t, 0, limit));
    }
    இல்லையெனில் {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      க்கு (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        எனில் (arraykeyisempty(t, limit))
          திரும்பு newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      எனில் (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        திரும்பு newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  எனில் (isdummy(t) || hashkeyisempty(t, asize + 1))
    திரும்பு asize;  /* 'asize + 1' is empty */
  இல்லையெனில்  /* 'asize + 1' is also non empty */
    திரும்பு hash_தேடல்(L, t, asize);
}



#எனில் defined(LUA_DEBUG)

/* export this function for the test library */

முனை_2 *luaH_mainposition (மாறிலி Table *t, மாறிலி TValue *சாவி) {
  திரும்பு mainpositionTV(t, சாவி);
}

#endif
