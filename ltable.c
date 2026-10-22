/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#ضمن "lprefix.h"


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

#ضمن <math.h>
#ضمن <limits.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lgc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "lvm.h"


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
تعريفنوع بنية { تۈگۈن_2 *dummy; تۈگۈن_2 follows_p_تۈگۈن; } Limbox_aux;

تعريفنوع اتحاد {
  تۈگۈن_2 *lastfree;
  حرف padding[offsetof(Limbox_aux, follows_p_تۈگۈن)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->تۈگۈن) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(صحيح) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(حجم(قىممەت_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, تۈگۈن_2)


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
ساكن ثابت تۈگۈن_2 dummynode_ = {
  {{عدم}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {عدم}}  /* key type, next, and key value */
};


ساكن ثابت TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ساكن تۈگۈن_2 *hashint (ثابت Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  إذا (ui <= cast_uint(INT_MAX))
    أرجع gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  وإلا
    أرجع hashmod(t, ui);
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
#إذا !defined(l_hashfloat)
ساكن unsigned l_hashfloat (lua_سان n) {
  صحيح i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_رەقەم(INT_MIN);
  إذا (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_رەقەم(HUGE_VAL));
    أرجع 0;
  }
  وإلا {  /* normal case */
    unsigned صحيح u = cast_uint(i) + cast_uint(ni);
    أرجع (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ساكن تۈگۈن_2 *mainpositionTV (ثابت Table *t, ثابت TValue *ئاچقۇچ) {
  بدل (ttypetag(ئاچقۇچ)) {
    حالة LUA_VNUMINT: {
      lua_Integer i = ivalue(ئاچقۇچ);
      أرجع hashint(t, i);
    }
    حالة LUA_VNUMFLT: {
      lua_سان n = fltvalue(ئاچقۇچ);
      أرجع hashmod(t, l_hashfloat(n));
    }
    حالة LUA_VSHRSTR: {
      TString *ts = tsvalue(ئاچقۇچ);
      أرجع hashstr(t, ts);
    }
    حالة LUA_VLNGSTR: {
      TString *ts = tsvalue(ئاچقۇچ);
      أرجع hashpow2(t, luaS_hashlongstr(ts));
    }
    حالة LUA_VFALSE:
      أرجع hashboolean(t, 0);
    حالة LUA_VTRUE:
      أرجع hashboolean(t, 1);
    حالة LUA_VLIGHTUSERDATA: {
      فراغ *p = pvalue(ئاچقۇچ);
      أرجع hashpointer(t, p);
    }
    حالة LUA_VLCF: {
      lua_CFunction f = fvalue(ئاچقۇچ);
      أرجع hashpointer(t, f);
    }
    افتراضي: {
      GCObject *o = gcvalue(ئاچقۇچ);
      أرجع hashpointer(t, o);
    }
  }
}


l_sinline تۈگۈن_2 *mainpositionfromnode (ثابت Table *t, تۈگۈن_2 *nd) {
  TValue ئاچقۇچ;
  getnodekey(cast(lua_State *, عدم), &ئاچقۇچ, nd);
  أرجع mainpositionTV(t, &ئاچقۇچ);
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
ساكن صحيح equalkey (ثابت TValue *k1, ثابت تۈگۈن_2 *n2, صحيح deadok) {
  إذا (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    إذا (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      أرجع luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    وإلا إذا (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      أرجع gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   وإلا
     أرجع 0;  /* otherwise, different variants cannot be equal */
  }
  وإلا {  /* equal variants */
    بدل (keytt(n2)) {
      حالة LUA_VNIL: حالة LUA_VFALSE: حالة LUA_VTRUE:
        أرجع 1;
      حالة LUA_VNUMINT:
        أرجع (ivalue(k1) == keyival(n2));
      حالة LUA_VNUMFLT:
        أرجع luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      حالة LUA_VLIGHTUSERDATA:
        أرجع pvalue(k1) == pvalueraw(keyval(n2));
      حالة LUA_VLCF:
        أرجع fvalue(k1) == fvalueraw(keyval(n2));
      حالة ctb(LUA_VLNGSTR):
        أرجع luaS_eqstr(tsvalue(k1), keystrval(n2));
      افتراضي:
        أرجع gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ساكن ثابت TValue *getgeneric (Table *t, ثابت TValue *ئاچقۇچ, صحيح deadok) {
  تۈگۈن_2 *n = mainpositionTV(t, ئاچقۇچ);
  لكل (;;) {  /* check whether 'key' is somewhere in the chain */
    إذا (equalkey(ئاچقۇچ, n, deadok))
      أرجع gval(n);  /* that's it */
    وإلا {
      صحيح nx = gnext(n);
      إذا (nx == 0)
        أرجع &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ساكن unsigned checkrange (lua_Integer k, unsigned limit) {
  أرجع (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
ساكن unsigned keyinarray (Table *t, ثابت TValue *ئاچقۇچ) {
  أرجع (ttisinteger(ئاچقۇچ)) ? ikeyinarray(t, ivalue(ئاچقۇچ)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ساكن unsigned findindex (lua_State *L, Table *t, TValue *ئاچقۇچ,
                               unsigned asize) {
  unsigned صحيح i;
  إذا (ttisnil(ئاچقۇچ)) أرجع 0;  /* first iteration */
  i = keyinarray(t, ئاچقۇچ);
  إذا (i != 0)  /* is 'key' inside array part? */
    أرجع i;  /* yes; that's the index */
  وإلا {
    ثابت TValue *n = getgeneric(t, ئاچقۇچ, 1);
    إذا (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    أرجع (i + 1) + asize;
  }
}


صحيح luaH_next (lua_State *L, Table *t, StkId ئاچقۇچ) {
  unsigned صحيح asize = t->asize;
  unsigned صحيح i = findindex(L, t, s2v(ئاچقۇچ), asize);  /* find original key */
  لكل (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    إذا (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(ئاچقۇچ), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(ئاچقۇچ + 1));
      أرجع 1;
    }
  }
  لكل (i -= asize; i < sizenode(t); i++) {  /* hash part */
    إذا (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      تۈگۈن_2 *n = gnode(t, i);
      getnodekey(L, s2v(ئاچقۇچ), n);
      setobj2s(L, ئاچقۇچ + 1, gval(n));
      أرجع 1;
    }
  }
  أرجع 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? حجم(Limbox) : 0)

/* 'node' size in bytes */
ساكن size_t sizehash (Table *t) {
  أرجع cast_sizet(sizenode(t)) * حجم(تۈگۈن_2) + extraLastfree(t);
}


ساكن فراغ freehash (lua_State *L, Table *t) {
  إذا (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    حرف *arr = cast_charp(t->تۈگۈن) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ساكن صحيح insertkey (Table *t, ثابت TValue *ئاچقۇچ, TValue *قىممەت);
ساكن فراغ newcheckedkey (Table *t, ثابت TValue *ئاچقۇچ, TValue *قىممەت);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
تعريفنوع بنية {
  unsigned total;
  unsigned na;
  صحيح deleted;
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
ساكن unsigned computesizes (Counters *ct) {
  صحيح i;
  unsigned صحيح twotoi;  /* 2^i (candidate for optimal size) */
  unsigned صحيح a = 0;  /* number of elements smaller than 2^i */
  unsigned صحيح na = 0;  /* number of elements to go to array part */
  unsigned صحيح optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  لكل (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    إذا (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  أرجع optimal;
}


ساكن فراغ countint (lua_Integer ئاچقۇچ, Counters *ct) {
  unsigned صحيح k = arrayindex(ئاچقۇچ);
  إذا (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline صحيح arraykeyisempty (ثابت Table *t, unsigned ئاچقۇچ) {
  صحيح tag = *getArrTag(t, ئاچقۇچ - 1);
  أرجع tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
ساكن فراغ numusearray (ثابت Table *t, Counters *ct) {
  صحيح lg;
  unsigned صحيح ttlg;  /* 2^lg */
  unsigned صحيح ause = 0;  /* summation of 'nums' */
  unsigned صحيح i = 1;  /* index to traverse all array keys */
  unsigned صحيح asize = t->asize;
  /* traverse each slice */
  لكل (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned صحيح lc = 0;  /* counter */
    unsigned صحيح lim = ttlg;
    إذا (lim > asize) {
      lim = asize;  /* adjust upper limit */
      إذا (i > lim)
        اكسر;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    لكل (; i <= lim; i++) {
      إذا (!arraykeyisempty(t, i))
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
ساكن فراغ numusehash (ثابت Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  طالما (i--) {
    تۈگۈن_2 *n = &t->تۈگۈن[i];
    إذا (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    وإلا {
      total++;
      إذا (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ساكن size_t concretesize (unsigned صحيح size) {
  إذا (size == 0)
    أرجع 0;
  وإلا  /* space for the two arrays plus an unsigned in between */
    أرجع size * (حجم(قىممەت_2) + 1) + حجم(unsigned);
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
ساكن قىممەت_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  إذا (oldasize == newasize)
    أرجع t->array;  /* nothing to be done */
  وإلا إذا (newasize == 0) {  /* erasing array? */
    قىممەت_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    أرجع عدم;
  }
  وإلا {
    size_t newasizeb = concretesize(newasize);
    قىممەت_2 *np = cast(قىممەت_2 *,
                  luaM_reallocvector(L, عدم, 0, newasizeb, lu_byte));
    إذا (np == عدم)  /* allocation error? */
      أرجع عدم;
    np += newasize;  /* shift pointer to the end of value segment */
    إذا (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      قىممەت_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    أرجع np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ساكن فراغ setnodevector (lua_State *L, Table *t, unsigned size) {
  إذا (size == 0) {  /* no elements to hash part? */
    t->تۈگۈن = cast(تۈگۈن_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  وإلا {
    صحيح i;
    صحيح lsize = luaO_ceillog2(size);
    إذا (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    إذا (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->تۈگۈن = luaM_newvector(L, size, تۈگۈن_2);
    وإلا {
      size_t bsize = size * حجم(تۈگۈن_2) + حجم(Limbox);
      حرف *تۈگۈن = luaM_newblock(L, bsize);
      t->تۈگۈن = cast(تۈگۈن_2 *, تۈگۈن + حجم(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    لكل (i = 0; i < cast_int(size); i++) {
      تۈگۈن_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ساكن فراغ reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  لكل (j = 0; j < size; j++) {
    تۈگۈن_2 *old = gnode(ot, j);
    إذا (!isempty(gval(old))) {
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
ساكن فراغ exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  تۈگۈن_2 *تۈگۈن = t1->تۈگۈن;
  صحيح bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->تۈگۈن = t2->تۈگۈن;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->تۈگۈن = تۈگۈن;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ساكن فراغ reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  لكل (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    إذا (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue ئاچقۇچ, aux;
      setivalue(&ئاچقۇچ, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &ئاچقۇچ, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ساكن فراغ clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  لكل (; oldasize < newasize; oldasize++)
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
فراغ luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  قىممەت_2 *newarray;
  إذا (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  إذا (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  إذا (l_unlikely(newarray == عدم && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  إذا (newarray != عدم)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


فراغ luaH_resizearray (lua_State *L, Table *t, unsigned صحيح nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ساكن فراغ rehash (lua_State *L, Table *t, ثابت TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  لكل (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  إذا (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  إذا (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  وإلا {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  إذا (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, حجم(Table));
  Table *t = gco2t(o);
  t->metatable = عدم;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = عدم;
  t->asize = 0;
  setnodevector(L, t, 0);
  أرجع t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, حجم(Table)) + concretesize(t->asize);
  إذا (!isdummy(t))
    sz += sizehash(t);
  أرجع sz;
}


/*
** Frees a table.
*/
فراغ luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ساكن تۈگۈن_2 *getfreepos (Table *t) {
  إذا (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    طالما (getlastfree(t) > t->تۈگۈن) {
      تۈگۈن_2 *حرر = --getlastfree(t);
      إذا (keyisnil(حرر))
        أرجع حرر;
    }
  }
  وإلا {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    طالما (i--) {  /* do a linear search */
      تۈگۈن_2 *حرر = gnode(t, i);
      إذا (keyisnil(حرر))
        أرجع حرر;
    }
  }
  أرجع عدم;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ساكن صحيح insertkey (Table *t, ثابت TValue *ئاچقۇچ, TValue *قىممەت) {
  تۈگۈن_2 *mp = mainpositionTV(t, ئاچقۇچ);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ئاچقۇچ, 0)));
  إذا (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    تۈگۈن_2 *othern;
    تۈگۈن_2 *f = getfreepos(t);  /* get a free place */
    إذا (f == عدم)  /* cannot find a free place? */
      أرجع 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    إذا (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      طالما (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      إذا (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    وإلا {  /* colliding node is in its own main position */
      /* new node will go into free position */
      إذا (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      وإلا lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ئاچقۇچ);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), قىممەت);
  أرجع 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ساكن فراغ newcheckedkey (Table *t, ثابت TValue *ئاچقۇچ, TValue *قىممەت) {
  unsigned i = keyinarray(t, ئاچقۇچ);
  إذا (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, قىممەت);  /* set value in the array */
  وإلا {
    صحيح done = insertkey(t, ئاچقۇچ, قىممەت);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(فراغ, done);  /* to avoid warnings */
  }
}


ساكن فراغ luaH_newkey (lua_State *L, Table *t, ثابت TValue *ئاچقۇچ,
                                                 TValue *قىممەت) {
  إذا (!ttisnil(قىممەت)) {  /* do not insert nil values */
    صحيح done = insertkey(t, ئاچقۇچ, قىممەت);
    إذا (!done) {  /* could not find a free place? */
      rehash(L, t, ئاچقۇچ);  /* grow table */
      newcheckedkey(t, ئاچقۇچ, قىممەت);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ئاچقۇچ);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (فراغ)0, (فراغ)0, 1);
  }
}


ساكن ثابت TValue *getintfromhash (Table *t, lua_Integer ئاچقۇچ) {
  تۈگۈن_2 *n = hashint(t, ئاچقۇچ);
  lua_assert(!ikeyinarray(t, ئاچقۇچ));
  لكل (;;) {  /* check whether 'key' is somewhere in the chain */
    إذا (keyisinteger(n) && keyival(n) == ئاچقۇچ)
      أرجع gval(n);  /* that's it */
    وإلا {
      صحيح nx = gnext(n);
      إذا (nx == 0) اكسر;
      n += nx;
    }
  }
  أرجع &absentkey;
}


ساكن صحيح hashkeyisempty (Table *t, lua_Unsigned ئاچقۇچ) {
  ثابت TValue *val = getintfromhash(t, l_castU2S(ئاچقۇچ));
  أرجع isempty(val);
}


ساكن lu_byte finishnodeget (ثابت TValue *val, TValue *res) {
  إذا (!ttisnil(val)) {
    setobj(((lua_State*)عدم), res, val);
  }
  أرجع ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer ئاچقۇچ, TValue *res) {
  unsigned k = ikeyinarray(t, ئاچقۇچ);
  إذا (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    إذا (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    أرجع tag;
  }
  وإلا
    أرجع finishnodeget(getintfromhash(t, ئاچقۇچ), res);
}


/*
** search function for short strings
*/
ثابت TValue *luaH_Hgetshortstr (Table *t, TString *ئاچقۇچ) {
  تۈگۈن_2 *n = hashstr(t, ئاچقۇچ);
  lua_assert(strisshr(ئاچقۇچ));
  لكل (;;) {  /* check whether 'key' is somewhere in the chain */
    إذا (keyisshrstr(n) && eqshrstr(keystrval(n), ئاچقۇچ))
      أرجع gval(n);  /* that's it */
    وإلا {
      صحيح nx = gnext(n);
      إذا (nx == 0)
        أرجع &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *ئاچقۇچ, TValue *res) {
  أرجع finishnodeget(luaH_Hgetshortstr(t, ئاچقۇچ), res);
}


ساكن ثابت TValue *Hgetlongstr (Table *t, TString *ئاچقۇچ) {
  TValue ko;
  lua_assert(!strisshr(ئاچقۇچ));
  setsvalue(cast(lua_State *, عدم), &ko, ئاچقۇچ);
  أرجع getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ساكن ثابت TValue *Hgetstr (Table *t, TString *ئاچقۇچ) {
  إذا (strisshr(ئاچقۇچ))
    أرجع luaH_Hgetshortstr(t, ئاچقۇچ);
  وإلا
    أرجع Hgetlongstr(t, ئاچقۇچ);
}


lu_byte luaH_getstr (Table *t, TString *ئاچقۇچ, TValue *res) {
  أرجع finishnodeget(Hgetstr(t, ئاچقۇچ), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, ثابت TValue *ئاچقۇچ, TValue *res) {
  ثابت TValue *slot;
  بدل (ttypetag(ئاچقۇچ)) {
    حالة LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(ئاچقۇچ));
      اكسر;
    حالة LUA_VNUMINT:
      أرجع luaH_getint(t, ivalue(ئاچقۇچ), res);
    حالة LUA_VNIL:
      slot = &absentkey;
      اكسر;
    حالة LUA_VNUMFLT: {
      lua_Integer k;
      إذا (luaV_flttointeger(fltvalue(ئاچقۇچ), &k, F2Ieq)) /* integral index? */
        أرجع luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    افتراضي:
      slot = getgeneric(t, ئاچقۇچ, 0);
      اكسر;
  }
  أرجع finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ساكن صحيح retpsetcode (Table *t, ثابت TValue *slot) {
  إذا (isabstkey(slot))
    أرجع HNOTFOUND;  /* no slot with that key */
  وإلا  /* return node encoded */
    أرجع cast_int((cast(تۈگۈن_2*, slot) - t->تۈگۈن)) + HFIRSTNODE;
}


ساكن صحيح finishnodeset (Table *t, ثابت TValue *slot, TValue *val) {
  إذا (!ttisnil(slot)) {
    setobj(((lua_State*)عدم), cast(TValue*, slot), val);
    أرجع HOK;  /* success */
  }
  وإلا
    أرجع retpsetcode(t, slot);
}


ساكن صحيح rawfinishnodeset (ثابت TValue *slot, TValue *val) {
  إذا (isabstkey(slot))
    أرجع 0;  /* no slot with that key */
  وإلا {
    setobj(((lua_State*)عدم), cast(TValue*, slot), val);
    أرجع 1;  /* success */
  }
}


صحيح luaH_psetint (Table *t, lua_Integer ئاچقۇچ, TValue *val) {
  lua_assert(!ikeyinarray(t, ئاچقۇچ));
  أرجع finishnodeset(t, getintfromhash(t, ئاچقۇچ), val);
}


ساكن صحيح psetint (Table *t, lua_Integer ئاچقۇچ, TValue *val) {
  صحيح hres;
  luaH_fastseti(t, ئاچقۇچ, val, hres);
  أرجع hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

صحيح luaH_psetshortstr (Table *t, TString *ئاچقۇچ, TValue *val) {
  ثابت TValue *slot = luaH_Hgetshortstr(t, ئاچقۇچ);
  إذا (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)عدم), cast(TValue*, slot), val);  /* update it */
    أرجع HOK;  /* done */
  }
  وإلا إذا (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    إذا (ttisnil(val))  /* new value is nil? */
      أرجع HOK;  /* done (value is already nil/absent) */
    إذا (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(ئاچقۇچ))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, عدم), &tk, ئاچقۇچ);
      إذا (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        أرجع HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  أرجع retpsetcode(t, slot);
}


صحيح luaH_psetstr (Table *t, TString *ئاچقۇچ, TValue *val) {
  إذا (strisshr(ئاچقۇچ))
    أرجع luaH_psetshortstr(t, ئاچقۇچ, val);
  وإلا
    أرجع finishnodeset(t, Hgetlongstr(t, ئاچقۇچ), val);
}


صحيح luaH_pset (Table *t, ثابت TValue *ئاچقۇچ, TValue *val) {
  بدل (ttypetag(ئاچقۇچ)) {
    حالة LUA_VSHRSTR: أرجع luaH_psetshortstr(t, tsvalue(ئاچقۇچ), val);
    حالة LUA_VNUMINT: أرجع psetint(t, ivalue(ئاچقۇچ), val);
    حالة LUA_VNIL: أرجع HNOTFOUND;
    حالة LUA_VNUMFLT: {
      lua_Integer k;
      إذا (luaV_flttointeger(fltvalue(ئاچقۇچ), &k, F2Ieq)) /* integral index? */
        أرجع psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    افتراضي:
      أرجع finishnodeset(t, getgeneric(t, ئاچقۇچ, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
فراغ luaH_finishset (lua_State *L, Table *t, ثابت TValue *ئاچقۇچ,
                                    TValue *قىممەت, صحيح hres) {
  lua_assert(hres != HOK);
  إذا (hres == HNOTFOUND) {
    TValue aux;
    ثابت TValue *actk = ئاچقۇچ;  /* actual key to insert */
    إذا (l_unlikely(ttisnil(ئاچقۇچ)))
      luaG_runerror(L, "table index is nil");
    وإلا إذا (ttisfloat(ئاچقۇچ)) {
      lua_سان f = fltvalue(ئاچقۇچ);
      lua_Integer k;
      إذا (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      وإلا إذا (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    وإلا إذا (isextstr(ئاچقۇچ)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ئاچقۇچ));
      setsvalue2s(L, L->چوققا.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->چوققا.p - 1), قىممەت);
      L->چوققا.p--;
      أرجع;
    }
    luaH_newkey(L, t, actk, قىممەت);
  }
  وإلا إذا (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), قىممەت);
  }
  وإلا {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), قىممەت);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
فراغ luaH_set (lua_State *L, Table *t, ثابت TValue *ئاچقۇچ, TValue *قىممەت) {
  صحيح hres = luaH_pset(t, ئاچقۇچ, قىممەت);
  إذا (hres != HOK)
    luaH_finishset(L, t, ئاچقۇچ, قىممەت, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
فراغ luaH_setint (lua_State *L, Table *t, lua_Integer ئاچقۇچ, TValue *قىممەت) {
  unsigned ik = ikeyinarray(t, ئاچقۇچ);
  إذا (ik > 0)
    obj2arr(t, ik - 1, قىممەت);
  وإلا {
    صحيح ok = rawfinishnodeset(getintfromhash(t, ئاچقۇچ), قىممەت);
    إذا (!ok) {
      TValue k;
      setivalue(&k, ئاچقۇچ);
      luaH_newkey(L, t, &k, قىممەت);
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
ساكن lua_Unsigned hash_ئىزدەش (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  صحيح n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  طالما (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    إذا (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    وإلا {
      j = LUA_MAXINTEGER;
      إذا (hashkeyisempty(t, j))  /* t[j] not present? */
        اكسر;  /* 'j' now is an absent index */
      وإلا  /* weird case */
        أرجع j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  طالما (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    إذا (hashkeyisempty(t, m)) j = m;
    وإلا i = m;
  }
  أرجع i;
}


ساكن unsigned صحيح binsearch (Table *array, unsigned صحيح i, unsigned صحيح j) {
  lua_assert(i <= j);
  طالما (j - i > 1u) {  /*ئىككىلىك ئىزدەش*/
    unsigned صحيح m = (i + j) / 2;
    إذا (arraykeyisempty(array, m)) j = m;
    وإلا i = m;
  }
  أرجع i;
}


/* return a border, saving it as a hint for next call */
ساكن lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  أرجع hint;
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
  إذا (asize > 0) {  /* is there an array part? */
    ثابت unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    إذا (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    إذا (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      لكل (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        إذا (!arraykeyisempty(t, limit))
          أرجع newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      أرجع newhint(t, binsearch(t, 0, limit));
    }
    وإلا {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      لكل (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        إذا (arraykeyisempty(t, limit))
          أرجع newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      إذا (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        أرجع newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  إذا (isdummy(t) || hashkeyisempty(t, asize + 1))
    أرجع asize;  /* 'asize + 1' is empty */
  وإلا  /* 'asize + 1' is also non empty */
    أرجع hash_ئىزدەش(L, t, asize);
}



#إذا defined(LUA_DEBUG)

/* export this function for the test library */

تۈگۈن_2 *luaH_mainposition (ثابت Table *t, ثابت TValue *ئاچقۇچ) {
  أرجع mainpositionTV(t, ئاچقۇچ);
}

#endif
