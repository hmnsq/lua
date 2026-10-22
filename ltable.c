/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


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

#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"


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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ { ಗಂಟು_2 *dummy; ಗಂಟು_2 follows_p_ಗಂಟು; } Limbox_aux;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ {
  ಗಂಟು_2 *lastfree;
  ವರ್ಣ padding[offsetof(Limbox_aux, follows_p_ಗಂಟು)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->ಗಂಟು) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(ಪೂರ್ಣಾಙ್ಕ) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(ಪರಿಮಾಣ(ಮೌಲ್ಯ_2) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, ಗಂಟು_2)


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
ಸ್ಥಿರ ಧ್ರುವ ಗಂಟು_2 dummynode_ = {
  {{ರಿಕ್ತ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ರಿಕ್ತ}}  /* key type, next, and key value */
};


ಸ್ಥಿರ ಧ್ರುವ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ಸ್ಥಿರ ಗಂಟು_2 *hashint (ಧ್ರುವ Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ಯದಿ (ui <= cast_uint(INT_MAX))
    ಪ್ರತ್ಯಾಗಮ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ hashmod(t, ui);
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
#ಯದಿ !defined(l_hashfloat)
ಸ್ಥಿರ unsigned l_hashfloat (lua_ಸಂಖ್ಯೆ n) {
  ಪೂರ್ಣಾಙ್ಕ i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_ಅಂಕೆ(INT_MIN);
  ಯದಿ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_ಅಂಕೆ(HUGE_VAL));
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಅನ್ಯಥಾ {  /* normal case */
    unsigned ಪೂರ್ಣಾಙ್ಕ u = cast_uint(i) + cast_uint(ni);
    ಪ್ರತ್ಯಾಗಮ (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ಸ್ಥಿರ ಗಂಟು_2 *mainpositionTV (ಧ್ರುವ Table *t, ಧ್ರುವ TValue *ಕೀಲಿ) {
  ವಿಕಲ್ಪ (ttypetag(ಕೀಲಿ)) {
    ಪಕ್ಷ LUA_VNUMINT: {
      lua_Integer i = ivalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashint(t, i);
    }
    ಪಕ್ಷ LUA_VNUMFLT: {
      lua_ಸಂಖ್ಯೆ n = fltvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashmod(t, l_hashfloat(n));
    }
    ಪಕ್ಷ LUA_VSHRSTR: {
      TString *ts = tsvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashstr(t, ts);
    }
    ಪಕ್ಷ LUA_VLNGSTR: {
      TString *ts = tsvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashpow2(t, luaS_hashlongstr(ts));
    }
    ಪಕ್ಷ LUA_VFALSE:
      ಪ್ರತ್ಯಾಗಮ hashboolean(t, 0);
    ಪಕ್ಷ LUA_VTRUE:
      ಪ್ರತ್ಯಾಗಮ hashboolean(t, 1);
    ಪಕ್ಷ LUA_VLIGHTUSERDATA: {
      ಶೂನ್ಯ *p = pvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashpointer(t, p);
    }
    ಪಕ್ಷ LUA_VLCF: {
      lua_CFunction f = fvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashpointer(t, f);
    }
    ಸ್ವಭಾವ: {
      GCObject *o = gcvalue(ಕೀಲಿ);
      ಪ್ರತ್ಯಾಗಮ hashpointer(t, o);
    }
  }
}


l_sinline ಗಂಟು_2 *mainpositionfromnode (ಧ್ರುವ Table *t, ಗಂಟು_2 *nd) {
  TValue ಕೀಲಿ;
  getnodekey(cast(lua_State *, ರಿಕ್ತ), &ಕೀಲಿ, nd);
  ಪ್ರತ್ಯಾಗಮ mainpositionTV(t, &ಕೀಲಿ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ equalkey (ಧ್ರುವ TValue *k1, ಧ್ರುವ ಗಂಟು_2 *n2, ಪೂರ್ಣಾಙ್ಕ deadok) {
  ಯದಿ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ಯದಿ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ಪ್ರತ್ಯಾಗಮ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ಅನ್ಯಥಾ ಯದಿ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ಪ್ರತ್ಯಾಗಮ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ಅನ್ಯಥಾ
     ಪ್ರತ್ಯಾಗಮ 0;  /* otherwise, different variants cannot be equal */
  }
  ಅನ್ಯಥಾ {  /* equal variants */
    ವಿಕಲ್ಪ (keytt(n2)) {
      ಪಕ್ಷ LUA_VNIL: ಪಕ್ಷ LUA_VFALSE: ಪಕ್ಷ LUA_VTRUE:
        ಪ್ರತ್ಯಾಗಮ 1;
      ಪಕ್ಷ LUA_VNUMINT:
        ಪ್ರತ್ಯಾಗಮ (ivalue(k1) == keyival(n2));
      ಪಕ್ಷ LUA_VNUMFLT:
        ಪ್ರತ್ಯಾಗಮ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      ಪಕ್ಷ LUA_VLIGHTUSERDATA:
        ಪ್ರತ್ಯಾಗಮ pvalue(k1) == pvalueraw(keyval(n2));
      ಪಕ್ಷ LUA_VLCF:
        ಪ್ರತ್ಯಾಗಮ fvalue(k1) == fvalueraw(keyval(n2));
      ಪಕ್ಷ ctb(LUA_VLNGSTR):
        ಪ್ರತ್ಯಾಗಮ luaS_eqstr(tsvalue(k1), keystrval(n2));
      ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ಸ್ಥಿರ ಧ್ರುವ TValue *getgeneric (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ deadok) {
  ಗಂಟು_2 *n = mainpositionTV(t, ಕೀಲಿ);
  ಪ್ರತಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಯದಿ (equalkey(ಕೀಲಿ, n, deadok))
      ಪ್ರತ್ಯಾಗಮ gval(n);  /* that's it */
    ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ nx = gnext(n);
      ಯದಿ (nx == 0)
        ಪ್ರತ್ಯಾಗಮ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ಸ್ಥಿರ unsigned checkrange (lua_Integer k, unsigned limit) {
  ಪ್ರತ್ಯಾಗಮ (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
ಸ್ಥಿರ unsigned keyinarray (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ) {
  ಪ್ರತ್ಯಾಗಮ (ttisinteger(ಕೀಲಿ)) ? ikeyinarray(t, ivalue(ಕೀಲಿ)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ಸ್ಥಿರ unsigned findindex (lua_State *L, Table *t, TValue *ಕೀಲಿ,
                               unsigned asize) {
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ (ttisnil(ಕೀಲಿ)) ಪ್ರತ್ಯಾಗಮ 0;  /* first iteration */
  i = keyinarray(t, ಕೀಲಿ);
  ಯದಿ (i != 0)  /* is 'key' inside array part? */
    ಪ್ರತ್ಯಾಗಮ i;  /* yes; that's the index */
  ಅನ್ಯಥಾ {
    ಧ್ರುವ TValue *n = getgeneric(t, ಕೀಲಿ, 1);
    ಯದಿ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ಪ್ರತ್ಯಾಗಮ (i + 1) + asize;
  }
}


ಪೂರ್ಣಾಙ್ಕ luaH_next (lua_State *L, Table *t, StkId ಕೀಲಿ) {
  unsigned ಪೂರ್ಣಾಙ್ಕ asize = t->asize;
  unsigned ಪೂರ್ಣಾಙ್ಕ i = findindex(L, t, s2v(ಕೀಲಿ), asize);  /* find original key */
  ಪ್ರತಿ (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    ಯದಿ (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(ಕೀಲಿ), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(ಕೀಲಿ + 1));
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತಿ (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ಯದಿ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ಗಂಟು_2 *n = gnode(t, i);
      getnodekey(L, s2v(ಕೀಲಿ), n);
      setobj2s(L, ಕೀಲಿ + 1, gval(n));
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? ಪರಿಮಾಣ(Limbox) : 0)

/* 'node' size in bytes */
ಸ್ಥಿರ size_t sizehash (Table *t) {
  ಪ್ರತ್ಯಾಗಮ cast_sizet(sizenode(t)) * ಪರಿಮಾಣ(ಗಂಟು_2) + extraLastfree(t);
}


ಸ್ಥಿರ ಶೂನ್ಯ freehash (lua_State *L, Table *t) {
  ಯದಿ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    ವರ್ಣ *arr = cast_charp(t->ಗಂಟು) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ insertkey (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ newcheckedkey (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  unsigned total;
  unsigned na;
  ಪೂರ್ಣಾಙ್ಕ deleted;
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
ಸ್ಥಿರ unsigned computesizes (Counters *ct) {
  ಪೂರ್ಣಾಙ್ಕ i;
  unsigned ಪೂರ್ಣಾಙ್ಕ twotoi;  /* 2^i (candidate for optimal size) */
  unsigned ಪೂರ್ಣಾಙ್ಕ a = 0;  /* number of elements smaller than 2^i */
  unsigned ಪೂರ್ಣಾಙ್ಕ na = 0;  /* number of elements to go to array part */
  unsigned ಪೂರ್ಣಾಙ್ಕ optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  ಪ್ರತಿ (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    ಯದಿ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ಪ್ರತ್ಯಾಗಮ optimal;
}


ಸ್ಥಿರ ಶೂನ್ಯ countint (lua_Integer ಕೀಲಿ, Counters *ct) {
  unsigned ಪೂರ್ಣಾಙ್ಕ k = arrayindex(ಕೀಲಿ);
  ಯದಿ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline ಪೂರ್ಣಾಙ್ಕ arraykeyisempty (ಧ್ರುವ Table *t, unsigned ಕೀಲಿ) {
  ಪೂರ್ಣಾಙ್ಕ tag = *getArrTag(t, ಕೀಲಿ - 1);
  ಪ್ರತ್ಯಾಗಮ tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ numusearray (ಧ್ರುವ Table *t, Counters *ct) {
  ಪೂರ್ಣಾಙ್ಕ lg;
  unsigned ಪೂರ್ಣಾಙ್ಕ ttlg;  /* 2^lg */
  unsigned ಪೂರ್ಣಾಙ್ಕ ause = 0;  /* summation of 'nums' */
  unsigned ಪೂರ್ಣಾಙ್ಕ i = 1;  /* index to traverse all array keys */
  unsigned ಪೂರ್ಣಾಙ್ಕ asize = t->asize;
  /* traverse each slice */
  ಪ್ರತಿ (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned ಪೂರ್ಣಾಙ್ಕ lc = 0;  /* counter */
    unsigned ಪೂರ್ಣಾಙ್ಕ lim = ttlg;
    ಯದಿ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ಯದಿ (i > lim)
        ಭಙ್ಗ;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    ಪ್ರತಿ (; i <= lim; i++) {
      ಯದಿ (!arraykeyisempty(t, i))
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
ಸ್ಥಿರ ಶೂನ್ಯ numusehash (ಧ್ರುವ Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  ಯಾವತ್ (i--) {
    ಗಂಟು_2 *n = &t->ಗಂಟು[i];
    ಯದಿ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ಅನ್ಯಥಾ {
      total++;
      ಯದಿ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ಸ್ಥಿರ size_t concretesize (unsigned ಪೂರ್ಣಾಙ್ಕ size) {
  ಯದಿ (size == 0)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಅನ್ಯಥಾ  /* space for the two arrays plus an unsigned in between */
    ಪ್ರತ್ಯಾಗಮ size * (ಪರಿಮಾಣ(ಮೌಲ್ಯ_2) + 1) + ಪರಿಮಾಣ(unsigned);
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
ಸ್ಥಿರ ಮೌಲ್ಯ_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  ಯದಿ (oldasize == newasize)
    ಪ್ರತ್ಯಾಗಮ t->array;  /* nothing to be done */
  ಅನ್ಯಥಾ ಯದಿ (newasize == 0) {  /* erasing array? */
    ಮೌಲ್ಯ_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಅನ್ಯಥಾ {
    size_t newasizeb = concretesize(newasize);
    ಮೌಲ್ಯ_2 *np = cast(ಮೌಲ್ಯ_2 *,
                  luaM_reallocvector(L, ರಿಕ್ತ, 0, newasizeb, lu_byte));
    ಯದಿ (np == ರಿಕ್ತ)  /* allocation error? */
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    np += newasize;  /* shift pointer to the end of value segment */
    ಯದಿ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      ಮೌಲ್ಯ_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ಪ್ರತ್ಯಾಗಮ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setnodevector (lua_State *L, Table *t, unsigned size) {
  ಯದಿ (size == 0) {  /* no elements to hash part? */
    t->ಗಂಟು = cast(ಗಂಟು_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ lsize = luaO_ceillog2(size);
    ಯದಿ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    ಯದಿ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ಗಂಟು = luaM_newvector(L, size, ಗಂಟು_2);
    ಅನ್ಯಥಾ {
      size_t bsize = size * ಪರಿಮಾಣ(ಗಂಟು_2) + ಪರಿಮಾಣ(Limbox);
      ವರ್ಣ *ಗಂಟು = luaM_newblock(L, bsize);
      t->ಗಂಟು = cast(ಗಂಟು_2 *, ಗಂಟು + ಪರಿಮಾಣ(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    ಪ್ರತಿ (i = 0; i < cast_int(size); i++) {
      ಗಂಟು_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  ಪ್ರತಿ (j = 0; j < size; j++) {
    ಗಂಟು_2 *old = gnode(ot, j);
    ಯದಿ (!isempty(gval(old))) {
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
ಸ್ಥಿರ ಶೂನ್ಯ exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ಗಂಟು_2 *ಗಂಟು = t1->ಗಂಟು;
  ಪೂರ್ಣಾಙ್ಕ bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->ಗಂಟು = t2->ಗಂಟು;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->ಗಂಟು = ಗಂಟು;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  ಪ್ರತಿ (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    ಯದಿ (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue ಕೀಲಿ, aux;
      setivalue(&ಕೀಲಿ, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &ಕೀಲಿ, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ಸ್ಥಿರ ಶೂನ್ಯ clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  ಪ್ರತಿ (; oldasize < newasize; oldasize++)
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
ಶೂನ್ಯ luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  ಮೌಲ್ಯ_2 *newarray;
  ಯದಿ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ಯದಿ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ಯದಿ (l_unlikely(newarray == ರಿಕ್ತ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ಯದಿ (newarray != ರಿಕ್ತ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


ಶೂನ್ಯ luaH_resizearray (lua_State *L, Table *t, unsigned ಪೂರ್ಣಾಙ್ಕ nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rehash (lua_State *L, Table *t, ಧ್ರುವ TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  ಪ್ರತಿ (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  ಯದಿ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ಯದಿ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ಅನ್ಯಥಾ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  ಯದಿ (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, ಪರಿಮಾಣ(Table));
  Table *t = gco2t(o);
  t->metatable = ರಿಕ್ತ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ರಿಕ್ತ;
  t->asize = 0;
  setnodevector(L, t, 0);
  ಪ್ರತ್ಯಾಗಮ t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, ಪರಿಮಾಣ(Table)) + concretesize(t->asize);
  ಯದಿ (!isdummy(t))
    sz += sizehash(t);
  ಪ್ರತ್ಯಾಗಮ sz;
}


/*
** Frees a table.
*/
ಶೂನ್ಯ luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ಸ್ಥಿರ ಗಂಟು_2 *getfreepos (Table *t) {
  ಯದಿ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ಯಾವತ್ (getlastfree(t) > t->ಗಂಟು) {
      ಗಂಟು_2 *ಮುಕ್ತ = --getlastfree(t);
      ಯದಿ (keyisnil(ಮುಕ್ತ))
        ಪ್ರತ್ಯಾಗಮ ಮುಕ್ತ;
    }
  }
  ಅನ್ಯಥಾ {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    ಯಾವತ್ (i--) {  /* do a linear search */
      ಗಂಟು_2 *ಮುಕ್ತ = gnode(t, i);
      ಯದಿ (keyisnil(ಮುಕ್ತ))
        ಪ್ರತ್ಯಾಗಮ ಮುಕ್ತ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ insertkey (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಗಂಟು_2 *mp = mainpositionTV(t, ಕೀಲಿ);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ಕೀಲಿ, 0)));
  ಯದಿ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ಗಂಟು_2 *othern;
    ಗಂಟು_2 *f = getfreepos(t);  /* get a free place */
    ಯದಿ (f == ರಿಕ್ತ)  /* cannot find a free place? */
      ಪ್ರತ್ಯಾಗಮ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ಯದಿ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ಯಾವತ್ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ಯದಿ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ಅನ್ಯಥಾ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ಯದಿ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ಅನ್ಯಥಾ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ಕೀಲಿ);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), ಮೌಲ್ಯ);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ಸ್ಥಿರ ಶೂನ್ಯ newcheckedkey (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  unsigned i = keyinarray(t, ಕೀಲಿ);
  ಯದಿ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, ಮೌಲ್ಯ);  /* set value in the array */
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ done = insertkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(ಶೂನ್ಯ, done);  /* to avoid warnings */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ luaH_newkey (lua_State *L, Table *t, ಧ್ರುವ TValue *ಕೀಲಿ,
                                                 TValue *ಮೌಲ್ಯ) {
  ಯದಿ (!ttisnil(ಮೌಲ್ಯ)) {  /* do not insert nil values */
    ಪೂರ್ಣಾಙ್ಕ done = insertkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);
    ಯದಿ (!done) {  /* could not find a free place? */
      rehash(L, t, ಕೀಲಿ);  /* grow table */
      newcheckedkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ಕೀಲಿ);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (ಶೂನ್ಯ)0, (ಶೂನ್ಯ)0, 1);
  }
}


ಸ್ಥಿರ ಧ್ರುವ TValue *getintfromhash (Table *t, lua_Integer ಕೀಲಿ) {
  ಗಂಟು_2 *n = hashint(t, ಕೀಲಿ);
  lua_assert(!ikeyinarray(t, ಕೀಲಿ));
  ಪ್ರತಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಯದಿ (keyisinteger(n) && keyival(n) == ಕೀಲಿ)
      ಪ್ರತ್ಯಾಗಮ gval(n);  /* that's it */
    ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ nx = gnext(n);
      ಯದಿ (nx == 0) ಭಙ್ಗ;
      n += nx;
    }
  }
  ಪ್ರತ್ಯಾಗಮ &absentkey;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hashkeyisempty (Table *t, lua_Unsigned ಕೀಲಿ) {
  ಧ್ರುವ TValue *val = getintfromhash(t, l_castU2S(ಕೀಲಿ));
  ಪ್ರತ್ಯಾಗಮ isempty(val);
}


ಸ್ಥಿರ lu_byte finishnodeget (ಧ್ರುವ TValue *val, TValue *res) {
  ಯದಿ (!ttisnil(val)) {
    setobj(((lua_State*)ರಿಕ್ತ), res, val);
  }
  ಪ್ರತ್ಯಾಗಮ ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer ಕೀಲಿ, TValue *res) {
  unsigned k = ikeyinarray(t, ಕೀಲಿ);
  ಯದಿ (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    ಯದಿ (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    ಪ್ರತ್ಯಾಗಮ tag;
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ finishnodeget(getintfromhash(t, ಕೀಲಿ), res);
}


/*
** search function for short strings
*/
ಧ್ರುವ TValue *luaH_Hgetshortstr (Table *t, TString *ಕೀಲಿ) {
  ಗಂಟು_2 *n = hashstr(t, ಕೀಲಿ);
  lua_assert(strisshr(ಕೀಲಿ));
  ಪ್ರತಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಯದಿ (keyisshrstr(n) && eqshrstr(keystrval(n), ಕೀಲಿ))
      ಪ್ರತ್ಯಾಗಮ gval(n);  /* that's it */
    ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ nx = gnext(n);
      ಯದಿ (nx == 0)
        ಪ್ರತ್ಯಾಗಮ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *ಕೀಲಿ, TValue *res) {
  ಪ್ರತ್ಯಾಗಮ finishnodeget(luaH_Hgetshortstr(t, ಕೀಲಿ), res);
}


ಸ್ಥಿರ ಧ್ರುವ TValue *Hgetlongstr (Table *t, TString *ಕೀಲಿ) {
  TValue ko;
  lua_assert(!strisshr(ಕೀಲಿ));
  setsvalue(cast(lua_State *, ರಿಕ್ತ), &ko, ಕೀಲಿ);
  ಪ್ರತ್ಯಾಗಮ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ಸ್ಥಿರ ಧ್ರುವ TValue *Hgetstr (Table *t, TString *ಕೀಲಿ) {
  ಯದಿ (strisshr(ಕೀಲಿ))
    ಪ್ರತ್ಯಾಗಮ luaH_Hgetshortstr(t, ಕೀಲಿ);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ Hgetlongstr(t, ಕೀಲಿ);
}


lu_byte luaH_getstr (Table *t, TString *ಕೀಲಿ, TValue *res) {
  ಪ್ರತ್ಯಾಗಮ finishnodeget(Hgetstr(t, ಕೀಲಿ), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *res) {
  ಧ್ರುವ TValue *slot;
  ವಿಕಲ್ಪ (ttypetag(ಕೀಲಿ)) {
    ಪಕ್ಷ LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(ಕೀಲಿ));
      ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VNUMINT:
      ಪ್ರತ್ಯಾಗಮ luaH_getint(t, ivalue(ಕೀಲಿ), res);
    ಪಕ್ಷ LUA_VNIL:
      slot = &absentkey;
      ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VNUMFLT: {
      lua_Integer k;
      ಯದಿ (luaV_flttointeger(fltvalue(ಕೀಲಿ), &k, F2Ieq)) /* integral index? */
        ಪ್ರತ್ಯಾಗಮ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ಸ್ವಭಾವ:
      slot = getgeneric(t, ಕೀಲಿ, 0);
      ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ retpsetcode (Table *t, ಧ್ರುವ TValue *slot) {
  ಯದಿ (isabstkey(slot))
    ಪ್ರತ್ಯಾಗಮ HNOTFOUND;  /* no slot with that key */
  ಅನ್ಯಥಾ  /* return node encoded */
    ಪ್ರತ್ಯಾಗಮ cast_int((cast(ಗಂಟು_2*, slot) - t->ಗಂಟು)) + HFIRSTNODE;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ finishnodeset (Table *t, ಧ್ರುವ TValue *slot, TValue *val) {
  ಯದಿ (!ttisnil(slot)) {
    setobj(((lua_State*)ರಿಕ್ತ), cast(TValue*, slot), val);
    ಪ್ರತ್ಯಾಗಮ HOK;  /* success */
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ retpsetcode(t, slot);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rawfinishnodeset (ಧ್ರುವ TValue *slot, TValue *val) {
  ಯದಿ (isabstkey(slot))
    ಪ್ರತ್ಯಾಗಮ 0;  /* no slot with that key */
  ಅನ್ಯಥಾ {
    setobj(((lua_State*)ರಿಕ್ತ), cast(TValue*, slot), val);
    ಪ್ರತ್ಯಾಗಮ 1;  /* success */
  }
}


ಪೂರ್ಣಾಙ್ಕ luaH_psetint (Table *t, lua_Integer ಕೀಲಿ, TValue *val) {
  lua_assert(!ikeyinarray(t, ಕೀಲಿ));
  ಪ್ರತ್ಯಾಗಮ finishnodeset(t, getintfromhash(t, ಕೀಲಿ), val);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ psetint (Table *t, lua_Integer ಕೀಲಿ, TValue *val) {
  ಪೂರ್ಣಾಙ್ಕ hres;
  luaH_fastseti(t, ಕೀಲಿ, val, hres);
  ಪ್ರತ್ಯಾಗಮ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

ಪೂರ್ಣಾಙ್ಕ luaH_psetshortstr (Table *t, TString *ಕೀಲಿ, TValue *val) {
  ಧ್ರುವ TValue *slot = luaH_Hgetshortstr(t, ಕೀಲಿ);
  ಯದಿ (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ರಿಕ್ತ), cast(TValue*, slot), val);  /* update it */
    ಪ್ರತ್ಯಾಗಮ HOK;  /* done */
  }
  ಅನ್ಯಥಾ ಯದಿ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ಯದಿ (ttisnil(val))  /* new value is nil? */
      ಪ್ರತ್ಯಾಗಮ HOK;  /* done (value is already nil/absent) */
    ಯದಿ (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(ಕೀಲಿ))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ರಿಕ್ತ), &tk, ಕೀಲಿ);
      ಯದಿ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ಪ್ರತ್ಯಾಗಮ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ಪ್ರತ್ಯಾಗಮ retpsetcode(t, slot);
}


ಪೂರ್ಣಾಙ್ಕ luaH_psetstr (Table *t, TString *ಕೀಲಿ, TValue *val) {
  ಯದಿ (strisshr(ಕೀಲಿ))
    ಪ್ರತ್ಯಾಗಮ luaH_psetshortstr(t, ಕೀಲಿ, val);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ finishnodeset(t, Hgetlongstr(t, ಕೀಲಿ), val);
}


ಪೂರ್ಣಾಙ್ಕ luaH_pset (Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *val) {
  ವಿಕಲ್ಪ (ttypetag(ಕೀಲಿ)) {
    ಪಕ್ಷ LUA_VSHRSTR: ಪ್ರತ್ಯಾಗಮ luaH_psetshortstr(t, tsvalue(ಕೀಲಿ), val);
    ಪಕ್ಷ LUA_VNUMINT: ಪ್ರತ್ಯಾಗಮ psetint(t, ivalue(ಕೀಲಿ), val);
    ಪಕ್ಷ LUA_VNIL: ಪ್ರತ್ಯಾಗಮ HNOTFOUND;
    ಪಕ್ಷ LUA_VNUMFLT: {
      lua_Integer k;
      ಯದಿ (luaV_flttointeger(fltvalue(ಕೀಲಿ), &k, F2Ieq)) /* integral index? */
        ಪ್ರತ್ಯಾಗಮ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ finishnodeset(t, getgeneric(t, ಕೀಲಿ, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
ಶೂನ್ಯ luaH_finishset (lua_State *L, Table *t, ಧ್ರುವ TValue *ಕೀಲಿ,
                                    TValue *ಮೌಲ್ಯ, ಪೂರ್ಣಾಙ್ಕ hres) {
  lua_assert(hres != HOK);
  ಯದಿ (hres == HNOTFOUND) {
    TValue aux;
    ಧ್ರುವ TValue *actk = ಕೀಲಿ;  /* actual key to insert */
    ಯದಿ (l_unlikely(ttisnil(ಕೀಲಿ)))
      luaG_runerror(L, "table index is nil");
    ಅನ್ಯಥಾ ಯದಿ (ttisfloat(ಕೀಲಿ)) {
      lua_ಸಂಖ್ಯೆ f = fltvalue(ಕೀಲಿ);
      lua_Integer k;
      ಯದಿ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ಅನ್ಯಥಾ ಯದಿ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ಅನ್ಯಥಾ ಯದಿ (isextstr(ಕೀಲಿ)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ಕೀಲಿ));
      setsvalue2s(L, L->ತುದಿ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->ತುದಿ.p - 1), ಮೌಲ್ಯ);
      L->ತುದಿ.p--;
      ಪ್ರತ್ಯಾಗಮ;
    }
    luaH_newkey(L, t, actk, ಮೌಲ್ಯ);
  }
  ಅನ್ಯಥಾ ಯದಿ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), ಮೌಲ್ಯ);
  }
  ಅನ್ಯಥಾ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), ಮೌಲ್ಯ);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
ಶೂನ್ಯ luaH_set (lua_State *L, Table *t, ಧ್ರುವ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಪೂರ್ಣಾಙ್ಕ hres = luaH_pset(t, ಕೀಲಿ, ಮೌಲ್ಯ);
  ಯದಿ (hres != HOK)
    luaH_finishset(L, t, ಕೀಲಿ, ಮೌಲ್ಯ, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
ಶೂನ್ಯ luaH_setint (lua_State *L, Table *t, lua_Integer ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  unsigned ik = ikeyinarray(t, ಕೀಲಿ);
  ಯದಿ (ik > 0)
    obj2arr(t, ik - 1, ಮೌಲ್ಯ);
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ ok = rawfinishnodeset(getintfromhash(t, ಕೀಲಿ), ಮೌಲ್ಯ);
    ಯದಿ (!ok) {
      TValue k;
      setivalue(&k, ಕೀಲಿ);
      luaH_newkey(L, t, &k, ಮೌಲ್ಯ);
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
ಸ್ಥಿರ lua_Unsigned hash_ಹುಡುಕು (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  ಪೂರ್ಣಾಙ್ಕ n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ಯಾವತ್ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ಯದಿ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ಅನ್ಯಥಾ {
      j = LUA_MAXINTEGER;
      ಯದಿ (hashkeyisempty(t, j))  /* t[j] not present? */
        ಭಙ್ಗ;  /* 'j' now is an absent index */
      ಅನ್ಯಥಾ  /* weird case */
        ಪ್ರತ್ಯಾಗಮ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ಯಾವತ್ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ಯದಿ (hashkeyisempty(t, m)) j = m;
    ಅನ್ಯಥಾ i = m;
  }
  ಪ್ರತ್ಯಾಗಮ i;
}


ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ binsearch (Table *array, unsigned ಪೂರ್ಣಾಙ್ಕ i, unsigned ಪೂರ್ಣಾಙ್ಕ j) {
  lua_assert(i <= j);
  ಯಾವತ್ (j - i > 1u) {  /*ದ್ವಿಮಾನ ಹುಡುಕು*/
    unsigned ಪೂರ್ಣಾಙ್ಕ m = (i + j) / 2;
    ಯದಿ (arraykeyisempty(array, m)) j = m;
    ಅನ್ಯಥಾ i = m;
  }
  ಪ್ರತ್ಯಾಗಮ i;
}


/* return a border, saving it as a hint for next call */
ಸ್ಥಿರ lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ಪ್ರತ್ಯಾಗಮ hint;
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
  ಯದಿ (asize > 0) {  /* is there an array part? */
    ಧ್ರುವ unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    ಯದಿ (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    ಯದಿ (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ಪ್ರತಿ (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        ಯದಿ (!arraykeyisempty(t, limit))
          ಪ್ರತ್ಯಾಗಮ newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ಪ್ರತ್ಯಾಗಮ newhint(t, binsearch(t, 0, limit));
    }
    ಅನ್ಯಥಾ {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ಪ್ರತಿ (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        ಯದಿ (arraykeyisempty(t, limit))
          ಪ್ರತ್ಯಾಗಮ newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      ಯದಿ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ಪ್ರತ್ಯಾಗಮ newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ಯದಿ (isdummy(t) || hashkeyisempty(t, asize + 1))
    ಪ್ರತ್ಯಾಗಮ asize;  /* 'asize + 1' is empty */
  ಅನ್ಯಥಾ  /* 'asize + 1' is also non empty */
    ಪ್ರತ್ಯಾಗಮ hash_ಹುಡುಕು(L, t, asize);
}



#ಯದಿ defined(LUA_DEBUG)

/* export this function for the test library */

ಗಂಟು_2 *luaH_mainposition (ಧ್ರುವ Table *t, ಧ್ರುವ TValue *ಕೀಲಿ) {
  ಪ್ರತ್ಯಾಗಮ mainpositionTV(t, ಕೀಲಿ);
}

#endif
