/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


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

#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lvm.h"


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
തരനിർവചനം ഘടന { കണ്ണി_2 *dummy; കണ്ണി_2 follows_p_കണ്ണി; } Limbox_aux;

തരനിർവചനം സംഘം {
  കണ്ണി_2 *lastfree;
  അക്ഷരം padding[offsetof(Limbox_aux, follows_p_കണ്ണി)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->കണ്ണി) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(പൂർണ്ണസംഖ്യ) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(വലുപ്പം(മൂല്യം_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, കണ്ണി_2)


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
സ്ഥിതം സ്ഥിരം കണ്ണി_2 dummynode_ = {
  {{ശൂന്യ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ശൂന്യ}}  /* key type, next, and key value */
};


സ്ഥിതം സ്ഥിരം TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
സ്ഥിതം കണ്ണി_2 *hashint (സ്ഥിരം Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  എങ്കിൽ (ui <= cast_uint(INT_MAX))
    തിരികെ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  അല്ലെങ്കിൽ
    തിരികെ hashmod(t, ui);
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
#എങ്കിൽ !defined(l_hashfloat)
സ്ഥിതം unsigned l_hashfloat (lua_സംഖ്യ n) {
  പൂർണ്ണസംഖ്യ i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_അക്കം(INT_MIN);
  എങ്കിൽ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_അക്കം(HUGE_VAL));
    തിരികെ 0;
  }
  അല്ലെങ്കിൽ {  /* normal case */
    unsigned പൂർണ്ണസംഖ്യ u = cast_uint(i) + cast_uint(ni);
    തിരികെ (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
സ്ഥിതം കണ്ണി_2 *mainpositionTV (സ്ഥിരം Table *t, സ്ഥിരം TValue *താക്കോൽ) {
  തിരഞ്ഞെടുപ്പ് (ttypetag(താക്കോൽ)) {
    അവസ്ഥ LUA_VNUMINT: {
      lua_Integer i = ivalue(താക്കോൽ);
      തിരികെ hashint(t, i);
    }
    അവസ്ഥ LUA_VNUMFLT: {
      lua_സംഖ്യ n = fltvalue(താക്കോൽ);
      തിരികെ hashmod(t, l_hashfloat(n));
    }
    അവസ്ഥ LUA_VSHRSTR: {
      TString *ts = tsvalue(താക്കോൽ);
      തിരികെ hashstr(t, ts);
    }
    അവസ്ഥ LUA_VLNGSTR: {
      TString *ts = tsvalue(താക്കോൽ);
      തിരികെ hashpow2(t, luaS_hashlongstr(ts));
    }
    അവസ്ഥ LUA_VFALSE:
      തിരികെ hashboolean(t, 0);
    അവസ്ഥ LUA_VTRUE:
      തിരികെ hashboolean(t, 1);
    അവസ്ഥ LUA_VLIGHTUSERDATA: {
      ശൂന്യം *p = pvalue(താക്കോൽ);
      തിരികെ hashpointer(t, p);
    }
    അവസ്ഥ LUA_VLCF: {
      lua_CFunction f = fvalue(താക്കോൽ);
      തിരികെ hashpointer(t, f);
    }
    സ്ഥിരസ്ഥിതി: {
      GCObject *o = gcvalue(താക്കോൽ);
      തിരികെ hashpointer(t, o);
    }
  }
}


l_sinline കണ്ണി_2 *mainpositionfromnode (സ്ഥിരം Table *t, കണ്ണി_2 *nd) {
  TValue താക്കോൽ;
  getnodekey(cast(lua_State *, ശൂന്യ), &താക്കോൽ, nd);
  തിരികെ mainpositionTV(t, &താക്കോൽ);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ equalkey (സ്ഥിരം TValue *k1, സ്ഥിരം കണ്ണി_2 *n2, പൂർണ്ണസംഖ്യ deadok) {
  എങ്കിൽ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    എങ്കിൽ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      തിരികെ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    അല്ലെങ്കിൽ എങ്കിൽ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      തിരികെ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   അല്ലെങ്കിൽ
     തിരികെ 0;  /* otherwise, different variants cannot be equal */
  }
  അല്ലെങ്കിൽ {  /* equal variants */
    തിരഞ്ഞെടുപ്പ് (keytt(n2)) {
      അവസ്ഥ LUA_VNIL: അവസ്ഥ LUA_VFALSE: അവസ്ഥ LUA_VTRUE:
        തിരികെ 1;
      അവസ്ഥ LUA_VNUMINT:
        തിരികെ (ivalue(k1) == keyival(n2));
      അവസ്ഥ LUA_VNUMFLT:
        തിരികെ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      അവസ്ഥ LUA_VLIGHTUSERDATA:
        തിരികെ pvalue(k1) == pvalueraw(keyval(n2));
      അവസ്ഥ LUA_VLCF:
        തിരികെ fvalue(k1) == fvalueraw(keyval(n2));
      അവസ്ഥ ctb(LUA_VLNGSTR):
        തിരികെ luaS_eqstr(tsvalue(k1), keystrval(n2));
      സ്ഥിരസ്ഥിതി:
        തിരികെ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
സ്ഥിതം സ്ഥിരം TValue *getgeneric (Table *t, സ്ഥിരം TValue *താക്കോൽ, പൂർണ്ണസംഖ്യ deadok) {
  കണ്ണി_2 *n = mainpositionTV(t, താക്കോൽ);
  വേണ്ടി (;;) {  /* check whether 'key' is somewhere in the chain */
    എങ്കിൽ (equalkey(താക്കോൽ, n, deadok))
      തിരികെ gval(n);  /* that's it */
    അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ nx = gnext(n);
      എങ്കിൽ (nx == 0)
        തിരികെ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
സ്ഥിതം unsigned checkrange (lua_Integer k, unsigned limit) {
  തിരികെ (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
സ്ഥിതം unsigned keyinarray (Table *t, സ്ഥിരം TValue *താക്കോൽ) {
  തിരികെ (ttisinteger(താക്കോൽ)) ? ikeyinarray(t, ivalue(താക്കോൽ)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
സ്ഥിതം unsigned findindex (lua_State *L, Table *t, TValue *താക്കോൽ,
                               unsigned asize) {
  unsigned പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ (ttisnil(താക്കോൽ)) തിരികെ 0;  /* first iteration */
  i = keyinarray(t, താക്കോൽ);
  എങ്കിൽ (i != 0)  /* is 'key' inside array part? */
    തിരികെ i;  /* yes; that's the index */
  അല്ലെങ്കിൽ {
    സ്ഥിരം TValue *n = getgeneric(t, താക്കോൽ, 1);
    എങ്കിൽ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    തിരികെ (i + 1) + asize;
  }
}


പൂർണ്ണസംഖ്യ luaH_next (lua_State *L, Table *t, StkId താക്കോൽ) {
  unsigned പൂർണ്ണസംഖ്യ asize = t->asize;
  unsigned പൂർണ്ണസംഖ്യ i = findindex(L, t, s2v(താക്കോൽ), asize);  /* find original key */
  വേണ്ടി (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    എങ്കിൽ (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(താക്കോൽ), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(താക്കോൽ + 1));
      തിരികെ 1;
    }
  }
  വേണ്ടി (i -= asize; i < sizenode(t); i++) {  /* hash part */
    എങ്കിൽ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      കണ്ണി_2 *n = gnode(t, i);
      getnodekey(L, s2v(താക്കോൽ), n);
      setobj2s(L, താക്കോൽ + 1, gval(n));
      തിരികെ 1;
    }
  }
  തിരികെ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? വലുപ്പം(Limbox) : 0)

/* 'node' size in bytes */
സ്ഥിതം size_t sizehash (Table *t) {
  തിരികെ cast_sizet(sizenode(t)) * വലുപ്പം(കണ്ണി_2) + extraLastfree(t);
}


സ്ഥിതം ശൂന്യം freehash (lua_State *L, Table *t) {
  എങ്കിൽ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    അക്ഷരം *arr = cast_charp(t->കണ്ണി) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

സ്ഥിതം പൂർണ്ണസംഖ്യ insertkey (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *മൂല്യം);
സ്ഥിതം ശൂന്യം newcheckedkey (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *മൂല്യം);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
തരനിർവചനം ഘടന {
  unsigned total;
  unsigned na;
  പൂർണ്ണസംഖ്യ deleted;
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
സ്ഥിതം unsigned computesizes (Counters *ct) {
  പൂർണ്ണസംഖ്യ i;
  unsigned പൂർണ്ണസംഖ്യ twotoi;  /* 2^i (candidate for optimal size) */
  unsigned പൂർണ്ണസംഖ്യ a = 0;  /* number of elements smaller than 2^i */
  unsigned പൂർണ്ണസംഖ്യ na = 0;  /* number of elements to go to array part */
  unsigned പൂർണ്ണസംഖ്യ optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  വേണ്ടി (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    എങ്കിൽ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  തിരികെ optimal;
}


സ്ഥിതം ശൂന്യം countint (lua_Integer താക്കോൽ, Counters *ct) {
  unsigned പൂർണ്ണസംഖ്യ k = arrayindex(താക്കോൽ);
  എങ്കിൽ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline പൂർണ്ണസംഖ്യ arraykeyisempty (സ്ഥിരം Table *t, unsigned താക്കോൽ) {
  പൂർണ്ണസംഖ്യ tag = *getArrTag(t, താക്കോൽ - 1);
  തിരികെ tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
സ്ഥിതം ശൂന്യം numusearray (സ്ഥിരം Table *t, Counters *ct) {
  പൂർണ്ണസംഖ്യ lg;
  unsigned പൂർണ്ണസംഖ്യ ttlg;  /* 2^lg */
  unsigned പൂർണ്ണസംഖ്യ ause = 0;  /* summation of 'nums' */
  unsigned പൂർണ്ണസംഖ്യ i = 1;  /* index to traverse all array keys */
  unsigned പൂർണ്ണസംഖ്യ asize = t->asize;
  /* traverse each slice */
  വേണ്ടി (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned പൂർണ്ണസംഖ്യ lc = 0;  /* counter */
    unsigned പൂർണ്ണസംഖ്യ lim = ttlg;
    എങ്കിൽ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      എങ്കിൽ (i > lim)
        നിർത്തുക;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    വേണ്ടി (; i <= lim; i++) {
      എങ്കിൽ (!arraykeyisempty(t, i))
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
സ്ഥിതം ശൂന്യം numusehash (സ്ഥിരം Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  ആകുമ്പോൾ (i--) {
    കണ്ണി_2 *n = &t->കണ്ണി[i];
    എങ്കിൽ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    അല്ലെങ്കിൽ {
      total++;
      എങ്കിൽ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
സ്ഥിതം size_t concretesize (unsigned പൂർണ്ണസംഖ്യ size) {
  എങ്കിൽ (size == 0)
    തിരികെ 0;
  അല്ലെങ്കിൽ  /* space for the two arrays plus an unsigned in between */
    തിരികെ size * (വലുപ്പം(മൂല്യം_2) + 1) + വലുപ്പം(unsigned);
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
സ്ഥിതം മൂല്യം_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  എങ്കിൽ (oldasize == newasize)
    തിരികെ t->array;  /* nothing to be done */
  അല്ലെങ്കിൽ എങ്കിൽ (newasize == 0) {  /* erasing array? */
    മൂല്യം_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    തിരികെ ശൂന്യ;
  }
  അല്ലെങ്കിൽ {
    size_t newasizeb = concretesize(newasize);
    മൂല്യം_2 *np = cast(മൂല്യം_2 *,
                  luaM_reallocvector(L, ശൂന്യ, 0, newasizeb, lu_byte));
    എങ്കിൽ (np == ശൂന്യ)  /* allocation error? */
      തിരികെ ശൂന്യ;
    np += newasize;  /* shift pointer to the end of value segment */
    എങ്കിൽ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      മൂല്യം_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    തിരികെ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
സ്ഥിതം ശൂന്യം setnodevector (lua_State *L, Table *t, unsigned size) {
  എങ്കിൽ (size == 0) {  /* no elements to hash part? */
    t->കണ്ണി = cast(കണ്ണി_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ lsize = luaO_ceillog2(size);
    എങ്കിൽ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    എങ്കിൽ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->കണ്ണി = luaM_newvector(L, size, കണ്ണി_2);
    അല്ലെങ്കിൽ {
      size_t bsize = size * വലുപ്പം(കണ്ണി_2) + വലുപ്പം(Limbox);
      അക്ഷരം *കണ്ണി = luaM_newblock(L, bsize);
      t->കണ്ണി = cast(കണ്ണി_2 *, കണ്ണി + വലുപ്പം(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    വേണ്ടി (i = 0; i < cast_int(size); i++) {
      കണ്ണി_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
സ്ഥിതം ശൂന്യം reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  വേണ്ടി (j = 0; j < size; j++) {
    കണ്ണി_2 *old = gnode(ot, j);
    എങ്കിൽ (!isempty(gval(old))) {
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
സ്ഥിതം ശൂന്യം exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  കണ്ണി_2 *കണ്ണി = t1->കണ്ണി;
  പൂർണ്ണസംഖ്യ bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->കണ്ണി = t2->കണ്ണി;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->കണ്ണി = കണ്ണി;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
സ്ഥിതം ശൂന്യം reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  വേണ്ടി (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    എങ്കിൽ (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue താക്കോൽ, aux;
      setivalue(&താക്കോൽ, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &താക്കോൽ, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
സ്ഥിതം ശൂന്യം clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  വേണ്ടി (; oldasize < newasize; oldasize++)
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
ശൂന്യം luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  മൂല്യം_2 *newarray;
  എങ്കിൽ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  എങ്കിൽ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  എങ്കിൽ (l_unlikely(newarray == ശൂന്യ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  എങ്കിൽ (newarray != ശൂന്യ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


ശൂന്യം luaH_resizearray (lua_State *L, Table *t, unsigned പൂർണ്ണസംഖ്യ nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
സ്ഥിതം ശൂന്യം rehash (lua_State *L, Table *t, സ്ഥിരം TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  വേണ്ടി (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  എങ്കിൽ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  എങ്കിൽ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  അല്ലെങ്കിൽ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  എങ്കിൽ (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, വലുപ്പം(Table));
  Table *t = gco2t(o);
  t->metatable = ശൂന്യ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ശൂന്യ;
  t->asize = 0;
  setnodevector(L, t, 0);
  തിരികെ t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, വലുപ്പം(Table)) + concretesize(t->asize);
  എങ്കിൽ (!isdummy(t))
    sz += sizehash(t);
  തിരികെ sz;
}


/*
** Frees a table.
*/
ശൂന്യം luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


സ്ഥിതം കണ്ണി_2 *getfreepos (Table *t) {
  എങ്കിൽ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ആകുമ്പോൾ (getlastfree(t) > t->കണ്ണി) {
      കണ്ണി_2 *സ്വതന്ത്രം = --getlastfree(t);
      എങ്കിൽ (keyisnil(സ്വതന്ത്രം))
        തിരികെ സ്വതന്ത്രം;
    }
  }
  അല്ലെങ്കിൽ {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    ആകുമ്പോൾ (i--) {  /* do a linear search */
      കണ്ണി_2 *സ്വതന്ത്രം = gnode(t, i);
      എങ്കിൽ (keyisnil(സ്വതന്ത്രം))
        തിരികെ സ്വതന്ത്രം;
    }
  }
  തിരികെ ശൂന്യ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ insertkey (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *മൂല്യം) {
  കണ്ണി_2 *mp = mainpositionTV(t, താക്കോൽ);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, താക്കോൽ, 0)));
  എങ്കിൽ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    കണ്ണി_2 *othern;
    കണ്ണി_2 *f = getfreepos(t);  /* get a free place */
    എങ്കിൽ (f == ശൂന്യ)  /* cannot find a free place? */
      തിരികെ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    എങ്കിൽ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ആകുമ്പോൾ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      എങ്കിൽ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    അല്ലെങ്കിൽ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      എങ്കിൽ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      അല്ലെങ്കിൽ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, താക്കോൽ);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), മൂല്യം);
  തിരികെ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
സ്ഥിതം ശൂന്യം newcheckedkey (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *മൂല്യം) {
  unsigned i = keyinarray(t, താക്കോൽ);
  എങ്കിൽ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, മൂല്യം);  /* set value in the array */
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ done = insertkey(t, താക്കോൽ, മൂല്യം);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(ശൂന്യം, done);  /* to avoid warnings */
  }
}


സ്ഥിതം ശൂന്യം luaH_newkey (lua_State *L, Table *t, സ്ഥിരം TValue *താക്കോൽ,
                                                 TValue *മൂല്യം) {
  എങ്കിൽ (!ttisnil(മൂല്യം)) {  /* do not insert nil values */
    പൂർണ്ണസംഖ്യ done = insertkey(t, താക്കോൽ, മൂല്യം);
    എങ്കിൽ (!done) {  /* could not find a free place? */
      rehash(L, t, താക്കോൽ);  /* grow table */
      newcheckedkey(t, താക്കോൽ, മൂല്യം);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), താക്കോൽ);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (ശൂന്യം)0, (ശൂന്യം)0, 1);
  }
}


സ്ഥിതം സ്ഥിരം TValue *getintfromhash (Table *t, lua_Integer താക്കോൽ) {
  കണ്ണി_2 *n = hashint(t, താക്കോൽ);
  lua_assert(!ikeyinarray(t, താക്കോൽ));
  വേണ്ടി (;;) {  /* check whether 'key' is somewhere in the chain */
    എങ്കിൽ (keyisinteger(n) && keyival(n) == താക്കോൽ)
      തിരികെ gval(n);  /* that's it */
    അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ nx = gnext(n);
      എങ്കിൽ (nx == 0) നിർത്തുക;
      n += nx;
    }
  }
  തിരികെ &absentkey;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ hashkeyisempty (Table *t, lua_Unsigned താക്കോൽ) {
  സ്ഥിരം TValue *val = getintfromhash(t, l_castU2S(താക്കോൽ));
  തിരികെ isempty(val);
}


സ്ഥിതം lu_byte finishnodeget (സ്ഥിരം TValue *val, TValue *res) {
  എങ്കിൽ (!ttisnil(val)) {
    setobj(((lua_State*)ശൂന്യ), res, val);
  }
  തിരികെ ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer താക്കോൽ, TValue *res) {
  unsigned k = ikeyinarray(t, താക്കോൽ);
  എങ്കിൽ (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    എങ്കിൽ (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    തിരികെ tag;
  }
  അല്ലെങ്കിൽ
    തിരികെ finishnodeget(getintfromhash(t, താക്കോൽ), res);
}


/*
** search function for short strings
*/
സ്ഥിരം TValue *luaH_Hgetshortstr (Table *t, TString *താക്കോൽ) {
  കണ്ണി_2 *n = hashstr(t, താക്കോൽ);
  lua_assert(strisshr(താക്കോൽ));
  വേണ്ടി (;;) {  /* check whether 'key' is somewhere in the chain */
    എങ്കിൽ (keyisshrstr(n) && eqshrstr(keystrval(n), താക്കോൽ))
      തിരികെ gval(n);  /* that's it */
    അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ nx = gnext(n);
      എങ്കിൽ (nx == 0)
        തിരികെ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *താക്കോൽ, TValue *res) {
  തിരികെ finishnodeget(luaH_Hgetshortstr(t, താക്കോൽ), res);
}


സ്ഥിതം സ്ഥിരം TValue *Hgetlongstr (Table *t, TString *താക്കോൽ) {
  TValue ko;
  lua_assert(!strisshr(താക്കോൽ));
  setsvalue(cast(lua_State *, ശൂന്യ), &ko, താക്കോൽ);
  തിരികെ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


സ്ഥിതം സ്ഥിരം TValue *Hgetstr (Table *t, TString *താക്കോൽ) {
  എങ്കിൽ (strisshr(താക്കോൽ))
    തിരികെ luaH_Hgetshortstr(t, താക്കോൽ);
  അല്ലെങ്കിൽ
    തിരികെ Hgetlongstr(t, താക്കോൽ);
}


lu_byte luaH_getstr (Table *t, TString *താക്കോൽ, TValue *res) {
  തിരികെ finishnodeget(Hgetstr(t, താക്കോൽ), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *res) {
  സ്ഥിരം TValue *slot;
  തിരഞ്ഞെടുപ്പ് (ttypetag(താക്കോൽ)) {
    അവസ്ഥ LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(താക്കോൽ));
      നിർത്തുക;
    അവസ്ഥ LUA_VNUMINT:
      തിരികെ luaH_getint(t, ivalue(താക്കോൽ), res);
    അവസ്ഥ LUA_VNIL:
      slot = &absentkey;
      നിർത്തുക;
    അവസ്ഥ LUA_VNUMFLT: {
      lua_Integer k;
      എങ്കിൽ (luaV_flttointeger(fltvalue(താക്കോൽ), &k, F2Ieq)) /* integral index? */
        തിരികെ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    സ്ഥിരസ്ഥിതി:
      slot = getgeneric(t, താക്കോൽ, 0);
      നിർത്തുക;
  }
  തിരികെ finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ retpsetcode (Table *t, സ്ഥിരം TValue *slot) {
  എങ്കിൽ (isabstkey(slot))
    തിരികെ HNOTFOUND;  /* no slot with that key */
  അല്ലെങ്കിൽ  /* return node encoded */
    തിരികെ cast_int((cast(കണ്ണി_2*, slot) - t->കണ്ണി)) + HFIRSTNODE;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ finishnodeset (Table *t, സ്ഥിരം TValue *slot, TValue *val) {
  എങ്കിൽ (!ttisnil(slot)) {
    setobj(((lua_State*)ശൂന്യ), cast(TValue*, slot), val);
    തിരികെ HOK;  /* success */
  }
  അല്ലെങ്കിൽ
    തിരികെ retpsetcode(t, slot);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ rawfinishnodeset (സ്ഥിരം TValue *slot, TValue *val) {
  എങ്കിൽ (isabstkey(slot))
    തിരികെ 0;  /* no slot with that key */
  അല്ലെങ്കിൽ {
    setobj(((lua_State*)ശൂന്യ), cast(TValue*, slot), val);
    തിരികെ 1;  /* success */
  }
}


പൂർണ്ണസംഖ്യ luaH_psetint (Table *t, lua_Integer താക്കോൽ, TValue *val) {
  lua_assert(!ikeyinarray(t, താക്കോൽ));
  തിരികെ finishnodeset(t, getintfromhash(t, താക്കോൽ), val);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ psetint (Table *t, lua_Integer താക്കോൽ, TValue *val) {
  പൂർണ്ണസംഖ്യ hres;
  luaH_fastseti(t, താക്കോൽ, val, hres);
  തിരികെ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

പൂർണ്ണസംഖ്യ luaH_psetshortstr (Table *t, TString *താക്കോൽ, TValue *val) {
  സ്ഥിരം TValue *slot = luaH_Hgetshortstr(t, താക്കോൽ);
  എങ്കിൽ (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ശൂന്യ), cast(TValue*, slot), val);  /* update it */
    തിരികെ HOK;  /* done */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    എങ്കിൽ (ttisnil(val))  /* new value is nil? */
      തിരികെ HOK;  /* done (value is already nil/absent) */
    എങ്കിൽ (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(താക്കോൽ))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ശൂന്യ), &tk, താക്കോൽ);
      എങ്കിൽ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        തിരികെ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  തിരികെ retpsetcode(t, slot);
}


പൂർണ്ണസംഖ്യ luaH_psetstr (Table *t, TString *താക്കോൽ, TValue *val) {
  എങ്കിൽ (strisshr(താക്കോൽ))
    തിരികെ luaH_psetshortstr(t, താക്കോൽ, val);
  അല്ലെങ്കിൽ
    തിരികെ finishnodeset(t, Hgetlongstr(t, താക്കോൽ), val);
}


പൂർണ്ണസംഖ്യ luaH_pset (Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *val) {
  തിരഞ്ഞെടുപ്പ് (ttypetag(താക്കോൽ)) {
    അവസ്ഥ LUA_VSHRSTR: തിരികെ luaH_psetshortstr(t, tsvalue(താക്കോൽ), val);
    അവസ്ഥ LUA_VNUMINT: തിരികെ psetint(t, ivalue(താക്കോൽ), val);
    അവസ്ഥ LUA_VNIL: തിരികെ HNOTFOUND;
    അവസ്ഥ LUA_VNUMFLT: {
      lua_Integer k;
      എങ്കിൽ (luaV_flttointeger(fltvalue(താക്കോൽ), &k, F2Ieq)) /* integral index? */
        തിരികെ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    സ്ഥിരസ്ഥിതി:
      തിരികെ finishnodeset(t, getgeneric(t, താക്കോൽ, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
ശൂന്യം luaH_finishset (lua_State *L, Table *t, സ്ഥിരം TValue *താക്കോൽ,
                                    TValue *മൂല്യം, പൂർണ്ണസംഖ്യ hres) {
  lua_assert(hres != HOK);
  എങ്കിൽ (hres == HNOTFOUND) {
    TValue aux;
    സ്ഥിരം TValue *actk = താക്കോൽ;  /* actual key to insert */
    എങ്കിൽ (l_unlikely(ttisnil(താക്കോൽ)))
      luaG_runerror(L, "table index is nil");
    അല്ലെങ്കിൽ എങ്കിൽ (ttisfloat(താക്കോൽ)) {
      lua_സംഖ്യ f = fltvalue(താക്കോൽ);
      lua_Integer k;
      എങ്കിൽ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    അല്ലെങ്കിൽ എങ്കിൽ (isextstr(താക്കോൽ)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(താക്കോൽ));
      setsvalue2s(L, L->മുകൾ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->മുകൾ.p - 1), മൂല്യം);
      L->മുകൾ.p--;
      തിരികെ;
    }
    luaH_newkey(L, t, actk, മൂല്യം);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), മൂല്യം);
  }
  അല്ലെങ്കിൽ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), മൂല്യം);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
ശൂന്യം luaH_set (lua_State *L, Table *t, സ്ഥിരം TValue *താക്കോൽ, TValue *മൂല്യം) {
  പൂർണ്ണസംഖ്യ hres = luaH_pset(t, താക്കോൽ, മൂല്യം);
  എങ്കിൽ (hres != HOK)
    luaH_finishset(L, t, താക്കോൽ, മൂല്യം, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
ശൂന്യം luaH_setint (lua_State *L, Table *t, lua_Integer താക്കോൽ, TValue *മൂല്യം) {
  unsigned ik = ikeyinarray(t, താക്കോൽ);
  എങ്കിൽ (ik > 0)
    obj2arr(t, ik - 1, മൂല്യം);
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ ok = rawfinishnodeset(getintfromhash(t, താക്കോൽ), മൂല്യം);
    എങ്കിൽ (!ok) {
      TValue k;
      setivalue(&k, താക്കോൽ);
      luaH_newkey(L, t, &k, മൂല്യം);
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
സ്ഥിതം lua_Unsigned hash_തിരയൽ (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  പൂർണ്ണസംഖ്യ n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ആകുമ്പോൾ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    എങ്കിൽ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    അല്ലെങ്കിൽ {
      j = LUA_MAXINTEGER;
      എങ്കിൽ (hashkeyisempty(t, j))  /* t[j] not present? */
        നിർത്തുക;  /* 'j' now is an absent index */
      അല്ലെങ്കിൽ  /* weird case */
        തിരികെ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ആകുമ്പോൾ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    എങ്കിൽ (hashkeyisempty(t, m)) j = m;
    അല്ലെങ്കിൽ i = m;
  }
  തിരികെ i;
}


സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ binsearch (Table *array, unsigned പൂർണ്ണസംഖ്യ i, unsigned പൂർണ്ണസംഖ്യ j) {
  lua_assert(i <= j);
  ആകുമ്പോൾ (j - i > 1u) {  /*ദ്വിമാന തിരയൽ*/
    unsigned പൂർണ്ണസംഖ്യ m = (i + j) / 2;
    എങ്കിൽ (arraykeyisempty(array, m)) j = m;
    അല്ലെങ്കിൽ i = m;
  }
  തിരികെ i;
}


/* return a border, saving it as a hint for next call */
സ്ഥിതം lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  തിരികെ hint;
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
  എങ്കിൽ (asize > 0) {  /* is there an array part? */
    സ്ഥിരം unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    എങ്കിൽ (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    എങ്കിൽ (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      വേണ്ടി (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        എങ്കിൽ (!arraykeyisempty(t, limit))
          തിരികെ newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      തിരികെ newhint(t, binsearch(t, 0, limit));
    }
    അല്ലെങ്കിൽ {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      വേണ്ടി (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        എങ്കിൽ (arraykeyisempty(t, limit))
          തിരികെ newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      എങ്കിൽ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        തിരികെ newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  എങ്കിൽ (isdummy(t) || hashkeyisempty(t, asize + 1))
    തിരികെ asize;  /* 'asize + 1' is empty */
  അല്ലെങ്കിൽ  /* 'asize + 1' is also non empty */
    തിരികെ hash_തിരയൽ(L, t, asize);
}



#എങ്കിൽ defined(LUA_DEBUG)

/* export this function for the test library */

കണ്ണി_2 *luaH_mainposition (സ്ഥിരം Table *t, സ്ഥിരം TValue *താക്കോൽ) {
  തിരികെ mainpositionTV(t, താക്കോൽ);
}

#endif
