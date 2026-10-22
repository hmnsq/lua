/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


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

#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lvm.h"


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
પ્રકારવ્યાખ્યા સંરચના { ગાંઠ_2 *dummy; ગાંઠ_2 follows_p_ગાંઠ; } Limbox_aux;

પ્રકારવ્યાખ્યા સંઘ {
  ગાંઠ_2 *lastfree;
  અક્ષર padding[offsetof(Limbox_aux, follows_p_ગાંઠ)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->ગાંઠ) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(પૂર્ણાંક) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(કદ(કિંમત_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, ગાંઠ_2)


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
સ્થિર અચલ ગાંઠ_2 dummynode_ = {
  {{ખાલી}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ખાલી}}  /* key type, next, and key value */
};


સ્થિર અચલ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
સ્થિર ગાંઠ_2 *hashint (અચલ Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  જો (ui <= cast_uint(INT_MAX))
    પાછા gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  નહીંતર
    પાછા hashmod(t, ui);
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
#જો !defined(l_hashfloat)
સ્થિર unsigned l_hashfloat (lua_સંખ્યા n) {
  પૂર્ણાંક i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_અંક(INT_MIN);
  જો (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_અંક(HUGE_VAL));
    પાછા 0;
  }
  નહીંતર {  /* normal case */
    unsigned પૂર્ણાંક u = cast_uint(i) + cast_uint(ni);
    પાછા (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
સ્થિર ગાંઠ_2 *mainpositionTV (અચલ Table *t, અચલ TValue *ચાવી) {
  પસંદગી (ttypetag(ચાવી)) {
    સ્થિતિ LUA_VNUMINT: {
      lua_Integer i = ivalue(ચાવી);
      પાછા hashint(t, i);
    }
    સ્થિતિ LUA_VNUMFLT: {
      lua_સંખ્યા n = fltvalue(ચાવી);
      પાછા hashmod(t, l_hashfloat(n));
    }
    સ્થિતિ LUA_VSHRSTR: {
      TString *ts = tsvalue(ચાવી);
      પાછા hashstr(t, ts);
    }
    સ્થિતિ LUA_VLNGSTR: {
      TString *ts = tsvalue(ચાવી);
      પાછા hashpow2(t, luaS_hashlongstr(ts));
    }
    સ્થિતિ LUA_VFALSE:
      પાછા hashboolean(t, 0);
    સ્થિતિ LUA_VTRUE:
      પાછા hashboolean(t, 1);
    સ્થિતિ LUA_VLIGHTUSERDATA: {
      શૂન્ય *p = pvalue(ચાવી);
      પાછા hashpointer(t, p);
    }
    સ્થિતિ LUA_VLCF: {
      lua_CFunction f = fvalue(ચાવી);
      પાછા hashpointer(t, f);
    }
    મૂળભૂત: {
      GCObject *o = gcvalue(ચાવી);
      પાછા hashpointer(t, o);
    }
  }
}


l_sinline ગાંઠ_2 *mainpositionfromnode (અચલ Table *t, ગાંઠ_2 *nd) {
  TValue ચાવી;
  getnodekey(cast(lua_State *, ખાલી), &ચાવી, nd);
  પાછા mainpositionTV(t, &ચાવી);
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
સ્થિર પૂર્ણાંક equalkey (અચલ TValue *k1, અચલ ગાંઠ_2 *n2, પૂર્ણાંક deadok) {
  જો (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    જો (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      પાછા luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    નહીંતર જો (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      પાછા gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   નહીંતર
     પાછા 0;  /* otherwise, different variants cannot be equal */
  }
  નહીંતર {  /* equal variants */
    પસંદગી (keytt(n2)) {
      સ્થિતિ LUA_VNIL: સ્થિતિ LUA_VFALSE: સ્થિતિ LUA_VTRUE:
        પાછા 1;
      સ્થિતિ LUA_VNUMINT:
        પાછા (ivalue(k1) == keyival(n2));
      સ્થિતિ LUA_VNUMFLT:
        પાછા luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      સ્થિતિ LUA_VLIGHTUSERDATA:
        પાછા pvalue(k1) == pvalueraw(keyval(n2));
      સ્થિતિ LUA_VLCF:
        પાછા fvalue(k1) == fvalueraw(keyval(n2));
      સ્થિતિ ctb(LUA_VLNGSTR):
        પાછા luaS_eqstr(tsvalue(k1), keystrval(n2));
      મૂળભૂત:
        પાછા gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
સ્થિર અચલ TValue *getgeneric (Table *t, અચલ TValue *ચાવી, પૂર્ણાંક deadok) {
  ગાંઠ_2 *n = mainpositionTV(t, ચાવી);
  માટે (;;) {  /* check whether 'key' is somewhere in the chain */
    જો (equalkey(ચાવી, n, deadok))
      પાછા gval(n);  /* that's it */
    નહીંતર {
      પૂર્ણાંક nx = gnext(n);
      જો (nx == 0)
        પાછા &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
સ્થિર unsigned checkrange (lua_Integer k, unsigned limit) {
  પાછા (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
સ્થિર unsigned keyinarray (Table *t, અચલ TValue *ચાવી) {
  પાછા (ttisinteger(ચાવી)) ? ikeyinarray(t, ivalue(ચાવી)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
સ્થિર unsigned findindex (lua_State *L, Table *t, TValue *ચાવી,
                               unsigned asize) {
  unsigned પૂર્ણાંક i;
  જો (ttisnil(ચાવી)) પાછા 0;  /* first iteration */
  i = keyinarray(t, ચાવી);
  જો (i != 0)  /* is 'key' inside array part? */
    પાછા i;  /* yes; that's the index */
  નહીંતર {
    અચલ TValue *n = getgeneric(t, ચાવી, 1);
    જો (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    પાછા (i + 1) + asize;
  }
}


પૂર્ણાંક luaH_next (lua_State *L, Table *t, StkId ચાવી) {
  unsigned પૂર્ણાંક asize = t->asize;
  unsigned પૂર્ણાંક i = findindex(L, t, s2v(ચાવી), asize);  /* find original key */
  માટે (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    જો (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(ચાવી), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(ચાવી + 1));
      પાછા 1;
    }
  }
  માટે (i -= asize; i < sizenode(t); i++) {  /* hash part */
    જો (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ગાંઠ_2 *n = gnode(t, i);
      getnodekey(L, s2v(ચાવી), n);
      setobj2s(L, ચાવી + 1, gval(n));
      પાછા 1;
    }
  }
  પાછા 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? કદ(Limbox) : 0)

/* 'node' size in bytes */
સ્થિર size_t sizehash (Table *t) {
  પાછા cast_sizet(sizenode(t)) * કદ(ગાંઠ_2) + extraLastfree(t);
}


સ્થિર શૂન્ય freehash (lua_State *L, Table *t) {
  જો (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    અક્ષર *arr = cast_charp(t->ગાંઠ) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

સ્થિર પૂર્ણાંક insertkey (Table *t, અચલ TValue *ચાવી, TValue *કિંમત);
સ્થિર શૂન્ય newcheckedkey (Table *t, અચલ TValue *ચાવી, TValue *કિંમત);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
પ્રકારવ્યાખ્યા સંરચના {
  unsigned total;
  unsigned na;
  પૂર્ણાંક deleted;
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
સ્થિર unsigned computesizes (Counters *ct) {
  પૂર્ણાંક i;
  unsigned પૂર્ણાંક twotoi;  /* 2^i (candidate for optimal size) */
  unsigned પૂર્ણાંક a = 0;  /* number of elements smaller than 2^i */
  unsigned પૂર્ણાંક na = 0;  /* number of elements to go to array part */
  unsigned પૂર્ણાંક optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  માટે (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    જો (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  પાછા optimal;
}


સ્થિર શૂન્ય countint (lua_Integer ચાવી, Counters *ct) {
  unsigned પૂર્ણાંક k = arrayindex(ચાવી);
  જો (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline પૂર્ણાંક arraykeyisempty (અચલ Table *t, unsigned ચાવી) {
  પૂર્ણાંક tag = *getArrTag(t, ચાવી - 1);
  પાછા tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
સ્થિર શૂન્ય numusearray (અચલ Table *t, Counters *ct) {
  પૂર્ણાંક lg;
  unsigned પૂર્ણાંક ttlg;  /* 2^lg */
  unsigned પૂર્ણાંક ause = 0;  /* summation of 'nums' */
  unsigned પૂર્ણાંક i = 1;  /* index to traverse all array keys */
  unsigned પૂર્ણાંક asize = t->asize;
  /* traverse each slice */
  માટે (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned પૂર્ણાંક lc = 0;  /* counter */
    unsigned પૂર્ણાંક lim = ttlg;
    જો (lim > asize) {
      lim = asize;  /* adjust upper limit */
      જો (i > lim)
        તોડો;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    માટે (; i <= lim; i++) {
      જો (!arraykeyisempty(t, i))
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
સ્થિર શૂન્ય numusehash (અચલ Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  જ્યારે (i--) {
    ગાંઠ_2 *n = &t->ગાંઠ[i];
    જો (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    નહીંતર {
      total++;
      જો (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
સ્થિર size_t concretesize (unsigned પૂર્ણાંક size) {
  જો (size == 0)
    પાછા 0;
  નહીંતર  /* space for the two arrays plus an unsigned in between */
    પાછા size * (કદ(કિંમત_2) + 1) + કદ(unsigned);
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
સ્થિર કિંમત_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  જો (oldasize == newasize)
    પાછા t->array;  /* nothing to be done */
  નહીંતર જો (newasize == 0) {  /* erasing array? */
    કિંમત_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    પાછા ખાલી;
  }
  નહીંતર {
    size_t newasizeb = concretesize(newasize);
    કિંમત_2 *np = cast(કિંમત_2 *,
                  luaM_reallocvector(L, ખાલી, 0, newasizeb, lu_byte));
    જો (np == ખાલી)  /* allocation error? */
      પાછા ખાલી;
    np += newasize;  /* shift pointer to the end of value segment */
    જો (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      કિંમત_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    પાછા np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
સ્થિર શૂન્ય setnodevector (lua_State *L, Table *t, unsigned size) {
  જો (size == 0) {  /* no elements to hash part? */
    t->ગાંઠ = cast(ગાંઠ_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  નહીંતર {
    પૂર્ણાંક i;
    પૂર્ણાંક lsize = luaO_ceillog2(size);
    જો (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    જો (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ગાંઠ = luaM_newvector(L, size, ગાંઠ_2);
    નહીંતર {
      size_t bsize = size * કદ(ગાંઠ_2) + કદ(Limbox);
      અક્ષર *ગાંઠ = luaM_newblock(L, bsize);
      t->ગાંઠ = cast(ગાંઠ_2 *, ગાંઠ + કદ(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    માટે (i = 0; i < cast_int(size); i++) {
      ગાંઠ_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
સ્થિર શૂન્ય reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  માટે (j = 0; j < size; j++) {
    ગાંઠ_2 *old = gnode(ot, j);
    જો (!isempty(gval(old))) {
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
સ્થિર શૂન્ય exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ગાંઠ_2 *ગાંઠ = t1->ગાંઠ;
  પૂર્ણાંક bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->ગાંઠ = t2->ગાંઠ;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->ગાંઠ = ગાંઠ;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
સ્થિર શૂન્ય reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  માટે (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    જો (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue ચાવી, aux;
      setivalue(&ચાવી, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &ચાવી, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
સ્થિર શૂન્ય clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  માટે (; oldasize < newasize; oldasize++)
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
શૂન્ય luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  કિંમત_2 *newarray;
  જો (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  જો (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  જો (l_unlikely(newarray == ખાલી && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  જો (newarray != ખાલી)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


શૂન્ય luaH_resizearray (lua_State *L, Table *t, unsigned પૂર્ણાંક nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
સ્થિર શૂન્ય rehash (lua_State *L, Table *t, અચલ TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  માટે (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  જો (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  જો (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  નહીંતર {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  જો (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, કદ(Table));
  Table *t = gco2t(o);
  t->metatable = ખાલી;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ખાલી;
  t->asize = 0;
  setnodevector(L, t, 0);
  પાછા t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, કદ(Table)) + concretesize(t->asize);
  જો (!isdummy(t))
    sz += sizehash(t);
  પાછા sz;
}


/*
** Frees a table.
*/
શૂન્ય luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


સ્થિર ગાંઠ_2 *getfreepos (Table *t) {
  જો (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    જ્યારે (getlastfree(t) > t->ગાંઠ) {
      ગાંઠ_2 *મુક્ત = --getlastfree(t);
      જો (keyisnil(મુક્ત))
        પાછા મુક્ત;
    }
  }
  નહીંતર {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    જ્યારે (i--) {  /* do a linear search */
      ગાંઠ_2 *મુક્ત = gnode(t, i);
      જો (keyisnil(મુક્ત))
        પાછા મુક્ત;
    }
  }
  પાછા ખાલી;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
સ્થિર પૂર્ણાંક insertkey (Table *t, અચલ TValue *ચાવી, TValue *કિંમત) {
  ગાંઠ_2 *mp = mainpositionTV(t, ચાવી);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ચાવી, 0)));
  જો (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ગાંઠ_2 *othern;
    ગાંઠ_2 *f = getfreepos(t);  /* get a free place */
    જો (f == ખાલી)  /* cannot find a free place? */
      પાછા 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    જો (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      જ્યારે (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      જો (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    નહીંતર {  /* colliding node is in its own main position */
      /* new node will go into free position */
      જો (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      નહીંતર lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ચાવી);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), કિંમત);
  પાછા 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
સ્થિર શૂન્ય newcheckedkey (Table *t, અચલ TValue *ચાવી, TValue *કિંમત) {
  unsigned i = keyinarray(t, ચાવી);
  જો (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, કિંમત);  /* set value in the array */
  નહીંતર {
    પૂર્ણાંક done = insertkey(t, ચાવી, કિંમત);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(શૂન્ય, done);  /* to avoid warnings */
  }
}


સ્થિર શૂન્ય luaH_newkey (lua_State *L, Table *t, અચલ TValue *ચાવી,
                                                 TValue *કિંમત) {
  જો (!ttisnil(કિંમત)) {  /* do not insert nil values */
    પૂર્ણાંક done = insertkey(t, ચાવી, કિંમત);
    જો (!done) {  /* could not find a free place? */
      rehash(L, t, ચાવી);  /* grow table */
      newcheckedkey(t, ચાવી, કિંમત);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ચાવી);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (શૂન્ય)0, (શૂન્ય)0, 1);
  }
}


સ્થિર અચલ TValue *getintfromhash (Table *t, lua_Integer ચાવી) {
  ગાંઠ_2 *n = hashint(t, ચાવી);
  lua_assert(!ikeyinarray(t, ચાવી));
  માટે (;;) {  /* check whether 'key' is somewhere in the chain */
    જો (keyisinteger(n) && keyival(n) == ચાવી)
      પાછા gval(n);  /* that's it */
    નહીંતર {
      પૂર્ણાંક nx = gnext(n);
      જો (nx == 0) તોડો;
      n += nx;
    }
  }
  પાછા &absentkey;
}


સ્થિર પૂર્ણાંક hashkeyisempty (Table *t, lua_Unsigned ચાવી) {
  અચલ TValue *val = getintfromhash(t, l_castU2S(ચાવી));
  પાછા isempty(val);
}


સ્થિર lu_byte finishnodeget (અચલ TValue *val, TValue *res) {
  જો (!ttisnil(val)) {
    setobj(((lua_State*)ખાલી), res, val);
  }
  પાછા ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer ચાવી, TValue *res) {
  unsigned k = ikeyinarray(t, ચાવી);
  જો (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    જો (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    પાછા tag;
  }
  નહીંતર
    પાછા finishnodeget(getintfromhash(t, ચાવી), res);
}


/*
** search function for short strings
*/
અચલ TValue *luaH_Hgetshortstr (Table *t, TString *ચાવી) {
  ગાંઠ_2 *n = hashstr(t, ચાવી);
  lua_assert(strisshr(ચાવી));
  માટે (;;) {  /* check whether 'key' is somewhere in the chain */
    જો (keyisshrstr(n) && eqshrstr(keystrval(n), ચાવી))
      પાછા gval(n);  /* that's it */
    નહીંતર {
      પૂર્ણાંક nx = gnext(n);
      જો (nx == 0)
        પાછા &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *ચાવી, TValue *res) {
  પાછા finishnodeget(luaH_Hgetshortstr(t, ચાવી), res);
}


સ્થિર અચલ TValue *Hgetlongstr (Table *t, TString *ચાવી) {
  TValue ko;
  lua_assert(!strisshr(ચાવી));
  setsvalue(cast(lua_State *, ખાલી), &ko, ચાવી);
  પાછા getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


સ્થિર અચલ TValue *Hgetstr (Table *t, TString *ચાવી) {
  જો (strisshr(ચાવી))
    પાછા luaH_Hgetshortstr(t, ચાવી);
  નહીંતર
    પાછા Hgetlongstr(t, ચાવી);
}


lu_byte luaH_getstr (Table *t, TString *ચાવી, TValue *res) {
  પાછા finishnodeget(Hgetstr(t, ચાવી), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, અચલ TValue *ચાવી, TValue *res) {
  અચલ TValue *slot;
  પસંદગી (ttypetag(ચાવી)) {
    સ્થિતિ LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(ચાવી));
      તોડો;
    સ્થિતિ LUA_VNUMINT:
      પાછા luaH_getint(t, ivalue(ચાવી), res);
    સ્થિતિ LUA_VNIL:
      slot = &absentkey;
      તોડો;
    સ્થિતિ LUA_VNUMFLT: {
      lua_Integer k;
      જો (luaV_flttointeger(fltvalue(ચાવી), &k, F2Ieq)) /* integral index? */
        પાછા luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    મૂળભૂત:
      slot = getgeneric(t, ચાવી, 0);
      તોડો;
  }
  પાછા finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
સ્થિર પૂર્ણાંક retpsetcode (Table *t, અચલ TValue *slot) {
  જો (isabstkey(slot))
    પાછા HNOTFOUND;  /* no slot with that key */
  નહીંતર  /* return node encoded */
    પાછા cast_int((cast(ગાંઠ_2*, slot) - t->ગાંઠ)) + HFIRSTNODE;
}


સ્થિર પૂર્ણાંક finishnodeset (Table *t, અચલ TValue *slot, TValue *val) {
  જો (!ttisnil(slot)) {
    setobj(((lua_State*)ખાલી), cast(TValue*, slot), val);
    પાછા HOK;  /* success */
  }
  નહીંતર
    પાછા retpsetcode(t, slot);
}


સ્થિર પૂર્ણાંક rawfinishnodeset (અચલ TValue *slot, TValue *val) {
  જો (isabstkey(slot))
    પાછા 0;  /* no slot with that key */
  નહીંતર {
    setobj(((lua_State*)ખાલી), cast(TValue*, slot), val);
    પાછા 1;  /* success */
  }
}


પૂર્ણાંક luaH_psetint (Table *t, lua_Integer ચાવી, TValue *val) {
  lua_assert(!ikeyinarray(t, ચાવી));
  પાછા finishnodeset(t, getintfromhash(t, ચાવી), val);
}


સ્થિર પૂર્ણાંક psetint (Table *t, lua_Integer ચાવી, TValue *val) {
  પૂર્ણાંક hres;
  luaH_fastseti(t, ચાવી, val, hres);
  પાછા hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

પૂર્ણાંક luaH_psetshortstr (Table *t, TString *ચાવી, TValue *val) {
  અચલ TValue *slot = luaH_Hgetshortstr(t, ચાવી);
  જો (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ખાલી), cast(TValue*, slot), val);  /* update it */
    પાછા HOK;  /* done */
  }
  નહીંતર જો (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    જો (ttisnil(val))  /* new value is nil? */
      પાછા HOK;  /* done (value is already nil/absent) */
    જો (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(ચાવી))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ખાલી), &tk, ચાવી);
      જો (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        પાછા HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  પાછા retpsetcode(t, slot);
}


પૂર્ણાંક luaH_psetstr (Table *t, TString *ચાવી, TValue *val) {
  જો (strisshr(ચાવી))
    પાછા luaH_psetshortstr(t, ચાવી, val);
  નહીંતર
    પાછા finishnodeset(t, Hgetlongstr(t, ચાવી), val);
}


પૂર્ણાંક luaH_pset (Table *t, અચલ TValue *ચાવી, TValue *val) {
  પસંદગી (ttypetag(ચાવી)) {
    સ્થિતિ LUA_VSHRSTR: પાછા luaH_psetshortstr(t, tsvalue(ચાવી), val);
    સ્થિતિ LUA_VNUMINT: પાછા psetint(t, ivalue(ચાવી), val);
    સ્થિતિ LUA_VNIL: પાછા HNOTFOUND;
    સ્થિતિ LUA_VNUMFLT: {
      lua_Integer k;
      જો (luaV_flttointeger(fltvalue(ચાવી), &k, F2Ieq)) /* integral index? */
        પાછા psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    મૂળભૂત:
      પાછા finishnodeset(t, getgeneric(t, ચાવી, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
શૂન્ય luaH_finishset (lua_State *L, Table *t, અચલ TValue *ચાવી,
                                    TValue *કિંમત, પૂર્ણાંક hres) {
  lua_assert(hres != HOK);
  જો (hres == HNOTFOUND) {
    TValue aux;
    અચલ TValue *actk = ચાવી;  /* actual key to insert */
    જો (l_unlikely(ttisnil(ચાવી)))
      luaG_runerror(L, "table index is nil");
    નહીંતર જો (ttisfloat(ચાવી)) {
      lua_સંખ્યા f = fltvalue(ચાવી);
      lua_Integer k;
      જો (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      નહીંતર જો (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    નહીંતર જો (isextstr(ચાવી)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ચાવી));
      setsvalue2s(L, L->ટોચ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->ટોચ.p - 1), કિંમત);
      L->ટોચ.p--;
      પાછા;
    }
    luaH_newkey(L, t, actk, કિંમત);
  }
  નહીંતર જો (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), કિંમત);
  }
  નહીંતર {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), કિંમત);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
શૂન્ય luaH_set (lua_State *L, Table *t, અચલ TValue *ચાવી, TValue *કિંમત) {
  પૂર્ણાંક hres = luaH_pset(t, ચાવી, કિંમત);
  જો (hres != HOK)
    luaH_finishset(L, t, ચાવી, કિંમત, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
શૂન્ય luaH_setint (lua_State *L, Table *t, lua_Integer ચાવી, TValue *કિંમત) {
  unsigned ik = ikeyinarray(t, ચાવી);
  જો (ik > 0)
    obj2arr(t, ik - 1, કિંમત);
  નહીંતર {
    પૂર્ણાંક ok = rawfinishnodeset(getintfromhash(t, ચાવી), કિંમત);
    જો (!ok) {
      TValue k;
      setivalue(&k, ચાવી);
      luaH_newkey(L, t, &k, કિંમત);
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
સ્થિર lua_Unsigned hash_શોધ (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  પૂર્ણાંક n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  જ્યારે (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    જો (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    નહીંતર {
      j = LUA_MAXINTEGER;
      જો (hashkeyisempty(t, j))  /* t[j] not present? */
        તોડો;  /* 'j' now is an absent index */
      નહીંતર  /* weird case */
        પાછા j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  જ્યારે (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    જો (hashkeyisempty(t, m)) j = m;
    નહીંતર i = m;
  }
  પાછા i;
}


સ્થિર unsigned પૂર્ણાંક binsearch (Table *array, unsigned પૂર્ણાંક i, unsigned પૂર્ણાંક j) {
  lua_assert(i <= j);
  જ્યારે (j - i > 1u) {  /*દ્વિઅંકી શોધ*/
    unsigned પૂર્ણાંક m = (i + j) / 2;
    જો (arraykeyisempty(array, m)) j = m;
    નહીંતર i = m;
  }
  પાછા i;
}


/* return a border, saving it as a hint for next call */
સ્થિર lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  પાછા hint;
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
  જો (asize > 0) {  /* is there an array part? */
    અચલ unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    જો (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    જો (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      માટે (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        જો (!arraykeyisempty(t, limit))
          પાછા newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      પાછા newhint(t, binsearch(t, 0, limit));
    }
    નહીંતર {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      માટે (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        જો (arraykeyisempty(t, limit))
          પાછા newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      જો (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        પાછા newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  જો (isdummy(t) || hashkeyisempty(t, asize + 1))
    પાછા asize;  /* 'asize + 1' is empty */
  નહીંતર  /* 'asize + 1' is also non empty */
    પાછા hash_શોધ(L, t, asize);
}



#જો defined(LUA_DEBUG)

/* export this function for the test library */

ગાંઠ_2 *luaH_mainposition (અચલ Table *t, અચલ TValue *ચાવી) {
  પાછા mainpositionTV(t, ચાવી);
}

#endif
