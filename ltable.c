/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ltable_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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
#ವ್ಯಾಖ್ಯಾನಿಸು LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ { ಗಂಟು_2 *dummy; ಗಂಟು_2 follows_p_ಗಂಟು; } Limbox_aux;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ {
  ಗಂಟು_2 *lastfree;
  ಅಕ್ಷರ padding[offsetof(Limbox_aux, follows_p_ಗಂಟು)];
} Limbox;

#ವ್ಯಾಖ್ಯಾನಿಸು haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#ವ್ಯಾಖ್ಯಾನಿಸು getlastfree(t)     ((cast(Limbox *, (t)->ಗಂಟು) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXABITS	(l_numbits(ಪೂರ್ಣಾಂಕ) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXASIZEB	(MAX_SIZET/(ಗಾತ್ರ(ಮೌಲ್ಯ_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXHSIZE	lua_M_ಎಲ್ಲೆ_N(1 << MAXHBITS, ಗಂಟು_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#ವ್ಯಾಖ್ಯಾನಿಸು hashstr(t,str)		hashpow2(t, (str)->hash)
#ವ್ಯಾಖ್ಯಾನಿಸು hashboolean(t,p)	hashpow2(t, p)


#ವ್ಯಾಖ್ಯಾನಿಸು hashpointer(t,p)	hashmod(t, point2uint(p))


