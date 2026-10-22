/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltable_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


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

#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ { ਗੰਢ_2 *dummy; ਗੰਢ_2 follows_p_ਗੰਢ; } Limbox_aux;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ {
  ਗੰਢ_2 *lastfree;
  ਅੱਖਰ padding[offsetof(Limbox_aux, follows_p_ਗੰਢ)];
} Limbox;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getlastfree(t)     ((cast(Limbox *, (t)->ਗੰਢ) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXABITS	(l_numbits(ਪੂਰਨ) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXASIZEB	(MAX_SIZET/(ਆਕਾਰ(ਮੁੱਲ_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXHSIZE	lua_M_ਹੱਦਬੰਦੀ_N(1 << MAXHBITS, ਗੰਢ_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hashstr(t,str)		hashpow2(t, (str)->hash)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hashboolean(t,p)	hashpow2(t, p)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hashpointer(t,p)	hashmod(t, point2uint(p))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
ਸਥਿਰ ਅਚਲ ਗੰਢ_2 dummynode_ = {
  {{ਨਿਰਾ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ਨਿਰਾ}}  /* key type, next, and key value */
};


ਸਥਿਰ ਅਚਲ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ਸਥਿਰ ਗੰਢ_2 *hashint (ਅਚਲ ਸਾਰਣੀ *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ਜੇ (ui <= cast_uint(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ))
    ਵਾਪਸ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ hashmod(t, ui);
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
#ਜੇ !defined(l_hashfloat)
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ l_hashfloat (lua_ਸੰਖਿਆ n) {
  ਪੂਰਨ i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_ਅੰਕ(INT_MIN);
  ਜੇ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_ਅੰਕ(HUGE_VAL));
    ਵਾਪਸ 0;
  }
  ਨਹੀਂਤਾਂ {  /* normal case */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ u = cast_uint(i) + cast_uint(ni);
    ਵਾਪਸ (u <= cast_uint(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ) ? u : ~u);
  }
}
#ਜੇ_ਅੰਤ


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ਸਥਿਰ ਗੰਢ_2 *mainpositionTV (ਅਚਲ ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ) {
  ਚੋਣ (ttypetag(ਕੁੰਜੀ)) {
    ਹਾਲਤ LUA_VNUMINT: {
      lua_Integer i = ivalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashint(t, i);
    }
    ਹਾਲਤ LUA_VNUMFLT: {
      lua_ਸੰਖਿਆ n = fltvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashmod(t, l_hashfloat(n));
    }
    ਹਾਲਤ LUA_VSHRSTR: {
      TString *ts = tsvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashstr(t, ts);
    }
    ਹਾਲਤ LUA_VLNGSTR: {
      TString *ts = tsvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashpow2(t, luaS_hashlongstr(ts));
    }
    ਹਾਲਤ LUA_VFALSE:
      ਵਾਪਸ hashboolean(t, 0);
    ਹਾਲਤ LUA_VTRUE:
      ਵਾਪਸ hashboolean(t, 1);
    ਹਾਲਤ LUA_VLIGHTUSERDATA: {
      ਖਾਲੀ *p = pvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashpointer(t, p);
    }
    ਹਾਲਤ LUA_VLCF: {
      lua_CFunction f = fvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashpointer(t, f);
    }
    ਮੂਲ: {
      GCObject *o = gcvalue(ਕੁੰਜੀ);
      ਵਾਪਸ hashpointer(t, o);
    }
  }
}


l_sinline ਗੰਢ_2 *mainpositionfromnode (ਅਚਲ ਸਾਰਣੀ *t, ਗੰਢ_2 *nd) {
  TValue ਕੁੰਜੀ;
  getnodekey(cast(lua_State *, ਨਿਰਾ), &ਕੁੰਜੀ, nd);
  ਵਾਪਸ mainpositionTV(t, &ਕੁੰਜੀ);
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
ਸਥਿਰ ਪੂਰਨ equalkey (ਅਚਲ TValue *k1, ਅਚਲ ਗੰਢ_2 *n2, ਪੂਰਨ deadok) {
  ਜੇ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ਜੇ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ਵਾਪਸ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ਨਹੀਂਤਾਂ ਜੇ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ਵਾਪਸ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ਨਹੀਂਤਾਂ
     ਵਾਪਸ 0;  /* otherwise, different variants cannot be equal */
  }
  ਨਹੀਂਤਾਂ {  /* equal variants */
    ਚੋਣ (keytt(n2)) {
      ਹਾਲਤ LUA_VNIL: ਹਾਲਤ LUA_VFALSE: ਹਾਲਤ LUA_VTRUE:
        ਵਾਪਸ 1;
      ਹਾਲਤ LUA_VNUMINT:
        ਵਾਪਸ (ivalue(k1) == keyival(n2));
      ਹਾਲਤ LUA_VNUMFLT:
        ਵਾਪਸ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      ਹਾਲਤ LUA_VLIGHTUSERDATA:
        ਵਾਪਸ pvalue(k1) == pvalueraw(keyval(n2));
      ਹਾਲਤ LUA_VLCF:
        ਵਾਪਸ fvalue(k1) == fvalueraw(keyval(n2));
      ਹਾਲਤ ctb(LUA_VLNGSTR):
        ਵਾਪਸ luaS_eqstr(tsvalue(k1), keystrval(n2));
      ਮੂਲ:
        ਵਾਪਸ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ਸਥਿਰ ਅਚਲ TValue *getgeneric (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, ਪੂਰਨ deadok) {
  ਗੰਢ_2 *n = mainpositionTV(t, ਕੁੰਜੀ);
  ਲਈ (;;) {  /* check whether 'key' is somewhere in the chain */
    ਜੇ (equalkey(ਕੁੰਜੀ, n, deadok))
      ਵਾਪਸ gval(n);  /* that's it */
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ nx = gnext(n);
      ਜੇ (nx == 0)
        ਵਾਪਸ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ checkrange (lua_Integer k, ਬਿਨਾਨਿਸ਼ਾਨੀ ਹੱਦਬੰਦੀ) {
  ਵਾਪਸ (l_castS2U(k) - 1u < ਹੱਦਬੰਦੀ) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ keyinarray (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ) {
  ਵਾਪਸ (ttisinteger(ਕੁੰਜੀ)) ? ikeyinarray(t, ivalue(ਕੁੰਜੀ)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ findindex (lua_State *L, ਸਾਰਣੀ *t, TValue *ਕੁੰਜੀ,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ asize) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਜੇ (ttisnil(ਕੁੰਜੀ)) ਵਾਪਸ 0;  /* first iteration */
  i = keyinarray(t, ਕੁੰਜੀ);
  ਜੇ (i != 0)  /* is 'key' inside array part? */
    ਵਾਪਸ i;  /* yes; that's the index */
  ਨਹੀਂਤਾਂ {
    ਅਚਲ TValue *n = getgeneric(t, ਕੁੰਜੀ, 1);
    ਜੇ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ਵਾਪਸ (i + 1) + asize;
  }
}


ਪੂਰਨ lua_H_ਅਗਲਾ (lua_State *L, ਸਾਰਣੀ *t, StkId ਕੁੰਜੀ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize = t->asize;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = findindex(L, t, s2v(ਕੁੰਜੀ), asize);  /* find original key */
  ਲਈ (; i < asize; i++) {  /* try first array part */
    lu_byte ਟੈਗ = *get_Arr_ਟੈਗ(t, i);
    ਜੇ (!tagisempty(ਟੈਗ)) {  /* a non-empty entry? */
      setivalue(s2v(ਕੁੰਜੀ), cast_int(i) + 1);
      farr2val(t, i, ਟੈਗ, s2v(ਕੁੰਜੀ + 1));
      ਵਾਪਸ 1;
    }
  }
  ਲਈ (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ਜੇ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ਗੰਢ_2 *n = gnode(t, i);
      getnodekey(L, s2v(ਕੁੰਜੀ), n);
      setobj2s(L, ਕੁੰਜੀ + 1, gval(n));
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ extraLastfree(t)	(haslastfree(t) ? ਆਕਾਰ(Limbox) : 0)

/* 'node' size in bytes */
ਸਥਿਰ size_t sizehash (ਸਾਰਣੀ *t) {
  ਵਾਪਸ cast_sizet(sizenode(t)) * ਆਕਾਰ(ਗੰਢ_2) + extraLastfree(t);
}


ਸਥਿਰ ਖਾਲੀ freehash (lua_State *L, ਸਾਰਣੀ *t) {
  ਜੇ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    ਅੱਖਰ *arr = cast_charp(t->ਗੰਢ) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ਸਥਿਰ ਪੂਰਨ insertkey (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *ਮੁੱਲ);
ਸਥਿਰ ਖਾਲੀ newcheckedkey (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *ਮੁੱਲ);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੱਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ na;
  ਪੂਰਨ deleted;
  ਬਿਨਾਨਿਸ਼ਾਨੀ nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ computesizes (Counters *ct) {
  ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ twotoi;  /* 2^i (candidate for optimal size) */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ a = 0;  /* number of elements smaller than 2^i */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ na = 0;  /* number of elements to go to array part */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  ਲਈ (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ nums = ct->nums[i];
    a += nums;
    ਜੇ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ਵਾਪਸ optimal;
}


ਸਥਿਰ ਖਾਲੀ countint (lua_Integer ਕੁੰਜੀ, Counters *ct) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ k = arrayindex(ਕੁੰਜੀ);
  ਜੇ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline ਪੂਰਨ arraykeyisempty (ਅਚਲ ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੰਜੀ) {
  ਪੂਰਨ ਟੈਗ = *get_Arr_ਟੈਗ(t, ਕੁੰਜੀ - 1);
  ਵਾਪਸ tagisempty(ਟੈਗ);
}


/*
** Count keys in array part of table 't'.
*/
ਸਥਿਰ ਖਾਲੀ numusearray (ਅਚਲ ਸਾਰਣੀ *t, Counters *ct) {
  ਪੂਰਨ lg;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ttlg;  /* 2^lg */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ause = 0;  /* summation of 'nums' */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 1;  /* index to traverse all array keys */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize = t->asize;
  /* traverse each slice */
  ਲਈ (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ lc = 0;  /* counter */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ lim = ttlg;
    ਜੇ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ਜੇ (i > lim)
        ਰੋਕੋ;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    ਲਈ (; i <= lim; i++) {
      ਜੇ (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->ਕੁੱਲ += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
ਸਥਿਰ ਖਾਲੀ numusehash (ਅਚਲ ਸਾਰਣੀ *t, Counters *ct) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = sizenode(t);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੱਲ = 0;
  ਜਦੋਂ (i--) {
    ਗੰਢ_2 *n = &t->ਗੰਢ[i];
    ਜੇ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ਨਹੀਂਤਾਂ {
      ਕੁੱਲ++;
      ਜੇ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->ਕੁੱਲ += ਕੁੱਲ;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ਸਥਿਰ size_t concretesize (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ) {
  ਜੇ (ਮਾਪ == 0)
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ  /* space for the two arrays plus an unsigned in between */
    ਵਾਪਸ ਮਾਪ * (ਆਕਾਰ(ਮੁੱਲ_2) + 1) + ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ);
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
ਸਥਿਰ ਮੁੱਲ_2 *resizearray (lua_State *L , ਸਾਰਣੀ *t,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ oldasize,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ newasize) {
  ਜੇ (oldasize == newasize)
    ਵਾਪਸ t->array;  /* nothing to be done */
  ਨਹੀਂਤਾਂ ਜੇ (newasize == 0) {  /* erasing array? */
    ਮੁੱਲ_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਨਹੀਂਤਾਂ {
    size_t newasizeb = concretesize(newasize);
    ਮੁੱਲ_2 *np = cast(ਮੁੱਲ_2 *,
                  luaM_reallocvector(L, ਨਿਰਾ, 0, newasizeb, lu_byte));
    ਜੇ (np == ਨਿਰਾ)  /* allocation error? */
      ਵਾਪਸ ਨਿਰਾ;
    np += newasize;  /* shift pointer to the end of value segment */
    ਜੇ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      ਮੁੱਲ_2 *op = t->array;  /* original array */
      ਬਿਨਾਨਿਸ਼ਾਨੀ tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ਵਾਪਸ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ਸਥਿਰ ਖਾਲੀ setnodevector (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ) {
  ਜੇ (ਮਾਪ == 0) {  /* no elements to hash part? */
    t->ਗੰਢ = cast(ਗੰਢ_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ i;
    ਪੂਰਨ lsize = luaO_ceillog2(ਮਾਪ);
    ਜੇ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    ਮਾਪ = twoto(lsize);
    ਜੇ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ਗੰਢ = luaM_newvector(L, ਮਾਪ, ਗੰਢ_2);
    ਨਹੀਂਤਾਂ {
      size_t bsize = ਮਾਪ * ਆਕਾਰ(ਗੰਢ_2) + ਆਕਾਰ(Limbox);
      ਅੱਖਰ *ਗੰਢ = luaM_newblock(L, bsize);
      t->ਗੰਢ = cast(ਗੰਢ_2 *, ਗੰਢ + ਆਕਾਰ(Limbox));
      getlastfree(t) = gnode(t, ਮਾਪ);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    ਲਈ (i = 0; i < cast_int(ਮਾਪ); i++) {
      ਗੰਢ_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ਸਥਿਰ ਖਾਲੀ reinserthash (lua_State *L, ਸਾਰਣੀ *ot, ਸਾਰਣੀ *t) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ j;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = sizenode(ot);
  ਲਈ (j = 0; j < ਮਾਪ; j++) {
    ਗੰਢ_2 *old = gnode(ot, j);
    ਜੇ (!isempty(gval(old))) {
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
ਸਥਿਰ ਖਾਲੀ exchangehashpart (ਸਾਰਣੀ *t1, ਸਾਰਣੀ *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ਗੰਢ_2 *ਗੰਢ = t1->ਗੰਢ;
  ਪੂਰਨ bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->ਗੰਢ = t2->ਗੰਢ;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->ਗੰਢ = ਗੰਢ;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ਸਥਿਰ ਖਾਲੀ reinsert_Old_ਟੁਕੜਾ (ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ oldasize,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ newasize) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ਲਈ (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte ਟੈਗ = *get_Arr_ਟੈਗ(t, i);
    ਜੇ (!tagisempty(ਟੈਗ)) {  /* a non-empty entry? */
      TValue ਕੁੰਜੀ, aux;
      setivalue(&ਕੁੰਜੀ, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, ਟੈਗ, &aux);  /* copy value into 'aux' */
      insertkey(t, &ਕੁੰਜੀ, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ਸਥਿਰ ਖਾਲੀ clear_New_ਟੁਕੜਾ (ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ oldasize, ਬਿਨਾਨਿਸ਼ਾਨੀ newasize) {
  ਲਈ (; oldasize < newasize; oldasize++)
    *get_Arr_ਟੈਗ(t, oldasize) = LUA_VEMPTY;
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
ਖਾਲੀ luaH_resize (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ newasize,
                                          ਬਿਨਾਨਿਸ਼ਾਨੀ nhsize) {
  ਸਾਰਣੀ newt;  /* to keep the new hash part */
  ਬਿਨਾਨਿਸ਼ਾਨੀ oldasize = t->asize;
  ਮੁੱਲ_2 *newarray;
  ਜੇ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ਜੇ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsert_Old_ਟੁਕੜਾ(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ਜੇ (l_unlikely(newarray == ਨਿਰਾ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ਜੇ (newarray != ਨਿਰਾ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clear_New_ਟੁਕੜਾ(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


ਖਾਲੀ luaH_resizearray (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nasize) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ਸਥਿਰ ਖਾਲੀ rehash (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ek) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ asize;  /* optimal size for array part */
  Counters ct;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ nsize;  /* size for the hash part */
  /* reset counts */
  ਲਈ (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.ਕੁੱਲ = 1;  /* count extra key */
  ਜੇ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ਜੇ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ਨਹੀਂਤਾਂ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.ਕੁੱਲ - ct.na;
  ਜੇ (ct.deleted) {  /* table has deleted entries? */
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


ਸਾਰਣੀ *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, ਆਕਾਰ(ਸਾਰਣੀ));
  ਸਾਰਣੀ *t = gco2t(o);
  t->metatable = ਨਿਰਾ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ਨਿਰਾ;
  t->asize = 0;
  setnodevector(L, t, 0);
  ਵਾਪਸ t;
}


lu_mem lua_H_ਮਾਪ (ਸਾਰਣੀ *t) {
  lu_mem sz = cast(lu_mem, ਆਕਾਰ(ਸਾਰਣੀ)) + concretesize(t->asize);
  ਜੇ (!isdummy(t))
    sz += sizehash(t);
  ਵਾਪਸ sz;
}


/*
** Frees a table.
*/
ਖਾਲੀ luaH_free (lua_State *L, ਸਾਰਣੀ *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ਸਥਿਰ ਗੰਢ_2 *getfreepos (ਸਾਰਣੀ *t) {
  ਜੇ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ਜਦੋਂ (getlastfree(t) > t->ਗੰਢ) {
      ਗੰਢ_2 *ਮੁਕਤ = --getlastfree(t);
      ਜੇ (keyisnil(ਮੁਕਤ))
        ਵਾਪਸ ਮੁਕਤ;
    }
  }
  ਨਹੀਂਤਾਂ {  /* no 'lastfree' information */
    ਬਿਨਾਨਿਸ਼ਾਨੀ i = sizenode(t);
    ਜਦੋਂ (i--) {  /* do a linear search */
      ਗੰਢ_2 *ਮੁਕਤ = gnode(t, i);
      ਜੇ (keyisnil(ਮੁਕਤ))
        ਵਾਪਸ ਮੁਕਤ;
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ਸਥਿਰ ਪੂਰਨ insertkey (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *ਮੁੱਲ) {
  ਗੰਢ_2 *mp = mainpositionTV(t, ਕੁੰਜੀ);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ਕੁੰਜੀ, 0)));
  ਜੇ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ਗੰਢ_2 *othern;
    ਗੰਢ_2 *f = getfreepos(t);  /* get a free place */
    ਜੇ (f == ਨਿਰਾ)  /* cannot find a free place? */
      ਵਾਪਸ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ਜੇ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ਜਦੋਂ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ਜੇ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ਨਹੀਂਤਾਂ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ਜੇ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ਨਹੀਂਤਾਂ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ਕੁੰਜੀ);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), ਮੁੱਲ);
  ਵਾਪਸ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ਸਥਿਰ ਖਾਲੀ newcheckedkey (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *ਮੁੱਲ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = keyinarray(t, ਕੁੰਜੀ);
  ਜੇ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, ਮੁੱਲ);  /* set value in the array */
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ ਪੂਰਾ = insertkey(t, ਕੁੰਜੀ, ਮੁੱਲ);  /* insert key in the hash part */
    lua_assert(ਪੂਰਾ);  /* it cannot fail */
    cast(ਖਾਲੀ, ਪੂਰਾ);  /* to avoid warnings */
  }
}


ਸਥਿਰ ਖਾਲੀ luaH_newkey (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ,
                                                 TValue *ਮੁੱਲ) {
  ਜੇ (!ttisnil(ਮੁੱਲ)) {  /* do not insert nil values */
    ਪੂਰਨ ਪੂਰਾ = insertkey(t, ਕੁੰਜੀ, ਮੁੱਲ);
    ਜੇ (!ਪੂਰਾ) {  /* could not find a free place? */
      rehash(L, t, ਕੁੰਜੀ);  /* grow table */
      newcheckedkey(t, ਕੁੰਜੀ, ਮੁੱਲ);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ਕੁੰਜੀ);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (ਖਾਲੀ)0, (ਖਾਲੀ)0, 1);
  }
}


ਸਥਿਰ ਅਚਲ TValue *getintfromhash (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ) {
  ਗੰਢ_2 *n = hashint(t, ਕੁੰਜੀ);
  lua_assert(!ikeyinarray(t, ਕੁੰਜੀ));
  ਲਈ (;;) {  /* check whether 'key' is somewhere in the chain */
    ਜੇ (keyisinteger(n) && keyival(n) == ਕੁੰਜੀ)
      ਵਾਪਸ gval(n);  /* that's it */
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ nx = gnext(n);
      ਜੇ (nx == 0) ਰੋਕੋ;
      n += nx;
    }
  }
  ਵਾਪਸ &absentkey;
}


ਸਥਿਰ ਪੂਰਨ hashkeyisempty (ਸਾਰਣੀ *t, lua_Unsigned ਕੁੰਜੀ) {
  ਅਚਲ TValue *val = getintfromhash(t, l_castU2S(ਕੁੰਜੀ));
  ਵਾਪਸ isempty(val);
}


ਸਥਿਰ lu_byte finishnodeget (ਅਚਲ TValue *val, TValue *res) {
  ਜੇ (!ttisnil(val)) {
    setobj(((lua_State*)ਨਿਰਾ), res, val);
  }
  ਵਾਪਸ ttypetag(val);
}


lu_byte luaH_getint (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *res) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ k = ikeyinarray(t, ਕੁੰਜੀ);
  ਜੇ (k > 0) {
    lu_byte ਟੈਗ = *get_Arr_ਟੈਗ(t, k - 1);
    ਜੇ (!tagisempty(ਟੈਗ))
      farr2val(t, k - 1, ਟੈਗ, res);
    ਵਾਪਸ ਟੈਗ;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ finishnodeget(getintfromhash(t, ਕੁੰਜੀ), res);
}


/*
** search function for short strings
*/
ਅਚਲ TValue *luaH_Hgetshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ) {
  ਗੰਢ_2 *n = hashstr(t, ਕੁੰਜੀ);
  lua_assert(strisshr(ਕੁੰਜੀ));
  ਲਈ (;;) {  /* check whether 'key' is somewhere in the chain */
    ਜੇ (keyisshrstr(n) && eqshrstr(keystrval(n), ਕੁੰਜੀ))
      ਵਾਪਸ gval(n);  /* that's it */
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ nx = gnext(n);
      ਜੇ (nx == 0)
        ਵਾਪਸ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *res) {
  ਵਾਪਸ finishnodeget(luaH_Hgetshortstr(t, ਕੁੰਜੀ), res);
}


ਸਥਿਰ ਅਚਲ TValue *Hgetlongstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ) {
  TValue ko;
  lua_assert(!strisshr(ਕੁੰਜੀ));
  setsvalue(cast(lua_State *, ਨਿਰਾ), &ko, ਕੁੰਜੀ);
  ਵਾਪਸ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ਸਥਿਰ ਅਚਲ TValue *Hgetstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ) {
  ਜੇ (strisshr(ਕੁੰਜੀ))
    ਵਾਪਸ luaH_Hgetshortstr(t, ਕੁੰਜੀ);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ Hgetlongstr(t, ਕੁੰਜੀ);
}


lu_byte luaH_getstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *res) {
  ਵਾਪਸ finishnodeget(Hgetstr(t, ਕੁੰਜੀ), res);
}


/*
** main search function
*/
lu_byte luaH_get (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *res) {
  ਅਚਲ TValue *ਥਾਂ;
  ਚੋਣ (ttypetag(ਕੁੰਜੀ)) {
    ਹਾਲਤ LUA_VSHRSTR:
      ਥਾਂ = luaH_Hgetshortstr(t, tsvalue(ਕੁੰਜੀ));
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VNUMINT:
      ਵਾਪਸ luaH_getint(t, ivalue(ਕੁੰਜੀ), res);
    ਹਾਲਤ LUA_VNIL:
      ਥਾਂ = &absentkey;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VNUMFLT: {
      lua_Integer k;
      ਜੇ (luaV_flttointeger(fltvalue(ਕੁੰਜੀ), &k, F2Ieq)) /* integral index? */
        ਵਾਪਸ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ਮੂਲ:
      ਥਾਂ = getgeneric(t, ਕੁੰਜੀ, 0);
      ਰੋਕੋ;
  }
  ਵਾਪਸ finishnodeget(ਥਾਂ, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ਸਥਿਰ ਪੂਰਨ retpsetcode (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਥਾਂ) {
  ਜੇ (isabstkey(ਥਾਂ))
    ਵਾਪਸ HNOTFOUND;  /* no slot with that key */
  ਨਹੀਂਤਾਂ  /* return node encoded */
    ਵਾਪਸ cast_int((cast(ਗੰਢ_2*, ਥਾਂ) - t->ਗੰਢ)) + HFIRSTNODE;
}


ਸਥਿਰ ਪੂਰਨ finishnodeset (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਥਾਂ, TValue *val) {
  ਜੇ (!ttisnil(ਥਾਂ)) {
    setobj(((lua_State*)ਨਿਰਾ), cast(TValue*, ਥਾਂ), val);
    ਵਾਪਸ HOK;  /* success */
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ retpsetcode(t, ਥਾਂ);
}


ਸਥਿਰ ਪੂਰਨ rawfinishnodeset (ਅਚਲ TValue *ਥਾਂ, TValue *val) {
  ਜੇ (isabstkey(ਥਾਂ))
    ਵਾਪਸ 0;  /* no slot with that key */
  ਨਹੀਂਤਾਂ {
    setobj(((lua_State*)ਨਿਰਾ), cast(TValue*, ਥਾਂ), val);
    ਵਾਪਸ 1;  /* success */
  }
}


ਪੂਰਨ luaH_psetint (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *val) {
  lua_assert(!ikeyinarray(t, ਕੁੰਜੀ));
  ਵਾਪਸ finishnodeset(t, getintfromhash(t, ਕੁੰਜੀ), val);
}


ਸਥਿਰ ਪੂਰਨ psetint (ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *val) {
  ਪੂਰਨ hres;
  luaH_fastseti(t, ਕੁੰਜੀ, val, hres);
  ਵਾਪਸ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

ਪੂਰਨ luaH_psetshortstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *val) {
  ਅਚਲ TValue *ਥਾਂ = luaH_Hgetshortstr(t, ਕੁੰਜੀ);
  ਜੇ (!ttisnil(ਥਾਂ)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ਨਿਰਾ), cast(TValue*, ਥਾਂ), val);  /* update it */
    ਵਾਪਸ HOK;  /*ਪੂਰਾ*/
  }
  ਨਹੀਂਤਾਂ ਜੇ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ਜੇ (ttisnil(val))  /* new value is nil? */
      ਵਾਪਸ HOK;  /* done (value is already nil/absent) */
    ਜੇ (isabstkey(ਥਾਂ) &&  /* key is absent? */
       !(isblack(t) && iswhite(ਕੁੰਜੀ))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ਨਿਰਾ), &tk, ਕੁੰਜੀ);
      ਜੇ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ਵਾਪਸ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ਵਾਪਸ retpsetcode(t, ਥਾਂ);
}


ਪੂਰਨ luaH_psetstr (ਸਾਰਣੀ *t, TString *ਕੁੰਜੀ, TValue *val) {
  ਜੇ (strisshr(ਕੁੰਜੀ))
    ਵਾਪਸ luaH_psetshortstr(t, ਕੁੰਜੀ, val);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ finishnodeset(t, Hgetlongstr(t, ਕੁੰਜੀ), val);
}


ਪੂਰਨ luaH_pset (ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *val) {
  ਚੋਣ (ttypetag(ਕੁੰਜੀ)) {
    ਹਾਲਤ LUA_VSHRSTR: ਵਾਪਸ luaH_psetshortstr(t, tsvalue(ਕੁੰਜੀ), val);
    ਹਾਲਤ LUA_VNUMINT: ਵਾਪਸ psetint(t, ivalue(ਕੁੰਜੀ), val);
    ਹਾਲਤ LUA_VNIL: ਵਾਪਸ HNOTFOUND;
    ਹਾਲਤ LUA_VNUMFLT: {
      lua_Integer k;
      ਜੇ (luaV_flttointeger(fltvalue(ਕੁੰਜੀ), &k, F2Ieq)) /* integral index? */
        ਵਾਪਸ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ਮੂਲ:
      ਵਾਪਸ finishnodeset(t, getgeneric(t, ਕੁੰਜੀ, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
ਖਾਲੀ luaH_finishset (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ,
                                    TValue *ਮੁੱਲ, ਪੂਰਨ hres) {
  lua_assert(hres != HOK);
  ਜੇ (hres == HNOTFOUND) {
    TValue aux;
    ਅਚਲ TValue *actk = ਕੁੰਜੀ;  /* actual key to insert */
    ਜੇ (l_unlikely(ttisnil(ਕੁੰਜੀ)))
      luaG_runerror(L, "table index is nil");
    ਨਹੀਂਤਾਂ ਜੇ (ttisfloat(ਕੁੰਜੀ)) {
      lua_ਸੰਖਿਆ f = fltvalue(ਕੁੰਜੀ);
      lua_Integer k;
      ਜੇ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ਨਹੀਂਤਾਂ ਜੇ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ਨਹੀਂਤਾਂ ਜੇ (isextstr(ਕੁੰਜੀ)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ਕੁੰਜੀ));
      setsvalue2s(L, L->ਸਿਖਰ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->ਸਿਖਰ.p - 1), ਮੁੱਲ);
      L->ਸਿਖਰ.p--;
      ਵਾਪਸ;
    }
    luaH_newkey(L, t, actk, ਮੁੱਲ);
  }
  ਨਹੀਂਤਾਂ ਜੇ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), ਮੁੱਲ);
  }
  ਨਹੀਂਤਾਂ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), ਮੁੱਲ);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
ਖਾਲੀ lua_H_ਸੈੱਟ (lua_State *L, ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ, TValue *ਮੁੱਲ) {
  ਪੂਰਨ hres = luaH_pset(t, ਕੁੰਜੀ, ਮੁੱਲ);
  ਜੇ (hres != HOK)
    luaH_finishset(L, t, ਕੁੰਜੀ, ਮੁੱਲ, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
ਖਾਲੀ luaH_setint (lua_State *L, ਸਾਰਣੀ *t, lua_Integer ਕੁੰਜੀ, TValue *ਮੁੱਲ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ik = ikeyinarray(t, ਕੁੰਜੀ);
  ਜੇ (ik > 0)
    obj2arr(t, ik - 1, ਮੁੱਲ);
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ ok = rawfinishnodeset(getintfromhash(t, ਕੁੰਜੀ), ਮੁੱਲ);
    ਜੇ (!ok) {
      TValue k;
      setivalue(&k, ਕੁੰਜੀ);
      luaH_newkey(L, t, &k, ਮੁੱਲ);
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
ਸਥਿਰ lua_Unsigned hash_ਖੋਜ (lua_State *L, ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  ਬਿਨਾਨਿਸ਼ਾਨੀ rnd = G(L)->ਬੀਜ;
  ਪੂਰਨ n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ_2 = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  ਬਿਨਾਨਿਸ਼ਾਨੀ incr = (rnd & ਨਕਾਬ_2) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ਜਦੋਂ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ਜੇ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ਨਹੀਂਤਾਂ {
      j = LUA_MAXINTEGER;
      ਜੇ (hashkeyisempty(t, j))  /* t[j] not present? */
        ਰੋਕੋ;  /* 'j' now is an absent index */
      ਨਹੀਂਤਾਂ  /* weird case */
        ਵਾਪਸ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ਜਦੋਂ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ਜੇ (hashkeyisempty(t, m)) j = m;
    ਨਹੀਂਤਾਂ i = m;
  }
  ਵਾਪਸ i;
}


ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ binsearch (ਸਾਰਣੀ *array, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j) {
  lua_assert(i <= j);
  ਜਦੋਂ (j - i > 1u) {  /*ਦੁਆਧਾਰੀ ਖੋਜ*/
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ m = (i + j) / 2;
    ਜੇ (arraykeyisempty(array, m)) j = m;
    ਨਹੀਂਤਾਂ i = m;
  }
  ਵਾਪਸ i;
}


/* return a border, saving it as a hint for next call */
ਸਥਿਰ lua_Unsigned newhint (ਸਾਰਣੀ *t, ਬਿਨਾਨਿਸ਼ਾਨੀ hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ਵਾਪਸ hint;
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
lua_Unsigned luaH_getn (lua_State *L, ਸਾਰਣੀ *t) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ asize = t->asize;
  ਜੇ (asize > 0) {  /* is there an array part? */
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ maxvicinity = 4;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਹੱਦਬੰਦੀ = *lenhint(t);  /* start with the hint */
    ਜੇ (ਹੱਦਬੰਦੀ == 0)
      ਹੱਦਬੰਦੀ = 1;  /* make limit a valid index in the array */
    ਜੇ (arraykeyisempty(t, ਹੱਦਬੰਦੀ)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      ਬਿਨਾਨਿਸ਼ਾਨੀ i;
      /* look for a border in the vicinity of the hint */
      ਲਈ (i = 0; i < maxvicinity && ਹੱਦਬੰਦੀ > 1; i++) {
        ਹੱਦਬੰਦੀ--;
        ਜੇ (!arraykeyisempty(t, ਹੱਦਬੰਦੀ))
          ਵਾਪਸ newhint(t, ਹੱਦਬੰਦੀ);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ਵਾਪਸ newhint(t, binsearch(t, 0, ਹੱਦਬੰਦੀ));
    }
    ਨਹੀਂਤਾਂ {  /* 'limit' is present in table; look for a border after it */
      ਬਿਨਾਨਿਸ਼ਾਨੀ i;
      /* look for a border in the vicinity of the hint */
      ਲਈ (i = 0; i < maxvicinity && ਹੱਦਬੰਦੀ < asize; i++) {
        ਹੱਦਬੰਦੀ++;
        ਜੇ (arraykeyisempty(t, ਹੱਦਬੰਦੀ))
          ਵਾਪਸ newhint(t, ਹੱਦਬੰਦੀ - 1);  /* 'limit - 1' is a border */
      }
      ਜੇ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ਵਾਪਸ newhint(t, binsearch(t, ਹੱਦਬੰਦੀ, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ਜੇ (isdummy(t) || hashkeyisempty(t, asize + 1))
    ਵਾਪਸ asize;  /* 'asize + 1' is empty */
  ਨਹੀਂਤਾਂ  /* 'asize + 1' is also non empty */
    ਵਾਪਸ hash_ਖੋਜ(L, t, asize);
}



#ਜੇ defined(LUA_DEBUG)

/* export this function for the test library */

ਗੰਢ_2 *luaH_mainposition (ਅਚਲ ਸਾਰਣੀ *t, ਅਚਲ TValue *ਕੁੰਜੀ) {
  ਵਾਪਸ mainpositionTV(t, ਕੁੰਜੀ);
}

#ਜੇ_ਅੰਤ
