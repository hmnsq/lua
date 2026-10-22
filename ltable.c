/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#شامل "lprefix.h"


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

#شامل <math.h>
#شامل <limits.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lvm.h"


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
تعریفنوع ساختار { گره_2 *dummy; گره_2 follows_p_گره; } Limbox_aux;

تعریفنوع اجتماع {
  گره_2 *lastfree;
  نویسه padding[offsetof(Limbox_aux, follows_p_گره)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->گره) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(صحیح) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(اندازه(مقدار_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, گره_2)


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
ایستا ثابت گره_2 dummynode_ = {
  {{پوچ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {پوچ}}  /* key type, next, and key value */
};


ایستا ثابت TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ایستا گره_2 *hashint (ثابت Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  اگر (ui <= cast_uint(INT_MAX))
    بازگشت gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  وگرنه
    بازگشت hashmod(t, ui);
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
#اگر !defined(l_hashfloat)
ایستا unsigned l_hashfloat (lua_عدد n) {
  صحیح i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_رقم(INT_MIN);
  اگر (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_رقم(HUGE_VAL));
    بازگشت 0;
  }
  وگرنه {  /* normal case */
    unsigned صحیح u = cast_uint(i) + cast_uint(ni);
    بازگشت (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ایستا گره_2 *mainpositionTV (ثابت Table *t, ثابت TValue *کلید) {
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VNUMINT: {
      lua_Integer i = ivalue(کلید);
      بازگشت hashint(t, i);
    }
    حالت LUA_VNUMFLT: {
      lua_عدد n = fltvalue(کلید);
      بازگشت hashmod(t, l_hashfloat(n));
    }
    حالت LUA_VSHRSTR: {
      TString *ts = tsvalue(کلید);
      بازگشت hashstr(t, ts);
    }
    حالت LUA_VLNGSTR: {
      TString *ts = tsvalue(کلید);
      بازگشت hashpow2(t, luaS_hashlongstr(ts));
    }
    حالت LUA_VFALSE:
      بازگشت hashboolean(t, 0);
    حالت LUA_VTRUE:
      بازگشت hashboolean(t, 1);
    حالت LUA_VLIGHTUSERDATA: {
      تهی *p = pvalue(کلید);
      بازگشت hashpointer(t, p);
    }
    حالت LUA_VLCF: {
      lua_CFunction f = fvalue(کلید);
      بازگشت hashpointer(t, f);
    }
    پیشفرض: {
      GCObject *o = gcvalue(کلید);
      بازگشت hashpointer(t, o);
    }
  }
}


l_sinline گره_2 *mainpositionfromnode (ثابت Table *t, گره_2 *nd) {
  TValue کلید;
  getnodekey(cast(lua_State *, پوچ), &کلید, nd);
  بازگشت mainpositionTV(t, &کلید);
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
ایستا صحیح equalkey (ثابت TValue *k1, ثابت گره_2 *n2, صحیح deadok) {
  اگر (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    اگر (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      بازگشت luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    وگرنه اگر (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      بازگشت gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   وگرنه
     بازگشت 0;  /* otherwise, different variants cannot be equal */
  }
  وگرنه {  /* equal variants */
    انتخاب (keytt(n2)) {
      حالت LUA_VNIL: حالت LUA_VFALSE: حالت LUA_VTRUE:
        بازگشت 1;
      حالت LUA_VNUMINT:
        بازگشت (ivalue(k1) == keyival(n2));
      حالت LUA_VNUMFLT:
        بازگشت luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      حالت LUA_VLIGHTUSERDATA:
        بازگشت pvalue(k1) == pvalueraw(keyval(n2));
      حالت LUA_VLCF:
        بازگشت fvalue(k1) == fvalueraw(keyval(n2));
      حالت ctb(LUA_VLNGSTR):
        بازگشت luaS_eqstr(tsvalue(k1), keystrval(n2));
      پیشفرض:
        بازگشت gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ایستا ثابت TValue *getgeneric (Table *t, ثابت TValue *کلید, صحیح deadok) {
  گره_2 *n = mainpositionTV(t, کلید);
  برای (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (equalkey(کلید, n, deadok))
      بازگشت gval(n);  /* that's it */
    وگرنه {
      صحیح nx = gnext(n);
      اگر (nx == 0)
        بازگشت &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ایستا unsigned checkrange (lua_Integer k, unsigned limit) {
  بازگشت (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
ایستا unsigned keyinarray (Table *t, ثابت TValue *کلید) {
  بازگشت (ttisinteger(کلید)) ? ikeyinarray(t, ivalue(کلید)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ایستا unsigned findindex (lua_State *L, Table *t, TValue *کلید,
                               unsigned asize) {
  unsigned صحیح i;
  اگر (ttisnil(کلید)) بازگشت 0;  /* first iteration */
  i = keyinarray(t, کلید);
  اگر (i != 0)  /* is 'key' inside array part? */
    بازگشت i;  /* yes; that's the index */
  وگرنه {
    ثابت TValue *n = getgeneric(t, کلید, 1);
    اگر (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    بازگشت (i + 1) + asize;
  }
}


صحیح luaH_next (lua_State *L, Table *t, StkId کلید) {
  unsigned صحیح asize = t->asize;
  unsigned صحیح i = findindex(L, t, s2v(کلید), asize);  /* find original key */
  برای (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    اگر (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(کلید), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(کلید + 1));
      بازگشت 1;
    }
  }
  برای (i -= asize; i < sizenode(t); i++) {  /* hash part */
    اگر (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      گره_2 *n = gnode(t, i);
      getnodekey(L, s2v(کلید), n);
      setobj2s(L, کلید + 1, gval(n));
      بازگشت 1;
    }
  }
  بازگشت 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? اندازه(Limbox) : 0)

/* 'node' size in bytes */
ایستا size_t sizehash (Table *t) {
  بازگشت cast_sizet(sizenode(t)) * اندازه(گره_2) + extraLastfree(t);
}


ایستا تهی freehash (lua_State *L, Table *t) {
  اگر (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    نویسه *arr = cast_charp(t->گره) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ایستا صحیح insertkey (Table *t, ثابت TValue *کلید, TValue *مقدار);
ایستا تهی newcheckedkey (Table *t, ثابت TValue *کلید, TValue *مقدار);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
تعریفنوع ساختار {
  unsigned total;
  unsigned na;
  صحیح deleted;
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
ایستا unsigned computesizes (Counters *ct) {
  صحیح i;
  unsigned صحیح twotoi;  /* 2^i (candidate for optimal size) */
  unsigned صحیح a = 0;  /* number of elements smaller than 2^i */
  unsigned صحیح na = 0;  /* number of elements to go to array part */
  unsigned صحیح optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  برای (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    اگر (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  بازگشت optimal;
}


ایستا تهی countint (lua_Integer کلید, Counters *ct) {
  unsigned صحیح k = arrayindex(کلید);
  اگر (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline صحیح arraykeyisempty (ثابت Table *t, unsigned کلید) {
  صحیح tag = *getArrTag(t, کلید - 1);
  بازگشت tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
ایستا تهی numusearray (ثابت Table *t, Counters *ct) {
  صحیح lg;
  unsigned صحیح ttlg;  /* 2^lg */
  unsigned صحیح ause = 0;  /* summation of 'nums' */
  unsigned صحیح i = 1;  /* index to traverse all array keys */
  unsigned صحیح asize = t->asize;
  /* traverse each slice */
  برای (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned صحیح lc = 0;  /* counter */
    unsigned صحیح lim = ttlg;
    اگر (lim > asize) {
      lim = asize;  /* adjust upper limit */
      اگر (i > lim)
        بشکن;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    برای (; i <= lim; i++) {
      اگر (!arraykeyisempty(t, i))
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
ایستا تهی numusehash (ثابت Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  وقتی (i--) {
    گره_2 *n = &t->گره[i];
    اگر (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    وگرنه {
      total++;
      اگر (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ایستا size_t concretesize (unsigned صحیح size) {
  اگر (size == 0)
    بازگشت 0;
  وگرنه  /* space for the two arrays plus an unsigned in between */
    بازگشت size * (اندازه(مقدار_2) + 1) + اندازه(unsigned);
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
ایستا مقدار_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  اگر (oldasize == newasize)
    بازگشت t->array;  /* nothing to be done */
  وگرنه اگر (newasize == 0) {  /* erasing array? */
    مقدار_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    بازگشت پوچ;
  }
  وگرنه {
    size_t newasizeb = concretesize(newasize);
    مقدار_2 *np = cast(مقدار_2 *,
                  luaM_reallocvector(L, پوچ, 0, newasizeb, lu_byte));
    اگر (np == پوچ)  /* allocation error? */
      بازگشت پوچ;
    np += newasize;  /* shift pointer to the end of value segment */
    اگر (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      مقدار_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    بازگشت np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ایستا تهی setnodevector (lua_State *L, Table *t, unsigned size) {
  اگر (size == 0) {  /* no elements to hash part? */
    t->گره = cast(گره_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  وگرنه {
    صحیح i;
    صحیح lsize = luaO_ceillog2(size);
    اگر (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    اگر (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->گره = luaM_newvector(L, size, گره_2);
    وگرنه {
      size_t bsize = size * اندازه(گره_2) + اندازه(Limbox);
      نویسه *گره = luaM_newblock(L, bsize);
      t->گره = cast(گره_2 *, گره + اندازه(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    برای (i = 0; i < cast_int(size); i++) {
      گره_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ایستا تهی reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  برای (j = 0; j < size; j++) {
    گره_2 *old = gnode(ot, j);
    اگر (!isempty(gval(old))) {
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
ایستا تهی exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  گره_2 *گره = t1->گره;
  صحیح bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->گره = t2->گره;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->گره = گره;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ایستا تهی reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  برای (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    اگر (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue کلید, aux;
      setivalue(&کلید, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &کلید, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ایستا تهی clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  برای (; oldasize < newasize; oldasize++)
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
تهی luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  مقدار_2 *newarray;
  اگر (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  اگر (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  اگر (l_unlikely(newarray == پوچ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  اگر (newarray != پوچ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


تهی luaH_resizearray (lua_State *L, Table *t, unsigned صحیح nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ایستا تهی rehash (lua_State *L, Table *t, ثابت TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  برای (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  اگر (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  اگر (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  وگرنه {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  اگر (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, اندازه(Table));
  Table *t = gco2t(o);
  t->metatable = پوچ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = پوچ;
  t->asize = 0;
  setnodevector(L, t, 0);
  بازگشت t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, اندازه(Table)) + concretesize(t->asize);
  اگر (!isdummy(t))
    sz += sizehash(t);
  بازگشت sz;
}


/*
** Frees a table.
*/
تهی luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ایستا گره_2 *getfreepos (Table *t) {
  اگر (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    وقتی (getlastfree(t) > t->گره) {
      گره_2 *آزاد = --getlastfree(t);
      اگر (keyisnil(آزاد))
        بازگشت آزاد;
    }
  }
  وگرنه {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    وقتی (i--) {  /* do a linear search */
      گره_2 *آزاد = gnode(t, i);
      اگر (keyisnil(آزاد))
        بازگشت آزاد;
    }
  }
  بازگشت پوچ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ایستا صحیح insertkey (Table *t, ثابت TValue *کلید, TValue *مقدار) {
  گره_2 *mp = mainpositionTV(t, کلید);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, کلید, 0)));
  اگر (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    گره_2 *othern;
    گره_2 *f = getfreepos(t);  /* get a free place */
    اگر (f == پوچ)  /* cannot find a free place? */
      بازگشت 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    اگر (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      وقتی (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      اگر (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    وگرنه {  /* colliding node is in its own main position */
      /* new node will go into free position */
      اگر (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      وگرنه lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, کلید);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), مقدار);
  بازگشت 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ایستا تهی newcheckedkey (Table *t, ثابت TValue *کلید, TValue *مقدار) {
  unsigned i = keyinarray(t, کلید);
  اگر (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, مقدار);  /* set value in the array */
  وگرنه {
    صحیح done = insertkey(t, کلید, مقدار);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(تهی, done);  /* to avoid warnings */
  }
}


ایستا تهی luaH_newkey (lua_State *L, Table *t, ثابت TValue *کلید,
                                                 TValue *مقدار) {
  اگر (!ttisnil(مقدار)) {  /* do not insert nil values */
    صحیح done = insertkey(t, کلید, مقدار);
    اگر (!done) {  /* could not find a free place? */
      rehash(L, t, کلید);  /* grow table */
      newcheckedkey(t, کلید, مقدار);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), کلید);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (تهی)0, (تهی)0, 1);
  }
}


ایستا ثابت TValue *getintfromhash (Table *t, lua_Integer کلید) {
  گره_2 *n = hashint(t, کلید);
  lua_assert(!ikeyinarray(t, کلید));
  برای (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (keyisinteger(n) && keyival(n) == کلید)
      بازگشت gval(n);  /* that's it */
    وگرنه {
      صحیح nx = gnext(n);
      اگر (nx == 0) بشکن;
      n += nx;
    }
  }
  بازگشت &absentkey;
}


ایستا صحیح hashkeyisempty (Table *t, lua_Unsigned کلید) {
  ثابت TValue *val = getintfromhash(t, l_castU2S(کلید));
  بازگشت isempty(val);
}


ایستا lu_byte finishnodeget (ثابت TValue *val, TValue *res) {
  اگر (!ttisnil(val)) {
    setobj(((lua_State*)پوچ), res, val);
  }
  بازگشت ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer کلید, TValue *res) {
  unsigned k = ikeyinarray(t, کلید);
  اگر (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    اگر (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    بازگشت tag;
  }
  وگرنه
    بازگشت finishnodeget(getintfromhash(t, کلید), res);
}


/*
** search function for short strings
*/
ثابت TValue *luaH_Hgetshortstr (Table *t, TString *کلید) {
  گره_2 *n = hashstr(t, کلید);
  lua_assert(strisshr(کلید));
  برای (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (keyisshrstr(n) && eqshrstr(keystrval(n), کلید))
      بازگشت gval(n);  /* that's it */
    وگرنه {
      صحیح nx = gnext(n);
      اگر (nx == 0)
        بازگشت &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *کلید, TValue *res) {
  بازگشت finishnodeget(luaH_Hgetshortstr(t, کلید), res);
}


ایستا ثابت TValue *Hgetlongstr (Table *t, TString *کلید) {
  TValue ko;
  lua_assert(!strisshr(کلید));
  setsvalue(cast(lua_State *, پوچ), &ko, کلید);
  بازگشت getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ایستا ثابت TValue *Hgetstr (Table *t, TString *کلید) {
  اگر (strisshr(کلید))
    بازگشت luaH_Hgetshortstr(t, کلید);
  وگرنه
    بازگشت Hgetlongstr(t, کلید);
}


lu_byte luaH_getstr (Table *t, TString *کلید, TValue *res) {
  بازگشت finishnodeget(Hgetstr(t, کلید), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, ثابت TValue *کلید, TValue *res) {
  ثابت TValue *slot;
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(کلید));
      بشکن;
    حالت LUA_VNUMINT:
      بازگشت luaH_getint(t, ivalue(کلید), res);
    حالت LUA_VNIL:
      slot = &absentkey;
      بشکن;
    حالت LUA_VNUMFLT: {
      lua_Integer k;
      اگر (luaV_flttointeger(fltvalue(کلید), &k, F2Ieq)) /* integral index? */
        بازگشت luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    پیشفرض:
      slot = getgeneric(t, کلید, 0);
      بشکن;
  }
  بازگشت finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ایستا صحیح retpsetcode (Table *t, ثابت TValue *slot) {
  اگر (isabstkey(slot))
    بازگشت HNOTFOUND;  /* no slot with that key */
  وگرنه  /* return node encoded */
    بازگشت cast_int((cast(گره_2*, slot) - t->گره)) + HFIRSTNODE;
}


ایستا صحیح finishnodeset (Table *t, ثابت TValue *slot, TValue *val) {
  اگر (!ttisnil(slot)) {
    setobj(((lua_State*)پوچ), cast(TValue*, slot), val);
    بازگشت HOK;  /* success */
  }
  وگرنه
    بازگشت retpsetcode(t, slot);
}


ایستا صحیح rawfinishnodeset (ثابت TValue *slot, TValue *val) {
  اگر (isabstkey(slot))
    بازگشت 0;  /* no slot with that key */
  وگرنه {
    setobj(((lua_State*)پوچ), cast(TValue*, slot), val);
    بازگشت 1;  /* success */
  }
}


صحیح luaH_psetint (Table *t, lua_Integer کلید, TValue *val) {
  lua_assert(!ikeyinarray(t, کلید));
  بازگشت finishnodeset(t, getintfromhash(t, کلید), val);
}


ایستا صحیح psetint (Table *t, lua_Integer کلید, TValue *val) {
  صحیح hres;
  luaH_fastseti(t, کلید, val, hres);
  بازگشت hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

صحیح luaH_psetshortstr (Table *t, TString *کلید, TValue *val) {
  ثابت TValue *slot = luaH_Hgetshortstr(t, کلید);
  اگر (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)پوچ), cast(TValue*, slot), val);  /* update it */
    بازگشت HOK;  /* done */
  }
  وگرنه اگر (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    اگر (ttisnil(val))  /* new value is nil? */
      بازگشت HOK;  /* done (value is already nil/absent) */
    اگر (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(کلید))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, پوچ), &tk, کلید);
      اگر (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        بازگشت HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  بازگشت retpsetcode(t, slot);
}


صحیح luaH_psetstr (Table *t, TString *کلید, TValue *val) {
  اگر (strisshr(کلید))
    بازگشت luaH_psetshortstr(t, کلید, val);
  وگرنه
    بازگشت finishnodeset(t, Hgetlongstr(t, کلید), val);
}


صحیح luaH_pset (Table *t, ثابت TValue *کلید, TValue *val) {
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VSHRSTR: بازگشت luaH_psetshortstr(t, tsvalue(کلید), val);
    حالت LUA_VNUMINT: بازگشت psetint(t, ivalue(کلید), val);
    حالت LUA_VNIL: بازگشت HNOTFOUND;
    حالت LUA_VNUMFLT: {
      lua_Integer k;
      اگر (luaV_flttointeger(fltvalue(کلید), &k, F2Ieq)) /* integral index? */
        بازگشت psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    پیشفرض:
      بازگشت finishnodeset(t, getgeneric(t, کلید, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
تهی luaH_finishset (lua_State *L, Table *t, ثابت TValue *کلید,
                                    TValue *مقدار, صحیح hres) {
  lua_assert(hres != HOK);
  اگر (hres == HNOTFOUND) {
    TValue aux;
    ثابت TValue *actk = کلید;  /* actual key to insert */
    اگر (l_unlikely(ttisnil(کلید)))
      luaG_runerror(L, "table index is nil");
    وگرنه اگر (ttisfloat(کلید)) {
      lua_عدد f = fltvalue(کلید);
      lua_Integer k;
      اگر (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      وگرنه اگر (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    وگرنه اگر (isextstr(کلید)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(کلید));
      setsvalue2s(L, L->بالاترین.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->بالاترین.p - 1), مقدار);
      L->بالاترین.p--;
      بازگشت;
    }
    luaH_newkey(L, t, actk, مقدار);
  }
  وگرنه اگر (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), مقدار);
  }
  وگرنه {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), مقدار);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
تهی luaH_set (lua_State *L, Table *t, ثابت TValue *کلید, TValue *مقدار) {
  صحیح hres = luaH_pset(t, کلید, مقدار);
  اگر (hres != HOK)
    luaH_finishset(L, t, کلید, مقدار, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
تهی luaH_setint (lua_State *L, Table *t, lua_Integer کلید, TValue *مقدار) {
  unsigned ik = ikeyinarray(t, کلید);
  اگر (ik > 0)
    obj2arr(t, ik - 1, مقدار);
  وگرنه {
    صحیح ok = rawfinishnodeset(getintfromhash(t, کلید), مقدار);
    اگر (!ok) {
      TValue k;
      setivalue(&k, کلید);
      luaH_newkey(L, t, &k, مقدار);
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
ایستا lua_Unsigned hash_جستجو (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  صحیح n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  وقتی (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    اگر (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    وگرنه {
      j = LUA_MAXINTEGER;
      اگر (hashkeyisempty(t, j))  /* t[j] not present? */
        بشکن;  /* 'j' now is an absent index */
      وگرنه  /* weird case */
        بازگشت j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  وقتی (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    اگر (hashkeyisempty(t, m)) j = m;
    وگرنه i = m;
  }
  بازگشت i;
}


ایستا unsigned صحیح binsearch (Table *array, unsigned صحیح i, unsigned صحیح j) {
  lua_assert(i <= j);
  وقتی (j - i > 1u) {  /*جستجوی دودویی*/
    unsigned صحیح m = (i + j) / 2;
    اگر (arraykeyisempty(array, m)) j = m;
    وگرنه i = m;
  }
  بازگشت i;
}


/* return a border, saving it as a hint for next call */
ایستا lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  بازگشت hint;
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
  اگر (asize > 0) {  /* is there an array part? */
    ثابت unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    اگر (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    اگر (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      برای (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        اگر (!arraykeyisempty(t, limit))
          بازگشت newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      بازگشت newhint(t, binsearch(t, 0, limit));
    }
    وگرنه {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      برای (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        اگر (arraykeyisempty(t, limit))
          بازگشت newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      اگر (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        بازگشت newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  اگر (isdummy(t) || hashkeyisempty(t, asize + 1))
    بازگشت asize;  /* 'asize + 1' is empty */
  وگرنه  /* 'asize + 1' is also non empty */
    بازگشت hash_جستجو(L, t, asize);
}



#اگر defined(LUA_DEBUG)

/* export this function for the test library */

گره_2 *luaH_mainposition (ثابت Table *t, ثابت TValue *کلید) {
  بازگشت mainpositionTV(t, کلید);
}

#endif