#ವ್ಯಾಖ್ಯಾನಿಸು dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಗಂಟು_2 dummynode_ = {
  {{ಖಾಲಿ}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ಖಾಲಿ}}  /* key type, next, and key value */
};


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ಸ್ಥಿರ ಗಂಟು_2 *hashint (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ಆದರೆ (ui <= cast_uint(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ))
    ಹಿಂತಿರುಗಿ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ hashmod(t, ui);
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
#ಆದರೆ !defined(l_hashfloat)
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ l_hashfloat (lua_ಸಂಖ್ಯೆ n) {
  ಪೂರ್ಣಾಂಕ i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_ಅಂಕೆ(INT_MIN);
  ಆದರೆ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_ಅಂಕೆ(HUGE_VAL));
    ಹಿಂತಿರುಗಿ 0;
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* normal case */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ u = cast_uint(i) + cast_uint(ni);
    ಹಿಂತಿರುಗಿ (u <= cast_uint(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ) ? u : ~u);
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ಸ್ಥಿರ ಗಂಟು_2 *mainpositionTV (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ) {
  ಆಯ್ಕೆ (ttypetag(ಕೀಲಿ)) {
    ಸ್ಥಿತಿ LUA_VNUMINT: {
      lua_Integer i = ivalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashint(t, i);
    }
    ಸ್ಥಿತಿ LUA_VNUMFLT: {
      lua_ಸಂಖ್ಯೆ n = fltvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashmod(t, l_hashfloat(n));
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR: {
      TString *ts = tsvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashstr(t, ts);
    }
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      TString *ts = tsvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashpow2(t, luaS_hashlongstr(ts));
    }
    ಸ್ಥಿತಿ LUA_VFALSE:
      ಹಿಂತಿರುಗಿ hashboolean(t, 0);
    ಸ್ಥಿತಿ LUA_VTRUE:
      ಹಿಂತಿರುಗಿ hashboolean(t, 1);
    ಸ್ಥಿತಿ LUA_VLIGHTUSERDATA: {
      ಶೂನ್ಯ *p = pvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashpointer(t, p);
    }
    ಸ್ಥಿತಿ LUA_VLCF: {
      lua_CFunction f = fvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashpointer(t, f);
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      GCObject *o = gcvalue(ಕೀಲಿ);
      ಹಿಂತಿರುಗಿ hashpointer(t, o);
    }
  }
}


l_sinline ಗಂಟು_2 *mainpositionfromnode (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, ಗಂಟು_2 *nd) {
  TValue ಕೀಲಿ;
  getnodekey(cast(lua_State *, ಖಾಲಿ), &ಕೀಲಿ, nd);
  ಹಿಂತಿರುಗಿ mainpositionTV(t, &ಕೀಲಿ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ equalkey (ಸ್ಥಿರಾಂಕ TValue *k1, ಸ್ಥಿರಾಂಕ ಗಂಟು_2 *n2, ಪೂರ್ಣಾಂಕ deadok) {
  ಆದರೆ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ಆದರೆ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ಹಿಂತಿರುಗಿ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ಹಿಂತಿರುಗಿ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ಇಲ್ಲದಿದ್ದರೆ
     ಹಿಂತಿರುಗಿ 0;  /* otherwise, different variants cannot be equal */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* equal variants */
    ಆಯ್ಕೆ (keytt(n2)) {
      ಸ್ಥಿತಿ LUA_VNIL: ಸ್ಥಿತಿ LUA_VFALSE: ಸ್ಥಿತಿ LUA_VTRUE:
        ಹಿಂತಿರುಗಿ 1;
      ಸ್ಥಿತಿ LUA_VNUMINT:
        ಹಿಂತಿರುಗಿ (ivalue(k1) == keyival(n2));
      ಸ್ಥಿತಿ LUA_VNUMFLT:
        ಹಿಂತಿರುಗಿ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      ಸ್ಥಿತಿ LUA_VLIGHTUSERDATA:
        ಹಿಂತಿರುಗಿ pvalue(k1) == pvalueraw(keyval(n2));
      ಸ್ಥಿತಿ LUA_VLCF:
        ಹಿಂತಿರುಗಿ fvalue(k1) == fvalueraw(keyval(n2));
      ಸ್ಥಿತಿ ctb(LUA_VLNGSTR):
        ಹಿಂತಿರುಗಿ luaS_eqstr(tsvalue(k1), keystrval(n2));
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ TValue *getgeneric (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ deadok) {
  ಗಂಟು_2 *n = mainpositionTV(t, ಕೀಲಿ);
  ಗಾಗಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಆದರೆ (equalkey(ಕೀಲಿ, n, deadok))
      ಹಿಂತಿರುಗಿ gval(n);  /* that's it */
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ nx = gnext(n);
      ಆದರೆ (nx == 0)
        ಹಿಂತಿರುಗಿ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ checkrange (lua_Integer k, ಚಿಹ್ನೆರಹಿತ ಎಲ್ಲೆ) {
  ಹಿಂತಿರುಗಿ (l_castS2U(k) - 1u < ಎಲ್ಲೆ) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ keyinarray (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ) {
  ಹಿಂತಿರುಗಿ (ttisinteger(ಕೀಲಿ)) ? ikeyinarray(t, ivalue(ಕೀಲಿ)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ findindex (lua_State *L, ಕೋಷ್ಟಕ *t, TValue *ಕೀಲಿ,
                               ಚಿಹ್ನೆರಹಿತ asize) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ (ttisnil(ಕೀಲಿ)) ಹಿಂತಿರುಗಿ 0;  /* first iteration */
  i = keyinarray(t, ಕೀಲಿ);
  ಆದರೆ (i != 0)  /* is 'key' inside array part? */
    ಹಿಂತಿರುಗಿ i;  /* yes; that's the index */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ TValue *n = getgeneric(t, ಕೀಲಿ, 1);
    ಆದರೆ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ಹಿಂತಿರುಗಿ (i + 1) + asize;
  }
}


ಪೂರ್ಣಾಂಕ lua_H_ಮುಂದಿನ (lua_State *L, ಕೋಷ್ಟಕ *t, StkId ಕೀಲಿ) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize = t->asize;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = findindex(L, t, s2v(ಕೀಲಿ), asize);  /* find original key */
  ಗಾಗಿ (; i < asize; i++) {  /* try first array part */
    lu_byte ಟ್ಯಾಗ್ = *get_Arr_ಟ್ಯಾಗ್(t, i);
    ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್)) {  /* a non-empty entry? */
      setivalue(s2v(ಕೀಲಿ), cast_int(i) + 1);
      farr2val(t, i, ಟ್ಯಾಗ್, s2v(ಕೀಲಿ + 1));
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಗಾಗಿ (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ಆದರೆ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ಗಂಟು_2 *n = gnode(t, i);
      getnodekey(L, s2v(ಕೀಲಿ), n);
      setobj2s(L, ಕೀಲಿ + 1, gval(n));
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#ವ್ಯಾಖ್ಯಾನಿಸು extraLastfree(t)	(haslastfree(t) ? ಗಾತ್ರ(Limbox) : 0)

/* 'node' size in bytes */
ಸ್ಥಿರ size_t sizehash (ಕೋಷ್ಟಕ *t) {
  ಹಿಂತಿರುಗಿ cast_sizet(sizenode(t)) * ಗಾತ್ರ(ಗಂಟು_2) + extraLastfree(t);
}


ಸ್ಥಿರ ಶೂನ್ಯ freehash (lua_State *L, ಕೋಷ್ಟಕ *t) {
  ಆದರೆ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    ಅಕ್ಷರ *arr = cast_charp(t->ಗಂಟು) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ insertkey (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ newcheckedkey (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  ಚಿಹ್ನೆರಹಿತ ಒಟ್ಟು;
  ಚಿಹ್ನೆರಹಿತ na;
  ಪೂರ್ಣಾಂಕ deleted;
  ಚಿಹ್ನೆರಹಿತ nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ computesizes (Counters *ct) {
  ಪೂರ್ಣಾಂಕ i;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ twotoi;  /* 2^i (candidate for optimal size) */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ a = 0;  /* number of elements smaller than 2^i */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ na = 0;  /* number of elements to go to array part */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  ಗಾಗಿ (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    ಚಿಹ್ನೆರಹಿತ nums = ct->nums[i];
    a += nums;
    ಆದರೆ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ಹಿಂತಿರುಗಿ optimal;
}


ಸ್ಥಿರ ಶೂನ್ಯ countint (lua_Integer ಕೀಲಿ, Counters *ct) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ k = arrayindex(ಕೀಲಿ);
  ಆದರೆ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline ಪೂರ್ಣಾಂಕ arraykeyisempty (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ ಕೀಲಿ) {
  ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = *get_Arr_ಟ್ಯಾಗ್(t, ಕೀಲಿ - 1);
  ಹಿಂತಿರುಗಿ tagisempty(ಟ್ಯಾಗ್);
}


/*
** Count keys in array part of table 't'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ numusearray (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, Counters *ct) {
  ಪೂರ್ಣಾಂಕ lg;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ttlg;  /* 2^lg */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ause = 0;  /* summation of 'nums' */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 1;  /* index to traverse all array keys */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize = t->asize;
  /* traverse each slice */
  ಗಾಗಿ (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ lc = 0;  /* counter */
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ lim = ttlg;
    ಆದರೆ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ಆದರೆ (i > lim)
        ನಿಲ್ಲಿಸು;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    ಗಾಗಿ (; i <= lim; i++) {
      ಆದರೆ (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->ಒಟ್ಟು += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
ಸ್ಥಿರ ಶೂನ್ಯ numusehash (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, Counters *ct) {
  ಚಿಹ್ನೆರಹಿತ i = sizenode(t);
  ಚಿಹ್ನೆರಹಿತ ಒಟ್ಟು = 0;
  ಆಗ (i--) {
    ಗಂಟು_2 *n = &t->ಗಂಟು[i];
    ಆದರೆ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      ಒಟ್ಟು++;
      ಆದರೆ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->ಒಟ್ಟು += ಒಟ್ಟು;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ಸ್ಥಿರ size_t concretesize (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2) {
  ಆದರೆ (ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ  /* space for the two arrays plus an unsigned in between */
    ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2 * (ಗಾತ್ರ(ಮೌಲ್ಯ_2) + 1) + ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ);
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
ಸ್ಥಿರ ಮೌಲ್ಯ_2 *resizearray (lua_State *L , ಕೋಷ್ಟಕ *t,
                               ಚಿಹ್ನೆರಹಿತ oldasize,
                               ಚಿಹ್ನೆರಹಿತ newasize) {
  ಆದರೆ (oldasize == newasize)
    ಹಿಂತಿರುಗಿ t->array;  /* nothing to be done */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (newasize == 0) {  /* erasing array? */
    ಮೌಲ್ಯ_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    size_t newasizeb = concretesize(newasize);
    ಮೌಲ್ಯ_2 *np = cast(ಮೌಲ್ಯ_2 *,
                  luaM_reallocvector(L, ಖಾಲಿ, 0, newasizeb, lu_byte));
    ಆದರೆ (np == ಖಾಲಿ)  /* allocation error? */
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    np += newasize;  /* shift pointer to the end of value segment */
    ಆದರೆ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      ಮೌಲ್ಯ_2 *op = t->array;  /* original array */
      ಚಿಹ್ನೆರಹಿತ tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      ಸ್ಮರಣೆ_ನಕಲು(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ಹಿಂತಿರುಗಿ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setnodevector (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2) {
  ಆದರೆ (ಗಾತ್ರ_2 == 0) {  /* no elements to hash part? */
    t->ಗಂಟು = cast(ಗಂಟು_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ lsize = luaO_ceillog2(ಗಾತ್ರ_2);
    ಆದರೆ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    ಗಾತ್ರ_2 = twoto(lsize);
    ಆದರೆ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ಗಂಟು = luaM_newvector(L, ಗಾತ್ರ_2, ಗಂಟು_2);
    ಇಲ್ಲದಿದ್ದರೆ {
      size_t bsize = ಗಾತ್ರ_2 * ಗಾತ್ರ(ಗಂಟು_2) + ಗಾತ್ರ(Limbox);
      ಅಕ್ಷರ *ಗಂಟು = luaM_newblock(L, bsize);
      t->ಗಂಟು = cast(ಗಂಟು_2 *, ಗಂಟು + ಗಾತ್ರ(Limbox));
      getlastfree(t) = gnode(t, ಗಾತ್ರ_2);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    ಗಾಗಿ (i = 0; i < cast_int(ಗಾತ್ರ_2); i++) {
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
ಸ್ಥಿರ ಶೂನ್ಯ reinserthash (lua_State *L, ಕೋಷ್ಟಕ *ot, ಕೋಷ್ಟಕ *t) {
  ಚಿಹ್ನೆರಹಿತ j;
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = sizenode(ot);
  ಗಾಗಿ (j = 0; j < ಗಾತ್ರ_2; j++) {
    ಗಂಟು_2 *old = gnode(ot, j);
    ಆದರೆ (!isempty(gval(old))) {
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
ಸ್ಥಿರ ಶೂನ್ಯ exchangehashpart (ಕೋಷ್ಟಕ *t1, ಕೋಷ್ಟಕ *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ಗಂಟು_2 *ಗಂಟು = t1->ಗಂಟು;
  ಪೂರ್ಣಾಂಕ bitdummy1 = t1->flags & BITDUMMY;
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
ಸ್ಥಿರ ಶೂನ್ಯ reinsert_Old_ತುಂಡು (ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ oldasize,
                                        ಚಿಹ್ನೆರಹಿತ newasize) {
  ಚಿಹ್ನೆರಹಿತ i;
  ಗಾಗಿ (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte ಟ್ಯಾಗ್ = *get_Arr_ಟ್ಯಾಗ್(t, i);
    ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್)) {  /* a non-empty entry? */
      TValue ಕೀಲಿ, aux;
      setivalue(&ಕೀಲಿ, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, ಟ್ಯಾಗ್, &aux);  /* copy value into 'aux' */
      insertkey(t, &ಕೀಲಿ, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ಸ್ಥಿರ ಶೂನ್ಯ clear_New_ತುಂಡು (ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ oldasize, ಚಿಹ್ನೆರಹಿತ newasize) {
  ಗಾಗಿ (; oldasize < newasize; oldasize++)
    *get_Arr_ಟ್ಯಾಗ್(t, oldasize) = LUA_VEMPTY;
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
ಶೂನ್ಯ luaH_resize (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ newasize,
                                          ಚಿಹ್ನೆರಹಿತ nhsize) {
  ಕೋಷ್ಟಕ newt;  /* to keep the new hash part */
  ಚಿಹ್ನೆರಹಿತ oldasize = t->asize;
  ಮೌಲ್ಯ_2 *newarray;
  ಆದರೆ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ಆದರೆ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsert_Old_ತುಂಡು(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ಆದರೆ (l_unlikely(newarray == ಖಾಲಿ && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ಆದರೆ (newarray != ಖಾಲಿ)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clear_New_ತುಂಡು(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


ಶೂನ್ಯ luaH_resizearray (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nasize) {
  ಚಿಹ್ನೆರಹಿತ nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rehash (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ek) {
  ಚಿಹ್ನೆರಹಿತ asize;  /* optimal size for array part */
  Counters ct;
  ಚಿಹ್ನೆರಹಿತ i;
  ಚಿಹ್ನೆರಹಿತ nsize;  /* size for the hash part */
  /* reset counts */
  ಗಾಗಿ (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.ಒಟ್ಟು = 1;  /* count extra key */
  ಆದರೆ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ಆದರೆ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.ಒಟ್ಟು - ct.na;
  ಆದರೆ (ct.deleted) {  /* table has deleted entries? */
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


ಕೋಷ್ಟಕ *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, ಗಾತ್ರ(ಕೋಷ್ಟಕ));
  ಕೋಷ್ಟಕ *t = gco2t(o);
  t->metatable = ಖಾಲಿ;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ಖಾಲಿ;
  t->asize = 0;
  setnodevector(L, t, 0);
  ಹಿಂತಿರುಗಿ t;
}


lu_mem lua_H_ಗಾತ್ರ (ಕೋಷ್ಟಕ *t) {
  lu_mem sz = cast(lu_mem, ಗಾತ್ರ(ಕೋಷ್ಟಕ)) + concretesize(t->asize);
  ಆದರೆ (!isdummy(t))
    sz += sizehash(t);
  ಹಿಂತಿರುಗಿ sz;
}


/*
** Frees a table.
*/
ಶೂನ್ಯ luaH_free (lua_State *L, ಕೋಷ್ಟಕ *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ಸ್ಥಿರ ಗಂಟು_2 *getfreepos (ಕೋಷ್ಟಕ *t) {
  ಆದರೆ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ಆಗ (getlastfree(t) > t->ಗಂಟು) {
      ಗಂಟು_2 *ಬಿಡುಗಡೆ = --getlastfree(t);
      ಆದರೆ (keyisnil(ಬಿಡುಗಡೆ))
        ಹಿಂತಿರುಗಿ ಬಿಡುಗಡೆ;
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* no 'lastfree' information */
    ಚಿಹ್ನೆರಹಿತ i = sizenode(t);
    ಆಗ (i--) {  /* do a linear search */
      ಗಂಟು_2 *ಬಿಡುಗಡೆ = gnode(t, i);
      ಆದರೆ (keyisnil(ಬಿಡುಗಡೆ))
        ಹಿಂತಿರುಗಿ ಬಿಡುಗಡೆ;
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ insertkey (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಗಂಟು_2 *mp = mainpositionTV(t, ಕೀಲಿ);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ಕೀಲಿ, 0)));
  ಆದರೆ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ಗಂಟು_2 *othern;
    ಗಂಟು_2 *f = getfreepos(t);  /* get a free place */
    ಆದರೆ (f == ಖಾಲಿ)  /* cannot find a free place? */
      ಹಿಂತಿರುಗಿ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ಆದರೆ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ಆಗ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ಆದರೆ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ಆದರೆ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ಇಲ್ಲದಿದ್ದರೆ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ಕೀಲಿ);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), ಮೌಲ್ಯ);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ಸ್ಥಿರ ಶೂನ್ಯ newcheckedkey (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಚಿಹ್ನೆರಹಿತ i = keyinarray(t, ಕೀಲಿ);
  ಆದರೆ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, ಮೌಲ್ಯ);  /* set value in the array */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ ಮುಗಿದ = insertkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);  /* insert key in the hash part */
    lua_assert(ಮುಗಿದ);  /* it cannot fail */
    cast(ಶೂನ್ಯ, ಮುಗಿದ);  /* to avoid warnings */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ luaH_newkey (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ,
                                                 TValue *ಮೌಲ್ಯ) {
  ಆದರೆ (!ttisnil(ಮೌಲ್ಯ)) {  /* do not insert nil values */
    ಪೂರ್ಣಾಂಕ ಮುಗಿದ = insertkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);
    ಆದರೆ (!ಮುಗಿದ) {  /* could not find a free place? */
      rehash(L, t, ಕೀಲಿ);  /* grow table */
      newcheckedkey(t, ಕೀಲಿ, ಮೌಲ್ಯ);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ಕೀಲಿ);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (ಶೂನ್ಯ)0, (ಶೂನ್ಯ)0, 1);
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ TValue *getintfromhash (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ) {
  ಗಂಟು_2 *n = hashint(t, ಕೀಲಿ);
  lua_assert(!ikeyinarray(t, ಕೀಲಿ));
  ಗಾಗಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಆದರೆ (keyisinteger(n) && keyival(n) == ಕೀಲಿ)
      ಹಿಂತಿರುಗಿ gval(n);  /* that's it */
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ nx = gnext(n);
      ಆದರೆ (nx == 0) ನಿಲ್ಲಿಸು;
      n += nx;
    }
  }
  ಹಿಂತಿರುಗಿ &absentkey;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hashkeyisempty (ಕೋಷ್ಟಕ *t, lua_Unsigned ಕೀಲಿ) {
  ಸ್ಥಿರಾಂಕ TValue *val = getintfromhash(t, l_castU2S(ಕೀಲಿ));
  ಹಿಂತಿರುಗಿ isempty(val);
}


ಸ್ಥಿರ lu_byte finishnodeget (ಸ್ಥಿರಾಂಕ TValue *val, TValue *res) {
  ಆದರೆ (!ttisnil(val)) {
    setobj(((lua_State*)ಖಾಲಿ), res, val);
  }
  ಹಿಂತಿರುಗಿ ttypetag(val);
}


lu_byte luaH_getint (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *res) {
  ಚಿಹ್ನೆರಹಿತ k = ikeyinarray(t, ಕೀಲಿ);
  ಆದರೆ (k > 0) {
    lu_byte ಟ್ಯಾಗ್ = *get_Arr_ಟ್ಯಾಗ್(t, k - 1);
    ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್))
      farr2val(t, k - 1, ಟ್ಯಾಗ್, res);
    ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ finishnodeget(getintfromhash(t, ಕೀಲಿ), res);
}


/*
** search function for short strings
*/
ಸ್ಥಿರಾಂಕ TValue *luaH_Hgetshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ) {
  ಗಂಟು_2 *n = hashstr(t, ಕೀಲಿ);
  lua_assert(strisshr(ಕೀಲಿ));
  ಗಾಗಿ (;;) {  /* check whether 'key' is somewhere in the chain */
    ಆದರೆ (keyisshrstr(n) && eqshrstr(keystrval(n), ಕೀಲಿ))
      ಹಿಂತಿರುಗಿ gval(n);  /* that's it */
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ nx = gnext(n);
      ಆದರೆ (nx == 0)
        ಹಿಂತಿರುಗಿ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *res) {
  ಹಿಂತಿರುಗಿ finishnodeget(luaH_Hgetshortstr(t, ಕೀಲಿ), res);
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ TValue *Hgetlongstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ) {
  TValue ko;
  lua_assert(!strisshr(ಕೀಲಿ));
  setsvalue(cast(lua_State *, ಖಾಲಿ), &ko, ಕೀಲಿ);
  ಹಿಂತಿರುಗಿ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ TValue *Hgetstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ) {
  ಆದರೆ (strisshr(ಕೀಲಿ))
    ಹಿಂತಿರುಗಿ luaH_Hgetshortstr(t, ಕೀಲಿ);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ Hgetlongstr(t, ಕೀಲಿ);
}


lu_byte luaH_getstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *res) {
  ಹಿಂತಿರುಗಿ finishnodeget(Hgetstr(t, ಕೀಲಿ), res);
}


/*
** main search function
*/
lu_byte luaH_get (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *res) {
  ಸ್ಥಿರಾಂಕ TValue *ಸ್ಥಾನ;
  ಆಯ್ಕೆ (ttypetag(ಕೀಲಿ)) {
    ಸ್ಥಿತಿ LUA_VSHRSTR:
      ಸ್ಥಾನ = luaH_Hgetshortstr(t, tsvalue(ಕೀಲಿ));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VNUMINT:
      ಹಿಂತಿರುಗಿ luaH_getint(t, ivalue(ಕೀಲಿ), res);
    ಸ್ಥಿತಿ LUA_VNIL:
      ಸ್ಥಾನ = &absentkey;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VNUMFLT: {
      lua_Integer k;
      ಆದರೆ (luaV_flttointeger(fltvalue(ಕೀಲಿ), &k, F2Ieq)) /* integral index? */
        ಹಿಂತಿರುಗಿ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಸ್ಥಾನ = getgeneric(t, ಕೀಲಿ, 0);
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ finishnodeget(ಸ್ಥಾನ, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ retpsetcode (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಸ್ಥಾನ) {
  ಆದರೆ (isabstkey(ಸ್ಥಾನ))
    ಹಿಂತಿರುಗಿ HNOTFOUND;  /* no slot with that key */
  ಇಲ್ಲದಿದ್ದರೆ  /* return node encoded */
    ಹಿಂತಿರುಗಿ cast_int((cast(ಗಂಟು_2*, ಸ್ಥಾನ) - t->ಗಂಟು)) + HFIRSTNODE;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finishnodeset (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಸ್ಥಾನ, TValue *val) {
  ಆದರೆ (!ttisnil(ಸ್ಥಾನ)) {
    setobj(((lua_State*)ಖಾಲಿ), cast(TValue*, ಸ್ಥಾನ), val);
    ಹಿಂತಿರುಗಿ HOK;  /* success */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ retpsetcode(t, ಸ್ಥಾನ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rawfinishnodeset (ಸ್ಥಿರಾಂಕ TValue *ಸ್ಥಾನ, TValue *val) {
  ಆದರೆ (isabstkey(ಸ್ಥಾನ))
    ಹಿಂತಿರುಗಿ 0;  /* no slot with that key */
  ಇಲ್ಲದಿದ್ದರೆ {
    setobj(((lua_State*)ಖಾಲಿ), cast(TValue*, ಸ್ಥಾನ), val);
    ಹಿಂತಿರುಗಿ 1;  /* success */
  }
}


ಪೂರ್ಣಾಂಕ luaH_psetint (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *val) {
  lua_assert(!ikeyinarray(t, ಕೀಲಿ));
  ಹಿಂತಿರುಗಿ finishnodeset(t, getintfromhash(t, ಕೀಲಿ), val);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ psetint (ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *val) {
  ಪೂರ್ಣಾಂಕ hres;
  luaH_fastseti(t, ಕೀಲಿ, val, hres);
  ಹಿಂತಿರುಗಿ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

ಪೂರ್ಣಾಂಕ luaH_psetshortstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *val) {
  ಸ್ಥಿರಾಂಕ TValue *ಸ್ಥಾನ = luaH_Hgetshortstr(t, ಕೀಲಿ);
  ಆದರೆ (!ttisnil(ಸ್ಥಾನ)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ಖಾಲಿ), cast(TValue*, ಸ್ಥಾನ), val);  /* update it */
    ಹಿಂತಿರುಗಿ HOK;  /*ಮುಗಿದ*/
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ಆದರೆ (ttisnil(val))  /* new value is nil? */
      ಹಿಂತಿರುಗಿ HOK;  /* done (value is already nil/absent) */
    ಆದರೆ (isabstkey(ಸ್ಥಾನ) &&  /* key is absent? */
       !(isblack(t) && iswhite(ಕೀಲಿ))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ಖಾಲಿ), &tk, ಕೀಲಿ);
      ಆದರೆ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ಹಿಂತಿರುಗಿ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ಹಿಂತಿರುಗಿ retpsetcode(t, ಸ್ಥಾನ);
}


ಪೂರ್ಣಾಂಕ luaH_psetstr (ಕೋಷ್ಟಕ *t, TString *ಕೀಲಿ, TValue *val) {
  ಆದರೆ (strisshr(ಕೀಲಿ))
    ಹಿಂತಿರುಗಿ luaH_psetshortstr(t, ಕೀಲಿ, val);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ finishnodeset(t, Hgetlongstr(t, ಕೀಲಿ), val);
}


ಪೂರ್ಣಾಂಕ luaH_pset (ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *val) {
  ಆಯ್ಕೆ (ttypetag(ಕೀಲಿ)) {
    ಸ್ಥಿತಿ LUA_VSHRSTR: ಹಿಂತಿರುಗಿ luaH_psetshortstr(t, tsvalue(ಕೀಲಿ), val);
    ಸ್ಥಿತಿ LUA_VNUMINT: ಹಿಂತಿರುಗಿ psetint(t, ivalue(ಕೀಲಿ), val);
    ಸ್ಥಿತಿ LUA_VNIL: ಹಿಂತಿರುಗಿ HNOTFOUND;
    ಸ್ಥಿತಿ LUA_VNUMFLT: {
      lua_Integer k;
      ಆದರೆ (luaV_flttointeger(fltvalue(ಕೀಲಿ), &k, F2Ieq)) /* integral index? */
        ಹಿಂತಿರುಗಿ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ finishnodeset(t, getgeneric(t, ಕೀಲಿ, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
ಶೂನ್ಯ luaH_finishset (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ,
                                    TValue *ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ hres) {
  lua_assert(hres != HOK);
  ಆದರೆ (hres == HNOTFOUND) {
    TValue aux;
    ಸ್ಥಿರಾಂಕ TValue *actk = ಕೀಲಿ;  /* actual key to insert */
    ಆದರೆ (l_unlikely(ttisnil(ಕೀಲಿ)))
      luaG_runerror(L, "table index is nil");
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisfloat(ಕೀಲಿ)) {
      lua_ಸಂಖ್ಯೆ f = fltvalue(ಕೀಲಿ);
      lua_Integer k;
      ಆದರೆ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isextstr(ಕೀಲಿ)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ಕೀಲಿ));
      setsvalue2s(L, L->ತುದಿ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->ತುದಿ.p - 1), ಮೌಲ್ಯ);
      L->ತುದಿ.p--;
      ಹಿಂತಿರುಗಿ;
    }
    luaH_newkey(L, t, actk, ಮೌಲ್ಯ);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), ಮೌಲ್ಯ);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), ಮೌಲ್ಯ);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
ಶೂನ್ಯ lua_H_ಗಣ (lua_State *L, ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಪೂರ್ಣಾಂಕ hres = luaH_pset(t, ಕೀಲಿ, ಮೌಲ್ಯ);
  ಆದರೆ (hres != HOK)
    luaH_finishset(L, t, ಕೀಲಿ, ಮೌಲ್ಯ, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
ಶೂನ್ಯ luaH_setint (lua_State *L, ಕೋಷ್ಟಕ *t, lua_Integer ಕೀಲಿ, TValue *ಮೌಲ್ಯ) {
  ಚಿಹ್ನೆರಹಿತ ik = ikeyinarray(t, ಕೀಲಿ);
  ಆದರೆ (ik > 0)
    obj2arr(t, ik - 1, ಮೌಲ್ಯ);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ ok = rawfinishnodeset(getintfromhash(t, ಕೀಲಿ), ಮೌಲ್ಯ);
    ಆದರೆ (!ok) {
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
ಸ್ಥಿರ lua_Unsigned hash_ಹುಡುಕು (lua_State *L, ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  ಚಿಹ್ನೆರಹಿತ rnd = G(L)->ಬೀಜ;
  ಪೂರ್ಣಾಂಕ n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ_2 = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  ಚಿಹ್ನೆರಹಿತ incr = (rnd & ಮುಖವಾಡ_2) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ಆಗ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ಆದರೆ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      j = LUA_MAXINTEGER;
      ಆದರೆ (hashkeyisempty(t, j))  /* t[j] not present? */
        ನಿಲ್ಲಿಸು;  /* 'j' now is an absent index */
      ಇಲ್ಲದಿದ್ದರೆ  /* weird case */
        ಹಿಂತಿರುಗಿ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ಆಗ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ಆದರೆ (hashkeyisempty(t, m)) j = m;
    ಇಲ್ಲದಿದ್ದರೆ i = m;
  }
  ಹಿಂತಿರುಗಿ i;
}


ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ binsearch (ಕೋಷ್ಟಕ *array, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j) {
  lua_assert(i <= j);
  ಆಗ (j - i > 1u) {  /*ದ್ವಿಮಾನ ಹುಡುಕು*/
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ m = (i + j) / 2;
    ಆದರೆ (arraykeyisempty(array, m)) j = m;
    ಇಲ್ಲದಿದ್ದರೆ i = m;
  }
  ಹಿಂತಿರುಗಿ i;
}


/* return a border, saving it as a hint for next call */
ಸ್ಥಿರ lua_Unsigned newhint (ಕೋಷ್ಟಕ *t, ಚಿಹ್ನೆರಹಿತ hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ಹಿಂತಿರುಗಿ hint;
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
lua_Unsigned luaH_getn (lua_State *L, ಕೋಷ್ಟಕ *t) {
  ಚಿಹ್ನೆರಹಿತ asize = t->asize;
  ಆದರೆ (asize > 0) {  /* is there an array part? */
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ maxvicinity = 4;
    ಚಿಹ್ನೆರಹಿತ ಎಲ್ಲೆ = *lenhint(t);  /* start with the hint */
    ಆದರೆ (ಎಲ್ಲೆ == 0)
      ಎಲ್ಲೆ = 1;  /* make limit a valid index in the array */
    ಆದರೆ (arraykeyisempty(t, ಎಲ್ಲೆ)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      ಚಿಹ್ನೆರಹಿತ i;
      /* look for a border in the vicinity of the hint */
      ಗಾಗಿ (i = 0; i < maxvicinity && ಎಲ್ಲೆ > 1; i++) {
        ಎಲ್ಲೆ--;
        ಆದರೆ (!arraykeyisempty(t, ಎಲ್ಲೆ))
          ಹಿಂತಿರುಗಿ newhint(t, ಎಲ್ಲೆ);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ಹಿಂತಿರುಗಿ newhint(t, binsearch(t, 0, ಎಲ್ಲೆ));
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* 'limit' is present in table; look for a border after it */
      ಚಿಹ್ನೆರಹಿತ i;
      /* look for a border in the vicinity of the hint */
      ಗಾಗಿ (i = 0; i < maxvicinity && ಎಲ್ಲೆ < asize; i++) {
        ಎಲ್ಲೆ++;
        ಆದರೆ (arraykeyisempty(t, ಎಲ್ಲೆ))
          ಹಿಂತಿರುಗಿ newhint(t, ಎಲ್ಲೆ - 1);  /* 'limit - 1' is a border */
      }
      ಆದರೆ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ಹಿಂತಿರುಗಿ newhint(t, binsearch(t, ಎಲ್ಲೆ, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ಆದರೆ (isdummy(t) || hashkeyisempty(t, asize + 1))
    ಹಿಂತಿರುಗಿ asize;  /* 'asize + 1' is empty */
  ಇಲ್ಲದಿದ್ದರೆ  /* 'asize + 1' is also non empty */
    ಹಿಂತಿರುಗಿ hash_ಹುಡುಕು(L, t, asize);
}



#ಆದರೆ defined(LUA_DEBUG)

/* export this function for the test library */

ಗಂಟು_2 *luaH_mainposition (ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t, ಸ್ಥಿರಾಂಕ TValue *ಕೀಲಿ) {
  ಹಿಂತಿರುಗಿ mainpositionTV(t, ಕೀಲಿ);
}

#ಆದರೆ_ಅಂತ್ಯ
